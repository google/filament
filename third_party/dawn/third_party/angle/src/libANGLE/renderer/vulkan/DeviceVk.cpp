//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DeviceVk.cpp:
//    Implements the class methods for DeviceVk.
//

#include "libANGLE/renderer/vulkan/DeviceVk.h"

#include <stdint.h>

#include "common/debug.h"
#include "common/vulkan/vulkan_icd.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{
namespace
{

DeviceVk *gDevice = nullptr;

class [[nodiscard]] ScopedEnv : public angle::vk::ScopedVkLoaderEnvironment
{
  public:
    ScopedEnv()
        : angle::vk::ScopedVkLoaderEnvironment(
              gDevice ? gDevice->getRenderer()->getEnableValidationLayers() : false,
              gDevice ? gDevice->getRenderer()->getEnabledICD() : angle::vk::ICD::Default)
    {
        if (!gDevice)
        {
            WARN() << "No DeviceVk instance.";
        }
    }
};

}  // namespace

DeviceVk::DeviceVk() = default;

DeviceVk::~DeviceVk()
{
    if (gDevice == this)
    {
        gDevice = nullptr;
    }
}

egl::Error DeviceVk::initialize()
{
    return egl::NoError();
}

egl::Error DeviceVk::getAttribute(const egl::Display *display, EGLint attribute, void **outValue)
{
    vk::Renderer *renderer =
        static_cast<rx::DisplayVk *>(display->getImplementation())->getRenderer();
    ASSERT(mRenderer == nullptr || mRenderer == renderer);
    mRenderer = renderer;
    switch (attribute)
    {
        case EGL_VULKAN_VERSION_ANGLE:
        {
            auto version = static_cast<intptr_t>(mRenderer->getDeviceVersion());
            *outValue    = reinterpret_cast<void *>(version);
            return egl::NoError();
        }
        case EGL_VULKAN_INSTANCE_ANGLE:
        {
            *outValue = mRenderer->getInstance();
            return egl::NoError();
        }
        case EGL_VULKAN_DEVICE_ANGLE:
        {
            *outValue = mRenderer->getDevice();
            return egl::NoError();
        }
        case EGL_VULKAN_PHYSICAL_DEVICE_ANGLE:
        {
            *outValue = mRenderer->getPhysicalDevice();
            return egl::NoError();
        }
        case EGL_VULKAN_QUEUE_ANGLE:
        {
            // egl::ContextPriority::Medium is the default context priority.
            *outValue = mRenderer->getQueue(egl::ContextPriority::Medium);
            return egl::NoError();
        }
        case EGL_VULKAN_QUEUE_FAMILIY_INDEX_ANGLE:
        {
            intptr_t index = static_cast<intptr_t>(mRenderer->getQueueFamilyIndex());
            *outValue      = reinterpret_cast<void *>(index);
            return egl::NoError();
        }
        case EGL_VULKAN_DEVICE_EXTENSIONS_ANGLE:
        {
            char **extensions = const_cast<char **>(mRenderer->getEnabledDeviceExtensions().data());
            *outValue         = reinterpret_cast<void *>(extensions);
            return egl::NoError();
        }
        case EGL_VULKAN_INSTANCE_EXTENSIONS_ANGLE:
        {
            char **extensions =
                const_cast<char **>(mRenderer->getEnabledInstanceExtensions().data());
            *outValue = reinterpret_cast<void *>(extensions);
            return egl::NoError();
        }
        case EGL_VULKAN_FEATURES_ANGLE:
        {
            const auto *features = &mRenderer->getEnabledFeatures();
            *outValue            = const_cast<void *>(reinterpret_cast<const void *>(features));
            return egl::NoError();
        }
        case EGL_VULKAN_GET_INSTANCE_PROC_ADDR:
        {
            *outValue = reinterpret_cast<void *>(DeviceVk::WrappedGetInstanceProcAddr);
            ASSERT(!gDevice || gDevice == this);
            gDevice = this;
            return egl::NoError();
        }
        default:
            return egl::EglBadAccess();
    }
}

void DeviceVk::generateExtensions(egl::DeviceExtensions *outExtensions) const
{
    outExtensions->deviceVulkan = true;
}

// static
VKAPI_ATTR VkResult VKAPI_CALL
DeviceVk::WrappedCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                const VkAllocationCallbacks *pAllocator,
                                VkInstance *pInstance)
{
    ScopedEnv scopedEnv;
    return vkCreateInstance(pCreateInfo, pAllocator, pInstance);
}

// static
VKAPI_ATTR VkResult VKAPI_CALL
DeviceVk::WrappedEnumerateInstanceExtensionProperties(const char *pLayerName,
                                                      uint32_t *pPropertyCount,
                                                      VkExtensionProperties *pProperties)
{
    ScopedEnv scopedEnv;
    return vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}

// static
VKAPI_ATTR VkResult VKAPI_CALL
DeviceVk::WrappedEnumerateInstanceLayerProperties(uint32_t *pPropertyCount,
                                                  VkLayerProperties *pProperties)
{
    ScopedEnv scopedEnv;
    return vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

// static
VKAPI_ATTR VkResult VKAPI_CALL DeviceVk::WrappedEnumerateInstanceVersion(uint32_t *pApiVersion)
{
    ScopedEnv scopedEnv;
    return vkEnumerateInstanceVersion(pApiVersion);
}

// static
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL DeviceVk::WrappedGetInstanceProcAddr(VkInstance instance,
                                                                              const char *pName)
{
    if (!pName || pName[0] != 'v' || pName[1] != 'k')
    {
        return nullptr;
    }

    if (instance != VK_NULL_HANDLE)
    {
        return vkGetInstanceProcAddr(instance, pName);
    }

    if (!strcmp(pName, "vkCreateInstance"))
    {
        return reinterpret_cast<PFN_vkVoidFunction>(DeviceVk::WrappedCreateInstance);
    }
    if (!strcmp(pName, "vkEnumerateInstanceExtensionProperties"))
    {
        return reinterpret_cast<PFN_vkVoidFunction>(
            DeviceVk::WrappedEnumerateInstanceExtensionProperties);
    }
    if (!strcmp(pName, "vkEnumerateInstanceLayerProperties"))
    {
        return reinterpret_cast<PFN_vkVoidFunction>(
            DeviceVk::WrappedEnumerateInstanceLayerProperties);
    }
    if (!strcmp(pName, "vkEnumerateInstanceVersion"))
    {
        if (!vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"))
        {
            // Vulkan 1.0 doesn't have vkEnumerateInstanceVersion.
            return nullptr;
        }
        return reinterpret_cast<PFN_vkVoidFunction>(DeviceVk::WrappedEnumerateInstanceVersion);
    }
    if (!strcmp(pName, "vkGetInstanceProcAddr"))
    {
        return reinterpret_cast<PFN_vkVoidFunction>(DeviceVk::WrappedGetInstanceProcAddr);
    }

    return vkGetInstanceProcAddr(instance, pName);
}

}  // namespace rx
