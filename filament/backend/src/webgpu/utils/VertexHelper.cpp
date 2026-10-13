/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "VertexHelper.h"

#include <backend/DriverEnums.h>

#include <utils/debug.h>
#include <utils/Panic.h>

#include <math/half.h>

#include <webgpu/webgpu_cpp.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <numeric>
#include <span>
#include <vector>

namespace filament::backend::webgpuutils {

namespace {

enum class ComponentType : uint8_t { SINT, UINT, HALF, FLOAT };

struct ElementLayout final {
    uint8_t componentCount;
    uint8_t componentSize;
    ComponentType componentType;
};

[[nodiscard]] constexpr ElementLayout getElementLayout(ElementType const type) {
    using CT = ComponentType;
    switch (type) {
        case ElementType::BYTE:    return { 1, 1, CT::SINT };
        case ElementType::BYTE2:   return { 2, 1, CT::SINT };
        case ElementType::BYTE3:   return { 3, 1, CT::SINT };
        case ElementType::BYTE4:   return { 4, 1, CT::SINT };
        case ElementType::UBYTE:   return { 1, 1, CT::UINT };
        case ElementType::UBYTE2:  return { 2, 1, CT::UINT };
        case ElementType::UBYTE3:  return { 3, 1, CT::UINT };
        case ElementType::UBYTE4:  return { 4, 1, CT::UINT };
        case ElementType::SHORT:   return { 1, 2, CT::SINT };
        case ElementType::SHORT2:  return { 2, 2, CT::SINT };
        case ElementType::SHORT3:  return { 3, 2, CT::SINT };
        case ElementType::SHORT4:  return { 4, 2, CT::SINT };
        case ElementType::USHORT:  return { 1, 2, CT::UINT };
        case ElementType::USHORT2: return { 2, 2, CT::UINT };
        case ElementType::USHORT3: return { 3, 2, CT::UINT };
        case ElementType::USHORT4: return { 4, 2, CT::UINT };
        case ElementType::INT:     return { 1, 4, CT::SINT };
        case ElementType::UINT:    return { 1, 4, CT::UINT };
        case ElementType::FLOAT:   return { 1, 4, CT::FLOAT };
        case ElementType::FLOAT2:  return { 2, 4, CT::FLOAT };
        case ElementType::FLOAT3:  return { 3, 4, CT::FLOAT };
        case ElementType::FLOAT4:  return { 4, 4, CT::FLOAT };
        case ElementType::HALF:    return { 1, 2, CT::HALF };
        case ElementType::HALF2:   return { 2, 2, CT::HALF };
        case ElementType::HALF3:   return { 3, 2, CT::HALF };
        case ElementType::HALF4:   return { 4, 2, CT::HALF };
    }
    return { 1, 1, CT::SINT };
}

[[nodiscard]] constexpr uint32_t alignUp4(uint32_t const value) {
    return (value + 3u) & ~3u;
}

// Selects the 1, 2 or 4 component variant. 3 components (only valid for 32-bit types) are handled
// by the caller.
[[nodiscard]] constexpr wgpu::VertexFormat pick(uint8_t const componentCount,
        wgpu::VertexFormat const x1, wgpu::VertexFormat const x2, wgpu::VertexFormat const x4) {
    switch (componentCount) {
        case 1:  return x1;
        case 2:  return x2;
        default: return x4;
    }
}

[[nodiscard]] wgpu::VertexFormat getFloat32Format(uint8_t const componentCount) {
    using VF = wgpu::VertexFormat;
    switch (componentCount) {
        case 1:  return VF::Float32;
        case 2:  return VF::Float32x2;
        case 3:  return VF::Float32x3;
        default: return VF::Float32x4;
    }
}

// Formats for 8-bit and 16-bit integer data that is read as normalized floats or integers.
[[nodiscard]] wgpu::VertexFormat getSmallIntegerFormat(ElementLayout const& layout,
        bool const normalized, uint8_t const componentCount) {
    using VF = wgpu::VertexFormat;
    bool const isSigned = layout.componentType == ComponentType::SINT;
    if (layout.componentSize == 1) {
        if (normalized) {
            return isSigned ? pick(componentCount, VF::Snorm8, VF::Snorm8x2, VF::Snorm8x4)
                            : pick(componentCount, VF::Unorm8, VF::Unorm8x2, VF::Unorm8x4);
        }
        return isSigned ? pick(componentCount, VF::Sint8, VF::Sint8x2, VF::Sint8x4)
                        : pick(componentCount, VF::Uint8, VF::Uint8x2, VF::Uint8x4);
    }
    if (normalized) {
        return isSigned ? pick(componentCount, VF::Snorm16, VF::Snorm16x2, VF::Snorm16x4)
                        : pick(componentCount, VF::Unorm16, VF::Unorm16x2, VF::Unorm16x4);
    }
    return isSigned ? pick(componentCount, VF::Sint16, VF::Sint16x2, VF::Sint16x4)
                    : pick(componentCount, VF::Uint16, VF::Uint16x2, VF::Uint16x4);
}

// Reads one integer component of the given layout from possibly unaligned memory.
[[nodiscard]] int64_t readInteger(uint8_t const* const src, ElementLayout const& layout) {
    bool const isSigned = layout.componentType == ComponentType::SINT;
    switch (layout.componentSize) {
        case 1: {
            uint8_t v;
            memcpy(&v, src, sizeof(v));
            return isSigned ? int64_t(int8_t(v)) : int64_t(v);
        }
        case 2: {
            uint16_t v;
            memcpy(&v, src, sizeof(v));
            return isSigned ? int64_t(int16_t(v)) : int64_t(v);
        }
        default: {
            uint32_t v;
            memcpy(&v, src, sizeof(v));
            return isSigned ? int64_t(int32_t(v)) : int64_t(v);
        }
    }
}

// The value a padded 4th component takes, matching the GL default of w = 1.
void writePaddingOne(uint8_t* const dst, ElementLayout const& layout, bool const normalized) {
    if (layout.componentType == ComponentType::HALF) {
        constexpr uint16_t one = 0x3C00; // 1.0 as a half float
        memcpy(dst, &one, sizeof(one));
        return;
    }
    bool const isSigned = layout.componentType == ComponentType::SINT;
    if (layout.componentSize == 1) {
        uint8_t const one = normalized ? (isSigned ? 0x7F : 0xFF) : 1;
        memcpy(dst, &one, sizeof(one));
    } else {
        uint16_t const one = normalized ? (isSigned ? 0x7FFF : 0xFFFF) : 1;
        memcpy(dst, &one, sizeof(one));
    }
}

void convertElement(VertexAttributeRepack const& attribute, uint8_t const* const src,
        uint8_t* const dst) {
    ElementLayout const layout = getElementLayout(attribute.type);
    uint8_t const n = layout.componentCount;
    uint8_t const size = layout.componentSize;
    switch (attribute.conversion) {
        case VertexConversion::NONE:
            memcpy(dst, src, n * size);
            break;
        case VertexConversion::PAD_TO_4:
            assert_invariant(n == 3);
            memcpy(dst, src, n * size);
            writePaddingOne(dst + n * size, layout,
                    attribute.flags & Attribute::FLAG_NORMALIZED);
            break;
        case VertexConversion::SCALED_TO_F16: {
            // Every 8-bit integer is exactly representable as a half float.
            static_assert(sizeof(math::half) == sizeof(uint16_t));
            for (uint8_t i = 0; i < n; ++i) {
                math::half const value(float(readInteger(src + i * size, layout)));
                memcpy(dst + i * sizeof(value), &value, sizeof(value));
            }
            if (n == 3) {
                constexpr uint16_t one = 0x3C00; // 1.0 as a half float
                memcpy(dst + 3 * sizeof(one), &one, sizeof(one));
            }
            break;
        }
        case VertexConversion::SCALED_TO_F32:
            for (uint8_t i = 0; i < n; ++i) {
                float const value = float(readInteger(src + i * size, layout));
                memcpy(dst + i * sizeof(value), &value, sizeof(value));
            }
            break;
    }
}

[[nodiscard]] constexpr int64_t floorDiv(int64_t const a, int64_t const b) {
    int64_t const q = a / b;
    return (a % b != 0 && ((a < 0) != (b < 0))) ? q - 1 : q;
}

[[nodiscard]] constexpr int64_t ceilDiv(int64_t const a, int64_t const b) {
    return -floorDiv(-a, b);
}

} // namespace

VertexFormatInfo getVertexFormatInfo(ElementType const type, uint8_t const attributeFlags) {
    using VF = wgpu::VertexFormat;
    ElementLayout const layout = getElementLayout(type);
    uint8_t const n = layout.componentCount;
    bool const normalized = attributeFlags & Attribute::FLAG_NORMALIZED;
    bool const integer = attributeFlags & Attribute::FLAG_INTEGER_TARGET;

    switch (layout.componentType) {
        case ComponentType::FLOAT:
            return { getFloat32Format(n), uint8_t(4 * n), VertexConversion::NONE };
        case ComponentType::HALF:
            if (n == 3) {
                return { VF::Float16x4, 8, VertexConversion::PAD_TO_4 };
            }
            return { pick(n, VF::Float16, VF::Float16x2, VF::Float16x4), uint8_t(2 * n),
                VertexConversion::NONE };
        case ComponentType::SINT:
        case ComponentType::UINT:
            break;
    }

    bool const isSigned = layout.componentType == ComponentType::SINT;
    if (layout.componentSize == 4) {
        // INT and UINT
        if (integer) {
            return { isSigned ? VF::Sint32 : VF::Uint32, 4, VertexConversion::NONE };
        }
        // WebGPU has no 32-bit normalized formats, so these are treated as scaled.
        return { VF::Float32, 4, VertexConversion::SCALED_TO_F32 };
    }

    // 8-bit and 16-bit integer types
    uint8_t const paddedCount = n == 3 ? 4 : n;
    if (normalized || integer) {
        return { getSmallIntegerFormat(layout, normalized, paddedCount),
            uint8_t(paddedCount * layout.componentSize),
            n == 3 ? VertexConversion::PAD_TO_4 : VertexConversion::NONE };
    }
    // Scaled: the shader expects floats holding the integer values.
    if (layout.componentSize == 1) {
        return { pick(paddedCount, VF::Float16, VF::Float16x2, VF::Float16x4),
            uint8_t(2 * paddedCount), VertexConversion::SCALED_TO_F16 };
    }
    return { getFloat32Format(n), uint8_t(4 * n), VertexConversion::SCALED_TO_F32 };
}

uint8_t getSourceElementSize(ElementType const type) {
    ElementLayout const layout = getElementLayout(type);
    return layout.componentCount * layout.componentSize;
}

VertexSlotRepack computeSlotRepack(uint32_t const srcStride,
        std::span<VertexSlotAttribute const> const attributes) {
    // VertexBuffer::Builder::attribute() replaces a stride of 0 with the element size, so a slot
    // always advances per vertex. The helpers below rely on this.
    FILAMENT_CHECK_PRECONDITION(srcStride != 0) << "A vertex stride of 0 is not supported";

    VertexSlotRepack repack{ .srcStride = srcStride };
    repack.attributes.reserve(attributes.size());

    bool compatible = srcStride % 4 == 0;
    uint32_t begin = std::numeric_limits<uint32_t>::max();
    uint32_t end = 0;
    for (VertexSlotAttribute const& attribute: attributes) {
        VertexFormatInfo const info = getVertexFormatInfo(attribute.type, attribute.flags);
        uint32_t const srcSize = getSourceElementSize(attribute.type);
        begin = std::min(begin, attribute.srcOffset);
        end = std::max(end, attribute.srcOffset + srcSize);
        uint32_t const requiredAlignment = std::min<uint32_t>(4, info.size);
        compatible = compatible && info.conversion == VertexConversion::NONE &&
                     attribute.srcOffset % requiredAlignment == 0 &&
                     attribute.srcOffset + info.size <= srcStride;
        repack.attributes.push_back({
            .srcOffset = attribute.srcOffset,
            .dstOffset = attribute.srcOffset,
            .type = attribute.type,
            .flags = attribute.flags,
            .conversion = info.conversion,
            .format = info.format,
        });
    }
    repack.srcVertexBegin = attributes.empty() ? 0 : begin;
    repack.srcVertexEnd = end;

    if (compatible) {
        repack.dstStride = srcStride;
        return repack;
    }

    // Pack the attributes in source order, each aligned to 4 bytes, which satisfies WebGPU's
    // min(4, formatSize) alignment requirement for every format.
    std::vector<size_t> order(repack.attributes.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](size_t const a, size_t const b) {
        return repack.attributes[a].srcOffset < repack.attributes[b].srcOffset;
    });
    uint32_t cursor = 0;
    for (size_t const index: order) {
        VertexAttributeRepack& attribute = repack.attributes[index];
        attribute.dstOffset = cursor;
        cursor = alignUp4(cursor + getVertexFormatInfo(attribute.type, attribute.flags).size);
    }
    repack.dstStride = std::max<uint32_t>(cursor, 4);
    repack.needsTransform = true;
    return repack;
}

uint32_t getAvailableVertexCount(VertexSlotRepack const& repack, uint32_t const slotOffset,
        size_t const bufferSize) {
    assert_invariant(repack.srcStride != 0);
    size_t const firstVertexEnd = size_t(slotOffset) + repack.srcVertexEnd;
    if (bufferSize < firstVertexEnd) {
        return 0;
    }
    size_t const count = (bufferSize - firstVertexEnd) / repack.srcStride + 1;
    return uint32_t(std::min<size_t>(count, std::numeric_limits<uint32_t>::max()));
}

void repackVertices(VertexSlotRepack const& repack, uint8_t const* const src, uint8_t* const dst,
        size_t const vertexCount) {
    memset(dst, 0, vertexCount * repack.dstStride);
    for (size_t vertex = 0; vertex < vertexCount; ++vertex) {
        uint8_t const* const srcVertex = src + vertex * repack.srcStride;
        uint8_t* const dstVertex = dst + vertex * repack.dstStride;
        for (VertexAttributeRepack const& attribute: repack.attributes) {
            convertElement(attribute, srcVertex + (attribute.srcOffset - repack.srcVertexBegin),
                    dstVertex + attribute.dstOffset);
        }
    }
}

VertexRange getUpdatedVertexRange(VertexSlotRepack const& repack, uint32_t const vertexCount,
        int64_t const updateBegin, int64_t const updateEnd) {
    if (vertexCount == 0 || updateBegin >= updateEnd || repack.attributes.empty()) {
        return {};
    }
    int64_t const stride = repack.srcStride;
    int64_t const a = repack.srcVertexBegin;
    int64_t const b = repack.srcVertexEnd;
    assert_invariant(stride != 0);
    // Vertex v reads the bytes [v * stride + a, v * stride + b).
    // It is touched by the update if that range intersects [updateBegin, updateEnd), and
    // completely covered if that range is contained in it.
    int64_t const lastVertex = int64_t(vertexCount) - 1;
    int64_t const firstTouched = std::max<int64_t>(0, floorDiv(updateBegin - b, stride) + 1);
    int64_t const lastTouched = std::min(lastVertex, floorDiv(updateEnd - a - 1, stride));
    if (firstTouched > lastTouched) {
        return {};
    }
    int64_t const firstComplete = ceilDiv(updateBegin - a, stride);
    int64_t const lastComplete = floorDiv(updateEnd - b, stride);
    return {
        .first = uint32_t(firstTouched),
        .count = uint32_t(lastTouched - firstTouched + 1),
        .complete = firstTouched >= firstComplete && lastTouched <= lastComplete,
    };
}

} // namespace filament::backend::webgpuutils
