//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vulkan_icd.h : Helper for creating vulkan instances & selecting physical device.

#ifndef COMMON_VULKAN_VULKAN_ICD_H_
#define COMMON_VULKAN_VULKAN_ICD_H_

#include <string>

#include "common/Optional.h"
#include "common/angleutils.h"
#include "common/vulkan/vk_headers.h"

namespace angle
{

namespace vk
{

// The minimum version of Vulkan that ANGLE requires.  If an instance or device below this version
// is encountered, initialization will skip the device if possible, or if no other suitable device
// is available then initialization will fail.
constexpr uint32_t kMinimumVulkanAPIVersion = VK_API_VERSION_1_1;

enum class ICD
{
    Default,
    Mock,
    SwiftShader,
};

struct SimpleDisplayWindow
{
    uint16_t width;
    uint16_t height;
};

class [[nodiscard]] ScopedVkLoaderEnvironment : angle::NonCopyable
{
  public:
    ScopedVkLoaderEnvironment(bool enableDebugLayers, vk::ICD icd);
    ~ScopedVkLoaderEnvironment();

    bool canEnableDebugLayers() const { return mEnableDebugLayers; }
    vk::ICD getEnabledICD() const { return mICD; }

  private:
    bool setICDEnvironment(const char *icd);

    bool mEnableDebugLayers;
    vk::ICD mICD;
    bool mChangedCWD;
    Optional<std::string> mPreviousCWD;
    bool mChangedICDEnv;
    Optional<std::string> mPreviousICDEnv;
    Optional<std::string> mPreviousCustomExtensionsEnv;
    bool mChangedNoDeviceSelect;
    Optional<std::string> mPreviousNoDeviceSelectEnv;
};

void ChoosePhysicalDevice(PFN_vkGetPhysicalDeviceProperties2 pGetPhysicalDeviceProperties2,
                          const std::vector<VkPhysicalDevice> &physicalDevices,
                          vk::ICD preferredICD,
                          uint32_t preferredVendorID,
                          uint32_t preferredDeviceID,
                          const uint8_t *preferredDeviceUUID,
                          const uint8_t *preferredDriverUUID,
                          VkDriverId preferredDriverID,
                          VkPhysicalDevice *physicalDeviceOut,
                          VkPhysicalDeviceProperties2 *physicalDeviceProperties2Out,
                          VkPhysicalDeviceIDProperties *physicalDeviceIDPropertiesOut,
                          VkPhysicalDeviceDriverProperties *physicalDeviceDriverPropertiesOut);

}  // namespace vk

}  // namespace angle

#endif  // COMMON_VULKAN_VULKAN_ICD_H_
