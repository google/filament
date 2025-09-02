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

#include <algorithm>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/common/Range.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d12/TextureCopySplitter.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/webgpu_cpp_print.h"
#include "gtest/gtest.h"

namespace dawn::native::d3d12 {
namespace {

// Suffix operator for TexelCount
constexpr TexelCount operator""_tc(uint64_t v) {
    return TexelCount{v};
}
// Suffix operator for BlockCount
constexpr BlockCount operator""_bc(uint64_t v) {
    return BlockCount{v};
}

struct TextureSpec {
    TexelOrigin3D origin;
    TexelExtent3D copySize;
    TypedTexelBlockInfo blockInfo;
};

struct BufferSpec {
    uint64_t offset;        // byte offset into buffer to copy to/from
    uint32_t bytesPerRow;   // bytes per block row (multiples of 256), aka row pitch
    BlockCount rowsPerImage;  // bock rows per image slice (user-defined)
};

// Check that each copy region fits inside the buffer footprint
void ValidateFootprints(const TextureSpec& textureSpec,
                        const BufferSpec& bufferSpec,
                        const TextureCopySubresource& copySubresource,
                        wgpu::TextureDimension dimension) {
    const TypedTexelBlockInfo& blockInfo = textureSpec.blockInfo;
    for (uint32_t i = 0; i < copySubresource.count; ++i) {
        const auto& copy = copySubresource.copies[i];
        // TODO(425944899): Rework this function to work in blocks, not texels
        const TexelExtent3D& copySize = blockInfo.ToTexel(copy.copySize);
        const TexelOrigin3D& bufferOffset = blockInfo.ToTexel(copy.bufferOffset);
        const TexelExtent3D& bufferSize = blockInfo.ToTexel(copy.bufferSize);
        ASSERT_LE(bufferOffset.x + copySize.width, bufferSize.width);
        ASSERT_LE(bufferOffset.y + copySize.height, bufferSize.height);
        ASSERT_LE(bufferOffset.z + copySize.depthOrArrayLayers, bufferSize.depthOrArrayLayers);

        BlockCount widthInBlocks = blockInfo.ToBlockWidth(textureSpec.copySize.width);
        BlockCount heightInBlocks = blockInfo.ToBlockHeight(textureSpec.copySize.height);
        uint64_t minimumRequiredBufferSize =
            bufferSpec.offset +
            // TOOD(425944899): add overload of RequiredBytesInCopy that accepts strong types
            utils::RequiredBytesInCopy(
                bufferSpec.bytesPerRow, static_cast<uint32_t>(bufferSpec.rowsPerImage),
                static_cast<uint32_t>(widthInBlocks), static_cast<uint32_t>(heightInBlocks),
                static_cast<uint32_t>(textureSpec.copySize.depthOrArrayLayers), blockInfo.byteSize);

        // The last pixel (buffer footprint) of each copy region depends on its
        // bufferOffset and copySize. It is not the last pixel where the bufferSize
        // ends.
        ASSERT_EQ(bufferOffset.x % blockInfo.width, 0_tc);
        ASSERT_EQ(copySize.width % blockInfo.width, 0_tc);
        TexelCount footprintWidth = bufferOffset.x + copySize.width;
        ASSERT_EQ(footprintWidth % blockInfo.width, 0_tc);
        BlockCount footprintWidthInBlocks = blockInfo.ToBlockWidth(footprintWidth);

        ASSERT_EQ(bufferOffset.y % blockInfo.height, 0_tc);
        ASSERT_EQ(copySize.height % blockInfo.height, 0_tc);
        TexelCount footprintHeight = bufferOffset.y + copySize.height;
        ASSERT_EQ(footprintHeight % blockInfo.height, 0_tc);
        BlockCount footprintHeightInBlocks = blockInfo.ToBlockHeight(footprintHeight);

        uint64_t bufferSizeForFootprint =
            copy.alignedOffset +
            utils::RequiredBytesInCopy(
                bufferSpec.bytesPerRow,
                static_cast<uint32_t>(blockInfo.ToBlockHeight(bufferSize.height)),
                static_cast<uint32_t>(footprintWidthInBlocks),
                static_cast<uint32_t>(footprintHeightInBlocks),
                static_cast<uint32_t>(blockInfo.ToBlockDepth(bufferSize.depthOrArrayLayers)),
                blockInfo.byteSize);

        // The buffer footprint of each copy region should not exceed the minimum
        // required buffer size. Otherwise, pixels accessed by copy may be OOB.
        ASSERT_LE(bufferSizeForFootprint, minimumRequiredBufferSize);
    }
}

// Check that the offset is aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512)
void ValidateOffset(const TextureCopySubresource& copySubresource, bool relaxed) {
    for (uint32_t i = 0; i < copySubresource.count; ++i) {
        if (!relaxed) {
            ASSERT_TRUE(Align(copySubresource.copies[i].alignedOffset,
                              D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT) ==
                        copySubresource.copies[i].alignedOffset);
        }
    }
}

template <typename T>
bool InclusiveRangesOverlap(T minA, T maxA, T minB, T maxB) {
    return (minA <= minB && minB <= maxA) || (minB <= minA && minA <= maxB);
}

// Check that no pair of copy regions intersect each other
void ValidateDisjoint(const TextureSpec& textureSpec,
                      const TextureCopySubresource& copySubresource) {
    const TypedTexelBlockInfo& blockInfo = textureSpec.blockInfo;
    for (uint32_t i = 0; i < copySubresource.count; ++i) {
        const auto& a = copySubresource.copies[i];
        // TODO(425944899): Rework this function to work in blocks, not texels
        const TexelExtent3D& copySizeA = blockInfo.ToTexel(a.copySize);
        const TexelOrigin3D& textureOffsetA = blockInfo.ToTexel(a.textureOffset);
        for (uint32_t j = i + 1; j < copySubresource.count; ++j) {
            const auto& b = copySubresource.copies[j];
            // If textureOffset.x is 0, and copySize.width is 2, we are copying pixel 0 and
            // 1. We never touch pixel 2 on x-axis. So the copied range on x-axis should be
            // [textureOffset.x, textureOffset.x + copySize.width - 1] and both ends are
            // included.
            const TexelExtent3D& copySizeB = blockInfo.ToTexel(b.copySize);
            const TexelOrigin3D& textureOffsetB = blockInfo.ToTexel(b.textureOffset);
            bool overlapX =
                InclusiveRangesOverlap(textureOffsetA.x, textureOffsetA.x + copySizeA.width - 1_tc,
                                       textureOffsetB.x, textureOffsetB.x + copySizeB.width - 1_tc);
            bool overlapY = InclusiveRangesOverlap(
                textureOffsetA.y, textureOffsetA.y + copySizeA.height - 1_tc, textureOffsetB.y,
                textureOffsetB.y + copySizeB.height - 1_tc);
            bool overlapZ = InclusiveRangesOverlap(
                textureOffsetA.z, textureOffsetA.z + copySizeA.depthOrArrayLayers - 1_tc,
                textureOffsetB.z, textureOffsetB.z + copySizeB.depthOrArrayLayers - 1_tc);
            ASSERT_TRUE(!overlapX || !overlapY || !overlapZ);
        }
    }
}

// Check that the union of the copy regions exactly covers the texture region
void ValidateTextureBounds(const TextureSpec& textureSpec,
                           const TextureCopySubresource& copySubresource) {
    ASSERT_GT(copySubresource.count, 0u);
    const TypedTexelBlockInfo& blockInfo = textureSpec.blockInfo;

    // TODO(425944899): Rework this function to work in blocks, not texels
    const TexelExtent3D& copySize0 = blockInfo.ToTexel(copySubresource.copies[0].copySize);
    const TexelOrigin3D& textureOffset0 =
        blockInfo.ToTexel(copySubresource.copies[0].textureOffset);
    TexelCount minX = textureOffset0.x;
    TexelCount minY = textureOffset0.y;
    TexelCount minZ = textureOffset0.z;
    TexelCount maxX = textureOffset0.x + copySize0.width;
    TexelCount maxY = textureOffset0.y + copySize0.height;
    TexelCount maxZ = textureOffset0.z + copySize0.depthOrArrayLayers;

    for (uint32_t i = 1; i < copySubresource.count; ++i) {
        const auto& copy = copySubresource.copies[i];
        const TexelOrigin3D& textureOffset = blockInfo.ToTexel(copy.textureOffset);
        minX = std::min(minX, textureOffset.x);
        minY = std::min(minY, textureOffset.y);
        minZ = std::min(minZ, textureOffset.z);
        const TexelExtent3D& copySize = blockInfo.ToTexel(copy.copySize);
        maxX = std::max(maxX, textureOffset.x + copySize.width);
        maxY = std::max(maxY, textureOffset.y + copySize.height);
        maxZ = std::max(maxZ, textureOffset.z + copySize.depthOrArrayLayers);
    }

    ASSERT_EQ(minX, textureSpec.origin.x);
    ASSERT_EQ(minY, textureSpec.origin.y);
    ASSERT_EQ(minZ, textureSpec.origin.z);
    ASSERT_EQ(maxX, textureSpec.origin.x + textureSpec.copySize.width);
    ASSERT_EQ(maxY, textureSpec.origin.y + textureSpec.copySize.height);
    ASSERT_EQ(maxZ, textureSpec.origin.z + textureSpec.copySize.depthOrArrayLayers);
}

// Validate that the number of pixels copied is exactly equal to the number of pixels in the
// texture region
void ValidatePixelCount(const TextureSpec& textureSpec,
                        const TextureCopySubresource& copySubresource) {
    const TypedTexelBlockInfo& blockInfo = textureSpec.blockInfo;
    TexelCount totalCopiedTexels{0};
    for (uint32_t i = 0; i < copySubresource.count; ++i) {
        const auto& copy = copySubresource.copies[i];
        // TODO(425944899): Rework this function to work in blocks, not texels
        const TexelExtent3D& copySize = blockInfo.ToTexel(copy.copySize);
        TexelCount copiedTexels = copySize.width * copySize.height * copySize.depthOrArrayLayers;
        ASSERT_GT(copiedTexels, 0_tc);
        totalCopiedTexels += copiedTexels;
    }
    ASSERT_EQ(totalCopiedTexels, textureSpec.copySize.width * textureSpec.copySize.height *
                                     textureSpec.copySize.depthOrArrayLayers);
}

// Check that every buffer offset is at the correct pixel location
void ValidateBufferOffset(const TextureSpec& textureSpec,
                          const BufferSpec& bufferSpec,
                          const TextureCopySubresource& copySubresource,
                          wgpu::TextureDimension dimension,
                          bool relaxed) {
    ASSERT_GT(copySubresource.count, 0u);
    const TypedTexelBlockInfo& blockInfo = textureSpec.blockInfo;

    for (uint32_t i = 0; i < copySubresource.count; ++i) {
        const auto& copy = copySubresource.copies[i];
        const BlockOrigin3D& bufferOffset = copy.bufferOffset;
        const BlockOrigin3D& textureOffset = copy.textureOffset;
        // Note that for relaxed, the row pitch (bytesPerRow) is not required to be 256 bytes,
        // but Dawn currently doesn't do anything about this.
        BlockCount rowPitchInBlocks = blockInfo.BytesToBlocks(bufferSpec.bytesPerRow);
        BlockCount slicePitchInBlocks = rowPitchInBlocks * bufferSpec.rowsPerImage;
        BlockCount absoluteOffsetInBlocks = blockInfo.BytesToBlocks(copy.alignedOffset) +
                                            bufferOffset.x + bufferOffset.y * rowPitchInBlocks;

        // There is one empty row at most in a 2D copy region. However, it is not true for
        // a 3D texture copy region when we are copying the last row of each slice. We may
        // need to offset a lot rows and copy.bufferOffset.y may be big.
        if (dimension == wgpu::TextureDimension::e2D) {
            ASSERT_LE(bufferOffset.y, BlockCount{1});
        }
        ASSERT_EQ(bufferOffset.z, 0_bc);

        ASSERT_GE(absoluteOffsetInBlocks, blockInfo.BytesToBlocks(bufferSpec.offset));

        BlockCount relativeOffsetInBlocks =
            absoluteOffsetInBlocks - blockInfo.BytesToBlocks(bufferSpec.offset);

        BlockCount zBlocks = relativeOffsetInBlocks / slicePitchInBlocks;
        BlockCount yBlocks = (relativeOffsetInBlocks % slicePitchInBlocks) / rowPitchInBlocks;
        BlockCount xBlocks = relativeOffsetInBlocks % rowPitchInBlocks;

        ASSERT_EQ(textureOffset.x - blockInfo.ToBlockWidth(textureSpec.origin.x), xBlocks);
        ASSERT_EQ(textureOffset.y - blockInfo.ToBlockHeight(textureSpec.origin.y), yBlocks);
        ASSERT_EQ(textureOffset.z - blockInfo.ToBlockDepth(textureSpec.origin.z), zBlocks);
    }
}

std::ostream& operator<<(std::ostream& os, const TextureSpec& textureSpec) {
    os << "TextureSpec(" << "[origin=(" << textureSpec.origin.x << ", " << textureSpec.origin.y
       << ", " << textureSpec.origin.z << "), copySize=(" << textureSpec.copySize.width << ", "
       << textureSpec.copySize.height << ", " << textureSpec.copySize.depthOrArrayLayers
       << ")], blockBytes=" << textureSpec.blockInfo.byteSize
       << ", blockWidth=" << textureSpec.blockInfo.width
       << ", blockHeight=" << textureSpec.blockInfo.height << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const BufferSpec& bufferSpec) {
    os << "BufferSpec(offset=" << bufferSpec.offset << ", bytesPerRow=" << bufferSpec.bytesPerRow
       << ", rowsPerImage=" << bufferSpec.rowsPerImage << ")";
    return os;
}
std::ostream& operator<<(std::ostream& os, const TextureCopySubresource& copySubresource) {
    os << "CopySplit\n";
    for (uint32_t i = 0; i < copySubresource.count; ++i) {
        const auto& copy = copySubresource.copies[i];
        auto& textureOffset = copy.textureOffset;
        auto& bufferOffset = copy.bufferOffset;
        auto& copySize = copy.copySize;
        os << "  " << i << ": Texture at (" << textureOffset.x << ", " << textureOffset.y << ", "
           << textureOffset.z << "), size (" << copySize.width << ", " << copySize.height << ", "
           << copySize.depthOrArrayLayers << ")\n";
        os << "  " << i << ": Buffer at (" << bufferOffset.x << ", " << bufferOffset.y << ", "
           << bufferOffset.z << "), footprint (" << copySize.width << ", " << copySize.height
           << ", " << copySize.depthOrArrayLayers << ")\n";
    }
    return os;
}

// Define base texture sizes and offsets to test with: some aligned, some unaligned
constexpr TextureSpec kBaseTextureSpecs[] = {
    // 1x1 2D copies
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {1_tc, 1_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {64_tc, 1_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {128_tc, 1_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {192_tc, 1_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {31_tc, 16_tc, 0_tc}, .copySize = {1_tc, 1_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {64_tc, 16_tc, 0_tc}, .copySize = {1_tc, 1_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {64_tc, 16_tc, 8_tc}, .copySize = {1_tc, 1_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    // 2x1, 1x2, and 2x2
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {64_tc, 2_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {64_tc, 1_tc, 2_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {64_tc, 2_tc, 2_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {128_tc, 2_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {128_tc, 1_tc, 2_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {128_tc, 2_tc, 2_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {192_tc, 2_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {192_tc, 1_tc, 2_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {192_tc, 2_tc, 2_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    // 1024x1024 2D and 3D
    {.origin = {0_tc, 0_tc, 0_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {256_tc, 512_tc, 0_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {64_tc, 48_tc, 0_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {64_tc, 48_tc, 16_tc},
     .copySize = {1024_tc, 1024_tc, 1024_tc},
     .blockInfo = {4, 1_tc, 1_tc}},
    // Non-power of two texture dims
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {257_tc, 31_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {17_tc, 93_tc, 1_tc}, .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {59_tc, 13_tc, 0_tc},
     .copySize = {257_tc, 31_tc, 1_tc},
     .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {17_tc, 73_tc, 0_tc},
     .copySize = {17_tc, 93_tc, 1_tc},
     .blockInfo = {4, 1_tc, 1_tc}},
    {.origin = {17_tc, 73_tc, 59_tc},
     .copySize = {17_tc, 93_tc, 99_tc},
     .blockInfo = {4, 1_tc, 1_tc}},
    // 4x4 block size 2D copies
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {4_tc, 4_tc, 1_tc}, .blockInfo = {8, 4_tc, 4_tc}},
    {.origin = {64_tc, 16_tc, 0_tc}, .copySize = {4_tc, 4_tc, 1_tc}, .blockInfo = {8, 4_tc, 4_tc}},
    {.origin = {64_tc, 16_tc, 8_tc}, .copySize = {4_tc, 4_tc, 1_tc}, .blockInfo = {8, 4_tc, 4_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {4_tc, 4_tc, 1_tc}, .blockInfo = {16, 4_tc, 4_tc}},
    {.origin = {64_tc, 16_tc, 0_tc}, .copySize = {4_tc, 4_tc, 1_tc}, .blockInfo = {16, 4_tc, 4_tc}},
    {.origin = {64_tc, 16_tc, 8_tc}, .copySize = {4_tc, 4_tc, 1_tc}, .blockInfo = {16, 4_tc, 4_tc}},
    // 4x4 block size 2D copies of 1024x1024 textures
    {.origin = {0_tc, 0_tc, 0_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {8, 4_tc, 4_tc}},
    {.origin = {256_tc, 512_tc, 0_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {8, 4_tc, 4_tc}},
    {.origin = {64_tc, 48_tc, 0_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {8, 4_tc, 4_tc}},
    {.origin = {64_tc, 48_tc, 16_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {8, 4_tc, 4_tc}},
    {.origin = {0_tc, 0_tc, 0_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {16, 4_tc, 4_tc}},
    {.origin = {256_tc, 512_tc, 0_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {16, 4_tc, 4_tc}},
    {.origin = {64_tc, 48_tc, 0_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {4, 16_tc, 4_tc}},
    {.origin = {64_tc, 48_tc, 16_tc},
     .copySize = {1024_tc, 1024_tc, 1_tc},
     .blockInfo = {16, 4_tc, 4_tc}},
    // 4x4 block size 3D copies
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {64_tc, 4_tc, 2_tc}, .blockInfo = {8, 4_tc, 4_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {64_tc, 4_tc, 2_tc}, .blockInfo = {16, 4_tc, 4_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {64_tc, 4_tc, 8_tc}, .blockInfo = {16, 4_tc, 4_tc}},
    {.origin = {0_tc, 0_tc, 0_tc}, .copySize = {128_tc, 4_tc, 8_tc}, .blockInfo = {8, 4_tc, 4_tc}},
};

// Define base buffer sizes to work with: some offsets aligned, some unaligned. bytesPerRow
// is the minimum required
std::array<BufferSpec, 15> BaseBufferSpecs(const TextureSpec& textureSpec) {
    const TypedTexelBlockInfo& blockInfo = textureSpec.blockInfo;
    uint32_t bytesPerRow =
        Align(blockInfo.ToBytes(blockInfo.ToBlockWidth(textureSpec.copySize.width)),
              kTextureBytesPerRowAlignment);

    auto alignNonPow2 = [](uint32_t value, uint32_t size) -> uint32_t {
        return value == 0 ? 0 : ((value - 1) / size + 1) * size;
    };

    BlockCount copyHeight = blockInfo.ToBlockHeight(textureSpec.copySize.height);

    return {
        BufferSpec{alignNonPow2(0, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(256, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(512, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(1024, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(1024, blockInfo.byteSize), bytesPerRow, copyHeight * 2_bc},

        BufferSpec{alignNonPow2(32, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(64, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(64, blockInfo.byteSize), bytesPerRow, copyHeight * 2_bc},

        BufferSpec{alignNonPow2(31, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(257, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(384, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(511, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(513, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(1023, blockInfo.byteSize), bytesPerRow, copyHeight},
        BufferSpec{alignNonPow2(1023, blockInfo.byteSize), bytesPerRow, copyHeight * 2_bc},
    };
}

// Define a list of values to set properties in the spec structs
constexpr uint32_t kCheckValues[] = {1,  2,  3,  4,   5,   6,   7,    8,     // small values
                                     16, 32, 64, 128, 256, 512, 1024, 2048,  // powers of 2
                                     15, 31, 63, 127, 257, 511, 1023, 2047,  // misalignments
                                     17, 33, 65, 129, 257, 513, 1025, 2049};

struct CopySplitTestParam {
    wgpu::TextureDimension dimension;
    bool relaxed;
};

class CopySplitTest : public testing::TestWithParam<CopySplitTestParam> {
  protected:
    void DoTest(const TextureSpec& textureSpec,
                const BufferSpec& bufferSpec,
                wgpu::TextureDimension dimension,
                bool relaxed) {
        const TypedTexelBlockInfo& blockInfo = textureSpec.blockInfo;
        DAWN_ASSERT(textureSpec.copySize.width % blockInfo.width == 0_tc &&
                    textureSpec.copySize.height % blockInfo.height == 0_tc);

        // Add trace so that failures emit the input test specs
        std::stringstream trace;
        trace << textureSpec << ", " << bufferSpec;
        SCOPED_TRACE(trace.str());

        // This code emulates RecordBufferTextureCopyWithBufferHandle

        BlockCount blocksPerRow = blockInfo.BytesToBlocks(bufferSpec.bytesPerRow);
        BlockCount rowsPerImage{bufferSpec.rowsPerImage};
        BlockOrigin3D origin = blockInfo.ToBlock(textureSpec.origin);
        BlockExtent3D copySize = blockInfo.ToBlock(textureSpec.copySize);

        switch (dimension) {
            case wgpu::TextureDimension::e1D: {
                // Skip test cases that are clearly for 2D/3D. Validation would catch
                // these cases before reaching the TextureCopySplitter.
                if (textureSpec.origin.z > 0_tc || textureSpec.copySize.depthOrArrayLayers > 1_tc) {
                    return;
                }
                TextureCopySubresource copySubresource = Compute2DTextureCopySubresource(
                    origin, copySize, blockInfo, bufferSpec.offset, blocksPerRow, relaxed);
                ValidateCopySplit(textureSpec, bufferSpec, copySubresource, dimension, relaxed);
                break;
            }
            case wgpu::TextureDimension::e2D: {
                if (relaxed) {
                    // Emulate Record2DBufferTextureCopyWithRelaxedOffsetAndPitch
                    TextureCopySubresource copySubresource = Compute2DTextureCopySubresource(
                        origin, copySize, blockInfo, bufferSpec.offset, blocksPerRow, relaxed);

                    for ([[maybe_unused]] BlockCount copyLayer :
                         Range(copySize.depthOrArrayLayers)) {
                        // Since Compute2DTextureCopySubresource returns a single copy subresource
                        // to be used for each layer, we need to update the texture spec to validate
                        // for a single layer.

                        // Unlike Record2DBufferTextureCopyWithRelaxedOffsetAndPitch, we don't need
                        // to compute a running buffer offset per layer, since ValidateFootprints
                        // assumes no extra offset on top of BufferSpec.offset.

                        // Modify texture spec for a single layer
                        auto textureSpecCopy = textureSpec;
                        textureSpecCopy.origin.z = 0_tc;
                        textureSpecCopy.copySize.depthOrArrayLayers = blockInfo.ToTexelDepth(1_bc);

                        ValidateCopySplit(textureSpecCopy, bufferSpec, copySubresource, dimension,
                                          relaxed);
                    }

                } else {
                    // Emulate Record2DBufferTextureCopyWithSplit
                    TextureCopySplits copySplits = Compute2DTextureCopySplits(
                        origin, copySize, blockInfo, bufferSpec.offset, blocksPerRow, rowsPerImage);
                    const uint64_t bytesPerLayer = blockInfo.ToBytes(blocksPerRow * rowsPerImage);
                    for (BlockCount copyLayer : Range(copySize.depthOrArrayLayers)) {
                        const uint32_t splitIndex =
                            static_cast<uint32_t>(copyLayer) % copySplits.copySubresources.size();
                        const TextureCopySubresource& copySubresourcePerLayer =
                            copySplits.copySubresources[splitIndex];

                        // Since Compute2DTextureCopySplits splits up the single copy into one or
                        // two alternating copies per layer, we need to modify the buffer and
                        // texture specs to validate for a single layer.

                        auto bufferSpecCopy = bufferSpec;
                        // Unlike Record2DBufferTextureCopyWithSplit, we don't need to compute a
                        // running buffer offset per layer. However, we do need to add an offset for
                        // the 2nd (4th, 6th, etc.) layers because the copy subresource values are
                        // computed assuming an offset of bytesPerLayer from the previous (1st)
                        // copy, and ValidateFootprints will check for that.
                        const uint64_t bufferOffsetForNextLayer =
                            bytesPerLayer * static_cast<uint32_t>(splitIndex);
                        bufferSpecCopy.offset += bufferOffsetForNextLayer;

                        // Modify texture spec for a single layer
                        auto textureSpecCopy = textureSpec;
                        textureSpecCopy.origin.z = 0_tc;
                        textureSpecCopy.copySize.depthOrArrayLayers = blockInfo.ToTexelDepth(1_bc);

                        ValidateCopySplit(textureSpecCopy, bufferSpecCopy, copySubresourcePerLayer,
                                          dimension, relaxed);
                    }
                }
                break;
            }
            case wgpu::TextureDimension::e3D: {
                TextureCopySubresource copySubresource =
                    Compute3DTextureCopySubresource(origin, copySize, blockInfo, bufferSpec.offset,
                                                    blocksPerRow, rowsPerImage, relaxed);
                ValidateCopySplit(textureSpec, bufferSpec, copySubresource, dimension, relaxed);
                break;
            }
            default:
                DAWN_UNREACHABLE();
                break;
        }
    }

    // Call from parameterized tests
    void DoTest(const TextureSpec& textureSpec, const BufferSpec& bufferSpec) {
        wgpu::TextureDimension dimension = GetParam().dimension;
        bool relaxed = GetParam().relaxed;
        DoTest(textureSpec, bufferSpec, dimension, relaxed);
    }

    void ValidateCopySplit(const TextureSpec& textureSpec,
                           const BufferSpec& bufferSpec,
                           const TextureCopySubresource& copySubresource,
                           wgpu::TextureDimension dimension,
                           bool relaxed) {
        ValidateFootprints(textureSpec, bufferSpec, copySubresource, dimension);
        ValidateOffset(copySubresource, relaxed);
        ValidateDisjoint(textureSpec, copySubresource);
        ValidateTextureBounds(textureSpec, copySubresource);
        ValidatePixelCount(textureSpec, copySubresource);
        ValidateBufferOffset(textureSpec, bufferSpec, copySubresource, dimension, relaxed);

        if (HasFatalFailure()) {
            std::ostringstream message;
            message << "Failed generating splits: " << textureSpec << ", " << bufferSpec << "\n"
                    << dimension << " " << copySubresource << "\n";
            FAIL() << message.str();
        }
    }
};

TEST_P(CopySplitTest, General) {
    for (const TextureSpec& textureSpec : kBaseTextureSpecs) {
        for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
            DoTest(textureSpec, bufferSpec);
        }
    }
}

TEST_P(CopySplitTest, TextureWidth) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t val : kCheckValues) {
            if (TexelCount{val} % textureSpec.blockInfo.width != 0_tc) {
                continue;
            }
            textureSpec.copySize.width = TexelCount{val};
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, TextureHeight) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t val : kCheckValues) {
            if (TexelCount{val} % textureSpec.blockInfo.height != 0_tc) {
                continue;
            }
            textureSpec.copySize.height = TexelCount{val};
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, TextureX) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t val : kCheckValues) {
            if (TexelCount{val} % textureSpec.blockInfo.width != 0_tc) {
                continue;
            }
            textureSpec.origin.x = TexelCount{val};
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, TextureY) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t val : kCheckValues) {
            if (TexelCount{val} % textureSpec.blockInfo.height != 0_tc) {
                continue;
            }
            textureSpec.origin.y = TexelCount{val};
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, TexelSize) {
    for (TextureSpec textureSpec : kBaseTextureSpecs) {
        for (uint32_t texelSize : {4, 8, 16, 32, 64}) {
            textureSpec.blockInfo.byteSize = texelSize;
            for (const BufferSpec& bufferSpec : BaseBufferSpecs(textureSpec)) {
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, BufferOffset) {
    for (const TextureSpec& textureSpec : kBaseTextureSpecs) {
        for (BufferSpec bufferSpec : BaseBufferSpecs(textureSpec)) {
            for (uint32_t val : kCheckValues) {
                bufferSpec.offset = textureSpec.blockInfo.byteSize * val;
                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, RowPitch) {
    for (const TextureSpec& textureSpec : kBaseTextureSpecs) {
        for (BufferSpec bufferSpec : BaseBufferSpecs(textureSpec)) {
            uint32_t baseRowPitch = bufferSpec.bytesPerRow;
            for (uint32_t i = 0; i < 5; ++i) {
                bufferSpec.bytesPerRow = baseRowPitch + i * 256;

                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

TEST_P(CopySplitTest, ImageHeight) {
    for (const TextureSpec& textureSpec : kBaseTextureSpecs) {
        for (BufferSpec bufferSpec : BaseBufferSpecs(textureSpec)) {
            BlockCount baseImageHeight = bufferSpec.rowsPerImage;
            for (uint32_t i = 0; i < 5; ++i) {
                bufferSpec.rowsPerImage = baseImageHeight + BlockCount{i * 256};

                DoTest(textureSpec, bufferSpec);
            }
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    ,
    CopySplitTest,
    testing::Values(CopySplitTestParam{.dimension = wgpu::TextureDimension::e1D, .relaxed = false},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e1D, .relaxed = true},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e2D, .relaxed = false},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e2D, .relaxed = true},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e3D, .relaxed = false},
                    CopySplitTestParam{.dimension = wgpu::TextureDimension::e3D, .relaxed = true}));

// Test for specific case that failed CTS for BCSliced3D formats (4x4 block) when the copy height
// is 1 block row, and we have a buffer offset that results in the copy region straddling
// bytesPerRow.
TEST_F(CopySplitTest, Block4x4_3D_CopyOneRow_StraddleBytesPerRowOffset) {
    constexpr TexelCount blockDim = 4_tc;
    constexpr uint32_t bytesPerBlock = 8;
    TextureSpec textureSpec = {.origin = {0_tc, 0_tc, 0_tc},
                               .copySize = {128_tc, 1_tc * blockDim, 8_tc},
                               .blockInfo = {bytesPerBlock, blockDim, blockDim}};
    BufferSpec bufferSpec = {.offset = 3592, .bytesPerRow = 256, .rowsPerImage = 1_bc};
    DoTest(textureSpec, bufferSpec, wgpu::TextureDimension::e3D, false);
}
// Test similar failure to above for 2x2 blocks
TEST_F(CopySplitTest, Block2x2_3D_CopyOneRow_StraddleBytesPerRowOffset) {
    constexpr TexelCount blockDim = 2_tc;
    constexpr uint32_t bytesPerBlock = 4;
    TextureSpec textureSpec = {.origin = {0_tc, 0_tc, 0_tc},
                               .copySize = {128_tc, 1_tc * blockDim, 8_tc},
                               .blockInfo = {bytesPerBlock, blockDim, blockDim}};
    BufferSpec bufferSpec = {.offset = 3592, .bytesPerRow = 256, .rowsPerImage = 1_bc};
    DoTest(textureSpec, bufferSpec, wgpu::TextureDimension::e3D, false);
}
// Also test 1x1, although this always passed as this one engages the "copySize.height is odd" path.
TEST_F(CopySplitTest, Block1x1_3D_CopyOneRow_StraddleBytesPerRowOffset) {
    constexpr TexelCount blockDim = 1_tc;
    constexpr uint32_t bytesPerBlock = 4;
    TextureSpec textureSpec = {.origin = {0_tc, 0_tc, 0_tc},
                               .copySize = {128_tc, 1_tc * blockDim, 8_tc},
                               .blockInfo = {bytesPerBlock, blockDim, blockDim}};
    BufferSpec bufferSpec = {.offset = 3592, .bytesPerRow = 256, .rowsPerImage = 1_bc};
    DoTest(textureSpec, bufferSpec, wgpu::TextureDimension::e3D, false);
}

TEST_F(CopySplitTest, Blah) {
    TextureSpec textureSpec = {.origin = {64_tc, 16_tc, 8_tc},
                               .copySize = {1_tc, 1_tc, 1_tc},
                               .blockInfo = {4, 1_tc, 1_tc}};
    BufferSpec bufferSpec = {.offset = 0, .bytesPerRow = 256, .rowsPerImage = 1_bc};
    DoTest(textureSpec, bufferSpec, wgpu::TextureDimension::e2D, false);
}
TEST_F(CopySplitTest, Blah2) {
    TextureSpec textureSpec = {.origin = {0_tc, 0_tc, 0_tc},
                               .copySize = {64_tc, 1_tc, 2_tc},
                               .blockInfo = {4, 1_tc, 1_tc}};
    BufferSpec bufferSpec = {.offset = 0, .bytesPerRow = 256, .rowsPerImage = 1_bc};
    DoTest(textureSpec, bufferSpec, wgpu::TextureDimension::e2D, false);
}
TEST_F(CopySplitTest, Blah3) {
    TextureSpec textureSpec = {.origin = {64_tc, 48_tc, 16_tc},
                               .copySize = {1024_tc, 1024_tc, 1024_tc},
                               .blockInfo = {4, 1_tc, 1_tc}};
    BufferSpec bufferSpec = {.offset = 0, .bytesPerRow = 4096, .rowsPerImage = 1024_bc};
    DoTest(textureSpec, bufferSpec, wgpu::TextureDimension::e2D, false);
}

}  // anonymous namespace
}  // namespace dawn::native::d3d12
