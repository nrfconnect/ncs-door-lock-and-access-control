/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "uwb_impl.h"

#include "aliro/aliro.h"

#include <qmalloc.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <algorithm>
#include <cstring>
#include <iterator>

#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
extern "C" {
#include <UwbApi.h>
#include <UwbApi_Proprietary.h>
#include <UwbApi_Utility.h>
}
#endif

LOG_MODULE_REGISTER(aliro_uwb_tsrr250, CONFIG_DOOR_LOCK_UWB_LOG_LEVEL);

namespace Aliro::Uwb {
namespace {

constexpr std::array<uint8_t, 12> kRandomKey{
	0xB5, 0xB5, 0xB5, 0xB5, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
};

constexpr uint8_t kUwbRangingServiceProtocolType{ 0x01 };
constexpr uint8_t kNotificationProtocolType{ 0x02 };
constexpr uint8_t kNotificationEventMessageId{ 0x00 };
constexpr uint8_t kNotificationRangingMessageId{ 0x01 };
constexpr uint8_t kRssM1MessageId{ 0x00 };
constexpr uint8_t kRssM2MessageId{ 0x01 };
constexpr uint8_t kRssM3MessageId{ 0x02 };
constexpr uint8_t kRssM4MessageId{ 0x03 };
constexpr uint8_t kRangingSessionSuspendRequestMessageId{ 0x04 };
constexpr uint8_t kRangingSessionSuspendResponseMessageId{ 0x05 };
constexpr uint8_t kRangingSessionResumeRequestMessageId{ 0x06 };
constexpr uint8_t kRangingSessionResumeResponseMessageId{ 0x07 };
constexpr uint8_t kInitiateRangingSessionAttributeId{ 0x00 };
constexpr uint8_t kInitiateRangingSessionResumeAttributeId{ 0x01 };
constexpr uint8_t kRangingSessionSuspendedAttributeId{ 0x05 };
constexpr size_t kBleMessageHeaderLength{ 4 };
constexpr size_t kRssM1FixedPayloadLength{ 13 };
constexpr uint16_t kRssM3PayloadLength{ 24 };
constexpr uint16_t kRssGeneralErrorPayloadLength{ 3 };
constexpr uint16_t kRangingSessionIdPayloadLength{ 6 };
constexpr uint16_t kRangingSessionStatusPayloadLength{ 3 };
constexpr uint16_t kRangingSessionResumeResponsePayloadLength{ 16 };
constexpr uint8_t kAttrUwbConfigId{ 0x00 };
constexpr uint8_t kAttrPulseShapeCombo{ 0x01 };
constexpr uint8_t kAttrRangingSessionId{ 0x02 };
constexpr uint8_t kAttrChannelBitmask{ 0x03 };
constexpr uint8_t kAttrRanMultiplier{ 0x04 };
constexpr uint8_t kAttrSlotBitmask{ 0x05 };
constexpr uint8_t kAttrSyncCodeIndexBitmask{ 0x06 };
constexpr uint8_t kAttrSyncCodeIndex{ 0x07 };
constexpr uint8_t kAttrHoppingConfig{ 0x08 };
constexpr uint8_t kAttrChapsPerSlot{ 0x09 };
constexpr uint8_t kAttrResponderNodesCount{ 0x0A };
constexpr uint8_t kAttrSlotsPerRound{ 0x0B };
constexpr uint8_t kAttrStsIndex0{ 0x0C };
constexpr uint8_t kAttrUwbTime0{ 0x0D };
constexpr uint8_t kAttrHopModeKey{ 0x0E };
constexpr uint8_t kAttrMacMode{ 0x0F };
constexpr uint8_t kAttrStatus{ 0x11 };
constexpr uint8_t kAttrError{ 0x01 };
constexpr uint8_t kUnknownErrorReason{ 0x00 };
constexpr uint8_t kWrongParametersErrorReason{ 0x02 };
constexpr uint8_t kSuccessStatus{ 0x01 };
constexpr uint8_t kCsaRangingStatusSuccess{ 0x00 };
constexpr uint16_t kCccRangingInvalidDistance{ 0xFFFF };
constexpr size_t kCsaRangingMinimumPayloadLength{ 38 };
constexpr uint8_t kCsaDistanceFilterSampleCount{ 3 };
constexpr uint16_t kCsaDistanceFilterMaxSpreadCm{ 30 };
constexpr uint8_t kDefaultChapsPerSlot{ 6 };
constexpr uint8_t kDefaultSlotsPerRound{ 24 };
constexpr uint8_t kDefaultResponderNodesCount{ 1 };
constexpr uint8_t kDefaultHoppingConfig{ 0x80 };
constexpr uint8_t kDefaultMacMode{ 1 };
constexpr uint8_t kNxpRangingStatusTxFailed{ 0x20 };
constexpr uint8_t kNxpRangingStatusRxTimeout{ 0x21 };
constexpr uint8_t kNxpRangingStatusMacDecodeFailed{ 0x25 };
constexpr std::array<uint8_t, 2> kNxpNearbyReaderMac{ 0x22, 0x11 };
constexpr std::array<uint8_t, 2> kNxpNearbyUserDeviceMac{ 0x11, 0x22 };
constexpr std::array<uint8_t, 6> kNxpNearbyStaticStsIv{ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
constexpr std::array<uint8_t, 2> kNxpNearbyVendorId{ 0x08, 0x07 };
constexpr uint8_t kNxpNearbyPreambleCodeIndex{ 9 };
constexpr uint16_t kNxpNearbySlotDurationRstu{ 2400 };
constexpr uint16_t kNxpNearbyRangingIntervalMs{ 240 };

#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
struct Sr250CalibrationEntry {
	const char *mName;
	uint8_t mChannel;
	eCalibParam mParamId;
	const uint8_t *mValue;
	uint16_t mLength;
};

constexpr std::array<uint8_t, 7> kSr250RfClockAccuracyCalibration{
	0x03, 0x25, 0x00, 0x25, 0x00, 0x04, 0x00
};

constexpr std::array<uint8_t, 13> kSr250RxAntennaDelayCalibration{
	0x04, 0x01, 0x2A, 0x24, 0x02, 0x2A, 0x24, 0x03, 0x2A, 0x24, 0x04, 0x2A, 0x24
};

constexpr std::array<uint8_t, 11> kSr250TxPowerPerAntennaCalibration{
	0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00
};

constexpr std::array<uint8_t, 4> kSr250TxAntennaDelayCalibration{ 0x01, 0x01, 0x2A, 0x24 };

constexpr std::array<Sr250CalibrationEntry, 7> kSr250CalibrationEntries{ {
	{ "RF_CLK_ACCURACY_CALIB", 0, RF_CLK_ACCURACY_CALIB, kSr250RfClockAccuracyCalibration.data(),
	  kSr250RfClockAccuracyCalibration.size() },
	{ "RX_ANT_DELAY_CALIB_CH5", 5, RX_ANT_DELAY_CALIB, kSr250RxAntennaDelayCalibration.data(),
	  kSr250RxAntennaDelayCalibration.size() },
	{ "RX_ANT_DELAY_CALIB_CH9", 9, RX_ANT_DELAY_CALIB, kSr250RxAntennaDelayCalibration.data(),
	  kSr250RxAntennaDelayCalibration.size() },
	{ "TX_POWER_PER_ANTENNA_CH5", 5, TX_POWER_PER_ANTENNA, kSr250TxPowerPerAntennaCalibration.data(),
	  kSr250TxPowerPerAntennaCalibration.size() },
	{ "TX_POWER_PER_ANTENNA_CH9", 9, TX_POWER_PER_ANTENNA, kSr250TxPowerPerAntennaCalibration.data(),
	  kSr250TxPowerPerAntennaCalibration.size() },
	{ "TX_ANT_DELAY_CALIB_CH5", 5, TX_ANT_DELAY_CALIB, kSr250TxAntennaDelayCalibration.data(),
	  kSr250TxAntennaDelayCalibration.size() },
	{ "TX_ANT_DELAY_CALIB_CH9", 9, TX_ANT_DELAY_CALIB, kSr250TxAntennaDelayCalibration.data(),
	  kSr250TxAntennaDelayCalibration.size() },
} };
#endif

constexpr AliroError ConvertUwbError(aliro_uwb_err uwbErr)
{
	switch (uwbErr) {
	case ALIRO_UWB_ERR_NONE:
		return ALIRO_NO_ERROR;
	case ALIRO_UWB_ERR_INVALID_PARAMETER:
	case ALIRO_UWB_ERR_MSG_MALFORMED:
		return ALIRO_INVALID_ARGUMENT;
	case ALIRO_UWB_ERR_UWBS_TIMEOUT:
		return ALIRO_TIMEOUT;
	case ALIRO_UWB_ERR_SESSION_ACTIVE:
	case ALIRO_UWB_ERR_SESSION_CONFIG:
	case ALIRO_UWB_ERR_MESSAGE_STATE:
	case ALIRO_UWB_ERR_INVALID_STATE:
		return ALIRO_INVALID_STATE;
	default:
		return ALIRO_ERROR_INTERNAL;
	}
}

aliro_uwb_message *AllocateMessage(size_t length)
{
	return static_cast<aliro_uwb_message *>(qmalloc(sizeof(aliro_uwb_message) + length));
}

bool IsInitiateRangingSessionMessage(const uint8_t *data, size_t length)
{
	if (data == nullptr || length < kBleMessageHeaderLength + 1) {
		return false;
	}

	const uint16_t payloadLength = sys_get_be16(&data[2]);

	return data[0] == kNotificationProtocolType && data[1] == kNotificationRangingMessageId &&
	       payloadLength == length - kBleMessageHeaderLength && data[4] == kInitiateRangingSessionAttributeId;
}

bool IsRangingNotificationMessage(const uint8_t *data, size_t length)
{
	if (data == nullptr || length < kBleMessageHeaderLength) {
		return false;
	}

	const uint16_t payloadLength = sys_get_be16(&data[2]);

	return data[0] == kNotificationProtocolType && data[1] == kNotificationRangingMessageId &&
	       payloadLength == length - kBleMessageHeaderLength;
}

bool IsUwbRangingServiceMessage(const uint8_t *data, size_t length, uint8_t messageId)
{
	if (data == nullptr || length < kBleMessageHeaderLength) {
		return false;
	}

	const uint16_t payloadLength = sys_get_be16(&data[2]);

	return data[0] == kUwbRangingServiceProtocolType && data[1] == messageId &&
	       payloadLength == length - kBleMessageHeaderLength;
}

bool ParseSessionIdentifierAttribute(const uint8_t *data, size_t length, uint32_t &sessionId)
{
	if (length != kBleMessageHeaderLength + kRangingSessionIdPayloadLength) {
		return false;
	}

	const uint8_t *payload = &data[kBleMessageHeaderLength];
	if (payload[0] != kAttrRangingSessionId || payload[1] != sizeof(uint32_t)) {
		return false;
	}

	sessionId = sys_get_be32(&payload[2]);
	return true;
}

uint32_t SelectRangingDword(uint32_t requested, uint32_t supported, uint32_t fallback)
{
	const uint32_t common = requested & supported;

	if (common != 0) {
		return common;
	}

	LOG_WRN("No common RSS-M bitmask value between requested 0x%08x and supported 0x%08x; using 0x%08x",
		requested, supported, fallback);
	return fallback;
}

#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
AliroError ConvertNxpStatus(tUWBAPI_STATUS status)
{
	switch (status) {
	case UWBAPI_STATUS_OK:
		return ALIRO_NO_ERROR;
	case UWBAPI_STATUS_TIMEOUT:
		return ALIRO_TIMEOUT;
	case UWBAPI_STATUS_INVALID_PARAM:
	case UWBAPI_STATUS_INVALID_RANGE:
		return ALIRO_INVALID_ARGUMENT;
	case UWBAPI_STATUS_SESSION_NOT_EXIST:
		return ALIRO_SESSION_NOT_FOUND;
	case UWBAPI_STATUS_NOT_INITIALIZED:
		return ALIRO_UWB_INIT_FAILED;
	default:
		return ALIRO_ERROR_INTERNAL;
	}
}

void NxpUwbCallback(eNotificationType notificationType, void *data, size_t length)
{
	UltraWideBandImpl::HandleNxpNotification(static_cast<uint8_t>(notificationType), data, length);
}

UWB_AppParams_List_t AppParam(eAppConfig paramId, uint32_t value)
{
	UWB_AppParams_List_t param{};
	param.param_id = paramId;
	param.param_type = kUWB_APPPARAMS_Type_u32;
	param.param_value.vu32 = value;
	return param;
}

void LogNxpCapabilities(const phUwbCapInfo_t &caps)
{
	LOG_INF("SR250 capabilities: device types 0x%02x, ranging methods 0x%04x", caps.deviceTypes,
		caps.rangingMethod);
	LOG_INF("SR250 CCC caps: slot mask 0x%02x, channel mask 0x%02x, hopping mask 0x%02x, min RAN %u",
		caps.slotBitmask, caps.channelBitmask, caps.hoppingConfigBitmask, caps.minRanMultiplier);

	for (uint8_t i = 0; i < caps.numAliroSupportedProtocolVersions; ++i) {
		LOG_INF("SR250 Aliro protocol version[%u]: 0x%04x", i, caps.aliroSupportedProtocolVersion[i]);
	}

	for (uint8_t i = 0; i < caps.numSupportedUWBConfigIDs; ++i) {
		LOG_INF("SR250 supported UWB config ID[%u]: 0x%04x", i, caps.supportedUWBConfigIDs[i]);
	}

	LOG_INF("SR250 Aliro MAC mode mask: 0x%02x", caps.aliroSupportedMacMode);
	LOG_HEXDUMP_INF(caps.supportedPulseShapeCombo, sizeof(caps.supportedPulseShapeCombo),
			"SR250 supported pulse-shape combos");
}

phRangingData_t &ParsedRangingDataBuffer()
{
	static phRangingData_t sRangingData;

	memset(&sRangingData, 0, sizeof(sRangingData));
	return sRangingData;
}

phCsaRangingData_t &ParsedCsaRangingDataBuffer()
{
	static phCsaRangingData_t sRangingData;

	memset(&sRangingData, 0, sizeof(sRangingData));
	return sRangingData;
}
#endif

} // namespace

UltraWideBandImpl::SessionContext::SessionContext(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
						  ProtocolVersion protocolVersion,
						  SessionContextHandle sessionContextHandle)
	: mSessionId(sessionId), mUrsk(ursk), mProtocolVersion(protocolVersion),
	  mSessionContextHandle(sessionContextHandle)
{
}

UltraWideBandImpl::WrappedRds UltraWideBandImpl::BuildWrappedRds(uint32_t nxpSessionHandle,
								 const CryptoTypes::Ursk &ursk)
{
	WrappedRds wrappedRds{};

	/*
	 * SR250 CCC wrapped RDS format from the CSA controlee demo:
	 * Session ID(4) || Random(12) || Plain text Session Key(32).
	 * Aliro already gives this backend the plaintext URSK, so the demo's
	 * static SessionKey becomes the Aliro URSK.
	 */
	sys_put_le32(nxpSessionHandle, wrappedRds.data());
	std::copy(kRandomKey.begin(), kRandomKey.end(), wrappedRds.begin() + kSessionIdLength);
	std::copy(ursk.begin(), ursk.end(), wrappedRds.begin() + kSessionIdLength + kRandomKeyLength);

	return wrappedRds;
}

AliroError UltraWideBandImpl::ApplySr250Calibration()
{
#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
	size_t appliedCount{};

	for (const auto &entry : kSr250CalibrationEntries) {
		tUWBAPI_STATUS status = UwbApi_SetCalibration(entry.mChannel, entry.mParamId,
							       const_cast<uint8_t *>(entry.mValue),
							       entry.mLength);
		if (status != UWBAPI_STATUS_OK) {
			LOG_WRN("SR250 calibration %s failed on channel %u: 0x%02x", entry.mName,
				entry.mChannel, status);
			continue;
		}

		++appliedCount;
		LOG_INF("Applied SR250 calibration %s on channel %u (%u bytes)", entry.mName, entry.mChannel,
			entry.mLength);
	}

	if (appliedCount == 0) {
		LOG_WRN("No SR250 calibration entries were accepted by the device");
		return ALIRO_ERROR_INTERNAL;
	}

	LOG_INF("Applied %u/%u SR250 calibration entries from Mini250 reference table",
		static_cast<unsigned int>(appliedCount),
		static_cast<unsigned int>(kSr250CalibrationEntries.size()));
	return ALIRO_NO_ERROR;
#else
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif
}

AliroError UltraWideBandImpl::_Init(const Callbacks &callbacks)
{
	mCallbacks = callbacks;
	mInitialized = false;

	LOG_INF("TSRR250 UWB backend selected");

#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
	phUwbappContext_t appContext{};
	phUwbDevInfo_t deviceInfo{};
	phUwbCapInfo_t deviceCapabilities{};

#if UWB_BLD_CFG_FW_DNLD_DIRECTLY_FROM_HOST
	appContext.fwImageCtx.fwImage = const_cast<uint8_t *>(&heliosEncryptedMainlineFwImage[0]);
	appContext.fwImageCtx.fwImgSize = heliosEncryptedMainlineFwImageLen;
#endif
	appContext.fwImageCtx.fwMode = MAINLINE_FW;
	appContext.pCallback = NxpUwbCallback;
	appContext.pTmlCallback = nullptr;

	tUWBAPI_STATUS status = UwbApi_Initialize(&appContext);
	if (status != UWBAPI_STATUS_OK) {
		LOG_ERR("UwbApi_Initialize failed: 0x%02x", status);
		return ConvertNxpStatus(status);
	}

	status = UwbApi_GetDeviceInfo(&deviceInfo);
	if (status != UWBAPI_STATUS_OK) {
		LOG_ERR("UwbApi_GetDeviceInfo failed: 0x%02x", status);
		(void)UwbApi_ShutDown();
		return ConvertNxpStatus(status);
	}

	status = UwbApi_GetDeviceCapability(&deviceCapabilities);
	if (status != UWBAPI_STATUS_OK) {
		LOG_ERR("UwbApi_GetDeviceCapability failed: 0x%02x", status);
		(void)UwbApi_ShutDown();
		return ConvertNxpStatus(status);
	}
	LogNxpCapabilities(deviceCapabilities);

	AliroError calibrationError = ApplySr250Calibration();
	if (calibrationError != ALIRO_NO_ERROR) {
		LOG_WRN("SR250 calibration was not fully applied; ranging may be inaccurate: %d",
			calibrationError.ToInt());
	}

	mInitialized = true;
	LOG_INF("NXP SR250 UWB API initialized");
	return ALIRO_NO_ERROR;
#else
	LOG_INF("NXP SR250 UWB API module is not linked; ranging is disabled");
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif
}

AliroError UltraWideBandImpl::_Deinit()
{
#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
	if (mSession) {
		(void)_TerminateRangingSession(mSession->mSessionContextHandle);
	}
	if (mInitialized) {
		(void)UwbApi_ShutDown();
	}
#else
	mSession.reset();
#endif
	if (mAliroCtx != nullptr) {
		aliro_uwb_adapter_destroy(mAliroCtx);
		mAliroCtx = nullptr;
	}
	mInitialized = false;
	return ALIRO_NO_ERROR;
}

void UltraWideBandImpl::TransmitBleMessage(aliro_uwb_message *message, UwbSessionContext uwbSessionCtx,
					   void *userData, [[maybe_unused]] bool timeout)
{
	if (message == nullptr || userData == nullptr) {
		LOG_ERR("Invalid Aliro UWB BLE transmit callback argument");
		goto exit;
	}

	{
		auto *uwbImpl = static_cast<UltraWideBandImpl *>(userData);
		auto *session = uwbImpl->FindSession(uwbSessionCtx);
		if (session == nullptr) {
			LOG_ERR("TSRR250 Aliro session context not found");
			goto exit;
		}

		Aliro::AliroStack::Instance().SendBleMessage(session->mSessionContextHandle, message->data,
							     message->len);
	}

exit:
	if (message != nullptr) {
		aliro_uwb_session_message_free(message);
	}
}

void UltraWideBandImpl::SessionHandlerCallback(aliro_uwb_session_event *event, void *userData)
{
	if (event == nullptr || userData == nullptr) {
		LOG_ERR("Invalid Aliro UWB session callback argument");
		goto exit;
	}

	{
		auto *uwbImpl = static_cast<UltraWideBandImpl *>(userData);
		auto *session = uwbImpl->FindSession(event->session);
		if (session == nullptr) {
			LOG_WRN("TSRR250 Aliro session event for unknown session");
			goto exit;
		}

		switch (event->type) {
		case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_STATUS: {
			const auto newState = event->data.status->session_state;

			if (newState == CHERRY_CCC_SESSION_STATE_IDLE) {
				uwbImpl->NotifySessionState(*session, RangingSessionState::Idle);
			} else if (newState == CHERRY_CCC_SESSION_STATE_ACTIVE) {
				const AliroError err = uwbImpl->StartNxpRangingSession(*session);
				if (err != ALIRO_NO_ERROR) {
					LOG_ERR("Failed to start TSRR250 ranging after RSS exchange: %d",
						err.ToInt());
				}
			} else if (newState == CHERRY_CCC_SESSION_STATE_DEINIT) {
				uwbImpl->NotifySessionState(*session, RangingSessionState::Destroyed);
			}
			break;
		}
		case ALIRO_UWB_SESSION_EVENT_TYPE_SESSION_ERROR:
			LOG_ERR("TSRR250 Aliro UWB session error: 0x%x",
				static_cast<uint32_t>(event->data.error->status_err));
			break;
		default:
			break;
		}
	}

exit:
	if (event != nullptr) {
		aliro_uwb_session_event_free(event);
	}
}

void UltraWideBandImpl::_BleTimeSync()
{
	/* The TSRR250 wiring provided does not include a BLE/UWB timesync GPIO. */
}

AliroError UltraWideBandImpl::_HandleBleMessage(const uint8_t *data, size_t length,
						SessionContextHandle sessionContextData)
{
	if (data == nullptr || length == 0) {
		return ALIRO_INVALID_ARGUMENT;
	}

	auto *session = FindSession(sessionContextData);
	if (session == nullptr) {
		if (mSession != nullptr) {
			LOG_WRN("Using active TSRR250 Aliro UWB session despite context handle mismatch: active %p, incoming %p",
				mSession->mSessionContextHandle.GetRaw(), sessionContextData.GetRaw());
			session = mSession.get();
		} else {
			LOG_ERR("Cannot handle TSRR250 Aliro UWB BLE message: session context not found for handle: %p",
				sessionContextData.GetRaw());
			return ALIRO_SESSION_NOT_FOUND;
		}
	}

	LOG_HEXDUMP_DBG(data, length, "TSRR250 incoming RSS message");

	if (IsInitiateRangingSessionMessage(data, length)) {
		LOG_INF("Handling TSRR250 InitiateRangingSession with native RSS-M1 builder");
		return SendRssM1(*session);
	}

	if (IsRangingNotificationMessage(data, length)) {
		return HandleRangingNotification(*session, data, length);
	}

	if (IsUwbRangingServiceMessage(data, length, kRssM2MessageId)) {
		LOG_INF("Handling TSRR250 RSS-M2 with native RSS-M3 builder");
		return HandleRssM2(*session, data, length);
	}

	if (IsUwbRangingServiceMessage(data, length, kRssM4MessageId)) {
		LOG_INF("Handling TSRR250 RSS-M4");
		return HandleRssM4(*session, data, length);
	}

	if (IsUwbRangingServiceMessage(data, length, kRangingSessionSuspendRequestMessageId)) {
		LOG_INF("Handling TSRR250 Ranging Session Suspend Request");
		return HandleRangingSessionSuspendRequest(*session, data, length);
	}

	if (IsUwbRangingServiceMessage(data, length, kRangingSessionResumeRequestMessageId)) {
		LOG_INF("Handling TSRR250 Ranging Session Resume Request");
		return HandleRangingSessionResumeRequest(*session, data, length);
	}

	if (IsUwbRangingServiceMessage(data, length, kRangingSessionResumeResponseMessageId)) {
		LOG_INF("Handling TSRR250 Ranging Session Resume Response");
		return HandleRangingSessionResumeResponse(*session, data, length);
	}

	if (session->mAliroSessionContext == nullptr) {
		LOG_INF("Creating TSRR250 Aliro UWB session for incoming ranging message");
		AliroError err = CreateAliroSession(*session);
		if (err != ALIRO_NO_ERROR) {
			LOG_ERR("Cannot handle TSRR250 Aliro UWB BLE message: session creation failed: %d", err.ToInt());
			return err;
		}
	}

	aliro_uwb_message *message = AllocateMessage(length);
	if (message == nullptr) {
		return ALIRO_NO_MEMORY;
	}

	memcpy(message->data, data, length);
	message->len = length;

	const auto err = aliro_uwb_session_message_handle(session->mAliroSessionContext, message);
	qfree(message);

	if (err != ALIRO_UWB_ERR_NONE) {
		LOG_ERR("Cannot handle TSRR250 Aliro UWB BLE message: 0x%x", static_cast<uint32_t>(err));
		return ConvertUwbError(err);
	}

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::_ConfigureRangingSession(SessionIdentifier sessionId, const CryptoTypes::Ursk &ursk,
						       ProtocolVersion protocolVersion,
						       SessionContextHandle sessionContextHandle)
{
	mSession = std::make_unique<SessionContext>(sessionId, ursk, protocolVersion, sessionContextHandle);
	LOG_INF("Prepared TSRR250 Aliro session 0x%08x, protocol 0x%04x", sessionId, protocolVersion);

	AliroError err = ConfigureNxpSession(*mSession);
	if (err != ALIRO_NO_ERROR) {
		mSession.reset();
		return err;
	}

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::_InitiateRangingSession(SessionContextHandle sessionContextData)
{
	LOG_INF("Initiating TSRR250 Aliro RSS setup for session handle: %p", sessionContextData.GetRaw());

	SessionContext *session = FindSession(sessionContextData);
	if (session == nullptr) {
		LOG_ERR("Cannot initiate TSRR250 RSS setup: session context not found for handle: %p",
			sessionContextData.GetRaw());
		return ALIRO_SESSION_NOT_FOUND;
	}

	return SendRssM1(*session);
}

AliroError UltraWideBandImpl::_TerminateRangingSession(SessionContextHandle sessionContextData)
{
	SessionContext *session = FindSession(sessionContextData);
	if (session == nullptr) {
		return ALIRO_SESSION_NOT_FOUND;
	}

#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
	if (session->mRangingStarted) {
		tUWBAPI_STATUS status = UwbApi_StopRangingSession(session->mNxpSessionHandle);
		if (status != UWBAPI_STATUS_OK) {
			LOG_WRN("UwbApi_StopRangingSession failed: 0x%02x", status);
		}
		session->mRangingStarted = false;
	}

	if (session->mNxpSessionInitialized) {
		tUWBAPI_STATUS status = UwbApi_SessionDeinit(session->mNxpSessionHandle);
		if (status != UWBAPI_STATUS_OK) {
			LOG_WRN("UwbApi_SessionDeinit failed: 0x%02x", status);
		}
	}

	NotifySessionState(*session, RangingSessionState::Destroyed);
#endif
	if (session->mAliroSessionContext != nullptr) {
		aliro_uwb_session_destroy(session->mAliroSessionContext);
		session->mAliroSessionContext = nullptr;
	}
	mSession.reset();
	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::_SuspendRangingSession(SessionContextHandle sessionContextData, bool force)
{
	ARG_UNUSED(force);

	SessionContext *session = FindSession(sessionContextData);
	if (session == nullptr) {
		return ALIRO_SESSION_NOT_FOUND;
	}

	return StopNxpRangingSession(*session);
}

AliroError UltraWideBandImpl::_ResumeRangingSession(SessionContextHandle sessionContextData)
{
	SessionContext *session = FindSession(sessionContextData);
	if (session == nullptr) {
		return ALIRO_SESSION_NOT_FOUND;
	}

	return StartNxpRangingSession(*session);
}

AliroError UltraWideBandImpl::CreateAliroAdapter()
{
	if (mAliroCtx != nullptr) {
		return ALIRO_NO_ERROR;
	}

	mAliroCtx = aliro_uwb_adapter_create_reader(nullptr, &mDeviceCaps, &mReaderConfig);
	if (mAliroCtx == nullptr) {
		LOG_ERR("Failed to create TSRR250 Aliro UWB adapter: Qorvo adapter requires a Cherry context");
		LOG_ERR("TSRR250 UWB RSS setup needs a TSRR250-backed Cherry/UCI bridge or native RSS message builder");
		return ALIRO_ERROR_INTERNAL;
	}

	LOG_INF("Created TSRR250 Aliro UWB adapter");
	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::CreateAliroSession(SessionContext &session)
{
	AliroError err = CreateAliroAdapter();
	if (err != ALIRO_NO_ERROR) {
		return err;
	}

	session.mAliroSessionContext =
		aliro_uwb_session_create(mAliroCtx, session.mSessionId, &SessionHandlerCallback, &TransmitBleMessage,
					 this);
	if (session.mAliroSessionContext == nullptr) {
		LOG_ERR("Failed to create TSRR250 Aliro UWB session");
		return ALIRO_NO_MEMORY;
	}

	err = ConvertUwbError(aliro_uwb_session_set_ursk(session.mAliroSessionContext, session.mUrsk.data()));
	if (err != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to set TSRR250 Aliro UWB URSK: %d", err.ToInt());
		return err;
	}

	err = ConvertUwbError(
		aliro_uwb_session_set_protocol_version(session.mAliroSessionContext, session.mProtocolVersion));
	if (err != ALIRO_NO_ERROR) {
		LOG_ERR("Failed to set TSRR250 Aliro UWB protocol version: %d", err.ToInt());
		return err;
	}

	LOG_INF("Created TSRR250 Aliro UWB session");
	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::HandleRssM2(SessionContext &session, const uint8_t *data, size_t length)
{
	if (!session.mRssM1Sent || session.mRssM3Sent) {
		LOG_ERR("RSS-M2 received in invalid state: m1=%u m3=%u", session.mRssM1Sent, session.mRssM3Sent);
		return ALIRO_INVALID_STATE;
	}

	SessionContext::RssM2Selection selection{};
	bool hasUwbConfig{};
	bool hasPulseShape{};
	bool hasChannel{};
	bool hasSyncCodeBitmask{};
	bool hasRan{};
	bool hasSlotBitmask{};
	bool hasHopping{};

	size_t offset = kBleMessageHeaderLength;
	while (offset < length) {
		if (length - offset < 2) {
			LOG_ERR("Malformed RSS-M2 TLV at offset %u", static_cast<unsigned int>(offset));
			return ALIRO_INVALID_ARGUMENT;
		}

		const uint8_t attrId = data[offset++];
		const uint8_t attrLength = data[offset++];

		if (length - offset < attrLength) {
			LOG_ERR("Malformed RSS-M2 attribute 0x%02x length %u", attrId, attrLength);
			return ALIRO_INVALID_ARGUMENT;
		}

		const uint8_t *value = &data[offset];
		switch (attrId) {
		case kAttrUwbConfigId:
			if (attrLength != sizeof(uint16_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			selection.mUwbConfigId = sys_get_be16(value);
			hasUwbConfig = true;
			break;
		case kAttrPulseShapeCombo:
			if (attrLength != sizeof(uint8_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			selection.mPulseShapeCombo = value[0];
			hasPulseShape = true;
			break;
		case kAttrChannelBitmask:
			if (attrLength != sizeof(uint8_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			selection.mChannelBitmask = value[0];
			hasChannel = true;
			break;
		case kAttrSyncCodeIndexBitmask:
			if (attrLength != sizeof(uint32_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			selection.mSyncCodeIndexBitmask = sys_get_be32(value);
			hasSyncCodeBitmask = true;
			break;
		case kAttrRanMultiplier:
			if (attrLength != sizeof(uint8_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			selection.mRanMultiplier = value[0];
			hasRan = true;
			break;
		case kAttrSlotBitmask:
			if (attrLength != sizeof(uint8_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			selection.mSlotBitmask = value[0];
			hasSlotBitmask = true;
			break;
		case kAttrHoppingConfig:
			if (attrLength != sizeof(uint8_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			selection.mHoppingConfigBitmask = value[0];
			hasHopping = true;
			break;
		default:
			LOG_WRN("Ignoring optional/unknown RSS-M2 attribute 0x%02x", attrId);
			break;
		}

		offset += attrLength;
	}

	if (!hasUwbConfig || !hasPulseShape || !hasChannel || !hasSyncCodeBitmask || !hasRan || !hasSlotBitmask ||
	    !hasHopping) {
		LOG_ERR("RSS-M2 missing required attributes: cfg=%u pulse=%u ch=%u sync=%u ran=%u slot=%u hop=%u",
			hasUwbConfig, hasPulseShape, hasChannel, hasSyncCodeBitmask, hasRan, hasSlotBitmask,
			hasHopping);
		return ALIRO_INVALID_ARGUMENT;
	}

	selection.mValid = true;
	session.mRssM2 = selection;

	LOG_INF("RSS-M2 selection: cfg=0x%04x pulse=0x%02x ch=0x%02x sync=0x%08x ran=%u slot=0x%02x hop=0x%02x",
		selection.mUwbConfigId, selection.mPulseShapeCombo, selection.mChannelBitmask,
		selection.mSyncCodeIndexBitmask, selection.mRanMultiplier, selection.mSlotBitmask,
		selection.mHoppingConfigBitmask);

	return SendRssM3(session);
}

AliroError UltraWideBandImpl::HandleRssM4(SessionContext &session, const uint8_t *data, size_t length)
{
	if (!session.mRssM3Sent || session.mRssM4Received) {
		LOG_ERR("RSS-M4 received in invalid state: m3=%u m4=%u", session.mRssM3Sent,
			session.mRssM4Received);
		return ALIRO_INVALID_STATE;
	}

	bool hasStsIndex{};
	bool hasUwbTime{};
	bool hasHopModeKey{};
	bool hasSyncCodeIndex{};
	uint32_t stsIndex{};
	uint8_t syncCodeIndex{};

	size_t offset = kBleMessageHeaderLength;
	while (offset < length) {
		if (length - offset < 2) {
			LOG_ERR("Malformed RSS-M4 TLV at offset %u", static_cast<unsigned int>(offset));
			return ALIRO_INVALID_ARGUMENT;
		}

		const uint8_t attrId = data[offset++];
		const uint8_t attrLength = data[offset++];

		if (length - offset < attrLength) {
			LOG_ERR("Malformed RSS-M4 attribute 0x%02x length %u", attrId, attrLength);
			return ALIRO_INVALID_ARGUMENT;
		}

		const uint8_t *value = &data[offset];
		switch (attrId) {
		case kAttrStsIndex0:
			if (attrLength != sizeof(uint32_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			stsIndex = sys_get_be32(value);
			hasStsIndex = true;
			break;
		case kAttrUwbTime0:
			if (attrLength != sizeof(uint64_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			hasUwbTime = true;
			break;
		case kAttrHopModeKey:
			if (attrLength != sizeof(uint32_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			hasHopModeKey = true;
			break;
		case kAttrSyncCodeIndex:
			if (attrLength != sizeof(uint8_t)) {
				return ALIRO_INVALID_ARGUMENT;
			}
			syncCodeIndex = value[0];
			hasSyncCodeIndex = true;
			break;
		default:
			LOG_WRN("Ignoring optional/unknown RSS-M4 attribute 0x%02x", attrId);
			break;
		}

		offset += attrLength;
	}

	if (!hasStsIndex || !hasUwbTime || !hasHopModeKey || !hasSyncCodeIndex) {
		LOG_ERR("RSS-M4 missing required attributes: sts=%u time=%u hopKey=%u sync=%u", hasStsIndex,
			hasUwbTime, hasHopModeKey, hasSyncCodeIndex);
		return ALIRO_INVALID_ARGUMENT;
	}

	session.mRssM4Received = true;
	session.mLastStsIndex0 = stsIndex;
	session.mSyncCodeIndex = syncCodeIndex;
	LOG_INF("RSS-M4 accepted: stsIndex=%u syncCodeIndex=%u", stsIndex, syncCodeIndex);

	return StartNxpRangingSession(session);
}

AliroError UltraWideBandImpl::HandleRangingSessionSuspendRequest(SessionContext &session, const uint8_t *data,
								 size_t length)
{
	uint32_t requestedSessionId{};
	if (!ParseSessionIdentifierAttribute(data, length, requestedSessionId)) {
		LOG_ERR("Malformed Ranging Session Suspend Request");
		return SendRssGeneralError(session, kWrongParametersErrorReason);
	}

	if (requestedSessionId != session.mSessionId) {
		LOG_ERR("Ranging Session Suspend Request session mismatch: got 0x%08x, expected 0x%08x",
			requestedSessionId, session.mSessionId);
		return SendRssGeneralError(session, kWrongParametersErrorReason);
	}

	AliroError err = StopNxpRangingSession(session);
	if (err != ALIRO_NO_ERROR) {
		return err;
	}

	return SendRangingSessionSuspendResponse(session, kSuccessStatus);
}

AliroError UltraWideBandImpl::HandleRangingSessionResumeRequest(SessionContext &session, const uint8_t *data,
								size_t length)
{
	uint32_t requestedSessionId{};
	if (!ParseSessionIdentifierAttribute(data, length, requestedSessionId)) {
		LOG_ERR("Malformed Ranging Session Resume Request");
		return SendRssGeneralError(session, kWrongParametersErrorReason);
	}

	if (requestedSessionId != session.mSessionId) {
		LOG_ERR("Ranging Session Resume Request session mismatch: got 0x%08x, expected 0x%08x",
			requestedSessionId, session.mSessionId);
		return SendRssGeneralError(session, kWrongParametersErrorReason);
	}

	AliroError err = StartNxpRangingSession(session);
	if (err != ALIRO_NO_ERROR) {
		return err;
	}

	return SendRangingSessionResumeResponse(session);
}

AliroError UltraWideBandImpl::HandleRangingSessionResumeResponse(SessionContext &session, const uint8_t *data,
								 size_t length)
{
	if (length != kBleMessageHeaderLength + kRangingSessionResumeResponsePayloadLength) {
		LOG_ERR("Malformed Ranging Session Resume Response length: %u", static_cast<unsigned int>(length));
		return SendRssGeneralError(session, kWrongParametersErrorReason);
	}

	const uint8_t *payload = &data[kBleMessageHeaderLength];
	if (payload[0] != kAttrStsIndex0 || payload[1] != sizeof(uint32_t) ||
	    payload[6] != kAttrUwbTime0 || payload[7] != sizeof(uint64_t)) {
		LOG_ERR("Malformed Ranging Session Resume Response attributes");
		return SendRssGeneralError(session, kWrongParametersErrorReason);
	}

	const uint32_t stsIndex = sys_get_be32(&payload[2]);
	if (stsIndex > 0x3FFFFFFF) {
		LOG_ERR("Invalid Ranging Session Resume Response STS index: 0x%08x", stsIndex);
		return SendRssGeneralError(session, kWrongParametersErrorReason);
	}

	session.mLastStsIndex0 = stsIndex;
	LOG_INF("Ranging Session Resume Response accepted: stsIndex=%u", stsIndex);
	return StartNxpRangingSession(session);
}

AliroError UltraWideBandImpl::HandleRangingNotification(SessionContext &session, const uint8_t *data, size_t length)
{
	if (length < kBleMessageHeaderLength + 2) {
		LOG_ERR("Malformed Ranging notification");
		return SendRssGeneralError(session, kWrongParametersErrorReason);
	}

	const uint16_t payloadLength = sys_get_be16(&data[2]);
	if (payloadLength != 2 || data[5] != 0) {
		LOG_ERR("Unexpected Ranging notification payload");
		return SendRssGeneralError(session, kWrongParametersErrorReason);
	}

	switch (data[4]) {
	case kInitiateRangingSessionResumeAttributeId:
		LOG_INF("Handling InitiateRangingSessionResume notification");
		return SendRangingSessionResumeRequest(session);
	case kRangingSessionSuspendedAttributeId:
		LOG_INF("Handling RangingSessionSuspended notification");
		return StopNxpRangingSession(session);
	default:
		LOG_WRN("Ignoring unsupported Ranging notification attribute 0x%02x", data[4]);
		return ALIRO_NO_ERROR;
	}
}

AliroError UltraWideBandImpl::SendRssM1(SessionContext &session)
{
	const uint16_t payloadLength = static_cast<uint16_t>(kRssM1FixedPayloadLength +
							    (mUwbConfigs.size() * sizeof(uint16_t)) +
							    mPulseShapeCombos.size());
	std::array<uint8_t, 64> message{};
	size_t offset = 0;

	auto append8 = [&](uint8_t value) {
		message[offset++] = value;
	};
	auto append16 = [&](uint16_t value) {
		sys_put_be16(value, &message[offset]);
		offset += sizeof(uint16_t);
	};
	auto append32 = [&](uint32_t value) {
		sys_put_be32(value, &message[offset]);
		offset += sizeof(uint32_t);
	};

	append8(kUwbRangingServiceProtocolType);
	append8(kRssM1MessageId);
	append16(payloadLength);

	append8(kAttrUwbConfigId);
	append8(static_cast<uint8_t>(mUwbConfigs.size() * sizeof(uint16_t)));
	for (const uint16_t configId : mUwbConfigs) {
		append16(configId);
	}

	append8(kAttrPulseShapeCombo);
	append8(static_cast<uint8_t>(mPulseShapeCombos.size()));
	for (const uint8_t combo : mPulseShapeCombos) {
		append8(combo);
	}

	append8(kAttrChannelBitmask);
	append8(1);
	append8(mCccCaps.channel_bitmask);

	append8(kAttrRangingSessionId);
	append8(sizeof(uint32_t));
	append32(session.mSessionId);

	if (offset != (payloadLength + kBleMessageHeaderLength)) {
		LOG_ERR("Invalid TSRR250 RSS-M1 length: %u, expected %u", offset,
			payloadLength + kBleMessageHeaderLength);
		return ALIRO_ERROR_INTERNAL;
	}

	LOG_HEXDUMP_DBG(message.data(), offset, "TSRR250 RSS-M1");
	Aliro::AliroStack::Instance().SendBleMessage(session.mSessionContextHandle, message.data(), offset);
	LOG_INF("Sent TSRR250 RSS-M1 for session 0x%08x", session.mSessionId);
	session.mRssM1Sent = true;

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::SendRssM3(SessionContext &session)
{
	std::array<uint8_t, 64> message{};
	size_t offset = 0;

	auto append8 = [&](uint8_t value) {
		message[offset++] = value;
	};
	auto append16 = [&](uint16_t value) {
		sys_put_be16(value, &message[offset]);
		offset += sizeof(uint16_t);
	};
	auto append32 = [&](uint32_t value) {
		sys_put_be32(value, &message[offset]);
		offset += sizeof(uint32_t);
	};

	if (!session.mRssM2.mValid) {
		LOG_ERR("Cannot send RSS-M3 before a valid RSS-M2 selection");
		return ALIRO_INVALID_STATE;
	}

	const uint8_t ranMultiplier =
		std::max<uint8_t>(CONFIG_DOOR_LOCK_UWB_MIN_RAN_MULTIPLIER, session.mRssM2.mRanMultiplier);
	const uint32_t syncCodeIndexBitmask = SelectRangingDword(session.mRssM2.mSyncCodeIndexBitmask,
								 mCccCaps.sync_code_index_bitmask,
								 mCccCaps.sync_code_index_bitmask);
	const uint8_t hoppingConfig = kDefaultHoppingConfig;

	append8(kUwbRangingServiceProtocolType);
	append8(kRssM3MessageId);
	append16(kRssM3PayloadLength);

	append8(kAttrRanMultiplier);
	append8(1);
	append8(ranMultiplier);

	append8(kAttrChapsPerSlot);
	append8(1);
	append8(kDefaultChapsPerSlot);

	append8(kAttrResponderNodesCount);
	append8(1);
	append8(kDefaultResponderNodesCount);

	append8(kAttrSlotsPerRound);
	append8(1);
	append8(kDefaultSlotsPerRound);

	append8(kAttrSyncCodeIndexBitmask);
	append8(sizeof(uint32_t));
	append32(syncCodeIndexBitmask);

	append8(kAttrHoppingConfig);
	append8(1);
	append8(hoppingConfig);

	append8(kAttrMacMode);
	append8(1);
	append8(kDefaultMacMode);

	if (offset != (kRssM3PayloadLength + kBleMessageHeaderLength)) {
		LOG_ERR("Invalid TSRR250 RSS-M3 length: %u, expected %u", static_cast<unsigned int>(offset),
			static_cast<unsigned int>(kRssM3PayloadLength + kBleMessageHeaderLength));
		return ALIRO_ERROR_INTERNAL;
	}

	LOG_HEXDUMP_DBG(message.data(), offset, "TSRR250 RSS-M3");
	Aliro::AliroStack::Instance().SendBleMessage(session.mSessionContextHandle, message.data(), offset);
	LOG_INF("Sent TSRR250 RSS-M3 for session 0x%08x: ran=%u chaps=%u slots=%u sync=0x%08x hop=0x%02x mac=0x%02x",
		session.mSessionId, ranMultiplier, kDefaultChapsPerSlot, kDefaultSlotsPerRound,
		syncCodeIndexBitmask, hoppingConfig, kDefaultMacMode);
	session.mRssM3Sent = true;

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::SendRssGeneralError(SessionContext &session, uint8_t reason)
{
	std::array<uint8_t, kBleMessageHeaderLength + kRssGeneralErrorPayloadLength> message{};

	message[0] = kNotificationProtocolType;
	message[1] = kNotificationEventMessageId;
	sys_put_be16(kRssGeneralErrorPayloadLength, &message[2]);
	message[4] = kAttrError;
	message[5] = 1;
	message[6] = reason;

	LOG_HEXDUMP_DBG(message.data(), message.size(), "TSRR250 RSS general error");
	Aliro::AliroStack::Instance().SendBleMessage(session.mSessionContextHandle, message.data(), message.size());

	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::SendRangingSessionSuspendRequest(SessionContext &session)
{
	std::array<uint8_t, kBleMessageHeaderLength + kRangingSessionIdPayloadLength> message{};

	message[0] = kUwbRangingServiceProtocolType;
	message[1] = kRangingSessionSuspendRequestMessageId;
	sys_put_be16(kRangingSessionIdPayloadLength, &message[2]);
	message[4] = kAttrRangingSessionId;
	message[5] = sizeof(uint32_t);
	sys_put_be32(session.mSessionId, &message[6]);

	LOG_HEXDUMP_DBG(message.data(), message.size(), "TSRR250 Ranging Session Suspend Request");
	Aliro::AliroStack::Instance().SendBleMessage(session.mSessionContextHandle, message.data(), message.size());
	LOG_INF("Sent TSRR250 Ranging Session Suspend Request for session 0x%08x", session.mSessionId);
	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::SendRangingSessionSuspendResponse(SessionContext &session, uint8_t status)
{
	std::array<uint8_t, kBleMessageHeaderLength + kRangingSessionStatusPayloadLength> message{};

	message[0] = kUwbRangingServiceProtocolType;
	message[1] = kRangingSessionSuspendResponseMessageId;
	sys_put_be16(kRangingSessionStatusPayloadLength, &message[2]);
	message[4] = kAttrStatus;
	message[5] = sizeof(uint8_t);
	message[6] = status;

	LOG_HEXDUMP_DBG(message.data(), message.size(), "TSRR250 Ranging Session Suspend Response");
	Aliro::AliroStack::Instance().SendBleMessage(session.mSessionContextHandle, message.data(), message.size());
	LOG_INF("Sent TSRR250 Ranging Session Suspend Response status=0x%02x", status);
	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::SendRangingSessionResumeRequest(SessionContext &session)
{
	std::array<uint8_t, kBleMessageHeaderLength + kRangingSessionIdPayloadLength> message{};

	message[0] = kUwbRangingServiceProtocolType;
	message[1] = kRangingSessionResumeRequestMessageId;
	sys_put_be16(kRangingSessionIdPayloadLength, &message[2]);
	message[4] = kAttrRangingSessionId;
	message[5] = sizeof(uint32_t);
	sys_put_be32(session.mSessionId, &message[6]);

	LOG_HEXDUMP_DBG(message.data(), message.size(), "TSRR250 Ranging Session Resume Request");
	Aliro::AliroStack::Instance().SendBleMessage(session.mSessionContextHandle, message.data(), message.size());
	LOG_INF("Sent TSRR250 Ranging Session Resume Request for session 0x%08x", session.mSessionId);
	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::SendRangingSessionResumeResponse(SessionContext &session)
{
	std::array<uint8_t, kBleMessageHeaderLength + kRangingSessionResumeResponsePayloadLength> message{};
	const uint32_t stsIndex = session.mLastStsIndex0 + 1;

	message[0] = kUwbRangingServiceProtocolType;
	message[1] = kRangingSessionResumeResponseMessageId;
	sys_put_be16(kRangingSessionResumeResponsePayloadLength, &message[2]);
	message[4] = kAttrStsIndex0;
	message[5] = sizeof(uint32_t);
	sys_put_be32(stsIndex, &message[6]);
	message[10] = kAttrUwbTime0;
	message[11] = sizeof(uint64_t);

	session.mLastStsIndex0 = stsIndex;
	LOG_HEXDUMP_DBG(message.data(), message.size(), "TSRR250 Ranging Session Resume Response");
	Aliro::AliroStack::Instance().SendBleMessage(session.mSessionContextHandle, message.data(), message.size());
	LOG_INF("Sent TSRR250 Ranging Session Resume Response stsIndex=%u", stsIndex);
	return ALIRO_NO_ERROR;
}

AliroError UltraWideBandImpl::ConfigureNxpSession(SessionContext &session)
{
#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
	if (!mInitialized) {
		return ALIRO_UWB_INIT_FAILED;
	}

	tUWBAPI_STATUS status = UwbApi_SessionInit(session.mSessionId, UWBD_CSA_SESSION, &session.mNxpSessionHandle);
	if (status != UWBAPI_STATUS_OK) {
		LOG_ERR("UwbApi_SessionInit failed: 0x%02x", status);
		return ConvertNxpStatus(status);
	}
	session.mNxpSessionInitialized = true;
	ResetCsaDistanceFilter(session);

	phCccRangingParams_t cccRangingParams{};
	cccRangingParams.deviceType = kUWB_DeviceType_CCC_Controlee;
	cccRangingParams.noOfControlees = kDefaultResponderNodesCount;
	cccRangingParams.responderSlotIndex = 0;
	cccRangingParams.slotDuration = kNxpNearbySlotDurationRstu;
	cccRangingParams.hoppingMode = 0;

	status = UwbApi_SetCccRangingParams(session.mNxpSessionHandle, &cccRangingParams);
	if (status != UWBAPI_STATUS_OK) {
		LOG_ERR("UwbApi_SetCccRangingParams failed: 0x%02x", status);
		return ConvertNxpStatus(status);
	}

	session.mWrappedRds = BuildWrappedRds(session.mNxpSessionHandle, session.mUrsk);
	status = UwbApi_SetAppConfigWrappedRDS(session.mNxpSessionHandle, session.mWrappedRds.data(),
					      session.mWrappedRds.size());
	if (status != UWBAPI_STATUS_OK) {
		LOG_ERR("UwbApi_SetAppConfigWrappedRDS failed: 0x%02x", status);
		return ConvertNxpStatus(status);
	}

	const UWB_AppParams_List_t appParams[] = {
		AppParam(DEVICE_ROLE, kUWB_DeviceRole_Responder),
		AppParam(MULTI_NODE_MODE, kUWB_MultiNodeMode_UniCast),
		AppParam(SCHEDULED_MODE, kUWB_ScheduledMode_TimeScheduled),
		AppParam(RANGING_ROUND_USAGE, kUWB_RangingRoundUsage_DS_TWR),
		AppParam(STS_CONFIG, kUWB_StsConfig_DynamicSts),
		AppParam(CHANNEL_NUMBER, CH_9),
		AppParam(RANGING_DURATION, 96),
		AppParam(STS_INDEX, 0),
		AppParam(AOA_RESULT_REQ, 0x01),
		AppParam(PREAMBLE_CODE_INDEX, kNxpNearbyPreambleCodeIndex),
		AppParam(SLOTS_PER_RR, kDefaultSlotsPerRound),
		AppParam(MAX_NUMBER_OF_MEASUREMENTS, 0xFFFF),
		AppParam(URSK_TTL, 720),
		AppParam(RANGING_PROTOCOL_VER, session.mProtocolVersion),
		AppParam(UWB_CONFIG_ID, 0xFFFF),
		AppParam(PULSESHAPE_COMBO, 0x00),
		AppParam(SESSION_INFO_NTF, 0x01),
	};

	status = UwbApi_SetAppConfigMultipleParams(session.mNxpSessionHandle, static_cast<uint8_t>(std::size(appParams)),
						  appParams);
	if (status != UWBAPI_STATUS_OK) {
		LOG_ERR("UwbApi_SetAppConfigMultipleParams CSA config failed: 0x%02x", status);
		return ConvertNxpStatus(status);
	}

	LOG_INF("Configured NXP Aliro CSA ranging session: handle=0x%08x slots=%u duration=%u protocol=0x%04x",
		session.mNxpSessionHandle, kDefaultSlotsPerRound, 96, session.mProtocolVersion);

	NotifySessionState(session, RangingSessionState::Idle);
	return ALIRO_NO_ERROR;
#else
	return ALIRO_NO_ERROR;
#endif
}

AliroError UltraWideBandImpl::StartNxpRangingSession(SessionContext &session)
{
#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
	if (session.mRangingStarted) {
		return ALIRO_NO_ERROR;
	}

	const bool resuming = session.mState == RangingSessionState::RangingSuspended;
	const UWB_AppParams_List_t startParams[] = {
		AppParam(STS_INDEX, session.mLastStsIndex0),
		AppParam(PREAMBLE_CODE_INDEX, session.mSyncCodeIndex),
		AppParam(UWB_CONFIG_ID, session.mRssM2.mValid ? session.mRssM2.mUwbConfigId : 0xFFFF),
		AppParam(PULSESHAPE_COMBO, session.mRssM2.mValid ? session.mRssM2.mPulseShapeCombo : 0x00),
	};

	tUWBAPI_STATUS status = UwbApi_SetAppConfigMultipleParams(session.mNxpSessionHandle,
								 static_cast<uint8_t>(std::size(startParams)),
								 startParams);
	if (status != UWBAPI_STATUS_OK) {
		LOG_ERR("UwbApi_SetAppConfigMultipleParams start config failed: 0x%02x", status);
		return ConvertNxpStatus(status);
	}

	LOG_INF("Starting NXP Aliro CSA ranging session with stsIndex=%u syncCodeIndex=%u cfg=0x%04x pulse=0x%02x",
		session.mLastStsIndex0, session.mSyncCodeIndex,
		session.mRssM2.mValid ? session.mRssM2.mUwbConfigId : 0xFFFF,
		session.mRssM2.mValid ? session.mRssM2.mPulseShapeCombo : 0x00);

	status = UwbApi_StartRangingSession(session.mNxpSessionHandle);
	if (status != UWBAPI_STATUS_OK) {
		LOG_ERR("UwbApi_StartRangingSession failed: 0x%02x", status);
		return ConvertNxpStatus(status);
	}

	session.mRangingStarted = true;
	ResetCsaDistanceFilter(session);
	NotifySessionState(session, resuming ? RangingSessionState::RangingResumed : RangingSessionState::Ranging);
	return ALIRO_NO_ERROR;
#else
	ARG_UNUSED(session);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif
}

AliroError UltraWideBandImpl::StopNxpRangingSession(SessionContext &session)
{
#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
	if (session.mRangingStarted) {
		tUWBAPI_STATUS status = UwbApi_StopRangingSession(session.mNxpSessionHandle);
		if (status != UWBAPI_STATUS_OK) {
			LOG_WRN("UwbApi_StopRangingSession failed: 0x%02x", status);
			return ConvertNxpStatus(status);
		}
		session.mRangingStarted = false;
	}
	ResetCsaDistanceFilter(session);

	NotifySessionState(session, RangingSessionState::RangingSuspended);
	return ALIRO_NO_ERROR;
#else
	ARG_UNUSED(session);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif
}

void UltraWideBandImpl::ResetCsaDistanceFilter(SessionContext &session)
{
	session.mCsaDistanceSamples.fill(0);
	session.mCsaDistanceSampleCount = 0;
}

bool UltraWideBandImpl::AddCsaDistanceSample(SessionContext &session, uint16_t distanceCm,
					     uint16_t &filteredDistanceCm)
{
	if (session.mCsaDistanceSampleCount < session.mCsaDistanceSamples.size()) {
		session.mCsaDistanceSamples[session.mCsaDistanceSampleCount++] = distanceCm;
	} else {
		std::rotate(session.mCsaDistanceSamples.begin(), session.mCsaDistanceSamples.begin() + 1,
			    session.mCsaDistanceSamples.end());
		session.mCsaDistanceSamples.back() = distanceCm;
	}

	if (session.mCsaDistanceSampleCount < kCsaDistanceFilterSampleCount) {
		LOG_DBG("SR250 CSA distance filter collecting samples: %u/%u",
			session.mCsaDistanceSampleCount, kCsaDistanceFilterSampleCount);
		return false;
	}

	std::array<uint16_t, kCsaDistanceFilterSampleCount> sortedSamples{};
	std::copy_n(session.mCsaDistanceSamples.begin(), sortedSamples.size(), sortedSamples.begin());
	std::sort(sortedSamples.begin(), sortedSamples.end());

	const uint16_t spread = sortedSamples.back() - sortedSamples.front();
	if (spread > kCsaDistanceFilterMaxSpreadCm) {
		LOG_WRN("SR250 CSA distance unstable: samples=%u,%u,%u spread=%u cm; waiting for stable samples",
			sortedSamples[0], sortedSamples[1], sortedSamples[2], spread);
		session.mCsaDistanceSamples[0] = distanceCm;
		session.mCsaDistanceSampleCount = 1;
		return false;
	}

	filteredDistanceCm = sortedSamples[1];
	LOG_INF("SR250 CSA filtered distance: %u cm (samples=%u,%u,%u)",
		filteredDistanceCm, sortedSamples[0], sortedSamples[1], sortedSamples[2]);
	return true;
}

void UltraWideBandImpl::ReportRangingDistance(SessionContext &session, uint16_t distanceCm)
{
	sys_put_be16(distanceCm, mCurrentDistanceCm.data());
	if (mCallbacks.mRangingData) {
		mCallbacks.mRangingData(
			session.mSessionContextHandle,
			UwbRangingData{ .mData = mCurrentDistanceCm.data(), .mLength = mCurrentDistanceCm.size() });
	}
}

void UltraWideBandImpl::NotifySessionState(SessionContext &session, RangingSessionState state)
{
	session.mState = state;
	if (mCallbacks.mRangingSessionStateChanged) {
		mCallbacks.mRangingSessionStateChanged(session.mSessionContextHandle, state);
	}
}

void UltraWideBandImpl::HandleNxpNotification(uint8_t notificationType, void *data, size_t length)
{
#ifdef CONFIG_NXP_UWB_ZEPHYR_MODULE
	if (data == nullptr) {
		return;
	}

	auto &uwbImpl = UltraWideBandImpl::Instance();
	if (!uwbImpl.mSession || !uwbImpl.mCallbacks.mRangingData) {
		return;
	}
	if (uwbImpl.mSession->mState != RangingSessionState::Ranging &&
	    uwbImpl.mSession->mState != RangingSessionState::RangingResumed) {
		return;
	}

	if (notificationType == UWBD_RANGING_DATA) {
		if (length < 25 || length > UINT16_MAX) {
			LOG_WRN("SR250 ranging notification has invalid length: %u", static_cast<unsigned int>(length));
			return;
		}

		phRangingData_t &rangingData = ParsedRangingDataBuffer();
		parseRangingNtf(static_cast<uint8_t *>(data), static_cast<uint16_t>(length), &rangingData);

		if (rangingData.sessionHandle != uwbImpl.mSession->mNxpSessionHandle) {
			LOG_DBG("Ignoring SR250 ranging notification for session 0x%08x", rangingData.sessionHandle);
			return;
		}

		if (rangingData.ranging_measure_type != MEASUREMENT_TYPE_TWOWAY) {
			LOG_WRN("Ignoring SR250 ranging measurement type 0x%02x", rangingData.ranging_measure_type);
			return;
		}

		if (rangingData.no_of_measurements == 0) {
			LOG_WRN("SR250 ranging notification has no measurements");
			return;
		}

		const uint8_t measurementCount = std::min<uint8_t>(rangingData.no_of_measurements, MAX_NUM_RESPONDERS);
		for (uint8_t i = 0; i < measurementCount; ++i) {
			const auto &measurement = rangingData.ranging_meas.range_meas_twr[i];
			if (measurement.status == kNxpRangingStatusRxTimeout) {
				LOG_WRN("SR250 ranging measurement[%u] RX timeout; no distance yet",
					i);
				continue;
			}
			if (measurement.status == kNxpRangingStatusMacDecodeFailed) {
				LOG_WRN("SR250 ranging measurement[%u] MAC decode failed; secure ranging parameters do not match",
					i);
				continue;
			}
			if (measurement.status != UWBAPI_STATUS_OK) {
				LOG_WRN("SR250 ranging measurement[%u] status 0x%02x; ignoring unavailable distance",
					i, measurement.status);
				continue;
			}
			if (measurement.distance == 0 || measurement.distance == kCccRangingInvalidDistance) {
				LOG_WRN("SR250 ranging measurement[%u] invalid distance %u cm; ignoring", i,
					measurement.distance);
				continue;
			}

			LOG_INF("SR250 ranging distance: %u cm", measurement.distance);
			uwbImpl.ReportRangingDistance(*uwbImpl.mSession, measurement.distance);
			return;
		}

		LOG_WRN("SR250 ranging notification did not contain a usable distance");
		return;
	}

#if UWBFTR_CCC && UWBFTR_CSA
	if (notificationType == UWBD_RANGING_CCC_DATA) {
		if (length < kCsaRangingMinimumPayloadLength) {
			LOG_WRN("SR250 CSA ranging notification too short: %u", static_cast<unsigned int>(length));
			return;
		}

		phCsaRangingData_t &csaData = ParsedCsaRangingDataBuffer();
		parseCsaRangingNtf(static_cast<uint8_t *>(data), static_cast<uint16_t>(length), &csaData);

		if (csaData.sessionHandle != uwbImpl.mSession->mNxpSessionHandle) {
			LOG_DBG("Ignoring SR250 CSA ranging notification for session 0x%08x", csaData.sessionHandle);
			return;
		}

		if (csaData.rangingStatus == kNxpRangingStatusTxFailed) {
			LOG_DBG("SR250 CSA ranging TX failed; waiting for a valid responder measurement");
			return;
		}

		if (csaData.rangingStatus != kCsaRangingStatusSuccess || csaData.distance == 0 ||
		    csaData.distance == kCccRangingInvalidDistance) {
			LOG_DBG("SR250 CSA ranging unavailable: status=0x%02x distance=%u; ignoring unavailable distance",
				csaData.rangingStatus, csaData.distance);
			return;
		}

		LOG_INF("SR250 CSA sample: status=0x%02x distance=%u cm sts=%u round=%u block=%u fom=%u/%u",
			csaData.rangingStatus, csaData.distance, csaData.stsIndex, csaData.rangingRoundIndex,
			csaData.blockIndex, csaData.uncertanityAnchorFom, csaData.uncertanityInitiatorFom);

		uint16_t filteredDistanceCm{};
		if (!uwbImpl.AddCsaDistanceSample(*uwbImpl.mSession, csaData.distance, filteredDistanceCm)) {
			return;
		}

		LOG_INF("SR250 CSA ranging distance: %u cm (raw=%u cm status=0x%02x)", filteredDistanceCm,
			csaData.distance, csaData.rangingStatus);
		uwbImpl.ReportRangingDistance(*uwbImpl.mSession, filteredDistanceCm);
		return;
	}
#endif // UWBFTR_CCC

	LOG_DBG("Ignoring SR250 UWB notification type 0x%02x", notificationType);
#else
	ARG_UNUSED(notificationType);
	ARG_UNUSED(data);
	ARG_UNUSED(length);
#endif
}

UltraWideBandImpl::SessionContext *UltraWideBandImpl::FindSession(SessionContextHandle sessionContextHandle)
{
	if (mSession && mSession->mSessionContextHandle == sessionContextHandle) {
		return mSession.get();
	}

	return nullptr;
}

UltraWideBandImpl::SessionContext *UltraWideBandImpl::FindSession(UwbSessionContext uwbSessionContext)
{
	if (!mSession || mSession->mAliroSessionContext != uwbSessionContext) {
		return nullptr;
	}

	return mSession.get();
}

} // namespace Aliro::Uwb
