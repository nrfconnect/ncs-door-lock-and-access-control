/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <disambiguator.h>

#include <cerrno>
#include <doorlock/utils/mutex_guard.h>
#include <doorlock/utils/utils.h>

namespace {
constexpr int16_t kCalibPdoaOffsetQ411{ 0 };
constexpr int16_t kCalibDistanceOffsetCm{ 0 };
} // namespace

static_assert(CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS <= ALIRO_DISAMBIGUATION_NB_SESSION_MAX,
	      "CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS must be less than or equal to ALIRO_DISAMBIGUATION_NB_SESSION_MAX");

namespace Aliro::Uwb::Disambiguation {

int Disambiguator::Init(const disambiguation_parameters &params)
{
	int err = k_mutex_init(&mMutex);
	VerifyOrReturnValue(err == 0, err);

	err = aliro_disambiguation_init_processing();
	VerifyOrReturnValue(err == 0, err);

	mDisambiguationParams = params;
	mInitialized = true;

	return 0;
}

void Disambiguator::ResetAllSessions()
{
	VerifyOrReturn(mInitialized);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	mCirCount = 0;
	mDistanceCount.fill(0);
	mPdoaCount.fill(0);
	mHasLastResult.fill(false);
	mLastResult.fill({});
	mConsecBackCount.fill(0);

	for (uint8_t sessionIdx = 0; sessionIdx < kMaxSessions; sessionIdx++) {
		aliro_disambiguation_reset_session(sessionIdx);
	}
}

void Disambiguator::ResetSession(uint8_t sessionIdx)
{
	VerifyOrReturn(mInitialized);
	VerifyOrReturn(sessionIdx < kMaxSessions);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	mDistanceCount[sessionIdx] = 0;
	mPdoaCount[sessionIdx] = 0;
	mHasLastResult[sessionIdx] = false;
	mLastResult[sessionIdx] = {};
	mConsecBackCount[sessionIdx] = 0;
	aliro_disambiguation_reset_session(sessionIdx);
}

void Disambiguator::AddDistanceMeasurement(uint16_t distanceCm, uint8_t sessionIdx, bool error)
{
	VerifyOrReturn(mInitialized);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	if (sessionIdx >= kMaxSessions) {
		return;
	}
	uint16_t adjusted = distanceCm;
	if (!error && kCalibDistanceOffsetCm != 0) {
		int32_t val = static_cast<int32_t>(distanceCm) - kCalibDistanceOffsetCm;
		if (val < 0) {
			val = 0;
		}
		adjusted = static_cast<uint16_t>(val);
	}
	aliro_disambiguation_put_distance_data(adjusted, sessionIdx, error,
					       CONFIG_DOOR_LOCK_ALIRO_UWB_MIN_RAN_MULTIPLIER);
	mDistanceCount[sessionIdx]++;
}

void Disambiguator::AddPdoaMeasurement(int16_t pdoaQ411, int16_t rssiQ88, uint8_t sessionIdx, bool pdoaError,
				       bool rssiError)
{
	VerifyOrReturn(mInitialized);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	if (sessionIdx >= kMaxSessions) {
		return;
	}
	int16_t adjusted = pdoaQ411;
	if (!pdoaError && kCalibPdoaOffsetQ411 != 0) {
		adjusted = static_cast<int16_t>(pdoaQ411 - kCalibPdoaOffsetQ411);
	}
	aliro_disambiguation_put_pdoa_rssi_data(adjusted, static_cast<uint16_t>(rssiQ88), sessionIdx, pdoaError,
						rssiError, CONFIG_DOOR_LOCK_ALIRO_UWB_MIN_RAN_MULTIPLIER);
	mPdoaCount[sessionIdx]++;
}

void Disambiguator::AddCirMeasurement(uint8_t *data, uint16_t size)
{
	VerifyOrReturn(mInitialized);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	VerifyOrReturn(data);
	VerifyOrReturn(size > 0);
	aliro_disambiguation_put_cir_buffer(data, size);
	mCirCount++;
}

int Disambiguator::Process(Result &out, uint8_t sessionIdx)
{
	VerifyOrReturnValue(mInitialized, -EBUSY);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	VerifyOrReturnValue(sessionIdx < kMaxSessions, -EINVAL);

	constexpr uint32_t kUwbReadyThreshold{ ALIRO_DISAMBIGUATION_WINDOW_SIZE /
					       (CONFIG_DOOR_LOCK_ALIRO_UWB_MIN_RAN_MULTIPLIER * 2) };
	if (mCirCount < ALIRO_DISAMBIGUATION_WINDOW_SIZE || mDistanceCount[sessionIdx] < kUwbReadyThreshold ||
	    mPdoaCount[sessionIdx] < kUwbReadyThreshold) {
		return -EBUSY;
	}

	disambiguation_debug_results results{};
	aliro_disambiguation(sessionIdx, &mDisambiguationParams, &results);

	// Temporal hysteresis: FRONT→BACK requires kFrontToBackConfirmCount consecutive BACK
	// results before the decision flips. BACK→FRONT flips immediately.
	// This prevents spurious BACK flashes caused by CIR saturation (pratio=0 repeating last
	// BACK decision) or transient noise in the disambiguation algorithm.
	constexpr uint8_t kFrontToBackConfirmCount = 8;
	const bool rawFront = (results.side != 0);
	if (rawFront) {
		mConsecBackCount[sessionIdx] = 0;
		out.mSideIsFront = true;
	} else if (mLastResult[sessionIdx].mSideIsFront) {
		mConsecBackCount[sessionIdx]++;
		out.mSideIsFront = (mConsecBackCount[sessionIdx] < kFrontToBackConfirmCount);
	} else {
		out.mSideIsFront = false;
	}

	out.mDistanceCm = results.distance_cm;
	out.mMeanPdoaDeg = results.mean_pdoa;
	out.mPRatio = results.p_ratio;
	out.mCir = results.CIR;
	out.mNoiseBlocks = results.noise_blocks;
	mLastResult[sessionIdx] = out;
	mHasLastResult[sessionIdx] = true;

	return 0;
}

std::optional<Result> Disambiguator::TryGetLastResult(uint8_t sessionIdx)
{
	VerifyOrReturnValue(mInitialized && sessionIdx < kMaxSessions, std::nullopt);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	VerifyOrReturnValue(mHasLastResult[sessionIdx], std::nullopt);

	return mLastResult[sessionIdx];
}

} // namespace Aliro::Uwb::Disambiguation
