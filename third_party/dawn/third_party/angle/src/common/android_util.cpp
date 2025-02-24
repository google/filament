//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// android_util.cpp: Utilities for the using the Android platform

#include "common/android_util.h"
#include "common/debug.h"

#if defined(ANGLE_PLATFORM_ANDROID)
#    include <sys/system_properties.h>
#endif

#if defined(ANGLE_PLATFORM_ANDROID) && __ANDROID_API__ >= 26
#    define ANGLE_AHARDWARE_BUFFER_SUPPORT
// NDK header file for access to Android Hardware Buffers
#    include <android/hardware_buffer.h>
#endif

// Taken from cutils/native_handle.h:
// https://android.googlesource.com/platform/system/core/+/master/libcutils/include/cutils/native_handle.h
typedef struct native_handle
{
    int version; /* sizeof(native_handle_t) */
    int numFds;  /* number of file-descriptors at &data[0] */
    int numInts; /* number of ints at &data[numFds] */
#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wzero-length-array"
#elif defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4200)
#endif
    int data[0]; /* numFds + numInts ints */
#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(_MSC_VER)
#    pragma warning(pop)
#endif
} native_handle_t;

// Taken from nativebase/nativebase.h
// https://android.googlesource.com/platform/frameworks/native/+/master/libs/nativebase/include/nativebase/nativebase.h
typedef const native_handle_t *buffer_handle_t;

// Taken from nativebase/nativebase.h
// https://android.googlesource.com/platform/frameworks/native/+/master/libs/nativebase/include/nativebase/nativebase.h
#define ANDROID_NATIVE_UNSIGNED_CAST(x) static_cast<unsigned int>(x)

#define ANDROID_NATIVE_MAKE_CONSTANT(a, b, c, d)                                         \
    ((ANDROID_NATIVE_UNSIGNED_CAST(a) << 24) | (ANDROID_NATIVE_UNSIGNED_CAST(b) << 16) | \
     (ANDROID_NATIVE_UNSIGNED_CAST(c) << 8) | (ANDROID_NATIVE_UNSIGNED_CAST(d)))

#define ANDROID_NATIVE_BUFFER_MAGIC ANDROID_NATIVE_MAKE_CONSTANT('_', 'b', 'f', 'r')

typedef struct android_native_base_t
{
    /* a magic value defined by the actual EGL native type */
    int magic;
    /* the sizeof() of the actual EGL native type */
    int version;
    void *reserved[4];
    /* reference-counting interface */
    void (*incRef)(struct android_native_base_t *base);
    void (*decRef)(struct android_native_base_t *base);
} android_native_base_t;

typedef struct ANativeWindowBuffer
{
    struct android_native_base_t common;
    int width;
    int height;
    int stride;
    int format;
    int usage_deprecated;
    uintptr_t layerCount;
    void *reserved[1];
    const native_handle_t *handle;
    uint64_t usage;
    // we needed extra space for storing the 64-bits usage flags
    // the number of slots to use from reserved_proc depends on the
    // architecture.
    void *reserved_proc[8 - (sizeof(uint64_t) / sizeof(void *))];
} ANativeWindowBuffer_t;

namespace angle
{

namespace android
{

namespace
{

// In the Android system:
// - AHardwareBuffer is essentially a typedef of GraphicBuffer. Conversion functions simply
// reinterpret_cast.
// - GraphicBuffer inherits from two base classes, ANativeWindowBuffer and RefBase.
//
// GraphicBuffer implements a getter for ANativeWindowBuffer (getNativeBuffer) by static_casting
// itself to its base class ANativeWindowBuffer. The offset of the ANativeWindowBuffer pointer
// from the GraphicBuffer pointer is 16 bytes. This is likely due to two pointers: The vtable of
// GraphicBuffer and the one pointer member of the RefBase class.
//
// This is not future proof at all. We need to look into getting utilities added to Android to
// perform this cast for us.
constexpr int kAHardwareBufferToANativeWindowBufferOffset = static_cast<int>(sizeof(void *)) * 2;

template <typename T1, typename T2>
T1 *OffsetPointer(T2 *ptr, int bytes)
{
    return reinterpret_cast<T1 *>(reinterpret_cast<intptr_t>(ptr) + bytes);
}

GLenum GetPixelFormatInfo(int pixelFormat, bool *isYUV)
{
    *isYUV = false;
    switch (pixelFormat)
    {
        case ANGLE_AHB_FORMAT_R8G8B8A8_UNORM:
            return GL_RGBA8;
        case ANGLE_AHB_FORMAT_R8G8B8X8_UNORM:
            return GL_RGB8;
        case ANGLE_AHB_FORMAT_R8G8B8_UNORM:
            return GL_RGB8;
        case ANGLE_AHB_FORMAT_R5G6B5_UNORM:
            return GL_RGB565;
        case ANGLE_AHB_FORMAT_B8G8R8A8_UNORM:
            return GL_BGRA8_EXT;
        case ANGLE_AHB_FORMAT_B5G5R5A1_UNORM:
            return GL_RGB5_A1;
        case ANGLE_AHB_FORMAT_B4G4R4A4_UNORM:
            return GL_RGBA4;
        case ANGLE_AHB_FORMAT_R16G16B16A16_FLOAT:
            return GL_RGBA16F;
        case ANGLE_AHB_FORMAT_R10G10B10A2_UNORM:
            return GL_RGB10_A2;
        case ANGLE_AHB_FORMAT_BLOB:
            return GL_NONE;
        case ANGLE_AHB_FORMAT_D16_UNORM:
            return GL_DEPTH_COMPONENT16;
        case ANGLE_AHB_FORMAT_D24_UNORM:
            return GL_DEPTH_COMPONENT24;
        case ANGLE_AHB_FORMAT_D24_UNORM_S8_UINT:
            return GL_DEPTH24_STENCIL8;
        case ANGLE_AHB_FORMAT_D32_FLOAT:
            return GL_DEPTH_COMPONENT32F;
        case ANGLE_AHB_FORMAT_D32_FLOAT_S8_UINT:
            return GL_DEPTH32F_STENCIL8;
        case ANGLE_AHB_FORMAT_S8_UINT:
            return GL_STENCIL_INDEX8;
        case ANGLE_AHB_FORMAT_R8_UNORM:
            return GL_R8;
        case ANGLE_AHB_FORMAT_Y8Cb8Cr8_420:
        case ANGLE_AHB_FORMAT_YV12:
        case ANGLE_AHB_FORMAT_IMPLEMENTATION_DEFINED:
            *isYUV = true;
            return GL_RGB8;
        default:
            // Treat unknown formats as RGB. They are vendor-specific YUV formats that would sample
            // as RGB.
            *isYUV = true;
            return GL_RGB8;
    }
}

}  // anonymous namespace

ANativeWindowBuffer *ClientBufferToANativeWindowBuffer(EGLClientBuffer clientBuffer)
{
    return reinterpret_cast<ANativeWindowBuffer *>(clientBuffer);
}

bool IsValidNativeWindowBuffer(ANativeWindowBuffer *windowBuffer)
{
    return windowBuffer->common.magic == ANDROID_NATIVE_BUFFER_MAGIC;
}

uint64_t GetAHBUsage(int eglNativeBufferUsage)
{
    uint64_t ahbUsage = 0;
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)
    if (eglNativeBufferUsage & EGL_NATIVE_BUFFER_USAGE_PROTECTED_BIT_ANDROID)
    {
        ahbUsage |= AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
    }
    if (eglNativeBufferUsage & EGL_NATIVE_BUFFER_USAGE_RENDERBUFFER_BIT_ANDROID)
    {
        ahbUsage |= AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER;
    }
    if (eglNativeBufferUsage & EGL_NATIVE_BUFFER_USAGE_TEXTURE_BIT_ANDROID)
    {
        ahbUsage |= AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    }
#endif  // ANGLE_AHARDWARE_BUFFER_SUPPORT
    return ahbUsage;
}

EGLClientBuffer CreateEGLClientBufferFromAHardwareBuffer(int width,
                                                         int height,
                                                         int depth,
                                                         int androidFormat,
                                                         int usage)
{
#if defined(ANGLE_AHARDWARE_BUFFER_SUPPORT)

    // The height and width are number of pixels of size format
    AHardwareBuffer_Desc aHardwareBufferDescription = {};
    aHardwareBufferDescription.width                = static_cast<uint32_t>(width);
    aHardwareBufferDescription.height               = static_cast<uint32_t>(height);
    aHardwareBufferDescription.layers               = static_cast<uint32_t>(depth);
    aHardwareBufferDescription.format               = androidFormat;
    aHardwareBufferDescription.usage                = GetAHBUsage(usage);

    // Allocate memory from Android Hardware Buffer
    AHardwareBuffer *aHardwareBuffer = nullptr;
    int res = AHardwareBuffer_allocate(&aHardwareBufferDescription, &aHardwareBuffer);
    if (res != 0)
    {
        return nullptr;
    }

    return AHardwareBufferToClientBuffer(aHardwareBuffer);
#else
    return nullptr;
#endif  // ANGLE_AHARDWARE_BUFFER_SUPPORT
}

void GetANativeWindowBufferProperties(const ANativeWindowBuffer *buffer,
                                      int *width,
                                      int *height,
                                      int *depth,
                                      int *pixelFormat,
                                      uint64_t *usage)
{
    *width       = buffer->width;
    *height      = buffer->height;
    *depth       = static_cast<int>(buffer->layerCount);
    *height      = buffer->height;
    *pixelFormat = buffer->format;
    *usage       = buffer->usage;
}

GLenum NativePixelFormatToGLInternalFormat(int pixelFormat)
{
    bool isYuv = false;
    return GetPixelFormatInfo(pixelFormat, &isYuv);
}

int GLInternalFormatToNativePixelFormat(GLenum internalFormat)
{
    switch (internalFormat)
    {
        case GL_R8:
            return ANGLE_AHB_FORMAT_R8_UNORM;
        case GL_RGBA8:
            return ANGLE_AHB_FORMAT_R8G8B8A8_UNORM;
        case GL_RGB8:
            return ANGLE_AHB_FORMAT_R8G8B8X8_UNORM;
        case GL_RGB565:
            return ANGLE_AHB_FORMAT_R5G6B5_UNORM;
        case GL_BGRA8_EXT:
            return ANGLE_AHB_FORMAT_B8G8R8A8_UNORM;
        case GL_RGB5_A1:
            return ANGLE_AHB_FORMAT_B5G5R5A1_UNORM;
        case GL_RGBA4:
            return ANGLE_AHB_FORMAT_B4G4R4A4_UNORM;
        case GL_RGBA16F:
            return ANGLE_AHB_FORMAT_R16G16B16A16_FLOAT;
        case GL_RGB10_A2:
            return ANGLE_AHB_FORMAT_R10G10B10A2_UNORM;
        case GL_NONE:
            return ANGLE_AHB_FORMAT_BLOB;
        case GL_DEPTH_COMPONENT16:
            return ANGLE_AHB_FORMAT_D16_UNORM;
        case GL_DEPTH_COMPONENT24:
            return ANGLE_AHB_FORMAT_D24_UNORM;
        case GL_DEPTH24_STENCIL8:
            return ANGLE_AHB_FORMAT_D24_UNORM_S8_UINT;
        case GL_DEPTH_COMPONENT32F:
            return ANGLE_AHB_FORMAT_D32_FLOAT;
        case GL_DEPTH32F_STENCIL8:
            return ANGLE_AHB_FORMAT_D32_FLOAT_S8_UINT;
        case GL_STENCIL_INDEX8:
            return ANGLE_AHB_FORMAT_S8_UINT;
        default:
            WARN() << "Unknown internalFormat: " << internalFormat << ". Treating as 0";
            return 0;
    }
}

bool NativePixelFormatIsYUV(int pixelFormat)
{
    bool isYuv = false;
    GetPixelFormatInfo(pixelFormat, &isYuv);
    return isYuv;
}

AHardwareBuffer *ANativeWindowBufferToAHardwareBuffer(ANativeWindowBuffer *windowBuffer)
{
    return OffsetPointer<AHardwareBuffer>(windowBuffer,
                                          -kAHardwareBufferToANativeWindowBufferOffset);
}

EGLClientBuffer AHardwareBufferToClientBuffer(const AHardwareBuffer *hardwareBuffer)
{
    return OffsetPointer<EGLClientBuffer>(hardwareBuffer,
                                          kAHardwareBufferToANativeWindowBufferOffset);
}

AHardwareBuffer *ClientBufferToAHardwareBuffer(EGLClientBuffer clientBuffer)
{
    return OffsetPointer<AHardwareBuffer>(clientBuffer,
                                          -kAHardwareBufferToANativeWindowBufferOffset);
}

bool GetSystemProperty(const char *propertyName, std::string *value)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    // PROP_VALUE_MAX from <sys/system_properties.h>
    std::vector<char> propertyBuf(PROP_VALUE_MAX);
    int len = __system_property_get(propertyName, propertyBuf.data());
    if (len <= 0)
    {
        return false;
    }
    *value = std::string(propertyBuf.data());
    return true;
#else
    return false;
#endif
}

}  // namespace android
}  // namespace angle
