/*
 * SPDX-FileCopyrightText: Copyright (c) 2023 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#if defined(__GNUC__)
/* GCC-specific code here. */

#define QFFS(x) __builtin_ffs(x)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#ifndef __cplusplus
#if __STDC__ && __STDC_VERSION__ < 202311L
#ifndef typeof
#define typeof(x) __typeof__(x) /* typeof was introduced in C23. */
#endif
#endif
#endif

#endif

#if defined(_IAR_SYSTEMS_ICC__)
/* IAR-specific code here. */
#endif

#if defined(__KEIL__) || defined(__C51__)
/* KEIL-specific code here. */
#endif
