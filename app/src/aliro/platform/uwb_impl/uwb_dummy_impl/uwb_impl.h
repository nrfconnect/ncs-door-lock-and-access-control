/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "uwb/uwb.h"

#include <cstddef>

namespace Aliro::Uwb {

/**
 * @class UltraWideBandImpl
 * @brief Dummy implementation of the UltraWideBand interface.
 *
 * This class provides the dummy implementation of the UltraWideBand interface, handling UWB operations
 * such as initialization, ranging session management, and BLE message handling.
 */
class UltraWideBandImpl : public UltraWideBand<UltraWideBandImpl> {
public:
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

	AliroError _Init(const Callbacks &callbacks);
	void _SetStackCallbacks(const StackCallbacks &callbacks);
	AliroError _Deinit();
	void _BleTimeSync();
	AliroError _HandleBleMessage(const uint8_t *data, size_t length, SessionContextHandle sessionContextData);
	AliroError _ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
					    ProtocolVersion protocolVersion, SessionContextHandle sessionContextData);
	AliroError _InitiateRangingSession(SessionContextHandle sessionContextData);
	AliroError _TerminateRangingSession(SessionContextHandle sessionContextData);
	AliroError _SuspendRangingSession(SessionContextHandle sessionContextData, bool force);
	AliroError _ResumeRangingSession(SessionContextHandle sessionContextData);

	// Delete copy and move constructors and assignment operators.
	UltraWideBandImpl(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl &operator=(const UltraWideBandImpl &) = delete;
	UltraWideBandImpl(UltraWideBandImpl &&) = delete;
	UltraWideBandImpl &operator=(UltraWideBandImpl &&) = delete;

private:
	UltraWideBandImpl() = default;
	~UltraWideBandImpl() = default;
};

} // namespace Aliro::Uwb
