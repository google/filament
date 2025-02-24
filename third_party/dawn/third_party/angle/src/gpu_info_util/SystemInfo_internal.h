//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_internal.h: Functions used by the SystemInfo_* files and unittests

#ifndef GPU_INFO_UTIL_SYSTEM_INFO_INTERNAL_H_
#define GPU_INFO_UTIL_SYSTEM_INFO_INTERNAL_H_

#include "common/platform.h"
#include "gpu_info_util/SystemInfo.h"

namespace angle
{

// Defined in SystemInfo_libpci when GPU_INFO_USE_LIBPCI is defined.
bool GetPCIDevicesWithLibPCI(std::vector<GPUDeviceInfo> *devices);
// Defined in SystemInfo_x11 when GPU_INFO_USE_X11 is defined.
bool GetNvidiaDriverVersionWithXNVCtrl(std::string *version);

// Target specific helper functions that can be compiled on all targets
// Live in SystemInfo.cpp
bool ParseAMDBrahmaDriverVersion(const std::string &content, std::string *version);
bool ParseAMDCatalystDriverVersion(const std::string &content, std::string *version);
bool CMDeviceIDToDeviceAndVendorID(const std::string &id, uint32_t *vendorId, uint32_t *deviceId);

#if defined(ANGLE_PLATFORM_MACOS) || defined(ANGLE_PLATFORM_MACCATALYST)
bool GetSystemInfo_mac(SystemInfo *info);
#else
bool GetSystemInfo_ios(SystemInfo *info);
#endif

#if defined(ANGLE_PLATFORM_MACOS) || defined(ANGLE_PLATFORM_MACCATALYST)
// Helper to get the active GPU ID from a given Core Graphics display ID.
uint64_t GetGpuIDFromDisplayID(uint32_t displayID);

#    if ANGLE_ENABLE_CGL
// Helper to get the active GPU ID from an OpenGL display mask.
uint64_t GetGpuIDFromOpenGLDisplayMask(uint32_t displayMask);
#    endif

#endif

#if defined(ANGLE_PLATFORM_MACOS) && ANGLE_ENABLE_METAL
// Get VendorID from metal device's registry ID
VendorID GetVendorIDFromMetalDeviceRegistryID(uint64_t registryID);
#endif


}  // namespace angle

#endif  // GPU_INFO_UTIL_SYSTEM_INFO_INTERNAL_H_
