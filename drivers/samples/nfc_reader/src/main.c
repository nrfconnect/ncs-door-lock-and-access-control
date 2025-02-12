/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/smf.h>

#include <rfal_nfc.h>
#include <rfal_platform.h>
#include <rfal_utils.h>

LOG_MODULE_REGISTER(app, CONFIG_NCS_NFC_READER_SAMPLE_LOG_LEVEL);

// NFC states
enum nfc_state {
	S_NO_INIT,
	S_IDLE,
	S_DISCOVERY,
};

// Forward declaration of functions.
static void no_init_entry(void *o);
static void no_init_run(void *o);

static void idle_entry(void *o);
static void idle_run(void *o);

static void discovery_entry(void *o);
static void discovery_run(void *o);
static void discovery_exit(void *o);

extern struct k_sem irq_sem;

// NFC reader configuration
static rfalNfcDiscoverParam nfc_conf;

// State machine context.
static struct smf_ctx hsm_ctx;

// State machine for NFC.
static const struct smf_state nfc_hsm[] = {
	[S_NO_INIT] = SMF_CREATE_STATE(no_init_entry, no_init_run, NULL, NULL, NULL),
	[S_IDLE] = SMF_CREATE_STATE(idle_entry, idle_run, NULL, NULL, NULL),
	[S_DISCOVERY] = SMF_CREATE_STATE(discovery_entry, discovery_run, discovery_exit, NULL, NULL),
};

static bool pal_init(void)
{
	int err = ncs_pal_spi_init();
	if (err) {
		LOG_ERR("NFC PAL spi init failed %d", err);
		return false;
	}

	err = ncs_pal_pwr_pin_set();
	if (err) {
		LOG_ERR("NFC PAL gpio init failed %d", err);
		return false;
	}
	return true;
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
	// Set only NFC-A technology Flag.
	nfc_conf.techs2Find |= RFAL_NFC_POLL_TECH_A;

	return rc;
}

// State machine callbacks
static void no_init_entry(void *o)
{
	LOG_INF("NOINIT state");
}

static void no_init_run(void *o)
{
	if (rfal_nfc_init()) {
		LOG_ERR("RFAL init failed");
		k_sleep(K_SECONDS(1));
		return;
	}

	// Transtion to the IDLE state.
	smf_set_state(&hsm_ctx, &nfc_hsm[S_IDLE]);
}

static void idle_entry(void *o)
{
	LOG_INF("IDLE state");
}

static void idle_run(void *o)
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

	// Transtion to the DISCOVERY state.
	smf_set_state(&hsm_ctx, &nfc_hsm[S_DISCOVERY]);
}

static void discovery_entry(void *o)
{
	LOG_INF("DISCOVERY state");
}

static void discovery_run(void *o)
{
	rfalNfcDevice *nfcDevice;
	if (rfalNfcIsDevActivated(rfalNfcGetState())) {
		rfalNfcGetActiveDevice(&nfcDevice);

		LOG_INF("Device type = %d", nfcDevice->type);

		switch (nfcDevice->type) {
		case RFAL_NFC_LISTEN_TYPE_NFCA:

			switch (nfcDevice->dev.nfca.type) {
			case RFAL_NFCA_T1T:
				LOG_HEXDUMP_INF(nfcDevice->nfcid, nfcDevice->nfcidLen,
						"ISO14443A/Topaz (NFC-A T1T) TAG "
						"found. UID: ");
				break;

			case RFAL_NFCA_T4T:
				LOG_HEXDUMP_INF(nfcDevice->nfcid, nfcDevice->nfcidLen,
						"NFCA Passive ISO-DEP device found. UID: ");
				break;

			case RFAL_NFCA_T4T_NFCDEP:
			case RFAL_NFCA_NFCDEP:
				LOG_HEXDUMP_INF(nfcDevice->nfcid, nfcDevice->nfcidLen,
						"NFCA Passive P2P device found. NFCID: ");
				break;

			default:
				LOG_HEXDUMP_INF(nfcDevice->nfcid, nfcDevice->nfcidLen,
						"ISO14443A/NFC-A card found. UID: %s");
				break;
			}
			break;

		default:
			break;
		}

		// Transtion to the IDLE state.
		smf_set_state(&hsm_ctx, &nfc_hsm[S_IDLE]);
	}
}

static void discovery_exit(void *o)
{
	rfalNfcDeactivate(RFAL_NFC_DEACTIVATE_IDLE);
	// Delay after read tag allows to avoid read the same tag agan too often.
	k_sleep(K_MSEC(500));
}

static void nfc_worker_fn(void *unused1, void *unused2, void *unused3)
{
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	int32_t ret;

	LOG_INF("NCS NFC reader sample application");

	if (!pal_init()) {
		LOG_ERR("NFC PAL init failed");
		return;
	}

	smf_set_initial(&hsm_ctx, &nfc_hsm[S_NO_INIT]);

	while (true) {
		rfalNfcWorker();

		ret = smf_run_state(&hsm_ctx);
		if (ret) {
			break;
		}
		k_sem_take(&irq_sem, K_MSEC(CONFIG_WORKER_POOL_TIMEOUT_MS));
	}
}

K_THREAD_DEFINE(nfc_worker_thread, CONFIG_WORKER_THREAD_STACK_SIZE, nfc_worker_fn, NULL, NULL, NULL,
		CONFIG_WORKER_THREAD_PRIORITY, 0, 0);
