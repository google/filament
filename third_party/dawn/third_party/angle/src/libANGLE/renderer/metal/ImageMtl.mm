//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageMtl.cpp:
//    Implements the class methods for ImageMtl.
//

#include "libANGLE/renderer/metal/ImageMtl.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/RenderBufferMtl.h"
#include "libANGLE/renderer/metal/TextureMtl.h"

namespace rx
{

namespace
{
angle::FormatID intendedFormatForMTLTexture(id<MTLTexture> texture,
                                            const egl::AttributeMap &attribs)
{
    angle::FormatID angleFormatId = mtl::Format::MetalToAngleFormatID(texture.pixelFormat);
    if (angleFormatId == angle::FormatID::NONE)
    {
        return angle::FormatID::NONE;
    }

    const angle::Format *textureAngleFormat = &angle::Format::Get(angleFormatId);
    ASSERT(textureAngleFormat);

    GLenum sizedInternalFormat = textureAngleFormat->glInternalFormat;

    if (attribs.contains(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE))
    {
        const GLenum internalFormat =
            static_cast<GLenum>(attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE));
        GLenum type       = gl::GetSizedInternalFormatInfo(sizedInternalFormat).type;
        const auto format = gl::Format(internalFormat, type);
        if (!format.valid())
        {
            return angle::FormatID::NONE;
        }

        sizedInternalFormat = format.info->sizedInternalFormat;
    }

    return angle::Format::InternalFormatToID(sizedInternalFormat);
}
}  // anonymous namespace

// TextureImageSiblingMtl implementation
TextureImageSiblingMtl::TextureImageSiblingMtl(EGLClientBuffer buffer,
                                               const egl::AttributeMap &attribs)
    : mBuffer(buffer), mAttribs(attribs), mGLFormat(GL_NONE)
{}

TextureImageSiblingMtl::~TextureImageSiblingMtl() {}

// Static
egl::Error TextureImageSiblingMtl::ValidateClientBuffer(const DisplayMtl *display,
                                                        EGLClientBuffer buffer,
                                                        const egl::AttributeMap &attribs)
{
    id<MTLTexture> texture = (__bridge id<MTLTexture>)(buffer);
    if (!texture || texture.device != display->getMetalDevice())
    {
        return egl::EglBadAttribute();
    }

    if (texture.textureType != MTLTextureType2D && texture.textureType != MTLTextureTypeCube &&
        texture.textureType != MTLTextureType2DArray)
    {
        return egl::EglBadAttribute();
    }

    angle::FormatID angleFormatId = intendedFormatForMTLTexture(texture, attribs);
    const mtl::Format &format     = display->getPixelFormat(angleFormatId);
    if (!format.valid())
    {
        return egl::EglBadAttribute() << "Unrecognized format";
    }

    if (format.metalFormat != texture.pixelFormat)
    {
        return egl::EglBadAttribute() << "Incompatible format";
    }

    unsigned textureArraySlice =
        static_cast<unsigned>(attribs.getAsInt(EGL_METAL_TEXTURE_ARRAY_SLICE_ANGLE, 0));
    if (texture.textureType != MTLTextureType2DArray && textureArraySlice > 0)
    {
        return egl::EglBadAttribute() << "Invalid texture type for non-zero texture array slice";
    }
    if (textureArraySlice >= texture.arrayLength)
    {
        return egl::EglBadAttribute() << "Invalid texture array slice: " << textureArraySlice;
    }

    return egl::NoError();
}

egl::Error TextureImageSiblingMtl::initialize(const egl::Display *display)
{
    DisplayMtl *displayMtl = mtl::GetImpl(display);
    if (initImpl(displayMtl) != angle::Result::Continue)
    {
        return egl::EglBadParameter();
    }

    return egl::NoError();
}

angle::Result TextureImageSiblingMtl::initImpl(DisplayMtl *displayMtl)
{
    mNativeTexture = mtl::Texture::MakeFromMetal((__bridge id<MTLTexture>)(mBuffer));

    if (mNativeTexture->textureType() == MTLTextureType2DArray)
    {
        mtl::TextureRef baseTexture = std::move(mNativeTexture);
        unsigned textureArraySlice =
            static_cast<unsigned>(mAttribs.getAsInt(EGL_METAL_TEXTURE_ARRAY_SLICE_ANGLE, 0));
        mNativeTexture =
            baseTexture->createSliceMipView(textureArraySlice, mtl::kZeroNativeMipLevel);
    }

    angle::FormatID angleFormatId = intendedFormatForMTLTexture(mNativeTexture->get(), mAttribs);
    mFormat                       = displayMtl->getPixelFormat(angleFormatId);

    if (mNativeTexture)
    {
        size_t resourceSize = EstimateTextureSizeInBytes(
            mFormat, mNativeTexture->widthAt0(), mNativeTexture->heightAt0(),
            mNativeTexture->depthAt0(), mNativeTexture->samples(), mNativeTexture->mipmapLevels());
        mNativeTexture->setEstimatedByteSize(resourceSize);
    }

    mGLFormat = gl::Format(mFormat.intendedAngleFormat().glInternalFormat);

    mRenderable = mFormat.getCaps().depthRenderable || mFormat.getCaps().colorRenderable;

    // Some formats are not filterable but renderable such as integer formats. In this case, treat
    // them as texturable as well.
    mTextureable = mFormat.getCaps().filterable || mRenderable;

    return angle::Result::Continue;
}

void TextureImageSiblingMtl::onDestroy(const egl::Display *display)
{
    mNativeTexture = nullptr;
}

gl::Format TextureImageSiblingMtl::getFormat() const
{
    return mGLFormat;
}

bool TextureImageSiblingMtl::isRenderable(const gl::Context *context) const
{
    return mRenderable;
}

bool TextureImageSiblingMtl::isTexturable(const gl::Context *context) const
{
    return mTextureable;
}

gl::Extents TextureImageSiblingMtl::getSize() const
{
    return mNativeTexture ? mNativeTexture->sizeAt0() : gl::Extents(0, 0, 0);
}

size_t TextureImageSiblingMtl::getSamples() const
{
    uint32_t samples = mNativeTexture ? mNativeTexture->samples() : 0;
    return samples > 1 ? samples : 0;
}

bool TextureImageSiblingMtl::isYUV() const
{
    // NOTE(hqle): not supporting YUV image yet.
    return false;
}

bool TextureImageSiblingMtl::hasProtectedContent() const
{
    return false;
}

// ImageMtl implementation
ImageMtl::ImageMtl(const egl::ImageState &state, const gl::Context *context) : ImageImpl(state) {}

ImageMtl::~ImageMtl() {}

void ImageMtl::onDestroy(const egl::Display *display)
{
    mNativeTexture = nullptr;
}

egl::Error ImageMtl::initialize(const egl::Display *display)
{
    if (mState.target == EGL_METAL_TEXTURE_ANGLE)
    {
        const TextureImageSiblingMtl *externalImageSibling =
            GetImplAs<TextureImageSiblingMtl>(GetAs<egl::ExternalImageSibling>(mState.source));

        mNativeTexture = externalImageSibling->getTexture();

        switch (mNativeTexture->textureType())
        {
            case MTLTextureType2D:
            case MTLTextureType2DArray:
                mImageTextureType = gl::TextureType::_2D;
                break;
            case MTLTextureTypeCube:
                mImageTextureType = gl::TextureType::CubeMap;
                break;
            default:
                UNREACHABLE();
        }

        mImageLevel = 0;
        mImageLayer = 0;
    }
    else
    {
        UNREACHABLE();
        return egl::EglBadAccess();
    }

    return egl::NoError();
}

angle::Result ImageMtl::orphan(const gl::Context *context, egl::ImageSibling *sibling)
{
    if (sibling == mState.source)
    {
        mNativeTexture = nullptr;
    }

    return angle::Result::Continue;
}

}  // namespace rx
