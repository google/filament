//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayD3D.cpp: D3D implementation of egl::Display

#include "libANGLE/renderer/d3d/DisplayD3D.h"

#include <EGL/eglext.h>

#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Thread.h"
#include "libANGLE/histogram_macros.h"
#include "libANGLE/renderer/d3d/EGLImageD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/SurfaceD3D.h"
#include "libANGLE/renderer/d3d/SwapChainD3D.h"

#if !defined(ANGLE_DEFAULT_D3D11)
// Enables use of the Direct3D 11 API for a default display, when available
#    define ANGLE_DEFAULT_D3D11 1
#endif

namespace rx
{

using CreateRendererD3DFunction = RendererD3D *(*)(egl::Display *);

egl::Error CreateRendererD3D(egl::Display *display, RendererD3D **outRenderer)
{
    ASSERT(outRenderer != nullptr);

    std::vector<CreateRendererD3DFunction> rendererCreationFunctions;

    if (display->getPlatform() == EGL_PLATFORM_ANGLE_ANGLE)
    {
        const auto &attribMap              = display->getAttributeMap();
        EGLNativeDisplayType nativeDisplay = display->getNativeDisplayId();

        EGLint requestedDisplayType = static_cast<EGLint>(
            attribMap.get(EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE));

#if defined(ANGLE_ENABLE_D3D11)
        const auto addD3D11 = nativeDisplay == EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE ||
                              nativeDisplay == EGL_D3D11_ONLY_DISPLAY_ANGLE ||
                              requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
#endif

#if defined(ANGLE_ENABLE_D3D9)
        const auto addD3D9 = nativeDisplay == EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE ||
                             requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE;
#endif

#if ANGLE_DEFAULT_D3D11
#    if defined(ANGLE_ENABLE_D3D11)
        if (addD3D11)
        {
            rendererCreationFunctions.push_back(CreateRenderer11);
        }
#    endif

#    if defined(ANGLE_ENABLE_D3D9)
        if (addD3D9)
        {
            rendererCreationFunctions.push_back(CreateRenderer9);
        }
#    endif
#else
#    if defined(ANGLE_ENABLE_D3D9)
        if (addD3D9)
        {
            rendererCreationFunctions.push_back(CreateRenderer9);
        }
#    endif

#    if defined(ANGLE_ENABLE_D3D11)
        if (addD3D11)
        {
            rendererCreationFunctions.push_back(CreateRenderer11);
        }
#    endif
#endif

        if (nativeDisplay != EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE &&
            nativeDisplay != EGL_D3D11_ONLY_DISPLAY_ANGLE &&
            requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE)
        {
            // The default display is requested, try the D3D9 and D3D11 renderers, order them using
            // the definition of ANGLE_DEFAULT_D3D11
#if ANGLE_DEFAULT_D3D11
#    if defined(ANGLE_ENABLE_D3D11)
            rendererCreationFunctions.push_back(CreateRenderer11);
#    endif
#    if defined(ANGLE_ENABLE_D3D9)
            rendererCreationFunctions.push_back(CreateRenderer9);
#    endif
#else
#    if defined(ANGLE_ENABLE_D3D9)
            rendererCreationFunctions.push_back(CreateRenderer9);
#    endif
#    if defined(ANGLE_ENABLE_D3D11)
            rendererCreationFunctions.push_back(CreateRenderer11);
#    endif
#endif
        }
    }
    else if (display->getPlatform() == EGL_PLATFORM_DEVICE_EXT)
    {
#if defined(ANGLE_ENABLE_D3D11)
        if (display->getDevice()->getExtensions().deviceD3D11)
        {
            rendererCreationFunctions.push_back(CreateRenderer11);
        }
#endif
    }
    else
    {
        UNIMPLEMENTED();
    }

    for (size_t i = 0; i < rendererCreationFunctions.size(); i++)
    {
        RendererD3D *renderer = rendererCreationFunctions[i](display);
        egl::Error result     = renderer->initialize();

#if defined(ANGLE_ENABLE_D3D11)
        if (renderer->getRendererClass() == RENDERER_D3D11)
        {
            ASSERT(result.getID() >= 0 && result.getID() < NUM_D3D11_INIT_ERRORS);
            ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.D3D11InitializeResult", result.getID(),
                                        NUM_D3D11_INIT_ERRORS);
        }
#endif

#if defined(ANGLE_ENABLE_D3D9)
        if (renderer->getRendererClass() == RENDERER_D3D9)
        {
            ASSERT(result.getID() >= 0 && result.getID() < NUM_D3D9_INIT_ERRORS);
            ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.D3D9InitializeResult", result.getID(),
                                        NUM_D3D9_INIT_ERRORS);
        }
#endif

        if (!result.isError())
        {
            *outRenderer = renderer;
            return result;
        }

        // Failed to create the renderer, try the next
        SafeDelete(renderer);
        ERR() << "Failed to create D3D renderer: " << result.getMessage();
    }

    return egl::EglNotInitialized() << "No available renderers.";
}

DisplayD3D::DisplayD3D(const egl::DisplayState &state) : DisplayImpl(state), mRenderer(nullptr) {}

SurfaceImpl *DisplayD3D::createWindowSurface(const egl::SurfaceState &state,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    ASSERT(mRenderer != nullptr);
    return new WindowSurfaceD3D(state, mRenderer, mDisplay, window, attribs);
}

SurfaceImpl *DisplayD3D::createPbufferSurface(const egl::SurfaceState &state,
                                              const egl::AttributeMap &attribs)
{
    ASSERT(mRenderer != nullptr);
    return new PbufferSurfaceD3D(state, mRenderer, mDisplay, 0, nullptr, attribs);
}

SurfaceImpl *DisplayD3D::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                       EGLenum buftype,
                                                       EGLClientBuffer clientBuffer,
                                                       const egl::AttributeMap &attribs)
{
    ASSERT(mRenderer != nullptr);
    return new PbufferSurfaceD3D(state, mRenderer, mDisplay, buftype, clientBuffer, attribs);
}

SurfaceImpl *DisplayD3D::createPixmapSurface(const egl::SurfaceState &state,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

ImageImpl *DisplayD3D::createImage(const egl::ImageState &state,
                                   const gl::Context *context,
                                   EGLenum target,
                                   const egl::AttributeMap &attribs)
{
    return new EGLImageD3D(state, target, attribs, mRenderer);
}

DeviceImpl *DisplayD3D::createDevice()
{
    return mRenderer->createEGLDevice();
}

rx::ContextImpl *DisplayD3D::createContext(const gl::State &state,
                                           gl::ErrorSet *errorSet,
                                           const egl::Config *configuration,
                                           const gl::Context *shareContext,
                                           const egl::AttributeMap &attribs)
{
    ASSERT(mRenderer != nullptr);
    return mRenderer->createContext(state, errorSet);
}

StreamProducerImpl *DisplayD3D::createStreamProducerD3DTexture(
    egl::Stream::ConsumerType consumerType,
    const egl::AttributeMap &attribs)
{
    ASSERT(mRenderer != nullptr);
    return mRenderer->createStreamProducerD3DTexture(consumerType, attribs);
}

ExternalImageSiblingImpl *DisplayD3D::createExternalImageSibling(const gl::Context *context,
                                                                 EGLenum target,
                                                                 EGLClientBuffer buffer,
                                                                 const egl::AttributeMap &attribs)
{
    ASSERT(mRenderer != nullptr);
    return mRenderer->createExternalImageSibling(context, target, buffer, attribs);
}

ShareGroupImpl *DisplayD3D::createShareGroup(const egl::ShareGroupState &state)
{
    return new ShareGroupD3D(state);
}

egl::Error DisplayD3D::makeCurrent(egl::Display *display,
                                   egl::Surface *drawSurface,
                                   egl::Surface *readSurface,
                                   gl::Context *context)
{
    // Ensure the appropriate global DebugAnnotator is used
    ASSERT(mRenderer != nullptr);
    mRenderer->setGlobalDebugAnnotator();

    return egl::NoError();
}

egl::Error DisplayD3D::initialize(egl::Display *display)
{
    ASSERT(mRenderer == nullptr && display != nullptr);
    mDisplay = display;
    ANGLE_TRY(CreateRendererD3D(display, &mRenderer));
    return egl::NoError();
}

void DisplayD3D::terminate()
{
    SafeDelete(mRenderer);
}

egl::ConfigSet DisplayD3D::generateConfigs()
{
    ASSERT(mRenderer != nullptr);
    return mRenderer->generateConfigs();
}

bool DisplayD3D::testDeviceLost()
{
    ASSERT(mRenderer != nullptr);
    return mRenderer->testDeviceLost();
}

egl::Error DisplayD3D::restoreLostDevice(const egl::Display *display)
{
    // Release surface resources to make the Reset() succeed
    for (auto surface : mState.surfaceMap)
    {
        ASSERT(!surface.second->getBoundTexture());
        SurfaceD3D *surfaceD3D = GetImplAs<SurfaceD3D>(surface.second);
        surfaceD3D->releaseSwapChain();
    }

    if (!mRenderer->resetDevice())
    {
        return egl::EglBadAlloc();
    }

    // Restore any surfaces that may have been lost
    for (auto surface : mState.surfaceMap)
    {
        SurfaceD3D *surfaceD3D = GetImplAs<SurfaceD3D>(surface.second);

        ANGLE_TRY(surfaceD3D->resetSwapChain(display));
    }

    return egl::NoError();
}

bool DisplayD3D::isValidNativeWindow(EGLNativeWindowType window) const
{
    return mRenderer->isValidNativeWindow(window);
}

egl::Error DisplayD3D::validateClientBuffer(const egl::Config *config,
                                            EGLenum buftype,
                                            EGLClientBuffer clientBuffer,
                                            const egl::AttributeMap &attribs) const
{
    switch (buftype)
    {
        case EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE:
            return mRenderer->validateShareHandle(config, static_cast<HANDLE>(clientBuffer),
                                                  attribs);

        case EGL_D3D_TEXTURE_ANGLE:
            return mRenderer->getD3DTextureInfo(config, static_cast<IUnknown *>(clientBuffer),
                                                attribs, nullptr, nullptr, nullptr, nullptr,
                                                nullptr, nullptr);

        default:
            return DisplayImpl::validateClientBuffer(config, buftype, clientBuffer, attribs);
    }
}

egl::Error DisplayD3D::validateImageClientBuffer(const gl::Context *context,
                                                 EGLenum target,
                                                 EGLClientBuffer clientBuffer,
                                                 const egl::AttributeMap &attribs) const
{
    switch (target)
    {
        case EGL_D3D11_TEXTURE_ANGLE:
        {
            return mRenderer->getD3DTextureInfo(nullptr, static_cast<IUnknown *>(clientBuffer),
                                                attribs, nullptr, nullptr, nullptr, nullptr,
                                                nullptr, nullptr);
        }

        default:
            return DisplayImpl::validateImageClientBuffer(context, target, clientBuffer, attribs);
    }
}

void DisplayD3D::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    mRenderer->generateDisplayExtensions(outExtensions);
}

std::string DisplayD3D::getRendererDescription()
{
    if (mRenderer)
    {
        return mRenderer->getRendererDescription();
    }
    return std::string();
}

std::string DisplayD3D::getVendorString()
{
    if (mRenderer)
    {
        return mRenderer->getVendorString();
    }
    return std::string();
}

std::string DisplayD3D::getVersionString(bool includeFullVersion)
{
    if (mRenderer)
    {
        return mRenderer->getVersionString(includeFullVersion);
    }
    return std::string();
}

void DisplayD3D::generateCaps(egl::Caps *outCaps) const
{
    // Display must be initialized to generate caps
    ASSERT(mRenderer != nullptr);

    outCaps->textureNPOT = mRenderer->getNativeExtensions().textureNpotOES;
}

egl::Error DisplayD3D::waitClient(const gl::Context *context)
{
    for (auto surface : mState.surfaceMap)
    {
        SurfaceD3D *surfaceD3D = GetImplAs<SurfaceD3D>(surface.second);
        ANGLE_TRY(surfaceD3D->checkForOutOfDateSwapChain(this));
    }

    return egl::NoError();
}

egl::Error DisplayD3D::waitNative(const gl::Context *context, EGLint engine)
{
    egl::Surface *drawSurface = context->getCurrentDrawSurface();
    egl::Surface *readSurface = context->getCurrentReadSurface();

    if (drawSurface != nullptr)
    {
        SurfaceD3D *drawSurfaceD3D = GetImplAs<SurfaceD3D>(drawSurface);
        ANGLE_TRY(drawSurfaceD3D->checkForOutOfDateSwapChain(this));
    }

    if (readSurface != nullptr)
    {
        SurfaceD3D *readSurfaceD3D = GetImplAs<SurfaceD3D>(readSurface);
        ANGLE_TRY(readSurfaceD3D->checkForOutOfDateSwapChain(this));
    }

    return egl::NoError();
}

gl::Version DisplayD3D::getMaxSupportedESVersion() const
{
    return mRenderer->getMaxSupportedESVersion();
}

gl::Version DisplayD3D::getMaxConformantESVersion() const
{
    return mRenderer->getMaxConformantESVersion();
}

void DisplayD3D::handleResult(HRESULT hr,
                              const char *message,
                              const char *file,
                              const char *function,
                              unsigned int line)
{
    ASSERT(FAILED(hr));

    std::stringstream errorStream;
    errorStream << "Internal D3D11 error: " << gl::FmtHR(hr) << ", in " << file << ", " << function
                << ":" << line << ". " << message;

    mStoredErrorString = errorStream.str();
}

void DisplayD3D::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    mRenderer->initializeFrontendFeatures(features);
}

void DisplayD3D::populateFeatureList(angle::FeatureList *features)
{
    mRenderer->getFeatures().populateFeatureList(features);
}

}  // namespace rx
