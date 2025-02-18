/*
 * Copyright 2021 Advanced Micro Devices, Inc.
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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if HAVE_ALLOCA_H
# include <alloca.h>
#endif

#include "CUnit/Basic.h"

#include "amdgpu_test.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"
#include "xf86drm.h"
#include <pthread.h>

#define GFX_COMPUTE_NOP  0xffff1000

static  amdgpu_device_handle device_handle;
static  uint32_t  major_version;
static  uint32_t  minor_version;
static char *sysfs_remove = NULL;
static bool do_cs;

CU_BOOL suite_hotunplug_tests_enable(void)
{
	CU_BOOL enable = CU_TRUE;
	drmDevicePtr device;

	if (drmGetDevice2(drm_amdgpu[0], DRM_DEVICE_GET_PCI_REVISION, &device)) {
		printf("\n\nGPU Failed to get DRM device PCI info!\n");
		return CU_FALSE;
	}

	if (device->bustype != DRM_BUS_PCI) {
		printf("\n\nGPU device is not on PCI bus!\n");
		amdgpu_device_deinitialize(device_handle);
		return CU_FALSE;
	}

	if (amdgpu_device_initialize(drm_amdgpu[0], &major_version,
					     &minor_version, &device_handle))
		return CU_FALSE;
	
	/* Latest tested amdgpu version to work with all the tests */
        if (minor_version < 46)
                enable = false;

        /* skip hotplug test on APUs */
        if(device_handle->dev_info.ids_flags & AMDGPU_IDS_FLAGS_FUSION)
                enable = false;

	if (amdgpu_device_deinitialize(device_handle))
		return CU_FALSE;

	return enable;
}

int suite_hotunplug_tests_init(void)
{
	/* We need to open/close device at each test manually */
	amdgpu_close_devices();

	return CUE_SUCCESS;
}

int suite_hotunplug_tests_clean(void)
{


	return CUE_SUCCESS;
}

static int amdgpu_hotunplug_trigger(const char *pathname)
{
	int fd, len;

	fd = open(pathname, O_WRONLY);
	if (fd < 0)
		return -errno;

	len = write(fd, "1", 1);
	close(fd);

	return len;
}

static int amdgpu_hotunplug_setup_test()
{
	int r;
	char *tmp_str;

	if (amdgpu_open_device_on_test_index(open_render_node) < 0) {
		printf("\n\n Failed to reopen device file!\n");
		return CUE_SINIT_FAILED;



	}

	r = amdgpu_device_initialize(drm_amdgpu[0], &major_version,
				   &minor_version, &device_handle);

	if (r) {
		if ((r == -EACCES) && (errno == EACCES))
			printf("\n\nError:%s. "
				"Hint:Try to run this test program as root.",
				strerror(errno));
		return CUE_SINIT_FAILED;
	}

	tmp_str = amdgpu_get_device_from_fd(drm_amdgpu[0]);
	if (!tmp_str){
		printf("\n\n Device path not found!\n");
		return  CUE_SINIT_FAILED;
	}

	sysfs_remove = realloc(tmp_str, strlen(tmp_str) * 2);
	strcat(sysfs_remove, "/remove");

	return 0;
}

static int amdgpu_hotunplug_teardown_test()
{
	if (amdgpu_device_deinitialize(device_handle))
		return CUE_SCLEAN_FAILED;

	amdgpu_close_devices();

	if (sysfs_remove)
		free(sysfs_remove);

	return 0;
}

static inline int amdgpu_hotunplug_remove()
{
	return amdgpu_hotunplug_trigger(sysfs_remove);
}

static inline int amdgpu_hotunplug_rescan()
{
	return amdgpu_hotunplug_trigger("/sys/bus/pci/rescan");
}

static int amdgpu_cs_sync(amdgpu_context_handle context,
			   unsigned int ip_type,
			   int ring,
			   unsigned int seqno)
{
	struct amdgpu_cs_fence fence = {
		.context = context,
		.ip_type = ip_type,
		.ring = ring,
		.fence = seqno,
	};
	uint32_t expired;

	return  amdgpu_cs_query_fence_status(&fence,
					   AMDGPU_TIMEOUT_INFINITE,
					   0, &expired);
}

static void *amdgpu_nop_cs()
{
	amdgpu_bo_handle ib_result_handle;
	void *ib_result_cpu;
	uint64_t ib_result_mc_address;
	uint32_t *ptr;
	int i, r;
	amdgpu_bo_list_handle bo_list;
	amdgpu_va_handle va_handle;
	amdgpu_context_handle context;
	struct amdgpu_cs_request ibs_request;
	struct amdgpu_cs_ib_info ib_info;

	r = amdgpu_cs_ctx_create(device_handle, &context);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_alloc_and_map(device_handle, 4096, 4096,
				    AMDGPU_GEM_DOMAIN_GTT, 0,
				    &ib_result_handle, &ib_result_cpu,
				    &ib_result_mc_address, &va_handle);
	CU_ASSERT_EQUAL(r, 0);

	ptr = ib_result_cpu;
	for (i = 0; i < 16; ++i)
		ptr[i] = GFX_COMPUTE_NOP;

	r = amdgpu_bo_list_create(device_handle, 1, &ib_result_handle, NULL, &bo_list);
	CU_ASSERT_EQUAL(r, 0);

	memset(&ib_info, 0, sizeof(struct amdgpu_cs_ib_info));
	ib_info.ib_mc_address = ib_result_mc_address;
	ib_info.size = 16;

	memset(&ibs_request, 0, sizeof(struct amdgpu_cs_request));
	ibs_request.ip_type = AMDGPU_HW_IP_GFX;
	ibs_request.ring = 0;
	ibs_request.number_of_ibs = 1;
	ibs_request.ibs = &ib_info;
	ibs_request.resources = bo_list;

	while (do_cs)
		amdgpu_cs_submit(context, 0, &ibs_request, 1);

	amdgpu_cs_sync(context, AMDGPU_HW_IP_GFX, 0, ibs_request.seq_no);
	amdgpu_bo_list_destroy(bo_list);
	amdgpu_bo_unmap_and_free(ib_result_handle, va_handle,
				 ib_result_mc_address, 4096);

	amdgpu_cs_ctx_free(context);

	return (void *)0;
}

static pthread_t* amdgpu_create_cs_thread()
{
	int r;
	pthread_t *thread = malloc(sizeof(*thread));
	if (!thread)
		return NULL;

	do_cs = true;

	r = pthread_create(thread, NULL, amdgpu_nop_cs, NULL);
	CU_ASSERT_EQUAL(r, 0);

	/* Give thread enough time to start*/
	usleep(100000);
	return thread;
}

static void amdgpu_destroy_cs_thread(pthread_t *thread)
{
	void *status;

	do_cs = false;

	pthread_join(*thread, &status);
	CU_ASSERT_EQUAL(status, 0);

	free(thread);
}


static void amdgpu_hotunplug_test(bool with_cs)
{
	int r;
	pthread_t *thread = NULL;

	r = amdgpu_hotunplug_setup_test();
	CU_ASSERT_EQUAL(r , 0);

	if (with_cs) {
		thread = amdgpu_create_cs_thread();
		CU_ASSERT_NOT_EQUAL(thread, NULL);
	}

	r = amdgpu_hotunplug_remove();
	CU_ASSERT_EQUAL(r > 0, 1);

	if (with_cs)
		amdgpu_destroy_cs_thread(thread);

	r = amdgpu_hotunplug_teardown_test();
	CU_ASSERT_EQUAL(r , 0);

	r = amdgpu_hotunplug_rescan();
	CU_ASSERT_EQUAL(r > 0, 1);
}

static void amdgpu_hotunplug_simple(void)
{
	amdgpu_hotunplug_test(false);
}

static void amdgpu_hotunplug_with_cs(void)
{
	amdgpu_hotunplug_test(true);
}

static void amdgpu_hotunplug_with_exported_bo(void)
{
	int r;
	uint32_t dma_buf_fd;
	unsigned int *ptr;
	amdgpu_bo_handle bo_handle;

	struct amdgpu_bo_alloc_request request = {
		.alloc_size = 4096,
		.phys_alignment = 4096,
		.preferred_heap = AMDGPU_GEM_DOMAIN_GTT,
		.flags = 0,
	};

	r = amdgpu_hotunplug_setup_test();
	CU_ASSERT_EQUAL(r , 0);

	amdgpu_bo_alloc(device_handle, &request, &bo_handle);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_export(bo_handle, amdgpu_bo_handle_type_dma_buf_fd, &dma_buf_fd);
	CU_ASSERT_EQUAL(r, 0);

	ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, dma_buf_fd, 0);
	CU_ASSERT_NOT_EQUAL(ptr,  MAP_FAILED);

	r = amdgpu_hotunplug_remove();
	CU_ASSERT_EQUAL(r > 0, 1);

	amdgpu_bo_free(bo_handle);

	r = amdgpu_hotunplug_teardown_test();
	CU_ASSERT_EQUAL(r , 0);

	*ptr = 0xdeafbeef;

	munmap(ptr, 4096);
	close (dma_buf_fd);

	r = amdgpu_hotunplug_rescan();
	CU_ASSERT_EQUAL(r > 0, 1);
}

static void amdgpu_hotunplug_with_exported_fence(void)
{
	amdgpu_bo_handle ib_result_handle;
	void *ib_result_cpu;
	uint64_t ib_result_mc_address;
	uint32_t *ptr, sync_obj_handle, sync_obj_handle2;
	int i, r;
	amdgpu_bo_list_handle bo_list;
	amdgpu_va_handle va_handle;
	uint32_t major2, minor2;
	amdgpu_device_handle device2;
	amdgpu_context_handle context;
	struct amdgpu_cs_request ibs_request;
	struct amdgpu_cs_ib_info ib_info;
	struct amdgpu_cs_fence fence_status = {0};
	int shared_fd;

	r = amdgpu_hotunplug_setup_test();
	CU_ASSERT_EQUAL(r , 0);

	r = amdgpu_device_initialize(drm_amdgpu[1], &major2, &minor2, &device2);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_cs_ctx_create(device_handle, &context);
	CU_ASSERT_EQUAL(r, 0);

	r = amdgpu_bo_alloc_and_map(device_handle, 4096, 4096,
				    AMDGPU_GEM_DOMAIN_GTT, 0,
				    &ib_result_handle, &ib_result_cpu,
				    &ib_result_mc_address, &va_handle);
	CU_ASSERT_EQUAL(r, 0);

	ptr = ib_result_cpu;
	for (i = 0; i < 16; ++i)
		ptr[i] = GFX_COMPUTE_NOP;

	r = amdgpu_bo_list_create(device_handle, 1, &ib_result_handle, NULL, &bo_list);
	CU_ASSERT_EQUAL(r, 0);

	memset(&ib_info, 0, sizeof(struct amdgpu_cs_ib_info));
	ib_info.ib_mc_address = ib_result_mc_address;
	ib_info.size = 16;

	memset(&ibs_request, 0, sizeof(struct amdgpu_cs_request));
	ibs_request.ip_type = AMDGPU_HW_IP_GFX;
	ibs_request.ring = 0;
	ibs_request.number_of_ibs = 1;
	ibs_request.ibs = &ib_info;
	ibs_request.resources = bo_list;

	CU_ASSERT_EQUAL(amdgpu_cs_submit(context, 0, &ibs_request, 1), 0);

	fence_status.context = context;
	fence_status.ip_type = AMDGPU_HW_IP_GFX;
	fence_status.ip_instance = 0;
	fence_status.fence = ibs_request.seq_no;

	CU_ASSERT_EQUAL(amdgpu_cs_fence_to_handle(device_handle, &fence_status,
						AMDGPU_FENCE_TO_HANDLE_GET_SYNCOBJ,
						&sync_obj_handle),
						0);

	CU_ASSERT_EQUAL(amdgpu_cs_export_syncobj(device_handle, sync_obj_handle, &shared_fd), 0);

	CU_ASSERT_EQUAL(amdgpu_cs_import_syncobj(device2, shared_fd, &sync_obj_handle2), 0);

	CU_ASSERT_EQUAL(amdgpu_cs_destroy_syncobj(device_handle, sync_obj_handle), 0);

	CU_ASSERT_EQUAL(amdgpu_bo_list_destroy(bo_list), 0);
	CU_ASSERT_EQUAL(amdgpu_bo_unmap_and_free(ib_result_handle, va_handle,
				 ib_result_mc_address, 4096), 0);
	CU_ASSERT_EQUAL(amdgpu_cs_ctx_free(context), 0);

	r = amdgpu_hotunplug_remove();
	CU_ASSERT_EQUAL(r > 0, 1);

	CU_ASSERT_EQUAL(amdgpu_cs_syncobj_wait(device2, &sync_obj_handle2, 1, 100000000, 0, NULL), 0);

	CU_ASSERT_EQUAL(amdgpu_cs_destroy_syncobj(device2, sync_obj_handle2), 0);

	amdgpu_device_deinitialize(device2);

	r = amdgpu_hotunplug_teardown_test();
	CU_ASSERT_EQUAL(r , 0);

	r = amdgpu_hotunplug_rescan();
	CU_ASSERT_EQUAL(r > 0, 1);
}


CU_TestInfo hotunplug_tests[] = {
	{ "Unplug card and rescan the bus to plug it back", amdgpu_hotunplug_simple },
	{ "Same as first test but with command submission", amdgpu_hotunplug_with_cs },
	{ "Unplug with exported bo", amdgpu_hotunplug_with_exported_bo },
	{ "Unplug with exported fence", amdgpu_hotunplug_with_exported_fence },
	CU_TEST_INFO_NULL,
};
