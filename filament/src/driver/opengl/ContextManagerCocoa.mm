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

#include "driver/opengl/ContextManagerCocoa.h"

#include <OpenGL/OpenGL.h>
#include <Cocoa/Cocoa.h>

#include "driver/DriverBase.h"

#include <filament/driver/ExternalContext.h>

#include <utils/Panic.h>

#include "driver/opengl/OpenGLDriver.h"

namespace filament {

using namespace driver;

struct ContextManagerCocoaImpl {
    NSOpenGLContext* mGLContext = nullptr;
    NSView* mCurrentView = nullptr;
};

ContextManagerCocoa::ContextManagerCocoa()
        : pImpl(new ContextManagerCocoaImpl) {
}

ContextManagerCocoa::~ContextManagerCocoa() noexcept {
    delete pImpl;
}

std::unique_ptr<Driver> ContextManagerCocoa::createDriver(void* const sharedGLContext) noexcept {
    // NSOpenGLPFAColorSize: when unspecified, a format that matches the screen is preferred
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] = {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFADoubleBuffer, (NSOpenGLPixelFormatAttribute) true,
            NSOpenGLPFAAccelerated,  (NSOpenGLPixelFormatAttribute) true,
            NSOpenGLPFANoRecovery,   (NSOpenGLPixelFormatAttribute) true,
            0, 0,
    };

    NSOpenGLContext* shareContext = (NSOpenGLContext*)sharedGLContext;
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
    NSOpenGLContext* nsOpenGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:shareContext];
    [pixelFormat release];

    GLint interval = 0;
    [nsOpenGLContext makeCurrentContext];
    [nsOpenGLContext setValues:&interval forParameter:NSOpenGLCPSwapInterval];

    pImpl->mGLContext = nsOpenGLContext;

    int result = bluegl::bind();
    ASSERT_POSTCONDITION(!result, "Unable to load OpenGL entry points.");
    return OpenGLDriver::create(this, sharedGLContext);
}

void ContextManagerCocoa::terminate() noexcept {
    [pImpl->mGLContext release];
    bluegl::unbind();
}

ExternalContext::SwapChain* ContextManagerCocoa::createSwapChain(void* nativewindow, uint64_t& flags) noexcept {
    // Transparent swap chain is not supported
    flags &= ~driver::SWAP_CHAIN_CONFIG_TRANSPARENT;
    return (SwapChain*) nativewindow;
}

void ContextManagerCocoa::destroySwapChain(ExternalContext::SwapChain* swapChain) noexcept {
}

void ContextManagerCocoa::makeCurrent(ExternalContext::SwapChain* swapChain) noexcept {
    NSView *nsView = (NSView*) swapChain;
    if (pImpl->mCurrentView != nsView) {
        pImpl->mCurrentView = nsView;
        // Calling setView could change the viewport and/or scissor box state, but this isn't
        // accounted for in our OpenGL driver- so we save their state and recall it afterwards.
        GLint viewport[4];
        GLint scissor[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor);

        [pImpl->mGLContext setView:nsView];

        // Recall viewport and scissor state.
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
    }
    // this is needed only when the view resized. Not sure how to do only when needed.
    [pImpl->mGLContext update];
}

void ContextManagerCocoa::commit(ExternalContext::SwapChain* swapChain) noexcept {
    [pImpl->mGLContext flushBuffer];
}

} // namespace filament
