/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/aliro.h"
#include "aliro/shell.h"
#include "aliro/types.h"
#include "aliro/utils.h"

#ifdef CONFIG_ACCESS_DECISION_INDICATOR
#include "access_decision_indicator.h"
#endif // CONFIG_ACCESS_DECISION_INDICATOR

#include "access_manager/access_manager.h"

#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <cstdio>
#include <stdlib.h>

LOG_MODULE_REGISTER(door_lock_app, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

using namespace Aliro;
using namespace Aliro::Access;

int main()
{
	AliroError ec{};
	LOG_INF("Starting nRF Door Lock Reference Application for the nRF Connect SDK");

#ifdef CONFIG_ACCESS_DECISION_INDICATOR
	VerifyOrReturnValue(Indicator::InitAccessDecisionIndicator() == ALIRO_NO_ERROR, EXIT_FAILURE,
			    LOG_ERR("Failed to initialize access decision indicator"));
#endif // CONFIG_ACCESS_DECISION_INDICATOR

	AccessManagerInstance().Init({ .mAccessGrantedIndicatorClb = Indicator::SignalAccessGranted });

	const AliroConfig config{
#ifdef CONFIG_DISABLE_ALIRO_NFC_TP
		.mEnableNfc = false,
#endif // CONFIG_DISABLE_ALIRO_NFC_TP

#ifdef CONFIG_ALIRO_BLE_TP
		.mMaxBleSessions = CONFIG_ALIRO_BLE_TP_MAX_SESSIONS,
#endif // CONFIG_ALIRO_BLE_TP
	};

	ec = AliroStack::Instance().Init(
		{ .mOnError = [](AliroError error) { LOG_ERR("Aliro error: %s", error.ToString()); } }, config);

	VerifyOrDie(ec == ALIRO_NO_ERROR, "Aliro stack initialization failed");

	ec = AliroStack::Instance().Start();

	VerifyOrDie(ec == ALIRO_NO_ERROR, "Aliro stack start failed");

	RegisterShellCommands();

	LOG_INF("Application started");

	return EXIT_SUCCESS;
}
