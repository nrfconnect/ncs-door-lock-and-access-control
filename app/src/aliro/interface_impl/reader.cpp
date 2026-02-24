/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/interface.h"

#include "reader.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(interface_reader, CONFIG_DOOR_LOCK_APP_LOG_LEVEL);

namespace Aliro::Interface::Reader {

AliroError GetIdentifier(Identifier &identifier)
{
	return DoorLock::Storage::Reader::GetIdentifier(identifier);
}

AliroError GetPublicKey(CryptoTypes::PublicKey &publicKey)
{
	return DoorLock::Storage::Reader::GetPublicKey(publicKey);
}

bool IsCertificateProvisioned()
{
#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
	return DoorLock::Storage::Reader::IsCertificateSet() && DoorLock::Storage::Reader::IsIssuerPublicKeySet();
#else // CONFIG_DOOR_LOCK_READER_CERTIFICATE
	return false;
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE
}

AliroError GetIssuerPublicKey(CryptoTypes::PublicKey &publicKey)
{
#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
	return DoorLock::Storage::Reader::GetIssuerPublicKey(publicKey);
#else // CONFIG_DOOR_LOCK_READER_CERTIFICATE
	ARG_UNUSED(publicKey);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE
}

AliroError GetCertificate(ConstData &certificate)
{
#ifdef CONFIG_DOOR_LOCK_READER_CERTIFICATE
	return DoorLock::Storage::Reader::GetCertificate(certificate);
#else // CONFIG_DOOR_LOCK_READER_CERTIFICATE
	ARG_UNUSED(certificate);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif // CONFIG_DOOR_LOCK_READER_CERTIFICATE
}

} // namespace Aliro::Interface::Reader
