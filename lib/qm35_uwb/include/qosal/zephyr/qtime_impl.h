/*
 * SPDX-FileCopyrightText: Copyright (c) 2023 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */
#pragma once

#include <zephyr/kernel.h>

#define QTIME_GET_UPTIME_TICKS() k_uptime_ticks()

#define QTIME_GET_STRING_TICKS_PER_S() STRINGIFY(Z_HZ_ticks)
