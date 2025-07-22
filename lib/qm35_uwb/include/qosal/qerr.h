/*
 * SPDX-FileCopyrightText: Copyright (c) 2023 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1 OR GPL-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * enum qerr - Return values for most QOSAL functions.
 * @QERR_SUCCESS: Operation successful.
 * @QERR_EADDRNOTAVAIL: Address not available.
 * @QERR_EAFNOSUPPORT: Address family not supported.
 * @QERR_EAGAIN: Resource temporarily unavailable.
 * @QERR_EBADF: Bad file descriptor.
 * @QERR_EBADMSG: Bad message.
 * @QERR_EBUSY: Device or resource busy.
 * @QERR_ECONNREFUSED: Connection refused.
 * @QERR_EEXIST: File exists.
 * @QERR_EFAULT: Bad address.
 * @QERR_EINTR: Interrupted system call.
 * @QERR_EINVAL: Invalid argument.
 * @QERR_EIO: I/O error.
 * @QERR_EMSGSIZE: Message too long.
 * @QERR_ENETDOWN: Network is down.
 * @QERR_ENOBUFS : No buffer space available.
 * @QERR_ENOENT: No such region or scheduler.
 * @QERR_ENOMEM: Not enough memory.
 * @QERR_ENOTSUP: Operation not supported.
 * @QERR_EPERM: Permission denied.
 * @QERR_EPIPE: Broken pipe.
 * @QERR_EPROTO: Protocol error.
 * @QERR_EPROTONOSUPPORT: Protocol not supported.
 * @QERR_ERANGE: Result too large.
 * @QERR_ETIME: Timer expired.
 * @QERR_ENODEV: No such device.
 * @QERR_ENOSPC: No space left.
 * @QERR_SE_EINVAL: Invalid arguments given to SE.
 * @QERR_SE_ENOKEY: No session key found for given id.
 * @QERR_SE_ENOSUBKEY: No sub-session key found for given id.
 * @QERR_SE_ERDSFETCHFAIL: Unexpected failure in SE while fetching keys.
 * @QERR_SE_ECANCEL: SE acknowledges cancellation of a request.
 */
enum qerr {
	/* Basic Qorvo errors. */
	QERR_SUCCESS = 0,
	QERR_EADDRNOTAVAIL = -1,
	QERR_EAFNOSUPPORT = -2,
	QERR_EAGAIN = -3,
	QERR_EBADF = -4,
	QERR_EBADMSG = -5,
	QERR_EBUSY = -6,
	QERR_ECONNREFUSED = -7,
	QERR_EEXIST = -8,
	QERR_EFAULT = -9,
	QERR_EINTR = -10,
	QERR_EINVAL = -11,
	QERR_EIO = -12,
	QERR_EMSGSIZE = -13,
	QERR_ENETDOWN = -14,
	QERR_ENOBUFS = -15,
	QERR_ENOENT = -16,
	QERR_ENOMEM = -17,
	QERR_ENOTSUP = -18,
	QERR_EPERM = -19,
	QERR_EPIPE = -20,
	QERR_EPROTO = -21,
	QERR_EPROTONOSUPPORT = -22,
	QERR_ERANGE = -23,
	QERR_ETIME = -24,
	QERR_ENODEV = -25,
	QERR_ENOSPC = -26,
	/* RFU: -27 to -159. */
	/* SE related Qorvo errors. */
	QERR_SE_EINVAL = -160,
	QERR_SE_ENOKEY = -161,
	QERR_SE_ENOSUBKEY = -162,
	QERR_SE_ERDSFETCHFAIL = -163,
	QERR_SE_ECANCEL = -164,
	/* RFU: -164 to -255. */
};

/**
 * qerr_convert_os_to_qerr() - Convert error from OS specific error to `qerr`.
 * @error: Error to be converted.
 *
 * Return: qerr converted from implementation-defined error.
 */
enum qerr qerr_convert_os_to_qerr(int error);

/**
 * qerr_convert_qerr_to_os() - Convert error from `qerr` to OS specific error.
 * @error: Error to be converted.
 *
 * Return: OS error converted from qerr error.
 */
int qerr_convert_qerr_to_os(enum qerr error);

/**
 * qerr_to_str() - Convert qerr to string.
 * Used for printing errors.
 *
 * @error: Error to be converted.
 *
 * Return: qerr converted to a string.
 */
const char *qerr_to_str(enum qerr error);

#ifdef __cplusplus
}
#endif
