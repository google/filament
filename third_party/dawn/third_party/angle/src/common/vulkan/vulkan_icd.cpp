//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// vulkan_icd.cpp : Helper for creating vulkan instances & selecting physical device.

#include "common/vulkan/vulkan_icd.h"

#include <functional>
#include <vector>

#include "common/Optional.h"
#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/system_utils.h"

namespace
{
void ResetEnvironmentVar(const char *variableName, const Optional<std::string> &value)
{
    if (!value.valid())
    {
        return;
    }

    if (value.value().empty())
    {
        angle::UnsetEnvironmentVar(variableName);
    }
    else
    {
        angle::SetEnvironmentVar(variableName, value.value().c_str());
    }
}
}  // namespace

namespace angle
{

namespace vk
{

namespace
{

[[maybe_unused]] const std::string WrapICDEnvironment(const char *icdEnvironment)
{
    // The libraries are bundled into the module directory
    std::string moduleDir = angle::GetModuleDirectory();
    std::string ret       = ConcatenatePath(moduleDir, icdEnvironment);
#if defined(ANGLE_PLATFORM_MACOS)
    std::string moduleDirWithLibraries = ConcatenatePath(moduleDir, "Libraries");
    ret += ":" + ConcatenatePath(moduleDirWithLibraries, icdEnvironment);
#endif
    return ret;
}

[[maybe_unused]] constexpr char kLoaderLayersPathEnv[] = "VK_LAYER_PATH";
[[maybe_unused]] constexpr char kLayerEnablesEnv[]     = "VK_LAYER_ENABLES";

constexpr char kLoaderICDFilenamesEnv[]              = "VK_ICD_FILENAMES";
constexpr char kANGLEPreferredDeviceEnv[]            = "ANGLE_PREFERRED_DEVICE";
constexpr char kValidationLayersCustomSTypeListEnv[] = "VK_LAYER_CUSTOM_STYPE_LIST";
constexpr char kNoDeviceSelect[]                     = "NODEVICE_SELECT";

constexpr uint32_t kMockVendorID = 0xba5eba11;
constexpr uint32_t kMockDeviceID = 0xf005ba11;
constexpr char kMockDeviceName[] = "Vulkan Mock Device";

constexpr uint32_t kGoogleVendorID      = 0x1AE0;
constexpr uint32_t kSwiftShaderDeviceID = 0xC0DE;
constexpr char kSwiftShaderDeviceName[] = "SwiftShader Device";

using ICDFilterFunc = std::function<bool(const VkPhysicalDeviceProperties &)>;

ICDFilterFunc GetFilterForICD(vk::ICD preferredICD)
{
    switch (preferredICD)
    {
        case vk::ICD::Mock:
            return [](const VkPhysicalDeviceProperties &deviceProperties) {
                return ((deviceProperties.vendorID == kMockVendorID) &&
                        (deviceProperties.deviceID == kMockDeviceID) &&
                        (strcmp(deviceProperties.deviceName, kMockDeviceName) == 0));
            };
        case vk::ICD::SwiftShader:
            return [](const VkPhysicalDeviceProperties &deviceProperties) {
                return ((deviceProperties.vendorID == kGoogleVendorID) &&
                        (deviceProperties.deviceID == kSwiftShaderDeviceID) &&
                        (strncmp(deviceProperties.deviceName, kSwiftShaderDeviceName,
                                 strlen(kSwiftShaderDeviceName)) == 0));
            };
        default:
            const std::string anglePreferredDevice =
                angle::GetEnvironmentVar(kANGLEPreferredDeviceEnv);
            return [anglePreferredDevice](const VkPhysicalDeviceProperties &deviceProperties) {
                return (anglePreferredDevice == deviceProperties.deviceName);
            };
    }
}

}  // namespace

// If we're loading the vulkan layers, we could be running from any random directory.
// Change to the executable directory so we can find the layers, then change back to the
// previous directory to be safe we don't disrupt the application.
ScopedVkLoaderEnvironment::ScopedVkLoaderEnvironment(bool enableDebugLayers, vk::ICD icd)
    : mEnableDebugLayers(enableDebugLayers),
      mICD(icd),
      mChangedCWD(false),
      mChangedICDEnv(false),
      mChangedNoDeviceSelect(false)
{
// Changing CWD and setting environment variables makes no sense on Android,
// since this code is a part of Java application there.
// Android Vulkan loader doesn't need this either.
#if !defined(ANGLE_PLATFORM_ANDROID) && !defined(ANGLE_PLATFORM_GGP)
    if (icd == vk::ICD::Mock)
    {
        if (!setICDEnvironment(WrapICDEnvironment(ANGLE_VK_MOCK_ICD_JSON).c_str()))
        {
            ERR() << "Error setting environment for Mock/Null Driver.";
        }
    }
#    if defined(ANGLE_VK_SWIFTSHADER_ICD_JSON)
    else if (icd == vk::ICD::SwiftShader)
    {
        if (!setICDEnvironment(WrapICDEnvironment(ANGLE_VK_SWIFTSHADER_ICD_JSON).c_str()))
        {
            ERR() << "Error setting environment for SwiftShader.";
        }
    }
#    endif  // defined(ANGLE_VK_SWIFTSHADER_ICD_JSON)

#    if !defined(ANGLE_PLATFORM_MACOS)
    if (mEnableDebugLayers || icd != vk::ICD::Default)
    {
        const auto &cwd = angle::GetCWD();
        if (!cwd.valid())
        {
            ERR() << "Error getting CWD for Vulkan layers init.";
            mEnableDebugLayers = false;
            mICD               = vk::ICD::Default;
        }
        else
        {
            mPreviousCWD          = cwd.value();
            std::string moduleDir = angle::GetModuleDirectory();
            mChangedCWD           = angle::SetCWD(moduleDir.c_str());
            if (!mChangedCWD)
            {
                ERR() << "Error setting CWD for Vulkan layers init.";
                mEnableDebugLayers = false;
                mICD               = vk::ICD::Default;
            }
        }
    }
#    endif  // defined(ANGLE_PLATFORM_MACOS)

    // Override environment variable to use the ANGLE layers.
    if (mEnableDebugLayers)
    {
#    if defined(ANGLE_VK_LAYERS_DIR)
        if (!angle::PrependPathToEnvironmentVar(kLoaderLayersPathEnv, ANGLE_VK_LAYERS_DIR))
        {
            ERR() << "Error setting environment for Vulkan layers init.";
            mEnableDebugLayers = false;
        }
#    endif  // defined(ANGLE_VK_LAYERS_DIR)
    }
#endif  // !defined(ANGLE_PLATFORM_ANDROID)

    if (IsMSan() || IsASan())
    {
        // device select layer cause memory sanitizer false positive, so disable
        // it for msan build.
        mPreviousNoDeviceSelectEnv = angle::GetEnvironmentVar(kNoDeviceSelect);
        angle::SetEnvironmentVar(kNoDeviceSelect, "1");
        mChangedNoDeviceSelect = true;
    }
}

ScopedVkLoaderEnvironment::~ScopedVkLoaderEnvironment()
{
    if (mChangedCWD)
    {
#if !defined(ANGLE_PLATFORM_ANDROID)
        ASSERT(mPreviousCWD.valid());
        angle::SetCWD(mPreviousCWD.value().c_str());
#endif  // !defined(ANGLE_PLATFORM_ANDROID)
    }
    if (mChangedICDEnv)
    {
        ResetEnvironmentVar(kLoaderICDFilenamesEnv, mPreviousICDEnv);
    }

    ResetEnvironmentVar(kValidationLayersCustomSTypeListEnv, mPreviousCustomExtensionsEnv);

    if (mChangedNoDeviceSelect)
    {
        ResetEnvironmentVar(kNoDeviceSelect, mPreviousNoDeviceSelectEnv);
    }
}

bool ScopedVkLoaderEnvironment::setICDEnvironment(const char *icd)
{
    // Override environment variable to use built Mock ICD
    // ANGLE_VK_ICD_JSON gets set to the built mock ICD in BUILD.gn
    mPreviousICDEnv = angle::GetEnvironmentVar(kLoaderICDFilenamesEnv);
    mChangedICDEnv  = angle::SetEnvironmentVar(kLoaderICDFilenamesEnv, icd);

    if (!mChangedICDEnv)
    {
        mICD = vk::ICD::Default;
    }
    return mChangedICDEnv;
}

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
                          VkPhysicalDeviceDriverProperties *physicalDeviceDriverPropertiesOut)
{
    ASSERT(!physicalDevices.empty());

    VkPhysicalDeviceProperties const *deviceProps = &physicalDeviceProperties2Out->properties;

    ICDFilterFunc filter = GetFilterForICD(preferredICD);

    const bool shouldChooseByPciId = (preferredVendorID != 0 || preferredDeviceID != 0);
    const bool shouldChooseByUUIDs = (preferredDeviceUUID != nullptr ||
                                      preferredDriverUUID != nullptr || preferredDriverID != 0);

    for (const VkPhysicalDevice &physicalDevice : physicalDevices)
    {
        *physicalDeviceProperties2Out       = {};
        physicalDeviceProperties2Out->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        physicalDeviceProperties2Out->pNext = physicalDeviceIDPropertiesOut;

        *physicalDeviceIDPropertiesOut       = {};
        physicalDeviceIDPropertiesOut->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
        physicalDeviceIDPropertiesOut->pNext = physicalDeviceDriverPropertiesOut;

        *physicalDeviceDriverPropertiesOut = {};
        physicalDeviceDriverPropertiesOut->sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;

        pGetPhysicalDeviceProperties2(physicalDevice, physicalDeviceProperties2Out);

        if (deviceProps->apiVersion < kMinimumVulkanAPIVersion)
        {
            // Skip any devices that don't support our minimum API version. This
            // takes precedence over all other considerations.
            continue;
        }

        if (filter(*deviceProps))
        {
            *physicalDeviceOut = physicalDevice;
            return;
        }

        if (shouldChooseByUUIDs)
        {
            bool matched = true;

            if (preferredDriverID != 0 &&
                preferredDriverID != physicalDeviceDriverPropertiesOut->driverID)
            {
                matched = false;
            }
            else if (preferredDeviceUUID != nullptr &&
                     memcmp(preferredDeviceUUID, physicalDeviceIDPropertiesOut->deviceUUID,
                            VK_UUID_SIZE) != 0)
            {
                matched = false;
            }
            else if (preferredDriverUUID != nullptr &&
                     memcmp(preferredDriverUUID, physicalDeviceIDPropertiesOut->driverUUID,
                            VK_UUID_SIZE) != 0)
            {
                matched = false;
            }

            if (matched)
            {
                *physicalDeviceOut = physicalDevice;
                return;
            }
        }

        if (shouldChooseByPciId)
        {
            // NOTE: If the system has multiple GPUs with the same vendor and
            // device IDs, this will arbitrarily select one of them.
            bool matchVendorID = true;
            bool matchDeviceID = true;

            if (preferredVendorID != 0 && preferredVendorID != deviceProps->vendorID)
            {
                matchVendorID = false;
            }

            if (preferredDeviceID != 0 && preferredDeviceID != deviceProps->deviceID)
            {
                matchDeviceID = false;
            }

            if (matchVendorID && matchDeviceID)
            {
                *physicalDeviceOut = physicalDevice;
                return;
            }
        }
    }

    Optional<VkPhysicalDevice> integratedDevice;
    VkPhysicalDeviceProperties2 integratedDeviceProperties2;
    VkPhysicalDeviceIDProperties integratedDeviceIDProperties;
    VkPhysicalDeviceDriverProperties integratedDeviceDriverProperties;

    for (const VkPhysicalDevice &physicalDevice : physicalDevices)
    {
        *physicalDeviceProperties2Out       = {};
        physicalDeviceProperties2Out->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        physicalDeviceProperties2Out->pNext = physicalDeviceIDPropertiesOut;

        *physicalDeviceIDPropertiesOut       = {};
        physicalDeviceIDPropertiesOut->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
        physicalDeviceIDPropertiesOut->pNext = physicalDeviceDriverPropertiesOut;

        *physicalDeviceDriverPropertiesOut = {};
        physicalDeviceDriverPropertiesOut->sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;

        pGetPhysicalDeviceProperties2(physicalDevice, physicalDeviceProperties2Out);

        if (deviceProps->apiVersion < kMinimumVulkanAPIVersion)
        {
            // Skip any devices that don't support our minimum API version. This
            // takes precedence over all other considerations.
            continue;
        }

        // If discrete GPU exists, uses it by default.
        if (deviceProps->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            *physicalDeviceOut = physicalDevice;
            return;
        }
        if (deviceProps->deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
            !integratedDevice.valid())
        {
            integratedDevice                       = physicalDevice;
            integratedDeviceProperties2            = *physicalDeviceProperties2Out;
            integratedDeviceIDProperties           = *physicalDeviceIDPropertiesOut;
            integratedDeviceDriverProperties       = *physicalDeviceDriverPropertiesOut;
            integratedDeviceProperties2.pNext      = nullptr;
            integratedDeviceIDProperties.pNext     = nullptr;
            integratedDeviceDriverProperties.pNext = nullptr;
            continue;
        }
    }

    // If only integrated GPU exists, use it by default.
    if (integratedDevice.valid())
    {
        *physicalDeviceOut             = integratedDevice.value();
        *physicalDeviceProperties2Out  = integratedDeviceProperties2;
        *physicalDeviceIDPropertiesOut = integratedDeviceIDProperties;
        return;
    }

    WARN() << "Preferred device ICD not found. Using default physicalDevice instead.";
    // Fallback to the first device.
    *physicalDeviceOut = physicalDevices[0];
    pGetPhysicalDeviceProperties2(*physicalDeviceOut, physicalDeviceProperties2Out);
}

}  // namespace vk

}  // namespace angle
