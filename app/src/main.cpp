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
#include "crypto/salt.h"
#include "errors.h"
#include "protocol_versions/protocol_version_arbiter.h"
#include "protocol_versions/supported_versions.h"
#include "reader_identifier.h"
#include "select_response.h"
#include "transport_protocol/control_flow_command.h"
#include "transport_protocol/transport_protocol_nfc.h"
#include "utils.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(aliro_app, CONFIG_NCS_ALIRO_LOG_LEVEL);

// TODO: Find better place for these
static Aliro::Salt sSalt;
static Aliro::AccessProtocolCrypto sAccessProtocolCrypto;
static bool sSecureChannel{ false };

static bool IsSecureChannel()
{
	return sSecureChannel;
}

static AliroError SelectCommandRecv(uint8_t *rawData, size_t dataLen)
{
	AliroError error{};
	Aliro::ProtocolVersionArbiter protocolVersion{};
	auto response = Aliro::SELECTResponse(rawData, dataLen);
	response.Deserialize();

	auto aid = response.GetApplicationIdentifier();
	auto versionList = response.GetSupportedProtocolVersions();

	if ((memcmp(aid.Begin(), Aliro::kExpPhase.begin(), Aliro::kExpPhase.size()) == 0) && versionList) {
		error = protocolVersion.Init(versionList.value().version.data(), versionList.value().versionsNum,
					     Aliro::ProtocolVersions::kSupportedVersions,
					     Aliro::ProtocolVersions::kNumberOfSupported);
	} else if ((memcmp(aid.Begin(), Aliro::kStepUpPhase.begin(), Aliro::kStepUpPhase.size()) == 0) &&
		   !versionList) {
		LOG_INF("Step-up phase is not yet implemented");
		return ALIRO_ERROR_NOT_IMPLEMENTED;
	} else {
		LOG_ERR("SELECT response: malformed payload");
		return ALIRO_INVALID_STATE;
	}

	if (protocolVersion.HasSupportedVersion()) {
		LOG_INF("SELECT -> AUTH0");

		Aliro::AUTH0Command cmd{};
		auto policy = Aliro::AUTH0Command::UserAuthenticationPolicy::UserDeviceSettings;
		auto expPhase = Aliro::AUTH0Command::ExpeditedPhaseType::ExpeditedStandard;
		Aliro::PublicKey readerEPubKey;

		AliroError error = sAccessProtocolCrypto.TransactionIdentifierGenerate();
		VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
				     LOG_ERR("Cannot generate transaction identifier, error code: %d", error.ToInt()));

		error = sAccessProtocolCrypto.GenerateReaderEphemeralKey(readerEPubKey);
		VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
				     LOG_ERR("Cannot generate ephemeral key pair, error code: %d", error.ToInt()));

		cmd.SetCommandParameters(expPhase);
		cmd.SetUserAuthenticationPolicy(policy);
		cmd.SetProtocolVersion(protocolVersion.GetCurrentVersion());
		cmd.SetEphemeralPubKey(readerEPubKey);
		cmd.SetTransactionIdentifier(sAccessProtocolCrypto.GetTransactionIdentifier());

		SharedByteSpan span = cmd.Serialize();

		VerifyOrReturnStatus(span, ALIRO_ERROR_INTERNAL, LOG_ERR("Serialization failed"));

		sSalt.SetProprietaryInfo(response.GetProprietaryInformation());
		sSalt.SetFlag(ToUnderlying(expPhase), ToUnderlying(policy));
		sSalt.SetEphemeralPubKeyX(sAccessProtocolCrypto.ReaderEphemeralKeyGetX());
		sSalt.SetTransactionId(sAccessProtocolCrypto.GetTransactionIdentifier());
		sSalt.SetProtocolVersion(protocolVersion.GetCurrentVersionSpan());
		sSalt.Create();

		return Aliro::NfcTpAliroAuth0(span.Begin(), span.Size());
	} else {
		Aliro::CONTROL_FLOWCommand cmd{};

		LOG_INF("SELECT -> CONTROL_FLOW");

		cmd.SetParameters(Aliro::CONTROL_FLOWCommand::S1Parameter::TransactionFail,
				  Aliro::CONTROL_FLOWCommand::S2Parameter::VersionNotSupported);
		SharedByteSpan span = cmd.Serialize();

		VerifyOrReturnStatus(span, ALIRO_ERROR_INTERNAL, LOG_ERR("Serialization failed"));

		return Aliro::NfcTpAliroControlFlow(span.Begin(), span.Size());
	}
}

static AliroError ErrorHandler(Aliro::AliroNfcErrorStatusCode statusCode)
{
	if (statusCode == Aliro::AliroNfcErrorStatusCode::kAliroNfcGenericError) {
		LOG_ERR("Generic error");
		return ALIRO_ERROR_INTERNAL;
	} else {
		Aliro::CONTROL_FLOWCommand cmd{};

		LOG_INF("AUTH0 -> CONTROL_FLOW");

		cmd.SetParameters(Aliro::CONTROL_FLOWCommand::S1Parameter::TransactionFail,
				  Aliro::CONTROL_FLOWCommand::S2Parameter::NoInformation);
		SharedByteSpan span = cmd.Serialize();

		VerifyOrReturnStatus(span, ALIRO_ERROR_INTERNAL, LOG_ERR("Serialization failed"));

		return Aliro::NfcTpAliroControlFlow(span.Begin(), span.Size());
	}
}

static AliroError Auth0Recv(uint8_t *rawData, size_t dataLen)
{
	LOG_INF("AUTH0 response received -> sending AUTH1 command");
	auto response = Aliro::AUTH0Response(rawData, dataLen);
	response.Deserialize();
	auto readerAuthentication =
		Aliro::AUTH1AuthenticationData(Aliro::AUTH1AuthenticationData::DeviceAuthenticationType::Reader);

	AliroError error = sAccessProtocolCrypto.SetUserDeviceEphemeralKey(
		SharedByteSpan(const_cast<Byte *>(response.GetCredentialEphemeralPubKey().Data()),
			       response.GetCredentialEphemeralPubKey().Size()));
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
			     LOG_ERR("Cannot import keys, error code: %d", error.ToInt()));

	readerAuthentication.SetCredentialEPubKX(sAccessProtocolCrypto.GetUserDeviceEphemeralKeyXCoordinate());
	readerAuthentication.SetReaderEPubKX(sAccessProtocolCrypto.ReaderEphemeralKeyGetX());
	readerAuthentication.SetTransactionId(sAccessProtocolCrypto.GetTransactionIdentifier());

	auto signMaterial = readerAuthentication.Serialize();

	Aliro::TransactionSignature signature{};
	error = Aliro::CryptoInstance().GenerateSignature(signMaterial.Begin(), signMaterial.Size(), signature);
	VerifyOrReturnStatus(error == ALIRO_NO_ERROR, error,
			     LOG_ERR("Cannot generate signature, error code: %d", error.ToInt()));

	Aliro::AUTH1Command cmd{};
	cmd.SetCommandParameters(Aliro::AUTH1Command::AccessCredentialType::AccessCredentialPublicKey);
	cmd.SetSignature(Aliro::Signature(signature.data(), signature.size()));
	SharedByteSpan span = cmd.Serialize();

	VerifyOrReturnStatus(span, ALIRO_ERROR_INTERNAL, LOG_ERR("Serialization failed"));

	// TODO: Move slat and key derivation to the `AccessProtocolCrypto` class.
	// The same for secure channel flag.
	// After transaction finish or on the error, reset secure channel flag and remove all keys.
	auto salt = sSalt.Get();
	Aliro::SessionBoundKeys sessionKeys;
	/* Aliro spec. v0.9.0, 8.3.1.11:  value for info: x component of the Access Credential
	 * ephemeral public key || auth0_command_vendor_extension TLV (if such tag
	 * was present in AUTH0 command) || auth0_response_vendor_extension
	 * TLV (if such tag was present in AUTH0 response)
	 * */
	SharedByteSpan info;
	info = sAccessProtocolCrypto.GetUserDeviceEphemeralKeyXCoordinate();
	error = Aliro::CryptoInstance().DeriveSessionKeys(sAccessProtocolCrypto.GetSharedKey(), info, salt,
							  sessionKeys);
	VerifyOrExit(error == ALIRO_NO_ERROR);

	Aliro::CryptoKeyStorage::Instance().SetSessionBoundKeys(sessionKeys);

	error = Aliro::NfcTpAliroAuth1(span.Begin(), span.Size());

	sSecureChannel = true;
exit:
	sSalt.Release();
	return error;
}

static AliroError Auth1Recv(uint8_t *rawData, size_t dataLen)
{
	LOG_INF("AUTH1 response received -> processing AUTH1 response");

	auto response = Aliro::AUTH1Response(rawData, dataLen);
	if (!response.Deserialize()) {
		LOG_ERR("AUTH1 deserialization failed, some mandatory fields missing in the payload");
		return ALIRO_INVALID_STATE;
	}

	auto signature = response.GetUserDeviceSignature();
	auto keySlot = response.GetKeySlot();
	auto publicKey = response.GetLongTermPublicKey();

	auto userDeviceAuthentication =
		Aliro::AUTH1AuthenticationData(Aliro::AUTH1AuthenticationData::DeviceAuthenticationType::UserDevice);

	userDeviceAuthentication.SetCredentialEPubKX(sAccessProtocolCrypto.GetUserDeviceEphemeralKeyXCoordinate());
	userDeviceAuthentication.SetReaderEPubKX(sAccessProtocolCrypto.ReaderEphemeralKeyGetX());
	userDeviceAuthentication.SetTransactionId(sAccessProtocolCrypto.GetTransactionIdentifier());

	auto materialToVerify = userDeviceAuthentication.Serialize();

	// TODO: Add signature verification

	/* Only one of below can be present in the AUTH1 response. */
	if (keySlot && !publicKey) {
		LOG_INF("AUTH1 response: Key slot received");
	} else if (!keySlot && publicKey) {
		LOG_INF("AUTH1 response: Public key received");
	} else {
		LOG_ERR("AUTH1 response: malformed payload");
		return ALIRO_ERROR_INTERNAL;
	}

	LOG_HEXDUMP_INF(materialToVerify.Data(), materialToVerify.Size(), "User device authentication data:");
	LOG_HEXDUMP_INF(signature.Data(), signature.Size(), "AUTH1 response: received UserDevice signature:");

	return ALIRO_NO_ERROR;
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

	static Aliro::NfcTpAliroCb apCb = { .userDeviceDetected =
						    []() {
							    LOG_INF("IDLE -> SELECT");
							    return Aliro::NfcTpAliroSelectExpPhase();
						    },
					    .communicationEnd = []() -> AliroError {
						    LOG_INF("CONTROL_FLOW -> IDLE");
						    return ALIRO_NO_ERROR;
					    },
					    .error = ErrorHandler,
					    .selectRecv = SelectCommandRecv,
					    .auth0Recv = Auth0Recv,
					    .auth1Recv = Auth1Recv,
					    .isSecureChannel = IsSecureChannel

	};

	/* Initialize crypto components. */
	VerifyOrDie(Aliro::CryptoInstance().Init() == ALIRO_NO_ERROR, LOG_ERR("Cannot initialize crypto engine."));
	VerifyOrDie(Aliro::CryptoKeyStorage::Instance().Init() == ALIRO_NO_ERROR,
		    LOG_ERR("Cannot initialize key storage."));
	/* Initialize NFC component. */
	VerifyOrDie(Aliro::NfcTpAliroInit(&apCb) == ALIRO_NO_ERROR, LOG_ERR("Cannot initialize NFC component."));

	return 0;
}
