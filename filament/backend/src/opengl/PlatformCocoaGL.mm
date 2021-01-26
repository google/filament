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

#include "OpenGLDriverFactory.h"
#include "gl_headers.h"

#include <utils/compiler.h>
#include <utils/Panic.h>

#include <OpenGL/OpenGL.h>
#include <Cocoa/Cocoa.h>

#include <vector>

namespace filament {

using namespace backend;

struct PlatformCocoaGLImpl {
    NSOpenGLContext* mGLContext = nullptr;
    NSView* mCurrentView = nullptr;
    std::vector<NSView*> mHeadlessSwapChains;
    NSRect mPreviousBounds = {};
    void updateOpenGLContext(NSView *nsView, bool resetView);
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
            NSOpenGLPFADepthSize,    (NSOpenGLPixelFormatAttribute) 24,
            NSOpenGLPFADoubleBuffer, (NSOpenGLPixelFormatAttribute) true,
            NSOpenGLPFAAccelerated,  (NSOpenGLPixelFormatAttribute) true,
            NSOpenGLPFANoRecovery,   (NSOpenGLPixelFormatAttribute) true,
            0, 0,
    };

    NSOpenGLContext* shareContext = (__bridge NSOpenGLContext*) sharedContext;
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
    NSOpenGLContext* nsOpenGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:shareContext];

    GLint interval = 0;
    [nsOpenGLContext makeCurrentContext];
    [nsOpenGLContext setValues:&interval forParameter:NSOpenGLCPSwapInterval];

    pImpl->mGLContext = nsOpenGLContext;

    int result = bluegl::bind();
    ASSERT_POSTCONDITION(!result, "Unable to load OpenGL entry points.");
    return OpenGLDriverFactory::create(this, sharedContext);
}

void PlatformCocoaGL::terminate() noexcept {
    pImpl->mGLContext = nil;
    bluegl::unbind();
}

Platform::SwapChain* PlatformCocoaGL::createSwapChain(void* nativewindow, uint64_t& flags) noexcept {
    // Transparent SwapChain is not supported
    flags &= ~backend::SWAP_CHAIN_CONFIG_TRANSPARENT;
    NSView* nsView = (__bridge NSView*)nativewindow;

    // If the SwapChain is being recreated (e.g. if the underlying surface has been resized),
    // then we need to force an update to occur in the subsequent makeCurrent, which can be done by
    // simply resetting the current view. In multi-window situations, this happens automatically.
    if (pImpl->mCurrentView == nsView) {
        pImpl->mCurrentView = nullptr;
    }

    return (SwapChain*) nativewindow;
}

Platform::SwapChain* PlatformCocoaGL::createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept {
    NSView* nsView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
    // adding the pointer to the array retains the NSView
    pImpl->mHeadlessSwapChains.push_back(nsView);
    return (__bridge SwapChain*)nsView;
}

void PlatformCocoaGL::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
    auto& v = pImpl->mHeadlessSwapChains;
    NSView* nsView = (__bridge NSView*)swapChain;
    auto it = std::find(v.begin(), v.end(), nsView);
    if (it != v.end()) {
        // removing the pointer from the array releases the NSView
        v.erase(it);
    }
}

void PlatformCocoaGL::makeCurrent(Platform::SwapChain* drawSwapChain,
        Platform::SwapChain* readSwapChain) noexcept {
    ASSERT_PRECONDITION_NON_FATAL(drawSwapChain == readSwapChain,
            "ContextManagerCocoa does not support using distinct draw/read swap chains.");
    NSView *nsView = (__bridge NSView*) drawSwapChain;

    NSRect currentBounds = [nsView convertRectToBacking:nsView.bounds];

    // Check if the view has been swapped out or resized.
    if (pImpl->mCurrentView != nsView) {
        pImpl->mCurrentView = nsView;
        pImpl->updateOpenGLContext(nsView, true);
    } else if (!CGRectEqualToRect(currentBounds, pImpl->mPreviousBounds)) {
        pImpl->updateOpenGLContext(nsView, false);
    }

    pImpl->mPreviousBounds = currentBounds;
}

void PlatformCocoaGL::commit(Platform::SwapChain* swapChain) noexcept {
    [pImpl->mGLContext flushBuffer];
}

bool PlatformCocoaGL::pumpEvents() noexcept {
    if (![NSThread isMainThread]) {
        return false;
    }
    [[NSRunLoop currentRunLoop] runUntilDate:[NSDate distantPast]];
    return true;
}

void PlatformCocoaGLImpl::updateOpenGLContext(NSView *nsView, bool resetView) {
    NSOpenGLContext* glContext = mGLContext;

    // NOTE: This is not documented well (if at all) but NSOpenGLContext requires "setView" and
    // "update" to be called from the UI thread. This became a hard requirement with the arrival
    // of macOS 10.15 (Catalina). If we were to call these methods from the GL thread, we would
    // see EXC_BAD_INSTRUCTION.
    if (![NSThread isMainThread]) {
        dispatch_sync(dispatch_get_main_queue(), ^(void) {
            if (resetView) {
                [glContext clearDrawable];
                [glContext setView:nsView];
            }
            [glContext update];
        });
    } else {
        if (resetView) {
            [glContext clearDrawable];
            [glContext setView:nsView];
        }
        [glContext update];
    }
}

} // namespace filament

#pragma clang diagnostic pop
