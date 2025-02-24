//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DXGISwapChainWindowSurfaceWGL.cpp: WGL implementation of egl::Surface for windows using a DXGI
// swapchain.

#include "libANGLE/renderer/gl/wgl/DXGISwapChainWindowSurfaceWGL.h"

#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"

#include <EGL/eglext.h>

namespace rx
{

DXGISwapChainWindowSurfaceWGL::DXGISwapChainWindowSurfaceWGL(const egl::SurfaceState &state,
                                                             StateManagerGL *stateManager,
                                                             EGLNativeWindowType window,
                                                             ID3D11Device *device,
                                                             HANDLE deviceHandle,
                                                             HDC deviceContext,
                                                             const FunctionsGL *functionsGL,
                                                             const FunctionsWGL *functionsWGL,
                                                             EGLint orientation)
    : SurfaceWGL(state),
      mWindow(window),
      mStateManager(stateManager),
      mFunctionsGL(functionsGL),
      mFunctionsWGL(functionsWGL),
      mDevice(device),
      mDeviceHandle(deviceHandle),
      mWGLDevice(deviceContext),
      mSwapChainFormat(DXGI_FORMAT_UNKNOWN),
      mSwapChainFlags(0),
      mDepthBufferFormat(GL_NONE),
      mFirstSwap(true),
      mSwapChain(nullptr),
      mSwapChain1(nullptr),
      mFramebufferID(0),
      mColorRenderbufferID(0),
      mRenderbufferBufferHandle(nullptr),
      mDepthRenderbufferID(0),
      mTextureID(0),
      mTextureHandle(nullptr),
      mWidth(0),
      mHeight(0),
      mSwapInterval(1),
      mOrientation(orientation)
{}

DXGISwapChainWindowSurfaceWGL::~DXGISwapChainWindowSurfaceWGL()
{
    if (mRenderbufferBufferHandle != nullptr)
    {
        mFunctionsWGL->dxUnlockObjectsNV(mDeviceHandle, 1, &mRenderbufferBufferHandle);
        mFunctionsWGL->dxUnregisterObjectNV(mDeviceHandle, mRenderbufferBufferHandle);
    }

    if (mFramebufferID != 0)
    {
        mStateManager->deleteFramebuffer(mFramebufferID);
        mFramebufferID = 0;
    }

    if (mColorRenderbufferID != 0)
    {
        mStateManager->deleteRenderbuffer(mColorRenderbufferID);
        mColorRenderbufferID = 0;
    }

    if (mDepthRenderbufferID != 0)
    {
        mStateManager->deleteRenderbuffer(mDepthRenderbufferID);
        mDepthRenderbufferID = 0;
    }

    SafeRelease(mSwapChain);
    SafeRelease(mSwapChain1);
}

egl::Error DXGISwapChainWindowSurfaceWGL::initialize(const egl::Display *display)
{
    if (mOrientation != EGL_SURFACE_ORIENTATION_INVERT_Y_ANGLE)
    {
        // TODO(geofflang): Support the orientation extensions fully.  Currently only inverting Y is
        // supported.  To support all orientations, an intermediate framebuffer will be needed with
        // a blit before swap.
        return egl::EglBadAttribute() << "DXGISwapChainWindowSurfaceWGL requires an orientation of "
                                         "EGL_SURFACE_ORIENTATION_INVERT_Y_ANGLE.";
    }

    RECT rect;
    if (!GetClientRect(mWindow, &rect))
    {
        return egl::EglBadNativeWindow() << "Failed to query the window size.";
    }
    mWidth  = rect.right - rect.left;
    mHeight = rect.bottom - rect.top;

    mSwapChainFormat   = DXGI_FORMAT_R8G8B8A8_UNORM;
    mSwapChainFlags    = 0;
    mDepthBufferFormat = GL_DEPTH24_STENCIL8;

    mFunctionsGL->genRenderbuffers(1, &mColorRenderbufferID);
    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mColorRenderbufferID);

    mFunctionsGL->genRenderbuffers(1, &mDepthRenderbufferID);
    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbufferID);

    return createSwapChain();
}

egl::Error DXGISwapChainWindowSurfaceWGL::makeCurrent(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error DXGISwapChainWindowSurfaceWGL::swap(const gl::Context *context)
{
    mFunctionsGL->flush();

    ANGLE_TRY(setObjectsLocked(false));

    HRESULT result = mSwapChain->Present(mSwapInterval, 0);
    mFirstSwap     = false;

    ANGLE_TRY(setObjectsLocked(true));

    if (FAILED(result))
    {
        return egl::EglBadAlloc() << "Failed to present swap chain, " << gl::FmtHR(result);
    }

    return checkForResize();
}

egl::Error DXGISwapChainWindowSurfaceWGL::postSubBuffer(const gl::Context *context,
                                                        EGLint x,
                                                        EGLint y,
                                                        EGLint width,
                                                        EGLint height)
{
    ASSERT(width > 0 && height > 0);
    ASSERT(mSwapChain1 != nullptr);

    mFunctionsGL->flush();

    ANGLE_TRY(setObjectsLocked(false));

    HRESULT result = S_OK;
    if (mFirstSwap)
    {
        result     = mSwapChain1->Present(mSwapInterval, 0);
        mFirstSwap = false;
    }
    else
    {
        RECT rect = {static_cast<LONG>(x), static_cast<LONG>(mHeight - y - height),
                     static_cast<LONG>(x + width), static_cast<LONG>(mHeight - y)};
        DXGI_PRESENT_PARAMETERS params = {1, &rect, nullptr, nullptr};
        result                         = mSwapChain1->Present1(mSwapInterval, 0, &params);
    }

    ANGLE_TRY(setObjectsLocked(true));

    if (FAILED(result))
    {
        return egl::EglBadAlloc() << "Failed to present swap chain, " << gl::FmtHR(result);
    }

    return checkForResize();
}

egl::Error DXGISwapChainWindowSurfaceWGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error DXGISwapChainWindowSurfaceWGL::bindTexImage(const gl::Context *context,
                                                       gl::Texture *texture,
                                                       EGLint buffer)
{
    ASSERT(mTextureHandle == nullptr);

    const TextureGL *textureGL = GetImplAs<TextureGL>(texture);
    GLuint textureID           = textureGL->getTextureID();

    ID3D11Texture2D *colorBuffer = nullptr;
    HRESULT result               = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                                         reinterpret_cast<void **>(&colorBuffer));
    if (FAILED(result))
    {
        return egl::EglBadAlloc() << "Failed to query texture from swap chain, "
                                  << gl::FmtHR(result);
    }

    mTextureHandle = mFunctionsWGL->dxRegisterObjectNV(mDeviceHandle, colorBuffer, textureID,
                                                       GL_TEXTURE_2D, WGL_ACCESS_READ_WRITE_NV);
    SafeRelease(colorBuffer);
    if (mTextureHandle == nullptr)
    {
        return egl::EglBadAlloc() << "Failed to register D3D object, "
                                  << gl::FmtErr(HRESULT_CODE(GetLastError()));
    }

    if (!mFunctionsWGL->dxLockObjectsNV(mDeviceHandle, 1, &mTextureHandle))
    {
        mFunctionsWGL->dxUnregisterObjectNV(mDeviceHandle, mTextureHandle);
        mTextureHandle = nullptr;

        return egl::EglBadAlloc() << "Failed to lock D3D object, "
                                  << gl::FmtErr(HRESULT_CODE(GetLastError()));
    }

    mTextureID = textureID;

    return egl::NoError();
}

egl::Error DXGISwapChainWindowSurfaceWGL::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    ASSERT(mTextureHandle != nullptr);

    if (!mFunctionsWGL->dxUnlockObjectsNV(mDeviceHandle, 1, &mTextureHandle))
    {
        return egl::EglBadAlloc() << "Failed to unlock D3D object, "
                                  << gl::FmtErr(HRESULT_CODE(GetLastError()));
    }

    if (!mFunctionsWGL->dxUnregisterObjectNV(mDeviceHandle, mTextureHandle))
    {
        return egl::EglBadAlloc() << "Failed to unregister D3D object, "
                                  << gl::FmtErr(HRESULT_CODE(GetLastError()));
    }

    mTextureID     = 0;
    mTextureHandle = nullptr;

    return egl::NoError();
}

void DXGISwapChainWindowSurfaceWGL::setSwapInterval(const egl::Display *display, EGLint interval)
{
    mSwapInterval = interval;
}

EGLint DXGISwapChainWindowSurfaceWGL::getWidth() const
{
    return static_cast<EGLint>(mWidth);
}

EGLint DXGISwapChainWindowSurfaceWGL::getHeight() const
{
    return static_cast<EGLint>(mHeight);
}

EGLint DXGISwapChainWindowSurfaceWGL::isPostSubBufferSupported() const
{
    return mSwapChain1 != nullptr;
}

EGLint DXGISwapChainWindowSurfaceWGL::getSwapBehavior() const
{
    return EGL_BUFFER_DESTROYED;
}

HDC DXGISwapChainWindowSurfaceWGL::getDC() const
{
    return mWGLDevice;
}

egl::Error DXGISwapChainWindowSurfaceWGL::attachToFramebuffer(const gl::Context *context,
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

        if (mDepthBufferFormat != GL_NONE)
        {
            const gl::InternalFormat &depthStencilFormatInfo =
                gl::GetSizedInternalFormatInfo(mDepthBufferFormat);
            if (depthStencilFormatInfo.depthBits > 0)
            {
                mFunctionsGL->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                                      GL_RENDERBUFFER, mDepthRenderbufferID);
            }
            if (depthStencilFormatInfo.stencilBits > 0)
            {
                mFunctionsGL->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                                      GL_RENDERBUFFER, mDepthRenderbufferID);
            }
        }

        mFramebufferID = framebufferID;
    }
    framebufferGL->setFramebufferID(mFramebufferID);
    return egl::NoError();
}

egl::Error DXGISwapChainWindowSurfaceWGL::detachFromFramebuffer(const gl::Context *context,
                                                                gl::Framebuffer *framebuffer)
{
    FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
    ASSERT(framebufferGL->getFramebufferID() == mFramebufferID);
    framebufferGL->setFramebufferID(0);
    return egl::NoError();
}

egl::Error DXGISwapChainWindowSurfaceWGL::setObjectsLocked(bool locked)
{
    if (mRenderbufferBufferHandle == nullptr)
    {
        ASSERT(mTextureHandle == nullptr);
        return egl::NoError();
    }

    HANDLE resources[] = {
        mRenderbufferBufferHandle,
        mTextureHandle,
    };
    GLint count = (mTextureHandle != nullptr) ? 2 : 1;

    if (locked)
    {
        if (!mFunctionsWGL->dxLockObjectsNV(mDeviceHandle, count, resources))
        {
            return egl::EglBadAlloc()
                   << "Failed to lock object, " << gl::FmtErr(HRESULT_CODE(GetLastError()));
        }
    }
    else
    {
        if (!mFunctionsWGL->dxUnlockObjectsNV(mDeviceHandle, count, resources))
        {
            return egl::EglBadAlloc()
                   << "Failed to lock object, " << gl::FmtErr(HRESULT_CODE(GetLastError()));
        }
    }

    return egl::NoError();
}

egl::Error DXGISwapChainWindowSurfaceWGL::checkForResize()
{
    RECT rect;
    if (!GetClientRect(mWindow, &rect))
    {
        return egl::EglBadNativeWindow() << "Failed to query the window size.";
    }

    size_t newWidth  = rect.right - rect.left;
    size_t newHeight = rect.bottom - rect.top;
    if (newWidth != mWidth || newHeight != mHeight)
    {
        mWidth  = newWidth;
        mHeight = newHeight;

        // TODO(geofflang): Handle resize by resizing the swap chain instead of re-creating it.
        egl::Error error = createSwapChain();
        if (error.isError())
        {
            return error;
        }
    }

    return egl::NoError();
}

static IDXGIFactory *GetDXGIFactoryFromDevice(ID3D11Device *device)
{
    IDXGIDevice *dxgiDevice = nullptr;
    HRESULT result =
        device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgiDevice));
    if (FAILED(result))
    {
        return nullptr;
    }

    IDXGIAdapter *dxgiAdapter = nullptr;
    result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void **>(&dxgiAdapter));
    SafeRelease(dxgiDevice);
    if (FAILED(result))
    {
        return nullptr;
    }

    IDXGIFactory *dxgiFactory = nullptr;
    result =
        dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void **>(&dxgiFactory));
    SafeRelease(dxgiAdapter);
    if (FAILED(result))
    {
        return nullptr;
    }

    return dxgiFactory;
}

egl::Error DXGISwapChainWindowSurfaceWGL::createSwapChain()
{
    egl::Error error = setObjectsLocked(false);
    if (error.isError())
    {
        return error;
    }

    if (mRenderbufferBufferHandle)
    {
        mFunctionsWGL->dxUnregisterObjectNV(mDeviceHandle, mRenderbufferBufferHandle);
        mRenderbufferBufferHandle = nullptr;
    }

    // If this surface is bound to a texture, unregister it.
    bool hadBoundSurface = (mTextureHandle != nullptr);
    if (hadBoundSurface)
    {
        mFunctionsWGL->dxUnregisterObjectNV(mDeviceHandle, mTextureHandle);
        mTextureHandle = nullptr;
    }

    IDXGIFactory *dxgiFactory = GetDXGIFactoryFromDevice(mDevice);
    if (dxgiFactory == nullptr)
    {
        return egl::EglBadNativeWindow() << "Failed to query the DXGIFactory.";
    }

    IDXGIFactory2 *dxgiFactory2 = nullptr;
    HRESULT result              = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2),
                                                              reinterpret_cast<void **>(&dxgiFactory2));
    if (SUCCEEDED(result))
    {
        ASSERT(dxgiFactory2 != nullptr);

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount           = 1;
        swapChainDesc.Format                = mSwapChainFormat;
        swapChainDesc.Width                 = static_cast<UINT>(mWidth);
        swapChainDesc.Height                = static_cast<UINT>(mHeight);
        swapChainDesc.Format                = mSwapChainFormat;
        swapChainDesc.Stereo                = FALSE;
        swapChainDesc.SampleDesc.Count      = 1;
        swapChainDesc.SampleDesc.Quality    = 0;
        swapChainDesc.BufferUsage =
            DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_BACK_BUFFER;
        swapChainDesc.BufferCount = 1;
        swapChainDesc.Scaling     = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect  = DXGI_SWAP_EFFECT_SEQUENTIAL;
        swapChainDesc.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags       = mSwapChainFlags;

        result = dxgiFactory2->CreateSwapChainForHwnd(mDevice, mWindow, &swapChainDesc, nullptr,
                                                      nullptr, &mSwapChain1);
        SafeRelease(dxgiFactory2);
        SafeRelease(dxgiFactory);
        if (FAILED(result))
        {
            return egl::EglBadAlloc()
                   << "Failed to create swap chain for window, " << gl::FmtHR(result);
        }

        mSwapChain = mSwapChain1;
        mSwapChain->AddRef();
    }
    else
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc               = {};
        swapChainDesc.BufferCount                        = 1;
        swapChainDesc.BufferDesc.Format                  = mSwapChainFormat;
        swapChainDesc.BufferDesc.Width                   = static_cast<UINT>(mWidth);
        swapChainDesc.BufferDesc.Height                  = static_cast<UINT>(mHeight);
        swapChainDesc.BufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.BufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.RefreshRate.Numerator   = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage =
            DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_BACK_BUFFER;
        swapChainDesc.Flags              = mSwapChainFlags;
        swapChainDesc.OutputWindow       = mWindow;
        swapChainDesc.SampleDesc.Count   = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed           = TRUE;
        swapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD;

        result = dxgiFactory->CreateSwapChain(mDevice, &swapChainDesc, &mSwapChain);
        SafeRelease(dxgiFactory);
        if (FAILED(result))
        {
            return egl::EglBadAlloc()
                   << "Failed to create swap chain for window, " << gl::FmtHR(result);
        }
    }

    ID3D11Texture2D *colorBuffer = nullptr;
    result                       = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                                         reinterpret_cast<void **>(&colorBuffer));
    if (FAILED(result))
    {
        return egl::EglBadAlloc() << "Failed to query texture from swap chain, "
                                  << gl::FmtHR(result);
    }

    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mColorRenderbufferID);
    mRenderbufferBufferHandle =
        mFunctionsWGL->dxRegisterObjectNV(mDeviceHandle, colorBuffer, mColorRenderbufferID,
                                          GL_RENDERBUFFER, WGL_ACCESS_READ_WRITE_NV);
    SafeRelease(colorBuffer);
    if (mRenderbufferBufferHandle == nullptr)
    {
        return egl::EglBadAlloc() << "Failed to register D3D object, "
                                  << gl::FmtErr(HRESULT_CODE(GetLastError()));
    }

    // Rebind the surface to the texture if needed.
    if (hadBoundSurface)
    {
        mTextureHandle = mFunctionsWGL->dxRegisterObjectNV(mDeviceHandle, colorBuffer, mTextureID,
                                                           GL_TEXTURE_2D, WGL_ACCESS_READ_WRITE_NV);
        if (mTextureHandle == nullptr)
        {
            return egl::EglBadAlloc()
                   << "Failed to register D3D object, " << gl::FmtErr(HRESULT_CODE(GetLastError()));
        }
    }

    error = setObjectsLocked(true);
    if (error.isError())
    {
        return error;
    }

    if (mDepthBufferFormat != GL_NONE)
    {
        ASSERT(mDepthRenderbufferID != 0);
        mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbufferID);
        mFunctionsGL->renderbufferStorage(GL_RENDERBUFFER, mDepthBufferFormat,
                                          static_cast<GLsizei>(mWidth),
                                          static_cast<GLsizei>(mHeight));
    }

    mFirstSwap = true;

    return egl::NoError();
}
}  // namespace rx
