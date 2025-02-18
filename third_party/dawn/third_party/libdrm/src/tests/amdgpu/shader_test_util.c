#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>

#include "CUnit/Basic.h"
#include "amdgpu_test.h"
#include "shader_code.h"

#define	PACKET3_DISPATCH_DIRECT				0x15
#define PACKET3_CONTEXT_CONTROL                   0x28
#define PACKET3_DRAW_INDEX_AUTO				0x2D
#define PACKET3_SET_CONTEXT_REG				0x69
#define PACKET3_SET_SH_REG                        0x76
#define PACKET3_SET_SH_REG_OFFSET                       0x77
#define PACKET3_SET_UCONFIG_REG				0x79
#define PACKET3_SET_SH_REG_INDEX			0x9B

#define	PACKET_TYPE3	3
#define PACKET3(op, n)	((PACKET_TYPE3 << 30) |				\
			 (((op) & 0xFF) << 8) |				\
			 ((n) & 0x3FFF) << 16)
#define PACKET3_COMPUTE(op, n) PACKET3(op, n) | (1 << 1)


struct shader_test_bo {
	amdgpu_bo_handle bo;
	unsigned size;
	unsigned heap;
	void *ptr;
	uint64_t mc_address;
	amdgpu_va_handle va;
};

struct shader_test_draw {
	struct shader_test_bo ps_bo;
	enum ps_type ps_type;
	struct shader_test_bo vs_bo;
	enum vs_type vs_type;
};
struct shader_test_dispatch {
	struct shader_test_bo cs_bo;
	enum cs_type cs_type;
};

struct shader_test_info {
	amdgpu_device_handle device_handle;
	enum amdgpu_test_gfx_version version;
	unsigned ip;
	unsigned ring;
	int hang;
	int hang_slow;
};

struct shader_test_priv {
	const struct shader_test_info *info;
	unsigned cmd_curr;

	union {
		struct shader_test_draw shader_draw;
		struct shader_test_dispatch shader_dispatch;
	};
	struct shader_test_bo vtx_attributes_mem;
	struct shader_test_bo cmd;
	struct shader_test_bo src;
	struct shader_test_bo dst;
};

static int shader_test_bo_alloc(amdgpu_device_handle device_handle,
					    struct shader_test_bo *shader_test_bo)
{
	return amdgpu_bo_alloc_and_map(device_handle, shader_test_bo->size, 4096,
				    shader_test_bo->heap, 0,
				    &(shader_test_bo->bo), (void **)&(shader_test_bo->ptr),
				    &(shader_test_bo->mc_address), &(shader_test_bo->va));
}

static int shader_test_bo_free(struct shader_test_bo *shader_test_bo)
{
	return amdgpu_bo_unmap_and_free(shader_test_bo->bo, shader_test_bo->va,
					shader_test_bo->mc_address,
					shader_test_bo->size);
}

void shader_test_for_each(amdgpu_device_handle device_handle, unsigned ip,
				       void (*fn)(struct shader_test_info *test_info))
{
	int r;
	uint32_t ring_id;
	struct shader_test_info test_info = {0};
	struct drm_amdgpu_info_hw_ip info = {0};

	r = amdgpu_query_hw_ip_info(device_handle, ip, 0, &info);
	CU_ASSERT_EQUAL(r, 0);
	if (!info.available_rings) {
		printf("SKIP ... as there's no %s ring\n",
				(ip == AMDGPU_HW_IP_GFX) ? "graphics": "compute");
		return;
	}

	switch (info.hw_ip_version_major) {
	case 9:
		test_info.version = AMDGPU_TEST_GFX_V9;
		break;
	case 10:
		test_info.version = AMDGPU_TEST_GFX_V10;
		break;
	case 11:
		test_info.version = AMDGPU_TEST_GFX_V11;
		break;
	default:
		printf("SKIP ... unsupported gfx version %d\n", info.hw_ip_version_major);
		return;
	}

	test_info.device_handle = device_handle;
	test_info.ip = ip;

	printf("\n");
	for (ring_id = 0; (1 << ring_id) & info.available_rings; ring_id++) {
		printf("%s ring %d\n", (ip == AMDGPU_HW_IP_GFX) ? "graphics": "compute",
					ring_id);
		test_info.ring = ring_id;
		fn(&test_info);
	}
}

static void write_context_control(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;

	if (test_priv->info->ip == AMDGPU_HW_IP_GFX) {
		ptr[i++] = PACKET3(PACKET3_CONTEXT_CONTROL, 1);
		ptr[i++] = 0x80000000;
		ptr[i++] = 0x80000000;
	}

	test_priv->cmd_curr = i;
}

static void shader_test_load_shader_hang_slow(struct shader_test_bo *shader_bo,
								   struct shader_test_shader_bin *shader_bin)
{
	int i, j, loop;

	loop = (shader_bo->size / sizeof(uint32_t) - shader_bin->header_length
		- shader_bin->foot_length) / shader_bin->body_length;

	memcpy(shader_bo->ptr, shader_bin->shader, shader_bin->header_length * sizeof(uint32_t));

	j = shader_bin->header_length;
	for (i = 0; i < loop; i++) {
		memcpy(shader_bo->ptr + j,
			shader_bin->shader + shader_bin->header_length,
			shader_bin->body_length * sizeof(uint32_t));
		j += shader_bin->body_length;
	}

	memcpy(shader_bo->ptr + j,
		shader_bin->shader + shader_bin->header_length + shader_bin->body_length,
		shader_bin->foot_length * sizeof(uint32_t));
}

static void amdgpu_dispatch_load_cs_shader_hang_slow(struct shader_test_priv *test_priv)
{
	struct amdgpu_gpu_info gpu_info = {0};
	struct shader_test_shader_bin *cs_shader_bin;
	int r;

	r = amdgpu_query_gpu_info(test_priv->info->device_handle, &gpu_info);
	CU_ASSERT_EQUAL(r, 0);

	switch (gpu_info.family_id) {
	case AMDGPU_FAMILY_AI:
		cs_shader_bin = &memcpy_cs_hang_slow_ai;
		break;
	case AMDGPU_FAMILY_RV:
		cs_shader_bin = &memcpy_cs_hang_slow_rv;
		break;
	default:
		cs_shader_bin = &memcpy_cs_hang_slow_nv;
		break;
	}

	shader_test_load_shader_hang_slow(&test_priv->shader_dispatch.cs_bo, cs_shader_bin);
}

static void amdgpu_dispatch_load_cs_shader(struct shader_test_priv *test_priv)
{
	if (test_priv->info->hang) {
		if (test_priv->info->hang_slow)
			amdgpu_dispatch_load_cs_shader_hang_slow(test_priv);
		else
			memcpy(test_priv->shader_dispatch.cs_bo.ptr, memcpy_shader_hang,
				sizeof(memcpy_shader_hang));
	} else {
		memcpy(test_priv->shader_dispatch.cs_bo.ptr,
			shader_test_cs[test_priv->info->version][test_priv->shader_dispatch.cs_type].shader,
			shader_test_cs[test_priv->info->version][test_priv->shader_dispatch.cs_type].shader_size);
	}
}

static void amdgpu_dispatch_init_gfx9(struct shader_test_priv *test_priv)
{
	int i;
	uint32_t *ptr = test_priv->cmd.ptr;

	/* Write context control and load shadowing register if necessary */
	write_context_control(test_priv);

	i = test_priv->cmd_curr;

	/* Issue commands to set default compute state. */
	/* clear mmCOMPUTE_START_Z - mmCOMPUTE_START_X */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 3);
	ptr[i++] = 0x204;
	i += 3;

	/* clear mmCOMPUTE_TMPRING_SIZE */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x218;
	ptr[i++] = 0;

	test_priv->cmd_curr = i;
}

static void amdgpu_dispatch_init_gfx10(struct shader_test_priv *test_priv)
{
	int i;
	uint32_t *ptr = test_priv->cmd.ptr;

	amdgpu_dispatch_init_gfx9(test_priv);

	i = test_priv->cmd_curr;

	/* mmCOMPUTE_SHADER_CHKSUM */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x22a;
	ptr[i++] = 0;
	/* mmCOMPUTE_REQ_CTRL */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 6);
	ptr[i++] = 0x222;
	i += 6;
	/* mmCP_COHER_START_DELAY */
	ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
	ptr[i++] = 0x7b;
	ptr[i++] = 0x20;

	test_priv->cmd_curr = i;
}

static void amdgpu_dispatch_init_gfx11(struct shader_test_priv *test_priv)
{
	int i;
	uint32_t *ptr = test_priv->cmd.ptr;

	/* Write context control and load shadowing register if necessary */
	write_context_control(test_priv);

	i = test_priv->cmd_curr;

	/* Issue commands to set default compute state. */
	/* clear mmCOMPUTE_START_Z - mmCOMPUTE_START_X */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 3);
	ptr[i++] = 0x204;
	i += 3;

	/* clear mmCOMPUTE_TMPRING_SIZE */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x218;
	ptr[i++] = 0;

	/* mmCOMPUTE_REQ_CTRL */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x222;
	ptr[i++] = 0;

	/* mmCOMPUTE_USER_ACCUM_0 .. 3*/
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x224;
	i += 4;

	/* mmCOMPUTE_SHADER_CHKSUM */
	ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
	ptr[i++] = 0x22a;
	ptr[i++] = 0;

	test_priv->cmd_curr = i;
}

static void amdgpu_dispatch_init(struct shader_test_priv *test_priv)
{
	switch (test_priv->info->version) {
	case AMDGPU_TEST_GFX_V9:
		amdgpu_dispatch_init_gfx9(test_priv);
		break;
	case AMDGPU_TEST_GFX_V10:
		amdgpu_dispatch_init_gfx10(test_priv);
		break;
	case AMDGPU_TEST_GFX_V11:
		amdgpu_dispatch_init_gfx11(test_priv);
		break;
	case AMDGPU_TEST_GFX_MAX:
		assert(1 && "Not Support gfx, never go here");
		break;
	}
}

static void amdgpu_dispatch_write_cumask(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;

	/*  Issue commands to set cu mask used in current dispatch */
	switch (test_priv->info->version) {
	case AMDGPU_TEST_GFX_V9:
		/* set mmCOMPUTE_STATIC_THREAD_MGMT_SE1 - mmCOMPUTE_STATIC_THREAD_MGMT_SE0 */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 2);
		ptr[i++] = 0x216;
		ptr[i++] = 0xffffffff;
		ptr[i++] = 0xffffffff;
		/* set mmCOMPUTE_STATIC_THREAD_MGMT_SE3 - mmCOMPUTE_STATIC_THREAD_MGMT_SE2 */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 2);
		ptr[i++] = 0x219;
		ptr[i++] = 0xffffffff;
		ptr[i++] = 0xffffffff;
		break;
	case AMDGPU_TEST_GFX_V10:
	case AMDGPU_TEST_GFX_V11:
		/* set mmCOMPUTE_STATIC_THREAD_MGMT_SE1 - mmCOMPUTE_STATIC_THREAD_MGMT_SE0 */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG_INDEX, 2);
		ptr[i++] = 0x30000216;
		ptr[i++] = 0xffffffff;
		ptr[i++] = 0xffffffff;
		/* set mmCOMPUTE_STATIC_THREAD_MGMT_SE3 - mmCOMPUTE_STATIC_THREAD_MGMT_SE2 */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG_INDEX, 2);
		ptr[i++] = 0x30000219;
		ptr[i++] = 0xffffffff;
		ptr[i++] = 0xffffffff;
		break;
	case AMDGPU_TEST_GFX_MAX:
		assert(1 && "Not Support gfx, never go here");
		break;
	}

	test_priv->cmd_curr = i;
}

static void amdgpu_dispatch_write2hw_gfx9(struct shader_test_priv *test_priv)
{
	const struct shader_test_cs_shader *cs_shader = &shader_test_cs[test_priv->info->version][test_priv->shader_dispatch.cs_type];
	int j, i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;
	uint64_t shader_addr = test_priv->shader_dispatch.cs_bo.mc_address;

	/* Writes shader state to HW */
	/* set mmCOMPUTE_PGM_HI - mmCOMPUTE_PGM_LO */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 2);
	ptr[i++] = 0x20c;
	ptr[i++] = (shader_addr >> 8);
	ptr[i++] = (shader_addr >> 40);
	/* write sh regs*/
	for (j = 0; j < cs_shader->num_sh_reg; j++) {
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
		/* - Gfx9ShRegBase */
		ptr[i++] = cs_shader->sh_reg[j].reg_offset - shader_test_gfx_info[test_priv->info->version].sh_reg_base;
		ptr[i++] = cs_shader->sh_reg[j].reg_value;
	}

	/* Write constant data */
	if (CS_BUFFERCLEAR == test_priv->shader_dispatch.cs_type) {
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x240;
		ptr[i++] = test_priv->dst.mc_address;
		ptr[i++] = (test_priv->dst.mc_address >> 32) | 0x100000;
		ptr[i++] = test_priv->dst.size / 16;
		ptr[i++] = 0x74fac;

		/* Sets a range of pixel shader constants */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x244;
		ptr[i++] = 0x22222222;
		ptr[i++] = 0x22222222;
		ptr[i++] = 0x22222222;
		ptr[i++] = 0x22222222;
	} else {
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x240;
		ptr[i++] = test_priv->src.mc_address;
		ptr[i++] = (test_priv->src.mc_address >> 32) | 0x100000;
		ptr[i++] = test_priv->src.size / 16;
		ptr[i++] = 0x74fac;

		/* Writes the UAV constant data to the SGPRs. */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x244;
		ptr[i++] = test_priv->dst.mc_address;
		ptr[i++] = (test_priv->dst.mc_address >> 32) | 0x100000;
		ptr[i++] = test_priv->dst.size / 16;
		ptr[i++] = 0x74fac;
	}

	test_priv->cmd_curr = i;
}

static void amdgpu_dispatch_write2hw_gfx10(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;
	const struct shader_test_cs_shader *cs_shader = &shader_test_cs[test_priv->info->version][test_priv->shader_dispatch.cs_type];
	int j;
	uint64_t shader_addr = test_priv->shader_dispatch.cs_bo.mc_address;

	/* Writes shader state to HW */
	/* set mmCOMPUTE_PGM_HI - mmCOMPUTE_PGM_LO */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 2);
	ptr[i++] = 0x20c;
	ptr[i++] = (shader_addr >> 8);
	ptr[i++] = (shader_addr >> 40);
	/* write sh regs*/
	for (j = 0; j < cs_shader->num_sh_reg; j++) {
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
		/* - Gfx9ShRegBase */
		ptr[i++] = cs_shader->sh_reg[j].reg_offset - shader_test_gfx_info[test_priv->info->version].sh_reg_base;
		ptr[i++] = cs_shader->sh_reg[j].reg_value;
	}

	/* mmCOMPUTE_PGM_RSRC3 */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x228;
	ptr[i++] = 0;

	if (CS_BUFFERCLEAR == test_priv->shader_dispatch.cs_type) {
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x240;
		ptr[i++] = test_priv->dst.mc_address;
		ptr[i++] = (test_priv->dst.mc_address >> 32) | 0x100000;
		ptr[i++] = test_priv->dst.size / 16;
		ptr[i++] = 0x1104bfac;

		/* Sets a range of pixel shader constants */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x244;
		ptr[i++] = 0x22222222;
		ptr[i++] = 0x22222222;
		ptr[i++] = 0x22222222;
		ptr[i++] = 0x22222222;
	} else {
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x240;
		ptr[i++] = test_priv->src.mc_address;
		ptr[i++] = (test_priv->src.mc_address >> 32) | 0x100000;
		ptr[i++] = test_priv->src.size / 16;
		ptr[i++] = 0x1104bfac;

		/* Writes the UAV constant data to the SGPRs. */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x244;
		ptr[i++] = test_priv->dst.mc_address;
		ptr[i++] = (test_priv->dst.mc_address>> 32) | 0x100000;
		ptr[i++] = test_priv->dst.size / 16;
		ptr[i++] = 0x1104bfac;
	}

	test_priv->cmd_curr = i;
}

static void amdgpu_dispatch_write2hw_gfx11(struct shader_test_priv *test_priv)
{
	enum amdgpu_test_gfx_version version = test_priv->info->version;
	const struct shader_test_cs_shader *cs_shader = &shader_test_cs[version][test_priv->shader_dispatch.cs_type];
	int j, i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;
	uint64_t shader_addr = test_priv->shader_dispatch.cs_bo.mc_address;

	/* Writes shader state to HW */
	/* set mmCOMPUTE_PGM_HI - mmCOMPUTE_PGM_LO */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 2);
	ptr[i++] = 0x20c;
	ptr[i++] = (shader_addr >> 8);
	ptr[i++] = (shader_addr >> 40);

	/* write sh regs*/
	for (j = 0; j < cs_shader->num_sh_reg; j++) {
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
		/* - Gfx9ShRegBase */
		ptr[i++] = cs_shader->sh_reg[j].reg_offset - shader_test_gfx_info[version].sh_reg_base;
		ptr[i++] = cs_shader->sh_reg[j].reg_value;
		if (cs_shader->sh_reg[j].reg_offset == 0x2E12)
			ptr[i-1] &= ~(1<<29);
	}

	/* mmCOMPUTE_PGM_RSRC3 */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x228;
	ptr[i++] = 0x3f0;

	/* Write constant data */
	/* Writes the texture resource constants data to the SGPRs */
	if (CS_BUFFERCLEAR == test_priv->shader_dispatch.cs_type) {
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x240;
		ptr[i++] = test_priv->dst.mc_address;
		ptr[i++] = (test_priv->dst.mc_address >> 32) | 0x100000;
		ptr[i++] = test_priv->dst.size / 16;
		ptr[i++] = 0x1003dfac;

		/* Sets a range of pixel shader constants */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x244;
		ptr[i++] = 0x22222222;
		ptr[i++] = 0x22222222;
		ptr[i++] = 0x22222222;
		ptr[i++] = 0x22222222;
	} else {
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x240;
		ptr[i++] = test_priv->src.mc_address;
		ptr[i++] = (test_priv->src.mc_address >> 32) | 0x100000;
		ptr[i++] = test_priv->src.size / 16;
		ptr[i++] = 0x1003dfac;

		/* Writes the UAV constant data to the SGPRs. */
		ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 4);
		ptr[i++] = 0x244;
		ptr[i++] = test_priv->dst.mc_address;
		ptr[i++] = (test_priv->dst.mc_address>> 32) | 0x100000;
		ptr[i++] = test_priv->dst.size / 16;
		ptr[i++] = 0x1003dfac;
	}

	test_priv->cmd_curr = i;
}

static void amdgpu_dispatch_write2hw(struct shader_test_priv *test_priv)
{
	switch (test_priv->info->version) {
	case AMDGPU_TEST_GFX_V9:
		amdgpu_dispatch_write2hw_gfx9(test_priv);
		break;
	case AMDGPU_TEST_GFX_V10:
		amdgpu_dispatch_write2hw_gfx10(test_priv);
		break;
	case AMDGPU_TEST_GFX_V11:
		amdgpu_dispatch_write2hw_gfx11(test_priv);
		break;
	case AMDGPU_TEST_GFX_MAX:
		assert(1 && "Not Support gfx, never go here");
		break;
	}
}

static void amdgpu_dispatch_write_dispatch_cmd(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;

	/* clear mmCOMPUTE_RESOURCE_LIMITS */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x215;
	ptr[i++] = 0;

	/* dispatch direct command */
	ptr[i++] = PACKET3_COMPUTE(PACKET3_DISPATCH_DIRECT, 3);
	ptr[i++] = (test_priv->dst.size / 16 + 0x40 - 1 ) / 0x40;//0x10;
	ptr[i++] = 1;
	ptr[i++] = 1;
	ptr[i++] = 1;

	test_priv->cmd_curr = i;
}
static void amdgpu_test_dispatch_memset(struct shader_test_info *test_info)
{
	amdgpu_context_handle context_handle;
	amdgpu_bo_handle resources[3];
	struct shader_test_priv test_priv;
	struct shader_test_bo *cmd = &(test_priv.cmd);
	struct shader_test_bo *dst = &(test_priv.dst);
	struct shader_test_bo *shader = &(test_priv.shader_dispatch.cs_bo);
	uint32_t *ptr_cmd;
	uint8_t *ptr_dst;
	int i, r;
	struct amdgpu_cs_request ibs_request = {0};
	struct amdgpu_cs_ib_info ib_info= {0};
	amdgpu_bo_list_handle bo_list;
	struct amdgpu_cs_fence fence_status = {0};
	uint32_t expired;
	uint8_t cptr[16];

	memset(&test_priv, 0, sizeof(test_priv));
	test_priv.info = test_info;
	test_priv.shader_dispatch.cs_type = CS_BUFFERCLEAR;
	r = amdgpu_cs_ctx_create(test_info->device_handle, &context_handle);
	CU_ASSERT_EQUAL(r, 0);

	cmd->size = 4096;
	cmd->heap = AMDGPU_GEM_DOMAIN_GTT;
	r = shader_test_bo_alloc(test_info->device_handle, cmd);
	CU_ASSERT_EQUAL(r, 0);
	ptr_cmd = cmd->ptr;
	memset(ptr_cmd, 0, cmd->size);

	shader->size = 4096;
	shader->heap = AMDGPU_GEM_DOMAIN_VRAM;
	r = shader_test_bo_alloc(test_info->device_handle, shader);
	CU_ASSERT_EQUAL(r, 0);
	memset(shader->ptr, 0, shader->size);
	amdgpu_dispatch_load_cs_shader(&test_priv);

	dst->size = 0x4000;
	dst->heap = AMDGPU_GEM_DOMAIN_VRAM;
	r = shader_test_bo_alloc(test_info->device_handle, dst);
	CU_ASSERT_EQUAL(r, 0);

	amdgpu_dispatch_init(&test_priv);

	/*  Issue commands to set cu mask used in current dispatch */
	amdgpu_dispatch_write_cumask(&test_priv);

	/* Writes shader state to HW */
	amdgpu_dispatch_write2hw(&test_priv);

	amdgpu_dispatch_write_dispatch_cmd(&test_priv);

	i = test_priv.cmd_curr;
	while (i & 7)
		ptr_cmd[i++] = 0xffff1000; /* type3 nop packet */
	test_priv.cmd_curr = i;

	resources[0] = dst->bo;
	resources[1] = shader->bo;
	resources[2] = cmd->bo;
	r = amdgpu_bo_list_create(test_info->device_handle, 3, resources, NULL, &bo_list);
	CU_ASSERT_EQUAL(r, 0);

	ib_info.ib_mc_address = cmd->mc_address;
	ib_info.size = test_priv.cmd_curr;
	ibs_request.ip_type = test_info->ip;
	ibs_request.ring = test_info->ring;
	ibs_request.resources = bo_list;
	ibs_request.number_of_ibs = 1;
	ibs_request.ibs = &ib_info;
	ibs_request.fence_info.handle = NULL;

	/* submit CS */
	r = amdgpu_cs_submit(context_handle, 0, &ibs_request, 1);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_list_destroy(bo_list);
	CU_ASSERT_EQUAL(r, 0);

	fence_status.ip_type = test_info->ip;
	fence_status.ip_instance = 0;
	fence_status.ring = test_info->ring;
	fence_status.context = context_handle;
	fence_status.fence = ibs_request.seq_no;

	/* wait for IB accomplished */
	r = amdgpu_cs_query_fence_status(&fence_status,
					 AMDGPU_TIMEOUT_INFINITE,
					 0, &expired);
	CU_ASSERT_EQUAL(r, 0);
	CU_ASSERT_EQUAL(expired, true);

	/* verify if memset test result meets with expected */
	i = 0;
	ptr_dst = (uint8_t *)(dst->ptr);
	memset(cptr, 0x22, 16);
	CU_ASSERT_EQUAL(memcmp(ptr_dst + i, cptr, 16), 0);
	i = dst->size - 16;
	CU_ASSERT_EQUAL(memcmp(ptr_dst + i, cptr, 16), 0);
	i = dst->size / 2;
	CU_ASSERT_EQUAL(memcmp(ptr_dst + i, cptr, 16), 0);

	r = shader_test_bo_free(dst);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(shader);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(cmd);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_cs_ctx_free(context_handle);
	CU_ASSERT_EQUAL(r, 0);
}

static
void amdgpu_test_dispatch_memcpy(struct shader_test_info *test_info)
{
	struct shader_test_priv test_priv;
	amdgpu_context_handle context_handle;
	amdgpu_bo_handle resources[4];
	struct shader_test_bo *cmd = &(test_priv.cmd);
	struct shader_test_bo *src = &(test_priv.src);
	struct shader_test_bo *dst = &(test_priv.dst);
	struct shader_test_bo *shader = &(test_priv.shader_dispatch.cs_bo);
	uint32_t *ptr_cmd;
	uint8_t *ptr_src;
	uint8_t *ptr_dst;
	int i, r;
	struct amdgpu_cs_request ibs_request = {0};
	struct amdgpu_cs_ib_info ib_info= {0};
	uint32_t expired, hang_state, hangs;
	amdgpu_bo_list_handle bo_list;
	struct amdgpu_cs_fence fence_status = {0};

	memset(&test_priv, 0, sizeof(test_priv));
	test_priv.info = test_info;
	test_priv.cmd.size = 4096;
	test_priv.cmd.heap = AMDGPU_GEM_DOMAIN_GTT;

	test_priv.shader_dispatch.cs_bo.heap = AMDGPU_GEM_DOMAIN_VRAM;
	test_priv.shader_dispatch.cs_type = CS_BUFFERCOPY;
	test_priv.src.heap = AMDGPU_GEM_DOMAIN_VRAM;
	test_priv.dst.heap = AMDGPU_GEM_DOMAIN_VRAM;
	if (test_info->hang_slow) {
		test_priv.shader_dispatch.cs_bo.size = 0x4000000;
		test_priv.src.size = 0x4000000;
		test_priv.dst.size = 0x4000000;
	} else {
		test_priv.shader_dispatch.cs_bo.size = 4096;
		test_priv.src.size = 0x4000;
		test_priv.dst.size = 0x4000;
	}

	r = amdgpu_cs_ctx_create(test_info->device_handle, &context_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_alloc(test_info->device_handle, cmd);
	CU_ASSERT_EQUAL(r, 0);
	ptr_cmd = cmd->ptr;
	memset(ptr_cmd, 0, cmd->size);

	r = shader_test_bo_alloc(test_info->device_handle, shader);
	CU_ASSERT_EQUAL(r, 0);
	memset(shader->ptr, 0, shader->size);
	amdgpu_dispatch_load_cs_shader(&test_priv);

	r = shader_test_bo_alloc(test_info->device_handle, src);
	CU_ASSERT_EQUAL(r, 0);
	ptr_src = (uint8_t *)(src->ptr);
	memset(ptr_src, 0x55, src->size);

	r = shader_test_bo_alloc(test_info->device_handle, dst);
	CU_ASSERT_EQUAL(r, 0);

	amdgpu_dispatch_init(&test_priv);

	/*  Issue commands to set cu mask used in current dispatch */
	amdgpu_dispatch_write_cumask(&test_priv);

	/* Writes shader state to HW */
	amdgpu_dispatch_write2hw(&test_priv);

	amdgpu_dispatch_write_dispatch_cmd(&test_priv);

	i = test_priv.cmd_curr;
	while (i & 7)
		ptr_cmd[i++] = 0xffff1000; /* type3 nop packet */
	test_priv.cmd_curr = i;

	resources[0] = shader->bo;
	resources[1] = src->bo;
	resources[2] = dst->bo;
	resources[3] = cmd->bo;
	r = amdgpu_bo_list_create(test_info->device_handle, 4, resources, NULL, &bo_list);
	CU_ASSERT_EQUAL(r, 0);

	ib_info.ib_mc_address = cmd->mc_address;
	ib_info.size = test_priv.cmd_curr;
	ibs_request.ip_type = test_info->ip;
	ibs_request.ring = test_info->ring;
	ibs_request.resources = bo_list;
	ibs_request.number_of_ibs = 1;
	ibs_request.ibs = &ib_info;
	ibs_request.fence_info.handle = NULL;
	r = amdgpu_cs_submit(context_handle, 0, &ibs_request, 1);
	CU_ASSERT_EQUAL(r, 0);

	fence_status.ip_type = test_info->ip;
	fence_status.ip_instance = 0;
	fence_status.ring = test_info->ring;
	fence_status.context = context_handle;
	fence_status.fence = ibs_request.seq_no;

	/* wait for IB accomplished */
	r = amdgpu_cs_query_fence_status(&fence_status,
					 AMDGPU_TIMEOUT_INFINITE,
					 0, &expired);

	if (!test_info->hang) {
		CU_ASSERT_EQUAL(r, 0);
		CU_ASSERT_EQUAL(expired, true);

		/* verify if memcpy test result meets with expected */
		i = 0;
		ptr_dst = (uint8_t *)dst->ptr;
		CU_ASSERT_EQUAL(memcmp(ptr_dst + i, ptr_src + i, 16), 0);
		i = dst->size - 16;
		CU_ASSERT_EQUAL(memcmp(ptr_dst + i, ptr_src + i, 16), 0);
		i = dst->size / 2;
		CU_ASSERT_EQUAL(memcmp(ptr_dst + i, ptr_src + i, 16), 0);
	} else {
		r = amdgpu_cs_query_reset_state(context_handle, &hang_state, &hangs);
		CU_ASSERT_EQUAL(r, 0);
		CU_ASSERT_EQUAL(hang_state, AMDGPU_CTX_UNKNOWN_RESET);
	}

	r = amdgpu_bo_list_destroy(bo_list);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(src);
	CU_ASSERT_EQUAL(r, 0);
	r = shader_test_bo_free(dst);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(shader);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(cmd);

	r = amdgpu_cs_ctx_free(context_handle);
	CU_ASSERT_EQUAL(r, 0);
}

static void shader_test_dispatch_cb(struct shader_test_info *test_info)
{
	amdgpu_test_dispatch_memset(test_info);
	amdgpu_test_dispatch_memcpy(test_info);
}
static void shader_test_dispatch_hang_cb(struct shader_test_info *test_info)
{
	test_info->hang = 0;
	amdgpu_test_dispatch_memcpy(test_info);

	test_info->hang = 1;
	amdgpu_test_dispatch_memcpy(test_info);

	test_info->hang = 0;
	amdgpu_test_dispatch_memcpy(test_info);
}

static void shader_test_dispatch_hang_slow_cb(struct shader_test_info *test_info)
{
	test_info->hang = 0;
	test_info->hang_slow = 0;
	amdgpu_test_dispatch_memcpy(test_info);

	test_info->hang = 1;
	test_info->hang_slow = 1;
	amdgpu_test_dispatch_memcpy(test_info);

	test_info->hang = 0;
	test_info->hang_slow = 0;
	amdgpu_test_dispatch_memcpy(test_info);
}

void amdgpu_test_dispatch_helper(amdgpu_device_handle device_handle, unsigned ip)
{
	shader_test_for_each(device_handle, ip, shader_test_dispatch_cb);
}

void amdgpu_test_dispatch_hang_helper(amdgpu_device_handle device_handle, uint32_t ip)
{
	shader_test_for_each(device_handle, ip, shader_test_dispatch_hang_cb);
}

void amdgpu_test_dispatch_hang_slow_helper(amdgpu_device_handle device_handle, uint32_t ip)
{
	shader_test_for_each(device_handle, ip, shader_test_dispatch_hang_slow_cb);
}

static void amdgpu_draw_load_ps_shader_hang_slow(struct shader_test_priv *test_priv)
{
	struct amdgpu_gpu_info gpu_info = {0};
	struct shader_test_shader_bin *ps_shader_bin = &memcpy_ps_hang_slow_navi21;
	int r;

	r = amdgpu_query_gpu_info(test_priv->info->device_handle, &gpu_info);
	CU_ASSERT_EQUAL(r, 0);

	switch (gpu_info.family_id) {
		case AMDGPU_FAMILY_AI:
		case AMDGPU_FAMILY_RV:
			ps_shader_bin = &memcpy_ps_hang_slow_ai;
			break;
		case AMDGPU_FAMILY_NV:
			if (gpu_info.chip_external_rev < 40)
				ps_shader_bin = &memcpy_ps_hang_slow_navi10;
			break;
	}

	shader_test_load_shader_hang_slow(&test_priv->shader_draw.ps_bo, ps_shader_bin);
}

static uint32_t round_up_size(uint32_t size)
{
	return (size + 255) & ~255;
}
static void amdgpu_draw_load_ps_shader(struct shader_test_priv *test_priv)
{
	uint8_t *ptr_shader = test_priv->shader_draw.ps_bo.ptr;
	const struct shader_test_ps_shader *shader;
	uint32_t shader_offset, num_export_fmt;
	uint32_t mem_offset, patch_code_offset;
	int i;

	if (test_priv->info->hang) {
		if (test_priv->info->hang_slow)
			amdgpu_draw_load_ps_shader_hang_slow(test_priv);
		else
			memcpy(ptr_shader, memcpy_shader_hang, sizeof(memcpy_shader_hang));

		return;
	}

	shader = &shader_test_ps[test_priv->info->version][test_priv->shader_draw.ps_type];
	num_export_fmt = 10;
	shader_offset = round_up_size(shader->shader_size);
	/* write main shader program */
	for (i = 0 ; i < num_export_fmt; i++) {
		mem_offset = i * shader_offset;
		memcpy(ptr_shader + mem_offset, shader->shader, shader->shader_size);
	}

	/* overwrite patch codes */
	for (i = 0 ; i < num_export_fmt; i++) {
		mem_offset = i * shader_offset + shader->patchinfo_code_offset[0] * sizeof(uint32_t);
		patch_code_offset = i * shader->patchinfo_code_size;
		memcpy(ptr_shader + mem_offset,
			shader->patchinfo_code + patch_code_offset,
			shader->patchinfo_code_size * sizeof(uint32_t));
	}
}

/* load RectPosTexFast_VS */
static void amdgpu_draw_load_vs_shader(struct shader_test_priv *test_priv)
{
	uint8_t *ptr_shader = test_priv->shader_draw.vs_bo.ptr;
	const struct shader_test_vs_shader *shader = &shader_test_vs[test_priv->info->version][test_priv->shader_draw.vs_type];

	memcpy(ptr_shader, shader->shader, shader->shader_size);
}

static void amdgpu_draw_init(struct shader_test_priv *test_priv)
{
	int i;
	uint32_t *ptr = test_priv->cmd.ptr;
	const struct shader_test_gfx_info *gfx_info = &shader_test_gfx_info[test_priv->info->version];

	/* Write context control and load shadowing register if necessary */
	write_context_control(test_priv);
	i = test_priv->cmd_curr;

	if (test_priv->info->version == AMDGPU_TEST_GFX_V11) {
		ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
		ptr[i++] = 0x446;
		ptr[i++] = (test_priv->vtx_attributes_mem.mc_address >> 16);
		// mmSPI_ATTRIBUTE_RING_SIZE
		ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
		ptr[i++] = 0x447;
		ptr[i++] = 0x20001;
	}
	memcpy(ptr + i, gfx_info->preamble_cache, gfx_info->size_preamble_cache);

	test_priv->cmd_curr = i + gfx_info->size_preamble_cache/sizeof(uint32_t);
}

static void amdgpu_draw_setup_and_write_drawblt_surf_info_gfx9(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;

	/* setup color buffer */
	/* offset   reg
	   0xA318   CB_COLOR0_BASE
	   0xA319   CB_COLOR0_BASE_EXT
	   0xA31A   CB_COLOR0_ATTRIB2
	   0xA31B   CB_COLOR0_VIEW
	   0xA31C   CB_COLOR0_INFO
	   0xA31D   CB_COLOR0_ATTRIB
	   0xA31E   CB_COLOR0_DCC_CONTROL
	   0xA31F   CB_COLOR0_CMASK
	   0xA320   CB_COLOR0_CMASK_BASE_EXT
	   0xA321   CB_COLOR0_FMASK
	   0xA322   CB_COLOR0_FMASK_BASE_EXT
	   0xA323   CB_COLOR0_CLEAR_WORD0
	   0xA324   CB_COLOR0_CLEAR_WORD1
	   0xA325   CB_COLOR0_DCC_BASE
	   0xA326   CB_COLOR0_DCC_BASE_EXT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 15);
	ptr[i++] = 0x318;
	ptr[i++] = test_priv->dst.mc_address >> 8;
	ptr[i++] = test_priv->dst.mc_address >> 40;
	ptr[i++] = test_priv->info->hang_slow ? 0x3ffc7ff : 0x7c01f;
	ptr[i++] = 0;
	ptr[i++] = 0x50438;
	ptr[i++] = 0x10140000;
	i += 9;

	/* mmCB_MRT0_EPITCH */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x1e8;
	ptr[i++] = test_priv->info->hang_slow ? 0xfff : 0x1f;

	/* 0xA32B   CB_COLOR1_BASE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x32b;
	ptr[i++] = 0;

	/* 0xA33A   CB_COLOR1_BASE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x33a;
	ptr[i++] = 0;

	/* SPI_SHADER_COL_FORMAT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x1c5;
	ptr[i++] = 9;

	/* Setup depth buffer */
	/* mmDB_Z_INFO */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 2);
	ptr[i++] = 0xe;
	i += 2;

	test_priv->cmd_curr = i;
}
static void amdgpu_draw_setup_and_write_drawblt_surf_info_gfx10(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;

	/* setup color buffer */
	/* 0xA318   CB_COLOR0_BASE
	   0xA319   CB_COLOR0_PITCH
	   0xA31A   CB_COLOR0_SLICE
	   0xA31B   CB_COLOR0_VIEW
	   0xA31C   CB_COLOR0_INFO
	   0xA31D   CB_COLOR0_ATTRIB
	   0xA31E   CB_COLOR0_DCC_CONTROL
	   0xA31F   CB_COLOR0_CMASK
	   0xA320   CB_COLOR0_CMASK_SLICE
	   0xA321   CB_COLOR0_FMASK
	   0xA322   CB_COLOR0_FMASK_SLICE
	   0xA323   CB_COLOR0_CLEAR_WORD0
	   0xA324   CB_COLOR0_CLEAR_WORD1
	   0xA325   CB_COLOR0_DCC_BASE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 14);
	ptr[i++] = 0x318;
	ptr[i++] = test_priv->dst.mc_address >> 8;
	i += 3;
	ptr[i++] = 0x50438;
	i += 9;

	/* 0xA390   CB_COLOR0_BASE_EXT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x390;
	ptr[i++] = test_priv->dst.mc_address >> 40;

	/* 0xA398   CB_COLOR0_CMASK_BASE_EXT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x398;
	ptr[i++] = 0;

	/* 0xA3A0   CB_COLOR0_FMASK_BASE_EXT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x3a0;
	ptr[i++] = 0;

	/* 0xA3A8   CB_COLOR0_DCC_BASE_EXT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x3a8;
	ptr[i++] = 0;

	/* 0xA3B0   CB_COLOR0_ATTRIB2 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x3b0;
	ptr[i++] = test_priv->info->hang_slow ? 0x3ffc7ff : 0x7c01f;

	/* 0xA3B8   CB_COLOR0_ATTRIB3 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x3b8;
	ptr[i++] = 0x9014000;

	/* 0xA32B   CB_COLOR1_BASE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x32b;
	ptr[i++] = 0;

	/* 0xA33A   CB_COLOR1_BASE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x33a;
	ptr[i++] = 0;

	/* SPI_SHADER_COL_FORMAT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x1c5;
	ptr[i++] = 9;

	/* Setup depth buffer */
	/* mmDB_Z_INFO */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 2);
	ptr[i++] = 0x10;
	i += 2;

	test_priv->cmd_curr = i;
}

static void amdgpu_draw_setup_and_write_drawblt_surf_info_gfx11(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;

	/* mmCB_COLOR0_BASE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x318;
	ptr[i++] = test_priv->dst.mc_address >> 8;
	/* mmCB_COLOR0_VIEW .. mmCB_COLOR0_DCC_CONTROL */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 4);
	ptr[i++] = 0x31b;
	i++;
	ptr[i++] = 0x5040e;
	i += 2;
	/* mmCB_COLOR0_DCC_BASE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x325;
	ptr[i++] = 0;
	/* mmCB_COLOR0_BASE_EXT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x390;
	ptr[i++] = (test_priv->dst.mc_address >> 40) & 0xFF;
	/* mmCB_COLOR0_DCC_BASE_EXT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x3a8;
	ptr[i++] = 0;
	/* mmCB_COLOR0_ATTRIB2 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x3b0;
	ptr[i++] = test_priv->info->hang_slow ? 0x1ffc7ff : 0x7c01f;
	/* mmCB_COLOR0_ATTRIB3 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x3b8;
	ptr[i++] = test_priv->info->hang_slow ? 0x1028000 : 0x1018000;
	/* mmCB_COLOR0_INFO */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x32b;
	ptr[i++] = 0;
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x33a;
	ptr[i++] = 0;
	/* mmSPI_SHADER_COL_FORMAT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x1c5;
	ptr[i++] = 0x9;
	/* mmDB_Z_INFO */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 2);
	ptr[i++] = 0x10;
	i += 2;

	test_priv->cmd_curr = i;
}

static void amdgpu_draw_setup_and_write_drawblt_surf_info(struct shader_test_priv *test_priv)
{
	switch (test_priv->info->version) {
	case AMDGPU_TEST_GFX_V9:
		amdgpu_draw_setup_and_write_drawblt_surf_info_gfx9(test_priv);
		break;
	case AMDGPU_TEST_GFX_V10:
		amdgpu_draw_setup_and_write_drawblt_surf_info_gfx10(test_priv);
		break;
	case AMDGPU_TEST_GFX_V11:
		amdgpu_draw_setup_and_write_drawblt_surf_info_gfx11(test_priv);
		break;
	case AMDGPU_TEST_GFX_MAX:
		assert(1 && "Not Support gfx, never go here");
		break;
	}
}

static void amdgpu_draw_setup_and_write_drawblt_state_gfx9(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;
	const struct shader_test_gfx_info *gfx_info = &shader_test_gfx_info[test_priv->info->version];

	/* mmPA_SC_TILE_STEERING_OVERRIDE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0xd7;
	ptr[i++] = 0;

	ptr[i++] = 0xffff1000;
	ptr[i++] = 0xc0021000;

	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0xd7;
	ptr[i++] = 1;

	/* mmPA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 16);
	ptr[i++] = 0x2fe;
	i += 16;

	/* mmPA_SC_CENTROID_PRIORITY_0 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 2);
	ptr[i++] = 0x2f5;
	i += 2;

	memcpy(ptr + i, gfx_info->cached_cmd, gfx_info->size_cached_cmd);
	if (test_priv->info->hang_slow)
		*(ptr + i + 12) = 0x8000800;

	test_priv->cmd_curr = i + gfx_info->size_cached_cmd/sizeof(uint32_t);
}

static void amdgpu_draw_setup_and_write_drawblt_state_gfx10(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;
	const struct shader_test_gfx_info *gfx_info = &shader_test_gfx_info[test_priv->info->version];

	/* mmPA_SC_TILE_STEERING_OVERRIDE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0xd7;
	ptr[i++] = 0;

	ptr[i++] = 0xffff1000;
	ptr[i++] = 0xc0021000;

	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0xd7;
	ptr[i++] = 0;

	/* mmPA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 16);
	ptr[i++] = 0x2fe;
	i += 16;

	/* mmPA_SC_CENTROID_PRIORITY_0 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 2);
	ptr[i++] = 0x2f5;
	i += 2;

	memcpy(ptr + i, gfx_info->cached_cmd, gfx_info->size_cached_cmd);
	if (test_priv->info->hang_slow)
		*(ptr + i + 12) = 0x8000800;
	i += gfx_info->size_cached_cmd/sizeof(uint32_t);

	/* mmCB_RMI_GL2_CACHE_CONTROL */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x104;
	ptr[i++] = 0x40aa0055;
	/* mmDB_RMI_L2_CACHE_CONTROL */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x1f;
	ptr[i++] = 0x2a0055;

	test_priv->cmd_curr = i;
}

static void amdgpu_draw_setup_and_write_drawblt_state_gfx11(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;
	const struct shader_test_gfx_info *gfx_info = &shader_test_gfx_info[test_priv->info->version];

	/* mmPA_SC_TILE_STEERING_OVERRIDE */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0xd7;
	ptr[i++] = 0;

	ptr[i++] = 0xffff1000;
	ptr[i++] = 0xc0021000;

	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0xd7;
	i++;

	/* mmPA_SC_AA_SAMPLE_LOCS_PIXEL_X0Y0_0 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 16);
	ptr[i++] = 0x2fe;
	i += 16;

	/* mmPA_SC_CENTROID_PRIORITY_0 */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 2);
	ptr[i++] = 0x2f5;
	i += 2;

	memcpy(ptr + i, gfx_info->cached_cmd, gfx_info->size_cached_cmd);
	if (test_priv->info->hang_slow)
		*(ptr + i + 12) = 0x8000800;

	test_priv->cmd_curr = i + gfx_info->size_cached_cmd/sizeof(uint32_t);
}

static void amdgpu_draw_setup_and_write_drawblt_state(struct shader_test_priv *test_priv)
{
	switch (test_priv->info->version) {
	case AMDGPU_TEST_GFX_V9:
		amdgpu_draw_setup_and_write_drawblt_state_gfx9(test_priv);
		break;
	case AMDGPU_TEST_GFX_V10:
		amdgpu_draw_setup_and_write_drawblt_state_gfx10(test_priv);
		break;
	case AMDGPU_TEST_GFX_V11:
		amdgpu_draw_setup_and_write_drawblt_state_gfx11(test_priv);
		break;
	case AMDGPU_TEST_GFX_MAX:
		assert(1 && "Not Support gfx, never go here");
		break;
	}
}

static void amdgpu_draw_vs_RectPosTexFast_write2hw_gfx9(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;
	uint64_t shader_addr = test_priv->shader_draw.vs_bo.mc_address;
	enum ps_type ps = test_priv->shader_draw.ps_type;

	/* mmPA_CL_VS_OUT_CNTL */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x207;
	ptr[i++] = 0;

	/* mmSPI_SHADER_PGM_RSRC3_VS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x46;
	ptr[i++] = 0xffff;

	/* mmSPI_SHADER_PGM_LO_VS...mmSPI_SHADER_PGM_HI_VS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 2);
	ptr[i++] = 0x48;
	ptr[i++] = shader_addr >> 8;
	ptr[i++] = shader_addr >> 40;

	/* mmSPI_SHADER_PGM_RSRC1_VS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x4a;
	ptr[i++] = 0xc0081;

	/* mmSPI_SHADER_PGM_RSRC2_VS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x4b;
	ptr[i++] = 0x18;

	/* mmSPI_VS_OUT_CONFIG */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x1b1;
	ptr[i++] = 2;

	/* mmSPI_SHADER_POS_FORMAT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x1c3;
	ptr[i++] = 4;

	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x4c;
	i += 2;
	ptr[i++] = test_priv->info->hang_slow ? 0x45000000 : 0x42000000;
	ptr[i++] = test_priv->info->hang_slow ? 0x45000000 : 0x42000000;

	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x50;
	i += 2;
	if (ps == PS_CONST) {
		i += 2;
	} else if (ps == PS_TEX) {
		ptr[i++] = 0x3f800000;
		ptr[i++] = 0x3f800000;
	}

	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x54;
	i += 4;

	test_priv->cmd_curr = i;
}

static void amdgpu_draw_vs_RectPosTexFast_write2hw_gfx10(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;
	uint64_t shader_addr = test_priv->shader_draw.vs_bo.mc_address;
	enum ps_type ps = test_priv->shader_draw.ps_type;

	/* mmPA_CL_VS_OUT_CNTL */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x207;
	ptr[i++] = 0;

	/* mmSPI_SHADER_PGM_RSRC3_VS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG_INDEX, 1);
	ptr[i++] = 0x30000046;
	ptr[i++] = 0xffff;
	/* mmSPI_SHADER_PGM_RSRC4_VS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG_INDEX, 1);
	ptr[i++] = 0x30000041;
	ptr[i++] = 0xffff;

	/* mmSPI_SHADER_PGM_LO_VS...mmSPI_SHADER_PGM_HI_VS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 2);
	ptr[i++] = 0x48;
	ptr[i++] = shader_addr >> 8;
	ptr[i++] = shader_addr >> 40;

	/* mmSPI_SHADER_PGM_RSRC1_VS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x4a;
	ptr[i++] = 0xc0041;
	/* mmSPI_SHADER_PGM_RSRC2_VS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 1);
	ptr[i++] = 0x4b;
	ptr[i++] = 0x18;

	/* mmSPI_VS_OUT_CONFIG */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x1b1;
	ptr[i++] = 2;

	/* mmSPI_SHADER_POS_FORMAT */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x1c3;
	ptr[i++] = 4;

	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x4c;
	i += 2;
	ptr[i++] = test_priv->info->hang_slow ? 0x45000000 : 0x42000000;
	ptr[i++] = test_priv->info->hang_slow ? 0x45000000 : 0x42000000;

	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x50;
	i += 2;
	if (ps == PS_CONST) {
		i += 2;
	} else if (ps == PS_TEX) {
		ptr[i++] = 0x3f800000;
		ptr[i++] = 0x3f800000;
	}

	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x54;
	i += 4;

	test_priv->cmd_curr = i;
}


static void amdgpu_draw_vs_RectPosTexFast_write2hw_gfx11(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;
	const struct shader_test_gfx_info *gfx_info = &shader_test_gfx_info[test_priv->info->version];
	uint64_t shader_addr = test_priv->shader_draw.vs_bo.mc_address;
	const struct shader_test_vs_shader *shader = &shader_test_vs[test_priv->info->version][test_priv->shader_draw.vs_type];
	enum ps_type ps = test_priv->shader_draw.ps_type;
	int j, offset;

	/* mmPA_CL_VS_OUT_CNTL */
	ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr[i++] = 0x207;
	ptr[i++] = 0;

	/* mmSPI_SHADER_PGM_RSRC3_GS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG_INDEX, 1);
	ptr[i++] = 0x30000087;
	ptr[i++] = 0xffff;
	/* mmSPI_SHADER_PGM_RSRC4_GS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG_INDEX, 1);
	ptr[i++] = 0x30000081;
	ptr[i++] = 0x1fff0001;

	/* mmSPI_SHADER_PGM_LO_ES */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 2);
	ptr[i++] = 0xc8;
	ptr[i++] = shader_addr >> 8;
	ptr[i++] = shader_addr >> 40;

	/* write sh reg */
	for (j = 0; j < shader->num_sh_reg; j++) {
		ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 1);
		ptr[i++] = shader->sh_reg[j].reg_offset - gfx_info->sh_reg_base;
		ptr[i++] = shader->sh_reg[j].reg_value;
	}
	/* write context reg */
	for (j = 0; j < shader->num_context_reg; j++) {
		switch (shader->context_reg[j].reg_offset) {
		case 0xA1B1: //mmSPI_VS_OUT_CONFIG
			ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
			ptr[i++] = shader->context_reg[j].reg_offset - gfx_info->context_reg_base;
			ptr[i++] = 2;
			break;
		case 0xA1C3: //mmSPI_SHADER_POS_FORMAT
			ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
			ptr[i++] = shader->context_reg[j].reg_offset - gfx_info->context_reg_base;
			ptr[i++] = 4;
			break;
		case 0xA2E4: //mmVGT_GS_INSTANCE_CNT
		case 0xA2CE: //mmVGT_GS_MAX_VERT_OUT
			break;
		default:
			ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
			ptr[i++] = shader->context_reg[j].reg_offset - gfx_info->context_reg_base;
			ptr[i++] = shader->context_reg[j].reg_value;
			break;
		}
	}

	// write constant
	// dst rect
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x8c;
	i += 2;
	ptr[i++] = test_priv->info->hang_slow ? 0x45000000 : 0x42000000;
	ptr[i++] = test_priv->info->hang_slow ? 0x45000000 : 0x42000000;
	// src rect
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x90;
	i += 2;
	if (ps == PS_CONST) {
		i += 2;
	} else if (ps == PS_TEX) {
		ptr[i++] = 0x3f800000;
		ptr[i++] = 0x3f800000;
	}

	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr[i++] = 0x94;
	i += 4;
	// vtx_attributes_mem
	ptr[i++] = 0xc02f1000;
	offset = i * sizeof(uint32_t);
	i += 44;
	ptr[i++] = test_priv->vtx_attributes_mem.mc_address & 0xffffffff;
	ptr[i++] = 0xc0100000 | ((test_priv->vtx_attributes_mem.mc_address >> 32) & 0xffff);
	ptr[i++] = test_priv->vtx_attributes_mem.size / 16;
	ptr[i++] = 0x2043ffac;
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG_OFFSET, 2);
	ptr[i++] = 0x98;
	ptr[i++] = offset;
	i++;

	test_priv->cmd_curr = i;
}

static void amdgpu_draw_vs_RectPosTexFast_write2hw(struct shader_test_priv *test_priv)
{
	switch (test_priv->info->version) {
	case AMDGPU_TEST_GFX_V9:
		amdgpu_draw_vs_RectPosTexFast_write2hw_gfx9(test_priv);
		break;
	case AMDGPU_TEST_GFX_V10:
		amdgpu_draw_vs_RectPosTexFast_write2hw_gfx10(test_priv);
		break;
	case AMDGPU_TEST_GFX_V11:
		amdgpu_draw_vs_RectPosTexFast_write2hw_gfx11(test_priv);
		break;
	case AMDGPU_TEST_GFX_MAX:
		assert(1 && "Not Support gfx, never go here");
		break;
	}
}

static void amdgpu_draw_ps_write2hw_gfx9_10(struct shader_test_priv *test_priv)
{
	int i, j;
	uint64_t shader_addr = test_priv->shader_draw.ps_bo.mc_address;
	const struct shader_test_ps_shader *ps = &shader_test_ps[test_priv->info->version][test_priv->shader_draw.ps_type];
	uint32_t *ptr = test_priv->cmd.ptr;

	i = test_priv->cmd_curr;

	if (test_priv->info->version == AMDGPU_TEST_GFX_V9) {
		/* 0x2c07   SPI_SHADER_PGM_RSRC3_PS
		   0x2c08   SPI_SHADER_PGM_LO_PS
		   0x2c09   SPI_SHADER_PGM_HI_PS */
		/* multiplicator 9 is from  SPI_SHADER_COL_FORMAT */
		if (!test_priv->info->hang)
			shader_addr += 256 * 9;
		ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 3);
		ptr[i++] = 0x7;
		ptr[i++] = 0xffff;
		ptr[i++] = shader_addr >> 8;
		ptr[i++] = shader_addr >> 40;
	} else {
		//if (!test_priv->info->hang)
			shader_addr += 256 * 9;
		/* 0x2c08	 SPI_SHADER_PGM_LO_PS
		     0x2c09	 SPI_SHADER_PGM_HI_PS */
		ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 2);
		ptr[i++] = 0x8;
		ptr[i++] = shader_addr >> 8;
		ptr[i++] = shader_addr >> 40;

		/* mmSPI_SHADER_PGM_RSRC3_PS */
		ptr[i++] = PACKET3(PACKET3_SET_SH_REG_INDEX, 1);
		ptr[i++] = 0x30000007;
		ptr[i++] = 0xffff;
		/* mmSPI_SHADER_PGM_RSRC4_PS */
		ptr[i++] = PACKET3(PACKET3_SET_SH_REG_INDEX, 1);
		ptr[i++] = 0x30000001;
		ptr[i++] = 0xffff;
	}

	for (j = 0; j < ps->num_sh_reg; j++) {
		ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 1);
		ptr[i++] = ps->sh_reg[j].reg_offset - 0x2c00;
		ptr[i++] = ps->sh_reg[j].reg_value;
	}

	for (j = 0; j < ps->num_context_reg; j++) {
		if (ps->context_reg[j].reg_offset != 0xA1C5) {
			ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
			ptr[i++] = ps->context_reg[j].reg_offset - 0xa000;
			ptr[i++] = ps->context_reg[j].reg_value;
		}

		if (ps->context_reg[j].reg_offset == 0xA1B4) {
			ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
			ptr[i++] = 0x1b3;
			ptr[i++] = 2;
		}
	}

	test_priv->cmd_curr = i;
}

static void amdgpu_draw_ps_write2hw_gfx11(struct shader_test_priv *test_priv)
{
	int i, j;
	uint64_t shader_addr = test_priv->shader_draw.ps_bo.mc_address;
	enum amdgpu_test_gfx_version version = test_priv->info->version;
	const struct shader_test_ps_shader *ps = &shader_test_ps[version][test_priv->shader_draw.ps_type];
	uint32_t *ptr = test_priv->cmd.ptr;
	uint32_t export_shader_offset;

	i = test_priv->cmd_curr;

	/* SPI_SHADER_PGM_LO_PS
	   SPI_SHADER_PGM_HI_PS */
	shader_addr >>= 8;
	if (!test_priv->info->hang) {
		export_shader_offset = (round_up_size(ps->shader_size) * 9) >> 8;
		shader_addr += export_shader_offset;
	}
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 2);
	ptr[i++] = 0x8;
	ptr[i++] = shader_addr & 0xffffffff;
	ptr[i++] = (shader_addr >> 32) & 0xffffffff;
	/* mmSPI_SHADER_PGM_RSRC3_PS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG_INDEX, 1);
	ptr[i++] = 0x30000007;
	ptr[i++] = 0xffff;
	/* mmSPI_SHADER_PGM_RSRC4_PS */
	ptr[i++] = PACKET3(PACKET3_SET_SH_REG_INDEX, 1);
	ptr[i++] = 0x30000001;
	ptr[i++] = 0x3fffff;

	for (j = 0; j < ps->num_sh_reg; j++) {
		ptr[i++] = PACKET3(PACKET3_SET_SH_REG, 1);
		ptr[i++] = ps->sh_reg[j].reg_offset - shader_test_gfx_info[version].sh_reg_base;
		ptr[i++] = ps->sh_reg[j].reg_value;
	}

	for (j = 0; j < ps->num_context_reg; j++) {
		/* !mmSPI_SHADER_COL_FORMAT */
		if (ps->context_reg[j].reg_offset != 0xA1C5) {
			ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
			ptr[i++] = ps->context_reg[j].reg_offset - shader_test_gfx_info[version].context_reg_base;
			ptr[i++] = ps->context_reg[j].reg_value;
		}

		/* mmSPI_PS_INPUT_ADDR */
		if (ps->context_reg[j].reg_offset == 0xA1B4) {
			ptr[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
			ptr[i++] = 0x1b3;
			ptr[i++] = 2;
		}
	}

	test_priv->cmd_curr = i;
}

static void amdgpu_draw_ps_write2hw(struct shader_test_priv *test_priv)
{
	switch (test_priv->info->version) {
	case AMDGPU_TEST_GFX_V9:
	case AMDGPU_TEST_GFX_V10:
		amdgpu_draw_ps_write2hw_gfx9_10(test_priv);
		break;
	case AMDGPU_TEST_GFX_V11:
		amdgpu_draw_ps_write2hw_gfx11(test_priv);
		break;
	case AMDGPU_TEST_GFX_MAX:
		assert(1 && "Not Support gfx, never go here");
		break;
	}
}

static void amdgpu_draw_draw(struct shader_test_priv *test_priv)
{
	int i = test_priv->cmd_curr;
	uint32_t *ptr = test_priv->cmd.ptr;

	switch (test_priv->info->version) {
	case AMDGPU_TEST_GFX_V9:
		/* mmIA_MULTI_VGT_PARAM */
		ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
		ptr[i++] = 0x40000258;
		ptr[i++] = 0xd00ff;
		/* mmVGT_PRIMITIVE_TYPE */
		ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
		ptr[i++] = 0x10000242;
		ptr[i++] = 0x11;
		break;
	case AMDGPU_TEST_GFX_V10:
		/* mmGE_CNTL */
		ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
		ptr[i++] = 0x25b;
		ptr[i++] = 0xff;
		/* mmVGT_PRIMITIVE_TYPE */
		ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
		ptr[i++] = 0x242;
		ptr[i++] = 0x11;
		break;
	case AMDGPU_TEST_GFX_V11:
		/* mmGE_CNTL */
		ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
		ptr[i++] = 0x25b;
		ptr[i++] = 0x80fc80;
		/* mmVGT_PRIMITIVE_TYPE */
		ptr[i++] = PACKET3(PACKET3_SET_UCONFIG_REG, 1);
		ptr[i++] = 0x242;
		ptr[i++] = 0x11;
		break;
	case AMDGPU_TEST_GFX_MAX:
		assert(1 && "Not Support gfx, never go here");
		break;
	}

	ptr[i++] = PACKET3(PACKET3_DRAW_INDEX_AUTO, 1);
	ptr[i++] = 3;
	ptr[i++] = 2;

	test_priv->cmd_curr = i;
}

static void amdgpu_memset_draw_test(struct shader_test_info *test_info)
{
	struct shader_test_priv test_priv;
	amdgpu_context_handle context_handle;
	struct shader_test_bo *ps_bo = &(test_priv.shader_draw.ps_bo);
	struct shader_test_bo *vs_bo = &(test_priv.shader_draw.vs_bo);
	struct shader_test_bo *dst = &(test_priv.dst);
	struct shader_test_bo *cmd = &(test_priv.cmd);
	struct shader_test_bo *vtx_attributes_mem = &(test_priv.vtx_attributes_mem);
	amdgpu_bo_handle resources[5];
	uint8_t *ptr_dst;
	uint32_t *ptr_cmd;
	int i, r;
	struct amdgpu_cs_request ibs_request = {0};
	struct amdgpu_cs_ib_info ib_info = {0};
	struct amdgpu_cs_fence fence_status = {0};
	uint32_t expired;
	amdgpu_bo_list_handle bo_list;
	uint8_t cptr[16];

	memset(&test_priv, 0, sizeof(test_priv));
	test_priv.info = test_info;

	r = amdgpu_cs_ctx_create(test_info->device_handle, &context_handle);
	CU_ASSERT_EQUAL(r, 0);

	ps_bo->size = 0x2000;
	ps_bo->heap = AMDGPU_GEM_DOMAIN_VRAM;
	r = shader_test_bo_alloc(test_info->device_handle, ps_bo);
	CU_ASSERT_EQUAL(r, 0);
	memset(ps_bo->ptr, 0, ps_bo->size);

	vs_bo->size = 4096;
	vs_bo->heap = AMDGPU_GEM_DOMAIN_VRAM;
	r = shader_test_bo_alloc(test_info->device_handle, vs_bo);
	CU_ASSERT_EQUAL(r, 0);
	memset(vs_bo->ptr, 0, vs_bo->size);

	test_priv.shader_draw.ps_type = PS_CONST;
	amdgpu_draw_load_ps_shader(&test_priv);

	test_priv.shader_draw.vs_type = VS_RECTPOSTEXFAST;
	amdgpu_draw_load_vs_shader(&test_priv);

	cmd->size = 4096;
	cmd->heap = AMDGPU_GEM_DOMAIN_GTT;
	r = shader_test_bo_alloc(test_info->device_handle, cmd);
	CU_ASSERT_EQUAL(r, 0);
	ptr_cmd = cmd->ptr;
	memset(ptr_cmd, 0, cmd->size);

	dst->size = 0x4000;
	dst->heap = AMDGPU_GEM_DOMAIN_VRAM;
	r = shader_test_bo_alloc(test_info->device_handle, dst);
	CU_ASSERT_EQUAL(r, 0);

	if (test_info->version == AMDGPU_TEST_GFX_V11) {
		vtx_attributes_mem->size = 0x4040000;
		vtx_attributes_mem->heap = AMDGPU_GEM_DOMAIN_VRAM;

		r = shader_test_bo_alloc(test_info->device_handle, vtx_attributes_mem);
		CU_ASSERT_EQUAL(r, 0);
	}

	amdgpu_draw_init(&test_priv);

	amdgpu_draw_setup_and_write_drawblt_surf_info(&test_priv);

	amdgpu_draw_setup_and_write_drawblt_state(&test_priv);

	amdgpu_draw_vs_RectPosTexFast_write2hw(&test_priv);

	amdgpu_draw_ps_write2hw(&test_priv);

	i = test_priv.cmd_curr;
	/* ps constant data */
	ptr_cmd[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr_cmd[i++] = 0xc;
	ptr_cmd[i++] = 0x33333333;
	ptr_cmd[i++] = 0x33333333;
	ptr_cmd[i++] = 0x33333333;
	ptr_cmd[i++] = 0x33333333;
	test_priv.cmd_curr = i;

	amdgpu_draw_draw(&test_priv);

	i = test_priv.cmd_curr;
	while (i & 7)
		ptr_cmd[i++] = 0xffff1000; /* type3 nop packet */
	test_priv.cmd_curr = i;

	i = 0;
	resources[i++] = dst->bo;
	resources[i++] = ps_bo->bo;
	resources[i++] = vs_bo->bo;
	resources[i++] = cmd->bo;
	if (vtx_attributes_mem->size)
		resources[i++] = vtx_attributes_mem->bo;
	r = amdgpu_bo_list_create(test_info->device_handle, i, resources, NULL, &bo_list);
	CU_ASSERT_EQUAL(r, 0);

	ib_info.ib_mc_address = cmd->mc_address;
	ib_info.size = test_priv.cmd_curr;
	ibs_request.ip_type = test_info->ip;
	ibs_request.ring = test_info->ring;
	ibs_request.resources = bo_list;
	ibs_request.number_of_ibs = 1;
	ibs_request.ibs = &ib_info;
	ibs_request.fence_info.handle = NULL;

	/* submit CS */
	r = amdgpu_cs_submit(context_handle, 0, &ibs_request, 1);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_list_destroy(bo_list);
	CU_ASSERT_EQUAL(r, 0);

	fence_status.ip_type = test_info->ip;
	fence_status.ip_instance = 0;
	fence_status.ring = test_info->ring;
	fence_status.context = context_handle;
	fence_status.fence = ibs_request.seq_no;

	/* wait for IB accomplished */
	r = amdgpu_cs_query_fence_status(&fence_status,
					 AMDGPU_TIMEOUT_INFINITE,
					 0, &expired);
	CU_ASSERT_EQUAL(r, 0);
	CU_ASSERT_EQUAL(expired, true);

	/* verify if memset test result meets with expected */
	i = 0;
	ptr_dst = dst->ptr;
	memset(cptr, 0x33, 16);
	CU_ASSERT_EQUAL(memcmp(ptr_dst + i, cptr, 16), 0);
	i = dst->size - 16;
	CU_ASSERT_EQUAL(memcmp(ptr_dst + i, cptr, 16), 0);
	i = dst->size / 2;
	CU_ASSERT_EQUAL(memcmp(ptr_dst + i, cptr, 16), 0);

	if (vtx_attributes_mem->size) {
		r = shader_test_bo_free(vtx_attributes_mem);
		CU_ASSERT_EQUAL(r, 0);
	}

	r = shader_test_bo_free(dst);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(cmd);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(ps_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(vs_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_cs_ctx_free(context_handle);
	CU_ASSERT_EQUAL(r, 0);
}

static void amdgpu_memcpy_draw_test(struct shader_test_info *test_info)
{
	struct shader_test_priv test_priv;
	amdgpu_context_handle context_handle;
	struct shader_test_bo *ps_bo = &(test_priv.shader_draw.ps_bo);
	struct shader_test_bo *vs_bo = &(test_priv.shader_draw.vs_bo);
	struct shader_test_bo *src = &(test_priv.src);
	struct shader_test_bo *dst = &(test_priv.dst);
	struct shader_test_bo *cmd = &(test_priv.cmd);
	struct shader_test_bo *vtx_attributes_mem = &(test_priv.vtx_attributes_mem);
	amdgpu_bo_handle resources[6];
	uint8_t *ptr_dst;
	uint8_t *ptr_src;
	uint32_t *ptr_cmd;
	int i, r;
	struct amdgpu_cs_request ibs_request = {0};
	struct amdgpu_cs_ib_info ib_info = {0};
	uint32_t hang_state, hangs;
	uint32_t expired;
	amdgpu_bo_list_handle bo_list;
	struct amdgpu_cs_fence fence_status = {0};

	memset(&test_priv, 0, sizeof(test_priv));
	test_priv.info = test_info;
	test_priv.cmd.size = 4096;
	test_priv.cmd.heap = AMDGPU_GEM_DOMAIN_GTT;

	ps_bo->heap = AMDGPU_GEM_DOMAIN_VRAM;
	test_priv.shader_draw.ps_type = PS_TEX;
	vs_bo->size = 4096;
	vs_bo->heap = AMDGPU_GEM_DOMAIN_VRAM;
	test_priv.shader_draw.vs_type = VS_RECTPOSTEXFAST;
	test_priv.src.heap = AMDGPU_GEM_DOMAIN_VRAM;
	test_priv.dst.heap = AMDGPU_GEM_DOMAIN_VRAM;
	if (test_info->hang_slow) {
		test_priv.shader_draw.ps_bo.size = 16*1024*1024;
		test_priv.src.size = 0x4000000;
		test_priv.dst.size = 0x4000000;
	} else {
		test_priv.shader_draw.ps_bo.size = 0x2000;
		test_priv.src.size = 0x4000;
		test_priv.dst.size = 0x4000;
	}

	r = amdgpu_cs_ctx_create(test_info->device_handle, &context_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_alloc(test_info->device_handle, ps_bo);
	CU_ASSERT_EQUAL(r, 0);
	memset(ps_bo->ptr, 0, ps_bo->size);

	r = shader_test_bo_alloc(test_info->device_handle, vs_bo);
	CU_ASSERT_EQUAL(r, 0);
	memset(vs_bo->ptr, 0, vs_bo->size);

	amdgpu_draw_load_ps_shader(&test_priv);
	amdgpu_draw_load_vs_shader(&test_priv);

	r = shader_test_bo_alloc(test_info->device_handle, cmd);
	CU_ASSERT_EQUAL(r, 0);
	ptr_cmd = cmd->ptr;
	memset(ptr_cmd, 0, cmd->size);

	r = shader_test_bo_alloc(test_info->device_handle, src);
	CU_ASSERT_EQUAL(r, 0);
	ptr_src = src->ptr;
	memset(ptr_src, 0x55, src->size);

	r = shader_test_bo_alloc(test_info->device_handle, dst);
	CU_ASSERT_EQUAL(r, 0);

	if (test_info->version == AMDGPU_TEST_GFX_V11) {
		vtx_attributes_mem->size = 0x4040000;
		vtx_attributes_mem->heap = AMDGPU_GEM_DOMAIN_VRAM;

		r = shader_test_bo_alloc(test_info->device_handle, vtx_attributes_mem);
		CU_ASSERT_EQUAL(r, 0);
	}

	amdgpu_draw_init(&test_priv);

	amdgpu_draw_setup_and_write_drawblt_surf_info(&test_priv);

	amdgpu_draw_setup_and_write_drawblt_state(&test_priv);

	amdgpu_draw_vs_RectPosTexFast_write2hw(&test_priv);

	amdgpu_draw_ps_write2hw(&test_priv);

	// write ps user constant data
	i = test_priv.cmd_curr;
	ptr_cmd[i++] = PACKET3(PACKET3_SET_SH_REG, 8);
	switch (test_info->version) {
	case AMDGPU_TEST_GFX_V9:
		ptr_cmd[i++] = 0xc;
		ptr_cmd[i++] = src->mc_address >> 8;
		ptr_cmd[i++] = src->mc_address >> 40 | 0x10e00000;
		ptr_cmd[i++] = test_info->hang_slow ? 0x1ffcfff : 0x7c01f;
		ptr_cmd[i++] = 0x90500fac;
		ptr_cmd[i++] = test_info->hang_slow ? 0x1ffe000 : 0x3e000;
		i += 3;
		break;
	case AMDGPU_TEST_GFX_V10:
		ptr_cmd[i++] = 0xc;
		ptr_cmd[i++] = src->mc_address >> 8;
		ptr_cmd[i++] = src->mc_address >> 40 | 0xc4b00000;
		ptr_cmd[i++] = test_info->hang_slow ? 0x81ffc1ff : 0x8007c007;
		ptr_cmd[i++] = 0x90500fac;
		i += 2;
		ptr_cmd[i++] = test_info->hang_slow ? 0 : 0x400;
		i++;
		break;
	case AMDGPU_TEST_GFX_V11:
		ptr_cmd[i++] = 0xc;
		ptr_cmd[i++] = src->mc_address >> 8;
		ptr_cmd[i++] = src->mc_address >> 40 | 0xc4b00000;
		ptr_cmd[i++] = test_info->hang_slow ? 0x1ffc1ff : 0x7c007;
		ptr_cmd[i++] = test_info->hang_slow ? 0x90a00fac : 0x90600fac;
		i += 2;
		ptr_cmd[i++] = 0x400;
		i++;
		break;
	case AMDGPU_TEST_GFX_MAX:
		assert(1 && "Not Support gfx, never go here");
		break;
	}

	ptr_cmd[i++] = PACKET3(PACKET3_SET_SH_REG, 4);
	ptr_cmd[i++] = 0x14;
	ptr_cmd[i++] = 0x92;
	i += 3;

	ptr_cmd[i++] = PACKET3(PACKET3_SET_CONTEXT_REG, 1);
	ptr_cmd[i++] = 0x191;
	ptr_cmd[i++] = 0;
	test_priv.cmd_curr = i;

	amdgpu_draw_draw(&test_priv);

	i = test_priv.cmd_curr;
	while (i & 7)
		ptr_cmd[i++] = 0xffff1000; /* type3 nop packet */
	test_priv.cmd_curr = i;

	i = 0;
	resources[i++] = dst->bo;
	resources[i++] = src->bo;
	resources[i++] = ps_bo->bo;
	resources[i++] = vs_bo->bo;
	resources[i++] = cmd->bo;
	if (vtx_attributes_mem->size)
		resources[i++] = vtx_attributes_mem->bo;
	r = amdgpu_bo_list_create(test_info->device_handle, i, resources, NULL, &bo_list);
	CU_ASSERT_EQUAL(r, 0);

	ib_info.ib_mc_address = cmd->mc_address;
	ib_info.size = test_priv.cmd_curr;
	ibs_request.ip_type = test_info->ip;
	ibs_request.ring = test_info->ring;
	ibs_request.resources = bo_list;
	ibs_request.number_of_ibs = 1;
	ibs_request.ibs = &ib_info;
	ibs_request.fence_info.handle = NULL;
	r = amdgpu_cs_submit(context_handle, 0, &ibs_request, 1);
	CU_ASSERT_EQUAL(r, 0);

	fence_status.ip_type = test_info->ip;
	fence_status.ip_instance = 0;
	fence_status.ring = test_info->ring;
	fence_status.context = context_handle;
	fence_status.fence = ibs_request.seq_no;

	/* wait for IB accomplished */
	r = amdgpu_cs_query_fence_status(&fence_status,
					 AMDGPU_TIMEOUT_INFINITE,
					 0, &expired);
	if (!test_info->hang) {
		CU_ASSERT_EQUAL(r, 0);
		CU_ASSERT_EQUAL(expired, true);

		/* verify if memcpy test result meets with expected */
		i = 0;
		ptr_dst = dst->ptr;
		CU_ASSERT_EQUAL(memcmp(ptr_dst + i, ptr_src + i, 16), 0);
		i = dst->size - 16;
		CU_ASSERT_EQUAL(memcmp(ptr_dst + i, ptr_src + i, 16), 0);
		i = dst->size / 2;
		CU_ASSERT_EQUAL(memcmp(ptr_dst + i, ptr_src + i, 16), 0);
	} else {
		r = amdgpu_cs_query_reset_state(context_handle, &hang_state, &hangs);
		CU_ASSERT_EQUAL(r, 0);
		CU_ASSERT_EQUAL(hang_state, AMDGPU_CTX_UNKNOWN_RESET);
	}

	r = amdgpu_bo_list_destroy(bo_list);
	CU_ASSERT_EQUAL(r, 0);

	if (vtx_attributes_mem->size) {
		r = shader_test_bo_free(vtx_attributes_mem);
		CU_ASSERT_EQUAL(r, 0);
	}

	r = shader_test_bo_free(src);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(dst);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(cmd);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(ps_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = shader_test_bo_free(vs_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_cs_ctx_free(context_handle);
	CU_ASSERT_EQUAL(r, 0);
}

static void shader_test_draw_cb(struct shader_test_info *test_info)
{
	amdgpu_memset_draw_test(test_info);
	amdgpu_memcpy_draw_test(test_info);
}

static void shader_test_draw_hang_cb(struct shader_test_info *test_info)
{
	test_info->hang = 0;
	amdgpu_memcpy_draw_test(test_info);

	test_info->hang = 1;
	amdgpu_memcpy_draw_test(test_info);

	test_info->hang = 0;
	amdgpu_memcpy_draw_test(test_info);
}

static void shader_test_draw_hang_slow_cb(struct shader_test_info *test_info)
{
	test_info->hang = 0;
	test_info->hang_slow = 0;
	amdgpu_memcpy_draw_test(test_info);

	test_info->hang = 1;
	test_info->hang_slow = 1;
	amdgpu_memcpy_draw_test(test_info);

	test_info->hang = 0;
	test_info->hang_slow = 0;
	amdgpu_memcpy_draw_test(test_info);
}


void amdgpu_test_draw_helper(amdgpu_device_handle device_handle)
{
	shader_test_for_each(device_handle, AMDGPU_HW_IP_GFX, shader_test_draw_cb);
}

void amdgpu_test_draw_hang_helper(amdgpu_device_handle device_handle)
{
	shader_test_for_each(device_handle, AMDGPU_HW_IP_GFX, shader_test_draw_hang_cb);
}

void amdgpu_test_draw_hang_slow_helper(amdgpu_device_handle device_handle)
{
	shader_test_for_each(device_handle, AMDGPU_HW_IP_GFX, shader_test_draw_hang_slow_cb);
}
