/*
 * Copyright 2018 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "handle_table.h"
#include "util_math.h"

drm_private int handle_table_insert(struct handle_table *table, uint32_t key,
				    void *value)
{
	if (key >= table->max_key) {
		uint32_t alignment = sysconf(_SC_PAGESIZE) / sizeof(void*);
		uint32_t max_key = ALIGN(key + 1, alignment);
		void **values;

		values = realloc(table->values, max_key * sizeof(void *));
		if (!values)
			return -ENOMEM;

		memset(values + table->max_key, 0, (max_key - table->max_key) *
		       sizeof(void *));

		table->max_key = max_key;
		table->values = values;
	}
	table->values[key] = value;
	return 0;
}

drm_private void handle_table_remove(struct handle_table *table, uint32_t key)
{
	if (key < table->max_key)
		table->values[key] = NULL;
}

drm_private void *handle_table_lookup(struct handle_table *table, uint32_t key)
{
	if (key < table->max_key)
		return table->values[key];
	else
		return NULL;
}

drm_private void handle_table_fini(struct handle_table *table)
{
	free(table->values);
	table->max_key = 0;
	table->values = NULL;
}
