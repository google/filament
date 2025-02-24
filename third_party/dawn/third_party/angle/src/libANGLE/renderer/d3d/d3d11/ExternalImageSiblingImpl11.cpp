//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/d3d/d3d11/ExternalImageSiblingImpl11.h"

#include "libANGLE/Context.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"

namespace rx
{
ExternalImageSiblingImpl11::ExternalImageSiblingImpl11(Renderer11 *renderer,
                                                       EGLClientBuffer buffer,
                                                       const egl::AttributeMap &attribs)
    : mRenderer(renderer), mBuffer(buffer), mAttribs(attribs)
{}

ExternalImageSiblingImpl11::~ExternalImageSiblingImpl11() {}

egl::Error ExternalImageSiblingImpl11::initialize(const egl::Display *display)
{
    const angle::Format *angleFormat = nullptr;
    ANGLE_TRY(mRenderer->getD3DTextureInfo(nullptr, static_cast<IUnknown *>(mBuffer), mAttribs,
                                           &mWidth, &mHeight, &mSamples, &mFormat, &angleFormat,
                                           &mArraySlice));
    ID3D11Texture2D *texture =
        d3d11::DynamicCastComObject<ID3D11Texture2D>(static_cast<IUnknown *>(mBuffer));
    ASSERT(texture != nullptr);

    D3D11_TEXTURE2D_DESC textureDesc = {};
    texture->GetDesc(&textureDesc);

    if (d3d11::IsSupportedMultiplanarFormat(textureDesc.Format))
    {
        if (!mAttribs.contains(EGL_D3D11_TEXTURE_PLANE_ANGLE))
        {
            return egl::EglBadParameter()
                   << "EGL_D3D11_TEXTURE_PLANE_ANGLE must be specified for YUV textures.";
        }

        EGLint plane = mAttribs.getAsInt(EGL_D3D11_TEXTURE_PLANE_ANGLE);
        if (plane < 0 || plane > 1)
        {
            return egl::EglBadParameter() << "Invalid client buffer texture plane: " << plane;
        }

        mTexture.set(texture, d3d11::GetYUVPlaneFormat(textureDesc.Format, plane));
    }
    else
    {
        // TextureHelper11 will release texture on destruction.
        mTexture.set(texture, d3d11::Format::Get(angleFormat->glInternalFormat,
                                                 mRenderer->getRenderer11DeviceCaps()));
    }

    IDXGIResource *resource = d3d11::DynamicCastComObject<IDXGIResource>(mTexture.get());
    ASSERT(resource != nullptr);
    DXGI_USAGE resourceUsage = 0;
    resource->GetUsage(&resourceUsage);
    SafeRelease(resource);

    mIsRenderable = (textureDesc.BindFlags & D3D11_BIND_RENDER_TARGET) &&
                    (resourceUsage & DXGI_USAGE_RENDER_TARGET_OUTPUT) &&
                    !(resourceUsage & DXGI_USAGE_READ_ONLY);

    mIsTexturable = (textureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE) &&
                    (resourceUsage & DXGI_USAGE_SHADER_INPUT);

    mIsTextureArray = (textureDesc.ArraySize > 1);

    return egl::NoError();
}

gl::Format ExternalImageSiblingImpl11::getFormat() const
{
    return mFormat;
}

bool ExternalImageSiblingImpl11::isRenderable(const gl::Context *context) const
{
    return mIsRenderable;
}

bool ExternalImageSiblingImpl11::isTexturable(const gl::Context *context) const
{
    return mIsTexturable;
}

bool ExternalImageSiblingImpl11::isYUV() const
{
    return false;
}

bool ExternalImageSiblingImpl11::hasProtectedContent() const
{
    return false;
}

gl::Extents ExternalImageSiblingImpl11::getSize() const
{
    return gl::Extents(mWidth, mHeight, 1);
}

size_t ExternalImageSiblingImpl11::getSamples() const
{
    return mSamples;
}

angle::Result ExternalImageSiblingImpl11::getAttachmentRenderTarget(
    const gl::Context *context,
    GLenum binding,
    const gl::ImageIndex &imageIndex,
    GLsizei samples,
    FramebufferAttachmentRenderTarget **rtOut)
{
    ANGLE_TRY(createRenderTarget(context));
    *rtOut = mRenderTarget.get();
    return angle::Result::Continue;
}

angle::Result ExternalImageSiblingImpl11::initializeContents(const gl::Context *context,
                                                             GLenum binding,
                                                             const gl::ImageIndex &imageIndex)
{
    UNREACHABLE();
    return angle::Result::Stop;
}

angle::Result ExternalImageSiblingImpl11::createRenderTarget(const gl::Context *context)
{
    if (mRenderTarget)
        return angle::Result::Continue;

    Context11 *context11            = GetImplAs<Context11>(context);
    const d3d11::Format &formatInfo = mTexture.getFormatSet();

    d3d11::RenderTargetView rtv;
    if (mIsRenderable)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = formatInfo.rtvFormat;
        if (mIsTextureArray)
        {
            if (mSamples == 0)
            {
                rtvDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.MipSlice        = 0;
                rtvDesc.Texture2DArray.FirstArraySlice = mArraySlice;
                rtvDesc.Texture2DArray.ArraySize       = 1;
            }
            else
            {
                rtvDesc.ViewDimension                    = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
                rtvDesc.Texture2DMSArray.FirstArraySlice = mArraySlice;
                rtvDesc.Texture2DMSArray.ArraySize       = 1;
            }
        }
        else
        {
            if (mSamples == 0)
            {
                rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
            }
            else
            {
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            }
        }

        ANGLE_TRY(mRenderer->allocateResource(context11, rtvDesc, mTexture.get(), &rtv));
        rtv.setInternalName("getAttachmentRenderTarget.RTV");
    }

    d3d11::SharedSRV srv;
    if (mIsTexturable)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = formatInfo.srvFormat;
        if (mIsTextureArray)
        {
            if (mSamples == 0)
            {
                srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.MostDetailedMip = 0;
                srvDesc.Texture2DArray.MipLevels       = 1;
                srvDesc.Texture2DArray.FirstArraySlice = mArraySlice;
                srvDesc.Texture2DArray.ArraySize       = 1;
            }
            else
            {
                srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
                srvDesc.Texture2DArray.FirstArraySlice = mArraySlice;
                srvDesc.Texture2DArray.ArraySize       = 1;
            }
        }
        else
        {
            if (mSamples == 0)
            {
                srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.MipLevels       = 1;
            }
            else
            {
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
            }
        }

        ANGLE_TRY(mRenderer->allocateResource(context11, srvDesc, mTexture.get(), &srv));
        srv.setInternalName("getAttachmentRenderTarget.SRV");
    }
    d3d11::SharedSRV blitSrv = srv.makeCopy();

    mRenderTarget = std::make_unique<TextureRenderTarget11>(
        std::move(rtv), mTexture, std::move(srv), std::move(blitSrv), mFormat.info->internalFormat,
        formatInfo, mWidth, mHeight, 1, mSamples);
    return angle::Result::Continue;
}

}  // namespace rx
