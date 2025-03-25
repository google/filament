/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <backend/platforms/PlatformCocoaTouchGL.h>

#define GLES_SILENCE_DEPRECATION
#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#include <UIKit/UIKit.h>
#include <CoreVideo/CoreVideo.h>

#include "DriverBase.h"

#include <backend/Platform.h>

#include <utils/Panic.h>

#include "CocoaTouchExternalImage.h"

namespace filament::backend {

using namespace backend;

struct PlatformCocoaTouchGLImpl {
    EAGLContext* mGLContext = nullptr;
    CAEAGLLayer* mCurrentGlLayer = nullptr;
    std::vector<CAEAGLLayer*> mHeadlessGlLayers;
    std::vector<EAGLContext*> mAdditionalContexts;
    CGRect mCurrentGlLayerRect;
    GLuint mDefaultFramebuffer = 0;
    GLuint mDefaultColorbuffer = 0;
    GLuint mDefaultDepthbuffer = 0;
    CVOpenGLESTextureCacheRef mTextureCache = nullptr;
    CocoaTouchExternalImage::SharedGl* mExternalImageSharedGl = nullptr;
    struct ExternalImageCocoaTouchGL : public Platform::ExternalImage {
        CVPixelBufferRef cvBuffer;
    protected:
        ~ExternalImageCocoaTouchGL() noexcept final;
    };
};

PlatformCocoaTouchGLImpl::ExternalImageCocoaTouchGL::~ExternalImageCocoaTouchGL() noexcept = default;

PlatformCocoaTouchGL::PlatformCocoaTouchGL()
        : pImpl(new PlatformCocoaTouchGLImpl) {
}

PlatformCocoaTouchGL::~PlatformCocoaTouchGL() noexcept {
    delete pImpl;
}

Driver* PlatformCocoaTouchGL::createDriver(void* sharedGLContext, const Platform::DriverConfig& driverConfig) noexcept {
    EAGLSharegroup* sharegroup = (__bridge EAGLSharegroup*) sharedGLContext;

    EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:sharegroup];
    FILAMENT_CHECK_POSTCONDITION(context) << "Unable to create OpenGL ES context.";

    [EAGLContext setCurrentContext:context];

    pImpl->mGLContext = context;

    // Create a default framebuffer with color and depth attachments.
    GLuint framebuffer;
    GLuint renderbuffer[2]; // color and depth
    glGenFramebuffers(1, &framebuffer);
    glGenRenderbuffers(2, renderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer[0]);

    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer[1]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer[1]);

    pImpl->mDefaultFramebuffer = framebuffer;
    pImpl->mDefaultColorbuffer = renderbuffer[0];
    pImpl->mDefaultDepthbuffer = renderbuffer[1];

    UTILS_UNUSED_IN_RELEASE CVReturn success = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault,
            nullptr, pImpl->mGLContext, nullptr, &pImpl->mTextureCache);
    assert_invariant(success == kCVReturnSuccess);

    pImpl->mExternalImageSharedGl = new CocoaTouchExternalImage::SharedGl();

    return OpenGLPlatform::createDefaultDriver(this, sharedGLContext, driverConfig);
}

bool PlatformCocoaTouchGL::isExtraContextSupported() const noexcept {
    return true;
}

void PlatformCocoaTouchGL::createContext(bool shared) {
    EAGLSharegroup* const sharegroup = shared ? pImpl->mGLContext.sharegroup : nil;
    EAGLContext* const context = [[EAGLContext alloc]
                                  initWithAPI:kEAGLRenderingAPIOpenGLES3
                                   sharegroup:sharegroup];
    FILAMENT_CHECK_POSTCONDITION(context) << "Unable to create extra OpenGL ES context.";
    [EAGLContext setCurrentContext:context];
    pImpl->mAdditionalContexts.push_back(context);
}

void PlatformCocoaTouchGL::terminate() noexcept {
    CFRelease(pImpl->mTextureCache);
    pImpl->mGLContext = nil;
    delete pImpl->mExternalImageSharedGl;
}

Platform::SwapChain* PlatformCocoaTouchGL::createSwapChain(void* nativewindow, uint64_t flags) noexcept {
    return (SwapChain*) nativewindow;
}

Platform::SwapChain* PlatformCocoaTouchGL::createSwapChain(uint32_t width, uint32_t height, uint64_t flags) noexcept {
    CAEAGLLayer* glLayer = [CAEAGLLayer layer];
    glLayer.frame = CGRectMake(0, 0, width, height);

    // adding the pointer to the array retains the CAEAGLLayer
    pImpl->mHeadlessGlLayers.push_back(glLayer);
  
    return (__bridge SwapChain*) glLayer;
}

void PlatformCocoaTouchGL::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
    CAEAGLLayer* glLayer = (__bridge CAEAGLLayer*) swapChain;

    if (pImpl->mCurrentGlLayer == glLayer) {
        pImpl->mCurrentGlLayer = nullptr;
    }
  
    auto& v = pImpl->mHeadlessGlLayers;
    auto it = std::find(v.begin(), v.end(), glLayer);
    if(it != v.end()) {
        // removing the pointer from the array releases the CAEAGLLayer
        v.erase(it);
    }
}

uint32_t PlatformCocoaTouchGL::getDefaultFramebufferObject() noexcept {
    return pImpl->mDefaultFramebuffer;
}

bool PlatformCocoaTouchGL::makeCurrent(ContextType type, SwapChain* drawSwapChain,
        SwapChain* readSwapChain) noexcept {
    ASSERT_PRECONDITION_NON_FATAL(drawSwapChain == readSwapChain,
            "PlatformCocoaTouchGL does not support using distinct draw/read swap chains.");
    CAEAGLLayer* const glLayer = (__bridge CAEAGLLayer*) drawSwapChain;

    [EAGLContext setCurrentContext:pImpl->mGLContext];

    if (pImpl->mCurrentGlLayer != glLayer ||
                !CGRectEqualToRect(pImpl->mCurrentGlLayerRect, glLayer.bounds)) {
        pImpl->mCurrentGlLayer = glLayer;
        pImpl->mCurrentGlLayerRect = glLayer.bounds;

        // associate our default color renderbuffer with the swapchain
        // this renderbuffer is an attachment of the default FBO
        glBindRenderbuffer(GL_RENDERBUFFER, pImpl->mDefaultColorbuffer);
        [pImpl->mGLContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:glLayer];

        // Retrieve width and height of color buffer.
        GLint width, height;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

        // resize the depth buffer accordingly
        glBindRenderbuffer(GL_RENDERBUFFER, pImpl->mDefaultDepthbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

        // Test the framebuffer for completeness.
        // We must save/restore the framebuffer binding because filament is tracking the state
        GLint oldFramebuffer;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, pImpl->mDefaultFramebuffer);
        GLenum const status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        FILAMENT_CHECK_POSTCONDITION(status == GL_FRAMEBUFFER_COMPLETE)
                << "Incomplete framebuffer.";
        glBindFramebuffer(GL_FRAMEBUFFER, oldFramebuffer);
    }
    return true;
}

void PlatformCocoaTouchGL::commit(Platform::SwapChain* swapChain) noexcept {
    glBindRenderbuffer(GL_RENDERBUFFER, pImpl->mDefaultColorbuffer);
    [pImpl->mGLContext presentRenderbuffer:GL_RENDERBUFFER];

    // This needs to be done periodically.
    CVOpenGLESTextureCacheFlush(pImpl->mTextureCache, 0);
}

OpenGLPlatform::ExternalTexture* PlatformCocoaTouchGL::createExternalImageTexture() noexcept {
    ExternalTexture* outTexture = new CocoaTouchExternalImage(pImpl->mTextureCache,
            *pImpl->mExternalImageSharedGl);
    return outTexture;
}

void PlatformCocoaTouchGL::destroyExternalImageTexture(ExternalTexture* texture) noexcept {
    auto* p = static_cast<CocoaTouchExternalImage*>(texture);
    delete p;
}

void PlatformCocoaTouchGL::retainExternalImage(void* externalImage) noexcept {
    // Take ownership of the passed in buffer. It will be released the next time
    // setExternalImage is called, or when the texture is destroyed.
    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) externalImage;
    CVPixelBufferRetain(pixelBuffer);
}

bool PlatformCocoaTouchGL::setExternalImage(void* externalImage, ExternalTexture* texture) noexcept {
    CVPixelBufferRef cvPixelBuffer = (CVPixelBufferRef) externalImage;
    CocoaTouchExternalImage* cocoaExternalImage = static_cast<CocoaTouchExternalImage*>(texture);
    if (!cocoaExternalImage->set(cvPixelBuffer)) {
        return false;
    }
    texture->target = cocoaExternalImage->getTarget();
    texture->id = cocoaExternalImage->getGlTexture();
    // we used to set the internalFormat, but it's not used anywhere on the gl backend side
    // cocoaExternalImage->getInternalFormat();
    return true;
}

Platform::ExternalImageHandle PlatformCocoaTouchGL::createExternalImage(void* cvPixelBuffer) noexcept {
    auto* p = new(std::nothrow) PlatformCocoaTouchGLImpl::ExternalImageCocoaTouchGL;
    p->cvBuffer = (CVPixelBufferRef) cvPixelBuffer;
    return ExternalImageHandle{ p };
}

void PlatformCocoaTouchGL::retainExternalImage(ExternalImageHandleRef externalImage) noexcept {
    auto const* const cocoaTouchGlExternalImage
            = static_cast<PlatformCocoaTouchGLImpl::ExternalImageCocoaTouchGL const*>(externalImage.get());
    // Take ownership of the passed in buffer. It will be released the next time
    // setExternalImage is called, or when the texture is destroyed.
    CVPixelBufferRef pixelBuffer = cocoaTouchGlExternalImage->cvBuffer;
    CVPixelBufferRetain(pixelBuffer);
}

bool PlatformCocoaTouchGL::setExternalImage(ExternalImageHandleRef externalImage, ExternalTexture* texture) noexcept {
    auto const* const cocoaTouchGlExternalImage
            = static_cast<PlatformCocoaTouchGLImpl::ExternalImageCocoaTouchGL const*>(externalImage.get());
    CVPixelBufferRef cvPixelBuffer = cocoaTouchGlExternalImage->cvBuffer;
    CocoaTouchExternalImage* cocoaExternalImage = static_cast<CocoaTouchExternalImage*>(texture);
    if (!cocoaExternalImage->set(cvPixelBuffer)) {
        return false;
    }
    texture->target = cocoaExternalImage->getTarget();
    texture->id = cocoaExternalImage->getGlTexture();
    // we used to set the internalFormat, but it's not used anywhere on the gl backend side
    // cocoaExternalImage->getInternalFormat();
    return true;
}


} // namespace filament::backend
