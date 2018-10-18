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

#include "driver/opengl/PlatformCocoaTouchGL.h"

#include <OpenGLES/EAGL.h>
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#include <UIKit/UIKit.h>

#include "driver/DriverBase.h"

#include <filament/driver/Platform.h>

#include <utils/Panic.h>

#include "driver/opengl/OpenGLDriver.h"

namespace filament {

using namespace driver;

struct PlatformCocoaTouchGLImpl {
    EAGLContext* mGLContext = nullptr;
    CAEAGLLayer* mCurrentGlLayer = nullptr;
    GLuint mDefaultFramebuffer = 0;
    GLuint mDefaultColorbuffer = 0;
    GLuint mDefaultDepthbuffer = 0;
};

PlatformCocoaTouchGL::PlatformCocoaTouchGL()
        : pImpl(new PlatformCocoaTouchGLImpl) {
}

PlatformCocoaTouchGL::~PlatformCocoaTouchGL() noexcept {
    delete pImpl;
}

Driver* PlatformCocoaTouchGL::createDriver(void* const sharedGLContext) noexcept {
    EAGLSharegroup* sharegroup = (EAGLSharegroup*) sharedGLContext;

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

    return OpenGLDriver::create(this, sharedGLContext);
}

void PlatformCocoaTouchGL::terminate() noexcept {
    [pImpl->mGLContext release];
}

Platform::SwapChain* PlatformCocoaTouchGL::createSwapChain(void* nativewindow, uint64_t& flags) noexcept {
    // Transparent swap chain is not supported
    flags &= ~driver::SWAP_CHAIN_CONFIG_TRANSPARENT;
    return (SwapChain*) nativewindow;
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
    CAEAGLLayer* glLayer = (CAEAGLLayer*) drawSwapChain;

    [EAGLContext setCurrentContext:pImpl->mGLContext];

    if (pImpl->mCurrentGlLayer != glLayer) {
        pImpl->mCurrentGlLayer = glLayer;

        glBindRenderbuffer(GL_RENDERBUFFER, pImpl->mDefaultColorbuffer);
        [pImpl->mGLContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:glLayer];

        // Retrieve width and height of color buffer.
        GLint width;
        GLint height;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);

        glBindRenderbuffer(GL_RENDERBUFFER, pImpl->mDefaultDepthbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    }

    // Test the framebuffer for completeness.
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    ASSERT_POSTCONDITION(status == GL_FRAMEBUFFER_COMPLETE, "Incomplete framebuffer.");
}

void PlatformCocoaTouchGL::commit(Platform::SwapChain* swapChain) noexcept {
    glBindRenderbuffer(GL_RENDERBUFFER, pImpl->mDefaultColorbuffer);
    [pImpl->mGLContext presentRenderbuffer:GL_RENDERBUFFER];
}

} // namespace filament
