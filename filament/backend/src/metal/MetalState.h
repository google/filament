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

#include <backend/DriverEnums.h>
#include <backend/Program.h>

#include <utils/bitset.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Hash.h>
#include <utils/Invocable.h>

#include <tsl/robin_map.h>

#include <memory>

namespace filament {
namespace backend {

//   Rasterization Bindings
//   ----------------------
//   Bindings    Buffer name                          Count
//   ------------------------------------------------------
//   0           Zero buffer (placeholder vertex buffer)  1
//   1-16        Filament vertex buffers                 16   limited by MAX_VERTEX_BUFFER_COUNT
//   20          Push constants                           1
//   21-24       Descriptor sets (argument buffers)       4   limited by MAX_DESCRIPTOR_SET_COUNT
//   25          Dynamic offset buffer                    1
//
//   Total                                               23

//   Compute Bindings
//   ----------------------
//   Bindings    Buffer name                          Count
//   ------------------------------------------------------
//   0-3         SSBO buffers                             4   MAX_SSBO_COUNT
//   20          Push constants                           1
//   21-24       Descriptor sets (argument buffers)       4   limited by MAX_DESCRIPTOR_SET_COUNT
//   25          Dynamic offset buffer                    1
//
//   Total                                               10

// The total number of vertex buffer "slots" that the Metal backend can bind.
// + 1 to account for the zero buffer, a placeholder buffer used internally by the Metal backend.
// MAX_VERTEX_BUFFER_COUNT represents the max number of vertex buffers Filament can bind.
static constexpr uint32_t LOGICAL_VERTEX_BUFFER_COUNT = MAX_VERTEX_BUFFER_COUNT + 1;

// The "zero" buffer is a small buffer for missing attributes.
static constexpr uint32_t ZERO_VERTEX_BUFFER_LOGICAL_INDEX = 0u;
static constexpr uint32_t ZERO_VERTEX_BUFFER_BINDING = 0u;

static constexpr uint32_t USER_VERTEX_BUFFER_BINDING_START = 1u;


// These constants must match the equivalent in CodeGenerator.h.
static constexpr uint32_t PUSH_CONSTANT_BUFFER_INDEX = 20u;
static constexpr uint32_t DESCRIPTOR_SET_BINDING_START = 21u;
static constexpr uint32_t DYNAMIC_OFFSET_BINDING = 25u;

// Forward declarations necessary here, definitions at end of file.
inline bool operator==(const MTLViewport& lhs, const MTLViewport& rhs);
inline bool operator!=(const MTLViewport& lhs, const MTLViewport& rhs);

// VertexDescription is part of Metal's pipeline state, and represents how vertex attributes are
// laid out in memory.
// Vertex attributes are "turned on" by setting format to something other than
// MTLVertexFormatInvalid, which is the default.
struct VertexDescription {
    struct Attribute {
        MTLVertexFormat format;     // 8 bytes
        uint32_t buffer;            // 4 bytes      a logical vertex buffer index
        uint32_t offset;            // 4 bytes
    };
    struct Layout {
        MTLVertexStepFunction step; // 8 bytes
        uint64_t stride;            // 8 bytes
    };
    Attribute attributes[MAX_VERTEX_ATTRIBUTE_COUNT] = {};      // 256 bytes
    // layouts[n] represents the layout of the vertex buffer at logical index n
    Layout layouts[LOGICAL_VERTEX_BUFFER_COUNT] = {};           // 272 bytes

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

// This assert checks that the struct is the size we expect without any "hidden" padding bytes
// inserted by the compiler.
static_assert(sizeof(VertexDescription) == 256 + 272, "VertexDescription unexpected size.");

struct BlendState {
    MTLBlendOperation alphaBlendOperation = MTLBlendOperationAdd;       // 8 bytes
    MTLBlendOperation rgbBlendOperation = MTLBlendOperationAdd;         // 8 bytes
    MTLBlendFactor destinationAlphaBlendFactor = MTLBlendFactorZero;    // 8 bytes
    MTLBlendFactor destinationRGBBlendFactor = MTLBlendFactorZero;      // 8 bytes
    MTLBlendFactor sourceAlphaBlendFactor = MTLBlendFactorOne;          // 8 bytes
    MTLBlendFactor sourceRGBBlendFactor = MTLBlendFactorOne;            // 8 bytes
    bool blendingEnabled = false;                                       // 1 byte
    char padding[7] = { 0 };                                            // 7 bytes

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

// This assert checks that the struct is the size we expect without any "hidden" padding bytes
// inserted by the compiler.
static_assert(sizeof(BlendState) == 56, "BlendState is unexpected size.");

// StateCache caches Metal state objects using StateType as a key.
// MetalType is the corresponding Metal API type.
// StateCreator is a functor that creates a new state of type MetalType.
// HashFn is a functor that hashes StateType.
template<typename StateType,
         typename MetalType,
         typename StateCreator,
         typename HashFn = utils::hash::MurmurHashFn<StateType>>
class StateCache {

    using MapType = tsl::robin_map<StateType, MetalType, HashFn>;

public:

    StateCache() = default;

    StateCache(const StateCache&) = delete;
    StateCache& operator=(const StateCache&) = delete;

    void setDevice(id<MTLDevice> device) noexcept { mDevice = device; }

    void removeIf(utils::Invocable<bool(const StateType&)> fn) noexcept {
        typename MapType::const_iterator it = mStateCache.begin();
        while (it != mStateCache.end()) {
            const auto& [key, _] = *it;
            if (UTILS_UNLIKELY(fn(key))) {
                it = mStateCache.erase(it);
            } else {
                ++it;
            }
        }
    }

    MetalType getOrCreateState(const StateType& state) noexcept {
        assert_invariant(mDevice);

        // Check if a valid state already exists in the cache.
        auto iter = mStateCache.find(state);
        if (UTILS_LIKELY(iter != mStateCache.end())) {
            auto foundState = iter.value();
            return foundState;
        }

        // If we reach this point, we couldn't find one in the cache; create a new one.
        const auto& metalObject = creator(mDevice, state);
        assert_invariant(metalObject);

        mStateCache.emplace(std::make_pair(
            state,
            metalObject
        ));

        return metalObject;
    }

private:

    StateCreator creator;
    id<MTLDevice> mDevice = nil;

    MapType mStateCache;

};

// StateTracker keeps track of state changes made to a Metal command encoder.
// Different kinds of state, like pipeline state, uniform buffer state, etc., are passed to the
// current Metal command encoder and persist throughout the lifetime of the encoder (a frame).
// StateTracker is used to prevent calling redundant state change methods.
template <typename StateType, typename StateEqual = std::equal_to<StateType>>
class StateTracker {
public:

    // Call to force the state to dirty at the beginning of each frame, as all state must be
    // re-bound.
    void invalidate() noexcept { mStateDirty = true; }

    void updateState(const StateType& newState) noexcept {
        if (!StateEqual()(mCurrentState, newState)) {
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

struct MetalPipelineState {
    id<MTLFunction> vertexFunction = nil;                                      // 8 bytes
    id<MTLFunction> fragmentFunction = nil;                                    // 8 bytes
    VertexDescription vertexDescription;                                       // 528 bytes
    MTLPixelFormat colorAttachmentPixelFormat[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = { MTLPixelFormatInvalid };  // 64 bytes
    MTLPixelFormat depthAttachmentPixelFormat = MTLPixelFormatInvalid;         // 8 bytes
    MTLPixelFormat stencilAttachmentPixelFormat = MTLPixelFormatInvalid;       // 8 bytes
    NSUInteger sampleCount = 1;                                                // 8 bytes
    BlendState blendState;                                                     // 56 bytes
    bool colorWrite = true;                                                    // 1 byte
    bool alphaToCoverage = false;                                              // 1 byte
    char padding[6] = { 0 };                                                   // 6 bytes

    bool operator==(const MetalPipelineState& rhs) const noexcept {
        return (
                this->vertexFunction == rhs.vertexFunction &&
                this->fragmentFunction == rhs.fragmentFunction &&
                this->vertexDescription == rhs.vertexDescription &&
                std::equal(this->colorAttachmentPixelFormat, this->colorAttachmentPixelFormat + MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT,
                        rhs.colorAttachmentPixelFormat) &&
                this->depthAttachmentPixelFormat == rhs.depthAttachmentPixelFormat &&
                this->stencilAttachmentPixelFormat == rhs.stencilAttachmentPixelFormat &&
                this->sampleCount == rhs.sampleCount &&
                this->blendState == rhs.blendState &&
                this->colorWrite == rhs.colorWrite &&
                this->alphaToCoverage == rhs.alphaToCoverage
        );
    }

    bool operator!=(const MetalPipelineState& rhs) const noexcept {
        return !operator==(rhs);
    }
};

// This assert checks that the struct is the size we expect without any "hidden" padding bytes
// inserted by the compiler.
static_assert(sizeof(MetalPipelineState) == 696, "MetalPipelineState unexpected size.");

struct PipelineStateCreator {
    id<MTLRenderPipelineState> operator()(id<MTLDevice> device, const MetalPipelineState& state)
            noexcept;
};

using PipelineStateTracker = StateTracker<MetalPipelineState>;

using PipelineStateCache = StateCache<MetalPipelineState, id<MTLRenderPipelineState>,
        PipelineStateCreator>;

// Depth-stencil State

struct DepthStencilState {
    struct StencilDescriptor {
        MTLCompareFunction stencilCompare = MTLCompareFunctionAlways;                   // 8 bytes
        MTLStencilOperation stencilOperationStencilFail = MTLStencilOperationKeep;      // 8 bytes
        MTLStencilOperation stencilOperationDepthFail = MTLStencilOperationKeep;        // 8 bytes
        MTLStencilOperation stencilOperationDepthStencilPass = MTLStencilOperationKeep; // 8 bytes
        uint32_t readMask = 0xFFFF;                                                     // 4 bytes
        uint32_t writeMask = 0xFFFF;                                                    // 4 bytes

        bool operator==(const StencilDescriptor& rhs) const {
            return stencilCompare == rhs.stencilCompare &&
                   stencilOperationStencilFail == rhs.stencilOperationStencilFail &&
                   stencilOperationDepthFail == rhs.stencilOperationDepthFail &&
                   stencilOperationDepthStencilPass == rhs.stencilOperationDepthStencilPass &&
                   readMask == rhs.readMask &&
                   writeMask == rhs.writeMask;
        }

        bool operator!=(const StencilDescriptor& rhs) const {
            return !(rhs == *this);
        }
    } front, back;

    MTLCompareFunction depthCompare = MTLCompareFunctionAlways;                         // 8 bytes
    bool depthWriteEnabled = false;                                                     // 1 byte
    bool stencilWriteEnabled = false;                                                   // 1 byte
    uint8_t padding[6] = { 0 };                                                         // 6 bytes

    bool operator==(const DepthStencilState& rhs) const {
        return depthCompare == rhs.depthCompare &&
                depthWriteEnabled == rhs.depthWriteEnabled &&
                front == rhs.front &&
                back == rhs.back &&
                stencilWriteEnabled == rhs.stencilWriteEnabled;
    }

    bool operator!=(const DepthStencilState& rhs) const noexcept {
        return !operator==(rhs);
    }
};

// This assert checks that the struct is the size we expect without any "hidden" padding bytes
// inserted by the compiler.
static_assert(sizeof(DepthStencilState) == 96, "DepthStencilState unexpected size.");

struct DepthStateCreator {
    id<MTLDepthStencilState> operator()(id<MTLDevice> device, const DepthStencilState& state)
            noexcept;
};

using DepthStencilStateTracker = StateTracker<DepthStencilState>;

using DepthStencilStateCache = StateCache<DepthStencilState, id<MTLDepthStencilState>,
        DepthStateCreator>;

struct MtlScissorRectEqual {
    bool operator()(const MTLScissorRect& lhs, const MTLScissorRect& rhs) const {
        return lhs.height == rhs.height &&
            lhs.width == rhs.width &&
            lhs.x == rhs.x &&
            lhs.y == rhs.y;
    }
};
using ScissorRectStateTracker = StateTracker<MTLScissorRect, MtlScissorRectEqual>;

// Uniform buffers

class MetalBufferObject;

struct BufferState {
    MetalBufferObject* buffer = nullptr;  // 8 bytes
    uint32_t offset = 0;                  // 4 bytes
    bool bound = false;                   // 1 byte
};

// Sampler states

struct SamplerState {
    SamplerParams samplerParams;

    bool operator==(const SamplerState& rhs) const noexcept {
        return this->samplerParams == rhs.samplerParams;
    }

    bool operator!=(const SamplerState& rhs) const noexcept {
        return !operator==(rhs);
    }
};

static_assert(sizeof(SamplerState) == 4, "SamplerState unexpected size.");

struct SamplerStateCreator {
    id<MTLSamplerState> operator()(id<MTLDevice> device, const SamplerState& state) noexcept;
};

using SamplerStateCache = StateCache<SamplerState, id<MTLSamplerState>, SamplerStateCreator>;

// Raster-related state

using CullModeStateTracker = StateTracker<MTLCullMode>;
using WindingStateTracker = StateTracker<MTLWinding>;
using DepthClampStateTracker = StateTracker<MTLDepthClipMode>;

// Argument encoder

struct ArgumentEncoderState {
    NSUInteger bufferCount;
    utils::FixedCapacityVector<MTLTextureType> textureTypes;

    explicit ArgumentEncoderState(
            NSUInteger bufferCount, utils::FixedCapacityVector<MTLTextureType>&& types)
        : bufferCount(bufferCount), textureTypes(std::move(types)) {}

    bool operator==(const ArgumentEncoderState& rhs) const noexcept {
        return std::equal(textureTypes.begin(), textureTypes.end(), rhs.textureTypes.begin(),
                       rhs.textureTypes.end()) &&
                bufferCount == rhs.bufferCount;
    }

    bool operator!=(const ArgumentEncoderState& rhs) const noexcept {
        return !operator==(rhs);
    }
};

struct ArgumentEncoderHasher {
    uint32_t operator()(const ArgumentEncoderState& key) const noexcept {
        return utils::hash::murmur3((const uint32_t*)key.textureTypes.data(),
                sizeof(MTLTextureType) * key.textureTypes.size() / 4, 0);
    }
};

struct ArgumentEncoderCreator {
    id<MTLArgumentEncoder> operator()(id<MTLDevice> device, const ArgumentEncoderState& state) noexcept;
};

using ArgumentEncoderCache = StateCache<ArgumentEncoderState, id<MTLArgumentEncoder>,
        ArgumentEncoderCreator, ArgumentEncoderHasher>;

template <NSUInteger N, ShaderStage stage>
class MetalBufferBindings {
public:
    MetalBufferBindings() { invalidate(); }

    void invalidate() {
        mDirtyBuffers.reset();
        mDirtyOffsets.reset();
        for (int i = 0; i < int(N); i++) {
            mDirtyBuffers.set(i, true);
            mDirtyOffsets.set(i, true);
        }
    }
    void setBuffer(const id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index);
    void bindBuffers(id<MTLCommandEncoder> encoder, NSUInteger startIndex);

private:
    static_assert(N <= 8);
    std::array<__weak id<MTLBuffer>, N> mBuffers = { nil };
    std::array<NSUInteger, N> mOffsets = { 0 };
    utils::bitset8 mDirtyBuffers;
    utils::bitset8 mDirtyOffsets;
};

} // namespace backend
} // namespace filament

#endif //TNT_METAL_STATE_H
