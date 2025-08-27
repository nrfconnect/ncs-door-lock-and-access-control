/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#pragma once

#include <app/clusters/door-lock-server/door-lock-delegate.h>

class DoorLockDelegate : public chip::app::Clusters::DoorLock::Delegate {
public:
	DoorLockDelegate() = default;
	~DoorLockDelegate() = default;

	CHIP_ERROR Init();

	virtual CHIP_ERROR GetAliroReaderVerificationKey(chip::MutableByteSpan &verificationKey) override;
	virtual CHIP_ERROR GetAliroReaderGroupIdentifier(chip::MutableByteSpan &groupIdentifier) override;
	virtual CHIP_ERROR GetAliroReaderGroupSubIdentifier(chip::MutableByteSpan &groupSubIdentifier) override;
	virtual CHIP_ERROR
	GetAliroExpeditedTransactionSupportedProtocolVersionAtIndex(size_t index,
								    chip::MutableByteSpan &protocolVersion) override;
	virtual CHIP_ERROR GetAliroGroupResolvingKey(chip::MutableByteSpan &groupResolvingKey) override;
	virtual CHIP_ERROR
	GetAliroSupportedBLEUWBProtocolVersionAtIndex(size_t index, chip::MutableByteSpan &protocolVersion) override;
	virtual uint8_t GetAliroBLEAdvertisingVersion() override;
	virtual uint16_t GetNumberOfAliroCredentialIssuerKeysSupported() override;
	virtual uint16_t GetNumberOfAliroEndpointKeysSupported() override;
	virtual CHIP_ERROR SetAliroReaderConfig(const chip::ByteSpan &signingKey, const chip::ByteSpan &verificationKey,
						const chip::ByteSpan &groupIdentifier,
						const chip::Optional<chip::ByteSpan> &groupResolvingKey) override;
	virtual CHIP_ERROR ClearAliroReaderConfig() override;
};
