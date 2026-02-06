/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/errors.h"
#include "aliro/platform/ble/ble_advertising_arbiter.h"

#include <bluetooth/services/nus.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>

#include <algorithm>
#include <array>

namespace Aliro::BtNus {

class NUSService {
public:
	using CommandCallback = void (*)(void *context);
	struct Command {
		std::array<uint8_t, CONFIG_DOOR_LOCK_BT_NUS_MAX_COMMAND_LEN> mCommand;
		size_t mCommandLength;
		CommandCallback mCallback;
		void *mContext;

		Command() = default;

		Command(const char *name, size_t length, CommandCallback callback, void *context)
			: mCommandLength(length), mCallback(callback), mContext(context)
		{
			std::copy_n(reinterpret_cast<const uint8_t *>(name), length, mCommand.begin());
		}
	};

	static NUSService &Instance()
	{
		static NUSService instance;
		return instance;
	}

	/**
	 * @brief Initialize and start the NUS service.
	 * @return ALIRO_NO_ERROR on success, error code otherwise.
	 */
	AliroError Start();

	/**
	 * @brief Stop the Nordic UART Service server.
	 */
	void StopServer();

	/**
	 * @brief Send data to the connected device.
	 * @return false if the device cannot send data, true otherwise.
	 */
	bool SendData(const char *const data, size_t length);

	/**
	 * @brief Register a new command for NUS service.
	 * @return false if there is no space for the next command, or the name is empty.
	 */
	bool RegisterCommand(const char *const name, size_t length, CommandCallback callback, void *context);

	/**
	 * @brief Check if the NUS service is started.
	 * @return true if the NUS service is started, false otherwise.
	 */
	bool IsNusStarted() const { return mIsStarted; }

	/**
	 * @brief Bluetooth LE connection callbacks
	 */
	void Connected(bt_conn *conn, uint8_t err);
	void Disconnected(bt_conn *conn, uint8_t reason);
	void SecurityChanged(bt_conn *conn, bt_security_t level, enum bt_security_err err);

private:
	NUSService() = default;
	NUSService(const NUSService &) = delete;
	NUSService &operator=(const NUSService &) = delete;
	NUSService(NUSService &&) = delete;
	NUSService &operator=(NUSService &&) = delete;

	void DispatchCommand(const char *const data, uint16_t len);

	void RxCallback(bt_conn *conn, const uint8_t *const data, uint16_t len);
	void AuthPasskeyDisplay(bt_conn *conn, unsigned int passkey);
	void AuthCancel(bt_conn *conn);
	void PairingComplete(bt_conn *conn, bool bonded);
	void PairingFailed(bt_conn *conn, enum bt_security_err reason);
	char *GetAddressString(bt_conn *conn);

	bool mIsStarted{ false };

	std::array<Command, CONFIG_DOOR_LOCK_BT_NUS_MAX_COMMANDS> mCommandsList{};
	uint8_t mCommandsCounter{ 0 };
	bt_conn *mBTConnection{ nullptr };

	// NUS service UUID.
	static constexpr std::array<uint8_t, BT_UUID_SIZE_128> kNusUuid{ BT_UUID_NUS_VAL };

	// Advertising flags.
	static constexpr uint8_t kAdvertisingFlags{ BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR };

	// Advertising data storage.
	DoorLock::Interface::BleAdvertisingArbiter::Request mRequest{};
};

} // namespace Aliro::BtNus
