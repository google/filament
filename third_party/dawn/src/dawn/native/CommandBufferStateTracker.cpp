// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/CommandBufferStateTracker.h"

#include <limits>
#include <optional>
#include <type_traits>
#include <utility>
#include <variant>

#include "absl/container/flat_hash_map.h"
#include "absl/container/inlined_vector.h"
#include "dawn/common/Assert.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/ComputePassEncoder.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/RenderPipeline.h"

// TODO(dawn:563): None of the error messages in this file include the buffer objects they are
// validating against. It would be nice to improve that, but difficult to do without incurring
// additional tracking costs.

namespace dawn::native {

namespace {
// Returns nullopt if all buffers in unverifiedBufferSizes are at least large enough to satisfy the
// minimum listed in pipelineMinBufferSizes, or the index of the first detected failing buffer
// otherwise.
std::optional<uint32_t> FindFirstUndersizedBuffer(
    const ityp::span<uint32_t, uint64_t> unverifiedBufferSizes,
    const std::vector<uint64_t>& pipelineMinBufferSizes) {
    DAWN_ASSERT(unverifiedBufferSizes.size() == pipelineMinBufferSizes.size());

    for (uint32_t i = 0; i < unverifiedBufferSizes.size(); ++i) {
        if (unverifiedBufferSizes[i] < pipelineMinBufferSizes[i]) {
            return i;
        }
    }

    return std::nullopt;
}

struct BufferAliasing {
    struct Entry {
        BindGroupIndex bindGroupIndex;
        BindingIndex bindingIndex;

        // Adjusted offset with dynamic offset
        uint64_t offset;
        uint64_t size;
    };
    Entry e0;
    Entry e1;
};

struct TextureAliasing {
    struct Entry {
        BindGroupIndex bindGroupIndex;
        BindingIndex bindingIndex;

        uint32_t baseMipLevel;
        uint32_t mipLevelCount;
        uint32_t baseArrayLayer;
        uint32_t arrayLayerCount;
    };
    Entry e0;
    Entry e1;
};

using WritableBindingAliasingResult = std::variant<std::monostate, BufferAliasing, TextureAliasing>;

template <typename Return>
Return FindStorageBufferBindingAliasing(const PipelineLayoutBase* pipelineLayout,
                                        const PerBindGroup<BindGroupBase*>& bindGroups,
                                        const PerBindGroup<std::vector<uint32_t>>& dynamicOffsets) {
    // If true, returns detailed validation error info. Otherwise simply returns if any binding
    // aliasing is found.
    constexpr bool kProduceDetails = std::is_same_v<Return, WritableBindingAliasingResult>;

    // Reduce the bindings array first to only preserve storage buffer bindings that could
    // potentially have ranges overlap.
    // There can at most be 8 storage buffer bindings (in default limits) per shader stage.
    absl::InlinedVector<BufferBinding, 8> storageBufferBindingsToCheck;
    absl::InlinedVector<std::pair<BindGroupIndex, BindingIndex>, 8> bufferBindingIndices;

    // Reduce the bindings array first to only preserve writable storage texture bindings that could
    // potentially have ranges overlap.
    // There can at most be 8 storage texture bindings (in default limits) per shader stage.
    absl::InlinedVector<const TextureViewBase*, 8> storageTextureViewsToCheck;
    absl::InlinedVector<std::pair<BindGroupIndex, BindingIndex>, 8> textureBindingIndices;

    for (BindGroupIndex groupIndex : pipelineLayout->GetBindGroupLayoutsMask()) {
        BindGroupLayoutInternalBase* bgl = bindGroups[groupIndex]->GetLayout();

        for (BindingIndex bindingIndex{0}; bindingIndex < bgl->GetBufferCount(); ++bindingIndex) {
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);
            // Buffer bindings are sorted to have smallest of bindingIndex.
            const BufferBindingInfo& layout =
                std::get<BufferBindingInfo>(bindingInfo.bindingLayout);

            // BindGroup validation already guarantees the buffer usage includes
            // wgpu::BufferUsage::Storage
            if (layout.type != wgpu::BufferBindingType::Storage) {
                continue;
            }

            const BufferBinding bufferBinding =
                bindGroups[groupIndex]->GetBindingAsBufferBinding(bindingIndex);

            if (bufferBinding.size == 0) {
                continue;
            }

            uint64_t adjustedOffset = bufferBinding.offset;
            // Apply dynamic offset if any.
            if (layout.hasDynamicOffset) {
                // SetBindGroup validation already guarantees offsets and sizes don't overflow.
                adjustedOffset += dynamicOffsets[groupIndex][static_cast<uint32_t>(bindingIndex)];
            }

            storageBufferBindingsToCheck.push_back(BufferBinding{
                bufferBinding.buffer,
                adjustedOffset,
                bufferBinding.size,
            });

            if constexpr (kProduceDetails) {
                bufferBindingIndices.emplace_back(groupIndex, bindingIndex);
            }
        }

        // TODO(dawn:1642): optimize: precompute start/end range of storage textures bindings.
        for (BindingIndex bindingIndex{bgl->GetBufferCount()};
             bindingIndex < bgl->GetBindingCount(); ++bindingIndex) {
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);

            const auto* layout = std::get_if<StorageTextureBindingInfo>(&bindingInfo.bindingLayout);
            if (layout == nullptr) {
                continue;
            }

            switch (layout->access) {
                case wgpu::StorageTextureAccess::WriteOnly:
                case wgpu::StorageTextureAccess::ReadWrite:
                    break;
                case wgpu::StorageTextureAccess::ReadOnly:
                    continue;
                case wgpu::StorageTextureAccess::BindingNotUsed:
                case wgpu::StorageTextureAccess::Undefined:
                default:
                    DAWN_UNREACHABLE();
            }

            const TextureViewBase* textureView =
                bindGroups[groupIndex]->GetBindingAsTextureView(bindingIndex);

            storageTextureViewsToCheck.push_back(textureView);

            if constexpr (kProduceDetails) {
                textureBindingIndices.emplace_back(groupIndex, bindingIndex);
            }
        }
    }

    // Iterate through each buffer bindings to find if any writable storage bindings aliasing
    // exists. Given that maxStorageBuffersPerShaderStage is 8, it doesn't seem too bad to do a
    // nested loop check.
    // TODO(dawn:1642): Maybe do algorithm optimization from O(N^2) to O(N*logN).
    for (size_t i = 0; i < storageBufferBindingsToCheck.size(); i++) {
        const auto& bufferBinding0 = storageBufferBindingsToCheck[i];

        for (size_t j = i + 1; j < storageBufferBindingsToCheck.size(); j++) {
            const auto& bufferBinding1 = storageBufferBindingsToCheck[j];

            if (bufferBinding0.buffer != bufferBinding1.buffer) {
                continue;
            }

            if (RangesOverlap(
                    bufferBinding0.offset, bufferBinding0.offset + bufferBinding0.size - 1,
                    bufferBinding1.offset, bufferBinding1.offset + bufferBinding1.size - 1)) {
                if constexpr (kProduceDetails) {
                    return WritableBindingAliasingResult{BufferAliasing{
                        {bufferBindingIndices[i].first, bufferBindingIndices[i].second,
                         bufferBinding0.offset, bufferBinding0.size},
                        {bufferBindingIndices[j].first, bufferBindingIndices[j].second,
                         bufferBinding1.offset, bufferBinding1.size},
                    }};
                } else {
                    return true;
                }
            }
        }
    }

    // Iterate through each texture views to find if any writable storage bindings aliasing exists.
    // Given that maxStorageTexturesPerShaderStage is 8,
    // it doesn't seem too bad to do a nested loop check.
    // TODO(dawn:1642): Maybe do algorithm optimization from O(N^2) to O(N*logN).
    for (size_t i = 0; i < storageTextureViewsToCheck.size(); i++) {
        const TextureViewBase* textureView0 = storageTextureViewsToCheck[i];

        DAWN_ASSERT(textureView0->GetAspects() == Aspect::Color);

        uint32_t baseMipLevel0 = textureView0->GetBaseMipLevel();
        uint32_t mipLevelCount0 = textureView0->GetLevelCount();
        uint32_t baseArrayLayer0 = textureView0->GetBaseArrayLayer();
        uint32_t arrayLayerCount0 = textureView0->GetLayerCount();

        for (size_t j = i + 1; j < storageTextureViewsToCheck.size(); j++) {
            const TextureViewBase* textureView1 = storageTextureViewsToCheck[j];

            if (textureView0->GetTexture() != textureView1->GetTexture()) {
                continue;
            }

            DAWN_ASSERT(textureView1->GetAspects() == Aspect::Color);

            uint32_t baseMipLevel1 = textureView1->GetBaseMipLevel();
            uint32_t mipLevelCount1 = textureView1->GetLevelCount();
            uint32_t baseArrayLayer1 = textureView1->GetBaseArrayLayer();
            uint32_t arrayLayerCount1 = textureView1->GetLayerCount();

            if (RangesOverlap(baseMipLevel0, baseMipLevel0 + mipLevelCount0 - 1, baseMipLevel1,
                              baseMipLevel1 + mipLevelCount1 - 1) &&
                RangesOverlap(baseArrayLayer0, baseArrayLayer0 + arrayLayerCount0 - 1,
                              baseArrayLayer1, baseArrayLayer1 + arrayLayerCount1 - 1)) {
                if constexpr (kProduceDetails) {
                    return WritableBindingAliasingResult{TextureAliasing{
                        {textureBindingIndices[i].first, textureBindingIndices[i].second,
                         baseMipLevel0, mipLevelCount0, baseArrayLayer0, arrayLayerCount0},
                        {textureBindingIndices[j].first, textureBindingIndices[j].second,
                         baseMipLevel1, mipLevelCount1, baseArrayLayer1, arrayLayerCount1},
                    }};
                } else {
                    return true;
                }
            }
        }
    }

    if constexpr (kProduceDetails) {
        return WritableBindingAliasingResult();
    } else {
        return false;
    }
}

bool TextureViewsMatch(const TextureViewBase* a, const TextureViewBase* b) {
    DAWN_ASSERT(a->GetTexture() == b->GetTexture());
    // If the texture format is multiplanar, the view formats are permitted to differ (e.g., R8
    // and RG8), referring to different planes of the same YUV texture. This cannot happen in
    // OpenGL that actually needs the validation of texture views matching so it's safe for
    // backends to ignore this here. We don't allow creating multiplanar texture views directly in
    // WebGPU so this code cannot be triggered in JavaScript and only occurs hit when Chromium
    // creates a YUV texture internally.
    return (a->GetFormat().GetIndex() == b->GetFormat().GetIndex() ||
            a->GetTexture()->GetFormat().IsMultiPlanar()) &&
           a->GetDimension() == b->GetDimension() && a->GetBaseMipLevel() == b->GetBaseMipLevel() &&
           a->GetLevelCount() == b->GetLevelCount() &&
           a->GetBaseArrayLayer() == b->GetBaseArrayLayer() &&
           a->GetLayerCount() == b->GetLayerCount() && a->GetSwizzleRed() == b->GetSwizzleRed() &&
           a->GetSwizzleGreen() == b->GetSwizzleGreen() &&
           a->GetSwizzleBlue() == b->GetSwizzleBlue() &&
           a->GetSwizzleAlpha() == b->GetSwizzleAlpha();
}

using VectorOfTextureViews = absl::InlinedVector<const TextureViewBase*, 8>;

bool TextureViewsAllMatch(const VectorOfTextureViews& views) {
    DAWN_ASSERT(!views.empty());

    const TextureViewBase* first = views[0];
    for (size_t i = 1; i < views.size(); ++i) {
        if (!TextureViewsMatch(first, views[i])) {
            return false;
        }
    }
    return true;
}

}  // namespace

enum ValidationAspect {
    VALIDATION_ASPECT_PIPELINE,
    VALIDATION_ASPECT_BIND_GROUPS,
    VALIDATION_ASPECT_VERTEX_BUFFERS,
    VALIDATION_ASPECT_INDEX_BUFFER,

    VALIDATION_ASPECT_COUNT
};
static_assert(VALIDATION_ASPECT_COUNT == CommandBufferStateTracker::kNumAspects);

static constexpr CommandBufferStateTracker::ValidationAspects kDispatchAspects =
    1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS;

static constexpr CommandBufferStateTracker::ValidationAspects kDrawAspects =
    1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS |
    1 << VALIDATION_ASPECT_VERTEX_BUFFERS;

static constexpr CommandBufferStateTracker::ValidationAspects kDrawIndexedAspects =
    1 << VALIDATION_ASPECT_PIPELINE | 1 << VALIDATION_ASPECT_BIND_GROUPS |
    1 << VALIDATION_ASPECT_VERTEX_BUFFERS | 1 << VALIDATION_ASPECT_INDEX_BUFFER;

static constexpr CommandBufferStateTracker::ValidationAspects kLazyAspects =
    1 << VALIDATION_ASPECT_BIND_GROUPS | 1 << VALIDATION_ASPECT_VERTEX_BUFFERS |
    1 << VALIDATION_ASPECT_INDEX_BUFFER;

CommandBufferStateTracker::CommandBufferStateTracker() = default;

CommandBufferStateTracker::CommandBufferStateTracker(const CommandBufferStateTracker&) = default;

CommandBufferStateTracker::CommandBufferStateTracker(CommandBufferStateTracker&&) = default;

CommandBufferStateTracker::~CommandBufferStateTracker() = default;

CommandBufferStateTracker& CommandBufferStateTracker::operator=(const CommandBufferStateTracker&) =
    default;

CommandBufferStateTracker& CommandBufferStateTracker::operator=(CommandBufferStateTracker&&) =
    default;

MaybeError CommandBufferStateTracker::ValidateCanDispatch() {
    return ValidateOperation(kDispatchAspects);
}

MaybeError CommandBufferStateTracker::ValidateCanDraw() {
    return ValidateOperation(kDrawAspects);
}

MaybeError CommandBufferStateTracker::ValidateCanDrawIndexed() {
    return ValidateOperation(kDrawIndexedAspects);
}

MaybeError CommandBufferStateTracker::ValidateNoDifferentTextureViewsOnSameTexture() {
    // TODO(dawn:1855): Look into optimizations as flat_hash_map does many allocations
    absl::flat_hash_map<const TextureBase*, VectorOfTextureViews> textureToViews;

    for (BindGroupIndex groupIndex : mLastPipelineLayout->GetBindGroupLayoutsMask()) {
        BindGroupBase* bindGroup = mBindgroups[groupIndex];
        BindGroupLayoutInternalBase* bgl = bindGroup->GetLayout();

        for (BindingIndex bindingIndex{0}; bindingIndex < bgl->GetBindingCount(); ++bindingIndex) {
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);
            if (!std::holds_alternative<TextureBindingInfo>(bindingInfo.bindingLayout) &&
                !std::holds_alternative<StorageTextureBindingInfo>(bindingInfo.bindingLayout)) {
                continue;
            }

            const TextureViewBase* textureViewBase =
                bindGroup->GetBindingAsTextureView(bindingIndex);

            textureToViews[textureViewBase->GetTexture()].push_back(textureViewBase);
        }
    }

    for (const auto& it : textureToViews) {
        const TextureBase* texture = it.first;
        const VectorOfTextureViews& views = it.second;
        DAWN_INVALID_IF(
            !TextureViewsAllMatch(views),
            "In compatibility mode, %s must not have different views in a single draw/dispatch "
            "command. texture views: %s",
            texture, ityp::span<size_t, const TextureViewBase* const>(views.data(), views.size()));
    }

    return {};
}

MaybeError CommandBufferStateTracker::ValidateBufferInRangeForVertexBuffer(uint32_t vertexCount,
                                                                           uint32_t firstVertex) {
    uint64_t strideCount = static_cast<uint64_t>(firstVertex) + vertexCount;

    if (strideCount == 0) {
        // All vertex step mode buffers are always in range if stride count is zero
        return {};
    }

    RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();

    const auto& vertexBuffersUsedAsVertexBuffer =
        lastRenderPipeline->GetVertexBuffersUsedAsVertexBuffer();

    for (auto usedSlotVertex : vertexBuffersUsedAsVertexBuffer) {
        const VertexBufferInfo& vertexBuffer = lastRenderPipeline->GetVertexBuffer(usedSlotVertex);
        uint64_t arrayStride = vertexBuffer.arrayStride;
        uint64_t bufferSize = mVertexBufferSizes[usedSlotVertex];

        if (arrayStride == 0) {
            DAWN_INVALID_IF(vertexBuffer.usedBytesInStride > bufferSize,
                            "Bound vertex buffer size (%u) at slot %u with an arrayStride of 0 "
                            "is smaller than the required size for all attributes (%u)",
                            bufferSize, usedSlotVertex, vertexBuffer.usedBytesInStride);
        } else {
            DAWN_ASSERT(strideCount != 0u);
            uint64_t requiredSize = (strideCount - 1u) * arrayStride + vertexBuffer.lastStride;
            // firstVertex and vertexCount are in uint32_t,
            // arrayStride must not be larger than kMaxVertexBufferArrayStride, which is
            // currently 2048, and vertexBuffer.lastStride = max(attribute.offset +
            // sizeof(attribute.format)) with attribute.offset being no larger than
            // kMaxVertexBufferArrayStride, so by doing checks in uint64_t we avoid
            // overflows.
            DAWN_INVALID_IF(
                requiredSize > bufferSize,
                "Vertex range (first: %u, count: %u) requires a larger buffer (%u) than "
                "the "
                "bound buffer size (%u) of the vertex buffer at slot %u with stride %u.",
                firstVertex, vertexCount, requiredSize, bufferSize, usedSlotVertex, arrayStride);
        }
    }

    return {};
}

MaybeError CommandBufferStateTracker::ValidateBufferInRangeForInstanceBuffer(
    uint32_t instanceCount,
    uint32_t firstInstance) {
    uint64_t strideCount = static_cast<uint64_t>(firstInstance) + instanceCount;

    if (strideCount == 0) {
        // All instance step mode buffers are always in range if stride count is zero
        return {};
    }

    RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();

    const auto& vertexBuffersUsedAsInstanceBuffer =
        lastRenderPipeline->GetVertexBuffersUsedAsInstanceBuffer();

    for (auto usedSlotInstance : vertexBuffersUsedAsInstanceBuffer) {
        const VertexBufferInfo& vertexBuffer =
            lastRenderPipeline->GetVertexBuffer(usedSlotInstance);
        uint64_t arrayStride = vertexBuffer.arrayStride;
        uint64_t bufferSize = mVertexBufferSizes[usedSlotInstance];
        if (arrayStride == 0) {
            DAWN_INVALID_IF(vertexBuffer.usedBytesInStride > bufferSize,
                            "Bound vertex buffer size (%u) at slot %u with an arrayStride of 0 "
                            "is smaller than the required size for all attributes (%u)",
                            bufferSize, usedSlotInstance, vertexBuffer.usedBytesInStride);
        } else {
            DAWN_ASSERT(strideCount != 0u);
            uint64_t requiredSize = (strideCount - 1u) * arrayStride + vertexBuffer.lastStride;
            // firstInstance and instanceCount are in uint32_t,
            // arrayStride must not be larger than kMaxVertexBufferArrayStride, which is
            // currently 2048, and vertexBuffer.lastStride = max(attribute.offset +
            // sizeof(attribute.format)) with attribute.offset being no larger than
            // kMaxVertexBufferArrayStride, so by doing checks in uint64_t we avoid
            // overflows.
            DAWN_INVALID_IF(
                requiredSize > bufferSize,
                "Instance range (first: %u, count: %u) requires a larger buffer (%u) than "
                "the "
                "bound buffer size (%u) of the vertex buffer at slot %u with stride %u.",
                firstInstance, instanceCount, requiredSize, bufferSize, usedSlotInstance,
                arrayStride);
        }
    }

    return {};
}

MaybeError CommandBufferStateTracker::ValidateIndexBufferInRange(uint32_t indexCount,
                                                                 uint32_t firstIndex) {
    // Validate the range of index buffer
    // firstIndex and indexCount are in uint32_t, while IndexFormatSize is 2 (for
    // wgpu::IndexFormat::Uint16) or 4 (for wgpu::IndexFormat::Uint32), so by doing checks in
    // uint64_t we avoid overflows.
    DAWN_INVALID_IF(
        (static_cast<uint64_t>(firstIndex) + indexCount) * IndexFormatSize(mIndexFormat) >
            mIndexBufferSize,
        "Index range (first: %u, count: %u, format: %s) does not fit in index buffer size "
        "(%u).",
        firstIndex, indexCount, mIndexFormat, mIndexBufferSize);
    return {};
}

MaybeError CommandBufferStateTracker::ValidateOperation(ValidationAspects requiredAspects) {
    // Fast return-true path if everything is good
    ValidationAspects missingAspects = requiredAspects & ~mAspects;
    if (missingAspects.none()) {
        return {};
    }

    // Generate an error immediately if a non-lazy aspect is missing as computing lazy aspects
    // requires the pipeline to be set.
    DAWN_TRY(CheckMissingAspects(missingAspects & ~kLazyAspects));

    RecomputeLazyAspects(missingAspects);

    DAWN_TRY(CheckMissingAspects(requiredAspects & ~mAspects));

    // Validation for kMaxBindGroupsPlusVertexBuffers is skipped because it is not necessary so far.
    static_assert(kMaxBindGroups + kMaxVertexBuffers <= kMaxBindGroupsPlusVertexBuffers);

    return {};
}

void CommandBufferStateTracker::RecomputeLazyAspects(ValidationAspects aspects) {
    DAWN_ASSERT(mAspects[VALIDATION_ASPECT_PIPELINE]);
    DAWN_ASSERT((aspects & ~kLazyAspects).none());

    if (aspects[VALIDATION_ASPECT_BIND_GROUPS]) {
        bool matches = true;

        for (BindGroupIndex i : mLastPipelineLayout->GetBindGroupLayoutsMask()) {
            if (mBindgroups[i] == nullptr ||
                !mLastPipelineLayout->GetFrontendBindGroupLayout(i)->IsLayoutEqual(
                    mBindgroups[i]->GetFrontendLayout()) ||
                FindFirstUndersizedBuffer(mBindgroups[i]->GetUnverifiedBufferSizes(),
                                          (*mMinBufferSizes)[i])
                    .has_value()) {
                matches = false;
                break;
            }
        }

        if (matches) {
            // Continue checking if there is writable storage buffer binding aliasing or not
            if (FindStorageBufferBindingAliasing<bool>(mLastPipelineLayout, mBindgroups,
                                                       mDynamicOffsets)) {
                matches = false;
            }
        }

        if (matches) {
            mAspects.set(VALIDATION_ASPECT_BIND_GROUPS);
        }
    }

    if (aspects[VALIDATION_ASPECT_VERTEX_BUFFERS]) {
        RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();

        const auto& requiredVertexBuffers = lastRenderPipeline->GetVertexBuffersUsed();
        if (IsSubset(requiredVertexBuffers, mVertexBuffersUsed)) {
            mAspects.set(VALIDATION_ASPECT_VERTEX_BUFFERS);
        }
    }

    if (aspects[VALIDATION_ASPECT_INDEX_BUFFER] && IndexBufferSet()) {
        RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();
        if (!IsStripPrimitiveTopology(lastRenderPipeline->GetPrimitiveTopology()) ||
            mIndexFormat == lastRenderPipeline->GetStripIndexFormat()) {
            mAspects.set(VALIDATION_ASPECT_INDEX_BUFFER);
        }
    }
}

MaybeError CommandBufferStateTracker::CheckMissingAspects(ValidationAspects aspects) {
    if (!aspects.any()) {
        return {};
    }

    DAWN_INVALID_IF(aspects[VALIDATION_ASPECT_PIPELINE], "No pipeline set.");

    if (aspects[VALIDATION_ASPECT_INDEX_BUFFER]) {
        DAWN_INVALID_IF(!IndexBufferSet(), "Index buffer was not set.");

        RenderPipelineBase* lastRenderPipeline = GetRenderPipeline();
        wgpu::IndexFormat pipelineIndexFormat = lastRenderPipeline->GetStripIndexFormat();

        if (IsStripPrimitiveTopology(lastRenderPipeline->GetPrimitiveTopology())) {
            DAWN_INVALID_IF(
                pipelineIndexFormat == wgpu::IndexFormat::Undefined,
                "%s has a strip primitive topology (%s) but a strip index format of %s, which "
                "prevents it for being used for indexed draw calls.",
                lastRenderPipeline, lastRenderPipeline->GetPrimitiveTopology(),
                pipelineIndexFormat);

            DAWN_INVALID_IF(
                mIndexFormat != pipelineIndexFormat,
                "Strip index format (%s) of %s does not match index buffer format (%s).",
                pipelineIndexFormat, lastRenderPipeline, mIndexFormat);
        }

        // The chunk of code above should be similar to the one in |RecomputeLazyAspects|.
        // It returns the first invalid state found. We shouldn't be able to reach this line
        // because to have invalid aspects one of the above conditions must have failed earlier.
        // If this is reached, make sure lazy aspects and the error checks above are consistent.
        DAWN_UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Index buffer is invalid.");
    }

    if (aspects[VALIDATION_ASPECT_VERTEX_BUFFERS]) {
        // Try to be helpful by finding one missing vertex buffer to surface in the error message.
        const auto missingVertexBuffers =
            GetRenderPipeline()->GetVertexBuffersUsed() & ~mVertexBuffersUsed;
        DAWN_ASSERT(missingVertexBuffers.any());

        VertexBufferSlot firstMissing = ityp::Sub(GetHighestBitIndexPlusOne(missingVertexBuffers),
                                                  VertexBufferSlot(uint8_t(1)));
        return DAWN_VALIDATION_ERROR("Vertex buffer slot %u required by %s was not set.",
                                     uint8_t(firstMissing), GetRenderPipeline());
    }

    if (aspects[VALIDATION_ASPECT_BIND_GROUPS]) {
        // TODO(crbug.com/dawn/2476): Validate TextureViewDescriptor YCbCrInfo matches with that in
        // SamplerDescriptor.
        for (BindGroupIndex i : mLastPipelineLayout->GetBindGroupLayoutsMask()) {
            DAWN_ASSERT(HasPipeline());

            DAWN_INVALID_IF(mBindgroups[i] == nullptr, "No bind group set at group index %u.", i);

            BindGroupLayoutBase* requiredBGL = mLastPipelineLayout->GetFrontendBindGroupLayout(i);
            BindGroupLayoutBase* currentBGL = mBindgroups[i]->GetFrontendLayout();

            DAWN_INVALID_IF(
                requiredBGL->GetPipelineCompatibilityToken() != kExplicitPCT &&
                    currentBGL->GetPipelineCompatibilityToken() !=
                        requiredBGL->GetPipelineCompatibilityToken(),
                "The current pipeline (%s) was created with a default layout, and is not "
                "compatible with the %s set at group index %u which uses a %s that was not "
                "created by the pipeline. Either use the bind group layout returned by calling "
                "getBindGroupLayout(%u) on the pipeline when creating the bind group, or "
                "provide an explicit pipeline layout when creating the pipeline.",
                mLastPipeline, mBindgroups[i], i, currentBGL, i);

            DAWN_INVALID_IF(
                requiredBGL->GetPipelineCompatibilityToken() == kExplicitPCT &&
                    currentBGL->GetPipelineCompatibilityToken() != kExplicitPCT,
                "%s set at group index %u uses a %s which was created as part of the default "
                "layout "
                "for a different pipeline than the current one (%s), and as a result is not "
                "compatible. Use an explicit bind group layout when creating bind groups and "
                "an explicit pipeline layout when creating pipelines to share bind groups "
                "between pipelines.",
                mBindgroups[i], i, currentBGL, mLastPipeline);

            DAWN_INVALID_IF(
                requiredBGL->GetInternalBindGroupLayout() !=
                    currentBGL->GetInternalBindGroupLayout(),
                "Bind group layout %s of pipeline layout %s does not match layout %s of bind "
                "group %s set at group index %u.",
                requiredBGL, mLastPipelineLayout, currentBGL, mBindgroups[i], i);

            std::optional<uint32_t> packedIndex = FindFirstUndersizedBuffer(
                mBindgroups[i]->GetUnverifiedBufferSizes(), (*mMinBufferSizes)[i]);
            if (packedIndex.has_value()) {
                // Find the binding index for this packed index.
                BindingIndex bindingIndex{std::numeric_limits<uint32_t>::max()};
                mBindgroups[i]->ForEachUnverifiedBufferBindingIndex(
                    [&](BindingIndex candidateBindingIndex, uint32_t candidatePackedIndex) {
                        if (candidatePackedIndex == *packedIndex) {
                            bindingIndex = candidateBindingIndex;
                        }
                    });
                DAWN_ASSERT(static_cast<uint32_t>(bindingIndex) !=
                            std::numeric_limits<uint32_t>::max());

                const auto& bindingInfo = mBindgroups[i]->GetLayout()->GetBindingInfo(bindingIndex);
                const BufferBinding& bufferBinding =
                    mBindgroups[i]->GetBindingAsBufferBinding(bindingIndex);

                BindingNumber bindingNumber = bindingInfo.binding;
                const BufferBase* buffer = bufferBinding.buffer;

                uint64_t bufferSize =
                    mBindgroups[i]->GetUnverifiedBufferSizes()[packedIndex.value()];
                uint64_t minBufferSize = (*mMinBufferSizes)[i][packedIndex.value()];

                const auto& layout = std::get<BufferBindingInfo>(bindingInfo.bindingLayout);
                return DAWN_VALIDATION_ERROR(
                    "%s bound with size %u at group %u, binding %u is too small. The pipeline (%s) "
                    "requires a buffer binding which is at least %u bytes.%s",
                    buffer, bufferSize, i, bindingNumber, mLastPipeline, minBufferSize,
                    (layout.type == wgpu::BufferBindingType::Uniform
                         ? " This binding is a uniform buffer binding. It is padded to a multiple "
                           "of 16 bytes, and as a result may be larger than the associated data in "
                           "the shader source."
                         : ""));
            }
        }

        auto result = FindStorageBufferBindingAliasing<WritableBindingAliasingResult>(
            mLastPipelineLayout, mBindgroups, mDynamicOffsets);

        if (std::holds_alternative<BufferAliasing>(result)) {
            const auto& a = std::get<BufferAliasing>(result);
            return DAWN_VALIDATION_ERROR(
                "Writable storage buffer binding aliasing found between %s set at bind group index "
                "%u, binding index %u, and %s set at bind group index %u, binding index %u, with "
                "overlapping ranges (offset: %u, size: %u) and (offset: %u, size: %u) in %s.",
                mBindgroups[a.e0.bindGroupIndex], a.e0.bindGroupIndex, a.e0.bindingIndex,
                mBindgroups[a.e1.bindGroupIndex], a.e1.bindGroupIndex, a.e1.bindingIndex,
                a.e0.offset, a.e0.size, a.e1.offset, a.e1.size,
                mBindgroups[a.e0.bindGroupIndex]
                    ->GetBindingAsBufferBinding(a.e0.bindingIndex)
                    .buffer);
        } else {
            DAWN_ASSERT(std::holds_alternative<TextureAliasing>(result));
            const auto& a = std::get<TextureAliasing>(result);
            return DAWN_VALIDATION_ERROR(
                "Writable storage texture binding aliasing found between %s set at bind group "
                "index %u, binding index %u, and %s set at bind group index %u, binding index %u, "
                "with subresources (base mipmap level: %u, mip level count: %u, base array layer: "
                "%u, array layer count: %u) and (base mipmap level: %u, mip level count: %u, base "
                "array layer: %u, array layer count: %u) in %s.",
                mBindgroups[a.e0.bindGroupIndex], a.e0.bindGroupIndex, a.e0.bindingIndex,
                mBindgroups[a.e1.bindGroupIndex], a.e1.bindGroupIndex, a.e1.bindingIndex,
                a.e0.baseMipLevel, a.e0.mipLevelCount, a.e0.baseArrayLayer, a.e0.arrayLayerCount,
                a.e1.baseMipLevel, a.e1.mipLevelCount, a.e1.baseArrayLayer, a.e1.arrayLayerCount,
                mBindgroups[a.e0.bindGroupIndex]
                    ->GetBindingAsTextureView(a.e0.bindingIndex)
                    ->GetTexture());
        }

        // The chunk of code above should be similar to the one in |RecomputeLazyAspects|.
        // It returns the first invalid state found. We shouldn't be able to reach this line
        // because to have invalid aspects one of the above conditions must have failed earlier.
        // If this is reached, make sure lazy aspects and the error checks above are consistent.
        DAWN_UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Bind groups are invalid.");
    }

    DAWN_UNREACHABLE();
}

void CommandBufferStateTracker::SetComputePipeline(ComputePipelineBase* pipeline) {
    SetPipelineCommon(pipeline);
}

void CommandBufferStateTracker::SetRenderPipeline(RenderPipelineBase* pipeline) {
    SetPipelineCommon(pipeline);
}

void CommandBufferStateTracker::UnsetBindGroup(BindGroupIndex index) {
    mBindgroups[index] = nullptr;
    mAspects.reset(VALIDATION_ASPECT_BIND_GROUPS);
}
void CommandBufferStateTracker::SetBindGroup(BindGroupIndex index,
                                             BindGroupBase* bindgroup,
                                             uint32_t dynamicOffsetCount,
                                             const uint32_t* dynamicOffsets) {
    mBindgroups[index] = bindgroup;
    mDynamicOffsets[index].assign(dynamicOffsets, dynamicOffsets + dynamicOffsetCount);
    mAspects.reset(VALIDATION_ASPECT_BIND_GROUPS);
}

void CommandBufferStateTracker::SetIndexBuffer(BufferBase* buffer,
                                               wgpu::IndexFormat format,
                                               uint64_t offset,
                                               uint64_t size) {
    mIndexBuffer = buffer;
    mIndexFormat = format;
    mIndexBufferSize = size;
    mIndexBufferOffset = offset;
}

void CommandBufferStateTracker::UnsetVertexBuffer(VertexBufferSlot slot) {
    mVertexBuffersUsed.set(slot, false);
    mVertexBufferSizes[slot] = 0;
    mAspects.reset(VALIDATION_ASPECT_VERTEX_BUFFERS);
}

void CommandBufferStateTracker::SetVertexBuffer(VertexBufferSlot slot, uint64_t size) {
    mVertexBuffersUsed.set(slot);
    mVertexBufferSizes[slot] = size;
}

void CommandBufferStateTracker::SetPipelineCommon(PipelineBase* pipeline) {
    mLastPipeline = pipeline;
    mLastPipelineLayout = pipeline != nullptr ? pipeline->GetLayout() : nullptr;
    mMinBufferSizes = pipeline != nullptr ? &pipeline->GetMinBufferSizes() : nullptr;

    mAspects.set(VALIDATION_ASPECT_PIPELINE);

    // Reset lazy aspects so they get recomputed on the next operation.
    mAspects &= ~kLazyAspects;
}

BindGroupBase* CommandBufferStateTracker::GetBindGroup(BindGroupIndex index) const {
    return mBindgroups[index];
}

const std::vector<uint32_t>& CommandBufferStateTracker::GetDynamicOffsets(
    BindGroupIndex index) const {
    return mDynamicOffsets[index];
}

bool CommandBufferStateTracker::HasPipeline() const {
    return mLastPipeline != nullptr;
}

bool CommandBufferStateTracker::IndexBufferSet() const {
    return mIndexBuffer != nullptr;
}

RenderPipelineBase* CommandBufferStateTracker::GetRenderPipeline() const {
    DAWN_ASSERT(HasPipeline() && mLastPipeline->GetType() == ObjectType::RenderPipeline);
    return static_cast<RenderPipelineBase*>(mLastPipeline);
}

ComputePipelineBase* CommandBufferStateTracker::GetComputePipeline() const {
    DAWN_ASSERT(HasPipeline() && mLastPipeline->GetType() == ObjectType::ComputePipeline);
    return static_cast<ComputePipelineBase*>(mLastPipeline);
}

PipelineLayoutBase* CommandBufferStateTracker::GetPipelineLayout() const {
    return mLastPipelineLayout;
}

BufferBase* CommandBufferStateTracker::GetIndexBuffer() const {
    return mIndexBuffer;
}

wgpu::IndexFormat CommandBufferStateTracker::GetIndexFormat() const {
    return mIndexFormat;
}

uint64_t CommandBufferStateTracker::GetIndexBufferSize() const {
    return mIndexBufferSize;
}

uint64_t CommandBufferStateTracker::GetIndexBufferOffset() const {
    return mIndexBufferOffset;
}

void CommandBufferStateTracker::End() {
    mLastPipelineLayout = nullptr;
    mLastPipeline = nullptr;
    mMinBufferSizes = nullptr;
    mBindgroups.fill(nullptr);
}

}  // namespace dawn::native
