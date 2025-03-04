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

#include "dawn/native/d3d12/UtilsD3D12.h"

#include <stringapiset.h>

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/BufferD3D12.h"
#include "dawn/native/d3d12/CommandRecordingContext.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/TextureCopySplitter.h"

namespace dawn::native::d3d12 {

namespace {

uint64_t RequiredCopySizeByD3D12(const uint32_t bytesPerRow,
                                 const uint32_t rowsPerImage,
                                 const Extent3D& copySize,
                                 const TexelBlockInfo& blockInfo) {
    uint64_t bytesPerImage = Safe32x32(bytesPerRow, rowsPerImage);

    // Required copy size for B2T/T2B copy on D3D12 is smaller than (but very close to)
    // depth * bytesPerImage. The latter is already checked at ComputeRequiredBytesInCopy()
    // in CommandValidation.cpp.
    uint64_t requiredCopySizeByD3D12 = bytesPerImage * (copySize.depthOrArrayLayers - 1);

    // When calculating the required copy size for B2T/T2B copy, D3D12 doesn't respect
    // rowsPerImage paddings on the last image for 3D texture, but it does respect
    // bytesPerRow paddings on the last row.
    DAWN_ASSERT(blockInfo.width == 1);
    DAWN_ASSERT(blockInfo.height == 1);
    uint64_t lastRowBytes = Safe32x32(blockInfo.byteSize, copySize.width);
    DAWN_ASSERT(rowsPerImage > copySize.height);
    uint64_t lastImageBytesByD3D12 = Safe32x32(bytesPerRow, rowsPerImage - 1) + lastRowBytes;

    requiredCopySizeByD3D12 += lastImageBytesByD3D12;
    return requiredCopySizeByD3D12;
}

// This function is used to access whether we need a workaround for D3D12's algorithm of
// calculating required buffer size for B2T/T2B copy. The workaround is needed only when
//   - It is a 3D texture (so the format is uncompressed).
//   - There are multiple depth images to be copied (copySize.depthOrArrayLayers > 1).
//   - It has rowsPerImage paddings (rowsPerImage > copySize.height).
//   - The buffer size doesn't meet D3D12's requirement.
bool NeedBufferSizeWorkaroundForBufferTextureCopyOnD3D12(const BufferCopy& bufferCopy,
                                                         const TextureCopy& textureCopy,
                                                         const Extent3D& copySize) {
    TextureBase* texture = textureCopy.texture.Get();
    if (texture->GetDimension() != wgpu::TextureDimension::e3D ||
        copySize.depthOrArrayLayers <= 1 || bufferCopy.rowsPerImage <= copySize.height) {
        return false;
    }

    const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(textureCopy.aspect).block;
    uint64_t requiredCopySizeByD3D12 = RequiredCopySizeByD3D12(
        bufferCopy.bytesPerRow, bufferCopy.rowsPerImage, copySize, blockInfo);
    return bufferCopy.buffer->GetAllocatedSize() - bufferCopy.offset < requiredCopySizeByD3D12;
}

}  // anonymous namespace

D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(wgpu::CompareFunction func) {
    switch (func) {
        case wgpu::CompareFunction::Never:
            return D3D12_COMPARISON_FUNC_NEVER;
        case wgpu::CompareFunction::Less:
            return D3D12_COMPARISON_FUNC_LESS;
        case wgpu::CompareFunction::LessEqual:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case wgpu::CompareFunction::Greater:
            return D3D12_COMPARISON_FUNC_GREATER;
        case wgpu::CompareFunction::GreaterEqual:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case wgpu::CompareFunction::Equal:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case wgpu::CompareFunction::NotEqual:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case wgpu::CompareFunction::Always:
            return D3D12_COMPARISON_FUNC_ALWAYS;

        case wgpu::CompareFunction::Undefined:
            DAWN_UNREACHABLE();
    }
}

D3D12_SHADER_VISIBILITY ShaderVisibilityType(wgpu::ShaderStage visibility) {
    DAWN_ASSERT(visibility != wgpu::ShaderStage::None);

    if (visibility == wgpu::ShaderStage::Vertex) {
        return D3D12_SHADER_VISIBILITY_VERTEX;
    }

    if (visibility == wgpu::ShaderStage::Fragment) {
        return D3D12_SHADER_VISIBILITY_PIXEL;
    }

    // For compute or any two combination of stages, visibility must be ALL
    return D3D12_SHADER_VISIBILITY_ALL;
}

D3D12_TEXTURE_COPY_LOCATION ComputeTextureCopyLocationForTexture(const Texture* texture,
                                                                 uint32_t level,
                                                                 uint32_t layer,
                                                                 Aspect aspect) {
    D3D12_TEXTURE_COPY_LOCATION copyLocation;
    copyLocation.pResource = texture->GetD3D12Resource();
    copyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    copyLocation.SubresourceIndex = texture->GetSubresourceIndex(level, layer, aspect);

    return copyLocation;
}

D3D12_BOX ComputeD3D12BoxFromOffsetAndSize(const Origin3D& offset, const Extent3D& copySize) {
    D3D12_BOX sourceRegion;
    sourceRegion.left = offset.x;
    sourceRegion.top = offset.y;
    sourceRegion.front = offset.z;
    sourceRegion.right = offset.x + copySize.width;
    sourceRegion.bottom = offset.y + copySize.height;
    sourceRegion.back = offset.z + copySize.depthOrArrayLayers;
    return sourceRegion;
}

void RecordBufferTextureCopyFromSplits(BufferTextureCopyDirection direction,
                                       ID3D12GraphicsCommandList* commandList,
                                       const TextureCopySubresource& baseCopySplit,
                                       ID3D12Resource* bufferResource,
                                       uint64_t baseOffset,
                                       uint64_t bufferBytesPerRow,
                                       TextureBase* textureBase,
                                       uint32_t textureMiplevel,
                                       uint32_t textureLayer,
                                       Aspect aspect) {
    Texture* texture = ToBackend(textureBase);
    const D3D12_TEXTURE_COPY_LOCATION textureLocation =
        ComputeTextureCopyLocationForTexture(texture, textureMiplevel, textureLayer, aspect);

    DXGI_FORMAT dxgiFormat = texture->GetD3D12CopyableSubresourceFormat(aspect);
    for (uint32_t i = 0; i < baseCopySplit.count; ++i) {
        const TextureCopySubresource::CopyInfo& info = baseCopySplit.copies[i];

        D3D12_TEXTURE_COPY_LOCATION bufferLocation = info.bufferLocation;
        bufferLocation.pResource = bufferResource;
        bufferLocation.PlacedFootprint.Offset += baseOffset;
        bufferLocation.PlacedFootprint.Footprint.Format = dxgiFormat;
        if (direction == BufferTextureCopyDirection::B2T) {
            commandList->CopyTextureRegion(&textureLocation, info.destinationOffset.x,
                                           info.destinationOffset.y, info.destinationOffset.z,
                                           &bufferLocation, &info.sourceRegion);
        } else {
            DAWN_ASSERT(direction == BufferTextureCopyDirection::T2B);
            commandList->CopyTextureRegion(&bufferLocation, info.destinationOffset.x,
                                           info.destinationOffset.y, info.destinationOffset.z,
                                           &textureLocation, &info.sourceRegion);
        }
    }
}

void Record2DBufferTextureCopyWithSplit(BufferTextureCopyDirection direction,
                                        ID3D12GraphicsCommandList* commandList,
                                        ID3D12Resource* bufferResource,
                                        const uint64_t offset,
                                        const uint32_t bytesPerRow,
                                        const uint32_t rowsPerImage,
                                        const TextureCopy& textureCopy,
                                        const TexelBlockInfo& blockInfo,
                                        const Extent3D& copySize) {
    // See comments in Compute2DTextureCopySplits() for more details.
    const TextureCopySplits copySplits = Compute2DTextureCopySplits(
        direction, textureCopy.origin, copySize, blockInfo, offset, bytesPerRow, rowsPerImage);

    const uint64_t bytesPerLayer = bytesPerRow * rowsPerImage;

    // copySplits.copySubresources[1] is always calculated for the second copy layer with
    // extra "bytesPerLayer" copy offset compared with the first copy layer. So
    // here we use an array bufferOffsetsForNextLayer to record the extra offsets
    // for each copy layer: bufferOffsetsForNextLayer[0] is the extra offset for
    // the next copy layer that uses copySplits.copySubresources[0], and
    // bufferOffsetsForNextLayer[1] is the extra offset for the next copy layer
    // that uses copySplits.copySubresources[1].
    std::array<uint64_t, TextureCopySplits::kMaxTextureCopySubresources> bufferOffsetsForNextLayer =
        {{0u, 0u}};

    for (uint32_t copyLayer = 0; copyLayer < copySize.depthOrArrayLayers; ++copyLayer) {
        const uint32_t splitIndex = copyLayer % copySplits.copySubresources.size();

        const TextureCopySubresource& copySplitPerLayerBase =
            copySplits.copySubresources[splitIndex];
        const uint64_t bufferOffsetForNextLayer = bufferOffsetsForNextLayer[splitIndex];
        const uint32_t copyTextureLayer = copyLayer + textureCopy.origin.z;

        RecordBufferTextureCopyFromSplits(direction, commandList, copySplitPerLayerBase,
                                          bufferResource, bufferOffsetForNextLayer, bytesPerRow,
                                          textureCopy.texture.Get(), textureCopy.mipLevel,
                                          copyTextureLayer, textureCopy.aspect);

        bufferOffsetsForNextLayer[splitIndex] += bytesPerLayer * copySplits.copySubresources.size();
    }
}

void Record2DBufferTextureCopyWithRelaxedOffsetAndPitch(BufferTextureCopyDirection direction,
                                                        ID3D12GraphicsCommandList* commandList,
                                                        ID3D12Resource* bufferResource,
                                                        const uint64_t offset,
                                                        const uint32_t bytesPerRow,
                                                        const uint32_t rowsPerImage,
                                                        const TextureCopy& textureCopy,
                                                        const TexelBlockInfo& blockInfo,
                                                        const Extent3D& copySize) {
    TextureCopySubresource copySubresource =
        Compute2DTextureCopySubresourceWithRelaxedRowPitchAndOffset(
            direction, textureCopy.origin, copySize, blockInfo, offset, bytesPerRow);

    uint64_t bytesPerLayer = bytesPerRow * rowsPerImage;
    uint64_t bufferOffsetForNextLayer = 0;
    for (uint32_t copyLayer = 0; copyLayer < copySize.depthOrArrayLayers; ++copyLayer) {
        uint32_t copyTextureLayer = copyLayer + textureCopy.origin.z;
        RecordBufferTextureCopyFromSplits(direction, commandList, copySubresource, bufferResource,
                                          bufferOffsetForNextLayer, bytesPerRow,
                                          textureCopy.texture.Get(), textureCopy.mipLevel,
                                          copyTextureLayer, textureCopy.aspect);
        bufferOffsetForNextLayer += bytesPerLayer;
    }
}

void RecordBufferTextureCopyWithBufferHandle(BufferTextureCopyDirection direction,
                                             ID3D12GraphicsCommandList* commandList,
                                             ID3D12Resource* bufferResource,
                                             const uint64_t offset,
                                             const uint32_t bytesPerRow,
                                             const uint32_t rowsPerImage,
                                             const TextureCopy& textureCopy,
                                             const Extent3D& copySize) {
    DAWN_ASSERT(HasOneBit(textureCopy.aspect));

    TextureBase* texture = textureCopy.texture.Get();
    const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(textureCopy.aspect).block;

    bool useRelaxedRowPitchAndOffset = texture->GetDevice()->IsToggleEnabled(
        Toggle::D3D12RelaxBufferTextureCopyPitchAndOffsetAlignment);

    switch (texture->GetDimension()) {
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();

        case wgpu::TextureDimension::e1D: {
            // 1D textures copy splits are a subset of the single-layer 2D texture copy splits,
            // at least while 1D textures can only have a single array layer.
            DAWN_ASSERT(texture->GetArrayLayers() == 1);
            TextureCopySubresource copyRegions;
            if (useRelaxedRowPitchAndOffset) {
                copyRegions = Compute2DTextureCopySubresourceWithRelaxedRowPitchAndOffset(
                    direction, textureCopy.origin, copySize, blockInfo, offset, bytesPerRow);
            } else {
                copyRegions = Compute2DTextureCopySubresource(
                    direction, textureCopy.origin, copySize, blockInfo, offset, bytesPerRow);
            }
            RecordBufferTextureCopyFromSplits(direction, commandList, copyRegions, bufferResource,
                                              0, bytesPerRow, texture, textureCopy.mipLevel, 0,
                                              textureCopy.aspect);
            break;
        }

        // Record the CopyTextureRegion commands for 2D textures, with special handling of array
        // layers since each require their own set of copies.
        case wgpu::TextureDimension::e2D:
            if (useRelaxedRowPitchAndOffset) {
                Record2DBufferTextureCopyWithRelaxedOffsetAndPitch(
                    direction, commandList, bufferResource, offset, bytesPerRow, rowsPerImage,
                    textureCopy, blockInfo, copySize);
            } else {
                Record2DBufferTextureCopyWithSplit(direction, commandList, bufferResource, offset,
                                                   bytesPerRow, rowsPerImage, textureCopy,
                                                   blockInfo, copySize);
            }
            break;

        case wgpu::TextureDimension::e3D: {
            TextureCopySubresource copyRegions;
            if (useRelaxedRowPitchAndOffset) {
                copyRegions = Compute3DTextureCopySubresourceWithRelaxedRowPitchAndOffset(
                    direction, textureCopy.origin, copySize, blockInfo, offset, bytesPerRow,
                    rowsPerImage);
            } else {
                // See comments in Compute3DTextureCopySplits() for more details.
                copyRegions =
                    Compute3DTextureCopySplits(direction, textureCopy.origin, copySize, blockInfo,
                                               offset, bytesPerRow, rowsPerImage);
            }
            RecordBufferTextureCopyFromSplits(direction, commandList, copyRegions, bufferResource,
                                              0, bytesPerRow, texture, textureCopy.mipLevel, 0,
                                              textureCopy.aspect);
            break;
        }
    }
}

void RecordBufferTextureCopy(BufferTextureCopyDirection direction,
                             ID3D12GraphicsCommandList* commandList,
                             const BufferCopy& bufferCopy,
                             const TextureCopy& textureCopy,
                             const Extent3D& copySize) {
    ID3D12Resource* bufferResource = ToBackend(bufferCopy.buffer)->GetD3D12Resource();

    if (NeedBufferSizeWorkaroundForBufferTextureCopyOnD3D12(bufferCopy, textureCopy, copySize)) {
        // Split the copy into two copies if the size of bufferCopy.buffer doesn't meet D3D12's
        // requirement and a workaround is needed:
        //   - The first copy will copy all depth images but the last depth image,
        //   - The second copy will copy the last depth image.
        Extent3D extentForAllButTheLastImage = copySize;
        extentForAllButTheLastImage.depthOrArrayLayers -= 1;
        RecordBufferTextureCopyWithBufferHandle(
            direction, commandList, bufferResource, bufferCopy.offset, bufferCopy.bytesPerRow,
            bufferCopy.rowsPerImage, textureCopy, extentForAllButTheLastImage);

        Extent3D extentForTheLastImage = copySize;
        extentForTheLastImage.depthOrArrayLayers = 1;

        TextureCopy textureCopyForTheLastImage = textureCopy;
        textureCopyForTheLastImage.origin.z += copySize.depthOrArrayLayers - 1;

        uint64_t copiedBytes =
            bufferCopy.bytesPerRow * bufferCopy.rowsPerImage * (copySize.depthOrArrayLayers - 1);
        RecordBufferTextureCopyWithBufferHandle(direction, commandList, bufferResource,
                                                bufferCopy.offset + copiedBytes,
                                                bufferCopy.bytesPerRow, bufferCopy.rowsPerImage,
                                                textureCopyForTheLastImage, extentForTheLastImage);
        return;
    }

    RecordBufferTextureCopyWithBufferHandle(direction, commandList, bufferResource,
                                            bufferCopy.offset, bufferCopy.bytesPerRow,
                                            bufferCopy.rowsPerImage, textureCopy, copySize);
}

void SetDebugName(Device* device, ID3D12Object* object, const char* prefix, std::string label) {
    if (!device->IsToggleEnabled(Toggle::UseUserDefinedLabelsInBackend)) {
        return;
    }

    if (!object) {
        return;
    }

    if (label.empty()) {
        object->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(prefix), prefix);
        return;
    }

    std::string objectName = prefix;
    objectName += "_";
    objectName += label;
    object->SetPrivateData(WKPDID_D3DDebugObjectName, objectName.length(), objectName.c_str());
}

D3D12_HEAP_TYPE GetD3D12HeapType(ResourceHeapKind resourceHeapKind) {
    switch (resourceHeapKind) {
        case ResourceHeapKind::Readback_OnlyBuffers:
        case ResourceHeapKind::Readback_AllBuffersAndTextures:
            return D3D12_HEAP_TYPE_READBACK;
        case ResourceHeapKind::Default_AllBuffersAndTextures:
        case ResourceHeapKind::Default_OnlyBuffers:
        case ResourceHeapKind::Default_OnlyNonRenderableOrDepthTextures:
        case ResourceHeapKind::Default_OnlyRenderableOrDepthTextures:
            return D3D12_HEAP_TYPE_DEFAULT;
        case ResourceHeapKind::Upload_OnlyBuffers:
        case ResourceHeapKind::Upload_AllBuffersAndTextures:
            return D3D12_HEAP_TYPE_UPLOAD;
        case ResourceHeapKind::Custom_WriteBack_OnlyBuffers:
            return D3D12_HEAP_TYPE_CUSTOM;
        case EnumCount:
            DAWN_UNREACHABLE();
    }
}

D3D12_HEAP_PROPERTIES GetD3D12HeapProperties(ResourceHeapKind resourceHeapKind) {
    D3D12_HEAP_PROPERTIES heapProperties = {};

    heapProperties.Type = GetD3D12HeapType(resourceHeapKind);

    // Now we only use `Custom_WriteBack_OnlyBuffers` resource heap on cache coherent UMA
    // architecture, where applications can more strongly entertain abandoning the attribution of
    // heaps and using the custom heap equivalent of upload heaps everywhere, and the upload heaps
    // are actually write-back on CacheCoherentUMA. See below link for more details:
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_feature_data_architecture
    if (resourceHeapKind == Custom_WriteBack_OnlyBuffers) {
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    } else {
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    }

    heapProperties.CreationNodeMask = 0;
    heapProperties.VisibleNodeMask = 0;

    return heapProperties;
}

}  // namespace dawn::native::d3d12
