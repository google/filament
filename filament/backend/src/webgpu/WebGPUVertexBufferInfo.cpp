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

#include "WebGPUConstants.h"

#include "DriverBase.h"
#include <backend/DriverEnums.h>

#include <utils/Panic.h>
#include <utils/ostream.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace filament::backend {

namespace {

[[nodiscard]] wgpu::VertexFormat getVertexFormat(const ElementType type, const bool normalized,
        const bool integer) {
    using VertexFormat = wgpu::VertexFormat;
    if (normalized) {
        switch (type) {
            // Single Component Types
            case ElementType::BYTE:    return VertexFormat::Snorm8;
            case ElementType::UBYTE:   return VertexFormat::Unorm8;
            case ElementType::SHORT:   return VertexFormat::Snorm16;
            case ElementType::USHORT:  return VertexFormat::Unorm16;
            // Two Component Types
            case ElementType::BYTE2:   return VertexFormat::Snorm8x2;
            case ElementType::UBYTE2:  return VertexFormat::Unorm8x2;
            case ElementType::SHORT2:  return VertexFormat::Snorm16x2;
            case ElementType::USHORT2: return VertexFormat::Unorm16x2;
            // Three Component Types
            // There is no vertex format type for 3 byte data in webgpu. Use
            // 4 byte signed normalized type and ignore the last byte.
            // TODO: This is to be verified.
            case ElementType::BYTE3:
                FWGPU_LOGW
                        << "Requested Filament vertex format BYTE3 (normalized) but getting "
                           "wgpu::VertexFormat::Snorm8x4 (no direct mapping in wgpu for x3 byte)"
                        << utils::io::endl;
                return VertexFormat::Snorm8x4; // NOT MINSPEC
            case ElementType::UBYTE3:
                FWGPU_LOGW
                        << "Requested Filament vertex format UBYTE3 (normalized) but getting "
                           "wgpu::VertexFormat::Unorm8x4 (no direct mapping in wgpu for x3 byte)"
                        << utils::io::endl;
                return VertexFormat::Unorm8x4; // NOT MINSPEC
            case ElementType::SHORT3:
                FWGPU_LOGW << "Requested Filament vertex format SHORT3 (normalized) but getting "
                              "wgpu::VertexFormat::Snorm16x4 (no direct mapping in wgpu for x3 "
                              "half/short)"
                           << utils::io::endl;
                return VertexFormat::Snorm16x4; // NOT MINSPEC
            case ElementType::USHORT3:
                FWGPU_LOGW << "Requested Filament vertex format USHORT3 (normalized) but getting "
                              "wgpu::VertexFormat::Unorm16x4 (no direct mapping in wgpu for x3 "
                              "half/short)"
                           << utils::io::endl;
                return VertexFormat::Unorm16x4; // NOT MINSPEC
            // Four Component Types
            case ElementType::BYTE4:   return VertexFormat::Snorm8x4;
            case ElementType::UBYTE4:  return VertexFormat::Unorm8x4;
            case ElementType::SHORT4:  return VertexFormat::Snorm16x4;
            case ElementType::USHORT4: return VertexFormat::Unorm16x4;
            default:
                PANIC_POSTCONDITION("Normalized format (%d enum value) does not exist in webgpu.",
                        type);
                return VertexFormat::Float32x3;
        }
    }
    switch (type) {
        // Single Component Types
        // There is no direct alternative for SSCALED in webgpu. Convert them to Float32 directly.
        // This will result in increased memory on the cpu side.
        // TODO: Is Float16 acceptable instead with some potential accuracy errors?
        case ElementType::BYTE:
            if (integer) return VertexFormat::Sint8;
            FWGPU_LOGW << "Requested Filament vertex format BYTE (float) but getting "
                          "wgpu::VertexFormat::Float16 (no direct mapping in wgpu for 8 bit float)"
                       << utils::io::endl;
            return wgpu::VertexFormat::Float16;
        case ElementType::UBYTE:
            if (integer) return VertexFormat::Uint8;
            FWGPU_LOGW << "Requested Filament vertex format UBYTE (float) but getting "
                          "wgpu::VertexFormat::Float16 (no direct mapping in wgpu for 8 bit float)"
                       << utils::io::endl;
            return VertexFormat::Float16;
        case ElementType::SHORT:
            if (integer) return VertexFormat::Sint16;
            FWGPU_LOGW << "Requested Filament vertex format SHORT (float) and getting "
                          "wgpu::VertexFormat::Float16 (potential loss of precision?)"
                       << utils::io::endl;
            return VertexFormat ::Float16;
        case ElementType::USHORT:
            if (integer) return VertexFormat::Uint16;
            FWGPU_LOGW << "Requested Filament vertex format USHORT (float) and getting "
                          "wgpu::VertexFormat::Float16 (potential loss of precision?)"
                       << utils::io::endl;
            return VertexFormat::Float16;
        case ElementType::HALF:    return VertexFormat::Float16;
        case ElementType::INT:     return VertexFormat::Sint32;
        case ElementType::UINT:    return VertexFormat::Uint32;
        case ElementType::FLOAT:   return VertexFormat::Float32;
        // Two Component Types
        case ElementType::BYTE2:
            if (integer) return VertexFormat::Sint8x2;
            FWGPU_LOGW
                    << "Requested Filament vertex format BYTE2 (float) but getting "
                       "wgpu::VertexFormat::Float16x2 (no direct mapping in wgpu for 8 bit float)"
                    << utils::io::endl;
            return VertexFormat::Float16x2;
        case ElementType::UBYTE2:
            if (integer) return VertexFormat::Uint8x2;
            FWGPU_LOGW
                    << "Requested Filament vertex format UBYTE2 (float) but getting "
                       "wgpu::VertexFormat::Float16x2 (no direct mapping in wgpu for 8 bit float)"
                    << utils::io::endl;
            return VertexFormat::Float16x2;
        case ElementType::SHORT2:
            if (integer) return VertexFormat::Sint16x2;
            FWGPU_LOGW << "Requested Filament vertex format SHORT2 (float) but getting "
                          "wgpu::VertexFormat::Float16x2 (potential loss of precision?)"
                       << utils::io::endl;
            return VertexFormat::Float16x2;
        case ElementType::USHORT2:
            if (integer) return VertexFormat::Uint16x2;
            FWGPU_LOGW << "Requested Filament vertex format USHORT2 (float) but getting "
                          "wgpu::VertexFormat::Float16x2 (potential loss of precision?)"
                       << utils::io::endl;
            return VertexFormat::Float16x2;
        case ElementType::HALF2:   return VertexFormat::Float16x2;
        case ElementType::FLOAT2:  return VertexFormat::Float32x2;
        // Three Component Types
        case ElementType::BYTE3:
            FWGPU_LOGW << "Requested Filament vertex format BYTE3 but getting "
                          "wgpu::VertexFormat::Sint8x4 (no direct mapping in wgpu for x3 byte)"
                       << utils::io::endl;
            return VertexFormat::Sint8x4; // NOT MINSPEC
        case ElementType::UBYTE3:
            FWGPU_LOGW << "Requested Filament vertex format UBYTE3 but getting "
                          "wgpu::VertexFormat::Uint8x4 (no direct mapping in wgpu for x3 byte)"
                       << utils::io::endl;
            return VertexFormat::Uint8x4; // NOT MINSPEC
        case ElementType::SHORT3:
            FWGPU_LOGW
                    << "Requested Filament vertex format SHORT3 but getting "
                       "wgpu::VertexFormat::Sint16x4 (no direct mapping in wgpu for x3 half/short)"
                    << utils::io::endl;
            return VertexFormat::Sint16x4; // NOT MINSPEC
        case ElementType::USHORT3:
            FWGPU_LOGW
                    << "Requested Filament vertex format USHORT3 but getting "
                       "wgpu::VertexFormat::Uint16x4 (no direct mapping in wgpu for x3 half/short)"
                    << utils::io::endl;
            return VertexFormat::Uint16x4; // NOT MINSPEC
        case ElementType::HALF3:
            FWGPU_LOGW
                    << "Requested Filament vertex format HALF3 but getting "
                       "wgpu::VertexFormat::Float16x4 (no direct mapping in wgpu for x3 half/short)"
                    << utils::io::endl;
            return VertexFormat::Float16x4; // NOT MINSPEC
        case ElementType::FLOAT3:  return VertexFormat::Float32x3;
        // Four Component Types
        case ElementType::BYTE4:
            if (integer) return VertexFormat::Sint8x4;
            FWGPU_LOGW
                    << "Requested Filament vertex format BYTE4 (float) but getting "
                       "wgpu::VertexFormat::Float16x4 (no direct mapping in wgpu for 8 bit float)"
                    << utils::io::endl;
            return VertexFormat::Float16x4;
        case ElementType::UBYTE4:
            if (integer) return VertexFormat::Uint8x4;
            FWGPU_LOGW
                    << "Requested Filament vertex format UBYTE4 (float) but getting "
                       "wgpu::VertexFormat::Float16x4 (no direct mapping in wgpu for 8 bit float)"
                    << utils::io::endl;
            return VertexFormat::Float16x4;
        case ElementType::SHORT4:
            if (integer) return VertexFormat::Sint16x4;
            FWGPU_LOGW << "Requested Filament vertex format SHORT4 (float) but getting "
                          "wgpu::VertexFormat::Float16x4 (potential loss of precision?)"
                       << utils::io::endl;
            return VertexFormat::Float16x4;
        case ElementType::USHORT4:
            if (integer) return VertexFormat::Uint16x4;
            FWGPU_LOGW << "Requested Filament vertex format USHORT4 (float) but getting "
                          "wgpu::VertexFormat::Float16x4 (potential loss of precision?)"
                       << utils::io::endl;
            return VertexFormat::Float16x4;
        case ElementType::HALF4:   return VertexFormat::Float16x4;
        case ElementType::FLOAT4:  return VertexFormat::Float32x4;
    }
}

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
 * NOTE: At this point, the vertex buffer layouts do not have attribute information. That needs
 *       to get updated in a subsequent step
 *
 * @param attributes Input vertex attribute information from Filament
 * @param attributeCount The expected number of "used" vertex attributes from Filament, the number
 * of attributes associated with a vertex buffer
 * @param deviceMaxVertexBuffers The device's limit of vertex buffers
 * @param outWebGPUSlotBindingInfos The WebGPU slot bindings to be used by the WebGPU driver to set
 * vertex buffers in the WebGPU API.
 * @param outVertexBufferLayouts The partially populated vertex buffer layouts to eventually pass to
 * WebGPU. The attribute count and attributes pointer is not yet populated at this time. This must
 * be done after this function.
 * @param outAttributeInfos Information about all the vertex attributes to be used, actually
 * associated with a vertex buffer, and the slot each one belongs
 */
void createBufferLayoutsBindingSlotsAndAttributeInfos(AttributeArray const& attributes,
        const uint8_t attributeCount, const uint32_t deviceMaxVertexBuffers,
        std::vector<WebGPUVertexBufferInfo::WebGPUSlotBindingInfo>& outWebGPUSlotBindingInfos,
        std::vector<wgpu::VertexBufferLayout>& outVertexBufferLayouts,
        std::array<AttributeInfo, MAX_VERTEX_ATTRIBUTE_COUNT>& outAttributeInfos) {
    uint8_t currentWebGPUSlotIndex = 0;
    uint8_t currentAttributeIndex = 0;
    outVertexBufferLayouts.reserve(MAX_VERTEX_BUFFER_COUNT);
    outWebGPUSlotBindingInfos.reserve(outVertexBufferLayouts.capacity());
    for (uint32_t attributeIndex = 0; attributeIndex < attributes.size(); ++attributeIndex) {
        const auto& attribute = attributes[attributeIndex];
        if (attribute.buffer == Attribute::BUFFER_UNUSED) {
            // We ignore "unused" attributes, ones not associated with a buffer. If a shader
            // references such an attribute we have a bug one way or another. Either the API/CPU
            // will produce an error (best case scenario), where an attribute is referenced in the
            // shader but not provided by the backend API (CPU-side), or the shader would be getting
            // junk/undefined data from the GPU, since we do not have a valid buffer of data to
            // provide to the shader/GPU.
            continue;
        }
        const bool isInteger = attribute.flags & Attribute::FLAG_INTEGER_TARGET;
        const bool isNormalized = attribute.flags & Attribute::FLAG_NORMALIZED;
        const wgpu::VertexFormat vertexFormat =
                getVertexFormat(attribute.type, isNormalized, isInteger);
        uint8_t existingSlot = INVALID_SLOT_INDEX;
        for (uint32_t slot = 0; slot < currentWebGPUSlotIndex; slot++) {
            WebGPUVertexBufferInfo::WebGPUSlotBindingInfo const& info =
                    outWebGPUSlotBindingInfos[slot];
            // We consider attributes to be in the same slot/layout only if they belong to the
            // same buffer and are interleaved; they cannot belong to separate partitions in the
            // same buffer, for example.
            // For the attributes to be interleaved, the stride must match (among other things).
            // The attribute offset being less than the slot's/layout's buffer offset indicates
            // that it is in a separate partition before this slot/layout, thus not part of it.
            // The difference from the attribute's offset and this slot's/layout's buffer offset
            // being greater than the stride indicates it is in a separate partition after this
            // slot/layout, thus not part of it.
            if (info.sourceBufferIndex == attribute.buffer && info.stride == attribute.stride &&
                    attribute.offset >= info.bufferOffset &&
                    ((attribute.offset - info.bufferOffset) < attribute.stride)) {
                existingSlot = slot;
                break;
            }
        }
        if (existingSlot == INVALID_SLOT_INDEX) {
            // New combination, use a new WebGPU slot
            FILAMENT_CHECK_PRECONDITION(currentWebGPUSlotIndex < MAX_VERTEX_BUFFER_COUNT &&
                                        currentWebGPUSlotIndex < deviceMaxVertexBuffers)
                    << "Number of vertex buffer layouts must not exceed MAX_VERTEX_BUFFER_COUNT ("
                    << MAX_VERTEX_BUFFER_COUNT << ") or the device limit ("
                    << deviceMaxVertexBuffers << ")";
            existingSlot = currentWebGPUSlotIndex++;
            outWebGPUSlotBindingInfos.push_back({
                .sourceBufferIndex = attribute.buffer,
                .bufferOffset = attribute.offset,
                .stride = attribute.stride });
            // we need to have a vertex buffer layout for each slot
            outVertexBufferLayouts.push_back({
                .stepMode = wgpu::VertexStepMode::Vertex,
                .arrayStride = attribute.stride
                // we do not know attributeCount or attributes at this time. We get those
                // in a subsequent pass over the attributeInfos we collect in this loop.
            });
        }
        // we don't use a designated initializer here because certain compilers can't handle it
        outAttributeInfos[currentAttributeIndex++] = AttributeInfo(
                existingSlot,
                {
                    .format = vertexFormat,
                    .offset =
                            attribute.offset - outWebGPUSlotBindingInfos[existingSlot].bufferOffset,
                    .shaderLocation = attributeIndex });
    }
    FILAMENT_CHECK_POSTCONDITION(currentAttributeIndex == attributeCount)
            << "Using " << currentAttributeIndex << " attributes, but " << attributeCount
            << " where indicated.";
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
    mVertexAttributes.reserve(attributeCount);
    if (attributeCount == 0) {
        mVertexBufferLayouts.resize(0);
        mWebGPUSlotBindingInfos.resize(0);
        return; // should not be possible, but being defensive. nothing to do otherwise
    }
    std::array<AttributeInfo, MAX_VERTEX_ATTRIBUTE_COUNT> attributeInfos{};
    createBufferLayoutsBindingSlotsAndAttributeInfos(attributes, attributeCount,
            deviceLimits.maxVertexBuffers, mWebGPUSlotBindingInfos, mVertexBufferLayouts,
            attributeInfos);
    // sort attribute infos by increasing slot (by increasing offset within each slot).
    // We do this to ensure that attributes for the same slot/layout are contiguous in
    // the vector, so the vertex buffer layout associated with these contiguous attributes
    // can directly reference them in the mVertexAttributes vector below.
    std::sort(attributeInfos.data(), attributeInfos.data() + attributeCount,
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
    for (uint32_t attributeIndex = 0; attributeIndex < attributeCount; ++attributeIndex) {
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
