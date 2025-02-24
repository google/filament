//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// dma_buf_utils.cpp: Utilities to interact with Linux dma bufs.

#include "dma_buf_utils.h"

#include "common/debug.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>

namespace angle
{
GLenum DrmFourCCFormatToGLInternalFormat(int fourccFormat, bool *isYUV)
{
    *isYUV = false;

    switch (fourccFormat)
    {
        case DRM_FORMAT_R8:
            return GL_R8;
        case DRM_FORMAT_R16:
            return GL_R16_EXT;
        case DRM_FORMAT_GR88:
            return GL_RG8_EXT;
        case DRM_FORMAT_ABGR8888:
            return GL_RGBA8;
        case DRM_FORMAT_XBGR8888:
            return GL_RGBX8_ANGLE;
        case DRM_FORMAT_ARGB8888:
            return GL_BGRA8_EXT;
        case DRM_FORMAT_XRGB8888:
            return GL_BGRX8_ANGLEX;
        case DRM_FORMAT_ABGR2101010:
        case DRM_FORMAT_ARGB2101010:
            return GL_RGB10_A2_EXT;
        case DRM_FORMAT_ABGR16161616F:
            return GL_RGBA16F_EXT;
        case DRM_FORMAT_RGB565:
            return GL_RGB565;
        case DRM_FORMAT_NV12:
        case DRM_FORMAT_YVU420:
        case DRM_FORMAT_P010:
            *isYUV = true;
            return GL_RGB8;
        default:
            UNREACHABLE();
            WARN() << "Unknown dma_buf format " << fourccFormat
                   << " used to initialize an EGL image.";
            return GL_RGB8;
    }
}

#if defined(ANGLE_ENABLE_VULKAN)
std::vector<int> VkFormatToDrmFourCCFormat(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SRGB:
            return {DRM_FORMAT_R8};
        case VK_FORMAT_R16_UNORM:
            return {DRM_FORMAT_R16};
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SRGB:
            return {DRM_FORMAT_GR88};
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SRGB:
            return {DRM_FORMAT_BGR888};
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SRGB:
            return {DRM_FORMAT_RGB888};
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
            return {DRM_FORMAT_ABGR8888, DRM_FORMAT_XBGR8888};
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SRGB:
            return {DRM_FORMAT_ARGB8888, DRM_FORMAT_XRGB8888};
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return {DRM_FORMAT_ARGB2101010};
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            return {DRM_FORMAT_ABGR2101010};
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
            return {DRM_FORMAT_RGB565};
        default:
            return {};
    }
}

std::vector<VkFormat> DrmFourCCFormatToVkFormats(int fourccFormat)
{
    switch (fourccFormat)
    {
        case DRM_FORMAT_R8:
            return {VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SRGB};
        case DRM_FORMAT_R16:
            return {VK_FORMAT_R16_UNORM};
        case DRM_FORMAT_GR88:
            return {VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SRGB};
        case DRM_FORMAT_BGR888:
            return {VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8_SRGB};
        case DRM_FORMAT_RGB888:
            return {VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_B8G8R8_SRGB};
        case DRM_FORMAT_ABGR8888:
        case DRM_FORMAT_XBGR8888:
            return {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB};
        case DRM_FORMAT_ARGB8888:
        case DRM_FORMAT_XRGB8888:
            return {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SRGB};
        case DRM_FORMAT_ABGR2101010:
            return {VK_FORMAT_A2R10G10B10_UNORM_PACK32};
        case DRM_FORMAT_ARGB2101010:
            return {VK_FORMAT_A2B10G10R10_UNORM_PACK32};
        case DRM_FORMAT_RGB565:
            return {VK_FORMAT_B5G6R5_UNORM_PACK16};
        case DRM_FORMAT_NV12:
            return {VK_FORMAT_G8_B8R8_2PLANE_420_UNORM};
        default:
            WARN() << "Unknown dma_buf format " << fourccFormat
                   << " used to initialize an EGL image.";
            return {};
    }
}

#endif  // ANGLE_ENABLE_VULKAN

#if defined(ANGLE_PLATFORM_LINUX) && defined(ANGLE_USES_GBM)
#    include <gbm.h>

int GLInternalFormatToGbmFourCCFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
        case GL_R8:
            return GBM_FORMAT_R8;
        case GL_RGB8:
            return GBM_FORMAT_GR88;
        case GL_RGB565:
            return GBM_FORMAT_RGB565;
        case GL_RGBA8:
            return GBM_FORMAT_ABGR8888;
        case GL_BGRA8_EXT:
            return GBM_FORMAT_ARGB8888;
        case GL_BGRX8_ANGLEX:
            return GBM_FORMAT_XRGB8888;
        case GL_RGBX8_ANGLE:
            return GBM_FORMAT_XBGR8888;
        case GL_RGB10_A2:
            return GBM_FORMAT_ABGR2101010;
        default:
            WARN() << "Unknown internalFormat: " << internalFormat << ". Treating as 0";
            return 0;
    }
}
#endif

}  // namespace angle
