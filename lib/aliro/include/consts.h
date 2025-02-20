/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// Chapter 8.3.1.11: BLE 0xC3, NFC 0x5E
enum class InterfaceByte : uint8_t { NFC = 0x5E, BLE = 0xC3 };

namespace Aliro {

static constexpr uint8_t kProtocolVersionLength{ 2 };

static constexpr uint8_t kReaderGroupIdentifierLength{ 16 };
static constexpr uint8_t kReaderGroupSubIdentifierLength{ 16 };
static constexpr uint8_t kReaderIdentifierLength = kReaderGroupIdentifierLength + kReaderGroupSubIdentifierLength;
static constexpr uint8_t kTransactionIdentifierLength{ 16 };

static constexpr uint8_t kCommandParametersLength{ 1 };
static constexpr uint8_t kAuthenticationPolicyLength{ 1 };

static constexpr uint8_t kCryptogramLength{ 64 };
static constexpr uint8_t kVendorExtensionMaxLength{ 127 };
static constexpr uint8_t kKeySlotLength{ 8 };
static constexpr uint8_t kSignedTimestampLength{ 20 };
static constexpr uint8_t kSignalingBitmapLength{ 2 };

static constexpr size_t kAidLen{ 9 };
static constexpr size_t kApplicationTypeLen{ 2 };
static constexpr size_t kExtendedInfoLen{ 8 };
static constexpr size_t kMaxCmdLen{ 2 };
static constexpr size_t kMaxRespLen{ 2 };

// Aliro Spec. Table 10-3
static constexpr std::array<uint8_t, kAidLen> kExpPhase = { 0xA0, 0x00, 0x00, 0x09, 0x09, 0xAC, 0xCE, 0x55, 0x01 };
static constexpr std::array<uint8_t, kAidLen> kStepUpPhase = { 0xA0, 0x00, 0x00, 0x09, 0x09, 0xAC, 0xCE, 0x55, 0x02 };
// Aliro Spec. Table 10-4
static constexpr std::array<uint8_t, kApplicationTypeLen> kCsaApplication = { 0x00, 0x00 };

} // namespace Aliro
