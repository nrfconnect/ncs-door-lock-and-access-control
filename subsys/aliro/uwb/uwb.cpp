/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#include "uwb.h"

#include "uwb_impl.h"

namespace Aliro::Uwb {

UltraWideBandImpl *UltraWideBand::Impl()
{
	return static_cast<UltraWideBandImpl *>(this);
}

const UltraWideBandImpl *UltraWideBand::Impl() const
{
	return static_cast<const UltraWideBandImpl *>(this);
}

int UltraWideBand::Init(const Callbacks &callbacks)
{
	return Impl()->_Init(callbacks);
}

int UltraWideBand::Deinit()
{
	return Impl()->_Deinit();
}

void UltraWideBand::BleTimeSync()
{
	Impl()->_BleTimeSync();
}

int UltraWideBand::HandleBleMessage(const uint8_t *data, size_t length, SessionContextHandle sessionContextData)
{
	return Impl()->_HandleBleMessage(data, length, sessionContextData);
}

int UltraWideBand::ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
					   ProtocolVersion protocolVersion, SessionContextHandle sessionContextHandle)
{
	return Impl()->_ConfigureRangingSession(sessionId, ursk, protocolVersion, sessionContextHandle);
}

int UltraWideBand::InitiateRangingSession(SessionContextHandle sessionContextData)
{
	return Impl()->_InitiateRangingSession(sessionContextData);
}

int UltraWideBand::TerminateRangingSession(SessionContextHandle sessionContextData)
{
	return Impl()->_TerminateRangingSession(sessionContextData);
}

int UltraWideBand::SuspendRangingSession(SessionContextHandle sessionContextData)
{
	return Impl()->_SuspendRangingSession(sessionContextData);
}

int UltraWideBand::ResumeRangingSession(SessionContextHandle sessionContextData)
{
	return Impl()->_ResumeRangingSession(sessionContextData);
}

int UltraWideBand::StartRadarSession()
{
	return Impl()->_StartRadarSession();
}

void UltraWideBand::StopRadarSession()
{
	Impl()->_StopRadarSession();
}

const char *UltraWideBand::GetFirmwareVersion()
{
	return Impl()->_GetFirmwareVersion();
}

bool UltraWideBand::IsInitialized()
{
	return Impl()->_IsInitialized();
}

#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
std::optional<uint8_t> UltraWideBand::GetDisambiguationSessionIdx(SessionContextHandle sessionContextData)
{
	return Impl()->_GetDisambiguationSessionIdx(sessionContextData);
}
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION

} // namespace Aliro::Uwb
