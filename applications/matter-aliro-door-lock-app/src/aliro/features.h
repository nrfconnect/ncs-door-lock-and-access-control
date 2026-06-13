/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <zephyr/sys/util.h>

namespace Aliro {

/**
 * @brief Aliro application feature bitmap bit positions.
 *
 */
static constexpr uint8_t kFeatureCredentialIssuerCaPublicKeySupported = static_cast<uint8_t>(BIT(0));
static constexpr uint8_t kFeatureReaderCertificateSupported = static_cast<uint8_t>(BIT(1));
static constexpr uint8_t kFeatureMatterSupported = static_cast<uint8_t>(BIT(2));

} // namespace Aliro
