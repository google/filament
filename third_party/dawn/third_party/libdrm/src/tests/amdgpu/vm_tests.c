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

#include "amdgpu_test.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"

static  amdgpu_device_handle device_handle;
static  uint32_t  major_version;
static  uint32_t  minor_version;
static  uint32_t  family_id;
static  uint32_t  chip_id;
static  uint32_t  chip_rev;

static void amdgpu_vmid_reserve_test(void);
static void amdgpu_vm_unaligned_map(void);
static void amdgpu_vm_mapping_test(void);

CU_BOOL suite_vm_tests_enable(void)
{
    CU_BOOL enable = CU_TRUE;

	if (amdgpu_device_initialize(drm_amdgpu[0], &major_version,
				     &minor_version, &device_handle))
		return CU_FALSE;

	if (device_handle->info.family_id == AMDGPU_FAMILY_SI) {
		printf("\n\nCurrently hangs the CP on this ASIC, VM suite disabled\n");
		enable = CU_FALSE;
	}

	if (amdgpu_device_deinitialize(device_handle))
		return CU_FALSE;

	return enable;
}

int suite_vm_tests_init(void)
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

int suite_vm_tests_clean(void)
{
	int r = amdgpu_device_deinitialize(device_handle);

	if (r == 0)
		return CUE_SUCCESS;
	else
		return CUE_SCLEAN_FAILED;
}


CU_TestInfo vm_tests[] = {
	{ "resere vmid test",  amdgpu_vmid_reserve_test },
	{ "unaligned map",  amdgpu_vm_unaligned_map },
	{ "vm mapping test",  amdgpu_vm_mapping_test },
	CU_TEST_INFO_NULL,
};

static void amdgpu_vmid_reserve_test(void)
{
	amdgpu_context_handle context_handle;
	amdgpu_bo_handle ib_result_handle;
	void *ib_result_cpu;
	uint64_t ib_result_mc_address;
	struct amdgpu_cs_request ibs_request;
	struct amdgpu_cs_ib_info ib_info;
	struct amdgpu_cs_fence fence_status;
	uint32_t expired, flags;
	int i, r;
	amdgpu_bo_list_handle bo_list;
	amdgpu_va_handle va_handle;
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

	flags = 0;
	r = amdgpu_vm_reserve_vmid(device_handle, flags);
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
		ptr[i] = 0xffff1000;

	memset(&ib_info, 0, sizeof(struct amdgpu_cs_ib_info));
	ib_info.ib_mc_address = ib_result_mc_address;
	ib_info.size = 16;

	memset(&ibs_request, 0, sizeof(struct amdgpu_cs_request));
	ibs_request.ip_type = gc_ip_type;
	ibs_request.ring = 0;
	ibs_request.number_of_ibs = 1;
	ibs_request.ibs = &ib_info;
	ibs_request.resources = bo_list;
	ibs_request.fence_info.handle = NULL;

	r = amdgpu_cs_submit(context_handle, 0,&ibs_request, 1);
	CU_ASSERT_EQUAL(r, 0);


	memset(&fence_status, 0, sizeof(struct amdgpu_cs_fence));
	fence_status.context = context_handle;
	fence_status.ip_type = gc_ip_type;
	fence_status.ip_instance = 0;
	fence_status.ring = 0;
	fence_status.fence = ibs_request.seq_no;

	r = amdgpu_cs_query_fence_status(&fence_status,
			AMDGPU_TIMEOUT_INFINITE,0, &expired);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_list_destroy(bo_list);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_unmap_and_free(ib_result_handle, va_handle,
				     ib_result_mc_address, 4096);
	CU_ASSERT_EQUAL(r, 0);

	flags = 0;
	r = amdgpu_vm_unreserve_vmid(device_handle, flags);
	CU_ASSERT_EQUAL(r, 0);


	r = amdgpu_cs_ctx_free(context_handle);
	CU_ASSERT_EQUAL(r, 0);
}

static void amdgpu_vm_unaligned_map(void)
{
	const uint64_t map_size = (4ULL << 30) - (2 << 12);
	struct amdgpu_bo_alloc_request request = {};
	amdgpu_bo_handle buf_handle;
	amdgpu_va_handle handle;
	uint64_t vmc_addr;
	int r;

	request.alloc_size = 4ULL << 30;
	request.phys_alignment = 4096;
	request.preferred_heap = AMDGPU_GEM_DOMAIN_VRAM;
	request.flags = AMDGPU_GEM_CREATE_NO_CPU_ACCESS;

	r = amdgpu_bo_alloc(device_handle, &request, &buf_handle);
	/* Don't let the test fail if the device doesn't have enough VRAM */
	if (r)
		return;

	r = amdgpu_va_range_alloc(device_handle, amdgpu_gpu_va_range_general,
				  4ULL << 30, 1ULL << 30, 0, &vmc_addr,
				  &handle, 0);
	CU_ASSERT_EQUAL(r, 0);
	if (r)
		goto error_va_alloc;

	vmc_addr += 1 << 12;

	r = amdgpu_bo_va_op(buf_handle, 0, map_size, vmc_addr, 0,
			    AMDGPU_VA_OP_MAP);
	CU_ASSERT_EQUAL(r, 0);
	if (r)
		goto error_va_alloc;

	amdgpu_bo_va_op(buf_handle, 0, map_size, vmc_addr, 0,
			AMDGPU_VA_OP_UNMAP);

error_va_alloc:
	amdgpu_bo_free(buf_handle);
}

static void amdgpu_vm_mapping_test(void)
{
	struct amdgpu_bo_alloc_request req = {0};
	struct drm_amdgpu_info_device dev_info;
	const uint64_t size = 4096;
	amdgpu_bo_handle buf;
	uint64_t addr;
	int r;

	req.alloc_size = size;
	req.phys_alignment = 0;
	req.preferred_heap = AMDGPU_GEM_DOMAIN_GTT;
	req.flags = 0;

	r = amdgpu_bo_alloc(device_handle, &req, &buf);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_query_info(device_handle, AMDGPU_INFO_DEV_INFO,
			      sizeof(dev_info), &dev_info);
	CU_ASSERT_EQUAL(r, 0);

	addr = dev_info.virtual_address_offset;
	r = amdgpu_bo_va_op(buf, 0, size, addr, 0, AMDGPU_VA_OP_MAP);
	CU_ASSERT_EQUAL(r, 0);

	addr = dev_info.virtual_address_max - size;
	r = amdgpu_bo_va_op(buf, 0, size, addr, 0, AMDGPU_VA_OP_MAP);
	CU_ASSERT_EQUAL(r, 0);

	if (dev_info.high_va_offset) {
		addr = dev_info.high_va_offset;
		r = amdgpu_bo_va_op(buf, 0, size, addr, 0, AMDGPU_VA_OP_MAP);
		CU_ASSERT_EQUAL(r, 0);

		addr = dev_info.high_va_max - size;
		r = amdgpu_bo_va_op(buf, 0, size, addr, 0, AMDGPU_VA_OP_MAP);
		CU_ASSERT_EQUAL(r, 0);
	}

	amdgpu_bo_free(buf);
}
