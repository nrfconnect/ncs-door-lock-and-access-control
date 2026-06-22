/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

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
