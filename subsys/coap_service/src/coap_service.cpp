/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "coap_service/coap_service.h"

#include "doorlock/utils/utils.h"
#include <zcbor_decode.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/coap.h>
#include <zephyr/net/coap_service.h>
#include <zephyr/net/net_ip.h>

LOG_MODULE_REGISTER(CoapService, CONFIG_DOOR_LOCK_COAP_SERVICE_LOG_LEVEL);

namespace DoorLock::CoAPService {

static GetStateCallback sGetStateCb = nullptr;
static SetStateCallback sSetStateCb = nullptr;

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

static int HandleGet(struct coap_resource *resource, struct coap_packet *request, struct net_sockaddr *addr,
		     socklen_t addrLen)
{
	VerifyOrReturnValue(sGetStateCb != nullptr, COAP_RESPONSE_CODE_INTERNAL_ERROR);

	State state = sGetStateCb();
	const char *message = state == State::Unlocked ? "unlocked" : "locked";

	uint8_t type = coap_header_get_type(request);
	uint16_t id = coap_header_get_id(request);

	uint8_t token[COAP_TOKEN_MAX_LEN];
	uint8_t tokenLen = coap_header_get_token(request, token);

	struct coap_packet response;
	uint8_t data[32];
	uint8_t responseType = (type == COAP_TYPE_CON) ? COAP_TYPE_ACK : COAP_TYPE_NON_CON;

	coap_packet_init(&response, data, sizeof(data), COAP_VERSION_1, responseType, tokenLen, token,
			 COAP_RESPONSE_CODE_CONTENT, id);
	coap_append_option_int(&response, COAP_OPTION_CONTENT_FORMAT, COAP_CONTENT_FORMAT_TEXT_PLAIN);
	coap_packet_append_payload_marker(&response);
	coap_packet_append_payload(&response, (uint8_t *)message, strlen(message));

	return coap_resource_send(resource, &response, addr, addrLen, NULL);
}

static int HandlePut(struct coap_resource *resource, struct coap_packet *request, struct net_sockaddr *addr,
		     socklen_t addrLen)
{
	VerifyOrReturnValue(sSetStateCb != nullptr, COAP_RESPONSE_CODE_INTERNAL_ERROR);

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

	sSetStateCb(state ? State::Unlocked : State::Locked);

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
			     .path = kDoorLockPath,
		     });

void Init(GetStateCallback getStateCb, SetStateCallback setStateCb)
{
	sGetStateCb = getStateCb;
	sSetStateCb = setStateCb;
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

} // namespace DoorLock::CoAPService
