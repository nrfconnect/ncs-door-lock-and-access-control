/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <matter_access/access_data_types.h>

namespace DoorLock::MatterAccess::Data {

void pack(void *buff, const void *data, size_t dataSize, size_t &offset)
{
	memcpy(reinterpret_cast<uint8_t *>(buff) + offset, data, dataSize);
	offset += dataSize;
}

void unpack(void *data, size_t dataSize, const void *buff, size_t &offset)
{
	memcpy(data, reinterpret_cast<const uint8_t *>(buff) + offset, dataSize);
	offset += dataSize;
}

CHIP_ERROR Credential::FillFromPlugin(const EmberAfPluginDoorLockCredentialInfo &credentialInfo)
{
	if (credentialInfo.credentialData.size() < kMaxCredentialLength) {
		mInfo.mFields.mStatus = static_cast<uint8_t>(credentialInfo.status);
		mInfo.mFields.mCredentialType = static_cast<uint8_t>(credentialInfo.credentialType);
		mInfo.mFields.mCreationSource = static_cast<uint8_t>(credentialInfo.creationSource);
		mInfo.mFields.mCreatedBy = static_cast<uint8_t>(credentialInfo.createdBy);
		mInfo.mFields.mModificationSource = static_cast<uint8_t>(credentialInfo.modificationSource);
		mInfo.mFields.mLastModifiedBy = static_cast<uint8_t>(credentialInfo.lastModifiedBy);
		mSecret.mDataLength = credentialInfo.credentialData.size();
		memcpy(mSecret.mData, credentialInfo.credentialData.data(), credentialInfo.credentialData.size());
		return CHIP_NO_ERROR;
	}
	return CHIP_ERROR_BUFFER_TOO_SMALL;
}

CHIP_ERROR Credential::ConvertToPlugin(EmberAfPluginDoorLockCredentialInfo &credentialInfo) const
{
	if (mSecret.mDataLength < RequiredBufferSize()) {
		credentialInfo.status = static_cast<DlCredentialStatus>(mInfo.mFields.mStatus);
		credentialInfo.credentialType = static_cast<CredentialTypeEnum>(mInfo.mFields.mCredentialType);
		credentialInfo.credentialData = chip::ByteSpan(mSecret.mData, mSecret.mDataLength);
		credentialInfo.creationSource = static_cast<DlAssetSource>(mInfo.mFields.mCreationSource);
		credentialInfo.createdBy = static_cast<chip::FabricIndex>(mInfo.mFields.mCreatedBy);
		credentialInfo.modificationSource = static_cast<DlAssetSource>(mInfo.mFields.mModificationSource);
		credentialInfo.lastModifiedBy = static_cast<chip::FabricIndex>(mInfo.mFields.mLastModifiedBy);
		return CHIP_NO_ERROR;
	}
	return CHIP_ERROR_BUFFER_TOO_SMALL;
}

size_t Credential::Serialize(void *buff, size_t buffSize)
{
	if (!buff || buffSize < RequiredBufferSize()) {
		return 0;
	}
	size_t offset{ 0 };

	pack(buff, mInfo.mRaw, sizeof(mInfo.mRaw), offset);
	pack(buff, &mSecret.mDataLength, sizeof(mSecret.mDataLength), offset);
	pack(buff, mSecret.mData, mSecret.mDataLength, offset);

	return offset;
}

CHIP_ERROR Credential::Deserialize(const void *buff, size_t buffSize)
{
	if (!buff) {
		return CHIP_ERROR_INVALID_ARGUMENT;
	}

	if (buffSize > RequiredBufferSize()) {
		return CHIP_ERROR_BUFFER_TOO_SMALL;
	}

	size_t offset{ 0 };

	unpack(mInfo.mRaw, sizeof(mInfo.mRaw), buff, offset);
	unpack(&mSecret.mDataLength, sizeof(mSecret.mDataLength), buff, offset);
	if (kMaxCredentialLength < mSecret.mDataLength) {
		/* Read data length cannot be parsed because is too big */
		return CHIP_ERROR_BUFFER_TOO_SMALL;
	}
	unpack(mSecret.mData, mSecret.mDataLength, buff, offset);

	return CHIP_NO_ERROR;
}

CHIP_ERROR User::FillFromPlugin(const EmberAfPluginDoorLockUserInfo &userInfo)
{
	if (DOOR_LOCK_USER_NAME_BUFFER_SIZE > userInfo.userName.size() &&
	    CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_USER >=
		    (userInfo.credentials.size() / sizeof(CredentialStruct))) {
		mName.mSize = userInfo.userName.size();
		memcpy(mName.mValue, userInfo.userName.data(), userInfo.userName.size());
		mOccupiedCredentials.mSize = userInfo.credentials.size() * sizeof(CredentialStruct);
		memcpy(mOccupiedCredentials.mData, userInfo.credentials.data(), mOccupiedCredentials.mSize);
		mInfo.mFields.mUserUniqueId = userInfo.userUniqueId;
		mInfo.mFields.mUserStatus = static_cast<uint8_t>(userInfo.userStatus);
		mInfo.mFields.mUserType = static_cast<uint8_t>(userInfo.userType);
		mInfo.mFields.mCredentialRule = static_cast<uint8_t>(userInfo.credentialRule);
		mInfo.mFields.mCreationSource = static_cast<uint8_t>(userInfo.creationSource);
		mInfo.mFields.mCreatedBy = static_cast<uint8_t>(userInfo.createdBy);
		mInfo.mFields.mModificationSource = static_cast<uint8_t>(userInfo.modificationSource);
		mInfo.mFields.mLastModifiedBy = static_cast<uint8_t>(userInfo.lastModifiedBy);

		return CHIP_NO_ERROR;
	}
	return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR User::ConvertToPlugin(EmberAfPluginDoorLockUserInfo &userInfo) const
{
	if (CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_USER >=
		    (mOccupiedCredentials.mSize / sizeof(CredentialStruct)) &&
	    DOOR_LOCK_USER_NAME_BUFFER_SIZE >= mName.mSize) {
		userInfo.userName = chip::CharSpan(mName.mValue, mName.mSize);
		userInfo.credentials = chip::Span<const CredentialStruct>(
			mOccupiedCredentials.mData, mOccupiedCredentials.mSize / sizeof(CredentialStruct));
		userInfo.userUniqueId = mInfo.mFields.mUserUniqueId;
		userInfo.userStatus = static_cast<UserStatusEnum>(mInfo.mFields.mUserStatus);
		userInfo.userType = static_cast<UserTypeEnum>(mInfo.mFields.mUserType);
		userInfo.credentialRule = static_cast<CredentialRuleEnum>(mInfo.mFields.mCredentialRule);
		userInfo.creationSource = static_cast<DlAssetSource>(mInfo.mFields.mCreationSource);
		userInfo.createdBy = static_cast<chip::FabricIndex>(mInfo.mFields.mCreatedBy);
		userInfo.modificationSource = static_cast<DlAssetSource>(mInfo.mFields.mModificationSource);
		userInfo.lastModifiedBy = static_cast<chip::FabricIndex>(mInfo.mFields.mLastModifiedBy);
		return CHIP_NO_ERROR;
	}
	return CHIP_ERROR_INTERNAL;
}

size_t User::Serialize(void *buff, size_t buffSize)
{
	if (!buff || buffSize < RequiredBufferSize()) {
		return 0;
	}

	size_t offset{ 0 };
	pack(buff, mInfo.mRaw, sizeof(mInfo.mRaw), offset);
	pack(buff, &mOccupiedCredentials.mSize, sizeof(mOccupiedCredentials.mSize), offset);
	pack(buff, mOccupiedCredentials.mData, mOccupiedCredentials.mSize, offset);
	pack(buff, &mName.mSize, sizeof(mName.mSize), offset);
	pack(buff, mName.mValue, mName.mSize, offset);

	return offset;
}

CHIP_ERROR User::Deserialize(const void *buff, size_t buffSize)
{
	if (!buff) {
		return CHIP_ERROR_INVALID_ARGUMENT;
	}

	if (buffSize > RequiredBufferSize()) {
		return CHIP_ERROR_BUFFER_TOO_SMALL;
	}

	size_t offset{ 0 };

	unpack(mInfo.mRaw, sizeof(mInfo.mRaw), buff, offset);
	unpack(&mOccupiedCredentials.mSize, sizeof(mOccupiedCredentials.mSize), buff, offset);
	if ((mOccupiedCredentials.mSize / sizeof(CredentialStruct)) >
	    CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_USER) {
		/* Read data length cannot be parsed because is too big */
		return CHIP_ERROR_BUFFER_TOO_SMALL;
	}
	unpack(mOccupiedCredentials.mData, mOccupiedCredentials.mSize, buff, offset);
	unpack(&mName.mSize, sizeof(mName.mSize), buff, offset);
	if (mName.mSize >= DOOR_LOCK_USER_NAME_BUFFER_SIZE) {
		/* Read data length cannot be parsed because is too big. */
		return CHIP_ERROR_BUFFER_TOO_SMALL;
	}
	unpack(mName.mValue, mName.mSize, buff, offset);

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_ALIRO
	/* Clear all Aliro evictable credentials, they will be updated later */
	RemoveAliroEvictableCredentials();
#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_ALIRO

	return CHIP_NO_ERROR;
}

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_ALIRO

CHIP_ERROR User::AddAliroEvictableCredential(uint16_t credentialIndex)
{
	constexpr auto credentialType{ CredentialTypeEnum::kAliroEvictableEndpointKey };

	const size_t numCredentials = mOccupiedCredentials.mSize / sizeof(CredentialStruct);
	for (size_t i = 0; i < numCredentials; ++i) {
		const auto &credentialStruct = mOccupiedCredentials.mData[i];
		if (credentialStruct.credentialType == credentialType &&
		    credentialStruct.credentialIndex == credentialIndex) {
			/** Credential already exists */
			return CHIP_NO_ERROR;
		}
	}

	const size_t insertIndex = numCredentials;
	VerifyOrReturnError(insertIndex < CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_USER,
			    CHIP_ERROR_NO_MEMORY);

	auto &credentialStruct = mOccupiedCredentials.mData[insertIndex];
	credentialStruct.credentialIndex = credentialIndex;
	credentialStruct.credentialType = credentialType;

	mOccupiedCredentials.mSize += sizeof(CredentialStruct);

	return CHIP_NO_ERROR;
}

void User::RemoveAliroEvictableCredential(uint16_t credentialIndex)
{
	constexpr auto credentialType{ CredentialTypeEnum::kAliroEvictableEndpointKey };

	for (size_t idxCred = 0; idxCred < mOccupiedCredentials.mSize / sizeof(CredentialStruct); ++idxCred) {
		const auto &credentialStruct = mOccupiedCredentials.mData[idxCred];
		if (credentialStruct.credentialType == credentialType &&
		    credentialStruct.credentialIndex == credentialIndex) {
			const size_t credentialsToMove =
				mOccupiedCredentials.mSize / sizeof(CredentialStruct) - idxCred - 1;
			memmove(&mOccupiedCredentials.mData[idxCred], &mOccupiedCredentials.mData[idxCred + 1],
				credentialsToMove * sizeof(CredentialStruct));

			mOccupiedCredentials.mSize -= sizeof(CredentialStruct);

			return;
		}
	}
}

void User::RemoveAliroEvictableCredentials()
{
	size_t writeIdx = 0;
	const size_t numCredentials = mOccupiedCredentials.mSize / sizeof(CredentialStruct);

	for (size_t readIdx = 0; readIdx < numCredentials; ++readIdx) {
		const auto &credential = mOccupiedCredentials.mData[readIdx];

		if (credential.credentialType == CredentialTypeEnum::kAliroEvictableEndpointKey) {
			continue;
		}

		if (writeIdx != readIdx) {
			mOccupiedCredentials.mData[writeIdx] = credential;
		}
		++writeIdx;
	}

	mOccupiedCredentials.mSize = writeIdx * sizeof(CredentialStruct);
}

#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_ALIRO

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES

CHIP_ERROR WeekDaySchedule::FillFromPlugin(const EmberAfPluginDoorLockWeekDaySchedule &plugin)
{
	mData.mFields.mDaysMask = static_cast<uint8_t>(plugin.daysMask);
	mData.mFields.mStartHour = plugin.startHour;
	mData.mFields.mStartMinute = plugin.startMinute;
	mData.mFields.mEndHour = plugin.endHour;
	mData.mFields.mEndMinute = plugin.endMinute;

	return CHIP_NO_ERROR;
}

CHIP_ERROR YearDaySchedule::FillFromPlugin(const EmberAfPluginDoorLockYearDaySchedule &plugin)
{
	mData.mFields.mLocalStartTime = plugin.localStartTime;
	mData.mFields.mLocalEndTime = plugin.localEndTime;

	return CHIP_NO_ERROR;
}

CHIP_ERROR HolidaySchedule::FillFromPlugin(const EmberAfPluginDoorLockHolidaySchedule &plugin)
{
	mData.mFields.mLocalStartTime = plugin.localStartTime;
	mData.mFields.mLocalEndTime = plugin.localEndTime;
	mData.mFields.mOperatingMode = static_cast<uint8_t>(plugin.operatingMode);

	return CHIP_NO_ERROR;
}

CHIP_ERROR WeekDaySchedule::ConvertToPlugin(EmberAfPluginDoorLockWeekDaySchedule &plugin) const
{
	plugin.daysMask = static_cast<DaysMaskMap>(mData.mFields.mDaysMask);
	plugin.startHour = mData.mFields.mStartHour;
	plugin.startMinute = mData.mFields.mStartMinute;
	plugin.endHour = mData.mFields.mEndHour;
	plugin.endMinute = mData.mFields.mEndMinute;

	return CHIP_NO_ERROR;
}

CHIP_ERROR YearDaySchedule::ConvertToPlugin(EmberAfPluginDoorLockYearDaySchedule &plugin) const
{
	plugin.localStartTime = mData.mFields.mLocalStartTime;
	plugin.localEndTime = mData.mFields.mLocalEndTime;

	return CHIP_NO_ERROR;
}

CHIP_ERROR HolidaySchedule::ConvertToPlugin(EmberAfPluginDoorLockHolidaySchedule &plugin) const
{
	plugin.localStartTime = mData.mFields.mLocalStartTime;
	plugin.localEndTime = mData.mFields.mLocalEndTime;
	plugin.operatingMode = static_cast<OperatingModeEnum>(mData.mFields.mOperatingMode);

	return CHIP_NO_ERROR;
}

#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES

} // namespace DoorLock::MatterAccess::Data
