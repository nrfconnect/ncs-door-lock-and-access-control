/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include "consts.h"
#include "util/span.h"
#include "util/utils.h"

namespace Aliro {

/**
 * @struct CONTROL_FLOWCommand
 *
 * @brief Provides methods for serializing CONTROL_FLOWCommand command to the TLV format according to the Aliro Spec.
 * Table 10-6.
 */
struct CONTROL_FLOWCommand {
	/**
	 * @enum S1Parameter.
	 * @brief Represents the S1 parameter values.
	 */
	enum class S1Parameter : Byte { TransactionFail = 0x00 };

	/**
	 * @enum S2Parameter.
	 * @brief Represents the S2 parameter values.
	 */
	enum class S2Parameter : Byte { NoInformation = 0x00, VersionNotSupported = 0x27 };

	/**
	 * @brief Serializes data to the TLV data format.
	 *
	 * @return When success a SharedByteSpan object containing the TLV payload, empty SharedByteSpan otherwise.
	 */
	SharedByteSpan Serialize() const;

	/**
	 * @brief Sets the S1 and S2 parameters based on S1Parameter and S2Parameter.
	 *
	 * @param s1Parameter [input] the S1 parameter.
	 * @param s2Parameter [input] the S2 parameter.
	 */
	void SetParameters(const S1Parameter &s1Parameter, const S2Parameter &s2Parameter);

private:
	Byte mS1Parameter{};
	Byte mS2Parameter{};
};

} // namespace Aliro
