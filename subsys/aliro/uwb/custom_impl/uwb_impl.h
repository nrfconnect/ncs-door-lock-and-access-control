/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "uwb.h"

#include <cstddef>

namespace Aliro::Uwb {

/**
 * @class UltraWideBandImpl
 * @brief Dummy implementation of the UltraWideBand interface.
 *
 * This class provides the dummy implementation of the UltraWideBand interface, handling UWB operations
 * such as initialization, ranging session management, and BLE message handling.
 */
class UltraWideBandImpl final : public UltraWideBand {
private:
	friend class UltraWideBand;
	friend UltraWideBand &UltraWideBandInstance();
	friend UltraWideBandImpl &UltraWideBandInstanceImpl();

	/**
	 * @brief Gets the instance of the UltraWideBand implementation.
	 *
	 * @return The instance of the UltraWideBand implementation.
	 */
	static UltraWideBandImpl &Instance()
	{
		static UltraWideBandImpl sInstance;
		return sInstance;
	}

	int _Init(const Callbacks &callbacks);
	int _Deinit();
	void _BleTimeSync();
	int _HandleBleMessage(const uint8_t *data, size_t length, SessionContextHandle sessionContextData);
	int _ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
				     ProtocolVersion protocolVersion, SessionContextHandle sessionContextHandle);
	int _InitiateRangingSession(SessionContextHandle sessionContextData);
	int _TerminateRangingSession(SessionContextHandle sessionContextData);
	int _SuspendRangingSession(SessionContextHandle sessionContextData);
	int _ResumeRangingSession(SessionContextHandle sessionContextData);
	const char *_GetFirmwareVersion() { return nullptr; }
	bool _IsInitialized() { return false; }
	void _StopRadarSession() {}

	UltraWideBandImpl() = default;
	~UltraWideBandImpl() = default;

	UltraWideBandImpl(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl &operator=(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl(UltraWideBandImpl &&) = delete;
	UltraWideBandImpl &operator=(UltraWideBandImpl &&) = delete;
};

/**
 * @brief Get the singleton instance of the UltraWideBand interface.
 *
 * @return Reference to the UltraWideBand instance.
 */
inline UltraWideBand &UltraWideBandInstance()
{
	return UltraWideBandImpl::Instance();
}

/**
 * @brief Get the singleton instance of the UltraWideBand implementation.
 *
 * @return Reference to the UltraWideBandImpl instance.
 */
inline UltraWideBandImpl &UltraWideBandInstanceImpl()
{
	return UltraWideBandImpl::Instance();
}

} // namespace Aliro::Uwb
