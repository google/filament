//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// D3DTextureSurfaceWGL.cpp: WGL implementation of egl::Surface for D3D texture interop.

#include "libANGLE/renderer/gl/wgl/D3DTextureSurfaceWGL.h"

#include "libANGLE/Surface.h"
#include "libANGLE/renderer/d3d_format.h"
#include "libANGLE/renderer/dxgi_format_map.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"

namespace rx
{

namespace
{

egl::Error GetD3D11TextureInfo(EGLenum buftype,
                               ID3D11Texture2D *texture11,
                               size_t *width,
                               size_t *height,
                               const angle::Format **angleFormat,
                               IUnknown **object,
                               IUnknown **device)
{
    D3D11_TEXTURE2D_DESC textureDesc;
    texture11->GetDesc(&textureDesc);

    if (buftype == EGL_D3D_TEXTURE_ANGLE)
    {
        // From table egl.restrictions in EGL_ANGLE_d3d_texture_client_buffer.
        switch (textureDesc.Format)
        {
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
                break;

            default:
                SafeRelease(texture11);
                return egl::EglBadParameter()
                       << "Unknown client buffer texture format: " << textureDesc.Format;
        }
    }

    ID3D11Device *d3d11Device = nullptr;
    texture11->GetDevice(&d3d11Device);
    if (d3d11Device == nullptr)
    {
        SafeRelease(texture11);
        return egl::EglBadParameter() << "Could not query the D3D11 device from the client buffer.";
    }

    if (angleFormat)
    {
        *angleFormat = &d3d11_angle::GetFormat(textureDesc.Format);
    }

    if (width)
    {
        *width = textureDesc.Width;
    }
    if (height)
    {
        *height = textureDesc.Height;
    }

    if (device)
    {
        *device = d3d11Device;
    }
    else
    {
        SafeRelease(d3d11Device);
    }

    if (object)
    {
        *object = texture11;
    }
    else
    {
        SafeRelease(texture11);
    }

    return egl::NoError();
}

egl::Error GetD3D9TextureInfo(EGLenum buftype,
                              IDirect3DTexture9 *texture9,
                              size_t *width,
                              size_t *height,
                              const angle::Format **angleFormat,
                              IUnknown **object,
                              IUnknown **device)
{
    D3DSURFACE_DESC surfaceDesc;
    if (FAILED(texture9->GetLevelDesc(0, &surfaceDesc)))
    {
        SafeRelease(texture9);
        return egl::EglBadParameter() << "Could not query description of the D3D9 surface.";
    }

    if (buftype == EGL_D3D_TEXTURE_ANGLE)
    {
        // From table egl.restrictions in EGL_ANGLE_d3d_texture_client_buffer.
        switch (surfaceDesc.Format)
        {
            case D3DFMT_R8G8B8:
            case D3DFMT_A8R8G8B8:
            case D3DFMT_A16B16G16R16F:
            case D3DFMT_A32B32G32R32F:
                break;

            default:
                SafeRelease(texture9);
                return egl::EglBadParameter()
                       << "Unknown client buffer texture format: " << surfaceDesc.Format;
        }
    }

    if (angleFormat)
    {
        const auto &d3dFormatInfo = d3d9::GetD3DFormatInfo(surfaceDesc.Format);
        ASSERT(d3dFormatInfo.info().id != angle::FormatID::NONE);
        *angleFormat = &d3dFormatInfo.info();
    }

    if (width)
    {
        *width = surfaceDesc.Width;
    }
    if (height)
    {
        *height = surfaceDesc.Height;
    }

    IDirect3DDevice9 *d3d9Device = nullptr;
    HRESULT result               = texture9->GetDevice(&d3d9Device);
    if (FAILED(result))
    {
        SafeRelease(texture9);
        return egl::EglBadParameter() << "Could not query the D3D9 device from the client buffer.";
    }

    if (device)
    {
        *device = d3d9Device;
    }
    else
    {
        SafeRelease(d3d9Device);
    }

    if (object)
    {
        *object = texture9;
    }
    else
    {
        SafeRelease(texture9);
    }

    return egl::NoError();
}

egl::Error GetD3DTextureInfo(EGLenum buftype,
                             EGLClientBuffer clientBuffer,
                             ID3D11Device *d3d11Device,
                             ID3D11Device1 *d3d11Device1,
                             size_t *width,
                             size_t *height,
                             const angle::Format **angleFormat,
                             IUnknown **object,
                             IUnknown **device)
{
    if (buftype == EGL_D3D_TEXTURE_ANGLE)
    {
        IUnknown *buffer            = static_cast<IUnknown *>(clientBuffer);
        ID3D11Texture2D *texture11  = nullptr;
        IDirect3DTexture9 *texture9 = nullptr;
        if (SUCCEEDED(buffer->QueryInterface<ID3D11Texture2D>(&texture11)))
        {
            return GetD3D11TextureInfo(buftype, texture11, width, height, angleFormat, object,
                                       device);
        }
        else if (SUCCEEDED(buffer->QueryInterface<IDirect3DTexture9>(&texture9)))
        {
            return GetD3D9TextureInfo(buftype, texture9, width, height, angleFormat, object,
                                      device);
        }
        else
        {
            return egl::EglBadParameter()
                   << "Provided buffer is not a IDirect3DTexture9 or ID3D11Texture2D.";
        }
    }
    else if (buftype == EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE)
    {
        ASSERT(d3d11Device);
        HANDLE shareHandle         = static_cast<HANDLE>(clientBuffer);
        ID3D11Texture2D *texture11 = nullptr;
        HRESULT result = d3d11Device->OpenSharedResource(shareHandle, __uuidof(ID3D11Texture2D),
                                                         reinterpret_cast<void **>(&texture11));
        if (FAILED(result) && d3d11Device1)
        {
            result = d3d11Device1->OpenSharedResource1(shareHandle, __uuidof(ID3D11Texture2D),
                                                       reinterpret_cast<void **>(&texture11));
        }

        if (FAILED(result))
        {
            return egl::EglBadParameter() << "Failed to open share handle, " << gl::FmtHR(result);
        }

        return GetD3D11TextureInfo(buftype, texture11, width, height, angleFormat, object, device);
    }
    else
    {
        UNREACHABLE();
        return egl::EglBadDisplay() << "Unknown buftype for D3DTextureSurfaceWGL.";
    }
}

}  // anonymous namespace

D3DTextureSurfaceWGL::D3DTextureSurfaceWGL(const egl::SurfaceState &state,
                                           StateManagerGL *stateManager,
                                           EGLenum buftype,
                                           EGLClientBuffer clientBuffer,
                                           DisplayWGL *display,
                                           HDC deviceContext,
                                           ID3D11Device *displayD3D11Device,
                                           ID3D11Device1 *displayD3D11Device1,
                                           const FunctionsGL *functionsGL,
                                           const FunctionsWGL *functionsWGL)
    : SurfaceWGL(state),
      mBuftype(buftype),
      mClientBuffer(clientBuffer),
      mDisplayD3D11Device(displayD3D11Device),
      mDisplayD3D11Device1(displayD3D11Device1),
      mDisplay(display),
      mStateManager(stateManager),
      mFunctionsGL(functionsGL),
      mFunctionsWGL(functionsWGL),
      mDeviceContext(deviceContext),
      mWidth(0),
      mHeight(0),
      mColorFormat(nullptr),
      mDeviceHandle(nullptr),
      mObject(nullptr),
      mKeyedMutex(nullptr),
      mBoundObjectTextureHandle(nullptr),
      mBoundObjectRenderbufferHandle(nullptr),
      mFramebufferID(0),
      mColorRenderbufferID(0),
      mDepthStencilRenderbufferID(0)
{}

D3DTextureSurfaceWGL::~D3DTextureSurfaceWGL()
{
    ASSERT(mBoundObjectTextureHandle == nullptr);

    SafeRelease(mObject);
    SafeRelease(mKeyedMutex);

    if (mDeviceHandle)
    {
        if (mBoundObjectRenderbufferHandle)
        {
            mFunctionsWGL->dxUnregisterObjectNV(mDeviceHandle, mBoundObjectRenderbufferHandle);
            mBoundObjectRenderbufferHandle = nullptr;
        }
        mStateManager->deleteRenderbuffer(mColorRenderbufferID);
        mStateManager->deleteRenderbuffer(mDepthStencilRenderbufferID);
        mStateManager->deleteFramebuffer(mFramebufferID);

        if (mBoundObjectTextureHandle)
        {
            mFunctionsWGL->dxUnregisterObjectNV(mDeviceHandle, mBoundObjectTextureHandle);
            mBoundObjectTextureHandle = nullptr;
        }

        mDisplay->releaseD3DDevice(mDeviceHandle);
        mDeviceHandle = nullptr;
    }
}

egl::Error D3DTextureSurfaceWGL::ValidateD3DTextureClientBuffer(EGLenum buftype,
                                                                EGLClientBuffer clientBuffer,
                                                                ID3D11Device *d3d11Device,
                                                                ID3D11Device1 *d3d11Device1)
{
    return GetD3DTextureInfo(buftype, clientBuffer, d3d11Device, d3d11Device1, nullptr, nullptr,
                             nullptr, nullptr, nullptr);
}

egl::Error D3DTextureSurfaceWGL::initialize(const egl::Display *display)
{
    IUnknown *device = nullptr;
    ANGLE_TRY(GetD3DTextureInfo(mBuftype, mClientBuffer, mDisplayD3D11Device, mDisplayD3D11Device1,
                                &mWidth, &mHeight, &mColorFormat, &mObject, &device));

    if (mColorFormat)
    {
        if (mState.attributes.contains(EGL_GL_COLORSPACE))
        {
            if (mColorFormat->id != angle::FormatID::R8G8B8A8_TYPELESS &&
                mColorFormat->id != angle::FormatID::B8G8R8A8_TYPELESS)
            {
                return egl::EglBadMatch()
                       << "EGL_GL_COLORSPACE may only be specified for TYPELESS textures";
            }
        }
    }

    // Grab the keyed mutex, if one exists
    mObject->QueryInterface(&mKeyedMutex);

    ASSERT(device != nullptr);
    egl::Error error = mDisplay->registerD3DDevice(device, &mDeviceHandle);
    SafeRelease(device);
    if (error.isError())
    {
        return error;
    }

    mFunctionsGL->genRenderbuffers(1, &mColorRenderbufferID);
    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mColorRenderbufferID);
    mBoundObjectRenderbufferHandle = mFunctionsWGL->dxRegisterObjectNV(
        mDeviceHandle, mObject, mColorRenderbufferID, GL_RENDERBUFFER, WGL_ACCESS_READ_WRITE_NV);
    if (mBoundObjectRenderbufferHandle == nullptr)
    {
        return egl::EglBadAlloc() << "Failed to register D3D object, "
                                  << gl::FmtErr(HRESULT_CODE(GetLastError()));
    }

    const egl::Config *config = mState.config;
    if (config->depthStencilFormat != GL_NONE)
    {
        mFunctionsGL->genRenderbuffers(1, &mDepthStencilRenderbufferID);
        mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mDepthStencilRenderbufferID);
        mFunctionsGL->renderbufferStorage(GL_RENDERBUFFER, config->depthStencilFormat,
                                          static_cast<GLsizei>(mWidth),
                                          static_cast<GLsizei>(mHeight));
    }

    return egl::NoError();
}

egl::Error D3DTextureSurfaceWGL::makeCurrent(const gl::Context *context)
{
    if (!mFunctionsWGL->dxLockObjectsNV(mDeviceHandle, 1, &mBoundObjectRenderbufferHandle))
    {
        DWORD error = GetLastError();
        return egl::EglBadAlloc() << "Failed to lock object, " << gl::FmtErr(HRESULT_CODE(error));
    }

    return egl::NoError();
}

egl::Error D3DTextureSurfaceWGL::unMakeCurrent(const gl::Context *context)
{
    if (!mFunctionsWGL->dxUnlockObjectsNV(mDeviceHandle, 1, &mBoundObjectRenderbufferHandle))
    {
        DWORD error = GetLastError();
        return egl::EglBadAlloc() << "Failed to unlock object, " << gl::FmtErr(HRESULT_CODE(error));
    }

    return egl::NoError();
}

egl::Error D3DTextureSurfaceWGL::swap(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error D3DTextureSurfaceWGL::postSubBuffer(const gl::Context *context,
                                               EGLint x,
                                               EGLint y,
                                               EGLint width,
                                               EGLint height)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error D3DTextureSurfaceWGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    switch (attribute)
    {
        case EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE:
            *value = (mBuftype == EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE) ? mClientBuffer : nullptr;
            break;

        case EGL_DXGI_KEYED_MUTEX_ANGLE:
            *value = mKeyedMutex;
            break;

        default:
            UNREACHABLE();
    }

    return egl::NoError();
}

egl::Error D3DTextureSurfaceWGL::bindTexImage(const gl::Context *context,
                                              gl::Texture *texture,
                                              EGLint buffer)
{
    ASSERT(mBoundObjectTextureHandle == nullptr);

    const TextureGL *textureGL = GetImplAs<TextureGL>(texture);
    GLuint textureID           = textureGL->getTextureID();

    mBoundObjectTextureHandle = mFunctionsWGL->dxRegisterObjectNV(
        mDeviceHandle, mObject, textureID, GL_TEXTURE_2D, WGL_ACCESS_READ_WRITE_NV);
    if (mBoundObjectTextureHandle == nullptr)
    {
        DWORD error = GetLastError();
        return egl::EglBadAlloc() << "Failed to register D3D object, "
                                  << gl::FmtErr(HRESULT_CODE(error));
    }

    if (!mFunctionsWGL->dxLockObjectsNV(mDeviceHandle, 1, &mBoundObjectTextureHandle))
    {
        DWORD error = GetLastError();
        return egl::EglBadAlloc() << "Failed to lock object, " << gl::FmtErr(HRESULT_CODE(error));
    }

    return egl::NoError();
}

egl::Error D3DTextureSurfaceWGL::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    ASSERT(mBoundObjectTextureHandle != nullptr);
    if (!mFunctionsWGL->dxUnlockObjectsNV(mDeviceHandle, 1, &mBoundObjectTextureHandle))
    {
        DWORD error = GetLastError();
        return egl::EglBadAlloc() << "Failed to unlock object, " << gl::FmtErr(HRESULT_CODE(error));
    }

    if (!mFunctionsWGL->dxUnregisterObjectNV(mDeviceHandle, mBoundObjectTextureHandle))
    {
        DWORD error = GetLastError();
        return egl::EglBadAlloc() << "Failed to unregister D3D object, "
                                  << gl::FmtErr(HRESULT_CODE(error));
    }
    mBoundObjectTextureHandle = nullptr;

    return egl::NoError();
}

void D3DTextureSurfaceWGL::setSwapInterval(const egl::Display *display, EGLint interval)
{
    UNIMPLEMENTED();
}

EGLint D3DTextureSurfaceWGL::getWidth() const
{
    return static_cast<EGLint>(mWidth);
}

EGLint D3DTextureSurfaceWGL::getHeight() const
{
    return static_cast<EGLint>(mHeight);
}

EGLint D3DTextureSurfaceWGL::isPostSubBufferSupported() const
{
    return EGL_FALSE;
}

EGLint D3DTextureSurfaceWGL::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

HDC D3DTextureSurfaceWGL::getDC() const
{
    return mDeviceContext;
}

const angle::Format *D3DTextureSurfaceWGL::getD3DTextureColorFormat() const
{
    return mColorFormat;
}

egl::Error D3DTextureSurfaceWGL::attachToFramebuffer(const gl::Context *context,
                                                     gl::Framebuffer *framebuffer)
{
    FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
    ASSERT(framebufferGL->getFramebufferID() == 0);
    if (mFramebufferID == 0)
    {
        GLuint framebufferID = 0;
        mFunctionsGL->genFramebuffers(1, &framebufferID);
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, framebufferID);
        mFunctionsGL->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                              mColorRenderbufferID);
        if (mState.config->depthSize > 0)
        {
            mFunctionsGL->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                  GL_RENDERBUFFER, mDepthStencilRenderbufferID);
        }
        if (mState.config->stencilSize > 0)
        {
            mFunctionsGL->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                                  GL_RENDERBUFFER, mDepthStencilRenderbufferID);
        }
        mFramebufferID = framebufferID;
    }
    framebufferGL->setFramebufferID(mFramebufferID);
    return egl::NoError();
}

egl::Error D3DTextureSurfaceWGL::detachFromFramebuffer(const gl::Context *context,
                                                       gl::Framebuffer *framebuffer)
{
    FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
    ASSERT(framebufferGL->getFramebufferID() == mFramebufferID);
    framebufferGL->setFramebufferID(0);
    return egl::NoError();
}

}  // namespace rx
