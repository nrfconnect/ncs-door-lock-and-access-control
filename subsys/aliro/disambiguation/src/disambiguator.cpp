/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <disambiguator.h>

#include <cerrno>
#include <doorlock/utils/mutex_guard.h>
#include <doorlock/utils/utils.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(Disambiguator, CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_LOG_LEVEL);

namespace {
/* Convert PDOA offset from degrees to Q4.11 fixed-point used by the library. */
constexpr double kPi{ 3.14159265358979323846 };
constexpr int16_t kCalibPdoaOffsetQ411{ static_cast<int16_t>(
	kPi * CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_PDOA_OFFSET_DEG / 180.0 * 2048.0) };
constexpr int16_t kCalibDistanceOffsetCm{ CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_DISTANCE_OFFSET_CM };
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
	mSessions.fill({});

	for (uint8_t sessionIdx = 0; sessionIdx < kMaxSessions; sessionIdx++) {
		aliro_disambiguation_reset_session(sessionIdx);
	}
}

void Disambiguator::ResetSession(uint8_t sessionIdx)
{
	VerifyOrReturn(mInitialized);
	VerifyOrReturn(sessionIdx < kMaxSessions);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	mSessions[sessionIdx] = {};
	aliro_disambiguation_reset_session(sessionIdx);
}

void Disambiguator::FlushCir()
{
	VerifyOrReturn(mInitialized);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	mCirCount = 0;
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
	mSessions[sessionIdx].distanceCount++;
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
	mSessions[sessionIdx].pdoaCount++;
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

	constexpr uint32_t kActualDupFactor{ CONFIG_DOOR_LOCK_ALIRO_UWB_MIN_RAN_MULTIPLIER * 2 };
	constexpr uint32_t kUwbReadyThreshold{ (ALIRO_DISAMBIGUATION_WINDOW_SIZE + kActualDupFactor - 1) /
					       kActualDupFactor };
	SessionState &sess = mSessions[sessionIdx];
	if (mCirCount < ALIRO_DISAMBIGUATION_WINDOW_SIZE || sess.distanceCount < kUwbReadyThreshold ||
	    sess.pdoaCount < kUwbReadyThreshold) {
		return -EBUSY;
	}

	disambiguation_debug_results results{};
	const int unlockAllowed = aliro_disambiguation(sessionIdx, &mDisambiguationParams, &results);

	// Bidirectional hysteresis: N consecutive readings required to flip FRONT↔BACK.
	constexpr uint8_t kFrontToBackConfirmCount = CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_FRONT_TO_BACK_CONFIRM;
	constexpr uint8_t kBackToFrontConfirmCount = CONFIG_DOOR_LOCK_ALIRO_UWB_DISAMBIGUATION_BACK_TO_FRONT_CONFIRM;
	const bool rawFront = (results.side != 0);
	if (rawFront) {
		sess.consecBackCount = 0;
		if (sess.lastResult.mSideIsFront) {
			out.mSideIsFront = true;
		} else {
			sess.consecFrontCount++;
			out.mSideIsFront = (sess.consecFrontCount >= kBackToFrontConfirmCount);
		}
	} else {
		sess.consecFrontCount = 0;
		if (sess.lastResult.mSideIsFront) {
			sess.consecBackCount++;
			out.mSideIsFront = (sess.consecBackCount < kFrontToBackConfirmCount);
		} else {
			out.mSideIsFront = false;
		}
	}

	out.mUnlockAllowed = out.mSideIsFront && (unlockAllowed != 0);

	const int32_t pRatioU6 = static_cast<int32_t>(results.p_ratio * 1000000.0f);
	const int32_t meanPdoaMilliDeg = static_cast<int32_t>(results.mean_pdoa * 1000.0f);
	LOG_DBG("sess%u %s pratio=%d pdoa=%d cir=%d blk=%d dist=%ucm consecF=%u consecB=%u", sessionIdx,
		out.mSideIsFront ? "FRONT" : "BACK ", pRatioU6, meanPdoaMilliDeg, results.CIR, results.noise_blocks,
		results.distance_cm, sess.consecFrontCount, sess.consecBackCount);

	out.mDistanceCm = results.distance_cm;
	out.mMeanPdoaDeg = results.mean_pdoa;
	out.mPRatio = results.p_ratio;
	out.mCir = results.CIR;
	out.mNoiseBlocks = results.noise_blocks;
	sess.lastResult = out;
	sess.hasResult = true;

	return 0;
}

bool Disambiguator::IsAnyUnlockAllowed() const
{
	DoorLock::Utils::MutexGuard lock{ mMutex };

	// Any session on the front side within the secure bubble allows unlock.
	for (const SessionState &sess : mSessions) {
		if (sess.hasResult && sess.lastResult.mUnlockAllowed) {
			return true;
		}
	}
	return false;
}

std::optional<Result> Disambiguator::TryGetLastResult(uint8_t sessionIdx)
{
	VerifyOrReturnValue(mInitialized && sessionIdx < kMaxSessions, std::nullopt);

	DoorLock::Utils::MutexGuard lock{ mMutex };

	VerifyOrReturnValue(mSessions[sessionIdx].hasResult, std::nullopt);

	return mSessions[sessionIdx].lastResult;
}

} // namespace Aliro::Uwb::Disambiguation
