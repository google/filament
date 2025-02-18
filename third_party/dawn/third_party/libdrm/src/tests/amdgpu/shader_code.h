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

#ifndef _shader_code_h_
#define _shader_code_h_

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

enum amdgpu_test_gfx_version {
	AMDGPU_TEST_GFX_V9 = 0,
	AMDGPU_TEST_GFX_V10,
	AMDGPU_TEST_GFX_V11,
	AMDGPU_TEST_GFX_MAX,
};

enum cs_type {
	CS_BUFFERCLEAR = 0,
	CS_BUFFERCOPY,
	CS_HANG,
	CS_HANG_SLOW,
};

enum ps_type {
	PS_CONST,
	PS_TEX,
	PS_HANG,
	PS_HANG_SLOW
};

enum vs_type {
	VS_RECTPOSTEXFAST,
};

struct reg_info {
	uint32_t reg_offset;			///< Memory mapped register offset
	uint32_t reg_value;			///< register value
};

#include "shader_code_hang.h"
#include "shader_code_gfx9.h"
#include "shader_code_gfx10.h"
#include "shader_code_gfx11.h"

struct shader_test_cs_shader {
	const uint32_t *shader;
	uint32_t shader_size;
	const struct reg_info *sh_reg;
	uint32_t num_sh_reg;
	const struct reg_info *context_reg;
	uint32_t num_context_reg;
};

struct shader_test_ps_shader {
	const uint32_t *shader;
	unsigned shader_size;
	uint32_t patchinfo_code_size;
	const uint32_t *patchinfo_code;
	const uint32_t *patchinfo_code_offset;
	const struct reg_info *sh_reg;
	uint32_t num_sh_reg;
	const struct reg_info *context_reg;
	uint32_t num_context_reg;
};

struct shader_test_vs_shader {
	const uint32_t *shader;
	uint32_t shader_size;
	const struct reg_info *sh_reg;
	uint32_t num_sh_reg;
	const struct reg_info *context_reg;
	uint32_t num_context_reg;
};

static const struct shader_test_cs_shader shader_test_cs[AMDGPU_TEST_GFX_MAX][2] = {
	// gfx9, cs_bufferclear
	{{bufferclear_cs_shader_gfx9, sizeof(bufferclear_cs_shader_gfx9), bufferclear_cs_shader_registers_gfx9, ARRAY_SIZE(bufferclear_cs_shader_registers_gfx9)},
	// gfx9, cs_buffercopy
	{buffercopy_cs_shader_gfx9, sizeof(buffercopy_cs_shader_gfx9), bufferclear_cs_shader_registers_gfx9, ARRAY_SIZE(bufferclear_cs_shader_registers_gfx9)}},
	// gfx10, cs_bufferclear
	{{bufferclear_cs_shader_gfx10, sizeof(bufferclear_cs_shader_gfx10), bufferclear_cs_shader_registers_gfx9, ARRAY_SIZE(bufferclear_cs_shader_registers_gfx9)},
	// gfx10, cs_buffercopy
	{buffercopy_cs_shader_gfx10, sizeof(bufferclear_cs_shader_gfx10), bufferclear_cs_shader_registers_gfx9, ARRAY_SIZE(bufferclear_cs_shader_registers_gfx9)}},
	// gfx11, cs_bufferclear
	{{bufferclear_cs_shader_gfx11, sizeof(bufferclear_cs_shader_gfx11), bufferclear_cs_shader_registers_gfx11, ARRAY_SIZE(bufferclear_cs_shader_registers_gfx11)},
	// gfx11, cs_buffercopy
	{buffercopy_cs_shader_gfx11, sizeof(bufferclear_cs_shader_gfx11), bufferclear_cs_shader_registers_gfx11, ARRAY_SIZE(bufferclear_cs_shader_registers_gfx11)}},
};

#define SHADER_PS_INFO(_ps, _n) \
	{ps_##_ps##_shader_gfx##_n, sizeof(ps_##_ps##_shader_gfx##_n), \
	ps_##_ps##_shader_patchinfo_code_size_gfx##_n, \
	&(ps_##_ps##_shader_patchinfo_code_gfx##_n)[0][0][0], \
	ps_##_ps##_shader_patchinfo_offset_gfx##_n, \
	ps_##_ps##_sh_registers_gfx##_n, ps_##_ps##_num_sh_registers_gfx##_n, \
	ps_##_ps##_context_registers_gfx##_n, ps_##_ps##_num_context_registers_gfx##_n}
static const struct shader_test_ps_shader shader_test_ps[AMDGPU_TEST_GFX_MAX][2] = {
	{SHADER_PS_INFO(const, 9), SHADER_PS_INFO(tex, 9)},
	{SHADER_PS_INFO(const, 10), SHADER_PS_INFO(tex, 10)},
	{SHADER_PS_INFO(const, 11), SHADER_PS_INFO(tex, 11)},
};

#define SHADER_VS_INFO(_vs, _n) \
	{vs_##_vs##_shader_gfx##_n, sizeof(vs_##_vs##_shader_gfx##_n), \
	vs_##_vs##_sh_registers_gfx##_n, vs_##_vs##_num_sh_registers_gfx##_n, \
	vs_##_vs##_context_registers_gfx##_n, vs_##_vs##_num_context_registers_gfx##_n}
static const struct shader_test_vs_shader shader_test_vs[AMDGPU_TEST_GFX_MAX][1] = {
	{SHADER_VS_INFO(RectPosTexFast, 9)},
	{SHADER_VS_INFO(RectPosTexFast, 10)},
	{SHADER_VS_INFO(RectPosTexFast, 11)},
};

struct shader_test_gfx_info {
	const uint32_t *preamble_cache;
	uint32_t size_preamble_cache;
	const uint32_t *cached_cmd;
	uint32_t size_cached_cmd;
	uint32_t sh_reg_base;
	uint32_t context_reg_base;
};

#define SHADER_TEST_GFX_INFO(_n) \
	preamblecache_gfx##_n, sizeof(preamblecache_gfx##_n), \
	cached_cmd_gfx##_n, sizeof(cached_cmd_gfx##_n), \
	sh_reg_base_gfx##_n, context_reg_base_gfx##_n

static struct shader_test_gfx_info shader_test_gfx_info[AMDGPU_TEST_GFX_MAX] = {
	{SHADER_TEST_GFX_INFO(9),},
	{SHADER_TEST_GFX_INFO(10),},
	{SHADER_TEST_GFX_INFO(11),},
};
#endif
