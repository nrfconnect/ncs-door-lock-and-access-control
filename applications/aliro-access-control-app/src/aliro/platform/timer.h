/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
 */

#pragma once

#include <cstdint>

#include <zephyr/kernel.h>

namespace Aliro {

/**
 * @brief Timer class for managing delayed execution of callbacks
 *
 * The Timer class provides a high-level interface for scheduling callback functions
 * to be executed after a specified timeout period.
 *
 * @note This class is not copyable or movable to prevent resource management issues.
 * @note The timer callback is executed in the context of a work queue, not in an ISR.
 */
class Timer {
public:
	/** @brief Context type passed to timer callbacks */
	using Context = void *;

	/** @brief Callback function type for timer expiration */
	using Callback = void (*)(Context context);

	/**
	 * @brief Construct a new Timer instance
	 *
	 * @param timeoutMs The timeout period in milliseconds
	 * @param callback Function to call when the timer expires
	 * @param context User-defined context passed to the callback function
	 *
	 * @note The timer is not automatically started upon construction.
	 *       Call Start() to begin the countdown.
	 */
	explicit Timer(uint32_t timeoutMs, Callback callback, Context context);

	/**
	 * @brief Destructor
	 *
	 * Stops the timer and waits for the work to be cancelled before destroying the timer.
	 */
	~Timer();

	/**
	 * @brief Start the timer countdown
	 *
	 * Begins the countdown for the specified timeout period. When the timeout
	 * expires, the callback function will be executed with the provided context.
	 */
	void Start();

	/**
	 * @brief Restart the timer countdown
	 *
	 * Resets the timer to begin a new countdown with the same timeout period.
	 */
	void Restart();

	/**
	 * @brief Check if the timer is running
	 *
	 * @return true if the timer is running, false otherwise
	 */
	bool IsRunning();

	/**
	 * @brief Stop the timer countdown
	 *
	 * Cancels the current timer countdown. If the timer was not running,
	 * this call has no effect. The callback will not be executed.
	 */
	void Stop();

private:
	Timer(const Timer &) = delete;
	Timer &operator=(const Timer &) = delete;
	Timer(Timer &&) = delete;
	Timer &operator=(Timer &&) = delete;

	k_timer mTimer;
	k_work mWork;
	const k_timeout_t mTimeout;
	Callback mCallback;
	Context mContext;
};

} // namespace Aliro
