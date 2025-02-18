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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "CUnit/Basic.h"

#include "amdgpu_test.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"

#define IB_SIZE 4096
#define MAX_RESOURCES 8

#define DMA_SIZE 4097 
#define DMA_DATA_BYTE 0xea

static bool do_p2p;

static amdgpu_device_handle executing_device_handle;
static uint32_t executing_device_major_version;
static uint32_t executing_device_minor_version;

static amdgpu_device_handle peer_exporting_device_handle;
static uint32_t peer_exporting_device_major_version;
static uint32_t peer_exporting_device_minor_version;

static amdgpu_context_handle context_handle;
static amdgpu_bo_handle ib_handle;
static uint32_t *ib_cpu;
static uint64_t ib_mc_address;
static amdgpu_va_handle ib_va_handle;
static uint32_t num_dword;

static amdgpu_bo_handle resources[MAX_RESOURCES];
static unsigned num_resources;

static uint8_t* reference_data;

static void amdgpu_cp_dma_host_to_vram(void);
static void amdgpu_cp_dma_vram_to_host(void);
static void amdgpu_cp_dma_p2p_vram_to_vram(void);
static void amdgpu_cp_dma_p2p_host_to_vram(void);
static void amdgpu_cp_dma_p2p_vram_to_host(void);

/**
 * Tests in cp dma test suite
 */
CU_TestInfo cp_dma_tests[] = {
	{ "CP DMA write Host to VRAM",  amdgpu_cp_dma_host_to_vram },
	{ "CP DMA write VRAM to Host",  amdgpu_cp_dma_vram_to_host },

	{ "Peer to Peer CP DMA write VRAM to VRAM",  amdgpu_cp_dma_p2p_vram_to_vram },
	{ "Peer to Peer CP DMA write Host to VRAM",  amdgpu_cp_dma_p2p_host_to_vram },
	{ "Peer to Peer CP DMA write VRAM to Host",  amdgpu_cp_dma_p2p_vram_to_host },
	CU_TEST_INFO_NULL,
};

struct amdgpu_cp_dma_bo{
	amdgpu_bo_handle buf_handle;
	amdgpu_va_handle va_handle;
	uint64_t gpu_va;
	uint64_t size;
};

static int allocate_bo_and_va(amdgpu_device_handle dev,
		uint64_t size, uint64_t alignment,
		uint32_t heap, uint64_t alloc_flags,
		struct amdgpu_cp_dma_bo *bo) {
	struct amdgpu_bo_alloc_request request = {};
	amdgpu_bo_handle buf_handle;
	amdgpu_va_handle va_handle;
	uint64_t vmc_addr;
	int r;

	request.alloc_size = size;
	request.phys_alignment = alignment;
	request.preferred_heap = heap;
	request.flags = alloc_flags;

	r = amdgpu_bo_alloc(dev, &request, &buf_handle);
	if (r)
		goto error_bo_alloc;

	r = amdgpu_va_range_alloc(dev, amdgpu_gpu_va_range_general,
			size, alignment, 0,
			&vmc_addr, &va_handle, 0);
	if (r)
		goto error_va_alloc;

	r = amdgpu_bo_va_op(buf_handle, 0, size, vmc_addr,
						AMDGPU_VM_PAGE_READABLE |
							AMDGPU_VM_PAGE_WRITEABLE |
							AMDGPU_VM_PAGE_EXECUTABLE,
						AMDGPU_VA_OP_MAP);
	if (r)
		goto error_va_map;

	bo->buf_handle = buf_handle;
	bo->va_handle = va_handle;
	bo->gpu_va = vmc_addr;
	bo->size = size;

	return 0;

error_va_map:
	amdgpu_bo_va_op(buf_handle, 0,
			size, vmc_addr, 0, AMDGPU_VA_OP_UNMAP);

error_va_alloc:
	amdgpu_va_range_free(va_handle);

error_bo_alloc:
	amdgpu_bo_free(buf_handle);

	return r;
}

static int import_dma_buf_to_bo(amdgpu_device_handle dev,
		int dmabuf_fd, struct amdgpu_cp_dma_bo *bo) {
	amdgpu_va_handle va_handle;
	uint64_t vmc_addr;
	int r;
	struct amdgpu_bo_import_result bo_import_result = {};

	r = amdgpu_bo_import(dev, amdgpu_bo_handle_type_dma_buf_fd,
			dmabuf_fd, &bo_import_result);
	if (r)
		goto error_bo_import;

	r = amdgpu_va_range_alloc(dev, amdgpu_gpu_va_range_general,
				bo_import_result.alloc_size, 0, 0,
				&vmc_addr, &va_handle, 0);
	if (r)
		goto error_va_alloc;

	r = amdgpu_bo_va_op(bo_import_result.buf_handle, 0,
			bo_import_result.alloc_size, vmc_addr,
			AMDGPU_VM_PAGE_READABLE |
				AMDGPU_VM_PAGE_WRITEABLE |
				AMDGPU_VM_PAGE_EXECUTABLE,
			AMDGPU_VA_OP_MAP);
	if (r)
		goto error_va_map;

	bo->buf_handle = bo_import_result.buf_handle;
	bo->va_handle = va_handle;
	bo->gpu_va = vmc_addr;
	bo->size = bo_import_result.alloc_size;

	return 0;

error_va_map:
	amdgpu_bo_va_op(bo_import_result.buf_handle, 0,
			bo_import_result.alloc_size, vmc_addr, 0, AMDGPU_VA_OP_UNMAP);

error_va_alloc:
	amdgpu_va_range_free(va_handle);

error_bo_import:
	amdgpu_bo_free(bo_import_result.buf_handle);

	return r;
}

static int free_bo(struct amdgpu_cp_dma_bo bo) {
	int r;
	r = amdgpu_bo_va_op(bo.buf_handle, 0,
			bo.size, bo.gpu_va, 0, AMDGPU_VA_OP_UNMAP);
	if(r)
		return r;

	r = amdgpu_va_range_free(bo.va_handle);
	if(r)
		return r;

	r = amdgpu_bo_free(bo.buf_handle);
	if(r)
		return r;

	return 0;
}

static int submit_and_sync() {
	struct amdgpu_cs_request ibs_request = {0};
	struct amdgpu_cs_ib_info ib_info = {0};
	struct amdgpu_cs_fence fence_status = {0};
	uint32_t expired;
	uint32_t family_id, chip_id, chip_rev;
	unsigned gc_ip_type;
	int r;

	r = amdgpu_bo_list_create(executing_device_handle,
			num_resources, resources,
			NULL, &ibs_request.resources);
	if (r)
		return r;

	family_id = executing_device_handle->info.family_id;
	chip_id = executing_device_handle->info.chip_external_rev;
	chip_rev = executing_device_handle->info.chip_rev;

	gc_ip_type = (asic_is_gfx_pipe_removed(family_id, chip_id, chip_rev)) ?
		AMDGPU_HW_IP_COMPUTE : AMDGPU_HW_IP_GFX;

	ib_info.ib_mc_address = ib_mc_address;
	ib_info.size = num_dword;

	ibs_request.ip_type = gc_ip_type;
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
	fence_status.ip_type = gc_ip_type;
	fence_status.fence = ibs_request.seq_no;

	r = amdgpu_cs_query_fence_status(&fence_status,
			AMDGPU_TIMEOUT_INFINITE,
			0, &expired);
	if (r)
		return r;

	return 0;
} 

static void cp_dma_cmd(struct amdgpu_cp_dma_bo src_bo,
		struct amdgpu_cp_dma_bo dst_bo) {
	_Static_assert(DMA_SIZE < (1 << 26), "DMA size exceeds CP DMA maximium!");

	ib_cpu[0] = 0xc0055000;
	ib_cpu[1] = 0x80000000;
	ib_cpu[2] = src_bo.gpu_va & 0x00000000ffffffff;
	ib_cpu[3] = (src_bo.gpu_va & 0xffffffff00000000) >> 32;
	ib_cpu[4] = dst_bo.gpu_va & 0x00000000ffffffff;
	ib_cpu[5] = (dst_bo.gpu_va & 0xffffffff00000000) >> 32;
	// size is read from the lower 26bits. 
	ib_cpu[6] = ((1 << 26) - 1) & DMA_SIZE;
	ib_cpu[7] = 0xffff1000;

	num_dword = 8;

	resources[0] = src_bo.buf_handle;
	resources[1] = dst_bo.buf_handle;
	resources[2] = ib_handle;
	num_resources = 3;
}

static void amdgpu_cp_dma(uint32_t src_heap, uint32_t dst_heap) {
	int r;
	struct amdgpu_cp_dma_bo src_bo = {0};
	struct amdgpu_cp_dma_bo dst_bo = {0};
	void *src_bo_cpu;
	void *dst_bo_cpu;

	/* allocate the src bo, set its data to DMA_DATA_BYTE */
	r = allocate_bo_and_va(executing_device_handle, DMA_SIZE, 4096,
			src_heap, AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED, &src_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_cpu_map(src_bo.buf_handle, (void **)&src_bo_cpu);
	CU_ASSERT_EQUAL(r, 0);
	memset(src_bo_cpu, DMA_DATA_BYTE, DMA_SIZE);

	r = amdgpu_bo_cpu_unmap(src_bo.buf_handle);
	CU_ASSERT_EQUAL(r, 0);

	/* allocate the dst bo and clear its content to all 0 */
	r = allocate_bo_and_va(executing_device_handle, DMA_SIZE, 4096,
			dst_heap, AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED, &dst_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_cpu_map(dst_bo.buf_handle, (void **)&dst_bo_cpu);
	CU_ASSERT_EQUAL(r, 0);

	_Static_assert(DMA_DATA_BYTE != 0, "Initialization data should be different from DMA data!");
	memset(dst_bo_cpu, 0, DMA_SIZE);

	/* record CP DMA command and dispatch the command */
	cp_dma_cmd(src_bo, dst_bo);

	r = submit_and_sync();
	CU_ASSERT_EQUAL(r, 0);

	/* verify the dst bo is filled with DMA_DATA_BYTE */
	CU_ASSERT_EQUAL(memcmp(dst_bo_cpu, reference_data, DMA_SIZE) == 0, true);

	r = amdgpu_bo_cpu_unmap(dst_bo.buf_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = free_bo(src_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = free_bo(dst_bo);
	CU_ASSERT_EQUAL(r, 0);
}

static void amdgpu_cp_dma_p2p(uint32_t src_heap, uint32_t dst_heap) {
	int r;
	struct amdgpu_cp_dma_bo exported_bo = {0};
	int dma_buf_fd;
	int dma_buf_fd_dup;
	struct amdgpu_cp_dma_bo src_bo = {0};
	struct amdgpu_cp_dma_bo imported_dst_bo = {0};
	void *exported_bo_cpu;
	void *src_bo_cpu;

	/* allocate a bo on the peer device and export it to dma-buf */
	r = allocate_bo_and_va(peer_exporting_device_handle, DMA_SIZE, 4096,
			src_heap, AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED, &exported_bo);
	CU_ASSERT_EQUAL(r, 0);

	/* map the exported bo and clear its content to 0 */
	_Static_assert(DMA_DATA_BYTE != 0, "Initialization data should be different from DMA data!");
	r = amdgpu_bo_cpu_map(exported_bo.buf_handle, (void **)&exported_bo_cpu);
	CU_ASSERT_EQUAL(r, 0);
	memset(exported_bo_cpu, 0, DMA_SIZE);

	r = amdgpu_bo_export(exported_bo.buf_handle,
			amdgpu_bo_handle_type_dma_buf_fd, (uint32_t*)&dma_buf_fd);
	CU_ASSERT_EQUAL(r, 0);

    // According to amdgpu_drm:
	// "Buffer must be "imported" only using new "fd"
	// (different from one used by "exporter")"
	dma_buf_fd_dup = dup(dma_buf_fd);
	r = close(dma_buf_fd);
	CU_ASSERT_EQUAL(r, 0);

	/* import the dma-buf to the executing device, imported bo is the DMA destination */
	r = import_dma_buf_to_bo(
			executing_device_handle, dma_buf_fd_dup, &imported_dst_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = close(dma_buf_fd_dup);
	CU_ASSERT_EQUAL(r, 0);

	/* allocate the src bo and set its content to DMA_DATA_BYTE */
	r = allocate_bo_and_va(executing_device_handle, DMA_SIZE, 4096,
			dst_heap, AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED, &src_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_cpu_map(src_bo.buf_handle, (void **)&src_bo_cpu);
	CU_ASSERT_EQUAL(r, 0);

	memset(src_bo_cpu, DMA_DATA_BYTE, DMA_SIZE);

	r = amdgpu_bo_cpu_unmap(src_bo.buf_handle);
	CU_ASSERT_EQUAL(r, 0);

	/* record CP DMA command and dispatch the command */
	cp_dma_cmd(src_bo, imported_dst_bo);

	r = submit_and_sync();
	CU_ASSERT_EQUAL(r, 0);

	/* verify the bo from the peer device is filled with DMA_DATA_BYTE */
	CU_ASSERT_EQUAL(memcmp(exported_bo_cpu, reference_data, DMA_SIZE) == 0, true);

	r = amdgpu_bo_cpu_unmap(exported_bo.buf_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = free_bo(exported_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = free_bo(imported_dst_bo);
	CU_ASSERT_EQUAL(r, 0);

	r = free_bo(src_bo);
	CU_ASSERT_EQUAL(r, 0);
}

static void amdgpu_cp_dma_host_to_vram(void) {
	amdgpu_cp_dma(AMDGPU_GEM_DOMAIN_GTT, AMDGPU_GEM_DOMAIN_VRAM);
}

static void amdgpu_cp_dma_vram_to_host(void) {
	amdgpu_cp_dma(AMDGPU_GEM_DOMAIN_VRAM, AMDGPU_GEM_DOMAIN_GTT);
}

static void amdgpu_cp_dma_p2p_vram_to_vram(void) {
	amdgpu_cp_dma_p2p(AMDGPU_GEM_DOMAIN_VRAM, AMDGPU_GEM_DOMAIN_VRAM);
}

static void amdgpu_cp_dma_p2p_host_to_vram(void) {
	amdgpu_cp_dma_p2p(AMDGPU_GEM_DOMAIN_GTT, AMDGPU_GEM_DOMAIN_VRAM);
}

static void amdgpu_cp_dma_p2p_vram_to_host(void) {
	amdgpu_cp_dma_p2p(AMDGPU_GEM_DOMAIN_VRAM, AMDGPU_GEM_DOMAIN_GTT);
}

int suite_cp_dma_tests_init() {
	int r;
	
	r = amdgpu_device_initialize(drm_amdgpu[0],
			&executing_device_major_version,
			&executing_device_minor_version,
			&executing_device_handle);
	if (r)
		return CUE_SINIT_FAILED;
	
	r = amdgpu_cs_ctx_create(executing_device_handle, &context_handle);
	if (r)
		return CUE_SINIT_FAILED;

	r = amdgpu_bo_alloc_and_map(executing_device_handle, IB_SIZE, 4096,
					AMDGPU_GEM_DOMAIN_GTT, 0,
					&ib_handle, (void**)&ib_cpu,
					&ib_mc_address, &ib_va_handle);
	if (r)
		return CUE_SINIT_FAILED;
	
	if (do_p2p) {
		r = amdgpu_device_initialize(drm_amdgpu[1],
				&peer_exporting_device_major_version,
				&peer_exporting_device_minor_version,
				&peer_exporting_device_handle);
		
		if (r)
			return CUE_SINIT_FAILED;
	}

	reference_data = (uint8_t*)malloc(DMA_SIZE);
	if (!reference_data)
		return CUE_SINIT_FAILED;
	memset(reference_data, DMA_DATA_BYTE, DMA_SIZE);

	return CUE_SUCCESS;
}

int suite_cp_dma_tests_clean() {
	int r;

	free(reference_data);

	r = amdgpu_bo_unmap_and_free(ib_handle, ib_va_handle,
				 ib_mc_address, IB_SIZE);
	if (r)
		return CUE_SCLEAN_FAILED;

	r = amdgpu_cs_ctx_free(context_handle);
	if (r)
		return CUE_SCLEAN_FAILED;

	r = amdgpu_device_deinitialize(executing_device_handle);
	if (r)
		return CUE_SCLEAN_FAILED;

	if (do_p2p) {
		r = amdgpu_device_deinitialize(peer_exporting_device_handle);
		if (r)
			return CUE_SCLEAN_FAILED;
	}

	return CUE_SUCCESS;
}

CU_BOOL suite_cp_dma_tests_enable(void) {
	int r = 0;

	if (amdgpu_device_initialize(drm_amdgpu[0],
			&executing_device_major_version,
			&executing_device_minor_version,
			&executing_device_handle))
		return CU_FALSE;

	if (!(executing_device_handle->info.family_id >= AMDGPU_FAMILY_AI &&
			executing_device_handle->info.family_id <= AMDGPU_FAMILY_NV)) {
		printf("Testing device has ASIC that is not supported by CP-DMA test suite!\n");
		return CU_FALSE;
	}

	if (amdgpu_device_deinitialize(executing_device_handle))
		return CU_FALSE;	

	if (drm_amdgpu[1] >= 0) {
		r = amdgpu_device_initialize(drm_amdgpu[1],
				&peer_exporting_device_major_version,
				&peer_exporting_device_minor_version,
				&peer_exporting_device_handle);
		
		if (r == 0 && (peer_exporting_device_handle->info.family_id >= AMDGPU_FAMILY_AI &&
						peer_exporting_device_handle->info.family_id <= AMDGPU_FAMILY_NV)) {
			do_p2p = true;
		}

		if (r == 0 && amdgpu_device_deinitialize(peer_exporting_device_handle) != 0) {
			printf("Deinitialize peer_exporting_device_handle failed!\n");
			return CU_FALSE;
		}
	}

	if (!do_p2p) {
		amdgpu_set_test_active("CP DMA Tests", "Peer to Peer CP DMA write VRAM to VRAM", CU_FALSE);
		amdgpu_set_test_active("CP DMA Tests", "Peer to Peer CP DMA write Host to VRAM", CU_FALSE);
		amdgpu_set_test_active("CP DMA Tests", "Peer to Peer CP DMA write VRAM to Host", CU_FALSE);
		printf("Peer device is not opened or has ASIC not supported by the suite, skip all Peer to Peer tests.\n");
	}
	
	return CU_TRUE;
}
