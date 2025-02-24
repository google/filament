// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/native/IndirectDrawValidationEncoder.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/ComputePassEncoder.h"
#include "dawn/native/ComputePipeline.h"
#include "dawn/native/Device.h"
#include "dawn/native/InternalPipelineStore.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/utils/WGPUHelpers.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

namespace {
// NOTE: This must match the workgroup_size attribute on the compute entry point below.
constexpr uint64_t kWorkgroupSize = 64;

// Bitmasks for BatchInfo::flags and MultiDrawConstants::flags
constexpr uint32_t kDuplicateBaseVertexInstance = 1;
constexpr uint32_t kIndexedDraw = 2;
constexpr uint32_t kValidationEnabled = 4;
constexpr uint32_t kIndirectFirstInstanceEnabled = 8;
constexpr uint32_t kUseFirstIndexToEmulateIndexBufferOffset = 16;
constexpr uint32_t kIndirectDrawCountBuffer = 32;

// Equivalent to the IndirectDraw struct defined in the shader below.
struct IndirectDraw {
    uint32_t indirectOffset;
    uint32_t numIndexBufferElementsLow;
    uint32_t numIndexBufferElementsHigh;
    uint32_t indexOffsetAsNumElements;
};
static_assert(sizeof(IndirectDraw) == sizeof(uint32_t) * 4);
static_assert(alignof(IndirectDraw) == alignof(uint32_t));

// Equivalent to the BatchInfo struct defined in the shader below.
struct BatchInfo {
    uint32_t numDraws;
    uint32_t flags;
};

// Equivalent to MultiDrawConstants struct defined in the shader below.
struct MultiDrawConstants {
    uint32_t maxDrawCount;
    uint32_t indirectOffsetInElements;
    uint32_t drawCountOffsetInElements;
    uint32_t numIndexBufferElementsLow;
    uint32_t numIndexBufferElementsHigh;
    uint32_t flags;
};

// The size, in bytes, of the IndirectDraw struct defined in the shader below.
constexpr uint32_t kIndirectDrawByteSize = sizeof(uint32_t) * 4;

// TODO(https://crbug.com/dawn/1108): Propagate validation feedback from this shader in
// various failure modes.
static const char sRenderValidationShaderSource[] = R"(

            const kWorkgroupSize = 64u;

            const kNumDrawIndirectParams = 4u;
            const kNumDrawIndexedIndirectParams = 5u;

            const kIndexCountEntry = 0u;
            const kFirstIndexEntry = 2u;

            // Bitmasks for BatchInfo::flags and MultiDrawConstants::flags
            const kDuplicateBaseVertexInstance = 1u;
            const kIndexedDraw = 2u;
            const kValidationEnabled = 4u;
            const kIndirectFirstInstanceEnabled = 8u;
            const kUseFirstIndexToEmulateIndexBufferOffset = 16u;
            const kIndirectDrawCountBuffer = 32u; // if set, drawCount is read from a buffer

            struct MultiDrawConstants {
                maxDrawCount: u32,
                indirectOffsetInElements: u32,
                drawCountOffsetInElements: u32,
                numIndexBufferElementsLow: u32,
                numIndexBufferElementsHigh: u32,
                flags : u32,
            }

            struct IndirectDraw {
                indirectOffset: u32,
                numIndexBufferElementsLow: u32,
                numIndexBufferElementsHigh: u32,
                indexOffsetAsNumElements: u32,
            }

            struct BatchInfo {
                numDraws: u32,
                flags: u32,
                draws: array<IndirectDraw>,
            }

            struct IndirectParams {
                data: array<u32>,
            }

            // We have two entry points, which use different descriptors at binding 0.
            // Even though they are overlapping, we only use one for each entry point.
            @group(0) @binding(0) var<storage, read> batch: BatchInfo;
            @group(0) @binding(0) var<storage, read> drawConstants: MultiDrawConstants;
            @group(0) @binding(1) var<storage, read_write> inputParams: IndirectParams;
            @group(0) @binding(2) var<storage, read_write> outputParams: IndirectParams;
            // Although the drawCountBuffer only has a u32 value, it is stored in a buffer
            // to allow for offsetting the buffer in the shader.
            @group(0) @binding(3) var<storage, read_write> indirectDrawCount : IndirectParams;

            fn numIndirectParamsPerDrawCallInput(flags : u32) -> u32 {
                // Indexed Draw has an extra parameter (firstIndex)
                if (bool(flags & kIndexedDraw)) {
                    return kNumDrawIndexedIndirectParams;
                }
                return kNumDrawIndirectParams;
            }

            fn numIndirectParamsPerDrawCallOutput(flags : u32) -> u32 {
                var numParams = numIndirectParamsPerDrawCallInput(flags);
                // 2 extra parameter for duplicated first/baseVertex and firstInstance
                if (bool(flags & kDuplicateBaseVertexInstance)) {
                    numParams = numParams + 2u;
                }
                return numParams;
            }

            fn fail(drawIndex: u32, flags : u32) {
                let numParams = numIndirectParamsPerDrawCallOutput(flags);
                let index = drawIndex * numParams;
                for(var i = 0u; i < numParams; i = i + 1u) {
                    outputParams.data[index + i] = 0u;
                }
            }

            fn set_pass_single(drawIndex: u32) {
                let numInputParams = numIndirectParamsPerDrawCallInput(batch.flags);
                var outIndex = drawIndex * numIndirectParamsPerDrawCallOutput(batch.flags);
                let inIndex = batch.draws[drawIndex].indirectOffset;

                // The first 2 parameter is reserved for the duplicated first/baseVertex and firstInstance

                if (bool(batch.flags & kDuplicateBaseVertexInstance)) {
                    // first/baseVertex and firstInstance are always last two parameters
                    let dupIndex = inIndex + numInputParams - 2u;
                    outputParams.data[outIndex] = inputParams.data[dupIndex];
                    outputParams.data[outIndex + 1u] = inputParams.data[dupIndex + 1u];

                    outIndex = outIndex + 2u;
                }

                for(var i = 0u; i < numInputParams; i = i + 1u) {
                    outputParams.data[outIndex + i] = inputParams.data[inIndex + i];
                }

                if (bool(batch.flags & kUseFirstIndexToEmulateIndexBufferOffset)) {
                    outputParams.data[outIndex + kFirstIndexEntry] += batch.draws[drawIndex].indexOffsetAsNumElements;
                }
            }

            fn set_pass_multi(drawIndex: u32) {
                let numInputParams = numIndirectParamsPerDrawCallInput(drawConstants.flags);
                var outIndex = drawIndex * numIndirectParamsPerDrawCallOutput(drawConstants.flags);
                let inIndex = drawIndex * numInputParams;
                let inputOffset = drawConstants.indirectOffsetInElements;

                if (bool(drawConstants.flags & kDuplicateBaseVertexInstance)) {
                    // first/baseVertex and firstInstance are always last two parameters
                    let dupIndex = inputOffset + inIndex + numInputParams - 2u;
                    outputParams.data[outIndex] = inputParams.data[dupIndex];
                    outputParams.data[outIndex + 1u] = inputParams.data[dupIndex + 1u];

                    outIndex = outIndex + 2u;
                }

                for(var i = 0u; i < numInputParams; i = i + 1u) {
                    outputParams.data[outIndex + i] = inputParams.data[inputOffset + inIndex + i];
                }
            }

            @compute @workgroup_size(kWorkgroupSize, 1, 1)
            fn validate_single_draw(@builtin(global_invocation_id) id : vec3u) {
                if (id.x >= batch.numDraws) {
                    return;
                }

                if(!bool(batch.flags & kValidationEnabled)) {
                    set_pass_single(id.x);
                    return;
                }

                let inputIndex = batch.draws[id.x].indirectOffset;
                if(!bool(batch.flags & kIndirectFirstInstanceEnabled)) {
                    // firstInstance is always the last parameter
                    let firstInstance = inputParams.data[inputIndex + numIndirectParamsPerDrawCallInput(batch.flags) - 1u];
                    if (firstInstance != 0u) {
                        fail(id.x, batch.flags);
                        return;
                    }
                }

                if (!bool(batch.flags & kIndexedDraw)) {
                    set_pass_single(id.x);
                    return;
                }

                let numIndexBufferElementsHigh = batch.draws[id.x].numIndexBufferElementsHigh;

                if (numIndexBufferElementsHigh >= 2u) {
                    // firstIndex and indexCount are both u32. The maximum possible sum of these
                    // values is 0x1fffffffe, which is less than 0x200000000. Nothing to validate.
                    set_pass_single(id.x);
                    return;
                }

                let numIndexBufferElementsLow = batch.draws[id.x].numIndexBufferElementsLow;

                let firstIndex = inputParams.data[inputIndex + kFirstIndexEntry];
                if (numIndexBufferElementsHigh == 0u &&
                    numIndexBufferElementsLow < firstIndex) {
                    fail(id.x, batch.flags);
                    return;
                }

                // Note that this subtraction may underflow, but only when
                // numIndexBufferElementsHigh is 1u. The result is still correct in that case.
                let maxIndexCount = numIndexBufferElementsLow - firstIndex;
                let indexCount = inputParams.data[inputIndex + kIndexCountEntry];
                if (indexCount > maxIndexCount) {
                    fail(id.x, batch.flags);
                    return;
                }
                set_pass_single(id.x);
            }

           @compute @workgroup_size(kWorkgroupSize, 1, 1)
            fn validate_multi_draw(@builtin(global_invocation_id) id : vec3u) {
                var drawCount = drawConstants.maxDrawCount;
                var drawCountOffset = drawConstants.drawCountOffsetInElements;

                if(bool(drawConstants.flags & kIndirectDrawCountBuffer)) {
                    let drawCountInBuffer = indirectDrawCount.data[drawCountOffset];
                    drawCount = min(drawCountInBuffer, drawCount);
                }

                if (id.x >= drawCount) {
                    return;
                }

                if(!bool(drawConstants.flags & kValidationEnabled)) {
                    set_pass_multi(id.x);
                    return;
                }

                if (!bool(drawConstants.flags & kIndexedDraw)) {
                    set_pass_multi(id.x);
                    return;
                }

                let numIndexBufferElementsHigh = drawConstants.numIndexBufferElementsHigh;

                if (numIndexBufferElementsHigh >= 2u) {
                    // firstIndex and indexCount are both u32. The maximum possible sum of these
                    // values is 0x1fffffffe, which is less than 0x200000000. Nothing to validate.
                    set_pass_multi(id.x);
                    return;
                }

                let numIndexBufferElementsLow = drawConstants.numIndexBufferElementsLow;
                let inputOffset = drawConstants.indirectOffsetInElements;
                let firstIndex = inputParams.data[inputOffset + id.x * numIndirectParamsPerDrawCallInput(drawConstants.flags) + kFirstIndexEntry];
                if (numIndexBufferElementsHigh == 0u &&
                    numIndexBufferElementsLow < firstIndex) {
                    fail(id.x, drawConstants.flags);
                    return;
                }

                // Note that this subtraction may underflow, but only when
                // numIndexBufferElementsHigh is 1u. The result is still correct in that case.
                let maxIndexCount = numIndexBufferElementsLow - firstIndex;
                let indexCount = inputParams.data[inputOffset + id.x * numIndirectParamsPerDrawCallInput(drawConstants.flags) + kIndexCountEntry];
                if (indexCount > maxIndexCount) {
                    fail(id.x, drawConstants.flags);
                    return;
                }
                set_pass_multi(id.x);

            }


        )";

static constexpr uint32_t GetOutputIndirectDrawSize(IndirectDrawMetadata::DrawType drawType,
                                                    bool duplicateBaseVertexInstance) {
    uint32_t drawSize = drawType == IndirectDrawMetadata::DrawType::Indexed
                            ? kDrawIndexedIndirectSize
                            : kDrawIndirectSize;
    if (duplicateBaseVertexInstance) {
        drawSize += 2 * sizeof(uint32_t);
    }
    return drawSize;
}

ResultOrError<dawn::Ref<ComputePipelineBase>> CreateRenderValidationPipelines(
    DeviceBase* device,
    const char* entryPoint,
    std::initializer_list<dawn::native::utils::BindingLayoutEntryInitializationHelper> entries) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();

    // If we need to apply the index buffer offset to the first index then
    // we can't handle buffers larger than 4gig otherwise we'll overflow first_index
    // which is a 32bit value.
    //
    // When a buffer is less than 4gig the largest index buffer offset you can pass to
    // SetIndexBuffer is 0xffff_fffe. Otherwise you'll get a validation error. This
    // is converted to count of indices and so at most 0x7fff_ffff.
    //
    // The largest valid first_index would be 0x7fff_ffff. Anything larger will fail
    // the validation used in this compute shader and the validated indirect buffer
    // will have 0,0,0,0,0.
    //
    // Adding 0x7fff_ffff + 0x7fff_ffff does not overflow so as long as we keep
    // maxBufferSize < 4gig we're safe.
    DAWN_ASSERT(!device->ShouldApplyIndexBufferOffsetToFirstIndex() ||
                device->GetLimits().v1.maxBufferSize < 0x1'0000'0000u);

    // Create compute shader module if not cached before.
    if (store->indirectDrawValidationShader == nullptr) {
        DAWN_TRY_ASSIGN(store->indirectDrawValidationShader,
                        utils::CreateShaderModule(device, sRenderValidationShaderSource));
    }

    Ref<BindGroupLayoutBase> bindGroupLayout;
    DAWN_TRY_ASSIGN(bindGroupLayout, utils::MakeBindGroupLayout(device, entries,
                                                                /* allowInternalBinding */ true));

    Ref<PipelineLayoutBase> pipelineLayout;
    DAWN_TRY_ASSIGN(pipelineLayout, utils::MakeBasicPipelineLayout(device, bindGroupLayout));

    ComputePipelineDescriptor computePipelineDescriptor = {};
    computePipelineDescriptor.layout = pipelineLayout.Get();
    computePipelineDescriptor.compute.module = store->indirectDrawValidationShader.Get();
    computePipelineDescriptor.compute.entryPoint = entryPoint;

    dawn::Ref<ComputePipelineBase> pipeline;
    DAWN_TRY_ASSIGN(pipeline, device->CreateComputePipeline(&computePipelineDescriptor));

    return pipeline;
}

ResultOrError<ComputePipelineBase*> GetOrCreateIndirectDrawValidationPipeline(DeviceBase* device) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();

    if (store->indirectDrawValidationPipeline == nullptr) {
        DAWN_TRY_ASSIGN(
            store->indirectDrawValidationPipeline,
            CreateRenderValidationPipelines(
                device, "validate_single_draw",
                {
                    {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage},
                    {1, wgpu::ShaderStage::Compute, kInternalStorageBufferBinding},
                    {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                }));
    }
    return store->indirectDrawValidationPipeline.Get();
}

ResultOrError<ComputePipelineBase*> GetOrCreateMultiDrawValidationPipeline(DeviceBase* device) {
    InternalPipelineStore* store = device->GetInternalPipelineStore();

    if (store->multiDrawValidationPipeline == nullptr) {
        DAWN_TRY_ASSIGN(
            store->multiDrawValidationPipeline,
            CreateRenderValidationPipelines(
                device, "validate_multi_draw",
                {
                    {0, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage},
                    {1, wgpu::ShaderStage::Compute, kInternalStorageBufferBinding},
                    {2, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage},
                    {3, wgpu::ShaderStage::Compute, kInternalStorageBufferBinding},
                }));
    }
    return store->multiDrawValidationPipeline.Get();
}

size_t GetBatchDataSize(uint32_t numDraws) {
    return sizeof(BatchInfo) + (numDraws * kIndirectDrawByteSize);
}

}  // namespace

uint32_t ComputeMaxDrawCallsPerIndirectValidationBatch(const CombinedLimits& limits) {
    const uint64_t batchDrawCallLimitByDispatchSize =
        static_cast<uint64_t>(limits.v1.maxComputeWorkgroupsPerDimension) * kWorkgroupSize;
    const uint64_t batchDrawCallLimitByStorageBindingSize =
        (limits.v1.maxStorageBufferBindingSize - sizeof(BatchInfo)) / kIndirectDrawByteSize;
    return static_cast<uint32_t>(
        std::min({batchDrawCallLimitByDispatchSize, batchDrawCallLimitByStorageBindingSize,
                  uint64_t(std::numeric_limits<uint32_t>::max())}));
}

MaybeError EncodeIndirectDrawValidationCommands(DeviceBase* device,
                                                CommandEncoder* commandEncoder,
                                                RenderPassResourceUsageTracker* usageTracker,
                                                IndirectDrawMetadata* indirectDrawMetadata) {
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());
    // Since encoding validation commands may create new objects, verify that the device is alive.
    // TODO(dawn:1199): This check is obsolete if device loss causes device.destroy().
    //   - This function only happens within the context of a TryEncode which would catch the
    //     same issue if device loss implied device.destroy().
    DAWN_TRY(device->ValidateIsAlive());

    // Metal converts the multi draws into an ICB and validates the indirect draw calls if needed at
    // the same time.
    bool skipMultiDrawValidation = device->BackendWillValidateMultiDraw();

    struct Batch {
        raw_ptr<const IndirectDrawMetadata::IndirectValidationBatch> metadata;
        uint64_t dataBufferOffset;
        uint64_t dataSize;
        uint64_t inputIndirectOffset;
        uint64_t inputIndirectSize;
        uint64_t outputParamsOffset;
        uint64_t outputParamsSize;
        raw_ptr<BatchInfo, AllowPtrArithmetic> batchInfo;
    };

    struct Pass {
        uint32_t flags;
        raw_ptr<BufferBase> inputIndirectBuffer;
        IndirectDrawMetadata::DrawType drawType;
        uint64_t outputParamsSize = 0;
        uint64_t batchDataSize = 0;
        std::unique_ptr<void, void (*)(void*)> batchData{nullptr, std::free};
        std::vector<Batch> batches;
    };

    // First stage is grouping all batches into passes. We try to pack as many batches into a
    // single pass as possible. Batches can be grouped together as long as they're validating
    // data from the same indirect buffer and draw type, but they may still be split into
    // multiple passes if the number of draw calls in a pass would exceed some (very high)
    // upper bound.
    uint64_t outputParamsSize = 0;
    std::vector<Pass> passes;
    IndirectDrawMetadata::IndexedIndirectBufferValidationInfoMap& bufferInfoMap =
        *indirectDrawMetadata->GetIndexedIndirectBufferValidationInfo();

    const std::vector<IndirectDrawMetadata::IndirectMultiDraw>& multiDraws =
        indirectDrawMetadata->GetIndirectMultiDraws();

    // Nothing to validate.
    if (bufferInfoMap.empty() && multiDraws.empty()) {
        return {};
    }

    const uint64_t maxStorageBufferBindingSize = device->GetLimits().v1.maxStorageBufferBindingSize;
    const uint32_t minStorageBufferOffsetAlignment =
        device->GetLimits().v1.minStorageBufferOffsetAlignment;

    const bool applyIndexBufferOffsetToFirstIndex =
        device->ShouldApplyIndexBufferOffsetToFirstIndex();

    for (auto& [config, validationInfo] : bufferInfoMap) {
        const uint64_t indirectDrawCommandSize =
            config.drawType == IndirectDrawMetadata::DrawType::Indexed ? kDrawIndexedIndirectSize
                                                                       : kDrawIndirectSize;

        uint64_t outputIndirectSize =
            GetOutputIndirectDrawSize(config.drawType, config.duplicateBaseVertexInstance);

        for (const IndirectDrawMetadata::IndirectValidationBatch& batch :
             validationInfo.GetBatches()) {
            const uint64_t minOffsetFromAlignedBoundary =
                batch.minOffset % minStorageBufferOffsetAlignment;
            const uint64_t minOffsetAlignedDown = batch.minOffset - minOffsetFromAlignedBoundary;

            Batch newBatch;
            newBatch.metadata = &batch;
            newBatch.dataSize = GetBatchDataSize(batch.draws.size());
            newBatch.inputIndirectOffset = minOffsetAlignedDown;
            newBatch.inputIndirectSize =
                batch.maxOffset + indirectDrawCommandSize - minOffsetAlignedDown;

            newBatch.outputParamsSize = batch.draws.size() * outputIndirectSize;
            newBatch.outputParamsOffset = Align(outputParamsSize, minStorageBufferOffsetAlignment);
            outputParamsSize = newBatch.outputParamsOffset + newBatch.outputParamsSize;
            if (outputParamsSize > maxStorageBufferBindingSize) {
                return DAWN_INTERNAL_ERROR("Too many drawIndexedIndirect calls to validate");
            }

            Pass* currentPass = passes.empty() ? nullptr : &passes.back();
            if (currentPass &&
                reinterpret_cast<uintptr_t>(currentPass->inputIndirectBuffer.get()) ==
                    config.inputIndirectBufferPtr &&
                currentPass->drawType == config.drawType) {
                uint64_t nextBatchDataOffset =
                    Align(currentPass->batchDataSize, minStorageBufferOffsetAlignment);
                uint64_t newPassBatchDataSize = nextBatchDataOffset + newBatch.dataSize;
                if (newPassBatchDataSize <= maxStorageBufferBindingSize) {
                    // We can fit this batch in the current pass.
                    newBatch.dataBufferOffset = nextBatchDataOffset;
                    currentPass->batchDataSize = newPassBatchDataSize;
                    currentPass->batches.push_back(newBatch);
                    continue;
                }
            }

            // We need to start a new pass for this batch.
            newBatch.dataBufferOffset = 0;

            Pass newPass{};
            newPass.inputIndirectBuffer = validationInfo.GetIndirectBuffer();
            newPass.drawType = config.drawType;
            newPass.batchDataSize = newBatch.dataSize;
            newPass.batches.push_back(newBatch);
            newPass.flags = 0;
            if (config.duplicateBaseVertexInstance) {
                newPass.flags |= kDuplicateBaseVertexInstance;
            }
            if (config.drawType == IndirectDrawMetadata::DrawType::Indexed) {
                newPass.flags |= kIndexedDraw;

                if (applyIndexBufferOffsetToFirstIndex) {
                    newPass.flags |= kUseFirstIndexToEmulateIndexBufferOffset;
                }
            }
            if (device->IsValidationEnabled()) {
                newPass.flags |= kValidationEnabled;
            }
            if (device->HasFeature(Feature::IndirectFirstInstance)) {
                newPass.flags |= kIndirectFirstInstanceEnabled;
            }
            passes.push_back(std::move(newPass));
        }
    }

    // Multi draw output params are stored after the single draw output params, so we need to
    // track the offset of the multi draw output params.
    outputParamsSize = Align(outputParamsSize, minStorageBufferOffsetAlignment);
    const uint64_t multiDrawOutputParamsOffset = outputParamsSize;

    uint64_t outputParamsSizeForMultiDraw = 0;
    if (!skipMultiDrawValidation) {
        // Calculate size of output params for multi draws
        for (auto& draw : multiDraws) {
            // Multi draw metadatas are added even if validation is disabled, because the Metal
            // backend needs to convert all multi draws into an ICB. If validation is disabled,
            // and the draw doesn't need duplication of base vertex and instance, we can skip
            // the compute pass. In general, non-indexed multi draws don't need validation.
            if ((draw.type == IndirectDrawMetadata::DrawType::NonIndexed ||
                 !device->IsValidationEnabled()) &&
                !draw.duplicateBaseVertexInstance) {
                // We will use the original indirect buffer directly as the indirect buffer.
                usageTracker->BufferUsedAs(draw.cmd->indirectBuffer.Get(),
                                           kIndirectBufferForBackendResourceTracking);
                continue;
            }
            outputParamsSizeForMultiDraw +=
                draw.cmd->maxDrawCount *
                GetOutputIndirectDrawSize(draw.type, draw.duplicateBaseVertexInstance);
            outputParamsSizeForMultiDraw =
                Align(outputParamsSizeForMultiDraw, minStorageBufferOffsetAlignment);

            if (outputParamsSizeForMultiDraw > maxStorageBufferBindingSize) {
                return DAWN_INTERNAL_ERROR("Too many multiDrawIndexedIndirect calls to validate");
            }
        }
    } else {
        // If we're skipping multi draw validation, we still need to track the indirect buffer
        // usage.
        for (auto& draw : multiDraws) {
            usageTracker->BufferUsedAs(draw.cmd->indirectBuffer.Get(),
                                       kIndirectBufferForBackendResourceTracking);
        }
    }

    outputParamsSize += outputParamsSizeForMultiDraw;

    // If there are no output params to validate, we can skip the rest of the encoding.
    // The above .empty() checks are not sufficient because there might exist non-indexed multi
    // draws, which don't need validation.
    if (outputParamsSize == 0) {
        return {};
    }

    auto* const store = device->GetInternalPipelineStore();
    ScratchBuffer& outputParamsBuffer = store->scratchIndirectStorage;
    ScratchBuffer& batchDataBuffer = store->scratchStorage;

    uint64_t requiredBatchDataBufferSize = 0;
    for (const Pass& pass : passes) {
        requiredBatchDataBufferSize = std::max(requiredBatchDataBufferSize, pass.batchDataSize);
    }
    // Needs to at least be able to store a MultiDrawConstants struct for the multi draw
    // validation.
    requiredBatchDataBufferSize =
        std::max(requiredBatchDataBufferSize, static_cast<uint64_t>(sizeof(MultiDrawConstants)));

    DAWN_TRY(outputParamsBuffer.EnsureCapacity(outputParamsSize));
    DAWN_TRY(batchDataBuffer.EnsureCapacity(requiredBatchDataBufferSize));

    // We swap the indirect buffer used so we need to explicitly add the usage.
    // `outputParamsBuffer` is an internal buffer so we don't need to validate it against the
    // resource usage scope rules.
    usageTracker->BufferUsedAs(outputParamsBuffer.GetBuffer(),
                               kIndirectBufferForBackendResourceTracking);

    // Now we allocate and populate host-side batch data to be copied to the GPU.
    for (Pass& pass : passes) {
        // We use std::malloc here because it guarantees maximal scalar alignment.
        pass.batchData = {std::malloc(pass.batchDataSize), std::free};
        memset(pass.batchData.get(), 0, pass.batchDataSize);
        uint8_t* batchData = static_cast<uint8_t*>(pass.batchData.get());
        for (Batch& batch : pass.batches) {
            batch.batchInfo = new (&batchData[batch.dataBufferOffset]) BatchInfo();
            batch.batchInfo->numDraws = static_cast<uint32_t>(batch.metadata->draws.size());
            batch.batchInfo->flags = pass.flags;

            IndirectDraw* indirectDraw = reinterpret_cast<IndirectDraw*>(batch.batchInfo.get() + 1);
            uint64_t outputParamsOffset = batch.outputParamsOffset;
            for (auto& draw : batch.metadata->draws) {
                // The shader uses this to index an array of u32, hence the division by 4 bytes.
                indirectDraw->indirectOffset =
                    static_cast<uint32_t>((draw.inputBufferOffset - batch.inputIndirectOffset) / 4);
                // The index buffer elements are 64 bit values, and so need to be set as a
                // low uint32_t and a high uint32_t.
                indirectDraw->numIndexBufferElementsLow =
                    static_cast<uint32_t>(draw.numIndexBufferElements & 0xFFFFFFFF);
                indirectDraw->numIndexBufferElementsHigh =
                    static_cast<uint32_t>((draw.numIndexBufferElements >> 32) & 0xFFFFFFFF);

                // This is only used in the GL backend.
                indirectDraw->indexOffsetAsNumElements = draw.indexBufferOffsetInElements;
                indirectDraw++;

                draw.cmd->indirectBuffer = outputParamsBuffer.GetBuffer();
                draw.cmd->indirectOffset = outputParamsOffset;
                if (pass.flags & kIndexedDraw) {
                    outputParamsOffset += kDrawIndexedIndirectSize;
                } else {
                    outputParamsOffset += kDrawIndirectSize;
                }
                if (pass.flags & kDuplicateBaseVertexInstance) {
                    // Add the extra offset for the duplicated base vertex and instance.
                    outputParamsOffset += 2 * sizeof(uint32_t);
                }
            }
        }
    }
    if (!passes.empty()) {
        ComputePipelineBase* pipeline;
        DAWN_TRY_ASSIGN(pipeline, GetOrCreateIndirectDrawValidationPipeline(device));

        Ref<BindGroupLayoutBase> layout;
        DAWN_TRY_ASSIGN(layout, pipeline->GetBindGroupLayout(0));

        BindGroupEntry bindings[3];
        BindGroupEntry& bufferDataBinding = bindings[0];
        bufferDataBinding.binding = 0;
        bufferDataBinding.buffer = batchDataBuffer.GetBuffer();

        BindGroupEntry& inputIndirectBinding = bindings[1];
        inputIndirectBinding.binding = 1;

        BindGroupEntry& outputParamsBinding = bindings[2];
        outputParamsBinding.binding = 2;
        outputParamsBinding.buffer = outputParamsBuffer.GetBuffer();

        BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = layout.Get();
        bindGroupDescriptor.entryCount = 3;
        bindGroupDescriptor.entries = bindings;

        // Finally, we can now encode our validation and duplication passes. Each pass first
        // does a WriteBuffer to get batch and pass data over to the GPU, followed by a single
        // compute pass. The compute pass encodes a separate SetBindGroup and Dispatch command
        // for each batch.
        for (const Pass& pass : passes) {
            commandEncoder->APIWriteBuffer(batchDataBuffer.GetBuffer(), 0,
                                           static_cast<const uint8_t*>(pass.batchData.get()),
                                           pass.batchDataSize);

            Ref<ComputePassEncoder> passEncoder = commandEncoder->BeginComputePass();
            passEncoder->APISetPipeline(pipeline);

            inputIndirectBinding.buffer = pass.inputIndirectBuffer;

            for (const Batch& batch : pass.batches) {
                bufferDataBinding.offset = batch.dataBufferOffset;
                bufferDataBinding.size = batch.dataSize;
                inputIndirectBinding.offset = batch.inputIndirectOffset;
                inputIndirectBinding.size = batch.inputIndirectSize;
                outputParamsBinding.offset = batch.outputParamsOffset;
                outputParamsBinding.size = batch.outputParamsSize;

                Ref<BindGroupBase> bindGroup;
                DAWN_TRY_ASSIGN(bindGroup, device->CreateBindGroup(&bindGroupDescriptor));

                const uint32_t numDrawsRoundedUp =
                    (batch.batchInfo->numDraws + kWorkgroupSize - 1) / kWorkgroupSize;
                passEncoder->APISetBindGroup(0, bindGroup.Get());
                passEncoder->APIDispatchWorkgroups(numDrawsRoundedUp);
            }

            passEncoder->APIEnd();
        }
    }
    if (!multiDraws.empty() && !skipMultiDrawValidation) {
        ScratchBuffer& drawConstantsBuffer = store->scratchStorage;

        ComputePipelineBase* pipeline;
        DAWN_TRY_ASSIGN(pipeline, GetOrCreateMultiDrawValidationPipeline(device));

        Ref<BindGroupLayoutBase> layout;
        DAWN_TRY_ASSIGN(layout, pipeline->GetBindGroupLayout(0));

        BindGroupEntry bindings[4];

        BindGroupEntry& drawConstantsBinding = bindings[0];
        drawConstantsBinding.binding = 0;
        drawConstantsBinding.buffer = drawConstantsBuffer.GetBuffer();

        BindGroupEntry& inputIndirectBinding = bindings[1];
        inputIndirectBinding.binding = 1;

        BindGroupEntry& outputParamsBinding = bindings[2];
        outputParamsBinding.binding = 2;
        outputParamsBinding.buffer = outputParamsBuffer.GetBuffer();

        BindGroupEntry& drawCountBinding = bindings[3];
        drawCountBinding.binding = 3;

        BindGroupDescriptor bindGroupDescriptor = {};
        bindGroupDescriptor.layout = layout.Get();
        bindGroupDescriptor.entryCount = 4;
        bindGroupDescriptor.entries = bindings;

        // Start of the region for multi draw output params.
        uint64_t outputOffset = multiDrawOutputParamsOffset;

        for (auto& draw : multiDraws) {
            // If the draw meets these conditions, there is no need to run the compute pass,
            // and there is no space allocated for the output params
            if ((draw.type == IndirectDrawMetadata::DrawType::NonIndexed ||
                 !device->IsValidationEnabled()) &&
                !draw.duplicateBaseVertexInstance) {
                continue;
            }

            uint64_t numIndexBufferElements = 0;
            if (draw.type == IndirectDrawMetadata::DrawType::Indexed) {
                const size_t formatSize = IndexFormatSize(draw.indexFormat);
                numIndexBufferElements = draw.indexBufferSize / formatSize;
            }

            // Same struct for both indexed and non-indexed draws.
            MultiDrawIndirectCmd* cmd = draw.cmd;

            // Align the output offset to the minStorageBufferOffsetAlignment.

            MultiDrawConstants drawConstants;
            drawConstants.maxDrawCount = draw.cmd->maxDrawCount;
            // We need to pass the remaining offset in elements after aligning to the
            // minStorageBufferOffsetAlignment. See comment below.
            drawConstants.indirectOffsetInElements = static_cast<uint32_t>(
                (cmd->indirectOffset % minStorageBufferOffsetAlignment) / sizeof(uint32_t));
            drawConstants.drawCountOffsetInElements = static_cast<uint32_t>(
                (cmd->drawCountOffset % minStorageBufferOffsetAlignment) / sizeof(uint32_t));
            drawConstants.numIndexBufferElementsLow =
                static_cast<uint32_t>(numIndexBufferElements & 0xFFFFFFFF);
            drawConstants.numIndexBufferElementsHigh =
                static_cast<uint32_t>((numIndexBufferElements >> 32) & 0xFFFFFFFF);

            drawConstants.flags = 0;
            if (device->IsValidationEnabled()) {
                drawConstants.flags |= kValidationEnabled;
            }
            if (draw.type == IndirectDrawMetadata::DrawType::Indexed) {
                drawConstants.flags |= kIndexedDraw;
            }
            if (cmd->drawCountBuffer != nullptr) {
                drawConstants.flags |= kIndirectDrawCountBuffer;
            }
            if (draw.duplicateBaseVertexInstance) {
                drawConstants.flags |= kDuplicateBaseVertexInstance;
            }

            inputIndirectBinding.buffer = cmd->indirectBuffer.Get();
            // We can't use the offset directly because the indirect offset is guaranteed to
            // be aligned to 4 bytes, but when binding the buffer alignment requirement is
            // minStorageBufferOffsetAlignment. Instead we align the offset to the
            // minStorageBufferOffsetAlignment. Then pass the remaining offset in elements.
            inputIndirectBinding.offset =
                AlignDown(cmd->indirectOffset, minStorageBufferOffsetAlignment);

            outputParamsBinding.buffer = outputParamsBuffer.GetBuffer();
            outputParamsBinding.offset = outputOffset;
            outputParamsBinding.size =
                draw.cmd->maxDrawCount *
                GetOutputIndirectDrawSize(draw.type, draw.duplicateBaseVertexInstance);

            if (cmd->drawCountBuffer != nullptr) {
                // If the drawCountBuffer is set, we need to bind it to the bind group.
                // The drawCountBuffer is used to read the drawCount for the multi draw call.
                // If the drawCount exceeds the maxDrawCount, it will be clamped to
                // maxDrawCount.
                drawCountBinding.buffer = cmd->drawCountBuffer.Get();
                drawCountBinding.offset =
                    AlignDown(cmd->drawCountOffset, minStorageBufferOffsetAlignment);
            } else {
                // This is an unused binding.
                // Bind group entry for the drawCountBuffer is not needed however we need to
                // bind something else than nullptr to the bind group entry to avoid validation
                // errors. This buffer is never used in the shader, since there is a flag
                // (kIndirectDrawCountBuffer) to check if the drawCountBuffer is set.
                drawCountBinding.buffer = cmd->indirectBuffer.Get();
                drawCountBinding.offset = 0;
            }

            Ref<BindGroupBase> bindGroup;
            DAWN_TRY_ASSIGN(bindGroup, device->CreateBindGroup(&bindGroupDescriptor));

            commandEncoder->APIWriteBuffer(drawConstantsBuffer.GetBuffer(), 0,
                                           reinterpret_cast<const uint8_t*>(&drawConstants),
                                           sizeof(MultiDrawConstants));

            Ref<ComputePassEncoder> passEncoder = commandEncoder->BeginComputePass();
            passEncoder->APISetPipeline(pipeline);
            passEncoder->APISetBindGroup(0, bindGroup.Get());

            uint32_t workgroupCount = cmd->maxDrawCount / kWorkgroupSize;
            // Integer division rounds down so adding 1 if there is a remainder.
            workgroupCount += cmd->maxDrawCount % kWorkgroupSize == 0 ? 0 : 1;
            passEncoder->APIDispatchWorkgroups(workgroupCount);
            passEncoder->APIEnd();

            // Update the draw command to use the validated indirect buffer.
            // The drawCountBuffer doesn't need to be updated because if it exceeds the
            // maxDrawCount it will be clamped to maxDrawCount.
            cmd->indirectBuffer = outputParamsBuffer.GetBuffer();
            cmd->indirectOffset = outputOffset;

            // Proceed to the next output offset.
            outputOffset += cmd->maxDrawCount *
                            GetOutputIndirectDrawSize(draw.type, draw.duplicateBaseVertexInstance);
            outputOffset = Align(outputOffset, minStorageBufferOffsetAlignment);
        }
    }

    return {};
}
}  // namespace dawn::native
