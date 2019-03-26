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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#include "PlatformCocoaGL.h"

#include <OpenGL/OpenGL.h>
#include <Cocoa/Cocoa.h>

#include "DriverBase.h"

#include <backend/Platform.h>

#include <utils/Panic.h>

#include "OpenGLDriver.h"

namespace filament {

using namespace backend;

struct PlatformCocoaGLImpl {
    NSOpenGLContext* mGLContext = nullptr;
    NSView* mCurrentView = nullptr;
};

PlatformCocoaGL::PlatformCocoaGL()
        : pImpl(new PlatformCocoaGLImpl) {
}

PlatformCocoaGL::~PlatformCocoaGL() noexcept {
    delete pImpl;
}

Driver* PlatformCocoaGL::createDriver(void* sharedContext) noexcept {
    // NSOpenGLPFAColorSize: when unspecified, a format that matches the screen is preferred
    NSOpenGLPixelFormatAttribute pixelFormatAttributes[] = {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFADoubleBuffer, (NSOpenGLPixelFormatAttribute) true,
            NSOpenGLPFAAccelerated,  (NSOpenGLPixelFormatAttribute) true,
            NSOpenGLPFANoRecovery,   (NSOpenGLPixelFormatAttribute) true,
            0, 0,
    };

    NSOpenGLContext* shareContext = (NSOpenGLContext*)sharedContext;
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
    NSOpenGLContext* nsOpenGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:shareContext];
    [pixelFormat release];

    GLint interval = 0;
    [nsOpenGLContext makeCurrentContext];
    [nsOpenGLContext setValues:&interval forParameter:NSOpenGLCPSwapInterval];

    pImpl->mGLContext = nsOpenGLContext;

    int result = bluegl::bind();
    ASSERT_POSTCONDITION(!result, "Unable to load OpenGL entry points.");
    return OpenGLDriver::create(this, sharedContext);
}

void PlatformCocoaGL::terminate() noexcept {
    [pImpl->mGLContext release];
    bluegl::unbind();
}

Platform::SwapChain* PlatformCocoaGL::createSwapChain(void* nativewindow, uint64_t& flags) noexcept {
    // Transparent swap chain is not supported
    flags &= ~backend::SWAP_CHAIN_CONFIG_TRANSPARENT;
    return (SwapChain*) nativewindow;
}

void PlatformCocoaGL::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
}

void PlatformCocoaGL::makeCurrent(Platform::SwapChain* drawSwapChain,
        Platform::SwapChain* readSwapChain) noexcept {
    ASSERT_PRECONDITION_NON_FATAL(drawSwapChain == readSwapChain,
            "ContextManagerCocoa does not support using distinct draw/read swap chains.");
    NSView *nsView = (NSView*) drawSwapChain;
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

void PlatformCocoaGL::commit(Platform::SwapChain* swapChain) noexcept {
    [pImpl->mGLContext flushBuffer];
}

} // namespace filament

#pragma clang diagnostic pop
