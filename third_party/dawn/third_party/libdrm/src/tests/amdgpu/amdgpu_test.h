/*
 * Copyright 2014 Advanced Micro Devices, Inc.
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

#ifndef _AMDGPU_TEST_H_
#define _AMDGPU_TEST_H_

#include "amdgpu.h"
#include "amdgpu_drm.h"

/**
 * Define max. number of card in system which we are able to handle
 */
#define MAX_CARDS_SUPPORTED     128

/* Forward reference for array to keep "drm" handles */
extern int drm_amdgpu[MAX_CARDS_SUPPORTED];

/* Global variables */
extern int open_render_node;

/*************************  Basic test suite ********************************/

/*
 * Define basic test suite to serve as the starting point for future testing
*/

/**
 * Initialize basic test suite
 */
int suite_basic_tests_init();

/**
 * Deinitialize basic test suite
 */
int suite_basic_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_basic_tests_enable(void);

/**
 * Tests in basic test suite
 */
extern CU_TestInfo basic_tests[];

/**
 * Initialize bo test suite
 */
int suite_bo_tests_init();

/**
 * Deinitialize bo test suite
 */
int suite_bo_tests_clean();

/**
 * Tests in bo test suite
 */
extern CU_TestInfo bo_tests[];

/**
 * Initialize cs test suite
 */
int suite_cs_tests_init();

/**
 * Deinitialize cs test suite
 */
int suite_cs_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_cs_tests_enable(void);

/**
 * Tests in cs test suite
 */
extern CU_TestInfo cs_tests[];

/**
 * Initialize vce test suite
 */
int suite_vce_tests_init();

/**
 * Deinitialize vce test suite
 */
int suite_vce_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_vce_tests_enable(void);

/**
 * Tests in vce test suite
 */
extern CU_TestInfo vce_tests[];

/**
+ * Initialize vcn test suite
+ */
int suite_vcn_tests_init();

/**
+ * Deinitialize vcn test suite
+ */
int suite_vcn_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_vcn_tests_enable(void);

/**
+ * Tests in vcn test suite
+ */
extern CU_TestInfo vcn_tests[];

/**
+ * Initialize jpeg test suite
+ */
int suite_jpeg_tests_init();

/**
+ * Deinitialize jpeg test suite
+ */
int suite_jpeg_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_jpeg_tests_enable(void);

/**
+ * Tests in vcn test suite
+ */
extern CU_TestInfo jpeg_tests[];

/**
 * Initialize uvd enc test suite
 */
int suite_uvd_enc_tests_init();

/**
 * Deinitialize uvd enc test suite
 */
int suite_uvd_enc_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_uvd_enc_tests_enable(void);

/**
 * Tests in uvd enc test suite
 */
extern CU_TestInfo uvd_enc_tests[];

/**
 * Initialize deadlock test suite
 */
int suite_deadlock_tests_init();

/**
 * Deinitialize deadlock test suite
 */
int suite_deadlock_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_deadlock_tests_enable(void);

/**
 * Tests in uvd enc test suite
 */
extern CU_TestInfo deadlock_tests[];

/**
 * Initialize vm test suite
 */
int suite_vm_tests_init();

/**
 * Deinitialize deadlock test suite
 */
int suite_vm_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_vm_tests_enable(void);

/**
 * Tests in vm test suite
 */
extern CU_TestInfo vm_tests[];


/**
 * Initialize ras test suite
 */
int suite_ras_tests_init();

/**
 * Deinitialize deadlock test suite
 */
int suite_ras_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_ras_tests_enable(void);

/**
 * Tests in ras test suite
 */
extern CU_TestInfo ras_tests[];


/**
 * Initialize syncobj timeline test suite
 */
int suite_syncobj_timeline_tests_init();

/**
 * Deinitialize syncobj timeline test suite
 */
int suite_syncobj_timeline_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_syncobj_timeline_tests_enable(void);

/**
 * Tests in syncobj timeline test suite
 */
extern CU_TestInfo syncobj_timeline_tests[];


/**
 * Initialize cp dma test suite
 */
int suite_cp_dma_tests_init();

/**
 * Deinitialize cp dma test suite
 */
int suite_cp_dma_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_cp_dma_tests_enable(void);

/**
 * Tests in cp dma test suite
 */
extern CU_TestInfo cp_dma_tests[];

/**
 * Initialize security test suite
 */
int suite_security_tests_init();

/**
 * Deinitialize security test suite
 */
int suite_security_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_security_tests_enable(void);

/**
 * Tests in security test suite
 */
extern CU_TestInfo security_tests[];

extern void
amdgpu_command_submission_write_linear_helper_with_secure(amdgpu_device_handle
							  device,
							  unsigned ip_type,
							  bool secure);

extern void amdgpu_test_dispatch_helper(amdgpu_device_handle device_handle, unsigned ip);
extern void amdgpu_test_dispatch_hang_helper(amdgpu_device_handle device_handle, uint32_t ip);
extern void amdgpu_test_dispatch_hang_slow_helper(amdgpu_device_handle device_handle, uint32_t ip);
extern void amdgpu_test_draw_helper(amdgpu_device_handle device_handle);
extern void amdgpu_test_draw_hang_helper(amdgpu_device_handle device_handle);
extern void amdgpu_test_draw_hang_slow_helper(amdgpu_device_handle device_handle);

/**
 * Initialize hotunplug test suite
 */
int suite_hotunplug_tests_init();

/**
 * Deinitialize hotunplug test suite
 */
int suite_hotunplug_tests_clean();

/**
 * Decide if the suite is enabled by default or not.
 */
CU_BOOL suite_hotunplug_tests_enable(void);

/**
 * Tests in uvd enc test suite
 */
extern CU_TestInfo hotunplug_tests[];


/**
 * Helper functions
 */
static inline amdgpu_bo_handle gpu_mem_alloc(
					amdgpu_device_handle device_handle,
					uint64_t size,
					uint64_t alignment,
					uint32_t type,
					uint64_t flags,
					uint64_t *vmc_addr,
					amdgpu_va_handle *va_handle)
{
	struct amdgpu_bo_alloc_request req = {0};
	amdgpu_bo_handle buf_handle = NULL;
	int r;

	req.alloc_size = size;
	req.phys_alignment = alignment;
	req.preferred_heap = type;
	req.flags = flags;

	r = amdgpu_bo_alloc(device_handle, &req, &buf_handle);
	CU_ASSERT_EQUAL(r, 0);
	if (r)
		return NULL;

	if (vmc_addr && va_handle) {
		r = amdgpu_va_range_alloc(device_handle,
					  amdgpu_gpu_va_range_general,
					  size, alignment, 0, vmc_addr,
					  va_handle, 0);
		CU_ASSERT_EQUAL(r, 0);
		if (r)
			goto error_free_bo;

		r = amdgpu_bo_va_op(buf_handle, 0, size, *vmc_addr, 0,
				    AMDGPU_VA_OP_MAP);
		CU_ASSERT_EQUAL(r, 0);
		if (r)
			goto error_free_va;
	}

	return buf_handle;

error_free_va:
	r = amdgpu_va_range_free(*va_handle);
	CU_ASSERT_EQUAL(r, 0);

error_free_bo:
	r = amdgpu_bo_free(buf_handle);
	CU_ASSERT_EQUAL(r, 0);

	return NULL;
}

static inline int gpu_mem_free(amdgpu_bo_handle bo,
			       amdgpu_va_handle va_handle,
			       uint64_t vmc_addr,
			       uint64_t size)
{
	int r;

	if (!bo)
		return 0;

	if (va_handle) {
		r = amdgpu_bo_va_op(bo, 0, size, vmc_addr, 0,
				    AMDGPU_VA_OP_UNMAP);
		CU_ASSERT_EQUAL(r, 0);
		if (r)
			return r;

		r = amdgpu_va_range_free(va_handle);
		CU_ASSERT_EQUAL(r, 0);
		if (r)
			return r;
	}

	r = amdgpu_bo_free(bo);
	CU_ASSERT_EQUAL(r, 0);

	return r;
}

static inline int
amdgpu_bo_alloc_wrap(amdgpu_device_handle dev, unsigned size,
		     unsigned alignment, unsigned heap, uint64_t flags,
		     amdgpu_bo_handle *bo)
{
	struct amdgpu_bo_alloc_request request = {};
	amdgpu_bo_handle buf_handle;
	int r;

	request.alloc_size = size;
	request.phys_alignment = alignment;
	request.preferred_heap = heap;
	request.flags = flags;

	r = amdgpu_bo_alloc(dev, &request, &buf_handle);
	if (r)
		return r;

	*bo = buf_handle;

	return 0;
}

int amdgpu_bo_alloc_and_map_raw(amdgpu_device_handle dev, unsigned size,
			unsigned alignment, unsigned heap, uint64_t alloc_flags,
			uint64_t mapping_flags, amdgpu_bo_handle *bo, void **cpu,
			uint64_t *mc_address,
			amdgpu_va_handle *va_handle);

static inline int
amdgpu_bo_alloc_and_map(amdgpu_device_handle dev, unsigned size,
			unsigned alignment, unsigned heap, uint64_t alloc_flags,
			amdgpu_bo_handle *bo, void **cpu, uint64_t *mc_address,
			amdgpu_va_handle *va_handle)
{
	return amdgpu_bo_alloc_and_map_raw(dev, size, alignment, heap,
					alloc_flags, 0, bo, cpu, mc_address, va_handle);
}

static inline int
amdgpu_bo_unmap_and_free(amdgpu_bo_handle bo, amdgpu_va_handle va_handle,
			 uint64_t mc_addr, uint64_t size)
{
	amdgpu_bo_cpu_unmap(bo);
	amdgpu_bo_va_op(bo, 0, size, mc_addr, 0, AMDGPU_VA_OP_UNMAP);
	amdgpu_va_range_free(va_handle);
	amdgpu_bo_free(bo);

	return 0;

}

static inline int
amdgpu_get_bo_list(amdgpu_device_handle dev, amdgpu_bo_handle bo1,
		   amdgpu_bo_handle bo2, amdgpu_bo_list_handle *list)
{
	amdgpu_bo_handle resources[] = {bo1, bo2};

	return amdgpu_bo_list_create(dev, bo2 ? 2 : 1, resources, NULL, list);
}


static inline CU_ErrorCode amdgpu_set_suite_active(const char *suite_name,
							  CU_BOOL active)
{
	CU_ErrorCode r = CU_set_suite_active(CU_get_suite(suite_name), active);

	if (r != CUE_SUCCESS)
		fprintf(stderr, "Failed to obtain suite %s\n", suite_name);

	return r;
}

static inline CU_ErrorCode amdgpu_set_test_active(const char *suite_name,
				  const char *test_name, CU_BOOL active)
{
	CU_ErrorCode r;
	CU_pSuite pSuite = CU_get_suite(suite_name);

	if (!pSuite) {
		fprintf(stderr, "Failed to obtain suite %s\n",
				suite_name);
		return CUE_NOSUITE;
	}

	r = CU_set_test_active(CU_get_test(pSuite, test_name), active);
	if (r != CUE_SUCCESS)
		fprintf(stderr, "Failed to obtain test %s\n", test_name);

	return r;
}


static inline bool asic_is_gfx_pipe_removed(uint32_t family_id, uint32_t chip_id, uint32_t chip_rev)
{

	if (family_id != AMDGPU_FAMILY_AI)
	return false;

	switch (chip_id - chip_rev) {
	/* Arcturus */
	case 0x32:
	/* Aldebaran */
	case 0x3c:
		return true;
	default:
		return false;
	}
}

void amdgpu_test_exec_cs_helper_raw(amdgpu_device_handle device_handle,
				    amdgpu_context_handle context_handle,
				    unsigned ip_type, int instance, int pm4_dw,
				    uint32_t *pm4_src, int res_cnt,
				    amdgpu_bo_handle *resources,
				    struct amdgpu_cs_ib_info *ib_info,
				    struct amdgpu_cs_request *ibs_request,
				    bool secure);

void amdgpu_close_devices();
int amdgpu_open_device_on_test_index(int render_node);
char *amdgpu_get_device_from_fd(int fd);

#endif  /* #ifdef _AMDGPU_TEST_H_ */
