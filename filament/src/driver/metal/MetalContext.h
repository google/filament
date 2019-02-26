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

struct MetalRenderTarget;
struct MetalSamplerBuffer;
struct MetalSwapChain;

struct MetalContext {
    id<MTLDevice> mDevice = nullptr;
    id<MTLCommandQueue> mCommandQueue = nullptr;

    // A pool for autoreleased objects throughout the lifetime of the Metal driver.
    NSAutoreleasePool* mDriverPool = nil;

    // A pool for autoreleased objects allocated during the execution of a frame.
    // The pool is created in beginFrame() and drained in endFrame().
    NSAutoreleasePool* mFramePool = nil;

    // Single use, re-created each frame.
    id<MTLCommandBuffer> mCurrentCommandBuffer = nullptr;
    id<MTLRenderCommandEncoder> mCurrentCommandEncoder = nullptr;

    RenderPassFlags mCurrentRenderPassFlags;
    MetalRenderTarget* mCurrentRenderTarget = nullptr;

    // State trackers.
    PipelineStateTracker mPipelineState;
    DepthStencilStateTracker mDepthStencilState;
    UniformBufferStateTracker mUniformState[VERTEX_BUFFER_START];
    CullModeStateTracker mCullModeState;

    // State caches.
    DepthStencilStateCache mDepthStencilStateCache;
    PipelineStateCache mPipelineStateCache;
    SamplerStateCache mSamplerStateCache;

    id<MTLSamplerState> mBoundSamplers[NUM_SAMPLER_BINDINGS] = {};
    id<MTLTexture> mBoundTextures[NUM_SAMPLER_BINDINGS] = {};
    bool mSamplersDirty = true;
    bool mTexturesDirty = true;

    MetalSamplerBuffer* mSamplerBindings[NUM_SAMPLER_BINDINGS] = {};

    // Surface-related properties.
    MetalSwapChain* mCurrentSurface = nullptr;
    id<CAMetalDrawable> mCurrentDrawable = nullptr;
    MTLPixelFormat mCurrentSurfacePixelFormat = MTLPixelFormatInvalid;
    MTLPixelFormat mCurrentDepthPixelFormat = MTLPixelFormatInvalid;
};

} // namespace metal
} // namespace driver
} // namespace filament

#endif //TNT_METALCONTEXT_H
