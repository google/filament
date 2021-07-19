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

#include <array>
#include <stack>

#if defined(FILAMENT_METAL_PROFILING)
#include <os/log.h>
#include <os/signpost.h>
#endif

#include <tsl/robin_set.h>

namespace filament {
namespace backend {
namespace metal {

class MetalDriver;
class MetalBlitter;
class MetalBufferPool;
class MetalRenderTarget;
class MetalSwapChain;
class TimerQueryInterface;
struct MetalUniformBuffer;
struct MetalIndexBuffer;
struct MetalSamplerGroup;
struct MetalVertexBuffer;

constexpr static uint8_t MAX_SAMPLE_COUNT = 8;  // Metal devices support at most 8 MSAA samples

struct MetalContext {
    MetalDriver* driver;
    id<MTLDevice> device = nullptr;
    id<MTLCommandQueue> commandQueue = nullptr;

    id<MTLCommandBuffer> pendingCommandBuffer = nullptr;
    id<MTLRenderCommandEncoder> currentRenderPassEncoder = nullptr;

    // Supported features.
    bool supportsTextureSwizzling = false;
    uint8_t maxColorRenderTargets = 4;

    // sampleCountLookup[requestedSamples] gives a <= sample count supported by the device.
    std::array<uint8_t, MAX_SAMPLE_COUNT + 1> sampleCountLookup;

    // Tracks resources used by command buffers.
    MetalResourceTracker resourceTracker;

    RenderPassFlags currentRenderPassFlags;
    MetalRenderTarget* currentRenderTarget = nullptr;

    // State trackers.
    PipelineStateTracker pipelineState;
    DepthStencilStateTracker depthStencilState;
    UniformBufferState uniformState[VERTEX_BUFFER_START];
    CullModeStateTracker cullModeState;
    WindingStateTracker windingState;

    // State caches.
    DepthStencilStateCache depthStencilStateCache;
    PipelineStateCache pipelineStateCache;
    SamplerStateCache samplerStateCache;

    MetalSamplerGroup* samplerBindings[SAMPLER_BINDING_COUNT] = {};

    // Keeps track of all alive sampler groups.
    tsl::robin_set<MetalSamplerGroup*> samplerGroups;

    MetalBufferPool* bufferPool;

    MetalSwapChain* currentDrawSwapChain = nil;
    MetalSwapChain* currentReadSwapChain = nil;

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

    std::stack<const char*> groupMarkers;

#if defined(FILAMENT_METAL_PROFILING)
    // Logging and profiling.
    os_log_t log;
    os_signpost_id_t signpostId;
#endif
};

id<MTLCommandBuffer> getPendingCommandBuffer(MetalContext* context);

void submitPendingCommands(MetalContext* context);

id<MTLTexture> getOrCreateEmptyTexture(MetalContext* context);

bool isInRenderPass(MetalContext* context);

} // namespace metal
} // namespace backend
} // namespace filament

#endif //TNT_METALCONTEXT_H
