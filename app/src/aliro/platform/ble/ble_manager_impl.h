/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/ble_types.h"
#include "aliro/transport_callbacks.h"

#ifdef CONFIG_ALIRO_BLE_UWB
#include "gatt_server/gatt_server.h"
#endif // CONFIG_ALIRO_BLE_UWB

#include "transport/ble/ble_iface.h"

#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>

#include <array>

namespace Aliro::BleInterface {

class BleManagerImpl : public BleIfc {
public:
	static BleManagerImpl &Instance()
	{
		static BleManagerImpl sInstance;
		return sInstance;
	}

	AliroError Init(const PlatformTransportCallbacks &callbacks) override;

	AliroError Send(ConnectionHandle handle, Data data) const override;

	AliroError Disconnect(ConnectionHandle handle) override;

	void DisconnectAll() override;

	AliroError StartAdvertising(const ConstData &data, BleTypes::AdvertisingDataFieldType type) override;
	AliroError UpdateAdvertisingData(const ConstData &data, BleTypes::AdvertisingDataFieldType type) override;
	AliroError StopAdvertising() override;

	AliroError GetTxPowerLevel(BleTypes::TxPowerLevel &txPowerLevel) const override;
	AliroError GetAddress(BleTypes::BleAddress &address) const override;

	size_t GetMaxSessions() const override;

private:
	enum class BleManagerState : uint8_t { Uninitialized, Initialized, Advertising };

	BleManagerImpl() = default;
	~BleManagerImpl() final = default;
	BleManagerImpl(const BleManagerImpl &) = delete;
	BleManagerImpl &operator=(const BleManagerImpl &) = delete;
	BleManagerImpl(BleManagerImpl &&) = delete;
	BleManagerImpl &operator=(BleManagerImpl &&) = delete;

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

#ifdef CONFIG_ALIRO_BLE_UWB
	GattServer mGattServer{};
#endif // CONFIG_ALIRO_BLE_UWB

	using AdvertisingData = std::array<bt_data, kAdvertisingDataSize>;
	using ScanResponseData = std::array<bt_data, kScanResponseSize>;
	AdvertisingData mAdvertisingData{};
	ScanResponseData mScanResponseData{};

	size_t mConnectionCount{};

	PlatformTransportCallbacks mTransportCallbacks{};

	BleManagerState mState{ BleManagerState::Uninitialized };
	bool IsInitialized() const { return mState >= BleManagerState::Initialized; }
	bool IsAdvertising() const { return mState == BleManagerState::Advertising; }

	void SetState(BleManagerState state) { mState = state; }
};

} // namespace Aliro::BleInterface
