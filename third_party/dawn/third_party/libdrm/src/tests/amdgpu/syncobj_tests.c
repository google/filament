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

#include "CUnit/Basic.h"
#include "xf86drm.h"

#include "amdgpu_test.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"
#include <pthread.h>

static  amdgpu_device_handle device_handle;
static  uint32_t  major_version;
static  uint32_t  minor_version;

static  uint32_t  family_id;
static  uint32_t  chip_id;
static  uint32_t  chip_rev;

static void amdgpu_syncobj_timeline_test(void);

CU_BOOL suite_syncobj_timeline_tests_enable(void)
{
	int r;
	uint64_t cap = 0;

	r = drmGetCap(drm_amdgpu[0], DRM_CAP_SYNCOBJ_TIMELINE, &cap);
	if (r || cap == 0)
		return CU_FALSE;

	return CU_TRUE;
}

int suite_syncobj_timeline_tests_init(void)
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

	return CUE_SUCCESS;
}

int suite_syncobj_timeline_tests_clean(void)
{
	int r = amdgpu_device_deinitialize(device_handle);

	if (r == 0)
		return CUE_SUCCESS;
	else
		return CUE_SCLEAN_FAILED;
}


CU_TestInfo syncobj_timeline_tests[] = {
	{ "syncobj timeline test",  amdgpu_syncobj_timeline_test },
	CU_TEST_INFO_NULL,
};

#define GFX_COMPUTE_NOP  0xffff1000
#define SDMA_NOP  0x0
static int syncobj_command_submission_helper(uint32_t syncobj_handle, bool
					     wait_or_signal, uint64_t point)
{
	amdgpu_context_handle context_handle;
	amdgpu_bo_handle ib_result_handle;
	void *ib_result_cpu;
	uint64_t ib_result_mc_address;
	struct drm_amdgpu_cs_chunk chunks[2];
	struct drm_amdgpu_cs_chunk_data chunk_data;
	struct drm_amdgpu_cs_chunk_syncobj syncobj_data;
	struct amdgpu_cs_fence fence_status;
	amdgpu_bo_list_handle bo_list;
	amdgpu_va_handle va_handle;
	uint32_t expired;
	int i, r;
	uint64_t seq_no;
	static uint32_t *ptr;
	struct amdgpu_gpu_info gpu_info = {0};
	unsigned gc_ip_type;

	r = amdgpu_query_gpu_info(device_handle, &gpu_info);
	CU_ASSERT_EQUAL(r, 0);

	family_id = device_handle->info.family_id;
	chip_id = device_handle->info.chip_external_rev;
	chip_rev = device_handle->info.chip_rev;

	gc_ip_type = (asic_is_gfx_pipe_removed(family_id, chip_id, chip_rev)) ?
			AMDGPU_HW_IP_COMPUTE : AMDGPU_HW_IP_GFX;

	r = amdgpu_cs_ctx_create(device_handle, &context_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_alloc_and_map(device_handle, 4096, 4096,
				    AMDGPU_GEM_DOMAIN_GTT, 0,
				    &ib_result_handle, &ib_result_cpu,
				    &ib_result_mc_address, &va_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_get_bo_list(device_handle, ib_result_handle, NULL,
			       &bo_list);
	CU_ASSERT_EQUAL(r, 0);

	ptr = ib_result_cpu;

	for (i = 0; i < 16; ++i)
		ptr[i] = wait_or_signal ? GFX_COMPUTE_NOP: SDMA_NOP;

	chunks[0].chunk_id = AMDGPU_CHUNK_ID_IB;
	chunks[0].length_dw = sizeof(struct drm_amdgpu_cs_chunk_ib) / 4;
	chunks[0].chunk_data = (uint64_t)(uintptr_t)&chunk_data;
	chunk_data.ib_data._pad = 0;
	chunk_data.ib_data.va_start = ib_result_mc_address;
	chunk_data.ib_data.ib_bytes = 16 * 4;
	chunk_data.ib_data.ip_type = wait_or_signal ? gc_ip_type :
		AMDGPU_HW_IP_DMA;
	chunk_data.ib_data.ip_instance = 0;
	chunk_data.ib_data.ring = 0;
	chunk_data.ib_data.flags = AMDGPU_IB_FLAG_EMIT_MEM_SYNC;

	chunks[1].chunk_id = wait_or_signal ?
		AMDGPU_CHUNK_ID_SYNCOBJ_TIMELINE_WAIT :
		AMDGPU_CHUNK_ID_SYNCOBJ_TIMELINE_SIGNAL;
	chunks[1].length_dw = sizeof(struct drm_amdgpu_cs_chunk_syncobj) / 4;
	chunks[1].chunk_data = (uint64_t)(uintptr_t)&syncobj_data;
	syncobj_data.handle = syncobj_handle;
	syncobj_data.point = point;
	syncobj_data.flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT;

	r = amdgpu_cs_submit_raw(device_handle,
				 context_handle,
				 bo_list,
				 2,
				 chunks,
				 &seq_no);
	CU_ASSERT_EQUAL(r, 0);


	memset(&fence_status, 0, sizeof(struct amdgpu_cs_fence));
	fence_status.context = context_handle;
	fence_status.ip_type = wait_or_signal ? gc_ip_type :
		AMDGPU_HW_IP_DMA;
	fence_status.ip_instance = 0;
	fence_status.ring = 0;
	fence_status.fence = seq_no;

	r = amdgpu_cs_query_fence_status(&fence_status,
			AMDGPU_TIMEOUT_INFINITE,0, &expired);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_list_destroy(bo_list);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_unmap_and_free(ib_result_handle, va_handle,
				     ib_result_mc_address, 4096);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_cs_ctx_free(context_handle);
	CU_ASSERT_EQUAL(r, 0);

	return r;
}

struct syncobj_point {
	uint32_t syncobj_handle;
	uint64_t point;
};

static void *syncobj_wait(void *data)
{
	struct syncobj_point *sp = (struct syncobj_point *)data;
	int r;

	r = syncobj_command_submission_helper(sp->syncobj_handle, true,
					      sp->point);
	CU_ASSERT_EQUAL(r, 0);

	return (void *)(long)r;
}

static void *syncobj_signal(void *data)
{
	struct syncobj_point *sp = (struct syncobj_point *)data;
	int r;

	r = syncobj_command_submission_helper(sp->syncobj_handle, false,
					      sp->point);
	CU_ASSERT_EQUAL(r, 0);

	return (void *)(long)r;
}

static void amdgpu_syncobj_timeline_test(void)
{
	static pthread_t wait_thread;
	static pthread_t signal_thread;
	static pthread_t c_thread;
	struct syncobj_point sp1, sp2, sp3;
	uint32_t syncobj_handle;
	uint64_t payload;
	uint64_t wait_point, signal_point;
	uint64_t timeout;
	struct timespec tp;
	int r, sync_fd;
	void *tmp;

	r =  amdgpu_cs_create_syncobj2(device_handle, 0, &syncobj_handle);
	CU_ASSERT_EQUAL(r, 0);

	// wait on point 5
	sp1.syncobj_handle = syncobj_handle;
	sp1.point = 5;
	r = pthread_create(&wait_thread, NULL, syncobj_wait, &sp1);
	CU_ASSERT_EQUAL(r, 0);

	// signal on point 10
	sp2.syncobj_handle = syncobj_handle;
	sp2.point = 10;
	r = pthread_create(&signal_thread, NULL, syncobj_signal, &sp2);
	CU_ASSERT_EQUAL(r, 0);

	r = pthread_join(wait_thread, &tmp);
	CU_ASSERT_EQUAL(r, 0);
	CU_ASSERT_EQUAL(tmp, 0);

	r = pthread_join(signal_thread, &tmp);
	CU_ASSERT_EQUAL(r, 0);
	CU_ASSERT_EQUAL(tmp, 0);

	//query timeline payload
	r = amdgpu_cs_syncobj_query(device_handle, &syncobj_handle,
				    &payload, 1);
	CU_ASSERT_EQUAL(r, 0);
	CU_ASSERT_EQUAL(payload, 10);

	//signal on point 16
	sp3.syncobj_handle = syncobj_handle;
	sp3.point = 16;
	r = pthread_create(&c_thread, NULL, syncobj_signal, &sp3);
	CU_ASSERT_EQUAL(r, 0);
	//CPU wait on point 16
	wait_point = 16;
	timeout = 0;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	timeout = tp.tv_sec * 1000000000ULL + tp.tv_nsec;
	timeout += 0x10000000000; //10s
	r = amdgpu_cs_syncobj_timeline_wait(device_handle, &syncobj_handle,
					    &wait_point, 1, timeout,
					    DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL |
					    DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT,
					    NULL);

	CU_ASSERT_EQUAL(r, 0);
	r = pthread_join(c_thread, &tmp);
	CU_ASSERT_EQUAL(r, 0);
	CU_ASSERT_EQUAL(tmp, 0);

	// export point 16 and import to point 18
	r = amdgpu_cs_syncobj_export_sync_file2(device_handle, syncobj_handle,
						16,
						DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT,
						&sync_fd);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_cs_syncobj_import_sync_file2(device_handle, syncobj_handle,
						18, sync_fd);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_cs_syncobj_query(device_handle, &syncobj_handle,
				    &payload, 1);
	CU_ASSERT_EQUAL(r, 0);
	CU_ASSERT_EQUAL(payload, 18);

	// CPU signal on point 20
	signal_point = 20;
	r = amdgpu_cs_syncobj_timeline_signal(device_handle, &syncobj_handle,
					      &signal_point, 1);
	CU_ASSERT_EQUAL(r, 0);
	r = amdgpu_cs_syncobj_query(device_handle, &syncobj_handle,
				    &payload, 1);
	CU_ASSERT_EQUAL(r, 0);
	CU_ASSERT_EQUAL(payload, 20);

	r = amdgpu_cs_destroy_syncobj(device_handle, syncobj_handle);
	CU_ASSERT_EQUAL(r, 0);

}
