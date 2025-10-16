/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "gatt_server.h"

#include "aliro/aliro.h"
#include "aliro/ble_types.h"
#include "aliro/utils.h"

#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>

#include <algorithm>
#include <cstring>

LOG_MODULE_REGISTER(GattServer, CONFIG_NCS_ALIRO_BLE_LOG_LEVEL);

namespace Aliro {

GattServer::~GattServer()
{
	Deinit();
}

ssize_t GattServer::ReaderDeclaredSpsmBleUwbProtocolversion(bt_conn *connectionId, const bt_gatt_attr *attribute,
							    void *data, uint16_t dataLength, uint16_t offset)
{
	VerifyOrReturnValue(connectionId && attribute && data, BT_GATT_ERR(BT_ATT_ERR_INVALID_HANDLE),
			    LOG_ERR("Invalid argument"));

	const auto *gattServer = static_cast<GattServer *>(attribute->user_data);

	VerifyOrReturnValue(gattServer->mGattAttributeValue.NotNull(), BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN),
			    LOG_ERR("Invalid GATT attribute buffer"));

	return bt_gatt_attr_read(connectionId, attribute, data, dataLength, offset,
				 gattServer->mGattAttributeValue.GetBufferData(),
				 gattServer->mGattAttributeValue.GetBufferSize());
}

ssize_t GattServer::UserDeviceSelectedSpsmBleUwbProtocolversion([[maybe_unused]] bt_conn *,
								const bt_gatt_attr *attribute, const void *data,
								uint16_t dataLength, [[maybe_unused]] uint16_t,
								[[maybe_unused]] uint8_t)
{
	LOG_DBG("%s", __func__);

	constexpr size_t kAttrMinLen{ sizeof(ProtocolVersion) + sizeof(uint8_t) };
	constexpr size_t kSupportedFeaturesLengthOffset{ sizeof(ProtocolVersion) };
	constexpr size_t kFeaturesSuppOffset{ kSupportedFeaturesLengthOffset +
					      GattAttributeValue::kSupportedFeaturesLength };

	VerifyOrReturnValue(attribute && data, BT_GATT_ERR(BT_ATT_ERR_INVALID_HANDLE), LOG_ERR("Invalid argument"));

	LOG_HEXDUMP_DBG(data, dataLength, "UserDeviceSelectedSpsmBleUwbProtocolversion");

	VerifyOrReturnValue(dataLength >= kAttrMinLen, BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN),
			    LOG_HEXDUMP_ERR(data, dataLength, "Invalid UserDeviceSelectedSpsmBleUwbProtocolversion"));

	const ProtocolVersion version{ sys_get_be16(static_cast<const uint8_t *>(data)) };
	const uint8_t featuresSuppLen = static_cast<const uint8_t *>(data)[kSupportedFeaturesLengthOffset];

	VerifyOrReturnValue(dataLength == kAttrMinLen + featuresSuppLen, BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN),
			    LOG_HEXDUMP_ERR(data, dataLength, "Invalid UserDeviceSelectedSpsmBleUwbProtocolversion"));

	LOG_INF("Protocol Version: 0x%04x, Supported Features:", version);

	if (featuresSuppLen >= sizeof(GattAttributeValue::SupportedFeatures)) {
		GattAttributeValue::SupportedFeatures supportedFeatures{};
		std::memcpy(&supportedFeatures, static_cast<const uint8_t *>(data) + kFeaturesSuppOffset,
			    sizeof(GattAttributeValue::SupportedFeatures));

		LOG_INF("Time Synchronization Procedure 0: %d", supportedFeatures.TimesyncProcedure0);
		LOG_INF("Time Synchronization Procedure 1: %d", supportedFeatures.TimesyncProcedure1);
		LOG_INF("LE Coded PHY                    : %d", supportedFeatures.LeCodedPhy);
	}

	size_t supportedVersionsCount{};
	const auto *supportedVersions = AliroStack::Instance().GetBleUwbProtocolVersions(supportedVersionsCount);

	const bool versionFound =
		std::any_of(supportedVersions, supportedVersions + supportedVersionsCount,
			    [version](ProtocolVersion supportedVersion) { return supportedVersion == version; });

	VerifyOrReturnValue(versionFound, BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED),
			    LOG_ERR("Unsupported Protocol Version selected"));

	return dataLength;
}

AliroError GattServer::Init(L2capServer::Spsm spsm)
{
	VerifyOrReturnStatus(L2capServer::IsValidDynamicSpsm(spsm), ALIRO_INVALID_ARGUMENT,
			     LOG_ERR("Invalid SPSM value"));

	size_t supportedVersionsCount{};
	const auto *supportedVersions = AliroStack::Instance().GetBleUwbProtocolVersions(supportedVersionsCount);

	// Generate the GATT attributes buffer once during initialization
	AliroError generateError = mGattAttributeValue.Generate(spsm, supportedVersions, supportedVersionsCount);
	VerifyOrReturnStatus(generateError == ALIRO_NO_ERROR, generateError,
			     LOG_ERR("Failed to generate GATT attribute buffer"));

	// Initialize the GATT service
	InitializeAttribute(GattAttributeIndex::kPrimaryService, BT_GATT_PRIMARY_SERVICE(&kServiceUuid));
	InitializeAttribute(GattAttributeIndex::kReaderCharacteristic,
			    BT_GATT_ATTRIBUTE(BT_UUID_GATT_CHRC, BT_GATT_PERM_READ, bt_gatt_attr_read_chrc, nullptr,
					      &kReaderCharacteristicAttributeValue));
	InitializeAttribute(GattAttributeIndex::kReaderCharacteristicValue,
			    BT_GATT_ATTRIBUTE(&kReaderCharacteristic.uuid, BT_GATT_PERM_READ,
					      ReaderDeclaredSpsmBleUwbProtocolversion, nullptr, this));
	InitializeAttribute(GattAttributeIndex::kUserDeviceSelectedCharacteristic,
			    BT_GATT_ATTRIBUTE(BT_UUID_GATT_CHRC, BT_GATT_PERM_READ, bt_gatt_attr_read_chrc, nullptr,
					      &kUserDeviceSelectedCharacteristicAttributeValue));
	InitializeAttribute(GattAttributeIndex::kUserDeviceSelectedCharacteristicValue,
			    BT_GATT_ATTRIBUTE(&kUserDeviceSelectedCharacteristic.uuid, BT_GATT_PERM_WRITE, nullptr,
					      UserDeviceSelectedSpsmBleUwbProtocolversion, this));

	// Register the GATT service
	mGattService = {
		.attrs = mGattAttributes.data(),
		.attr_count = mGattAttributes.size(),
	};

	int error = bt_gatt_service_register(&mGattService);
	VerifyOrReturnStatus(error == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to register GATT service: %d", error));

	return ALIRO_NO_ERROR;
}

AliroError GattServer::Deinit()
{
	int error = bt_gatt_service_unregister(&mGattService);
	VerifyOrReturnStatus(error == 0, ALIRO_ERROR_INTERNAL, LOG_ERR("Failed to unregister GATT service: %d", error));

	return ALIRO_NO_ERROR;
}

void GattServer::InitializeAttribute(GattAttributeIndex index, const bt_gatt_attr &attr)
{
	mGattAttributes[static_cast<size_t>(index)] = attr;
}

} // namespace Aliro
