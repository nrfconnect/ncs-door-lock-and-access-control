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
 * The fields match the Matter Basic Information cluster: Vendor ID, Product ID, and SoftwareVersionString.
 *
 * @return Reader Descriptor fields for this product (vendor ID, product ID, firmware version).
 */
Aliro::ReaderDescriptor GetReaderDescriptor();

} // namespace DoorLock
