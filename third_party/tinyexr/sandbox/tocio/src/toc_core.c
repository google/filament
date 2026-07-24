/*
 * tocio - allocator helpers, bump arena, op-list builder, result strings.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

/* ============================================================================
 * Default allocator (hosted: malloc/free; freestanding: none -> caller must
 * supply one). The malloc branch is #ifdef'd out so the freestanding gate's
 * symbol scan never sees malloc/free.
 * ========================================================================== */
#ifndef TOC_FREESTANDING
#include <stdlib.h>
static void *toc_sys_alloc(void *user, size_t size) {
    (void)user;
    return malloc(size ? size : 1);
}
static void toc_sys_free(void *user, void *ptr) {
    (void)user;
    free(ptr);
}
static const toc_allocator g_default = {NULL, toc_sys_alloc, toc_sys_free};
#else
static const toc_allocator g_default = {NULL, NULL, NULL};
#endif

const toc_allocator *toc_default_allocator(void) { return &g_default; }

void *toc_malloc(const toc_allocator *a, size_t size) {
    if (!a) a = &g_default;
    if (!a->alloc) return NULL;
    return a->alloc(a->user, size ? size : 1);
}
void *toc_calloc(const toc_allocator *a, size_t count, size_t size) {
    size_t total;
    void *p;
    if (toc_mul_ovf(count, size, &total)) return NULL;
    p = toc_malloc(a, total ? total : 1);
    if (p) memset(p, 0, total ? total : 1);
    return p;
}
void toc_free(const toc_allocator *a, void *ptr) {
    if (!a) a = &g_default;
    if (ptr && a->free) a->free(a->user, ptr);
}
char *toc_strndup(const toc_allocator *a, const char *s, size_t n) {
    char *p = (char *)toc_malloc(a, n + 1);
    if (!p) return NULL;
    memcpy(p, s, n);
    p[n] = 0;
    return p;
}

const char *toc_result_string(toc_result r) {
    switch (r) {
        case TOC_SUCCESS: return "success";
        case TOC_ERROR_INVALID_ARGUMENT: return "invalid argument";
        case TOC_ERROR_PARSE: return "parse error";
        case TOC_ERROR_UNSUPPORTED: return "unsupported feature";
        case TOC_ERROR_OUT_OF_MEMORY: return "out of memory";
        case TOC_ERROR_IO: return "I/O error";
        case TOC_ERROR_NOT_FOUND: return "not found";
        case TOC_ERROR_NONINVERTIBLE: return "non-invertible transform";
    }
    return "unknown error";
}

/* ============================================================================
 * Bump arena
 * ========================================================================== */
#define TOC_ARENA_MIN 8192u

void toc_arena_init(toc_arena *ar, const toc_allocator *a) {
    ar->alloc = a ? *a : g_default;
    ar->head = NULL;
}

void *toc_arena_alloc(toc_arena *ar, size_t size) {
    toc_arena_block *b = ar->head;
    size_t aligned = (size + 15u) & ~(size_t)15u;
    if (!b || b->used + aligned > b->cap) {
        size_t cap = aligned > TOC_ARENA_MIN ? aligned : TOC_ARENA_MIN;
        size_t total;
        if (toc_mul_ovf(1, cap, &total)) return NULL;
        b = (toc_arena_block *)toc_malloc(&ar->alloc,
                                          sizeof(toc_arena_block) + cap);
        if (!b) return NULL;
        b->next = ar->head;
        b->used = 0;
        b->cap = cap;
        ar->head = b;
    }
    {
        char *p = (char *)(b + 1) + b->used;
        b->used += aligned;
        return p;
    }
}

char *toc_arena_strndup(toc_arena *ar, const char *s, size_t n) {
    char *p = (char *)toc_arena_alloc(ar, n + 1);
    if (!p) return NULL;
    memcpy(p, s, n);
    p[n] = 0;
    return p;
}

void toc_arena_free(toc_arena *ar) {
    toc_arena_block *b = ar->head;
    while (b) {
        toc_arena_block *next = b->next;
        toc_free(&ar->alloc, b);
        b = next;
    }
    ar->head = NULL;
}

/* ============================================================================
 * Op-list builder
 * ========================================================================== */
toc_op *toc_op_list_push(toc_op_list *list, toc_op_kind kind) {
    if (list->count == list->cap) {
        size_t ncap = list->cap ? list->cap * 2 : 8;
        size_t bytes;
        toc_op *n;
        if (toc_mul_ovf(ncap, sizeof(toc_op), &bytes)) return NULL;
        n = (toc_op *)toc_malloc(&list->alloc, bytes);
        if (!n) return NULL;
        if (list->ops) {
            memcpy(n, list->ops, list->count * sizeof(toc_op));
            toc_free(&list->alloc, list->ops);
        }
        list->ops = n;
        list->cap = ncap;
    }
    {
        toc_op *op = &list->ops[list->count++];
        memset(op, 0, sizeof(*op));
        op->kind = kind;
        return op;
    }
}

int toc_op_list_own(toc_op_list *list, float *data) {
    if (list->owned_count == list->owned_cap) {
        size_t ncap = list->owned_cap ? list->owned_cap * 2 : 4;
        size_t bytes;
        float **n;
        if (toc_mul_ovf(ncap, sizeof(float *), &bytes)) return 0;
        n = (float **)toc_malloc(&list->alloc, bytes);
        if (!n) return 0;
        if (list->owned) {
            memcpy(n, list->owned, list->owned_count * sizeof(float *));
            toc_free(&list->alloc, list->owned);
        }
        list->owned = n;
        list->owned_cap = ncap;
    }
    list->owned[list->owned_count++] = data;
    return 1;
}

void toc_op_list_free(toc_op_list *list) {
    const toc_allocator *a;
    size_t i;
    if (!list) return;
    a = &list->alloc;
    for (i = 0; i < list->owned_count; ++i) toc_free(a, list->owned[i]);
    toc_free(a, list->owned);
    toc_free(a, list->ops);
    {
        toc_allocator alloc = list->alloc;
        memset(list, 0, sizeof(*list));
        toc_free(&alloc, list);
    }
}
