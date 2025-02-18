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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

#include "drm.h"
#include "xf86drmMode.h"
#include "xf86drm.h"
#include "amdgpu.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"

#define MAX_CARDS_SUPPORTED	4
#define NUM_BUFFER_OBJECTS	1024

#define SDMA_PACKET(op, sub_op, e)      ((((e) & 0xFFFF) << 16) |  \
					(((sub_op) & 0xFF) << 8) | \
					(((op) & 0xFF) << 0))

#define SDMA_OPCODE_COPY				  1
#       define SDMA_COPY_SUB_OPCODE_LINEAR		0


#define SDMA_PACKET_SI(op, b, t, s, cnt)	((((op) & 0xF) << 28) | \
						(((b) & 0x1) << 26) |	\
						(((t) & 0x1) << 23) |	\
						(((s) & 0x1) << 22) |	\
						(((cnt) & 0xFFFFF) << 0))
#define SDMA_OPCODE_COPY_SI     3


/** Help string for command line parameters */
static const char usage[] =
	"Usage: %s [-?h] [-b v|g|vg size] "
	"[-c from to size count]\n"
	"where:\n"
	"	b - Allocate a BO in VRAM, GTT or VRAM|GTT of size bytes.\n"
	"	    This flag can be used multiple times. The first bo will\n"
	"	    have id `1`, then second id `2`, ...\n"
	"       c - Copy size bytes from BO (bo_id1) to BO (bo_id2), count times\n"
	"       h - Display this help\n"
	"\n"
	"Sizes can be postfixes with k, m or g for kilo, mega and gigabyte scaling\n";

/** Specified options strings for getopt */
static const char options[]   = "?hb:c:";

/* Open AMD devices.
 * Returns the fd of the first device it could open.
 */
static int amdgpu_open_device(void)
{
	drmDevicePtr devices[MAX_CARDS_SUPPORTED];
	unsigned int i;
	int drm_count;

	drm_count = drmGetDevices2(0, devices, MAX_CARDS_SUPPORTED);
	if (drm_count < 0) {
		fprintf(stderr, "drmGetDevices2() returned an error %d\n",
			drm_count);
		return drm_count;
	}

	for (i = 0; i < drm_count; i++) {
		drmVersionPtr version;
		int fd;

		/* If this is not PCI device, skip*/
		if (devices[i]->bustype != DRM_BUS_PCI)
			continue;

		/* If this is not AMD GPU vender ID, skip*/
		if (devices[i]->deviceinfo.pci->vendor_id != 0x1002)
			continue;

		if (!(devices[i]->available_nodes & 1 << DRM_NODE_RENDER))
			continue;

		fd = open(devices[i]->nodes[DRM_NODE_RENDER], O_RDWR | O_CLOEXEC);

		/* This node is not available. */
		if (fd < 0) continue;

		version = drmGetVersion(fd);
		if (!version) {
			fprintf(stderr,
				"Warning: Cannot get version for %s."
				"Error is %s\n",
				devices[i]->nodes[DRM_NODE_RENDER],
				strerror(errno));
			close(fd);
			continue;
		}

		if (strcmp(version->name, "amdgpu")) {
			/* This is not AMDGPU driver, skip.*/
			drmFreeVersion(version);
			close(fd);
			continue;
		}

		drmFreeVersion(version);
		drmFreeDevices(devices, drm_count);
		return fd;
	}

	return -1;
}

amdgpu_device_handle device_handle;
amdgpu_context_handle context_handle;

amdgpu_bo_handle resources[NUM_BUFFER_OBJECTS];
uint64_t virtual[NUM_BUFFER_OBJECTS];
unsigned int num_buffers;
uint32_t *pm4;

int alloc_bo(uint32_t domain, uint64_t size)
{
	struct amdgpu_bo_alloc_request request = {};
	amdgpu_bo_handle bo;
	amdgpu_va_handle va;
	uint64_t addr;
	int r;

	if (num_buffers >= NUM_BUFFER_OBJECTS)
		return -ENOSPC;

	request.alloc_size = size;
	request.phys_alignment = 0;
	request.preferred_heap = domain;
	request.flags = 0;
	r = amdgpu_bo_alloc(device_handle, &request, &bo);
	if (r)
		return r;

	r = amdgpu_va_range_alloc(device_handle, amdgpu_gpu_va_range_general,
				  size, 0, 0, &addr, &va, 0);
	if (r)
		return r;

	r = amdgpu_bo_va_op_raw(device_handle, bo, 0, size, addr,
				AMDGPU_VM_PAGE_READABLE | AMDGPU_VM_PAGE_WRITEABLE |
				AMDGPU_VM_PAGE_EXECUTABLE, AMDGPU_VA_OP_MAP);
	if (r)
		return r;

	resources[num_buffers] = bo;
	virtual[num_buffers] = addr;
	fprintf(stdout, "Allocated BO number %u at 0x%" PRIx64 ", domain 0x%x, size %" PRIu64 "\n",
		num_buffers++, addr, domain, size);
	return 0;
}

int submit_ib(uint32_t from, uint32_t to, uint64_t size, uint32_t count)
{
	struct amdgpu_cs_request ibs_request;
	struct amdgpu_cs_fence fence_status;
	struct amdgpu_cs_ib_info ib_info;
	uint64_t copied = size, delta;
	struct timespec start, stop;

	uint64_t src = virtual[from];
	uint64_t dst = virtual[to];
	uint32_t expired;
	int i, r;

	i = 0;
	while (size) {
		uint64_t bytes = size < 0x40000 ? size : 0x40000;

		if (device_handle->info.family_id == AMDGPU_FAMILY_SI) {
			pm4[i++] = SDMA_PACKET_SI(SDMA_OPCODE_COPY_SI, 0, 0, 0,
						  bytes);
			pm4[i++] = 0xffffffff & dst;
			pm4[i++] = 0xffffffff & src;
			pm4[i++] = (0xffffffff00000000 & dst) >> 32;
			pm4[i++] = (0xffffffff00000000 & src) >> 32;
		} else {
			pm4[i++] = SDMA_PACKET(SDMA_OPCODE_COPY,
					       SDMA_COPY_SUB_OPCODE_LINEAR,
					       0);
			if ( device_handle->info.family_id >= AMDGPU_FAMILY_AI)
				pm4[i++] = bytes - 1;
			else
				pm4[i++] = bytes;
			pm4[i++] = 0;
			pm4[i++] = 0xffffffff & src;
			pm4[i++] = (0xffffffff00000000 & src) >> 32;
			pm4[i++] = 0xffffffff & dst;
			pm4[i++] = (0xffffffff00000000 & dst) >> 32;
		}

		size -= bytes;
		src += bytes;
		dst += bytes;
	}

	memset(&ib_info, 0, sizeof(ib_info));
	ib_info.ib_mc_address = virtual[0];
	ib_info.size = i;

	memset(&ibs_request, 0, sizeof(ibs_request));
	ibs_request.ip_type = AMDGPU_HW_IP_DMA;
	ibs_request.ring = 0;
	ibs_request.number_of_ibs = 1;
	ibs_request.ibs = &ib_info;
	ibs_request.fence_info.handle = NULL;

	r = clock_gettime(CLOCK_MONOTONIC, &start);
	if (r)
		return errno;

	r = amdgpu_bo_list_create(device_handle, num_buffers, resources, NULL,
				  &ibs_request.resources);
	if (r)
		return r;

	for (i = 0; i < count; ++i) {
		r = amdgpu_cs_submit(context_handle, 0, &ibs_request, 1);
		if (r)
			return r;
	}

	r = amdgpu_bo_list_destroy(ibs_request.resources);
	if (r)
		return r;

	memset(&fence_status, 0, sizeof(fence_status));
	fence_status.ip_type = ibs_request.ip_type;
	fence_status.ip_instance = 0;
	fence_status.ring = ibs_request.ring;
	fence_status.context = context_handle;
	fence_status.fence = ibs_request.seq_no;
	r = amdgpu_cs_query_fence_status(&fence_status,
					 AMDGPU_TIMEOUT_INFINITE,
					 0, &expired);
	if (r)
		return r;

	r = clock_gettime(CLOCK_MONOTONIC, &stop);
	if (r)
		return errno;

	delta = stop.tv_nsec + stop.tv_sec * 1000000000UL;
	delta -= start.tv_nsec + start.tv_sec * 1000000000UL;

	fprintf(stdout, "Submitted %u IBs to copy from %u(%" PRIx64 ") to %u(%" PRIx64 ") %" PRIu64 " bytes took %" PRIu64 " usec\n",
		count, from, virtual[from], to, virtual[to], copied, delta / 1000);
	return 0;
}

void next_arg(int argc, char **argv, const char *msg)
{
	optarg = argv[optind++];
	if (optind > argc || optarg[0] == '-') {
		fprintf(stderr, "%s\n", msg);
		exit(EXIT_FAILURE);
	}
}

uint64_t parse_size(void)
{
	uint64_t size;
	char ext[2];

	ext[0] = 0;
	if (sscanf(optarg, "%" PRIi64 "%1[kmgKMG]", &size, ext) < 1) {
		fprintf(stderr, "Can't parse size arg: %s\n", optarg);
		exit(EXIT_FAILURE);
	}
	switch (ext[0]) {
	case 'k':
	case 'K':
		size *= 1024;
		break;
	case 'm':
	case 'M':
		size *= 1024 * 1024;
		break;
	case 'g':
	case 'G':
		size *= 1024 * 1024 * 1024;
		break;
	default:
		break;
	}
	return size;
}

int main(int argc, char **argv)
{
	uint32_t major_version, minor_version;
	uint32_t domain, from, to, count;
       	uint64_t size;
	int fd, r, c;

	fd = amdgpu_open_device();
       	if (fd < 0) {
		perror("Cannot open AMDGPU device");
		exit(EXIT_FAILURE);
	}

	r = amdgpu_device_initialize(fd, &major_version, &minor_version, &device_handle);
	if (r) {
		fprintf(stderr, "amdgpu_device_initialize returned %d\n", r);
		exit(EXIT_FAILURE);
	}

	r = amdgpu_cs_ctx_create(device_handle, &context_handle);
	if (r) {
		fprintf(stderr, "amdgpu_cs_ctx_create returned %d\n", r);
		exit(EXIT_FAILURE);
	}

	if (argc == 1) {
		fprintf(stderr, usage, argv[0]);
		exit(EXIT_FAILURE);
	}

	r = alloc_bo(AMDGPU_GEM_DOMAIN_GTT, 2ULL * 1024 * 1024);
	if (r) {
		fprintf(stderr, "Buffer allocation failed with %d\n", r);
		exit(EXIT_FAILURE);
	}

	r = amdgpu_bo_cpu_map(resources[0], (void **)&pm4);
	if (r) {
		fprintf(stderr, "Buffer mapping failed with %d\n", r);
		exit(EXIT_FAILURE);
	}

	opterr = 0;
	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'b':
			if (!strcmp(optarg, "v"))
				domain = AMDGPU_GEM_DOMAIN_VRAM;
			else if (!strcmp(optarg, "g"))
				domain = AMDGPU_GEM_DOMAIN_GTT;
			else if (!strcmp(optarg, "vg"))
				domain = AMDGPU_GEM_DOMAIN_VRAM | AMDGPU_GEM_DOMAIN_GTT;
			else {
				fprintf(stderr, "Invalid domain: %s\n", optarg);
				exit(EXIT_FAILURE);
			}
			next_arg(argc, argv, "Missing buffer size");
			size = parse_size();
			if (size < getpagesize()) {
				fprintf(stderr, "Buffer size to small %" PRIu64 "\n", size);
				exit(EXIT_FAILURE);
			}
			r = alloc_bo(domain, size);
			if (r) {
				fprintf(stderr, "Buffer allocation failed with %d\n", r);
				exit(EXIT_FAILURE);
			}
			break;
		case 'c':
			if (sscanf(optarg, "%u", &from) != 1) {
				fprintf(stderr, "Can't parse from buffer: %s\n", optarg);
				exit(EXIT_FAILURE);
			}
			next_arg(argc, argv, "Missing to buffer");
			if (sscanf(optarg, "%u", &to) != 1) {
				fprintf(stderr, "Can't parse to buffer: %s\n", optarg);
				exit(EXIT_FAILURE);
			}
			next_arg(argc, argv, "Missing size");
			size = parse_size();
			next_arg(argc, argv, "Missing count");
			count = parse_size();
			r = submit_ib(from, to, size, count);
			if (r) {
				fprintf(stderr, "IB submission failed with %d\n", r);
				exit(EXIT_FAILURE);
			}
			break;
		case '?':
		case 'h':
			fprintf(stderr, usage, argv[0]);
			exit(EXIT_SUCCESS);
		default:
			fprintf(stderr, usage, argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	return EXIT_SUCCESS;
}
