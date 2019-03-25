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

#ifndef TNT_METALCONTEXT_H
#define TNT_METALCONTEXT_H

#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include "MetalState.h"

namespace filament {
namespace driver {
namespace metal {

class MetalRenderTarget;
struct MetalSamplerGroup;
struct MetalSwapChain;

struct MetalContext {
    id<MTLDevice> device = nullptr;
    id<MTLCommandQueue> commandQueue = nullptr;

    // A pool for autoreleased objects throughout the lifetime of the Metal driver.
    NSAutoreleasePool* driverPool = nil;

    // A pool for autoreleased objects allocated during the execution of a frame.
    // The pool is created in beginFrame() and drained in endFrame().
    NSAutoreleasePool* framePool = nil;

    // Single use, re-created each frame.
    id<MTLCommandBuffer> currentCommandBuffer = nullptr;
    id<MTLRenderCommandEncoder> currentCommandEncoder = nullptr;

    RenderPassFlags currentRenderPassFlags;
    MetalRenderTarget* currentRenderTarget = nullptr;

    // State trackers.
    PipelineStateTracker pipelineState;
    DepthStencilStateTracker depthStencilState;
    UniformBufferStateTracker uniformState[VERTEX_BUFFER_START];
    CullModeStateTracker cullModeState;

    // State caches.
    DepthStencilStateCache depthStencilStateCache;
    PipelineStateCache pipelineStateCache;
    SamplerStateCache samplerStateCache;

    MetalSamplerGroup* samplerBindings[NUM_SAMPLER_BINDINGS] = {};

    // Surface-related properties.
    MetalSwapChain* currentSurface = nullptr;
    id<CAMetalDrawable> currentDrawable = nullptr;
    MTLPixelFormat currentSurfacePixelFormat = MTLPixelFormatInvalid;
    MTLPixelFormat currentDepthPixelFormat = MTLPixelFormatInvalid;
};

// Acquire the current surface's CAMetalDrawable for the current frame if it has not already been
// acquired.
// This method returns the drawable and stores it in the context's currentDrawable field.
id<CAMetalDrawable> acquireDrawable(MetalContext* context);

} // namespace metal
} // namespace driver
} // namespace filament

#endif //TNT_METALCONTEXT_H
