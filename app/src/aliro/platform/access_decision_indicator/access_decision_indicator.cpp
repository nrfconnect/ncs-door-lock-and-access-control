/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_decision_indicator.h"

#include "aliro/utils.h"

#include "aliro/aliro_work/aliro_work.h"

#include <tuple>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

namespace {

constexpr int kDelayMs{ CONFIG_RESET_ACCESS_DECISION_INDICATOR_STATE_DELAY_MS };
constexpr int kLedOn{ 1 };
constexpr int kLedOff{ 0 };
constexpr gpio_dt_spec kAccessGrantedLed = GPIO_DT_SPEC_GET(DT_ALIAS(access_decision_indicator), gpios);

static K_WORK_DELAYABLE_DEFINE(ResetIndicatorStateWork,
			       []([[maybe_unused]] k_work *) { (void)gpio_pin_set_dt(&kAccessGrantedLed, kLedOff); });

} // namespace

namespace Aliro::Access::Indicator {

AliroError InitAccessDecisionIndicator()
{
	VerifyOrReturnStatus(gpio_is_ready_dt(&kAccessGrantedLed), ALIRO_ERROR_INTERNAL);
	VerifyOrReturnStatus(gpio_pin_configure_dt(&kAccessGrantedLed, GPIO_OUTPUT_INACTIVE) == 0,
			     ALIRO_ERROR_INTERNAL);

	return ALIRO_NO_ERROR;
}

void SignalAccessGranted()
{
	(void)gpio_pin_set_dt(&kAccessGrantedLed, kLedOn);
	std::ignore = AliroWorkReschedule(&ResetIndicatorStateWork, K_MSEC(kDelayMs));
}

} // namespace Aliro::Access::Indicator
