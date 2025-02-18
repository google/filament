/*
 * GLX Hardware Device Driver common code
 * Copyright (C) 1999 Wittawat Yamwong
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * KEITH WHITWELL, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * Memory manager code.  Primarily used by device drivers to manage texture
 * heaps, etc.
 */

#ifndef MM_H
#define MM_H

#include "libdrm_macros.h"

struct mem_block {
	struct mem_block *next, *prev;
	struct mem_block *next_free, *prev_free;
	struct mem_block *heap;
	int ofs, size;
	unsigned int free:1;
	unsigned int reserved:1;
};

/** 
 * input: total size in bytes
 * return: a heap pointer if OK, NULL if error
 */
drm_private extern struct mem_block *mmInit(int ofs, int size);

/**
 * Allocate 'size' bytes with 2^align2 bytes alignment,
 * restrict the search to free memory after 'startSearch'
 * depth and back buffers should be in different 4mb banks
 * to get better page hits if possible
 * input:	size = size of block
 *       	align2 = 2^align2 bytes alignment
 *		startSearch = linear offset from start of heap to begin search
 * return: pointer to the allocated block, 0 if error
 */
drm_private extern struct mem_block *mmAllocMem(struct mem_block *heap,
						int size, int align2,
						int startSearch);

/**
 * Free block starts at offset
 * input: pointer to a block
 * return: 0 if OK, -1 if error
 */
drm_private extern int mmFreeMem(struct mem_block *b);

/**
 * destroy MM
 */
drm_private extern void mmDestroy(struct mem_block *mmInit);

/**
 * For debugging purpose.
 */
drm_private extern void mmDumpMemInfo(const struct mem_block *mmInit);

#endif
