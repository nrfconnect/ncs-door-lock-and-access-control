/*
 * @copyright SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 *            SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Option to define QASSERT externally
 */
#include <qassert_impl.h> /* IWYU pragma: export */

#ifndef QASSERT
#include <assert.h>
/**
 * QASSERT - Assert macro to use, the implementation must manage if the assert
 * is implemented or stubbed depending on the definition of NDEBUG in accordance
 * to the C standard.
 *
 * @cond: Condition to be tested, if false, it doesn't return and displays an
 * error message
 */
#define QASSERT(cond) assert(cond)
#endif

#ifdef __cplusplus
}
#endif
