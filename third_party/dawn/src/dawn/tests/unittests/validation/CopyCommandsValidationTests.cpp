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

#include <tuple>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class CopyCommandTest : public ValidationTest {
  protected:
    wgpu::Buffer CreateBuffer(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;

        return device.CreateBuffer(&descriptor);
    }

    wgpu::Texture Create2DTexture(uint32_t width,
                                  uint32_t height,
                                  uint32_t mipLevelCount,
                                  uint32_t arrayLayerCount,
                                  wgpu::TextureFormat format,
                                  wgpu::TextureUsage usage,
                                  uint32_t sampleCount = 1) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size.width = width;
        descriptor.size.height = height;
        descriptor.size.depthOrArrayLayers = arrayLayerCount;
        descriptor.sampleCount = sampleCount;
        descriptor.format = format;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = usage;
        wgpu::Texture tex = device.CreateTexture(&descriptor);
        return tex;
    }

    wgpu::Texture Create3DTexture(uint32_t width,
                                  uint32_t height,
                                  uint32_t depth,
                                  uint32_t mipLevelCount,
                                  wgpu::TextureFormat format,
                                  wgpu::TextureUsage usage) {
        wgpu::TextureDescriptor descriptor;
        descriptor.dimension = wgpu::TextureDimension::e3D;
        descriptor.size.width = width;
        descriptor.size.height = height;
        descriptor.size.depthOrArrayLayers = depth;
        descriptor.format = format;
        descriptor.mipLevelCount = mipLevelCount;
        descriptor.usage = usage;
        wgpu::Texture tex = device.CreateTexture(&descriptor);
        return tex;
    }

    uint32_t BufferSizeForTextureCopy(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm) {
        uint32_t bytesPerPixel = utils::GetTexelBlockSizeInBytes(format);
        uint32_t bytesPerRow = Align(width * bytesPerPixel, kTextureBytesPerRowAlignment);
        return (bytesPerRow * (height - 1) + width * bytesPerPixel) * depth;
    }

    void ValidateExpectation(wgpu::CommandEncoder encoder, utils::Expectation expectation) {
        if (expectation == utils::Expectation::Success) {
            encoder.Finish();
        } else {
            ASSERT_DEVICE_ERROR(encoder.Finish());
        }
    }

    void TestB2TCopy(utils::Expectation expectation,
                     wgpu::Buffer srcBuffer,
                     uint64_t srcOffset,
                     uint32_t srcBytesPerRow,
                     uint32_t srcRowsPerImage,
                     wgpu::Texture destTexture,
                     uint32_t destLevel,
                     wgpu::Origin3D destOrigin,
                     wgpu::Extent3D extent3D,
                     wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(srcBuffer, srcOffset, srcBytesPerRow, srcRowsPerImage);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(destTexture, destLevel, destOrigin, aspect);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &extent3D);

        ValidateExpectation(encoder, expectation);
    }

    void TestT2BCopy(utils::Expectation expectation,
                     wgpu::Texture srcTexture,
                     uint32_t srcLevel,
                     wgpu::Origin3D srcOrigin,
                     wgpu::Buffer destBuffer,
                     uint64_t destOffset,
                     uint32_t destBytesPerRow,
                     uint32_t destRowsPerImage,
                     wgpu::Extent3D extent3D,
                     wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            destBuffer, destOffset, destBytesPerRow, destRowsPerImage);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, srcLevel, srcOrigin, aspect);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &extent3D);

        ValidateExpectation(encoder, expectation);
    }

    void TestT2TCopy(utils::Expectation expectation,
                     wgpu::Texture srcTexture,
                     uint32_t srcLevel,
                     wgpu::Origin3D srcOrigin,
                     wgpu::Texture dstTexture,
                     uint32_t dstLevel,
                     wgpu::Origin3D dstOrigin,
                     wgpu::Extent3D extent3D,
                     wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
        wgpu::TexelCopyTextureInfo srcTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(srcTexture, srcLevel, srcOrigin, aspect);
        wgpu::TexelCopyTextureInfo dstTexelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(dstTexture, dstLevel, dstOrigin, aspect);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToTexture(&srcTexelCopyTextureInfo, &dstTexelCopyTextureInfo, &extent3D);

        ValidateExpectation(encoder, expectation);
    }

    void TestBothTBCopies(utils::Expectation expectation,
                          wgpu::Buffer buffer,
                          uint64_t bufferOffset,
                          uint32_t bufferBytesPerRow,
                          uint32_t rowsPerImage,
                          wgpu::Texture texture,
                          uint32_t level,
                          wgpu::Origin3D origin,
                          wgpu::Extent3D extent3D) {
        TestB2TCopy(expectation, buffer, bufferOffset, bufferBytesPerRow, rowsPerImage, texture,
                    level, origin, extent3D);
        TestT2BCopy(expectation, texture, level, origin, buffer, bufferOffset, bufferBytesPerRow,
                    rowsPerImage, extent3D);
    }

    void TestBothT2TCopies(utils::Expectation expectation,
                           wgpu::Texture texture1,
                           uint32_t level1,
                           wgpu::Origin3D origin1,
                           wgpu::Texture texture2,
                           uint32_t level2,
                           wgpu::Origin3D origin2,
                           wgpu::Extent3D extent3D) {
        TestT2TCopy(expectation, texture1, level1, origin1, texture2, level2, origin2, extent3D);
        TestT2TCopy(expectation, texture2, level2, origin2, texture1, level1, origin1, extent3D);
    }

    void TestBothTBCopiesExactBufferSize(uint32_t bufferBytesPerRow,
                                         uint32_t rowsPerImage,
                                         wgpu::Texture texture,
                                         wgpu::TextureFormat textureFormat,
                                         wgpu::Origin3D origin,
                                         wgpu::Extent3D extent3D) {
        // Check the minimal valid bufferSize.
        uint64_t bufferSize =
            utils::RequiredBytesInCopy(bufferBytesPerRow, rowsPerImage, extent3D, textureFormat);
        wgpu::Buffer source =
            CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
        TestBothTBCopies(utils::Expectation::Success, source, 0, bufferBytesPerRow, rowsPerImage,
                         texture, 0, origin, extent3D);

        // Check bufferSize was indeed minimal.
        uint64_t invalidSize = bufferSize - 1;
        wgpu::Buffer invalidSource =
            CreateBuffer(invalidSize, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
        TestBothTBCopies(utils::Expectation::Failure, invalidSource, 0, bufferBytesPerRow,
                         rowsPerImage, texture, 0, origin, extent3D);
    }
};

// Test copies between buffer and multiple array layers of an uncompressed texture
TEST_F(CopyCommandTest, CopyToMultipleArrayLayers) {
    wgpu::Texture destination =
        CopyCommandTest::Create2DTexture(4, 2, 1, 5, wgpu::TextureFormat::RGBA8Unorm,
                                         wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

    // Copy to all array layers
    TestBothTBCopiesExactBufferSize(256, 2, destination, wgpu::TextureFormat::RGBA8Unorm, {0, 0, 0},
                                    {4, 2, 5});

    // Copy to the highest array layer
    TestBothTBCopiesExactBufferSize(256, 2, destination, wgpu::TextureFormat::RGBA8Unorm, {0, 0, 4},
                                    {4, 2, 1});

    // Copy to array layers in the middle
    TestBothTBCopiesExactBufferSize(256, 2, destination, wgpu::TextureFormat::RGBA8Unorm, {0, 0, 1},
                                    {4, 2, 3});

    // Copy with a non-packed rowsPerImage
    TestBothTBCopiesExactBufferSize(256, 3, destination, wgpu::TextureFormat::RGBA8Unorm, {0, 0, 0},
                                    {4, 2, 5});

    // Copy with bytesPerRow = 512
    TestBothTBCopiesExactBufferSize(512, 2, destination, wgpu::TextureFormat::RGBA8Unorm, {0, 0, 1},
                                    {4, 2, 3});
}

class CopyCommandTest_B2B : public CopyCommandTest {};

// TODO(cwallez@chromium.org): Test that copies are forbidden inside renderpasses

// Test a successfull B2B copy
TEST_F(CopyCommandTest_B2B, Success) {
    wgpu::Buffer source = CreateBuffer(16, wgpu::BufferUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    // Copy different copies, including some that touch the OOB condition
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(source, 0, destination, 0, 16);
        encoder.CopyBufferToBuffer(source, 8, destination, 0, 8);
        encoder.CopyBufferToBuffer(source, 0, destination, 8, 8);
        encoder.Finish();
    }

    // Empty copies are valid
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(source, 0, destination, 0, 0);
        encoder.CopyBufferToBuffer(source, 0, destination, 16, 0);
        encoder.CopyBufferToBuffer(source, 16, destination, 0, 0);
        encoder.Finish();
    }
}

// Test a successful B2B copy where the last external reference is dropped.
// This is a regression test for crbug.com/1217741 where submitting a command
// buffer with dropped resources when the copy size is 0 was a use-after-free.
TEST_F(CopyCommandTest_B2B, DroppedBuffer) {
    wgpu::Buffer source = CreateBuffer(16, wgpu::BufferUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(source, 0, destination, 0, 0);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();

    source = nullptr;
    destination = nullptr;
    device.GetQueue().Submit(1, &commandBuffer);
}

// Test B2B copies with OOB
TEST_F(CopyCommandTest_B2B, OutOfBounds) {
    wgpu::Buffer source = CreateBuffer(16, wgpu::BufferUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    // OOB on the source
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(source, 8, destination, 0, 12);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // OOB on the destination
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(source, 0, destination, 8, 12);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test B2B copies with incorrect buffer usage
TEST_F(CopyCommandTest_B2B, BadUsage) {
    wgpu::Buffer source = CreateBuffer(16, wgpu::BufferUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);
    wgpu::Buffer vertex = CreateBuffer(16, wgpu::BufferUsage::Vertex);

    // Source with incorrect usage
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(vertex, 0, destination, 0, 16);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Destination with incorrect usage
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(source, 0, vertex, 0, 16);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test B2B copies with unaligned data size
TEST_F(CopyCommandTest_B2B, UnalignedSize) {
    wgpu::Buffer source = CreateBuffer(16, wgpu::BufferUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(source, 8, destination, 0, sizeof(uint8_t));
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test B2B copies with unaligned offset
TEST_F(CopyCommandTest_B2B, UnalignedOffset) {
    wgpu::Buffer source = CreateBuffer(16, wgpu::BufferUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    // Unaligned source offset
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(source, 9, destination, 0, 4);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Unaligned destination offset
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(source, 8, destination, 1, 4);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test B2B copies with buffers in error state cause errors.
TEST_F(CopyCommandTest_B2B, BuffersInErrorState) {
    wgpu::BufferDescriptor errorBufferDescriptor;
    errorBufferDescriptor.size = 4;
    errorBufferDescriptor.usage =
        wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    ASSERT_DEVICE_ERROR(wgpu::Buffer errorBuffer = device.CreateBuffer(&errorBufferDescriptor));

    constexpr uint64_t bufferSize = 4;
    wgpu::Buffer validBuffer =
        CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(errorBuffer, 0, validBuffer, 0, 4);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(validBuffer, 0, errorBuffer, 0, 4);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test it is not allowed to do B2B copies within same buffer.
TEST_F(CopyCommandTest_B2B, CopyWithinSameBuffer) {
    constexpr uint32_t kBufferSize = 16u;
    wgpu::Buffer buffer =
        CreateBuffer(kBufferSize, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    // srcOffset < dstOffset, and srcOffset + copySize > dstOffset (overlapping)
    {
        constexpr uint32_t kSrcOffset = 0u;
        constexpr uint32_t kDstOffset = 4u;
        constexpr uint32_t kCopySize = 8u;
        DAWN_ASSERT(kDstOffset > kSrcOffset && kDstOffset < kSrcOffset + kCopySize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(buffer, kSrcOffset, buffer, kDstOffset, kCopySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // srcOffset < dstOffset, and srcOffset + copySize == dstOffset (not overlapping)
    {
        constexpr uint32_t kSrcOffset = 0u;
        constexpr uint32_t kDstOffset = 8u;
        constexpr uint32_t kCopySize = kDstOffset - kSrcOffset;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(buffer, kSrcOffset, buffer, kDstOffset, kCopySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // srcOffset > dstOffset, and srcOffset < dstOffset + copySize (overlapping)
    {
        constexpr uint32_t kSrcOffset = 4u;
        constexpr uint32_t kDstOffset = 0u;
        constexpr uint32_t kCopySize = 8u;
        DAWN_ASSERT(kSrcOffset > kDstOffset && kSrcOffset < kDstOffset + kCopySize);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(buffer, kSrcOffset, buffer, kDstOffset, kCopySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // srcOffset > dstOffset, and srcOffset + copySize == dstOffset (not overlapping)
    {
        constexpr uint32_t kSrcOffset = 8u;
        constexpr uint32_t kDstOffset = 0u;
        constexpr uint32_t kCopySize = kSrcOffset - kDstOffset;
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(buffer, kSrcOffset, buffer, kDstOffset, kCopySize);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

class CopyCommandTest_B2T : public CopyCommandTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Depth32FloatStencil8};
    }
};

// Test a successfull B2T copy
TEST_F(CopyCommandTest_B2T, Success) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // Different copies, including some that touch the OOB condition
    {
        // Copy 4x4 block in corner of first mip.
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 4, destination, 0, {0, 0, 0},
                    {4, 4, 1});
        // Copy 4x4 block in opposite corner of first mip.
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 4, destination, 0, {12, 12, 0},
                    {4, 4, 1});
        // Copy 4x4 block in the 4x4 mip.
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 4, destination, 2, {0, 0, 0},
                    {4, 4, 1});
        // Copy with a buffer offset
        TestB2TCopy(utils::Expectation::Success, source, bufferSize - 4, 256, 1, destination, 0,
                    {0, 0, 0}, {1, 1, 1});
        TestB2TCopy(utils::Expectation::Success, source, bufferSize - 4, 256,
                    wgpu::kCopyStrideUndefined, destination, 0, {0, 0, 0}, {1, 1, 1});
    }

    // Copies with a 256-byte aligned bytes per row but unaligned texture region
    {
        // Unaligned region
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 4, destination, 0, {0, 0, 0},
                    {3, 4, 1});
        // Unaligned region with texture offset
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 3, destination, 0, {5, 7, 0},
                    {2, 3, 1});
        // Unaligned region, with buffer offset
        TestB2TCopy(utils::Expectation::Success, source, 31 * 4, 256, 3, destination, 0, {0, 0, 0},
                    {3, 3, 1});
    }

    // bytesPerRow is undefined
    {
        TestB2TCopy(utils::Expectation::Success, source, 0, wgpu::kCopyStrideUndefined, 2,
                    destination, 0, {0, 0, 0}, {1, 1, 1});
        TestB2TCopy(utils::Expectation::Success, source, 0, wgpu::kCopyStrideUndefined, 2,
                    destination, 0, {0, 0, 0}, {3, 1, 1});
        // Fail because height or depth is greater than 1:
        TestB2TCopy(utils::Expectation::Failure, source, 0, wgpu::kCopyStrideUndefined, 2,
                    destination, 0, {0, 0, 0}, {1, 2, 1});
        TestB2TCopy(utils::Expectation::Failure, source, 0, wgpu::kCopyStrideUndefined, 2,
                    destination, 0, {0, 0, 0}, {1, 1, 2});
    }

    // Empty copies are valid
    {
        // An empty copy
        TestB2TCopy(utils::Expectation::Success, source, 0, 0, 0, destination, 0, {0, 0, 0},
                    {0, 0, 1});
        TestB2TCopy(utils::Expectation::Success, source, 0, wgpu::kCopyStrideUndefined, 0,
                    destination, 0, {0, 0, 0}, {0, 0, 1});
        // An empty copy with depth = 0
        TestB2TCopy(utils::Expectation::Success, source, 0, 0, 0, destination, 0, {0, 0, 0},
                    {0, 0, 0});
        TestB2TCopy(utils::Expectation::Success, source, 0, wgpu::kCopyStrideUndefined, 0,
                    destination, 0, {0, 0, 0}, {0, 0, 0});
        // An empty copy touching the end of the buffer
        TestB2TCopy(utils::Expectation::Success, source, bufferSize, 0, 0, destination, 0,
                    {0, 0, 0}, {0, 0, 1});
        TestB2TCopy(utils::Expectation::Success, source, bufferSize, wgpu::kCopyStrideUndefined, 0,
                    destination, 0, {0, 0, 0}, {0, 0, 1});
        // An empty copy touching the side of the texture
        TestB2TCopy(utils::Expectation::Success, source, 0, 0, 0, destination, 0, {16, 16, 0},
                    {0, 0, 1});
        TestB2TCopy(utils::Expectation::Success, source, 0, wgpu::kCopyStrideUndefined, 0,
                    destination, 0, {16, 16, 0}, {0, 0, 1});

        // An empty copy with depth = 1 and bytesPerRow > 0
        TestB2TCopy(utils::Expectation::Success, source, 0, kTextureBytesPerRowAlignment, 0,
                    destination, 0, {0, 0, 0}, {0, 0, 1});
        // An empty copy with height > 0, depth = 0, bytesPerRow > 0 and rowsPerImage > 0
        TestB2TCopy(utils::Expectation::Success, source, 0, kTextureBytesPerRowAlignment, 3,
                    destination, 0, {0, 0, 0}, {0, 1, 0});
    }
}

// Test OOB conditions on the buffer
TEST_F(CopyCommandTest_B2T, OutOfBoundsOnBuffer) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // OOB on the buffer because we copy too many pixels
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 5, destination, 0, {0, 0, 0},
                {4, 5, 1});

    // OOB on the buffer because of the offset
    TestB2TCopy(utils::Expectation::Failure, source, 4, 256, 4, destination, 0, {0, 0, 0},
                {4, 4, 1});

    // OOB on the buffer because (bytes per row * (height - 1) + width * bytesPerPixel) * depth
    // overflows
    TestB2TCopy(utils::Expectation::Failure, source, 0, 512, 3, destination, 0, {0, 0, 0},
                {4, 3, 1});

    // Not OOB on the buffer although bytes per row * height overflows
    // but (bytes per row * (height - 1) + width * bytesPerPixel) * depth does not overflow
    {
        uint32_t sourceBufferSize = BufferSizeForTextureCopy(7, 3, 1);
        ASSERT_TRUE(256 * 3 > sourceBufferSize) << "bytes per row * height should overflow buffer";
        wgpu::Buffer sourceBuffer = CreateBuffer(sourceBufferSize, wgpu::BufferUsage::CopySrc);

        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 3, destination, 0, {0, 0, 0},
                    {7, 3, 1});
    }
}

// Test OOB conditions on the texture
TEST_F(CopyCommandTest_B2T, OutOfBoundsOnTexture) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // OOB on the texture because x + width overflows
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 4, destination, 0, {13, 12, 0},
                {4, 4, 1});

    // OOB on the texture because y + width overflows
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 4, destination, 0, {12, 13, 0},
                {4, 4, 1});

    // OOB on the texture because we overflow a non-zero mip
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 4, destination, 2, {1, 0, 0},
                {4, 4, 1});

    // OOB on the texture even on an empty copy when we copy to a non-existent mip.
    TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 0, destination, 5, {0, 0, 0}, {0, 0, 1});

    // OOB on the texture because slice overflows
    TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 0, destination, 0, {0, 0, 2}, {0, 0, 1});
}

// Test that we force Depth=1 on copies to 2D textures
TEST_F(CopyCommandTest_B2T, DepthConstraintFor2DTextures) {
    wgpu::Buffer source = CreateBuffer(16 * 4, wgpu::BufferUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // Depth > 1 on an empty copy still errors
    TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 0, destination, 0, {0, 0, 0}, {0, 0, 2});
}

// Test B2T copies with incorrect buffer usage
TEST_F(CopyCommandTest_B2T, IncorrectUsage) {
    wgpu::Buffer source = CreateBuffer(16 * 4, wgpu::BufferUsage::CopySrc);
    wgpu::Buffer vertex = CreateBuffer(16 * 4, wgpu::BufferUsage::Vertex);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);
    wgpu::Texture sampled = Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                                            wgpu::TextureUsage::TextureBinding);

    // Incorrect source usage
    TestB2TCopy(utils::Expectation::Failure, vertex, 0, 256, 4, destination, 0, {0, 0, 0},
                {4, 4, 1});

    // Incorrect destination usage
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 4, sampled, 0, {0, 0, 0}, {4, 4, 1});
}

TEST_F(CopyCommandTest_B2T, BytesPerRowConstraints) {
    uint64_t bufferSize = BufferSizeForTextureCopy(128, 16, 1);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);
    wgpu::Texture destination = Create2DTexture(128, 16, 5, 5, wgpu::TextureFormat::RGBA8Unorm,
                                                wgpu::TextureUsage::CopyDst);

    // bytes per row is 0
    {
        // copyHeight > 1
        TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 4, destination, 0, {0, 0, 0},
                    {64, 4, 1});
        TestB2TCopy(utils::Expectation::Success, source, 0, 0, 4, destination, 0, {0, 0, 0},
                    {0, 4, 1});

        // copyDepth > 1
        TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 1, destination, 0, {0, 0, 0},
                    {64, 1, 4});
        TestB2TCopy(utils::Expectation::Success, source, 0, 0, 1, destination, 0, {0, 0, 0},
                    {0, 1, 4});

        // copyHeight = 1 and copyDepth = 1
        TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 1, destination, 0, {0, 0, 0},
                    {64, 1, 1});
    }

    // bytes per row is not 256-byte aligned
    {
        // copyHeight > 1
        TestB2TCopy(utils::Expectation::Failure, source, 0, 128, 4, destination, 0, {0, 0, 0},
                    {4, 4, 1});

        // copyHeight = 1 and copyDepth = 1
        TestB2TCopy(utils::Expectation::Failure, source, 0, 128, 1, destination, 0, {0, 0, 0},
                    {4, 1, 1});
    }

    // bytes per row is less than width * bytesPerPixel
    {
        // copyHeight > 1
        TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 2, destination, 0, {0, 0, 0},
                    {65, 2, 1});
        // copyHeight == 0
        TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 0, destination, 0, {0, 0, 0},
                    {65, 0, 1});

        // copyDepth > 1
        TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 1, destination, 0, {0, 0, 0},
                    {65, 1, 2});
        // copyDepth == 0
        TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 1, destination, 0, {0, 0, 0},
                    {65, 1, 0});

        // copyHeight = 1 and copyDepth = 1
        TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 1, destination, 0, {0, 0, 0},
                    {65, 1, 1});
    }
}

TEST_F(CopyCommandTest_B2T, RowsPerImageConstraints) {
    uint64_t bufferSize = BufferSizeForTextureCopy(5, 5, 6);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 1, 5, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // rowsPerImage is zero
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 0, destination, 0, {0, 0, 0},
                {1, 1, 1});
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 0, destination, 0, {0, 0, 0},
                {4, 4, 1});

    // rowsPerImage is undefined
    TestB2TCopy(utils::Expectation::Success, source, 0, 256, wgpu::kCopyStrideUndefined,
                destination, 0, {0, 0, 0}, {4, 4, 1});
    // Fail because depth > 1:
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, wgpu::kCopyStrideUndefined,
                destination, 0, {0, 0, 0}, {4, 4, 2});

    // rowsPerImage is equal to copy height (Valid)
    TestB2TCopy(utils::Expectation::Success, source, 0, 256, 4, destination, 0, {0, 0, 0},
                {4, 4, 1});
    TestB2TCopy(utils::Expectation::Success, source, 0, 256, 4, destination, 0, {0, 0, 0},
                {4, 4, 2});

    // rowsPerImage is larger than copy height (Valid)
    TestB2TCopy(utils::Expectation::Success, source, 0, 256, 5, destination, 0, {0, 0, 0},
                {4, 4, 1});
    TestB2TCopy(utils::Expectation::Success, source, 0, 256, 5, destination, 0, {0, 0, 0},
                {4, 4, 2});

    // rowsPerImage is less than copy height (Invalid)
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 3, destination, 0, {0, 0, 0},
                {4, 4, 1});
}

// Test B2T copies with incorrect buffer offset usage for color texture
TEST_F(CopyCommandTest_B2T, IncorrectBufferOffsetForColorTexture) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // Correct usage
    TestB2TCopy(utils::Expectation::Success, source, bufferSize - 4, 256, 1, destination, 0,
                {0, 0, 0}, {1, 1, 1});

    // Incorrect usages
    {
        TestB2TCopy(utils::Expectation::Failure, source, bufferSize - 5, 256, 1, destination, 0,
                    {0, 0, 0}, {1, 1, 1});
        TestB2TCopy(utils::Expectation::Failure, source, bufferSize - 6, 256, 1, destination, 0,
                    {0, 0, 0}, {1, 1, 1});
        TestB2TCopy(utils::Expectation::Failure, source, bufferSize - 7, 256, 1, destination, 0,
                    {0, 0, 0}, {1, 1, 1});
    }
}

// Test B2T copies with incorrect buffer offset usage for depth-stencil texture
TEST_F(CopyCommandTest_B2T, IncorrectBufferOffsetForDepthStencilTexture) {
    // TODO(dawn:570, dawn:666): List other valid parameters after missing texture formats
    // are implemented, e.g. Stencil8.
    std::array<std::tuple<wgpu::TextureFormat, wgpu::TextureAspect>, 4> params = {
        std::make_tuple(wgpu::TextureFormat::Depth16Unorm, wgpu::TextureAspect::DepthOnly),
        std::make_tuple(wgpu::TextureFormat::Depth16Unorm, wgpu::TextureAspect::All),
        std::make_tuple(wgpu::TextureFormat::Depth24PlusStencil8, wgpu::TextureAspect::StencilOnly),
        std::make_tuple(wgpu::TextureFormat::Depth32FloatStencil8,
                        wgpu::TextureAspect::StencilOnly),
    };

    uint64_t bufferSize = BufferSizeForTextureCopy(32, 32, 1);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);

    for (auto param : params) {
        wgpu::TextureFormat textureFormat = std::get<0>(param);
        wgpu::TextureAspect textureAspect = std::get<1>(param);

        wgpu::Texture destination =
            Create2DTexture(16, 16, 5, 1, textureFormat, wgpu::TextureUsage::CopyDst);

        for (uint64_t srcOffset = 0; srcOffset < 8; srcOffset++) {
            utils::Expectation expectation =
                (srcOffset % 4 == 0) ? utils::Expectation::Success : utils::Expectation::Failure;
            TestB2TCopy(expectation, source, srcOffset, 256, 16, destination, 0, {0, 0, 0},
                        {16, 16, 1}, textureAspect);
        }
    }
}

// Test multisampled textures cannot be used in B2T copies.
TEST_F(CopyCommandTest_B2T, CopyToMultisampledTexture) {
    uint64_t bufferSize = BufferSizeForTextureCopy(16, 16, 1);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(2, 2, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment, 4);

    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 2, destination, 0, {0, 0, 0},
                {2, 2, 1});
}

// Test B2T copies with buffer or texture in error state causes errors.
TEST_F(CopyCommandTest_B2T, BufferOrTextureInErrorState) {
    wgpu::BufferDescriptor errorBufferDescriptor;
    errorBufferDescriptor.size = 4;
    errorBufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopySrc;
    ASSERT_DEVICE_ERROR(wgpu::Buffer errorBuffer = device.CreateBuffer(&errorBufferDescriptor));

    wgpu::TextureDescriptor errorTextureDescriptor;
    errorTextureDescriptor.size.depthOrArrayLayers = 0;
    ASSERT_DEVICE_ERROR(wgpu::Texture errorTexture = device.CreateTexture(&errorTextureDescriptor));

    wgpu::TexelCopyBufferInfo errorTexelCopyBufferInfo =
        utils::CreateTexelCopyBufferInfo(errorBuffer, 0, 0, 0);
    wgpu::TexelCopyTextureInfo errorTexelCopyTextureInfo =
        utils::CreateTexelCopyTextureInfo(errorTexture, 0, {0, 0, 0});

    wgpu::Extent3D extent3D = {0, 0, 0};

    {
        wgpu::Texture destination = Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                                                    wgpu::TextureUsage::CopyDst);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(destination, 0, {0, 0, 0});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&errorTexelCopyBufferInfo, &texelCopyTextureInfo, &extent3D);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
        wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);

        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(source, 0, 0, 0);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&texelCopyBufferInfo, &errorTexelCopyTextureInfo, &extent3D);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Regression tests for a bug in the computation of texture copy buffer size in Dawn.
TEST_F(CopyCommandTest_B2T, TextureCopyBufferSizeLastRowComputation) {
    constexpr uint32_t kBytesPerRow = 256;
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    constexpr std::array<wgpu::TextureFormat, 2> kFormats = {wgpu::TextureFormat::RGBA8Unorm,
                                                             wgpu::TextureFormat::RG8Unorm};

    {
        // kBytesPerRow * (kHeight - 1) + kWidth is not large enough to be the valid buffer size in
        // this test because the buffer sizes in B2T copies are not in texels but in bytes.
        constexpr uint32_t kInvalidBufferSize = kBytesPerRow * (kHeight - 1) + kWidth;

        for (wgpu::TextureFormat format : kFormats) {
            wgpu::Buffer source = CreateBuffer(kInvalidBufferSize, wgpu::BufferUsage::CopySrc);
            wgpu::Texture destination =
                Create2DTexture(kWidth, kHeight, 1, 1, format, wgpu::TextureUsage::CopyDst);
            TestB2TCopy(utils::Expectation::Failure, source, 0, kBytesPerRow, kHeight, destination,
                        0, {0, 0, 0}, {kWidth, kHeight, 1});
        }
    }

    {
        for (wgpu::TextureFormat format : kFormats) {
            uint32_t validBufferSize = BufferSizeForTextureCopy(kWidth, kHeight, 1, format);
            wgpu::Texture destination =
                Create2DTexture(kWidth, kHeight, 1, 1, format, wgpu::TextureUsage::CopyDst);

            // Verify the return value of BufferSizeForTextureCopy() is exactly the minimum valid
            // buffer size in this test.
            {
                uint32_t invalidBuffferSize = validBufferSize - 1;
                wgpu::Buffer source = CreateBuffer(invalidBuffferSize, wgpu::BufferUsage::CopySrc);
                TestB2TCopy(utils::Expectation::Failure, source, 0, kBytesPerRow, kHeight,
                            destination, 0, {0, 0, 0}, {kWidth, kHeight, 1});
            }

            {
                wgpu::Buffer source = CreateBuffer(validBufferSize, wgpu::BufferUsage::CopySrc);
                TestB2TCopy(utils::Expectation::Success, source, 0, kBytesPerRow, kHeight,
                            destination, 0, {0, 0, 0}, {kWidth, kHeight, 1});
            }
        }
    }
}

// Test copy from buffer to mip map of non square texture
TEST_F(CopyCommandTest_B2T, CopyToMipmapOfNonSquareTexture) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 2, 1);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);
    uint32_t maxMipmapLevel = 3;
    wgpu::Texture destination = Create2DTexture(
        4, 2, maxMipmapLevel, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // Copy to top level mip map
    TestB2TCopy(utils::Expectation::Success, source, 0, 256, 1, destination, maxMipmapLevel - 1,
                {0, 0, 0}, {1, 1, 1});
    // Copy to high level mip map
    TestB2TCopy(utils::Expectation::Success, source, 0, 256, 1, destination, maxMipmapLevel - 2,
                {0, 0, 0}, {2, 1, 1});
    // Mip level out of range
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 1, destination, maxMipmapLevel,
                {0, 0, 0}, {1, 1, 1});
    // Copy origin out of range
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 1, destination, maxMipmapLevel - 2,
                {1, 0, 0}, {2, 1, 1});
    // Copy size out of range
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 2, destination, maxMipmapLevel - 2,
                {0, 0, 0}, {2, 2, 1});
}

// Test whether or not it is valid to copy to a depth texture
TEST_F(CopyCommandTest_B2T, CopyToDepthAspect) {
    uint64_t bufferSize = BufferSizeForTextureCopy(16, 16, 1, wgpu::TextureFormat::Depth32Float);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);

    constexpr std::array<wgpu::TextureFormat, 1> kAllowBufferToDepthCopyFormats = {
        wgpu::TextureFormat::Depth16Unorm};

    for (wgpu::TextureFormat format : kAllowBufferToDepthCopyFormats) {
        wgpu::Texture destination =
            Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopyDst);

        // Test it is valid to copy this format from a buffer into a depth texture
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 16, destination, 0, {0, 0, 0},
                    {16, 16, 1}, wgpu::TextureAspect::DepthOnly);
        if (utils::IsDepthOnlyFormat(format)) {
            // Test "all" of a depth texture which is only the depth aspect.
            TestB2TCopy(utils::Expectation::Success, source, 0, 256, 16, destination, 0, {0, 0, 0},
                        {16, 16, 1}, wgpu::TextureAspect::All);
        }
    }

    constexpr std::array<wgpu::TextureFormat, 4> kDisallowBufferToDepthCopyFormats = {
        wgpu::TextureFormat::Depth32Float,
        wgpu::TextureFormat::Depth24Plus,
        wgpu::TextureFormat::Depth24PlusStencil8,
        wgpu::TextureFormat::Depth32FloatStencil8,
    };

    for (wgpu::TextureFormat format : kDisallowBufferToDepthCopyFormats) {
        wgpu::Texture destination =
            Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopyDst);

        // Test it is invalid to copy from a buffer into a depth texture
        TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 16, destination, 0, {0, 0, 0},
                    {16, 16, 1}, wgpu::TextureAspect::DepthOnly);

        if (utils::IsDepthOnlyFormat(format)) {
            // Test "all" of a depth texture which is only the depth aspect.
            TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 16, destination, 0, {0, 0, 0},
                        {16, 16, 1}, wgpu::TextureAspect::All);
        }
    }
}

// Test copy to only the stencil aspect of a texture
TEST_F(CopyCommandTest_B2T, CopyToStencilAspect) {
    uint64_t bufferSize = BufferSizeForTextureCopy(16, 16, 1, wgpu::TextureFormat::R8Uint);
    wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);

    for (wgpu::TextureFormat format : utils::kStencilFormats) {
        // Test it is valid to copy from a buffer into the stencil aspect of a depth/stencil texture
        {
            wgpu::Texture destination =
                Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopyDst);

            // TODO(dawn:666): Test "all" of Stencil8 format when it's implemented.

            TestB2TCopy(utils::Expectation::Success, source, 0, 256, 16, destination, 0, {0, 0, 0},
                        {16, 16, 1}, wgpu::TextureAspect::StencilOnly);

            // And that it fails if the buffer is one byte too small
            wgpu::Buffer sourceSmall = CreateBuffer(bufferSize - 1, wgpu::BufferUsage::CopySrc);
            TestB2TCopy(utils::Expectation::Failure, sourceSmall, 0, 256, 16, destination, 0,
                        {0, 0, 0}, {16, 16, 1}, wgpu::TextureAspect::StencilOnly);
        }

        // A copy fails when using a depth/stencil texture, and the entire subresource isn't copied
        {
            wgpu::Texture destination =
                Create2DTexture(16, 16, 1, 1, format,
                                wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

            TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 15, destination, 0, {0, 0, 0},
                        {15, 15, 1}, wgpu::TextureAspect::StencilOnly);

            TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 1, destination, 0, {0, 0, 0},
                        {1, 1, 1}, wgpu::TextureAspect::StencilOnly);
        }

        // Non-zero mip: A copy fails when using a depth/stencil texture, and the entire subresource
        // isn't copied
        {
            uint64_t mipBufferSize = BufferSizeForTextureCopy(8, 8, 1, wgpu::TextureFormat::R8Uint);
            wgpu::Buffer mipSource = CreateBuffer(mipBufferSize, wgpu::BufferUsage::CopySrc);

            wgpu::Texture destination =
                Create2DTexture(16, 16, 2, 1, format,
                                wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

            // Whole mip is success
            TestB2TCopy(utils::Expectation::Success, mipSource, 0, 256, 8, destination, 1,
                        {0, 0, 0}, {8, 8, 1}, wgpu::TextureAspect::StencilOnly);

            // Partial mip fails
            TestB2TCopy(utils::Expectation::Failure, mipSource, 0, 256, 7, destination, 1,
                        {0, 0, 0}, {7, 7, 1}, wgpu::TextureAspect::StencilOnly);

            TestB2TCopy(utils::Expectation::Failure, mipSource, 0, 256, 1, destination, 1,
                        {0, 0, 0}, {1, 1, 1}, wgpu::TextureAspect::StencilOnly);
        }

        // Non-zero mip, non-pow-2: A copy fails when using a depth/stencil texture, and the entire
        // subresource isn't copied
        {
            uint64_t mipBufferSize = BufferSizeForTextureCopy(8, 8, 1, wgpu::TextureFormat::R8Uint);
            wgpu::Buffer mipSource = CreateBuffer(mipBufferSize, wgpu::BufferUsage::CopySrc);

            wgpu::Texture destination =
                Create2DTexture(17, 17, 2, 1, format,
                                wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment);

            // Whole mip is success
            TestB2TCopy(utils::Expectation::Success, mipSource, 0, 256, 8, destination, 1,
                        {0, 0, 0}, {8, 8, 1}, wgpu::TextureAspect::StencilOnly);

            // Partial mip fails
            TestB2TCopy(utils::Expectation::Failure, mipSource, 0, 256, 7, destination, 1,
                        {0, 0, 0}, {7, 7, 1}, wgpu::TextureAspect::StencilOnly);

            TestB2TCopy(utils::Expectation::Failure, mipSource, 0, 256, 1, destination, 1,
                        {0, 0, 0}, {1, 1, 1}, wgpu::TextureAspect::StencilOnly);
        }
    }

    // Test it is invalid to copy from a buffer into the stencil aspect of Depth24Plus (no stencil)
    {
        wgpu::Texture destination = Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::Depth24Plus,
                                                    wgpu::TextureUsage::CopyDst);

        TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 16, destination, 0, {0, 0, 0},
                    {16, 16, 1}, wgpu::TextureAspect::StencilOnly);
    }

    // Test it is invalid to copy from a buffer into the stencil aspect of a color texture
    {
        wgpu::Texture destination = Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::RGBA8Uint,
                                                    wgpu::TextureUsage::CopyDst);

        TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 16, destination, 0, {0, 0, 0},
                    {16, 16, 1}, wgpu::TextureAspect::StencilOnly);
    }
}

// Test that CopyB2T throws an error when requiredBytesInCopy overflows uint64_t
TEST_F(CopyCommandTest_B2T, RequiredBytesInCopyOverflow) {
    wgpu::Buffer source = CreateBuffer(10000, wgpu::BufferUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(1, 1, 1, 16, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // Success
    TestB2TCopy(utils::Expectation::Success, source, 0, (1 << 31), (1 << 31), destination, 0,
                {0, 0, 0}, {1, 1, 1});
    // Failure because bytesPerImage * (depth - 1) overflows
    TestB2TCopy(utils::Expectation::Failure, source, 0, (1 << 31), (1 << 31), destination, 0,
                {0, 0, 0}, {1, 1, 16});
}

class CopyCommandTest_T2B : public CopyCommandTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Depth32FloatStencil8};
    }
};

// Test a successfull T2B copy
TEST_F(CopyCommandTest_T2B, Success) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    // Different copies, including some that touch the OOB condition
    {
        // Copy from 4x4 block in corner of first mip.
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256, 4,
                    {4, 4, 1});
        // Copy from 4x4 block in opposite corner of first mip.
        TestT2BCopy(utils::Expectation::Success, source, 0, {12, 12, 0}, destination, 0, 256, 4,
                    {4, 4, 1});
        // Copy from 4x4 block in the 4x4 mip.
        TestT2BCopy(utils::Expectation::Success, source, 2, {0, 0, 0}, destination, 0, 256, 4,
                    {4, 4, 1});
        // Copy with a buffer offset
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, bufferSize - 4,
                    256, 1, {1, 1, 1});
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, bufferSize - 4,
                    256, wgpu::kCopyStrideUndefined, {1, 1, 1});
    }

    // Copies with a 256-byte aligned bytes per row but unaligned texture region
    {
        // Unaligned region
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256, 4,
                    {3, 4, 1});
        // Unaligned region with texture offset
        TestT2BCopy(utils::Expectation::Success, source, 0, {5, 7, 0}, destination, 0, 256, 3,
                    {2, 3, 1});
        // Unaligned region, with buffer offset
        TestT2BCopy(utils::Expectation::Success, source, 2, {0, 0, 0}, destination, 31 * 4, 256, 3,
                    {3, 3, 1});
    }

    // bytesPerRow is undefined
    {
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                    wgpu::kCopyStrideUndefined, 2, {1, 1, 1});
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                    wgpu::kCopyStrideUndefined, 2, {3, 1, 1});
        // Fail because height or depth is greater than 1:
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                    wgpu::kCopyStrideUndefined, 2, {1, 2, 1});
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                    wgpu::kCopyStrideUndefined, 2, {1, 1, 2});
    }

    // Empty copies are valid
    {
        // An empty copy
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 0, 0,
                    {0, 0, 1});
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                    wgpu::kCopyStrideUndefined, 0, {0, 0, 1});
        // An empty copy with depth = 0
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 0, 0,
                    {0, 0, 0});
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                    wgpu::kCopyStrideUndefined, 0, {0, 0, 0});
        // An empty copy touching the end of the buffer
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, bufferSize, 0,
                    0, {0, 0, 1});
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, bufferSize,
                    wgpu::kCopyStrideUndefined, 0, {0, 0, 1});
        // An empty copy touching the side of the texture
        TestT2BCopy(utils::Expectation::Success, source, 0, {16, 16, 0}, destination, 0, 0, 0,
                    {0, 0, 1});
        TestT2BCopy(utils::Expectation::Success, source, 0, {16, 16, 0}, destination, 0,
                    wgpu::kCopyStrideUndefined, 0, {0, 0, 1});

        // An empty copy with depth = 1 and bytesPerRow > 0
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                    kTextureBytesPerRowAlignment, 0, {0, 0, 1});
        // An empty copy with height > 0, depth = 0, bytesPerRow > 0 and rowsPerImage > 0
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                    kTextureBytesPerRowAlignment, 3, {0, 1, 0});
    }
}

// Edge cases around requiredBytesInCopy computation for empty copies
TEST_F(CopyCommandTest_T2B, Empty) {
    wgpu::Texture source =
        Create2DTexture(16, 16, 1, 2, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);

    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0},
                CreateBuffer(0, wgpu::BufferUsage::CopyDst), 0, 256, 4, {0, 0, 0});
    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0},
                CreateBuffer(0, wgpu::BufferUsage::CopyDst), 0, 256, 4, {4, 0, 0});
    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0},
                CreateBuffer(0, wgpu::BufferUsage::CopyDst), 0, 256, 4, {4, 4, 0});

    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0},
                CreateBuffer(1024, wgpu::BufferUsage::CopyDst), 0, 256, 4, {4, 0, 2});
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0},
                CreateBuffer(1023, wgpu::BufferUsage::CopyDst), 0, 256, 4, {4, 0, 2});

    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0},
                CreateBuffer(1792, wgpu::BufferUsage::CopyDst), 0, 256, 4, {0, 4, 2});
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0},
                CreateBuffer(1791, wgpu::BufferUsage::CopyDst), 0, 256, 4, {0, 4, 2});

    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0},
                CreateBuffer(1024, wgpu::BufferUsage::CopyDst), 0, 256, 4, {0, 0, 2});
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0},
                CreateBuffer(1023, wgpu::BufferUsage::CopyDst), 0, 256, 4, {0, 0, 2});
}

// Test OOB conditions on the texture
TEST_F(CopyCommandTest_T2B, OutOfBoundsOnTexture) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    // OOB on the texture because x + width overflows
    TestT2BCopy(utils::Expectation::Failure, source, 0, {13, 12, 0}, destination, 0, 256, 4,
                {4, 4, 1});

    // OOB on the texture because y + width overflows
    TestT2BCopy(utils::Expectation::Failure, source, 0, {12, 13, 0}, destination, 0, 256, 4,
                {4, 4, 1});

    // OOB on the texture because we overflow a non-zero mip
    TestT2BCopy(utils::Expectation::Failure, source, 2, {1, 0, 0}, destination, 0, 256, 4,
                {4, 4, 1});

    // OOB on the texture even on an empty copy when we copy from a non-existent mip.
    TestT2BCopy(utils::Expectation::Failure, source, 5, {0, 0, 0}, destination, 0, 0, 4, {0, 0, 1});
}

// Test OOB conditions on the buffer
TEST_F(CopyCommandTest_T2B, OutOfBoundsOnBuffer) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    // OOB on the buffer because we copy too many pixels
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 5,
                {4, 5, 1});

    // OOB on the buffer because of the offset
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 4, 256, 4,
                {4, 4, 1});

    // OOB on the buffer because (bytes per row * (height - 1) + width * bytesPerPixel) * depth
    // overflows
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 512, 3,
                {4, 3, 1});

    // Not OOB on the buffer although bytes per row * height overflows
    // but (bytes per row * (height - 1) + width * bytesPerPixel) * depth does not overflow
    {
        uint32_t destinationBufferSize = BufferSizeForTextureCopy(7, 3, 1);
        ASSERT_TRUE(256 * 3 > destinationBufferSize)
            << "bytes per row * height should overflow buffer";
        wgpu::Buffer destinationBuffer =
            CreateBuffer(destinationBufferSize, wgpu::BufferUsage::CopyDst);
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destinationBuffer, 0, 256, 3,
                    {7, 3, 1});
    }
}

// Test that we force Depth=1 on copies from to 2D textures
TEST_F(CopyCommandTest_T2B, DepthConstraintFor2DTextures) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    // Depth > 1 on an empty copy still errors
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 0, 0, {0, 0, 2});
}

// Test T2B copies with incorrect buffer usage
TEST_F(CopyCommandTest_T2B, IncorrectUsage) {
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Texture sampled = Create2DTexture(16, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                                            wgpu::TextureUsage::TextureBinding);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);
    wgpu::Buffer vertex = CreateBuffer(bufferSize, wgpu::BufferUsage::Vertex);

    // Incorrect source usage
    TestT2BCopy(utils::Expectation::Failure, sampled, 0, {0, 0, 0}, destination, 0, 256, 4,
                {4, 4, 1});

    // Incorrect destination usage
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, vertex, 0, 256, 4, {4, 4, 1});
}

TEST_F(CopyCommandTest_T2B, BytesPerRowConstraints) {
    uint64_t bufferSize = BufferSizeForTextureCopy(128, 16, 1);
    wgpu::Texture source = Create2DTexture(128, 16, 5, 5, wgpu::TextureFormat::RGBA8Unorm,
                                           wgpu::TextureUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    // bytes per row is 0
    {
        // copyHeight > 1
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 0, 4,
                    {64, 4, 1});
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 0, 4,
                    {0, 4, 1});

        // copyDepth > 1
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 0, 1,
                    {64, 1, 4});
        TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 0, 1,
                    {0, 1, 4});

        // copyHeight = 1 and copyDepth = 1
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 0, 1,
                    {64, 1, 1});
    }

    // bytes per row is not 256-byte aligned
    {
        // copyHeight > 1
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 128, 4,
                    {4, 4, 1});

        // copyHeight = 1 and copyDepth = 1
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 128, 1,
                    {4, 1, 1});
    }

    // bytes per row is less than width * bytesPerPixel
    {
        // copyHeight > 1
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 2,
                    {65, 2, 1});
        // copyHeight == 0
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 0,
                    {65, 0, 1});

        // copyDepth > 1
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 1,
                    {65, 1, 2});
        // copyDepth == 0
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 1,
                    {65, 1, 0});

        // copyHeight = 1 and copyDepth = 1
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 1,
                    {65, 1, 1});
    }
}

TEST_F(CopyCommandTest_T2B, RowsPerImageConstraints) {
    uint64_t bufferSize = BufferSizeForTextureCopy(5, 5, 6);
    wgpu::Texture source =
        Create2DTexture(16, 16, 1, 5, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    // rowsPerImage is zero (Valid)
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 0,
                {1, 1, 1});
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 0,
                {4, 4, 1});

    // rowsPerImage is undefined
    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256,
                wgpu::kCopyStrideUndefined, {4, 4, 1});
    // Fail because depth > 1:
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256,
                wgpu::kCopyStrideUndefined, {4, 4, 2});

    // rowsPerImage is equal to copy height (Valid)
    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256, 4,
                {4, 4, 1});
    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256, 4,
                {4, 4, 2});

    // rowsPerImage exceeds copy height (Valid)
    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256, 5,
                {4, 4, 1});
    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256, 5,
                {4, 4, 2});

    // rowsPerImage is less than copy height (Invalid)
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 3,
                {4, 4, 1});
}

// Test T2B copies with incorrect buffer offset usage for color texture
TEST_F(CopyCommandTest_T2B, IncorrectBufferOffsetForColorTexture) {
    uint64_t bufferSize = BufferSizeForTextureCopy(128, 16, 1);
    wgpu::Texture source = Create2DTexture(128, 16, 5, 1, wgpu::TextureFormat::RGBA8Unorm,
                                           wgpu::TextureUsage::CopySrc);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    // Correct usage
    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, bufferSize - 4, 256,
                1, {1, 1, 1});

    // Incorrect usages
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, bufferSize - 5, 256,
                1, {1, 1, 1});
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, bufferSize - 6, 256,
                1, {1, 1, 1});
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, bufferSize - 7, 256,
                1, {1, 1, 1});
}

// Test T2B copies with incorrect buffer offset usage for depth-stencil texture
TEST_F(CopyCommandTest_T2B, IncorrectBufferOffsetForDepthStencilTexture) {
    // TODO(dawn:570, dawn:666): List other valid parameters after missing texture formats
    // are implemented, e.g. Stencil8.
    std::array<std::tuple<wgpu::TextureFormat, wgpu::TextureAspect>, 7> params = {
        std::make_tuple(wgpu::TextureFormat::Depth16Unorm, wgpu::TextureAspect::DepthOnly),
        std::make_tuple(wgpu::TextureFormat::Depth16Unorm, wgpu::TextureAspect::All),
        std::make_tuple(wgpu::TextureFormat::Depth24PlusStencil8, wgpu::TextureAspect::StencilOnly),
        std::make_tuple(wgpu::TextureFormat::Depth32Float, wgpu::TextureAspect::DepthOnly),
        std::make_tuple(wgpu::TextureFormat::Depth32Float, wgpu::TextureAspect::All),
        std::make_tuple(wgpu::TextureFormat::Depth32FloatStencil8, wgpu::TextureAspect::DepthOnly),
        std::make_tuple(wgpu::TextureFormat::Depth32FloatStencil8,
                        wgpu::TextureAspect::StencilOnly),
    };

    uint64_t bufferSize = BufferSizeForTextureCopy(32, 32, 1);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    for (auto param : params) {
        wgpu::TextureFormat textureFormat = std::get<0>(param);
        wgpu::TextureAspect textureAspect = std::get<1>(param);

        wgpu::Texture source =
            Create2DTexture(16, 16, 5, 1, textureFormat, wgpu::TextureUsage::CopySrc);

        for (uint64_t dstOffset = 0; dstOffset < 8; dstOffset++) {
            utils::Expectation expectation =
                (dstOffset % 4 == 0) ? utils::Expectation::Success : utils::Expectation::Failure;
            TestT2BCopy(expectation, source, 0, {0, 0, 0}, destination, dstOffset, 256, 16,
                        {16, 16, 1}, textureAspect);
        }
    }
}

// Test multisampled textures cannot be used in T2B copies.
TEST_F(CopyCommandTest_T2B, CopyFromMultisampledTexture) {
    wgpu::Texture source =
        Create2DTexture(2, 2, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment, 4);
    uint64_t bufferSize = BufferSizeForTextureCopy(16, 16, 1);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 2,
                {2, 2, 1});
}

// Test T2B copies with buffer or texture in error state cause errors.
TEST_F(CopyCommandTest_T2B, BufferOrTextureInErrorState) {
    wgpu::BufferDescriptor errorBufferDescriptor;
    errorBufferDescriptor.size = 4;
    errorBufferDescriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopySrc;
    ASSERT_DEVICE_ERROR(wgpu::Buffer errorBuffer = device.CreateBuffer(&errorBufferDescriptor));

    wgpu::TextureDescriptor errorTextureDescriptor;
    errorTextureDescriptor.size.depthOrArrayLayers = 0;
    ASSERT_DEVICE_ERROR(wgpu::Texture errorTexture = device.CreateTexture(&errorTextureDescriptor));

    wgpu::TexelCopyBufferInfo errorTexelCopyBufferInfo =
        utils::CreateTexelCopyBufferInfo(errorBuffer, 0, 0, 0);
    wgpu::TexelCopyTextureInfo errorTexelCopyTextureInfo =
        utils::CreateTexelCopyTextureInfo(errorTexture, 0, {0, 0, 0});

    wgpu::Extent3D extent3D = {0, 0, 0};

    {
        uint64_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
        wgpu::Buffer source = CreateBuffer(bufferSize, wgpu::BufferUsage::CopySrc);

        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(source, 0, 0, 0);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&errorTexelCopyTextureInfo, &texelCopyBufferInfo, &extent3D);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::Texture destination = Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                                                    wgpu::TextureUsage::CopyDst);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(destination, 0, {0, 0, 0});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &errorTexelCopyBufferInfo, &extent3D);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Regression tests for a bug in the computation of texture copy buffer size in Dawn.
TEST_F(CopyCommandTest_T2B, TextureCopyBufferSizeLastRowComputation) {
    constexpr uint32_t kBytesPerRow = 256;
    constexpr uint32_t kWidth = 4;
    constexpr uint32_t kHeight = 4;

    constexpr std::array<wgpu::TextureFormat, 2> kFormats = {wgpu::TextureFormat::RGBA8Unorm,
                                                             wgpu::TextureFormat::RG8Unorm};

    {
        // kBytesPerRow * (kHeight - 1) + kWidth is not large enough to be the valid buffer size in
        // this test because the buffer sizes in T2B copies are not in texels but in bytes.
        constexpr uint32_t kInvalidBufferSize = kBytesPerRow * (kHeight - 1) + kWidth;

        for (wgpu::TextureFormat format : kFormats) {
            wgpu::Texture source =
                Create2DTexture(kWidth, kHeight, 1, 1, format, wgpu::TextureUsage::CopySrc);

            wgpu::Buffer destination = CreateBuffer(kInvalidBufferSize, wgpu::BufferUsage::CopyDst);
            TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                        kBytesPerRow, kHeight, {kWidth, kHeight, 1});
        }
    }

    {
        for (wgpu::TextureFormat format : kFormats) {
            uint32_t validBufferSize = BufferSizeForTextureCopy(kWidth, kHeight, 1, format);
            wgpu::Texture source =
                Create2DTexture(kWidth, kHeight, 1, 1, format, wgpu::TextureUsage::CopySrc);

            // Verify the return value of BufferSizeForTextureCopy() is exactly the minimum valid
            // buffer size in this test.
            {
                uint32_t invalidBufferSize = validBufferSize - 1;
                wgpu::Buffer destination =
                    CreateBuffer(invalidBufferSize, wgpu::BufferUsage::CopyDst);
                TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0,
                            kBytesPerRow, kHeight, {kWidth, kHeight, 1});
            }

            {
                wgpu::Buffer destination =
                    CreateBuffer(validBufferSize, wgpu::BufferUsage::CopyDst);
                TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                            kBytesPerRow, kHeight, {kWidth, kHeight, 1});
            }
        }
    }
}

// Test copy from mip map of non square texture to buffer
TEST_F(CopyCommandTest_T2B, CopyFromMipmapOfNonSquareTexture) {
    uint32_t maxMipmapLevel = 3;
    wgpu::Texture source = Create2DTexture(4, 2, maxMipmapLevel, 1, wgpu::TextureFormat::RGBA8Unorm,
                                           wgpu::TextureUsage::CopySrc);
    uint64_t bufferSize = BufferSizeForTextureCopy(4, 2, 1);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    // Copy from top level mip map
    TestT2BCopy(utils::Expectation::Success, source, maxMipmapLevel - 1, {0, 0, 0}, destination, 0,
                256, 1, {1, 1, 1});
    // Copy from high level mip map
    TestT2BCopy(utils::Expectation::Success, source, maxMipmapLevel - 2, {0, 0, 0}, destination, 0,
                256, 1, {2, 1, 1});
    // Mip level out of range
    TestT2BCopy(utils::Expectation::Failure, source, maxMipmapLevel, {0, 0, 0}, destination, 0, 256,
                1, {2, 1, 1});
    // Copy origin out of range
    TestT2BCopy(utils::Expectation::Failure, source, maxMipmapLevel - 2, {2, 0, 0}, destination, 0,
                256, 1, {2, 1, 1});
    // Copy size out of range
    TestT2BCopy(utils::Expectation::Failure, source, maxMipmapLevel - 2, {1, 0, 0}, destination, 0,
                256, 1, {2, 1, 1});
}

// Test copy from only the depth aspect of a texture
TEST_F(CopyCommandTest_T2B, CopyFromDepthAspect) {
    uint64_t bufferSize = BufferSizeForTextureCopy(16, 16, 1, wgpu::TextureFormat::Depth32Float);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    constexpr std::array<wgpu::TextureFormat, 3> kAllowDepthCopyFormats = {
        wgpu::TextureFormat::Depth16Unorm, wgpu::TextureFormat::Depth32Float,
        wgpu::TextureFormat::Depth32FloatStencil8};
    for (wgpu::TextureFormat format : kAllowDepthCopyFormats) {
        {
            wgpu::Texture source =
                Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopySrc);

            // Test it is valid to copy the depth aspect of these depth/stencil texture
            TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256, 16,
                        {16, 16, 1}, wgpu::TextureAspect::DepthOnly);

            if (utils::IsDepthOnlyFormat(format)) {
                // Test "all" of a depth texture which is only the depth aspect.
                TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256,
                            16, {16, 16, 1}, wgpu::TextureAspect::All);
            }
        }
    }

    constexpr std::array<wgpu::TextureFormat, 2> kDisallowDepthCopyFormats = {
        wgpu::TextureFormat::Depth24Plus,
        wgpu::TextureFormat::Depth24PlusStencil8,
    };
    for (wgpu::TextureFormat format : kDisallowDepthCopyFormats) {
        {
            wgpu::Texture source =
                Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopySrc);

            // Test it is invalid to copy from the depth aspect of these depth/stencil texture
            TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 16,
                        {16, 16, 1}, wgpu::TextureAspect::DepthOnly);
        }
    }

    {
        wgpu::Texture source = Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::R32Float,
                                               wgpu::TextureUsage::CopySrc);

        // Test it is invalid to copy from the depth aspect of a color texture
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 16,
                    {16, 16, 1}, wgpu::TextureAspect::DepthOnly);
    }
}

// Test copy from only the stencil aspect of a texture
TEST_F(CopyCommandTest_T2B, CopyFromStencilAspect) {
    uint64_t bufferSize = BufferSizeForTextureCopy(16, 16, 1, wgpu::TextureFormat::R8Uint);
    wgpu::Buffer destination = CreateBuffer(bufferSize, wgpu::BufferUsage::CopyDst);

    for (wgpu::TextureFormat format : utils::kStencilFormats) {
        {
            wgpu::Texture source =
                Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopySrc);

            // TODO(dawn:666): Test "all" of Stencil8 format when it's implemented

            // Test it is valid to copy from the stencil aspect of a depth/stencil texture
            TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, 256, 16,
                        {16, 16, 1}, wgpu::TextureAspect::StencilOnly);

            // Test it is invalid if the buffer is too small
            wgpu::Buffer destinationSmall =
                CreateBuffer(bufferSize - 1, wgpu::BufferUsage::CopyDst);
            TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destinationSmall, 0, 256,
                        16, {16, 16, 1}, wgpu::TextureAspect::StencilOnly);
        }

        // A copy fails when using a depth/stencil texture, and the entire subresource isn't
        // copied
        {
            wgpu::Texture source =
                Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopySrc);

            TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 15,
                        {15, 15, 1}, wgpu::TextureAspect::StencilOnly);

            TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 1,
                        {1, 1, 1}, wgpu::TextureAspect::StencilOnly);
        }

        // Non-zero mip: A copy fails when using a depth/stencil texture, and the entire
        // subresource isn't copied
        {
            wgpu::Texture source =
                Create2DTexture(16, 16, 2, 1, format, wgpu::TextureUsage::CopySrc);

            // Whole mip is success
            TestT2BCopy(utils::Expectation::Success, source, 1, {0, 0, 0}, destination, 0, 256, 8,
                        {8, 8, 1}, wgpu::TextureAspect::StencilOnly);

            // Partial mip fails
            TestT2BCopy(utils::Expectation::Failure, source, 1, {0, 0, 0}, destination, 0, 256, 7,
                        {7, 7, 1}, wgpu::TextureAspect::StencilOnly);

            TestT2BCopy(utils::Expectation::Failure, source, 1, {0, 0, 0}, destination, 0, 256, 1,
                        {1, 1, 1}, wgpu::TextureAspect::StencilOnly);
        }

        // Non-zero mip, non-pow-2: A copy fails when using a depth/stencil texture, and the
        // entire subresource isn't copied
        {
            wgpu::Texture source =
                Create2DTexture(17, 17, 2, 1, format, wgpu::TextureUsage::CopySrc);

            // Whole mip is success
            TestT2BCopy(utils::Expectation::Success, source, 1, {0, 0, 0}, destination, 0, 256, 8,
                        {8, 8, 1}, wgpu::TextureAspect::StencilOnly);

            // Partial mip fails
            TestT2BCopy(utils::Expectation::Failure, source, 1, {0, 0, 0}, destination, 0, 256, 7,
                        {7, 7, 1}, wgpu::TextureAspect::StencilOnly);

            TestT2BCopy(utils::Expectation::Failure, source, 1, {0, 0, 0}, destination, 0, 256, 1,
                        {1, 1, 1}, wgpu::TextureAspect::StencilOnly);
        }
    }

    {
        wgpu::Texture source =
            Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::R8Uint, wgpu::TextureUsage::CopySrc);

        // Test it is invalid to copy from the stencil aspect of a color texture
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 16,
                    {16, 16, 1}, wgpu::TextureAspect::StencilOnly);
    }
    {
        wgpu::Texture source = Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::Depth24Plus,
                                               wgpu::TextureUsage::CopySrc);

        // Test it is invalid to copy from the stencil aspect of a depth-only texture
        TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, 256, 16,
                    {16, 16, 1}, wgpu::TextureAspect::StencilOnly);
    }
}

// Test that CopyT2B throws an error when requiredBytesInCopy overflows uint64_t
TEST_F(CopyCommandTest_T2B, RequiredBytesInCopyOverflow) {
    wgpu::Buffer destination = CreateBuffer(10000, wgpu::BufferUsage::CopyDst);
    wgpu::Texture source =
        Create2DTexture(1, 1, 1, 16, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);

    // Success
    TestT2BCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, (1 << 31),
                (1 << 31), {1, 1, 1});
    // Failure because bytesPerImage * (depth - 1) overflows
    TestT2BCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, (1 << 31),
                (1 << 31), {1, 1, 16});
}

class CopyCommandTest_T2T : public CopyCommandTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::Depth32FloatStencil8};
    }

    wgpu::TextureFormat GetCopyCompatibleFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::BGRA8Unorm:
                return wgpu::TextureFormat::BGRA8UnormSrgb;
            case wgpu::TextureFormat::BGRA8UnormSrgb:
                return wgpu::TextureFormat::BGRA8Unorm;
            case wgpu::TextureFormat::RGBA8Unorm:
                return wgpu::TextureFormat::RGBA8UnormSrgb;
            case wgpu::TextureFormat::RGBA8UnormSrgb:
                return wgpu::TextureFormat::RGBA8Unorm;
            default:
                DAWN_UNREACHABLE();
        }
    }
};

TEST_F(CopyCommandTest_T2T, Success) {
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // Different copies, including some that touch the OOB condition
    {
        // Copy a region along top left boundary
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {4, 4, 1});

        // Copy entire texture
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {16, 16, 1});

        // Copy a region along bottom right boundary
        TestT2TCopy(utils::Expectation::Success, source, 0, {8, 8, 0}, destination, 0, {8, 8, 0},
                    {8, 8, 1});

        // Copy region into mip
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 2, {0, 0, 0},
                    {4, 4, 1});

        // Copy mip into region
        TestT2TCopy(utils::Expectation::Success, source, 2, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {4, 4, 1});

        // Copy between slices
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 1}, destination, 0, {0, 0, 1},
                    {16, 16, 1});

        // Copy multiple slices (srcTexelCopyTextureInfo.arrayLayer + copySize.depthOrArrayLayers ==
        // srcTexelCopyTextureInfo.texture.arrayLayerCount)
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 2}, destination, 0, {0, 0, 0},
                    {16, 16, 2});

        // Copy multiple slices (dstTexelCopyTextureInfo.arrayLayer + copySize.depthOrArrayLayers ==
        // dstTexelCopyTextureInfo.texture.arrayLayerCount)
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {0, 0, 2},
                    {16, 16, 2});
    }

    // Empty copies are valid
    {
        // An empty copy
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {0, 0, 1});

        // An empty copy with depth = 0
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {0, 0, 0});

        // An empty copy touching the side of the source texture
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {16, 16, 0},
                    {0, 0, 1});

        // An empty copy touching the side of the destination texture
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {16, 16, 0},
                    {0, 0, 1});
    }
}

TEST_F(CopyCommandTest_T2T, IncorrectUsage) {
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // Incorrect source usage causes failure
    TestT2TCopy(utils::Expectation::Failure, destination, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                {16, 16, 1});

    // Incorrect destination usage causes failure
    TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, source, 0, {0, 0, 0},
                {16, 16, 1});
}

TEST_F(CopyCommandTest_T2T, OutOfBounds) {
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 4, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // OOB on source
    {
        // x + width overflows
        TestT2TCopy(utils::Expectation::Failure, source, 0, {1, 0, 0}, destination, 0, {0, 0, 0},
                    {16, 16, 1});

        // y + height overflows
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 1, 0}, destination, 0, {0, 0, 0},
                    {16, 16, 1});

        // non-zero mip overflows
        TestT2TCopy(utils::Expectation::Failure, source, 1, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {9, 9, 1});

        // arrayLayer + depth OOB
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 3}, destination, 0, {0, 0, 0},
                    {16, 16, 2});

        // empty copy on non-existent mip fails
        TestT2TCopy(utils::Expectation::Failure, source, 6, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {0, 0, 1});

        // empty copy from non-existent slice fails
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 4}, destination, 0, {0, 0, 0},
                    {0, 0, 1});
    }

    // OOB on destination
    {
        // x + width overflows
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {1, 0, 0},
                    {16, 16, 1});

        // y + height overflows
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {0, 1, 0},
                    {16, 16, 1});

        // non-zero mip overflows
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 1, {0, 0, 0},
                    {9, 9, 1});

        // arrayLayer + depth OOB
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {0, 0, 3},
                    {16, 16, 2});

        // empty copy on non-existent mip fails
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 6, {0, 0, 0},
                    {0, 0, 1});

        // empty copy on non-existent slice fails
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {0, 0, 4},
                    {0, 0, 1});
    }
}

TEST_F(CopyCommandTest_T2T, 2DTextureDepthStencil) {
    for (wgpu::TextureFormat format : utils::kDepthAndStencilFormats) {
        wgpu::Texture source = Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopySrc);

        wgpu::Texture destination =
            Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopyDst);

        // Success when entire depth stencil subresource is copied
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {16, 16, 1});

        // Failure when depth stencil subresource is partially copied
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {15, 15, 1});

        // Failure when selecting the depth aspect (not all)
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {16, 16, 1}, wgpu::TextureAspect::DepthOnly);

        // Failure when selecting the stencil aspect (not all)
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {16, 16, 1}, wgpu::TextureAspect::StencilOnly);
    }
}

TEST_F(CopyCommandTest_T2T, 2DTextureDepthOnly) {
    constexpr std::array<wgpu::TextureFormat, 2> kDepthOnlyFormats = {
        wgpu::TextureFormat::Depth24Plus, wgpu::TextureFormat::Depth32Float};

    for (wgpu::TextureFormat format : kDepthOnlyFormats) {
        wgpu::Texture source = Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopySrc);

        wgpu::Texture destination =
            Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopyDst);

        // Success when entire subresource is copied
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {16, 16, 1});

        // Failure when depth subresource is partially copied
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {15, 15, 1});

        // Success when selecting the depth aspect (not all)
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {16, 16, 1}, wgpu::TextureAspect::DepthOnly);

        // Failure when selecting the stencil aspect (not all)
        TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {16, 16, 1}, wgpu::TextureAspect::StencilOnly);
    }
}

TEST_F(CopyCommandTest_T2T, 2DTextureArrayDepthStencil) {
    for (wgpu::TextureFormat format : utils::kDepthAndStencilFormats) {
        {
            wgpu::Texture source =
                Create2DTexture(16, 16, 1, 3, format, wgpu::TextureUsage::CopySrc);
            wgpu::Texture destination =
                Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopyDst);

            // Success when entire depth stencil subresource (layer) is the copy source
            TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 1}, destination, 0,
                        {0, 0, 0}, {16, 16, 1});
        }

        {
            wgpu::Texture source =
                Create2DTexture(16, 16, 1, 1, format, wgpu::TextureUsage::CopySrc);
            wgpu::Texture destination =
                Create2DTexture(16, 16, 1, 3, format, wgpu::TextureUsage::CopyDst);

            // Success when entire depth stencil subresource (layer) is the copy destination
            TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0,
                        {0, 0, 1}, {16, 16, 1});
        }

        {
            wgpu::Texture source =
                Create2DTexture(16, 16, 1, 3, format, wgpu::TextureUsage::CopySrc);
            wgpu::Texture destination =
                Create2DTexture(16, 16, 1, 3, format, wgpu::TextureUsage::CopyDst);

            // Success when src and dst are an entire depth stencil subresource (layer)
            TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 2}, destination, 0,
                        {0, 0, 1}, {16, 16, 1});

            // Success when src and dst are an array of entire depth stencil subresources
            TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 1}, destination, 0,
                        {0, 0, 0}, {16, 16, 2});
        }
    }
}

TEST_F(CopyCommandTest_T2T, FormatsMismatch) {
    wgpu::Texture source =
        Create2DTexture(16, 16, 5, 2, wgpu::TextureFormat::RGBA8Uint, wgpu::TextureUsage::CopySrc);
    wgpu::Texture destination =
        Create2DTexture(16, 16, 5, 2, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);

    // Failure when formats don't match
    TestT2TCopy(utils::Expectation::Failure, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                {0, 0, 1});
}

// Test copying between textures that have srgb compatible texture formats;
TEST_F(CopyCommandTest_T2T, SrgbFormatsCompatibility) {
    for (wgpu::TextureFormat srcTextureFormat :
         {wgpu::TextureFormat::BGRA8Unorm, wgpu::TextureFormat::BGRA8UnormSrgb,
          wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA8UnormSrgb}) {
        wgpu::TextureFormat dstTextureFormat = GetCopyCompatibleFormat(srcTextureFormat);
        wgpu::Texture source =
            Create2DTexture(16, 16, 5, 2, srcTextureFormat, wgpu::TextureUsage::CopySrc);
        wgpu::Texture destination =
            Create2DTexture(16, 16, 5, 2, dstTextureFormat, wgpu::TextureUsage::CopyDst);

        // Failure when formats don't match
        TestT2TCopy(utils::Expectation::Success, source, 0, {0, 0, 0}, destination, 0, {0, 0, 0},
                    {0, 0, 1});
    }
}

TEST_F(CopyCommandTest_T2T, MultisampledCopies) {
    wgpu::Texture sourceMultiSampled1x = Create2DTexture(
        16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopySrc, 1);
    wgpu::Texture sourceMultiSampled4x =
        Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::RenderAttachment, 4);
    wgpu::Texture destinationMultiSampled4x =
        Create2DTexture(16, 16, 1, 1, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::RenderAttachment, 4);

    // Success when entire multisampled subresource is copied
    {
        TestT2TCopy(utils::Expectation::Success, sourceMultiSampled4x, 0, {0, 0, 0},
                    destinationMultiSampled4x, 0, {0, 0, 0}, {16, 16, 1});
    }

    // Failures
    {
        // An empty copy with mismatched samples fails
        TestT2TCopy(utils::Expectation::Failure, sourceMultiSampled1x, 0, {0, 0, 0},
                    destinationMultiSampled4x, 0, {0, 0, 0}, {0, 0, 1});

        // A copy fails when samples are greater than 1, and entire subresource isn't copied
        TestT2TCopy(utils::Expectation::Failure, sourceMultiSampled4x, 0, {0, 0, 0},
                    destinationMultiSampled4x, 0, {0, 0, 0}, {15, 15, 1});
    }
}

// Test copy to mip map of non square textures
TEST_F(CopyCommandTest_T2T, CopyToMipmapOfNonSquareTexture) {
    uint32_t maxMipmapLevel = 3;
    wgpu::Texture source = Create2DTexture(4, 2, maxMipmapLevel, 1, wgpu::TextureFormat::RGBA8Unorm,
                                           wgpu::TextureUsage::CopySrc);
    wgpu::Texture destination = Create2DTexture(
        4, 2, maxMipmapLevel, 1, wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureUsage::CopyDst);
    // Copy to top level mip map
    TestT2TCopy(utils::Expectation::Success, source, maxMipmapLevel - 1, {0, 0, 0}, destination,
                maxMipmapLevel - 1, {0, 0, 0}, {1, 1, 1});
    // Copy to high level mip map
    TestT2TCopy(utils::Expectation::Success, source, maxMipmapLevel - 2, {0, 0, 0}, destination,
                maxMipmapLevel - 2, {0, 0, 0}, {2, 1, 1});
    // Mip level out of range
    TestT2TCopy(utils::Expectation::Failure, source, maxMipmapLevel, {0, 0, 0}, destination,
                maxMipmapLevel, {0, 0, 0}, {2, 1, 1});
    // Copy origin out of range
    TestT2TCopy(utils::Expectation::Failure, source, maxMipmapLevel - 2, {2, 0, 0}, destination,
                maxMipmapLevel - 2, {2, 0, 0}, {2, 1, 1});
    // Copy size out of range
    TestT2TCopy(utils::Expectation::Failure, source, maxMipmapLevel - 2, {1, 0, 0}, destination,
                maxMipmapLevel - 2, {0, 0, 0}, {2, 1, 1});
}

// Test copy within the same texture
TEST_F(CopyCommandTest_T2T, CopyWithinSameTexture) {
    wgpu::Texture texture =
        Create2DTexture(32, 32, 2, 4, wgpu::TextureFormat::RGBA8Unorm,
                        wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst);

    // The base array layer of the copy source being equal to that of the copy destination is not
    // allowed.
    {
        constexpr uint32_t kBaseArrayLayer = 0;

        // copyExtent.z == 1
        {
            constexpr uint32_t kCopyArrayLayerCount = 1;
            TestT2TCopy(utils::Expectation::Failure, texture, 0, {0, 0, kBaseArrayLayer}, texture,
                        0, {2, 2, kBaseArrayLayer}, {1, 1, kCopyArrayLayerCount});
        }

        // copyExtent.z > 1
        {
            constexpr uint32_t kCopyArrayLayerCount = 2;
            TestT2TCopy(utils::Expectation::Failure, texture, 0, {0, 0, kBaseArrayLayer}, texture,
                        0, {2, 2, kBaseArrayLayer}, {1, 1, kCopyArrayLayerCount});
        }
    }

    // The array slices of the source involved in the copy have no overlap with those of the
    // destination is allowed.
    {
        constexpr uint32_t kCopyArrayLayerCount = 2;

        // srcBaseArrayLayer < dstBaseArrayLayer
        {
            constexpr uint32_t kSrcBaseArrayLayer = 0;
            constexpr uint32_t kDstBaseArrayLayer = kSrcBaseArrayLayer + kCopyArrayLayerCount;

            TestT2TCopy(utils::Expectation::Success, texture, 0, {0, 0, kSrcBaseArrayLayer},
                        texture, 0, {0, 0, kDstBaseArrayLayer}, {1, 1, kCopyArrayLayerCount});
        }

        // srcBaseArrayLayer > dstBaseArrayLayer
        {
            constexpr uint32_t kSrcBaseArrayLayer = 2;
            constexpr uint32_t kDstBaseArrayLayer = kSrcBaseArrayLayer - kCopyArrayLayerCount;
            TestT2TCopy(utils::Expectation::Success, texture, 0, {0, 0, kSrcBaseArrayLayer},
                        texture, 0, {0, 0, kDstBaseArrayLayer}, {1, 1, kCopyArrayLayerCount});
        }
    }

    // Copy between different mipmap levels is allowed.
    {
        constexpr uint32_t kSrcMipLevel = 0;
        constexpr uint32_t kDstMipLevel = 1;

        // Copy one slice
        {
            constexpr uint32_t kCopyArrayLayerCount = 1;
            TestT2TCopy(utils::Expectation::Success, texture, kSrcMipLevel, {0, 0, 0}, texture,
                        kDstMipLevel, {1, 1, 0}, {1, 1, kCopyArrayLayerCount});
        }

        // The base array layer of the copy source is equal to that of the copy destination.
        {
            constexpr uint32_t kCopyArrayLayerCount = 2;
            constexpr uint32_t kBaseArrayLayer = 0;

            TestT2TCopy(utils::Expectation::Success, texture, kSrcMipLevel, {0, 0, kBaseArrayLayer},
                        texture, kDstMipLevel, {1, 1, kBaseArrayLayer},
                        {1, 1, kCopyArrayLayerCount});
        }

        // The array slices of the source involved in the copy have overlaps with those of the
        // destination, and the copy areas have overlaps.
        {
            constexpr uint32_t kCopyArrayLayerCount = 2;

            constexpr uint32_t kSrcBaseArrayLayer = 0;
            constexpr uint32_t kDstBaseArrayLayer = 1;
            DAWN_ASSERT(kSrcBaseArrayLayer + kCopyArrayLayerCount > kDstBaseArrayLayer);

            constexpr wgpu::Extent3D kCopyExtent = {1, 1, kCopyArrayLayerCount};

            TestT2TCopy(utils::Expectation::Success, texture, kSrcMipLevel,
                        {0, 0, kSrcBaseArrayLayer}, texture, kDstMipLevel,
                        {0, 0, kDstBaseArrayLayer}, kCopyExtent);
        }
    }

    // The array slices of the source involved in the copy have overlaps with those of the
    // destination is not allowed.
    {
        constexpr uint32_t kMipmapLevel = 0;
        constexpr uint32_t kMinBaseArrayLayer = 0;
        constexpr uint32_t kMaxBaseArrayLayer = 1;
        constexpr uint32_t kCopyArrayLayerCount = 3;
        DAWN_ASSERT(kMinBaseArrayLayer + kCopyArrayLayerCount > kMaxBaseArrayLayer);

        constexpr wgpu::Extent3D kCopyExtent = {4, 4, kCopyArrayLayerCount};

        const wgpu::Origin3D srcOrigin = {0, 0, kMinBaseArrayLayer};
        const wgpu::Origin3D dstOrigin = {4, 4, kMaxBaseArrayLayer};
        TestT2TCopy(utils::Expectation::Failure, texture, kMipmapLevel, srcOrigin, texture,
                    kMipmapLevel, dstOrigin, kCopyExtent);
    }

    // Copy between different mipmap levels and array slices is allowed.
    TestT2TCopy(utils::Expectation::Success, texture, 0, {0, 0, 1}, texture, 1, {1, 1, 0},
                {1, 1, 1});

    // Copy between 3D texture of both overlapping depth ranges is not allowed.
    {
        wgpu::Texture texture3D =
            Create3DTexture(32, 32, 4, 2, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst);

        constexpr uint32_t kMipmapLevel = 0;
        constexpr wgpu::Origin3D kSrcOrigin = {0, 0, 0};
        constexpr wgpu::Origin3D kDstOrigin = {0, 0, 1};
        constexpr wgpu::Extent3D kCopyExtent = {4, 4, 2};

        TestT2TCopy(utils::Expectation::Failure, texture3D, kMipmapLevel, kSrcOrigin, texture3D,
                    kMipmapLevel, kDstOrigin, kCopyExtent);
    }

    // Copy between 3D texture of both non-overlapping depth ranges is not allowed.
    {
        wgpu::Texture texture3D =
            Create3DTexture(32, 32, 4, 2, wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst);

        constexpr uint32_t kMipmapLevel = 0;
        constexpr wgpu::Origin3D kSrcOrigin = {0, 0, 0};
        constexpr wgpu::Origin3D kDstOrigin = {0, 0, 2};
        constexpr wgpu::Extent3D kCopyExtent = {4, 4, 1};

        TestT2TCopy(utils::Expectation::Failure, texture3D, kMipmapLevel, kSrcOrigin, texture3D,
                    kMipmapLevel, kDstOrigin, kCopyExtent);
    }
}

class CopyCommandTest_CompressedTextureFormats : public CopyCommandTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::TextureCompressionBC, wgpu::FeatureName::TextureCompressionETC2,
                wgpu::FeatureName::TextureCompressionASTC};
    }

    wgpu::Texture Create2DTexture(wgpu::TextureFormat format,
                                  uint32_t mipmapLevels,
                                  uint32_t width,
                                  uint32_t height) {
        constexpr wgpu::TextureUsage kUsage = wgpu::TextureUsage::CopyDst |
                                              wgpu::TextureUsage::CopySrc |
                                              wgpu::TextureUsage::TextureBinding;
        constexpr uint32_t kArrayLayers = 1;
        return CopyCommandTest::Create2DTexture(width, height, mipmapLevels, kArrayLayers, format,
                                                kUsage, 1);
    }

    // By default, we use a 4x4 tiling of the format block size.
    wgpu::Texture Create2DTexture(wgpu::TextureFormat format) {
        uint32_t width = utils::GetTextureFormatBlockWidth(format) * 4;
        uint32_t height = utils::GetTextureFormatBlockHeight(format) * 4;
        return Create2DTexture(format, 1, width, height);
    }

    wgpu::TextureFormat GetCopyCompatibleFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::BC1RGBAUnorm:
                return wgpu::TextureFormat::BC1RGBAUnormSrgb;
            case wgpu::TextureFormat::BC1RGBAUnormSrgb:
                return wgpu::TextureFormat::BC1RGBAUnorm;
            case wgpu::TextureFormat::BC2RGBAUnorm:
                return wgpu::TextureFormat::BC2RGBAUnormSrgb;
            case wgpu::TextureFormat::BC2RGBAUnormSrgb:
                return wgpu::TextureFormat::BC2RGBAUnorm;
            case wgpu::TextureFormat::BC3RGBAUnorm:
                return wgpu::TextureFormat::BC3RGBAUnormSrgb;
            case wgpu::TextureFormat::BC3RGBAUnormSrgb:
                return wgpu::TextureFormat::BC3RGBAUnorm;
            case wgpu::TextureFormat::BC7RGBAUnorm:
                return wgpu::TextureFormat::BC7RGBAUnormSrgb;
            case wgpu::TextureFormat::BC7RGBAUnormSrgb:
                return wgpu::TextureFormat::BC7RGBAUnorm;
            case wgpu::TextureFormat::ETC2RGB8Unorm:
                return wgpu::TextureFormat::ETC2RGB8UnormSrgb;
            case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
                return wgpu::TextureFormat::ETC2RGB8Unorm;
            case wgpu::TextureFormat::ETC2RGB8A1Unorm:
                return wgpu::TextureFormat::ETC2RGB8A1UnormSrgb;
            case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
                return wgpu::TextureFormat::ETC2RGB8A1Unorm;
            case wgpu::TextureFormat::ETC2RGBA8Unorm:
                return wgpu::TextureFormat::ETC2RGBA8UnormSrgb;
            case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
                return wgpu::TextureFormat::ETC2RGBA8Unorm;
            case wgpu::TextureFormat::ASTC4x4Unorm:
                return wgpu::TextureFormat::ASTC4x4UnormSrgb;
            case wgpu::TextureFormat::ASTC4x4UnormSrgb:
                return wgpu::TextureFormat::ASTC4x4Unorm;
            case wgpu::TextureFormat::ASTC5x4Unorm:
                return wgpu::TextureFormat::ASTC5x4UnormSrgb;
            case wgpu::TextureFormat::ASTC5x4UnormSrgb:
                return wgpu::TextureFormat::ASTC5x4Unorm;
            case wgpu::TextureFormat::ASTC5x5Unorm:
                return wgpu::TextureFormat::ASTC5x5UnormSrgb;
            case wgpu::TextureFormat::ASTC5x5UnormSrgb:
                return wgpu::TextureFormat::ASTC5x5Unorm;
            case wgpu::TextureFormat::ASTC6x5Unorm:
                return wgpu::TextureFormat::ASTC6x5UnormSrgb;
            case wgpu::TextureFormat::ASTC6x5UnormSrgb:
                return wgpu::TextureFormat::ASTC6x5Unorm;
            case wgpu::TextureFormat::ASTC6x6Unorm:
                return wgpu::TextureFormat::ASTC6x6UnormSrgb;
            case wgpu::TextureFormat::ASTC6x6UnormSrgb:
                return wgpu::TextureFormat::ASTC6x6Unorm;
            case wgpu::TextureFormat::ASTC8x5Unorm:
                return wgpu::TextureFormat::ASTC8x5UnormSrgb;
            case wgpu::TextureFormat::ASTC8x5UnormSrgb:
                return wgpu::TextureFormat::ASTC8x5Unorm;
            case wgpu::TextureFormat::ASTC8x6Unorm:
                return wgpu::TextureFormat::ASTC8x6UnormSrgb;
            case wgpu::TextureFormat::ASTC8x6UnormSrgb:
                return wgpu::TextureFormat::ASTC8x6Unorm;
            case wgpu::TextureFormat::ASTC8x8Unorm:
                return wgpu::TextureFormat::ASTC8x8UnormSrgb;
            case wgpu::TextureFormat::ASTC8x8UnormSrgb:
                return wgpu::TextureFormat::ASTC8x8Unorm;
            case wgpu::TextureFormat::ASTC10x5Unorm:
                return wgpu::TextureFormat::ASTC10x5UnormSrgb;
            case wgpu::TextureFormat::ASTC10x5UnormSrgb:
                return wgpu::TextureFormat::ASTC10x5Unorm;
            case wgpu::TextureFormat::ASTC10x6Unorm:
                return wgpu::TextureFormat::ASTC10x6UnormSrgb;
            case wgpu::TextureFormat::ASTC10x6UnormSrgb:
                return wgpu::TextureFormat::ASTC10x6Unorm;
            case wgpu::TextureFormat::ASTC10x8Unorm:
                return wgpu::TextureFormat::ASTC10x8UnormSrgb;
            case wgpu::TextureFormat::ASTC10x8UnormSrgb:
                return wgpu::TextureFormat::ASTC10x8Unorm;
            case wgpu::TextureFormat::ASTC10x10Unorm:
                return wgpu::TextureFormat::ASTC10x10UnormSrgb;
            case wgpu::TextureFormat::ASTC10x10UnormSrgb:
                return wgpu::TextureFormat::ASTC10x10Unorm;
            case wgpu::TextureFormat::ASTC12x10Unorm:
                return wgpu::TextureFormat::ASTC12x10UnormSrgb;
            case wgpu::TextureFormat::ASTC12x10UnormSrgb:
                return wgpu::TextureFormat::ASTC12x10Unorm;
            case wgpu::TextureFormat::ASTC12x12Unorm:
                return wgpu::TextureFormat::ASTC12x12UnormSrgb;
            case wgpu::TextureFormat::ASTC12x12UnormSrgb:
                return wgpu::TextureFormat::ASTC12x12Unorm;
            default:
                DAWN_UNREACHABLE();
        }
    }
};

// Tests to verify that bufferOffset must be a multiple of the compressed texture blocks in bytes
// in buffer-to-texture or texture-to-buffer copies with compressed texture formats.
TEST_F(CopyCommandTest_CompressedTextureFormats, BufferOffset) {
    wgpu::Buffer buffer =
        CreateBuffer(512, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        wgpu::Texture texture = Create2DTexture(format);
        uint32_t blockWidth = utils::GetTextureFormatBlockWidth(format);
        uint32_t blockHeight = utils::GetTextureFormatBlockHeight(format);

        // Valid usages of BufferOffset in B2T and T2B copies with compressed texture formats.
        {
            uint32_t validBufferOffset = utils::GetTexelBlockSizeInBytes(format);
            TestBothTBCopies(utils::Expectation::Success, buffer, validBufferOffset, 256, 4,
                             texture, 0, {0, 0, 0}, {blockWidth, blockHeight, 1});
        }

        // Failures on invalid bufferOffset.
        {
            uint32_t kInvalidBufferOffset = utils::GetTexelBlockSizeInBytes(format) / 2;
            TestBothTBCopies(utils::Expectation::Failure, buffer, kInvalidBufferOffset, 256, 4,
                             texture, 0, {0, 0, 0}, {blockWidth, blockHeight, 1});
        }
    }
}

// Tests to verify that bytesPerRow must not be less than (width / blockWidth) * blockSizeInBytes.
// Note that in Dawn we require bytesPerRow be a multiple of 256, which ensures bytesPerRow will
// always be the multiple of compressed texture block width in bytes.
TEST_F(CopyCommandTest_CompressedTextureFormats, BytesPerRow) {
    wgpu::Buffer buffer =
        CreateBuffer(1024, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    // Used to compute test width and height. We choose 320 because it isn't divisible by 256 and
    // hence will need to be aligned.
    constexpr uint32_t kInvalidBytesPerRow = 320;

    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        // Compute the test width and height such that the smallest BytesPerRow is always equal to
        // 320. We choose 320 because it isn't divisible by 256 and hence needs to be aligned.
        uint32_t blockWidth = utils::GetTextureFormatBlockWidth(format);
        uint32_t blockHeight = utils::GetTextureFormatBlockHeight(format);
        uint32_t blockByteSize = utils::GetTexelBlockSizeInBytes(format);
        uint32_t testWidth = kInvalidBytesPerRow * blockWidth / blockByteSize;
        uint32_t testHeight = kInvalidBytesPerRow * blockHeight / blockByteSize;
        wgpu::Texture texture = Create2DTexture(format, 1, testWidth, testHeight);

        // Failures on the BytesPerRow that is not large enough.
        {
            constexpr uint32_t kSmallBytesPerRow = 256;
            TestBothTBCopies(utils::Expectation::Failure, buffer, 0, kSmallBytesPerRow, 4, texture,
                             0, {0, 0, 0}, {testWidth, blockHeight, 1});
        }

        // Test it is not valid to use a BytesPerRow that is not a multiple of 256.
        {
            TestBothTBCopies(utils::Expectation::Failure, buffer, 0, kInvalidBytesPerRow, 4,
                             texture, 0, {0, 0, 0}, {testWidth, blockHeight, 1});
        }

        // Test the smallest valid BytesPerRow should work.
        {
            uint32_t smallestValidBytesPerRow = Align(kInvalidBytesPerRow, 256);
            TestBothTBCopies(utils::Expectation::Success, buffer, 0, smallestValidBytesPerRow, 4,
                             texture, 0, {0, 0, 0}, {testWidth, blockHeight, 1});
        }
    }
}

// rowsPerImage must be >= heightInBlocks.
TEST_F(CopyCommandTest_CompressedTextureFormats, RowsPerImage) {
    wgpu::Buffer buffer =
        CreateBuffer(1024, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        wgpu::Texture texture = Create2DTexture(format);
        uint32_t blockWidth = utils::GetTextureFormatBlockWidth(format);
        uint32_t blockHeight = utils::GetTextureFormatBlockHeight(format);

        // Valid usages of rowsPerImage in B2T and T2B copies with compressed texture formats.
        {
            constexpr uint32_t kValidRowsPerImage = 5;
            TestBothTBCopies(utils::Expectation::Success, buffer, 0, 256, kValidRowsPerImage,
                             texture, 0, {0, 0, 0}, {blockWidth, blockHeight * 4, 1});
        }
        {
            constexpr uint32_t kValidRowsPerImage = 4;
            TestBothTBCopies(utils::Expectation::Success, buffer, 0, 256, kValidRowsPerImage,
                             texture, 0, {0, 0, 0}, {blockWidth, blockHeight * 4, 1});
        }

        // rowsPerImage is smaller than height.
        {
            constexpr uint32_t kInvalidRowsPerImage = 3;
            TestBothTBCopies(utils::Expectation::Failure, buffer, 0, 256, kInvalidRowsPerImage,
                             texture, 0, {0, 0, 0}, {blockWidth, blockHeight * 5, 1});
        }
    }
}

// Tests to verify that ImageOffset.x must be a multiple of the compressed texture block width and
// ImageOffset.y must be a multiple of the compressed texture block height in buffer-to-texture,
// texture-to-buffer or texture-to-texture copies with compressed texture formats.
TEST_F(CopyCommandTest_CompressedTextureFormats, ImageOffset) {
    wgpu::Buffer buffer =
        CreateBuffer(512, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        wgpu::Texture texture = Create2DTexture(format);
        wgpu::Texture texture2 = Create2DTexture(format);
        uint32_t blockWidth = utils::GetTextureFormatBlockWidth(format);
        uint32_t blockHeight = utils::GetTextureFormatBlockHeight(format);

        wgpu::Origin3D smallestValidOrigin3D = {blockWidth, blockHeight, 0};

        // Valid usages of ImageOffset in B2T, T2B and T2T copies with compressed texture formats.
        {
            TestBothTBCopies(utils::Expectation::Success, buffer, 0, 256, 4, texture, 0,
                             smallestValidOrigin3D, {blockWidth, blockHeight, 1});
            TestBothT2TCopies(utils::Expectation::Success, texture, 0, {0, 0, 0}, texture2, 0,
                              smallestValidOrigin3D, {blockWidth, blockHeight, 1});
        }

        // Failures on invalid ImageOffset.x.
        {
            wgpu::Origin3D invalidOrigin3D = {smallestValidOrigin3D.x - 1, smallestValidOrigin3D.y,
                                              0};
            TestBothTBCopies(utils::Expectation::Failure, buffer, 0, 256, 4, texture, 0,
                             invalidOrigin3D, {blockWidth, blockHeight, 1});
            TestBothT2TCopies(utils::Expectation::Failure, texture, 0, invalidOrigin3D, texture2, 0,
                              {0, 0, 0}, {blockWidth, blockHeight, 1});
        }

        // Failures on invalid ImageOffset.y.
        {
            wgpu::Origin3D invalidOrigin3D = {smallestValidOrigin3D.x, smallestValidOrigin3D.y - 1,
                                              0};
            TestBothTBCopies(utils::Expectation::Failure, buffer, 0, 256, 4, texture, 0,
                             invalidOrigin3D, {blockWidth, blockHeight, 1});
            TestBothT2TCopies(utils::Expectation::Failure, texture, 0, invalidOrigin3D, texture2, 0,
                              {0, 0, 0}, {blockWidth, blockHeight, 1});
        }
    }
}

// Tests to verify that ImageExtent.x must be a multiple of the compressed texture block width and
// ImageExtent.y must be a multiple of the compressed texture block height in buffer-to-texture,
// texture-to-buffer or texture-to-texture copies with compressed texture formats.
TEST_F(CopyCommandTest_CompressedTextureFormats, ImageExtent) {
    wgpu::Buffer buffer =
        CreateBuffer(1024, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    constexpr uint32_t kMipmapLevels = 3;
    // We choose a prime that is greater than the current max texel dimension size as a multiplier
    // to compute the test texture size so that we can be certain that its level 2 mipmap (x4)
    // cannot be a multiple of the dimension. This is useful for testing padding at the edges of
    // the mipmaps.
    constexpr uint32_t kBlockPerDim = 13;

    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        uint32_t blockWidth = utils::GetTextureFormatBlockWidth(format);
        uint32_t blockHeight = utils::GetTextureFormatBlockHeight(format);
        uint32_t testWidth = blockWidth * kBlockPerDim;
        uint32_t testHeight = blockHeight * kBlockPerDim;
        wgpu::Texture texture = Create2DTexture(format, kMipmapLevels, testWidth, testHeight);
        wgpu::Texture texture2 = Create2DTexture(format, kMipmapLevels, testWidth, testHeight);

        wgpu::Extent3D smallestValidExtent3D = {blockWidth, blockHeight, 1};

        // Valid usages of ImageExtent in B2T, T2B and T2T copies with compressed texture formats.
        {
            TestBothTBCopies(utils::Expectation::Success, buffer, 0, 256, 4, texture, 0, {0, 0, 0},
                             smallestValidExtent3D);
            TestBothT2TCopies(utils::Expectation::Success, texture, 0, {0, 0, 0}, texture2, 0,
                              {0, 0, 0}, smallestValidExtent3D);
        }

        // Valid usages of ImageExtent in B2T, T2B and T2T copies with compressed texture formats
        // and non-zero mipmap levels.
        {
            constexpr uint32_t kTestMipmapLevel = 2;
            wgpu::Origin3D testOrigin = {
                ((testWidth >> kTestMipmapLevel) / blockWidth) * blockWidth,
                ((testHeight >> kTestMipmapLevel) / blockHeight) * blockHeight, 0};
            TestBothTBCopies(utils::Expectation::Success, buffer, 0, 256, 4, texture,
                             kTestMipmapLevel, testOrigin, smallestValidExtent3D);
            TestBothT2TCopies(utils::Expectation::Success, texture, kTestMipmapLevel, testOrigin,
                              texture2, 0, {0, 0, 0}, smallestValidExtent3D);
        }

        // Failures on invalid ImageExtent.x.
        {
            wgpu::Extent3D inValidExtent3D = {smallestValidExtent3D.width - 1,
                                              smallestValidExtent3D.height, 1};
            TestBothTBCopies(utils::Expectation::Failure, buffer, 0, 256, 4, texture, 0, {0, 0, 0},
                             inValidExtent3D);
            TestBothT2TCopies(utils::Expectation::Failure, texture, 0, {0, 0, 0}, texture2, 0,
                              {0, 0, 0}, inValidExtent3D);
        }

        // Failures on invalid ImageExtent.y.
        {
            wgpu::Extent3D inValidExtent3D = {smallestValidExtent3D.width,
                                              smallestValidExtent3D.height - 1, 1};
            TestBothTBCopies(utils::Expectation::Failure, buffer, 0, 256, 4, texture, 0, {0, 0, 0},
                             inValidExtent3D);
            TestBothT2TCopies(utils::Expectation::Failure, texture, 0, {0, 0, 0}, texture2, 0,
                              {0, 0, 0}, inValidExtent3D);
        }
    }
}

// Test copies between buffer and multiple array layers of a compressed texture
TEST_F(CopyCommandTest_CompressedTextureFormats, CopyToMultipleArrayLayers) {
    constexpr uint32_t kWidthMultiplier = 3;
    constexpr uint32_t kHeightMultiplier = 4;
    for (wgpu::TextureFormat format : utils::kCompressedFormats) {
        uint32_t blockWidth = utils::GetTextureFormatBlockWidth(format);
        uint32_t blockHeight = utils::GetTextureFormatBlockHeight(format);
        uint32_t testWidth = kWidthMultiplier * blockWidth;
        uint32_t testHeight = kHeightMultiplier * blockHeight;
        wgpu::Texture texture = CopyCommandTest::Create2DTexture(
            testWidth, testHeight, 1, 20, format,
            wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc);

        // Copy to all array layers
        TestBothTBCopiesExactBufferSize(256, 4, texture, format, {0, 0, 0},
                                        {testWidth, testHeight, 20});

        // Copy to the highest array layer
        TestBothTBCopiesExactBufferSize(256, 4, texture, format, {0, 0, 19},
                                        {testWidth, testHeight, 1});

        // Copy to array layers in the middle
        TestBothTBCopiesExactBufferSize(256, 4, texture, format, {0, 0, 1},
                                        {testWidth, testHeight, 18});

        // Copy touching the texture corners with a non-packed rowsPerImage
        TestBothTBCopiesExactBufferSize(256, 6, texture, format, {blockWidth, blockHeight, 4},
                                        {testWidth - blockWidth, testHeight - blockHeight, 16});
    }
}

// Test copying between textures that have srgb compatible texture formats;
TEST_F(CopyCommandTest_CompressedTextureFormats, SrgbFormatCompatibility) {
    constexpr std::array<wgpu::TextureFormat, 42> srcFormats = {
        wgpu::TextureFormat::BC1RGBAUnorm,    wgpu::TextureFormat::BC1RGBAUnormSrgb,
        wgpu::TextureFormat::BC2RGBAUnorm,    wgpu::TextureFormat::BC2RGBAUnormSrgb,
        wgpu::TextureFormat::BC3RGBAUnorm,    wgpu::TextureFormat::BC3RGBAUnormSrgb,
        wgpu::TextureFormat::BC7RGBAUnorm,    wgpu::TextureFormat::BC7RGBAUnormSrgb,
        wgpu::TextureFormat::ETC2RGB8Unorm,   wgpu::TextureFormat::ETC2RGB8UnormSrgb,
        wgpu::TextureFormat::ETC2RGB8A1Unorm, wgpu::TextureFormat::ETC2RGB8A1UnormSrgb,
        wgpu::TextureFormat::ETC2RGBA8Unorm,  wgpu::TextureFormat::ETC2RGBA8UnormSrgb,
        wgpu::TextureFormat::ASTC4x4Unorm,    wgpu::TextureFormat::ASTC4x4UnormSrgb,
        wgpu::TextureFormat::ASTC5x4Unorm,    wgpu::TextureFormat::ASTC5x4UnormSrgb,
        wgpu::TextureFormat::ASTC5x5Unorm,    wgpu::TextureFormat::ASTC5x5UnormSrgb,
        wgpu::TextureFormat::ASTC6x5Unorm,    wgpu::TextureFormat::ASTC6x5UnormSrgb,
        wgpu::TextureFormat::ASTC6x6Unorm,    wgpu::TextureFormat::ASTC6x6UnormSrgb,
        wgpu::TextureFormat::ASTC8x5Unorm,    wgpu::TextureFormat::ASTC8x5UnormSrgb,
        wgpu::TextureFormat::ASTC8x6Unorm,    wgpu::TextureFormat::ASTC8x6UnormSrgb,
        wgpu::TextureFormat::ASTC8x8Unorm,    wgpu::TextureFormat::ASTC8x8UnormSrgb,
        wgpu::TextureFormat::ASTC10x5Unorm,   wgpu::TextureFormat::ASTC10x5UnormSrgb,
        wgpu::TextureFormat::ASTC10x6Unorm,   wgpu::TextureFormat::ASTC10x6UnormSrgb,
        wgpu::TextureFormat::ASTC10x8Unorm,   wgpu::TextureFormat::ASTC10x8UnormSrgb,
        wgpu::TextureFormat::ASTC10x10Unorm,  wgpu::TextureFormat::ASTC10x10UnormSrgb,
        wgpu::TextureFormat::ASTC12x10Unorm,  wgpu::TextureFormat::ASTC12x10UnormSrgb,
        wgpu::TextureFormat::ASTC12x12Unorm,  wgpu::TextureFormat::ASTC12x12UnormSrgb};

    constexpr uint32_t kBlockPerDim = 2;
    constexpr uint32_t kMipmapLevels = 1;
    for (wgpu::TextureFormat srcFormat : srcFormats) {
        uint32_t blockWidth = utils::GetTextureFormatBlockWidth(srcFormat);
        uint32_t blockHeight = utils::GetTextureFormatBlockHeight(srcFormat);
        uint32_t testWidth = blockWidth * kBlockPerDim;
        uint32_t testHeight = blockHeight * kBlockPerDim;
        wgpu::Texture texture = Create2DTexture(srcFormat, kMipmapLevels, testWidth, testHeight);
        wgpu::Texture texture2 = Create2DTexture(GetCopyCompatibleFormat(srcFormat), kMipmapLevels,
                                                 testWidth, testHeight);
        wgpu::Extent3D extent3D = {testWidth, testHeight, 1};

        TestBothT2TCopies(utils::Expectation::Success, texture, 0, {0, 0, 0}, texture2, 0,
                          {0, 0, 0}, extent3D);
    }
}

class CopyCommandTest_ClearBuffer : public CopyCommandTest {};

TEST_F(CopyCommandTest_ClearBuffer, Success) {
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    // Clear different ranges, including some that touch the OOB condition
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ClearBuffer(destination, 0, 16);
        encoder.ClearBuffer(destination, 0, 8);
        encoder.ClearBuffer(destination, 8, 8);
        encoder.Finish();
    }

    // Size is allowed to be omitted
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ClearBuffer(destination, 0);
        encoder.ClearBuffer(destination, 8);
        encoder.Finish();
    }

    // Size and Offset are allowed to be omitted
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ClearBuffer(destination);
        encoder.Finish();
    }
}

// Test a successful ClearBuffer where the last external reference is dropped.
TEST_F(CopyCommandTest_ClearBuffer, DroppedBuffer) {
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.ClearBuffer(destination, 0, 8);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();

    destination = nullptr;
    device.GetQueue().Submit(1, &commandBuffer);
}

// Test ClearBuffer copies with OOB
TEST_F(CopyCommandTest_ClearBuffer, OutOfBounds) {
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ClearBuffer(destination, 8, 12);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        // Despite being zero length, should still raise an error due to being out of bounds.
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.ClearBuffer(destination, 20, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test ClearBuffer with incorrect buffer usage
TEST_F(CopyCommandTest_ClearBuffer, BadUsage) {
    wgpu::Buffer vertex = CreateBuffer(16, wgpu::BufferUsage::Vertex);

    // Destination with incorrect usage
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.ClearBuffer(vertex, 0, 16);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test ClearBuffer with unaligned data size
TEST_F(CopyCommandTest_ClearBuffer, UnalignedSize) {
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.ClearBuffer(destination, 0, 2);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test ClearBuffer with unaligned offset
TEST_F(CopyCommandTest_ClearBuffer, UnalignedOffset) {
    wgpu::Buffer destination = CreateBuffer(16, wgpu::BufferUsage::CopyDst);

    // Unaligned destination offset
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.ClearBuffer(destination, 2, 4);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test ClearBuffer with buffers in error state cause errors.
TEST_F(CopyCommandTest_ClearBuffer, BuffersInErrorState) {
    wgpu::BufferDescriptor errorBufferDescriptor;
    errorBufferDescriptor.size = 4;
    errorBufferDescriptor.usage =
        wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    ASSERT_DEVICE_ERROR(wgpu::Buffer errorBuffer = device.CreateBuffer(&errorBufferDescriptor));

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.ClearBuffer(errorBuffer, 0, 4);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

}  // anonymous namespace
}  // namespace dawn
