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
#include "MetalShaderCompiler.h"
#include "MetalState.h"

#include <backend/DriverEnums.h>

#include <CoreVideo/CVMetalTextureCache.h>
#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include <utils/FixedCircularBuffer.h>

#include <array>
#include <atomic>
#include <stack>

#if defined(FILAMENT_METAL_PROFILING)
#include <os/log.h>
#include <os/signpost.h>
#endif

#include <tsl/robin_set.h>

namespace filament {
namespace backend {

class MetalDriver;
class MetalBlitter;
class MetalBufferPool;
class MetalBumpAllocator;
class MetalRenderTarget;
class MetalSwapChain;
class MetalTexture;
class MetalTimerQueryInterface;
struct MetalUniformBuffer;
struct MetalIndexBuffer;
struct MetalVertexBuffer;
struct MetalDescriptorSet;

constexpr static uint8_t MAX_SAMPLE_COUNT = 8;  // Metal devices support at most 8 MSAA samples

class MetalPushConstantBuffer {
public:
    void setPushConstant(PushConstantVariant value, uint8_t index);
    bool isDirty() const { return mDirty; }
    void setBytes(id<MTLCommandEncoder> encoder, ShaderStage stage);
    void clear();

private:
    std::vector<PushConstantVariant> mPushConstants;
    bool mDirty = false;
};

class MetalDynamicOffsets {
public:
    void setOffsets(uint32_t set, const uint32_t* offsets, uint32_t count) {
        assert(set < MAX_DESCRIPTOR_SET_COUNT);

        auto getStartIndexForSet = [&](uint32_t s) {
            uint32_t startIndex = 0;
            for (uint32_t i = 0; i < s; i++) {
                startIndex += mCounts[i];
            }
            return startIndex;
        };

        const bool resizeNecessary = mCounts[set] != count;
        if (UTILS_UNLIKELY(resizeNecessary)) {
            int delta = count - mCounts[set];

            auto thisSetStart = mOffsets.begin() + getStartIndexForSet(set);
            if (delta > 0) {
                mOffsets.insert(thisSetStart, delta, 0);
            } else {
                mOffsets.erase(thisSetStart, thisSetStart - delta);
            }

            mCounts[set] = count;
        }

        if (resizeNecessary ||
                !std::equal(
                        offsets, offsets + count, mOffsets.begin() + getStartIndexForSet(set))) {
            std::copy(offsets, offsets + count, mOffsets.begin() + getStartIndexForSet(set));
            mDirty = true;
        }
    }
    bool isDirty() const { return mDirty; }
    void setDirty(bool dirty) { mDirty = dirty; }

    std::pair<uint32_t, const uint32_t*> getOffsets() const {
        return { mOffsets.size(), mOffsets.data() };
    }

private:
    std::array<uint32_t, MAX_DESCRIPTOR_SET_COUNT> mCounts = { 0 };
    std::vector<uint32_t> mOffsets;
    bool mDirty = false;
};

struct MetalContext {
    MetalDriver* driver;
    id<MTLDevice> device = nullptr;
    id<MTLCommandQueue> commandQueue = nullptr;

    // The ID of pendingCommandBuffer (or the next command buffer, if pendingCommandBuffer is nil).
    uint64_t pendingCommandBufferId = 1;
    // read from driver thread, set from completion handlers
    std::atomic<uint64_t> latestCompletedCommandBufferId = 0;
    id<MTLCommandBuffer> pendingCommandBuffer = nil;
    id<MTLRenderCommandEncoder> currentRenderPassEncoder = nil;
    uint32_t currentFrame = 0;

    std::atomic<bool> memorylessLimitsReached = false;

    // Supported features.
    bool supportsTextureSwizzling = false;
    bool supportsAutoDepthResolve = false;
    bool supportsMemorylessRenderTargets = false;
    bool supportsDepthClamp = false;
    uint8_t maxColorRenderTargets = 4;
    struct {
        uint8_t common;
        uint8_t apple;
        uint8_t mac;
    } highestSupportedGpuFamily;

    struct {
        bool staticTextureTargetError;
    } bugs;

    // sampleCountLookup[requestedSamples] gives a <= sample count supported by the device.
    std::array<uint8_t, MAX_SAMPLE_COUNT + 1> sampleCountLookup;

    // Tracks resources used by command buffers.
    MetalResourceTracker resourceTracker;

    RenderPassFlags currentRenderPassFlags;
    MetalRenderTarget* currentRenderTarget = nullptr;
    bool validPipelineBound = false;
    bool currentRenderPassAbandoned = false;

    // State trackers.
    PipelineStateTracker pipelineState;
    DepthStencilStateTracker depthStencilState;
    CullModeStateTracker cullModeState;
    WindingStateTracker windingState;
    DepthClampStateTracker depthClampState;
    ScissorRectStateTracker scissorRectState;
    Handle<HwRenderPrimitive> currentRenderPrimitive;

    // State caches.
    DepthStencilStateCache depthStencilStateCache;
    PipelineStateCache pipelineStateCache;
    SamplerStateCache samplerStateCache;
    ArgumentEncoderCache argumentEncoderCache;

    PolygonOffset currentPolygonOffset = {0.0f, 0.0f};

    std::array<MetalPushConstantBuffer, Program::SHADER_TYPE_COUNT> currentPushConstants;

    // Keeps track of descriptor sets we've finalized for the current render pass.
    tsl::robin_set<MetalDescriptorSet*> finalizedDescriptorSets;
    std::array<MetalDescriptorSet*, MAX_DESCRIPTOR_SET_COUNT> currentDescriptorSets = {};
    MetalBufferBindings<MAX_DESCRIPTOR_SET_COUNT, ShaderStage::VERTEX> vertexDescriptorBindings;
    MetalBufferBindings<MAX_DESCRIPTOR_SET_COUNT, ShaderStage::FRAGMENT> fragmentDescriptorBindings;
    MetalBufferBindings<MAX_DESCRIPTOR_SET_COUNT, ShaderStage::COMPUTE> computeDescriptorBindings;
    MetalDynamicOffsets dynamicOffsets;

    // Keeps track of all alive textures.
    tsl::robin_set<MetalTexture*> textures;

    MetalBufferPool* bufferPool;
    MetalBumpAllocator* bumpAllocator;

    MetalSwapChain* currentDrawSwapChain = nil;
    MetalSwapChain* currentReadSwapChain = nil;

    // External textures.
    CVMetalTextureCacheRef textureCache = nullptr;
    id<MTLComputePipelineState> externalImageComputePipelineState = nil;

    // Empty texture used to prevent GPU errors when a sampler has been bound without a texture.
    id<MTLTexture> emptyTexture = nil;
    id<MTLBuffer> emptyBuffer = nil;

    MetalBlitter* blitter = nullptr;

    // Fences, only supported on macOS 10.14 and iOS 12 and above.
    API_AVAILABLE(macos(10.14), ios(12.0))
    MTLSharedEventListener* eventListener = nil;
    // signalId is incremented in the MetalFence constructor, which is called on
    // both the driver (MetalTimerQueryFence::beginTimeElapsedQuery) and main
    // threads (in createFenceS), so an atomic is necessary.
    std::atomic<uint64_t> signalId = 1;

    MetalTimerQueryInterface* timerQueryImpl;

    std::stack<const char*> groupMarkers;

    MTLViewport currentViewport;

    MetalShaderCompiler* shaderCompiler = nullptr;

#if defined(FILAMENT_METAL_PROFILING)
    // Logging and profiling.
    os_log_t log;
    os_signpost_id_t signpostId;
#endif
};

void initializeSupportedGpuFamilies(MetalContext* context);

id<MTLCommandBuffer> getPendingCommandBuffer(MetalContext* context);

void submitPendingCommands(MetalContext* context);

id<MTLTexture> getOrCreateEmptyTexture(MetalContext* context);

bool isInRenderPass(MetalContext* context);

} // namespace backend
} // namespace filament

#endif //TNT_METALCONTEXT_H
