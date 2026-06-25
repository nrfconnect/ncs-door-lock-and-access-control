/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "last_operation.h"

#include <doorlock/utils/mutex_guard.h>

#include <zephyr/kernel.h>

namespace Aliro {

namespace {

K_MUTEX_DEFINE(sLastOperationMutex);
LastOperation sLastOperation;

} // namespace

void SetLastOperation(bool isNfcSession, const CryptoTypes::PublicKey &accessCredentialPublicKey)
{
	DoorLock::Utils::MutexGuard lock{ sLastOperationMutex };
	const auto source = isNfcSession ? OperationSource::ThisUserDeviceInNfc :
					   OperationSource::ThisUserDeviceInBluetoothLeUwbAliroFlow;
	sLastOperation = LastOperation{ .source = source, .accessCredentialPublicKey = accessCredentialPublicKey };
}

void SetLastOperation(OperationSource source)
{
	DoorLock::Utils::MutexGuard lock{ sLastOperationMutex };
	sLastOperation = LastOperation{ .source = source, .accessCredentialPublicKey = std::nullopt };
}

LastOperation GetLastOperation()
{
	DoorLock::Utils::MutexGuard lock{ sLastOperationMutex };
	return sLastOperation;
}

} // namespace Aliro
