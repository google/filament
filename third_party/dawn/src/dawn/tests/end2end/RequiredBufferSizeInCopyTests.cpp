// Copyright 2022 The Dawn & Tint Authors
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

#include "dawn/common/Platform.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

constexpr static wgpu::Extent3D kCopySize = {1, 1};
constexpr static uint64_t kOffset = 0;
constexpr static uint64_t kBytesPerRow = 256;
constexpr static wgpu::TextureFormat kFormat = wgpu::TextureFormat::RGBA8Unorm;
constexpr static uint32_t kBytesPerBlock = 4;

enum class Type { B2TCopy, T2BCopy };

std::ostream& operator<<(std::ostream& o, Type copyType) {
    switch (copyType) {
        case Type::B2TCopy:
            o << "B2TCopy";
            break;
        case Type::T2BCopy:
            o << "T2BCopy";
            break;
    }
    return o;
}

using TextureDimension = wgpu::TextureDimension;
using CopyDepth = uint32_t;
using ExtraRowsPerImage = uint64_t;
DAWN_TEST_PARAM_STRUCT(RequiredBufferSizeInCopyTestsParams,
                       Type,
                       TextureDimension,
                       CopyDepth,
                       ExtraRowsPerImage);

// Tests in this file are used to expose an error on D3D12 about required minimum buffer size.
// See detailed bug reports at crbug.com/dawn/1278, 1288, 1289.

// When we do B2T or T2B copy from/to a buffer with paddings, it may wrongly calculate
// the required buffer size on D3D12.

// Using the data in this test as an example, in which copySize = {1, 1, 2}, offset = 0, bytesPerRow
// = 256, and rowsPerImage = 2 (there is 1-row padding for every image), and assuming we are copying
// a non-compressed format like rgba8unorm, the required minimum buffer size should be:
//   offset + bytesPerRow * rowsPerImage * (copySize.depthOrArrayLayers - 1)
//     + bytesPerRow * (copySize.height - 1) + bytesPerBlock * copySize.width.
// It is 0 + 256 * 2 * (2 - 1) + 256 * (1 - 1) + 4 * 1 = 516.

// However, the required minimum buffer size on D3D12 (including WARP) is:
//   offset + bytesPerRow * rowsPerImage * (copySize.depthOrArrayLayers - 1)
//     + bytesPerRow * (rowsPerImage - 1) + bytesPerBlock * copySize.width.
// Or
//   offset + bytesPerRow * rowsPerImage * copySize.depthOrArrayLayers
//     + bytesPerBlock * copySize.width - bytesPerRow.
// It is 0 + 256 * 2 * (2 - 1) + 256 * (2 - 1) + 4 * 1 = 772.

// It looks like D3D12 requires unnecessary buffer storage for rowsPerImagePadding in the last
// image. It does respect bytesPerRowPadding in the last row and doesn't require storage for
// that part, though.
//
// Further tests reveal that this incorrect calculation exist only when the texture dimension
// is 3D, and there are rowsPerImage paddings, and there are multiple depth images.

class RequiredBufferSizeInCopyTests
    : public DawnTestWithParams<RequiredBufferSizeInCopyTestsParams> {
  protected:
    void DoTest(const uint64_t bufferSize,
                const wgpu::Extent3D copySize,
                const uint64_t rowsPerImage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = bufferSize;
        descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);

        wgpu::TextureDescriptor texDesc = {};
        texDesc.dimension = GetParam().mTextureDimension;
        texDesc.size = copySize;
        texDesc.format = kFormat;
        texDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
        wgpu::Texture texture = device.CreateTexture(&texDesc);

        wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
            utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
        wgpu::TexelCopyBufferInfo texelCopyBufferInfo =
            utils::CreateTexelCopyBufferInfo(buffer, kOffset, kBytesPerRow, rowsPerImage);

        // Initialize copied data and set expected data for buffer and texture.
        DAWN_ASSERT(sizeof(uint32_t) == kBytesPerBlock);
        uint32_t numOfBufferElements = bufferSize / kBytesPerBlock;
        std::vector<uint32_t> data(numOfBufferElements, 1);
        std::vector<uint32_t> expectedBufferData(numOfBufferElements, 0);
        std::vector<uint32_t> expectedTextureData(copySize.depthOrArrayLayers, 0);
        // Initialize the first element on every image to be 0x80808080
        uint64_t imageSize = kBytesPerRow * rowsPerImage;
        DAWN_ASSERT(bufferSize >= (imageSize * (copySize.depthOrArrayLayers - 1) + kBytesPerBlock));
        uint32_t numOfImageElements = imageSize / kBytesPerBlock;
        for (uint32_t i = 0; i < copySize.depthOrArrayLayers; ++i) {
            data[i * numOfImageElements] = 0x80808080;
            expectedBufferData[i * numOfImageElements] = 0x80808080;
            expectedTextureData[i] = 0x80808080;
        }

        // Do B2T copy or T2B copy
        wgpu::CommandEncoder encoder = this->device.CreateCommandEncoder();
        switch (GetParam().mType) {
            case Type::T2BCopy: {
                wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
                    utils::CreateTexelCopyBufferLayout(kOffset, kBytesPerRow, rowsPerImage);

                queue.WriteTexture(&texelCopyTextureInfo, data.data(), bufferSize,
                                   &texelCopyBufferLayout, &copySize);

                encoder.CopyTextureToBuffer(&texelCopyTextureInfo, &texelCopyBufferInfo, &copySize);
                break;
            }
            case Type::B2TCopy:
                queue.WriteBuffer(buffer, 0, data.data(), bufferSize);
                encoder.CopyBufferToTexture(&texelCopyBufferInfo, &texelCopyTextureInfo, &copySize);
                break;
        }
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Verify the data in buffer (T2B copy) or texture (B2T copy)
        switch (GetParam().mType) {
            case Type::T2BCopy:
                EXPECT_BUFFER_U32_RANGE_EQ(expectedBufferData.data(), buffer, 0, bufferSize / 4);
                break;
            case Type::B2TCopy:
                EXPECT_TEXTURE_EQ(expectedTextureData.data(), texture, {0, 0, 0}, copySize);
                break;
        }
    }
};

// The buffer contains full data on the last image and has storage for all kinds of paddings.
TEST_P(RequiredBufferSizeInCopyTests, AbundantBufferSize) {
    wgpu::Extent3D copySize = kCopySize;
    copySize.depthOrArrayLayers = GetParam().mCopyDepth;
    const uint64_t extraRowsPerImage = GetParam().mExtraRowsPerImage;
    const uint64_t rowsPerImage = extraRowsPerImage + copySize.height;

    uint64_t size = kOffset + kBytesPerRow * rowsPerImage * copySize.depthOrArrayLayers;
    DoTest(size, copySize, rowsPerImage);
}

// The buffer has storage for rowsPerImage paddings on the last image but not bytesPerRow
// paddings on the last row, which is exactly what D3D12 requires. See the comments at the
// beginning of class RequiredBufferSizeInCopyTests for details.
TEST_P(RequiredBufferSizeInCopyTests, BufferSizeOnBoundary) {
    wgpu::Extent3D copySize = kCopySize;
    copySize.depthOrArrayLayers = GetParam().mCopyDepth;
    const uint64_t extraRowsPerImage = GetParam().mExtraRowsPerImage;
    // If there are no rowsPerImage paddings, buffer size required by D3D12 will be exactly the
    // same with the minimum size required by WebGPU spec, which can be covered by tests below
    // at MinimunBufferSize.
    if (extraRowsPerImage > 0) {
        const uint64_t rowsPerImage = extraRowsPerImage + copySize.height;

        uint64_t size = kOffset + kBytesPerRow * rowsPerImage * (copySize.depthOrArrayLayers - 1) +
                        kBytesPerRow * (rowsPerImage - 1) + kBytesPerBlock * copySize.width;
        DoTest(size, copySize, rowsPerImage);

        size -= kBytesPerBlock;
        DoTest(size, copySize, rowsPerImage);
    }
}

// The buffer doesn't have storage for any paddings on the last image. WebGPU spec doesn't require
// storage for these paddings, and the copy operation will never access to these paddings. So it
// should work.
TEST_P(RequiredBufferSizeInCopyTests, MinimumBufferSize) {
    wgpu::Extent3D copySize = kCopySize;
    copySize.depthOrArrayLayers = GetParam().mCopyDepth;
    const uint64_t extraRowsPerImage = GetParam().mExtraRowsPerImage;
    const uint64_t rowsPerImage = extraRowsPerImage + copySize.height;

    uint64_t size =
        kOffset + utils::RequiredBytesInCopy(kBytesPerRow, rowsPerImage, copySize, kFormat);
    DoTest(size, copySize, rowsPerImage);
}

DAWN_INSTANTIATE_TEST_P(RequiredBufferSizeInCopyTests,
                        {D3D11Backend(), D3D12Backend(), MetalBackend(), OpenGLBackend(),
                         OpenGLESBackend(), VulkanBackend()},
                        {Type::T2BCopy, Type::B2TCopy},
                        {wgpu::TextureDimension::e3D, wgpu::TextureDimension::e2D},
                        {2u, 1u},
                        {1u, 0u});

}  // anonymous namespace
}  // namespace dawn
