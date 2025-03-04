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

#include "dawn/native/d3d12/TextureCopySplitter.h"

#include "dawn/common/Assert.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

namespace {
Origin3D ComputeTexelOffsets(const TexelBlockInfo& blockInfo,
                             uint32_t offset,
                             uint32_t bytesPerRow) {
    DAWN_ASSERT(bytesPerRow != 0);
    uint32_t byteOffsetX = offset % bytesPerRow;
    uint32_t byteOffsetY = offset - byteOffsetX;

    return {byteOffsetX / blockInfo.byteSize * blockInfo.width,
            byteOffsetY / bytesPerRow * blockInfo.height, 0};
}

uint64_t OffsetToFirstCopiedTexel(const TexelBlockInfo& blockInfo,
                                  uint32_t bytesPerRow,
                                  uint64_t alignedOffset,
                                  Origin3D bufferOffset) {
    DAWN_ASSERT(bufferOffset.z == 0);
    return alignedOffset + bufferOffset.x * blockInfo.byteSize / blockInfo.width +
           bufferOffset.y * bytesPerRow / blockInfo.height;
}

uint64_t AlignDownForDataPlacement(uint32_t offset) {
    return offset & ~static_cast<uint64_t>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1);
}

void ComputeSourceRegionForCopyInfo(TextureCopySubresource::CopyInfo* copyInfo,
                                    BufferTextureCopyDirection direction,
                                    Origin3D bufferOffset,
                                    Origin3D textureOffset,
                                    Extent3D copySize) {
    switch (direction) {
        case BufferTextureCopyDirection::B2T:
            copyInfo->sourceRegion = ComputeD3D12BoxFromOffsetAndSize(bufferOffset, copySize);
            copyInfo->destinationOffset = textureOffset;
            break;
        case BufferTextureCopyDirection::T2B:
            copyInfo->sourceRegion = ComputeD3D12BoxFromOffsetAndSize(textureOffset, copySize);
            copyInfo->destinationOffset = bufferOffset;
            break;
        default:
            DAWN_UNREACHABLE();
    }
}

void FillFootprintAndOffsetOfBufferLocation(D3D12_TEXTURE_COPY_LOCATION* bufferLocation,
                                            uint64_t alignedOffset,
                                            Extent3D bufferSize,
                                            uint32_t bytesPerRow) {
    bufferLocation->Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    bufferLocation->pResource = nullptr;
    bufferLocation->PlacedFootprint.Offset = alignedOffset;
    bufferLocation->PlacedFootprint.Footprint.Width = bufferSize.width;
    bufferLocation->PlacedFootprint.Footprint.Height = bufferSize.height;
    bufferLocation->PlacedFootprint.Footprint.Depth = bufferSize.depthOrArrayLayers;
    bufferLocation->PlacedFootprint.Footprint.RowPitch = bytesPerRow;
    bufferLocation->PlacedFootprint.Footprint.Format = DXGI_FORMAT_UNKNOWN;
}

}  // namespace

Extent3D TextureCopySubresource::CopyInfo::GetCopySize() const {
    return {sourceRegion.right - sourceRegion.left, sourceRegion.bottom - sourceRegion.top,
            sourceRegion.back - sourceRegion.front};
}

Origin3D TextureCopySubresource::CopyInfo::GetBufferOffset(
    BufferTextureCopyDirection direction) const {
    switch (direction) {
        case BufferTextureCopyDirection::B2T:
            return {sourceRegion.left, sourceRegion.top, sourceRegion.front};
        case BufferTextureCopyDirection::T2B:
            return destinationOffset;
        default:
            DAWN_UNREACHABLE();
    }
}

Origin3D TextureCopySubresource::CopyInfo::GetTextureOffset(
    BufferTextureCopyDirection direction) const {
    switch (direction) {
        case BufferTextureCopyDirection::B2T:
            return destinationOffset;
        case BufferTextureCopyDirection::T2B:
            return {sourceRegion.left, sourceRegion.top, sourceRegion.front};
        default:
            DAWN_UNREACHABLE();
    }
}

uint64_t TextureCopySubresource::CopyInfo::GetAlignedOffset() const {
    return bufferLocation.PlacedFootprint.Offset;
}

Extent3D TextureCopySubresource::CopyInfo::GetBufferSize() const {
    return {bufferLocation.PlacedFootprint.Footprint.Width,
            bufferLocation.PlacedFootprint.Footprint.Height,
            bufferLocation.PlacedFootprint.Footprint.Depth};
}

void TextureCopySubresource::CopyInfo::SetAlignedOffset(uint64_t alignedOffset) {
    bufferLocation.PlacedFootprint.Offset = alignedOffset;
}

void TextureCopySubresource::CopyInfo::SetHeightInBufferLocation(uint32_t bufferHeight) {
    bufferLocation.PlacedFootprint.Footprint.Height = bufferHeight;
}

void TextureCopySubresource::CopyInfo::SetDepthInBufferLocation(uint32_t bufferDepth) {
    bufferLocation.PlacedFootprint.Footprint.Depth = bufferDepth;
}

TextureCopySubresource::CopyInfo* TextureCopySubresource::AddCopy() {
    DAWN_ASSERT(this->count < kMaxTextureCopyRegions);
    return &this->copies[this->count++];
}

void FillTextureCopySubresourceCopyInfo(TextureCopySubresource::CopyInfo* copyInfo,
                                        BufferTextureCopyDirection direction,
                                        Origin3D bufferOffset,
                                        Origin3D textureOffset,
                                        Extent3D copySize,
                                        Extent3D bufferSize,
                                        uint64_t alignedOffset,
                                        uint32_t bytesPerRow) {
    ComputeSourceRegionForCopyInfo(copyInfo, direction, bufferOffset, textureOffset, copySize);
    FillFootprintAndOffsetOfBufferLocation(&copyInfo->bufferLocation, alignedOffset, bufferSize,
                                           bytesPerRow);
}

TextureCopySubresource Compute2DTextureCopySubresource(BufferTextureCopyDirection direction,
                                                       Origin3D origin,
                                                       Extent3D copySize,
                                                       const TexelBlockInfo& blockInfo,
                                                       uint64_t offset,
                                                       uint32_t bytesPerRow) {
    TextureCopySubresource copy;

    DAWN_ASSERT(bytesPerRow % blockInfo.byteSize == 0);

    // The copies must be 512-aligned. To do this, we calculate the first 512-aligned address
    // preceding our data.
    uint64_t alignedOffset = AlignDownForDataPlacement(offset);

    // If the provided offset to the data was already 512-aligned, we can simply copy the data
    // without further translation.
    if (offset == alignedOffset) {
        TextureCopySubresource::CopyInfo* copyInfo = copy.AddCopy();

        Origin3D textureOffset = origin;
        Origin3D bufferOffset = {0, 0, 0};

        Extent3D bufferSize = copySize;

        FillTextureCopySubresourceCopyInfo(copyInfo, direction, bufferOffset, textureOffset,
                                           copySize, bufferSize, alignedOffset, bytesPerRow);
        return copy;
    }

    DAWN_ASSERT(alignedOffset < offset);
    DAWN_ASSERT(offset - alignedOffset < D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

    // We must reinterpret our aligned offset into X and Y offsets with respect to the row
    // pitch.
    //
    // You can visualize the data in the buffer like this:
    // |-----------------------++++++++++++++++++++++++++++++++|
    // ^ 512-aligned address   ^ Aligned offset               ^ End of copy data
    //
    // Now when you consider the row pitch, you can visualize the data like this:
    // |~~~~~~~~~~~~~~~~|
    // |~~~~~+++++++++++|
    // |++++++++++++++++|
    // |+++++~~~~~~~~~~~|
    // |<---row pitch-->|
    //
    // The X and Y offsets calculated in ComputeTexelOffsets can be visualized like this:
    // |YYYYYYYYYYYYYYYY|
    // |XXXXXX++++++++++|
    // |++++++++++++++++|
    // |++++++~~~~~~~~~~|
    // |<---row pitch-->|
    Origin3D texelOffset =
        ComputeTexelOffsets(blockInfo, static_cast<uint32_t>(offset - alignedOffset), bytesPerRow);

    DAWN_ASSERT(texelOffset.y <= blockInfo.height);
    DAWN_ASSERT(texelOffset.z == 0);

    uint32_t copyBytesPerRowPitch = copySize.width / blockInfo.width * blockInfo.byteSize;
    uint32_t byteOffsetInRowPitch = texelOffset.x / blockInfo.width * blockInfo.byteSize;
    if (copyBytesPerRowPitch + byteOffsetInRowPitch <= bytesPerRow) {
        // The region's rows fit inside the bytes per row. In this case, extend the width of the
        // PlacedFootprint and copy the buffer with an offset location
        //  |<------------- bytes per row ------------->|
        //
        //  |-------------------------------------------|
        //  |                                           |
        //  |                 +++++++++++++++++~~~~~~~~~|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++~~~~~~~~~|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++~~~~~~~~~|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++~~~~~~~~~|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++         |
        //  |-------------------------------------------|

        // Copy 0:
        //  |----------------------------------|
        //  |                                  |
        //  |                 +++++++++++++++++|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++|
        //  |~~~~~~~~~~~~~~~~~+++++++++++++++++|
        //  |----------------------------------|

        TextureCopySubresource::CopyInfo* copyInfo = copy.AddCopy();

        Origin3D textureOffset = origin;
        Origin3D bufferOffset = texelOffset;

        Extent3D bufferSize = {copySize.width + texelOffset.x, copySize.height + texelOffset.y,
                               copySize.depthOrArrayLayers};

        FillTextureCopySubresourceCopyInfo(copyInfo, direction, bufferOffset, textureOffset,
                                           copySize, bufferSize, alignedOffset, bytesPerRow);

        return copy;
    }

    // The region's rows straddle the bytes per row. Split the copy into two copies
    //  |<------------- bytes per row ------------->|
    //
    //  |-------------------------------------------|
    //  |                                           |
    //  |                                   ++++++++|
    //  |+++++++++~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |+++++++++~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |+++++++++~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |+++++++++~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |+++++++++                                  |
    //  |-------------------------------------------|

    //  Copy 0:
    //  |-------------------------------------------|
    //  |                                           |
    //  |                                   ++++++++|
    //  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~++++++++|
    //  |-------------------------------------------|

    //  Copy 1:
    //  |---------|
    //  |         |
    //  |         |
    //  |+++++++++|
    //  |+++++++++|
    //  |+++++++++|
    //  |+++++++++|
    //  |+++++++++|
    //  |---------|

    // Copy 0
    Origin3D textureOffset0 = origin;

    DAWN_ASSERT(bytesPerRow > byteOffsetInRowPitch);
    uint32_t texelsPerRow = bytesPerRow / blockInfo.byteSize * blockInfo.width;
    Extent3D copySize0 = {texelsPerRow - texelOffset.x, copySize.height,
                          copySize.depthOrArrayLayers};

    Origin3D bufferOffset0 = texelOffset;
    Extent3D bufferSize0 = {texelsPerRow, copySize.height + texelOffset.y,
                            copySize.depthOrArrayLayers};

    TextureCopySubresource::CopyInfo* copyInfo0 = copy.AddCopy();
    FillTextureCopySubresourceCopyInfo(copyInfo0, direction, bufferOffset0, textureOffset0,
                                       copySize0, bufferSize0, alignedOffset, bytesPerRow);

    // Copy 1
    uint64_t offsetForCopy1 = offset + copySize0.width / blockInfo.width * blockInfo.byteSize;
    uint64_t alignedOffsetForCopy1 = AlignDownForDataPlacement(offsetForCopy1);
    Origin3D texelOffsetForCopy1 = ComputeTexelOffsets(
        blockInfo, static_cast<uint32_t>(offsetForCopy1 - alignedOffsetForCopy1), bytesPerRow);

    DAWN_ASSERT(texelOffsetForCopy1.y <= blockInfo.height);
    DAWN_ASSERT(texelOffsetForCopy1.z == 0);

    Origin3D textureOffset1 = {origin.x + copySize0.width, origin.y, origin.z};

    DAWN_ASSERT(copySize.width > copySize0.width);
    Extent3D copySize1 = {copySize.width - copySize0.width, copySize.height,
                          copySize.depthOrArrayLayers};

    Origin3D bufferOffset1 = texelOffsetForCopy1;
    Extent3D bufferSize1 = {copySize1.width + texelOffsetForCopy1.x,
                            copySize.height + texelOffsetForCopy1.y, copySize.depthOrArrayLayers};

    TextureCopySubresource::CopyInfo* copyInfo1 = copy.AddCopy();
    FillTextureCopySubresourceCopyInfo(copyInfo1, direction, bufferOffset1, textureOffset1,
                                       copySize1, bufferSize1, alignedOffsetForCopy1, bytesPerRow);

    return copy;
}

TextureCopySplits Compute2DTextureCopySplits(BufferTextureCopyDirection direction,
                                             Origin3D origin,
                                             Extent3D copySize,
                                             const TexelBlockInfo& blockInfo,
                                             uint64_t offset,
                                             uint32_t bytesPerRow,
                                             uint32_t rowsPerImage) {
    TextureCopySplits copies;

    const uint64_t bytesPerLayer = bytesPerRow * rowsPerImage;

    // The function Compute2DTextureCopySubresource() decides how to split the copy based on:
    // - the alignment of the buffer offset with D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512)
    // - the alignment of the buffer offset with D3D12_TEXTURE_DATA_PITCH_ALIGNMENT (256)
    // Each layer of a 2D array might need to be split, but because of the WebGPU
    // constraint that "bytesPerRow" must be a multiple of 256, all odd (resp. all even) layers
    // will be at an offset multiple of 512 of each other, which means they will all result in
    // the same 2D split. Thus we can just compute the copy splits for the first and second
    // layers, and reuse them for the remaining layers by adding the related offset of each
    // layer. Moreover, if "rowsPerImage" is even, both the first and second copy layers can
    // share the same copy split, so in this situation we just need to compute copy split once
    // and reuse it for all the layers.
    Extent3D copyOneLayerSize = copySize;
    Origin3D copyFirstLayerOrigin = origin;
    copyOneLayerSize.depthOrArrayLayers = 1;
    copyFirstLayerOrigin.z = 0;

    copies.copySubresources[0] = Compute2DTextureCopySubresource(
        direction, copyFirstLayerOrigin, copyOneLayerSize, blockInfo, offset, bytesPerRow);

    // When the copy only refers one texture 2D array layer,
    // copies.copySubresources[1] will never be used so we can safely early return here.
    if (copySize.depthOrArrayLayers == 1) {
        return copies;
    }

    if (bytesPerLayer % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT == 0) {
        copies.copySubresources[1] = copies.copySubresources[0];
        uint64_t alignedOffset0 =
            copies.copySubresources[1].copies[0].GetAlignedOffset() + bytesPerLayer;
        uint64_t alignedOffset1 =
            copies.copySubresources[1].copies[1].GetAlignedOffset() + bytesPerLayer;
        copies.copySubresources[1].copies[0].SetAlignedOffset(alignedOffset0);
        copies.copySubresources[1].copies[1].SetAlignedOffset(alignedOffset1);
    } else {
        const uint64_t bufferOffsetNextLayer = offset + bytesPerLayer;
        copies.copySubresources[1] =
            Compute2DTextureCopySubresource(direction, copyFirstLayerOrigin, copyOneLayerSize,
                                            blockInfo, bufferOffsetNextLayer, bytesPerRow);
    }

    return copies;
}

void Recompute3DTextureCopyRegionWithEmptyFirstRowAndEvenCopyHeight(
    BufferTextureCopyDirection direction,
    Origin3D origin,
    Extent3D copySize,
    const TexelBlockInfo& blockInfo,
    uint32_t bytesPerRow,
    uint32_t rowsPerImage,
    TextureCopySubresource& copy,
    uint32_t i) {
    // Let's assign data and show why copy region generated by ComputeTextureCopySubresource
    // is incorrect if there is an empty row at the beginning of the copy block.
    // Assuming that bytesPerRow is 256 and we are doing a B2T copy, and copy size is {width: 2,
    // height: 4, depthOrArrayLayers: 3}. Then the data layout in buffer is demonstrated
    // as below:
    //
    //               |<----- bytes per row ------>|
    //
    //               |----------------------------|
    //  row (N - 1)  |                            |
    //  row N        |                 ++~~~~~~~~~|
    //  row (N + 1)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 2)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 3)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 4)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 5)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 6)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 7)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 8)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 9)  |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 10) |~~~~~~~~~~~~~~~~~++~~~~~~~~~|
    //  row (N + 11) |~~~~~~~~~~~~~~~~~++         |
    //               |----------------------------|

    // The copy we mean to do is the following:
    //
    //   - image 0: row N to row (N + 3),
    //   - image 1: row (N + 4) to row (N + 7),
    //   - image 2: row (N + 8) to row (N + 11).
    //
    // Note that alignedOffset is at the beginning of row (N - 1), while buffer offset makes
    // the copy start at row N. Row (N - 1) is the empty row between alignedOffset and offset.
    //
    // The 2D copy region of image 0 we received from Compute2DTextureCopySubresource() is
    // the following:
    //
    //              |-------------------|
    //  row (N - 1) |                   |
    //  row N       |                 ++|
    //  row (N + 1) |~~~~~~~~~~~~~~~~~++|
    //  row (N + 2) |~~~~~~~~~~~~~~~~~++|
    //  row (N + 3) |~~~~~~~~~~~~~~~~~++|
    //              |-------------------|
    //
    // However, if we simply expand the copy region of image 0 to all depth ranges of a 3D
    // texture, we will copy 5 rows every time, and every first row of each slice will be
    // skipped. As a result, the copied data will be:
    //
    //   - image 0: row N to row (N + 3), which is correct. Row (N - 1) is skipped.
    //   - image 1: row (N + 5) to row (N + 8) because row (N + 4) is skipped. It is incorrect.
    //
    // Likewise, all other image followed will be incorrect because we wrongly keep skipping
    // one row for each depth slice.
    //
    // Solution: split the copy region to two copies: copy 3 (rowsPerImage - 1) rows in and
    // expand to all depth slices in the first copy. 3 rows + one skipped rows = 4 rows, which
    // equals to rowsPerImage. Then copy the last row in the second copy. However, the copy
    // block of the last row of the last image may out-of-bound (see the details below), so
    // we need an extra copy for the very last row.

    // Copy 0: copy 3 rows, not 4 rows.
    //                _____________________
    //               /                    /|
    //              /                    / |
    //              |-------------------|  |
    //  row (N - 1) |                   |  |
    //  row N       |                 ++|  |
    //  row (N + 1) |~~~~~~~~~~~~~~~~~++| /
    //  row (N + 2) |~~~~~~~~~~~~~~~~~++|/
    //              |-------------------|

    // Copy 1: move down two rows and copy the last row on image 0, and expand to
    // copySize.depthOrArrayLayers - 1 depth slices. Note that if we expand it to all depth
    // slices, the last copy block will be row (N + 9) to row (N + 12). Row (N + 11) might
    // be the last row of the entire buffer. Then row (N + 12) will be out-of-bound.
    //                _____________________
    //               /                    /|
    //              /                    / |
    //              |-------------------|  |
    //  row (N + 1) |                   |  |
    //  row (N + 2) |                   |  |
    //  row (N + 3) |                 ++| /
    //  row (N + 4) |~~~~~~~~~~~~~~~~~~~|/
    //              |-------------------|
    //
    //  copy 2: copy the last row of the last image.
    //              |-------------------|
    //  row (N + 11)|                 ++|
    //              |-------------------|

    // Copy 0: copy copySize0.height - 1 rows
    TextureCopySubresource::CopyInfo& copy0 = copy.copies[i];
    Extent3D copySize0 = copy0.GetCopySize();
    copySize0.height = copySize.height - blockInfo.height;
    uint32_t bufferHeight0 = rowsPerImage * blockInfo.height;  // rowsPerImageInTexels
    copy0.SetHeightInBufferLocation(bufferHeight0);
    const Origin3D bufferOffset0 = copy0.GetBufferOffset(direction);
    const Origin3D textureOffset0 = copy0.GetTextureOffset(direction);
    ComputeSourceRegionForCopyInfo(&copy0, direction, bufferOffset0, textureOffset0, copySize0);

    // Copy 1: move down 2 rows and copy the last row on image 0, and expand to all depth slices
    // but the last one.
    TextureCopySubresource::CopyInfo* copy1 = copy.AddCopy();
    *copy1 = copy0;
    uint64_t alignedOffset1 = copy1->GetAlignedOffset() + 2 * bytesPerRow;
    copy1->SetAlignedOffset(alignedOffset1);
    Origin3D textureOffset1 = textureOffset0;
    Origin3D bufferOffset1 = bufferOffset0;
    textureOffset1.y += copySize.height - blockInfo.height;
    // Offset two rows from the copy height for bufferOffset1 (See the figure above):
    //   - one for the row we advanced in the buffer: row (N + 4).
    //   - one for the last row we want to copy: row (N + 3) itself.
    bufferOffset1.y = copySize.height - 2 * blockInfo.height;
    Extent3D copySize1 = copySize0;
    copySize1.height = blockInfo.height;
    copySize1.depthOrArrayLayers--;
    uint32_t bufferDepth1 = copy1->GetBufferSize().depthOrArrayLayers;
    bufferDepth1--;
    copy1->SetDepthInBufferLocation(bufferDepth1);
    ComputeSourceRegionForCopyInfo(copy1, direction, bufferOffset1, textureOffset1, copySize1);

    // Copy 2: copy the last row of the last image.
    uint64_t offsetForCopy0 =
        OffsetToFirstCopiedTexel(blockInfo, bytesPerRow, copy0.GetAlignedOffset(), bufferOffset0);
    uint64_t offsetForLastRowOfLastImage =
        offsetForCopy0 +
        bytesPerRow * (copySize0.height + rowsPerImage * (copySize.depthOrArrayLayers - 1));
    uint64_t alignedOffsetForLastRowOfLastImage =
        AlignDownForDataPlacement(offsetForLastRowOfLastImage);
    Origin3D texelOffsetForLastRowOfLastImage = ComputeTexelOffsets(
        blockInfo,
        static_cast<uint32_t>(offsetForLastRowOfLastImage - alignedOffsetForLastRowOfLastImage),
        bytesPerRow);

    TextureCopySubresource::CopyInfo* copy2 = copy.AddCopy();
    uint64_t alignedOffset2 = alignedOffsetForLastRowOfLastImage;
    Origin3D textureOffset2 = textureOffset1;
    textureOffset2.z = origin.z + copySize.depthOrArrayLayers - 1;
    Extent3D copySize2 = copySize1;
    copySize2.depthOrArrayLayers = 1;
    Origin3D bufferOffset2 = bufferOffset1;
    bufferOffset2 = texelOffsetForLastRowOfLastImage;
    DAWN_ASSERT(copySize2.height == 1);
    ComputeSourceRegionForCopyInfo(copy2, direction, bufferOffset2, textureOffset2, copySize2);
    Extent3D bufferSize2 = {copy1->GetBufferSize().width, bufferOffset2.y + copySize2.height, 1};
    FillFootprintAndOffsetOfBufferLocation(&copy2->bufferLocation, alignedOffset2, bufferSize2,
                                           bytesPerRow);
}

void Recompute3DTextureCopyRegionWithEmptyFirstRowAndOddCopyHeight(
    BufferTextureCopyDirection direction,
    Extent3D copySize,
    uint32_t bytesPerRow,
    TextureCopySubresource& copy,
    uint32_t i) {
    // Read the comments of Recompute3DTextureCopyRegionWithEmptyFirstRowAndEvenCopyHeight() for
    // the reason why it is incorrect if we simply extend the copy region to all depth slices
    // when there is an empty first row at the copy region.
    //
    // If the copy height is odd, we can use two copies to make it correct:
    //   - copy 0: only copy the first depth slice. Keep other arguments the same.
    //   - copy 1: copy all rest depth slices because it will start without an empty row if
    //     copy height is odd. Odd height + one (empty row) is even. An even row number times
    //     bytesPerRow (256) will be aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512)

    // Copy 0: copy the first depth slice (image 0)
    TextureCopySubresource::CopyInfo& copy0 = copy.copies[i];
    Extent3D copySize0 = copy0.GetCopySize();
    copySize0.depthOrArrayLayers = 1;
    const uint32_t kBufferDepth0 = 1u;
    copy0.SetDepthInBufferLocation(kBufferDepth0);
    const Origin3D bufferOffset0 = copy0.GetBufferOffset(direction);
    const Origin3D textureOffset0 = copy0.GetTextureOffset(direction);
    ComputeSourceRegionForCopyInfo(&copy0, direction, bufferOffset0, textureOffset0, copySize0);

    // Copy 1: copy the rest depth slices in one shot
    TextureCopySubresource::CopyInfo* copy1 = copy.AddCopy();
    *copy1 = copy0;
    DAWN_ASSERT(copySize.height % 2 == 1);
    uint64_t alignedOffset1 = copy0.GetAlignedOffset() + (copySize.height + 1) * bytesPerRow;
    DAWN_ASSERT(alignedOffset1 % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT == 0);
    copy1->SetAlignedOffset(alignedOffset1);
    // textureOffset1.z should add one because the first slice has already been copied in copy0.
    Origin3D textureOffset1 = textureOffset0;
    textureOffset1.z++;
    // bufferOffset1.y should be 0 because we skipped the first depth slice and there is no empty
    // row in this copy region.
    Origin3D bufferOffset1 = bufferOffset0;
    bufferOffset1.y = 0;
    Extent3D copySize1 = copySize0;
    copySize1.height = copySize.height;
    copySize1.depthOrArrayLayers = copySize.depthOrArrayLayers - 1;
    copy1->SetHeightInBufferLocation(copySize.height);
    copy1->SetDepthInBufferLocation(copySize.depthOrArrayLayers - 1);
    ComputeSourceRegionForCopyInfo(copy1, direction, bufferOffset1, textureOffset1, copySize1);
}

TextureCopySubresource Compute3DTextureCopySplits(BufferTextureCopyDirection direction,
                                                  Origin3D origin,
                                                  Extent3D copySize,
                                                  const TexelBlockInfo& blockInfo,
                                                  uint64_t offset,
                                                  uint32_t bytesPerRow,
                                                  uint32_t rowsPerImage) {
    // To compute the copy region(s) for 3D textures, we call Compute2DTextureCopySubresource
    // and get copy region(s) for the first slice of the copy, then extend to all depth slices
    // and become a 3D copy. However, this doesn't work as easily as that due to some corner
    // cases.
    //
    // For example, if bufferSize.height is greater than rowsPerImage in the generated copy
    // region and we simply extend the 2D copy region to all copied depth slices, copied data
    // will be incorrectly offset for each depth slice except the first one.
    //
    // For these special cases, we need to recompute the copy regions for 3D textures via
    // split the incorrect copy region to a couple more copy regions.

    // Call Compute2DTextureCopySubresource and get copy regions. This function has already
    // forwarded "copySize.depthOrArrayLayers" to all depth slices.
    TextureCopySubresource copySubresource = Compute2DTextureCopySubresource(
        direction, origin, copySize, blockInfo, offset, bytesPerRow);

    DAWN_ASSERT(copySubresource.count <= 2);
    // If copySize.depthOrArrayLayers is 1, we can return copySubresource. Because we don't need to
    // extend the copy region(s) to other depth slice(s).
    if (copySize.depthOrArrayLayers == 1) {
        return copySubresource;
    }

    uint32_t rowsPerImageInTexels = rowsPerImage * blockInfo.height;
    // The copy region(s) generated by Compute2DTextureCopySubresource might be incorrect.
    // However, we may append a couple more copy regions in the for loop below. We don't need
    // to revise these new added copy regions.
    uint32_t originalCopyCount = copySubresource.count;
    for (uint32_t i = 0; i < originalCopyCount; ++i) {
        // There can be one empty row at most in a copy region.
        uint32_t bufferHeight = copySubresource.copies[i].GetBufferSize().height;
        DAWN_ASSERT(bufferHeight <= rowsPerImageInTexels + blockInfo.height);
        DAWN_ASSERT(bufferHeight <= rowsPerImageInTexels + blockInfo.height);

        if (bufferHeight == rowsPerImageInTexels) {
            // If the copy region's bufferHeight equals to rowsPerImageInTexels, we can use this
            // copy region without any modification.
            continue;
        }

        if (bufferHeight < rowsPerImageInTexels) {
            // If we are copying multiple depth slices, we should skip rowsPerImageInTexels rows for
            // each slice even though we only copy partial rows in each slice sometimes.
            copySubresource.copies[i].SetHeightInBufferLocation(rowsPerImageInTexels);
        } else {
            // bufferHeight > rowsPerImageInTexels. There is an empty row in this copy region due to
            // alignment adjustment.

            // bytesPerRow is definitely 256, and it is definitely a full copy on height.
            // Otherwise, bufferHeight won't be greater than rowsPerImageInTexels and there won't be
            // an empty row at the beginning of this copy region.
            DAWN_ASSERT(bytesPerRow == D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
            DAWN_ASSERT(copySize.height == rowsPerImageInTexels);

            if (copySize.height % 2 == 0) {
                // If copySize.height is even and there is an empty row at the beginning of the
                // first slice of the copy region, the offset of all depth slices will never be
                // aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512) and there is always
                // an empty row at each depth slice. We need a totally different approach to
                // split the copy region.
                Recompute3DTextureCopyRegionWithEmptyFirstRowAndEvenCopyHeight(
                    direction, origin, copySize, blockInfo, bytesPerRow, rowsPerImage,
                    copySubresource, i);
            } else {
                // If copySize.height is odd and there is an empty row at the beginning of the
                // first slice of the copy region, we can split the copy region into two copies:
                // copy0 to copy the first slice, copy1 to copy the rest slices because the
                // offset of slice 1 is aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512)
                // without an empty row. This is an easier case relative to cases with even copy
                // height.
                Recompute3DTextureCopyRegionWithEmptyFirstRowAndOddCopyHeight(
                    direction, copySize, bytesPerRow, copySubresource, i);
            }
        }
    }

    return copySubresource;
}

TextureCopySubresource Compute2DTextureCopySubresourceWithRelaxedRowPitchAndOffset(
    BufferTextureCopyDirection direction,
    Origin3D origin,
    Extent3D copySize,
    const TexelBlockInfo& blockInfo,
    uint64_t offset,
    uint32_t bytesPerRow) {
    TextureCopySubresource copy;
    auto* copyInfo = copy.AddCopy();

    // You can visualize the data in the buffer (bufferLocation) like this:
    // * copy data is visualized as '+'.
    //
    //                bufferOffset(0, 0, 0)
    //                        ^
    //                        |
    // |<-------Offset------->|<-----------RowPitch----------->|----------|
    // |----------------------|++++++++++++++++++++++~~~~~~~~~~|    |     |
    //                        |++++++++++++++++++++++~~~~~~~~~~|CopyHeight|
    //                        |++++++++++++++++++++++|         |    |     |
    //                        |<-----CopyWidth------>|         |----------|
    //
    Origin3D textureOffset = {origin.x, origin.y, 0};
    Origin3D bufferOffset = {0, 0, 0};
    Extent3D copySizeOneLayer = {copySize.width, copySize.height, 1};
    ComputeSourceRegionForCopyInfo(copyInfo, direction, bufferOffset, textureOffset,
                                   copySizeOneLayer);

    Extent3D bufferSize = {copySize.width, copySize.height, 1};

    FillFootprintAndOffsetOfBufferLocation(&copyInfo->bufferLocation, offset, bufferSize,
                                           bytesPerRow);

    return copy;
}

TextureCopySubresource Compute3DTextureCopySubresourceWithRelaxedRowPitchAndOffset(
    BufferTextureCopyDirection direction,
    Origin3D origin,
    Extent3D copySize,
    const TexelBlockInfo& blockInfo,
    uint64_t offset,
    uint32_t bytesPerRow,
    uint32_t rowsPerImage) {
    TextureCopySubresource copy;

    Origin3D bufferOffset = {0, 0, 0};

    // You can visualize the data in the buffer (bufferLocation) like the inline comments.
    // * copy data is visualized as '+'.
    uint32_t depthInCopy1 = copySize.depthOrArrayLayers - 1;
    if (depthInCopy1 > 0) {
        // `bufferLocation` in the 1st copy (first `depthInCopy1` images, optional):
        //
        //                bufferOffset(0, 0, 0)
        //                        ^
        //                        |
        // |<-------Offset1------>|<-----------RowPitch----------->|----------|------------|
        // |----------------------|++++++++++++++++++++++~~~~~~~~~~|    |     |     |      |
        //                        |++++++++++++++++++++++~~~~~~~~~~|CopyHeight|     |      |
        //                        |++++++++++++++++++++++~~~~~~~~~~|    |     |RowsPerImage|
        //                        |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|----------|     |      |
        //                        |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|          |     |      |
        // |---End of 1st image-->|--------------------------------|----------|------------|
        //                        |++++++++++++++++++++++~~~~~~~~~~|          |     |      |
        //                        |++++++++++++++++++++++~~~~~~~~~~|          |     |      |
        //                        |++++++++++++++++++++++~~~~~~~~~~|          |RowsPerImage|
        //                        |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|          |     |      |
        //                        |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|          |     |      |
        // |---End of 2nd image-->|--------------------------------|----------|------------|
        //                        |<-----CopyWidth------>|
        //

        Origin3D textureOffset1 = origin;
        uint32_t offset1 = offset;

        uint32_t rowsPerImage1 = rowsPerImage;

        auto* copyInfo1 = copy.AddCopy();
        Extent3D copySize1 = {copySize.width, copySize.height, depthInCopy1};
        ComputeSourceRegionForCopyInfo(copyInfo1, direction, bufferOffset, textureOffset1,
                                       copySize1);

        Extent3D bufferSize1 = {copySize.width, rowsPerImage1, depthInCopy1};

        FillFootprintAndOffsetOfBufferLocation(&copyInfo1->bufferLocation, offset1, bufferSize1,
                                               bytesPerRow);
    }

    {
        // We have to use the 2nd copy because there may not be enough memory to hold
        // (RowPitch * RowsPerImage) data for the last image in the buffer.
        //
        // `bufferLocation` in the 2nd copy (the last image):
        //
        //                bufferOffset (0, 0, 0)
        //                Begin of the last image
        //                        ^
        //                        |
        // |<-------Offset2------>|<-----------RowPitch----------->|----------|
        // |----------------------|++++++++++++++++++++++~~~~~~~~~~|    |     |
        //                        |++++++++++++++++++++++~~~~~~~~~~|CopyHeight|
        //                        |++++++++++++++++++++++|         |    |     |
        //                        |----------------------|---------|----------|
        //                        |<-----CopyWidth------>|
        //                                               ^
        //                                     End of all buffer data
        //
        DAWN_ASSERT(copySize.depthOrArrayLayers >= 1);
        Origin3D textureOffset2 = {origin.x, origin.y, origin.z + depthInCopy1};
        uint32_t offset2 = offset + bytesPerRow * rowsPerImage * depthInCopy1;
        uint32_t depthInCopy2 = 1;
        uint32_t rowsPerImage2 = copySize.height;

        auto* copyInfo2 = copy.AddCopy();
        Extent3D copySize2 = {copySize.width, copySize.height, depthInCopy2};
        ComputeSourceRegionForCopyInfo(copyInfo2, direction, bufferOffset, textureOffset2,
                                       copySize2);

        Extent3D bufferSize2 = {copySize.width, rowsPerImage2, depthInCopy2};

        FillFootprintAndOffsetOfBufferLocation(&copyInfo2->bufferLocation, offset2, bufferSize2,
                                               bytesPerRow);
    }

    return copy;
}

}  // namespace dawn::native::d3d12
