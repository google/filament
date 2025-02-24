//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StreamProducerD3DTexture.cpp: Implements the stream producer for D3D11 textures

#include "libANGLE/renderer/d3d/d3d11/StreamProducerD3DTexture.h"

#include "common/utilities.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

#include <array>

namespace rx
{

namespace
{

egl::Error GetGLDescFromTex(ID3D11Texture2D *const tex,
                            const UINT planeIndex,
                            egl::Stream::GLTextureDescription *const out)
{
    if (!tex)
        return egl::EglBadParameter() << "Texture is null";

    D3D11_TEXTURE2D_DESC desc;
    tex->GetDesc(&desc);

    if (desc.Width < 1 || desc.Height < 1)
        return egl::EglBadParameter() << "Width or height < 1";

    out->width     = desc.Width;
    out->height    = desc.Height;
    out->mipLevels = 0;

    std::array<uint32_t, 2> planeFormats = {};
    switch (desc.Format)
    {
        case DXGI_FORMAT_NV12:
            planeFormats = {GL_R8, GL_RG8};
            break;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            planeFormats = {GL_R16_EXT, GL_RG16_EXT};
            break;

        case DXGI_FORMAT_R8_UNORM:
            planeFormats = {GL_R8};
            break;
        case DXGI_FORMAT_R8G8_UNORM:
            planeFormats[0] = GL_RG8;
            break;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            planeFormats[0] = GL_RGBA8;
            break;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            planeFormats[0] = GL_BGRA8_EXT;
            break;

        case DXGI_FORMAT_R16_UNORM:
            planeFormats[0] = GL_R16_EXT;
            break;
        case DXGI_FORMAT_R16G16_UNORM:
            planeFormats[0] = GL_RG16_EXT;
            break;
        case DXGI_FORMAT_R16G16B16A16_UNORM:
            planeFormats[0] = GL_RGBA16_EXT;
            break;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
            planeFormats[0] = GL_RGBA16F;
            break;

        default:
            return egl::EglBadParameter() << "Unsupported format";
    }

    if (planeFormats[1])  // If we have YUV planes, expect 4:2:0.
    {
        if ((desc.Width % 2) != 0 || (desc.Height % 2) != 0)
            return egl::EglBadParameter() << "YUV 4:2:0 textures must have even width and height.";
    }
    if (planeIndex > 0)
    {
        out->width /= 2;
        out->height /= 2;
    }

    out->internalFormat = 0;
    if (planeIndex < planeFormats.size())
    {
        out->internalFormat = planeFormats[planeIndex];
    }
    if (!out->internalFormat)
        return egl::EglBadParameter() << "Plane out of range";

    return egl::NoError();
}

}  // namespace

StreamProducerD3DTexture::StreamProducerD3DTexture(Renderer11 *renderer)
    : mRenderer(renderer), mTexture(nullptr), mArraySlice(0), mPlaneOffset(0)
{}

StreamProducerD3DTexture::~StreamProducerD3DTexture()
{
    SafeRelease(mTexture);
}

egl::Error StreamProducerD3DTexture::validateD3DTexture(const void *pointer,
                                                        const egl::AttributeMap &attributes) const
{
    // We must remove the const qualifier because "GetDevice" and "GetDesc" are non-const in D3D11.
    ID3D11Texture2D *textureD3D = static_cast<ID3D11Texture2D *>(const_cast<void *>(pointer));

    // Check that the texture originated from our device
    angle::ComPtr<ID3D11Device> device;
    textureD3D->GetDevice(&device);
    if (device.Get() != mRenderer->getDevice())
    {
        return egl::EglBadParameter() << "Texture not created on ANGLE D3D device";
    }

    const auto planeId = static_cast<UINT>(attributes.get(EGL_NATIVE_BUFFER_PLANE_OFFSET_IMG, 0));
    egl::Stream::GLTextureDescription unused;
    return GetGLDescFromTex(textureD3D, planeId, &unused);
}

void StreamProducerD3DTexture::postD3DTexture(void *pointer, const egl::AttributeMap &attributes)
{
    ASSERT(pointer != nullptr);
    ID3D11Texture2D *textureD3D = static_cast<ID3D11Texture2D *>(pointer);

    // Release the previous texture if there is one
    SafeRelease(mTexture);

    mTexture = textureD3D;
    mTexture->AddRef();
    mPlaneOffset = static_cast<UINT>(attributes.get(EGL_NATIVE_BUFFER_PLANE_OFFSET_IMG, 0));
    mArraySlice  = static_cast<UINT>(attributes.get(EGL_D3D_TEXTURE_SUBRESOURCE_ID_ANGLE, 0));
}

egl::Stream::GLTextureDescription StreamProducerD3DTexture::getGLFrameDescription(int planeIndex)
{
    const auto planeOffsetIndex = static_cast<UINT>(planeIndex + mPlaneOffset);
    egl::Stream::GLTextureDescription ret;
    ANGLE_SWALLOW_ERR(GetGLDescFromTex(mTexture, planeOffsetIndex, &ret));
    return ret;
}

ID3D11Texture2D *StreamProducerD3DTexture::getD3DTexture()
{
    return mTexture;
}

UINT StreamProducerD3DTexture::getArraySlice()
{
    return mArraySlice;
}

}  // namespace rx
