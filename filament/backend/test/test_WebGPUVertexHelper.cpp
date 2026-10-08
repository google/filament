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

#include "webgpu/utils/VertexHelper.h"

#include <backend/DriverEnums.h>

#include <math/half.h>

#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

#include <cstdint>
#include <cstring>
#include <vector>

namespace filament::backend::webgpuutils {

namespace {

constexpr uint8_t NORMALIZED = Attribute::FLAG_NORMALIZED;
constexpr uint8_t INTEGER = Attribute::FLAG_INTEGER_TARGET;

template<typename T>
T readAt(std::vector<uint8_t> const& bytes, size_t offset) {
    T value;
    memcpy(&value, bytes.data() + offset, sizeof(T));
    return value;
}

float readHalfAt(std::vector<uint8_t> const& bytes, size_t offset) {
    math::half value;
    memcpy(&value, bytes.data() + offset, sizeof(value));
    return float(value);
}

} // namespace

TEST(WebGPUVertexHelper, NativeFormatsAreNotConverted) {
    struct Case {
        ElementType type;
        uint8_t flags;
        wgpu::VertexFormat format;
    };
    for (Case const& c: {
             Case{ ElementType::FLOAT3, 0, wgpu::VertexFormat::Float32x3 },
             Case{ ElementType::HALF4, 0, wgpu::VertexFormat::Float16x4 },
             Case{ ElementType::UBYTE4, NORMALIZED, wgpu::VertexFormat::Unorm8x4 },
             Case{ ElementType::SHORT2, NORMALIZED, wgpu::VertexFormat::Snorm16x2 },
             Case{ ElementType::USHORT4, INTEGER, wgpu::VertexFormat::Uint16x4 },
             Case{ ElementType::UINT, INTEGER, wgpu::VertexFormat::Uint32 },
         }) {
        VertexFormatInfo const info = getVertexFormatInfo(c.type, c.flags);
        EXPECT_EQ(info.format, c.format);
        EXPECT_EQ(info.conversion, VertexConversion::NONE);
        EXPECT_EQ(info.size, getSourceElementSize(c.type));
    }
}

TEST(WebGPUVertexHelper, ThreeComponentFormatsArePadded) {
    VertexFormatInfo info = getVertexFormatInfo(ElementType::UBYTE3, NORMALIZED);
    EXPECT_EQ(info.format, wgpu::VertexFormat::Unorm8x4);
    EXPECT_EQ(info.conversion, VertexConversion::PAD_TO_4);
    EXPECT_EQ(info.size, 4);

    info = getVertexFormatInfo(ElementType::SHORT3, INTEGER);
    EXPECT_EQ(info.format, wgpu::VertexFormat::Sint16x4);
    EXPECT_EQ(info.conversion, VertexConversion::PAD_TO_4);
    EXPECT_EQ(info.size, 8);

    info = getVertexFormatInfo(ElementType::HALF3, 0);
    EXPECT_EQ(info.format, wgpu::VertexFormat::Float16x4);
    EXPECT_EQ(info.conversion, VertexConversion::PAD_TO_4);
    EXPECT_EQ(info.size, 8);
}

TEST(WebGPUVertexHelper, ScaledFormatsAreConvertedToFloat) {
    VertexFormatInfo info = getVertexFormatInfo(ElementType::BYTE2, 0);
    EXPECT_EQ(info.format, wgpu::VertexFormat::Float16x2);
    EXPECT_EQ(info.conversion, VertexConversion::SCALED_TO_F16);

    info = getVertexFormatInfo(ElementType::UBYTE3, 0);
    EXPECT_EQ(info.format, wgpu::VertexFormat::Float16x4);
    EXPECT_EQ(info.conversion, VertexConversion::SCALED_TO_F16);

    info = getVertexFormatInfo(ElementType::USHORT3, 0);
    EXPECT_EQ(info.format, wgpu::VertexFormat::Float32x3);
    EXPECT_EQ(info.conversion, VertexConversion::SCALED_TO_F32);

    info = getVertexFormatInfo(ElementType::INT, 0);
    EXPECT_EQ(info.format, wgpu::VertexFormat::Float32);
    EXPECT_EQ(info.conversion, VertexConversion::SCALED_TO_F32);
}

TEST(WebGPUVertexHelper, CompatibleLayoutIsKept) {
    VertexSlotAttribute const attributes[] = {
        { .srcOffset = 0, .type = ElementType::FLOAT3 },
        { .srcOffset = 12, .type = ElementType::HALF2 },
    };
    VertexSlotRepack const repack = computeSlotRepack(16, attributes);
    EXPECT_FALSE(repack.needsTransform);
    EXPECT_EQ(repack.dstStride, 16u);
    ASSERT_EQ(repack.attributes.size(), 2u);
    EXPECT_EQ(repack.attributes[0].dstOffset, 0u);
    EXPECT_EQ(repack.attributes[1].dstOffset, 12u);
}

TEST(WebGPUVertexHelper, StrideNotMultipleOfFourIsRepacked) {
    VertexSlotAttribute const attributes[] = {
        { .srcOffset = 0, .type = ElementType::UBYTE2, .flags = NORMALIZED },
    };
    VertexSlotRepack const repack = computeSlotRepack(2, attributes);
    EXPECT_TRUE(repack.needsTransform);
    EXPECT_EQ(repack.dstStride, 4u);
}

TEST(WebGPUVertexHelper, InterleavedPaddedAttribute) {
    // position: FLOAT3 at 0, color: UBYTE3 normalized at 12, stride 15 (tightly packed).
    VertexSlotAttribute const attributes[] = {
        { .srcOffset = 0, .type = ElementType::FLOAT3 },
        { .srcOffset = 12, .type = ElementType::UBYTE3, .flags = NORMALIZED },
    };
    VertexSlotRepack const repack = computeSlotRepack(15, attributes);
    ASSERT_TRUE(repack.needsTransform);
    EXPECT_EQ(repack.dstStride, 16u);
    EXPECT_EQ(repack.attributes[0].dstOffset, 0u);
    EXPECT_EQ(repack.attributes[1].dstOffset, 12u);
    EXPECT_EQ(repack.attributes[1].format, wgpu::VertexFormat::Unorm8x4);

    std::vector<uint8_t> src(2 * 15);
    for (int vertex = 0; vertex < 2; ++vertex) {
        float const position[3] = { float(vertex), 2.0f, 3.0f };
        memcpy(src.data() + vertex * 15, position, sizeof(position));
        uint8_t const color[3] = { 10, uint8_t(20 + vertex), 30 };
        memcpy(src.data() + vertex * 15 + 12, color, sizeof(color));
    }
    std::vector<uint8_t> dst(2 * repack.dstStride, 0xCD);
    repackVertices(repack, src.data(), dst.data(), 2);

    for (int vertex = 0; vertex < 2; ++vertex) {
        size_t const base = vertex * repack.dstStride;
        EXPECT_EQ(readAt<float>(dst, base + 0), float(vertex));
        EXPECT_EQ(readAt<float>(dst, base + 8), 3.0f);
        EXPECT_EQ(dst[base + 12], 10);
        EXPECT_EQ(dst[base + 13], 20 + vertex);
        EXPECT_EQ(dst[base + 14], 30);
        EXPECT_EQ(dst[base + 15], 0xFF); // padded w = 1.0
    }
}

TEST(WebGPUVertexHelper, ScaledBytesBecomeExactHalfs) {
    VertexSlotAttribute const attributes[] = {
        { .srcOffset = 0, .type = ElementType::BYTE3 },
    };
    VertexSlotRepack const repack = computeSlotRepack(4, attributes);
    ASSERT_TRUE(repack.needsTransform);
    EXPECT_EQ(repack.dstStride, 8u);

    std::vector<uint8_t> const src = { uint8_t(-128), 0, 127, 0xEE };
    std::vector<uint8_t> dst(repack.dstStride);
    repackVertices(repack, src.data(), dst.data(), 1);
    EXPECT_EQ(readHalfAt(dst, 0), -128.0f);
    EXPECT_EQ(readHalfAt(dst, 2), 0.0f);
    EXPECT_EQ(readHalfAt(dst, 4), 127.0f);
    EXPECT_EQ(readHalfAt(dst, 6), 1.0f);
}

TEST(WebGPUVertexHelper, ScaledShortsBecomeFloats) {
    VertexSlotAttribute const attributes[] = {
        { .srcOffset = 0, .type = ElementType::USHORT3 },
    };
    VertexSlotRepack const repack = computeSlotRepack(8, attributes);
    ASSERT_TRUE(repack.needsTransform);
    EXPECT_EQ(repack.dstStride, 12u);

    uint16_t const values[4] = { 0, 1234, 65535, 0xEEEE };
    std::vector<uint8_t> src(sizeof(values));
    memcpy(src.data(), values, sizeof(values));
    std::vector<uint8_t> dst(repack.dstStride);
    repackVertices(repack, src.data(), dst.data(), 1);
    EXPECT_EQ(readAt<float>(dst, 0), 0.0f);
    EXPECT_EQ(readAt<float>(dst, 4), 1234.0f);
    EXPECT_EQ(readAt<float>(dst, 8), 65535.0f);
}

TEST(WebGPUVertexHelper, AvailableVertexCount) {
    VertexSlotAttribute const attributes[] = {
        { .srcOffset = 0, .type = ElementType::SHORT3 },
    };
    VertexSlotRepack const repack = computeSlotRepack(8, attributes);
    // The last vertex only needs 6 bytes.
    EXPECT_EQ(getAvailableVertexCount(repack, 0, 8 * 3 + 6), 4u);
    EXPECT_EQ(getAvailableVertexCount(repack, 0, 8 * 3 + 5), 3u);
    EXPECT_EQ(getAvailableVertexCount(repack, 16, 8), 0u);
}

TEST(WebGPUVertexHelper, UpdatedVertexRange) {
    VertexSlotAttribute const attributes[] = {
        { .srcOffset = 0, .type = ElementType::SHORT3 },
    };
    VertexSlotRepack const repack = computeSlotRepack(8, attributes);

    // Whole vertices 1 and 2.
    VertexRange range = getUpdatedVertexRange(repack, 10, 8, 24);
    EXPECT_EQ(range.first, 1u);
    EXPECT_EQ(range.count, 2u);
    EXPECT_TRUE(range.complete);

    // The unused padding of the last vertex does not need to be covered.
    range = getUpdatedVertexRange(repack, 10, 8, 22);
    EXPECT_EQ(range.count, 2u);
    EXPECT_TRUE(range.complete);

    // Starts in the middle of vertex 0.
    range = getUpdatedVertexRange(repack, 10, 4, 16);
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.count, 2u);
    EXPECT_FALSE(range.complete);

    // Only touches the unused padding of vertex 0.
    range = getUpdatedVertexRange(repack, 10, 6, 8);
    EXPECT_EQ(range.count, 0u);

    // Covers more than the slot, before and after it.
    range = getUpdatedVertexRange(repack, 4, -100, 100);
    EXPECT_EQ(range.first, 0u);
    EXPECT_EQ(range.count, 4u);
    EXPECT_TRUE(range.complete);
}

} // namespace filament::backend::webgpuutils
