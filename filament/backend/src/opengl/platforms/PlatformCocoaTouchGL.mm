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

#include "PlatformCocoaTouchGL.h"

#define GLES_SILENCE_DEPRECATION
#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#include <UIKit/UIKit.h>
#include <CoreVideo/CoreVideo.h>

#include "DriverBase.h"

#include <backend/Platform.h>

#include <utils/Panic.h>

#include "../OpenGLDriverFactory.h"

#include "../OpenGLDriver.h"
#include "CocoaTouchExternalImage.h"

namespace filament {

using namespace backend;

struct PlatformCocoaTouchGLImpl {
    EAGLContext* mGLContext = nullptr;
    CAEAGLLayer* mCurrentGlLayer = nullptr;
    CGRect mCurrentGlLayerRect;
    GLuint mDefaultFramebuffer = 0;
    GLuint mDefaultColorbuffer = 0;
    GLuint mDefaultDepthbuffer = 0;
    CVOpenGLESTextureCacheRef mTextureCache = nullptr;
    CocoaTouchExternalImage::SharedGl* mExternalImageSharedGl = nullptr;
};

PlatformCocoaTouchGL::PlatformCocoaTouchGL()
        : pImpl(new PlatformCocoaTouchGLImpl) {
}

PlatformCocoaTouchGL::~PlatformCocoaTouchGL() noexcept {
    delete pImpl;
}

Driver* PlatformCocoaTouchGL::createDriver(void* const sharedGLContext) noexcept {
    EAGLSharegroup* sharegroup = (__bridge EAGLSharegroup*) sharedGLContext;

    EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:sharegroup];
    ASSERT_POSTCONDITION(context, "Unable to create OpenGL ES context.");

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

    CVReturn success = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, nullptr,
            pImpl->mGLContext, nullptr, &pImpl->mTextureCache);
    assert_invariant(success == kCVReturnSuccess);

    pImpl->mExternalImageSharedGl = new CocoaTouchExternalImage::SharedGl();

    return OpenGLDriverFactory::create(this, sharedGLContext);
}

void PlatformCocoaTouchGL::terminate() noexcept {
    CFRelease(pImpl->mTextureCache);
    pImpl->mGLContext = nil;
    delete pImpl->mExternalImageSharedGl;
}

Platform::SwapChain* PlatformCocoaTouchGL::createSwapChain(void* nativewindow, uint64_t& flags) noexcept {
    // Transparent swap chain is not supported
    flags &= ~backend::SWAP_CHAIN_CONFIG_TRANSPARENT;
    return (SwapChain*) nativewindow;
}

Platform::SwapChain* PlatformCocoaTouchGL::createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept {
    // TODO: implement headless SwapChain
    return nullptr;
}

void PlatformCocoaTouchGL::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
}

void PlatformCocoaTouchGL::createDefaultRenderTarget(uint32_t& framebuffer, uint32_t& colorbuffer,
        uint32_t& depthbuffer) noexcept {
    framebuffer = pImpl->mDefaultFramebuffer;
    colorbuffer = pImpl->mDefaultColorbuffer;
    depthbuffer = pImpl->mDefaultDepthbuffer;
}

void PlatformCocoaTouchGL::makeCurrent(SwapChain* drawSwapChain, SwapChain* readSwapChain) noexcept {
    ASSERT_PRECONDITION_NON_FATAL(drawSwapChain == readSwapChain,
                                  "PlatformCocoaTouchGL does not support using distinct draw/read swap chains.");
    CAEAGLLayer* glLayer = (__bridge CAEAGLLayer*) drawSwapChain;

    [EAGLContext setCurrentContext:pImpl->mGLContext];

    if (pImpl->mCurrentGlLayer != glLayer ||
                !CGRectEqualToRect(pImpl->mCurrentGlLayerRect, glLayer.bounds)) {
        pImpl->mCurrentGlLayer = glLayer;
        pImpl->mCurrentGlLayerRect = glLayer.bounds;

        glBindFramebuffer(GL_FRAMEBUFFER, pImpl->mDefaultFramebuffer);

        glBindRenderbuffer(GL_RENDERBUFFER, pImpl->mDefaultColorbuffer);
        [pImpl->mGLContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:glLayer];

        // Retrieve width and height of color buffer.
        GLint width;
        GLint height;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

        glBindRenderbuffer(GL_RENDERBUFFER, pImpl->mDefaultDepthbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

        // Test the framebuffer for completeness.
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        ASSERT_POSTCONDITION(status == GL_FRAMEBUFFER_COMPLETE, "Incomplete framebuffer.");
    }
}

void PlatformCocoaTouchGL::commit(Platform::SwapChain* swapChain) noexcept {
    glBindRenderbuffer(GL_RENDERBUFFER, pImpl->mDefaultColorbuffer);
    [pImpl->mGLContext presentRenderbuffer:GL_RENDERBUFFER];

    // This needs to be done periodically.
    CVOpenGLESTextureCacheFlush(pImpl->mTextureCache, 0);
}

bool PlatformCocoaTouchGL::setExternalImage(void* externalImage, void* texture) noexcept {
    CVPixelBufferRef cvPixelBuffer = (CVPixelBufferRef) externalImage;
    auto* driverTexture = (OpenGLDriver::GLTexture*) texture;

    CocoaTouchExternalImage* cocoaExternalImage = (CocoaTouchExternalImage*) driverTexture->platformPImpl;

    if (!cocoaExternalImage->set(cvPixelBuffer)) {
        return false;
    }

    driverTexture->gl.id = cocoaExternalImage->getGlTexture();
    driverTexture->gl.internalFormat = cocoaExternalImage->getInternalFormat();
    driverTexture->gl.target = cocoaExternalImage->getTarget();
    driverTexture->gl.baseLevel = 0;
    driverTexture->gl.maxLevel = 0;

    return true;
}

void PlatformCocoaTouchGL::retainExternalImage(void* externalImage) noexcept {
    // Take ownership of the passed in buffer. It will be released the next time
    // setExternalImage is called, or when the texture is destroyed.
    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) externalImage;
    CVPixelBufferRetain(pixelBuffer);
}

void PlatformCocoaTouchGL::releaseExternalImage(void* externalImage) noexcept {
    CVPixelBufferRef pixelBuffer = (CVPixelBufferRef) externalImage;
    CVPixelBufferRelease(pixelBuffer);
}

void PlatformCocoaTouchGL::createExternalImageTexture(void* texture) noexcept {
    auto* driverTexture = (OpenGLDriver::GLTexture*) texture;

    driverTexture->platformPImpl = new CocoaTouchExternalImage(pImpl->mTextureCache,
            *pImpl->mExternalImageSharedGl);
}

void PlatformCocoaTouchGL::destroyExternalImage(void* texture) noexcept {
    auto* driverTexture = (OpenGLDriver::GLTexture*) texture;
    auto* p = (CocoaTouchExternalImage*) driverTexture->platformPImpl;
    delete p;
}

} // namespace filament
