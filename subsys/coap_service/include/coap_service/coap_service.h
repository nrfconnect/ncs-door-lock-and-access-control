/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

namespace DoorLock::CoAPService {

using GetStateCallback = bool (*)();
using SetStateCallback = void (*)(bool);

void Init(GetStateCallback getStateCb, SetStateCallback setStateCb);

int Start();
int Stop();

}; // namespace DoorLock::CoAPService
