/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/interface.h"

#include "aliro/access_manager/access_manager.h"
#include "aliro/platform/nfc/nfc_transport_rfal.h"

#ifdef CONFIG_NCS_ALIRO_BLE_UWB
#include <aliro_service/aliro_service.h>
#endif // CONFIG_NCS_ALIRO_BLE_UWB

namespace Aliro::Interface::Session {

AliroError Send(ConnectionHandle handle, Data data)
{
	if (handle.IsNfc()) {
		return NfcTransportRfal::Instance().Send(data);
	}
#ifdef CONFIG_NCS_ALIRO_BLE_UWB
	else if (handle.IsBle()) {
		return AliroError::FromInt(DoorLock::AliroService::Send(handle, data));
	}
#endif // CONFIG_NCS_ALIRO_BLE_UWB

	return ALIRO_INVALID_ARGUMENT;
}

void HandleTermination(ConnectionHandle handle)
{
	if (handle.IsNfc()) {
		NfcTransportRfal::Instance().Terminate();
	}
#ifdef CONFIG_NCS_ALIRO_BLE_UWB
	else if (handle.IsBle()) {
		DoorLock::AliroService::Terminate(handle);
	}
#endif // CONFIG_NCS_ALIRO_BLE_UWB

	AccessManagerInstance().HandleSessionTermination(handle);
}

#ifdef CONFIG_NCS_ALIRO_BLE_UWB

AliroError StartRangingSession(ConnectionHandle handle, uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
			       ProtocolVersion protocolVersion)
{
	return AccessManagerInstance().StartRangingSession(rangingSessionId, ursk, protocolVersion, handle);
}

#endif // CONFIG_NCS_ALIRO_BLE_UWB

} // namespace Aliro::Interface::Session
