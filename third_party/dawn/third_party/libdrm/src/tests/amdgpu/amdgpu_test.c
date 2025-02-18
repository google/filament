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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdint.h>
#ifdef __linux__
#include <linux/limits.h>
#elif __FreeBSD__
/* SPECNAMELEN in FreeBSD is defined here: */
#include <sys/param.h>
#endif
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif

#include "drm.h"
#include "xf86drmMode.h"
#include "xf86drm.h"

#include "CUnit/Basic.h"

#include "amdgpu_test.h"
#include "amdgpu_internal.h"

/* Test suite names */
#define BASIC_TESTS_STR "Basic Tests"
#define BO_TESTS_STR "BO Tests"
#define CS_TESTS_STR "CS Tests"
#define VCE_TESTS_STR "VCE Tests"
#define VCN_TESTS_STR "VCN Tests"
#define JPEG_TESTS_STR "JPEG Tests"
#define UVD_ENC_TESTS_STR "UVD ENC Tests"
#define DEADLOCK_TESTS_STR "Deadlock Tests"
#define VM_TESTS_STR "VM Tests"
#define RAS_TESTS_STR "RAS Tests"
#define SYNCOBJ_TIMELINE_TESTS_STR "SYNCOBJ TIMELINE Tests"
#define SECURITY_TESTS_STR "Security Tests"
#define HOTUNPLUG_TESTS_STR "Hotunplug Tests"
#define CP_DMA_TESTS_STR "CP DMA Tests"

/**
 *  Open handles for amdgpu devices
 *
 */
int drm_amdgpu[MAX_CARDS_SUPPORTED];

/** Open render node to test */
int open_render_node = 0;	/* By default run most tests on primary node */

/** The table of all known test suites to run */
static CU_SuiteInfo suites[] = {
	{
		.pName = BASIC_TESTS_STR,
		.pInitFunc = suite_basic_tests_init,
		.pCleanupFunc = suite_basic_tests_clean,
		.pTests = basic_tests,
	},
	{
		.pName = BO_TESTS_STR,
		.pInitFunc = suite_bo_tests_init,
		.pCleanupFunc = suite_bo_tests_clean,
		.pTests = bo_tests,
	},
	{
		.pName = CS_TESTS_STR,
		.pInitFunc = suite_cs_tests_init,
		.pCleanupFunc = suite_cs_tests_clean,
		.pTests = cs_tests,
	},
	{
		.pName = VCE_TESTS_STR,
		.pInitFunc = suite_vce_tests_init,
		.pCleanupFunc = suite_vce_tests_clean,
		.pTests = vce_tests,
	},
	{
		.pName = VCN_TESTS_STR,
		.pInitFunc = suite_vcn_tests_init,
		.pCleanupFunc = suite_vcn_tests_clean,
		.pTests = vcn_tests,
	},
	{
		.pName = JPEG_TESTS_STR,
		.pInitFunc = suite_jpeg_tests_init,
		.pCleanupFunc = suite_jpeg_tests_clean,
		.pTests = jpeg_tests,
	},
	{
		.pName = UVD_ENC_TESTS_STR,
		.pInitFunc = suite_uvd_enc_tests_init,
		.pCleanupFunc = suite_uvd_enc_tests_clean,
		.pTests = uvd_enc_tests,
	},
	{
		.pName = DEADLOCK_TESTS_STR,
		.pInitFunc = suite_deadlock_tests_init,
		.pCleanupFunc = suite_deadlock_tests_clean,
		.pTests = deadlock_tests,
	},
	{
		.pName = VM_TESTS_STR,
		.pInitFunc = suite_vm_tests_init,
		.pCleanupFunc = suite_vm_tests_clean,
		.pTests = vm_tests,
	},
	{
		.pName = RAS_TESTS_STR,
		.pInitFunc = suite_ras_tests_init,
		.pCleanupFunc = suite_ras_tests_clean,
		.pTests = ras_tests,
	},
	{
		.pName = SYNCOBJ_TIMELINE_TESTS_STR,
		.pInitFunc = suite_syncobj_timeline_tests_init,
		.pCleanupFunc = suite_syncobj_timeline_tests_clean,
		.pTests = syncobj_timeline_tests,
	},
	{
		.pName = SECURITY_TESTS_STR,
		.pInitFunc = suite_security_tests_init,
		.pCleanupFunc = suite_security_tests_clean,
		.pTests = security_tests,
	},
	{
		.pName = HOTUNPLUG_TESTS_STR,
		.pInitFunc = suite_hotunplug_tests_init,
		.pCleanupFunc = suite_hotunplug_tests_clean,
		.pTests = hotunplug_tests,
	},
	{
		.pName = CP_DMA_TESTS_STR,
		.pInitFunc = suite_cp_dma_tests_init,
		.pCleanupFunc = suite_cp_dma_tests_clean,
		.pTests = cp_dma_tests,
	},

	CU_SUITE_INFO_NULL,
};

typedef CU_BOOL (*active__stat_func)(void);

typedef struct Suites_Active_Status {
	char*             pName;
	active__stat_func pActive;
}Suites_Active_Status;

static CU_BOOL always_active()
{
	return CU_TRUE;
}

static Suites_Active_Status suites_active_stat[] = {
		{
			.pName = BASIC_TESTS_STR,
			.pActive = suite_basic_tests_enable,
		},
		{
			.pName = BO_TESTS_STR,
			.pActive = always_active,
		},
		{
			.pName = CS_TESTS_STR,
			.pActive = suite_cs_tests_enable,
		},
		{
			.pName = VCE_TESTS_STR,
			.pActive = suite_vce_tests_enable,
		},
		{
			.pName = VCN_TESTS_STR,
			.pActive = suite_vcn_tests_enable,
		},
		{
			.pName = JPEG_TESTS_STR,
			.pActive = suite_jpeg_tests_enable,
		},
		{
			.pName = UVD_ENC_TESTS_STR,
			.pActive = suite_uvd_enc_tests_enable,
		},
		{
			.pName = DEADLOCK_TESTS_STR,
			.pActive = suite_deadlock_tests_enable,
		},
		{
			.pName = VM_TESTS_STR,
			.pActive = suite_vm_tests_enable,
		},
		{
			.pName = RAS_TESTS_STR,
			.pActive = suite_ras_tests_enable,
		},
		{
			.pName = SYNCOBJ_TIMELINE_TESTS_STR,
			.pActive = suite_syncobj_timeline_tests_enable,
		},
		{
			.pName = SECURITY_TESTS_STR,
			.pActive = suite_security_tests_enable,
		},
		{
			.pName = HOTUNPLUG_TESTS_STR,
			.pActive = suite_hotunplug_tests_enable,
		},
		{
			.pName = CP_DMA_TESTS_STR,
			.pActive = suite_cp_dma_tests_enable,
		},
};


/*
 * Display information about all  suites and their tests
 *
 * NOTE: Must be run after registry is initialized and suites registered.
 */
static void display_test_suites(void)
{
	int iSuite;
	int iTest;
	CU_pSuite pSuite = NULL;
	CU_pTest  pTest  = NULL;

	printf("%5s: %2s: %8s: %s\n", "What", "ID", "Status", "Name");

	for (iSuite = 0; suites[iSuite].pName != NULL; iSuite++) {

		pSuite = CU_get_suite_by_index((unsigned int) iSuite + 1,
					       CU_get_registry());

		if (!pSuite) {
			fprintf(stderr, "Invalid suite id : %d\n", iSuite + 1);
			continue;
		}

		printf("Suite: %2d: %8s: %s\n",
		       iSuite + 1,
		       pSuite->fActive ? "ENABLED" : "DISABLED",
		       suites[iSuite].pName);

		if (!pSuite->fActive)
			continue;

		for (iTest = 0; suites[iSuite].pTests[iTest].pName != NULL;
		     iTest++) {
			pTest = CU_get_test_by_index((unsigned int) iTest + 1,
						     pSuite);
			if (!pTest) {
				fprintf(stderr, "Invalid test id : %d\n", iTest + 1);
				continue;
			}
			printf(" Test: %2d: %8s: %s\n",
			       iTest + 1,
			       pSuite->fActive && pTest->fActive ? "ENABLED" : "DISABLED",
			       suites[iSuite].pTests[iTest].pName);
		}
	}
}

/** Help string for command line parameters */
static const char usage[] =
	"Usage: %s [-hlpr] [-s <suite id>] [-e <s>[.<t>] [-e ...]] [-t <test id>] [-f] "
	"[-b <pci_bus_id>] [-d <pci_device_id>]\n"
	"Where,\n"
	"  -b      Specify device's PCI bus id to run tests\n"
	"  -d      Specify device's PCI device id to run tests (optional)\n"
	"  -e <s>[.<t>]  Disable test <t> of suite <s>. If only <s> is given, then disable\n"
	"          the whole suite. Can be specified more than once on the command line\n"
	"          to disable multiple tests or suites.\n"
	"  -f      Force executing inactive suite or test\n"
	"  -h      Display this help\n"
	"  -l      Display all test suites and their tests\n"
	"  -p      Display information of AMDGPU devices in system\n"
	"  -r      Run the tests on render node\n"
	"  -s <s>  Enable only test suite <s>\n"
	"  -t <t>  Enable only test <t> of test suite <s>\n";
/** Specified options strings for getopt */
static const char options[]   = "hlrps:t:e:b:d:f";

/* Open AMD devices.
 * Return the number of AMD device opened.
 */
static int amdgpu_open_devices(int open_render_node)
{
	drmDevicePtr devices[MAX_CARDS_SUPPORTED];
	int i;
	int drm_node;
	int amd_index = 0;
	int drm_count;
	int fd;
	drmVersionPtr version;

	for (i = 0; i < MAX_CARDS_SUPPORTED; i++) {
		drm_amdgpu[i] = -1;
	}

	drm_count = drmGetDevices2(0, devices, MAX_CARDS_SUPPORTED);

	if (drm_count < 0) {
		fprintf(stderr,
			"drmGetDevices2() returned an error %d\n",
			drm_count);
		return 0;
	}

	for (i = 0; i < drm_count; i++) {
		/* If this is not PCI device, skip*/
		if (devices[i]->bustype != DRM_BUS_PCI)
			continue;

		/* If this is not AMD GPU vender ID, skip*/
		if (devices[i]->deviceinfo.pci->vendor_id != 0x1002)
			continue;

		if (open_render_node)
			drm_node = DRM_NODE_RENDER;
		else
			drm_node = DRM_NODE_PRIMARY;

		fd = -1;
		if (devices[i]->available_nodes & 1 << drm_node)
			fd = open(
				devices[i]->nodes[drm_node],
				O_RDWR | O_CLOEXEC);

		/* This node is not available. */
		if (fd < 0) continue;

		version = drmGetVersion(fd);
		if (!version) {
			fprintf(stderr,
				"Warning: Cannot get version for %s."
				"Error is %s\n",
				devices[i]->nodes[drm_node],
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

		drm_amdgpu[amd_index] = fd;
		amd_index++;
	}

	drmFreeDevices(devices, drm_count);
	return amd_index;
}

/* Close AMD devices.
 */
void amdgpu_close_devices()
{
	int i;
	for (i = 0; i < MAX_CARDS_SUPPORTED; i++)
		if (drm_amdgpu[i] >=0) {
			close(drm_amdgpu[i]);
		}
}

/* Print AMD devices information */
static void amdgpu_print_devices()
{
	int i;
	drmDevicePtr device;

	/* Open the first AMD device to print driver information. */
	if (drm_amdgpu[0] >=0) {
		/* Display AMD driver version information.*/
		drmVersionPtr retval = drmGetVersion(drm_amdgpu[0]);

		if (retval == NULL) {
			perror("Cannot get version for AMDGPU device");
			return;
		}

		printf("Driver name: %s, Date: %s, Description: %s.\n",
			retval->name, retval->date, retval->desc);
		drmFreeVersion(retval);
	}

	/* Display information of AMD devices */
	printf("Devices:\n");
	for (i = 0; i < MAX_CARDS_SUPPORTED && drm_amdgpu[i] >=0; i++)
		if (drmGetDevice2(drm_amdgpu[i],
			DRM_DEVICE_GET_PCI_REVISION,
			&device) == 0) {
			if (device->bustype == DRM_BUS_PCI) {
				printf("PCI ");
				printf(" domain:%04x",
					device->businfo.pci->domain);
				printf(" bus:%02x",
					device->businfo.pci->bus);
				printf(" device:%02x",
					device->businfo.pci->dev);
				printf(" function:%01x",
					device->businfo.pci->func);
				printf(" vendor_id:%04x",
					device->deviceinfo.pci->vendor_id);
				printf(" device_id:%04x",
					device->deviceinfo.pci->device_id);
				printf(" subvendor_id:%04x",
					device->deviceinfo.pci->subvendor_id);
				printf(" subdevice_id:%04x",
					device->deviceinfo.pci->subdevice_id);
				printf(" revision_id:%02x",
					device->deviceinfo.pci->revision_id);
				printf("\n");
			}
			drmFreeDevice(&device);
		}
}

/* Find a match AMD device in PCI bus
 * Return the index of the device or -1 if not found
 */
static int amdgpu_find_device(uint8_t bus, uint16_t dev)
{
	int i;
	drmDevicePtr device;

	for (i = 0; i < MAX_CARDS_SUPPORTED && drm_amdgpu[i] >= 0; i++) {
		if (drmGetDevice2(drm_amdgpu[i],
			DRM_DEVICE_GET_PCI_REVISION,
			&device) == 0) {
			if (device->bustype == DRM_BUS_PCI)
				if ((bus == 0xFF || device->businfo.pci->bus == bus) &&
					device->deviceinfo.pci->device_id == dev) {
					drmFreeDevice(&device);
					return i;
				}

			drmFreeDevice(&device);
		}
	}

	return -1;
}

static void amdgpu_disable_suites()
{
	amdgpu_device_handle device_handle;
	uint32_t major_version, minor_version, family_id;
	drmDevicePtr devices[MAX_CARDS_SUPPORTED];
	int i, drm_count;
	int size = sizeof(suites_active_stat) / sizeof(suites_active_stat[0]);

	if (amdgpu_device_initialize(drm_amdgpu[0], &major_version,
				   &minor_version, &device_handle))
		return;

	family_id = device_handle->info.family_id;

	if (amdgpu_device_deinitialize(device_handle))
		return;

	drm_count = drmGetDevices2(0, devices, MAX_CARDS_SUPPORTED);

	/* Set active status for suites based on their policies */
	for (i = 0; i < size; ++i)
		if (amdgpu_set_suite_active(suites_active_stat[i].pName,
				suites_active_stat[i].pActive()))
			fprintf(stderr, "suite deactivation failed - %s\n", CU_get_error_msg());

	/* Explicitly disable specific tests due to known bugs or preferences */
	/*
	* BUG: Compute ring stalls and never recovers when the address is
	* written after the command already submitted
	*/
	if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
			"compute ring block test (set amdgpu.lockup_timeout=50)", CU_FALSE))
		fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
				"sdma ring block test (set amdgpu.lockup_timeout=50)", CU_FALSE))
		fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	/* This test was ran on GFX9 only */
	//if (family_id < AMDGPU_FAMILY_AI || family_id > AMDGPU_FAMILY_RV)
		if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
				"gfx ring bad dispatch test (set amdgpu.lockup_timeout=50)", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	/* This test was ran on GFX9 only */
	//if (family_id < AMDGPU_FAMILY_AI || family_id > AMDGPU_FAMILY_RV)
		if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
				"compute ring bad dispatch test (set amdgpu.lockup_timeout=50,50)", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	/* This test was ran on GFX9 only */
	//if (family_id < AMDGPU_FAMILY_AI || family_id > AMDGPU_FAMILY_RV)
		if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
				"gfx ring bad slow dispatch test (set amdgpu.lockup_timeout=50)", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	/* This test was ran on GFX9 only */
	//if (family_id < AMDGPU_FAMILY_AI || family_id > AMDGPU_FAMILY_RV)
		if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
				"compute ring bad slow dispatch test (set amdgpu.lockup_timeout=50,50)", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	//if (family_id < AMDGPU_FAMILY_AI || family_id > AMDGPU_FAMILY_RV)
		if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
				"gfx ring bad draw test (set amdgpu.lockup_timeout=50)", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	/* This test was ran on GFX9 only */
	//if (family_id < AMDGPU_FAMILY_AI || family_id > AMDGPU_FAMILY_RV)
		if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
				"gfx ring slow bad draw test (set amdgpu.lockup_timeout=50)", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
			"sdma ring corrupted header test (set amdgpu.lockup_timeout=50)", CU_FALSE))
		fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	if (amdgpu_set_test_active(DEADLOCK_TESTS_STR,
			"sdma ring slow linear copy test (set amdgpu.lockup_timeout=50)", CU_FALSE))
		fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	if (amdgpu_set_test_active(BASIC_TESTS_STR, "bo eviction Test", CU_FALSE))
		fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	/* This test was ran on GFX8 and GFX9 only */
	if (family_id < AMDGPU_FAMILY_VI || family_id > AMDGPU_FAMILY_RV)
		if (amdgpu_set_test_active(BASIC_TESTS_STR, "Sync dependency Test", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	/* This test was ran on GFX9 only */
	if (family_id < AMDGPU_FAMILY_AI || family_id > AMDGPU_FAMILY_RV) {
		if (amdgpu_set_test_active(BASIC_TESTS_STR, "Dispatch Test (GFX)", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());
		if (amdgpu_set_test_active(BASIC_TESTS_STR, "Dispatch Test (Compute)", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());
	}

	/* This test was ran on GFX9 only */
	if (family_id < AMDGPU_FAMILY_AI || family_id > AMDGPU_FAMILY_RV)
		if (amdgpu_set_test_active(BASIC_TESTS_STR, "Draw Test", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	/* This test was ran on GFX9 only */
	//if (family_id < AMDGPU_FAMILY_AI || family_id > AMDGPU_FAMILY_RV)
		if (amdgpu_set_test_active(BASIC_TESTS_STR, "GPU reset Test", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());

	/* You need at least 2 devices for this	*/
	if (drm_count < 2)
		if (amdgpu_set_test_active(HOTUNPLUG_TESTS_STR, "Unplug with exported fence", CU_FALSE))
			fprintf(stderr, "test deactivation failed - %s\n", CU_get_error_msg());
}

int test_device_index;

int amdgpu_open_device_on_test_index(int render_node)
{
	int i;

	if (amdgpu_open_devices(open_render_node) <= 0) {
		perror("Cannot open AMDGPU device");
		return -1;
	}

	if (test_device_index >= 0) {
		/* Most tests run on device of drm_amdgpu[0].
		 * Swap the chosen device to drm_amdgpu[0].
		 */
		i = drm_amdgpu[0];
		drm_amdgpu[0] = drm_amdgpu[test_device_index];
		drm_amdgpu[test_device_index] = i;
	}

	return 0;


}


static bool amdgpu_node_is_drm(int maj, int min)
{
#ifdef __linux__
    char path[64];
    struct stat sbuf;

    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device/drm",
             maj, min);
    return stat(path, &sbuf) == 0;
#elif defined(__FreeBSD__)
    char name[SPECNAMELEN];

    if (!devname_r(makedev(maj, min), S_IFCHR, name, sizeof(name)))
      return 0;
    /* Handle drm/ and dri/ as both are present in different FreeBSD version
     * FreeBSD on amd64/i386/powerpc external kernel modules create node in
     * in /dev/drm/ and links in /dev/dri while a WIP in kernel driver creates
     * only device nodes in /dev/dri/ */
    return (!strncmp(name, "drm/", 4) || !strncmp(name, "dri/", 4));
#else
    return maj == DRM_MAJOR;
#endif
}

char *amdgpu_get_device_from_fd(int fd)
{
#ifdef __linux__
    struct stat sbuf;
    char path[PATH_MAX + 1];
    unsigned int maj, min;

    if (fstat(fd, &sbuf))
        return NULL;

    maj = major(sbuf.st_rdev);
    min = minor(sbuf.st_rdev);

    if (!amdgpu_node_is_drm(maj, min) || !S_ISCHR(sbuf.st_mode))
        return NULL;

    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device", maj, min);
    return strdup(path);
#else
    return NULL;
#endif
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_A) (sizeof(_A)/sizeof(_A[0]))
#endif

static void amdgpu_test_disable(long suite, long test)
{
	const char *suite_name;

	if (suite < 1)
		return;

	/* The array is 0-based, so subract 1. */
	suite--;
	if (suite >= ARRAY_SIZE(suites) - 1)
		return;

	suite_name = suites[suite].pName;
	if (test < 1) {
		fprintf(stderr, "Deactivating suite %s\n", suite_name);
		amdgpu_set_suite_active(suite_name, CU_FALSE);
	} else {
		int ii;

		/* The array is 0-based so subtract 1. */
		test--;
		for (ii = 0; suites[suite].pTests[ii].pName; ii++) {
			if (ii == test) {
				fprintf(stderr, "Deactivating %s:%s\n",
					suite_name,
					suites[suite].pTests[ii].pName);
				amdgpu_set_test_active(suite_name,
						       suites[suite].pTests[ii].pName,
						       CU_FALSE);
				break;
			}
		}

		if (suites[suite].pTests[ii].pName == NULL)
			fprintf(stderr, "No such suite.test %ld.%ld\n", suite, test);
	}
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main(int argc, char **argv)
{
	int c;			/* Character received from getopt */
	int i = 0;
	int suite_id = -1;	/* By default run everything */
	int test_id  = -1;	/* By default run all tests in the suite */
	int pci_bus_id = -1;    /* By default PC bus ID is not specified */
	int pci_device_id = 0;  /* By default PC device ID is zero */
	int display_devices = 0;/* By default not to display devices' info */
	CU_pSuite pSuite = NULL;
	CU_pTest  pTest  = NULL;
	int display_list = 0;
	int force_run = 0;

	/* Parse command line string.
	 * Process various command line options as early as possible.
	 */
	opterr = 0;		/* Do not print error messages from getopt */
	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'h':
			fprintf(stderr, usage, argv[0]);
			exit(EXIT_SUCCESS);
		}
	}

	for (i = 0; i < MAX_CARDS_SUPPORTED; i++)
		drm_amdgpu[i] = -1;

	if (amdgpu_open_devices(open_render_node) <= 0) {
		perror("Cannot open AMDGPU device");
		exit(EXIT_FAILURE);
	}

	if (drm_amdgpu[0] < 0) {
		perror("Cannot open AMDGPU device");
		exit(EXIT_FAILURE);
	}

	/* Parse command line string */
	opterr = 0;		/* Do not print error messages from getopt */
	optind = 1;
	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'p':
			display_devices = 1;
			break;
		}
	}

	if (display_devices) {
		amdgpu_print_devices();
		amdgpu_close_devices();
		exit(EXIT_SUCCESS);
	}

	/* Parse command line string */
	opterr = 0;		/* Do not print error messages from getopt */
	optind = 1;
	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'b':
			pci_bus_id = atoi(optarg);
			break;
		case 'd':
			sscanf(optarg, "%x", &pci_device_id);
			break;
		}
	}

	if (pci_bus_id > 0 || pci_device_id) {
		/* A device was specified to run the test */
		test_device_index = amdgpu_find_device(pci_bus_id,
						       pci_device_id);

		if (test_device_index >= 0) {
			/* Most tests run on device of drm_amdgpu[0].
			 * Swap the chosen device to drm_amdgpu[0].
			 */
			i = drm_amdgpu[0];
			drm_amdgpu[0] = drm_amdgpu[test_device_index];
			drm_amdgpu[test_device_index] = i;
		} else {
			fprintf(stderr,
				"The specified GPU device does not exist.\n");
			exit(EXIT_FAILURE);
		}
	}

	/* Initialize test suites to run */

	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry()) {
		amdgpu_close_devices();
		return CU_get_error();
	}

	/* Register suites. */
	if (CU_register_suites(suites) != CUE_SUCCESS) {
		fprintf(stderr, "suite registration failed - %s\n",
				CU_get_error_msg());
		CU_cleanup_registry();
		amdgpu_close_devices();
		exit(EXIT_FAILURE);
	}

	/* Run tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);

	/* Disable suites and individual tests based on misc. conditions */
	amdgpu_disable_suites();

	/* Parse command line string */
	opterr = 0;		/* Do not print error messages from getopt */
	optind = 1;
	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'l':
			display_list = 1;
			break;
		}
	}

	if (display_list) {
		display_test_suites();
		goto end;
	}

	/* Parse command line string */
	opterr = 0;		/* Do not print error messages from getopt */
	optind = 1;
	while ((c = getopt(argc, argv, options)) != -1) {
		long esuite = -1;
		long etest = -1;
		char *endp;
		switch (c) {
		case 's':
			suite_id = atoi(optarg);
			break;
		case 't':
			test_id = atoi(optarg);
			break;
		case 'r':
                       open_render_node = 1;
                       break;
		case 'f':
			force_run = 1;
			break;
		case 'e':
			esuite = strtol(optarg, &endp, 0);
			if (endp == optarg) {
				fprintf(stderr, "No digits given for -e argument\n");
				goto end;
			} else if (endp && *endp == '.' && esuite > 0) {
				char *tt = endp + 1;
				etest = strtol(tt, &endp, 0);
				if (endp == tt) {
					fprintf(stderr, "No digits given for test in -e s.t argument\n");
					goto end;
				} else if (endp && *endp != '\0') {
					fprintf(stderr, "Bad input given for test in -e s.t argument\n");
					goto end;
				} else if (etest < 1) {
					fprintf(stderr, "Test in -e s.t argument cannot be smaller than 1\n");
					goto end;
				}
			} else if (endp && *endp != '\0') {
				fprintf(stderr, "Bad input given for suite for -e s argument\n");
				goto end;
			} else if (esuite < 1) {
				fprintf(stderr, "Suite in -e s argument cannot be smaller than 1\n");
				goto end;
			}
			amdgpu_test_disable(esuite, etest);
			break;
		case 'h':
		case 'p':
		case 'b':
		case 'd':
		case 'l':
			/* Those have been processed earlier.
			 */
			break;
		case '?':
		default:
			fprintf(stderr, "Unknown command line option '%c'. Try -h.\n",
				c == '?' ? optopt : c);
			goto end;
		}
	}

	if (suite_id != -1) {	/* If user specify particular suite? */
		pSuite = CU_get_suite_by_index((unsigned int) suite_id,
						CU_get_registry());

		if (pSuite) {

			if (force_run)
				CU_set_suite_active(pSuite, CU_TRUE);

			if (test_id != -1) {   /* If user specify test id */
				pTest = CU_get_test_by_index(
						(unsigned int) test_id,
						pSuite);
				if (pTest) {
					if (force_run)
						CU_set_test_active(pTest, CU_TRUE);

					CU_basic_run_test(pSuite, pTest);
				}
				else {
					fprintf(stderr, "Invalid test id: %d\n",
								test_id);
					CU_cleanup_registry();
					amdgpu_close_devices();
					exit(EXIT_FAILURE);
				}
			} else
				CU_basic_run_suite(pSuite);
		} else {
			fprintf(stderr, "Invalid suite id : %d\n",
					suite_id);
			CU_cleanup_registry();
			amdgpu_close_devices();
			exit(EXIT_FAILURE);
		}
	} else
		CU_basic_run_tests();

end:
	CU_cleanup_registry();
	amdgpu_close_devices();
	return CU_get_error();
}
