/*
 * Copyright 2022 Advanced Micro Devices, Inc.
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

#ifndef _shader_code_hang_h_
#define _shader_code_hang_h_

static const unsigned int memcpy_shader_hang[] = {
        0xFFFFFFFF, 0xBEFE0A7E, 0xBEFC0304, 0xC0C20100,
        0xC0800300, 0xC8080000, 0xC80C0100, 0xC8090001,
        0xC80D0101, 0xBF8C007F, 0xF0800F00, 0x00010002,
        0xBEFE040C, 0xBF8C0F70, 0xBF800000, 0xBF800000,
        0xF800180F, 0x03020100, 0xBF810000
};

struct shader_test_shader_bin {
	const uint32_t *shader;
	uint32_t header_length;
	uint32_t body_length;
	uint32_t foot_length;
};

static const unsigned int memcpy_cs_hang_slow_ai_codes[] = {
    0xd1fd0000, 0x04010c08, 0xe00c2000, 0x80000100,
    0xbf8c0f70, 0xe01c2000, 0x80010100, 0xbf810000
};

static struct shader_test_shader_bin memcpy_cs_hang_slow_ai = {
        memcpy_cs_hang_slow_ai_codes, 4, 3, 1
};

static const unsigned int memcpy_cs_hang_slow_rv_codes[] = {
    0x8e00860c, 0x32000000, 0xe00c2000, 0x80010100,
    0xbf8c0f70, 0xe01c2000, 0x80020100, 0xbf810000
};

static struct shader_test_shader_bin memcpy_cs_hang_slow_rv = {
        memcpy_cs_hang_slow_rv_codes, 4, 3, 1
};

static const unsigned int memcpy_cs_hang_slow_nv_codes[] = {
    0xd7460000, 0x04010c08, 0xe00c2000, 0x80000100,
    0xbf8c0f70, 0xe01ca000, 0x80010100, 0xbf810000
};

static struct shader_test_shader_bin memcpy_cs_hang_slow_nv = {
        memcpy_cs_hang_slow_nv_codes, 4, 3, 1
};


static const unsigned int memcpy_ps_hang_slow_ai_codes[] = {
	0xbefc000c, 0xbe8e017e, 0xbefe077e, 0xd4080000,
	0xd4090001, 0xd40c0100, 0xd40d0101, 0xf0800f00,
	0x00400002, 0xbefe010e, 0xbf8c0f70, 0xbf800000,
	0xbf800000, 0xbf800000, 0xbf800000, 0xc400180f,
	0x03020100, 0xbf810000
};

static struct shader_test_shader_bin memcpy_ps_hang_slow_ai = {
        memcpy_ps_hang_slow_ai_codes, 7, 2, 9
};

static const unsigned int memcpy_ps_hang_slow_navi10_codes[] = {
	0xBEFC030C,0xBE8E047E,0xBEFE0A7E,0xC8080000,
	0xC80C0100,0xC8090001,0xC80D0101,0xF0800F0A,
	0x00400402,0x00000003,0xBEFE040E,0xBF8C0F70,
	0xBF800000,0xBF800000,0xBF800000,0xBF800000,
	0xF800180F,0x07060504,0xBF810000
};

static struct shader_test_shader_bin memcpy_ps_hang_slow_navi10 = {
	memcpy_ps_hang_slow_navi10_codes, 7, 3, 9
};

static const unsigned int memcpy_ps_hang_slow_navi21_codes[] = {
    0xBEFC030C, 0xBE8E047E, 0xBEFE0A7E, 0xC8080000, 0xC8000100, 0xC8090001, 0xC8010101, 0x87FE0E7E, // header
    0xF0800F0A, 0x00400002, 0x00000000, // body - image_sample instruction
    0xBFA3FFE3, 0xBEFE040E, 0xBF8C3F70, 0xBF800000, 0xBF800000, 0xBF800000, 0xBF800000, 0xF800180F, 0x03020100, 0xBF810000 // footer
};

static struct shader_test_shader_bin memcpy_ps_hang_slow_navi21 = {
	memcpy_ps_hang_slow_navi21_codes, 8, 3, 10
};

#endif
