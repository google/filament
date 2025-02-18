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

#ifndef SRC_DAWN_NATIVE_D3D12_TEXTURECOPYSPLITTER_H_
#define SRC_DAWN_NATIVE_D3D12_TEXTURECOPYSPLITTER_H_

#include <array>

#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

struct TexelBlockInfo;

}  // namespace dawn::native

namespace dawn::native::d3d12 {

struct TextureCopySubresource {
    static constexpr unsigned int kMaxTextureCopyRegions = 4;

    struct CopyInfo {
        Origin3D destinationOffset;

        D3D12_BOX sourceRegion;
        D3D12_TEXTURE_COPY_LOCATION bufferLocation;

        Origin3D GetBufferOffset(BufferTextureCopyDirection direction) const;
        Origin3D GetTextureOffset(BufferTextureCopyDirection direction) const;
        Extent3D GetCopySize() const;
        uint64_t GetAlignedOffset() const;
        Extent3D GetBufferSize() const;

        void SetAlignedOffset(uint64_t alignedOffset);
        void SetHeightInBufferLocation(uint32_t bufferHeight);
        void SetDepthInBufferLocation(uint32_t bufferDepth);
    };

    CopyInfo* AddCopy();

    uint32_t count = 0;
    std::array<CopyInfo, kMaxTextureCopyRegions> copies;
};

struct TextureCopySplits {
    static constexpr uint32_t kMaxTextureCopySubresources = 2;

    std::array<TextureCopySubresource, kMaxTextureCopySubresources> copySubresources;
};

// This function is shared by 2D and 3D texture copy splitter. But it only knows how to handle
// 2D non-arrayed textures correctly, and just forwards "copySize.depthOrArrayLayers". See
// details in Compute{2D|3D}TextureCopySplits about how we generate copy regions for 2D array
// and 3D textures based on this function.
// The resulting copies triggered by API like CopyTextureRegion are equivalent to the copy
// regions defines by the arguments of TextureCopySubresource returned by this function and its
// counterparts. These arguments should strictly conform to particular invariants. Otherwise,
// D3D12 driver may report validation errors when we call CopyTextureRegion. Some important
// invariants are listed below. For more details
// of these invariants, see src/dawn/tests/unittests/d3d12/CopySplitTests.cpp.
//   - Inside each copy region: 1) its buffer offset plus copy size should be less than its
//     buffer size, 2) its buffer offset on y-axis should be less than copy format's
//     blockInfo.height, 3) its buffer offset on z-axis should be 0.
//   - Each copy region has an offset (aka alignedOffset) aligned to
//     D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT
//   - The buffer footprint of each copy region should be entirely within the copied buffer,
//     which means that the last "texel" of the buffer footprint doesn't go past the end of
//     the buffer even though the last "texel" might not be copied.
//   - If there are multiple copy regions, each copy region should not overlap with the others.
//   - Copy region(s) combined should exactly be equivalent to the texture region to be copied.
//   - Every pixel accessed by every copy region should not be out of the bound of the copied
//     texture and buffer.
TextureCopySubresource Compute2DTextureCopySubresource(BufferTextureCopyDirection direction,
                                                       Origin3D origin,
                                                       Extent3D copySize,
                                                       const TexelBlockInfo& blockInfo,
                                                       uint64_t offset,
                                                       uint32_t bytesPerRow);

TextureCopySplits Compute2DTextureCopySplits(BufferTextureCopyDirection direction,
                                             Origin3D origin,
                                             Extent3D copySize,
                                             const TexelBlockInfo& blockInfo,
                                             uint64_t offset,
                                             uint32_t bytesPerRow,
                                             uint32_t rowsPerImage);

TextureCopySubresource Compute3DTextureCopySplits(BufferTextureCopyDirection direction,
                                                  Origin3D origin,
                                                  Extent3D copySize,
                                                  const TexelBlockInfo& blockInfo,
                                                  uint64_t offset,
                                                  uint32_t bytesPerRow,
                                                  uint32_t rowsPerImage);

// Compute the `TextureCopySubresource` for one subresource of a 2D texture with relaxed row pitch
// and offset.
TextureCopySubresource Compute2DTextureCopySubresourceWithRelaxedRowPitchAndOffset(
    BufferTextureCopyDirection direction,
    Origin3D origin,
    Extent3D copySize,
    const TexelBlockInfo& blockInfo,
    uint64_t offset,
    uint32_t bytesPerRow);

// Compute the `TextureCopySubresource` for one subresource of a 3D texture with relaxed row pitch
// and offset.
TextureCopySubresource Compute3DTextureCopySubresourceWithRelaxedRowPitchAndOffset(
    BufferTextureCopyDirection direction,
    Origin3D origin,
    Extent3D copySize,
    const TexelBlockInfo& blockInfo,
    uint64_t offset,
    uint32_t bytesPerRow,
    uint32_t rowsPerImage);

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_TEXTURECOPYSPLITTER_H_
