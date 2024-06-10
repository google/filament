/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <AppKit/AppKit.h>
#include <QuartzCore/QuartzCore.h>

#include "PlatformRunner.h"

static test::NativeView nativeView;

namespace test {

test::NativeView getNativeView() {
    return nativeView;
}

}

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property test::Backend backend;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    NSView* view = [self createView];

    if (self.backend == test::Backend::OPENGL) {
        nativeView.ptr = (void*) view;
    }
    if (self.backend == test::Backend::METAL) {
        nativeView.ptr = (void*) view.layer;
    }
    if (self.backend == test::Backend::VULKAN) {
        nativeView.ptr = (void*) view;
    }
    CGSize drawableSize = ((CAMetalLayer*) view.layer).drawableSize;
    nativeView.width = static_cast<size_t>(drawableSize.width);
    nativeView.height = static_cast<size_t>(drawableSize.height);

    test::runTests();
    // exit(runTests());
}

- (NSView*)createView {
    NSRect frame = NSMakeRect(0, 0, test::WINDOW_WIDTH, test::WINDOW_HEIGHT);
    NSWindow* window  = [[NSWindow alloc] initWithContentRect:frame
                                                     styleMask:NSWindowStyleMaskBorderless
                                                       backing:NSBackingStoreBuffered
                                                         defer:NO];
    [window setBackgroundColor:[NSColor blackColor]];
    [window makeKeyAndOrderFront:window];

    NSView* view = window.contentView;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    [view setWantsBestResolutionOpenGLSurface:YES];
#pragma clang diagnostic pop

    [view setWantsLayer:YES];

    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.bounds = view.bounds;

    // This is important, as it allows us to read pixels from the default swap chain.
    metalLayer.framebufferOnly = NO;

    // For test cases, we explicitly don't use convertSizeToBacking here because we want test cases
    // to run the same on retina and non-retina displays.
    metalLayer.drawableSize = CGSizeMake(512, 512);

    // This is set to NO by default, but is also important to ensure we can bypass the compositor
    // in full-screen mode
    // See "Direct to Display" http://metalkit.org/2017/06/30/introducing-metal-2.html.
    metalLayer.opaque = YES;

    [view setLayer:metalLayer];

    return view;
}

@end

int main(int argc, char* argv[]) {
    auto backend = test::parseArgumentsForBackend(argc, argv);
    test::initTests(backend, false, argc, argv);
    AppDelegate* delegate = [AppDelegate new];
    delegate.backend = backend;
    NSApplication* app = [NSApplication sharedApplication];
    [app setDelegate:delegate];
    [app run];
}
