/*
 * Copyright 2017 Advanced Micro Devices, Inc.
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

#include <inttypes.h>
#include <stdio.h>

#include "CUnit/Basic.h"

#include "util_math.h"

#include "amdgpu_drm.h"
#include "amdgpu_internal.h"
#include "amdgpu_test.h"
#include "decode_messages.h"

/* jpeg registers */
#define mmUVD_JPEG_CNTL				0x0200
#define mmUVD_JPEG_RB_BASE			0x0201
#define mmUVD_JPEG_RB_WPTR			0x0202
#define mmUVD_JPEG_RB_RPTR			0x0203
#define mmUVD_JPEG_RB_SIZE			0x0204
#define mmUVD_JPEG_TIER_CNTL2			0x021a
#define mmUVD_JPEG_UV_TILING_CTRL		0x021c
#define mmUVD_JPEG_TILING_CTRL			0x021e
#define mmUVD_JPEG_OUTBUF_RPTR			0x0220
#define mmUVD_JPEG_OUTBUF_WPTR			0x0221
#define mmUVD_JPEG_PITCH			0x0222
#define mmUVD_JPEG_INT_EN			0x0229
#define mmUVD_JPEG_UV_PITCH			0x022b
#define mmUVD_JPEG_INDEX			0x023e
#define mmUVD_JPEG_DATA				0x023f
#define mmUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH	0x0438
#define mmUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW	0x0439
#define mmUVD_LMI_JPEG_READ_64BIT_BAR_HIGH	0x045a
#define mmUVD_LMI_JPEG_READ_64BIT_BAR_LOW	0x045b
#define mmUVD_CTX_INDEX				0x0528
#define mmUVD_CTX_DATA				0x0529
#define mmUVD_SOFT_RESET			0x05a0

#define vcnipUVD_JPEG_DEC_SOFT_RST		0x402f
#define vcnipUVD_JRBC_IB_COND_RD_TIMER		0x408e
#define vcnipUVD_JRBC_IB_REF_DATA		0x408f
#define vcnipUVD_LMI_JPEG_READ_64BIT_BAR_HIGH	0x40e1
#define vcnipUVD_LMI_JPEG_READ_64BIT_BAR_LOW	0x40e0
#define vcnipUVD_JPEG_RB_BASE			0x4001
#define vcnipUVD_JPEG_RB_SIZE			0x4004
#define vcnipUVD_JPEG_RB_WPTR			0x4002
#define vcnipUVD_JPEG_PITCH			0x401f
#define vcnipUVD_JPEG_UV_PITCH			0x4020
#define vcnipJPEG_DEC_ADDR_MODE			0x4027
#define vcnipJPEG_DEC_Y_GFX10_TILING_SURFACE	0x4024
#define vcnipJPEG_DEC_UV_GFX10_TILING_SURFACE	0x4025
#define vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH	0x40e3
#define vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW	0x40e2
#define vcnipUVD_JPEG_INDEX			0x402c
#define vcnipUVD_JPEG_DATA			0x402d
#define vcnipUVD_JPEG_TIER_CNTL2		0x400f
#define vcnipUVD_JPEG_OUTBUF_RPTR		0x401e
#define vcnipUVD_JPEG_OUTBUF_CNTL		0x401c
#define vcnipUVD_JPEG_INT_EN			0x400a
#define vcnipUVD_JPEG_CNTL			0x4000
#define vcnipUVD_JPEG_RB_RPTR			0x4003
#define vcnipUVD_JPEG_OUTBUF_WPTR		0x401d


#define RDECODE_PKT_REG_J(x)		((unsigned)(x)&0x3FFFF)
#define RDECODE_PKT_RES_J(x)		(((unsigned)(x)&0x3F) << 18)
#define RDECODE_PKT_COND_J(x)		(((unsigned)(x)&0xF) << 24)
#define RDECODE_PKT_TYPE_J(x)		(((unsigned)(x)&0xF) << 28)
#define RDECODE_PKTJ(reg, cond, type)	(RDECODE_PKT_REG_J(reg) | \
					 RDECODE_PKT_RES_J(0) | \
					 RDECODE_PKT_COND_J(cond) | \
					 RDECODE_PKT_TYPE_J(type))

#define UVD_BASE_INST0_SEG1		0x00007E00
#define SOC15_REG_ADDR(reg)		(UVD_BASE_INST0_SEG1 + reg)

#define COND0				0
#define COND1				1
#define COND3				3
#define TYPE0				0
#define TYPE1				1
#define TYPE3				3
#define JPEG_DEC_DT_PITCH		0x100
#define JPEG_DEC_BSD_SIZE		0x180
#define JPEG_DEC_LUMA_OFFSET		0
#define JPEG_DEC_CHROMA_OFFSET		0x1000
#define JPEG_DEC_SUM			4096
#define IB_SIZE				4096
#define MAX_RESOURCES			16

struct amdgpu_jpeg_bo {
	amdgpu_bo_handle handle;
	amdgpu_va_handle va_handle;
	uint64_t addr;
	uint64_t size;
	uint8_t *ptr;
};

static amdgpu_device_handle device_handle;
static uint32_t major_version;
static uint32_t minor_version;
static uint32_t family_id;
static uint32_t chip_rev;
static uint32_t chip_id;
static uint32_t asic_id;
static uint32_t chip_rev;
static uint32_t chip_id;

static amdgpu_context_handle context_handle;
static amdgpu_bo_handle ib_handle;
static amdgpu_va_handle ib_va_handle;
static uint64_t ib_mc_address;
static uint32_t *ib_cpu;
static uint32_t len;

static amdgpu_bo_handle resources[MAX_RESOURCES];
static unsigned num_resources;
bool jpeg_direct_reg;

static void set_reg_jpeg(unsigned reg, unsigned cond, unsigned type,
                         uint32_t val);
static void send_cmd_bitstream(uint64_t addr);
static void send_cmd_target(uint64_t addr);
static void send_cmd_bitstream_direct(uint64_t addr);
static void send_cmd_target_direct(uint64_t addr);

static void amdgpu_cs_jpeg_decode(void);

CU_TestInfo jpeg_tests[] = {
	{"JPEG decode", amdgpu_cs_jpeg_decode},
	CU_TEST_INFO_NULL,
};

CU_BOOL suite_jpeg_tests_enable(void)
{
	struct drm_amdgpu_info_hw_ip info;
	int r;

	if (amdgpu_device_initialize(drm_amdgpu[0], &major_version, &minor_version,
	                             &device_handle))
		return CU_FALSE;

	family_id = device_handle->info.family_id;
	asic_id = device_handle->info.asic_id;
	chip_rev = device_handle->info.chip_rev;
	chip_id = device_handle->info.chip_external_rev;

	r = amdgpu_query_hw_ip_info(device_handle, AMDGPU_HW_IP_VCN_JPEG, 0, &info);

	if (amdgpu_device_deinitialize(device_handle))
		return CU_FALSE;

	if (r != 0 || !info.available_rings ||
	        (family_id < AMDGPU_FAMILY_RV &&
	         (family_id == AMDGPU_FAMILY_AI &&
	          (chip_id - chip_rev) < 0x32))) { /* Arcturus */
		printf("\n\nThe ASIC NOT support JPEG, suite disabled\n");
		return CU_FALSE;
	}

	if (info.hw_ip_version_major == 1)
		jpeg_direct_reg = false;
	else if (info.hw_ip_version_major > 1 && info.hw_ip_version_major <= 4)
		jpeg_direct_reg = true;
	else
		return CU_FALSE;

	return CU_TRUE;
}

int suite_jpeg_tests_init(void)
{
	int r;

	r = amdgpu_device_initialize(drm_amdgpu[0], &major_version, &minor_version,
	                             &device_handle);
	if (r)
		return CUE_SINIT_FAILED;

	family_id = device_handle->info.family_id;

	r = amdgpu_cs_ctx_create(device_handle, &context_handle);
	if (r)
		return CUE_SINIT_FAILED;

	r = amdgpu_bo_alloc_and_map(device_handle, IB_SIZE, 4096,
	                            AMDGPU_GEM_DOMAIN_GTT, 0, &ib_handle,
	                            (void **)&ib_cpu, &ib_mc_address, &ib_va_handle);
	if (r)
		return CUE_SINIT_FAILED;

	return CUE_SUCCESS;
}

int suite_jpeg_tests_clean(void)
{
	int r;

	r = amdgpu_bo_unmap_and_free(ib_handle, ib_va_handle, ib_mc_address, IB_SIZE);
	if (r)
		return CUE_SCLEAN_FAILED;

	r = amdgpu_cs_ctx_free(context_handle);
	if (r)
		return CUE_SCLEAN_FAILED;

	r = amdgpu_device_deinitialize(device_handle);
	if (r)
		return CUE_SCLEAN_FAILED;

	return CUE_SUCCESS;
}

static int submit(unsigned ndw, unsigned ip)
{
	struct amdgpu_cs_request ibs_request = {0};
	struct amdgpu_cs_ib_info ib_info = {0};
	struct amdgpu_cs_fence fence_status = {0};
	uint32_t expired;
	int r;

	ib_info.ib_mc_address = ib_mc_address;
	ib_info.size = ndw;

	ibs_request.ip_type = ip;

	r = amdgpu_bo_list_create(device_handle, num_resources, resources, NULL,
	                          &ibs_request.resources);
	if (r)
		return r;

	ibs_request.number_of_ibs = 1;
	ibs_request.ibs = &ib_info;
	ibs_request.fence_info.handle = NULL;

	r = amdgpu_cs_submit(context_handle, 0, &ibs_request, 1);
	if (r)
		return r;

	r = amdgpu_bo_list_destroy(ibs_request.resources);
	if (r)
		return r;

	fence_status.context = context_handle;
	fence_status.ip_type = ip;
	fence_status.fence = ibs_request.seq_no;

	r = amdgpu_cs_query_fence_status(&fence_status, AMDGPU_TIMEOUT_INFINITE, 0,
	                                 &expired);
	if (r)
		return r;

	return 0;
}

static void alloc_resource(struct amdgpu_jpeg_bo *jpeg_bo, unsigned size,
                           unsigned domain)
{
	struct amdgpu_bo_alloc_request req = {0};
	amdgpu_bo_handle buf_handle;
	amdgpu_va_handle va_handle;
	uint64_t va = 0;
	int r;

	req.alloc_size = ALIGN(size, 4096);
	req.preferred_heap = domain;
	r = amdgpu_bo_alloc(device_handle, &req, &buf_handle);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_va_range_alloc(device_handle, amdgpu_gpu_va_range_general,
	                          req.alloc_size, 1, 0, &va, &va_handle, 0);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_bo_va_op(buf_handle, 0, req.alloc_size, va, 0, AMDGPU_VA_OP_MAP);
	CU_ASSERT_EQUAL(r, 0);
	jpeg_bo->addr = va;
	jpeg_bo->handle = buf_handle;
	jpeg_bo->size = req.alloc_size;
	jpeg_bo->va_handle = va_handle;
	r = amdgpu_bo_cpu_map(jpeg_bo->handle, (void **)&jpeg_bo->ptr);
	CU_ASSERT_EQUAL(r, 0);
	memset(jpeg_bo->ptr, 0, size);
	r = amdgpu_bo_cpu_unmap(jpeg_bo->handle);
	CU_ASSERT_EQUAL(r, 0);
}

static void free_resource(struct amdgpu_jpeg_bo *jpeg_bo)
{
	int r;

	r = amdgpu_bo_va_op(jpeg_bo->handle, 0, jpeg_bo->size, jpeg_bo->addr, 0,
	                    AMDGPU_VA_OP_UNMAP);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_va_range_free(jpeg_bo->va_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_free(jpeg_bo->handle);
	CU_ASSERT_EQUAL(r, 0);
	memset(jpeg_bo, 0, sizeof(*jpeg_bo));
}

static void set_reg_jpeg(unsigned reg, unsigned cond, unsigned type,
                         uint32_t val)
{
	ib_cpu[len++] = RDECODE_PKTJ(reg, cond, type);
	ib_cpu[len++] = val;
}

/* send a bitstream buffer command */
static void send_cmd_bitstream(uint64_t addr)
{

	/* jpeg soft reset */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 1);

	/* ensuring the Reset is asserted in SCLK domain */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C2);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, 0x01400200);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (1 << 9));
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_SOFT_RESET), COND0, TYPE3, (1 << 9));

	/* wait mem */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 0);

	/* ensuring the Reset is de-asserted in SCLK domain */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (0 << 9));
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_SOFT_RESET), COND0, TYPE3, (1 << 9));

	/* set UVD_LMI_JPEG_READ_64BIT_BAR_LOW/HIGH based on bitstream buffer address */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_LMI_JPEG_READ_64BIT_BAR_HIGH), COND0, TYPE0,
	             (addr >> 32));
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_LMI_JPEG_READ_64BIT_BAR_LOW), COND0, TYPE0,
	             (unsigned int)addr);

	/* set jpeg_rb_base */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_RB_BASE), COND0, TYPE0, 0);

	/* set jpeg_rb_base */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_RB_SIZE), COND0, TYPE0, 0xFFFFFFF0);

	/* set jpeg_rb_wptr */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_RB_WPTR), COND0, TYPE0,
	             (JPEG_DEC_BSD_SIZE >> 2));
}

/* send a target buffer command */
static void send_cmd_target(uint64_t addr)
{

	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_PITCH), COND0, TYPE0,
	             (JPEG_DEC_DT_PITCH >> 4));
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_UV_PITCH), COND0, TYPE0,
	             (JPEG_DEC_DT_PITCH >> 4));

	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_TILING_CTRL), COND0, TYPE0, 0);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_UV_TILING_CTRL), COND0, TYPE0, 0);

	/* set UVD_LMI_JPEG_WRITE_64BIT_BAR_LOW/HIGH based on target buffer address */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH), COND0,
	             TYPE0, (addr >> 32));
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW), COND0, TYPE0,
	             (unsigned int)addr);

	/* set output buffer data address */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_INDEX), COND0, TYPE0, 0);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_DATA), COND0, TYPE0,
	             JPEG_DEC_LUMA_OFFSET);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_INDEX), COND0, TYPE0, 1);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_DATA), COND0, TYPE0,
	             JPEG_DEC_CHROMA_OFFSET);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_TIER_CNTL2), COND0, TYPE3, 0);

	/* set output buffer read pointer */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_OUTBUF_RPTR), COND0, TYPE0, 0);

	/* enable error interrupts */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_INT_EN), COND0, TYPE0, 0xFFFFFFFE);

	/* start engine command */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 0x6);

	/* wait for job completion, wait for job JBSI fetch done */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0,
	             (JPEG_DEC_BSD_SIZE >> 2));
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C2);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, 0x01400200);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_RB_RPTR), COND0, TYPE3, 0xFFFFFFFF);

	/* wait for job jpeg outbuf idle */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, 0xFFFFFFFF);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_OUTBUF_WPTR), COND0, TYPE3,
	             0x00000001);

	/* stop engine */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 0x4);

	/* asserting jpeg lmi drop */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x0005);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0,
	             (1 << 23 | 1 << 0));
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE1, 0);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, 0);

	/* asserting jpeg reset */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 1);

	/* ensure reset is asserted in sclk domain */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (1 << 9));
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_SOFT_RESET), COND0, TYPE3, (1 << 9));

	/* de-assert jpeg reset */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_JPEG_CNTL), COND0, TYPE0, 0);

	/* ensure reset is de-asserted in sclk domain */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x01C3);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, (0 << 9));
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_SOFT_RESET), COND0, TYPE3, (1 << 9));

	/* de-asserting jpeg lmi drop */
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_INDEX), COND0, TYPE0, 0x0005);
	set_reg_jpeg(SOC15_REG_ADDR(mmUVD_CTX_DATA), COND0, TYPE0, 0);
}

/* send a bitstream buffer command */
static void send_cmd_bitstream_direct(uint64_t addr)
{

	/* jpeg soft reset */
	set_reg_jpeg(vcnipUVD_JPEG_DEC_SOFT_RST, COND0, TYPE0, 1);

	/* ensuring the Reset is asserted in SCLK domain */
	set_reg_jpeg(vcnipUVD_JRBC_IB_COND_RD_TIMER, COND0, TYPE0, 0x01400200);
	set_reg_jpeg(vcnipUVD_JRBC_IB_REF_DATA, COND0, TYPE0, (0x1 << 0x10));
	set_reg_jpeg(vcnipUVD_JPEG_DEC_SOFT_RST, COND3, TYPE3, (0x1 << 0x10));

	/* wait mem */
	set_reg_jpeg(vcnipUVD_JPEG_DEC_SOFT_RST, COND0, TYPE0, 0);

	/* ensuring the Reset is de-asserted in SCLK domain */
	set_reg_jpeg(vcnipUVD_JRBC_IB_REF_DATA, COND0, TYPE0, (0 << 0x10));
	set_reg_jpeg(vcnipUVD_JPEG_DEC_SOFT_RST, COND3, TYPE3, (0x1 << 0x10));

	/* set UVD_LMI_JPEG_READ_64BIT_BAR_LOW/HIGH based on bitstream buffer address */
	set_reg_jpeg(vcnipUVD_LMI_JPEG_READ_64BIT_BAR_HIGH, COND0, TYPE0,
	             (addr >> 32));
	set_reg_jpeg(vcnipUVD_LMI_JPEG_READ_64BIT_BAR_LOW, COND0, TYPE0, addr);

	/* set jpeg_rb_base */
	set_reg_jpeg(vcnipUVD_JPEG_RB_BASE, COND0, TYPE0, 0);

	/* set jpeg_rb_base */
	set_reg_jpeg(vcnipUVD_JPEG_RB_SIZE, COND0, TYPE0, 0xFFFFFFF0);

	/* set jpeg_rb_wptr */
	set_reg_jpeg(vcnipUVD_JPEG_RB_WPTR, COND0, TYPE0, (JPEG_DEC_BSD_SIZE >> 2));
}

/* send a target buffer command */
static void send_cmd_target_direct(uint64_t addr)
{

	set_reg_jpeg(vcnipUVD_JPEG_PITCH, COND0, TYPE0, (JPEG_DEC_DT_PITCH >> 4));
	set_reg_jpeg(vcnipUVD_JPEG_UV_PITCH, COND0, TYPE0, (JPEG_DEC_DT_PITCH >> 4));

	set_reg_jpeg(vcnipJPEG_DEC_ADDR_MODE, COND0, TYPE0, 0);
	set_reg_jpeg(vcnipJPEG_DEC_Y_GFX10_TILING_SURFACE, COND0, TYPE0, 0);
	set_reg_jpeg(vcnipJPEG_DEC_UV_GFX10_TILING_SURFACE, COND0, TYPE0, 0);

	/* set UVD_LMI_JPEG_WRITE_64BIT_BAR_LOW/HIGH based on target buffer address */
	set_reg_jpeg(vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_HIGH, COND0, TYPE0,
	             (addr >> 32));
	set_reg_jpeg(vcnipUVD_LMI_JPEG_WRITE_64BIT_BAR_LOW, COND0, TYPE0, addr);

	/* set output buffer data address */
	set_reg_jpeg(vcnipUVD_JPEG_INDEX, COND0, TYPE0, 0);
	set_reg_jpeg(vcnipUVD_JPEG_DATA, COND0, TYPE0, JPEG_DEC_LUMA_OFFSET);
	set_reg_jpeg(vcnipUVD_JPEG_INDEX, COND0, TYPE0, 1);
	set_reg_jpeg(vcnipUVD_JPEG_DATA, COND0, TYPE0, JPEG_DEC_CHROMA_OFFSET);
	set_reg_jpeg(vcnipUVD_JPEG_TIER_CNTL2, COND0, 0, 0);

	/* set output buffer read pointer */
	set_reg_jpeg(vcnipUVD_JPEG_OUTBUF_RPTR, COND0, TYPE0, 0);
	set_reg_jpeg(vcnipUVD_JPEG_OUTBUF_CNTL, COND0, TYPE0,
	             ((0x00001587 & (~0x00000180L)) | (0x1 << 0x7) | (0x1 << 0x6)));

	/* enable error interrupts */
	set_reg_jpeg(vcnipUVD_JPEG_INT_EN, COND0, TYPE0, 0xFFFFFFFE);

	/* start engine command */
	set_reg_jpeg(vcnipUVD_JPEG_CNTL, COND0, TYPE0, 0xE);

	/* wait for job completion, wait for job JBSI fetch done */
	set_reg_jpeg(vcnipUVD_JRBC_IB_REF_DATA, COND0, TYPE0,
	             (JPEG_DEC_BSD_SIZE >> 2));
	set_reg_jpeg(vcnipUVD_JRBC_IB_COND_RD_TIMER, COND0, TYPE0, 0x01400200);
	set_reg_jpeg(vcnipUVD_JPEG_RB_RPTR, COND3, TYPE3, 0xFFFFFFFF);

	/* wait for job jpeg outbuf idle */
	set_reg_jpeg(vcnipUVD_JRBC_IB_REF_DATA, COND0, TYPE0, 0xFFFFFFFF);
	set_reg_jpeg(vcnipUVD_JPEG_OUTBUF_WPTR, COND3, TYPE3, 0x00000001);

	/* stop engine */
	set_reg_jpeg(vcnipUVD_JPEG_CNTL, COND0, TYPE0, 0x4);
}

static void amdgpu_cs_jpeg_decode(void)
{

	struct amdgpu_jpeg_bo dec_buf;
	int size, r;
	uint8_t *dec;
	int sum = 0, i, j;

	size = 16 * 1024; /* 8K bitstream + 8K output */
	num_resources = 0;
	alloc_resource(&dec_buf, size, AMDGPU_GEM_DOMAIN_VRAM);
	resources[num_resources++] = dec_buf.handle;
	resources[num_resources++] = ib_handle;
	r = amdgpu_bo_cpu_map(dec_buf.handle, (void **)&dec_buf.ptr);
	CU_ASSERT_EQUAL(r, 0);
	memcpy(dec_buf.ptr, jpeg_bitstream, sizeof(jpeg_bitstream));

	len = 0;

	if (jpeg_direct_reg == true) {
		send_cmd_bitstream_direct(dec_buf.addr);
		send_cmd_target_direct(dec_buf.addr + (size / 2));
	} else {
		send_cmd_bitstream(dec_buf.addr);
		send_cmd_target(dec_buf.addr + (size / 2));
	}

	amdgpu_bo_cpu_unmap(dec_buf.handle);
	r = submit(len, AMDGPU_HW_IP_VCN_JPEG);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_cpu_map(dec_buf.handle, (void **)&dec_buf.ptr);
	CU_ASSERT_EQUAL(r, 0);

	dec = dec_buf.ptr + (size / 2);

	/* calculate result checksum */
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			sum += *((dec + JPEG_DEC_LUMA_OFFSET + i * JPEG_DEC_DT_PITCH) + j);
	for (i = 0; i < 4; i++)
		for (j = 0; j < 8; j++)
			sum += *((dec + JPEG_DEC_CHROMA_OFFSET + i * JPEG_DEC_DT_PITCH) + j);

	amdgpu_bo_cpu_unmap(dec_buf.handle);
	CU_ASSERT_EQUAL(sum, JPEG_DEC_SUM);

	free_resource(&dec_buf);
}
