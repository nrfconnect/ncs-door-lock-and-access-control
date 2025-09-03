/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <qatomic_impl.h>

#ifndef QATOMIC

#include <stdatomic.h>

#define qatomic_int atomic_int
#define qatomic_bool atomic_bool

#define qatomic_init(x, value) atomic_init(x, value)
#define qatomic_load(x) atomic_load(x)
#define qatomic_store(x, value) atomic_store(x, value)
#define qatomic_exchange(x, value) atomic_exchange(x, value)
#define qatomic_fetch_add(x, value) atomic_fetch_add(x, value)
#define qatomic_fetch_sub(x, value) atomic_fetch_sub(x, value)

#endif /* QATOMIC */
