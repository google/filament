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

#include <stdio.h>
#include <inttypes.h>

#include "CUnit/Basic.h"

#include "util_math.h"

#include "amdgpu_test.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"
#include "frame.h"
#include "uve_ib.h"

#define IB_SIZE		4096
#define MAX_RESOURCES	16

struct amdgpu_uvd_enc_bo {
	amdgpu_bo_handle handle;
	amdgpu_va_handle va_handle;
	uint64_t addr;
	uint64_t size;
	uint8_t *ptr;
};

struct amdgpu_uvd_enc {
	unsigned width;
	unsigned height;
	struct amdgpu_uvd_enc_bo session;
	struct amdgpu_uvd_enc_bo vbuf;
	struct amdgpu_uvd_enc_bo bs;
	struct amdgpu_uvd_enc_bo fb;
	struct amdgpu_uvd_enc_bo cpb;
};

static amdgpu_device_handle device_handle;
static uint32_t major_version;
static uint32_t minor_version;
static uint32_t family_id;

static amdgpu_context_handle context_handle;
static amdgpu_bo_handle ib_handle;
static amdgpu_va_handle ib_va_handle;
static uint64_t ib_mc_address;
static uint32_t *ib_cpu;

static struct amdgpu_uvd_enc enc;
static amdgpu_bo_handle resources[MAX_RESOURCES];
static unsigned num_resources;

static void amdgpu_cs_uvd_enc_create(void);
static void amdgpu_cs_uvd_enc_session_init(void);
static void amdgpu_cs_uvd_enc_encode(void);
static void amdgpu_cs_uvd_enc_destroy(void);


CU_TestInfo uvd_enc_tests[] = {
	{ "UVD ENC create",  amdgpu_cs_uvd_enc_create },
	{ "UVD ENC session init",  amdgpu_cs_uvd_enc_session_init },
	{ "UVD ENC encode",  amdgpu_cs_uvd_enc_encode },
	{ "UVD ENC destroy",  amdgpu_cs_uvd_enc_destroy },
	CU_TEST_INFO_NULL,
};

CU_BOOL suite_uvd_enc_tests_enable(void)
{
	int r;
	struct drm_amdgpu_info_hw_ip info;

	if (amdgpu_device_initialize(drm_amdgpu[0], &major_version,
					     &minor_version, &device_handle))
		return CU_FALSE;

	r = amdgpu_query_hw_ip_info(device_handle, AMDGPU_HW_IP_UVD_ENC, 0, &info);

	if (amdgpu_device_deinitialize(device_handle))
		return CU_FALSE;

	if (!info.available_rings)
		printf("\n\nThe ASIC NOT support UVD ENC, suite disabled.\n");

	return (r == 0 && (info.available_rings ? CU_TRUE : CU_FALSE));
}


int suite_uvd_enc_tests_init(void)
{
	int r;

	r = amdgpu_device_initialize(drm_amdgpu[0], &major_version,
				     &minor_version, &device_handle);
	if (r)
		return CUE_SINIT_FAILED;

	family_id = device_handle->info.family_id;

	r = amdgpu_cs_ctx_create(device_handle, &context_handle);
	if (r)
		return CUE_SINIT_FAILED;

	r = amdgpu_bo_alloc_and_map(device_handle, IB_SIZE, 4096,
				    AMDGPU_GEM_DOMAIN_GTT, 0,
				    &ib_handle, (void**)&ib_cpu,
				    &ib_mc_address, &ib_va_handle);
	if (r)
		return CUE_SINIT_FAILED;

	return CUE_SUCCESS;
}

int suite_uvd_enc_tests_clean(void)
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

static void alloc_resource(struct amdgpu_uvd_enc_bo *uvd_enc_bo,
			unsigned size, unsigned domain)
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
	uvd_enc_bo->addr = va;
	uvd_enc_bo->handle = buf_handle;
	uvd_enc_bo->size = req.alloc_size;
	uvd_enc_bo->va_handle = va_handle;
	r = amdgpu_bo_cpu_map(uvd_enc_bo->handle, (void **)&uvd_enc_bo->ptr);
	CU_ASSERT_EQUAL(r, 0);
	memset(uvd_enc_bo->ptr, 0, size);
	r = amdgpu_bo_cpu_unmap(uvd_enc_bo->handle);
	CU_ASSERT_EQUAL(r, 0);
}

static void free_resource(struct amdgpu_uvd_enc_bo *uvd_enc_bo)
{
	int r;

	r = amdgpu_bo_va_op(uvd_enc_bo->handle, 0, uvd_enc_bo->size,
			    uvd_enc_bo->addr, 0, AMDGPU_VA_OP_UNMAP);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_va_range_free(uvd_enc_bo->va_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_free(uvd_enc_bo->handle);
	CU_ASSERT_EQUAL(r, 0);
	memset(uvd_enc_bo, 0, sizeof(*uvd_enc_bo));
}

static void amdgpu_cs_uvd_enc_create(void)
{
	enc.width = 160;
	enc.height = 128;

	num_resources  = 0;
	alloc_resource(&enc.session, 128 * 1024, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc.session.handle;
	resources[num_resources++] = ib_handle;
}

static void check_result(struct amdgpu_uvd_enc *enc)
{
	uint64_t sum;
	uint32_t s = 175602;
	uint32_t *ptr, size;
	int j, r;

	r = amdgpu_bo_cpu_map(enc->fb.handle, (void **)&enc->fb.ptr);
	CU_ASSERT_EQUAL(r, 0);
	ptr = (uint32_t *)enc->fb.ptr;
	size = ptr[6];
	r = amdgpu_bo_cpu_unmap(enc->fb.handle);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_bo_cpu_map(enc->bs.handle, (void **)&enc->bs.ptr);
	CU_ASSERT_EQUAL(r, 0);
	for (j = 0, sum = 0; j < size; ++j)
		sum += enc->bs.ptr[j];
	CU_ASSERT_EQUAL(sum, s);
	r = amdgpu_bo_cpu_unmap(enc->bs.handle);
	CU_ASSERT_EQUAL(r, 0);

}

static void amdgpu_cs_uvd_enc_session_init(void)
{
	int len, r;

	len = 0;
	memcpy((ib_cpu + len), uve_session_info, sizeof(uve_session_info));
	len += sizeof(uve_session_info) / 4;
	ib_cpu[len++] = enc.session.addr >> 32;
	ib_cpu[len++] = enc.session.addr;

	memcpy((ib_cpu + len), uve_task_info, sizeof(uve_task_info));
	len += sizeof(uve_task_info) / 4;
	ib_cpu[len++] = 0x000000d8;
	ib_cpu[len++] = 0x00000000;
	ib_cpu[len++] = 0x00000000;

	memcpy((ib_cpu + len), uve_op_init, sizeof(uve_op_init));
	len += sizeof(uve_op_init) / 4;

	memcpy((ib_cpu + len), uve_session_init, sizeof(uve_session_init));
	len += sizeof(uve_session_init) / 4;

	memcpy((ib_cpu + len), uve_layer_ctrl, sizeof(uve_layer_ctrl));
	len += sizeof(uve_layer_ctrl) / 4;

	memcpy((ib_cpu + len), uve_slice_ctrl, sizeof(uve_slice_ctrl));
	len += sizeof(uve_slice_ctrl) / 4;

	memcpy((ib_cpu + len), uve_spec_misc, sizeof(uve_spec_misc));
	len += sizeof(uve_spec_misc) / 4;

	memcpy((ib_cpu + len), uve_rc_session_init, sizeof(uve_rc_session_init));
	len += sizeof(uve_rc_session_init) / 4;

	memcpy((ib_cpu + len), uve_deblocking_filter, sizeof(uve_deblocking_filter));
	len += sizeof(uve_deblocking_filter) / 4;

	memcpy((ib_cpu + len), uve_quality_params, sizeof(uve_quality_params));
	len += sizeof(uve_quality_params) / 4;

	memcpy((ib_cpu + len), uve_op_init_rc, sizeof(uve_op_init_rc));
	len += sizeof(uve_op_init_rc) / 4;

	memcpy((ib_cpu + len), uve_op_init_rc_vbv_level, sizeof(uve_op_init_rc_vbv_level));
	len += sizeof(uve_op_init_rc_vbv_level) / 4;

	r = submit(len, AMDGPU_HW_IP_UVD_ENC);
	CU_ASSERT_EQUAL(r, 0);
}

static void amdgpu_cs_uvd_enc_encode(void)
{
	int len, r, i;
	uint64_t luma_offset, chroma_offset;
	uint32_t vbuf_size, bs_size = 0x003f4800, cpb_size;
	unsigned align = (family_id >= AMDGPU_FAMILY_AI) ? 256 : 16;
	vbuf_size = ALIGN(enc.width, align) * ALIGN(enc.height, 16) * 1.5;
	cpb_size = vbuf_size * 10;


	num_resources  = 0;
	alloc_resource(&enc.fb, 4096, AMDGPU_GEM_DOMAIN_VRAM);
	resources[num_resources++] = enc.fb.handle;
	alloc_resource(&enc.bs, bs_size, AMDGPU_GEM_DOMAIN_VRAM);
	resources[num_resources++] = enc.bs.handle;
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

	len = 0;
	memcpy((ib_cpu + len), uve_session_info, sizeof(uve_session_info));
	len += sizeof(uve_session_info) / 4;
	ib_cpu[len++] = enc.session.addr >> 32;
	ib_cpu[len++] = enc.session.addr;

	memcpy((ib_cpu + len), uve_task_info, sizeof(uve_task_info));
	len += sizeof(uve_task_info) / 4;
	ib_cpu[len++] = 0x000005e0;
	ib_cpu[len++] = 0x00000001;
	ib_cpu[len++] = 0x00000001;

	memcpy((ib_cpu + len), uve_nalu_buffer_1, sizeof(uve_nalu_buffer_1));
	len += sizeof(uve_nalu_buffer_1) / 4;

	memcpy((ib_cpu + len), uve_nalu_buffer_2, sizeof(uve_nalu_buffer_2));
	len += sizeof(uve_nalu_buffer_2) / 4;

	memcpy((ib_cpu + len), uve_nalu_buffer_3, sizeof(uve_nalu_buffer_3));
	len += sizeof(uve_nalu_buffer_3) / 4;

	memcpy((ib_cpu + len), uve_nalu_buffer_4, sizeof(uve_nalu_buffer_4));
	len += sizeof(uve_nalu_buffer_4) / 4;

	memcpy((ib_cpu + len), uve_slice_header, sizeof(uve_slice_header));
	len += sizeof(uve_slice_header) / 4;

	ib_cpu[len++] = 0x00000254;
	ib_cpu[len++] = 0x00000010;
	ib_cpu[len++] = enc.cpb.addr >> 32;
	ib_cpu[len++] = enc.cpb.addr;
	memcpy((ib_cpu + len), uve_ctx_buffer, sizeof(uve_ctx_buffer));
	len += sizeof(uve_ctx_buffer) / 4;

	memcpy((ib_cpu + len), uve_bitstream_buffer, sizeof(uve_bitstream_buffer));
	len += sizeof(uve_bitstream_buffer) / 4;
	ib_cpu[len++] = 0x00000000;
	ib_cpu[len++] = enc.bs.addr >> 32;
	ib_cpu[len++] = enc.bs.addr;
	ib_cpu[len++] = 0x003f4800;
	ib_cpu[len++] = 0x00000000;

	memcpy((ib_cpu + len), uve_feedback_buffer, sizeof(uve_feedback_buffer));
	len += sizeof(uve_feedback_buffer) / 4;
	ib_cpu[len++] = enc.fb.addr >> 32;
	ib_cpu[len++] = enc.fb.addr;
	ib_cpu[len++] = 0x00000010;
	ib_cpu[len++] = 0x00000028;

	memcpy((ib_cpu + len), uve_feedback_buffer_additional, sizeof(uve_feedback_buffer_additional));
	len += sizeof(uve_feedback_buffer_additional) / 4;

	memcpy((ib_cpu + len), uve_intra_refresh, sizeof(uve_intra_refresh));
	len += sizeof(uve_intra_refresh) / 4;

	memcpy((ib_cpu + len), uve_layer_select, sizeof(uve_layer_select));
	len += sizeof(uve_layer_select) / 4;

	memcpy((ib_cpu + len), uve_rc_layer_init, sizeof(uve_rc_layer_init));
	len += sizeof(uve_rc_layer_init) / 4;

	memcpy((ib_cpu + len), uve_layer_select, sizeof(uve_layer_select));
	len += sizeof(uve_layer_select) / 4;

	memcpy((ib_cpu + len), uve_rc_per_pic, sizeof(uve_rc_per_pic));
	len += sizeof(uve_rc_per_pic) / 4;

	unsigned luma_size = ALIGN(enc.width, align) * ALIGN(enc.height, 16);
	luma_offset = enc.vbuf.addr;
	chroma_offset = luma_offset + luma_size;
	ib_cpu[len++] = 0x00000054;
	ib_cpu[len++] = 0x0000000c;
	ib_cpu[len++] = 0x00000002;
	ib_cpu[len++] = 0x003f4800;
	ib_cpu[len++] = luma_offset >> 32;
	ib_cpu[len++] = luma_offset;
	ib_cpu[len++] = chroma_offset >> 32;
	ib_cpu[len++] = chroma_offset;
	memcpy((ib_cpu + len), uve_encode_param, sizeof(uve_encode_param));
	ib_cpu[len] = ALIGN(enc.width, align);
	ib_cpu[len + 1] = ALIGN(enc.width, align);
	len += sizeof(uve_encode_param) / 4;

	memcpy((ib_cpu + len), uve_op_speed_enc_mode, sizeof(uve_op_speed_enc_mode));
	len += sizeof(uve_op_speed_enc_mode) / 4;

	memcpy((ib_cpu + len), uve_op_encode, sizeof(uve_op_encode));
	len += sizeof(uve_op_encode) / 4;

	r = submit(len, AMDGPU_HW_IP_UVD_ENC);
	CU_ASSERT_EQUAL(r, 0);

	check_result(&enc);

	free_resource(&enc.fb);
	free_resource(&enc.bs);
	free_resource(&enc.vbuf);
	free_resource(&enc.cpb);
}

static void amdgpu_cs_uvd_enc_destroy(void)
{
	int len, r;

	num_resources  = 0;
	resources[num_resources++] = ib_handle;

	len = 0;
	memcpy((ib_cpu + len), uve_session_info, sizeof(uve_session_info));
	len += sizeof(uve_session_info) / 4;
	ib_cpu[len++] = enc.session.addr >> 32;
	ib_cpu[len++] = enc.session.addr;

	memcpy((ib_cpu + len), uve_task_info, sizeof(uve_task_info));
	len += sizeof(uve_task_info) / 4;
	ib_cpu[len++] = 0xffffffff;
	ib_cpu[len++] = 0x00000002;
	ib_cpu[len++] = 0x00000000;

	memcpy((ib_cpu + len), uve_op_close, sizeof(uve_op_close));
	len += sizeof(uve_op_close) / 4;

	r = submit(len, AMDGPU_HW_IP_UVD_ENC);
	CU_ASSERT_EQUAL(r, 0);

	free_resource(&enc.session);
}
