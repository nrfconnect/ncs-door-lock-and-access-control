/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "access_protocol/auth0_command.h"
#include "access_protocol/auth0_response.h"
#include "access_protocol/auth1_command.h"
#include "access_protocol/auth1_response.h"
#include "access_protocol/auth_data.h"
#include "crypto/crypto.h"
#include "crypto/crypto_access_protocol.h"
#include "crypto/crypto_key_storage.h"
#include "errors.h"
#include "protocol_versions/protocol_version_arbiter.h"
#include "protocol_versions/supported_versions.h"
#include "reader_identifier/reader_identifier.h"
#include "transport_protocol/control_flow_command.h"
#include "transport_protocol/select_response.h"
#include "transport_protocol/transport_protocol_nfc.h"
#include "util/utils.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(door_lock_app, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

using namespace Aliro;

static AliroError SelectCommandRecv(uint8_t *rawData, size_t dataLen)
{
	AliroError error{};
	ProtocolVersionArbiter protocolVersion{};
	auto response = SELECTResponse(rawData, dataLen);
	response.Deserialize();

	auto aid = response.GetApplicationIdentifier();
	auto versionList = response.GetSupportedProtocolVersions();

	if ((memcmp(aid.Begin(), kExpPhase.begin(), kExpPhase.size()) == 0) && versionList) {
		error = protocolVersion.Init(versionList.value().version.data(), versionList.value().versionsNum,
					     ProtocolVersions::kSupportedVersions,
					     ProtocolVersions::kNumberOfSupported);
	} else if ((memcmp(aid.Begin(), kStepUpPhase.begin(), kStepUpPhase.size()) == 0) && !versionList) {
		LOG_INF("Step-up phase is not yet implemented");
		return ALIRO_ERROR_NOT_IMPLEMENTED;
	} else {
		LOG_ERR("SELECT response: malformed payload");
		return ALIRO_INVALID_STATE;
	}

	if (protocolVersion.HasSupportedVersion()) {
		LOG_INF("SELECT -> AUTH0");

		AUTH0Command cmd{};
		auto policy = AUTH0Command::UserAuthenticationPolicy::UserDeviceSettings;
		auto expPhase = AUTH0Command::ExpeditedPhaseType::ExpeditedStandard;
		PublicKey readerEPubKey;

		error = AccessProtocolCrypto::Instance().StartSession(readerEPubKey);
		VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
				     LOG_ERR("Cannot start new session, error code: %d", error.ToInt()));

		cmd.SetCommandParameters(expPhase);
		cmd.SetUserAuthenticationPolicy(policy);
		cmd.SetProtocolVersion(protocolVersion.GetCurrentVersion());
		cmd.SetEphemeralPubKey(readerEPubKey);
		cmd.SetTransactionIdentifier(AccessProtocolCrypto::Instance().GetTransactionIdentifier());

		SharedByteSpan span = cmd.Serialize();

		VerifyOrReturnStatus(span, ALIRO_ERROR_INTERNAL, LOG_ERR("Serialization failed"));

		AccessProtocolCrypto::Instance().mSalt.SetProprietaryInfo(response.GetProprietaryInformation());
		AccessProtocolCrypto::Instance().mSalt.SetFlag(ToUnderlying(expPhase), ToUnderlying(policy));
		AccessProtocolCrypto::Instance().mSalt.SetProtocolVersion(protocolVersion.GetCurrentVersionSpan());

		return NfcTpAliroAuth0(span.Begin(), span.Size());
	} else {
		CONTROL_FLOWCommand cmd{};

		LOG_INF("SELECT -> CONTROL_FLOW");

		cmd.SetParameters(CONTROL_FLOWCommand::S1Parameter::TransactionFail,
				  CONTROL_FLOWCommand::S2Parameter::VersionNotSupported);
		SharedByteSpan span = cmd.Serialize();

		VerifyOrReturnStatus(span, ALIRO_ERROR_INTERNAL, LOG_ERR("Serialization failed"));

		return NfcTpAliroControlFlow(span.Begin(), span.Size());
	}
}

static AliroError ErrorHandler(AliroNfcErrorStatusCode statusCode)
{
	if (statusCode == AliroNfcErrorStatusCode::kAliroNfcGenericError) {
		LOG_ERR("Generic error");
		return ALIRO_ERROR_INTERNAL;
	} else {
		CONTROL_FLOWCommand cmd{};

		LOG_INF("AUTH0 -> CONTROL_FLOW");

		cmd.SetParameters(CONTROL_FLOWCommand::S1Parameter::TransactionFail,
				  CONTROL_FLOWCommand::S2Parameter::NoInformation);
		SharedByteSpan span = cmd.Serialize();

		VerifyOrReturnStatus(span, ALIRO_ERROR_INTERNAL, LOG_ERR("Serialization failed"));

		return NfcTpAliroControlFlow(span.Begin(), span.Size());
	}
}

static AliroError Auth0Recv(uint8_t *rawData, size_t dataLen)
{
	LOG_INF("AUTH0 response received -> sending AUTH1 command");
	auto response = AUTH0Response(rawData, dataLen);
	response.Deserialize();
	auto readerAuthentication = AUTH1AuthenticationData(AUTH1AuthenticationData::DeviceAuthenticationType::Reader);

	AliroError error =
		CryptoKeyCache::Instance().SetUserDeviceEphemeralKey(response.GetCredentialEphemeralPubKey());
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
			     LOG_ERR("Cannot import keys, error code: %d", error.ToInt()));

	readerAuthentication.SetCredentialEPubKX(CryptoKeyCache::Instance().GetUserDeviceEphemeralKeyXCoordinate());
	readerAuthentication.SetReaderEPubKX(CryptoKeyCache::Instance().mReaderEphemeralKey.GetXCoordinate());
	readerAuthentication.SetTransactionId(AccessProtocolCrypto::Instance().GetTransactionIdentifier());

	auto signMaterial = readerAuthentication.Serialize();

	TransactionSignature signature{};
	error = CryptoInstance().GenerateSignature(signMaterial.Begin(), signMaterial.Size(), signature);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
			     LOG_ERR("Cannot generate signature, error code: %d", error.ToInt()));

	AUTH1Command cmd{};
	cmd.SetCommandParameters(AUTH1Command::AccessCredentialType::AccessCredentialPublicKey);
	cmd.SetSignature(Signature(signature.data(), signature.size()));
	SharedByteSpan span = cmd.Serialize();
	VerifyOrReturnStatus(span, ALIRO_ERROR_INTERNAL, LOG_ERR("Serialization failed"));

	error = AccessProtocolCrypto::Instance().EstablishSecureChannel(response.GetCredentialEphemeralPubKey());
	VerifyOrExit(error == ALIRO_NO_ERROR);

	error = NfcTpAliroAuth1(span.Begin(), span.Size());
	VerifyOrExit(error == ALIRO_NO_ERROR);

	return error;
exit:
	AccessProtocolCrypto::Instance().FinishSession();
	return error;
}

static AliroError Auth1Recv(uint8_t *rawData, size_t dataLen)
{
	LOG_INF("AUTH1 response received -> processing AUTH1 response");

	auto response = AUTH1Response(rawData, dataLen);

	VerifyOrReturnStatus(response.Deserialize(), ALIRO_INVALID_STATE,
			     LOG_ERR("AUTH1 deserialization failed, some mandatory fields missing in the payload");
			     AccessProtocolCrypto::Instance().FinishSession());

	auto signature = response.GetUserDeviceSignature();
	auto keySlot = response.GetKeySlot();
	auto publicKey = response.GetLongTermPublicKey();

	auto userDeviceAuthentication =
		AUTH1AuthenticationData(AUTH1AuthenticationData::DeviceAuthenticationType::UserDevice);

	userDeviceAuthentication.SetCredentialEPubKX(CryptoKeyCache::Instance().GetUserDeviceEphemeralKeyXCoordinate());
	userDeviceAuthentication.SetReaderEPubKX(CryptoKeyCache::Instance().mReaderEphemeralKey.GetXCoordinate());
	userDeviceAuthentication.SetTransactionId(AccessProtocolCrypto::Instance().GetTransactionIdentifier());

	auto materialToVerify = userDeviceAuthentication.Serialize();

	// TODO: Add signature verification

	/* Only one of below can be present in the AUTH1 response. */
	if (keySlot && !publicKey) {
		LOG_INF("AUTH1 response: Key slot received");
	} else if (!keySlot && publicKey) {
		LOG_INF("AUTH1 response: Public key received");
	} else {
		LOG_ERR("AUTH1 response: malformed payload");
		AccessProtocolCrypto::Instance().FinishSession();
		return ALIRO_ERROR_INTERNAL;
	}

	LOG_HEXDUMP_INF(materialToVerify.Data(), materialToVerify.Size(), "User device authentication data:");
	LOG_HEXDUMP_INF(signature.Data(), signature.Size(), "AUTH1 response: received UserDevice signature:");

	AccessProtocolCrypto::Instance().FinishSession();
	return ALIRO_NO_ERROR;
}

static void PrintReaderGroupIdentifier()
{
#ifdef CONFIG_ALIRO_PRINT_READER_GROUP_ID

	using ReaderId = Aliro::ReaderIdentifier;
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

	static NfcTpAliroCb apCb = {
		.userDeviceDetected =
			[]() {
				LOG_INF("IDLE -> SELECT");
				return NfcTpAliroSelectExpPhase();
			},
		.communicationEnd = []() -> AliroError {
			LOG_INF("CONTROL_FLOW -> IDLE");
			AccessProtocolCrypto::Instance().FinishSession();
			return ALIRO_NO_ERROR;
		},
		.error = ErrorHandler,
		.selectRecv = SelectCommandRecv,
		.auth0Recv = Auth0Recv,
		.auth1Recv = Auth1Recv,
	};

	/* Initialize crypto components. */
	VerifyOrDie(CryptoInstance().Init() == ALIRO_NO_ERROR, LOG_ERR("Cannot initialize crypto engine."));
	VerifyOrDie(CryptoKeyStorage::Instance().Init() == ALIRO_NO_ERROR, LOG_ERR("Cannot initialize key storage."));
	/* Initialize NFC component. */
	VerifyOrDie(NfcTpAliroInit(&apCb) == ALIRO_NO_ERROR, LOG_ERR("Cannot initialize NFC component."));

	PrintReaderGroupIdentifier();

	return 0;
}
