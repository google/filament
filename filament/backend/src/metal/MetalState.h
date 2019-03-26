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

#ifndef TNT_METAL_STATE_H
#define TNT_METAL_STATE_H

#include <Metal/Metal.h>

#include "private/backend/Driver.h"
#include "private/backend/Program.h"

#include <backend/DriverEnums.h>

#include <memory>
#include <tsl/robin_map.h>
#include <utils/Hash.h>

namespace filament {
namespace backend {

inline bool operator==(const backend::SamplerParams& lhs, const backend::SamplerParams& rhs) {
    return lhs.u == rhs.u;
}

namespace metal {

static constexpr uint32_t MAX_VERTEX_ATTRIBUTE_COUNT = backend::MAX_VERTEX_ATTRIBUTE_COUNT;
static constexpr uint32_t SAMPLER_GROUP_COUNT = Program::UNIFORM_BINDING_COUNT;
static constexpr uint32_t SAMPLER_BINDING_COUNT = backend::MAX_SAMPLER_COUNT;
static constexpr uint32_t VERTEX_BUFFER_START = Program::UNIFORM_BINDING_COUNT;

// Forward declarations necessary here, definitions at end of file.
inline bool operator==(const MTLViewport& lhs, const MTLViewport& rhs);
inline bool operator!=(const MTLViewport& lhs, const MTLViewport& rhs);

// VertexDescription is part of Metal's pipeline state, and represents how vertex attributes are
// laid out in memory.
// Vertex attributes are "turned on" by setting format to something other than
// MTLVertexFormatInvalid, which is the default.
struct VertexDescription {
    struct Attribute {
        MTLVertexFormat format;
        uint32_t buffer;
        uint32_t offset;
    };
    struct Layout {
        uint32_t stride;
    };
    Attribute attributes[MAX_VERTEX_ATTRIBUTE_COUNT] = {};
    Layout layouts[MAX_VERTEX_ATTRIBUTE_COUNT] = {};

    bool operator==(const VertexDescription& rhs) const noexcept {
        bool result = true;
        for (uint32_t i = 0; i < MAX_VERTEX_ATTRIBUTE_COUNT; i++) {
            result &= (
                    this->attributes[i].format == rhs.attributes[i].format &&
                    this->attributes[i].buffer == rhs.attributes[i].buffer &&
                    this->attributes[i].offset == rhs.attributes[i].offset
            );
        }
        for (uint32_t i = 0; i < MAX_VERTEX_ATTRIBUTE_COUNT; i++) {
            result &= this->layouts[i].stride == rhs.layouts[i].stride;
        }
        return result;
    }

    bool operator!=(const VertexDescription& rhs) const noexcept {
        return !operator==(rhs);
    }
};

struct BlendState {
    MTLBlendOperation alphaBlendOperation = MTLBlendOperationAdd;
    MTLBlendOperation rgbBlendOperation = MTLBlendOperationAdd;
    MTLBlendFactor destinationAlphaBlendFactor = MTLBlendFactorZero;
    MTLBlendFactor destinationRGBBlendFactor = MTLBlendFactorZero;
    MTLBlendFactor sourceAlphaBlendFactor = MTLBlendFactorOne;
    MTLBlendFactor sourceRGBBlendFactor = MTLBlendFactorOne;
    bool blendingEnabled = false;
    char padding[3] = { 0 };

    bool operator==(const BlendState& rhs) const noexcept {
        return (
                this->blendingEnabled == rhs.blendingEnabled &&
                this->alphaBlendOperation == rhs.alphaBlendOperation &&
                this->rgbBlendOperation == rhs.rgbBlendOperation &&
                this->destinationAlphaBlendFactor == rhs.destinationAlphaBlendFactor &&
                this->destinationRGBBlendFactor == rhs.destinationRGBBlendFactor &&
                this->sourceAlphaBlendFactor == rhs.sourceAlphaBlendFactor &&
                this->sourceRGBBlendFactor == rhs.sourceRGBBlendFactor
        );
    }

    bool operator!=(const BlendState& rhs) const noexcept {
        return !operator==(rhs);
    }
};

// StateCache caches Metal state objects using StateType as a key.
// MetalType is the corresponding Metal API type.
// StateCreator is a functor that creates a new state of type MetalType. It is assumed that this
// type is created with a positive reference count (i.e., a new* method, per Apple convention), and
// thus each cached object is released in StateCache's destructor.
template<typename StateType,
         typename MetalType,
         typename StateCreator>
class StateCache {

public:

    StateCache() = default;

    StateCache(const StateCache&) = delete;
    StateCache& operator=(const StateCache&) = delete;

    ~StateCache() {
        for (auto it = mStateCache.begin(); it != mStateCache.end(); ++it) {
            [it.value() release];
        }
    }

    void setDevice(id<MTLDevice> device) noexcept { mDevice = device; }

    MetalType getOrCreateState(const StateType& state) noexcept {
        // Check if a valid state already exists in the cache.
        auto iter = mStateCache.find(state);
        if (UTILS_LIKELY(iter != mStateCache.end())) {
            auto foundState = iter.value();
            return foundState;
        }

        // If we reach this point, we couldn't find one in the cache; create a new one.
        const auto& metalObject = creator(mDevice, state);

        mStateCache.emplace(std::make_pair(
            state,
            metalObject
        ));

        return metalObject;
    }

private:

    StateCreator creator;
    id<MTLDevice> mDevice = nil;

    using HashFn = utils::hash::MurmurHashFn<StateType>;
    tsl::robin_map<StateType, MetalType, HashFn> mStateCache;

};

// StateTracker keeps track of state changes made to a Metal command encoder.
// Different kinds of state, like pipeline state, uniform buffer state, etc, are passed to the
// current Metal command encoder and persist throughout the lifetime of the encoder (a frame).
// StateTracker is used to prevent calling redundant state change methods.
template<typename StateType>
class StateTracker {

public:

    // Call to force the state to dirty at the beginning of each frame, as all state must be
    // re-bound.
    void invalidate() noexcept { mStateDirty = true; }

    void updateState(const StateType& newState) noexcept {
        if (mCurrentState != newState) {
            mCurrentState = newState;
            mStateDirty = true;
        }
    }

    // Returns true if the state has changed since the last call to stateChanged.
    bool stateChanged() noexcept {
        bool ret = mStateDirty;
        mStateDirty = false;
        return ret;
    }

    const StateType& getState() const {
        return mCurrentState;
    }

private:

    bool mStateDirty = true;
    StateType mCurrentState = {};

};

// Pipeline state

struct PipelineState {
    id<MTLFunction> vertexFunction = nil;
    id<MTLFunction> fragmentFunction = nil;
    VertexDescription vertexDescription;
    MTLPixelFormat colorAttachmentPixelFormat = MTLPixelFormatInvalid;
    MTLPixelFormat depthAttachmentPixelFormat = MTLPixelFormatInvalid;
    NSUInteger sampleCount = 1;
    BlendState blendState;

    bool operator==(const PipelineState& rhs) const noexcept {
        return (
                this->vertexFunction == rhs.vertexFunction &&
                this->fragmentFunction == rhs.fragmentFunction &&
                this->vertexDescription == rhs.vertexDescription &&
                this->colorAttachmentPixelFormat == rhs.colorAttachmentPixelFormat &&
                this->depthAttachmentPixelFormat == rhs.depthAttachmentPixelFormat &&
                this->sampleCount == rhs.sampleCount &&
                this->blendState == rhs.blendState
        );
    }

    bool operator!=(const PipelineState& rhs) const noexcept {
        return !operator==(rhs);
    }
};

struct PipelineStateCreator {
    id<MTLRenderPipelineState> operator()(id<MTLDevice> device, const PipelineState& state)
            noexcept;
};

using PipelineStateTracker = StateTracker<PipelineState>;

using PipelineStateCache = StateCache<PipelineState, id<MTLRenderPipelineState>,
        PipelineStateCreator>;

// Depth-stencil State

struct DepthStencilState {
    MTLCompareFunction compareFunction = MTLCompareFunctionNever;
    bool depthWriteEnabled = false;

    bool operator==(const DepthStencilState& rhs) const noexcept {
        return this->compareFunction == rhs.compareFunction &&
               this->depthWriteEnabled == rhs.depthWriteEnabled;
    }

    bool operator!=(const DepthStencilState& rhs) const noexcept {
        return !operator==(rhs);
    }
};

struct DepthStateCreator {
    id<MTLDepthStencilState> operator()(id<MTLDevice> device, const DepthStencilState& state)
            noexcept;
};

using DepthStencilStateTracker = StateTracker<DepthStencilState>;

using DepthStencilStateCache = StateCache<DepthStencilState, id<MTLDepthStencilState>,
        DepthStateCreator>;

// Uniform buffers

struct UniformBufferState {
    bool bound = false;
    Handle<HwUniformBuffer> ubh;
    uint64_t offset = 0;

    bool operator==(const UniformBufferState& rhs) const noexcept {
        return this->bound == rhs.bound &&
               this->ubh.getId() == rhs.ubh.getId() &&
               this->offset == rhs.offset;
    }

    bool operator!=(const UniformBufferState& rhs) const noexcept {
        return !operator==(rhs);
    }
};

using UniformBufferStateTracker = StateTracker<UniformBufferState>;

// Sampler states

struct SamplerStateCreator {
    id<MTLSamplerState> operator()(id<MTLDevice> device, const backend::SamplerParams& state)
            noexcept;
};

using SamplerStateCache = StateCache<backend::SamplerParams, id<MTLSamplerState>,
        SamplerStateCreator>;

// Raster-related state

using CullModeStateTracker = StateTracker<MTLCullMode>;

} // namespace metal
} // namespace backend
} // namespace filament

#endif //TNT_METAL_STATE_H
