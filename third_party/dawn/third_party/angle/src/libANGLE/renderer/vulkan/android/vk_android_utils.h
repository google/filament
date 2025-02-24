//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// vk_android_utils.h: Vulkan utilities for using the Android platform

#ifndef LIBANGLE_RENDERER_VULKAN_ANDROID_VK_ANDROID_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_ANDROID_VK_ANDROID_UTILS_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "common/vulkan/vk_headers.h"
#include "libANGLE/Error.h"

class Buffer;
class DeviceMemory;

namespace rx
{
class ErrorContext;

namespace vk
{
class Renderer;
angle::Result InitAndroidExternalMemory(ErrorContext *context,
                                        EGLClientBuffer clientBuffer,
                                        VkMemoryPropertyFlags memoryProperties,
                                        Buffer *buffer,
                                        VkMemoryPropertyFlags *memoryPropertyFlagsOut,
                                        uint32_t *memoryTypeIndexOut,
                                        DeviceMemory *deviceMemoryOut,
                                        VkDeviceSize *sizeOut);

void ReleaseAndroidExternalMemory(Renderer *renderer, EGLClientBuffer clientBuffer);
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ANDROID_VK_ANDROID_UTILS_H_
