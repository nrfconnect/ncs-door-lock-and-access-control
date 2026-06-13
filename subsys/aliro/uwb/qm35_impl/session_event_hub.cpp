/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "session_event_hub.h"

#include <doorlock/utils/mutex_guard.h>
#include <doorlock/utils/utils.h>

#include <aliro_uwb_adapter/aliro_uwb_adapter.h>
#include <aliro_uwb_adapter/aliro_uwb_session.h>

namespace {

K_MUTEX_DEFINE(sMutex);
sys_slist_t sSubscribers{};

} // namespace

namespace Aliro::Uwb {
namespace SessionEventHub {

void Register(Subscriber &subscriber)
{
	DoorLock::Utils::MutexGuard lock{ sMutex };

	VerifyOrReturn(!sys_slist_find(&sSubscribers, &subscriber, nullptr));

	sys_slist_append(&sSubscribers, &subscriber);
}

void Unregister(Subscriber &subscriber)
{
	DoorLock::Utils::MutexGuard lock{ sMutex };

	(void)sys_slist_find_and_remove(&sSubscribers, &subscriber);
}

void RemoveAll()
{
	DoorLock::Utils::MutexGuard lock{ sMutex };

	while (sys_slist_get(&sSubscribers))
		;
}

void DispatchSessionEvent(aliro_uwb_session_event *event, SessionContext *uwbSessionCtx)
{
	VerifyOrReturn(event && uwbSessionCtx);

	DoorLock::Utils::MutexGuard lock{ sMutex };

	sys_snode_t *node{};
	SYS_SLIST_FOR_EACH_NODE (&sSubscribers, node) {
		auto *subscriber = static_cast<Subscriber *>(node);
		VerifyAndCall(subscriber->mOnSessionEvent, *event, *uwbSessionCtx, subscriber->mCtx);
	}
}

} // namespace SessionEventHub
} // namespace Aliro::Uwb
