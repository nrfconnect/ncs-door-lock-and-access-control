/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <reader_storage/reader.h>

#include "core.h"
#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
#include "cert.h"
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_SUPPORT
#include "group.h"
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_SUPPORT

#include <aliro/utils.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(reader_storage, CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_LOG_LEVEL);

namespace DoorLock::ReaderStorage {

namespace {

#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL
ReaderDataChangedCallback sDataChangedCallback{ nullptr };
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL

bool sIsInitialized{ false };

} // namespace

AliroError Init(ReaderDataChangedCallback dataChangedCallback)
{
#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL
	sDataChangedCallback = dataChangedCallback;
#else
	ARG_UNUSED(dataChangedCallback);
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL

	ReturnErrorOnFailure(LoadCore());

#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
	ReturnErrorOnFailure(LoadCert());
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT

	sIsInitialized = true;

	return ALIRO_NO_ERROR;
}

AliroError ClearAll()
{
	ReturnErrorOnFailure(ClearIdentifier());
	ReturnErrorOnFailure(ClearPrivateKey());

#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
	ReturnErrorOnFailure(ClearCert());
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT

#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_SUPPORT
	ReturnErrorOnFailure(ClearGroup());
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_GROUP_RESOLVING_KEY_SUPPORT

	return ALIRO_NO_ERROR;
}

#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL

bool IsInitialized()
{
	return sIsInitialized;
}

void NotifyDataChanged()
{
	VerifyAndCall(sDataChangedCallback);
}

#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_SHELL

} // namespace DoorLock::ReaderStorage
