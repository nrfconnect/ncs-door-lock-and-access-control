/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

#include <aliro_uwb_adapter/aliro_uwb_session.h>
#include <qmalloc.h>

namespace Aliro::Uwb {

/**
 * @brief Wrapper class for aliro_uwb_message.
 *
 * This class is used to wrap the aliro_uwb_message struct and provide a more convenient interface for
 * interacting with the UWB message.
 */
struct Message {
	/**
	 * @brief Constructor.
	 * Allocates memory for the message and initializes the message structure.
	 *
	 * @param dataLength The length of the data to allocate for the message.
	 */
	explicit Message(size_t dataLength)
		: mMessage(static_cast<aliro_uwb_message *>(qmalloc(sizeof(aliro_uwb_message) + dataLength)))
	{
	}

	/**
	 * @brief Destructor.
	 * Releases the allocated memory for the message.
	 */
	~Message()
	{
		if (mMessage) {
			qfree(mMessage);
		}
	}

	/**
	 * @brief Get the message pointer.
	 *
	 * @return The message pointer.
	 */
	aliro_uwb_message *get() const { return mMessage; }

	/**
	 * @brief Get the message pointer.
	 *
	 * @return The message pointer.
	 */
	aliro_uwb_message *operator->() const { return mMessage; }

	/**
	 * @brief Check if the message is valid.
	 *
	 * @return True if the memory is allocated for the message, false otherwise.
	 */
	explicit operator bool() const { return mMessage != nullptr; }

	// Delete copy constructor and assignment.
	Message(const Message &) = delete;
	Message &operator=(const Message &) = delete;

	// Delete move constructor and assignment.
	Message(const Message &&) = delete;
	Message &operator=(const Message &&) = delete;

private:
	aliro_uwb_message *mMessage{ nullptr };
};

} // namespace Aliro::Uwb
