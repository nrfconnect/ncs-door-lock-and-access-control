/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <qosal_impl.h>

#ifndef __KERNEL__
#include <qassert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#else
#include <linux/string.h>
#include <linux/types.h>
#define QASSERT(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define QUOTA_MAGIC 0xDEADDEAD

#ifndef CONFIG_MEM_QUOTA_ID1
#define CONFIG_MEM_QUOTA_ID1 8192
#endif

#ifndef CONFIG_MEM_QUOTA_ID2
#define CONFIG_MEM_QUOTA_ID2 4096
#endif

#ifndef CONFIG_MEM_QUOTA_RANGING_REPORT
#define CONFIG_MEM_QUOTA_RANGING_REPORT MEM_QUOTA_ID1
#endif

#ifndef CONFIG_MEM_QUOTA_PSDU_REPORT
#define CONFIG_MEM_QUOTA_PSDU_REPORT MEM_QUOTA_ID1
#endif

#ifndef CONFIG_MEM_QUOTA_UCI_REPORT
#define CONFIG_MEM_QUOTA_UCI_REPORT MEM_QUOTA_ID2
#endif

enum mem_quota_id {
	MEM_QUOTA_ID_INFINITE,
#ifdef CONFIG_MEM_QUOTA_ID1
	MEM_QUOTA_ID1,
#endif
#ifdef CONFIG_MEM_QUOTA_ID2
	MEM_QUOTA_ID2,
#endif
#ifdef CONFIG_MEM_QUOTA_ID3
	MEM_QUOTA_ID3,
#endif
#ifdef CONFIG_MEM_QUOTA_ID4
	MEM_QUOTA_ID4,
#endif
	MEM_QUOTA_ID_MAX
};
#if !defined(_GCC_MAX_ALIGN_T) && !defined(__CLANG_MAX_ALIGN_T_DEFINED) && \
	!defined(__DEFINED_max_align_t)
#if defined(__ZEPHYR__)
#include <zephyr/types.h>
typedef z_max_align_t max_align_t;
#else
typedef struct {
	long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
	long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
#endif
#endif /* _GCC_MAX_ALIGN_T && __CLANG_MAX_ALIGN_T_DEFINED */

struct quota_alloc_prefix {
	union {
		struct {
			enum mem_quota_id quota_id;
			uint16_t size;
			uint32_t magic;
		};
		char align[__alignof__(max_align_t)];
	};
};

#define M2Q(m) (struct quota_alloc_prefix *)((uintptr_t)(m) - sizeof(struct quota_alloc_prefix))
#define Q2M(q) (void *)((uint8_t *)(q) + sizeof(struct quota_alloc_prefix))

extern uint32_t allocation_quotas[];

/**
 * qmalloc_internal() - Allocate memory.
 * @size: Number of bytes to allocate.
 *
 * Return: Pointer to the allocated memory.
 */
void *qmalloc_internal(size_t size);

/**
 * qrealloc_internal() - Attempts to resize the memory.
 * @ptr: Pointer to previously allocated memory.
 * @new_size: New size for the memory, in bytes.
 *
 * Return: Pointer to the reallocated memory.
 */
void *qrealloc_internal(void *ptr, size_t new_size);

/**
 * qfree_internal() - Free memory allocated with qmalloc().
 * @ptr: Pointer to previously allocated memory.
 */
void qfree_internal(void *ptr);

/**
 * qmalloc() - Allocate memory.
 * @size: Number of bytes to allocate.
 *
 * Return: Pointer to the allocated memory.
 */
static inline void *qmalloc(size_t size)
{
	struct quota_alloc_prefix *q = NULL;
	void *res = NULL;
	if (!size)
		return res;

	q = (struct quota_alloc_prefix *)qmalloc_internal(size + sizeof(struct quota_alloc_prefix));
	if (q) {
		q->size = size;
		q->quota_id = MEM_QUOTA_ID_INFINITE;
		q->magic = QUOTA_MAGIC;
		allocation_quotas[MEM_QUOTA_ID_INFINITE] -= size;
		res = Q2M(q);
	}
	return res;
}

/**
 * qcalloc() - Allocate memory and set it to 0.
 * @nb_items: Number of items to allocate.
 * @item_size: Size of an item.
 *
 * Return: Pointer to the allocated memory.
 */
static inline void *qcalloc(size_t nb_items, size_t item_size)
{
	struct quota_alloc_prefix *q = NULL;
	size_t size = nb_items * item_size;
	void *res = NULL;
	if (!size)
		return res;

	q = (struct quota_alloc_prefix *)qmalloc_internal(size + sizeof(struct quota_alloc_prefix));
	if (q) {
		res = Q2M(q);
		memset(res, 0, size);
		q->size = size;
		q->quota_id = MEM_QUOTA_ID_INFINITE;
		q->magic = QUOTA_MAGIC;
		allocation_quotas[MEM_QUOTA_ID_INFINITE] -= size;
	}
	return res;
}

/**
 * qrealloc() - Attempts to resize the memory.
 * @ptr: Pointer to previously allocated memory.
 * @new_size: New size for the memory, in bytes.
 *
 * Return: Pointer to the reallocated memory.
 */

static inline void *qrealloc(void *ptr, size_t new_size)
{
	struct quota_alloc_prefix *q = ptr ? M2Q(ptr) : NULL, *newq = NULL;
	uint16_t old_size = ptr ? q->size : 0;
	enum mem_quota_id quota_id = q ? q->quota_id : MEM_QUOTA_ID_INFINITE;
	void *res = NULL;
	int sdiff = new_size - old_size;

	if (new_size == 0) {
		/* free old alloc */
		qfree_internal(q);
		return res;
	}

	QASSERT(quota_id < MEM_QUOTA_ID_MAX);
	QASSERT(!q || (q->magic == QUOTA_MAGIC));
	if ((sdiff < 0) || (allocation_quotas[quota_id] >= (size_t)sdiff)) {
		newq = (struct quota_alloc_prefix *)qrealloc_internal(
			q, new_size + sizeof(struct quota_alloc_prefix));
		if (newq) {
			newq->size = new_size;
			newq->quota_id = quota_id;
			newq->magic = QUOTA_MAGIC;
			allocation_quotas[quota_id] -= sdiff;
			res = Q2M(newq);
		} else {
			/* memory freed */
			allocation_quotas[quota_id] += old_size;
		}
	} else if (q) {
		/* Not enough space, free the orignal buffer */
		qfree_internal(q);
		allocation_quotas[quota_id] += old_size;
	}
	return res;
}

/**
 * qmalloc_quota() - Allocate memory.
 * @size: Number of bytes to allocate.
 * @qid: Quota to use for this allocation.
 *
 * Return: Pointer to the allocated memory.
 */
static inline void *qmalloc_quota(size_t size, enum mem_quota_id qid)
{
	struct quota_alloc_prefix *q = NULL;
	void *res = NULL;
	if (!size)
		return res;

	QASSERT(qid < MEM_QUOTA_ID_MAX);
	if (allocation_quotas[qid] >= size) {
		q = (struct quota_alloc_prefix *)qmalloc_internal(
			size + sizeof(struct quota_alloc_prefix));
		if (q) {
			q->size = size;
			q->quota_id = qid;
			q->magic = QUOTA_MAGIC;
			allocation_quotas[qid] -= size;
			res = Q2M(q);
		}
	}
	return res;
}

/**
 * qcalloc_quota() - Allocate memory and set it to 0.
 * @nb_items: Number of items to allocate.
 * @item_size: Size of an item.
 * @qid: Quota to use for this allocation.
 *
 * Return: Pointer to the allocated memory.
 */
static inline void *qcalloc_quota(size_t nb_items, size_t item_size, enum mem_quota_id qid)
{
	struct quota_alloc_prefix *q = NULL;
	size_t size = nb_items * item_size;
	void *res = NULL;
	if (!size)
		return res;

	QASSERT(qid < MEM_QUOTA_ID_MAX);
	if (allocation_quotas[qid] >= size) {
		q = (struct quota_alloc_prefix *)qmalloc_internal(
			size + sizeof(struct quota_alloc_prefix));
		if (q) {
			memset(Q2M(q), 0, size);
			q->size = size;
			q->quota_id = qid;
			q->magic = QUOTA_MAGIC;
			allocation_quotas[qid] -= size;
			res = Q2M(q);
		}
	}
	return res;
}

/**
 * qfree() - Free memory allocated with qmalloc().
 * @ptr: Pointer to previously allocated memory.
 */
static inline void qfree(void *ptr)
{
	struct quota_alloc_prefix *q = ptr ? M2Q(ptr) : NULL;
	if (q) {
		enum mem_quota_id quota_id = q->quota_id;
		QASSERT(q->magic == QUOTA_MAGIC);
		QASSERT(quota_id < MEM_QUOTA_ID_MAX);
		allocation_quotas[quota_id] += q->size;
	}
	qfree_internal(q);
}

#ifdef __cplusplus
}
#endif
