/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "coap_service/coap_service.h"

#include <zcbor_decode.h>
#include <zephyr/net/coap.h>
#include <zephyr/net/coap_service.h>
#include <zephyr/net/net_ip.h>

namespace DoorLock::CoAPService {

static GetStateCallback sGetStateCb = nullptr;
static SetStateCallback sSetStateCb = nullptr;

static const uint16_t door_lock_service_port = 5683;

static int door_lock_get(struct coap_resource *resource, struct coap_packet *request, struct net_sockaddr *addr,
			 socklen_t addr_len)
{
	uint8_t data[32];
	struct coap_packet response;
	uint16_t id;
	uint8_t token[COAP_TOKEN_MAX_LEN];
	uint8_t tkl, type;

	if (sGetStateCb == nullptr) {
		return COAP_RESPONSE_CODE_INTERNAL_ERROR;
	}

	bool state = sGetStateCb();
	const char *msg = state ? "unlocked" : "locked";

	type = coap_header_get_type(request);
	id = coap_header_get_id(request);
	tkl = coap_header_get_token(request, token);

	/* Determine response type */
	type = (type == COAP_TYPE_CON) ? COAP_TYPE_ACK : COAP_TYPE_NON_CON;

	coap_packet_init(&response, data, sizeof(data), COAP_VERSION_1, type, tkl, token, COAP_RESPONSE_CODE_CONTENT,
			 id);

	/* Set content format */
	coap_append_option_int(&response, COAP_OPTION_CONTENT_FORMAT, COAP_CONTENT_FORMAT_TEXT_PLAIN);

	/* Append payload */
	coap_packet_append_payload_marker(&response);
	coap_packet_append_payload(&response, (uint8_t *)msg, strlen(msg));

	/* Send to response back to the client */
	return coap_resource_send(resource, &response, addr, addr_len, NULL);
}

static int door_lock_put(struct coap_resource *resource, struct coap_packet *request, struct net_sockaddr *addr,
			 socklen_t addr_len)
{
	if (sSetStateCb == nullptr) {
		return COAP_RESPONSE_CODE_INTERNAL_ERROR;
	}

	struct coap_option content_formats[2];
	int format_count = coap_find_options(request, COAP_OPTION_CONTENT_FORMAT, content_formats, 2);

	if (format_count != 1 || coap_option_value_to_int(&content_formats[0]) != 60) {
		return COAP_RESPONSE_CODE_BAD_REQUEST;
	}

	uint16_t payload_len;
	const uint8_t *payload = coap_packet_get_payload(request, &payload_len);

	if (payload == nullptr || payload_len != 1) {
		return COAP_RESPONSE_CODE_BAD_REQUEST;
	}

	ZCBOR_STATE_D(cbor_state, 0, payload, payload_len, 1, 0);
	bool state;
	if (!zcbor_bool_decode(cbor_state, &state)) {
		return COAP_RESPONSE_CODE_BAD_REQUEST;
	}

	sSetStateCb(state);

	return COAP_RESPONSE_CODE_CHANGED;
}

extern "C" {
COAP_SERVICE_DEFINE(door_lock_service, "0.0.0.0", &door_lock_service_port, 0);
}

static const char *const door_lock_path[] = { "door-lock", NULL };
COAP_RESOURCE_DEFINE(door_lock, door_lock_service,
		     {
			     .get = door_lock_get,
			     .put = door_lock_put,
			     .path = door_lock_path,
		     });

void Init(GetStateCallback getStateCb, SetStateCallback setStateCb)
{
	sGetStateCb = getStateCb;
	sSetStateCb = setStateCb;
}

int Start()
{
	return coap_service_start(&door_lock_service);
}

int Stop()
{
	return coap_service_stop(&door_lock_service);
}

}; // namespace DoorLock::CoAPService
