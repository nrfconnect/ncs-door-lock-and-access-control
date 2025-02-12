/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <rfal_ncs_pal.h>
#include <rfal_nfc.h>
#include <rfal_nfc_config.h>
#include <rfal_utils.h>

LOG_MODULE_REGISTER(app, CONFIG_NCS_NFC_READER_SAMPLE_LOG_LEVEL);

extern struct k_sem irq_sem;

// NFC reader configuration
static rfalNfcDiscoverParam nfc_conf;

static void set_discovery_state()
{
	ReturnCode rc = RFAL_ERR_NONE;

	if (rfalNfcGetState() > RFAL_NFC_STATE_IDLE) {
		rc = rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_IDLE);
		if (rc) {
			LOG_ERR("RFAL: NFC deactivation failed, return code: %d", rc);
			return;
		}
	}

	rc = rfalNfcDiscover(&nfc_conf);
	if (rc) {
		LOG_ERR("RFAL: NFC discovery failed, return code: %d", rc);
		return;
	}
}

static void notify_cb(rfalNfcState state)
{
	LOG_INF("Notify state: %d", state);

	if (state == RFAL_NFC_STATE_ACTIVATED) {
		rfalNfcDevice *nfcDevice;

		rfalNfcGetActiveDevice(&nfcDevice);

		LOG_INF("Device type = %d", nfcDevice->type);

		switch (nfcDevice->type) {
		case RFAL_NFC_LISTEN_TYPE_NFCA:

			switch (nfcDevice->dev.nfca.type) {
			case RFAL_NFCA_T4T:
				LOG_HEXDUMP_INF(nfcDevice->nfcid, nfcDevice->nfcidLen,
						"NFCA Passive ISO-DEP device found. UID: ");
				set_discovery_state();
				break;

			default:
				LOG_INF("Unsupported NFC tag");
				break;
			}
			break;

		default:
			break;
		}
	}
}

static ReturnCode rfal_nfc_init(void)
{
	ReturnCode rc = rfalNfcInitialize();

	if (rc) {
		LOG_ERR("RFAL init failed, return code: %d", rc);
		return rc;
	}

	// Set default discovery parameters.
	rfalNfcDefaultDiscParams(&nfc_conf);
	// Set wake-up configuration.
	rfalNfcWakeupConfig(&nfc_conf);
	// Set only NFC-A technology Flag.
	nfc_conf.techs2Find |= RFAL_NFC_POLL_TECH_A;
	nfc_conf.notifyCb = notify_cb;

	set_discovery_state();

	return rc;
}

static void nfc_worker_fn(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	LOG_INF("NCS NFC reader sample application");

	if (rfal_ncs_pal_init()) {
		LOG_ERR("NFC PAL init failed");
		return;
	}

	if (rfal_nfc_init()) {
		LOG_ERR("RFAL init failed");
		return;
	}

	while (true) {
		rfalNfcWorker();
		k_sem_take(&irq_sem, K_MSEC(CONFIG_WORKER_POOL_TIMEOUT_MS));
	}
}

K_THREAD_DEFINE(nfc_worker_thread, CONFIG_WORKER_THREAD_STACK_SIZE, nfc_worker_fn, NULL, NULL, NULL,
		CONFIG_WORKER_THREAD_PRIORITY, 0, 0);
