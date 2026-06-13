/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "session_context.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/slist.h>

#include <cstddef>
#include <cstdint>

struct aliro_uwb_session_event;

namespace Aliro::Uwb {

/**
 * @brief UWB session event subscriber list (mutex + slist).
 *
 * Hub mutex protects the subscriber list; callbacks are invoked with that mutex held.
 */
namespace SessionEventHub {

/**
 * @brief Thread-safe list of UWB session event extensions (Zephyr-style multi-callback).
 *
 * UltraWideBandImpl invokes the hub during core session handling.
 */
struct Subscriber : sys_snode_t {
	void (*mOnSessionEvent)(const aliro_uwb_session_event &event, const SessionContext &uwbSessionCtx, void *ctx);
	void *mCtx{ nullptr };
};

void Register(Subscriber &subscriber);
void Unregister(Subscriber &subscriber);
void RemoveAll();
void DispatchSessionEvent(aliro_uwb_session_event *event, SessionContext *uwbSessionCtx);

} // namespace SessionEventHub

} // namespace Aliro::Uwb
