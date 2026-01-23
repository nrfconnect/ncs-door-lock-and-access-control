/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/interface.h"

namespace Aliro::Interface::AccessDocument {

std::optional<bool> VerifyValidityPeriod(const Time &, const Time &)
{
	return std::nullopt;
}

} // namespace Aliro::Interface::AccessDocument
