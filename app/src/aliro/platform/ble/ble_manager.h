/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/ble_types.h"

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
#include "aliro/interface.h"
#include "gatt_server/gatt_server.h"
#endif // CONFIG_DOOR_LOCK_BLE_UWB

#include "aliro/errors.h"
#include "aliro/protocol_version.h"
#include "aliro/transport_callbacks.h"
#include "aliro/types.h"

#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>

#include <array>

namespace Aliro {

/**
 * @brief BLE Manager.
 *
 * This class manages the BLE stack and provides BLE control functionality.
 */
class BleManager final {
#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	// Allow Interface::Ble functions to access private methods
	friend AliroError Interface::Ble::Send(ConnectionHandle handle, Data data);
	friend size_t Interface::Ble::GetMaxSessions();
	friend ProtocolVersion Interface::Ble::GetProtocolVersion(ConnectionHandle handle);
	friend AliroError Interface::Ble::Terminate(ConnectionHandle handle);
#endif // CONFIG_DOOR_LOCK_BLE_UWB

public:
	/**
	 * @brief Get the instance of the BLE manager.
	 *
	 * @return The instance of the BLE manager.
	 */
	static BleManager &Instance()
	{
		static BleManager sInstance;
		return sInstance;
	}

	/**
	 * @brief Initialize the BLE manager and Bluetooth stack.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError Init();

	/**
	 * @brief Start BLE advertising with Aliro service data.
	 *
	 * @param data The Aliro advertising service data.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError StartAdvertising(const BleTypes::AdvertisingServiceData &data);

	/**
	 * @brief Start BLE advertising with custom service data.
	 *
	 * @param data The raw service data (e.g., UUID).
	 * @param type The advertising data field type.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError StartAdvertising(const ConstData &data, BleTypes::AdvertisingDataFieldType type);

	/**
	 * @brief Stop BLE advertising.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError StopAdvertising();

	/**
	 * @brief Disconnect all active BLE connections.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError DisconnectAll();

	/**
	 * @brief Update BLE advertising data with Aliro service data while advertising is active.
	 *
	 * @param data The updated Aliro advertising service data.
	 *
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError UpdateAdvertisingData(const BleTypes::AdvertisingServiceData &data);

	/**
	 * @brief Get the current BLE address.
	 */
	AliroError GetAddress(BleTypes::BleAddress &address) const;

	/**
	 * @brief Get the current TX power level.
	 */
	AliroError GetTxPowerLevel(BleTypes::TxPowerLevel &txPowerLevel) const;

private:
	enum class BleManagerState : uint8_t { Uninitialized, Initialized, Advertising };

	BleManager() = default;
	~BleManager() = default;
	BleManager(const BleManager &) = delete;
	BleManager &operator=(const BleManager &) = delete;
	BleManager(BleManager &&) = delete;
	BleManager &operator=(BleManager &&) = delete;

	// Interface::Ble implementation methods (accessed via friend functions)
	/**
	 * @brief Send data over an established BLE connection.
	 */
	AliroError Send(ConnectionHandle handle, Data data) const;
	/**
	 * @brief Get the maximum number of concurrent BLE sessions.
	 */
	size_t GetMaxSessions() const;
	/**
	 * @brief Get the protocol version for a connection.
	 */
	ProtocolVersion GetProtocolVersion(ConnectionHandle handle) const;
	/**
	 * @brief Terminate a BLE connection (called by stack to disconnect).
	 */
	AliroError Terminate(ConnectionHandle handle);

	// Bluetooth connection callbacks
	void Connected(bt_conn *connId, uint8_t error);
	void Disconnected(bt_conn *connId, uint8_t reason);
	void Recycled();
#ifdef CONFIG_BT_SMP
	void SecurityChanged(bt_conn *connId, bt_security_t level, enum bt_security_err error);
#endif // CONFIG_BT_SMP

	int CreateRandomStaticAddress();
	[[maybe_unused]] AliroError GetRandomStaticAddress();
	void ResumeAdvertising();
	void ResumeAdvertisingHandler();
	AliroError StartAdvertising();
	AliroError SetAdvertisingData(const ConstData &data, BleTypes::AdvertisingDataFieldType type);
	AliroError UpdateAdvertisingData(const ConstData &data, BleTypes::AdvertisingDataFieldType type);

	// currently not used
	[[maybe_unused]] bt_ready_cb_t mReadyCb{ nullptr };

	bt_conn_cb mConnCallbacks{};
	bt_addr_le_t mAddress{};
	[[maybe_unused]] k_work mAdvResumeWork{};

	static constexpr size_t kAdvertisingDataSize{ 2 };
	static constexpr size_t kScanResponseSize{ 1 };
	static constexpr uint32_t kAdvertisingPriority{ 1 };
	static constexpr uint8_t kAdvertisingFlags{ BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };
	static constexpr uint32_t kIntervalMin{ BT_GAP_ADV_FAST_INT_MIN_2 };
	static constexpr uint32_t kIntervalMax{ BT_GAP_ADV_FAST_INT_MAX_2 };
	static constexpr uint32_t kAdvertisingOptions{ BT_LE_ADV_OPT_CONN };
	static constexpr size_t kAdvertisingFlagsIndex{ 0 };
	static constexpr size_t kAdvertisingServiceDataIndex{ 1 };
	static constexpr size_t kScanResponseIndex{ 0 };

	// Legacy BLE ADV packets are limited to 31 bytes, 2 bytes are reserved flags (1 byte for flags, 1 byte for
	// service data length). There are 29 bytes left for advertising data.
	static constexpr size_t kMaxAdvertisingDataSize{ 29 };
	std::array<uint8_t, kMaxAdvertisingDataSize> mAdvertisingServiceData{};
	uint8_t mAdvertisingServiceDataSize{};
	BleTypes::AdvertisingDataFieldType mAdvertisingDataFieldType{ BleTypes::AdvertisingDataFieldType::Uuid16 };

#ifdef CONFIG_DOOR_LOCK_BLE_UWB
	GattServer mGattServer{};
#endif // CONFIG_DOOR_LOCK_BLE_UWB

	using AdvertisingData = std::array<bt_data, kAdvertisingDataSize>;
	using ScanResponseData = std::array<bt_data, kScanResponseSize>;
	AdvertisingData mAdvertisingData{};
	ScanResponseData mScanResponseData{};

	size_t mConnectionCount{};

	BleManagerState mState{ BleManagerState::Uninitialized };
	bool IsInitialized() const { return mState >= BleManagerState::Initialized; }
	bool IsAdvertising() const { return mState == BleManagerState::Advertising; }

	void SetState(BleManagerState state) { mState = state; }
};

} // namespace Aliro
