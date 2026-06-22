/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "aliro/connection_handle.h"
#include "aliro/types.h"

#include <cherry/cherry_ccc.h>
#include <zephyr/sys/slist.h>

struct aliro_uwb_session;

namespace Aliro::Uwb {

using SessionContextHandle = Aliro::ConnectionHandle;

/**
 * @brief UWB session context stored in the active-sessions list.
 */
struct SessionContext {
	SessionContext(aliro_uwb_session *uwbSessionContext, SessionContextHandle sessionContextData)
		: mUwbSessionContext(uwbSessionContext), mSessionContextData(sessionContextData)
	{
	}

	sys_snode_t mSessionContextNode{};
	aliro_uwb_session *mUwbSessionContext;
	SessionContextHandle mSessionContextData;
	cherry_ccc_session_state mSessionState{ CHERRY_CCC_SESSION_STATE_INIT };
	RangingSessionState mRangingSessionState{ RangingSessionState::Uninitialized };
#ifdef CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
	uint8_t mDisambiguationSessionIdx{ 0 };
#endif // CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_FRONT_BACK_DETECTION
};

} // namespace Aliro::Uwb
