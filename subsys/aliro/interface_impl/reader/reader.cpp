/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <aliro/interface.h>

#include <reader_storage/reader.h>

namespace Aliro::Interface::Reader {

AliroError GetIdentifier(Identifier &identifier)
{
	return DoorLock::ReaderStorage::GetIdentifier(identifier);
}

AliroError GetPublicKey(CryptoTypes::PublicKey &publicKey)
{
	return DoorLock::ReaderStorage::GetPublicKey(publicKey);
}

bool IsCertificateProvisioned()
{
#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
	return DoorLock::ReaderStorage::IsCertificateSet() && DoorLock::ReaderStorage::IsIssuerPublicKeySet();
#else // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
	return false;
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
}

AliroError GetIssuerPublicKey(CryptoTypes::PublicKey &publicKey)
{
#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
	return DoorLock::ReaderStorage::GetIssuerPublicKey(publicKey);
#else // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
	ARG_UNUSED(publicKey);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
}

AliroError GetCertificate(ConstData &certificate)
{
#ifdef CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
	return DoorLock::ReaderStorage::GetCertificate(certificate);
#else // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
	ARG_UNUSED(certificate);
	return ALIRO_ERROR_NOT_IMPLEMENTED;
#endif // CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_SUPPORT
}

} // namespace Aliro::Interface::Reader
