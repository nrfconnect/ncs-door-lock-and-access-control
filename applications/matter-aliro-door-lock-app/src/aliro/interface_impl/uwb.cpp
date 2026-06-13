/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

#include "aliro/interface.h"

#include "uwb_impl.h"

namespace Aliro::Interface {

namespace Uwb {

int HandleBleMessage(ConnectionHandle sessionContext, const uint8_t *data, size_t length)
{
	return ::Aliro::Uwb::UltraWideBandInstance().HandleBleMessage(data, length, sessionContext);
}

} // namespace Uwb

} // namespace Aliro::Interface

#endif // CONFIG_NCS_ALIRO_BLE_UWB
