/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "crypto/crypto_common.h"
#include "util/span.h"
#include "util/static_byte_span.h"

#include <zephyr/sys/util_macro.h>

namespace Aliro {

using VendorExtension = SharedByteSpan;
using CommandParameters = Byte;
using Signature = StaticByteSpan<kEccP256SignatureLength>;

} // namespace Aliro
