/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace Aliro {

namespace NfcTransport {

using Byte = uint8_t;

struct Data {
	Byte *mData{ nullptr };
	size_t mLength{ 0 };
};

} // namespace NfcTransport

} // namespace Aliro
