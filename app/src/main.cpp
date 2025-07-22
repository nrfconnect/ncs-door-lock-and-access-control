/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/aliro.h"
#include "aliro/shell.h"
#include "aliro/utils.h"

#ifdef CONFIG_ALIRO_BLE_TP
#include "uwb/uwb.h"
#include "uwb_impl.h"
#endif // CONFIG_ALIRO_BLE_TP

#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#ifdef CONFIG_ACCESS_DECISION_INDICATOR
#include "access_decision_indicator.h"
#endif // CONFIG_ACCESS_DECISION_INDICATOR

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

#ifdef CONFIG_ALIRO_BLE_TP
	Uwb::UltraWideBandImpl::Callbacks callbacks{};
	ec = Uwb::UltraWideBandImpl::Instance().Init(callbacks);
	if (ec == ALIRO_ERROR_NOT_IMPLEMENTED) {
		LOG_INF("UWB is not implemented");
	} else {
		VerifyOrReturnValue(ec == ALIRO_NO_ERROR, EXIT_FAILURE, LOG_ERR("Failed to initialize UWB module"));
	}
#endif // CONFIG_ALIRO_BLE_TP

	ec = AliroStack::Instance().Init(
		{ .mOnAccessAttempt =
			  [](Status status) {
				  if (status == Status::Denied) {
					  LOG_INF("ACCESS DENIED");
				  } else {
					  LOG_INF("ACCESS GRANTED");
#ifdef CONFIG_ACCESS_DECISION_INDICATOR
					  Indicator::SignalAccessGranted();
#endif // CONFIG_ACCESS_DECISION_INDICATOR
				  }
			  },
		  .mOnError = [](AliroError error) { LOG_ERR("Aliro error: %s", error.ToString()); } });

	VerifyOrDie(ec == ALIRO_NO_ERROR, "Aliro stack initialization failed");

	ec = AliroStack::Instance().Start();

	VerifyOrDie(ec == ALIRO_NO_ERROR, "Aliro stack start failed");

	RegisterShellCommands();

	LOG_INF("Application started");

	return EXIT_SUCCESS;
}
