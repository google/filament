/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include "backend/platforms/WebGPUPlatform.h"

#include <cstdint>

#include <webgpu/webgpu_cpp.h>

#include "utils/Panic.h"
#include "utils/ostream.h"

// Platform specific includes and defines
#if defined(__APPLE__)
    #include <Cocoa/Cocoa.h>
    #import <QuartzCore/CAMetalLayer.h>
#elif defined(FILAMENT_IOS)
    // Metal is not available when building for the iOS simulator on Desktop.
    #define METAL_AVAILABLE __has_include(<QuartzCore/CAMetalLayer.h>)
    #if METAL_AVAILABLE
        #import <Metal/Metal.h>
        #import <QuartzCore/CAMetalLayer.h>
    #endif
    // is this needed?
    #define METALVIEW_TAG 255
#else
    #error Not a supported Apple + WebGPU platform
#endif

namespace filament::backend {

WebGPUPlatform::SurfaceBundle WebGPUPlatform::createSurface(void* nativeWindow,
        uint64_t /*flags*/) {
    WebGPUPlatform::SurfaceBundle surfaceBundle{};
#if defined(__APPLE__)
    auto nsView = (__bridge NSView*) nativeWindow;
    FILAMENT_CHECK_POSTCONDITION(nsView) << "Unable to obtain Metal-backed NSView.";
    [nsView setWantsLayer:YES];
    id metalLayer = [CAMetalLayer layer];
    [nsView setLayer:metalLayer];
    wgpu::SurfaceSourceMetalLayer surfaceSourceMetalLayer{};
    surfaceSourceMetalLayer.layer = (__bridge void*) metalLayer;
    wgpu::SurfaceDescriptor surfaceDescriptor = {
        .nextInChain = &surfaceSourceMetalLayer,
        .label = "metal_surface",
    };
    surfaceBundle.surface = mInstance.CreateSurface(&surfaceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(surfaceBundle.surface != nullptr)
            << "Unable to create Metal-backed surface.";
#elif defined(FILAMENT_IOS)
    CAMetalLayer* metalLayer = (CAMetalLayer*) nativeWindow;
    wgpu::SurfaceSourceMetalLayer surfaceSourceMetalLayer{};
    surfaceSourceMetalLayer.layer = (__bridge void*) metalLayer;
    wgpu::SurfaceDescriptor surfaceDescriptor = {
        .nextInChain = &surfaceSourceMetalLayer,
        .label = "metal_surface",
    };
    surfaceBundle.surface = mInstance.CreateSurface(&surfaceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(surfaceBundle.surface != nullptr)
            << "Unable to create Metal-backed surface.";
#else
    #error Not a supported Apple + WebGPU platform
#endif
    return surfaceBundle;
}

}// namespace filament::backend
