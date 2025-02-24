//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// vk_android_utils.cpp: Vulkan utilities for using the Android platform

#include "libANGLE/renderer/vulkan/android/vk_android_utils.h"

#include "common/android_util.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

#if defined(ANGLE_PLATFORM_ANDROID)
#    include "libANGLE/Display.h"
#    include "libANGLE/renderer/vulkan/android/AHBFunctions.h"
#    include "libANGLE/renderer/vulkan/android/DisplayVkAndroid.h"
#endif

namespace rx
{
namespace vk
{
namespace
{
#if defined(ANGLE_PLATFORM_ANDROID)
DisplayVkAndroid *GetDisplayVkAndroid(Renderer *renderer)
{
    DisplayVk *displayVk = static_cast<DisplayVk *>(renderer->getGlobalOps());
    return static_cast<DisplayVkAndroid *>(displayVk);
}
#endif
}  // anonymous namespace

angle::Result GetClientBufferMemoryRequirements(ErrorContext *context,
                                                const AHardwareBuffer *hardwareBuffer,
                                                VkMemoryRequirements &memRequirements)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    const AHBFunctions &ahbFunctions =
        GetDisplayVkAndroid(context->getRenderer())->getAHBFunctions();
    ASSERT(ahbFunctions.valid());

    AHardwareBuffer_Desc aHardwareBufferDescription = {};
    ahbFunctions.describe(hardwareBuffer, &aHardwareBufferDescription);
    if (aHardwareBufferDescription.format != AHARDWAREBUFFER_FORMAT_BLOB)
    {
        ERR() << "Trying to import non-BLOB AHB as client buffer.";
        return angle::Result::Stop;
    }

    // Get Android Buffer Properties
    VkAndroidHardwareBufferPropertiesANDROID bufferProperties = {};
    bufferProperties.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    bufferProperties.pNext = nullptr;

    VkDevice device = context->getRenderer()->getDevice();
    ANGLE_VK_TRY(context, vkGetAndroidHardwareBufferPropertiesANDROID(device, hardwareBuffer,
                                                                      &bufferProperties));

    memRequirements.size           = bufferProperties.allocationSize;
    memRequirements.alignment      = 0;
    memRequirements.memoryTypeBits = bufferProperties.memoryTypeBits;

    return angle::Result::Continue;
#else
    ANGLE_VK_UNREACHABLE(context);
    return angle::Result::Stop;

#endif
}

angle::Result InitAndroidExternalMemory(ErrorContext *context,
                                        EGLClientBuffer clientBuffer,
                                        VkMemoryPropertyFlags memoryProperties,
                                        Buffer *buffer,
                                        VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                                        uint32_t *memoryTypeIndexOut,
                                        DeviceMemory *deviceMemoryOut,
                                        VkDeviceSize *sizeOut)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    const AHBFunctions &functions = GetDisplayVkAndroid(context->getRenderer())->getAHBFunctions();
    ASSERT(functions.valid());

    struct AHardwareBuffer *hardwareBuffer =
        angle::android::ClientBufferToAHardwareBuffer(clientBuffer);

    VkMemoryRequirements externalMemoryRequirements = {};
    ANGLE_TRY(
        GetClientBufferMemoryRequirements(context, hardwareBuffer, externalMemoryRequirements));

    // Import Vulkan DeviceMemory from Android Hardware Buffer.
    VkImportAndroidHardwareBufferInfoANDROID importHardwareBufferInfo = {};
    importHardwareBufferInfo.sType  = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    importHardwareBufferInfo.buffer = hardwareBuffer;

    ANGLE_VK_TRY(context, AllocateBufferMemoryWithRequirements(
                              context, MemoryAllocationType::BufferExternal, memoryProperties,
                              externalMemoryRequirements, &importHardwareBufferInfo, buffer,
                              memoryPropertyFlagsOut, memoryTypeIndexOut, deviceMemoryOut));
    *sizeOut = externalMemoryRequirements.size;

    functions.acquire(hardwareBuffer);

    return angle::Result::Continue;
#else
    ANGLE_VK_UNREACHABLE(context);
    return angle::Result::Stop;
#endif
}

void ReleaseAndroidExternalMemory(Renderer *renderer, EGLClientBuffer clientBuffer)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    const AHBFunctions &functions = GetDisplayVkAndroid(renderer)->getAHBFunctions();
    ASSERT(functions.valid());
    struct AHardwareBuffer *hardwareBuffer =
        angle::android::ClientBufferToAHardwareBuffer(clientBuffer);
    functions.release(hardwareBuffer);
#endif
}
}  // namespace vk
}  // namespace rx
