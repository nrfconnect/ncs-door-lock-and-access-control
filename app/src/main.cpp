/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "app_callbacks.h"
#include "storage.h"

#include "crypto/crypto.h"
#include "reader_identifier/reader_identifier.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(door_lock_app, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);


using namespace Aliro;

static void StorageInit()
{
	/* Load user device public key from storage. */
	EccP256PublicKey userDevicePublicKey{};
	int erc = KeyValueStorage::Instance().Get(kStorageKeyNameAccessCredentialPublicKey, userDevicePublicKey.data(),
						  userDevicePublicKey.size());
	if (!erc) {
		VerifyOrDie(CryptoKeyStorage::Instance().SetAccessCredentialPublicKey(PublicKey(
				    userDevicePublicKey.data(), userDevicePublicKey.size())) == ALIRO_NO_ERROR,
			    LOG_ERR("Cannot set public key."));
		LOG_INF("User device public key set");
	} else if (erc == -ENODATA) {
		LOG_INF("No User device public key available");
	} else {
		LOG_ERR("Cannot get user device public key, error code: %d", erc);
	}

	/* Load reader identifier. */
	ReaderIdentifier::GroupIdentifier groupIdentifier{};
	ReaderIdentifier::SubIdentifier subIdentifier{};
	erc = KeyValueStorage::Instance().Get(kStorageKeyNameGroupId, groupIdentifier);
	if (erc == -ENODATA) {
		LOG_INF("No group identifier available");
	} else {
		LOG_ERR("Cannot get group identifier, error code: %d", erc);
	}

	erc = KeyValueStorage::Instance().Get(kStorageKeyNameGroupSubId, subIdentifier);
	if (erc) {
		LOG_INF("No group sub identifier available");
	} else {
		LOG_ERR("Cannot get group sub identifier, error code: %d", erc);
	}

	ReaderIdentifier::Instance().Init(groupIdentifier, subIdentifier);
}

static void PrintReaderGroupIdentifier()
{
#ifdef CONFIG_ALIRO_PRINT_READER_GROUP_ID

	using ReaderId = ReaderIdentifier;
	LOG_HEXDUMP_INF(ReaderId::Instance().Get().Data(), ReaderId::Instance().Get().Size(), "Reader Identifier:");

	// Printf is used for output formatting - reader group ID can be printed on one line.
	printf("Provision the Test Harness with the following Reader Group Identifier:\n");
	// First 16 bytes of the Reader Identifier constitute the Reader Group Identifier
	auto end = ReaderId::Instance().Get().Begin() + ReaderId::Instance().Get().Size() / 2;
	for (auto it = ReaderId::Instance().Get().Begin(); it != end; ++it) {
		printf("%02x", *it);
	}
	printf("\n\n");

#endif
}

int main()
{
	LOG_INF("                                                                        \r\n"
		"                                          @@@   @@@@                    \r\n"
		"    @@@@@@@@@@@@                          @@@                           \r\n"
		"  @@@@@      @@@@@              @@@@   @@ @@@    @@      @@     @@@@    \r\n"
		" @@@@          @@@@          @@@@  @@@@@@ @@@    @@   @@@@@  @@@   @@@@ \r\n"
		"@@@@     @@      @@@        @@@       @@@ @@@    @@  @@@   @@@       @@@\r\n"
		"@@@     @@@@@    @@@       @@@        @@@ @@@    @@  @@    @@@        @@\r\n"
		"@@@   @@@@@@@@   @@@       @@@        @@@ @@@    @@  @@    @@@        @@\r\n"
		" @@@    @@@@    @@@@        @@@      @@@@ @@@    @@  @@     @@       @@@\r\n"
		"  @@@   @@@@   @@@@           @@@@@@@@@@@  @@@@@ @@  @@      @@@@@@@@@  \r\n"
		"   @@@@ @@@@ @@@@                                                       \r\n"
		"     @@ @@@@ @@                                                         \r\n");

	static NfcTpAliroCb appCallbacks = { .userDeviceDetected = AliroUserDeviceDetectedClb,
					     .communicationEnd = AliroCommunicationEndClb,
					     .error = AliroErrorHandlerClb,
					     .selectRecv = AliroSelectCommandRecvClb,
					     .auth0Recv = AliroAuth0RecvClb,
					     .auth1Recv = AliroAuth1RecvClb };

	/* Initialize crypto components. */
	VerifyOrDie(CryptoInstance().Init() == ALIRO_NO_ERROR, LOG_ERR("Cannot initialize crypto engine."));
	VerifyOrDie(CryptoKeyStorage::Instance().Init() == ALIRO_NO_ERROR, LOG_ERR("Cannot initialize key storage."));

	/* Initialize storage and load data. */
	StorageInit();

	/* Initialize NFC component. */
	VerifyOrDie(NfcTpAliroInit(&appCallbacks) == ALIRO_NO_ERROR, LOG_ERR("Cannot initialize NFC component."));

	PrintReaderGroupIdentifier();

	return 0;
}
