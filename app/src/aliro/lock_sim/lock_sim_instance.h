/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#pragma once

#include "lock_sim.h"

namespace Aliro {

/**
 * @brief Get the LockSim instance.
 *
 * @return The LockSim instance.
 */
inline LockSim &LockSimInstance()
{
	static LockSim lockSim;
	return lockSim;
}

} // namespace Aliro
