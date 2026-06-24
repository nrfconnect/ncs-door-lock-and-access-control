/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <matter_access/access_manager.h>
#include <matter_access/access_storage.h>

#include <platform/CHIPDeviceLayer.h>

#include <zephyr/logging/log.h>

#include <utility>

LOG_MODULE_REGISTER(matter_access, CONFIG_DOOR_LOCK_MATTER_ACCESS_LOG_LEVEL);

using namespace chip;

namespace DoorLock::MatterAccess {
namespace {
/*
 * For each itemIndex in indexList: calls AccessStorage::Remove(itemType, [extraArgs...,] itemIndex)
 * Finally: calls AccessStorage::Remove(indexesType[, extraArgs...])
 */
template <uint16_t N, class... ExtraArgs>
void RemoveCollection(const Data::IndexList<N> &indexList, AccessStorage::Type itemType,
		      AccessStorage::Type indexesType, ExtraArgs &&...extraArgs)
{
	for (size_t i = 0; i < indexList.mList.mLength; i++) {
		uint16_t itemIndex = indexList.mList.mIndexes[i];

		if (!AccessStorage::Instance().Remove(itemType, std::forward<ExtraArgs>(extraArgs)..., itemIndex)) {
			LOG_ERR("Cannot remove item %u with type %u", itemIndex, static_cast<unsigned>(itemType));
		}
	}

	if (indexList.mList.mLength > 0) {
		if (!AccessStorage::Instance().Remove(indexesType, std::forward<ExtraArgs>(extraArgs)...)) {
			LOG_ERR("Cannot remove indexes with type %u", static_cast<unsigned>(indexesType));
		}
	}
}
} /* namespace */

template <Data::CredentialsBits CRED_BIT_MASK>
void AccessManager<CRED_BIT_MASK>::Init(SetOrLoadCredentialCallback setCredentialClbk,
					ClearCredentialCallback clearCredentialClbk,
					ValidateCredentialCallback validateCredentialClbk,
					SetOrLoadCredentialCallback loadCredentialClbk)
{
	InitializeAllCredentials();
	mSetCredentialCallback = setCredentialClbk;
	mClearCredentialCallback = clearCredentialClbk;
	mValidateCredentialCallback = validateCredentialClbk;
	mLoadCredentialCallback = loadCredentialClbk;
	AccessStorage::Instance().Init();
	LoadUsersFromPersistentStorage();
	LoadCredentialsFromPersistentStorage();
#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES
	LoadSchedulesFromPersistentStorage();
#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES
}

template <Data::CredentialsBits CRED_BIT_MASK> void AccessManager<CRED_BIT_MASK>::FactoryReset()
{
	/* Remove users */
	RemoveCollection(mUsersIndexes, AccessStorage::Type::User, AccessStorage::Type::UsersIndexes);

	/* Remove credentials */
	for (auto type = to_underlying(CredentialTypeEnum::kPin);
	     type < to_underlying(CredentialTypeEnum::kUnknownEnumValue); ++type) {
		RemoveCollection(mCredentialsIndexes.Get(static_cast<CredentialTypeEnum>(type)),
				 AccessStorage::Type::Credential, AccessStorage::Type::CredentialsIndexes, type);
	}

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES
	/* Remove schedules */
	for (uint16_t userIndex = 1; userIndex <= CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_USERS; userIndex++) {
		RemoveCollection(mWeekDayScheduleIndexes.Get(userIndex), AccessStorage::Type::WeekDaySchedule,
				 AccessStorage::Type::WeekDayScheduleIndexes, userIndex);
		RemoveCollection(mYearDayScheduleIndexes.Get(userIndex), AccessStorage::Type::YearDaySchedule,
				 AccessStorage::Type::YearDayScheduleIndexes, userIndex);
	}

	RemoveCollection(mHolidayScheduleIndexes, AccessStorage::Type::HolidaySchedule,
			 AccessStorage::Type::HolidayScheduleIndexes);
#endif

	/* Remove other data */
	if (!AccessStorage::Instance().Remove(AccessStorage::Type::RequirePIN)) {
		LOG_ERR("Cannot remove RequirePINforRemoteOperation");
	}

	/* Reinitialize to clear removed users/credentials/schedules */
	InitializeAllCredentials();
}

template <Data::CredentialsBits CRED_BIT_MASK>
bool AccessManager<CRED_BIT_MASK>::ValidateCustom(CredentialTypeEnum type, chip::MutableByteSpan &secret)
{
	/* Run a custom verification within the application layer for RFID credential type */
	if (mValidateCredentialCallback && type == CredentialTypeEnum::kRfid && secret.size() > 0) {
		mValidateCredentialCallback(type, secret);
		return true;
	}
	return false;
}

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_PIN
template <Data::CredentialsBits CRED_BIT_MASK>
bool AccessManager<CRED_BIT_MASK>::ValidatePIN(const Optional<ByteSpan> &pinCode, OperationErrorEnum &err,
					       Nullable<ValidatePINResult> &result)
{
	/* Optionality of the PIN code is validated by the caller, so assume it is OK not to provide the PIN
	 * code. */
	if (!pinCode.HasValue()) {
		return true;
	}

	EmberAfPluginDoorLockCredentialInfo credential;

	/* Check the PIN code */
	for (size_t index = 1; index <= CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_TYPE; ++index) {
		if (CHIP_NO_ERROR != mCredentials.GetCredentials(CredentialTypeEnum::kPin, credential, index)) {
			err = OperationErrorEnum::kInvalidCredential;
			continue;
		}

		if (credential.status == DlCredentialStatus::kAvailable) {
			continue;
		}

		if (!credential.credentialData.data_equal(pinCode.Value())) {
			err = OperationErrorEnum::kInvalidCredential;
			continue;
		}

		Data::User *user{ nullptr };
		if (GetCredentialUser(index, CredentialTypeEnum::kPin, &user) == CHIP_NO_ERROR) {
			result = ValidatePINResult{
				.mUserId = static_cast<uint16_t>(user->mInfo.mFields.mUserUniqueId),
				.mCredential =
					LockOpCredentials{ CredentialTypeEnum::kPin, static_cast<uint16_t>(index) },
			};
		} else {
			result = {};
		}

		LOG_DBG("Valid lock PIN code provided");
		err = OperationErrorEnum::kUnspecified;
		return true;
	}
	LOG_DBG("Invalid lock PIN code provided");
	return false;
}
#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_PIN

template <Data::CredentialsBits CRED_BIT_MASK> void AccessManager<CRED_BIT_MASK>::InitializeUsers()
{
	for (size_t userIndex = 0; userIndex < CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_USERS; ++userIndex) {
		auto &user = mUsers[userIndex];

		user.mName.mSize = 0;
		memset(user.mName.mValue, 0, DOOR_LOCK_USER_NAME_BUFFER_SIZE);
		for (size_t credIdx = 0; credIdx < CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_USER;
		     ++credIdx) {
			user.mOccupiedCredentials.mData[credIdx] = {};
		}
		user.mOccupiedCredentials.mSize = 0;
		memset(user.mInfo.mRaw, 0, sizeof(user.mInfo));
		/* max better than 0 which is the special home user unique ID*/
		user.mInfo.mFields.mUserUniqueId = std::numeric_limits<uint32_t>::max();
	}
}

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES
template <Data::CredentialsBits CRED_BIT_MASK> void AccessManager<CRED_BIT_MASK>::InitializeSchedules()
{
	for (size_t userIndex = 0; userIndex < CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_USERS; ++userIndex) {
		for (size_t weekDayIndex = 0;
		     weekDayIndex < CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES_MAX_WEEKDAY_SCHEDULES_PER_USER;
		     ++weekDayIndex) {
			auto &schedule = mWeekDaySchedule[userIndex][weekDayIndex];
			memset(schedule.mData.mRaw, 0, sizeof(schedule.mData));
		}
		for (size_t yearDayIndex = 0;
		     yearDayIndex < CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES_MAX_YEARDAY_SCHEDULES_PER_USER;
		     ++yearDayIndex) {
			auto &schedule = mYearDaySchedule[userIndex][yearDayIndex];
			memset(schedule.mData.mRaw, 0, sizeof(schedule.mData));
		}
	}
	for (size_t holidayIndex = 0; holidayIndex < CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES_MAX_HOLIDAY_SCHEDULES;
	     ++holidayIndex) {
		auto &schedule = mHolidaySchedule[holidayIndex];
		memset(schedule.mData.mRaw, 0, sizeof(schedule.mData));
	}
}
#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES

template <Data::CredentialsBits CRED_BIT_MASK> void AccessManager<CRED_BIT_MASK>::InitializeAllCredentials()
{
	mCredentials.Initialize();
	InitializeUsers();
#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES
	InitializeSchedules();
#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_SCHEDULES
}

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_PIN
template <Data::CredentialsBits CRED_BIT_MASK> void AccessManager<CRED_BIT_MASK>::SetRequirePIN(bool require)
{
	if (mRequirePINForRemoteOperation != require) {
		mRequirePINForRemoteOperation = require;
		if (!AccessStorage::Instance().Store(AccessStorage::Type::RequirePIN, &mRequirePINForRemoteOperation,
						     sizeof(mRequirePINForRemoteOperation))) {
			LOG_ERR("Cannot store RequirePINforRemoteOperation.");
		}
	}
}
#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_PIN

/* Explicitly instantiate supported template variant to avoid linker errors. */
template class AccessManager<kAccessManagerSupportedCredentialTypesBitMask>;

} // namespace DoorLock::MatterAccess
