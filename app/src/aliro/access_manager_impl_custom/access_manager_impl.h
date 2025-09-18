
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

	void _SetApplicationCallbacks(const ApplicationCallbacks &callbacks);
	void _SetStackCallbacks(const StackCallbacks &callbacks);
	AliroError _VerifyAccessCredential(const CryptoTypes::PublicKey &userPublicKey, bool isNfcSession,
					   SessionContext sessionContext);
#ifdef CONFIG_ALIRO_BLE_UWB
	AliroError _StartRangingSession(uint32_t rangingSessionId, const CryptoTypes::Ursk &ursk,
					SessionContext sessionContext);
#endif // CONFIG_ALIRO_BLE_UWB
	AliroError _AddPublicKey(const CryptoTypes::PublicKey &publicKey);
	AliroError _RemovePublicKey(const CryptoTypes::PublicKey &publicKey);
	void _ClearStoredKeys();
	void _SetMaxAllowedDistance(uint32_t maxDistance);
	uint32_t _GetMaxAllowedDistance();
	void _HandleRangingSessionData(SessionContext sessionContext, const UwbRangingData &uwbData);
	void _HandleSessionTermination(SessionContext sessionContext);
};

inline AccessManager &AccessManagerInstance()
{
	static AccessManagerImpl sInstance{};
	return sInstance;
}

} // namespace Aliro
