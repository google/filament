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

#include "dawn/native/d3d12/CommandBufferD3D12.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/Range.h"
#include "dawn/native/BindGroupTracker.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Error.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/d3d12/BindGroupD3D12.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/ComputePipelineD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/QuerySetD3D12.h"
#include "dawn/native/d3d12/RenderPassBuilderD3D12.h"
#include "dawn/native/d3d12/RenderPipelineD3D12.h"
#include "dawn/native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/StagingDescriptorAllocatorD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn::native::d3d12 {

namespace {

DXGI_FORMAT DXGIIndexFormat(wgpu::IndexFormat format) {
    switch (format) {
        case wgpu::IndexFormat::Undefined:
            return DXGI_FORMAT_UNKNOWN;
        case wgpu::IndexFormat::Uint16:
            return DXGI_FORMAT_R16_UINT;
        case wgpu::IndexFormat::Uint32:
            return DXGI_FORMAT_R32_UINT;
    }
}

D3D12_QUERY_TYPE D3D12QueryType(wgpu::QueryType type) {
    switch (type) {
        case wgpu::QueryType::Occlusion:
            return D3D12_QUERY_TYPE_BINARY_OCCLUSION;
        case wgpu::QueryType::Timestamp:
            return D3D12_QUERY_TYPE_TIMESTAMP;
    }
}

bool CanUseCopyResource(const TextureCopy& src, const TextureCopy& dst, const Extent3D& copySize) {
    // Checked by validation
    DAWN_ASSERT(src.texture->GetSampleCount() == dst.texture->GetSampleCount());
    DAWN_ASSERT(src.texture->GetFormat().CopyCompatibleWith(dst.texture->GetFormat()));
    DAWN_ASSERT(src.aspect == dst.aspect);

    const Extent3D& srcSize = src.texture->GetSize(src.aspect);
    const Extent3D& dstSize = dst.texture->GetSize(dst.aspect);

    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-copyresource
    // In order to use D3D12's copy resource, the textures must be the same dimensions, and
    // the copy must be of the entire resource.
    return src.aspect == src.texture->GetFormat().aspects &&
           src.texture->GetDimension() == dst.texture->GetDimension() &&  //
           dst.texture->GetNumMipLevels() == 1 &&                         //
           src.texture->GetNumMipLevels() == 1 &&  // A copy command is of a single mip, so
                                                   // if a resource has more than one, we
                                                   // definitely cannot use CopyResource.
           copySize.width == dstSize.width &&      //
           copySize.width == srcSize.width &&      //
           copySize.height == dstSize.height &&    //
           copySize.height == srcSize.height &&    //
           copySize.depthOrArrayLayers == dstSize.depthOrArrayLayers &&  //
           copySize.depthOrArrayLayers == srcSize.depthOrArrayLayers;
}

bool CanUseCopyResource(CopyBufferToBufferCmd* copy) {
    return copy->sourceOffset == 0 && copy->destinationOffset == 0 &&
           copy->size == copy->source->GetSize() && copy->size == copy->destination->GetSize() &&
           copy->source->GetAllocatedSize() == copy->destination->GetAllocatedSize();
}

void RecordWriteTimestampCmd(ID3D12GraphicsCommandList* commandList,
                             QuerySetBase* querySet,
                             uint32_t queryIndex) {
    DAWN_ASSERT(D3D12QueryType(ToBackend(querySet)->GetQueryType()) == D3D12_QUERY_TYPE_TIMESTAMP);
    commandList->EndQuery(ToBackend(querySet)->GetQueryHeap(), D3D12_QUERY_TYPE_TIMESTAMP,
                          queryIndex);
}

void RecordResolveQuerySetCmd(ID3D12GraphicsCommandList* commandList,
                              Device* device,
                              QuerySet* querySet,
                              uint32_t firstQuery,
                              uint32_t queryCount,
                              Buffer* destination,
                              uint64_t destinationOffset) {
    const std::vector<bool>& availability = querySet->GetQueryAvailability();

    auto currentIt = availability.begin() + firstQuery;
    auto lastIt = availability.begin() + firstQuery + queryCount;

    // Traverse available queries in the range of [firstQuery, firstQuery +  queryCount - 1]
    while (currentIt != lastIt) {
        auto firstTrueIt = std::find(currentIt, lastIt, true);
        // No available query found for resolving
        if (firstTrueIt == lastIt) {
            break;
        }
        auto nextFalseIt = std::find(firstTrueIt, lastIt, false);

        // The query index of firstTrueIt where the resolving starts
        uint32_t resolveQueryIndex = std::distance(availability.begin(), firstTrueIt);
        // The queries count between firstTrueIt and nextFalseIt need to be resolved
        uint32_t resolveQueryCount = std::distance(firstTrueIt, nextFalseIt);

        // Calculate destinationOffset based on the current resolveQueryIndex and firstQuery
        uint32_t resolveDestinationOffset =
            destinationOffset + (resolveQueryIndex - firstQuery) * sizeof(uint64_t);

        // Resolve the queries between firstTrueIt and nextFalseIt (which is at most lastIt)
        commandList->ResolveQueryData(
            querySet->GetQueryHeap(), D3D12QueryType(querySet->GetQueryType()), resolveQueryIndex,
            resolveQueryCount, destination->GetD3D12Resource(), resolveDestinationOffset);

        // Set current iterator to next false
        currentIt = nextFalseIt;
    }
}

void RecordFirstIndexOffset(ID3D12GraphicsCommandList* commandList,
                            RenderPipeline* pipeline,
                            uint32_t firstVertex,
                            uint32_t firstInstance) {
    if (!pipeline->UsesVertexOrInstanceIndex()) {
        return;
    }
    std::array<uint32_t, 2> offsets{firstVertex, firstInstance};
    PipelineLayout* layout = ToBackend(pipeline->GetLayout());
    commandList->SetGraphicsRoot32BitConstants(layout->GetFirstIndexOffsetParameterIndex(),
                                               offsets.size(), offsets.data(), 0);
}

bool ShouldCopyUsingTemporaryBuffer(DeviceBase* device,
                                    const TextureCopy& srcCopy,
                                    const TextureCopy& dstCopy) {
    if (device->IsToggleEnabled(
            Toggle::D3D12UseTempBufferInTextureToTextureCopyBetweenDifferentDimensions) &&
        srcCopy.texture->GetDimension() != dstCopy.texture->GetDimension()) {
        return true;
    }

    if (device->IsToggleEnabled(
            Toggle::UseTempBufferInSmallFormatTextureToTextureCopyFromGreaterToLessMipLevel)) {
        bool copyToLesserLevel = srcCopy.mipLevel > dstCopy.mipLevel;
        DAWN_ASSERT(srcCopy.texture->GetFormat().CopyCompatibleWith(dstCopy.texture->GetFormat()));

        // GetAspectInfo(aspect) requires HasOneBit(aspect) == true, plus the texel block
        // sizes of depth stencil formats are always no less than 4 bytes.
        bool isSmallColorFormat =
            HasOneBit(srcCopy.aspect) &&
            srcCopy.texture->GetFormat().GetAspectInfo(srcCopy.aspect).block.byteSize < 4u;
        if (copyToLesserLevel && isSmallColorFormat) {
            return true;
        }
    }

    return false;
}

MaybeError RecordCopyTextureWithTemporaryBuffer(CommandRecordingContext* recordingContext,
                                                const TextureCopy& srcCopy,
                                                const TextureCopy& dstCopy,
                                                const Extent3D& copySize) {
    DAWN_ASSERT(srcCopy.texture->GetFormat().format == dstCopy.texture->GetFormat().format);
    DAWN_ASSERT(srcCopy.aspect == dstCopy.aspect);
    dawn::native::Format format = srcCopy.texture->GetFormat();
    const TexelBlockInfo& blockInfo = format.GetAspectInfo(srcCopy.aspect).block;
    DAWN_ASSERT(copySize.width % blockInfo.width == 0);
    uint32_t widthInBlocks = copySize.width / blockInfo.width;
    DAWN_ASSERT(copySize.height % blockInfo.height == 0);
    uint32_t heightInBlocks = copySize.height / blockInfo.height;

    // Create tempBuffer
    uint32_t bytesPerRow = Align(blockInfo.byteSize * widthInBlocks, kTextureBytesPerRowAlignment);
    uint32_t rowsPerImage = heightInBlocks;

    // The size of temporary buffer isn't needed to be a multiple of 4 because we don't
    // need to set mappedAtCreation to be true.
    auto tempBufferSize =
        ComputeRequiredBytesInCopy(blockInfo, copySize, bytesPerRow, rowsPerImage);

    BufferDescriptor tempBufferDescriptor;
    tempBufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    tempBufferDescriptor.size = tempBufferSize.AcquireSuccess();
    Device* device = ToBackend(srcCopy.texture->GetDevice());
    Ref<BufferBase> tempBufferBase;
    DAWN_TRY_ASSIGN(tempBufferBase, device->CreateBuffer(&tempBufferDescriptor));
    Ref<Buffer> tempBuffer = ToBackend(std::move(tempBufferBase));

    BufferCopy bufferCopy;
    bufferCopy.buffer = tempBuffer;
    bufferCopy.offset = 0;
    bufferCopy.bytesPerRow = bytesPerRow;
    bufferCopy.rowsPerImage = rowsPerImage;

    // Copy from source texture into tempBuffer
    tempBuffer->TrackUsageAndTransitionNow(recordingContext, wgpu::BufferUsage::CopyDst);
    RecordBufferTextureCopy(BufferTextureCopyDirection::T2B, recordingContext->GetCommandList(),
                            bufferCopy, srcCopy, copySize);

    // Copy from tempBuffer into destination texture
    tempBuffer->TrackUsageAndTransitionNow(recordingContext, wgpu::BufferUsage::CopySrc);
    RecordBufferTextureCopy(BufferTextureCopyDirection::B2T, recordingContext->GetCommandList(),
                            bufferCopy, dstCopy, copySize);

    // Save tempBuffer into recordingContext
    recordingContext->AddToTempBuffers(std::move(tempBuffer));

    return {};
}

bool ShouldCopyUsingTemporaryBuffer(DeviceBase* device,
                                    const BufferCopy& bufferCopy,
                                    const TextureCopy& textureCopy) {
    // Currently we only need the workaround for some D3D12 platforms.
    if (device->IsToggleEnabled(
            Toggle::D3D12UseTempBufferInDepthStencilTextureAndBufferCopyWithNonZeroBufferOffset)) {
        if ((ToBackend(textureCopy.texture)->GetD3D12ResourceFlags() &
             D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) &&
            bufferCopy.offset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT > 0) {
            return true;
        }
    }
    return false;
}

MaybeError RecordBufferTextureCopyWithTemporaryBuffer(CommandRecordingContext* recordingContext,
                                                      BufferTextureCopyDirection copyDirection,
                                                      const BufferCopy& bufferCopy,
                                                      const TextureCopy& textureCopy,
                                                      const Extent3D& copySize) {
    dawn::native::Format format = textureCopy.texture->GetFormat();
    const TexelBlockInfo& blockInfo = format.GetAspectInfo(textureCopy.aspect).block;

    // Create tempBuffer
    // The size of temporary buffer isn't needed to be a multiple of 4 because we don't
    // need to set mappedAtCreation to be true.
    auto tempBufferSize = ComputeRequiredBytesInCopy(blockInfo, copySize, bufferCopy.bytesPerRow,
                                                     bufferCopy.rowsPerImage);

    BufferDescriptor tempBufferDescriptor;
    tempBufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    tempBufferDescriptor.size = tempBufferSize.AcquireSuccess();
    Device* device = ToBackend(textureCopy.texture->GetDevice());
    Ref<BufferBase> tempBufferBase;
    DAWN_TRY_ASSIGN(tempBufferBase, device->CreateBuffer(&tempBufferDescriptor));
    // D3D12 aligns the entire buffer to at least 64KB, so the virtual address of tempBuffer will
    // always be aligned to D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT (512).
    Ref<Buffer> tempBuffer = ToBackend(std::move(tempBufferBase));
    DAWN_ASSERT(tempBuffer->GetVA() % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT == 0);

    BufferCopy tempBufferCopy;
    tempBufferCopy.buffer = tempBuffer;
    tempBufferCopy.offset = 0;
    tempBufferCopy.bytesPerRow = bufferCopy.bytesPerRow;
    tempBufferCopy.rowsPerImage = bufferCopy.rowsPerImage;

    tempBuffer->TrackUsageAndTransitionNow(recordingContext, wgpu::BufferUsage::CopyDst);

    ID3D12GraphicsCommandList* commandList = recordingContext->GetCommandList();
    switch (copyDirection) {
        case BufferTextureCopyDirection::B2T: {
            commandList->CopyBufferRegion(tempBuffer->GetD3D12Resource(), 0,
                                          ToBackend(bufferCopy.buffer)->GetD3D12Resource(),
                                          bufferCopy.offset, tempBufferDescriptor.size);
            tempBuffer->TrackUsageAndTransitionNow(recordingContext, wgpu::BufferUsage::CopySrc);
            RecordBufferTextureCopy(BufferTextureCopyDirection::B2T,
                                    recordingContext->GetCommandList(), tempBufferCopy, textureCopy,
                                    copySize);
            break;
        }
        case BufferTextureCopyDirection::T2B: {
            RecordBufferTextureCopy(BufferTextureCopyDirection::T2B,
                                    recordingContext->GetCommandList(), tempBufferCopy, textureCopy,
                                    copySize);
            tempBuffer->TrackUsageAndTransitionNow(recordingContext, wgpu::BufferUsage::CopySrc);
            commandList->CopyBufferRegion(ToBackend(bufferCopy.buffer)->GetD3D12Resource(),
                                          bufferCopy.offset, tempBuffer->GetD3D12Resource(), 0,
                                          tempBufferDescriptor.size);
            break;
        }
        default:
            DAWN_UNREACHABLE();
            break;
    }

    // Save tempBuffer into recordingContext
    recordingContext->AddToTempBuffers(std::move(tempBuffer));

    return {};
}

void RecordNumWorkgroupsForDispatch(ID3D12GraphicsCommandList* commandList,
                                    ComputePipeline* pipeline,
                                    DispatchCmd* dispatch) {
    if (!pipeline->UsesNumWorkgroups()) {
        return;
    }

    PipelineLayout* layout = ToBackend(pipeline->GetLayout());
    commandList->SetComputeRoot32BitConstants(layout->GetNumWorkgroupsParameterIndex(), 3, dispatch,
                                              0);
}

// Records the necessary barriers for a synchronization scope using the resource usage data
// pre-computed in the frontend. Also performs lazy initialization if required. Returns whether any
// UAV are used in the synchronization scope if `passHasUAV` is passed and no errors are hit.
MaybeError TransitionAndClearForSyncScope(CommandRecordingContext* commandContext,
                                          const SyncScopeResourceUsage& usages,
                                          bool* passHasUAV = nullptr) {
    std::vector<D3D12_RESOURCE_BARRIER> barriers;

    ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();

    wgpu::BufferUsage bufferUsages = wgpu::BufferUsage::None;

    for (size_t i = 0; i < usages.buffers.size(); ++i) {
        Buffer* buffer = ToBackend(usages.buffers[i]);

        // TODO(crbug.com/dawn/852): clear storage buffers with ClearUnorderedAccessView*().
        DAWN_TRY(buffer->EnsureDataInitialized(commandContext));

        D3D12_RESOURCE_BARRIER barrier;
        if (buffer->TrackUsageAndGetResourceBarrier(commandContext, &barrier,
                                                    usages.bufferSyncInfos[i].usage)) {
            barriers.push_back(barrier);
        }
        bufferUsages |= usages.bufferSyncInfos[i].usage;
    }

    wgpu::TextureUsage textureUsages = wgpu::TextureUsage::None;

    for (size_t i = 0; i < usages.textures.size(); ++i) {
        Texture* texture = ToBackend(usages.textures[i]);

        // Clear subresources that are not render attachments. Render attachments will be
        // cleared in RecordBeginRenderPass by setting the loadop to clear when the texture
        // subresource has not been initialized before the render pass.
        DAWN_TRY(usages.textureSyncInfos[i].Iterate(
            [&](const SubresourceRange& range, const TextureSyncInfo& syncInfo) -> MaybeError {
                if (syncInfo.usage & ~wgpu::TextureUsage::RenderAttachment) {
                    DAWN_TRY(texture->EnsureSubresourceContentInitialized(commandContext, range));
                }
                textureUsages |= syncInfo.usage;
                return {};
            }));

        ToBackend(usages.textures[i])
            ->TrackUsageAndGetResourceBarrierForPass(commandContext, &barriers,
                                                     usages.textureSyncInfos[i]);
    }

    if (barriers.size()) {
        commandList->ResourceBarrier(barriers.size(), barriers.data());
    }

    if (passHasUAV) {
        *passHasUAV =
            bufferUsages & wgpu::BufferUsage::Storage ||
            textureUsages & (wgpu::TextureUsage::StorageBinding | kWriteOnlyStorageTexture);
    }
    return {};
}

}  // anonymous namespace

class DescriptorHeapState;

template <typename PipelineType>
class BindGroupStateTracker : public BindGroupTrackerBase<false, uint64_t> {
    using Base = BindGroupTrackerBase;

  public:
    BindGroupStateTracker(Device* device, DescriptorHeapState* heapState)
        : BindGroupTrackerBase(), mDevice(device), mHeapState(heapState) {}

    MaybeError Apply(CommandRecordingContext* commandContext) {
        BeforeApply();

        ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();
        UpdateRootSignatureIfNecessary(commandList);

        auto& viewAllocator = mDevice->GetViewShaderVisibleDescriptorAllocator();
        auto& samplerAllocator = mDevice->GetSamplerShaderVisibleDescriptorAllocator();

        // Bindgroups are allocated in shader-visible descriptor heaps which are managed by a
        // ringbuffer. There can be a single shader-visible descriptor heap of each type bound
        // at any given time. This means that when we switch heaps, all other currently bound
        // bindgroups must be re-populated. Bindgroups can fail allocation gracefully which is
        // the signal to change the bounded heaps.
        // Re-populating all bindgroups after the last one fails causes duplicated allocations
        // to occur on overflow.
        bool didCreateBindGroupViews = true;
        bool didCreateBindGroupSamplers = true;
        for (BindGroupIndex index : IterateBitSet(mDirtyBindGroups)) {
            BindGroup* group = ToBackend(mBindGroups[index]);
            didCreateBindGroupViews = group->PopulateViews(viewAllocator);
            didCreateBindGroupSamplers = group->PopulateSamplers(samplerAllocator);
            if (!didCreateBindGroupViews && !didCreateBindGroupSamplers) {
                break;
            }
        }

        if (!didCreateBindGroupViews || !didCreateBindGroupSamplers) {
            if (!didCreateBindGroupViews) {
                DAWN_TRY(viewAllocator->AllocateAndSwitchShaderVisibleHeap());
            }

            if (!didCreateBindGroupSamplers) {
                DAWN_TRY(samplerAllocator->AllocateAndSwitchShaderVisibleHeap());
            }

            mDirtyBindGroupsObjectChangedOrIsDynamic |= mBindGroupLayoutsMask;
            mDirtyBindGroups |= mBindGroupLayoutsMask;

            // Must be called before applying the bindgroups.
            SetID3D12DescriptorHeaps(commandList);

            for (BindGroupIndex index : IterateBitSet(mBindGroupLayoutsMask)) {
                BindGroup* group = ToBackend(mBindGroups[index]);
                didCreateBindGroupViews = group->PopulateViews(viewAllocator);
                didCreateBindGroupSamplers = group->PopulateSamplers(samplerAllocator);
                DAWN_ASSERT(didCreateBindGroupViews);
                DAWN_ASSERT(didCreateBindGroupSamplers);
            }
        }

        for (BindGroupIndex index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
            BindGroup* group = ToBackend(mBindGroups[index]);
            ApplyBindGroup(commandList, ToBackend(mPipelineLayout), index, group,
                           mDynamicOffsets[index]);
        }

        AfterApply();

        return {};
    }

    void ResetRootSamplerTables() { mBoundRootSamplerTables = {}; }

    void SetID3D12DescriptorHeaps(ID3D12GraphicsCommandList* commandList);

  private:
    enum class RootBufferViewType { CBV, SRV, UAV };

    static constexpr bool kIsRenderPipeline = std::is_same_v<PipelineType, RenderPipeline>;
    static constexpr bool kIsComputePipeline = std::is_same_v<PipelineType, ComputePipeline>;

    void SetRootSignature(ID3D12GraphicsCommandList* commandList, PipelineLayoutBase* layout) {
        if constexpr (kIsRenderPipeline) {
            commandList->SetGraphicsRootSignature(ToBackend(layout)->GetRootSignature());
        } else {
            static_assert(kIsComputePipeline);
            commandList->SetComputeRootSignature(ToBackend(layout)->GetRootSignature());
        }
    }

    void SetRootBufferView(ID3D12GraphicsCommandList* commandList,
                           wgpu::BufferBindingType bindingType,
                           uint32_t parameterIndex,
                           const D3D12_GPU_VIRTUAL_ADDRESS& bufferLocation) {
        switch (bindingType) {
            case wgpu::BufferBindingType::Uniform:
                if constexpr (kIsRenderPipeline) {
                    commandList->SetGraphicsRootConstantBufferView(parameterIndex, bufferLocation);
                } else {
                    static_assert(kIsComputePipeline);
                    commandList->SetComputeRootConstantBufferView(parameterIndex, bufferLocation);
                }
                break;
            case wgpu::BufferBindingType::Storage:
            case kInternalStorageBufferBinding:
                if constexpr (kIsRenderPipeline) {
                    commandList->SetGraphicsRootUnorderedAccessView(parameterIndex, bufferLocation);
                } else {
                    static_assert(kIsComputePipeline);
                    commandList->SetComputeRootUnorderedAccessView(parameterIndex, bufferLocation);
                }
                break;
            case wgpu::BufferBindingType::ReadOnlyStorage:
                if constexpr (kIsRenderPipeline) {
                    commandList->SetGraphicsRootShaderResourceView(parameterIndex, bufferLocation);
                } else {
                    static_assert(kIsComputePipeline);
                    commandList->SetComputeRootShaderResourceView(parameterIndex, bufferLocation);
                }
                break;
            case wgpu::BufferBindingType::BindingNotUsed:
            case wgpu::BufferBindingType::Undefined:
                DAWN_UNREACHABLE();
        }
    }

    void SetRootDescriptorTable(ID3D12GraphicsCommandList* commandList,
                                uint32_t parameterIndex,
                                const D3D12_GPU_DESCRIPTOR_HANDLE& baseDescriptor) {
        if constexpr (kIsRenderPipeline) {
            commandList->SetGraphicsRootDescriptorTable(parameterIndex, baseDescriptor);
        } else {
            static_assert(kIsComputePipeline);
            commandList->SetComputeRootDescriptorTable(parameterIndex, baseDescriptor);
        }
    }

    void SetRootConstant(ID3D12GraphicsCommandList* commandList,
                         uint32_t parameterIndex,
                         uint32_t rootConstantsLength,
                         const void* rootConstantsData,
                         uint32_t registerOffset) {
        if constexpr (kIsRenderPipeline) {
            commandList->SetGraphicsRoot32BitConstants(parameterIndex, rootConstantsLength,
                                                       rootConstantsData, registerOffset);
        } else {
            static_assert(kIsComputePipeline);
            commandList->SetComputeRoot32BitConstants(parameterIndex, rootConstantsLength,
                                                      rootConstantsData, registerOffset);
        }
    }

    void UpdateRootSignatureIfNecessary(ID3D12GraphicsCommandList* commandList) {
        if (mLastAppliedPipelineLayout != mPipelineLayout) {
            SetRootSignature(commandList, mPipelineLayout);
            // Invalidate the root sampler tables previously set in the root signature.
            ResetRootSamplerTables();
        }
    }

    void ApplyBindGroup(ID3D12GraphicsCommandList* commandList,
                        const PipelineLayout* pipelineLayout,
                        BindGroupIndex index,
                        BindGroup* group,
                        const ityp::vector<BindingIndex, uint64_t>& dynamicOffsets) {
        DAWN_ASSERT(dynamicOffsets.size() == group->GetLayout()->GetDynamicBufferCount());

        // Usually, the application won't set the same offsets many times,
        // so always try to apply dynamic offsets even if the offsets stay the same
        if (dynamicOffsets.size() != BindingIndex(0)) {
            // Update dynamic offsets.
            // Dynamic buffer bindings are packed at the beginning of the layout.
            for (BindingIndex bindingIndex{0}; bindingIndex < dynamicOffsets.size();
                 ++bindingIndex) {
                const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);
                if (bindingInfo.visibility == wgpu::ShaderStage::None) {
                    // Skip dynamic buffers that are not visible. D3D12 does not have None
                    // visibility.
                    continue;
                }

                uint32_t parameterIndex =
                    pipelineLayout->GetDynamicRootParameterIndex(index, bindingIndex);
                BufferBinding binding = group->GetBindingAsBufferBinding(bindingIndex);

                // Calculate buffer locations that root descriptors links to. The location
                // is (base buffer location + initial offset + dynamic offset)
                uint64_t dynamicOffset = dynamicOffsets[bindingIndex];
                uint64_t offset = binding.offset + dynamicOffset;
                D3D12_GPU_VIRTUAL_ADDRESS bufferLocation =
                    ToBackend(binding.buffer)->GetVA() + offset;

                SetRootBufferView(commandList,
                                  std::get<BufferBindingInfo>(bindingInfo.bindingLayout).type,
                                  parameterIndex, bufferLocation);
            }
        }

        // It's not necessary to update descriptor tables if only the dynamic offset changed.
        if (!mDirtyBindGroups[index]) {
            return;
        }

        const uint32_t cbvUavSrvCount =
            ToBackend(group->GetLayout())->GetCbvUavSrvDescriptorCount();
        const uint32_t samplerCount = ToBackend(group->GetLayout())->GetSamplerDescriptorCount();

        if (cbvUavSrvCount > 0) {
            uint32_t parameterIndex = pipelineLayout->GetCbvUavSrvRootParameterIndex(index);
            const D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor = group->GetBaseViewDescriptor();
            SetRootDescriptorTable(commandList, parameterIndex, baseDescriptor);
        }

        if (samplerCount > 0) {
            uint32_t parameterIndex = pipelineLayout->GetSamplerRootParameterIndex(index);
            const D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor = group->GetBaseSamplerDescriptor();
            // Check if the group requires its sampler table to be set in the pipeline.
            // This because sampler heap allocations could be cached and use the same table.
            if (mBoundRootSamplerTables[index].ptr != baseDescriptor.ptr) {
                SetRootDescriptorTable(commandList, parameterIndex, baseDescriptor);
                mBoundRootSamplerTables[index] = baseDescriptor;
            }
        }

        const auto& dynamicStorageBufferLengths = group->GetDynamicStorageBufferLengths();
        if (dynamicStorageBufferLengths.size() != 0) {
            uint32_t parameterIndex =
                pipelineLayout->GetDynamicStorageBufferLengthsParameterIndex();
            uint32_t firstRegisterOffset =
                pipelineLayout->GetDynamicStorageBufferLengthInfo()[index].firstRegisterOffset;

            SetRootConstant(commandList, parameterIndex, dynamicStorageBufferLengths.size(),
                            dynamicStorageBufferLengths.data(), firstRegisterOffset);
        }
    }

    raw_ptr<Device> mDevice;
    raw_ptr<DescriptorHeapState> mHeapState;

    PerBindGroup<D3D12_GPU_DESCRIPTOR_HANDLE> mBoundRootSamplerTables = {};
};

class DescriptorHeapState {
  public:
    explicit DescriptorHeapState(Device* device)
        : mDevice(device),
          mComputeBindingTracker(device, this),
          mGraphicsBindingTracker(device, this) {}

    void SetID3D12DescriptorHeaps(ID3D12GraphicsCommandList* commandList) {
        DAWN_ASSERT(commandList != nullptr);
        std::array<ID3D12DescriptorHeap*, 2> descriptorHeaps = {
            mDevice->GetViewShaderVisibleDescriptorAllocator()->GetShaderVisibleHeap(),
            mDevice->GetSamplerShaderVisibleDescriptorAllocator()->GetShaderVisibleHeap()};
        DAWN_ASSERT(descriptorHeaps[0] != nullptr);
        DAWN_ASSERT(descriptorHeaps[1] != nullptr);
        commandList->SetDescriptorHeaps(descriptorHeaps.size(), descriptorHeaps.data());

        // Descriptor table state is undefined at the beginning of a command list and after
        // descriptor heaps are changed on a command list. Invalidate the root sampler tables to
        // reset the root descriptor table for samplers, otherwise the shader cannot access the
        // descriptor heaps.
        mComputeBindingTracker.ResetRootSamplerTables();
        mGraphicsBindingTracker.ResetRootSamplerTables();
    }

    BindGroupStateTracker<ComputePipeline>* GetComputeBindingTracker() {
        return &mComputeBindingTracker;
    }
    BindGroupStateTracker<RenderPipeline>* GetGraphicsBindingTracker() {
        return &mGraphicsBindingTracker;
    }

  private:
    raw_ptr<Device> mDevice;
    BindGroupStateTracker<ComputePipeline> mComputeBindingTracker;
    BindGroupStateTracker<RenderPipeline> mGraphicsBindingTracker;
};

template <typename PipelineType>
void BindGroupStateTracker<PipelineType>::SetID3D12DescriptorHeaps(
    ID3D12GraphicsCommandList* commandList) {
    mHeapState->SetID3D12DescriptorHeaps(commandList);
}

namespace {
class VertexBufferTracker {
  public:
    void OnSetVertexBuffer(VertexBufferSlot slot, Buffer* buffer, uint64_t offset, uint64_t size) {
        mStartSlot = std::min(mStartSlot, slot);
        mEndSlot = std::max(mEndSlot, ityp::PlusOne(slot));

        auto* d3d12BufferView = &mD3D12BufferViews[slot];
        d3d12BufferView->BufferLocation = buffer->GetVA() + offset;
        d3d12BufferView->SizeInBytes = size;
        // The bufferView stride is set based on the vertex state before a draw.
    }

    void Apply(ID3D12GraphicsCommandList* commandList, const RenderPipeline* renderPipeline) {
        DAWN_ASSERT(renderPipeline != nullptr);

        VertexBufferSlot startSlot = mStartSlot;
        VertexBufferSlot endSlot = mEndSlot;

        // If the vertex state has changed, we need to update the StrideInBytes
        // for the D3D12 buffer views. We also need to extend the dirty range to
        // touch all these slots because the stride may have changed.
        if (mLastAppliedRenderPipeline != renderPipeline) {
            mLastAppliedRenderPipeline = renderPipeline;

            for (VertexBufferSlot slot : IterateBitSet(renderPipeline->GetVertexBuffersUsed())) {
                startSlot = std::min(startSlot, slot);
                endSlot = std::max(endSlot, ityp::PlusOne(slot));
                mD3D12BufferViews[slot].StrideInBytes =
                    renderPipeline->GetVertexBuffer(slot).arrayStride;
            }
        }

        if (endSlot <= startSlot) {
            return;
        }

        // mD3D12BufferViews is kept up to date with the most recent data passed
        // to SetVertexBuffer. This makes it correct to only track the start
        // and end of the dirty range. When Apply is called,
        // we will at worst set non-dirty vertex buffers in duplicate.
        commandList->IASetVertexBuffers(static_cast<uint8_t>(startSlot),
                                        static_cast<uint8_t>(ityp::Sub(endSlot, startSlot)),
                                        &mD3D12BufferViews[startSlot]);

        mStartSlot = kMaxVertexBuffersTyped;
        mEndSlot = {};
    }

  private:
    // startSlot and endSlot indicate the range of dirty vertex buffers.
    // If there are multiple calls to SetVertexBuffer, the start and end
    // represent the union of the dirty ranges (the union may have non-dirty
    // data in the middle of the range).
    raw_ptr<const RenderPipeline> mLastAppliedRenderPipeline = nullptr;
    VertexBufferSlot mStartSlot{kMaxVertexBuffers};
    VertexBufferSlot mEndSlot{};
    PerVertexBuffer<D3D12_VERTEX_BUFFER_VIEW> mD3D12BufferViews = {};
};

void ResolveMultisampledRenderPass(CommandRecordingContext* commandContext,
                                   BeginRenderPassCmd* renderPass) {
    DAWN_ASSERT(renderPass != nullptr);

    for (ColorAttachmentIndex i :
         IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
        TextureViewBase* resolveTarget = renderPass->colorAttachments[i].resolveTarget.Get();
        if (resolveTarget == nullptr) {
            continue;
        }

        TextureViewBase* colorView = renderPass->colorAttachments[i].view.Get();
        Texture* colorTexture = ToBackend(colorView->GetTexture());
        Texture* resolveTexture = ToBackend(resolveTarget->GetTexture());

        // Transition the usages of the color attachment and resolve target.
        colorTexture->TrackUsageAndTransitionNow(
            commandContext, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, colorView->GetSubresourceRange());
        resolveTexture->TrackUsageAndTransitionNow(commandContext,
                                                   D3D12_RESOURCE_STATE_RESOLVE_DEST,
                                                   resolveTarget->GetSubresourceRange());

        // Do MSAA resolve with ResolveSubResource().
        ID3D12Resource* colorTextureHandle = colorTexture->GetD3D12Resource();
        ID3D12Resource* resolveTextureHandle = resolveTexture->GetD3D12Resource();
        const uint32_t resolveTextureSubresourceIndex = resolveTexture->GetSubresourceIndex(
            resolveTarget->GetBaseMipLevel(), resolveTarget->GetBaseArrayLayer(), Aspect::Color);
        constexpr uint32_t kColorTextureSubresourceIndex = 0;
        commandContext->GetCommandList()->ResolveSubresource(
            resolveTextureHandle, resolveTextureSubresourceIndex, colorTextureHandle,
            kColorTextureSubresourceIndex, colorTexture->GetD3D12Format());
    }
}

}  // anonymous namespace

// static
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor) {}

MaybeError CommandBuffer::RecordCommands(CommandRecordingContext* commandContext) {
    Device* device = ToBackend(GetDevice());

    DescriptorHeapState descriptorHeapState(device);
    // Make sure we use the correct descriptors for this command list. Could be done once per
    // actual command list but here is ok because there should be few command buffers.
    ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();
    descriptorHeapState.SetID3D12DescriptorHeaps(commandList);

    size_t nextComputePassNumber = 0;
    size_t nextRenderPassNumber = 0;

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::BeginComputePass: {
                BeginComputePassCmd* cmd = mCommands.NextCommand<BeginComputePassCmd>();

                DAWN_TRY(RecordComputePass(
                    commandContext, descriptorHeapState.GetComputeBindingTracker(), cmd,
                    GetResourceUsages().computePasses[nextComputePassNumber]));

                nextComputePassNumber++;
                break;
            }

            case Command::BeginRenderPass: {
                BeginRenderPassCmd* beginRenderPassCmd =
                    mCommands.NextCommand<BeginRenderPassCmd>();

                bool passHasUAV;
                DAWN_TRY(TransitionAndClearForSyncScope(
                    commandContext, GetResourceUsages().renderPasses[nextRenderPassNumber],
                    &passHasUAV));

                LazyClearRenderPassAttachments(beginRenderPassCmd);
                DAWN_TRY(RecordRenderPass(commandContext,
                                          descriptorHeapState.GetGraphicsBindingTracker(),
                                          beginRenderPassCmd, passHasUAV));

                nextRenderPassNumber++;
                break;
            }

            case Command::CopyBufferToBuffer: {
                CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                if (copy->size == 0) {
                    // Skip no-op copies.
                    break;
                }
                Buffer* srcBuffer = ToBackend(copy->source.Get());
                Buffer* dstBuffer = ToBackend(copy->destination.Get());

                DAWN_TRY(srcBuffer->EnsureDataInitialized(commandContext));
                [[maybe_unused]] bool cleared;
                DAWN_TRY_ASSIGN(cleared, dstBuffer->EnsureDataInitializedAsDestination(
                                             commandContext, copy->destinationOffset, copy->size));

                srcBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopySrc);
                dstBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopyDst);

                if (CanUseCopyResource(copy)) {
                    commandList->CopyResource(dstBuffer->GetD3D12Resource(),
                                              srcBuffer->GetD3D12Resource());
                } else {
                    commandList->CopyBufferRegion(
                        dstBuffer->GetD3D12Resource(), copy->destinationOffset,
                        srcBuffer->GetD3D12Resource(), copy->sourceOffset, copy->size);
                }
                break;
            }

            case Command::CopyBufferToTexture: {
                CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }
                Buffer* buffer = ToBackend(copy->source.buffer.Get());
                Texture* texture = ToBackend(copy->destination.texture.Get());

                DAWN_TRY(buffer->EnsureDataInitialized(commandContext));

                SubresourceRange subresources =
                    GetSubresourcesAffectedByCopy(copy->destination, copy->copySize);

                if (IsCompleteSubresourceCopiedTo(texture, copy->copySize,
                                                  copy->destination.mipLevel,
                                                  copy->destination.aspect)) {
                    texture->SetIsSubresourceContentInitialized(true, subresources);
                } else {
                    DAWN_TRY(
                        texture->EnsureSubresourceContentInitialized(commandContext, subresources));
                }

                buffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopySrc);
                texture->TrackUsageAndTransitionNow(commandContext, wgpu::TextureUsage::CopyDst,
                                                    subresources);

                if (ShouldCopyUsingTemporaryBuffer(GetDevice(), copy->source, copy->destination)) {
                    DAWN_TRY(RecordBufferTextureCopyWithTemporaryBuffer(
                        commandContext, BufferTextureCopyDirection::B2T, copy->source,
                        copy->destination, copy->copySize));
                    break;
                }
                RecordBufferTextureCopy(BufferTextureCopyDirection::B2T, commandList, copy->source,
                                        copy->destination, copy->copySize);

                break;
            }

            case Command::CopyTextureToBuffer: {
                CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }
                Texture* texture = ToBackend(copy->source.texture.Get());
                Buffer* buffer = ToBackend(copy->destination.buffer.Get());

                DAWN_TRY(buffer->EnsureDataInitializedAsDestination(commandContext, copy));

                SubresourceRange subresources =
                    GetSubresourcesAffectedByCopy(copy->source, copy->copySize);

                DAWN_TRY(
                    texture->EnsureSubresourceContentInitialized(commandContext, subresources));

                texture->TrackUsageAndTransitionNow(commandContext, wgpu::TextureUsage::CopySrc,
                                                    subresources);
                buffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopyDst);

                if (ShouldCopyUsingTemporaryBuffer(GetDevice(), copy->destination, copy->source)) {
                    DAWN_TRY(RecordBufferTextureCopyWithTemporaryBuffer(
                        commandContext, BufferTextureCopyDirection::T2B, copy->destination,
                        copy->source, copy->copySize));
                    break;
                }
                RecordBufferTextureCopy(BufferTextureCopyDirection::T2B, commandList,
                                        copy->destination, copy->source, copy->copySize);

                break;
            }

            case Command::CopyTextureToTexture: {
                CopyTextureToTextureCmd* copy = mCommands.NextCommand<CopyTextureToTextureCmd>();
                if (copy->copySize.width == 0 || copy->copySize.height == 0 ||
                    copy->copySize.depthOrArrayLayers == 0) {
                    // Skip no-op copies.
                    continue;
                }
                Texture* source = ToBackend(copy->source.texture.Get());
                Texture* destination = ToBackend(copy->destination.texture.Get());

                SubresourceRange srcRange =
                    GetSubresourcesAffectedByCopy(copy->source, copy->copySize);
                SubresourceRange dstRange =
                    GetSubresourcesAffectedByCopy(copy->destination, copy->copySize);

                DAWN_TRY(source->EnsureSubresourceContentInitialized(commandContext, srcRange));
                if (IsCompleteSubresourceCopiedTo(destination, copy->copySize,
                                                  copy->destination.mipLevel,
                                                  copy->destination.aspect)) {
                    destination->SetIsSubresourceContentInitialized(true, dstRange);
                } else {
                    DAWN_TRY(
                        destination->EnsureSubresourceContentInitialized(commandContext, dstRange));
                }

                if (copy->source.texture.Get() == copy->destination.texture.Get() &&
                    copy->source.mipLevel == copy->destination.mipLevel) {
                    // When there are overlapped subresources, the layout of the overlapped
                    // subresources should all be COMMON instead of what we set now. Currently
                    // it is not allowed to copy with overlapped subresources, but we still
                    // add the DAWN_ASSERT here as a reminder for this possible misuse.
                    DAWN_ASSERT(!IsRangeOverlapped(copy->source.origin.z,
                                                   copy->destination.origin.z,
                                                   copy->copySize.depthOrArrayLayers));
                }
                source->TrackUsageAndTransitionNow(commandContext, wgpu::TextureUsage::CopySrc,
                                                   srcRange);
                destination->TrackUsageAndTransitionNow(commandContext, wgpu::TextureUsage::CopyDst,
                                                        dstRange);

                DAWN_ASSERT(srcRange.aspects == dstRange.aspects);
                if (ShouldCopyUsingTemporaryBuffer(GetDevice(), copy->source, copy->destination)) {
                    DAWN_TRY(RecordCopyTextureWithTemporaryBuffer(
                        commandContext, copy->source, copy->destination, copy->copySize));
                    break;
                }

                if (CanUseCopyResource(copy->source, copy->destination, copy->copySize)) {
                    commandList->CopyResource(destination->GetD3D12Resource(),
                                              source->GetD3D12Resource());
                } else if (source->GetDimension() == wgpu::TextureDimension::e3D &&
                           destination->GetDimension() == wgpu::TextureDimension::e3D) {
                    for (Aspect aspect : IterateEnumMask(srcRange.aspects)) {
                        D3D12_TEXTURE_COPY_LOCATION srcLocation =
                            ComputeTextureCopyLocationForTexture(source, copy->source.mipLevel, 0,
                                                                 aspect);
                        D3D12_TEXTURE_COPY_LOCATION dstLocation =
                            ComputeTextureCopyLocationForTexture(
                                destination, copy->destination.mipLevel, 0, aspect);

                        D3D12_BOX sourceRegion =
                            ComputeD3D12BoxFromOffsetAndSize(copy->source.origin, copy->copySize);

                        commandList->CopyTextureRegion(
                            &dstLocation, copy->destination.origin.x, copy->destination.origin.y,
                            copy->destination.origin.z, &srcLocation, &sourceRegion);
                    }
                } else {
                    const dawn::native::Extent3D copyExtentOneSlice = {copy->copySize.width,
                                                                       copy->copySize.height, 1u};

                    for (Aspect aspect : IterateEnumMask(srcRange.aspects)) {
                        for (uint32_t z = 0; z < copy->copySize.depthOrArrayLayers; ++z) {
                            uint32_t sourceLayer = 0;
                            uint32_t sourceZ = 0;
                            switch (source->GetDimension()) {
                                case wgpu::TextureDimension::Undefined:
                                    DAWN_UNREACHABLE();
                                case wgpu::TextureDimension::e1D:
                                    DAWN_ASSERT(copy->source.origin.z == 0);
                                    break;
                                case wgpu::TextureDimension::e2D:
                                    sourceLayer = copy->source.origin.z + z;
                                    break;
                                case wgpu::TextureDimension::e3D:
                                    sourceZ = copy->source.origin.z + z;
                                    break;
                            }

                            uint32_t destinationLayer = 0;
                            uint32_t destinationZ = 0;
                            switch (destination->GetDimension()) {
                                case wgpu::TextureDimension::Undefined:
                                    DAWN_UNREACHABLE();
                                case wgpu::TextureDimension::e1D:
                                    DAWN_ASSERT(copy->destination.origin.z == 0);
                                    break;
                                case wgpu::TextureDimension::e2D:
                                    destinationLayer = copy->destination.origin.z + z;
                                    break;
                                case wgpu::TextureDimension::e3D:
                                    destinationZ = copy->destination.origin.z + z;
                                    break;
                            }
                            D3D12_TEXTURE_COPY_LOCATION srcLocation =
                                ComputeTextureCopyLocationForTexture(source, copy->source.mipLevel,
                                                                     sourceLayer, aspect);

                            D3D12_TEXTURE_COPY_LOCATION dstLocation =
                                ComputeTextureCopyLocationForTexture(destination,
                                                                     copy->destination.mipLevel,
                                                                     destinationLayer, aspect);

                            Origin3D sourceOriginInSubresource = copy->source.origin;
                            sourceOriginInSubresource.z = sourceZ;
                            D3D12_BOX sourceRegion = ComputeD3D12BoxFromOffsetAndSize(
                                sourceOriginInSubresource, copyExtentOneSlice);

                            commandList->CopyTextureRegion(&dstLocation, copy->destination.origin.x,
                                                           copy->destination.origin.y, destinationZ,
                                                           &srcLocation, &sourceRegion);
                        }
                    }
                }
                break;
            }

            case Command::ClearBuffer: {
                ClearBufferCmd* cmd = mCommands.NextCommand<ClearBufferCmd>();
                if (cmd->size == 0) {
                    // Skip no-op fills.
                    break;
                }
                Buffer* dstBuffer = ToBackend(cmd->buffer.Get());

                bool clearedToZero;
                DAWN_TRY_ASSIGN(clearedToZero, dstBuffer->EnsureDataInitializedAsDestination(
                                                   commandContext, cmd->offset, cmd->size));

                if (!clearedToZero) {
                    DAWN_TRY(device->ClearBufferToZero(commandContext, cmd->buffer.Get(),
                                                       cmd->offset, cmd->size));
                }

                break;
            }

            case Command::ResolveQuerySet: {
                ResolveQuerySetCmd* cmd = mCommands.NextCommand<ResolveQuerySetCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());
                uint32_t firstQuery = cmd->firstQuery;
                uint32_t queryCount = cmd->queryCount;
                Buffer* destination = ToBackend(cmd->destination.Get());
                uint64_t destinationOffset = cmd->destinationOffset;

                [[maybe_unused]] bool cleared;
                DAWN_TRY_ASSIGN(
                    cleared, destination->EnsureDataInitializedAsDestination(
                                 commandContext, destinationOffset, queryCount * sizeof(uint64_t)));

                // Resolving unavailable queries is undefined behaviour on D3D12, we only can
                // resolve the available part of sparse queries. In order to resolve the
                // unavailables as 0s, we need to clear the resolving region of the destination
                // buffer to 0s.
                auto startIt = querySet->GetQueryAvailability().begin() + firstQuery;
                auto endIt = querySet->GetQueryAvailability().begin() + firstQuery + queryCount;
                bool hasUnavailableQueries = std::find(startIt, endIt, false) != endIt;
                // Workaround for resolving overlapping queries to a same buffer on Intel Gen12 GPUs
                // due to D3D12 driver issue.
                // See http://crbug.com/dawn/1546 for more information.
                bool clearNeeded = device->IsToggleEnabled(Toggle::ClearBufferBeforeResolveQueries);
                if (hasUnavailableQueries || clearNeeded) {
                    DAWN_TRY(device->ClearBufferToZero(commandContext, destination,
                                                       destinationOffset,
                                                       queryCount * sizeof(uint64_t)));
                }

                destination->TrackUsageAndTransitionNow(commandContext,
                                                        wgpu::BufferUsage::QueryResolve);

                RecordResolveQuerySetCmd(commandList, device, querySet, firstQuery, queryCount,
                                         destination, destinationOffset);

                break;
            }

            case Command::WriteTimestamp: {
                WriteTimestampCmd* cmd = mCommands.NextCommand<WriteTimestampCmd>();

                RecordWriteTimestampCmd(commandList, cmd->querySet.Get(), cmd->queryIndex);
                break;
            }

            case Command::InsertDebugMarker: {
                InsertDebugMarkerCmd* cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
                const char* label = mCommands.NextData<char>(cmd->length + 1);

                if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                    // PIX color is 1 byte per channel in ARGB format
                    constexpr uint64_t kPIXBlackColor = 0xff000000;
                    ToBackend(GetDevice())
                        ->GetFunctions()
                        ->pixSetMarkerOnCommandList(commandList, kPIXBlackColor, label);
                }
                break;
            }

            case Command::PopDebugGroup: {
                mCommands.NextCommand<PopDebugGroupCmd>();

                if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                    ToBackend(GetDevice())->GetFunctions()->pixEndEventOnCommandList(commandList);
                }
                break;
            }

            case Command::PushDebugGroup: {
                PushDebugGroupCmd* cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                const char* label = mCommands.NextData<char>(cmd->length + 1);

                if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                    // PIX color is 1 byte per channel in ARGB format
                    constexpr uint64_t kPIXBlackColor = 0xff000000;
                    ToBackend(GetDevice())
                        ->GetFunctions()
                        ->pixBeginEventOnCommandList(commandList, kPIXBlackColor, label);
                }
                break;
            }

            case Command::WriteBuffer: {
                WriteBufferCmd* write = mCommands.NextCommand<WriteBufferCmd>();
                const uint64_t offset = write->offset;
                const uint64_t size = write->size;
                if (size == 0) {
                    continue;
                }

                Buffer* dstBuffer = ToBackend(write->buffer.Get());
                uint8_t* data = mCommands.NextData<uint8_t>(size);

                UploadHandle uploadHandle;
                DAWN_TRY_ASSIGN(uploadHandle,
                                device->GetDynamicUploader()->Allocate(
                                    size, device->GetQueue()->GetPendingCommandSerial(),
                                    kCopyBufferToBufferOffsetAlignment));
                DAWN_ASSERT(uploadHandle.mappedBuffer != nullptr);
                memcpy(uploadHandle.mappedBuffer, data, size);

                [[maybe_unused]] bool cleared;
                DAWN_TRY_ASSIGN(cleared, dstBuffer->EnsureDataInitializedAsDestination(
                                             commandContext, offset, size));

                dstBuffer->TrackUsageAndTransitionNow(commandContext, wgpu::BufferUsage::CopyDst);
                commandList->CopyBufferRegion(
                    dstBuffer->GetD3D12Resource(), offset,
                    ToBackend(uploadHandle.stagingBuffer)->GetD3D12Resource(),
                    uploadHandle.startOffset, size);
                break;
            }

            default:
                DAWN_UNREACHABLE();
        }
    }

    return {};
}

MaybeError CommandBuffer::RecordComputePass(CommandRecordingContext* commandContext,
                                            BindGroupStateTracker<ComputePipeline>* bindingTracker,
                                            BeginComputePassCmd* computePass,
                                            const ComputePassResourceUsage& resourceUsages) {
    uint64_t currentDispatch = 0;
    ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();

    // Write timestamp at the beginning of compute pass if it's set.
    if (computePass->timestampWrites.beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
        RecordWriteTimestampCmd(commandList, computePass->timestampWrites.querySet.Get(),
                                computePass->timestampWrites.beginningOfPassWriteIndex);
    }

    Command type;
    ComputePipeline* lastPipeline = nullptr;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::Dispatch: {
                DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();

                // Skip noop dispatches, it can cause D3D12 warning from validation layers and
                // leads to device lost.
                if (dispatch->x == 0 || dispatch->y == 0 || dispatch->z == 0) {
                    break;
                }

                DAWN_TRY(TransitionAndClearForSyncScope(
                    commandContext, resourceUsages.dispatchUsages[currentDispatch]));
                DAWN_TRY(bindingTracker->Apply(commandContext));

                RecordNumWorkgroupsForDispatch(commandList, lastPipeline, dispatch);
                commandList->Dispatch(dispatch->x, dispatch->y, dispatch->z);
                currentDispatch++;
                break;
            }

            case Command::DispatchIndirect: {
                DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();

                DAWN_TRY(TransitionAndClearForSyncScope(
                    commandContext, resourceUsages.dispatchUsages[currentDispatch]));
                DAWN_TRY(bindingTracker->Apply(commandContext));

                ComPtr<ID3D12CommandSignature> signature =
                    lastPipeline->GetDispatchIndirectCommandSignature();
                commandList->ExecuteIndirect(
                    signature.Get(), 1, ToBackend(dispatch->indirectBuffer)->GetD3D12Resource(),
                    dispatch->indirectOffset, nullptr, 0);
                currentDispatch++;
                break;
            }

            case Command::EndComputePass: {
                mCommands.NextCommand<EndComputePassCmd>();

                // Write timestamp at the end of compute pass if it's set.
                if (computePass->timestampWrites.endOfPassWriteIndex !=
                    wgpu::kQuerySetIndexUndefined) {
                    RecordWriteTimestampCmd(commandList,
                                            computePass->timestampWrites.querySet.Get(),
                                            computePass->timestampWrites.endOfPassWriteIndex);
                }
                return {};
            }

            case Command::SetComputePipeline: {
                SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                ComputePipeline* pipeline = ToBackend(cmd->pipeline).Get();

                commandList->SetPipelineState(pipeline->GetPipelineState());

                bindingTracker->OnSetPipeline(pipeline);
                lastPipeline = pipeline;
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                BindGroup* group = ToBackend(cmd->group.Get());
                uint32_t* dynamicOffsets = nullptr;

                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = mCommands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }

                bindingTracker->OnSetBindGroup(cmd->index, group, cmd->dynamicOffsetCount,
                                               dynamicOffsets);
                break;
            }

            case Command::InsertDebugMarker: {
                InsertDebugMarkerCmd* cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
                const char* label = mCommands.NextData<char>(cmd->length + 1);

                if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                    // PIX color is 1 byte per channel in ARGB format
                    constexpr uint64_t kPIXBlackColor = 0xff000000;
                    ToBackend(GetDevice())
                        ->GetFunctions()
                        ->pixSetMarkerOnCommandList(commandList, kPIXBlackColor, label);
                }
                break;
            }

            case Command::PopDebugGroup: {
                mCommands.NextCommand<PopDebugGroupCmd>();

                if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                    ToBackend(GetDevice())->GetFunctions()->pixEndEventOnCommandList(commandList);
                }
                break;
            }

            case Command::PushDebugGroup: {
                PushDebugGroupCmd* cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                const char* label = mCommands.NextData<char>(cmd->length + 1);

                if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                    // PIX color is 1 byte per channel in ARGB format
                    constexpr uint64_t kPIXBlackColor = 0xff000000;
                    ToBackend(GetDevice())
                        ->GetFunctions()
                        ->pixBeginEventOnCommandList(commandList, kPIXBlackColor, label);
                }
                break;
            }

            case Command::WriteTimestamp: {
                WriteTimestampCmd* cmd = mCommands.NextCommand<WriteTimestampCmd>();

                RecordWriteTimestampCmd(commandList, cmd->querySet.Get(), cmd->queryIndex);
                break;
            }

            default:
                DAWN_UNREACHABLE();
        }
    }

    return {};
}

MaybeError CommandBuffer::SetupRenderPass(CommandRecordingContext* commandContext,
                                          BeginRenderPassCmd* renderPass,
                                          RenderPassBuilder* renderPassBuilder) {
    Device* device = ToBackend(GetDevice());

    CPUDescriptorHeapAllocation nullRTVAllocation;
    D3D12_CPU_DESCRIPTOR_HANDLE nullRTV;

    const auto& colorAttachmentsMaskBitSet = renderPass->attachmentState->GetColorAttachmentsMask();
    for (auto i : Range(kMaxColorAttachmentsTyped)) {
        if (colorAttachmentsMaskBitSet.test(i)) {
            RenderPassColorAttachmentInfo& attachmentInfo = renderPass->colorAttachments[i];
            TextureView* view = ToBackend(attachmentInfo.view.Get());

            // Set view attachment.
            CPUDescriptorHeapAllocation rtvAllocation;
            DAWN_TRY_ASSIGN(
                rtvAllocation,
                device->GetRenderTargetViewAllocator()->AllocateTransientCPUDescriptors());

            D3D12_RENDER_TARGET_VIEW_DESC viewDesc =
                view->GetRTVDescriptor(attachmentInfo.depthSlice);
            const D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor = rtvAllocation.GetBaseDescriptor();

            device->GetD3D12Device()->CreateRenderTargetView(
                ToBackend(view->GetTexture())->GetD3D12Resource(), &viewDesc, baseDescriptor);

            renderPassBuilder->SetRenderTargetView(i, baseDescriptor, false);

            // Set color load operation.
            renderPassBuilder->SetRenderTargetBeginningAccess(
                i, attachmentInfo.loadOp, attachmentInfo.clearColor, view->GetD3D12Format());

            // Set color store operation.
            if (attachmentInfo.resolveTarget != nullptr) {
                TextureView* resolveDestinationView = ToBackend(attachmentInfo.resolveTarget.Get());
                Texture* resolveDestinationTexture =
                    ToBackend(resolveDestinationView->GetTexture());

                resolveDestinationTexture->TrackUsageAndTransitionNow(
                    commandContext, D3D12_RESOURCE_STATE_RESOLVE_DEST,
                    resolveDestinationView->GetSubresourceRange());

                renderPassBuilder->SetRenderTargetEndingAccessResolve(i, attachmentInfo.storeOp,
                                                                      view, resolveDestinationView);
            } else {
                renderPassBuilder->SetRenderTargetEndingAccess(i, attachmentInfo.storeOp);
            }
        } else {
            if (!nullRTVAllocation.IsValid()) {
                DAWN_TRY_ASSIGN(
                    nullRTVAllocation,
                    device->GetRenderTargetViewAllocator()->AllocateTransientCPUDescriptors());
                nullRTV = nullRTVAllocation.GetBaseDescriptor();
                D3D12_RENDER_TARGET_VIEW_DESC nullRTVDesc;
                nullRTVDesc.Format = GetNullRTVDXGIFormatForD3D12RenderPass();
                nullRTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                nullRTVDesc.Texture2D.MipSlice = 0;
                nullRTVDesc.Texture2D.PlaneSlice = 0;
                device->GetD3D12Device()->CreateRenderTargetView(nullptr, &nullRTVDesc, nullRTV);
            }

            renderPassBuilder->SetRenderTargetView(i, nullRTV, true);
        }
    }

    if (renderPass->attachmentState->HasDepthStencilAttachment()) {
        RenderPassDepthStencilAttachmentInfo& attachmentInfo = renderPass->depthStencilAttachment;
        TextureView* view = ToBackend(renderPass->depthStencilAttachment.view.Get());

        // Set depth attachment.
        CPUDescriptorHeapAllocation dsvAllocation;
        DAWN_TRY_ASSIGN(dsvAllocation,
                        device->GetDepthStencilViewAllocator()->AllocateTransientCPUDescriptors());

        const D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc =
            view->GetDSVDescriptor(attachmentInfo.depthReadOnly, attachmentInfo.stencilReadOnly);
        const D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor = dsvAllocation.GetBaseDescriptor();

        device->GetD3D12Device()->CreateDepthStencilView(
            ToBackend(view->GetTexture())->GetD3D12Resource(), &viewDesc, baseDescriptor);

        renderPassBuilder->SetDepthStencilView(baseDescriptor, attachmentInfo.depthReadOnly,
                                               attachmentInfo.stencilReadOnly);

        const bool hasDepth = view->GetTexture()->GetFormat().HasDepth();
        const bool hasStencil = view->GetTexture()->GetFormat().HasStencil();

        // Set depth/stencil load operations.
        if (hasDepth) {
            renderPassBuilder->SetDepthAccess(attachmentInfo.depthLoadOp,
                                              attachmentInfo.depthStoreOp,
                                              attachmentInfo.clearDepth, view->GetD3D12Format());
        } else {
            renderPassBuilder->SetDepthNoAccess();
        }

        if (hasStencil) {
            renderPassBuilder->SetStencilAccess(
                attachmentInfo.stencilLoadOp, attachmentInfo.stencilStoreOp,
                attachmentInfo.clearStencil, view->GetD3D12Format());
        } else {
            renderPassBuilder->SetStencilNoAccess();
        }

    } else {
        renderPassBuilder->SetDepthStencilNoAccess();
    }

    return {};
}

void CommandBuffer::EmulateBeginRenderPass(CommandRecordingContext* commandContext,
                                           const RenderPassBuilder* renderPassBuilder) const {
    ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();

    // Clear framebuffer attachments as needed.
    {
        for (const auto& attachment : renderPassBuilder->GetRenderPassRenderTargetDescriptors()) {
            // Load op - color
            if (attachment.cpuDescriptor.ptr != 0 &&
                attachment.BeginningAccess.Type == D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR) {
                commandList->ClearRenderTargetView(
                    attachment.cpuDescriptor, attachment.BeginningAccess.Clear.ClearValue.Color, 0,
                    nullptr);
            }
        }

        if (renderPassBuilder->HasDepthOrStencil()) {
            D3D12_CLEAR_FLAGS clearFlags = {};
            float depthClear = 0.0f;
            uint8_t stencilClear = 0u;

            if (renderPassBuilder->GetRenderPassDepthStencilDescriptor()
                    ->DepthBeginningAccess.Type == D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR) {
                clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                depthClear = renderPassBuilder->GetRenderPassDepthStencilDescriptor()
                                 ->DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth;
            }
            if (renderPassBuilder->GetRenderPassDepthStencilDescriptor()
                    ->StencilBeginningAccess.Type ==
                D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR) {
                clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                stencilClear = renderPassBuilder->GetRenderPassDepthStencilDescriptor()
                                   ->StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil;
            }

            if (clearFlags) {
                commandList->ClearDepthStencilView(
                    renderPassBuilder->GetRenderPassDepthStencilDescriptor()->cpuDescriptor,
                    clearFlags, depthClear, stencilClear, 0, nullptr);
            }
        }
    }

    commandList->OMSetRenderTargets(
        static_cast<uint8_t>(renderPassBuilder->GetHighestColorAttachmentIndexPlusOne()),
        renderPassBuilder->GetRenderTargetViews(), FALSE,
        renderPassBuilder->HasDepthOrStencil()
            ? &renderPassBuilder->GetRenderPassDepthStencilDescriptor()->cpuDescriptor
            : nullptr);
}

MaybeError CommandBuffer::RecordRenderPass(CommandRecordingContext* commandContext,
                                           BindGroupStateTracker<RenderPipeline>* bindingTracker,
                                           BeginRenderPassCmd* renderPass,
                                           const bool passHasUAV) {
    Device* device = ToBackend(GetDevice());
    const bool useRenderPass = device->IsToggleEnabled(Toggle::UseD3D12RenderPass);

    // renderPassBuilder must be scoped to RecordRenderPass because any underlying
    // D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS structs must remain
    // valid until after EndRenderPass() has been called.
    RenderPassBuilder renderPassBuilder(passHasUAV);

    DAWN_TRY(SetupRenderPass(commandContext, renderPass, &renderPassBuilder));

    // Use D3D12's native render pass API if it's available, otherwise emulate the
    // beginning and ending access operations.
    if (useRenderPass) {
        commandContext->GetCommandList4()->BeginRenderPass(
            static_cast<uint8_t>(renderPassBuilder.GetHighestColorAttachmentIndexPlusOne()),
            renderPassBuilder.GetRenderPassRenderTargetDescriptors().data(),
            renderPassBuilder.HasDepthOrStencil()
                ? renderPassBuilder.GetRenderPassDepthStencilDescriptor()
                : nullptr,
            renderPassBuilder.GetRenderPassFlags());
    } else {
        EmulateBeginRenderPass(commandContext, &renderPassBuilder);
    }

    ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();

    // Write timestamp at the beginning of render pass if it's set.
    if (renderPass->timestampWrites.beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
        RecordWriteTimestampCmd(commandList, renderPass->timestampWrites.querySet.Get(),
                                renderPass->timestampWrites.beginningOfPassWriteIndex);
    }

    // Set up default dynamic state
    {
        uint32_t width = renderPass->width;
        uint32_t height = renderPass->height;
        D3D12_VIEWPORT viewport = {0.f, 0.f, static_cast<float>(width), static_cast<float>(height),
                                   0.f, 1.f};
        D3D12_RECT scissorRect = {0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height)};
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissorRect);

        static constexpr std::array<float, 4> defaultBlendFactor = {0, 0, 0, 0};
        commandList->OMSetBlendFactor(&defaultBlendFactor[0]);

        commandList->OMSetStencilRef(0);
    }

    RenderPipeline* lastPipeline = nullptr;
    VertexBufferTracker vertexBufferTracker = {};

    auto EncodeRenderBundleCommand = [&](CommandIterator* iter, Command type) -> MaybeError {
        switch (type) {
            case Command::Draw: {
                DrawCmd* draw = iter->NextCommand<DrawCmd>();

                DAWN_TRY(bindingTracker->Apply(commandContext));
                vertexBufferTracker.Apply(commandList, lastPipeline);
                RecordFirstIndexOffset(commandList, lastPipeline, draw->firstVertex,
                                       draw->firstInstance);
                commandList->DrawInstanced(draw->vertexCount, draw->instanceCount,
                                           draw->firstVertex, draw->firstInstance);
                break;
            }

            case Command::DrawIndexed: {
                DrawIndexedCmd* draw = iter->NextCommand<DrawIndexedCmd>();

                DAWN_TRY(bindingTracker->Apply(commandContext));
                vertexBufferTracker.Apply(commandList, lastPipeline);
                RecordFirstIndexOffset(commandList, lastPipeline, draw->baseVertex,
                                       draw->firstInstance);
                commandList->DrawIndexedInstanced(draw->indexCount, draw->instanceCount,
                                                  draw->firstIndex, draw->baseVertex,
                                                  draw->firstInstance);
                break;
            }

            case Command::DrawIndirect: {
                DrawIndirectCmd* draw = iter->NextCommand<DrawIndirectCmd>();

                DAWN_TRY(bindingTracker->Apply(commandContext));
                vertexBufferTracker.Apply(commandList, lastPipeline);

                Buffer* buffer = ToBackend(draw->indirectBuffer.Get());
                ComPtr<ID3D12CommandSignature> signature =
                    lastPipeline->GetDrawIndirectCommandSignature();
                commandList->ExecuteIndirect(signature.Get(), 1, buffer->GetD3D12Resource(),
                                             draw->indirectOffset, nullptr, 0);
                break;
            }

            case Command::DrawIndexedIndirect: {
                DrawIndexedIndirectCmd* draw = iter->NextCommand<DrawIndexedIndirectCmd>();

                DAWN_TRY(bindingTracker->Apply(commandContext));
                vertexBufferTracker.Apply(commandList, lastPipeline);

                Buffer* buffer = ToBackend(draw->indirectBuffer.Get());
                DAWN_ASSERT(buffer != nullptr);

                ComPtr<ID3D12CommandSignature> signature =
                    lastPipeline->GetDrawIndexedIndirectCommandSignature();
                commandList->ExecuteIndirect(signature.Get(), 1, buffer->GetD3D12Resource(),
                                             draw->indirectOffset, nullptr, 0);
                break;
            }

            case Command::MultiDrawIndirect: {
                MultiDrawIndirectCmd* draw = iter->NextCommand<MultiDrawIndirectCmd>();

                DAWN_TRY(bindingTracker->Apply(commandContext));
                vertexBufferTracker.Apply(commandList, lastPipeline);

                Buffer* indirectBuffer = ToBackend(draw->indirectBuffer.Get());
                DAWN_ASSERT(indirectBuffer != nullptr);

                Buffer* countBuffer = ToBackend(draw->drawCountBuffer.Get());

                // There is no distinction between DrawIndirect and MultiDrawIndirect in D3D12.
                // This is why we can use the same command signature for both.
                ComPtr<ID3D12CommandSignature> signature =
                    lastPipeline->GetDrawIndirectCommandSignature();

                commandList->ExecuteIndirect(
                    signature.Get(), draw->maxDrawCount, indirectBuffer->GetD3D12Resource(),
                    draw->indirectOffset,
                    countBuffer != nullptr ? countBuffer->GetD3D12Resource() : nullptr,
                    countBuffer != nullptr ? draw->drawCountOffset : 0);

                break;
            }

            case Command::MultiDrawIndexedIndirect: {
                MultiDrawIndexedIndirectCmd* draw =
                    iter->NextCommand<MultiDrawIndexedIndirectCmd>();

                DAWN_TRY(bindingTracker->Apply(commandContext));
                vertexBufferTracker.Apply(commandList, lastPipeline);

                Buffer* indirectBuffer = ToBackend(draw->indirectBuffer.Get());
                DAWN_ASSERT(indirectBuffer != nullptr);

                Buffer* countBuffer = ToBackend(draw->drawCountBuffer.Get());

                // There is no distinction between DrawIndexedIndirect and MultiDrawIndexedIndirect
                // in D3D12. This is why we can use the same command signature for both.
                ComPtr<ID3D12CommandSignature> signature =
                    lastPipeline->GetDrawIndexedIndirectCommandSignature();

                commandList->ExecuteIndirect(
                    signature.Get(), draw->maxDrawCount, indirectBuffer->GetD3D12Resource(),
                    draw->indirectOffset,
                    countBuffer != nullptr ? countBuffer->GetD3D12Resource() : nullptr,
                    countBuffer != nullptr ? draw->drawCountOffset : 0);
                break;
            }

            case Command::InsertDebugMarker: {
                InsertDebugMarkerCmd* cmd = iter->NextCommand<InsertDebugMarkerCmd>();
                const char* label = iter->NextData<char>(cmd->length + 1);

                if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                    // PIX color is 1 byte per channel in ARGB format
                    constexpr uint64_t kPIXBlackColor = 0xff000000;
                    ToBackend(GetDevice())
                        ->GetFunctions()
                        ->pixSetMarkerOnCommandList(commandList, kPIXBlackColor, label);
                }
                break;
            }

            case Command::PopDebugGroup: {
                iter->NextCommand<PopDebugGroupCmd>();

                if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                    ToBackend(GetDevice())->GetFunctions()->pixEndEventOnCommandList(commandList);
                }
                break;
            }

            case Command::PushDebugGroup: {
                PushDebugGroupCmd* cmd = iter->NextCommand<PushDebugGroupCmd>();
                const char* label = iter->NextData<char>(cmd->length + 1);

                if (ToBackend(GetDevice())->GetFunctions()->IsPIXEventRuntimeLoaded()) {
                    // PIX color is 1 byte per channel in ARGB format
                    constexpr uint64_t kPIXBlackColor = 0xff000000;
                    ToBackend(GetDevice())
                        ->GetFunctions()
                        ->pixBeginEventOnCommandList(commandList, kPIXBlackColor, label);
                }
                break;
            }

            case Command::SetRenderPipeline: {
                SetRenderPipelineCmd* cmd = iter->NextCommand<SetRenderPipelineCmd>();
                RenderPipeline* pipeline = ToBackend(cmd->pipeline).Get();

                commandList->SetPipelineState(pipeline->GetPipelineState());
                commandList->IASetPrimitiveTopology(pipeline->GetD3D12PrimitiveTopology());

                bindingTracker->OnSetPipeline(pipeline);

                lastPipeline = pipeline;
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = iter->NextCommand<SetBindGroupCmd>();
                BindGroup* group = ToBackend(cmd->group.Get());
                uint32_t* dynamicOffsets = nullptr;

                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = iter->NextData<uint32_t>(cmd->dynamicOffsetCount);
                }

                bindingTracker->OnSetBindGroup(cmd->index, group, cmd->dynamicOffsetCount,
                                               dynamicOffsets);
                break;
            }

            case Command::SetIndexBuffer: {
                SetIndexBufferCmd* cmd = iter->NextCommand<SetIndexBufferCmd>();

                D3D12_INDEX_BUFFER_VIEW bufferView;
                bufferView.Format = DXGIIndexFormat(cmd->format);
                bufferView.BufferLocation = ToBackend(cmd->buffer)->GetVA() + cmd->offset;
                bufferView.SizeInBytes = cmd->size;

                commandList->IASetIndexBuffer(&bufferView);
                break;
            }

            case Command::SetVertexBuffer: {
                SetVertexBufferCmd* cmd = iter->NextCommand<SetVertexBufferCmd>();

                vertexBufferTracker.OnSetVertexBuffer(cmd->slot, ToBackend(cmd->buffer.Get()),
                                                      cmd->offset, cmd->size);
                break;
            }

            default:
                DAWN_UNREACHABLE();
                break;
        }
        return {};
    };

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndRenderPass: {
                mCommands.NextCommand<EndRenderPassCmd>();

                // Write timestamp at the end of render pass if it's set.
                if (renderPass->timestampWrites.endOfPassWriteIndex !=
                    wgpu::kQuerySetIndexUndefined) {
                    RecordWriteTimestampCmd(commandList, renderPass->timestampWrites.querySet.Get(),
                                            renderPass->timestampWrites.endOfPassWriteIndex);
                }

                if (useRenderPass) {
                    commandContext->GetCommandList4()->EndRenderPass();
                } else if (renderPass->attachmentState->GetSampleCount() > 1) {
                    ResolveMultisampledRenderPass(commandContext, renderPass);
                }
                return {};
            }

            case Command::SetStencilReference: {
                SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();

                commandList->OMSetStencilRef(cmd->reference);
                break;
            }

            case Command::SetViewport: {
                SetViewportCmd* cmd = mCommands.NextCommand<SetViewportCmd>();
                D3D12_VIEWPORT viewport;
                viewport.TopLeftX = cmd->x;
                viewport.TopLeftY = cmd->y;
                viewport.Width = cmd->width;
                viewport.Height = cmd->height;
                viewport.MinDepth = cmd->minDepth;
                viewport.MaxDepth = cmd->maxDepth;

                commandList->RSSetViewports(1, &viewport);
                break;
            }

            case Command::SetScissorRect: {
                SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                D3D12_RECT rect;
                rect.left = cmd->x;
                rect.top = cmd->y;
                rect.right = cmd->x + cmd->width;
                rect.bottom = cmd->y + cmd->height;

                commandList->RSSetScissorRects(1, &rect);
                break;
            }

            case Command::SetBlendConstant: {
                SetBlendConstantCmd* cmd = mCommands.NextCommand<SetBlendConstantCmd>();
                const std::array<float, 4> color = ConvertToFloatColor(cmd->color);
                commandList->OMSetBlendFactor(color.data());
                break;
            }

            case Command::ExecuteBundles: {
                ExecuteBundlesCmd* cmd = mCommands.NextCommand<ExecuteBundlesCmd>();
                auto bundles = mCommands.NextData<Ref<RenderBundleBase>>(cmd->count);

                for (uint32_t i = 0; i < cmd->count; ++i) {
                    CommandIterator* iter = bundles[i]->GetCommands();
                    iter->Reset();
                    while (iter->NextCommandId(&type)) {
                        DAWN_TRY(EncodeRenderBundleCommand(iter, type));
                    }
                }
                break;
            }

            case Command::BeginOcclusionQuery: {
                BeginOcclusionQueryCmd* cmd = mCommands.NextCommand<BeginOcclusionQueryCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());
                DAWN_ASSERT(D3D12QueryType(querySet->GetQueryType()) ==
                            D3D12_QUERY_TYPE_BINARY_OCCLUSION);
                commandList->BeginQuery(querySet->GetQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION,
                                        cmd->queryIndex);
                break;
            }

            case Command::EndOcclusionQuery: {
                EndOcclusionQueryCmd* cmd = mCommands.NextCommand<EndOcclusionQueryCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());
                DAWN_ASSERT(D3D12QueryType(querySet->GetQueryType()) ==
                            D3D12_QUERY_TYPE_BINARY_OCCLUSION);
                commandList->EndQuery(querySet->GetQueryHeap(), D3D12_QUERY_TYPE_BINARY_OCCLUSION,
                                      cmd->queryIndex);
                break;
            }

            case Command::WriteTimestamp: {
                WriteTimestampCmd* cmd = mCommands.NextCommand<WriteTimestampCmd>();

                RecordWriteTimestampCmd(commandList, cmd->querySet.Get(), cmd->queryIndex);
                break;
            }

            default: {
                DAWN_TRY(EncodeRenderBundleCommand(&mCommands, type));
                break;
            }
        }
    }
    return {};
}
}  // namespace dawn::native::d3d12
