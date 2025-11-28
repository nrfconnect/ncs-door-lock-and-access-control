/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_DOOR_LOCK_STEP_UP_PHASE

#pragma once

#include "aliro/errors.h"
#include "aliro/types.h"

#include <cstddef>

namespace Aliro {

struct ValidityIterations {
	ValidityIteration mAccessIteration{};

	bool operator!=(const ValidityIterations &other) const { return mAccessIteration != other.mAccessIteration; }
};

AliroError ReadValidityIterations(size_t credentialIssuerKeyIndex, ValidityIterations &iterations);
AliroError StoreValidityIterations(size_t credentialIssuerKeyIndex, const ValidityIterations &iterations);
AliroError ClearValidityIterations(size_t credentialIssuerKeyIndex);

} // namespace Aliro

#endif // CONFIG_DOOR_LOCK_STEP_UP_PHASE
