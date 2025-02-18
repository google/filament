// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/ProgrammableEncoder.h"

#include <cstring>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

ProgrammableEncoder::ProgrammableEncoder(DeviceBase* device,
                                         StringView label,
                                         EncodingContext* encodingContext)
    : ApiObjectBase(device, label),
      mEncodingContext(encodingContext),
      mValidationEnabled(device->IsValidationEnabled()) {}

ProgrammableEncoder::ProgrammableEncoder(DeviceBase* device,
                                         EncodingContext* encodingContext,
                                         ErrorTag errorTag,
                                         StringView label)
    : ApiObjectBase(device, errorTag, label),
      mEncodingContext(encodingContext),
      mValidationEnabled(device->IsValidationEnabled()) {}

bool ProgrammableEncoder::IsValidationEnabled() const {
    return mValidationEnabled;
}

MaybeError ProgrammableEncoder::ValidateProgrammableEncoderEnd() const {
    DAWN_INVALID_IF(mDebugGroupStackSize != 0,
                    "PushDebugGroup called %u time(s) without a corresponding PopDebugGroup.",
                    mDebugGroupStackSize);
    return {};
}

void ProgrammableEncoder::APIInsertDebugMarker(StringView markerIn) {
    std::string_view marker = utils::NormalizeMessageString(markerIn);
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            InsertDebugMarkerCmd* cmd =
                allocator->Allocate<InsertDebugMarkerCmd>(Command::InsertDebugMarker);
            AddNullTerminatedString(allocator, marker, &cmd->length);

            return {};
        },
        "encoding %s.InsertDebugMarker(%s).", this, marker);
}

void ProgrammableEncoder::APIPopDebugGroup() {
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (IsValidationEnabled()) {
                DAWN_INVALID_IF(mDebugGroupStackSize == 0,
                                "PopDebugGroup called when no debug groups are currently pushed.");
            }
            allocator->Allocate<PopDebugGroupCmd>(Command::PopDebugGroup);
            mDebugGroupStackSize--;
            mEncodingContext->PopDebugGroupLabel();

            return {};
        },
        "encoding %s.PopDebugGroup().", this);
}

void ProgrammableEncoder::APIPushDebugGroup(StringView groupLabelIn) {
    std::string_view groupLabel = utils::NormalizeMessageString(groupLabelIn);
    mEncodingContext->TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            PushDebugGroupCmd* cmd =
                allocator->Allocate<PushDebugGroupCmd>(Command::PushDebugGroup);
            const char* label = AddNullTerminatedString(allocator, groupLabel, &cmd->length);

            mDebugGroupStackSize++;
            mEncodingContext->PushDebugGroupLabel(std::string_view(label, cmd->length));

            return {};
        },
        "encoding %s.PushDebugGroup(%s).", this, groupLabel);
}

MaybeError ProgrammableEncoder::ValidateSetBindGroup(BindGroupIndex index,
                                                     BindGroupBase* group,
                                                     uint32_t dynamicOffsetCountIn,
                                                     const uint32_t* dynamicOffsetsIn) const {
    DAWN_INVALID_IF(index >= kMaxBindGroupsTyped, "Bind group index (%u) exceeds the maximum (%u).",
                    index, kMaxBindGroupsTyped);

    ityp::span<BindingIndex, const uint32_t> dynamicOffsets(dynamicOffsetsIn,
                                                            BindingIndex(dynamicOffsetCountIn));

    if (group == nullptr) {
        uint32_t size = static_cast<uint32_t>(dynamicOffsets.size());
        DAWN_INVALID_IF(size != 0, "The number of dynamic offsets (%u) is not zero", size);
        return {};
    }

    DAWN_TRY(GetDevice()->ValidateObject(group));

    // Dynamic offsets count must match the number required by the layout perfectly.
    const BindGroupLayoutInternalBase* layout = group->GetLayout();
    DAWN_INVALID_IF(
        layout->GetDynamicBufferCount() != dynamicOffsets.size(),
        "The number of dynamic offsets (%u) does not match the number of dynamic buffers (%u) "
        "in %s.",
        dynamicOffsets.size(), layout->GetDynamicBufferCount(), layout);

    for (BindingIndex i{0}; i < dynamicOffsets.size(); ++i) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(i);

        // BGL creation sorts bindings such that the dynamic buffer bindings are first.
        const BufferBindingInfo& bindingLayout =
            std::get<BufferBindingInfo>(bindingInfo.bindingLayout);
        DAWN_ASSERT(bindingLayout.hasDynamicOffset);

        uint64_t requiredAlignment;
        switch (bindingLayout.type) {
            case wgpu::BufferBindingType::Uniform:
                requiredAlignment = GetDevice()->GetLimits().v1.minUniformBufferOffsetAlignment;
                break;
            case wgpu::BufferBindingType::Storage:
            case wgpu::BufferBindingType::ReadOnlyStorage:
            case kInternalStorageBufferBinding:
                requiredAlignment = GetDevice()->GetLimits().v1.minStorageBufferOffsetAlignment;
                break;
            case wgpu::BufferBindingType::BindingNotUsed:
            case wgpu::BufferBindingType::Undefined:
                DAWN_UNREACHABLE();
        }

        DAWN_INVALID_IF(!IsAligned(dynamicOffsets[i], requiredAlignment),
                        "Dynamic Offset[%u] (%u) is not %u byte aligned.", i, dynamicOffsets[i],
                        requiredAlignment);

        BufferBinding bufferBinding = group->GetBindingAsBufferBinding(i);

        // During BindGroup creation, validation ensures binding offset + binding size
        // <= buffer size.
        DAWN_ASSERT(bufferBinding.buffer->GetSize() >= bufferBinding.size);
        DAWN_ASSERT(bufferBinding.buffer->GetSize() - bufferBinding.size >= bufferBinding.offset);

        if ((dynamicOffsets[i] >
             bufferBinding.buffer->GetSize() - bufferBinding.offset - bufferBinding.size)) {
            DAWN_INVALID_IF(
                (bufferBinding.buffer->GetSize() - bufferBinding.offset) == bufferBinding.size,
                "Dynamic Offset[%u] (%u) is out of bounds of %s with a size of %u and a bound "
                "range of (offset: %u, size: %u). The binding goes to the end of the buffer "
                "even with a dynamic offset of 0. Did you forget to specify "
                "the binding's size?",
                i, dynamicOffsets[i], bufferBinding.buffer, bufferBinding.buffer->GetSize(),
                bufferBinding.offset, bufferBinding.size);

            return DAWN_VALIDATION_ERROR(
                "Dynamic Offset[%u] (%u) is out of bounds of "
                "%s with a size of %u and a bound range of (offset: %u, size: %u).",
                i, dynamicOffsets[i], bufferBinding.buffer, bufferBinding.buffer->GetSize(),
                bufferBinding.offset, bufferBinding.size);
        }
    }

    return {};
}

void ProgrammableEncoder::RecordSetBindGroup(CommandAllocator* allocator,
                                             BindGroupIndex index,
                                             BindGroupBase* group,
                                             uint32_t dynamicOffsetCount,
                                             const uint32_t* dynamicOffsets) const {
    SetBindGroupCmd* cmd = allocator->Allocate<SetBindGroupCmd>(Command::SetBindGroup);
    cmd->index = index;
    cmd->group = group;
    cmd->dynamicOffsetCount = dynamicOffsetCount;
    if (dynamicOffsetCount > 0) {
        uint32_t* offsets = allocator->AllocateData<uint32_t>(cmd->dynamicOffsetCount);
        memcpy(offsets, dynamicOffsets, dynamicOffsetCount * sizeof(uint32_t));
    }
}

}  // namespace dawn::native
