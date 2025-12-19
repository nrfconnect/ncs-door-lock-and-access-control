/*
 * SPDX-FileCopyrightText: Copyright (c) 2022 Qorvo, Inc.
 * SPDX-License-Identifier: GPL-2.0 OR Apache-2.0
 */

#pragma once

#if defined(__KERNEL__)

#include <linux/byteorder/generic.h>

#define htobe16(x) cpu_to_be16(x)
#define htole16(x) cpu_to_le16(x)
#define htobe32(x) cpu_to_be32(x)
#define htole32(x) cpu_to_le32(x)
#define htobe64(x) cpu_to_be64(x)
#define htole64(x) cpu_to_le64(x)
#define be16toh(x) be16_to_cpu(x)
#define le16toh(x) le16_to_cpu(x)
#define be32toh(x) be32_to_cpu(x)
#define le32toh(x) le32_to_cpu(x)
#define be64toh(x) be64_to_cpu(x)
#define le64toh(x) le64_to_cpu(x)

#elif defined(__linux__)

#include <endian.h>

#elif defined(__BYTE_ORDER__)

#if defined(__GNUC__)

/* GCC/clang builtins. */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htobe16(x) __builtin_bswap16(x)
#define htole16(x) (x)
#define htobe32(x) __builtin_bswap32(x)
#define htole32(x) (x)
#define htobe64(x) __builtin_bswap64(x)
#define htole64(x) (x)
#define be16toh(x) __builtin_bswap16(x)
#define le16toh(x) (x)
#define be32toh(x) __builtin_bswap32(x)
#define le32toh(x) (x)
#define be64toh(x) __builtin_bswap64(x)
#define le64toh(x) (x)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htobe16(x) (x)
#define htole16(x) __builtin_bswap16(x)
#define htobe32(x) (x)
#define htole32(x) __builtin_bswap32(x)
#define htobe64(x) (x)
#define htole64(x) __builtin_bswap64(x)
#define be16toh(x) (x)
#define le16toh(x) __builtin_bswap16(x)
#define be32toh(x) (x)
#define le32toh(x) __builtin_bswap32(x)
#define be64toh(x) (x)
#define le64toh(x) __builtin_bswap64(x)
#else
#error "Unknown __BYTE_ORDER__ value."
#endif

#elif defined(_MSC_VER)

#include <stdlib.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define htobe16(x) _byteswap_ushort(x)
#define htole16(x) (x)
#define htobe32(x) _byteswap_ulong(x)
#define htole32(x) (x)
#define htobe64(x) _byteswap_uint64(x)
#define htole64(x) (x)
#define be16toh(x) _byteswap_ushort(x)
#define le16toh(x) (x)
#define be32toh(x) _byteswap_ulong(x)
#define le32toh(x) (x)
#define be64toh(x) _byteswap_uint64(x)
#define le64toh(x) (x)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define htobe16(x) (x)
#define htole16(x) _byteswap_ushort(x)
#define htobe32(x) (x)
#define htole32(x) _byteswap_ulong(x)
#define htobe64(x) (x)
#define htole64(x) _byteswap_uint64(x)
#define be16toh(x) (x)
#define le16toh(x) _byteswap_ushort(x)
#define be32toh(x) (x)
#define le32toh(x) _byteswap_ulong(x)
#define be64toh(x) (x)
#define le64toh(x) _byteswap_uint64(x)
#else
#error "Unknown __BYTE_ORDER__ value."
#endif

#else
#error "Unsupported compiler."
#endif

#else /* __BYTE_ORDER__ */
#error "__BYTE_ORDER__ not defined on this platform."
#endif /* __BYTE_ORDER__ */
