/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <filamentapp/NativeWindowHelper.h>

#include <utils/Panic.h>

#include <Cocoa/Cocoa.h>
#include <QuartzCore/QuartzCore.h>

#include <SDL_syswm.h>

void* getNativeWindow(SDL_Window* sdlWindow) {
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    FILAMENT_CHECK_POSTCONDITION(SDL_GetWindowWMInfo(sdlWindow, &wmi))
            << "SDL version unsupported!";
    NSWindow* win = wmi.info.cocoa.window;
    NSView* view = [win contentView];
    return view;
}

void prepareNativeWindow(SDL_Window* sdlWindow) {
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    FILAMENT_CHECK_POSTCONDITION(SDL_GetWindowWMInfo(sdlWindow, &wmi))
            << "SDL version unsupported!";
    NSWindow* win = wmi.info.cocoa.window;
    [win setColorSpace:[NSColorSpace sRGBColorSpace]];
}

void* setUpMetalLayer(void* nativeView) {
    NSView* view = (NSView*) nativeView;
    [view setWantsLayer:YES];
    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.bounds = view.bounds;

    // It's important to set the drawableSize to the actual backing pixels. When rendering
    // full-screen, we can skip the macOS compositor if the size matches the display size.
    metalLayer.drawableSize = [view convertSizeToBacking:view.bounds.size];

    // In its implementation of vkGetPhysicalDeviceSurfaceCapabilitiesKHR, MoltenVK takes into
    // consideration both the size (in points) of the bounds, and the contentsScale of the
    // CAMetalLayer from which the Vulkan surface was created.
    // See also https://github.com/KhronosGroup/MoltenVK/issues/428
    metalLayer.contentsScale = view.window.backingScaleFactor;

    // This is set to NO by default, but is also important to ensure we can bypass the compositor
    // in full-screen mode
    // See "Direct to Display" http://metalkit.org/2017/06/30/introducing-metal-2.html.
    metalLayer.opaque = YES;

    [view setLayer:metalLayer];

    return metalLayer;
}

void* resizeMetalLayer(void* nativeView) {
    NSView* view = (NSView*) nativeView;
    CAMetalLayer* metalLayer = (CAMetalLayer*) view.layer;
    CGSize viewSize = view.bounds.size;
    NSSize newDrawableSize = [view convertSizeToBacking:view.bounds.size];
    metalLayer.drawableSize = newDrawableSize;
    metalLayer.contentsScale = view.window.backingScaleFactor;
    return metalLayer;
}
