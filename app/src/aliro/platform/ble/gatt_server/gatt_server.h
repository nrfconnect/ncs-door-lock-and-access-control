/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "gatt_attribute_value.h"

#include "aliro/ble_types.h"
#include "aliro/errors.h"

#include <zephyr/bluetooth/gatt.h>

#include <array>
#include <cstddef>
#include <cstdint>

namespace Aliro {

/**
 * @class GattServer
 * @brief Implements the GATT server for the Aliro service.
 */
class GattServer {
public:
	/**
	 * @brief Constructor for GattServer.
	 *
	 * Initializes the GATT server with default values.
	 */
	GattServer() = default;

	/**
	 * @brief Destructor for GattServer.
	 */
	~GattServer();

	// Deleted copy and move constructors and assignment operators.
	GattServer &operator=(const GattServer &) = delete;
	GattServer(const GattServer &) = delete;
	GattServer(GattServer &&) = delete;
	GattServer &operator=(GattServer &&) = delete;

	/**
	 * @brief Initializes the GATT server.
	 * This function registers the GATT service and its characteristics.
	 *
	 * @param spsm The L2CAP SPSM value to be used by the GATT server.
	 *
	 * @return ALIRO_NO_ERROR on success, an error code otherwise.
	 */
	AliroError Init(L2capServer::Spsm spsm);

	/**
	 * @brief Deinitializes the GATT server.
	 * This function unregisters the GATT service and its characteristics.
	 *
	 * @return ALIRO_NO_ERROR on success, an error code otherwise.
	 */
	AliroError Deinit();

	/**
	 * @brief Callback for reading the Reader Declared SPSM BLE UWB Protocol Version characteristic.
	 */
	static ssize_t ReaderDeclaredSpsmBleUwbProtocolversion(bt_conn *connectionId, const bt_gatt_attr *attribute,
							       void *data, uint16_t dataLength, uint16_t offset);

	/**
	 * @brief Callback for writing to the User Device Selected SPSM BLE UWB Protocol Version characteristic.
	 */
	static ssize_t UserDeviceSelectedSpsmBleUwbProtocolversion(bt_conn *connectionId, const bt_gatt_attr *attribute,
								   const void *data, uint16_t dataLength,
								   uint16_t offset, uint8_t flags);

private:
	/**
	 * @brief Defines the index for GATT attributes the `mGattAttributes` GATT attribute array.
	 */
	enum class GattAttributeIndex : size_t {
		kPrimaryService = 0,
		kReaderCharacteristic,
		kReaderCharacteristicValue,
		kUserDeviceSelectedCharacteristic,
		kUserDeviceSelectedCharacteristicValue,
		kGattAttributesCount
	};

	/**
	 * @brief Initializes a GATT attribute in the `mGattAttributes` array.
	 *
	 * @param index The index of the attribute to initialize.
	 * @param attr The attribute to initialize.
	 */
	void InitializeAttribute(GattAttributeIndex index, const bt_gatt_attr &attr);

	static constexpr size_t kGattAttributesSize{ static_cast<size_t>(GattAttributeIndex::kGattAttributesCount) };

	std::array<bt_gatt_attr, kGattAttributesSize> mGattAttributes{};
	bt_gatt_service mGattService{};
	GattAttributeValue mGattAttributeValue{};

	bt_gatt_chrc kReaderCharacteristicAttributeValue =
		BT_GATT_CHRC_INIT(&kReaderCharacteristic.uuid, 0U, BT_GATT_CHRC_READ);
	bt_gatt_chrc kUserDeviceSelectedCharacteristicAttributeValue =
		BT_GATT_CHRC_INIT(&kUserDeviceSelectedCharacteristic.uuid, 0U, BT_GATT_CHRC_WRITE);

	static constexpr bt_uuid_16 kServiceUuid = BT_UUID_INIT_16(BleTypes::kAliroServiceUuid);

	static constexpr bt_uuid_128 kReaderCharacteristic =
		BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xD3B5A130, 0x9E23, 0x4B3A, 0x8BE4, 0x6B1EE5F980A3));
	static constexpr bt_uuid_128 kUserDeviceSelectedCharacteristic =
		BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xBD4B9502, 0x3F54, 0x11EC, 0xB919, 0x0242AC120005));
};

} // namespace Aliro
