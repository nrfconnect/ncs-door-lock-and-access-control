/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "kpersistent_manager/kpersistent_manager.h"

#include <cstddef>

namespace Aliro {

class KpersistentManagerImpl : public KpersistentManager {
public:
	AliroError GetKpersistentCount(size_t &count) override;

	AliroError GetKpersistentKeyIds(CryptoTypes::KeyId *keyIds, size_t &count) override;

	AliroError PreserveKpersistent(const CryptoTypes::PublicKey &publicKey,
				       CryptoTypes::KeyId &kpersistentKeyId) override;

	AliroError RemoveKpersistent(size_t kpersistentKeyOffset) override;

	void RemoveAllKpersistent() override;

	AliroError GetAccessCredentialPublicKey(CryptoTypes::KeyId kpersistentKeyId,
						CryptoTypes::PublicKey &publicKey) override;

private:
	size_t mKpersistentCount{};
};

} // namespace Aliro
