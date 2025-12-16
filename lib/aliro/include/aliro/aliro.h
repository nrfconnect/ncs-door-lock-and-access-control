/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include "access_manager/access_manager.h"
#include "aliro/errors.h"
#include "aliro/protocol_version.h"
#include "aliro/types.h"
#include "kpersistent_manager/kpersistent_manager.h"
#include "transport/ble/ble_iface.h"

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "aliro/ble_types.h"
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#include <cstddef>

namespace Aliro {

struct AliroConfig {
	/**
	 * @brief Enable NFC transport.
	 */
	bool mEnableNfc{ true };

	/**
	 * @brief The Kpersistent manager needed for the Expedited-fast phase.
	 */
	[[maybe_unused]] KpersistentManager *mKpersistentManager{};

	/**
	 * @brief The BLE interface.
	 */
	BleInterface::BleIfc *mBle{};
};

/**
 * @brief Aliro stack.
 */
class AliroStack {
public:
	/**
	 * @brief Aliro stack callbacks.
	 */
	struct Callbacks {
		/**
		 * @brief Callback for errors.
		 *
		 * This callback is called when an error occurs.
		 *
		 * @param error The error that occurred.
		 */
		void (*mOnError)(AliroError error){ nullptr };
	};

	/**
	 * @brief Gets the instance of the Aliro stack.
	 *
	 * @return The instance of the Aliro stack.
	 */
	static AliroStack &Instance()
	{
		static AliroStack sInstance;
		return sInstance;
	}

	/**
	 * @brief Initializes the Aliro stack.
	 *
	 * @param callbacks The Access callbacks.
	 * @param config The Aliro configuration.
	 *
	 * @return ALIRO_NO_ERROR if the stack was initialized successfully, an error code otherwise.
	 */
	AliroError Init(const Callbacks &callbacks, const AliroConfig &config);

	/**
	 * @brief Provision the Reader.
	 *
	 * @param privateKeyId The private key ID.
	 * @param groupResolvingKeyId The group resolving key ID.
	 * @param identifier The reader identifier.
	 * @param credentialIssuerCAPublicKeyId The credential issuer CA public key ID.
	 *
	 * @return ALIRO_NO_ERROR if the Reader was provisioned successfully, an error code otherwise.
	 */
	AliroError Provision(CryptoTypes::KeyId privateKeyId, CryptoTypes::KeyId groupResolvingKeyId,
			     const Identifier &identifier, CryptoTypes::KeyId credentialIssuerCAPublicKeyId);

	/**
	 * @brief Sets the reader identifier.
	 *
	 * @param identifier The reader identifier.
	 */
	AliroError SetReaderIdentifier(const Identifier &identifier);

	/**
	 * @brief Sets the credential issuer CA public key ID.
	 *
	 * @param credentialIssuerCAPublicKeyId The credential issuer CA public key ID.
	 */
	void SetCredentialIssuerCAPublicKeyId(CryptoTypes::KeyId credentialIssuerCAPublicKeyId) const;

	/**
	 * @brief Starts the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR if the stack was started successfully, an error code otherwise.
	 */
	AliroError Start() const;

	/**
	 * @brief Stops the Aliro stack.
	 *
	 * @return ALIRO_NO_ERROR if the stack was stopped successfully, an error code otherwise.
	 */
	AliroError Stop() const;

	/**
	 * @brief Gets the Aliro configuration.
	 *
	 * @return The Aliro configuration.
	 */
	const AliroConfig &GetConfig() const { return mConfig; }

	/**
	 * @brief Gets the Aliro library version string.
	 *
	 * @return The Aliro library version.
	 */
	static const char *GetLibraryVersion();

	/**
	 * @brief Gets the Expedited Standard protocol version.

	 * @param versionCount The number of protocol versions.
	 *
	 * @return Pointer to the static Expedited Standard protocol version list.
	 */
	const ProtocolVersion *GetExpeditedStandardProtocolVersions(size_t &versionCount) const;

#ifdef CONFIG_DOOR_LOCK_BLE_UWB

	/**
	 * @brief Sends the Reader Status Changed Message ID.
	 *
	 * @param operationSource The operation source that caused the state change.
	 * @param readerState The current reader state.
	 * @param sessionContext Optionally the session context for specific User Device.
	 *
	 * @return ALIRO_NO_ERROR if the Reader Status Changed Message ID is sent successfully, an error code otherwise.
	 */
	AliroError SendReaderStatusChangedMessage(
		OperationSource operationSource, ReaderStateByte readerState,
		std::optional<AccessManager::SessionContext> sessionContext = std::nullopt) const;

	/**
	 * @brief Sets the BLE notification which will be advertised.
	 *
	 * @param notification The BLE notification.
	 *
	 * @return ALIRO_NO_ERROR if the notification was set successfully, an error code otherwise.
	 */
	AliroError SetBleNotification(BleTypes::AdvertisingServiceData::Notification notification) const;

	/**
	 * @brief Gets the BLE advertising version.
	 *
	 * @return The BLE advertising version.
	 */
	uint8_t GetBleAdvertisingVersion() const;

	/**
	 * @brief Gets the BLE/UWB protocol version.

	 * @param versionCount The number of protocol versions.
	 *
	 * @return Pointer to the static BLE/UWB protocol version list.
	 */
	const ProtocolVersion *GetBleUwbProtocolVersions(size_t &versionCount) const;

#endif // CONFIG_DOOR_LOCK_BLE_UWB

private:
	Callbacks mCallbacks{};
	AliroConfig mConfig;
	bool mProvisioned{ false };
};

} // namespace Aliro
