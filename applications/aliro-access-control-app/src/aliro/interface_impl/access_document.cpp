/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro/interface.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(interface_access_document, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro::Interface::AccessDocument {

std::optional<bool> VerifyValidityPeriod(const Time &validFrom, const Time &validUntil)
{
	ARG_UNUSED(validFrom);
	ARG_UNUSED(validUntil);
	LOG_WRN("Time concept is not supported");
	return std::nullopt;
}

} // namespace Aliro::Interface::AccessDocument
