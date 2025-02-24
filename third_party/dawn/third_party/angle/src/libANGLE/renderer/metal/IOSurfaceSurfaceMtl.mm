//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IOSurfaceSurfaceMtl.mm:
//    Implements the class methods for IOSurfaceSurfaceMtl.
//

#include "libANGLE/renderer/metal/IOSurfaceSurfaceMtl.h"

#include <TargetConditionals.h>

#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/FrameBufferMtl.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#include "libANGLE/renderer/metal/mtl_utils.h"

// Compiler can turn on programmatical frame capture in release build by defining
// ANGLE_METAL_FRAME_CAPTURE flag.
#if defined(NDEBUG) && !defined(ANGLE_METAL_FRAME_CAPTURE)
#    define ANGLE_METAL_FRAME_CAPTURE_ENABLED 0
#else
#    define ANGLE_METAL_FRAME_CAPTURE_ENABLED 1
#endif
namespace rx
{

namespace
{

struct IOSurfaceFormatInfo
{
    GLenum internalFormat;
    GLenum type;
    size_t componentBytes;

    angle::FormatID nativeAngleFormatId;
};

// clang-format off
// GL_RGB is a special case. The native angle::FormatID would be either R8G8B8X8_UNORM
// or B8G8R8X8_UNORM based on the IOSurface's pixel format.
constexpr std::array<IOSurfaceFormatInfo, 9> kIOSurfaceFormats = {{
    {GL_RED,         GL_UNSIGNED_BYTE,                  1, angle::FormatID::R8_UNORM},
    {GL_RED,         GL_UNSIGNED_SHORT,                 2, angle::FormatID::R16_UNORM},
    {GL_RG,          GL_UNSIGNED_BYTE,                  2, angle::FormatID::R8G8_UNORM},
    {GL_RG,          GL_UNSIGNED_SHORT,                 4, angle::FormatID::R16G16_UNORM},
    {GL_RGB,         GL_UNSIGNED_BYTE,                  4, angle::FormatID::NONE},
    {GL_RGBA,        GL_UNSIGNED_BYTE,                  4, angle::FormatID::R8G8B8A8_UNORM},
    {GL_BGRA_EXT,    GL_UNSIGNED_BYTE,                  4, angle::FormatID::B8G8R8A8_UNORM},
    {GL_RGBA,        GL_HALF_FLOAT,                     8, angle::FormatID::R16G16B16A16_FLOAT},
    {GL_RGB10_A2,    GL_UNSIGNED_INT_2_10_10_10_REV,    4, angle::FormatID::B10G10R10A2_UNORM},
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

// IOSurfaceSurfaceMtl implementation.
IOSurfaceSurfaceMtl::IOSurfaceSurfaceMtl(DisplayMtl *display,
                                         const egl::SurfaceState &state,
                                         EGLClientBuffer buffer,
                                         const egl::AttributeMap &attribs)
    : OffscreenSurfaceMtl(display, state, attribs), mIOSurface((__bridge IOSurfaceRef)(buffer))
{
    CFRetain(mIOSurface);

    mIOSurfacePlane = static_cast<int>(attribs.get(EGL_IOSURFACE_PLANE_ANGLE));

    EGLAttrib internalFormat = attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE);
    EGLAttrib type           = attribs.get(EGL_TEXTURE_TYPE_ANGLE);
    mIOSurfaceFormatIdx =
        FindIOSurfaceFormatIndex(static_cast<GLenum>(internalFormat), static_cast<GLenum>(type));
    ASSERT(mIOSurfaceFormatIdx >= 0);

    angle::FormatID actualAngleFormatId =
        kIOSurfaceFormats[mIOSurfaceFormatIdx].nativeAngleFormatId;
    if (actualAngleFormatId == angle::FormatID::NONE)
    {
        // The actual angle::Format depends on the IOSurface's format.
        ASSERT(internalFormat == GL_RGB);
        switch (IOSurfaceGetPixelFormat(mIOSurface))
        {
            case 'BGRA':
                actualAngleFormatId = angle::FormatID::B8G8R8X8_UNORM;
                break;
            case 'RGBA':
                actualAngleFormatId = angle::FormatID::R8G8B8X8_UNORM;
                break;
            default:
                UNREACHABLE();
        }
    }

    mColorFormat = display->getPixelFormat(actualAngleFormatId);
}
IOSurfaceSurfaceMtl::~IOSurfaceSurfaceMtl()
{
    if (mIOSurface != nullptr)
    {
        CFRelease(mIOSurface);
        mIOSurface = nullptr;
    }
}

egl::Error IOSurfaceSurfaceMtl::bindTexImage(const gl::Context *context,
                                             gl::Texture *texture,
                                             EGLint buffer)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    StartFrameCapture(contextMtl);

    // Initialize offscreen texture if needed:
    ANGLE_TO_EGL_TRY(ensureColorTextureCreated(context));

    return OffscreenSurfaceMtl::bindTexImage(context, texture, buffer);
}

egl::Error IOSurfaceSurfaceMtl::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    egl::Error re = OffscreenSurfaceMtl::releaseTexImage(context, buffer);
    StopFrameCapture();
    return re;
}

angle::Result IOSurfaceSurfaceMtl::getAttachmentRenderTarget(
    const gl::Context *context,
    GLenum binding,
    const gl::ImageIndex &imageIndex,
    GLsizei samples,
    FramebufferAttachmentRenderTarget **rtOut)
{
    // Initialize offscreen texture if needed:
    ANGLE_TRY(ensureColorTextureCreated(context));

    return OffscreenSurfaceMtl::getAttachmentRenderTarget(context, binding, imageIndex, samples,
                                                          rtOut);
}

angle::Result IOSurfaceSurfaceMtl::ensureColorTextureCreated(const gl::Context *context)
{
    if (mColorTexture)
    {
        return angle::Result::Continue;
    }
    ContextMtl *contextMtl = mtl::GetImpl(context);
    ANGLE_MTL_OBJC_SCOPE
    {
        auto texDesc =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:mColorFormat.metalFormat
                                                               width:mSize.width
                                                              height:mSize.height
                                                           mipmapped:NO];

        texDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;

        mColorTexture =
            mtl::Texture::MakeFromMetal(contextMtl->getMetalDevice().newTextureWithDescriptor(
                texDesc, mIOSurface, mIOSurfacePlane));

        if (mColorTexture)
        {
            size_t resourceSize = EstimateTextureSizeInBytes(
                mColorFormat, mColorTexture->widthAt0(), mColorTexture->heightAt0(),
                mColorTexture->depthAt0(), mColorTexture->samples(), mColorTexture->mipmapLevels());

            mColorTexture->setEstimatedByteSize(resourceSize);
        }
    }

    mColorRenderTarget.set(mColorTexture, mtl::kZeroNativeMipLevel, 0, mColorFormat);

    if (kIOSurfaceFormats[mIOSurfaceFormatIdx].internalFormat == GL_RGB)
    {
        // This format has emulated alpha channel. Initialize texture's alpha channel to 1.0.
        const mtl::Format &rgbClearFormat =
            contextMtl->getPixelFormat(angle::FormatID::R8G8B8_UNORM);
        ANGLE_TRY(mtl::InitializeTextureContentsGPU(
            context, mColorTexture, rgbClearFormat,
            mtl::ImageNativeIndex::FromBaseZeroGLIndex(gl::ImageIndex::Make2D(0)),
            MTLColorWriteMaskAlpha));

        // Disable subsequent rendering to alpha channel.
        mColorTexture->setColorWritableMask(MTLColorWriteMaskAll & (~MTLColorWriteMaskAlpha));
    }
    // Robust resource init: currently we do not allow passing contents with IOSurfaces.
    mColorTextureInitialized = false;

    return angle::Result::Continue;
}

// static
bool IOSurfaceSurfaceMtl::ValidateAttributes(EGLClientBuffer buffer,
                                             const egl::AttributeMap &attribs)
{
    IOSurfaceRef ioSurface = (__bridge IOSurfaceRef)(buffer);

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

    // FIXME: Check that the format matches this IOSurface plane for pixel formats that we know of.
    // We could map IOSurfaceGetPixelFormat to expected type plane and format type.
    // However, the caller might supply us non-public pixel format, which makes exhaustive checks
    // problematic.
    if (IOSurfaceGetBytesPerElementOfPlane(ioSurface, plane) !=
        kIOSurfaceFormats[formatIndex].componentBytes)
    {
        WARN() << "IOSurface bytes per elements does not match the pbuffer internal format.";
    }

    return true;
}
}  // namespace rx
