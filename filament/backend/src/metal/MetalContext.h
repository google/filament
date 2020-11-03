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

#include "MetalResourceTracker.h"
#include "MetalState.h"

#include <CoreVideo/CVMetalTextureCache.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

namespace filament {
namespace backend {
namespace metal {

class MetalBlitter;
class MetalBufferPool;
class MetalRenderTarget;
class MetalSwapChain;
class TimerQueryInterface;
struct MetalUniformBuffer;
struct MetalIndexBuffer;
struct MetalSamplerGroup;
struct MetalVertexBuffer;

struct MetalContext {
    id<MTLDevice> device = nullptr;
    id<MTLCommandQueue> commandQueue = nullptr;

    id<MTLCommandBuffer> pendingCommandBuffer = nullptr;
    id<MTLRenderCommandEncoder> currentRenderPassEncoder = nullptr;

    // Tracks resources used by command buffers.
    MetalResourceTracker resourceTracker;

    RenderPassFlags currentRenderPassFlags;
    MetalRenderTarget* currentRenderTarget = nullptr;

    // State trackers.
    PipelineStateTracker pipelineState;
    DepthStencilStateTracker depthStencilState;
    UniformBufferState uniformState[VERTEX_BUFFER_START];
    CullModeStateTracker cullModeState;

    // State caches.
    DepthStencilStateCache depthStencilStateCache;
    PipelineStateCache pipelineStateCache;
    SamplerStateCache samplerStateCache;

    MetalSamplerGroup* samplerBindings[SAMPLER_BINDING_COUNT] = {};

    MetalBufferPool* bufferPool;

    MetalSwapChain* currentSurface = nil;

    // External textures.
    CVMetalTextureCacheRef textureCache = nullptr;
    id<MTLComputePipelineState> externalImageComputePipelineState = nil;

    // Empty texture used to prevent GPU errors when a sampler has been bound without a texture.
    id<MTLTexture> emptyTexture = nil;

    MetalBlitter* blitter = nullptr;

    // Fences, only supported on macOS 10.14 and iOS 12 and above.
    API_AVAILABLE(macos(10.14), ios(12.0))
    MTLSharedEventListener* eventListener = nil;
    uint64_t signalId = 1;

    TimerQueryInterface* timerQueryImpl;
};

id<MTLCommandBuffer> getPendingCommandBuffer(MetalContext* context);

void submitPendingCommands(MetalContext* context);

id<MTLTexture> getOrCreateEmptyTexture(MetalContext* context);

bool isInRenderPass(MetalContext* context);

} // namespace metal
} // namespace backend
} // namespace filament

#endif //TNT_METALCONTEXT_H
