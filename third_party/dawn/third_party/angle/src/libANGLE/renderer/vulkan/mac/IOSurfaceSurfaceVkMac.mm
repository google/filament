//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IOSurfaceSurfaceVkMac.mm:
//    Implements methods from IOSurfaceSurfaceVkMac.
//

#include "libANGLE/renderer/vulkan/mac/IOSurfaceSurfaceVkMac.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

#include <IOSurface/IOSurface.h>

namespace rx
{

namespace
{

struct IOSurfaceFormatInfo
{
    GLenum internalFormat;
    GLenum type;

    size_t componentBytes;

    GLenum nativeSizedInternalFormat;
};

// clang-format off
constexpr std::array<IOSurfaceFormatInfo, 9> kIOSurfaceFormats = {{
    {GL_RED,      GL_UNSIGNED_BYTE,                1, GL_R8},
    {GL_RED,      GL_UNSIGNED_SHORT,               2, GL_R16_EXT},
    {GL_RG,       GL_UNSIGNED_BYTE,                2, GL_RG8},
    {GL_RG,       GL_UNSIGNED_SHORT,               4, GL_RG16_EXT},
    {GL_RGB,      GL_UNSIGNED_BYTE,                4, GL_RGBX8_ANGLE},
    {GL_BGRA_EXT, GL_UNSIGNED_BYTE,                4, GL_BGRA8_EXT},
    {GL_RGB10_A2, GL_UNSIGNED_INT_2_10_10_10_REV,  4, GL_BGR10_A2_ANGLEX},
    {GL_RGBA,     GL_HALF_FLOAT,                   8, GL_RGBA16F},
}};
// clang-format on

int FindIOSurfaceFormatIndex(GLenum internalFormat, GLenum type)
{
    for (int i = 0; i < static_cast<int>(kIOSurfaceFormats.size()); ++i)
    {
        const auto &formatInfo = kIOSurfaceFormats[i];
        if (formatInfo.internalFormat == internalFormat && formatInfo.type == type)
        {
            return i;
        }
    }
    return -1;
}

}  // anonymous namespace

IOSurfaceSurfaceVkMac::IOSurfaceSurfaceVkMac(const egl::SurfaceState &state,
                                             EGLClientBuffer buffer,
                                             const egl::AttributeMap &attribs,
                                             vk::Renderer *renderer)
    : OffscreenSurfaceVk(state, renderer), mIOSurface(nullptr), mPlane(0), mFormatIndex(-1)
{
    // Keep reference to the IOSurface so it doesn't get deleted while the pbuffer exists.
    mIOSurface = reinterpret_cast<IOSurfaceRef>(buffer);
    CFRetain(mIOSurface);

    // Extract attribs useful for the call to CGLTexImageIOSurface2D
    mWidth  = static_cast<int>(attribs.get(EGL_WIDTH));
    mHeight = static_cast<int>(attribs.get(EGL_HEIGHT));
    mPlane  = static_cast<int>(attribs.get(EGL_IOSURFACE_PLANE_ANGLE));

    EGLAttrib internalFormat = attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE);
    EGLAttrib type           = attribs.get(EGL_TEXTURE_TYPE_ANGLE);
    mFormatIndex =
        FindIOSurfaceFormatIndex(static_cast<GLenum>(internalFormat), static_cast<GLenum>(type));
    ASSERT(mFormatIndex >= 0);
}

IOSurfaceSurfaceVkMac::~IOSurfaceSurfaceVkMac()
{
    if (mIOSurface != nullptr)
    {
        CFRelease(mIOSurface);
        mIOSurface = nullptr;
    }
}

egl::Error IOSurfaceSurfaceVkMac::initialize(const egl::Display *display)
{
    DisplayVk *displayVk = vk::GetImpl(display);
    angle::Result result = initializeImpl(displayVk);
    return angle::ToEGL(result, EGL_BAD_SURFACE);
}

angle::Result IOSurfaceSurfaceVkMac::initializeImpl(DisplayVk *displayVk)
{
    vk::Renderer *renderer    = displayVk->getRenderer();
    const egl::Config *config = mState.config;

    // Should never be > 1
    GLint samples = 1;
    if (config->sampleBuffers && config->samples > 1)
    {
        samples = config->samples;
    }
    ANGLE_VK_CHECK(displayVk, samples == 1, VK_ERROR_INITIALIZATION_FAILED);

    const vk::Format &format =
        renderer->getFormat(kIOSurfaceFormats[mFormatIndex].nativeSizedInternalFormat);

    // Swiftshader will use the raw pointer to the buffer referenced by the IOSurfaceRef
    ANGLE_TRY(mColorAttachment.initialize(displayVk, mWidth, mHeight, format, samples,
                                          mState.isRobustResourceInitEnabled(),
                                          mState.hasProtectedContent()));

    mColorRenderTarget.init(&mColorAttachment.image, &mColorAttachment.imageViews, nullptr, nullptr,
                            {}, gl::LevelIndex(0), 0, 1, RenderTargetTransience::Default);

    return angle::Result::Continue;
}

egl::Error IOSurfaceSurfaceVkMac::unMakeCurrent(const gl::Context *context)
{
    ASSERT(context != nullptr);
    ContextVk *contextVk = vk::GetImpl(context);
    angle::Result result =
        contextVk->flushAndSubmitCommands(nullptr, nullptr, RenderPassClosureReason::ContextChange);
    return angle::ToEGL(result, EGL_BAD_SURFACE);
}

int IOSurfaceSurfaceVkMac::computeAlignment() const
{
    size_t rowBytes         = IOSurfaceGetBytesPerRowOfPlane(mIOSurface, mPlane);
    size_t desiredAlignment = IOSurfaceAlignProperty(kIOSurfaceBytesPerRow, 1);
    size_t alignment        = 1;
    while (alignment < desiredAlignment)
    {
        if (rowBytes & alignment)
        {
            break;
        }
        alignment <<= 1;
    }
    return static_cast<int>(alignment);
}

egl::Error IOSurfaceSurfaceVkMac::bindTexImage(const gl::Context *context,
                                               gl::Texture *texture,
                                               EGLint buffer)
{
    IOSurfaceLock(mIOSurface, 0, nullptr);

    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    size_t width             = IOSurfaceGetWidthOfPlane(mIOSurface, mPlane);
    size_t height            = IOSurfaceGetHeightOfPlane(mIOSurface, mPlane);
    size_t rowLengthInPixels = IOSurfaceGetBytesPerRowOfPlane(mIOSurface, mPlane) /
                               IOSurfaceGetBytesPerElementOfPlane(mIOSurface, mPlane);

    gl::PixelUnpackState pixelUnpack;
    pixelUnpack.alignment   = computeAlignment();
    pixelUnpack.rowLength   = static_cast<GLint>(rowLengthInPixels);
    pixelUnpack.imageHeight = static_cast<GLint>(height);

    void *source = IOSurfaceGetBaseAddressOfPlane(mIOSurface, mPlane);

    const gl::InternalFormat &internalFormatInfo =
        gl::GetSizedInternalFormatInfo(kIOSurfaceFormats[mFormatIndex].nativeSizedInternalFormat);
    const vk::Format &format =
        renderer->getFormat(kIOSurfaceFormats[mFormatIndex].nativeSizedInternalFormat);

    bool updateAppliedImmediately = false;
    angle::Result result          = mColorAttachment.image.stageSubresourceUpdate(
        contextVk, gl::ImageIndex::Make2D(0),
        gl::Extents(static_cast<int>(width), pixelUnpack.imageHeight, 1), gl::Offset(),
        internalFormatInfo, pixelUnpack, kIOSurfaceFormats[mFormatIndex].type,
        reinterpret_cast<uint8_t *>(source), format, vk::ImageAccess::Renderable,
        vk::ApplyImageUpdate::Defer, &updateAppliedImmediately);

    IOSurfaceUnlock(mIOSurface, 0, nullptr);

    return angle::ToEGL(result, EGL_BAD_SURFACE);
}

egl::Error IOSurfaceSurfaceVkMac::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    ASSERT(context != nullptr);
    ContextVk *contextVk = vk::GetImpl(context);

    angle::Result result = mColorAttachment.image.flushAllStagedUpdates(contextVk);

    if (result != angle::Result::Continue)
    {
        return angle::ToEGL(result, EGL_BAD_SURFACE);
    }

    gl::Rectangle bounds(0, 0, mWidth, mHeight);

    const angle::Format &dstFormat = angle::Format::Get(angle::Format::InternalFormatToID(
        kIOSurfaceFormats[mFormatIndex].nativeSizedInternalFormat));

    IOSurfaceLock(mIOSurface, 0, nullptr);

    size_t outputRowPitchInBytes = IOSurfaceGetBytesPerRowOfPlane(mIOSurface, mPlane);

    PackPixelsParams params(bounds, dstFormat, static_cast<GLuint>(outputRowPitchInBytes),
                            contextVk->isViewportFlipEnabledForDrawFBO(), nullptr, 0);

    result = mColorAttachment.image.readPixels(contextVk, bounds, params, VK_IMAGE_ASPECT_COLOR_BIT,
                                               gl::LevelIndex(0), 0,
                                               IOSurfaceGetBaseAddressOfPlane(mIOSurface, mPlane));

    IOSurfaceUnlock(mIOSurface, 0, nullptr);

    return angle::ToEGL(result, EGL_BAD_SURFACE);
}

// static
bool IOSurfaceSurfaceVkMac::ValidateAttributes(const DisplayVk *displayVk,
                                               EGLClientBuffer buffer,
                                               const egl::AttributeMap &attribs)
{
    ASSERT(displayVk != nullptr);
    IOSurfaceRef ioSurface = reinterpret_cast<IOSurfaceRef>(buffer);

    // The plane must exist for this IOSurface. IOSurfaceGetPlaneCount can return 0 for non-planar
    // ioSurfaces but we will treat non-planar like it is a single plane.
    size_t surfacePlaneCount = std::max(size_t(1), IOSurfaceGetPlaneCount(ioSurface));
    EGLAttrib plane          = attribs.get(EGL_IOSURFACE_PLANE_ANGLE);
    if (plane < 0 || static_cast<size_t>(plane) >= surfacePlaneCount)
    {
        return false;
    }

    // The width height specified must be at least (1, 1) and at most the plane size
    EGLAttrib width  = attribs.get(EGL_WIDTH);
    EGLAttrib height = attribs.get(EGL_HEIGHT);
    if (width <= 0 || static_cast<size_t>(width) > IOSurfaceGetWidthOfPlane(ioSurface, plane) ||
        height <= 0 || static_cast<size_t>(height) > IOSurfaceGetHeightOfPlane(ioSurface, plane))
    {
        return false;
    }

    // Find this IOSurface format
    EGLAttrib internalFormat = attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE);
    EGLAttrib type           = attribs.get(EGL_TEXTURE_TYPE_ANGLE);

    int formatIndex =
        FindIOSurfaceFormatIndex(static_cast<GLenum>(internalFormat), static_cast<GLenum>(type));

    if (formatIndex < 0)
    {
        return false;
    }

    // Check that the format matches this IOSurface plane
    if (IOSurfaceGetBytesPerElementOfPlane(ioSurface, plane) !=
        kIOSurfaceFormats[formatIndex].componentBytes)
    {
        return false;
    }

    return true;
}

}  // namespace rx
