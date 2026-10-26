/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "coap_service/coap_service.h"

#include "aliro/types.h"
#include "doorlock/utils/utils.h"
#include <zcbor_decode.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/coap.h>
#include <zephyr/net/coap_service.h>
#include <zephyr/net/net_ip.h>

LOG_MODULE_REGISTER(CoapService, CONFIG_DOOR_LOCK_COAP_SERVICE_LOG_LEVEL);

namespace DoorLock::CoAPService {

static GetStateCallback sGetStateCb = nullptr;
static UnlockCallback sUnlockCb = nullptr;
static LockCallback sLockCb = nullptr;
static Aliro::ReaderStateByte sNotifyState = Aliro::ReaderStateByte::Unknown;

static constexpr uint16_t kServicePort = CONFIG_DOOR_LOCK_COAP_SERVICE_PORT;

static constexpr bool IsResourcePathValid()
{
	for (char c : CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH) {
		if (c == '/') {
			return false;
		}
	}
	return sizeof(CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH) > 1;
}

static_assert(IsResourcePathValid(), "DOOR_LOCK_COAP_RESOURCE_PATH must be non-empty and contain no slashes");
static_assert(
	sizeof(CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH) - 1 <= sizeof(coap_option{}.value),
	"Resource path segment too long; enable COAP_EXTENDED_OPTIONS_LEN and increase COAP_EXTENDED_OPTIONS_LEN_VALUE");
static_assert(CONFIG_COAP_SERVER_MESSAGE_OPTIONS >= 2,
	      "COAP_SERVER_MESSAGE_OPTIONS must fit the URI-Path and Content-Format options");

static const char *GetStateMessage(Aliro::ReaderStateByte state)
{
	switch (state) {
	case Aliro::ReaderStateByte::Secured:
		return "secured";
	case Aliro::ReaderStateByte::Unsecured:
		return "unsecured";
	case Aliro::ReaderStateByte::Blocked:
		return "blocked";
	case Aliro::ReaderStateByte::EnteringSecured:
		return "enteringsecured";
	case Aliro::ReaderStateByte::EnteringUnsecured:
		return "enteringunsecured";
	case Aliro::ReaderStateByte::Unknown:
		return "unknown";
	}

	return "unknown";
}

static int SendStateResponse(struct coap_resource *resource, const struct net_sockaddr *addr,
			     socklen_t addrLen, Aliro::ReaderStateByte state, uint8_t type, uint16_t id,
			     const uint8_t *token, uint8_t tokenLen, bool includeObserve)
{
	struct coap_packet response;
	uint8_t data[CONFIG_COAP_SERVER_MESSAGE_SIZE];
	int err;

	err = coap_packet_init(&response, data, sizeof(data), COAP_VERSION_1, type, tokenLen, token,
			       COAP_RESPONSE_CODE_CONTENT, id);
	VerifyOrReturnValue(err >= 0, err);

	if (includeObserve) {
		err = coap_append_option_int(&response, COAP_OPTION_OBSERVE,
					     static_cast<unsigned int>(resource->age));
		VerifyOrReturnValue(err >= 0, err);
	}

	err = coap_append_option_int(&response, COAP_OPTION_CONTENT_FORMAT,
				     COAP_CONTENT_FORMAT_TEXT_PLAIN);
	VerifyOrReturnValue(err >= 0, err);

	err = coap_packet_append_payload_marker(&response);
	VerifyOrReturnValue(err >= 0, err);

	const char *message = GetStateMessage(state);
	err = coap_packet_append_payload(&response, (uint8_t *)message, strlen(message));
	VerifyOrReturnValue(err >= 0, err);

	return coap_resource_send(resource, &response, addr, addrLen, NULL);
}

static int HandleGet(struct coap_resource *resource, struct coap_packet *request, struct net_sockaddr *addr,
		     socklen_t addrLen)
{
	VerifyOrReturnValue(sGetStateCb != nullptr, COAP_RESPONSE_CODE_INTERNAL_ERROR);

	uint8_t token[COAP_TOKEN_MAX_LEN];
	uint8_t tokenLen = coap_header_get_token(request, token);
	const uint8_t type = coap_header_get_type(request);
	const uint8_t responseType = (type == COAP_TYPE_CON) ? COAP_TYPE_ACK : COAP_TYPE_NON_CON;
	const bool observing = coap_resource_parse_observe(resource, request, addr) == 0;

	return SendStateResponse(resource, addr, addrLen, sGetStateCb(), responseType,
				 coap_header_get_id(request), token, tokenLen, observing);
}

static void HandleNotify(struct coap_resource *resource, struct coap_observer *observer)
{
	const int err = SendStateResponse(resource, net_sad(&observer->addr),
					  net_family2size(observer->addr.ss_family), sNotifyState,
					  COAP_TYPE_CON, coap_next_id(), observer->token, observer->tkl, true);
	if (err < 0) {
		LOG_ERR("Failed to send CoAP notification: %d", err);
	}
}

static int HandlePut(struct coap_resource *resource, struct coap_packet *request, struct net_sockaddr *addr,
		     socklen_t addrLen)
{
	VerifyOrReturnValue(sUnlockCb != nullptr, COAP_RESPONSE_CODE_INTERNAL_ERROR);
	VerifyOrReturnValue(sLockCb != nullptr, COAP_RESPONSE_CODE_INTERNAL_ERROR);

	struct coap_option contentFormats[2];
	int formatCount = coap_find_options(request, COAP_OPTION_CONTENT_FORMAT, contentFormats, 2);

	VerifyOrReturnValue(formatCount == 1 &&
				    coap_option_value_to_int(&contentFormats[0]) == COAP_CONTENT_FORMAT_APP_CBOR,
			    COAP_RESPONSE_CODE_BAD_REQUEST);

	uint16_t payloadLen;
	const uint8_t *payload = coap_packet_get_payload(request, &payloadLen);

	VerifyOrReturnValue(payload != nullptr && payloadLen == 1, COAP_RESPONSE_CODE_BAD_REQUEST);

	ZCBOR_STATE_D(cborState, 0, payload, payloadLen, 1, 0);
	bool state;
	VerifyOrReturnValue(zcbor_bool_decode(cborState, &state), COAP_RESPONSE_CODE_BAD_REQUEST);

	if (state) {
		sUnlockCb();
	} else {
		sLockCb();
	}

	return COAP_RESPONSE_CODE_CHANGED;
}

extern "C" {
COAP_SERVICE_DEFINE(door_lock_service, "0.0.0.0", &kServicePort, 0);
}

static const char *const kDoorLockPath[] = { CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH, nullptr };
COAP_RESOURCE_DEFINE(door_lock, door_lock_service,
		     {
			     .get = HandleGet,
			     .put = HandlePut,
			     .notify = HandleNotify,
			     .path = kDoorLockPath,
		     });

void Init(GetStateCallback getStateCb, UnlockCallback unlockCb, LockCallback lockCb)
{
	sGetStateCb = getStateCb;
	sUnlockCb = unlockCb;
	sLockCb = lockCb;
}

int Start()
{
	const int err = coap_service_start(&door_lock_service);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to start CoAP service: %d", err));

	LOG_INF("CoAP service started at /%s (UDP %u)", CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH, kServicePort);
	return 0;
}

int Stop()
{
	const int err = coap_service_stop(&door_lock_service);
	VerifyOrReturnValue(err == 0, err, LOG_ERR("Failed to stop CoAP service: %d", err));

	LOG_INF("CoAP service stopped");
	return 0;
}

void Notify(Aliro::ReaderStateByte state)
{
	sNotifyState = state;
	const int err = coap_resource_notify(&door_lock);
	if (err < 0) {
		LOG_ERR("Failed to notify CoAP observers: %d", err);
	}
}

} // namespace DoorLock::CoAPService
