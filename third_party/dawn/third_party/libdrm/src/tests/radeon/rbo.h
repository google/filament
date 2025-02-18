/*
 * Copyright © 2011 Red Hat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Jerome Glisse <j.glisse@gmail.com>
 */
#ifndef RBO_H
#define RBO_H

#include "util_double_list.h"

struct rbo {
    struct list_head    list;
    int                 fd;
    unsigned            refcount;
    unsigned            mapcount;
    unsigned            handle;
    unsigned            size;
    unsigned            alignment;
    void                *data;
};

struct rbo *rbo(int fd, unsigned handle, unsigned size,
                unsigned alignment, void *ptr);
int rbo_map(struct rbo *bo);
void rbo_unmap(struct rbo *bo);
struct rbo *rbo_incref(struct rbo *bo);
struct rbo *rbo_decref(struct rbo *bo);
int rbo_wait(struct rbo *bo);

#endif
