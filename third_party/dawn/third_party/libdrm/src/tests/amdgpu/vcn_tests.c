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
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

#include "CUnit/Basic.h"

#include <unistd.h>
#include "util_math.h"

#include "amdgpu_test.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"
#include "decode_messages.h"
#include "frame.h"

#define IB_SIZE		4096
#define MAX_RESOURCES	16

#define DECODE_CMD_MSG_BUFFER                              0x00000000
#define DECODE_CMD_DPB_BUFFER                              0x00000001
#define DECODE_CMD_DECODING_TARGET_BUFFER                  0x00000002
#define DECODE_CMD_FEEDBACK_BUFFER                         0x00000003
#define DECODE_CMD_PROB_TBL_BUFFER                         0x00000004
#define DECODE_CMD_SESSION_CONTEXT_BUFFER                  0x00000005
#define DECODE_CMD_BITSTREAM_BUFFER                        0x00000100
#define DECODE_CMD_IT_SCALING_TABLE_BUFFER                 0x00000204
#define DECODE_CMD_CONTEXT_BUFFER                          0x00000206

#define DECODE_IB_PARAM_DECODE_BUFFER                      (0x00000001)

#define DECODE_CMDBUF_FLAGS_MSG_BUFFER                     (0x00000001)
#define DECODE_CMDBUF_FLAGS_DPB_BUFFER                     (0x00000002)
#define DECODE_CMDBUF_FLAGS_BITSTREAM_BUFFER               (0x00000004)
#define DECODE_CMDBUF_FLAGS_DECODING_TARGET_BUFFER         (0x00000008)
#define DECODE_CMDBUF_FLAGS_FEEDBACK_BUFFER                (0x00000010)
#define DECODE_CMDBUF_FLAGS_IT_SCALING_BUFFER              (0x00000200)
#define DECODE_CMDBUF_FLAGS_CONTEXT_BUFFER                 (0x00000800)
#define DECODE_CMDBUF_FLAGS_PROB_TBL_BUFFER                (0x00001000)
#define DECODE_CMDBUF_FLAGS_SESSION_CONTEXT_BUFFER         (0x00100000)

static bool vcn_dec_sw_ring = false;
static bool vcn_unified_ring = false;

#define H264_NAL_TYPE_NON_IDR_SLICE 1
#define H264_NAL_TYPE_DP_A_SLICE 2
#define H264_NAL_TYPE_DP_B_SLICE 3
#define H264_NAL_TYPE_DP_C_SLICE 0x4
#define H264_NAL_TYPE_IDR_SLICE 0x5
#define H264_NAL_TYPE_SEI 0x6
#define H264_NAL_TYPE_SEQ_PARAM 0x7
#define H264_NAL_TYPE_PIC_PARAM 0x8
#define H264_NAL_TYPE_ACCESS_UNIT 0x9
#define H264_NAL_TYPE_END_OF_SEQ 0xa
#define H264_NAL_TYPE_END_OF_STREAM 0xb
#define H264_NAL_TYPE_FILLER_DATA 0xc
#define H264_NAL_TYPE_SEQ_EXTENSION 0xd

#define H264_START_CODE 0x000001

struct amdgpu_vcn_bo {
	amdgpu_bo_handle handle;
	amdgpu_va_handle va_handle;
	uint64_t addr;
	uint64_t size;
	uint8_t *ptr;
};

typedef struct rvcn_decode_buffer_s {
	unsigned int valid_buf_flag;
	unsigned int msg_buffer_address_hi;
	unsigned int msg_buffer_address_lo;
	unsigned int dpb_buffer_address_hi;
	unsigned int dpb_buffer_address_lo;
	unsigned int target_buffer_address_hi;
	unsigned int target_buffer_address_lo;
	unsigned int session_contex_buffer_address_hi;
	unsigned int session_contex_buffer_address_lo;
	unsigned int bitstream_buffer_address_hi;
	unsigned int bitstream_buffer_address_lo;
	unsigned int context_buffer_address_hi;
	unsigned int context_buffer_address_lo;
	unsigned int feedback_buffer_address_hi;
	unsigned int feedback_buffer_address_lo;
	unsigned int luma_hist_buffer_address_hi;
	unsigned int luma_hist_buffer_address_lo;
	unsigned int prob_tbl_buffer_address_hi;
	unsigned int prob_tbl_buffer_address_lo;
	unsigned int sclr_coeff_buffer_address_hi;
	unsigned int sclr_coeff_buffer_address_lo;
	unsigned int it_sclr_table_buffer_address_hi;
	unsigned int it_sclr_table_buffer_address_lo;
	unsigned int sclr_target_buffer_address_hi;
	unsigned int sclr_target_buffer_address_lo;
	unsigned int cenc_size_info_buffer_address_hi;
	unsigned int cenc_size_info_buffer_address_lo;
	unsigned int mpeg2_pic_param_buffer_address_hi;
	unsigned int mpeg2_pic_param_buffer_address_lo;
	unsigned int mpeg2_mb_control_buffer_address_hi;
	unsigned int mpeg2_mb_control_buffer_address_lo;
	unsigned int mpeg2_idct_coeff_buffer_address_hi;
	unsigned int mpeg2_idct_coeff_buffer_address_lo;
} rvcn_decode_buffer_t;

typedef struct rvcn_decode_ib_package_s {
	unsigned int package_size;
	unsigned int package_type;
} rvcn_decode_ib_package_t;


struct amdgpu_vcn_reg {
	uint32_t data0;
	uint32_t data1;
	uint32_t cmd;
	uint32_t nop;
	uint32_t cntl;
};

typedef struct BufferInfo_t {
	uint32_t numOfBitsInBuffer;
	const uint8_t *decBuffer;
	uint8_t decData;
	uint32_t decBufferSize;
	const uint8_t *end;
} bufferInfo;

typedef struct h264_decode_t {
	uint8_t profile;
	uint8_t level_idc;
	uint8_t nal_ref_idc;
	uint8_t nal_unit_type;
	uint32_t pic_width, pic_height;
	uint32_t slice_type;
} h264_decode;

static amdgpu_device_handle device_handle;
static uint32_t major_version;
static uint32_t minor_version;
static uint32_t family_id;
static uint32_t chip_rev;
static uint32_t chip_id;
static uint32_t asic_id;
static uint32_t chip_rev;
static struct amdgpu_vcn_bo enc_buf;
static struct amdgpu_vcn_bo cpb_buf;
static uint32_t enc_task_id;

static amdgpu_context_handle context_handle;
static amdgpu_bo_handle ib_handle;
static amdgpu_va_handle ib_va_handle;
static uint64_t ib_mc_address;
static uint32_t *ib_cpu;
static uint32_t *ib_checksum;
static uint32_t *ib_size_in_dw;

static rvcn_decode_buffer_t *decode_buffer;
struct amdgpu_vcn_bo session_ctx_buf;

static amdgpu_bo_handle resources[MAX_RESOURCES];
static unsigned num_resources;

static uint8_t vcn_reg_index;
static struct amdgpu_vcn_reg reg[] = {
	{0x81c4, 0x81c5, 0x81c3, 0x81ff, 0x81c6},
	{0x504, 0x505, 0x503, 0x53f, 0x506},
	{0x10, 0x11, 0xf, 0x29, 0x26d},
};

uint32_t gWidth, gHeight, gSliceType;
static uint32_t vcn_ip_version_major;
static uint32_t vcn_ip_version_minor;
static void amdgpu_cs_vcn_dec_create(void);
static void amdgpu_cs_vcn_dec_decode(void);
static void amdgpu_cs_vcn_dec_destroy(void);

static void amdgpu_cs_vcn_enc_create(void);
static void amdgpu_cs_vcn_enc_encode(void);
static void amdgpu_cs_vcn_enc_destroy(void);

static void amdgpu_cs_sq_head(uint32_t *base, int *offset, bool enc);
static void amdgpu_cs_sq_ib_tail(uint32_t *end);
static void h264_check_0s (bufferInfo * bufInfo, int count);
static int32_t h264_se (bufferInfo * bufInfo);
static inline uint32_t bs_read_u1(bufferInfo *bufinfo);
static inline int bs_eof(bufferInfo *bufinfo);
static inline uint32_t bs_read_u(bufferInfo* bufinfo, int n);
static inline uint32_t bs_read_ue(bufferInfo* bufinfo);
static uint32_t remove_03 (uint8_t *bptr, uint32_t len);
static void scaling_list (uint32_t ix, uint32_t sizeOfScalingList, bufferInfo *bufInfo);
static void h264_parse_sequence_parameter_set (h264_decode * dec, bufferInfo *bufInfo);
static void h264_slice_header (h264_decode *dec, bufferInfo *bufInfo);
static uint8_t h264_parse_nal (h264_decode *dec, bufferInfo *bufInfo);
static uint32_t h264_find_next_start_code (uint8_t *pBuf, uint32_t bufLen);
static int verify_checksum(uint8_t *buffer, uint32_t buffer_size);

CU_TestInfo vcn_tests[] = {

	{ "VCN DEC create",  amdgpu_cs_vcn_dec_create },
	{ "VCN DEC decode",  amdgpu_cs_vcn_dec_decode },
	{ "VCN DEC destroy",  amdgpu_cs_vcn_dec_destroy },

	{ "VCN ENC create",  amdgpu_cs_vcn_enc_create },
	{ "VCN ENC encode",  amdgpu_cs_vcn_enc_encode },
	{ "VCN ENC destroy",  amdgpu_cs_vcn_enc_destroy },
	CU_TEST_INFO_NULL,
};

CU_BOOL suite_vcn_tests_enable(void)
{
	struct drm_amdgpu_info_hw_ip info;
	bool enc_ring, dec_ring;
	int r;

	if (amdgpu_device_initialize(drm_amdgpu[0], &major_version,
				   &minor_version, &device_handle))
		return CU_FALSE;

	family_id = device_handle->info.family_id;
	asic_id = device_handle->info.asic_id;
	chip_rev = device_handle->info.chip_rev;
	chip_id = device_handle->info.chip_external_rev;

	r = amdgpu_query_hw_ip_info(device_handle, AMDGPU_HW_IP_VCN_ENC, 0, &info);
	if (!r) {
		vcn_ip_version_major = info.hw_ip_version_major;
		vcn_ip_version_minor = info.hw_ip_version_minor;
		enc_ring = !!info.available_rings;
		/* in vcn 4.0 it re-uses encoding queue as unified queue */
		if (vcn_ip_version_major >= 4) {
			vcn_unified_ring = true;
			vcn_dec_sw_ring = true;
			dec_ring = enc_ring;
		} else {
			r = amdgpu_query_hw_ip_info(device_handle, AMDGPU_HW_IP_VCN_DEC, 0, &info);
			dec_ring = !!info.available_rings;
		}
	}

	if (amdgpu_device_deinitialize(device_handle))
		return CU_FALSE;

	if (r) {
		printf("\n\nASIC query hw info failed\n");
		return CU_FALSE;
	}

	if (!(dec_ring || enc_ring) ||
	    (family_id < AMDGPU_FAMILY_RV &&
	     (family_id == AMDGPU_FAMILY_AI &&
	      (chip_id - chip_rev) < 0x32))) {  /* Arcturus */
		printf("\n\nThe ASIC NOT support VCN, suite disabled\n");
		return CU_FALSE;
	}

	if (!dec_ring) {
		amdgpu_set_test_active("VCN Tests", "VCN DEC create", CU_FALSE);
		amdgpu_set_test_active("VCN Tests", "VCN DEC decode", CU_FALSE);
		amdgpu_set_test_active("VCN Tests", "VCN DEC destroy", CU_FALSE);
	}

	if (family_id == AMDGPU_FAMILY_AI || !enc_ring) {
		amdgpu_set_test_active("VCN Tests", "VCN ENC create", CU_FALSE);
		amdgpu_set_test_active("VCN Tests", "VCN ENC encode", CU_FALSE);
		amdgpu_set_test_active("VCN Tests", "VCN ENC destroy", CU_FALSE);
	}

	if (vcn_ip_version_major == 1)
		vcn_reg_index = 0;
	else if (vcn_ip_version_major == 2 && vcn_ip_version_minor == 0)
		vcn_reg_index = 1;
	else if ((vcn_ip_version_major == 2 && vcn_ip_version_minor >= 5) ||
				vcn_ip_version_major == 3)
		vcn_reg_index = 2;

	return CU_TRUE;
}

int suite_vcn_tests_init(void)
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

int suite_vcn_tests_clean(void)
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

static void amdgpu_cs_sq_head(uint32_t *base, int *offset, bool enc)
{
	/* signature */
	*(base + (*offset)++) = 0x00000010;
	*(base + (*offset)++) = 0x30000002;
	ib_checksum = base + (*offset)++;
	ib_size_in_dw = base + (*offset)++;

	/* engine info */
	*(base + (*offset)++) = 0x00000010;
	*(base + (*offset)++) = 0x30000001;
	*(base + (*offset)++) = enc ? 2 : 3;
	*(base + (*offset)++) = 0x00000000;
}

static void amdgpu_cs_sq_ib_tail(uint32_t *end)
{
	uint32_t size_in_dw;
	uint32_t checksum = 0;

	/* if the pointers are invalid, no need to process */
	if (ib_checksum == NULL || ib_size_in_dw == NULL)
		return;

	size_in_dw = end - ib_size_in_dw - 1;
	*ib_size_in_dw = size_in_dw;
	*(ib_size_in_dw + 4) = size_in_dw * sizeof(uint32_t);

	for (int i = 0; i < size_in_dw; i++)
		checksum += *(ib_checksum + 2 + i);

	*ib_checksum = checksum;

	ib_checksum = NULL;
	ib_size_in_dw = NULL;
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

static void alloc_resource(struct amdgpu_vcn_bo *vcn_bo,
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
	vcn_bo->addr = va;
	vcn_bo->handle = buf_handle;
	vcn_bo->size = req.alloc_size;
	vcn_bo->va_handle = va_handle;
	r = amdgpu_bo_cpu_map(vcn_bo->handle, (void **)&vcn_bo->ptr);
	CU_ASSERT_EQUAL(r, 0);
	memset(vcn_bo->ptr, 0, size);
	r = amdgpu_bo_cpu_unmap(vcn_bo->handle);
	CU_ASSERT_EQUAL(r, 0);
}

static void free_resource(struct amdgpu_vcn_bo *vcn_bo)
{
	int r;

	r = amdgpu_bo_va_op(vcn_bo->handle, 0, vcn_bo->size,
			    vcn_bo->addr, 0, AMDGPU_VA_OP_UNMAP);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_va_range_free(vcn_bo->va_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_free(vcn_bo->handle);
	CU_ASSERT_EQUAL(r, 0);
	memset(vcn_bo, 0, sizeof(*vcn_bo));
}

static void vcn_dec_cmd(uint64_t addr, unsigned cmd, int *idx)
{
	if (vcn_dec_sw_ring == false) {
		ib_cpu[(*idx)++] = reg[vcn_reg_index].data0;
		ib_cpu[(*idx)++] = addr;
		ib_cpu[(*idx)++] = reg[vcn_reg_index].data1;
		ib_cpu[(*idx)++] = addr >> 32;
		ib_cpu[(*idx)++] = reg[vcn_reg_index].cmd;
		ib_cpu[(*idx)++] = cmd << 1;
		return;
	}

	/* Support decode software ring message */
	if (!(*idx)) {
		rvcn_decode_ib_package_t *ib_header;

		if (vcn_unified_ring)
			amdgpu_cs_sq_head(ib_cpu, idx, false);

		ib_header = (rvcn_decode_ib_package_t *)&ib_cpu[*idx];
		ib_header->package_size = sizeof(struct rvcn_decode_buffer_s) +
			sizeof(struct rvcn_decode_ib_package_s);

		(*idx)++;
		ib_header->package_type = (DECODE_IB_PARAM_DECODE_BUFFER);
		(*idx)++;

		decode_buffer = (rvcn_decode_buffer_t *)&(ib_cpu[*idx]);
		*idx += sizeof(struct rvcn_decode_buffer_s) / 4;
		memset(decode_buffer, 0, sizeof(struct rvcn_decode_buffer_s));
	}

	switch(cmd) {
		case DECODE_CMD_MSG_BUFFER:
			decode_buffer->valid_buf_flag |= DECODE_CMDBUF_FLAGS_MSG_BUFFER;
			decode_buffer->msg_buffer_address_hi = (addr >> 32);
			decode_buffer->msg_buffer_address_lo = (addr);
		break;
		case DECODE_CMD_DPB_BUFFER:
			decode_buffer->valid_buf_flag |= (DECODE_CMDBUF_FLAGS_DPB_BUFFER);
			decode_buffer->dpb_buffer_address_hi = (addr >> 32);
			decode_buffer->dpb_buffer_address_lo = (addr);
		break;
		case DECODE_CMD_DECODING_TARGET_BUFFER:
			decode_buffer->valid_buf_flag |= (DECODE_CMDBUF_FLAGS_DECODING_TARGET_BUFFER);
			decode_buffer->target_buffer_address_hi = (addr >> 32);
			decode_buffer->target_buffer_address_lo = (addr);
		break;
		case DECODE_CMD_FEEDBACK_BUFFER:
			decode_buffer->valid_buf_flag |= (DECODE_CMDBUF_FLAGS_FEEDBACK_BUFFER);
			decode_buffer->feedback_buffer_address_hi = (addr >> 32);
			decode_buffer->feedback_buffer_address_lo = (addr);
		break;
		case DECODE_CMD_PROB_TBL_BUFFER:
			decode_buffer->valid_buf_flag |= (DECODE_CMDBUF_FLAGS_PROB_TBL_BUFFER);
			decode_buffer->prob_tbl_buffer_address_hi = (addr >> 32);
			decode_buffer->prob_tbl_buffer_address_lo = (addr);
		break;
		case DECODE_CMD_SESSION_CONTEXT_BUFFER:
			decode_buffer->valid_buf_flag |= (DECODE_CMDBUF_FLAGS_SESSION_CONTEXT_BUFFER);
			decode_buffer->session_contex_buffer_address_hi = (addr >> 32);
			decode_buffer->session_contex_buffer_address_lo = (addr);
		break;
		case DECODE_CMD_BITSTREAM_BUFFER:
			decode_buffer->valid_buf_flag |= (DECODE_CMDBUF_FLAGS_BITSTREAM_BUFFER);
			decode_buffer->bitstream_buffer_address_hi = (addr >> 32);
			decode_buffer->bitstream_buffer_address_lo = (addr);
		break;
		case DECODE_CMD_IT_SCALING_TABLE_BUFFER:
			decode_buffer->valid_buf_flag |= (DECODE_CMDBUF_FLAGS_IT_SCALING_BUFFER);
			decode_buffer->it_sclr_table_buffer_address_hi = (addr >> 32);
			decode_buffer->it_sclr_table_buffer_address_lo = (addr);
		break;
		case DECODE_CMD_CONTEXT_BUFFER:
			decode_buffer->valid_buf_flag |= (DECODE_CMDBUF_FLAGS_CONTEXT_BUFFER);
			decode_buffer->context_buffer_address_hi = (addr >> 32);
			decode_buffer->context_buffer_address_lo = (addr);
		break;
		default:
			printf("Not Support!\n");
	}
}

static void amdgpu_cs_vcn_dec_create(void)
{
	struct amdgpu_vcn_bo msg_buf;
	unsigned ip;
	int len, r;

	num_resources  = 0;
	alloc_resource(&msg_buf, 4096, AMDGPU_GEM_DOMAIN_GTT);
	alloc_resource(&session_ctx_buf, 32 * 4096, AMDGPU_GEM_DOMAIN_VRAM);
	resources[num_resources++] = msg_buf.handle;
	resources[num_resources++] = session_ctx_buf.handle;
	resources[num_resources++] = ib_handle;

	r = amdgpu_bo_cpu_map(msg_buf.handle, (void **)&msg_buf.ptr);
	CU_ASSERT_EQUAL(r, 0);

	memset(msg_buf.ptr, 0, 4096);
	memcpy(msg_buf.ptr, vcn_dec_create_msg, sizeof(vcn_dec_create_msg));

	len = 0;

	vcn_dec_cmd(session_ctx_buf.addr, 5, &len);
	if (vcn_dec_sw_ring == true) {
		vcn_dec_cmd(msg_buf.addr, 0, &len);
	} else {
		ib_cpu[len++] = reg[vcn_reg_index].data0;
		ib_cpu[len++] = msg_buf.addr;
		ib_cpu[len++] = reg[vcn_reg_index].data1;
		ib_cpu[len++] = msg_buf.addr >> 32;
		ib_cpu[len++] = reg[vcn_reg_index].cmd;
		ib_cpu[len++] = 0;
		for (; len % 16; ) {
			ib_cpu[len++] = reg[vcn_reg_index].nop;
			ib_cpu[len++] = 0;
		}
	}

	if (vcn_unified_ring) {
		amdgpu_cs_sq_ib_tail(ib_cpu + len);
		ip = AMDGPU_HW_IP_VCN_ENC;
	} else
		ip = AMDGPU_HW_IP_VCN_DEC;

	r = submit(len, ip);

	CU_ASSERT_EQUAL(r, 0);

	free_resource(&msg_buf);
}

static void amdgpu_cs_vcn_dec_decode(void)
{
	const unsigned dpb_size = 15923584, dt_size = 737280;
	uint64_t msg_addr, fb_addr, bs_addr, dpb_addr, ctx_addr, dt_addr, it_addr, sum;
	struct amdgpu_vcn_bo dec_buf;
	int size, len, i, r;
	unsigned ip;
	uint8_t *dec;

	size = 4*1024; /* msg */
	size += 4*1024; /* fb */
	size += 4096; /*it_scaling_table*/
	size += ALIGN(sizeof(uvd_bitstream), 4*1024);
	size += ALIGN(dpb_size, 4*1024);
	size += ALIGN(dt_size, 4*1024);

	num_resources = 0;
	alloc_resource(&dec_buf, size, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = dec_buf.handle;
	resources[num_resources++] = ib_handle;

	r = amdgpu_bo_cpu_map(dec_buf.handle, (void **)&dec_buf.ptr);
	dec = dec_buf.ptr;

	CU_ASSERT_EQUAL(r, 0);
	memset(dec_buf.ptr, 0, size);
	memcpy(dec_buf.ptr, vcn_dec_decode_msg, sizeof(vcn_dec_decode_msg));
	memcpy(dec_buf.ptr + sizeof(vcn_dec_decode_msg),
			avc_decode_msg, sizeof(avc_decode_msg));

	dec += 4*1024;
	memcpy(dec, feedback_msg, sizeof(feedback_msg));
	dec += 4*1024;
	memcpy(dec, uvd_it_scaling_table, sizeof(uvd_it_scaling_table));

	dec += 4*1024;
	memcpy(dec, uvd_bitstream, sizeof(uvd_bitstream));

	dec += ALIGN(sizeof(uvd_bitstream), 4*1024);

	dec += ALIGN(dpb_size, 4*1024);

	msg_addr = dec_buf.addr;
	fb_addr = msg_addr + 4*1024;
	it_addr = fb_addr + 4*1024;
	bs_addr = it_addr + 4*1024;
	dpb_addr = ALIGN(bs_addr + sizeof(uvd_bitstream), 4*1024);
	ctx_addr = ALIGN(dpb_addr + 0x006B9400, 4*1024);
	dt_addr = ALIGN(dpb_addr + dpb_size, 4*1024);

	len = 0;
	vcn_dec_cmd(session_ctx_buf.addr, 0x5, &len);
	vcn_dec_cmd(msg_addr, 0x0, &len);
	vcn_dec_cmd(dpb_addr, 0x1, &len);
	vcn_dec_cmd(dt_addr, 0x2, &len);
	vcn_dec_cmd(fb_addr, 0x3, &len);
	vcn_dec_cmd(bs_addr, 0x100, &len);
	vcn_dec_cmd(it_addr, 0x204, &len);
	vcn_dec_cmd(ctx_addr, 0x206, &len);

	if (vcn_dec_sw_ring == false) {
		ib_cpu[len++] = reg[vcn_reg_index].cntl;
		ib_cpu[len++] = 0x1;
		for (; len % 16; ) {
			ib_cpu[len++] = reg[vcn_reg_index].nop;
			ib_cpu[len++] = 0;
		}
	}

	if (vcn_unified_ring) {
		amdgpu_cs_sq_ib_tail(ib_cpu + len);
		ip = AMDGPU_HW_IP_VCN_ENC;
	} else
		ip = AMDGPU_HW_IP_VCN_DEC;

	r = submit(len, ip);
	CU_ASSERT_EQUAL(r, 0);

	for (i = 0, sum = 0; i < dt_size; ++i)
		sum += dec[i];

	CU_ASSERT_EQUAL(sum, SUM_DECODE);

	free_resource(&dec_buf);
}

static void amdgpu_cs_vcn_dec_destroy(void)
{
	struct amdgpu_vcn_bo msg_buf;
	unsigned ip;
	int len, r;

	num_resources = 0;
	alloc_resource(&msg_buf, 1024, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = msg_buf.handle;
	resources[num_resources++] = ib_handle;

	r = amdgpu_bo_cpu_map(msg_buf.handle, (void **)&msg_buf.ptr);
	CU_ASSERT_EQUAL(r, 0);

	memset(msg_buf.ptr, 0, 1024);
	memcpy(msg_buf.ptr, vcn_dec_destroy_msg, sizeof(vcn_dec_destroy_msg));

	len = 0;
	vcn_dec_cmd(session_ctx_buf.addr, 5, &len);
	if (vcn_dec_sw_ring == true) {
		vcn_dec_cmd(msg_buf.addr, 0, &len);
	} else {
		ib_cpu[len++] = reg[vcn_reg_index].data0;
		ib_cpu[len++] = msg_buf.addr;
		ib_cpu[len++] = reg[vcn_reg_index].data1;
		ib_cpu[len++] = msg_buf.addr >> 32;
		ib_cpu[len++] = reg[vcn_reg_index].cmd;
		ib_cpu[len++] = 0;
		for (; len % 16; ) {
			ib_cpu[len++] = reg[vcn_reg_index].nop;
			ib_cpu[len++] = 0;
		}
	}

	if (vcn_unified_ring) {
		amdgpu_cs_sq_ib_tail(ib_cpu + len);
		ip = AMDGPU_HW_IP_VCN_ENC;
	} else
		ip = AMDGPU_HW_IP_VCN_DEC;

	r = submit(len, ip);
	CU_ASSERT_EQUAL(r, 0);

	free_resource(&msg_buf);
	free_resource(&session_ctx_buf);
}

static void amdgpu_cs_vcn_enc_create(void)
{
	int len, r;
	uint32_t *p_task_size = NULL;
	uint32_t task_offset = 0, st_offset;
	uint32_t *st_size = NULL;
	unsigned width = 160, height = 128, buf_size;
	uint32_t fw_maj = 1, fw_min = 9;

	if (vcn_ip_version_major == 2) {
		fw_maj = 1;
		fw_min = 1;
	} else if (vcn_ip_version_major == 3) {
		fw_maj = 1;
		fw_min = 0;
	}

	gWidth = width;
	gHeight = height;
	buf_size = ALIGN(width, 256) * ALIGN(height, 32) * 3 / 2;
	enc_task_id = 1;

	num_resources = 0;
	alloc_resource(&enc_buf, 128 * 1024, AMDGPU_GEM_DOMAIN_GTT);
	alloc_resource(&cpb_buf, buf_size * 2, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc_buf.handle;
	resources[num_resources++] = cpb_buf.handle;
	resources[num_resources++] = ib_handle;

	r = amdgpu_bo_cpu_map(enc_buf.handle, (void**)&enc_buf.ptr);
	memset(enc_buf.ptr, 0, 128 * 1024);
	r = amdgpu_bo_cpu_unmap(enc_buf.handle);

	r = amdgpu_bo_cpu_map(cpb_buf.handle, (void**)&enc_buf.ptr);
	memset(enc_buf.ptr, 0, buf_size * 2);
	r = amdgpu_bo_cpu_unmap(cpb_buf.handle);

	len = 0;

	if (vcn_unified_ring)
		amdgpu_cs_sq_head(ib_cpu, &len, true);

	/* session info */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000001;	/* RENCODE_IB_PARAM_SESSION_INFO */
	ib_cpu[len++] = ((fw_maj << 16) | (fw_min << 0));
	ib_cpu[len++] = enc_buf.addr >> 32;
	ib_cpu[len++] = enc_buf.addr;
	ib_cpu[len++] = 1;	/* RENCODE_ENGINE_TYPE_ENCODE; */
	*st_size = (len - st_offset) * 4;

	/* task info */
	task_offset = len;
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000002;	/* RENCODE_IB_PARAM_TASK_INFO */
	p_task_size = &ib_cpu[len++];
	ib_cpu[len++] = enc_task_id++;	/* task_id */
	ib_cpu[len++] = 0;	/* feedback */
	*st_size = (len - st_offset) * 4;

	/* op init */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x01000001;	/* RENCODE_IB_OP_INITIALIZE */
	*st_size = (len - st_offset) * 4;

	/* session_init */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000003;	/* RENCODE_IB_PARAM_SESSION_INIT */
	ib_cpu[len++] = 1;	/* RENCODE_ENCODE_STANDARD_H264 */
	ib_cpu[len++] = width;
	ib_cpu[len++] = height;
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 0;	/* pre encode mode */
	ib_cpu[len++] = 0;	/* chroma enabled : false */
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 0;
	*st_size = (len - st_offset) * 4;

	/* slice control */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00200001;	/* RENCODE_H264_IB_PARAM_SLICE_CONTROL */
	ib_cpu[len++] = 0;	/* RENCODE_H264_SLICE_CONTROL_MODE_FIXED_MBS */
	ib_cpu[len++] = ALIGN(width, 16) / 16 * ALIGN(height, 16) / 16;
	*st_size = (len - st_offset) * 4;

	/* enc spec misc */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00200002;	/* RENCODE_H264_IB_PARAM_SPEC_MISC */
	ib_cpu[len++] = 0;	/* constrained intra pred flag */
	ib_cpu[len++] = 0;	/* cabac enable */
	ib_cpu[len++] = 0;	/* cabac init idc */
	ib_cpu[len++] = 1;	/* half pel enabled */
	ib_cpu[len++] = 1;	/* quarter pel enabled */
	ib_cpu[len++] = 100;	/* BASELINE profile */
	ib_cpu[len++] = 11;	/* level */
	if (vcn_ip_version_major >= 3) {
		ib_cpu[len++] = 0;	/* b_picture_enabled */
		ib_cpu[len++] = 0;	/* weighted_bipred_idc */
	}
	*st_size = (len - st_offset) * 4;

	/* deblocking filter */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00200004;	/* RENCODE_H264_IB_PARAM_DEBLOCKING_FILTER */
	ib_cpu[len++] = 0;	/* disable deblocking filter idc */
	ib_cpu[len++] = 0;	/* alpha c0 offset */
	ib_cpu[len++] = 0;	/* tc offset */
	ib_cpu[len++] = 0;	/* cb offset */
	ib_cpu[len++] = 0;	/* cr offset */
	*st_size = (len - st_offset) * 4;

	/* layer control */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000004;	/* RENCODE_IB_PARAM_LAYER_CONTROL */
	ib_cpu[len++] = 1;	/* max temporal layer */
	ib_cpu[len++] = 1;	/* no of temporal layer */
	*st_size = (len - st_offset) * 4;

	/* rc_session init */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000006;	/* RENCODE_IB_PARAM_RATE_CONTROL_SESSION_INIT */
	ib_cpu[len++] = 0;	/* rate control */
	ib_cpu[len++] = 48;	/* vbv buffer level */
	*st_size = (len - st_offset) * 4;

	/* quality params */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000009;	/* RENCODE_IB_PARAM_QUALITY_PARAMS */
	ib_cpu[len++] = 0;	/* vbaq mode */
	ib_cpu[len++] = 0;	/* scene change sensitivity */
	ib_cpu[len++] = 0;	/* scene change min idr interval */
	ib_cpu[len++] = 0;
	if (vcn_ip_version_major >= 3)
		ib_cpu[len++] = 0;
	*st_size = (len - st_offset) * 4;

	/* layer select */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000005;	/* RENCODE_IB_PARAM_LAYER_SELECT */
	ib_cpu[len++] = 0;	/* temporal layer */
	*st_size = (len - st_offset) * 4;

	/* rc layer init */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000007;	/* RENCODE_IB_PARAM_RATE_CONTROL_LAYER_INIT */
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 25;
	ib_cpu[len++] = 1;
	ib_cpu[len++] = 0x01312d00;
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 0;
	*st_size = (len - st_offset) * 4;

	/* layer select */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000005;	/* RENCODE_IB_PARAM_LAYER_SELECT */
	ib_cpu[len++] = 0;	/* temporal layer */
	*st_size = (len - st_offset) * 4;

	/* rc per pic */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000008;	/* RENCODE_IB_PARAM_RATE_CONTROL_PER_PICTURE */
	ib_cpu[len++] = 20;
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 51;
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 1;
	ib_cpu[len++] = 0;
	ib_cpu[len++] = 1;
	ib_cpu[len++] = 0;
	*st_size = (len - st_offset) * 4;

	/* op init rc */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x01000004;	/* RENCODE_IB_OP_INIT_RC */
	*st_size = (len - st_offset) * 4;

	/* op init rc vbv */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x01000005;	/* RENCODE_IB_OP_INIT_RC_VBV_BUFFER_LEVEL */
	*st_size = (len - st_offset) * 4;

	*p_task_size = (len - task_offset) * 4;

	if (vcn_unified_ring)
		amdgpu_cs_sq_ib_tail(ib_cpu + len);

	r = submit(len, AMDGPU_HW_IP_VCN_ENC);
	CU_ASSERT_EQUAL(r, 0);
}

static int32_t h264_se (bufferInfo * bufInfo)
{
	uint32_t ret;

	ret = bs_read_ue (bufInfo);
	if ((ret & 0x1) == 0) {
		ret >>= 1;
		int32_t temp = 0 - ret;
		return temp;
	}

	return (ret + 1) >> 1;
}

static void h264_check_0s (bufferInfo * bufInfo, int count)
{
	uint32_t val;

	val = bs_read_u (bufInfo, count);
	if (val != 0) {
		printf ("field error - %d bits should be 0 is %x\n", count, val);
	}
}

static inline int bs_eof(bufferInfo * bufinfo)
{
	if (bufinfo->decBuffer >= bufinfo->end)
		return 1;
	else
		return 0;
}

static inline uint32_t bs_read_u1(bufferInfo *bufinfo)
{
	uint32_t r = 0;
	uint32_t temp = 0;

	bufinfo->numOfBitsInBuffer--;
	if (! bs_eof(bufinfo)) {
		temp = (((bufinfo->decData)) >> bufinfo->numOfBitsInBuffer);
		r = temp & 0x01;
	}

	if (bufinfo->numOfBitsInBuffer == 0) {
		bufinfo->decBuffer++;
		bufinfo->decData = *bufinfo->decBuffer;
		bufinfo->numOfBitsInBuffer = 8;
	}

	return r;
}

static inline uint32_t bs_read_u(bufferInfo* bufinfo, int n)
{
	uint32_t r = 0;
	int i;

	for (i = 0; i < n; i++) {
		r |= ( bs_read_u1(bufinfo) << ( n - i - 1 ) );
	}

	return r;
}

static inline uint32_t bs_read_ue(bufferInfo* bufinfo)
{
	int32_t r = 0;
	int i = 0;

	while( (bs_read_u1(bufinfo) == 0) && (i < 32) && (!bs_eof(bufinfo))) {
		i++;
	}
	r = bs_read_u(bufinfo, i);
	r += (1 << i) - 1;
	return r;
}

static uint32_t remove_03 (uint8_t * bptr, uint32_t len)
{
	uint32_t nal_len = 0;
	while (nal_len + 2 < len) {
		if (bptr[0] == 0 && bptr[1] == 0 && bptr[2] == 3) {
			bptr += 2;
			nal_len += 2;
			len--;
			memmove (bptr, bptr + 1, len - nal_len);
		} else {
			bptr++;
			nal_len++;
		}
	}
	return len;
}

static void scaling_list (uint32_t ix, uint32_t sizeOfScalingList, bufferInfo * bufInfo)
{
	uint32_t lastScale = 8, nextScale = 8;
	uint32_t jx;
	int deltaScale;

	for (jx = 0; jx < sizeOfScalingList; jx++) {
		if (nextScale != 0) {
			deltaScale = h264_se (bufInfo);
			nextScale = (lastScale + deltaScale + 256) % 256;
		}
		if (nextScale == 0) {
			lastScale = lastScale;
		} else {
			lastScale = nextScale;
		}
	}
}

static void h264_parse_sequence_parameter_set (h264_decode * dec, bufferInfo * bufInfo)
{
	uint32_t temp;

	dec->profile = bs_read_u (bufInfo, 8);
	bs_read_u (bufInfo, 1);		/* constaint_set0_flag */
	bs_read_u (bufInfo, 1);		/* constaint_set1_flag */
	bs_read_u (bufInfo, 1);		/* constaint_set2_flag */
	bs_read_u (bufInfo, 1);		/* constaint_set3_flag */
	bs_read_u (bufInfo, 1);		/* constaint_set4_flag */
	bs_read_u (bufInfo, 1);		/* constaint_set5_flag */


	h264_check_0s (bufInfo, 2);
	dec->level_idc = bs_read_u (bufInfo, 8);
	bs_read_ue (bufInfo);	/* SPS id*/

	if (dec->profile == 100 || dec->profile == 110 ||
		dec->profile == 122 || dec->profile == 144) {
		uint32_t chroma_format_idc = bs_read_ue (bufInfo);
		if (chroma_format_idc == 3) {
			bs_read_u (bufInfo, 1);	/* residual_colour_transform_flag */
		}
		bs_read_ue (bufInfo);	/* bit_depth_luma_minus8 */
		bs_read_ue (bufInfo);	/* bit_depth_chroma_minus8 */
		bs_read_u (bufInfo, 1);	/* qpprime_y_zero_transform_bypass_flag */
		uint32_t seq_scaling_matrix_present_flag = bs_read_u (bufInfo, 1);

		if (seq_scaling_matrix_present_flag) {
			for (uint32_t ix = 0; ix < 8; ix++) {
				temp = bs_read_u (bufInfo, 1);
				if (temp) {
					scaling_list (ix, ix < 6 ? 16 : 64, bufInfo);
				}
			}
		}
	}

	bs_read_ue (bufInfo);	/* log2_max_frame_num_minus4 */
	uint32_t pic_order_cnt_type = bs_read_ue (bufInfo);

	if (pic_order_cnt_type == 0) {
		bs_read_ue (bufInfo);	/* log2_max_pic_order_cnt_lsb_minus4 */
	} else if (pic_order_cnt_type == 1) {
		bs_read_u (bufInfo, 1);	/* delta_pic_order_always_zero_flag */
		h264_se (bufInfo);	/* offset_for_non_ref_pic */
		h264_se (bufInfo);	/* offset_for_top_to_bottom_field */
		temp = bs_read_ue (bufInfo);
		for (uint32_t ix = 0; ix < temp; ix++) {
			 h264_se (bufInfo);	/* offset_for_ref_frame[index] */
		}
	}
	bs_read_ue (bufInfo);	/* num_ref_frames */
	bs_read_u (bufInfo, 1);	/* gaps_in_frame_num_flag */
	uint32_t PicWidthInMbs = bs_read_ue (bufInfo) + 1;

	dec->pic_width = PicWidthInMbs * 16;
	uint32_t PicHeightInMapUnits = bs_read_ue (bufInfo) + 1;

	dec->pic_height = PicHeightInMapUnits * 16;
	uint32_t frame_mbs_only_flag = bs_read_u (bufInfo, 1);
	if (!frame_mbs_only_flag) {
		bs_read_u (bufInfo, 1);	/* mb_adaptive_frame_field_flag */
	}
	bs_read_u (bufInfo, 1);	/* direct_8x8_inference_flag */
	temp = bs_read_u (bufInfo, 1);
	if (temp) {
		bs_read_ue (bufInfo);	/* frame_crop_left_offset */
		bs_read_ue (bufInfo);	/* frame_crop_right_offset */
		bs_read_ue (bufInfo);	/* frame_crop_top_offset */
		bs_read_ue (bufInfo);	/* frame_crop_bottom_offset */
	}
	temp = bs_read_u (bufInfo, 1);	/* VUI Parameters  */
}

static void h264_slice_header (h264_decode * dec, bufferInfo * bufInfo)
{
	uint32_t temp;

	bs_read_ue (bufInfo);	/* first_mb_in_slice */
	temp = bs_read_ue (bufInfo);
	dec->slice_type = ((temp > 5) ? (temp - 5) : temp);
}

static uint8_t h264_parse_nal (h264_decode * dec, bufferInfo * bufInfo)
{
	uint8_t type = 0;

	h264_check_0s (bufInfo, 1);
	dec->nal_ref_idc = bs_read_u (bufInfo, 2);
	dec->nal_unit_type = type = bs_read_u (bufInfo, 5);
	switch (type)
	{
	case H264_NAL_TYPE_NON_IDR_SLICE:
	case H264_NAL_TYPE_IDR_SLICE:
		h264_slice_header (dec, bufInfo);
		break;
	case H264_NAL_TYPE_SEQ_PARAM:
		h264_parse_sequence_parameter_set (dec, bufInfo);
		break;
	case H264_NAL_TYPE_PIC_PARAM:
	case H264_NAL_TYPE_SEI:
	case H264_NAL_TYPE_ACCESS_UNIT:
	case H264_NAL_TYPE_SEQ_EXTENSION:
		/* NOP */
		break;
	default:
		printf ("Nal type unknown %d \n ", type);
		break;
	}
	return type;
}

static uint32_t h264_find_next_start_code (uint8_t * pBuf, uint32_t bufLen)
{
	uint32_t val;
	uint32_t offset, startBytes;

	offset = startBytes = 0;
	if (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 0 && pBuf[3] == 1) {
		pBuf += 4;
		offset = 4;
		startBytes = 1;
	} else if (pBuf[0] == 0 && pBuf[1] == 0 && pBuf[2] == 1) {
		pBuf += 3;
		offset = 3;
		startBytes = 1;
	}
	val = 0xffffffff;
	while (offset < bufLen - 3) {
		val <<= 8;
		val |= *pBuf++;
		offset++;
		if (val == H264_START_CODE)
			return offset - 4;

		if ((val & 0x00ffffff) == H264_START_CODE)
			return offset - 3;
	}
	if (bufLen - offset <= 3 && startBytes == 0) {
		startBytes = 0;
		return 0;
	}

	return offset;
}

static int verify_checksum(uint8_t *buffer, uint32_t buffer_size)
{
	uint32_t buffer_pos = 0;
	int done = 0;
	h264_decode dec;

	memset(&dec, 0, sizeof(h264_decode));
	do {
		uint32_t ret;

		ret = h264_find_next_start_code (buffer + buffer_pos,
				 buffer_size - buffer_pos);
		if (ret == 0) {
			done = 1;
			if (buffer_pos == 0) {
				fprintf (stderr,
				 "couldn't find start code in buffer from 0\n");
			}
		} else {
		/* have a complete NAL from buffer_pos to end */
			if (ret > 3) {
				uint32_t nal_len;
				bufferInfo bufinfo;

				nal_len = remove_03 (buffer + buffer_pos, ret);
				bufinfo.decBuffer = buffer + buffer_pos + (buffer[buffer_pos + 2] == 1 ? 3 : 4);
				bufinfo.decBufferSize = (nal_len - (buffer[buffer_pos + 2] == 1 ? 3 : 4)) * 8;
				bufinfo.end = buffer + buffer_pos + nal_len;
				bufinfo.numOfBitsInBuffer = 8;
				bufinfo.decData = *bufinfo.decBuffer;
				h264_parse_nal (&dec, &bufinfo);
			}
			buffer_pos += ret;	/*  buffer_pos points to next code */
		}
	} while (done == 0);

	if ((dec.pic_width == gWidth) &&
		(dec.pic_height == gHeight) &&
		(dec.slice_type == gSliceType))
	    return 0;
	else
		return -1;
}

static void check_result(struct amdgpu_vcn_bo fb_buf, struct amdgpu_vcn_bo bs_buf, int frame_type)
{
	uint32_t *fb_ptr;
	uint8_t *bs_ptr;
	uint32_t size;
	int r;
/* 	uint64_t s[3] = {0, 1121279001727, 1059312481445}; */

	r = amdgpu_bo_cpu_map(fb_buf.handle, (void **)&fb_buf.ptr);
	CU_ASSERT_EQUAL(r, 0);
	fb_ptr = (uint32_t*)fb_buf.ptr;
	size = fb_ptr[6];
	r = amdgpu_bo_cpu_unmap(fb_buf.handle);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_bo_cpu_map(bs_buf.handle, (void **)&bs_buf.ptr);
	CU_ASSERT_EQUAL(r, 0);

	bs_ptr = (uint8_t*)bs_buf.ptr;
	r = verify_checksum(bs_ptr, size);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_bo_cpu_unmap(bs_buf.handle);

	CU_ASSERT_EQUAL(r, 0);
}

static void amdgpu_cs_vcn_ib_zero_count(int *len, int num)
{
	for (int i = 0; i < num; i++)
		ib_cpu[(*len)++] = 0;
}

static void amdgpu_cs_vcn_enc_encode_frame(int frame_type)
{
	struct amdgpu_vcn_bo bs_buf, fb_buf, input_buf;
	int len, r;
	unsigned width = 160, height = 128, buf_size;
	uint32_t *p_task_size = NULL;
	uint32_t task_offset = 0, st_offset;
	uint32_t *st_size = NULL;
	uint32_t fw_maj = 1, fw_min = 9;

	if (vcn_ip_version_major == 2) {
		fw_maj = 1;
		fw_min = 1;
	} else if (vcn_ip_version_major == 3) {
		fw_maj = 1;
		fw_min = 0;
	}
	gSliceType = frame_type;
	buf_size = ALIGN(width, 256) * ALIGN(height, 32) * 3 / 2;

	num_resources = 0;
	alloc_resource(&bs_buf, 4096, AMDGPU_GEM_DOMAIN_GTT);
	alloc_resource(&fb_buf, 4096, AMDGPU_GEM_DOMAIN_GTT);
	alloc_resource(&input_buf, buf_size, AMDGPU_GEM_DOMAIN_GTT);
	resources[num_resources++] = enc_buf.handle;
	resources[num_resources++] = cpb_buf.handle;
	resources[num_resources++] = bs_buf.handle;
	resources[num_resources++] = fb_buf.handle;
	resources[num_resources++] = input_buf.handle;
	resources[num_resources++] = ib_handle;


	r = amdgpu_bo_cpu_map(bs_buf.handle, (void**)&bs_buf.ptr);
	memset(bs_buf.ptr, 0, 4096);
	r = amdgpu_bo_cpu_unmap(bs_buf.handle);

	r = amdgpu_bo_cpu_map(fb_buf.handle, (void**)&fb_buf.ptr);
	memset(fb_buf.ptr, 0, 4096);
	r = amdgpu_bo_cpu_unmap(fb_buf.handle);

	r = amdgpu_bo_cpu_map(input_buf.handle, (void **)&input_buf.ptr);
	CU_ASSERT_EQUAL(r, 0);

	for (int i = 0; i < ALIGN(height, 32) * 3 / 2; i++)
		memcpy(input_buf.ptr + i * ALIGN(width, 256), frame + i * width, width);

	r = amdgpu_bo_cpu_unmap(input_buf.handle);
	CU_ASSERT_EQUAL(r, 0);

	len = 0;

	if (vcn_unified_ring)
		amdgpu_cs_sq_head(ib_cpu, &len, true);

	/* session info */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000001;	/* RENCODE_IB_PARAM_SESSION_INFO */
	ib_cpu[len++] = ((fw_maj << 16) | (fw_min << 0));
	ib_cpu[len++] = enc_buf.addr >> 32;
	ib_cpu[len++] = enc_buf.addr;
	ib_cpu[len++] = 1;	/* RENCODE_ENGINE_TYPE_ENCODE */;
	*st_size = (len - st_offset) * 4;

	/* task info */
	task_offset = len;
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000002;	/* RENCODE_IB_PARAM_TASK_INFO */
	p_task_size = &ib_cpu[len++];
	ib_cpu[len++] = enc_task_id++;	/* task_id */
	ib_cpu[len++] = 1;	/* feedback */
	*st_size = (len - st_offset) * 4;

	if (frame_type == 2) {
		/* sps */
		st_offset = len;
		st_size = &ib_cpu[len++];	/* size */
		if(vcn_ip_version_major == 1)
			ib_cpu[len++] = 0x00000020;	/* RENCODE_IB_PARAM_DIRECT_OUTPUT_NALU vcn 1 */
		else
			ib_cpu[len++] = 0x0000000a;	/* RENCODE_IB_PARAM_DIRECT_OUTPUT_NALU other vcn */
		ib_cpu[len++] = 0x00000002;	/* RENCODE_DIRECT_OUTPUT_NALU_TYPE_SPS */
		ib_cpu[len++] = 0x00000011;	/* sps len */
		ib_cpu[len++] = 0x00000001;	/* start code */
		ib_cpu[len++] = 0x6764440b;
		ib_cpu[len++] = 0xac54c284;
		ib_cpu[len++] = 0x68078442;
		ib_cpu[len++] = 0x37000000;
		*st_size = (len - st_offset) * 4;

		/* pps */
		st_offset = len;
		st_size = &ib_cpu[len++];	/* size */
		if(vcn_ip_version_major == 1)
			ib_cpu[len++] = 0x00000020;	/* RENCODE_IB_PARAM_DIRECT_OUTPUT_NALU vcn 1*/
		else
			ib_cpu[len++] = 0x0000000a;	/* RENCODE_IB_PARAM_DIRECT_OUTPUT_NALU other vcn*/
		ib_cpu[len++] = 0x00000003;	/* RENCODE_DIRECT_OUTPUT_NALU_TYPE_PPS */
		ib_cpu[len++] = 0x00000008;	/* pps len */
		ib_cpu[len++] = 0x00000001;	/* start code */
		ib_cpu[len++] = 0x68ce3c80;
		*st_size = (len - st_offset) * 4;
	}

	/* slice header */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	if(vcn_ip_version_major == 1)
		ib_cpu[len++] = 0x0000000a; /* RENCODE_IB_PARAM_SLICE_HEADER vcn 1 */
	else
		ib_cpu[len++] = 0x0000000b; /* RENCODE_IB_PARAM_SLICE_HEADER other vcn */
	if (frame_type == 2) {
		ib_cpu[len++] = 0x65000000;
		ib_cpu[len++] = 0x11040000;
	} else {
		ib_cpu[len++] = 0x41000000;
		ib_cpu[len++] = 0x34210000;
	}
	ib_cpu[len++] = 0xe0000000;
	amdgpu_cs_vcn_ib_zero_count(&len, 13);

	ib_cpu[len++] = 0x00000001;
	ib_cpu[len++] = 0x00000008;
	ib_cpu[len++] = 0x00020000;
	ib_cpu[len++] = 0x00000000;
	ib_cpu[len++] = 0x00000001;
	ib_cpu[len++] = 0x00000015;
	ib_cpu[len++] = 0x00020001;
	ib_cpu[len++] = 0x00000000;
	ib_cpu[len++] = 0x00000001;
	ib_cpu[len++] = 0x00000003;
	amdgpu_cs_vcn_ib_zero_count(&len, 22);
	*st_size = (len - st_offset) * 4;

	/* encode params */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	if(vcn_ip_version_major == 1)
		ib_cpu[len++] = 0x0000000b;	/* RENCODE_IB_PARAM_ENCODE_PARAMS vcn 1 */
	else
		ib_cpu[len++] = 0x0000000f;	/* RENCODE_IB_PARAM_ENCODE_PARAMS other vcn */
	ib_cpu[len++] = frame_type;
	ib_cpu[len++] = 0x0001f000;
	ib_cpu[len++] = input_buf.addr >> 32;
	ib_cpu[len++] = input_buf.addr;
	ib_cpu[len++] = (input_buf.addr + ALIGN(width, 256) * ALIGN(height, 32)) >> 32;
	ib_cpu[len++] = input_buf.addr + ALIGN(width, 256) * ALIGN(height, 32);
	ib_cpu[len++] = 0x00000100;
	ib_cpu[len++] = 0x00000080;
	ib_cpu[len++] = 0x00000000;
	ib_cpu[len++] = 0xffffffff;
	ib_cpu[len++] = 0x00000000;
	*st_size = (len - st_offset) * 4;

	/* encode params h264 */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00200003;	/* RENCODE_H264_IB_PARAM_ENCODE_PARAMS */
	if (vcn_ip_version_major <= 2) {
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0xffffffff;
	} else {
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0xffffffff;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0xffffffff;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000001;
	}
	*st_size = (len - st_offset) * 4;

	/* encode context */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	if(vcn_ip_version_major == 1)
		ib_cpu[len++] = 0x0000000d;	/* ENCODE_CONTEXT_BUFFER  vcn 1 */
	else
		ib_cpu[len++] = 0x00000011;	/* ENCODE_CONTEXT_BUFFER  other vcn */
	ib_cpu[len++] = cpb_buf.addr >> 32;
	ib_cpu[len++] = cpb_buf.addr;
	ib_cpu[len++] = 0x00000000;	/* swizzle mode */
	ib_cpu[len++] = 0x00000100;	/* luma pitch */
	ib_cpu[len++] = 0x00000100;	/* chroma pitch */
	ib_cpu[len++] = 0x00000002; /* no reconstructed picture */
	ib_cpu[len++] = 0x00000000;	/* reconstructed pic 1 luma offset */
	ib_cpu[len++] = ALIGN(width, 256) * ALIGN(height, 32);	/* pic1 chroma offset */
	if(vcn_ip_version_major == 4)
		amdgpu_cs_vcn_ib_zero_count(&len, 2);
	ib_cpu[len++] = ALIGN(width, 256) * ALIGN(height, 32) * 3 / 2;	/* pic2 luma offset */
	ib_cpu[len++] = ALIGN(width, 256) * ALIGN(height, 32) * 5 / 2;	/* pic2 chroma offset */

	amdgpu_cs_vcn_ib_zero_count(&len, 280);
	*st_size = (len - st_offset) * 4;

	/* bitstream buffer */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	if(vcn_ip_version_major == 1)
		ib_cpu[len++] = 0x0000000e;	/* VIDEO_BITSTREAM_BUFFER vcn 1 */
	else
		ib_cpu[len++] = 0x00000012;	/* VIDEO_BITSTREAM_BUFFER other vcn */

	ib_cpu[len++] = 0x00000000;	/* mode */
	ib_cpu[len++] = bs_buf.addr >> 32;
	ib_cpu[len++] = bs_buf.addr;
	ib_cpu[len++] = 0x0001f000;
	ib_cpu[len++] = 0x00000000;
	*st_size = (len - st_offset) * 4;

	/* feedback */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	if(vcn_ip_version_major == 1)
		ib_cpu[len++] = 0x00000010;	/* FEEDBACK_BUFFER vcn 1 */
	else
		ib_cpu[len++] = 0x00000015;	/* FEEDBACK_BUFFER vcn 2,3 */
	ib_cpu[len++] = 0x00000000;
	ib_cpu[len++] = fb_buf.addr >> 32;
	ib_cpu[len++] = fb_buf.addr;
	ib_cpu[len++] = 0x00000010;
	ib_cpu[len++] = 0x00000028;
	*st_size = (len - st_offset) * 4;

	/* intra refresh */
	st_offset = len;
	st_size = &ib_cpu[len++];
	if(vcn_ip_version_major == 1)
		ib_cpu[len++] = 0x0000000c;	/* INTRA_REFRESH vcn 1 */
	else
		ib_cpu[len++] = 0x00000010;	/* INTRA_REFRESH vcn 2,3 */
	ib_cpu[len++] = 0x00000000;
	ib_cpu[len++] = 0x00000000;
	ib_cpu[len++] = 0x00000000;
	*st_size = (len - st_offset) * 4;

	if(vcn_ip_version_major != 1) {
		/* Input Format */
		st_offset = len;
		st_size = &ib_cpu[len++];
		ib_cpu[len++] = 0x0000000c;
		ib_cpu[len++] = 0x00000000;	/* RENCODE_COLOR_VOLUME_G22_BT709 */
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;	/* RENCODE_COLOR_BIT_DEPTH_8_BIT */
		ib_cpu[len++] = 0x00000000;	/* RENCODE_COLOR_PACKING_FORMAT_NV12 */
		*st_size = (len - st_offset) * 4;

		/* Output Format */
		st_offset = len;
		st_size = &ib_cpu[len++];
		ib_cpu[len++] = 0x0000000d;
		ib_cpu[len++] = 0x00000000;	/* RENCODE_COLOR_VOLUME_G22_BT709 */
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;
		ib_cpu[len++] = 0x00000000;	/* RENCODE_COLOR_BIT_DEPTH_8_BIT */
		*st_size = (len - st_offset) * 4;
	}
	/* op_speed */
	st_offset = len;
	st_size = &ib_cpu[len++];
	ib_cpu[len++] = 0x01000006;	/* SPEED_ENCODING_MODE */
	*st_size = (len - st_offset) * 4;

	/* op_enc */
	st_offset = len;
	st_size = &ib_cpu[len++];
	ib_cpu[len++] = 0x01000003;
	*st_size = (len - st_offset) * 4;

	*p_task_size = (len - task_offset) * 4;

	if (vcn_unified_ring)
		amdgpu_cs_sq_ib_tail(ib_cpu + len);

	r = submit(len, AMDGPU_HW_IP_VCN_ENC);
	CU_ASSERT_EQUAL(r, 0);

	/* check result */
	check_result(fb_buf, bs_buf, frame_type);

	free_resource(&fb_buf);
	free_resource(&bs_buf);
	free_resource(&input_buf);
}

static void amdgpu_cs_vcn_enc_encode(void)
{
	amdgpu_cs_vcn_enc_encode_frame(2);	/* IDR frame */
}

static void amdgpu_cs_vcn_enc_destroy(void)
{
	int len = 0, r;
	uint32_t *p_task_size = NULL;
	uint32_t task_offset = 0, st_offset;
	uint32_t *st_size = NULL;
	uint32_t fw_maj = 1, fw_min = 9;

	if (vcn_ip_version_major == 2) {
		fw_maj = 1;
		fw_min = 1;
	} else if (vcn_ip_version_major == 3) {
		fw_maj = 1;
		fw_min = 0;
	}

	num_resources = 0;
/* 	alloc_resource(&enc_buf, 128 * 1024, AMDGPU_GEM_DOMAIN_GTT); */
	resources[num_resources++] = enc_buf.handle;
	resources[num_resources++] = ib_handle;

	if (vcn_unified_ring)
		amdgpu_cs_sq_head(ib_cpu, &len, true);

	/* session info */
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000001;	/* RENCODE_IB_PARAM_SESSION_INFO */
	ib_cpu[len++] = ((fw_maj << 16) | (fw_min << 0));
	ib_cpu[len++] = enc_buf.addr >> 32;
	ib_cpu[len++] = enc_buf.addr;
	ib_cpu[len++] = 1;	/* RENCODE_ENGINE_TYPE_ENCODE; */
	*st_size = (len - st_offset) * 4;

	/* task info */
	task_offset = len;
	st_offset = len;
	st_size = &ib_cpu[len++];	/* size */
	ib_cpu[len++] = 0x00000002;	/* RENCODE_IB_PARAM_TASK_INFO */
	p_task_size = &ib_cpu[len++];
	ib_cpu[len++] = enc_task_id++;	/* task_id */
	ib_cpu[len++] = 0;	/* feedback */
	*st_size = (len - st_offset) * 4;

	/*  op close */
	st_offset = len;
	st_size = &ib_cpu[len++];
	ib_cpu[len++] = 0x01000002;	/* RENCODE_IB_OP_CLOSE_SESSION */
	*st_size = (len - st_offset) * 4;

	*p_task_size = (len - task_offset) * 4;

	if (vcn_unified_ring)
		amdgpu_cs_sq_ib_tail(ib_cpu + len);

	r = submit(len, AMDGPU_HW_IP_VCN_ENC);
	CU_ASSERT_EQUAL(r, 0);

	free_resource(&cpb_buf);
	free_resource(&enc_buf);
}
