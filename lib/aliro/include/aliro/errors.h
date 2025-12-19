/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <cstdint>

enum AliroErrorCode : uint8_t {
	ALIRO_NO_ERROR,
	ALIRO_NO_MEMORY,
	ALIRO_ERROR_INTERNAL,
	ALIRO_INVALID_STATE,
	ALIRO_INVALID_ARGUMENT,
	ALIRO_INVALID_SIGNATURE,
	ALIRO_INVALID_AUTHENTICATION_TAG,
	ALIRO_INVALID_PUBLIC_KEY,
	ALIRO_INVALID_ACCESS_DOCUMENT,
	ALIRO_PUBLIC_KEY_NOT_FOUND,
	ALIRO_KEY_ALREADY_EXISTS,
	ALIRO_TIMEOUT,
	ALIRO_ERROR_NOT_IMPLEMENTED,
	ALIRO_TLV_INVALID_TAG,
	ALIRO_TLV_INVALID_LEN,
	ALIRO_TLV_BUFFER_TOO_SMALL,
	ALIRO_TLV_WRONG_DATA_TYPE,
	ALIRO_TLV_END_OF_TLV,
	ALIRO_UWB_INIT_FAILED,
	ALIRO_UWB_DFU_FAILED,
	ALIRO_ERROR_UNKNOWN,
	ALIRO_SESSION_NOT_FOUND,
	ALIRO_SESSION_TERMINATE,
	ALIRO_VERSION_NOT_SUPPORTED,
	ALIRO_APDU_STATUS_INVALID,
	ALIRO_ENCRYPTION_COUNTER_OVERFLOW,
	ALIRO_DECRYPTION_COUNTER_OVERFLOW,
	ALIRO_ERROR_MAX,
};

/**
 * @brief Error code wrapper class.
 */
class AliroError {
public:
	AliroError() = default;
	constexpr AliroError(AliroErrorCode code) : mCode(code) {}

	friend bool operator==(AliroError lhs, AliroError rhs) { return lhs.ToErrorCode() == rhs.ToErrorCode(); }
	friend bool operator!=(AliroError lhs, AliroErrorCode rhs) { return lhs.ToErrorCode() != rhs; }
	friend bool operator!=(AliroErrorCode lhs, AliroError rhs) { return lhs != rhs.ToErrorCode(); }

	/**
	 * @brief Convert to error code.
	 *
	 * @return Error code.
	 */
	AliroErrorCode ToErrorCode() const { return mCode; }

	/**
	 * @brief Convert to undelying integer representation.
	 *
	 * This method may be a remedy in the case of implicit conversion warnings
	 * from the compiler.
	 *
	 * @return Underlying integer number error code
	 */
	int ToInt() const { return static_cast<int>(mCode); }

	/**
	 * @brief Convert to string.
	 *
	 * This method may come in handy when printing the error messages.
	 *
	 * @return Corresponding string error message
	 */
	const char *ToString() const;

	/**
	 * @brief Convert from integer error code.
	 *
	 * This method can be used as a default mapping in the code that doesn't implement
	 * custom conversion from integer error codes to the AliroError.
	 *
	 * @return Corresponding AliroError
	 */
	static AliroError FromInt(int ec);

private:
	AliroErrorCode mCode{ ALIRO_NO_ERROR };
};
