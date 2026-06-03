/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro_service/aliro_service.h>

#include <cstdint>

namespace DoorLock::AliroService {

class AdvertisingState {
public:
	/**
	 * @brief Check whether advertising is currently blocked.
	 *
	 * @retval true At least one advertising block reason is active.
	 * @retval false No advertising block reasons are active.
	 */
	bool IsBlocked() const { return mReasons != 0U; }

	/**
	 * @brief Add an advertising block reason.
	 *
	 * @param[in] reason Block reason to mark as active.
	 *
	 * @retval true This call transitioned the state from unblocked to blocked.
	 * @retval false Advertising was already blocked before this call.
	 */
	bool Block(AdvertisingBlockReason reason)
	{
		const bool wasBlocked = IsBlocked();
		mReasons |= ToReasonMask(reason);
		return !wasBlocked;
	}

	/**
	 * @brief Remove an advertising block reason.
	 *
	 * @param[in] reason Block reason to clear.
	 *
	 * @retval true This call removed the last active block reason.
	 * @retval false Advertising remains blocked, or the reason was not active.
	 */
	bool Unblock(AdvertisingBlockReason reason)
	{
		const uint32_t previousReasons = mReasons;
		mReasons &= ~ToReasonMask(reason);
		return previousReasons != mReasons && !IsBlocked();
	}

private:
	static constexpr uint32_t ToReasonMask(AdvertisingBlockReason reason)
	{
		return 1U << static_cast<uint32_t>(reason);
	}

	uint32_t mReasons{};
};

} // namespace DoorLock::AliroService
