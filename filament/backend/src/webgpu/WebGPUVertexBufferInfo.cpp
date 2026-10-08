/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "WebGPUVertexBufferInfo.h"

#include "DriverBase.h"

#include "webgpu/utils/VertexHelper.h"

#include <backend/DriverEnums.h>

#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace filament::backend {

namespace {

constexpr uint32_t INVALID_SLOT_INDEX = MAX_VERTEX_BUFFER_COUNT;

struct AttributeInfo final {
    uint8_t slot = INVALID_SLOT_INDEX;
    wgpu::VertexAttribute attribute = {};
    AttributeInfo()
        : slot{ INVALID_SLOT_INDEX },
          attribute({}) {}
    AttributeInfo(uint8_t slot, wgpu::VertexAttribute attribute)
        : slot{ slot },
          attribute{ attribute } {}
};

/**
 * Creates the necessary vertex buffer layouts (+ a WebGPU slot per layout) given the input
 * attributes provided by Filament, as well as populates the outAttributeInfos, which associates
 * attributes to slots.
 *
 * Each slot also gets a repack plan (see webgpuutils::VertexSlotRepack). The vertex buffer layouts
 * and attributes always describe the WebGPU layout from that plan, which differs from the source
 * layout when the source formats, offsets or stride are not directly usable by WebGPU. In that
 * case WebGPUBufferObject converts the source data when it is attached to a vertex buffer.
 *
 * NOTE: At this point, the vertex buffer layouts do not have attribute information. That needs
 *       to get updated in a subsequent step
 *
 * @param attributes Input vertex attribute information from Filament
 * @param deviceMaxVertexBuffers The device's limit of vertex buffers
 * @param outWebGPUSlotBindingInfos The WebGPU slot bindings to be used by the WebGPU driver to set
 * vertex buffers in the WebGPU API.
 * @param outVertexBufferLayouts The partially populated vertex buffer layouts to eventually pass to
 * WebGPU. The attribute count and attributes pointer is not yet populated at this time. This must
 * be done after this function.
 * @param outAttributeInfos Information about all the vertex attributes to be used, actually
 * associated with a vertex buffer, and the slot each one belongs
 * @param outActualAttributeCount The number of entries populated in outAttributeInfos
 */
void createBufferLayoutsBindingSlotsAndAttributeInfos(AttributeArray const& attributes,
        const uint32_t deviceMaxVertexBuffers,
        std::vector<WebGPUVertexBufferInfo::WebGPUSlotBindingInfo>& outWebGPUSlotBindingInfos,
        std::vector<wgpu::VertexBufferLayout>& outVertexBufferLayouts,
        std::array<AttributeInfo, MAX_VERTEX_ATTRIBUTE_COUNT>& outAttributeInfos,
        uint8_t& outActualAttributeCount) {
    uint8_t currentWebGPUSlotIndex = 0;
    uint8_t currentAttributeIndex = 0;
    outVertexBufferLayouts.reserve(MAX_VERTEX_BUFFER_COUNT);
    outWebGPUSlotBindingInfos.reserve(outVertexBufferLayouts.capacity());

    // Pass 1: assign every used attribute to a WebGPU slot, based on its source buffer, stride
    // and offset.
    std::array<uint8_t, MAX_VERTEX_ATTRIBUTE_COUNT> attributeSlots{};
    attributeSlots.fill(INVALID_SLOT_INDEX);
    // Find a valid attribute to use as a dummy for unused slots (typically POSITION at index 0)
    uint32_t dummyAttributeIndex = MAX_VERTEX_ATTRIBUTE_COUNT;
    for (uint32_t attributeIndex = 0; attributeIndex < attributes.size(); ++attributeIndex) {
        Attribute const& attribute = attributes[attributeIndex];
        if (attribute.buffer == Attribute::BUFFER_UNUSED) {
            continue;
        }
        if (dummyAttributeIndex == MAX_VERTEX_ATTRIBUTE_COUNT) {
            dummyAttributeIndex = attributeIndex;
        }
        uint8_t existingSlot = INVALID_SLOT_INDEX;
        for (uint32_t slot = 0; slot < currentWebGPUSlotIndex; slot++) {
            WebGPUVertexBufferInfo::WebGPUSlotBindingInfo const& info =
                    outWebGPUSlotBindingInfos[slot];
            if (info.sourceBufferIndex == attribute.buffer && info.stride == attribute.stride &&
                    attribute.offset >= info.bufferOffset &&
                    ((attribute.offset - info.bufferOffset) < attribute.stride)) {
                existingSlot = slot;
                break;
            }
        }
        if (existingSlot == INVALID_SLOT_INDEX) {
            FILAMENT_CHECK_PRECONDITION(currentWebGPUSlotIndex < MAX_VERTEX_BUFFER_COUNT &&
                                        currentWebGPUSlotIndex < deviceMaxVertexBuffers)
                    << "Number of vertex buffer layouts must not exceed MAX_VERTEX_BUFFER_COUNT ("
                    << MAX_VERTEX_BUFFER_COUNT << ") or the device limit ("
                    << deviceMaxVertexBuffers << ")";
            existingSlot = currentWebGPUSlotIndex++;
            outWebGPUSlotBindingInfos.push_back({ .sourceBufferIndex = attribute.buffer,
                .bufferOffset = attribute.offset,
                .stride = attribute.stride });
            outVertexBufferLayouts.push_back({ .stepMode = wgpu::VertexStepMode::Vertex });
        }
        attributeSlots[attributeIndex] = existingSlot;
    }

    // Pass 2: compute the WebGPU layout of every slot, converting formats, offsets and the stride
    // where the source layout is not directly usable.
    std::array<uint32_t, MAX_VERTEX_ATTRIBUTE_COUNT> dstOffsets{};
    std::array<wgpu::VertexFormat, MAX_VERTEX_ATTRIBUTE_COUNT> dstFormats{};
    for (uint8_t slot = 0; slot < currentWebGPUSlotIndex; ++slot) {
        WebGPUVertexBufferInfo::WebGPUSlotBindingInfo& info = outWebGPUSlotBindingInfos[slot];
        std::array<webgpuutils::VertexSlotAttribute, MAX_VERTEX_ATTRIBUTE_COUNT> slotAttributes{};
        std::array<uint32_t, MAX_VERTEX_ATTRIBUTE_COUNT> slotAttributeIndices{};
        size_t slotAttributeCount = 0;
        for (uint32_t attributeIndex = 0; attributeIndex < attributes.size(); ++attributeIndex) {
            if (attributeSlots[attributeIndex] != slot) {
                continue;
            }
            Attribute const& attribute = attributes[attributeIndex];
            slotAttributeIndices[slotAttributeCount] = attributeIndex;
            slotAttributes[slotAttributeCount++] = {
                .srcOffset = attribute.offset - info.bufferOffset,
                .type = attribute.type,
                .flags = attribute.flags,
            };
        }
        info.repack = webgpuutils::computeSlotRepack(info.stride,
                { slotAttributes.data(), slotAttributeCount });
        for (size_t i = 0; i < slotAttributeCount; ++i) {
            dstOffsets[slotAttributeIndices[i]] = info.repack.attributes[i].dstOffset;
            dstFormats[slotAttributeIndices[i]] = info.repack.attributes[i].format;
        }
        outVertexBufferLayouts[slot].arrayStride = info.repack.dstStride;
    }

    /*
     * WebGPU Strict Validation Workaround:
     *
     * 1. Completeness: WebGPU requires ALL shader-declared attributes to exist in the Pipeline's
     *    VertexState, even if unused by a specific mesh variant (e.g., BONE_WEIGHTS).
     * 2. Stride Boundary: Offset + Format_Size <= arrayStride.
     *
     * If we omit an unused attribute, WebGPU crashes (violates 1).
     * If we pad it as Float32x4 (16 bytes) into Slot 0 (e.g., Position, stride 12), WebGPU crashes
     * (violates 2):
     *
     *   [ Slot 0 Stride: 12 bytes ]
     *   |-------POSITION--------|
     *   |------ BONE_WEIGHTS (Float32x4 = 16b) -----| <-- OVERFLOWS STRIDE!
     *
     * FIX: We alias unused attributes into Slot 0 as a 4-byte format (Float32 or Uint32).
     *
     *   [ Slot 0 Stride: 12 bytes ]
     *   |-------POSITION--------|
     *   |-BONE-| <-- (Float32 = 4b) Safely fits inside the 12-byte stride!
     *
     * WebGPU safely promotes the 4-byte Float32 into a WGSL vec4<f32> as (x, 0.0, 0.0, 1.0).
     */
    // Pass 3: emit the WebGPU attributes, aliasing unused ones into the dummy attribute's slot.
    for (uint32_t attributeIndex = 0; attributeIndex < attributes.size(); ++attributeIndex) {
        Attribute const& attribute = attributes[attributeIndex];
        if (attribute.buffer == Attribute::BUFFER_UNUSED) {
            if (dummyAttributeIndex == MAX_VERTEX_ATTRIBUTE_COUNT) {
                continue;
            }
            // HACK: Re-use the dummy buffer for disabled attributes to satisfy WebGPU validation.
            // Filament's shaders expect vec4 or uvec4. We provide a dummy format. Offset 0 always
            // fits a 4-byte format, since WebGPU strides are multiples of 4.
            const bool isInteger = attribute.flags & Attribute::FLAG_INTEGER_TARGET;
            outAttributeInfos[currentAttributeIndex++] =
                    AttributeInfo(attributeSlots[dummyAttributeIndex],
                            { .format = isInteger ? wgpu::VertexFormat::Uint8x4
                                                  : wgpu::VertexFormat::Unorm8x4,
                                .offset = 0,
                                .shaderLocation = attributeIndex });
            continue;
        }
        outAttributeInfos[currentAttributeIndex++] = AttributeInfo(attributeSlots[attributeIndex],
                { .format = dstFormats[attributeIndex],
                    .offset = dstOffsets[attributeIndex],
                    .shaderLocation = attributeIndex });
    }

    outActualAttributeCount = currentAttributeIndex;
    outVertexBufferLayouts.shrink_to_fit();
    outWebGPUSlotBindingInfos.shrink_to_fit();
}

} // namespace

WebGPUVertexBufferInfo::WebGPUVertexBufferInfo(const uint8_t bufferCount,
        const uint8_t attributeCount, AttributeArray const& attributes,
        wgpu::Limits const& deviceLimits)
    : HwVertexBufferInfo{ bufferCount, attributeCount } {
    FILAMENT_CHECK_PRECONDITION(attributeCount <= MAX_VERTEX_ATTRIBUTE_COUNT &&
                                attributeCount <= deviceLimits.maxVertexAttributes)
            << "The number of vertex attributes requested (" << attributeCount
            << ") exceeds Filament's MAX_VERTEX_ATTRIBUTE_COUNT limit ("
            << MAX_VERTEX_ATTRIBUTE_COUNT << ") and/or the device's limit ("
            << deviceLimits.maxVertexAttributes << ")";
    mVertexAttributes.reserve(MAX_VERTEX_ATTRIBUTE_COUNT); // Reserve max as we might add dummies
    if (attributeCount == 0) {
        mVertexBufferLayouts.resize(0);
        mWebGPUSlotBindingInfos.resize(0);
        return; // should not be possible, but being defensive. nothing to do otherwise
    }
    std::array<AttributeInfo, MAX_VERTEX_ATTRIBUTE_COUNT> attributeInfos{};
    uint8_t actualAttributeCount = 0;
    createBufferLayoutsBindingSlotsAndAttributeInfos(attributes, deviceLimits.maxVertexBuffers,
            mWebGPUSlotBindingInfos, mVertexBufferLayouts, attributeInfos, actualAttributeCount);
    // sort attribute infos by increasing slot (by increasing offset within each slot).
    // We do this to ensure that attributes for the same slot/layout are contiguous in
    // the vector, so the vertex buffer layout associated with these contiguous attributes
    // can directly reference them in the mVertexAttributes vector below.
    std::sort(attributeInfos.data(), attributeInfos.data() + actualAttributeCount,
            [](AttributeInfo const& first, AttributeInfo const& second) {
                if (first.slot < second.slot) {
                    return true;
                }
                if (first.slot == second.slot) {
                    if (first.attribute.offset < second.attribute.offset) {
                        return true;
                    }
                }
                return false;
            });
    // populate mVertexAttributes and update mVertexBufferLayouts to reference the correct
    // attributes in it (which will be contiguous in memory as ensured by the sorting above)...
    for (uint32_t attributeIndex = 0; attributeIndex < actualAttributeCount; ++attributeIndex) {
        AttributeInfo const& info = attributeInfos[attributeIndex];
        mVertexAttributes.push_back(info.attribute);
        if (mVertexBufferLayouts[info.slot].attributeCount == 0) {
            mVertexBufferLayouts[info.slot].attributes = &mVertexAttributes[attributeIndex];
        }
        mVertexBufferLayouts[info.slot].attributeCount++;
    }
}

size_t WebGPUVertexBufferInfo::getVertexBufferLayoutCount() const {
    return mVertexBufferLayouts.size();
}

}// namespace filament::backend
