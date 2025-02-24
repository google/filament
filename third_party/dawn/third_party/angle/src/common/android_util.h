//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// android_util.h: Utilities for the using the Android platform

#ifndef COMMON_ANDROIDUTIL_H_
#define COMMON_ANDROIDUTIL_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <array>
#include <cstdint>
#include <string>

#include "angle_gl.h"

struct ANativeWindowBuffer;
struct AHardwareBuffer;

namespace angle
{

namespace android
{

// clang-format off
/**
 * Buffer pixel formats mirrored from Android to avoid unnecessary complications
 * when trying to keep the enums defined, but not redefined, across various build
 * systems and across various releases/branches.
 *
 * Taken from
 * https://android.googlesource.com/platform/hardware/interfaces/+/refs/heads/master/graphics/common/aidl/android/hardware/graphics/common/PixelFormat.aidl
 */
enum {
    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM
     *   Vulkan: VK_FORMAT_R8G8B8A8_UNORM
     *   OpenGL ES: GL_RGBA8
     */
    ANGLE_AHB_FORMAT_R8G8B8A8_UNORM           = 1,

    /**
     * 32 bits per pixel, 8 bits per channel format where alpha values are
     * ignored (always opaque).
     *
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM
     *   Vulkan: VK_FORMAT_R8G8B8A8_UNORM
     *   OpenGL ES: GL_RGB8
     */
    ANGLE_AHB_FORMAT_R8G8B8X8_UNORM           = 2,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM
     *   Vulkan: VK_FORMAT_R8G8B8_UNORM
     *   OpenGL ES: GL_RGB8
     */
    ANGLE_AHB_FORMAT_R8G8B8_UNORM             = 3,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM
     *   Vulkan: VK_FORMAT_R5G6B5_UNORM_PACK16
     *   OpenGL ES: GL_RGB565
     */
    ANGLE_AHB_FORMAT_R5G6B5_UNORM             = 4,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM (deprecated)
     */
    ANGLE_AHB_FORMAT_B8G8R8A8_UNORM           = 5,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_B5G5R5A1_UNORM (deprecated)
     */
    ANGLE_AHB_FORMAT_B5G5R5A1_UNORM           = 6,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_B4G4R4A4_UNORM (deprecated)
     */
    ANGLE_AHB_FORMAT_B4G4R4A4_UNORM           = 7,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT
     *   Vulkan: VK_FORMAT_R16G16B16A16_SFLOAT
     *   OpenGL ES: GL_RGBA16F
     */
    ANGLE_AHB_FORMAT_R16G16B16A16_FLOAT       = 0x16,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM
     *   Vulkan: VK_FORMAT_A2B10G10R10_UNORM_PACK32
     *   OpenGL ES: GL_RGB10_A2
     */
    ANGLE_AHB_FORMAT_R10G10B10A2_UNORM        = 0x2b,

    /**
     * An opaque binary blob format that must have height 1, with width equal to
     * the buffer size in bytes.
     *
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_BLOB
     */
    ANGLE_AHB_FORMAT_BLOB                     = 0x21,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D16_UNORM
     *   Vulkan: VK_FORMAT_D16_UNORM
     *   OpenGL ES: GL_DEPTH_COMPONENT16
     */
    ANGLE_AHB_FORMAT_D16_UNORM                = 0x30,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D24_UNORM
     *   Vulkan: VK_FORMAT_X8_D24_UNORM_PACK32
     *   OpenGL ES: GL_DEPTH_COMPONENT24
     */
    ANGLE_AHB_FORMAT_D24_UNORM                = 0x31,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT
     *   Vulkan: VK_FORMAT_D24_UNORM_S8_UINT
     *   OpenGL ES: GL_DEPTH24_STENCIL8
     */
    ANGLE_AHB_FORMAT_D24_UNORM_S8_UINT        = 0x32,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D32_FLOAT
     *   Vulkan: VK_FORMAT_D32_SFLOAT
     *   OpenGL ES: GL_DEPTH_COMPONENT32F
     */
    ANGLE_AHB_FORMAT_D32_FLOAT                = 0x33,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT
     *   Vulkan: VK_FORMAT_D32_SFLOAT_S8_UINT
     *   OpenGL ES: GL_DEPTH32F_STENCIL8
     */
    ANGLE_AHB_FORMAT_D32_FLOAT_S8_UINT        = 0x34,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT
     *   Vulkan: VK_FORMAT_S8_UINT
     *   OpenGL ES: GL_STENCIL_INDEX8
     */
    ANGLE_AHB_FORMAT_S8_UINT                  = 0x35,

    /**
     * YUV 420 888 format.
     * Must have an even width and height. Can be accessed in OpenGL
     * shaders through an external sampler. Does not support mip-maps
     * cube-maps or multi-layered textures.
     *
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420
     */
    ANGLE_AHB_FORMAT_Y8Cb8Cr8_420             = 0x23,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_YV12
     *   Vulkan: VK_FORMAT_S8_UINT
     *   OpenGL ES: GL_STENCIL_INDEX8
     */
    ANGLE_AHB_FORMAT_YV12                     = 0x32315659,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED
     *   Vulkan: VK_FORMAT_S8_UINT
     *   OpenGL ES: GL_STENCIL_INDEX8
     */
    ANGLE_AHB_FORMAT_IMPLEMENTATION_DEFINED   = 0x22,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R8_UNORM
     *   Vulkan: VK_FORMAT_R8_UNORM
     *   OpenGL ES: GL_R8
     */
    ANGLE_AHB_FORMAT_R8_UNORM   = 0x38,
};
// clang-format on

constexpr std::array<GLenum, 3> kSupportedSizedInternalFormats = {GL_RGBA8, GL_RGB8, GL_RGB565};

ANativeWindowBuffer *ClientBufferToANativeWindowBuffer(EGLClientBuffer clientBuffer);
EGLClientBuffer AHardwareBufferToClientBuffer(const AHardwareBuffer *hardwareBuffer);
AHardwareBuffer *ClientBufferToAHardwareBuffer(EGLClientBuffer clientBuffer);

EGLClientBuffer CreateEGLClientBufferFromAHardwareBuffer(int width,
                                                         int height,
                                                         int depth,
                                                         int androidFormat,
                                                         int usage);

void GetANativeWindowBufferProperties(const ANativeWindowBuffer *buffer,
                                      int *width,
                                      int *height,
                                      int *depth,
                                      int *pixelFormat,
                                      uint64_t *usage);
GLenum NativePixelFormatToGLInternalFormat(int pixelFormat);
int GLInternalFormatToNativePixelFormat(GLenum internalFormat);

bool NativePixelFormatIsYUV(int pixelFormat);

AHardwareBuffer *ANativeWindowBufferToAHardwareBuffer(ANativeWindowBuffer *windowBuffer);

uint64_t GetAHBUsage(int eglNativeBufferUsage);

bool IsValidNativeWindowBuffer(ANativeWindowBuffer *windowBuffer);

bool GetSystemProperty(const char *propertyName, std::string *value);
static constexpr const char *kManufacturerSystemPropertyName = "ro.product.manufacturer";
static constexpr const char *kModelSystemPropertyName        = "ro.product.model";
static constexpr const char *kSDKSystemPropertyName          = "ro.build.version.sdk";

}  // namespace android
}  // namespace angle

#endif  // COMMON_ANDROIDUTIL_H_
