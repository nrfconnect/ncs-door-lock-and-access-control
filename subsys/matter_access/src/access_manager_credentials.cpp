/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <matter_access/access_manager.h>
#include <matter_access/access_storage.h>

#include <platform/CHIPDeviceLayer.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(matter_access, CONFIG_DOOR_LOCK_MATTER_ACCESS_LOG_LEVEL);

using namespace chip;

namespace DoorLock::MatterAccess {

template <Data::CredentialsBits CRED_BIT_MASK>
bool AccessManager<CRED_BIT_MASK>::GetCredential(uint16_t credentialIndex, CredentialTypeEnum credentialType,
						 EmberAfPluginDoorLockCredentialInfo &credential)
{
	VerifyOrReturnError(credentialIndex > 0 &&
				    credentialIndex <= CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_TYPE,
			    false);
	VerifyOrReturnError(CHIP_NO_ERROR == mCredentials.GetCredentials(credentialType, credential, credentialIndex),
			    false);

	return true;
}

template <Data::CredentialsBits CRED_BIT_MASK>
bool AccessManager<CRED_BIT_MASK>::SetCredential(uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
						 DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
						 const ByteSpan &secret)
{
	/* Programming PIN is a special case - it is unique and its index assumed to be 0, currently we do not
	 * support it */
	VerifyOrReturnError(credentialType != CredentialTypeEnum::kProgrammingPIN, false);
	VerifyOrReturnError(credentialIndex > 0 &&
				    credentialIndex <= CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_TYPE,
			    false);
	VerifyOrReturnError(secret.size() <= Data::kMaxCredentialLength, false);

	bool success{ false };
	auto &credentials = mCredentials.GetCredentialsTypes(credentialType, success);
	VerifyOrReturnError(success, false);
	auto &credential = credentials[credentialIndex - 1];
	return DoSetCredential(credential, credentialIndex, creator, modifier, credentialStatus, credentialType,
			       secret);
}

template <Data::CredentialsBits CRED_BIT_MASK>
bool AccessManager<CRED_BIT_MASK>::DoSetCredential(Data::Credential &credential, uint16_t credentialIndex,
						   FabricIndex creator, FabricIndex modifier,
						   DlCredentialStatus credentialStatus,
						   CredentialTypeEnum credentialType, const ByteSpan &secret)
{
	uint32_t uniqueUserId;
	credential.mInfo.mFields.mStatus = static_cast<uint8_t>(credentialStatus);
	credential.mInfo.mFields.mCredentialType = static_cast<uint8_t>(credentialType);
	credential.mInfo.mFields.mCreationSource = static_cast<uint8_t>(DlAssetSource::kMatterIM);
	credential.mInfo.mFields.mCreatedBy = static_cast<uint8_t>(creator);
	credential.mInfo.mFields.mModificationSource = static_cast<uint8_t>(DlAssetSource::kMatterIM);
	credential.mInfo.mFields.mLastModifiedBy = static_cast<uint8_t>(modifier);

	if (!secret.empty()) {
		credential.mSecret.mDataLength = secret.size();
		memcpy(credential.mSecret.mData, secret.data(), secret.size());
	} else if (CHIP_NO_ERROR == Instance().GetCredentialUserId(credentialIndex, credentialType, uniqueUserId)) {
		/* Credential already exists, and the new secret is empty, so the credential is being cleared.
		 * Inform the higher layer about clearing the credential and provide current credential data.
		 */
		if (Instance().mClearCredentialCallback && DlCredentialStatus::kAvailable == credentialStatus) {
			ByteSpan existingSecretToClear = { credential.mSecret.mData, credential.mSecret.mDataLength };
			Instance().mClearCredentialCallback(credentialIndex, credentialType, existingSecretToClear);
		}
		memset(credential.mSecret.mData, 0, credential.mSecret.mDataLength);
		credential.mSecret.mDataLength = 0;
	}

	uint8_t credentialSerialized[Data::Credential::RequiredBufferSize()] = { 0 };
	size_t serializedSize = credential.Serialize(credentialSerialized, sizeof(credentialSerialized));

	uint8_t credentialIndexesSerialized[CredentialsIndexes::CredentialList::RequiredBufferSize()] = { 0 };
	size_t credIndexesSerializedSize{ 0 };

	/* Process the credential indexes only if the credential itself was properly serialized. */
	if (0 < serializedSize) {
		auto &credIndexes = Instance().mCredentialsIndexes.Get(credentialType);

		if (CHIP_ERROR_BUFFER_TOO_SMALL == credIndexes.AddIndex(credentialIndex)) {
			return false;
		}

		credIndexesSerializedSize =
			credIndexes.Serialize(credentialIndexesSerialized, sizeof(credentialIndexesSerialized));
	}

	/* Store to persistent storage */
	if (0 < serializedSize && 0 < credIndexesSerializedSize) {
		if (!AccessStorage::Instance().Store(AccessStorage::Type::Credential, credentialSerialized,
						     serializedSize, chip::to_underlying(credentialType),
						     credentialIndex)) {
			LOG_ERR("Cannot store given credentials Type: %d Idx: %d",
				static_cast<uint16_t>(credentialType), credentialIndex);
		} else {
			if (!AccessStorage::Instance().Store(AccessStorage::Type::CredentialsIndexes,
							     credentialIndexesSerialized, credIndexesSerializedSize,
							     chip::to_underlying(credentialType))) {
				LOG_ERR("Cannot store credentials counter. The persistent database will be corrupted.");
			}
		}
	} else {
		LOG_ERR("Cannot serialize given credentials Type: %d Idx: %d", static_cast<uint16_t>(credentialType),
			credentialIndex);
	}

	LOG_INF("Setting lock credential %u: %s", static_cast<unsigned>(credentialIndex),
		credential.mInfo.mFields.mStatus == static_cast<uint8_t>(DlCredentialStatus::kAvailable) ? "available" :
													   "occupied");

	if (CHIP_NO_ERROR == Instance().GetCredentialUserId(credentialIndex, credentialType, uniqueUserId)) {
		if (Instance().mSetCredentialCallback && credential.mSecret.mDataLength > 0) {
			ByteSpan secret = { credential.mSecret.mData, credential.mSecret.mDataLength };
			Instance().mSetCredentialCallback(credentialIndex, credentialType, secret);
		}
	}

	return true;
}

template <Data::CredentialsBits CRED_BIT_MASK>
CHIP_ERROR AccessManager<CRED_BIT_MASK>::GetCredentialUserId(uint16_t credentialIndex,
							     CredentialTypeEnum credentialType, uint32_t &userId)
{
	for (size_t idxUsr = 0; idxUsr < CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_USERS; ++idxUsr) {
		auto &user = Instance().mUsers[idxUsr];
		for (size_t idxCred = 0; idxCred < CONFIG_DOOR_LOCK_MATTER_ACCESS_MAX_NUM_CREDENTIALS_PER_USER;
		     ++idxCred) {
			auto &credentialStruct = user.mOccupiedCredentials.mData[idxCred];
			if (credentialStruct.credentialIndex == credentialIndex &&
			    credentialStruct.credentialType == credentialType) {
				userId = user.mInfo.mFields.mUserUniqueId;
				return CHIP_NO_ERROR;
			}
		}
	}
	return CHIP_ERROR_NOT_FOUND;
}

template <Data::CredentialsBits CRED_BIT_MASK> void AccessManager<CRED_BIT_MASK>::LoadCredentialsFromPersistentStorage()
{
	uint8_t credentialData[Data::Credential::RequiredBufferSize()] = { 0 };
	uint8_t credentialIndexesSerialized[CredentialsIndexes::CredentialList::RequiredBufferSize()] = { 0 };
	size_t outSize{ 0 };

	for (uint8_t type = chip::to_underlying(CredentialTypeEnum::kPin);
	     type < chip::to_underlying(CredentialTypeEnum::kUnknownEnumValue); ++type) {
		bool success{ false };
		auto &credentials = mCredentials.GetCredentialsTypes(static_cast<CredentialTypeEnum>(type), success);

		if (!success) {
			continue;
		}
		outSize = 0;
		if (!AccessStorage::Instance().Load(AccessStorage::Type::CredentialsIndexes,
						    credentialIndexesSerialized, sizeof(credentialIndexesSerialized),
						    outSize, type)) {
			LOG_INF("No stored indexes for credential of type: %d", type);
			continue;
		}

		auto &credIndexes = mCredentialsIndexes.Get(static_cast<CredentialTypeEnum>(type));
		if (CHIP_NO_ERROR != credIndexes.Deserialize(credentialIndexesSerialized, outSize)) {
			LOG_ERR("Cannot deserialize indexes for credentials of type: %d", type);
			continue;
		}

		for (size_t idx = 0; idx < credIndexes.mList.mLength; idx++) {
			/* Read the actual index from the indexList */
			uint16_t credentialIndex = credIndexes.mList.mIndexes[idx];
			outSize = 0;
			if (!AccessStorage::Instance().Load(AccessStorage::Type::Credential, credentialData,
							    sizeof(credentialData), outSize, type, credentialIndex)) {
				LOG_ERR("Cannot load credentials of type %d for index: %d", static_cast<uint8_t>(type),
					credentialIndex);
			}

			auto &credential = credentials[credentialIndex - 1];

			if (CHIP_NO_ERROR != credential.Deserialize(credentialData, outSize)) {
				LOG_ERR("Cannot deserialize credentials of type %d for index: %d",
					static_cast<uint8_t>(type), credentialIndex);
			}

			if (Instance().mLoadCredentialCallback) {
				ByteSpan secret = { credential.mSecret.mData, credential.mSecret.mDataLength };
				Instance().mLoadCredentialCallback(credentialIndex,
								   static_cast<CredentialTypeEnum>(type), secret);
			}
		}
	}

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_PIN
	outSize = 0;
	if (!AccessStorage::Instance().Load(AccessStorage::Type::RequirePIN, &mRequirePINForRemoteOperation,
					    sizeof(mRequirePINForRemoteOperation), outSize) ||
	    outSize != sizeof(mRequirePINForRemoteOperation)) {
		LOG_DBG("Cannot load RequirePINforRemoteOperation");
	}
#endif // CONFIG_DOOR_LOCK_MATTER_ACCESS_CREDENTIAL_TYPES_PIN
}

#ifdef CONFIG_DOOR_LOCK_MATTER_ACCESS_DEBUG

template <Data::CredentialsBits CRED_BIT_MASK>
const char *AccessManager<CRED_BIT_MASK>::GetCredentialName(CredentialTypeEnum type)
{
	switch (type) {
	case CredentialTypeEnum::kPin:
		return "PIN";
	case CredentialTypeEnum::kRfid:
		return "RFID";
	case CredentialTypeEnum::kFingerprint:
		return "Fingerprint";
	case CredentialTypeEnum::kFingerVein:
		return "Finger vein";
	case CredentialTypeEnum::kFace:
		return "Face";
	default:
		return "None";
	}
}

template <Data::CredentialsBits CRED_BIT_MASK>
void AccessManager<CRED_BIT_MASK>::PrintCredential(CredentialTypeEnum type, uint16_t index)
{
	bool success{ false };
	auto &credentialList = mCredentials.GetCredentialsTypes(type, success);

	VerifyOrReturn(success,
		       LOG_INF("%s credential type is not supported. Nothing to print.", GetCredentialName(type)));

	/* User is going to provide an index which is started from 1 instead of 0 */
	auto arrayIdx = index - 1;

	LOG_INF("\t -- Index: %d", index);
	LOG_INF("\t -- Type: %s", GetCredentialName(type));
	LOG_INF("\t\t -- Status: %s",
		static_cast<uint8_t>(DlCredentialStatus::kAvailable) == credentialList[arrayIdx].mInfo.mFields.mStatus ?
			"available" :
			"occupied");
	LOG_INF("\t\t -- Creation source: %d", credentialList[arrayIdx].mInfo.mFields.mCreationSource);
	LOG_INF("\t\t -- Created by: %d", credentialList[arrayIdx].mInfo.mFields.mCreatedBy);
	LOG_INF("\t\t -- Modification source: %d", credentialList[arrayIdx].mInfo.mFields.mModificationSource);
	LOG_INF("\t\t -- Modified by: %d", credentialList[arrayIdx].mInfo.mFields.mLastModifiedBy);
	LOG_HEXDUMP_INF(credentialList[arrayIdx].mSecret.mData, credentialList[arrayIdx].mSecret.mDataLength,
			"\t\t -- Credential data:");
}

#endif

/* Explicitly instantiate supported template variant to avoid linker errors. */
template class AccessManager<kAccessManagerSupportedCredentialTypesBitMask>;

} // namespace DoorLock::MatterAccess
