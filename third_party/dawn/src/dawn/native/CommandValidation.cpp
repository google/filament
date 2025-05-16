// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/CommandValidation.h"

#include <algorithm>
#include <array>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Numeric.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBufferStateTracker.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/webgpu_absl_format.h"

namespace dawn::native {

namespace {

std::string ToBufferSyncScopeResourceUsage(wgpu::BufferUsage syncScopeBufferUsage) {
    constexpr std::array kUsageInfoList = {
        std::make_pair(wgpu::BufferUsage::Index, "Index"),
        std::make_pair(wgpu::BufferUsage::Vertex, "Vertex"),
        std::make_pair(wgpu::BufferUsage::Indirect, "Indirect"),
        std::make_pair(wgpu::BufferUsage::Uniform, "Uniform"),
        std::make_pair(wgpu::BufferUsage::Storage, "Storage(read-write)"),
        std::make_pair(kReadOnlyStorageBuffer, "Storage(read-only)")};

    std::stringstream stream;
    bool first = true;
    for (const auto& [usage, info] : kUsageInfoList) {
        if (syncScopeBufferUsage & usage) {
            if (!first) {
                stream << "|";
            }
            first = false;
            stream << info;
            syncScopeBufferUsage &= ~usage;
        }
    }

    if (static_cast<bool>(syncScopeBufferUsage)) {
        if (!first) {
            stream << "|";
        }
        stream << "BufferUsage::0x" << std::hex
               << static_cast<typename std::underlying_type<wgpu::BufferUsage>::type>(
                      syncScopeBufferUsage);
    }

    return stream.str();
}

std::string ToTextureSyncScopeResourceUsage(wgpu::TextureUsage syncScopeTextureUsage) {
    constexpr std::array kUsageInfoList = {
        std::make_pair(wgpu::TextureUsage::TextureBinding, "TextureBinding"),
        std::make_pair(wgpu::TextureUsage::StorageBinding, "Storage(read-write)"),
        std::make_pair(kWriteOnlyStorageTexture, "Storage(write-only)"),
        std::make_pair(kReadOnlyStorageTexture, "Storage(read-only)"),
        std::make_pair(wgpu::TextureUsage::RenderAttachment, "RenderAttachment"),
        std::make_pair(kReadOnlyRenderAttachment, "RenderAttachment(read-only)")};

    std::stringstream stream;
    bool first = true;
    for (const auto& [usage, info] : kUsageInfoList) {
        if (syncScopeTextureUsage & usage) {
            if (!first) {
                stream << "|";
            }
            first = false;
            stream << info;
            syncScopeTextureUsage &= ~usage;
        }
    }

    if (static_cast<bool>(syncScopeTextureUsage)) {
        if (!first) {
            stream << "|";
        }
        stream << "TextureUsage::0x" << std::hex
               << static_cast<typename std::underlying_type<wgpu::TextureUsage>::type>(
                      syncScopeTextureUsage);
    }

    return stream.str();
}

}  // namespace

// Performs validation of the "synchronization scope" rules of WebGPU.
MaybeError ValidateSyncScopeResourceUsage(const SyncScopeResourceUsage& scope) {
    // Buffers can only be used as single-write or multiple read.
    for (size_t i = 0; i < scope.bufferSyncInfos.size(); ++i) {
        const wgpu::BufferUsage usage = scope.bufferSyncInfos[i].usage;
        bool readOnly = IsSubset(usage, kReadOnlyBufferUsages);
        bool singleUse = wgpu::HasZeroOrOneBits(usage);

        DAWN_INVALID_IF(!readOnly && !singleUse,
                        "%s usage (%s) includes writable usage and another usage in the same "
                        "synchronization scope.",
                        scope.buffers[i], ToBufferSyncScopeResourceUsage(usage));
    }

    // Check that every single subresource is used as either a single-write usage or a
    // combination of readonly usages.
    for (size_t i = 0; i < scope.textureSyncInfos.size(); ++i) {
        const TextureSubresourceSyncInfo& textureSyncInfo = scope.textureSyncInfos[i];
        DAWN_TRY(textureSyncInfo.Iterate(
            [&](const SubresourceRange&, const TextureSyncInfo& syncInfo) -> MaybeError {
                bool readOnly = IsSubset(syncInfo.usage, kReadOnlyTextureUsages);
                bool singleUse = wgpu::HasZeroOrOneBits(syncInfo.usage);
                if (readOnly || singleUse) {
                    return {};
                }
                // kResolveTextureLoadAndStoreUsages are kResolveAttachmentLoadingUsage &
                // RenderAttachment usage used in the same pass.
                // This is accepted because kResolveAttachmentLoadingUsage is an internal loading
                // operation for blitting a resolve target to an MSAA attachment. And there won't be
                // and read-after-write hazard.
                if (syncInfo.usage == kResolveTextureLoadAndStoreUsages) {
                    return {};
                }
                return DAWN_VALIDATION_ERROR(
                    "%s usage (%s) includes writable usage and another usage in the same "
                    "synchronization scope.",
                    scope.textures[i], ToTextureSyncScopeResourceUsage(syncInfo.usage));
            }));
    }
    return {};
}

MaybeError ValidateTimestampQuery(const DeviceBase* device,
                                  const QuerySetBase* querySet,
                                  uint32_t queryIndex,
                                  Feature requiredFeature) {
    DAWN_TRY(device->ValidateObject(querySet));

    DAWN_INVALID_IF(!device->HasFeature(requiredFeature),
                    "Timestamp queries used without the %s feature enabled.",
                    ToAPI(requiredFeature));

    DAWN_INVALID_IF(querySet->GetQueryType() != wgpu::QueryType::Timestamp,
                    "The type of %s is not %s.", querySet, wgpu::QueryType::Timestamp);

    DAWN_INVALID_IF(queryIndex >= querySet->GetQueryCount(),
                    "Query index (%u) exceeds the number of queries (%u) in %s.", queryIndex,
                    querySet->GetQueryCount(), querySet);

    return {};
}

MaybeError ValidatePassTimestampWrites(const DeviceBase* device,
                                       const PassTimestampWrites* timestampWrites) {
    DAWN_INVALID_IF(!device->HasFeature(Feature::TimestampQuery),
                    "Timestamp queries used without the timestamp-query feature enabled.");

    UnpackedPtr<PassTimestampWrites> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(timestampWrites));

    QuerySetBase* querySet = unpacked->querySet;
    DAWN_ASSERT(unpacked->querySet != nullptr);
    DAWN_TRY(device->ValidateObject(querySet));
    DAWN_INVALID_IF(querySet->GetQueryType() != wgpu::QueryType::Timestamp,
                    "The type of %s is not %s.", querySet, wgpu::QueryType::Timestamp);

    if (unpacked->beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
        DAWN_INVALID_IF(unpacked->beginningOfPassWriteIndex >= querySet->GetQueryCount(),
                        "beginningOfPassWriteIndex (%u) exceeds the number of queries (%u) in %s.",
                        unpacked->beginningOfPassWriteIndex, querySet->GetQueryCount(), querySet);
    }
    if (unpacked->endOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
        DAWN_INVALID_IF(unpacked->endOfPassWriteIndex >= querySet->GetQueryCount(),
                        "endOfPassWriteIndex (%u) exceeds the number of queries (%u) in %s.",
                        unpacked->endOfPassWriteIndex, querySet->GetQueryCount(), querySet);
    }

    DAWN_INVALID_IF(unpacked->beginningOfPassWriteIndex == wgpu::kQuerySetIndexUndefined &&
                        unpacked->endOfPassWriteIndex == wgpu::kQuerySetIndexUndefined,
                    "Both beginningOfPassWriteIndex and endOfPassWriteIndex are undefined.");

    DAWN_INVALID_IF(unpacked->beginningOfPassWriteIndex == unpacked->endOfPassWriteIndex,
                    "beginningOfPassWriteIndex (%u) is equal to endOfPassWriteIndex (%u).",
                    unpacked->beginningOfPassWriteIndex, unpacked->endOfPassWriteIndex);

    return {};
}

MaybeError ValidateWriteBuffer(const DeviceBase* device,
                               const BufferBase* buffer,
                               uint64_t bufferOffset,
                               uint64_t size) {
    DAWN_TRY(device->ValidateObject(buffer));

    DAWN_INVALID_IF(bufferOffset % 4 != 0, "BufferOffset (%u) is not a multiple of 4.",
                    bufferOffset);

    DAWN_INVALID_IF(size % 4 != 0, "Size (%u) is not a multiple of 4.", size);

    uint64_t bufferSize = buffer->GetSize();
    DAWN_INVALID_IF(bufferOffset > bufferSize || size > (bufferSize - bufferOffset),
                    "Write range (bufferOffset: %u, size: %u) does not fit in %s size (%u).",
                    bufferOffset, size, buffer, bufferSize);

    DAWN_TRY(ValidateCanUseAs(buffer, wgpu::BufferUsage::CopyDst));

    return {};
}

bool IsRangeOverlapped(uint32_t startA, uint32_t startB, uint32_t length) {
    if (length < 1) {
        return false;
    }
    return RangesOverlap<uint64_t>(
        static_cast<uint64_t>(startA),
        static_cast<uint64_t>(startA) + static_cast<uint64_t>(length) - 1,
        static_cast<uint64_t>(startB),
        static_cast<uint64_t>(startB) + static_cast<uint64_t>(length) - 1);
}

ResultOrError<uint64_t> ComputeRequiredBytesInCopy(const TexelBlockInfo& blockInfo,
                                                   const Extent3D& copySize,
                                                   uint32_t bytesPerRow,
                                                   uint32_t rowsPerImage) {
    DAWN_ASSERT(copySize.width % blockInfo.width == 0);
    DAWN_ASSERT(copySize.height % blockInfo.height == 0);
    uint32_t widthInBlocks = copySize.width / blockInfo.width;
    uint32_t heightInBlocks = copySize.height / blockInfo.height;
    uint64_t bytesInLastRow = Safe32x32(widthInBlocks, blockInfo.byteSize);

    if (copySize.depthOrArrayLayers == 0) {
        return 0;
    }

    // Check for potential overflows for the rest of the computations. We have the following
    // inequalities:
    //
    //   bytesInLastRow <= bytesPerRow
    //   heightInBlocks <= rowsPerImage
    //
    // So:
    //
    //   bytesInLastImage  = bytesPerRow * (heightInBlocks - 1) + bytesInLastRow
    //                    <= bytesPerRow * heightInBlocks
    //                    <= bytesPerRow * rowsPerImage
    //                    <= bytesPerImage
    //
    // This means that if the computation of depth * bytesPerImage doesn't overflow, none of the
    // computations for requiredBytesInCopy will. (and it's not a very pessimizing check)
    DAWN_ASSERT(copySize.depthOrArrayLayers <= 1 || (bytesPerRow != wgpu::kCopyStrideUndefined &&
                                                     rowsPerImage != wgpu::kCopyStrideUndefined));
    uint64_t bytesPerImage = Safe32x32(bytesPerRow, rowsPerImage);
    DAWN_INVALID_IF(
        bytesPerImage > std::numeric_limits<uint64_t>::max() / copySize.depthOrArrayLayers,
        "The number of bytes per image (%u) exceeds the maximum (%u) when copying %u images.",
        bytesPerImage, std::numeric_limits<uint64_t>::max() / copySize.depthOrArrayLayers,
        copySize.depthOrArrayLayers);

    uint64_t requiredBytesInCopy = bytesPerImage * (copySize.depthOrArrayLayers - 1);
    if (heightInBlocks > 0) {
        DAWN_ASSERT(heightInBlocks <= 1 || bytesPerRow != wgpu::kCopyStrideUndefined);
        uint64_t bytesInLastImage = Safe32x32(bytesPerRow, heightInBlocks - 1) + bytesInLastRow;
        requiredBytesInCopy += bytesInLastImage;
    }
    return requiredBytesInCopy;
}

MaybeError ValidateCopySizeFitsInBuffer(const Ref<BufferBase>& buffer,
                                        uint64_t offset,
                                        uint64_t size,
                                        BufferSizeType checkBufferSizeType) {
    uint64_t bufferSize = 0;
    switch (checkBufferSizeType) {
        case BufferSizeType::Size:
            bufferSize = buffer->GetSize();
            break;
        case BufferSizeType::AllocatedSize:
            bufferSize = buffer->GetAllocatedSize();
            break;
    }
    bool fitsInBuffer = offset <= bufferSize && (size <= (bufferSize - offset));
    DAWN_INVALID_IF(!fitsInBuffer,
                    "Copy range (offset: %u, size: %u) does not fit in %s size (%u).", offset, size,
                    buffer.Get(), bufferSize);

    return {};
}

// Replace wgpu::kCopyStrideUndefined with real values, so backends don't have to think about
// it.
void ApplyDefaultTexelCopyBufferLayoutOptions(TexelCopyBufferLayout* layout,
                                              const TexelBlockInfo& blockInfo,
                                              const Extent3D& copyExtent) {
    DAWN_ASSERT(layout != nullptr);
    DAWN_ASSERT(copyExtent.height % blockInfo.height == 0);
    uint32_t heightInBlocks = copyExtent.height / blockInfo.height;

    if (layout->bytesPerRow == wgpu::kCopyStrideUndefined) {
        DAWN_ASSERT(copyExtent.width % blockInfo.width == 0);
        uint32_t widthInBlocks = copyExtent.width / blockInfo.width;
        uint32_t bytesInLastRow = widthInBlocks * blockInfo.byteSize;

        DAWN_ASSERT(heightInBlocks <= 1 && copyExtent.depthOrArrayLayers <= 1);
        layout->bytesPerRow = Align(bytesInLastRow, kTextureBytesPerRowAlignment);
    }
    if (layout->rowsPerImage == wgpu::kCopyStrideUndefined) {
        DAWN_ASSERT(copyExtent.depthOrArrayLayers <= 1);
        layout->rowsPerImage = heightInBlocks;
    }
}

MaybeError ValidateLinearTextureData(const TexelCopyBufferLayout& layout,
                                     uint64_t byteSize,
                                     const TexelBlockInfo& blockInfo,
                                     const Extent3D& copyExtent) {
    DAWN_ASSERT(copyExtent.height % blockInfo.height == 0);
    uint32_t heightInBlocks = copyExtent.height / blockInfo.height;

    DAWN_INVALID_IF(
        copyExtent.depthOrArrayLayers > 1 && (layout.bytesPerRow == wgpu::kCopyStrideUndefined ||
                                              layout.rowsPerImage == wgpu::kCopyStrideUndefined),
        "Copy depth (%u) is > 1, but bytesPerRow (%u) or rowsPerImage (%u) are not specified.",
        copyExtent.depthOrArrayLayers,
        WrapUndefined(layout.bytesPerRow, wgpu::kCopyStrideUndefined),
        WrapUndefined(layout.rowsPerImage, wgpu::kCopyStrideUndefined));

    DAWN_INVALID_IF(heightInBlocks > 1 && layout.bytesPerRow == wgpu::kCopyStrideUndefined,
                    "HeightInBlocks (%u) is > 1, but bytesPerRow is not specified.",
                    heightInBlocks);

    // Validation for other members in layout:
    DAWN_ASSERT(copyExtent.width % blockInfo.width == 0);
    uint32_t widthInBlocks = copyExtent.width / blockInfo.width;
    DAWN_ASSERT(Safe32x32(widthInBlocks, blockInfo.byteSize) <=
                std::numeric_limits<uint32_t>::max());
    uint32_t bytesInLastRow = widthInBlocks * blockInfo.byteSize;

    // These != wgpu::kCopyStrideUndefined checks are technically redundant with the > checks,
    // but they should get optimized out.
    DAWN_INVALID_IF(
        layout.bytesPerRow != wgpu::kCopyStrideUndefined && bytesInLastRow > layout.bytesPerRow,
        "The byte size of each row (%u) is > bytesPerRow (%u).", bytesInLastRow,
        layout.bytesPerRow);

    DAWN_INVALID_IF(
        layout.rowsPerImage != wgpu::kCopyStrideUndefined && heightInBlocks > layout.rowsPerImage,
        "The height of each image in blocks (%u) is > rowsPerImage (%u).", heightInBlocks,
        layout.rowsPerImage);

    // We compute required bytes in copy after validating texel block alignments
    // because the divisibility conditions are necessary for the algorithm to be valid,
    // also the bytesPerRow bound is necessary to avoid overflows.
    uint64_t requiredBytesInCopy;
    DAWN_TRY_ASSIGN(
        requiredBytesInCopy,
        ComputeRequiredBytesInCopy(blockInfo, copyExtent, layout.bytesPerRow, layout.rowsPerImage));

    bool fitsInData =
        layout.offset <= byteSize && (requiredBytesInCopy <= (byteSize - layout.offset));
    DAWN_INVALID_IF(
        !fitsInData,
        "Required size for texture data layout (%u) exceeds the linear data size (%u) with "
        "offset (%u).",
        requiredBytesInCopy, byteSize, layout.offset);

    return {};
}

MaybeError ValidateTexelCopyBufferInfo(DeviceBase const* device,
                                       const TexelCopyBufferInfo& texelCopyBufferInfo) {
    DAWN_TRY(device->ValidateObject(texelCopyBufferInfo.buffer));
    auto alignment = kTextureBytesPerRowAlignment;
    if (device->HasFeature(Feature::DawnTexelCopyBufferRowAlignment)) {
        alignment =
            device->GetLimits().texelCopyBufferRowAlignmentLimits.minTexelCopyBufferRowAlignment;
    }
    if (texelCopyBufferInfo.layout.bytesPerRow != wgpu::kCopyStrideUndefined) {
        DAWN_INVALID_IF(texelCopyBufferInfo.layout.bytesPerRow % alignment != 0,
                        "bytesPerRow (%u) is not a multiple of %u.",
                        texelCopyBufferInfo.layout.bytesPerRow, alignment);
    }

    return {};
}

MaybeError ValidateTexelCopyTextureInfo(DeviceBase const* device,
                                        const TexelCopyTextureInfo& textureCopy,
                                        const Extent3D& copySize) {
    const TextureBase* texture = textureCopy.texture;
    DAWN_TRY(device->ValidateObject(texture));

    DAWN_INVALID_IF(textureCopy.mipLevel >= texture->GetNumMipLevels(),
                    "MipLevel (%u) is greater than the number of mip levels (%u) in %s.",
                    textureCopy.mipLevel, texture->GetNumMipLevels(), texture);

    DAWN_TRY(ValidateTextureAspect(textureCopy.aspect));

    const auto aspect = SelectFormatAspects(texture->GetFormat(), textureCopy.aspect);
    DAWN_INVALID_IF(aspect == Aspect::None,
                    "%s format (%s) does not have the selected aspect (%s).", texture,
                    texture->GetFormat().format, textureCopy.aspect);

    if (texture->GetSampleCount() > 1 || texture->GetFormat().HasDepthOrStencil()) {
        Extent3D subresourceSize =
            texture->GetMipLevelSingleSubresourcePhysicalSize(textureCopy.mipLevel, aspect);
        DAWN_ASSERT(texture->GetDimension() == wgpu::TextureDimension::e2D);
        DAWN_INVALID_IF(
            textureCopy.origin.x != 0 || textureCopy.origin.y != 0 ||
                subresourceSize.width != copySize.width ||
                subresourceSize.height != copySize.height,
            "Copy origin (%s) and size (%s) does not cover the entire subresource (origin: "
            "[x: 0, y: 0], size: %s) of %s. The entire subresource must be copied when the "
            "format (%s) is a depth/stencil format or the sample count (%u) is > 1.",
            &textureCopy.origin, &copySize, &subresourceSize, texture, texture->GetFormat().format,
            texture->GetSampleCount());
    }

    return {};
}

MaybeError ValidateTextureCopyRange(DeviceBase const* device,
                                    const TexelCopyTextureInfo& textureCopy,
                                    const Extent3D& copySize) {
    const TextureBase* texture = textureCopy.texture;
    const Format& format = textureCopy.texture->GetFormat();
    const Aspect aspect = ConvertAspect(format, textureCopy.aspect);

    DAWN_ASSERT(!format.IsMultiPlanar() || HasOneBit(aspect));

    // Validation for the copy being in-bounds:
    Extent3D mipSize =
        texture->GetMipLevelSingleSubresourcePhysicalSize(textureCopy.mipLevel, aspect);
    // For 1D/2D textures, include the array layer as depth so it can be checked with other
    // dimensions.
    if (texture->GetDimension() != wgpu::TextureDimension::e3D) {
        mipSize.depthOrArrayLayers = texture->GetArrayLayers();
    }
    // All texture dimensions are in uint32_t so by doing checks in uint64_t we avoid
    // overflows.
    DAWN_INVALID_IF(
        static_cast<uint64_t>(textureCopy.origin.x) + static_cast<uint64_t>(copySize.width) >
                static_cast<uint64_t>(mipSize.width) ||
            static_cast<uint64_t>(textureCopy.origin.y) + static_cast<uint64_t>(copySize.height) >
                static_cast<uint64_t>(mipSize.height) ||
            static_cast<uint64_t>(textureCopy.origin.z) +
                    static_cast<uint64_t>(copySize.depthOrArrayLayers) >
                static_cast<uint64_t>(mipSize.depthOrArrayLayers),
        "Texture copy range (origin: %s, copySize: %s) touches outside of %s mip level %u "
        "size (%s).",
        &textureCopy.origin, &copySize, texture, textureCopy.mipLevel, &mipSize);

    // Validation for the texel block alignments:
    if (format.isCompressed) {
        const TexelBlockInfo& blockInfo = format.GetAspectInfo(textureCopy.aspect).block;
        DAWN_INVALID_IF(
            textureCopy.origin.x % blockInfo.width != 0,
            "Texture copy origin.x (%u) is not a multiple of compressed texture format block "
            "width (%u).",
            textureCopy.origin.x, blockInfo.width);
        DAWN_INVALID_IF(
            textureCopy.origin.y % blockInfo.height != 0,
            "Texture copy origin.y (%u) is not a multiple of compressed texture format block "
            "height (%u).",
            textureCopy.origin.y, blockInfo.height);
        DAWN_INVALID_IF(
            copySize.width % blockInfo.width != 0,
            "copySize.width (%u) is not a multiple of compressed texture format block width "
            "(%u).",
            copySize.width, blockInfo.width);
        DAWN_INVALID_IF(copySize.height % blockInfo.height != 0,
                        "copySize.height (%u) is not a multiple of compressed texture format block "
                        "height (%u).",
                        copySize.height, blockInfo.height);
    }

    return {};
}

// Always returns a single aspect (color, stencil, depth, or ith plane for multi-planar
// formats).
ResultOrError<Aspect> SingleAspectUsedByTexelCopyTextureInfo(const TexelCopyTextureInfo& view) {
    const Format& format = view.texture->GetFormat();
    switch (view.aspect) {
        case wgpu::TextureAspect::All: {
            DAWN_INVALID_IF(
                !HasOneBit(format.aspects),
                "More than a single aspect (%s) is selected for multi-planar format (%s) in "
                "%s <-> linear data copy.",
                view.aspect, format.format, view.texture);

            Aspect single = format.aspects;
            return single;
        }
        case wgpu::TextureAspect::DepthOnly:
            DAWN_ASSERT(format.aspects & Aspect::Depth);
            return Aspect::Depth;
        case wgpu::TextureAspect::StencilOnly:
            DAWN_ASSERT(format.aspects & Aspect::Stencil);
            return Aspect::Stencil;
        case wgpu::TextureAspect::Plane0Only:
            return Aspect::Plane0;
        case wgpu::TextureAspect::Plane1Only:
            return Aspect::Plane1;
        case wgpu::TextureAspect::Plane2Only:
            return Aspect::Plane2;
        case wgpu::TextureAspect::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MaybeError ValidateLinearToDepthStencilCopyRestrictions(const TexelCopyTextureInfo& dst) {
    Aspect aspectUsed;
    DAWN_TRY_ASSIGN(aspectUsed, SingleAspectUsedByTexelCopyTextureInfo(dst));

    const Format& format = dst.texture->GetFormat();
    switch (format.format) {
        case wgpu::TextureFormat::Depth16Unorm:
            return {};
        default:
            DAWN_INVALID_IF(aspectUsed == Aspect::Depth,
                            "Cannot copy into the depth aspect of %s with format %s.", dst.texture,
                            format.format);
            break;
    }

    return {};
}

MaybeError ValidateTextureToTextureCopyCommonRestrictions(DeviceBase const* device,
                                                          const TexelCopyTextureInfo& src,
                                                          const TexelCopyTextureInfo& dst,
                                                          const Extent3D& copySize) {
    const uint32_t srcSamples = src.texture->GetSampleCount();
    const uint32_t dstSamples = dst.texture->GetSampleCount();

    DAWN_INVALID_IF(
        srcSamples != dstSamples,
        "Source %s sample count (%u) and destination %s sample count (%u) does not match.",
        src.texture, srcSamples, dst.texture, dstSamples);

    DAWN_INVALID_IF(device->IsCompatibilityMode() && srcSamples != 1,
                    "Source %s and destination %s with sample count (%u) > 1 cannot be copied in "
                    "compatibility mode.",
                    src.texture, dst.texture, srcSamples);

    // Metal cannot select a single aspect for texture-to-texture copies.
    const Format& format = src.texture->GetFormat();
    DAWN_INVALID_IF(
        SelectFormatAspects(format, src.aspect) != format.aspects,
        "Source %s aspect (%s) doesn't select all the aspects of the source format (%s).",
        src.texture, src.aspect, format.format);

    DAWN_INVALID_IF(
        SelectFormatAspects(format, dst.aspect) != format.aspects,
        "Destination %s aspect (%s) doesn't select all the aspects of the destination format "
        "(%s).",
        dst.texture, dst.aspect, format.format);

    if (src.texture == dst.texture) {
        switch (src.texture->GetDimension()) {
            case wgpu::TextureDimension::Undefined:
                DAWN_UNREACHABLE();

            case wgpu::TextureDimension::e1D:
                DAWN_ASSERT(src.mipLevel == 0);
                return DAWN_VALIDATION_ERROR("Copy is from %s to itself.", src.texture);

            case wgpu::TextureDimension::e2D:
                DAWN_INVALID_IF(
                    src.mipLevel == dst.mipLevel &&
                        IsRangeOverlapped(src.origin.z, dst.origin.z, copySize.depthOrArrayLayers),
                    "Copy source and destination are overlapping layer ranges "
                    "([%u, %u) and [%u, %u)) of %s mip level %u",
                    src.origin.z, src.origin.z + copySize.depthOrArrayLayers, dst.origin.z,
                    dst.origin.z + copySize.depthOrArrayLayers, src.texture, src.mipLevel);
                break;

            case wgpu::TextureDimension::e3D:
                DAWN_INVALID_IF(src.mipLevel == dst.mipLevel,
                                "Copy is from %s mip level %u to itself.", src.texture,
                                src.mipLevel);
                break;
        }
    }

    return {};
}

MaybeError ValidateTextureToTextureCopyRestrictions(DeviceBase const* device,
                                                    const TexelCopyTextureInfo& src,
                                                    const TexelCopyTextureInfo& dst,
                                                    const Extent3D& copySize) {
    // Metal requires texture-to-texture copies happens between texture formats that equal to
    // each other or only have diff on srgb-ness.
    DAWN_INVALID_IF(!src.texture->GetFormat().CopyCompatibleWith(dst.texture->GetFormat()),
                    "Source %s format (%s) and destination %s format (%s) are not copy compatible.",
                    src.texture, src.texture->GetFormat().format, dst.texture,
                    dst.texture->GetFormat().format);

    return ValidateTextureToTextureCopyCommonRestrictions(device, src, dst, copySize);
}

MaybeError ValidateCanUseAs(const TextureBase* texture,
                            wgpu::TextureUsage usage,
                            UsageValidationMode mode) {
    DAWN_ASSERT(wgpu::HasZeroOrOneBits(usage));
    switch (mode) {
        case UsageValidationMode::Default:
            DAWN_INVALID_IF(!(texture->GetUsage() & usage), "%s usage (%s) doesn't include %s.",
                            texture, texture->GetUsage(), usage);
            break;
        case UsageValidationMode::Internal:
            DAWN_INVALID_IF(!(texture->GetInternalUsage() & usage),
                            "%s internal usage (%s) doesn't include %s.", texture,
                            texture->GetInternalUsage(), usage);
            break;
    }
    return {};
}

MaybeError ValidateCanUseAs(const TextureViewBase* textureView,
                            wgpu::TextureUsage usage,
                            UsageValidationMode mode) {
    DAWN_ASSERT(wgpu::HasZeroOrOneBits(usage));
    DAWN_ASSERT(IsSubset(usage, kTextureViewOnlyUsages));
    switch (mode) {
        case UsageValidationMode::Default:
            DAWN_INVALID_IF(!(textureView->GetUsage() & usage), "%s usage (%s) doesn't include %s.",
                            textureView, textureView->GetUsage(), usage);
            break;
        case UsageValidationMode::Internal:
            DAWN_INVALID_IF(!(textureView->GetInternalUsage() & usage),
                            "%s internal usage (%s) doesn't include %s.", textureView,
                            textureView->GetInternalUsage(), usage);
            break;
    }
    return {};
}

MaybeError ValidateCanUseAs(const BufferBase* buffer, wgpu::BufferUsage usage) {
    DAWN_ASSERT(wgpu::HasZeroOrOneBits(usage));
    DAWN_INVALID_IF(!(buffer->GetUsage() & usage), "%s usage (%s) doesn't include %s.", buffer,
                    buffer->GetUsage(), usage);
    return {};
}

MaybeError ValidateCanUseAsInternal(const BufferBase* buffer, wgpu::BufferUsage usage) {
    DAWN_INVALID_IF(!(buffer->GetInternalUsage() & usage),
                    "%s internal usage (%s) doesn't include %s.", buffer,
                    buffer->GetInternalUsage(), usage);
    return {};
}

namespace {
std::string TextureFormatsToString(const ColorAttachmentFormats& formats) {
    std::ostringstream ss;
    ss << "[ ";
    for (const Format* format : formats) {
        ss << absl::StrFormat("%s", format->format) << " ";
    }
    ss << "]";
    return ss.str();
}
}  // anonymous namespace

MaybeError ValidateColorAttachmentBytesPerSample(DeviceBase* device,
                                                 const ColorAttachmentFormats& formats) {
    uint32_t totalByteSize = 0;
    for (const Format* format : formats) {
        totalByteSize = Align(totalByteSize, format->renderTargetComponentAlignment);
        totalByteSize += format->renderTargetPixelByteCost;
    }
    uint32_t maxColorAttachmentBytesPerSample =
        device->GetLimits().v1.maxColorAttachmentBytesPerSample;
    DAWN_INVALID_IF(
        totalByteSize > maxColorAttachmentBytesPerSample,
        "Total color attachment bytes per sample (%u) exceeds maximum (%u) with formats "
        "(%s).%s",
        totalByteSize, maxColorAttachmentBytesPerSample, TextureFormatsToString(formats),
        DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter()->GetLimits().v1,
                                    maxColorAttachmentBytesPerSample, totalByteSize));

    return {};
}

MaybeError ValidatePLSInfo(
    const DeviceBase* device,
    uint64_t totalSize,
    ityp::span<size_t, StorageAttachmentInfoForValidation> storageAttachments) {
    DAWN_INVALID_IF(
        !(device->HasFeature(Feature::PixelLocalStorageCoherent) ||
          device->HasFeature(Feature::PixelLocalStorageNonCoherent)),
        "Pixel Local Storage feature used without either of the pixel-local-storage-coherent or "
        "pixel-local-storage-non-coherent features enabled.");

    // Validate totalPixelLocalStorageSize
    DAWN_INVALID_IF(totalSize % kPLSSlotByteSize != 0,
                    "totalPixelLocalStorageSize (%i) is not a multiple of %i.", totalSize,
                    kPLSSlotByteSize);
    DAWN_INVALID_IF(totalSize > kMaxPLSSize,
                    "totalPixelLocalStorageSize (%i) is larger than maxPixelLocalStorageSize (%i).",
                    totalSize, kMaxPLSSize);

    std::array<size_t, kMaxPLSSlots> indexForSlot;
    constexpr size_t kSlotNotSet = std::numeric_limits<size_t>::max();
    indexForSlot.fill(kSlotNotSet);
    for (size_t i = 0; i < storageAttachments.size(); i++) {
        const Format& format = device->GetValidInternalFormat(storageAttachments[i].format);
        DAWN_ASSERT(format.supportsStorageAttachment);

        // Validate the slot's offset.
        uint64_t offset = storageAttachments[i].offset;
        DAWN_INVALID_IF(offset % kPLSSlotByteSize != 0,
                        "storageAttachments[%i].offset (%i) is not a multiple of %i.", i, offset,
                        kPLSSlotByteSize);
        DAWN_INVALID_IF(
            offset > kMaxPLSSize,
            "storageAttachments[%i].offset (%i) is larger than maxPixelLocalStorageSize (%i).", i,
            offset, kMaxPLSSize);
        // This can't overflow because kMaxPLSSize + max texel byte size is way less than 2^32.
        DAWN_INVALID_IF(
            offset + format.GetAspectInfo(Aspect::Color).block.byteSize > totalSize,
            "storageAttachments[%i]'s footprint [%i, %i) does not fit in the total size (%i).", i,
            offset, format.GetAspectInfo(Aspect::Color).block.byteSize, totalSize);

        // Validate that there are no collisions, each storage attachment takes a single slot so
        // we don't need to loop over all slots for a storage attachment.
        DAWN_ASSERT(format.GetAspectInfo(Aspect::Color).block.byteSize == kPLSSlotByteSize);
        size_t slot = offset / kPLSSlotByteSize;
        DAWN_INVALID_IF(indexForSlot[slot] != kSlotNotSet,
                        "storageAttachments[%i] and storageAttachment[%i] conflict.", i,
                        indexForSlot[slot]);
        indexForSlot[slot] = i;
    }

    return {};
}

}  // namespace dawn::native
