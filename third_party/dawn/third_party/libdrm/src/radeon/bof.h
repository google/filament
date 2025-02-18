/*
 * Copyright 2010 Jerome Glisse <glisse@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *      Jerome Glisse
 */
#ifndef BOF_H
#define BOF_H

#include <stdio.h>
#include <stdint.h>

#define BOF_TYPE_STRING		0
#define BOF_TYPE_NULL		1
#define BOF_TYPE_BLOB		2
#define BOF_TYPE_OBJECT		3
#define BOF_TYPE_ARRAY		4
#define BOF_TYPE_INT32		5

struct bof;

typedef struct bof {
	struct bof	**array;
	unsigned	centry;
	unsigned	nentry;
	unsigned	refcount;
	FILE		*file;
	uint32_t	type;
	uint32_t	size;
	uint32_t	array_size;
	void		*value;
	long		offset;
} bof_t;

extern int bof_file_flush(bof_t *root);
extern bof_t *bof_file_new(const char *filename);
extern int bof_object_dump(bof_t *object, const char *filename);

/* object */
extern bof_t *bof_object(void);
extern bof_t *bof_object_get(bof_t *object, const char *keyname);
extern int bof_object_set(bof_t *object, const char *keyname, bof_t *value);
/* array */
extern bof_t *bof_array(void);
extern int bof_array_append(bof_t *array, bof_t *value);
extern bof_t *bof_array_get(bof_t *bof, unsigned i);
extern unsigned bof_array_size(bof_t *bof);
/* blob */
extern bof_t *bof_blob(unsigned size, void *value);
extern unsigned bof_blob_size(bof_t *bof);
extern void *bof_blob_value(bof_t *bof);
/* string */
extern bof_t *bof_string(const char *value);
/* int32 */
extern bof_t *bof_int32(int32_t value);
extern int32_t bof_int32_value(bof_t *bof);
/* common functions */
extern void bof_decref(bof_t *bof);
extern void bof_incref(bof_t *bof);
extern bof_t *bof_load_file(const char *filename);
extern int bof_dump_file(bof_t *bof, const char *filename);
extern void bof_print(bof_t *bof);

static inline int bof_is_object(bof_t *bof){return (bof->type == BOF_TYPE_OBJECT);}
static inline int bof_is_blob(bof_t *bof){return (bof->type == BOF_TYPE_BLOB);}
static inline int bof_is_null(bof_t *bof){return (bof->type == BOF_TYPE_NULL);}
static inline int bof_is_int32(bof_t *bof){return (bof->type == BOF_TYPE_INT32);}
static inline int bof_is_array(bof_t *bof){return (bof->type == BOF_TYPE_ARRAY);}
static inline int bof_is_string(bof_t *bof){return (bof->type == BOF_TYPE_STRING);}

#endif
