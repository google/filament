/*
 * Copyright 2011      Luc Verhaegen <libv@codethink.co.uk>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef BMP_DUMP_H
#define BMP_DUMP_H 1
#include <stdbool.h>
/* write 32-bit image (y axis upwards) */
void bmp_dump32(char *buffer, unsigned width, unsigned height, bool bgra, const char *filename);
/* write 32-bit image (y axis downwards) */
void bmp_dump32_noflip(char *buffer, unsigned width, unsigned height, bool bgra, const char *filename);
/* write 32-bit image */
void bmp_dump32_ex(char *buffer, unsigned width, unsigned height, bool flip, bool bgra, bool alpha, const char *filename);

#endif /* BMP_DUMP_H */
