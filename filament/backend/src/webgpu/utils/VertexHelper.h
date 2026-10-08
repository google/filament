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

#ifndef TNT_FILAMENT_BACKEND_WEBGPU_VERTEXHELPER_H
#define TNT_FILAMENT_BACKEND_WEBGPU_VERTEXHELPER_H

#include <backend/DriverEnums.h>

#include <webgpu/webgpu_cpp.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

/**
 * Helpers for mapping Filament vertex attributes onto WebGPU vertex formats.
 *
 * WebGPU lacks several vertex formats that Filament exposes:
 *  - There are no 3-component 8-bit, 16-bit or half-float formats. These are padded to 4
 *    components, with the extra component set to the type's "one" value (matching the GL default
 *    of w = 1).
 *  - There are no "scaled" formats, where integer data is read as float without normalization.
 *    8-bit integers are converted to half floats, which represent them exactly, and 16-bit or
 *    32-bit integers are converted to 32-bit floats.
 *  - Vertex strides must be multiples of 4, and attribute offsets must be multiples of
 *    min(4, formatSize).
 *
 * The converted formats keep the base type expected by the shader (f32 for normalized and scaled
 * data, i32/u32 for integer data), so no shader changes are needed.
 *
 * Everything here operates on CPU memory only, so it can be unit-tested without a device.
 */
namespace filament::backend::webgpuutils {

// How the bytes of one source element are turned into one element of the WebGPU format.
enum class VertexConversion : uint8_t {
    NONE,          // The source bytes are already in the WebGPU format and are copied as-is.
    PAD_TO_4,      // 3 components are copied and a 4th "one" component is appended.
    SCALED_TO_F16, // 8-bit integers are converted to half floats (3 components are padded to 4).
    SCALED_TO_F32, // 16-bit or 32-bit integers are converted to 32-bit floats.
};

struct VertexFormatInfo final {
    wgpu::VertexFormat format = wgpu::VertexFormat::Float32;
    uint8_t size = 0; // byte size of one element in the WebGPU format
    VertexConversion conversion = VertexConversion::NONE;
};

// Returns the WebGPU format used for a Filament element type with the given Attribute::FLAG_*
// flags, and the conversion needed to produce it from the source bytes.
[[nodiscard]] VertexFormatInfo getVertexFormatInfo(ElementType type, uint8_t attributeFlags);

// Byte size of one source element of the given type.
[[nodiscard]] uint8_t getSourceElementSize(ElementType type);

// One attribute of a WebGPU vertex buffer slot, as declared by Filament.
struct VertexSlotAttribute final {
    uint32_t srcOffset = 0; // relative to the start of a source vertex
    ElementType type = ElementType::BYTE;
    uint8_t flags = 0;      // Attribute::FLAG_*
};

// Describes how one attribute is moved from the source layout to the WebGPU layout.
struct VertexAttributeRepack final {
    uint32_t srcOffset = 0; // relative to the start of a source vertex
    uint32_t dstOffset = 0; // relative to the start of a WebGPU vertex
    ElementType type = ElementType::BYTE;
    uint8_t flags = 0;
    VertexConversion conversion = VertexConversion::NONE;
    wgpu::VertexFormat format = wgpu::VertexFormat::Float32;

    bool operator==(VertexAttributeRepack const&) const = default;
};

// Describes the source and WebGPU layouts of one WebGPU vertex buffer slot (i.e. a group of
// attributes that share a source buffer and stride).
struct VertexSlotRepack final {
    uint32_t srcStride = 0;
    uint32_t dstStride = 0;
    // The range of bytes read by the attributes, relative to the start of a source vertex.
    uint32_t srcVertexBegin = 0;
    uint32_t srcVertexEnd = 0;
    // False when the source layout can be used by WebGPU directly. In that case dstStride equals
    // srcStride and every dstOffset equals its srcOffset.
    bool needsTransform = false;
    // In the same order as the attributes passed to computeSlotRepack().
    std::vector<VertexAttributeRepack> attributes;

    bool operator==(VertexSlotRepack const&) const = default;
};

// Computes the WebGPU layout of a slot. When the source layout is not directly usable, the
// attributes are packed in source-offset order, each aligned to 4 bytes, and the stride is rounded
// up to a multiple of 4. `srcStride` must not be 0.
[[nodiscard]] VertexSlotRepack computeSlotRepack(uint32_t srcStride,
        std::span<VertexSlotAttribute const> attributes);

// Returns how many whole source vertices of a slot fit in a buffer of `bufferSize` bytes when the
// slot starts at `slotOffset`.
[[nodiscard]] uint32_t getAvailableVertexCount(VertexSlotRepack const& repack,
        uint32_t slotOffset, size_t bufferSize);

// Converts `vertexCount` source vertices into `dst`, which must hold vertexCount * dstStride bytes.
// `src` points to the first byte read, i.e. byte `srcVertexBegin` of the first source vertex.
// Bytes of `dst` not covered by an attribute are zeroed.
void repackVertices(VertexSlotRepack const& repack, uint8_t const* src, uint8_t* dst,
        size_t vertexCount);

struct VertexRange final {
    uint32_t first = 0;
    uint32_t count = 0;
    // True if every vertex whose attribute bytes intersect the update is entirely covered by it.
    bool complete = true;
};

// Computes which vertices of a slot are affected by an update of the byte range
// [updateBegin, updateEnd), expressed relative to the start of the slot (so it can be negative).
[[nodiscard]] VertexRange getUpdatedVertexRange(VertexSlotRepack const& repack,
        uint32_t vertexCount, int64_t updateBegin, int64_t updateEnd);

} // namespace filament::backend::webgpuutils

#endif // TNT_FILAMENT_BACKEND_WEBGPU_VERTEXHELPER_H
