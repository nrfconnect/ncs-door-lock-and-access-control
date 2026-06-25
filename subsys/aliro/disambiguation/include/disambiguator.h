/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include <zephyr/kernel.h>

#include <aliro_disambiguation.h>

namespace Aliro::Uwb::Disambiguation {

/** @brief Output of @ref Disambiguator::Process: door decision, phone side, and ranging diagnostics. */
struct Result {
	bool mSideIsFront{ false };
	bool mUnlockAllowed{ false };
	uint16_t mDistanceCm{ 0 };
	float mMeanPdoaDeg{ 0.0f };
	float mPRatio{ 0.0f };
	int mCir{ 0 };
	int mNoiseBlocks{ 0 };

	/** @return True if the peer is on the front side (@ref mSideIsFront). */
	bool IsFront() const { return mSideIsFront; }
	/** @return True if unlock is allowed (front side and within secure bubble). */
	bool IsUnlockAllowed() const { return mUnlockAllowed; }
};

/** @brief Singleton UWB front/back and door disambiguation over buffered CIR, distance, and PDOA/RSSI. */
class Disambiguator {
public:
	/** @brief Returns the process-wide disambiguator instance. */
	static Disambiguator &Instance()
	{
		static Disambiguator sInstance;
		return sInstance;
	}

	/** @brief Enables the module.
	 *  @param params Disambiguation parameters.
	 *  @retval 0 on success, or a negative error code on failure.
	 */
	int Init(const disambiguation_parameters &params);

	/** @brief Clears measurement state and processing state for a single session.
	 *  @param sessionIdx Session index in @c [0, CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS).
	 */
	void ResetSession(uint8_t sessionIdx);

	/** @brief Resets the global CIR counter, blocking Process() until the 40-sample window refills.
	 *  Call after removing the last active session.
	 */
	void FlushCir();

	/** @brief Feeds one distance sample for a session (internally calibrated when not in error).
	 *  @param distanceCm Measured distance in centimeters.
	 *  @param sessionIdx Session index in @c [0, CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS).
	 *  @param error When true, the sample is passed through as an error to processing.
	 */
	void AddDistanceMeasurement(uint16_t distanceCm, uint8_t sessionIdx, bool error);

	/** @brief Feeds one PDOA and RSSI sample pair for a session.
	 *  @param pdoaQ411 Phase difference of arrival in Q4.11 fixed-point units.
	 *  @param rssiQ88 RSSI in Q8.8 fixed-point units (passed as @c uint16_t to processing).
	 *  @param sessionIdx Session index in @c [0, CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS).
	 *  @param pdoaError When true, PDOA is treated as invalid for processing.
	 *  @param rssiError When true, RSSI is treated as invalid for processing.
	 */
	void AddPdoaMeasurement(int16_t pdoaQ411, int16_t rssiQ88, uint8_t sessionIdx, bool pdoaError, bool rssiError);

	/** @brief Feeds a raw CIR buffer (global to all sessions in the current implementation).
	 *  @param data CIR bytes from the UWB stack.
	 *  @param size Length of @p data in bytes.
	 */
	void AddCirMeasurement(uint8_t *data, uint16_t size);

	/** @brief Runs disambiguation when enough CIR, distance, and PDOA samples are buffered.
	 *  @param[out] out Filled with door/side decision and algorithm metrics on success.
	 *  @param sessionIdx Session index in @c [0, CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS).
	 *  @retval 0 Success; @p out is stored as the last result for @p sessionIdx.
	 *  @retval -EINVAL @p sessionIdx is out of range.
	 *  @retval -EBUSY Not initialized, or CIR/distance/PDOA counts below the internal ready threshold.
	 */
	int Process(Result &out, uint8_t sessionIdx);

	/** @brief Returns a snapshot of the last successful @ref Process output for a session.
	 *  @param sessionIdx Session index.
	 *  @return Copy of the last result, or empty if unavailable.
	 */
	std::optional<Result> TryGetLastResult(uint8_t sessionIdx);

	/** @brief Returns true if any active session currently allows unlock.
	 *
	 *  Unlock is allowed when the disambiguator reports front side AND the
	 *  user is within @c secure_bubble_radius for that session.
	 */
	bool IsAnyUnlockAllowed() const;

private:
	Disambiguator() = default;
	~Disambiguator() = default;
	Disambiguator(const Disambiguator &) = delete;
	Disambiguator &operator=(const Disambiguator &) = delete;
	Disambiguator(Disambiguator &&) = delete;
	Disambiguator &operator=(Disambiguator &&) = delete;

	void ResetAllSessions();

	constexpr static uint8_t kMaxSessions{ CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS };

	mutable k_mutex mMutex{};
	bool mInitialized{ false };
	uint32_t mCirCount{ 0 };
	disambiguation_parameters mDisambiguationParams{};

	struct SessionState {
		bool hasResult{ false };
		uint32_t distanceCount{ 0 };
		uint32_t pdoaCount{ 0 };
		Result lastResult{};
		/* FRONT→BACK requires kFrontToBackConfirmCount consecutive BACKs. */
		uint8_t consecBackCount{ 0 };
		/* BACK→FRONT requires kBackToFrontConfirmCount consecutive FRONTs. */
		uint8_t consecFrontCount{ 0 };
	};

	std::array<SessionState, kMaxSessions> mSessions{};
};

} // namespace Aliro::Uwb::Disambiguation
