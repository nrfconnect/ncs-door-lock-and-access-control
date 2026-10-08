/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro/types.h>

namespace DoorLock {

/**
 * @brief Builds the Reader Descriptor for @ref Aliro::AliroStack::Init().
 *
 * @return Reader Descriptor fields for this product (vendor OUI/CID, product ID, firmware version).
 */
Aliro::ReaderDescriptor GetReaderDescriptor();

} // namespace DoorLock
