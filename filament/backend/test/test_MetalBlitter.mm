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

#include "BackendTest.h"

#include "metal/MetalBlitter.h"
#include "metal/MetalDriver.h"
#include "metal/MetalContext.h"

#include "Lifetimes.h"
#include "Skip.h"

namespace test {

using namespace filament;
using namespace filament::backend;
using namespace filament::math;
using namespace utils;

struct MetalBlitterTest
    : public BackendTest,
      public testing::WithParamInterface<std::tuple<MTLPixelFormat, MTLPixelFormat>> {
    MetalBlitterTest() {}
};

constexpr int kTextureSize = 32;

template<typename T>
constexpr T redValue() {
    if constexpr (T::SIZE == 1) return T{ 1 };
    if constexpr (T::SIZE == 2) return T{ 1, 0 };
    if constexpr (T::SIZE == 3) return T{ 1, 0, 0 };
    if constexpr (T::SIZE == 4) return T{ 1, 0, 0, 1 };
}

template<typename T>
void fillTextureWithRed(id<MTLTexture> texture) {
    std::vector<T> textureData(kTextureSize * kTextureSize, redValue<T>());
    [texture replaceRegion:MTLRegionMake2D(0, 0, kTextureSize, kTextureSize)
               mipmapLevel:0
                 withBytes:textureData.data()
               bytesPerRow:kTextureSize * sizeof(T)];
}

template<typename T>
void verifyTextureIsRed(id<MTLTexture> texture) {
    std::vector<T> resultData(kTextureSize * kTextureSize);
    [texture getBytes:resultData.data()
          bytesPerRow:kTextureSize * sizeof(T)
           fromRegion:MTLRegionMake2D(0, 0, kTextureSize, kTextureSize)
          mipmapLevel:0];

    std::vector<T> expectedData(kTextureSize * kTextureSize, redValue<T>());
    EXPECT_EQ(resultData, expectedData);
}

TEST_P(MetalBlitterTest, Blit) {
    SKIP_IF_NOT(Backend::METAL, "MetalBlitter only works with the Metal backend");

    const auto& [srcFormat, dstFormat] = GetParam();

    MetalDriver& metalDriver = static_cast<MetalDriver&>(getDriver());
    MetalContext& metalContext = *metalDriver.getContext();

    MetalBlitter blitter(metalContext);

    // Create a source texture
    MTLTextureDescriptor* desc = [MTLTextureDescriptor new];
    desc.width = kTextureSize;
    desc.height = kTextureSize;
    desc.pixelFormat = srcFormat;
    desc.usage = MTLTextureUsageRenderTarget;
    id<MTLTexture> srcTexture = [metalContext.device newTextureWithDescriptor:desc];

    // Create a destination texture with the parameterized format.
    desc.pixelFormat = dstFormat;
    id<MTLTexture> dstTexture = [metalContext.device newTextureWithDescriptor:desc];

    // Fill the source texture with red.
    switch (srcFormat) {
        case MTLPixelFormatRGBA32Float:
            fillTextureWithRed<float4>(srcTexture);
            break;
        case MTLPixelFormatRG32Float:
            fillTextureWithRed<float2>(srcTexture);
            break;
        case MTLPixelFormatRGBA32Uint:
            fillTextureWithRed<uint4>(srcTexture);
            break;
        case MTLPixelFormatRG32Uint:
            fillTextureWithRed<uint2>(srcTexture);
            break;
        case MTLPixelFormatRGBA32Sint:
            fillTextureWithRed<int4>(srcTexture);
            break;
        case MTLPixelFormatRG32Sint:
            fillTextureWithRed<int2>(srcTexture);
            break;
        default:
            FAIL() << "Source format not implemented in test";
    }

    id<MTLCommandBuffer> cmdBuffer = [metalContext.commandQueue commandBuffer];
    MetalBlitter::BlitArgs args{};
    args.source.texture = srcTexture;
    args.source.region = MTLRegionMake2D(0, 0, kTextureSize, kTextureSize);
    args.destination.texture = dstTexture;
    args.destination.region = MTLRegionMake2D(0, 0, kTextureSize, kTextureSize);
    args.filter = SamplerMagFilter::NEAREST;
    blitter.blit(cmdBuffer, args, "MetalBlitterTest");

    [cmdBuffer commit];
    [cmdBuffer waitUntilCompleted];

    // Verify the destination texture is red.
    switch (dstFormat) {
        case MTLPixelFormatRGBA32Float:
            verifyTextureIsRed<float4>(dstTexture);
            break;
        case MTLPixelFormatRG32Float:
            verifyTextureIsRed<float2>(dstTexture);
            break;
        case MTLPixelFormatRGBA32Uint:
            verifyTextureIsRed<uint4>(dstTexture);
            break;
        case MTLPixelFormatRG32Uint:
            verifyTextureIsRed<uint2>(dstTexture);
            break;
        case MTLPixelFormatRGBA32Sint:
            verifyTextureIsRed<int4>(dstTexture);
            break;
        case MTLPixelFormatRG32Sint:
            verifyTextureIsRed<int2>(dstTexture);
            break;
        default:
            FAIL() << "Destination format not implemented in test";
    }
}

// Helper to give names to the tests.
static std::string testNameGenerator(
        const testing::TestParamInfo<MetalBlitterTest::ParamType>& info) {
    auto const& [srcFormat, dstFormat] = info.param;

    auto formatToString = [](MTLPixelFormat format) {
        switch (format) {
            case MTLPixelFormatRGBA32Float: return "RGBA32Float";
            case MTLPixelFormatRG32Float:   return "RG32Float";
            case MTLPixelFormatRGBA32Uint:  return "RGBA32Uint";
            case MTLPixelFormatRG32Uint:    return "RG32Uint";
            case MTLPixelFormatRGBA32Sint:  return "RGBA32Sint";
            case MTLPixelFormatRG32Sint:    return "RG32Sint";
            default:                        return "Unknown";
        }
    };

    return std::string(formatToString(srcFormat)) + "_to_" + std::string(formatToString(dstFormat));
}

// This instantiates all the test cases. Each call to std::make_tuple defines a single test case
// that uses MetalBlitter to blit from a source texture to a destination texture.
// The first argument is the source texture format.
// The second argument is the destination texture format.
INSTANTIATE_TEST_SUITE_P(MetalBlitterTests, MetalBlitterTest,
        testing::Values(
                // equal formats, fast path
                std::make_tuple(MTLPixelFormatRGBA32Float, MTLPixelFormatRGBA32Float),

                // the rest of the test cases take the slow path

                std::make_tuple(MTLPixelFormatRGBA32Float, MTLPixelFormatRG32Float),
                std::make_tuple(MTLPixelFormatRG32Float, MTLPixelFormatRGBA32Float),

                std::make_tuple(MTLPixelFormatRGBA32Uint, MTLPixelFormatRG32Uint),
                std::make_tuple(MTLPixelFormatRG32Uint, MTLPixelFormatRGBA32Uint),

                std::make_tuple(MTLPixelFormatRG32Sint, MTLPixelFormatRGBA32Sint),
                std::make_tuple(MTLPixelFormatRGBA32Sint, MTLPixelFormatRG32Sint)
        ),
        testNameGenerator
);

} // namespace test
