/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <app/clusters/time-synchronization-server/DefaultTimeSyncDelegate.h>

class TimeSyncDelegate : public chip::app::Clusters::TimeSynchronization::DefaultTimeSyncDelegate {
public:
	void UTCTimeAvailabilityChanged(uint64_t time) override;
};
