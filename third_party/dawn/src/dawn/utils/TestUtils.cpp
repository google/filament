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

#include <algorithm>
#include <memory>
#include <ostream>
#include <thread>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn::utils {

const RGBA8 RGBA8::kZero = RGBA8(0, 0, 0, 0);
const RGBA8 RGBA8::kBlack = RGBA8(0, 0, 0, 255);
const RGBA8 RGBA8::kRed = RGBA8(255, 0, 0, 255);
const RGBA8 RGBA8::kGreen = RGBA8(0, 255, 0, 255);
const RGBA8 RGBA8::kBlue = RGBA8(0, 0, 255, 255);
const RGBA8 RGBA8::kYellow = RGBA8(255, 255, 0, 255);
const RGBA8 RGBA8::kWhite = RGBA8(255, 255, 255, 255);

std::ostream& operator<<(std::ostream& stream, const RGBA8& color) {
    return stream << "RGBA8(" << static_cast<int>(color.r) << ", " << static_cast<int>(color.g)
                  << ", " << static_cast<int>(color.b) << ", " << static_cast<int>(color.a) << ")";
}

uint32_t GetMinimumBytesPerRow(wgpu::TextureFormat format,
                               uint32_t width,
                               uint32_t textureBytesPerRowAlignment) {
    const uint32_t bytesPerBlock = dawn::utils::GetTexelBlockSizeInBytes(format);
    const uint32_t blockWidth = dawn::utils::GetTextureFormatBlockWidth(format);
    DAWN_ASSERT(width % blockWidth == 0);
    return Align(bytesPerBlock * (width / blockWidth), textureBytesPerRowAlignment);
}

TextureDataCopyLayout GetTextureDataCopyLayoutForTextureAtLevel(
    wgpu::TextureFormat format,
    wgpu::Extent3D textureSizeAtLevel0,
    uint32_t mipmapLevel,
    wgpu::TextureDimension dimension,
    uint32_t rowsPerImage,
    uint32_t textureBytesPerRowAlignment) {
    // Compressed texture formats not supported in this function yet.
    DAWN_ASSERT(dawn::utils::GetTextureFormatBlockWidth(format) == 1);

    TextureDataCopyLayout layout;

    layout.mipSize = {std::max(textureSizeAtLevel0.width >> mipmapLevel, 1u),
                      std::max(textureSizeAtLevel0.height >> mipmapLevel, 1u),
                      textureSizeAtLevel0.depthOrArrayLayers};

    if (dimension == wgpu::TextureDimension::e3D) {
        layout.mipSize.depthOrArrayLayers =
            std::max(textureSizeAtLevel0.depthOrArrayLayers >> mipmapLevel, 1u);
    }

    layout.bytesPerRow =
        GetMinimumBytesPerRow(format, layout.mipSize.width, textureBytesPerRowAlignment);

    if (rowsPerImage == wgpu::kCopyStrideUndefined) {
        rowsPerImage = layout.mipSize.height;
    }
    layout.rowsPerImage = rowsPerImage;

    uint32_t appliedRowsPerImage = rowsPerImage > 0 ? rowsPerImage : layout.mipSize.height;
    layout.bytesPerImage = layout.bytesPerRow * appliedRowsPerImage;

    layout.byteLength =
        RequiredBytesInCopy(layout.bytesPerRow, appliedRowsPerImage, layout.mipSize, format);

    const uint32_t bytesPerTexel = dawn::utils::GetTexelBlockSizeInBytes(format);
    layout.texelBlocksPerRow = layout.bytesPerRow / bytesPerTexel;
    layout.texelBlocksPerImage = layout.bytesPerImage / bytesPerTexel;
    layout.texelBlockCount = layout.byteLength / bytesPerTexel;

    return layout;
}

uint64_t RequiredBytesInCopy(uint64_t bytesPerRow,
                             uint64_t rowsPerImage,
                             wgpu::Extent3D copyExtent,
                             wgpu::TextureFormat textureFormat) {
    uint32_t blockSize = dawn::utils::GetTexelBlockSizeInBytes(textureFormat);
    uint32_t blockWidth = dawn::utils::GetTextureFormatBlockWidth(textureFormat);
    uint32_t blockHeight = dawn::utils::GetTextureFormatBlockHeight(textureFormat);
    DAWN_ASSERT(copyExtent.width % blockWidth == 0);
    uint32_t widthInBlocks = copyExtent.width / blockWidth;
    DAWN_ASSERT(copyExtent.height % blockHeight == 0);
    uint32_t heightInBlocks = copyExtent.height / blockHeight;
    return RequiredBytesInCopy(bytesPerRow, rowsPerImage, widthInBlocks, heightInBlocks,
                               copyExtent.depthOrArrayLayers, blockSize);
}

uint64_t RequiredBytesInCopy(uint64_t bytesPerRow,
                             uint64_t rowsPerImage,
                             uint64_t widthInBlocks,
                             uint64_t heightInBlocks,
                             uint64_t depth,
                             uint64_t bytesPerBlock) {
    if (depth == 0) {
        return 0;
    }

    uint64_t bytesPerImage = bytesPerRow * rowsPerImage;
    uint64_t requiredBytesInCopy = bytesPerImage * (depth - 1);
    if (heightInBlocks != 0) {
        uint64_t lastRowBytes = widthInBlocks * bytesPerBlock;
        uint64_t lastImageBytes = bytesPerRow * (heightInBlocks - 1) + lastRowBytes;
        requiredBytesInCopy += lastImageBytes;
    }
    return requiredBytesInCopy;
}

uint64_t GetTexelCountInCopyRegion(uint64_t bytesPerRow,
                                   uint64_t rowsPerImage,
                                   wgpu::Extent3D copyExtent,
                                   wgpu::TextureFormat textureFormat) {
    return RequiredBytesInCopy(bytesPerRow, rowsPerImage, copyExtent, textureFormat) /
           dawn::utils::GetTexelBlockSizeInBytes(textureFormat);
}

void UnalignDynamicUploader(wgpu::Device device) {
    std::vector<uint8_t> data = {1};

    wgpu::TextureDescriptor descriptor = {};
    descriptor.size = {1, 1, 1};
    descriptor.format = wgpu::TextureFormat::R8Unorm;
    descriptor.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
    wgpu::Texture texture = device.CreateTexture(&descriptor);

    wgpu::TexelCopyTextureInfo texelCopyTextureInfo =
        dawn::utils::CreateTexelCopyTextureInfo(texture, 0, {0, 0, 0});
    wgpu::TexelCopyBufferLayout texelCopyBufferLayout =
        dawn::utils::CreateTexelCopyBufferLayout(0, wgpu::kCopyStrideUndefined);
    wgpu::Extent3D copyExtent = {1, 1, 1};

    // WriteTexture with exactly 1 byte of data.
    device.GetQueue().WriteTexture(&texelCopyTextureInfo, data.data(), 1, &texelCopyBufferLayout,
                                   &copyExtent);
}

uint32_t VertexFormatSize(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8:
        case wgpu::VertexFormat::Sint8:
        case wgpu::VertexFormat::Unorm8:
        case wgpu::VertexFormat::Snorm8:
            return 1;
        case wgpu::VertexFormat::Uint8x2:
        case wgpu::VertexFormat::Sint8x2:
        case wgpu::VertexFormat::Unorm8x2:
        case wgpu::VertexFormat::Snorm8x2:
        case wgpu::VertexFormat::Uint16:
        case wgpu::VertexFormat::Sint16:
        case wgpu::VertexFormat::Unorm16:
        case wgpu::VertexFormat::Snorm16:
        case wgpu::VertexFormat::Float16:
            return 2;
        case wgpu::VertexFormat::Uint8x4:
        case wgpu::VertexFormat::Sint8x4:
        case wgpu::VertexFormat::Unorm8x4:
        case wgpu::VertexFormat::Snorm8x4:
        case wgpu::VertexFormat::Uint16x2:
        case wgpu::VertexFormat::Sint16x2:
        case wgpu::VertexFormat::Unorm16x2:
        case wgpu::VertexFormat::Snorm16x2:
        case wgpu::VertexFormat::Float16x2:
        case wgpu::VertexFormat::Float32:
        case wgpu::VertexFormat::Uint32:
        case wgpu::VertexFormat::Sint32:
        case wgpu::VertexFormat::Unorm10_10_10_2:
        case wgpu::VertexFormat::Unorm8x4BGRA:
            return 4;
        case wgpu::VertexFormat::Uint16x4:
        case wgpu::VertexFormat::Sint16x4:
        case wgpu::VertexFormat::Unorm16x4:
        case wgpu::VertexFormat::Snorm16x4:
        case wgpu::VertexFormat::Float16x4:
        case wgpu::VertexFormat::Float32x2:
        case wgpu::VertexFormat::Uint32x2:
        case wgpu::VertexFormat::Sint32x2:
            return 8;
        case wgpu::VertexFormat::Float32x3:
        case wgpu::VertexFormat::Uint32x3:
        case wgpu::VertexFormat::Sint32x3:
            return 12;
        case wgpu::VertexFormat::Float32x4:
        case wgpu::VertexFormat::Uint32x4:
        case wgpu::VertexFormat::Sint32x4:
            return 16;
    }
    DAWN_UNREACHABLE();
}

void RunInParallel(uint32_t numThreads,
                   const std::function<void(uint32_t)>& workerFunc,
                   const std::function<void()>& mainThreadFunc) {
    std::vector<std::unique_ptr<std::thread>> threads(numThreads);

    for (uint32_t i = 0; i < threads.size(); ++i) {
        threads[i] = std::make_unique<std::thread>([i, workerFunc] { workerFunc(i); });
    }

    if (mainThreadFunc != nullptr) {
        mainThreadFunc();
    }

    for (auto& thread : threads) {
        thread->join();
    }
}

}  // namespace dawn::utils
