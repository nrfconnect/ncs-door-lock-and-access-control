
/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "access_manager/access_manager.h"

namespace Aliro {

/**
 * @brief A template for custom access manager implementation.
 *
 * This class is a template for custom Access Manager implementation.
 * It can be used as a base to implement customized Access Manager.
 */
class AccessManagerImpl : public AccessManager {
private:
	friend class AccessManager;

	AliroError _Init(const Callbacks &callbacks);
	AliroError _StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey, SessionContext sessionContext);
#ifdef CONFIG_ALIRO_BLE_TP
	AliroError _StartAccessDecision(const CryptoTypes::PublicKey &userPublicKey, uint32_t rangingSessionId,
					const CryptoTypes::Ursk &ursk, SessionContext sessionContext);
#endif // CONFIG_ALIRO_BLE_TP
	AliroError _AddPublicKey(const CryptoTypes::PublicKey &publicKey);
	AliroError _RemovePublicKey(const CryptoTypes::PublicKey &publicKey);
	void _ClearStoredKeys();
	void _SetMaxAllowedDistance(uint32_t maxDistance);
	uint32_t _GetMaxAllowedDistance();
	void _HandleRangingSessionData(const UwbRangingData &uwbData);
	void _HandleSessionTermination(SessionContext sessionContext);
};

inline AccessManager &AccessManagerInstance()
{
	static AccessManagerImpl sInstance{};
	return sInstance;
}

} // namespace Aliro
