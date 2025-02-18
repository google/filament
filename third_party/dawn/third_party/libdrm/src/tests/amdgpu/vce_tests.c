/*
 * Copyright 2015 Advanced Micro Devices, Inc.
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

#include <stdio.h>
#include <inttypes.h>

#include "CUnit/Basic.h"

#include "util_math.h"

#include "amdgpu_test.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"

#include "vce_ib.h"
#include "frame.h"

#define IB_SIZE		4096
#define MAX_RESOURCES	16
#define FW_53_0_03 ((53 << 24) | (0 << 16) | (03 << 8))

struct amdgpu_vce_bo {
	amdgpu_bo_handle handle;
	amdgpu_va_handle va_handle;
	uint64_t addr;
	uint64_t size;
	uint8_t *ptr;
};

struct amdgpu_vce_encode {
	unsigned width;
	unsigned height;
	struct amdgpu_vce_bo vbuf;
	struct amdgpu_vce_bo bs[2];
	struct amdgpu_vce_bo fb[2];
	struct amdgpu_vce_bo cpb;
	unsigned ib_len;
	bool two_instance;
	struct amdgpu_vce_bo mvrefbuf;
	struct amdgpu_vce_bo mvb;
	unsigned mvbuf_size;
};

static amdgpu_device_handle device_handle;
static uint32_t major_version;
static uint32_t minor_version;
static uint32_t family_id;
static uint32_t vce_harvest_config;
static uint32_t chip_rev;
static uint32_t chip_id;
static uint32_t ids_flags;
static bool is_mv_supported = true;

static amdgpu_context_handle context_handle;
static amdgpu_bo_handle ib_handle;
static amdgpu_va_handle ib_va_handle;
static uint64_t ib_mc_address;
static uint32_t *ib_cpu;

static struct amdgpu_vce_encode enc;
static amdgpu_bo_handle resources[MAX_RESOURCES];
static unsigned num_resources;

static void amdgpu_cs_vce_create(void);
static void amdgpu_cs_vce_encode(void);
static void amdgpu_cs_vce_encode_mv(void);
static void amdgpu_cs_vce_destroy(void);

CU_TestInfo vce_tests[] = {
	{ "VCE create",  amdgpu_cs_vce_create },
	{ "VCE encode",  amdgpu_cs_vce_encode },
	{ "VCE MV dump",  amdgpu_cs_vce_encode_mv },
	{ "VCE destroy",  amdgpu_cs_vce_destroy },
	CU_TEST_INFO_NULL,
};

CU_BOOL suite_vce_tests_enable(void)
{
	uint32_t version, feature;
	CU_BOOL ret_mv = CU_FALSE;

	if (amdgpu_device_initialize(drm_amdgpu[0], &major_version,
					     &minor_version, &device_handle))
		return CU_FALSE;

	family_id = device_handle->info.family_id;
	chip_rev = device_handle->info.chip_rev;
	chip_id = device_handle->info.chip_external_rev;
	ids_flags = device_handle->info.ids_flags;

	amdgpu_query_firmware_version(device_handle, AMDGPU_INFO_FW_VCE, 0,
					  0, &version, &feature);

	if (amdgpu_device_deinitialize(device_handle))
		return CU_FALSE;

	if (family_id >= AMDGPU_FAMILY_RV || family_id == AMDGPU_FAMILY_SI ||
		asic_is_gfx_pipe_removed(family_id, chip_id, chip_rev)) {
		printf("\n\nThe ASIC NOT support VCE, suite disabled\n");
		return CU_FALSE;
	}

	if (!(chip_id == (chip_rev + 0x3C) || /* FIJI */
			chip_id == (chip_rev + 0x50) || /* Polaris 10*/
			chip_id == (chip_rev + 0x5A) || /* Polaris 11*/
			chip_id == (chip_rev + 0x64) || /* Polaris 12*/
			(family_id >= AMDGPU_FAMILY_AI && !ids_flags))) /* dGPU > Polaris */
		printf("\n\nThe ASIC NOT support VCE MV, suite disabled\n");
	else if (FW_53_0_03 > version)
		printf("\n\nThe ASIC FW version NOT support VCE MV, suite disabled\n");
	else
		ret_mv = CU_TRUE;

	if (ret_mv == CU_FALSE) {
		amdgpu_set_test_active("VCE Tests", "VCE MV dump", ret_mv);
		is_mv_supported = false;
	}

	return CU_TRUE;
}

int suite_vce_tests_init(void)
{
	int r;

	r = amdgpu_device_initialize(drm_amdgpu[0], &major_version,
				     &minor_version, &device_handle);
	if (r) {
		if ((r == -EACCES) && (errno == EACCES))
			printf("\n\nError:%s. "
				"Hint:Try to run this test program as root.",
				strerror(errno));

		return CUE_SINIT_FAILED;
	}

	family_id = device_handle->info.family_id;
	vce_harvest_config = device_handle->info.vce_harvest_config;

	r = amdgpu_cs_ctx_create(device_handle, &context_handle);
	if (r)
		return CUE_SINIT_FAILED;

	r = amdgpu_bo_alloc_and_map(device_handle, IB_SIZE, 4096,
				    AMDGPU_GEM_DOMAIN_GTT, 0,
				    &ib_handle, (void**)&ib_cpu,
				    &ib_mc_address, &ib_va_handle);
	if (r)
		return CUE_SINIT_FAILED;

	memset(&enc, 0, sizeof(struct amdgpu_vce_encode));

	return CUE_SUCCESS;
}

int suite_vce_tests_clean(void)
{
	int r;

	r = amdgpu_bo_unmap_and_free(ib_handle, ib_va_handle,
				     ib_mc_address, IB_SIZE);
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

	r = amdgpu_bo_list_create(device_handle, num_resources, resources,
				  NULL, &ibs_request.resources);
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

	r = amdgpu_cs_query_fence_status(&fence_status,
					 AMDGPU_TIMEOUT_INFINITE,
					 0, &expired);
	if (r)
		return r;

	return 0;
}

static void alloc_resource(struct amdgpu_vce_bo *vce_bo, unsigned size, unsigned domain)
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
	r = amdgpu_va_range_alloc(device_handle,
				  amdgpu_gpu_va_range_general,
				  req.alloc_size, 1, 0, &va,
				  &va_handle, 0);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_bo_va_op(buf_handle, 0, req.alloc_size, va, 0,
			    AMDGPU_VA_OP_MAP);
	CU_ASSERT_EQUAL(r, 0);
	vce_bo->addr = va;
	vce_bo->handle = buf_handle;
	vce_bo->size = req.alloc_size;
	vce_bo->va_handle = va_handle;
	r = amdgpu_bo_cpu_map(vce_bo->handle, (void **)&vce_bo->ptr);
	CU_ASSERT_EQUAL(r, 0);
	memset(vce_bo->ptr, 0, size);
	r = amdgpu_bo_cpu_unmap(vce_bo->handle);
	CU_ASSERT_EQUAL(r, 0);
}

static void free_resource(struct amdgpu_vce_bo *vce_bo)
{
	int r;

	r = amdgpu_bo_va_op(vce_bo->handle, 0, vce_bo->size,
			    vce_bo->addr, 0, AMDGPU_VA_OP_UNMAP);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_va_range_free(vce_bo->va_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_free(vce_bo->handle);
	CU_ASSERT_EQUAL(r, 0);
	memset(vce_bo, 0, sizeof(*vce_bo));
}

static void amdgpu_cs_vce_create(void)
{
	unsigned align = (family_id >= AMDGPU_FAMILY_AI) ? 256 : 16;
	int len, r;

	enc.width = vce_create[6];
	enc.height = vce_create[7];

	num_resources  = 0;
	alloc_resource(&enc.fb[0], 4096, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.fb[0].handle;
	resources[num_resources++] = ib_handle;

	len = 0;
	memcpy(ib_cpu, vce_session, sizeof(vce_session));
	len += sizeof(vce_session) / 4;
	memcpy((ib_cpu + len), vce_taskinfo, sizeof(vce_taskinfo));
	len += sizeof(vce_taskinfo) / 4;
	memcpy((ib_cpu + len), vce_create, sizeof(vce_create));
	ib_cpu[len + 8] = ALIGN(enc.width, align);
	ib_cpu[len + 9] = ALIGN(enc.width, align);
	if (is_mv_supported == true) {/* disableTwoInstance */
		if (family_id >= AMDGPU_FAMILY_AI)
			ib_cpu[len + 11] = 0x01000001;
		else
			ib_cpu[len + 11] = 0x01000201;
	}
	len += sizeof(vce_create) / 4;
	memcpy((ib_cpu + len), vce_feedback, sizeof(vce_feedback));
	ib_cpu[len + 2] = enc.fb[0].addr >> 32;
	ib_cpu[len + 3] = enc.fb[0].addr;
	len += sizeof(vce_feedback) / 4;

	r = submit(len, AMDGPU_HW_IP_VCE);
	CU_ASSERT_EQUAL(r, 0);

	free_resource(&enc.fb[0]);
}

static void amdgpu_cs_vce_config(void)
{
	int len = 0, r;

	memcpy((ib_cpu + len), vce_session, sizeof(vce_session));
	len += sizeof(vce_session) / 4;
	memcpy((ib_cpu + len), vce_taskinfo, sizeof(vce_taskinfo));
	ib_cpu[len + 3] = 2;
	ib_cpu[len + 6] = 0xffffffff;
	len += sizeof(vce_taskinfo) / 4;
	memcpy((ib_cpu + len), vce_rate_ctrl, sizeof(vce_rate_ctrl));
	len += sizeof(vce_rate_ctrl) / 4;
	memcpy((ib_cpu + len), vce_config_ext, sizeof(vce_config_ext));
	len += sizeof(vce_config_ext) / 4;
	memcpy((ib_cpu + len), vce_motion_est, sizeof(vce_motion_est));
	len += sizeof(vce_motion_est) / 4;
	memcpy((ib_cpu + len), vce_rdo, sizeof(vce_rdo));
	len += sizeof(vce_rdo) / 4;
	memcpy((ib_cpu + len), vce_pic_ctrl, sizeof(vce_pic_ctrl));
	if (is_mv_supported == true)
		ib_cpu[len + 27] = 0x00000001; /* encSliceMode */
	len += sizeof(vce_pic_ctrl) / 4;

	r = submit(len, AMDGPU_HW_IP_VCE);
	CU_ASSERT_EQUAL(r, 0);
}

static void amdgpu_cs_vce_encode_idr(struct amdgpu_vce_encode *enc)
{

	uint64_t luma_offset, chroma_offset;
	unsigned align = (family_id >= AMDGPU_FAMILY_AI) ? 256 : 16;
	unsigned luma_size = ALIGN(enc->width, align) * ALIGN(enc->height, 16);
	int len = 0, i, r;

	luma_offset = enc->vbuf.addr;
	chroma_offset = luma_offset + luma_size;

	memcpy((ib_cpu + len), vce_session, sizeof(vce_session));
	len += sizeof(vce_session) / 4;
	memcpy((ib_cpu + len), vce_taskinfo, sizeof(vce_taskinfo));
	len += sizeof(vce_taskinfo) / 4;
	memcpy((ib_cpu + len), vce_bs_buffer, sizeof(vce_bs_buffer));
	ib_cpu[len + 2] = enc->bs[0].addr >> 32;
	ib_cpu[len + 3] = enc->bs[0].addr;
	len += sizeof(vce_bs_buffer) / 4;
	memcpy((ib_cpu + len), vce_context_buffer, sizeof(vce_context_buffer));
	ib_cpu[len + 2] = enc->cpb.addr >> 32;
	ib_cpu[len + 3] = enc->cpb.addr;
	len += sizeof(vce_context_buffer) / 4;
	memcpy((ib_cpu + len), vce_aux_buffer, sizeof(vce_aux_buffer));
	for (i = 0; i <  8; ++i)
		ib_cpu[len + 2 + i] = luma_size * 1.5 * (i + 2);
	for (i = 0; i <  8; ++i)
		ib_cpu[len + 10 + i] = luma_size * 1.5;
	len += sizeof(vce_aux_buffer) / 4;
	memcpy((ib_cpu + len), vce_feedback, sizeof(vce_feedback));
	ib_cpu[len + 2] = enc->fb[0].addr >> 32;
	ib_cpu[len + 3] = enc->fb[0].addr;
	len += sizeof(vce_feedback) / 4;
	memcpy((ib_cpu + len), vce_encode, sizeof(vce_encode));
	ib_cpu[len + 9] = luma_offset >> 32;
	ib_cpu[len + 10] = luma_offset;
	ib_cpu[len + 11] = chroma_offset >> 32;
	ib_cpu[len + 12] = chroma_offset;
	ib_cpu[len + 14] = ALIGN(enc->width, align);
	ib_cpu[len + 15] = ALIGN(enc->width, align);
	ib_cpu[len + 73] = luma_size * 1.5;
	ib_cpu[len + 74] = luma_size * 2.5;
	len += sizeof(vce_encode) / 4;
	enc->ib_len = len;
	if (!enc->two_instance) {
		r = submit(len, AMDGPU_HW_IP_VCE);
		CU_ASSERT_EQUAL(r, 0);
	}
}

static void amdgpu_cs_vce_encode_p(struct amdgpu_vce_encode *enc)
{
	uint64_t luma_offset, chroma_offset;
	int len, i, r;
	unsigned align = (family_id >= AMDGPU_FAMILY_AI) ? 256 : 16;
	unsigned luma_size = ALIGN(enc->width, align) * ALIGN(enc->height, 16);

	len = (enc->two_instance) ? enc->ib_len : 0;
	luma_offset = enc->vbuf.addr;
	chroma_offset = luma_offset + luma_size;

	if (!enc->two_instance) {
		memcpy((ib_cpu + len), vce_session, sizeof(vce_session));
		len += sizeof(vce_session) / 4;
	}
	memcpy((ib_cpu + len), vce_taskinfo, sizeof(vce_taskinfo));
	len += sizeof(vce_taskinfo) / 4;
	memcpy((ib_cpu + len), vce_bs_buffer, sizeof(vce_bs_buffer));
	ib_cpu[len + 2] = enc->bs[1].addr >> 32;
	ib_cpu[len + 3] = enc->bs[1].addr;
	len += sizeof(vce_bs_buffer) / 4;
	memcpy((ib_cpu + len), vce_context_buffer, sizeof(vce_context_buffer));
	ib_cpu[len + 2] = enc->cpb.addr >> 32;
	ib_cpu[len + 3] = enc->cpb.addr;
	len += sizeof(vce_context_buffer) / 4;
	memcpy((ib_cpu + len), vce_aux_buffer, sizeof(vce_aux_buffer));
	for (i = 0; i <  8; ++i)
		ib_cpu[len + 2 + i] = luma_size * 1.5 * (i + 2);
	for (i = 0; i <  8; ++i)
		ib_cpu[len + 10 + i] = luma_size * 1.5;
	len += sizeof(vce_aux_buffer) / 4;
	memcpy((ib_cpu + len), vce_feedback, sizeof(vce_feedback));
	ib_cpu[len + 2] = enc->fb[1].addr >> 32;
	ib_cpu[len + 3] = enc->fb[1].addr;
	len += sizeof(vce_feedback) / 4;
	memcpy((ib_cpu + len), vce_encode, sizeof(vce_encode));
	ib_cpu[len + 2] = 0;
	ib_cpu[len + 9] = luma_offset >> 32;
	ib_cpu[len + 10] = luma_offset;
	ib_cpu[len + 11] = chroma_offset >> 32;
	ib_cpu[len + 12] = chroma_offset;
	ib_cpu[len + 14] = ALIGN(enc->width, align);
	ib_cpu[len + 15] = ALIGN(enc->width, align);
	ib_cpu[len + 18] = 0;
	ib_cpu[len + 19] = 0;
	ib_cpu[len + 56] = 3;
	ib_cpu[len + 57] = 0;
	ib_cpu[len + 58] = 0;
	ib_cpu[len + 59] = luma_size * 1.5;
	ib_cpu[len + 60] = luma_size * 2.5;
	ib_cpu[len + 73] = 0;
	ib_cpu[len + 74] = luma_size;
	ib_cpu[len + 81] = 1;
	ib_cpu[len + 82] = 1;
	len += sizeof(vce_encode) / 4;

	r = submit(len, AMDGPU_HW_IP_VCE);
	CU_ASSERT_EQUAL(r, 0);
}

static void check_result(struct amdgpu_vce_encode *enc)
{
	uint64_t sum;
	uint32_t s[2] = {180325, 15946};
	uint32_t *ptr, size;
	int i, j, r;

	for (i = 0; i < 2; ++i) {
		r = amdgpu_bo_cpu_map(enc->fb[i].handle, (void **)&enc->fb[i].ptr);
		CU_ASSERT_EQUAL(r, 0);
		ptr = (uint32_t *)enc->fb[i].ptr;
		size = ptr[4] - ptr[9];
		r = amdgpu_bo_cpu_unmap(enc->fb[i].handle);
		CU_ASSERT_EQUAL(r, 0);
		r = amdgpu_bo_cpu_map(enc->bs[i].handle, (void **)&enc->bs[i].ptr);
		CU_ASSERT_EQUAL(r, 0);
		for (j = 0, sum = 0; j < size; ++j)
			sum += enc->bs[i].ptr[j];
		CU_ASSERT_EQUAL(sum, s[i]);
		r = amdgpu_bo_cpu_unmap(enc->bs[i].handle);
		CU_ASSERT_EQUAL(r, 0);
	}
}

static void amdgpu_cs_vce_encode(void)
{
	uint32_t vbuf_size, bs_size = 0x154000, cpb_size;
	unsigned align = (family_id >= AMDGPU_FAMILY_AI) ? 256 : 16;
	int i, r;

	vbuf_size = ALIGN(enc.width, align) * ALIGN(enc.height, 16) * 1.5;
	cpb_size = vbuf_size * 10;
	num_resources = 0;
	alloc_resource(&enc.fb[0], 4096, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.fb[0].handle;
	alloc_resource(&enc.fb[1], 4096, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.fb[1].handle;
	alloc_resource(&enc.bs[0], bs_size, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.bs[0].handle;
	alloc_resource(&enc.bs[1], bs_size, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.bs[1].handle;
	alloc_resource(&enc.vbuf, vbuf_size, AMDGPU_GEM_DOMAIN_VRAM);
	resources[num_resources++] = enc.vbuf.handle;
	alloc_resource(&enc.cpb, cpb_size, AMDGPU_GEM_DOMAIN_VRAM);
	resources[num_resources++] = enc.cpb.handle;
	resources[num_resources++] = ib_handle;

	r = amdgpu_bo_cpu_map(enc.vbuf.handle, (void **)&enc.vbuf.ptr);
	CU_ASSERT_EQUAL(r, 0);

	memset(enc.vbuf.ptr, 0, vbuf_size);
	for (i = 0; i < enc.height; ++i) {
		memcpy(enc.vbuf.ptr, (frame + i * enc.width), enc.width);
		enc.vbuf.ptr += ALIGN(enc.width, align);
	}
	for (i = 0; i < enc.height / 2; ++i) {
		memcpy(enc.vbuf.ptr, ((frame + enc.height * enc.width) + i * enc.width), enc.width);
		enc.vbuf.ptr += ALIGN(enc.width, align);
	}

	r = amdgpu_bo_cpu_unmap(enc.vbuf.handle);
	CU_ASSERT_EQUAL(r, 0);

	amdgpu_cs_vce_config();

	if (family_id >= AMDGPU_FAMILY_VI) {
		vce_taskinfo[3] = 3;
		amdgpu_cs_vce_encode_idr(&enc);
		amdgpu_cs_vce_encode_p(&enc);
		check_result(&enc);

		/* two pipes */
		vce_encode[16] = 0;
		amdgpu_cs_vce_encode_idr(&enc);
		amdgpu_cs_vce_encode_p(&enc);
		check_result(&enc);

		/* two instances */
		if (vce_harvest_config == 0) {
			enc.two_instance = true;
			vce_taskinfo[2] = 0x83;
			vce_taskinfo[4] = 1;
			amdgpu_cs_vce_encode_idr(&enc);
			vce_taskinfo[2] = 0xffffffff;
			vce_taskinfo[4] = 2;
			amdgpu_cs_vce_encode_p(&enc);
			check_result(&enc);
		}
	} else {
		vce_taskinfo[3] = 3;
		vce_encode[16] = 0;
		amdgpu_cs_vce_encode_idr(&enc);
		amdgpu_cs_vce_encode_p(&enc);
		check_result(&enc);
	}

	free_resource(&enc.fb[0]);
	free_resource(&enc.fb[1]);
	free_resource(&enc.bs[0]);
	free_resource(&enc.bs[1]);
	free_resource(&enc.vbuf);
	free_resource(&enc.cpb);
}

static void amdgpu_cs_vce_mv(struct amdgpu_vce_encode *enc)
{
	uint64_t luma_offset, chroma_offset;
	uint64_t mv_ref_luma_offset;
	unsigned align = (family_id >= AMDGPU_FAMILY_AI) ? 256 : 16;
	unsigned luma_size = ALIGN(enc->width, align) * ALIGN(enc->height, 16);
	int len = 0, i, r;

	luma_offset = enc->vbuf.addr;
	chroma_offset = luma_offset + luma_size;
	mv_ref_luma_offset = enc->mvrefbuf.addr;

	memcpy((ib_cpu + len), vce_session, sizeof(vce_session));
	len += sizeof(vce_session) / 4;
	memcpy((ib_cpu + len), vce_taskinfo, sizeof(vce_taskinfo));
	len += sizeof(vce_taskinfo) / 4;
	memcpy((ib_cpu + len), vce_bs_buffer, sizeof(vce_bs_buffer));
	ib_cpu[len + 2] = enc->bs[0].addr >> 32;
	ib_cpu[len + 3] = enc->bs[0].addr;
	len += sizeof(vce_bs_buffer) / 4;
	memcpy((ib_cpu + len), vce_context_buffer, sizeof(vce_context_buffer));
	ib_cpu[len + 2] = enc->cpb.addr >> 32;
	ib_cpu[len + 3] = enc->cpb.addr;
	len += sizeof(vce_context_buffer) / 4;
	memcpy((ib_cpu + len), vce_aux_buffer, sizeof(vce_aux_buffer));
	for (i = 0; i <  8; ++i)
		ib_cpu[len + 2 + i] = luma_size * 1.5 * (i + 2);
	for (i = 0; i <  8; ++i)
		ib_cpu[len + 10 + i] = luma_size * 1.5;
	len += sizeof(vce_aux_buffer) / 4;
	memcpy((ib_cpu + len), vce_feedback, sizeof(vce_feedback));
	ib_cpu[len + 2] = enc->fb[0].addr >> 32;
	ib_cpu[len + 3] = enc->fb[0].addr;
	len += sizeof(vce_feedback) / 4;
	memcpy((ib_cpu + len), vce_mv_buffer, sizeof(vce_mv_buffer));
	ib_cpu[len + 2] = mv_ref_luma_offset >> 32;
	ib_cpu[len + 3] = mv_ref_luma_offset;
	ib_cpu[len + 4] = ALIGN(enc->width, align);
	ib_cpu[len + 5] = ALIGN(enc->width, align);
	ib_cpu[len + 6] = luma_size;
	ib_cpu[len + 7] = enc->mvb.addr >> 32;
	ib_cpu[len + 8] = enc->mvb.addr;
	len += sizeof(vce_mv_buffer) / 4;
	memcpy((ib_cpu + len), vce_encode, sizeof(vce_encode));
	ib_cpu[len + 2] = 0;
	ib_cpu[len + 3] = 0;
	ib_cpu[len + 4] = 0x154000;
	ib_cpu[len + 9] = luma_offset >> 32;
	ib_cpu[len + 10] = luma_offset;
	ib_cpu[len + 11] = chroma_offset >> 32;
	ib_cpu[len + 12] = chroma_offset;
	ib_cpu[len + 13] = ALIGN(enc->height, 16);;
	ib_cpu[len + 14] = ALIGN(enc->width, align);
	ib_cpu[len + 15] = ALIGN(enc->width, align);
	/* encDisableMBOffloading-encDisableTwoPipeMode-encInputPicArrayMode-encInputPicAddrMode */
	ib_cpu[len + 16] = 0x01010000;
	ib_cpu[len + 18] = 0; /* encPicType */
	ib_cpu[len + 19] = 0; /* encIdrFlag */
	ib_cpu[len + 20] = 0; /* encIdrPicId */
	ib_cpu[len + 21] = 0; /* encMGSKeyPic */
	ib_cpu[len + 22] = 0; /* encReferenceFlag */
	ib_cpu[len + 23] = 0; /* encTemporalLayerIndex */
	ib_cpu[len + 55] = 0; /* pictureStructure */
	ib_cpu[len + 56] = 0; /* encPicType -ref[0] */
	ib_cpu[len + 61] = 0; /* pictureStructure */
	ib_cpu[len + 62] = 0; /* encPicType -ref[1] */
	ib_cpu[len + 67] = 0; /* pictureStructure */
	ib_cpu[len + 68] = 0; /* encPicType -ref1 */
	ib_cpu[len + 81] = 1; /* frameNumber */
	ib_cpu[len + 82] = 2; /* pictureOrderCount */
	ib_cpu[len + 83] = 0xffffffff; /* numIPicRemainInRCGOP */
	ib_cpu[len + 84] = 0xffffffff; /* numPPicRemainInRCGOP */
	ib_cpu[len + 85] = 0xffffffff; /* numBPicRemainInRCGOP */
	ib_cpu[len + 86] = 0xffffffff; /* numIRPicRemainInRCGOP */
	ib_cpu[len + 87] = 0; /* remainedIntraRefreshPictures */
	len += sizeof(vce_encode) / 4;

	enc->ib_len = len;
	r = submit(len, AMDGPU_HW_IP_VCE);
	CU_ASSERT_EQUAL(r, 0);
}

static void check_mv_result(struct amdgpu_vce_encode *enc)
{
	uint64_t sum;
	uint32_t s = 140790;
	int j, r;

	r = amdgpu_bo_cpu_map(enc->fb[0].handle, (void **)&enc->fb[0].ptr);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_bo_cpu_unmap(enc->fb[0].handle);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_bo_cpu_map(enc->mvb.handle, (void **)&enc->mvb.ptr);
	CU_ASSERT_EQUAL(r, 0);
	for (j = 0, sum = 0; j < enc->mvbuf_size; ++j)
		sum += enc->mvb.ptr[j];
	CU_ASSERT_EQUAL(sum, s);
	r = amdgpu_bo_cpu_unmap(enc->mvb.handle);
	CU_ASSERT_EQUAL(r, 0);
}

static void amdgpu_cs_vce_encode_mv(void)
{
	uint32_t vbuf_size, bs_size = 0x154000, cpb_size;
	unsigned align = (family_id >= AMDGPU_FAMILY_AI) ? 256 : 16;
	int i, r;

	vbuf_size = ALIGN(enc.width, align) * ALIGN(enc.height, 16) * 1.5;
	enc.mvbuf_size = ALIGN(enc.width, 16) * ALIGN(enc.height, 16) / 8;
	cpb_size = vbuf_size * 10;
	num_resources = 0;
	alloc_resource(&enc.fb[0], 4096, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.fb[0].handle;
	alloc_resource(&enc.bs[0], bs_size, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.bs[0].handle;
	alloc_resource(&enc.mvb, enc.mvbuf_size, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.mvb.handle;
	alloc_resource(&enc.vbuf, vbuf_size, AMDGPU_GEM_DOMAIN_VRAM);
	resources[num_resources++] = enc.vbuf.handle;
	alloc_resource(&enc.mvrefbuf, vbuf_size, AMDGPU_GEM_DOMAIN_VRAM);
	resources[num_resources++] = enc.mvrefbuf.handle;
	alloc_resource(&enc.cpb, cpb_size, AMDGPU_GEM_DOMAIN_VRAM);
	resources[num_resources++] = enc.cpb.handle;
	resources[num_resources++] = ib_handle;

	r = amdgpu_bo_cpu_map(enc.vbuf.handle, (void **)&enc.vbuf.ptr);
	CU_ASSERT_EQUAL(r, 0);

	memset(enc.vbuf.ptr, 0, vbuf_size);
	for (i = 0; i < enc.height; ++i) {
		memcpy(enc.vbuf.ptr, (frame + i * enc.width), enc.width);
		enc.vbuf.ptr += ALIGN(enc.width, align);
	}
	for (i = 0; i < enc.height / 2; ++i) {
		memcpy(enc.vbuf.ptr, ((frame + enc.height * enc.width) + i * enc.width), enc.width);
		enc.vbuf.ptr += ALIGN(enc.width, align);
	}

	r = amdgpu_bo_cpu_unmap(enc.vbuf.handle);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_cpu_map(enc.mvrefbuf.handle, (void **)&enc.mvrefbuf.ptr);
	CU_ASSERT_EQUAL(r, 0);

	memset(enc.mvrefbuf.ptr, 0, vbuf_size);
	for (i = 0; i < enc.height; ++i) {
		memcpy(enc.mvrefbuf.ptr, (frame + (enc.height - i -1) * enc.width), enc.width);
		enc.mvrefbuf.ptr += ALIGN(enc.width, align);
	}
	for (i = 0; i < enc.height / 2; ++i) {
		memcpy(enc.mvrefbuf.ptr,
		((frame + enc.height * enc.width) + (enc.height / 2 - i -1) * enc.width), enc.width);
		enc.mvrefbuf.ptr += ALIGN(enc.width, align);
	}

	r = amdgpu_bo_cpu_unmap(enc.mvrefbuf.handle);
	CU_ASSERT_EQUAL(r, 0);

	amdgpu_cs_vce_config();

	vce_taskinfo[3] = 3;
	amdgpu_cs_vce_mv(&enc);
	check_mv_result(&enc);

	free_resource(&enc.fb[0]);
	free_resource(&enc.bs[0]);
	free_resource(&enc.vbuf);
	free_resource(&enc.cpb);
	free_resource(&enc.mvrefbuf);
	free_resource(&enc.mvb);
}

static void amdgpu_cs_vce_destroy(void)
{
	int len, r;

	num_resources  = 0;
	alloc_resource(&enc.fb[0], 4096, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.fb[0].handle;
	resources[num_resources++] = ib_handle;

	len = 0;
	memcpy(ib_cpu, vce_session, sizeof(vce_session));
	len += sizeof(vce_session) / 4;
	memcpy((ib_cpu + len), vce_taskinfo, sizeof(vce_taskinfo));
	ib_cpu[len + 3] = 1;
	len += sizeof(vce_taskinfo) / 4;
	memcpy((ib_cpu + len), vce_feedback, sizeof(vce_feedback));
	ib_cpu[len + 2] = enc.fb[0].addr >> 32;
	ib_cpu[len + 3] = enc.fb[0].addr;
	len += sizeof(vce_feedback) / 4;
	memcpy((ib_cpu + len), vce_destroy, sizeof(vce_destroy));
	len += sizeof(vce_destroy) / 4;

	r = submit(len, AMDGPU_HW_IP_VCE);
	CU_ASSERT_EQUAL(r, 0);

	free_resource(&enc.fb[0]);
}
