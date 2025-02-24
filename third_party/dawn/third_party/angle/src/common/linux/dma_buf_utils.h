//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// dma_buf_utils.h: Utilities to interact with Linux dma bufs.

#ifndef COMMON_LINUX_DMA_BUF_UTILS_H_
#define COMMON_LINUX_DMA_BUF_UTILS_H_

#include <angle_gl.h>

#if defined(ANGLE_ENABLE_VULKAN)
#    include <vulkan/vulkan_core.h>
#    include <vector>
#endif

// Refer to:
// https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/include/uapi/drm/drm_fourcc.h
// https://source.chromium.org/chromium/chromium/src/+/main:ui/gl/gl_image_native_pixmap.cc;l=24
#define FOURCC(a, b, c, d)                                          \
    ((static_cast<uint32_t>(a)) | (static_cast<uint32_t>(b) << 8) | \
     (static_cast<uint32_t>(c) << 16) | (static_cast<uint32_t>(d) << 24))

#define DRM_FORMAT_R8 FOURCC('R', '8', ' ', ' ')
#define DRM_FORMAT_R16 FOURCC('R', '1', '6', ' ')
#define DRM_FORMAT_GR88 FOURCC('G', 'R', '8', '8')
#define DRM_FORMAT_RGB565 FOURCC('R', 'G', '1', '6')
#define DRM_FORMAT_RGB888 FOURCC('R', 'G', '2', '4')
#define DRM_FORMAT_BGR888 FOURCC('B', 'G', '2', '4')
#define DRM_FORMAT_ARGB8888 FOURCC('A', 'R', '2', '4')
#define DRM_FORMAT_ABGR8888 FOURCC('A', 'B', '2', '4')
#define DRM_FORMAT_XRGB8888 FOURCC('X', 'R', '2', '4')
#define DRM_FORMAT_XBGR8888 FOURCC('X', 'B', '2', '4')
#define DRM_FORMAT_ABGR2101010 FOURCC('A', 'B', '3', '0')
#define DRM_FORMAT_ARGB2101010 FOURCC('A', 'R', '3', '0')
#define DRM_FORMAT_YVU420 FOURCC('Y', 'V', '1', '2')
#define DRM_FORMAT_NV12 FOURCC('N', 'V', '1', '2')
#define DRM_FORMAT_P010 FOURCC('P', '0', '1', '0')
#define DRM_FORMAT_ABGR16161616F FOURCC('A', 'B', '4', 'H')

namespace angle
{
GLenum DrmFourCCFormatToGLInternalFormat(int format, bool *isYUV);

#if defined(ANGLE_PLATFORM_LINUX) && defined(ANGLE_USES_GBM)
int GLInternalFormatToGbmFourCCFormat(GLenum internalFormat);
#endif

#if defined(ANGLE_ENABLE_VULKAN)
std::vector<int> VkFormatToDrmFourCCFormat(VkFormat format);
std::vector<VkFormat> DrmFourCCFormatToVkFormats(int fourccFormat);
#endif

}  // namespace angle

#endif  // COMMON_LINUX_DMA_BUF_UTILS_H_
