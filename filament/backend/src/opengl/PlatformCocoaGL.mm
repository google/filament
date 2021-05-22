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

struct CocoaGLSwapChain : public Platform::SwapChain {
    CocoaGLSwapChain(NSView* inView);
    ~CocoaGLSwapChain() noexcept;

    NSView* view;
    NSRect previousBounds;
    NSRect previousWindowFrame;
    __strong NSMutableArray<id<NSObject>>* observers;
    NSRect currentBounds;
    NSRect currentWindowFrame;
};

struct PlatformCocoaGLImpl {
    NSOpenGLContext* mGLContext = nullptr;
    CocoaGLSwapChain* mCurrentSwapChain = nullptr;
    std::vector<NSView*> mHeadlessSwapChains;
    void updateOpenGLContext(NSView *nsView, bool resetView, bool clearView);
};

CocoaGLSwapChain::CocoaGLSwapChain( NSView* inView )
        : view(inView)
        , previousBounds(NSZeroRect)
        , previousWindowFrame(NSZeroRect)
        , observers([NSMutableArray array])
        , currentBounds(NSZeroRect)
        , currentWindowFrame(NSZeroRect) {
    __weak NSView* weakView = view;
    
    void (^noteHandler)(NSNotification *note) = ^(NSNotification *note) {
        if (weakView != nil) {
            __strong NSView* strongView = weakView;
            currentBounds = [strongView convertRectToBacking: strongView.bounds];
            currentWindowFrame = strongView.window.frame;
        }
    };
    
    // Various methods below should only be called from the main thread:
    // -[NSView bounds], -[NSView convertRectToBacking:], -[NSView window],
    // -[NSWindow frame], -[NSView superview],
    // -[NSView setPostsFrameChangedNotifications:],
    // -[NSView setPostsBoundsChangedNotifications:]
    dispatch_async(dispatch_get_main_queue(), ^(void) {
        if (weakView != nil) {
            __strong NSView* strongView = weakView;
            
            currentBounds = [strongView convertRectToBacking: strongView.bounds];
            currentWindowFrame = strongView.window.frame;

            id<NSObject> observer = [NSNotificationCenter.defaultCenter
                addObserverForName: NSWindowDidResizeNotification
                object: strongView.window
                queue: nil
                usingBlock: noteHandler];
            [observers addObject: observer];
            observer = [NSNotificationCenter.defaultCenter
                addObserverForName: NSWindowDidMoveNotification
                object: strongView.window
                queue: nil
                usingBlock: noteHandler];
           [observers addObject: observer];

            NSView* aView = strongView;
            while (aView != nil) {
                aView.postsFrameChangedNotifications = YES;
                aView.postsBoundsChangedNotifications = YES;
                observer = [NSNotificationCenter.defaultCenter
                    addObserverForName: NSViewFrameDidChangeNotification
                    object: aView
                    queue: nil
                    usingBlock: noteHandler];
                [observers addObject: observer];
                observer = [NSNotificationCenter.defaultCenter
                    addObserverForName: NSViewBoundsDidChangeNotification
                    object: aView
                    queue: nil
                    usingBlock: noteHandler];
                [observers addObject: observer];
                
                aView = aView.superview;
            }
        }
    });
}

CocoaGLSwapChain::~CocoaGLSwapChain() noexcept {
    for (id<NSObject> observer in observers) {
         [NSNotificationCenter.defaultCenter removeObserver: observer];
    }
}

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

    CocoaGLSwapChain* swapChain = new CocoaGLSwapChain( nsView );

    // If the SwapChain is being recreated (e.g. if the underlying surface has been resized),
    // then we need to force an update to occur in the subsequent makeCurrent, which can be done by
    // simply resetting the current view. In multi-window situations, this happens automatically.
    if (pImpl->mCurrentSwapChain && pImpl->mCurrentSwapChain->view == nsView) {
        pImpl->mCurrentSwapChain = nullptr;
    }

    return swapChain;
}

Platform::SwapChain* PlatformCocoaGL::createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept {
    NSView* nsView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];

    // adding the pointer to the array retains the NSView
    pImpl->mHeadlessSwapChains.push_back(nsView);

    CocoaGLSwapChain* swapChain = new CocoaGLSwapChain( nsView );

    return swapChain;
}

void PlatformCocoaGL::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
    CocoaGLSwapChain* cocoaSwapChain = static_cast<CocoaGLSwapChain*>(swapChain);
    if (pImpl->mCurrentSwapChain == cocoaSwapChain) {
        pImpl->mCurrentSwapChain = nullptr;
    }

    auto& v = pImpl->mHeadlessSwapChains;
    auto it = std::find(v.begin(), v.end(), cocoaSwapChain->view);
    if (it != v.end()) {
        // removing the pointer from the array releases the NSView
        v.erase(it);
    }
    delete cocoaSwapChain;
}

void PlatformCocoaGL::makeCurrent(Platform::SwapChain* drawSwapChain,
        Platform::SwapChain* readSwapChain) noexcept {
    ASSERT_PRECONDITION_NON_FATAL(drawSwapChain == readSwapChain,
            "ContextManagerCocoa does not support using distinct draw/read swap chains.");
    CocoaGLSwapChain* swapChain = (CocoaGLSwapChain*)drawSwapChain;
    NSRect currentBounds = swapChain->currentBounds;
    NSRect currentWindowFrame = swapChain->currentWindowFrame;

    // Check if the view has been swapped out or resized.
    // updateOpenGLContext() needs to call -clearDrawable if the view was
    // resized, otherwise we do not want to call -clearDrawable, as in addition
    // to disassociating the context from the view as the documentation says,
    // it also does what its name implies and clears the drawable pixels.
    // This is problematic with multiple windows, but still necessary if the
    // window has resized.
    if (pImpl->mCurrentSwapChain != swapChain) {
        pImpl->mCurrentSwapChain = swapChain;
        if (!NSEqualRects(currentBounds, swapChain->previousBounds)) {
            // A window is being resized or moved, but the last draw was a
            // different window (for example, it updates on a timer):
            // just call -setView first, otherwise -clearDrawable would clear
            // the other window. But if we do this the first time that the swap
            // chain has been created then resizing will show a black image.
            if (!NSIsEmptyRect(swapChain->previousBounds)) {
                pImpl->updateOpenGLContext(swapChain->view, true, false);
            }
            // Now call -clearDrawable, otherwise we get garbage during if we
            // are resizing.
            pImpl->updateOpenGLContext(swapChain->view, true, true);
        } else {
            // We are drawing another window: only call -setView.
            pImpl->updateOpenGLContext(swapChain->view, true, false);
        }
    } else if (!NSEqualRects(currentWindowFrame, swapChain->previousWindowFrame)) {
        // Same window has moved or resized: need to clear and set view.
        pImpl->updateOpenGLContext(swapChain->view, true, true);
    }

    swapChain->previousBounds = currentBounds;
    swapChain->previousWindowFrame = currentWindowFrame;
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

void PlatformCocoaGLImpl::updateOpenGLContext(NSView *nsView, bool resetView,
                                              bool clearView) {
    NSOpenGLContext* glContext = mGLContext;

    // NOTE: This is not documented well (if at all) but NSOpenGLContext requires "setView" and
    // "update" to be called from the UI thread. This became a hard requirement with the arrival
    // of macOS 10.15 (Catalina). If we were to call these methods from the GL thread, we would
    // see EXC_BAD_INSTRUCTION.
    if (![NSThread isMainThread]) {
        dispatch_sync(dispatch_get_main_queue(), ^(void) {
            if (resetView) {
                if (clearView) {
                    [glContext clearDrawable];
                }
                [glContext setView:nsView];
            }
            [glContext update];
        });
    } else {
        if (resetView) {
            if (clearView) {
                [glContext clearDrawable];
            }
            [glContext setView:nsView];
        }
        [glContext update];
    }
}

} // namespace filament

#pragma clang diagnostic pop
