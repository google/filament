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
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "bof.h"

/*
 * helpers
 */
static int bof_entry_grow(bof_t *bof)
{
	bof_t **array;

	if (bof->array_size < bof->nentry)
		return 0;
	array = realloc(bof->array, (bof->nentry + 16) * sizeof(void*));
	if (array == NULL)
		return -ENOMEM;
	bof->array = array;
	bof->nentry += 16;
	return 0;
}

/*
 * object 
 */
bof_t *bof_object(void)
{
	bof_t *object;

	object = calloc(1, sizeof(bof_t));
	if (object == NULL)
		return NULL;
	object->refcount = 1;
	object->type = BOF_TYPE_OBJECT;
	object->size = 12;
	return object;
}

bof_t *bof_object_get(bof_t *object, const char *keyname)
{
	unsigned i;

	for (i = 0; i < object->array_size; i += 2) {
		if (!strcmp(object->array[i]->value, keyname)) {
			return object->array[i + 1];
		}
	}
	return NULL;
}

int bof_object_set(bof_t *object, const char *keyname, bof_t *value)
{
	bof_t *key;
	int r;

	if (object->type != BOF_TYPE_OBJECT)
		return -EINVAL;
	r = bof_entry_grow(object);
	if (r)
		return r;
	key = bof_string(keyname);
	if (key == NULL)
		return -ENOMEM;
	object->array[object->array_size++] = key;
	object->array[object->array_size++] = value;
	object->size += value->size;
	object->size += key->size;
	bof_incref(value);
	return 0;
}

/*
 * array
 */
bof_t *bof_array(void)
{
	bof_t *array = bof_object();

	if (array == NULL)
		return NULL;
	array->type = BOF_TYPE_ARRAY;
	array->size = 12;
	return array;
}

int bof_array_append(bof_t *array, bof_t *value)
{
	int r;
	if (array->type != BOF_TYPE_ARRAY)
		return -EINVAL;
	r = bof_entry_grow(array);
	if (r)
		return r;
	array->array[array->array_size++] = value;
	array->size += value->size;
	bof_incref(value);
	return 0;
}

bof_t *bof_array_get(bof_t *bof, unsigned i)
{
	if (!bof_is_array(bof) || i >= bof->array_size)
		return NULL;
	return bof->array[i];
}

unsigned bof_array_size(bof_t *bof)
{
	if (!bof_is_array(bof))
		return 0;
	return bof->array_size;
}

/*
 * blob
 */
bof_t *bof_blob(unsigned size, void *value)
{
	bof_t *blob = bof_object();

	if (blob == NULL)
		return NULL;
	blob->type = BOF_TYPE_BLOB;
	blob->value = calloc(1, size);
	if (blob->value == NULL) {
		bof_decref(blob);
		return NULL;
	}
	blob->size = size;
	memcpy(blob->value, value, size);
	blob->size += 12;
	return blob;
}

unsigned bof_blob_size(bof_t *bof)
{
	if (!bof_is_blob(bof))
		return 0;
	return bof->size - 12;
}

void *bof_blob_value(bof_t *bof)
{
	if (!bof_is_blob(bof))
		return NULL;
	return bof->value;
}

/*
 * string
 */
bof_t *bof_string(const char *value)
{
	bof_t *string = bof_object();

	if (string == NULL)
		return NULL;
	string->type = BOF_TYPE_STRING;
	string->size = strlen(value) + 1;
	string->value = calloc(1, string->size);
	if (string->value == NULL) {
		bof_decref(string);
		return NULL;
	}
	strcpy(string->value, value);
	string->size += 12;
	return string;
}

/*
 *  int32
 */
bof_t *bof_int32(int32_t value)
{
	bof_t *int32 = bof_object();

	if (int32 == NULL)
		return NULL;
	int32->type = BOF_TYPE_INT32;
	int32->size = 4;
	int32->value = calloc(1, int32->size);
	if (int32->value == NULL) {
		bof_decref(int32);
		return NULL;
	}
	memcpy(int32->value, &value, 4);
	int32->size += 12;
	return int32;
}

int32_t bof_int32_value(bof_t *bof)
{
	return *((uint32_t*)bof->value);
}

/*
 *  common
 */
static void bof_indent(int level)
{
	int i;

	for (i = 0; i < level; i++)
		fprintf(stderr, " ");
}

static void bof_print_bof(bof_t *bof, int level, int entry)
{
	bof_indent(level);
	if (bof == NULL) {
		fprintf(stderr, "--NULL-- for entry %d\n", entry);
		return;
	}
	switch (bof->type) {
	case BOF_TYPE_STRING:
		fprintf(stderr, "%p string [%s %d]\n", bof, (char*)bof->value, bof->size);
		break;
	case BOF_TYPE_INT32:
		fprintf(stderr, "%p int32 [%d %d]\n", bof, *(int*)bof->value, bof->size);
		break;
	case BOF_TYPE_BLOB:
		fprintf(stderr, "%p blob [%d]\n", bof, bof->size);
		break;
	case BOF_TYPE_NULL:
		fprintf(stderr, "%p null [%d]\n", bof, bof->size);
		break;
	case BOF_TYPE_OBJECT:
		fprintf(stderr, "%p object [%d %d]\n", bof, bof->array_size / 2, bof->size);
		break;
	case BOF_TYPE_ARRAY:
		fprintf(stderr, "%p array [%d %d]\n", bof, bof->array_size, bof->size);
		break;
	default:
		fprintf(stderr, "%p unknown [%d]\n", bof, bof->type);
		return;
	}
}

static void bof_print_rec(bof_t *bof, int level, int entry)
{
	unsigned i;

	bof_print_bof(bof, level, entry);
	for (i = 0; i < bof->array_size; i++) {
		bof_print_rec(bof->array[i], level + 2, i);
	}
}

void bof_print(bof_t *bof)
{
	bof_print_rec(bof, 0, 0);
}

static int bof_read(bof_t *root, FILE *file, long end, int level)
{
	bof_t *bof = NULL;
	int r;

	if (ftell(file) >= end) {
		return 0;
	}
	r = bof_entry_grow(root);
	if (r)
		return r;
	bof = bof_object();
	if (bof == NULL)
		return -ENOMEM;
	bof->offset = ftell(file);
	r = fread(&bof->type, 4, 1, file);
	if (r != 1)
		goto out_err;
	r = fread(&bof->size, 4, 1, file);
	if (r != 1)
		goto out_err;
	r = fread(&bof->array_size, 4, 1, file);
	if (r != 1)
		goto out_err;
	switch (bof->type) {
	case BOF_TYPE_STRING:
	case BOF_TYPE_INT32:
	case BOF_TYPE_BLOB:
		bof->value = calloc(1, bof->size - 12);
		if (bof->value == NULL) {
			goto out_err;
		}
		r = fread(bof->value, bof->size - 12, 1, file);
		if (r != 1) {
			fprintf(stderr, "error reading %d\n", bof->size - 12);
			goto out_err;
		}
		break;
	case BOF_TYPE_NULL:
		return 0;
	case BOF_TYPE_OBJECT:
	case BOF_TYPE_ARRAY:
		r = bof_read(bof, file, bof->offset + bof->size, level + 2);
		if (r)
			goto out_err;
		break;
	default:
		fprintf(stderr, "invalid type %d\n", bof->type);
		goto out_err;
	}
	root->array[root->centry++] = bof;
	return bof_read(root, file, end, level);
out_err:
	bof_decref(bof);
	return -EINVAL;
}

bof_t *bof_load_file(const char *filename)
{
	bof_t *root = bof_object();
	int r;

	if (root == NULL) {
		fprintf(stderr, "%s failed to create root object\n", __func__);
		return NULL;
	}
	root->file = fopen(filename, "r");
	if (root->file == NULL)
		goto out_err;
	r = fseek(root->file, 0L, SEEK_SET);
	if (r) {
		fprintf(stderr, "%s failed to seek into file %s\n", __func__, filename);
		goto out_err;
	}
	root->offset = ftell(root->file);
	r = fread(&root->type, 4, 1, root->file);
	if (r != 1)
		goto out_err;
	r = fread(&root->size, 4, 1, root->file);
	if (r != 1)
		goto out_err;
	r = fread(&root->array_size, 4, 1, root->file);
	if (r != 1)
		goto out_err;
	r = bof_read(root, root->file, root->offset + root->size, 2);
	if (r)
		goto out_err;
	return root;
out_err:
	bof_decref(root);
	return NULL;
}

void bof_incref(bof_t *bof)
{
	bof->refcount++;
}

void bof_decref(bof_t *bof)
{
	unsigned i;

	if (bof == NULL)
		return;
	if (--bof->refcount > 0)
		return;
	for (i = 0; i < bof->array_size; i++) {
		bof_decref(bof->array[i]);
		bof->array[i] = NULL;
	}
	bof->array_size = 0;
	if (bof->file) {
		fclose(bof->file);
		bof->file = NULL;
	}
	free(bof->array);
	free(bof->value);
	free(bof);
}

static int bof_file_write(bof_t *bof, FILE *file)
{
	unsigned i;
	int r;

	r = fwrite(&bof->type, 4, 1, file);
	if (r != 1)
		return -EINVAL;
	r = fwrite(&bof->size, 4, 1, file);
	if (r != 1)
		return -EINVAL;
	r = fwrite(&bof->array_size, 4, 1, file);
	if (r != 1)
		return -EINVAL;
	switch (bof->type) {
	case BOF_TYPE_NULL:
		if (bof->size)
			return -EINVAL;
		break;
	case BOF_TYPE_STRING:
	case BOF_TYPE_INT32:
	case BOF_TYPE_BLOB:
		r = fwrite(bof->value, bof->size - 12, 1, file);
		if (r != 1)
			return -EINVAL;
		break;
	case BOF_TYPE_OBJECT:
	case BOF_TYPE_ARRAY:
		for (i = 0; i < bof->array_size; i++) {
			r = bof_file_write(bof->array[i], file);
			if (r)
				return r;
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

int bof_dump_file(bof_t *bof, const char *filename)
{
	unsigned i;
	int r = 0;

	if (bof->file) {
		fclose(bof->file);
		bof->file = NULL;
	}
	bof->file = fopen(filename, "w");
	if (bof->file == NULL) {
		fprintf(stderr, "%s failed to open file %s\n", __func__, filename);
		r = -EINVAL;
		goto out_err;
	}
	r = fseek(bof->file, 0L, SEEK_SET);
	if (r) {
		fprintf(stderr, "%s failed to seek into file %s\n", __func__, filename);
		goto out_err;
	}
	r = fwrite(&bof->type, 4, 1, bof->file);
	if (r != 1)
		goto out_err;
	r = fwrite(&bof->size, 4, 1, bof->file);
	if (r != 1)
		goto out_err;
	r = fwrite(&bof->array_size, 4, 1, bof->file);
	if (r != 1)
		goto out_err;
	for (i = 0; i < bof->array_size; i++) {
		r = bof_file_write(bof->array[i], bof->file);
		if (r)
			return r;
	}
out_err:
	fclose(bof->file);
	bof->file = NULL;
	return r;
}
