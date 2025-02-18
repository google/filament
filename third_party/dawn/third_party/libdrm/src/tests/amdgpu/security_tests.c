/*
 * Copyright 2019 Advanced Micro Devices, Inc.
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

#include <string.h>
#include <unistd.h>
#ifdef __FreeBSD__
#include <sys/endian.h>
#else
#include <endian.h>
#endif
#include <strings.h>
#include <xf86drm.h>

static amdgpu_device_handle device_handle;
static uint32_t major_version;
static uint32_t minor_version;

static struct drm_amdgpu_info_hw_ip  sdma_info;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_Arr)  (sizeof(_Arr)/sizeof((_Arr)[0]))
#endif


/* --------------------- Secure bounce test ------------------------ *
 *
 * The secure bounce test tests that we can evict a TMZ buffer,
 * and page it back in, via a bounce buffer, as it encryption/decryption
 * depends on its physical address, and have the same data, i.e. data
 * integrity is preserved.
 *
 * The steps are as follows (from Christian K.):
 *
 * Buffer A which is TMZ protected and filled by the CPU with a
 * certain pattern. That the GPU is reading only random nonsense from
 * that pattern is irrelevant for the test.
 *
 * This buffer A is then secure copied into buffer B which is also
 * TMZ protected.
 *
 * Buffer B is moved around, from VRAM to GTT, GTT to SYSTEM,
 * etc.
 *
 * Then, we use another secure copy of buffer B back to buffer A.
 *
 * And lastly we check with the CPU the pattern.
 *
 * Assuming that we don't have memory contention and buffer A stayed
 * at the same place, we should still see the same pattern when read
 * by the CPU.
 *
 * If we don't see the same pattern then something in the buffer
 * migration code is not working as expected.
 */

#define SECURE_BOUNCE_TEST_STR    "secure bounce"
#define SECURE_BOUNCE_FAILED_STR  SECURE_BOUNCE_TEST_STR " failed"

#define PRINT_ERROR(_Res)   fprintf(stderr, "%s:%d: %s (%d)\n",	\
				    __func__, __LINE__, strerror(-(_Res)), _Res)

#define PACKET_LCOPY_SIZE         7
#define PACKET_NOP_SIZE          12

struct sec_amdgpu_bo {
	struct amdgpu_bo *bo;
	struct amdgpu_va *va;
};

struct command_ctx {
	struct amdgpu_device    *dev;
	struct amdgpu_cs_ib_info cs_ibinfo;
	struct amdgpu_cs_request cs_req;
	struct amdgpu_context   *context;
	int ring_id;
};

/**
 * amdgpu_bo_alloc_map -- Allocate and map a buffer object (BO)
 * @dev: The AMDGPU device this BO belongs to.
 * @size: The size of the BO.
 * @alignment: Alignment of the BO.
 * @gem_domain: One of AMDGPU_GEM_DOMAIN_xyz.
 * @alloc_flags: One of AMDGPU_GEM_CREATE_xyz.
 * @sbo: the result
 *
 * Allocate a buffer object (BO) with the desired attributes
 * as specified by the argument list and write out the result
 * into @sbo.
 *
 * Return 0 on success and @sbo->bo and @sbo->va are set,
 * or -errno on error.
 */
static int amdgpu_bo_alloc_map(struct amdgpu_device *dev,
			       unsigned size,
			       unsigned alignment,
			       unsigned gem_domain,
			       uint64_t alloc_flags,
			       struct sec_amdgpu_bo *sbo)
{
	void *cpu;
	uint64_t mc_addr;

	return amdgpu_bo_alloc_and_map_raw(dev,
					   size,
					   alignment,
					   gem_domain,
					   alloc_flags,
					   0,
					   &sbo->bo,
					   &cpu, &mc_addr,
					   &sbo->va);
}

static void amdgpu_bo_unmap_free(struct sec_amdgpu_bo *sbo,
				 const uint64_t size)
{
	(void) amdgpu_bo_unmap_and_free(sbo->bo,
					sbo->va,
					sbo->va->address,
					size);
	sbo->bo = NULL;
	sbo->va = NULL;
}

static void amdgpu_sdma_lcopy(uint32_t *packet,
			      const uint64_t dst,
			      const uint64_t src,
			      const uint32_t size,
			      const int secure)
{
	/* Set the packet to Linear copy with TMZ set.
	 */
	packet[0] = htole32(secure << 18 | 1);
	packet[1] = htole32(size-1);
	packet[2] = htole32(0);
	packet[3] = htole32((uint32_t)(src & 0xFFFFFFFFU));
	packet[4] = htole32((uint32_t)(src >> 32));
	packet[5] = htole32((uint32_t)(dst & 0xFFFFFFFFU));
	packet[6] = htole32((uint32_t)(dst >> 32));
}

static void amdgpu_sdma_nop(uint32_t *packet, uint32_t nop_count)
{
	/* A packet of the desired number of NOPs.
	 */
	packet[0] = htole32(nop_count << 16);
	for ( ; nop_count > 0; nop_count--)
		packet[nop_count-1] = 0;
}

/**
 * amdgpu_bo_lcopy -- linear copy with TMZ set, using sDMA
 * @dev: AMDGPU device to which both buffer objects belong to
 * @dst: destination buffer object
 * @src: source buffer object
 * @size: size of memory to move, in bytes.
 * @secure: Set to 1 to perform secure copy, 0 for clear
 *
 * Issues and waits for completion of a Linear Copy with TMZ
 * set, to the sDMA engine. @size should be a multiple of
 * at least 16 bytes.
 */
static void amdgpu_bo_lcopy(struct command_ctx *ctx,
			    struct sec_amdgpu_bo *dst,
			    struct sec_amdgpu_bo *src,
			    const uint32_t size,
			    int secure)
{
	struct amdgpu_bo *bos[] = { dst->bo, src->bo };
	uint32_t packet[PACKET_LCOPY_SIZE];

	amdgpu_sdma_lcopy(packet,
			  dst->va->address,
			  src->va->address,
			  size, secure);
	amdgpu_test_exec_cs_helper_raw(ctx->dev, ctx->context,
				       AMDGPU_HW_IP_DMA, ctx->ring_id,
				       ARRAY_SIZE(packet), packet,
				       ARRAY_SIZE(bos), bos,
				       &ctx->cs_ibinfo, &ctx->cs_req,
				       secure == 1);
}

/**
 * amdgpu_bo_move -- Evoke a move of the buffer object (BO)
 * @dev: device to which this buffer object belongs to
 * @bo: the buffer object to be moved
 * @whereto: one of AMDGPU_GEM_DOMAIN_xyz
 * @secure: set to 1 to submit secure IBs
 *
 * Evokes a move of the buffer object @bo to the GEM domain
 * descibed by @whereto.
 *
 * Returns 0 on sucess; -errno on error.
 */
static int amdgpu_bo_move(struct command_ctx *ctx,
			  struct amdgpu_bo *bo,
			  uint64_t whereto,
			  int secure)
{
	struct amdgpu_bo *bos[] = { bo };
	struct drm_amdgpu_gem_op gop = {
		.handle  = bo->handle,
		.op      = AMDGPU_GEM_OP_SET_PLACEMENT,
		.value   = whereto,
	};
	uint32_t packet[PACKET_NOP_SIZE];
	int res;

	/* Change the buffer's placement.
	 */
	res = drmIoctl(ctx->dev->fd, DRM_IOCTL_AMDGPU_GEM_OP, &gop);
	if (res)
		return -errno;

	/* Now issue a NOP to actually evoke the MM to move
	 * it to the desired location.
	 */
	amdgpu_sdma_nop(packet, PACKET_NOP_SIZE);
	amdgpu_test_exec_cs_helper_raw(ctx->dev, ctx->context,
				       AMDGPU_HW_IP_DMA, ctx->ring_id,
				       ARRAY_SIZE(packet), packet,
				       ARRAY_SIZE(bos), bos,
				       &ctx->cs_ibinfo, &ctx->cs_req,
				       secure == 1);
	return 0;
}

/* Safe, O Sec!
 */
static const uint8_t secure_pattern[] = { 0x5A, 0xFE, 0x05, 0xEC };

#define SECURE_BUFFER_SIZE       (4 * 1024 * sizeof(secure_pattern))

static void amdgpu_secure_bounce(void)
{
	struct sec_amdgpu_bo alice, bob;
	struct command_ctx   sb_ctx;
	long page_size;
	uint8_t *pp;
	int res;

	page_size = sysconf(_SC_PAGESIZE);

	memset(&sb_ctx, 0, sizeof(sb_ctx));
	sb_ctx.dev = device_handle;
	res = amdgpu_cs_ctx_create(sb_ctx.dev, &sb_ctx.context);
	if (res) {
		PRINT_ERROR(res);
		CU_FAIL(SECURE_BOUNCE_FAILED_STR);
		return;
	}

	/* Use the first present ring.
	 */
	res = ffs(sdma_info.available_rings) - 1;
	if (res == -1) {
		PRINT_ERROR(-ENOENT);
		CU_FAIL(SECURE_BOUNCE_FAILED_STR);
		goto Out_free_ctx;
	}
	sb_ctx.ring_id = res;

	/* Allocate a buffer named Alice in VRAM.
	 */
	res = amdgpu_bo_alloc_map(device_handle,
				  SECURE_BUFFER_SIZE,
				  page_size,
				  AMDGPU_GEM_DOMAIN_VRAM,
				  AMDGPU_GEM_CREATE_ENCRYPTED,
				  &alice);
	if (res) {
		PRINT_ERROR(res);
		CU_FAIL(SECURE_BOUNCE_FAILED_STR);
		return;
	}

	/* Fill Alice with a pattern.
	 */
	for (pp = alice.bo->cpu_ptr;
	     pp < (__typeof__(pp)) alice.bo->cpu_ptr + SECURE_BUFFER_SIZE;
	     pp += sizeof(secure_pattern))
		memcpy(pp, secure_pattern, sizeof(secure_pattern));

	/* Allocate a buffer named Bob in VRAM.
	 */
	res = amdgpu_bo_alloc_map(device_handle,
				  SECURE_BUFFER_SIZE,
				  page_size,
				  AMDGPU_GEM_DOMAIN_VRAM,
				  AMDGPU_GEM_CREATE_ENCRYPTED,
				  &bob);
	if (res) {
		PRINT_ERROR(res);
		CU_FAIL(SECURE_BOUNCE_FAILED_STR);
		goto Out_free_Alice;
	}

	/* sDMA TMZ copy from Alice to Bob.
	 */
	amdgpu_bo_lcopy(&sb_ctx, &bob, &alice, SECURE_BUFFER_SIZE, 1);

	/* Move Bob to the GTT domain.
	 */
	res = amdgpu_bo_move(&sb_ctx, bob.bo, AMDGPU_GEM_DOMAIN_GTT, 0);
	if (res) {
		PRINT_ERROR(res);
		CU_FAIL(SECURE_BOUNCE_FAILED_STR);
		goto Out_free_all;
	}

	/* sDMA TMZ copy from Bob to Alice.
	 */
	amdgpu_bo_lcopy(&sb_ctx, &alice, &bob, SECURE_BUFFER_SIZE, 1);

	/* Verify the contents of Alice.
	 */
	for (pp = alice.bo->cpu_ptr;
	     pp < (__typeof__(pp)) alice.bo->cpu_ptr + SECURE_BUFFER_SIZE;
	     pp += sizeof(secure_pattern)) {
		res = memcmp(pp, secure_pattern, sizeof(secure_pattern));
		if (res) {
			fprintf(stderr, SECURE_BOUNCE_FAILED_STR);
			CU_FAIL(SECURE_BOUNCE_FAILED_STR);
			break;
		}
	}

Out_free_all:
	amdgpu_bo_unmap_free(&bob, SECURE_BUFFER_SIZE);
Out_free_Alice:
	amdgpu_bo_unmap_free(&alice, SECURE_BUFFER_SIZE);
Out_free_ctx:
	res = amdgpu_cs_ctx_free(sb_ctx.context);
	CU_ASSERT_EQUAL(res, 0);
}

/* ----------------------------------------------------------------- */

static void amdgpu_security_alloc_buf_test(void)
{
	amdgpu_bo_handle bo;
	amdgpu_va_handle va_handle;
	uint64_t bo_mc;
	int r;

	/* Test secure buffer allocation in VRAM */
	bo = gpu_mem_alloc(device_handle, 4096, 4096,
			   AMDGPU_GEM_DOMAIN_VRAM,
			   AMDGPU_GEM_CREATE_ENCRYPTED,
			   &bo_mc, &va_handle);

	r = gpu_mem_free(bo, va_handle, bo_mc, 4096);
	CU_ASSERT_EQUAL(r, 0);

	/* Test secure buffer allocation in system memory */
	bo = gpu_mem_alloc(device_handle, 4096, 4096,
			   AMDGPU_GEM_DOMAIN_GTT,
			   AMDGPU_GEM_CREATE_ENCRYPTED,
			   &bo_mc, &va_handle);

	r = gpu_mem_free(bo, va_handle, bo_mc, 4096);
	CU_ASSERT_EQUAL(r, 0);

	/* Test secure buffer allocation in invisible VRAM */
	bo = gpu_mem_alloc(device_handle, 4096, 4096,
			   AMDGPU_GEM_DOMAIN_GTT,
			   AMDGPU_GEM_CREATE_ENCRYPTED |
			   AMDGPU_GEM_CREATE_NO_CPU_ACCESS,
			   &bo_mc, &va_handle);

	r = gpu_mem_free(bo, va_handle, bo_mc, 4096);
	CU_ASSERT_EQUAL(r, 0);
}

static void amdgpu_security_gfx_submission_test(void)
{
	amdgpu_command_submission_write_linear_helper_with_secure(device_handle,
								  AMDGPU_HW_IP_GFX,
								  true);
}

static void amdgpu_security_sdma_submission_test(void)
{
	amdgpu_command_submission_write_linear_helper_with_secure(device_handle,
								  AMDGPU_HW_IP_DMA,
								  true);
}

/* ----------------------------------------------------------------- */

CU_TestInfo security_tests[] = {
	{ "allocate secure buffer test",        amdgpu_security_alloc_buf_test },
	{ "graphics secure command submission", amdgpu_security_gfx_submission_test },
	{ "sDMA secure command submission",     amdgpu_security_sdma_submission_test },
	{ SECURE_BOUNCE_TEST_STR,               amdgpu_secure_bounce },
	CU_TEST_INFO_NULL,
};

CU_BOOL suite_security_tests_enable(void)
{
	CU_BOOL enable = CU_TRUE;

	if (amdgpu_device_initialize(drm_amdgpu[0], &major_version,
				     &minor_version, &device_handle))
		return CU_FALSE;


	if (!(device_handle->dev_info.ids_flags & AMDGPU_IDS_FLAGS_TMZ)) {
		printf("\n\nDon't support TMZ (trust memory zone), security suite disabled\n");
		enable = CU_FALSE;
	}

	if ((major_version < 3) ||
		((major_version == 3) && (minor_version < 37))) {
		printf("\n\nDon't support TMZ (trust memory zone), kernel DRM version (%d.%d)\n",
			major_version, minor_version);
		printf("is older, security suite disabled\n");
		enable = CU_FALSE;
	}

	if (amdgpu_device_deinitialize(device_handle))
		return CU_FALSE;

	return enable;
}

int suite_security_tests_init(void)
{
	int res;

	res = amdgpu_device_initialize(drm_amdgpu[0], &major_version,
				       &minor_version, &device_handle);
	if (res) {
		PRINT_ERROR(res);
		return CUE_SINIT_FAILED;
	}

	res = amdgpu_query_hw_ip_info(device_handle,
				      AMDGPU_HW_IP_DMA,
				      0, &sdma_info);
	if (res) {
		PRINT_ERROR(res);
		return CUE_SINIT_FAILED;
	}

	return CUE_SUCCESS;
}

int suite_security_tests_clean(void)
{
	int res;

	res = amdgpu_device_deinitialize(device_handle);
	if (res)
		return CUE_SCLEAN_FAILED;

	return CUE_SUCCESS;
}
