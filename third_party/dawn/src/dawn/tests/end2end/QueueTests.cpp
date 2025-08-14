// Copyright 2020 The Dawn & Tint Authors
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

#include <vector>

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class QueueTests : public DawnTest {};

// Test that GetQueue always returns the same object.
TEST_P(QueueTests, GetQueueSameObject) {
    wgpu::Queue q1 = device.GetQueue();
    wgpu::Queue q2 = device.GetQueue();
    EXPECT_EQ(q1.Get(), q2.Get());
}

DAWN_INSTANTIATE_TEST(QueueTests,
                      D3D11Backend(),
                      D3D11Backend({"d3d11_delay_flush_to_gpu"}),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

class QueueWriteBufferTests : public DawnTest {};

// Test the simplest WriteBuffer setting one u32 at offset 0.
TEST_P(QueueWriteBufferTests, SmallDataAtZero) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t value = 0x01020304;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);
}

// Test an empty WriteBuffer
TEST_P(QueueWriteBufferTests, ZeroSized) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t initialValue = 0x42;
    queue.WriteBuffer(buffer, 0, &initialValue, sizeof(initialValue));

    queue.WriteBuffer(buffer, 0, nullptr, 0);

    // The content of the buffer isn't changed
    EXPECT_BUFFER_U32_EQ(initialValue, buffer, 0);
}

// Call WriteBuffer at offset 0 via a u32 twice. Test that data is updated accoordingly.
TEST_P(QueueWriteBufferTests, SetTwice) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t value = 0x01020304;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);

    value = 0x05060708;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);
}

// Test that WriteBuffer offset works.
TEST_P(QueueWriteBufferTests, SmallDataAtOffset) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4000;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    constexpr uint64_t kOffset = 2000;
    uint32_t value = 0x01020304;
    queue.WriteBuffer(buffer, kOffset, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, kOffset);
}

// Stress test for many calls to WriteBuffer
TEST_P(QueueWriteBufferTests, ManyWriteBuffer) {
    // Note: Increasing the size of the buffer will likely cause timeout issues.
    // In D3D12, timeout detection occurs when the GPU scheduler tries but cannot preempt the task
    // executing these commands in-flight. If this takes longer than ~2s, a device reset occurs and
    // fails the test. Since GPUs may or may not complete by then, this test must be disabled OR
    // modified to be well-below the timeout limit.

    // TODO(crbug.com/dawn/228): Re-enable once the issue with Metal on 10.14.6 is fixed.
    DAWN_SUPPRESS_TEST_IF(IsMacOS() && IsIntel() && IsMetal());

    // The Vulkan Validation Layers' memory barrier validation keeps track of every range written
    // to independently which causes validation of each WriteBuffer to take increasing time, and
    // this test to take forever. Skip it when VVLs are enabled.
    DAWN_SUPPRESS_TEST_IF(IsVulkan() && IsBackendValidationEnabled());

    constexpr uint64_t kSize = 4000 * 1000;
    constexpr uint32_t kElements = 250 * 250;
    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        queue.WriteBuffer(buffer, i * sizeof(uint32_t), &i, sizeof(i));
        expectedData.push_back(i);
    }

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

// Test using WriteBuffer for lots of data
TEST_P(QueueWriteBufferTests, LargeWriteBuffer) {
    constexpr uint64_t kSize = 4000 * 1000;
    constexpr uint32_t kElements = 1000 * 1000;
    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        expectedData.push_back(i);
    }

    queue.WriteBuffer(buffer, 0, expectedData.data(), kElements * sizeof(uint32_t));

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

// Test using WriteBuffer for super large data block
TEST_P(QueueWriteBufferTests, SuperLargeWriteBuffer) {
    constexpr uint64_t kSize = 12000 * 1000;
    constexpr uint64_t kElements = 3000 * 1000;
    wgpu::BufferDescriptor descriptor;
    descriptor.size = kSize;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint32_t> expectedData;
    for (uint32_t i = 0; i < kElements; ++i) {
        expectedData.push_back(i);
    }

    queue.WriteBuffer(buffer, 0, expectedData.data(), kElements * sizeof(uint32_t));

    EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data(), buffer, 0, kElements);
}

// Test using the max buffer size. Regression test for dawn:1985. We don't bother validating the
// results for this case since that would take a lot longer, just that there are no errors.
TEST_P(QueueWriteBufferTests, MaxBufferSizeWriteBuffer) {
    uint32_t maxBufferSize = GetSupportedLimits().maxBufferSize;
    wgpu::BufferDescriptor descriptor;
    descriptor.size = maxBufferSize;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    std::vector<uint8_t> data(maxBufferSize);
    queue.WriteBuffer(buffer, 0, data.data(), maxBufferSize);
}

// Test a special code path: writing when dynamic uploader already contatins some unaligned
// data, it might be necessary to use a ring buffer with properly aligned offset.
TEST_P(QueueWriteBufferTests, UnalignedDynamicUploader) {
    // TODO(crbug.com/413053623): implement webgpu::Texture
    DAWN_SUPPRESS_TEST_IF(IsWebGPUOnWebGPU());

    utils::UnalignDynamicUploader(device);

    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    uint32_t value = 0x01020304;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));

    EXPECT_BUFFER_U32_EQ(value, buffer, 0);
}

// Test using various offset and size alignments to write a uniform buffer.
TEST_P(QueueWriteBufferTests, WriteUniformBufferWithVariousOffsetAndSizeAlignments) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 128;
    descriptor.usage =
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

    constexpr size_t kElementCount = 16;
    uint32_t data[kElementCount] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    constexpr size_t kElementBytes = sizeof(data[0]);
    queue.WriteBuffer(buffer, 0, data, sizeof(data));
    EXPECT_BUFFER_U32_RANGE_EQ(data, buffer, 0, kElementCount);

    // Alignments: offset -- 4, size -- 4
    size_t offset = 1;
    data[offset] = 100;
    size_t size = kElementBytes;
    queue.WriteBuffer(buffer, offset * kElementBytes, &data[offset], size);
    EXPECT_BUFFER_U32_RANGE_EQ(data, buffer, 0, kElementCount);

    // Alignments: offset -- 16, size -- 16
    offset = 4;
    data[offset] = 101;
    data[offset + 1] = 102;
    data[offset + 2] = 103;
    data[offset + 3] = 104;
    size = 4 * kElementBytes;
    queue.WriteBuffer(buffer, offset * kElementBytes, &data[offset], size);
    EXPECT_BUFFER_U32_RANGE_EQ(data, buffer, 0, kElementCount);

    // Alignments: offset -- 4, size -- 16
    offset = 10;
    data[offset] = 105;
    data[offset + 1] = 106;
    data[offset + 2] = 107;
    data[offset + 3] = 108;
    queue.WriteBuffer(buffer, offset * kElementBytes, &data[offset], size);
    EXPECT_BUFFER_U32_RANGE_EQ(data, buffer, 0, kElementCount);

    // Alignments: offset -- 16, size -- 4
    offset = 12;
    data[offset] = 109;
    size = kElementBytes;
    queue.WriteBuffer(buffer, offset * kElementBytes, &data[offset], size);
    EXPECT_BUFFER_U32_RANGE_EQ(data, buffer, 0, kElementCount);
}

DAWN_INSTANTIATE_TEST(QueueWriteBufferTests,
                      D3D11Backend(),
                      D3D11Backend({"d3d11_delay_flush_to_gpu"}),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend(),
                      WebGPUBackend());

// For MinimumDataSpec bytesPerRow and rowsPerImage, compute a default from the copy extent.
constexpr uint32_t kStrideComputeDefault = 0xFFFF'FFFEul;

namespace {
using TextureFormat = wgpu::TextureFormat;
DAWN_TEST_PARAM_STRUCT(WriteTextureFormatParams, TextureFormat);

struct TextureSpec {
    wgpu::Origin3D copyOrigin;
    wgpu::Extent3D textureSize;
    uint32_t level;
};

struct DataSpec {
    uint64_t size;
    uint64_t offset;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
};

void PackTextureData(const uint8_t* srcData,
                     uint32_t width,
                     uint32_t height,
                     uint32_t srcBytesPerRow,
                     uint8_t* dstData,
                     uint32_t dstBytesPerRow,
                     uint32_t bytesPerTexel) {
    for (uint64_t y = 0; y < height; ++y) {
        for (uint64_t x = 0; x < width; ++x) {
            uint64_t src = x * bytesPerTexel + y * srcBytesPerRow;
            uint64_t dst = x * bytesPerTexel + y * dstBytesPerRow;

            for (uint64_t i = 0; i < bytesPerTexel; i++) {
                dstData[dst + i] = srcData[src + i];
            }
        }
    }
}

void FillData(uint8_t* data, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        data[i] = static_cast<uint8_t>(i % 253);
    }
}
}  // namespace

class QueueWriteTextureTests : public DawnTestWithParams<WriteTextureFormatParams> {
  protected:
    static DataSpec MinimumDataSpec(wgpu::Extent3D writeSize,
                                    uint32_t overrideBytesPerRow = kStrideComputeDefault,
                                    uint32_t overrideRowsPerImage = kStrideComputeDefault) {
        uint32_t bytesPerRow =
            writeSize.width * utils::GetTexelBlockSizeInBytes(GetParam().mTextureFormat);
        if (overrideBytesPerRow != kStrideComputeDefault) {
            bytesPerRow = overrideBytesPerRow;
        }
        uint32_t rowsPerImage = writeSize.height;
        if (overrideRowsPerImage != kStrideComputeDefault) {
            rowsPerImage = overrideRowsPerImage;
        }

        uint32_t totalDataSize = utils::RequiredBytesInCopy(bytesPerRow, rowsPerImage, writeSize,
                                                            GetParam().mTextureFormat);
        return {totalDataSize, 0, bytesPerRow, rowsPerImage};
    }

    void DoTest(const TextureSpec& textureSpec,
                const DataSpec& dataSpec,
                const wgpu::Extent3D& copySize,
                const wgpu::TextureViewDimension bindingViewDimension =
                    wgpu::TextureViewDimension::Undefined) {
        // Create data of size `size` and populate it
        std::vector<uint8_t> data(dataSpec.size);
        FillData(data.data(), data.size());

        // Create a texture that is `width` x `height` with (`level` + 1) mip levels.
        wgpu::TextureDescriptor descriptor = {};
        wgpu::TextureBindingViewDimensionDescriptor textureBindingViewDimensionDesc;
        if (IsCompatibilityMode() &&
            bindingViewDimension != wgpu::TextureViewDimension::Undefined) {
            textureBindingViewDimensionDesc.textureBindingViewDimension = bindingViewDimension;
            descriptor.nextInChain = &textureBindingViewDimensionDesc;
        }
        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.size = textureSpec.textureSize;
        descriptor.format = GetParam().mTextureFormat;
        descriptor.mipLevelCount = textureSpec.level + 1;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        wgpu::TexelCopyBufferLayout texelCopyBufferLayout = utils::CreateTexelCopyBufferLayout(
            dataSpec.offset, dataSpec.bytesPerRow, dataSpec.rowsPerImage);

        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, textureSpec.level, textureSpec.copyOrigin);

        queue.WriteTexture(&texelCopyTextureInfo, data.data(), dataSpec.size,
                           &texelCopyBufferLayout, &copySize);

        const uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(GetParam().mTextureFormat);
        wgpu::Extent3D mipSize = {textureSpec.textureSize.width >> textureSpec.level,
                                  textureSpec.textureSize.height >> textureSpec.level,
                                  textureSpec.textureSize.depthOrArrayLayers};
        uint32_t bytesPerRow = dataSpec.bytesPerRow;
        if (bytesPerRow == wgpu::kCopyStrideUndefined) {
            bytesPerRow = mipSize.width * bytesPerTexel;
        }
        uint32_t alignedBytesPerRow = Align(bytesPerRow, bytesPerTexel);
        uint32_t appliedRowsPerImage =
            dataSpec.rowsPerImage > 0 ? dataSpec.rowsPerImage : mipSize.height;
        uint32_t bytesPerImage = bytesPerRow * appliedRowsPerImage;

        const uint32_t maxArrayLayer = textureSpec.copyOrigin.z + copySize.depthOrArrayLayers;

        uint64_t dataOffset = dataSpec.offset;
        const uint32_t byteSizeLastLayer =
            alignedBytesPerRow * (mipSize.height - 1) + mipSize.width * bytesPerTexel;

        for (uint32_t slice = textureSpec.copyOrigin.z; slice < maxArrayLayer; ++slice) {
            // Pack the data in the specified copy region to have the same
            // format as the expected texture data.
            std::vector<uint8_t> expected(byteSizeLastLayer, 0);
            PackTextureData(data.data() + dataOffset, copySize.width, copySize.height,
                            dataSpec.bytesPerRow, expected.data(), copySize.width * bytesPerTexel,
                            bytesPerTexel);

            EXPECT_TEXTURE_EQ(expected.data(), texture,
                              {textureSpec.copyOrigin.x, textureSpec.copyOrigin.y, slice},
                              {copySize.width, copySize.height, 1}, descriptor.format,
                              static_cast<uint8_t>(0), textureSpec.level)
                << "Write to texture failed copying " << dataSpec.size << "-byte data with offset "
                << dataSpec.offset << " and bytes per row " << dataSpec.bytesPerRow << " to [("
                << textureSpec.copyOrigin.x << ", " << textureSpec.copyOrigin.y << "), ("
                << textureSpec.copyOrigin.x + copySize.width << ", "
                << textureSpec.copyOrigin.y + copySize.height << ")) region of "
                << textureSpec.textureSize.width << " x " << textureSpec.textureSize.height
                << " texture at mip level " << textureSpec.level << " layer " << slice << "\n";

            dataOffset += bytesPerImage;
        }
    }
};

// Test writing the whole texture for varying texture sizes.
TEST_P(QueueWriteTextureTests, VaryingTextureSize) {
    for (unsigned int w : {127, 128}) {
        for (unsigned int h : {63, 64}) {
            for (unsigned int d : {1, 3, 4}) {
                TextureSpec textureSpec;
                textureSpec.textureSize = {w, h, d};
                textureSpec.copyOrigin = {0, 0, 0};
                textureSpec.level = 0;

                DoTest(textureSpec, MinimumDataSpec({w, h, d}), {w, h, d});
            }
        }
    }
}

// Test uploading a large amount of data with writeTexture.
TEST_P(QueueWriteTextureTests, LargeWriteTexture) {
    TextureSpec textureSpec;
    textureSpec.textureSize = {2048, 2048, 2};
    textureSpec.copyOrigin = {0, 0, 0};
    textureSpec.level = 0;

    DoTest(textureSpec, MinimumDataSpec(textureSpec.textureSize), textureSpec.textureSize);
}

// Test writing a pixel with an offset.
TEST_P(QueueWriteTextureTests, VaryingTextureOffset) {
    // The entire subresource must be copied when the format is a depth/stencil format.
    DAWN_TEST_UNSUPPORTED_IF(utils::IsDepthOrStencilFormat(GetParam().mTextureFormat));

    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    DataSpec pixelData = MinimumDataSpec({1, 1, 1});

    constexpr wgpu::Extent3D kCopySize = {1, 1, 1};
    constexpr wgpu::Extent3D kTextureSize = {kWidth, kHeight, 1};
    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = kTextureSize;
    defaultTextureSpec.level = 0;

    for (unsigned int w : {0u, kWidth / 7, kWidth / 3, kWidth - 1}) {
        for (unsigned int h : {0u, kHeight / 7, kHeight / 3, kHeight - 1}) {
            TextureSpec textureSpec = defaultTextureSpec;
            textureSpec.copyOrigin = {w, h, 0};
            DoTest(textureSpec, pixelData, kCopySize);
        }
    }
}

// Test writing a pixel with an offset to a texture array
TEST_P(QueueWriteTextureTests, VaryingTextureArrayOffset) {
    // The entire subresource must be copied when the format is a depth/stencil format.
    DAWN_TEST_UNSUPPORTED_IF(utils::IsDepthOrStencilFormat(GetParam().mTextureFormat));
    // TODO(crbug.com/dawn/2095): Failing on ANGLE + SwiftShader, needs investigation.
    DAWN_SUPPRESS_TEST_IF(IsANGLESwiftShader());

    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    constexpr uint32_t kDepth = 62;
    DataSpec pixelData = MinimumDataSpec({1, 1, 1});

    constexpr wgpu::Extent3D kCopySize = {1, 1, 1};
    constexpr wgpu::Extent3D kTextureSize = {kWidth, kHeight, kDepth};
    TextureSpec defaultTextureSpec;
    defaultTextureSpec.textureSize = kTextureSize;
    defaultTextureSpec.level = 0;

    for (unsigned int w : {0u, kWidth / 7, kWidth / 3, kWidth - 1}) {
        for (unsigned int h : {0u, kHeight / 7, kHeight / 3, kHeight - 1}) {
            for (unsigned int d : {0u, kDepth / 7, kDepth / 3, kDepth - 1}) {
                TextureSpec textureSpec = defaultTextureSpec;
                textureSpec.copyOrigin = {w, h, d};
                DoTest(textureSpec, pixelData, kCopySize);
            }
        }
    }
}

// Test writing with varying write sizes.
TEST_P(QueueWriteTextureTests, VaryingWriteSize) {
    // The entire subresource must be copied when the format is a depth/stencil format.
    DAWN_TEST_UNSUPPORTED_IF(utils::IsDepthOrStencilFormat(GetParam().mTextureFormat));
    constexpr uint32_t kWidth = 257;
    constexpr uint32_t kHeight = 127;
    for (unsigned int w : {13, 63, 128, 256}) {
        for (unsigned int h : {16, 19, 32, 63}) {
            TextureSpec textureSpec;
            textureSpec.copyOrigin = {0, 0, 0};
            textureSpec.level = 0;
            textureSpec.textureSize = {kWidth, kHeight, 1};
            DoTest(textureSpec, MinimumDataSpec({w, h, 1}), {w, h, 1});
        }
    }
}

// Test writing with varying write sizes to texture arrays.
TEST_P(QueueWriteTextureTests, VaryingArrayWriteSize) {
    // The entire subresource must be copied when the format is a depth/stencil format.
    DAWN_TEST_UNSUPPORTED_IF(utils::IsDepthOrStencilFormat(GetParam().mTextureFormat));
    constexpr uint32_t kWidth = 257;
    constexpr uint32_t kHeight = 127;
    constexpr uint32_t kDepth = 65;
    for (unsigned int w : {13, 63, 128, 256}) {
        for (unsigned int h : {16, 19, 32, 63}) {
            for (unsigned int d : {3, 6}) {
                TextureSpec textureSpec;
                textureSpec.copyOrigin = {0, 0, 0};
                textureSpec.level = 0;
                textureSpec.textureSize = {kWidth, kHeight, kDepth};
                DoTest(textureSpec, MinimumDataSpec({w, h, d}), {w, h, d});
            }
        }
    }
}

// Test writing to varying mips
TEST_P(QueueWriteTextureTests, TextureWriteToMip) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec defaultTextureSpec;
    defaultTextureSpec.copyOrigin = {0, 0, 0};
    defaultTextureSpec.textureSize = {kWidth, kHeight, 1};

    for (unsigned int i = 1; i < 4; ++i) {
        TextureSpec textureSpec = defaultTextureSpec;
        textureSpec.level = i;
        DoTest(textureSpec, MinimumDataSpec({kWidth >> i, kHeight >> i, 1}),
               {kWidth >> i, kHeight >> i, 1});
    }
}

// Test writing with different multiples of texel block size as data offset
TEST_P(QueueWriteTextureTests, VaryingDataOffset) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, 0};
    textureSpec.textureSize = {kWidth, kHeight, 1};
    textureSpec.level = 0;

    for (uint64_t offset : {1, 2, 4, 17, 64, 128, 300}) {
        DataSpec dataSpec = MinimumDataSpec({kWidth, kHeight, 1});
        dataSpec.size += offset;
        dataSpec.offset += offset;
        DoTest(textureSpec, dataSpec, {kWidth, kHeight, 1});
    }
}

// Test writing with rowsPerImage greater than needed.
TEST_P(QueueWriteTextureTests, VaryingRowsPerImage) {
    constexpr uint32_t kWidth = 65;
    constexpr uint32_t kHeight = 31;
    constexpr uint32_t kDepth = 17;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kDepth};
    textureSpec.level = 0;

    auto TestBody = [&](wgpu::Origin3D copyOrigin, wgpu::Extent3D copySize) {
        textureSpec.copyOrigin = copyOrigin;
        for (unsigned int r : {1, 2, 3, 64, 200}) {
            DataSpec dataSpec =
                MinimumDataSpec(copySize, kStrideComputeDefault, copySize.height + r);
            DoTest(textureSpec, dataSpec, copySize);
        }
    };

    TestBody({0, 0, 0}, textureSpec.textureSize);

    if (utils::IsDepthOrStencilFormat(GetParam().mTextureFormat)) {
        // The entire subresource must be copied when the format is a depth/stencil format.
        return;
    }

    TestBody({1, 1, 1}, {kWidth - 1, kHeight - 1, kDepth - 1});
}

// Test with bytesPerRow greater than needed
TEST_P(QueueWriteTextureTests, VaryingBytesPerRow) {
    constexpr uint32_t kWidth = 257;
    constexpr uint32_t kHeight = 129;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 1};
    textureSpec.level = 0;

    auto TestBody = [&](wgpu::Origin3D copyOrigin, wgpu::Extent3D copyExtent) {
        textureSpec.copyOrigin = copyOrigin;
        for (unsigned int b : {1, 2, 3, 4}) {
            uint32_t bytesPerRow =
                copyExtent.width * utils::GetTexelBlockSizeInBytes(GetParam().mTextureFormat) + b;
            DoTest(textureSpec, MinimumDataSpec(copyExtent, bytesPerRow), copyExtent);
        }
    };

    TestBody({0, 0, 0}, textureSpec.textureSize);

    if (utils::IsDepthOrStencilFormat(GetParam().mTextureFormat)) {
        // The entire subresource must be copied when the format is a depth/stencil format.
        return;
    }

    TestBody({1, 2, 0}, {17, 19, 1});
}

// Test with bytesPerRow greater than needed for cube textures.
// Made for testing compat behavior.
TEST_P(QueueWriteTextureTests, VaryingBytesPerRowCube) {
    auto format = GetParam().mTextureFormat;
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 4 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsQualcomm());
    // TODO(crbug.com/dawn/2295): diagnose this failure on Pixel 6 OpenGLES
    DAWN_SUPPRESS_TEST_IF(IsOpenGLES() && IsAndroid() && IsARM());
    // TODO(crbug.com/dawn/2131): diagnose this failure on Win Angle D3D11
    DAWN_SUPPRESS_TEST_IF(IsANGLED3D11());
    // TODO(crbug.com/dawn/42241333): diagnose stencil8 failure on Angle Swiftshader
    DAWN_SUPPRESS_TEST_IF(format == wgpu::TextureFormat::Stencil8 && IsANGLESwiftShader());

    // TODO(383765096): D3D11 doesn't allow calling Gather() on R8_UINT
    DAWN_SUPPRESS_TEST_IF(format == wgpu::TextureFormat::Stencil8 && IsD3D11() &&
                          IsCompatibilityMode());

    constexpr uint32_t kWidth = 257;
    constexpr uint32_t kHeight = 257;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, 6};
    textureSpec.level = 0;

    auto TestBody = [&](wgpu::Origin3D copyOrigin, wgpu::Extent3D copyExtent) {
        textureSpec.copyOrigin = copyOrigin;
        for (unsigned int b : {1, 2, 3, 4}) {
            uint32_t bytesPerRow = copyExtent.width * utils::GetTexelBlockSizeInBytes(format) + b;
            DoTest(textureSpec, MinimumDataSpec(copyExtent, bytesPerRow), copyExtent,
                   wgpu::TextureViewDimension::Cube);
        }
    };

    TestBody({0, 0, 0}, textureSpec.textureSize);

    if (utils::IsDepthOrStencilFormat(format)) {
        // The entire subresource must be copied when the format is a depth/stencil format.
        return;
    }

    TestBody({1, 2, 0}, {17, 17, 1});
}

// Test that writing with bytesPerRow = 0 and bytesPerRow < bytesInACompleteRow works
// when we're copying one row only
TEST_P(QueueWriteTextureTests, BytesPerRowWithOneRowCopy) {
    // The entire subresource must be copied when the format is a depth/stencil format.
    DAWN_TEST_UNSUPPORTED_IF(utils::IsDepthOrStencilFormat(GetParam().mTextureFormat));

    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, 0};
    textureSpec.textureSize = {kWidth, kHeight, 1};
    textureSpec.level = 0;

    {
        constexpr wgpu::Extent3D copyExtent = {5, 1, 1};
        DataSpec dataSpec = MinimumDataSpec(copyExtent);

        // bytesPerRow undefined
        dataSpec.bytesPerRow = wgpu::kCopyStrideUndefined;
        DoTest(textureSpec, dataSpec, copyExtent);
    }
}

// Test with bytesPerRow greater than needed in a write to a texture array.
TEST_P(QueueWriteTextureTests, VaryingArrayBytesPerRow) {
    // TODO(crbug.com/dawn/2095): Failing on ANGLE + SwiftShader, needs investigation.
    DAWN_SUPPRESS_TEST_IF(IsANGLESwiftShader());

    // TODO(383779503): reading stencil texture is too slow on D3D11.
    DAWN_SUPPRESS_TEST_IF(IsD3D11() && GetParam().mTextureFormat == wgpu::TextureFormat::Stencil8);

    constexpr uint32_t kWidth = 257;
    constexpr uint32_t kHeight = 129;
    constexpr uint32_t kLayers = 65;

    TextureSpec textureSpec;
    textureSpec.textureSize = {kWidth, kHeight, kLayers};
    textureSpec.level = 0;

    auto TestBody = [&](wgpu::Origin3D copyOrigin, wgpu::Extent3D copyExtent) {
        textureSpec.copyOrigin = copyOrigin;
        // Test with bytesPerRow divisible by blockWidth
        for (unsigned int b : {1, 2, 3, 65, 300}) {
            uint32_t bytesPerRow =
                (copyExtent.width + b) * utils::GetTexelBlockSizeInBytes(GetParam().mTextureFormat);
            uint32_t rowsPerImage = copyExtent.height;
            DoTest(textureSpec, MinimumDataSpec(copyExtent, bytesPerRow, rowsPerImage), copyExtent);
        }

        // Test with bytesPerRow not divisible by blockWidth
        for (unsigned int b : {1, 2, 3, 19, 301}) {
            uint32_t bytesPerRow =
                copyExtent.width * utils::GetTexelBlockSizeInBytes(GetParam().mTextureFormat) + b;
            uint32_t rowsPerImage = copyExtent.height;
            DoTest(textureSpec, MinimumDataSpec(copyExtent, bytesPerRow, rowsPerImage), copyExtent);
        }
    };

    TestBody({0, 0, 0}, textureSpec.textureSize);

    if (utils::IsDepthOrStencilFormat(GetParam().mTextureFormat)) {
        // The entire subresource must be copied when the format is a depth/stencil format.
        return;
    }

    TestBody({1, 2, 3}, {17, 19, 21});
}

// Test valid special cases of bytesPerRow and rowsPerImage (0 or undefined).
TEST_P(QueueWriteTextureTests, StrideSpecialCases) {
    // The entire subresource must be copied when the format is a depth/stencil format.
    DAWN_TEST_UNSUPPORTED_IF(utils::IsDepthOrStencilFormat(GetParam().mTextureFormat));

    TextureSpec textureSpec;
    textureSpec.copyOrigin = {0, 0, 0};
    textureSpec.textureSize = {4, 4, 4};
    textureSpec.level = 0;

    // bytesPerRow 0
    for (const wgpu::Extent3D copyExtent :
         {wgpu::Extent3D{0, 2, 2}, {0, 0, 2}, {0, 2, 0}, {0, 0, 0}}) {
        DoTest(textureSpec, MinimumDataSpec(copyExtent, 0, 2), copyExtent);
    }

    // bytesPerRow undefined
    for (const wgpu::Extent3D copyExtent :
         {wgpu::Extent3D{2, 1, 1}, {2, 0, 1}, {2, 1, 0}, {2, 0, 0}}) {
        DoTest(textureSpec, MinimumDataSpec(copyExtent, wgpu::kCopyStrideUndefined, 2), copyExtent);
    }

    // rowsPerImage 0
    for (const wgpu::Extent3D copyExtent :
         {wgpu::Extent3D{2, 0, 2}, {2, 0, 0}, {0, 0, 2}, {0, 0, 0}}) {
        DoTest(textureSpec, MinimumDataSpec(copyExtent, 256, 0), copyExtent);
    }

    // rowsPerImage undefined
    for (const wgpu::Extent3D copyExtent : {wgpu::Extent3D{2, 2, 1}, {2, 2, 0}}) {
        DoTest(textureSpec, MinimumDataSpec(copyExtent, 256, wgpu::kCopyStrideUndefined),
               copyExtent);
    }
}

// Testing a special code path: writing when dynamic uploader already contatins some unaligned
// data, it might be necessary to use a ring buffer with properly aligned offset.
TEST_P(QueueWriteTextureTests, UnalignedDynamicUploader) {
    utils::UnalignDynamicUploader(device);

    constexpr wgpu::Extent3D size = {10, 10, 1};

    TextureSpec textureSpec;
    textureSpec.textureSize = size;
    textureSpec.copyOrigin = {0, 0, 0};
    textureSpec.level = 0;

    DoTest(textureSpec, MinimumDataSpec(size), size);
}

DAWN_INSTANTIATE_TEST_P(QueueWriteTextureTests,
                        {D3D11Backend(), D3D11Backend({"d3d11_delay_flush_to_gpu"}), D3D12Backend(),
                         D3D12Backend({"d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_"
                                       "copy_with_non_zero_buffer_offset"}),
                         MetalBackend(),
                         MetalBackend({"use_blit_for_buffer_to_depth_texture_copy",
                                       "use_blit_for_buffer_to_stencil_texture_copy"}),
                         OpenGLBackend(), OpenGLESBackend(),
                         OpenGLESBackend({"use_blit_for_stencil_texture_write"}), VulkanBackend()},
                        {
                            wgpu::TextureFormat::R8Unorm,
                            wgpu::TextureFormat::RG8Unorm,
                            wgpu::TextureFormat::RGBA8Unorm,
                            wgpu::TextureFormat::Stencil8,
                        });

class QueueWriteTextureSimpleTests : public DawnTest {
  protected:
    void DoSimpleWriteTextureTest(uint32_t width, uint32_t height) {
        constexpr wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;
        constexpr uint32_t kPixelSize = 4;

        std::vector<uint32_t> data(width * height);
        for (size_t i = 0; i < data.size(); i++) {
            data[i] = 0xFFFFFFFF;
        }

        wgpu::TextureDescriptor descriptor = {};
        descriptor.size = {width, height, 1};
        descriptor.format = kFormat;
        descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        wgpu::Texture texture = device.CreateTexture(&descriptor);

        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(0, width * kPixelSize);
        wgpu::Extent3D copyExtent = {width, height, 1};
        device.GetQueue().WriteTexture(&texelCopyTextureInfo, data.data(),
                                       width * height * kPixelSize, &texelCopyBufferLayout,
                                       &copyExtent);

        EXPECT_TEXTURE_EQ(data.data(), texture, {0, 0}, {width, height});
    }
};

// This tests for a bug that occurred within the D3D12 CopyTextureSplitter, which incorrectly copied
// data when the internal offset was larger than 256, but less than 512 and the copy size was 64
// width or less with a height of 1.
TEST_P(QueueWriteTextureSimpleTests, WriteTo64x1TextureFromUnalignedDynamicUploader) {
    // First, WriteTexture with 96 pixels, or 384 bytes to create an offset in the dynamic uploader.
    DoSimpleWriteTextureTest(96, 1);

    // Now test writing to a 64x1 texture. Because a 64x1 texture's row pitch is equal to its slice
    // pitch, the texture copy offset could be calculated incorrectly inside the internal D3D12
    // TextureCopySplitter.
    DoSimpleWriteTextureTest(64, 1);
}

// This tests for a bug in the allocation of internal staging buffer, which incorrectly copied depth
// stencil data to the internal offset that is not a multiple of 4.
TEST_P(QueueWriteTextureSimpleTests, WriteStencilAspectWithSourceOffsetUnalignedTo4) {
    // TODO(crbug.com/dawn/2095): Failing on ANGLE + SwiftShader, needs investigation.
    DAWN_SUPPRESS_TEST_IF(IsANGLESwiftShader());

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    textureDescriptor.size = {1, 1, 1};
    wgpu::Texture dstTexture1 = device.CreateTexture(&textureDescriptor);
    wgpu::Texture dstTexture2 = device.CreateTexture(&textureDescriptor);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = 8u;
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer outputBuffer = device.CreateBuffer(&bufferDescriptor);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

    constexpr wgpu::Extent3D kWriteSize = {1, 1, 1};
    constexpr uint8_t kData[] = {1, 2};
    constexpr uint32_t kBytesPerRowForWriteTexture = 1u;

    std::vector<uint8_t> expectedData(8, 0);

    // In the first call of queue.writeTexture(), Dawn will allocate a new staging buffer in its
    // internal ring buffer and write the user data into it at the offset 0.
    {
        constexpr uint32_t kDataOffset1 = 0u;
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(kDataOffset1, kBytesPerRowForWriteTexture);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
            dstTexture1, 0, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);
        queue.WriteTexture(&texelCopyTextureInfo, kData, sizeof(kData), &texelCopyBufferLayout,
                           &kWriteSize);

        constexpr uint32_t kOutputBufferOffset1 = 0u;
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            outputBuffer, kOutputBufferOffset1, kTextureBytesPerRowAlignment);
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &kWriteSize);

        expectedData[kOutputBufferOffset1] = kData[kDataOffset1];
    }

    // In the second call of queue.writeTexture(), Dawn will still use the same staging buffer
    // allocated in the first call, whose first 2 bytes have been used in the first call of
    // queue.writeTexture(). Dawn should write the user data at the offset 4 bytes since the
    // destination texture aspect is stencil.
    {
        constexpr uint32_t kDataOffset2 = 1u;
        wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
            utils::CreateTexelCopyBufferLayout(kDataOffset2, kBytesPerRowForWriteTexture);
        wgpu::TexelCopyTextureInfo texelCopyTextureInfo = utils::CreateTexelCopyTextureInfo(
            dstTexture2, 0, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);
        queue.WriteTexture(&texelCopyTextureInfo, kData, sizeof(kData), &texelCopyBufferLayout,
                           &kWriteSize);

        constexpr uint32_t kOutputBufferOffset2 = 4u;
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo = utils::CreateTexelCopyBufferInfo(
            outputBuffer, kOutputBufferOffset2, kTextureBytesPerRowAlignment);
        encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &kWriteSize);

        expectedData[kOutputBufferOffset2] = kData[kDataOffset2];
    }

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    EXPECT_BUFFER_U8_RANGE_EQ(expectedData.data(), outputBuffer, 0, 8);
}

// Tests calling queue.writeTexture() to a depth texture after calling queue.writeTexture() on
// another texture always works. On some D3D12 backends the buffer offset of buffer-to-texture
// copies must be a multiple of 512 when the destination texture is a depth stencil texture.
TEST_P(QueueWriteTextureSimpleTests, WriteDepthAspectAfterOtherQueueWriteTextureCalls) {
    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.format = wgpu::TextureFormat::Depth16Unorm;
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    textureDescriptor.size = {1, 1, 1};
    wgpu::Texture depthTexture1 = device.CreateTexture(&textureDescriptor);
    wgpu::Texture depthTexture2 = device.CreateTexture(&textureDescriptor);

    constexpr uint16_t kExpectedData1 = (204 << 8) | 205;
    wgpu::TexelCopyTextureInfo texelCopyTextureInfo1 =
        utils::CreateTexelCopyTextureInfo(depthTexture1);
    // (Off-topic) spot-test for defaulting of .aspect.
    texelCopyTextureInfo1.aspect = wgpu::TextureAspect::Undefined;
    wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
        utils::CreateTexelCopyBufferLayout(0, sizeof(kExpectedData1));
    queue.WriteTexture(&texelCopyTextureInfo1, &kExpectedData1, sizeof(kExpectedData1),
                       &texelCopyBufferLayout, &textureDescriptor.size);

    constexpr uint16_t kExpectedData2 = (206 << 8) | 207;
    wgpu::TexelCopyTextureInfo texelCopyTextureInfo2 =
        utils::CreateTexelCopyTextureInfo(depthTexture2);
    queue.WriteTexture(&texelCopyTextureInfo2, &kExpectedData2, sizeof(kExpectedData2),
                       &texelCopyBufferLayout, &textureDescriptor.size);

    EXPECT_TEXTURE_EQ(&kExpectedData1, depthTexture1, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::DepthOnly);
    EXPECT_TEXTURE_EQ(&kExpectedData2, depthTexture2, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::DepthOnly);
}

// Tests calling queue.writeTexture() to the stencil aspect after calling queue.writeTexture() on
// another texture always works. On some D3D12 backends the buffer offset of buffer-to-texture
// copies must be a multiple of 512 when the destination texture is a depth stencil texture.
TEST_P(QueueWriteTextureSimpleTests, WriteStencilAspectAfterOtherQueueWriteTextureCalls) {
    // TODO(crbug.com/dawn/2095): Failing on ANGLE + SwiftShader, needs investigation.
    DAWN_SUPPRESS_TEST_IF(IsANGLESwiftShader());

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.format = wgpu::TextureFormat::Depth24PlusStencil8;
    textureDescriptor.usage = wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst;
    textureDescriptor.size = {1, 1, 1};
    wgpu::Texture depthStencilTexture1 = device.CreateTexture(&textureDescriptor);
    wgpu::Texture depthStencilTexture2 = device.CreateTexture(&textureDescriptor);

    constexpr uint8_t kExpectedData1 = 204u;
    wgpu::TexelCopyTextureInfo texelCopyTextureInfo1 = utils::CreateTexelCopyTextureInfo(
        depthStencilTexture1, 0, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);
    wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
        utils::CreateTexelCopyBufferLayout(0, sizeof(kExpectedData1));
    queue.WriteTexture(&texelCopyTextureInfo1, &kExpectedData1, sizeof(kExpectedData1),
                       &texelCopyBufferLayout, &textureDescriptor.size);

    constexpr uint8_t kExpectedData2 = 205;
    wgpu::TexelCopyTextureInfo texelCopyTextureInfo2 = utils::CreateTexelCopyTextureInfo(
        depthStencilTexture2, 0, {0, 0, 0}, wgpu::TextureAspect::StencilOnly);
    queue.WriteTexture(&texelCopyTextureInfo2, &kExpectedData2, sizeof(kExpectedData2),
                       &texelCopyBufferLayout, &textureDescriptor.size);

    EXPECT_TEXTURE_EQ(&kExpectedData1, depthStencilTexture1, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::StencilOnly);
    EXPECT_TEXTURE_EQ(&kExpectedData2, depthStencilTexture2, {0, 0}, {1, 1}, 0,
                      wgpu::TextureAspect::StencilOnly);
}

DAWN_INSTANTIATE_TEST(QueueWriteTextureSimpleTests,
                      D3D11Backend(),
                      D3D11Backend({"d3d11_use_unmonitored_fence"}),
                      D3D11Backend({"d3d11_disable_fence"}),
                      D3D11Backend({"d3d11_delay_flush_to_gpu"}),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
