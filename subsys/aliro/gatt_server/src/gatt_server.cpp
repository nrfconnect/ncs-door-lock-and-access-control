/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <gatt_server/gatt_server.h>

#include "gatt_attribute_value.h"

#include <aliro/aliro.h>
#include <doorlock/bluetooth/services/aliro.h>
#include <doorlock/utils/utils.h>

#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <algorithm>
#include <array>
#include <cstring>

LOG_MODULE_REGISTER(GattServer, CONFIG_DOOR_LOCK_ALIRO_GATT_SERVER_LOG_LEVEL);

namespace DoorLock::GattServer {

namespace {

ssize_t ReaderSpsmBleUwbProtocolversion(bt_conn *connectionId, const bt_gatt_attr *attribute, void *data,
					uint16_t dataLength, uint16_t offset);
ssize_t UserDeviceBleUwbProtocolversion(bt_conn *connectionId, const bt_gatt_attr *attribute, const void *data,
					uint16_t dataLength, uint16_t offset, uint8_t flags);
void DisconnectedCallback(bt_conn *conn, uint8_t reason);

bt_gatt_attr sAliroServiceAttributes[]{
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ALIRO_SERVICE),
	BT_GATT_CHARACTERISTIC(BT_UUID_ALIRO_READER_SPSM_BLE_UWB_PROTOCOL_VERSION_CHARACTERISTIC, BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, ReaderSpsmBleUwbProtocolversion, nullptr, nullptr),
	BT_GATT_CHARACTERISTIC(BT_UUID_ALIRO_DEVICE_BLE_UWB_PROTOCOL_VERSION_CHARACTERISTIC, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_WRITE, nullptr, UserDeviceBleUwbProtocolversion, nullptr),
};

bt_gatt_service sAliroService = BT_GATT_SERVICE(sAliroServiceAttributes);

GattAttributeValue sGattAttributeValue{};
std::array<Aliro::ProtocolVersion, CONFIG_BT_MAX_CONN> sConnectionProtocolVersion{};

BT_CONN_CB_DEFINE(sConnCallbacks){
	.disconnected = DisconnectedCallback,
};

void SetProtocolVersion(const bt_conn *conn, Aliro::ProtocolVersion protocolVersion)
{
	const auto index = bt_conn_index(conn);
	sConnectionProtocolVersion[index] = protocolVersion;
}

ssize_t ReaderSpsmBleUwbProtocolversion(bt_conn *connectionId, const bt_gatt_attr *attribute, void *data,
					uint16_t dataLength, uint16_t offset)
{
	VerifyOrReturnValue(connectionId && attribute && data, BT_GATT_ERR(BT_ATT_ERR_INVALID_HANDLE),
			    LOG_ERR("Invalid argument"));

	VerifyOrReturnValue(sGattAttributeValue.NotNull(), BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN),
			    LOG_ERR("Invalid GATT attribute buffer"));

	return bt_gatt_attr_read(connectionId, attribute, data, dataLength, offset, sGattAttributeValue.GetBufferData(),
				 sGattAttributeValue.GetBufferSize());
}

ssize_t UserDeviceBleUwbProtocolversion(bt_conn *connectionId, const bt_gatt_attr *attribute, const void *data,
					uint16_t dataLength, uint16_t, uint8_t)
{
	LOG_DBG("%s", __func__);

	constexpr size_t kAttrMinLen{ sizeof(Aliro::ProtocolVersion) + sizeof(uint8_t) };
	constexpr size_t kSupportedFeaturesLengthOffset{ sizeof(Aliro::ProtocolVersion) };
	constexpr size_t kFeaturesSuppOffset{ kSupportedFeaturesLengthOffset +
					      GattAttributeValue::kSupportedFeaturesLength };

	VerifyOrReturnValue(attribute && data, BT_GATT_ERR(BT_ATT_ERR_INVALID_HANDLE), LOG_ERR("Invalid argument"));

	LOG_HEXDUMP_DBG(data, dataLength, "UserDeviceBleUwbProtocolversion");

	VerifyOrReturnValue(dataLength >= kAttrMinLen, BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN),
			    LOG_ERR("Invalid attribute length, expected at least: %zu, got: %zu", kAttrMinLen,
				    dataLength));

	const Aliro::ProtocolVersion version{ sys_get_be16(static_cast<const uint8_t *>(data)) };
	const uint8_t featuresSuppLen = static_cast<const uint8_t *>(data)[kSupportedFeaturesLengthOffset];

	VerifyOrReturnValue(dataLength == kAttrMinLen + featuresSuppLen, BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN),
			    LOG_ERR("Invalid attribute length, expected: %zu, got: %zu", kAttrMinLen + featuresSuppLen,
				    dataLength));

	LOG_INF("Protocol Version: 0x%04x, Supported Features:", version);

	if (featuresSuppLen >= sizeof(SupportedFeatures)) {
		SupportedFeatures supportedFeatures{};
		std::memcpy(&supportedFeatures, static_cast<const uint8_t *>(data) + kFeaturesSuppOffset,
			    sizeof(SupportedFeatures));

		LOG_INF("Time Synchronization Procedure 0: %d", supportedFeatures.TimesyncProcedure0);
		LOG_INF("Time Synchronization Procedure 1: %d", supportedFeatures.TimesyncProcedure1);
		LOG_INF("LE Coded PHY                    : %d", supportedFeatures.LeCodedPhy);
	}

	size_t supportedVersionsCount{};
	const auto *supportedVersions = Aliro::AliroStack::Instance().GetBleUwbProtocolVersions(supportedVersionsCount);

	const bool versionFound =
		std::any_of(supportedVersions, supportedVersions + supportedVersionsCount,
			    [version](Aliro::ProtocolVersion supportedVersion) { return supportedVersion == version; });

	VerifyOrReturnValue(versionFound, dataLength, LOG_DBG("Unsupported protocol version: 0x%04x", version));

	SetProtocolVersion(connectionId, version);

	return dataLength;
}

void DisconnectedCallback(bt_conn *conn, uint8_t)
{
	SetProtocolVersion(conn, Aliro::BleTypes::kInvalidProtocolVersion);
}

} // namespace

int Init(uint16_t spsm, const SupportedFeatures &supportedFeatures)
{
	VerifyOrReturnValue(spsm != 0, -EINVAL, LOG_ERR("Invalid SPSM value"));

	size_t supportedVersionsCount{};
	const auto *supportedVersions = Aliro::AliroStack::Instance().GetBleUwbProtocolVersions(supportedVersionsCount);

	// Generate the GATT attributes buffer once during initialization.
	int error = sGattAttributeValue.Generate(spsm, supportedVersions, supportedVersionsCount, supportedFeatures);
	VerifyOrReturnValue(error == 0, error, LOG_ERR("Failed to generate GATT attribute buffer: %d", error));

	sConnectionProtocolVersion.fill(Aliro::BleTypes::kInvalidProtocolVersion);

	return 0;
}

int Register()
{
	const int error = bt_gatt_service_register(&sAliroService);
	VerifyOrReturnValue(error == 0, error, LOG_ERR("Failed to register GATT service: %d", error));

	return 0;
}

int Unregister()
{
	const int error = bt_gatt_service_unregister(&sAliroService);
	VerifyOrReturnValue(error == 0, error, LOG_ERR("Failed to unregister GATT service: %d", error));

	return 0;
}

Aliro::ProtocolVersion GetBleUwbProtocolVersion(const bt_conn *conn)
{
	const auto index = bt_conn_index(conn);
	return sConnectionProtocolVersion[index];
}

} // namespace DoorLock::GattServer
