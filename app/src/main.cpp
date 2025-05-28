/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/aliro.h"
#include "aliro/shell.h"
#include "aliro/utils.h"

#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <cstdio>

LOG_MODULE_REGISTER(door_lock_app, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

int main()
{
	LOG_INF("Starting nRF Door Lock Reference Application for the nRF Connect SDK");

	AliroError ec = Aliro::AliroStack::Instance().Init(
		{ .mOnAccessAttempt =
			  [](Aliro::Access::Status status) {
				  if (status == Aliro::Access::Status::Denied) {
					  LOG_INF("ACCESS DENIED");
				  } else {
					  LOG_INF("ACCESS GRANTED");
				  }
			  },
		  .mOnError = [](AliroError error) { LOG_ERR("Aliro error: %s", error.ToString()); } });

	VerifyOrDie(ec == ALIRO_NO_ERROR, "Aliro stack initialization failed");

	ec = Aliro::AliroStack::Instance().Start();

	VerifyOrDie(ec == ALIRO_NO_ERROR, "Aliro stack start failed");

	Aliro::RegisterShellCommands();

	return 0;
}
