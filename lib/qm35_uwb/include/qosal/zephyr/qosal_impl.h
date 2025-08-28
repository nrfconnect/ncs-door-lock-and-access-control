/*
 * SPDX-FileCopyrightText: Copyright (c) 2022 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Zephyr version file. */
#include <version.h>

#ifdef CONFIG_THREAD_NAME
#define QOSAL_IMPL_THREAD_MAX_NAME_LEN CONFIG_THREAD_MAX_NAME_LEN
#else
#define QOSAL_IMPL_THREAD_MAX_NAME_LEN 0
#endif

#define QOSAL_IMPL_THREAD_STACK_DEFINE(name, stack_size) K_THREAD_STACK_DEFINE(name, stack_size)

#if defined(CONFIG_LOG) && !defined(CONFIG_LOG_MODE_MINIMAL)

#ifdef CONFIG_LOG_RUNTIME_FILTERING
/* Use dynamic log_source. */
#define DECLARE_EXT_LOG_MODULE(module) \
	extern const struct log_source_dynamic_data log_dynamic_##module
#define QOSAL_IMPL_LOG_MODULE(module) &log_dynamic_##module
#else
/* Use const log_source. */
#define DECLARE_EXT_LOG_MODULE(module) extern const struct log_source_const_data log_const_##module
#define QOSAL_IMPL_LOG_MODULE(module) &log_const_##module
#endif

#if (KERNEL_VERSION_MAJOR >= 4) || (KERNEL_VERSION_MAJOR >= 3) && (KERNEL_VERSION_MINOR >= 6)
#define CONFIG_LOG_DOMAIN_ID Z_LOG_LOCAL_DOMAIN_ID
#endif

/*
 * The reason of redefining the log function inside QOSAL_IMPL_PRINT_LEVEL_MOD
 * is due to the zephyr log system trying to compute at runtime log containing
 * "const char * " strings.
 */

#if (KERNEL_VERSION_MAJOR >= 4) || (KERNEL_VERSION_MAJOR >= 3) && (KERNEL_VERSION_MINOR >= 4)

/* Redefine Z_LOG_MSG_SIMPLE_CREATE by just forcing the flag
 * CBPRINTF_PACKAGE_CONST_CHAR_RO */
#define Q_LOG_MSG_SIMPLE_CREATE(_cstr_cnt, _domain_id, _source, _level, ...)                      \
	do {                                                                                      \
		int _plen;                                                                        \
		CBPRINTF_STATIC_PACKAGE(NULL, 0, _plen, Z_LOG_MSG_ALIGN_OFFSET,                   \
					CBPRINTF_PACKAGE_CONST_CHAR_RO |                          \
						Z_LOG_MSG_CBPRINTF_FLAGS(_cstr_cnt),              \
					__VA_ARGS__);                                             \
		size_t _msg_wlen = Z_LOG_MSG_ALIGNED_WLEN(_plen, 0);                              \
		struct log_msg *_msg = z_log_msg_alloc(_msg_wlen);                                \
		struct log_msg_desc _desc =                                                       \
			Z_LOG_MSG_DESC_INITIALIZER(_domain_id, _level, (uint32_t)_plen, 0);       \
		LOG_MSG_DBG("creating message zero copy: package len: %d, msg: %p\n", _plen,      \
			    _msg);                                                                \
		if (_msg) {                                                                       \
			CBPRINTF_STATIC_PACKAGE(_msg->data, _plen, _plen, Z_LOG_MSG_ALIGN_OFFSET, \
						CBPRINTF_PACKAGE_CONST_CHAR_RO |                  \
							Z_LOG_MSG_CBPRINTF_FLAGS(_cstr_cnt),      \
						__VA_ARGS__);                                     \
		}                                                                                 \
		z_log_msg_finalize(_msg, (void *)_source, _desc, NULL);                           \
	} while (0)

/*
 * Ideally the whole macro should be replaced by Z_LOG_MSG_CREATE with
 * CONFIG_LOG_SPEED and CONFIG_LOG2_MSG_PKG_ALWAYS_ADD_RO_STRING_IDXS, however,
 * as the latter also forces the copy of RO strings in the log, it can increase
 * drastically the ROM use for the sake of the performance.
 */
#define QOSAL_IMPL_PRINT_LEVEL_MOD(level, module, ...)                                            \
	DECLARE_EXT_LOG_MODULE(module);                                                           \
	do {                                                                                      \
		if (!IS_ENABLED(CONFIG_LOG_SPEED) ||                                              \
		    CBPRINTF_MUST_RUNTIME_PACKAGE(CBPRINTF_PACKAGE_CONST_CHAR_RO |                \
							  Z_LOG_MSG_CBPRINTF_FLAGS(0),            \
						  __VA_ARGS__)) {                                 \
			Z_LOG_MSG_STACK_CREATE(0, CONFIG_LOG_DOMAIN_ID,                           \
					       QOSAL_IMPL_LOG_MODULE(module), LOG_LEVEL_##level,  \
					       NULL, 0, __VA_ARGS__);                             \
		} else {                                                                          \
			Q_LOG_MSG_SIMPLE_CREATE(0, CONFIG_LOG_DOMAIN_ID,                          \
						QOSAL_IMPL_LOG_MODULE(module), LOG_LEVEL_##level, \
						__VA_ARGS__);                                     \
		}                                                                                 \
	} while (0)

#elif (KERNEL_VERSION_MAJOR >= 3) && (KERNEL_VERSION_MINOR >= 1)

/* Redefine Z_LOG_MSG2_SIMPLE_CREATE by just forcing the flag
 * CBPRINTF_PACKAGE_CONST_CHAR_RO */
#define Q_LOG_MSG2_SIMPLE_CREATE(_cstr_cnt, _domain_id, _source, _level, ...)                      \
	do {                                                                                       \
		int _plen;                                                                         \
		CBPRINTF_STATIC_PACKAGE(NULL, 0, _plen, Z_LOG_MSG2_ALIGN_OFFSET,                   \
					CBPRINTF_PACKAGE_CONST_CHAR_RO |                           \
						Z_LOG_MSG2_CBPRINTF_FLAGS(_cstr_cnt),              \
					__VA_ARGS__);                                              \
		size_t _msg_wlen = Z_LOG_MSG2_ALIGNED_WLEN(_plen, 0);                              \
		struct log_msg2 *_msg = z_log_msg2_alloc(_msg_wlen);                               \
		struct log_msg2_desc _desc =                                                       \
			Z_LOG_MSG_DESC_INITIALIZER(_domain_id, _level, (uint32_t)_plen, 0);        \
		LOG_MSG2_DBG("creating message zero copy: package len: %d, msg: %p\n", _plen,      \
			     _msg);                                                                \
		if (_msg) {                                                                        \
			CBPRINTF_STATIC_PACKAGE(_msg->data, _plen, _plen, Z_LOG_MSG2_ALIGN_OFFSET, \
						CBPRINTF_PACKAGE_CONST_CHAR_RO |                   \
							Z_LOG_MSG2_CBPRINTF_FLAGS(_cstr_cnt),      \
						__VA_ARGS__);                                      \
		}                                                                                  \
		z_log_msg2_finalize(_msg, (void *)_source, _desc, NULL);                           \
	} while (0)

/*
 * Ideally the whole macro should be replaced by Z_LOG_MSG2_CREATE with
 * CONFIG_LOG_SPEED and CONFIG_LOG2_MSG_PKG_ALWAYS_ADD_RO_STRING_IDXS, however,
 * as the latter also forces the copy of RO strings in the log, it can increase
 * drastically the ROM use for the sake of the performance.
 */
#define QOSAL_IMPL_PRINT_LEVEL_MOD(level, module, ...)                                             \
	DECLARE_EXT_LOG_MODULE(module);                                                            \
	do {                                                                                       \
		if (!IS_ENABLED(CONFIG_LOG_SPEED) ||                                               \
		    CBPRINTF_MUST_RUNTIME_PACKAGE(CBPRINTF_PACKAGE_CONST_CHAR_RO |                 \
							  Z_LOG_MSG2_CBPRINTF_FLAGS(0),            \
						  __VA_ARGS__)) {                                  \
			Z_LOG_MSG2_STACK_CREATE(0, CONFIG_LOG_DOMAIN_ID,                           \
						QOSAL_IMPL_LOG_MODULE(module), LOG_LEVEL_##level,  \
						NULL, 0, __VA_ARGS__);                             \
		} else {                                                                           \
			Q_LOG_MSG2_SIMPLE_CREATE(0, CONFIG_LOG_DOMAIN_ID,                          \
						 QOSAL_IMPL_LOG_MODULE(module), LOG_LEVEL_##level, \
						 __VA_ARGS__);                                     \
		}                                                                                  \
	} while (0)

#else /* Zephyr version < 3.1. */

#define QOSAL_IMPL_PRINT_LEVEL_MOD(level, module, ...)                                             \
	DECLARE_EXT_LOG_MODULE(module);                                                            \
	do {                                                                                       \
		if (!IS_ENABLED(CONFIG_LOG_SPEED) ||                                               \
		    CBPRINTF_MUST_RUNTIME_PACKAGE(0, CBPRINTF_MUST_RUNTIME_PACKAGE_CONST_CHAR,     \
						  __VA_ARGS__)) {                                  \
			z_log_msg2_runtime_create(CONFIG_LOG_DOMAIN_ID,                            \
						  QOSAL_IMPL_LOG_MODULE(module),                   \
						  LOG_LEVEL_##level, NULL, 0, __VA_ARGS__);        \
		} else {                                                                           \
			Z_LOG_MSG2_SIMPLE_CREATE(CONFIG_LOG_DOMAIN_ID,                             \
						 QOSAL_IMPL_LOG_MODULE(module), LOG_LEVEL_##level, \
						 __VA_ARGS__);                                     \
		}                                                                                  \
	} while (false)

#endif /* Zephyr version. */

#define QOSAL_IMPL_PRINT_LEVEL(level, ...) QOSAL_IMPL_PRINT_LEVEL_MOD(level, qlog, __VA_ARGS__)

#define QOSAL_IMPL_PRINT_FORMAT(level, tag, fmt, ...)                                \
	do {                                                                         \
		if (tag && *tag != '\0') {                                           \
			QOSAL_IMPL_PRINT_LEVEL(level, tag ":\t" fmt, ##__VA_ARGS__); \
		} else {                                                             \
			QOSAL_IMPL_PRINT_LEVEL(level, fmt, ##__VA_ARGS__);           \
		}                                                                    \
	} while (0)

#else
#define QOSAL_IMPL_PRINT_LEVEL_MOD(...)
#define QOSAL_IMPL_PRINT_FORMAT(...)
#endif

#define QOSAL_IMPL_LOG_INFO(...) QOSAL_IMPL_PRINT_FORMAT(INF, LOG_TAG, __VA_ARGS__)
#define QOSAL_IMPL_LOG_ERR(...) QOSAL_IMPL_PRINT_FORMAT(ERR, LOG_TAG, __VA_ARGS__)
#define QOSAL_IMPL_LOG_WARN(...) QOSAL_IMPL_PRINT_FORMAT(WRN, LOG_TAG, __VA_ARGS__)
#define QOSAL_IMPL_LOG_DEBUG(...) QOSAL_IMPL_PRINT_FORMAT(DBG, LOG_TAG, __VA_ARGS__)

#define QOSAL_IMPL_IRQ_CONNECT(irqn, prio, handler) IRQ_CONNECT(irqn, prio, handler, NULL, 0)

/* The following functions are valid on all ARM Cortex-M. */
#define QOSAL_IMPL_IRQ_ENABLE(irqn) NVIC_EnableIRQ(irqn)
#define QOSAL_IMPL_IRQ_DISABLE(irqn) NVIC_DisableIRQ(irqn)
#define QOSAL_IMPL_IRQ_CLEAR_PENDING(irqn) NVIC_ClearPendingIRQ(irqn)
#define QOSAL_IMPL_IRQ_GET(irqn) NVIC_GetEnableIRQ(irqn)

#ifdef CONFIG_UWB_ZEPHYR_SPECIFIC
#define QOSAL_PRINT_TRACE(...) QOSAL_IMPL_PRINT_LEVEL_MOD(INF, uwbstack, __VA_ARGS__)
#elif !defined(CONFIG_LOG)
#define QOSAL_PRINT_TRACE(...)
#endif /* CONFIG_UWB_ZEPHYR_SPECIFIC & CONFIG_LOG */
