/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#pragma once

#include <app/clusters/door-lock-server/door-lock-delegate.h>

namespace DoorLock {

class DoorLockDelegate : public chip::app::Clusters::DoorLock::Delegate {
public:
	CHIP_ERROR GetAliroReaderVerificationKey(chip::MutableByteSpan &verificationKey) override;
	CHIP_ERROR GetAliroReaderGroupIdentifier(chip::MutableByteSpan &groupIdentifier) override;
	CHIP_ERROR GetAliroReaderGroupSubIdentifier(chip::MutableByteSpan &groupSubIdentifier) override;
	CHIP_ERROR
	GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
								    chip::MutableByteSpan &protocolVersion) override;
	CHIP_ERROR GetAliroGroupResolvingKey(chip::MutableByteSpan &groupResolvingKey) override;
	CHIP_ERROR
	GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, chip::MutableByteSpan &protocolVersion) override;
	uint8_t GetAliroBLEAdvertisingVersion() override;
	uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() override;
	uint16_t GetNumberOfAliroEndpointKeysSupported() override;
	CHIP_ERROR SetAliroReaderConfig(const chip::ByteSpan &signingKey, const chip::ByteSpan &verificationKey,
					const chip::ByteSpan &groupIdentifier,
					const chip::Optional<chip::ByteSpan> &groupResolvingKey) override;
	CHIP_ERROR ClearAliroReaderConfig() override;

private:
	/** @brief Called after a new Aliro reader configuration has been stored successfully.
	 *
	 * Can be used to start the Aliro stack once configuration data is available.
	 */
	virtual void OnAliroReaderConfigSet() = 0;

	/** @brief Called before the existing Aliro reader configuration is cleared from storage.
	 *
	 * Can be used to stop the Aliro stack before the configuration is removed.
	 */
	virtual void OnAliroReaderConfigClear() = 0;
};

} // namespace DoorLock
