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

	AliroError _Init([[maybe_unused]] const Callbacks &);
	AliroError _Deinit();
	void _BleTimeSync() const;
	AliroError _HandleBleMessage([[maybe_unused]] const uint8_t *, [[maybe_unused]] size_t);
	AliroError _ConfigureRangingSession([[maybe_unused]] SessionIdentifier,
					    [[maybe_unused]] const CryptoTypes::Ursk &, [[maybe_unused]] void *);
	AliroError _InitiateRangingSession();
	AliroError _TerminateRangingSession();
	AliroError _SuspendRangingSession();
	AliroError _ResumeRangingSession();

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
