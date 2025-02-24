//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SystemInfo_vulkan.h: Reusable Vulkan implementation for SystemInfo.h

#ifndef GPU_INFO_UTIL_SYSTEM_INFO_VULKAN_H_
#define GPU_INFO_UTIL_SYSTEM_INFO_VULKAN_H_

#include "common/vulkan/vulkan_icd.h"
#include "gpu_info_util/SystemInfo.h"

namespace angle
{
struct SystemInfo;

// Reusable vulkan implementation of GetSystemInfo(). See SystemInfo.h.
bool GetSystemInfoVulkan(SystemInfo *info);
bool GetSystemInfoVulkanWithICD(SystemInfo *info, vk::ICD preferredICD);

VersionInfo ParseAMDVulkanDriverVersion(uint32_t version);
VersionInfo ParseArmVulkanDriverVersion(uint32_t version);
VersionInfo ParseBroadcomVulkanDriverVersion(uint32_t version);
VersionInfo ParseSwiftShaderVulkanDriverVersion(uint32_t version);
VersionInfo ParseImaginationVulkanDriverVersion(uint32_t version);
VersionInfo ParseIntelWindowsVulkanDriverVersion(uint32_t version);
VersionInfo ParseKazanVulkanDriverVersion(uint32_t version);
VersionInfo ParseNvidiaVulkanDriverVersion(uint32_t version);
VersionInfo ParseQualcommVulkanDriverVersion(uint32_t version);
VersionInfo ParseSamsungVulkanDriverVersion(uint32_t version);
VersionInfo ParseVeriSiliconVulkanDriverVersion(uint32_t version);
VersionInfo ParseVivanteVulkanDriverVersion(uint32_t version);
VersionInfo ParseMesaVulkanDriverVersion(uint32_t version);
VersionInfo ParseMoltenVulkanDriverVersion(uint32_t version);
}  // namespace angle

#endif  // GPU_INFO_UTIL_SYSTEM_INFO_VULKAN_H_
