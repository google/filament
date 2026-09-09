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

#include "src/dawn/native/IndirectDrawMetadata.h"

#include <algorithm>
#include <cstdint>
#include <tuple>
#include <utility>

#include "src/dawn/common/Constants.h"
#include "src/dawn/native/IndirectDrawValidationEncoder.h"
#include "src/dawn/native/Limits.h"
#include "src/dawn/native/RenderBundle.h"

namespace dawn::native {

uint64_t ComputeMaxIndirectValidationBatchOffsetRange(const CombinedLimits& limits) {
    return limits.v1.maxStorageBufferBindingSize - limits.v1.minStorageBufferOffsetAlignment -
           kDrawIndexedIndirectSize;
}

IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::IndexedIndirectBufferValidationInfo(
    BufferBase* indirectBuffer)
    : mIndirectBuffer(indirectBuffer) {}

void IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::AddIndirectDraw(
    uint32_t maxDrawCallsPerIndirectValidationBatch,
    uint64_t maxBatchOffsetRange,
    IndirectDraw draw) {
    const uint64_t newOffset = draw.inputBufferOffset;
    auto it = mBatches.begin();
    while (it != mBatches.end()) {
        IndirectValidationBatch& batch = *it;
        if (batch.draws.size() >= maxDrawCallsPerIndirectValidationBatch) {
            // This batch is full. If its minOffset is to the right of the new offset, we can
            // just insert a new batch here.
            if (newOffset < batch.minOffset) {
                break;
            }

            // Otherwise keep looking.
            ++it;
            continue;
        }

        if (newOffset >= batch.minOffset && newOffset <= batch.maxOffset) {
            batch.draws.push_back(std::move(draw));
            return;
        }

        if (newOffset < batch.minOffset && batch.maxOffset - newOffset <= maxBatchOffsetRange) {
            // We can extend this batch to the left in order to fit the new offset.
            batch.minOffset = newOffset;
            batch.draws.push_back(std::move(draw));
            return;
        }

        if (newOffset > batch.maxOffset && newOffset - batch.minOffset <= maxBatchOffsetRange) {
            // We can extend this batch to the right in order to fit the new offset.
            batch.maxOffset = newOffset;
            batch.draws.push_back(std::move(draw));
            return;
        }

        if (newOffset < batch.minOffset) {
            // We want to insert a new batch just before this one.
            break;
        }

        ++it;
    }

    IndirectValidationBatch newBatch;
    newBatch.minOffset = newOffset;
    newBatch.maxOffset = newOffset;
    newBatch.draws.push_back(std::move(draw));

    mBatches.insert(it, std::move(newBatch));
}

void AdjustValidatedDrawIndex(std::vector<IndirectDrawMetadata::IndirectDraw>& draws,
                              std::vector<IndirectDrawMetadata::IndirectDraw>::iterator begin,
                              IndirectDrawIndex indirectDrawIndexOffset) {
    // Ensure that the validatedDrawIndex is properly offset for every newly inserted draw.
    for (auto it = begin; it != draws.end(); ++it) {
        it->validatedDrawIndex += indirectDrawIndexOffset;
    }
}

void IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::AddBatch(
    uint32_t maxDrawCallsPerIndirectValidationBatch,
    uint64_t maxBatchOffsetRange,
    const IndirectValidationBatch& newBatch,
    IndirectDrawIndex indirectDrawIndexOffset) {
    auto it = mBatches.begin();
    while (it != mBatches.end()) {
        // TODO(crbug.com/495489174): Investigate simplifying this.
        IndirectValidationBatch& batch = *it;
        uint64_t min = std::min(newBatch.minOffset, batch.minOffset);
        uint64_t max = std::max(newBatch.maxOffset, batch.maxOffset);
        if (max - min <= maxBatchOffsetRange &&
            batch.draws.size() + newBatch.draws.size() <= maxDrawCallsPerIndirectValidationBatch) {
            // This batch fits within the limits of an existing batch. Merge it.
            batch.minOffset = min;
            batch.maxOffset = max;

            auto insertedDraws =
                batch.draws.insert(batch.draws.end(), newBatch.draws.begin(), newBatch.draws.end());
            AdjustValidatedDrawIndex(batch.draws, insertedDraws, indirectDrawIndexOffset);
            return;
        }

        if (newBatch.minOffset < batch.minOffset) {
            break;
        }

        ++it;
    }
    {
        mBatches.push_back(newBatch);
        IndirectValidationBatch& batch = mBatches.back();
        AdjustValidatedDrawIndex(batch.draws, batch.draws.begin(), indirectDrawIndexOffset);
    }
}

const std::vector<IndirectDrawMetadata::IndirectValidationBatch>&
IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::GetBatches() const {
    return mBatches;
}

BufferBase* IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::GetIndirectBuffer() const {
    return mIndirectBuffer.Get();
}

IndirectDrawMetadata::IndirectDrawMetadata(const CombinedLimits& limits)
    : mMaxBatchOffsetRange(ComputeMaxIndirectValidationBatchOffsetRange(limits)),
      mMaxDrawCallsPerBatch(ComputeMaxDrawCallsPerIndirectValidationBatch(limits)) {}

IndirectDrawMetadata::~IndirectDrawMetadata() = default;

IndirectDrawMetadata::IndirectDrawMetadata(IndirectDrawMetadata&&) = default;

IndirectDrawMetadata& IndirectDrawMetadata::operator=(IndirectDrawMetadata&&) = default;

IndirectDrawMetadata::IndexedIndirectBufferValidationInfoMap*
IndirectDrawMetadata::GetIndexedIndirectBufferValidationInfo() {
    return &mIndexedIndirectBufferValidationInfo;
}

const std::vector<IndirectDrawMetadata::IndirectMultiDraw>&
IndirectDrawMetadata::GetIndirectMultiDraws() const {
    return mMultiDraws;
}

void IndirectDrawMetadata::SetValidatedIndirectDrawArgs(const IndirectDraw& draw,
                                                        BufferBase* indirectBuffer,
                                                        uint64_t indirectOffset) {
    // The first time validated args are set ensure the array of args is large enough to store all
    // of the args that will be needed.
    if (mValidatedIndirectDraws.size() < mNextIndirectDrawIndex) {
        mValidatedIndirectDraws.resize(mNextIndirectDrawIndex);
    }
    DAWN_ASSERT(draw.validatedDrawIndex < mValidatedIndirectDraws.size());

    IndirectDrawMetadata::ValidatedIndirectDraw& validatedDraw =
        mValidatedIndirectDraws[draw.validatedDrawIndex];
    validatedDraw.indirectBuffer = indirectBuffer;
    validatedDraw.indirectOffset = indirectOffset;

    // TODO(crbug.com/495489174): Currently running without validation does not populate the
    // validated indirect draws array. Setting the indirectBuffer of the command to null is used as
    // a signifier that the validated array should be used. This should be replaced in the future
    // with code that explicitly sets the validated indirect draw array in all cases.
    draw.cmd->indirectBuffer = nullptr;
    draw.cmd->indirectOffset = indirectOffset;
}

IndirectDrawMetadata::ValidatedIndirectDraw IndirectDrawMetadata::GetValidatedIndirectDraw(
    DrawIndirectCmd* cmd,
    IndirectDrawIndex indirectDrawIndex) const {
    DAWN_ASSERT(cmd);
    if (cmd->indirectBuffer != nullptr) {
        return {
            .indirectBuffer = cmd->indirectBuffer.Get(),
            .indirectOffset = cmd->indirectOffset,
        };
    }
    return mValidatedIndirectDraws[indirectDrawIndex];
}

void IndirectDrawMetadata::AddBundle(RenderBundleBase* bundle) {
    IndirectDrawIndex bundleIndirectDrawCount{0u};
    for (const auto& [config, validationInfo] :
         bundle->GetIndirectDrawMetadata().mIndexedIndirectBufferValidationInfo) {
        auto it = mIndexedIndirectBufferValidationInfo.lower_bound(config);
        if (it == mIndexedIndirectBufferValidationInfo.end() || it->first != config) {
            it = mIndexedIndirectBufferValidationInfo.emplace_hint(
                it, config,
                IndexedIndirectBufferValidationInfo(validationInfo.GetIndirectBuffer()));
        }

        // Merge the bundle's batches in.
        for (const IndirectValidationBatch& batch : validationInfo.GetBatches()) {
            it->second.AddBatch(mMaxDrawCallsPerBatch, mMaxBatchOffsetRange, batch,
                                mNextIndirectDrawIndex);
            bundleIndirectDrawCount += IndirectDrawIndex(batch.draws.size());
        }
    }
    mNextIndirectDrawIndex += bundleIndirectDrawCount;
}

void IndirectDrawMetadata::AddIndexedIndirectDraw(wgpu::IndexFormat indexFormat,
                                                  uint64_t indexBufferSize,
                                                  uint64_t indexBufferOffset,
                                                  BufferBase* indirectBuffer,
                                                  uint64_t indirectOffset,
                                                  bool duplicateBaseVertexInstance,
                                                  DrawIndexedIndirectCmd* cmd) {
    uint64_t numIndexBufferElements;
    uint64_t indexBufferOffsetInElements;
    switch (indexFormat) {
        case wgpu::IndexFormat::Uint16:
            numIndexBufferElements = indexBufferSize / 2;
            indexBufferOffsetInElements = indexBufferOffset / 2;
            break;
        case wgpu::IndexFormat::Uint32:
            numIndexBufferElements = indexBufferSize / 4;
            indexBufferOffsetInElements = indexBufferOffset / 4;
            break;
        case wgpu::IndexFormat::Undefined:
            DAWN_UNREACHABLE();
    }

    const IndexedIndirectConfig config{reinterpret_cast<uintptr_t>(indirectBuffer),
                                       duplicateBaseVertexInstance, DrawType::Indexed};
    auto it = mIndexedIndirectBufferValidationInfo.find(config);
    if (it == mIndexedIndirectBufferValidationInfo.end()) {
        auto result = mIndexedIndirectBufferValidationInfo.emplace(
            config, IndexedIndirectBufferValidationInfo(indirectBuffer));
        it = result.first;
    }
    IndirectDraw draw{};
    draw.validatedDrawIndex = mNextIndirectDrawIndex++;
    draw.inputBufferOffset = indirectOffset;
    draw.numIndexBufferElements = numIndexBufferElements;
    draw.indexBufferOffsetInElements = indexBufferOffsetInElements;
    draw.cmd = cmd;
    it->second.AddIndirectDraw(mMaxDrawCallsPerBatch, mMaxBatchOffsetRange, draw);
}

void IndirectDrawMetadata::AddIndirectDraw(BufferBase* indirectBuffer,
                                           uint64_t indirectOffset,
                                           bool duplicateBaseVertexInstance,
                                           DrawIndirectCmd* cmd) {
    const IndexedIndirectConfig config{reinterpret_cast<uintptr_t>(indirectBuffer),
                                       duplicateBaseVertexInstance, DrawType::NonIndexed};
    auto it = mIndexedIndirectBufferValidationInfo.find(config);
    if (it == mIndexedIndirectBufferValidationInfo.end()) {
        auto result = mIndexedIndirectBufferValidationInfo.emplace(
            config, IndexedIndirectBufferValidationInfo(indirectBuffer));
        it = result.first;
    }

    IndirectDraw draw{};
    draw.validatedDrawIndex = mNextIndirectDrawIndex++;
    draw.inputBufferOffset = indirectOffset;
    draw.numIndexBufferElements = 0;
    draw.cmd = cmd;
    it->second.AddIndirectDraw(mMaxDrawCallsPerBatch, mMaxBatchOffsetRange, draw);
}

void IndirectDrawMetadata::ClearIndexedIndirectBufferValidationInfo() {
    mIndexedIndirectBufferValidationInfo.clear();
}

void IndirectDrawMetadata::AddMultiDrawIndirect(wgpu::PrimitiveTopology topology,
                                                bool duplicateBaseVertexInstance,
                                                MultiDrawIndirectCmd* cmd) {
    IndirectMultiDraw multiDraw;
    multiDraw.type = DrawType::NonIndexed;
    multiDraw.cmd = cmd;
    multiDraw.topology = topology;
    multiDraw.duplicateBaseVertexInstance = duplicateBaseVertexInstance;
    mMultiDraws.push_back(multiDraw);
}

void IndirectDrawMetadata::AddMultiDrawIndexedIndirect(BufferBase* indexBuffer,
                                                       wgpu::IndexFormat indexFormat,
                                                       uint64_t indexBufferSize,
                                                       uint64_t indexBufferOffset,
                                                       wgpu::PrimitiveTopology topology,
                                                       bool duplicateBaseVertexInstance,
                                                       MultiDrawIndexedIndirectCmd* cmd) {
    IndirectMultiDraw multiDraw;
    multiDraw.type = DrawType::Indexed;
    multiDraw.indexBuffer = indexBuffer;
    multiDraw.cmd = cmd;
    multiDraw.indexBufferSize = indexBufferSize;
    multiDraw.indexBufferOffsetInBytes = indexBufferOffset;
    multiDraw.indexFormat = indexFormat;
    multiDraw.topology = topology;
    multiDraw.duplicateBaseVertexInstance = duplicateBaseVertexInstance;

    mMultiDraws.push_back(multiDraw);
}

bool IndirectDrawMetadata::IndexedIndirectConfig::operator<(
    const IndexedIndirectConfig& other) const {
    return std::tie(inputIndirectBufferPtr, duplicateBaseVertexInstance, drawType) <
           std::tie(other.inputIndirectBufferPtr, other.duplicateBaseVertexInstance,
                    other.drawType);
}

}  // namespace dawn::native
