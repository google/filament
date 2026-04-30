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

#include "dawn/native/vulkan/CommandBufferVk.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "absl/functional/function_ref.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/Range.h"
#include "dawn/native/BindGroupTracker.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/ImmediateConstantsTracker.h"
#include "dawn/native/RenderBundle.h"
#include "dawn/native/vulkan/BindGroupVk.h"
#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/CommandRecordingContextVk.h"
#include "dawn/native/vulkan/ComputePipelineVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/FramebufferCache.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/QuerySetVk.h"
#include "dawn/native/vulkan/QueueVk.h"
#include "dawn/native/vulkan/RenderPassCache.h"
#include "dawn/native/vulkan/RenderPipelineVk.h"
#include "dawn/native/vulkan/ResolveTextureLoadingUtilsVk.h"
#include "dawn/native/vulkan/ResourceTableVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {

VkIndexType VulkanIndexType(wgpu::IndexFormat format) {
    switch (format) {
        case wgpu::IndexFormat::Uint16:
            return VK_INDEX_TYPE_UINT16;
        case wgpu::IndexFormat::Uint32:
            return VK_INDEX_TYPE_UINT32;
        case wgpu::IndexFormat::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

bool HasSameTextureCopyExtent(const TextureCopy& srcCopy,
                              const TextureCopy& dstCopy,
                              const TexelExtent3D& copySize) {
    TexelExtent3D imageExtentSrc = ComputeTextureCopyExtent(srcCopy, copySize);
    TexelExtent3D imageExtentDst = ComputeTextureCopyExtent(dstCopy, copySize);
    return imageExtentSrc.width == imageExtentDst.width &&
           imageExtentSrc.height == imageExtentDst.height &&
           imageExtentSrc.depthOrArrayLayers == imageExtentDst.depthOrArrayLayers;
}

VkImageCopy ComputeImageCopyRegion(const TextureCopy& srcCopy,
                                   const TextureCopy& dstCopy,
                                   const TexelExtent3D& copySize,
                                   Aspect aspect) {
    const Texture* srcTexture = ToBackend(srcCopy.texture.Get());
    const Texture* dstTexture = ToBackend(dstCopy.texture.Get());

    VkImageCopy region;
    region.srcSubresource.aspectMask = VulkanAspectMask(aspect);
    region.srcSubresource.mipLevel = srcCopy.mipLevel;
    region.dstSubresource.aspectMask = VulkanAspectMask(aspect);
    region.dstSubresource.mipLevel = dstCopy.mipLevel;

    bool has3DTextureInCopy = false;

    region.srcOffset.x = static_cast<uint32_t>(srcCopy.origin.x);
    region.srcOffset.y = static_cast<uint32_t>(srcCopy.origin.y);
    switch (srcTexture->GetDimension()) {
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();
        case wgpu::TextureDimension::e1D:
            region.srcSubresource.baseArrayLayer = 0;
            region.srcSubresource.layerCount = 1;
            region.srcOffset.z = 0;
            break;
        case wgpu::TextureDimension::e2D:
            region.srcSubresource.baseArrayLayer = static_cast<uint32_t>(srcCopy.origin.z);
            region.srcSubresource.layerCount = static_cast<uint32_t>(copySize.depthOrArrayLayers);
            region.srcOffset.z = 0;
            break;
        case wgpu::TextureDimension::e3D:
            has3DTextureInCopy = true;
            region.srcSubresource.baseArrayLayer = 0;
            region.srcSubresource.layerCount = 1;
            region.srcOffset.z = static_cast<uint32_t>(srcCopy.origin.z);
            break;
    }

    region.dstOffset.x = static_cast<uint32_t>(dstCopy.origin.x);
    region.dstOffset.y = static_cast<uint32_t>(dstCopy.origin.y);
    switch (dstTexture->GetDimension()) {
        case wgpu::TextureDimension::Undefined:
            DAWN_UNREACHABLE();
        case wgpu::TextureDimension::e1D:
            region.dstSubresource.baseArrayLayer = 0;
            region.dstSubresource.layerCount = 1;
            region.dstOffset.z = 0;
            break;
        case wgpu::TextureDimension::e2D:
            region.dstSubresource.baseArrayLayer = static_cast<uint32_t>(dstCopy.origin.z);
            region.dstSubresource.layerCount = static_cast<uint32_t>(copySize.depthOrArrayLayers);
            region.dstOffset.z = 0;
            break;
        case wgpu::TextureDimension::e3D:
            has3DTextureInCopy = true;
            region.dstSubresource.baseArrayLayer = 0;
            region.dstSubresource.layerCount = 1;
            region.dstOffset.z = static_cast<uint32_t>(dstCopy.origin.z);
            break;
    }

    DAWN_ASSERT(HasSameTextureCopyExtent(srcCopy, dstCopy, copySize));
    TexelExtent3D imageExtent = ComputeTextureCopyExtent(dstCopy, copySize);
    region.extent.width = static_cast<uint32_t>(imageExtent.width);
    region.extent.height = static_cast<uint32_t>(imageExtent.height);
    region.extent.depth =
        has3DTextureInCopy ? static_cast<uint32_t>(copySize.depthOrArrayLayers) : 1;

    return region;
}

// Returns the render area from `renderPassCmd` aligned to `granularity`.
VkRect2D GetAlignedRenderArea(VkExtent2D granularity, BeginRenderPassCmd* renderPassCmd) {
    VkRect2D renderArea;

    if (granularity.width == 0 || granularity.width == 1) {
        renderArea.offset.x = renderPassCmd->renderArea.x;
        renderArea.extent.width = renderPassCmd->renderArea.width;
    } else {
        renderArea.offset.x = AlignDown(renderPassCmd->renderArea.x, granularity.width);

        uint32_t right =
            Align(renderPassCmd->renderArea.x + renderPassCmd->renderArea.width, granularity.width);
        if (right > renderPassCmd->width) {
            right = renderPassCmd->width;
        }
        renderArea.extent.width = right - renderArea.offset.x;
    }

    if (granularity.height == 0 || granularity.height == 1) {
        renderArea.offset.y = renderPassCmd->renderArea.y;
        renderArea.extent.height = renderPassCmd->renderArea.height;
    } else {
        renderArea.offset.y = AlignDown(renderPassCmd->renderArea.y, granularity.height);

        uint32_t bottom = Align(renderPassCmd->renderArea.y + renderPassCmd->renderArea.height,
                                granularity.height);
        if (bottom > renderPassCmd->height) {
            bottom = renderPassCmd->height;
        }
        renderArea.extent.height = bottom - renderArea.offset.y;
    }

    return renderArea;
}

// Returns the render area from `renderPassCmd` aligned to render area granularity for
// `renderPassVk`.
VkRect2D GetAlignedRenderArea(Device* device,
                              VkRenderPass renderPassVk,
                              BeginRenderPassCmd* renderPassCmd) {
    VkExtent2D granularity;
    device->fn.GetRenderAreaGranularity(device->GetVkDevice(), renderPassVk, &granularity);

    return GetAlignedRenderArea(granularity, renderPassCmd);
}

class DescriptorSetTracker : public BindGroupTrackerBase<true> {
  public:
    bool AreLayoutsCompatible() override {
        return mPipelineLayout == mLastAppliedPipelineLayout &&
               mLastAppliedImmediateConstantSize == mImmediateConstantSize;
    }

    const BindGroupBase* GetBindGroup(BindGroupIndex index) const { return mBindGroups[index]; }

    void DirtyAll() {
        mDirtyBindGroupsObjectChangedOrIsDynamic.set();
        mDirtyBindGroups.set();
        mLastResourceTable = nullptr;
    }

    template <typename VkPipelineType>
    void OnSetPipeline(VkPipelineType* pipeline) {
        BindGroupTrackerBase::OnSetPipeline(pipeline);

        mVkLayout = pipeline->GetVkLayout();
        mImmediateConstantSize = pipeline->GetImmediateConstantSize();
        mUsesResourceTable = pipeline->GetLayout()->UsesResourceTable();
    }

    void SetResourceTable(ResourceTable* resourceTable) { mResourceTable = resourceTable; }

    void Apply(const VulkanFunctions& vk,
               VkCommandBuffer commandBuffer,
               VkPipelineBindPoint bindPoint) {
        Apply(vk, commandBuffer, bindPoint,
              [&](BindGroupIndex i) { return ToBackend(mBindGroups[i])->GetHandle(); });
    }

    template <typename F>
    void Apply(const VulkanFunctions& vk,
               VkCommandBuffer commandBuffer,
               VkPipelineBindPoint bindPoint,
               F GetDescriptorSet) {
        BeforeApply();

        BindGroupMask dirtyBindGroups = mDirtyBindGroupsObjectChangedOrIsDynamic;

        // Changing push constant range invalidates all descriptor sets.
        // Also clear the last resource table so it gets rebound below.
        if (mLastAppliedImmediateConstantSize != mImmediateConstantSize) {
            dirtyBindGroups = mBindGroupLayoutsMask;
            mLastResourceTable = nullptr;
        }

        // When the usage of the resource table changes between pipelines, or the resource table
        // itself is changed, dirty all bind groups because they shift by 1 (the resource table
        // occupies VkDescriptorSet 0).
        if (mLastUsesResourceTable != mUsesResourceTable || mLastResourceTable != mResourceTable) {
            dirtyBindGroups = mBindGroupLayoutsMask;

            // Set the resource table as the first set if it starts being used.
            if (mUsesResourceTable) {
                DAWN_ASSERT(mResourceTable != nullptr);
                VkDescriptorSet set = mResourceTable->GetHandle();
                vk.CmdBindDescriptorSets(commandBuffer, bindPoint, mVkLayout, 0, 1, &*set, 0,
                                         nullptr);
            }
        }
        BindGroupIndex startOfBindGroups{mUsesResourceTable ? 1u : 0u};

        for (BindGroupIndex dirtyBGIndex : dirtyBindGroups) {
            VkDescriptorSet set = GetDescriptorSet(dirtyBGIndex);
            uint32_t setIndex = uint32_t{dirtyBGIndex + startOfBindGroups};

            const auto dynamicOffsetSpan = GetDynamicOffsets(dirtyBGIndex);
            uint32_t count = static_cast<uint32_t>(dynamicOffsetSpan.size());
            const uint32_t* dynamicOffset = count > 0 ? dynamicOffsetSpan.data() : nullptr;

            vk.CmdBindDescriptorSets(commandBuffer, bindPoint, mVkLayout, setIndex, 1, &*set, count,
                                     dynamicOffset);
        }

        // Update PipelineLayout
        AfterApply();

        mLastAppliedImmediateConstantSize = mImmediateConstantSize;
        mLastUsesResourceTable = mUsesResourceTable;
        mLastResourceTable = mResourceTable;
    }

    RAW_PTR_EXCLUSION VkPipelineLayout mVkLayout;
    raw_ptr<ResourceTable> mLastResourceTable = nullptr;
    raw_ptr<ResourceTable> mResourceTable = nullptr;
    bool mLastUsesResourceTable = false;
    bool mUsesResourceTable = false;
    uint32_t mLastAppliedImmediateConstantSize = 0;
    uint32_t mImmediateConstantSize = 0;
};

template <typename T>
class ImmediateConstantTracker : public T {
  public:
    ImmediateConstantTracker() = default;

    void DirtyAll() { this->mDirty.set(); }

    void Apply(const VulkanFunctions& vk, VkCommandBuffer commandBuffer) {
        DAWN_ASSERT(this->mLastPipeline != nullptr);
        Apply(vk, commandBuffer, ToBackend(this->mLastPipeline)->GetVkLayout(),
              this->mLastPipeline->GetImmediateMask());
    }

    void Apply(const VulkanFunctions& vk,
               VkCommandBuffer commandBuffer,
               VkPipelineLayout layout,
               ImmediateConstantMask pipelineMask) {
        for (auto&& [offset, size] : IterateRanges(this->mDirty & pipelineMask)) {
            uint32_t immediateContentStartOffset =
                static_cast<uint32_t>(offset) * kImmediateConstantElementByteSize;
            uint32_t pushConstantRangeStartOffset =
                GetImmediateIndexInPipeline(static_cast<uint32_t>(offset), pipelineMask) *
                kImmediateConstantElementByteSize;
            vk.CmdPushConstants(commandBuffer, layout, kImmediateShaderStages,
                                pushConstantRangeStartOffset,
                                size * kImmediateConstantElementByteSize,
                                this->mContent.template Get<uint32_t>(immediateContentStartOffset));
        }

        // Reset all dirty bits after uploading.
        this->mDirty.reset();
    }
};

// Records the necessary barriers for a synchronization scope using the resource usage
// data pre-computed in the frontend. Also performs lazy initialization if required.
MaybeError PrepareResourcesForSyncScope(Device* device,
                                        CommandRecordingContext* recordingContext,
                                        const SyncScopeResourceUsage& scope) {
    // Apply pending updates to all resource tables used in usages scope.
    // This has to be done before transitioning resources.
    for (auto& resourceTable : scope.usedResourceTables) {
        DAWN_TRY(ToBackend(resourceTable)->ApplyPendingUpdates(recordingContext));
    }

    // Separate barriers with vertex stages in destination stages from all other barriers.
    // This avoids creating unnecessary fragment->vertex dependencies when merging barriers.
    // Eg. merging a compute->vertex barrier and a fragment->fragment barrier would create
    // a compute|fragment->vertex|fragment barrier.
    static constexpr VkPipelineStageFlags kVertexStages = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT |
                                                          VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                                                          VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

    struct ImageBarriers {
        std::vector<VkImageMemoryBarrier> imageBarriers;
        VkPipelineStageFlags srcStages = 0;
        VkPipelineStageFlags dstStages = 0;
    };

    ImageBarriers vertexImageBarriers;
    ImageBarriers nonVertexImageBarriers;

    BufferBarrier vertexBufferBarrier;
    BufferBarrier nonVertexBufferBarrier;

    for (size_t i = 0; i < scope.buffers.size(); ++i) {
        Buffer* buffer = ToBackend(scope.buffers[i]);
        buffer->EnsureDataInitialized(recordingContext);

        // `kIndirectBufferForFrontendValidation` is only for the front-end validation and should be
        // totally ignored in the backend resource tracking because:
        // 1. When old usage contains kIndirectBufferForFrontendValidation while new usage just
        //    removes kIndirectBufferForFrontendValidation, the barrier is actually unnecessary.
        // 2. When usage == kIndirectBufferForFrontendValidation, dstStages would be NONE, which is
        //    not allowed by Vulkan SPEC unless synchronization2 is enabled.
        // We remove `kIndirectBufferForFrontendValidation` in this function instead of in the
        // function `BufferVk::TrackUsageAndGetResourceBarrier()` because on Vulkan backend this
        // is the only place that we need to handle `kIndirectBufferForFrontendValidation`.
        wgpu::BufferUsage usage =
            scope.bufferSyncInfos[i].usage & (~kIndirectBufferForFrontendValidation);

        recordingContext->CheckBufferNeedsEagerTransition(buffer, usage);
        BufferBarrier barrier =
            buffer->TrackUsageAndGetResourceBarrier(usage, scope.bufferSyncInfos[i].shaderStages);

        if (barrier.dstStages & kVertexStages) {
            vertexBufferBarrier.Merge(barrier);
        } else {
            nonVertexBufferBarrier.Merge(barrier);
        }
    }

    auto MergeImageBarriers = [](ImageBarriers* barriers, VkPipelineStageFlags srcStages,
                                 VkPipelineStageFlags dstStages,
                                 const std::vector<VkImageMemoryBarrier>& imageBarriers) {
        barriers->srcStages |= srcStages;
        barriers->dstStages |= dstStages;
        barriers->imageBarriers.insert(barriers->imageBarriers.end(), imageBarriers.begin(),
                                       imageBarriers.end());
    };

    // TODO(crbug.com/dawn/851): Add image barriers directly to the correct vector.
    std::vector<VkImageMemoryBarrier> imageBarriers;
    for (size_t i = 0; i < scope.textures.size(); ++i) {
        Texture* texture = ToBackend(scope.textures[i]);

        VkPipelineStageFlags srcStages = 0;
        VkPipelineStageFlags dstStages = 0;

        // Clear subresources that are not render attachments. Render attachments will be
        // cleared in RecordBeginRenderPass by setting the loadop to clear when the texture
        // subresource has not been initialized before the render pass.
        DAWN_TRY(scope.textureSyncInfos[i].Iterate(
            [&](const SubresourceRange& range, const TextureSyncInfo& syncInfo) -> MaybeError {
                if (syncInfo.usage & ~wgpu::TextureUsage::RenderAttachment) {
                    DAWN_TRY(texture->EnsureSubresourceContentInitialized(recordingContext, range));
                }
                return {};
            }));
        texture->TransitionUsageForPass(recordingContext, scope.textureSyncInfos[i], &imageBarriers,
                                        &srcStages, &dstStages);

        if (!imageBarriers.empty()) {
            MergeImageBarriers(
                (dstStages & kVertexStages) ? &vertexImageBarriers : &nonVertexImageBarriers,
                srcStages, dstStages, imageBarriers);
            imageBarriers.clear();
        }
    }

    for (const ImageBarriers& barriers : {vertexImageBarriers, nonVertexImageBarriers}) {
        if (!barriers.imageBarriers.empty()) {
            device->fn.CmdPipelineBarrier(
                recordingContext->commandBuffer, barriers.srcStages, barriers.dstStages, 0, 0,
                nullptr, 0, nullptr, barriers.imageBarriers.size(), barriers.imageBarriers.data());
        }
    }
    recordingContext->EmitBufferBarrierIfNecessary(device, vertexBufferBarrier);
    recordingContext->EmitBufferBarrierIfNecessary(device, nonVertexBufferBarrier);

    return {};
}

// Reset the query sets used on render pass because the reset command must be called outside
// render pass.
void ResetUsedQuerySetsOnRenderPass(Device* device,
                                    VkCommandBuffer commands,
                                    QuerySetBase* querySet,
                                    const std::vector<bool>& availability) {
    DAWN_ASSERT(availability.size() == querySet->GetQueryAvailability().size());

    auto currentIt = availability.begin();
    auto lastIt = availability.end();
    // Traverse the used queries which availability are true.
    while (currentIt != lastIt) {
        auto firstTrueIt = std::find(currentIt, lastIt, true);
        // No used queries need to be reset
        if (firstTrueIt == lastIt) {
            break;
        }

        auto nextFalseIt = std::find(firstTrueIt, lastIt, false);

        uint32_t queryIndex = std::distance(availability.begin(), firstTrueIt);
        uint32_t queryCount = std::distance(firstTrueIt, nextFalseIt);

        // Reset the queries between firstTrueIt and nextFalseIt (which is at most
        // lastIt)
        device->fn.CmdResetQueryPool(commands, ToBackend(querySet)->GetHandle(), queryIndex,
                                     queryCount);

        // Set current iterator to next false
        currentIt = nextFalseIt;
    }
}

void RecordWriteTimestampCmd(CommandRecordingContext* recordingContext,
                             Device* device,
                             QuerySetBase* querySet,
                             uint32_t queryIndex,
                             bool isRenderPass,
                             VkPipelineStageFlagBits pipelineStage) {
    VkCommandBuffer commands = recordingContext->commandBuffer;

    // The queries must be reset between uses, and the reset command cannot be called in render
    // pass.
    if (!isRenderPass) {
        device->fn.CmdResetQueryPool(commands, ToBackend(querySet)->GetHandle(), queryIndex, 1);
    }

    device->fn.CmdWriteTimestamp(commands, pipelineStage, ToBackend(querySet)->GetHandle(),
                                 queryIndex);
}

void RecordResolveQuerySetCmd(VkCommandBuffer commands,
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
        device->fn.CmdCopyQueryPoolResults(commands, querySet->GetHandle(), resolveQueryIndex,
                                           resolveQueryCount, destination->GetHandle(),
                                           resolveDestinationOffset, sizeof(uint64_t),
                                           VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

        // Set current iterator to next false
        currentIt = nextFalseIt;
    }
}

VkClearValue ToVkClearValue(dawn::native::Color clearColor, TextureComponentType baseType) {
    VkClearValue clearValue;
    switch (baseType) {
        case TextureComponentType::Float: {
            const std::array<float, 4> appliedClearColor = ConvertToFloatColor(clearColor);
            for (uint32_t j = 0; j < 4; ++j) {
                clearValue.color.float32[j] = appliedClearColor[j];
            }
            break;
        }
        case TextureComponentType::Uint: {
            const std::array<uint32_t, 4> appliedClearColor =
                ConvertToUnsignedIntegerColor(clearColor);
            for (uint32_t j = 0; j < 4; ++j) {
                clearValue.color.uint32[j] = appliedClearColor[j];
            }
            break;
        }
        case TextureComponentType::Sint: {
            const std::array<int32_t, 4> appliedClearColor =
                ConvertToSignedIntegerColor(clearColor);
            for (uint32_t j = 0; j < 4; ++j) {
                clearValue.color.int32[j] = appliedClearColor[j];
            }
            break;
        }
    }
    return clearValue;
}

// A number of WebGPU commands cannot be immediately turned into Vulkan commands and instead just
// dirty state which needs to be applied just before the Draw/Dispatch. This State class factors the
// tracking logic of both Compute and Render passes.
template <typename BaseImmediateTracker, typename Pipeline, VkPipelineBindPoint PipelineBindPoint>
struct ProgrammablePassState : public StackAllocated {
    ProgrammablePassState(const VulkanFunctions& vk, CommandRecordingContext* recordingContext)
        : vk(vk), recordingContext(recordingContext) {}

    void OnSetPipeline(Pipeline* pipeline) {
        lastPipeline = pipeline;
        descriptorSets.OnSetPipeline<Pipeline>(pipeline);
        immediates.OnSetPipeline(pipeline);
    }

    // Synchronizes all the dirty state before doing the operation.
    template <typename F>
    MaybeError SyncAndRun(F&& DoOperation) {
        VkCommandBuffer commands = recordingContext->commandBuffer;

        if (lastAppliedPipeline != lastPipeline) {
            if (lastPipeline->RequiresSpecialization()) {
                return RunWithSpecialization(DoOperation);
            }

            vk.CmdBindPipeline(commands, PipelineBindPoint, lastPipeline->GetHandle());
            lastPipeline->ApplyDynamicState(commands, lastAppliedPipeline);
            lastAppliedPipeline = lastPipeline;
        }

        descriptorSets.Apply(vk, commands, PipelineBindPoint);
        immediates.Apply(vk, commands);

        DoOperation(vk, commands);
        return {};
    }

    using PipelineSpecialization = Pipeline::Specialization;
    PipelineSpecialization ComputeSpecializationBaseOnState() const {
        PipelineSpecialization s;

        // Add the specialization to use a static samplers for external textures that need them.
        PipelineBase::SamplerForExternalTextureMap samplerForET =
            lastPipeline->ComputeSamplerForExternalTextureMap();
        for (const auto& [etBindPoint, maybeSamplerBindPoint] : samplerForET) {
            const BindGroupBase* etBindGroup = descriptorSets.GetBindGroup(etBindPoint.group);

            // Multiplanar external textures never use a static sampler. (et may be null when a 2D
            // view is bound directly in lieu of an external texture).
            const ExternalTextureBase* et =
                etBindGroup->GetBoundExternalTexture(etBindPoint.binding).Get();
            if (et != nullptr && !et->HasSingleView()) {
                continue;
            }

            // Get the ExternalTextureBindingInfo from which we can get the indices for its plane0
            // and staticSampler bind points.
            const auto& bindingInfo =
                etBindGroup->GetLayout()->GetAPIBindingInfo(etBindPoint.binding);
            const auto& etInfo = std::get<ExternalTextureBindingInfo>(bindingInfo.bindingLayout);
            DAWN_ASSERT(etInfo.staticSampler.has_value());

            const TextureView* view =
                ToBackend(etBindGroup->GetBindingAsTextureView(etInfo.plane0));
            DAWN_ASSERT(view != nullptr);

            // Use static samplers for YCbCr external textures. However when the toggle is enabled,
            // we use a static sampler for all the single-planar external textures, which helps with
            // testing the code paths on any Vulkan-capable device.
            if (view->GetFormat().format != wgpu::TextureFormat::OpaqueYCbCrAndroid &&
                !lastPipeline->GetDevice()->IsToggleEnabled(
                    Toggle::VulkanForceStaticSamplersForExternalTextures)) {
                continue;
            }

            // Find the sampler it's used with, its filtering parameters will be copied in the
            // static sampler (or use "near" if no sampler is used with the external texture).
            const Sampler* sampler = nullptr;
            if (maybeSamplerBindPoint) {
                sampler = ToBackend(descriptorSets.GetBindGroup(maybeSamplerBindPoint->group)
                                        ->GetBindingAsSampler(maybeSamplerBindPoint->binding));
            }

            // Tell both the shader that we'll be using a YCbCr external texture at the bindpoint,
            // and tell BindGroupLayouts to use a specific static sampler.
            s.ycbcrExternalTextures.insert(etBindPoint);
            s.layout.bindGroups[etBindPoint.group].staticSamplers[*etInfo.staticSampler] =
                StaticSamplerSpecialization::From(view, sampler);
        }

        return s;
    }

    // The handling of specialization of pipelines is split to a separate method that's not
    // templated, to try to avoid inlining, so as to not hugely increase the stack size of the
    // callees.
    MaybeError RunWithSpecialization(
        absl::FunctionRef<void(const VulkanFunctions&, VkCommandBuffer)> DoOperation) {
        // Make sure to reapply all the state in case the specialization changes something in how
        // the state should be applied (for example with the VkPipelineLayout).
        DirtyAll();

        // At the moment the only specialization is for using static samplers in ExternalTextures.
        DAWN_ASSERT(lastPipeline->GetDevice()->NeedsStaticSamplerForExternalTexture() &&
                    lastPipeline->GetLayout()->HasExternalTextures());
        PipelineSpecialization specialization = ComputeSpecializationBaseOnState();

        // Recreate the descriptor sets using the specialized VkDescriptorSetLayout.
        PerBindGroup<std::unique_ptr<OwnedDescriptorSet>> newDescriptorSets;
        for (BindGroupIndex group : lastPipeline->GetLayout()->GetBindGroupLayoutsMask()) {
            auto bgl = ToBackend(lastPipeline->GetLayout()->GetBindGroupLayout(group));
            auto boundBG = ToBackend(descriptorSets.GetBindGroup(group));
            DAWN_TRY_ASSIGN(
                newDescriptorSets[group],
                bgl->GetSpecializedSetFor(boundBG, specialization.layout.bindGroups[group]));
        }

        // Get the specialized pipeline.
        PipelineHandles pipelineHandles;
        DAWN_TRY_ASSIGN(pipelineHandles,
                        lastPipeline->GetOrCreateSpecializedHandle(std::move(specialization)));

        // Reapply all the state and run the operation.
        VkCommandBuffer commands = recordingContext->commandBuffer;

        vk.CmdBindPipeline(commands, PipelineBindPoint, pipelineHandles.pipeline);
        descriptorSets.Apply(vk, commands, PipelineBindPoint,
                             [&](BindGroupIndex i) { return newDescriptorSets[i]->GetHandle(); });
        immediates.Apply(vk, commands, pipelineHandles.layout, lastPipeline->GetImmediateMask());

        DoOperation(vk, commands);

        // Make sure none of the state applied for the specialization is used in further commands.
        DirtyAll();

        return {};
    }

    void DirtyAll() {
        descriptorSets.DirtyAll();
        immediates.DirtyAll();
        lastAppliedPipeline = nullptr;
    }

    const VulkanFunctions& vk;
    CommandRecordingContext* recordingContext;

    DescriptorSetTracker descriptorSets;
    ImmediateConstantTracker<BaseImmediateTracker> immediates;

    Pipeline* lastPipeline = nullptr;
    Pipeline* lastAppliedPipeline = nullptr;
};

using RenderPassState = ProgrammablePassState<RenderImmediateConstantsTrackerBase,
                                              RenderPipeline,
                                              VK_PIPELINE_BIND_POINT_GRAPHICS>;
using ComputePassState = ProgrammablePassState<ComputeImmediateConstantsTrackerBase,
                                               ComputePipeline,
                                               VK_PIPELINE_BIND_POINT_COMPUTE>;

}  // anonymous namespace

MaybeError RecordBeginDynamicRenderPass(CommandRecordingContext* recordingContext,
                                        Device* device,
                                        BeginRenderPassCmd* renderPass) {
    // Needed to work around some quirks introduced by vulkan_platform.h which causes some platforms
    // to hit compiler errors if Vulkan struct members are assigned to VK_NULL_HANDLE directly.
    static const VkImageView nullImageView = VK_NULL_HANDLE;

    VkRenderingInfoKHR renderInfo;
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderInfo.pNext = nullptr;
    renderInfo.flags = 0;
    // TODO(https://crbug.com/489152883): If VK_KHR_maintenance5 is available this should call
    // vkGetRenderingAreaGranularity(). Otherwise a good approach might be to take a guess at the
    // maximum granularity for the GPU. This needs some more thought and using 32x32 granularity is
    // stop gap.
    VkExtent2D granularity{32, 32};
    renderInfo.renderArea = GetAlignedRenderArea(granularity, renderPass);
    renderInfo.layerCount = 1;
    renderInfo.viewMask = 0;
    renderInfo.pDepthAttachment = nullptr;
    renderInfo.pStencilAttachment = nullptr;

    PerColorAttachment<VkRenderingAttachmentInfoKHR> colorAttachments;

    ColorAttachmentMask attachmentMask = renderPass->attachmentState->GetColorAttachmentsMask();
    ColorAttachmentIndex attachmentCount = GetHighestBitIndexPlusOne(attachmentMask);

    // Initialize all potentially used color attachments with null/default values, which allows the
    // attachments to be sparse.
    for (auto i : Range(attachmentCount)) {
        auto& colorAttachment = colorAttachments[i];
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachment.pNext = nullptr;
        colorAttachment.imageView = nullImageView;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
        colorAttachment.resolveImageView = nullImageView;
        colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    }

    // Set the color attachments used by this pass. May be sparse.
    for (auto i : attachmentMask) {
        auto& attachmentInfo = renderPass->colorAttachments[i];
        TextureView* view = ToBackend(attachmentInfo.view.Get());
        if (view == nullptr) {
            continue;
        }

        auto& colorAttachment = colorAttachments[i];
        if (view->GetDimension() == wgpu::TextureViewDimension::e3D) {
            VkImageView handleFor2DViewOn3D;
            DAWN_TRY_ASSIGN(handleFor2DViewOn3D,
                            view->GetOrCreate2DViewOn3D(attachmentInfo.depthSlice));
            colorAttachment.imageView = handleFor2DViewOn3D;
        } else {
            colorAttachment.imageView = view->GetHandle();
        }

        if (attachmentInfo.resolveTarget != nullptr) {
            TextureView* resolveView = ToBackend(attachmentInfo.resolveTarget.Get());
            colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            colorAttachment.resolveImageView = resolveView->GetHandle();
            colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        } else if (renderPass->msaaRenderToSingleSampled &&
                   view->GetTexture()->GetSampleCount() == 1) {
            // VUID-VkRenderingAttachmentInfo-None-12256: If MSRTSS is used and the attachment has
            // a sample count of 1, the resolve mode can't be VK_RESOLVE_MODE_NONE.
            colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        colorAttachment.loadOp = VulkanAttachmentLoadOp(attachmentInfo.loadOp);
        colorAttachment.storeOp = VulkanAttachmentStoreOp(attachmentInfo.storeOp);
        colorAttachment.clearValue = ToVkClearValue(
            attachmentInfo.clearColor, view->GetFormat().GetAspectInfo(Aspect::Color).baseType);
    }

    renderInfo.colorAttachmentCount = static_cast<uint32_t>(attachmentCount);
    renderInfo.pColorAttachments = colorAttachments.data();

    // Set the depth/stencil attachment used by this pass.
    VkRenderingAttachmentInfoKHR depthAttachment;
    VkRenderingAttachmentInfoKHR stencilAttachment;
    if (renderPass->attachmentState->HasDepthStencilAttachment()) {
        const auto& attachmentInfo = renderPass->depthStencilAttachment;
        TextureView* view = ToBackend(attachmentInfo.view.Get());
        DAWN_ASSERT(view);

        const Format& dsFormat = view->GetTexture()->GetFormat();
        VkImageLayout imageLayout = VulkanImageLayoutForDepthStencilAttachment(
            dsFormat, attachmentInfo.depthReadOnly, attachmentInfo.stencilReadOnly);

        if (dsFormat.HasDepth()) {
            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            depthAttachment.pNext = nullptr;
            depthAttachment.imageView = view->GetHandle();
            depthAttachment.imageLayout = imageLayout;
            depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
            depthAttachment.resolveImageView = nullImageView;
            depthAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.loadOp = VulkanAttachmentLoadOp(attachmentInfo.depthLoadOp);
            depthAttachment.storeOp = VulkanAttachmentStoreOp(attachmentInfo.depthStoreOp);
            depthAttachment.clearValue.depthStencil.depth = attachmentInfo.clearDepth;
            renderInfo.pDepthAttachment = &depthAttachment;
        }

        if (dsFormat.HasStencil()) {
            stencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            stencilAttachment.pNext = nullptr;
            stencilAttachment.imageView = view->GetHandle();
            stencilAttachment.imageLayout = imageLayout;
            stencilAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
            stencilAttachment.resolveImageView = nullImageView;
            stencilAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            stencilAttachment.loadOp = VulkanAttachmentLoadOp(attachmentInfo.stencilLoadOp);
            stencilAttachment.storeOp = VulkanAttachmentStoreOp(attachmentInfo.stencilStoreOp);
            stencilAttachment.clearValue.depthStencil.stencil = attachmentInfo.clearStencil;
            renderInfo.pStencilAttachment = &stencilAttachment;
        }
    }

    VkMultisampledRenderToSingleSampledInfoEXT msrtss = {};
    if (renderPass->msaaRenderToSingleSampled) {
        msrtss.sType = VK_STRUCTURE_TYPE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_INFO_EXT;
        msrtss.pNext = nullptr;
        msrtss.multisampledRenderToSingleSampledEnable = VK_TRUE;
        msrtss.rasterizationSamples =
            VulkanSampleCount(renderPass->attachmentState->GetSampleCount());

        renderInfo.pNext = &msrtss;
    }

    // TODO(crbug.com/463893794): Handle ExpandResolveTexture.

    device->fn.CmdBeginRenderingKHR(recordingContext->commandBuffer, &renderInfo);

    return {};
}

MaybeError RecordBeginRenderPass(CommandRecordingContext* recordingContext,
                                 Device* device,
                                 BeginRenderPassCmd* renderPass) {
    if (device->GetRenderPassType() == VulkanRenderPassType::DynamicRendering) {
        return RecordBeginDynamicRenderPass(recordingContext, device, renderPass);
    }

    VkCommandBuffer commands = recordingContext->commandBuffer;

    // Query a VkRenderPass from the cache
    VkRenderPass renderPassVK = VK_NULL_HANDLE;
    uint32_t renderPassId = 0;
    {
        RenderPassCacheQuery query;

        for (auto i : renderPass->attachmentState->GetColorAttachmentsMask()) {
            const auto& attachmentInfo = renderPass->colorAttachments[i];
            bool hasResolveTarget = attachmentInfo.resolveTarget != nullptr;
            bool renderToSingleSampled = renderPass->msaaRenderToSingleSampled &&
                                         attachmentInfo.view->GetTexture()->GetSampleCount() == 1;

            query.SetColor(i, attachmentInfo.view->GetFormat().format, attachmentInfo.loadOp,
                           attachmentInfo.storeOp, hasResolveTarget, renderToSingleSampled);
        }

        if (renderPass->attachmentState->HasDepthStencilAttachment()) {
            const auto& attachmentInfo = renderPass->depthStencilAttachment;

            query.SetDepthStencil(attachmentInfo.view->GetTexture()->GetFormat().format,
                                  attachmentInfo.depthLoadOp, attachmentInfo.depthStoreOp,
                                  attachmentInfo.depthReadOnly, attachmentInfo.stencilLoadOp,
                                  attachmentInfo.stencilStoreOp, attachmentInfo.stencilReadOnly);
        }

        query.SetSampleCount(renderPass->attachmentState->GetSampleCount());

        RenderPassCache::RenderPassInfo renderPassInfo;
        DAWN_TRY_ASSIGN(renderPassInfo, device->GetRenderPassCache()->GetRenderPass(query));
        renderPassVK = renderPassInfo.renderPass;
        renderPassId = renderPassInfo.uniqueId;
    }

    // Query a framebuffer from the cache and gather the clear values for the attachments at the
    // same time.
    FramebufferCacheQuery framebufferQuery;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    {
        framebufferQuery.SetRenderPass(renderPassId, renderPass->width, renderPass->height);

        // Fill in the attachment info that will be chained in the framebuffer create info.
        for (auto i : renderPass->attachmentState->GetColorAttachmentsMask()) {
            auto& attachmentInfo = renderPass->colorAttachments[i];
            TextureView* view = ToBackend(attachmentInfo.view.Get());
            if (view == nullptr) {
                continue;
            }

            VkClearValue clearValue = ToVkClearValue(
                attachmentInfo.clearColor, view->GetFormat().GetAspectInfo(Aspect::Color).baseType);
            uint32_t depthSlice = view->GetDimension() == wgpu::TextureViewDimension::e3D
                                      ? attachmentInfo.depthSlice
                                      : 0;
            DAWN_TRY(framebufferQuery.AddAttachment(view, clearValue, depthSlice));
        }

        if (renderPass->attachmentState->HasDepthStencilAttachment()) {
            auto& attachmentInfo = renderPass->depthStencilAttachment;
            TextureView* view = ToBackend(attachmentInfo.view.Get());

            VkClearValue clearValue;
            clearValue.depthStencil.depth = attachmentInfo.clearDepth;
            clearValue.depthStencil.stencil = attachmentInfo.clearStencil;

            DAWN_TRY(framebufferQuery.AddAttachment(view, clearValue));
        }

        for (auto i : renderPass->attachmentState->GetColorAttachmentsMask()) {
            if (renderPass->colorAttachments[i].resolveTarget != nullptr) {
                TextureView* view = ToBackend(renderPass->colorAttachments[i].resolveTarget.Get());
                DAWN_TRY(framebufferQuery.AddAttachment(view));
            }
        }

        DAWN_TRY_ASSIGN(
            framebuffer,
            device->GetFramebufferCache()->GetOrCreate(
                framebufferQuery,
                [&](const FramebufferCacheQuery& query) -> ResultOrError<VkFramebuffer> {
                    VkFramebufferCreateInfo createInfo;
                    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    createInfo.pNext = nullptr;
                    createInfo.flags = 0;
                    createInfo.renderPass = renderPassVK;
                    createInfo.attachmentCount = query.attachmentCount;
                    createInfo.pAttachments = AsVkArray(query.attachments.data());
                    createInfo.width = query.width;
                    createInfo.height = query.height;
                    createInfo.layers = 1;

                    VkFramebuffer framebuffer;
                    DAWN_TRY(CheckVkSuccess(
                        device->fn.CreateFramebuffer(device->GetVkDevice(), &createInfo, nullptr,
                                                     &*framebuffer),
                        "CreateFramebuffer"));
                    return framebuffer;
                }));
    }

    VkRenderPassBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.renderPass = renderPassVK;
    beginInfo.framebuffer = framebuffer;
    beginInfo.renderArea = GetAlignedRenderArea(device, renderPassVK, renderPass);
    beginInfo.clearValueCount = framebufferQuery.attachmentCount;
    beginInfo.pClearValues = framebufferQuery.clearValues.data();

    if (renderPass->attachmentState->GetExpandResolveInfo().attachmentsToExpandResolve.any()) {
        DAWN_TRY(BeginRenderPassAndExpandResolveTextureWithDraw(device, recordingContext,
                                                                renderPass, beginInfo));
    } else {
        device->fn.CmdBeginRenderPass(commands, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    return {};
}

void RecordEndRenderPass(CommandRecordingContext* recordingContext, Device* device) {
    VkCommandBuffer commands = recordingContext->commandBuffer;

    if (device->GetRenderPassType() == VulkanRenderPassType::DynamicRendering) {
        device->fn.CmdEndRenderingKHR(commands);
    } else {
        device->fn.CmdEndRenderPass(commands);
    }
}

// static
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor) {}

MaybeError CommandBuffer::RecordCopyImageWithTemporaryBuffer(
    CommandRecordingContext* recordingContext,
    const TextureCopy& srcCopy,
    const TextureCopy& dstCopy,
    const TexelExtent3D& texelCopySize) {
    DAWN_ASSERT(srcCopy.texture->GetFormat().CopyCompatibleWith(dstCopy.texture->GetFormat()));
    DAWN_ASSERT(srcCopy.aspect == dstCopy.aspect);
    const TypedTexelBlockInfo& blockInfo = GetBlockInfo(srcCopy);
    const BlockExtent3D copySize = blockInfo.ToBlock(texelCopySize);
    BlockCount widthInBlocks = copySize.width;
    BlockCount heightInBlocks = copySize.height;

    // Create the temporary buffer. Note that We don't need to respect WebGPU's 256 alignment
    // because it isn't a hard constraint in Vulkan.
    uint64_t tempBufferSize =
        blockInfo.ToBytes(widthInBlocks * heightInBlocks * copySize.depthOrArrayLayers);
    BufferDescriptor tempBufferDescriptor;
    tempBufferDescriptor.size = tempBufferSize;
    tempBufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

    Device* device = ToBackend(GetDevice());
    Ref<BufferBase> tempBufferBase;
    DAWN_TRY_ASSIGN(tempBufferBase, device->CreateBuffer(&tempBufferDescriptor));
    Buffer* tempBuffer = ToBackend(tempBufferBase.Get());
    auto scopedUseTempBuffer = tempBuffer->UseInternal();

    BufferCopy tempBufferCopy;
    tempBufferCopy.buffer = tempBuffer;
    tempBufferCopy.rowsPerImage = heightInBlocks;
    tempBufferCopy.offset = 0;
    tempBufferCopy.blocksPerRow = widthInBlocks;

    VkCommandBuffer commands = recordingContext->commandBuffer;
    VkImage srcImage = ToBackend(srcCopy.texture)->GetHandle();
    VkImage dstImage = ToBackend(dstCopy.texture)->GetHandle();

    tempBuffer->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);
    VkBufferImageCopy srcToTempBufferRegion =
        ComputeBufferImageCopyRegion(tempBufferCopy, srcCopy, copySize);

    // The Dawn CopySrc usage is always mapped to GENERAL
    device->fn.CmdCopyImageToBuffer(commands, srcImage, VK_IMAGE_LAYOUT_GENERAL,
                                    tempBuffer->GetHandle(), 1, &srcToTempBufferRegion);

    tempBuffer->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopySrc);
    VkBufferImageCopy tempBufferToDstRegion =
        ComputeBufferImageCopyRegion(tempBufferCopy, dstCopy, copySize);

    // Dawn guarantees dstImage be in the TRANSFER_DST_OPTIMAL layout after the
    // copy command.
    device->fn.CmdCopyBufferToImage(commands, tempBuffer->GetHandle(), dstImage,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                    &tempBufferToDstRegion);

    recordingContext->tempBuffers.emplace_back(std::move(tempBuffer));

    return {};
}

MaybeError CommandBuffer::RecordCommands(CommandRecordingContext* recordingContext) {
    Device* device = ToBackend(GetDevice());
    VkCommandBuffer commands = recordingContext->commandBuffer;

    // Records the necessary barriers for the resource usage pre-computed by the frontend.
    // And resets the used query sets which are rewritten on the render pass.
    auto PrepareResourcesForRenderPass = [](Device* device,
                                            CommandRecordingContext* recordingContext,
                                            const RenderPassResourceUsage& usages) -> MaybeError {
        DAWN_TRY(PrepareResourcesForSyncScope(device, recordingContext, usages));

        // Reset all query set used on current render pass together before beginning render pass
        // because the reset command must be called outside render pass
        for (size_t i = 0; i < usages.querySets.size(); ++i) {
            ResetUsedQuerySetsOnRenderPass(device, recordingContext->commandBuffer,
                                           usages.querySets[i], usages.queryAvailabilities[i]);
        }
        return {};
    };

    size_t nextComputePassNumber = 0;
    size_t nextRenderPassNumber = 0;

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::CopyBufferToBuffer: {
                CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                if (copy->size == 0) {
                    // Skip no-op copies.
                    break;
                }

                Buffer* srcBuffer = ToBackend(copy->source.Get());
                Buffer* dstBuffer = ToBackend(copy->destination.Get());

                srcBuffer->EnsureDataInitialized(recordingContext);
                dstBuffer->EnsureDataInitializedAsDestination(recordingContext,
                                                              copy->destinationOffset, copy->size);

                srcBuffer->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopySrc);
                dstBuffer->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);

                VkBufferCopy region;
                region.srcOffset = copy->sourceOffset;
                region.dstOffset = copy->destinationOffset;
                region.size = copy->size;

                VkBuffer srcHandle = srcBuffer->GetHandle();
                VkBuffer dstHandle = dstBuffer->GetHandle();
                device->fn.CmdCopyBuffer(commands, srcHandle, dstHandle, 1, &region);
                break;
            }

            case Command::CopyBufferToTexture: {
                CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                if (copy->copySize.IsEmpty()) {
                    // Skip no-op copies.
                    continue;
                }
                auto& src = copy->source;
                auto& dst = copy->destination;

                ToBackend(src.buffer)->EnsureDataInitialized(recordingContext);

                const TypedTexelBlockInfo& blockInfo = GetBlockInfo(dst);
                VkBufferImageCopy region =
                    ComputeBufferImageCopyRegion(src, dst, blockInfo.ToBlock(copy->copySize));
                VkImageSubresourceLayers subresource = region.imageSubresource;

                SubresourceRange range =
                    GetSubresourcesAffectedByCopy(copy->destination, copy->copySize);

                if (IsCompleteSubresourceCopiedTo(dst.texture.Get(), copy->copySize,
                                                  subresource.mipLevel, dst.aspect)) {
                    // Since texture has been overwritten, it has been "initialized"
                    dst.texture->SetIsSubresourceContentInitialized(true, range);
                } else {
                    DAWN_TRY(ToBackend(dst.texture)
                                 ->EnsureSubresourceContentInitialized(recordingContext, range));
                }

                ToBackend(src.buffer)
                    ->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopySrc);
                ToBackend(dst.texture)
                    ->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst,
                                         wgpu::ShaderStage::None, range);

                VkBuffer srcBuffer = ToBackend(src.buffer)->GetHandle();
                VkImage dstImage = ToBackend(dst.texture)->GetHandle();

                // Dawn guarantees dstImage be in the TRANSFER_DST_OPTIMAL layout after the
                // copy command.
                device->fn.CmdCopyBufferToImage(commands, srcBuffer, dstImage,
                                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
                break;
            }

            case Command::CopyTextureToBuffer: {
                CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                if (copy->copySize.IsEmpty()) {
                    // Skip no-op copies.
                    continue;
                }
                auto& src = copy->source;
                auto& dst = copy->destination;

                ToBackend(dst.buffer)->EnsureDataInitializedAsDestination(recordingContext, copy);

                const TypedTexelBlockInfo& blockInfo = GetBlockInfo(src);
                VkBufferImageCopy region =
                    ComputeBufferImageCopyRegion(dst, src, blockInfo.ToBlock(copy->copySize));

                SubresourceRange range =
                    GetSubresourcesAffectedByCopy(copy->source, copy->copySize);

                DAWN_TRY(ToBackend(src.texture)
                             ->EnsureSubresourceContentInitialized(recordingContext, range));

                ToBackend(src.texture)
                    ->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopySrc,
                                         wgpu::ShaderStage::None, range);
                ToBackend(dst.buffer)
                    ->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);

                VkImage srcImage = ToBackend(src.texture)->GetHandle();
                VkBuffer dstBuffer = ToBackend(dst.buffer)->GetHandle();
                // The Dawn CopySrc usage is always mapped to GENERAL
                device->fn.CmdCopyImageToBuffer(commands, srcImage, VK_IMAGE_LAYOUT_GENERAL,
                                                dstBuffer, 1, &region);
                break;
            }

            case Command::CopyTextureToTexture: {
                CopyTextureToTextureCmd* copy = mCommands.NextCommand<CopyTextureToTextureCmd>();
                if (copy->copySize.IsEmpty()) {
                    // Skip no-op copies.
                    continue;
                }
                TextureCopy& src = copy->source;
                TextureCopy& dst = copy->destination;
                SubresourceRange srcRange = GetSubresourcesAffectedByCopy(src, copy->copySize);
                SubresourceRange dstRange = GetSubresourcesAffectedByCopy(dst, copy->copySize);

                DAWN_TRY(ToBackend(src.texture)
                             ->EnsureSubresourceContentInitialized(recordingContext, srcRange));
                if (IsCompleteSubresourceCopiedTo(dst.texture.Get(), copy->copySize, dst.mipLevel,
                                                  dst.aspect)) {
                    // Since destination texture has been overwritten, it has been "initialized"
                    dst.texture->SetIsSubresourceContentInitialized(true, dstRange);
                } else {
                    DAWN_TRY(ToBackend(dst.texture)
                                 ->EnsureSubresourceContentInitialized(recordingContext, dstRange));
                }

                if (src.texture.Get() == dst.texture.Get() && src.mipLevel == dst.mipLevel) {
                    // When there are overlapped subresources, the layout of the overlapped
                    // subresources should all be GENERAL instead of what we set now. Currently
                    // it is not allowed to copy with overlapped subresources, but we still
                    // add the DAWN_ASSERT here as a reminder for this possible misuse.
                    DAWN_ASSERT(!IsRangeOverlapped(src.origin.z, dst.origin.z,
                                                   copy->copySize.depthOrArrayLayers));
                }

                ToBackend(src.texture)
                    ->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopySrc,
                                         wgpu::ShaderStage::None, srcRange);
                ToBackend(dst.texture)
                    ->TransitionUsageNow(recordingContext, wgpu::TextureUsage::CopyDst,
                                         wgpu::ShaderStage::None, dstRange);

                // In some situations we cannot do texture-to-texture copies with vkCmdCopyImage
                // because as Vulkan SPEC always validates image copies with the virtual size of
                // the image subresource, when the extent that fits in the copy region of one
                // subresource but does not fit in the one of another subresource, we will fail
                // to find a valid extent to satisfy the requirements on both source and
                // destination image subresource. For example, when the source is the first
                // level of a 16x16 texture in BC format, and the destination is the third level
                // of a 60x60 texture in the same format, neither 16x16 nor 15x15 is valid as
                // the extent of vkCmdCopyImage.
                // Our workaround for this issue is replacing the texture-to-texture copy with
                // one texture-to-buffer copy and one buffer-to-texture copy.
                bool copyUsingTemporaryBuffer =
                    device->IsToggleEnabled(
                        Toggle::UseTemporaryBufferInCompressedTextureToTextureCopy) &&
                    src.texture->GetFormat().isCompressed &&
                    !HasSameTextureCopyExtent(src, dst, copy->copySize);

                if (!copyUsingTemporaryBuffer) {
                    VkImage srcImage = ToBackend(src.texture)->GetHandle();
                    VkImage dstImage = ToBackend(dst.texture)->GetHandle();
                    Aspect aspects = ToBackend(src.texture)->GetDisjointVulkanAspects();

                    for (Aspect aspect : IterateEnumMask(aspects)) {
                        VkImageCopy region =
                            ComputeImageCopyRegion(src, dst, copy->copySize, aspect);

                        // Dawn guarantees dstImage be in the TRANSFER_DST_OPTIMAL layout after
                        // the copy command.
                        device->fn.CmdCopyImage(commands, srcImage, VK_IMAGE_LAYOUT_GENERAL,
                                                dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                                &region);
                    }
                } else {
                    DAWN_TRY(RecordCopyImageWithTemporaryBuffer(recordingContext, src, dst,
                                                                copy->copySize));
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
                bool clearedToZero = dstBuffer->EnsureDataInitializedAsDestination(
                    recordingContext, cmd->offset, cmd->size);

                if (!clearedToZero) {
                    dstBuffer->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);
                    device->fn.CmdFillBuffer(recordingContext->commandBuffer,
                                             dstBuffer->GetHandle(), cmd->offset, cmd->size, 0u);
                }

                break;
            }

            case Command::BeginRenderPass: {
                BeginRenderPassCmd* cmd = mCommands.NextCommand<BeginRenderPassCmd>();

                DAWN_TRY(PrepareResourcesForRenderPass(
                    device, recordingContext,
                    GetResourceUsages().renderPasses[nextRenderPassNumber]));

                DAWN_TRY(LazyClearRenderPassAttachments(
                    device, cmd, [&](TextureBase* texture, const SubresourceRange& range) {
                        return ToBackend(texture)->EnsureSubresourceContentInitialized(
                            recordingContext, range);
                    }));
                DAWN_TRY(RecordRenderPass(recordingContext, cmd));

                recordingContext->hasRecordedRenderPass = true;
                nextRenderPassNumber++;
                break;
            }

            case Command::BeginComputePass: {
                BeginComputePassCmd* cmd = mCommands.NextCommand<BeginComputePassCmd>();

                // If required, split the command buffer any time a compute pass follows a render
                // pass to work around a Qualcomm bug.
                if (recordingContext->hasRecordedRenderPass &&
                    device->IsToggleEnabled(
                        Toggle::VulkanSplitCommandBufferOnComputePassAfterRenderPass)) {
                    // Identified a potential crash case, split the command buffer.
                    DAWN_TRY(
                        ToBackend(device->GetQueue())->SplitRecordingContext(recordingContext));
                    commands = recordingContext->commandBuffer;
                }

                DAWN_TRY(
                    RecordComputePass(recordingContext, cmd,
                                      GetResourceUsages().computePasses[nextComputePassNumber]));

                nextComputePassNumber++;
                break;
            }

            case Command::ResolveQuerySet: {
                ResolveQuerySetCmd* cmd = mCommands.NextCommand<ResolveQuerySetCmd>();
                QuerySet* querySet = ToBackend(cmd->querySet.Get());
                Buffer* destination = ToBackend(cmd->destination.Get());

                destination->EnsureDataInitializedAsDestination(
                    recordingContext, cmd->destinationOffset, cmd->queryCount * sizeof(uint64_t));

                // vkCmdCopyQueryPoolResults only can retrieve available queries because
                // VK_QUERY_RESULT_WAIT_BIT is set. In order to resolve the unavailable queries
                // as 0s, we need to clear the resolving region of the destination buffer to 0s.
                auto startIt = querySet->GetQueryAvailability().begin() + cmd->firstQuery;
                auto endIt =
                    querySet->GetQueryAvailability().begin() + cmd->firstQuery + cmd->queryCount;
                bool hasUnavailableQueries = std::find(startIt, endIt, false) != endIt;
                // Workaround for resolving overlapping queries to a same buffer on Intel Gen12 GPUs
                // due to Mesa driver issue.
                // See http://crbug.com/dawn/1823 for more information.
                bool clearNeeded = device->IsToggleEnabled(Toggle::ClearBufferBeforeResolveQueries);
                if (hasUnavailableQueries || clearNeeded) {
                    destination->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);
                    device->fn.CmdFillBuffer(commands, destination->GetHandle(),
                                             cmd->destinationOffset,
                                             cmd->queryCount * sizeof(uint64_t), 0u);
                }

                destination->TransitionUsageNow(recordingContext, wgpu::BufferUsage::QueryResolve);

                RecordResolveQuerySetCmd(commands, device, querySet, cmd->firstQuery,
                                         cmd->queryCount, destination, cmd->destinationOffset);

                break;
            }

            case Command::WriteTimestamp: {
                WriteTimestampCmd* cmd = mCommands.NextCommand<WriteTimestampCmd>();

                RecordWriteTimestampCmd(recordingContext, device, cmd->querySet.Get(),
                                        cmd->queryIndex, false, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
                break;
            }

            case Command::InsertDebugMarker: {
                if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
                    InsertDebugMarkerCmd* cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
                    const char* label = mCommands.NextData<char>(cmd->length + 1);
                    VkDebugUtilsLabelEXT utilsLabel;
                    utilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                    utilsLabel.pNext = nullptr;
                    utilsLabel.pLabelName = label;
                    // Default color to black
                    utilsLabel.color[0] = 0.0;
                    utilsLabel.color[1] = 0.0;
                    utilsLabel.color[2] = 0.0;
                    utilsLabel.color[3] = 1.0;
                    device->fn.CmdInsertDebugUtilsLabelEXT(commands, &utilsLabel);
                } else {
                    SkipCommand(&mCommands, Command::InsertDebugMarker);
                }
                break;
            }

            case Command::PopDebugGroup: {
                if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
                    mCommands.NextCommand<PopDebugGroupCmd>();
                    device->fn.CmdEndDebugUtilsLabelEXT(commands);
                } else {
                    SkipCommand(&mCommands, Command::PopDebugGroup);
                }
                break;
            }

            case Command::PushDebugGroup: {
                if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
                    PushDebugGroupCmd* cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                    const char* label = mCommands.NextData<char>(cmd->length + 1);
                    VkDebugUtilsLabelEXT utilsLabel;
                    utilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                    utilsLabel.pNext = nullptr;
                    utilsLabel.pLabelName = label;
                    // Default color to black
                    utilsLabel.color[0] = 0.0;
                    utilsLabel.color[1] = 0.0;
                    utilsLabel.color[2] = 0.0;
                    utilsLabel.color[3] = 1.0;
                    device->fn.CmdBeginDebugUtilsLabelEXT(commands, &utilsLabel);
                } else {
                    SkipCommand(&mCommands, Command::PushDebugGroup);
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

                DAWN_TRY(device->GetDynamicUploader()->WithUploadReservation(
                    size, kCopyBufferToBufferOffsetAlignment,
                    [&](UploadReservation reservation) -> MaybeError {
                        memcpy(reservation.mappedPointer, data, size);

                        dstBuffer->EnsureDataInitializedAsDestination(recordingContext, offset,
                                                                      size);
                        dstBuffer->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);

                        VkBufferCopy copy;
                        copy.srcOffset = reservation.offsetInBuffer;
                        copy.dstOffset = offset;
                        copy.size = size;

                        device->fn.CmdCopyBuffer(commands,
                                                 ToBackend(reservation.buffer)->GetHandle(),
                                                 dstBuffer->GetHandle(), 1, &copy);
                        return {};
                    }));
                break;
            }

            default:
                break;
        }
    }

    return {};
}

MaybeError CommandBuffer::RecordComputePass(CommandRecordingContext* recordingContext,
                                            BeginComputePassCmd* computePassCmd,
                                            const ComputePassResourceUsage& resourceUsages) {
    Device* device = ToBackend(GetDevice());

    // Write timestamp at the beginning of compute pass if it's set
    if (computePassCmd->timestampWrites.beginningOfPassWriteIndex !=
        wgpu::kQuerySetIndexUndefined) {
        RecordWriteTimestampCmd(recordingContext, device,
                                computePassCmd->timestampWrites.querySet.Get(),
                                computePassCmd->timestampWrites.beginningOfPassWriteIndex, false,
                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    }

    VkCommandBuffer commands = recordingContext->commandBuffer;

    uint64_t currentDispatch = 0;
    ComputePassState state(device->fn, recordingContext);

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::EndComputePass: {
                mCommands.NextCommand<EndComputePassCmd>();

                // Write timestamp at the end of compute pass if it's set.
                if (computePassCmd->timestampWrites.endOfPassWriteIndex !=
                    wgpu::kQuerySetIndexUndefined) {
                    RecordWriteTimestampCmd(recordingContext, device,
                                            computePassCmd->timestampWrites.querySet.Get(),
                                            computePassCmd->timestampWrites.endOfPassWriteIndex,
                                            false, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
                }
                return {};
            }

            case Command::Dispatch: {
                DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();

                DAWN_TRY(PrepareResourcesForSyncScope(
                    device, recordingContext, resourceUsages.dispatchUsages[currentDispatch]));
                currentDispatch++;

                DAWN_TRY(state.SyncAndRun([&](const VulkanFunctions& vk, VkCommandBuffer commands) {
                    vk.CmdDispatch(commands, dispatch->x, dispatch->y, dispatch->z);
                }));
                break;
            }

            case Command::DispatchIndirect: {
                DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();
                VkBuffer indirectBuffer = ToBackend(dispatch->indirectBuffer)->GetHandle();

                DAWN_TRY(PrepareResourcesForSyncScope(
                    device, recordingContext, resourceUsages.dispatchUsages[currentDispatch]));
                currentDispatch++;

                DAWN_TRY(state.SyncAndRun([&](const VulkanFunctions& vk, VkCommandBuffer commands) {
                    vk.CmdDispatchIndirect(commands, indirectBuffer,
                                           static_cast<VkDeviceSize>(dispatch->indirectOffset));
                }));
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();

                BindGroup* bindGroup = ToBackend(cmd->group.Get());
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = mCommands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                }

                state.descriptorSets.OnSetBindGroup(cmd->index, bindGroup, cmd->dynamicOffsetCount,
                                                    dynamicOffsets);
                break;
            }

            case Command::SetComputePipeline: {
                SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                state.OnSetPipeline(ToBackend(cmd->pipeline).Get());
                break;
            }

            case Command::InsertDebugMarker: {
                if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
                    InsertDebugMarkerCmd* cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
                    const char* label = mCommands.NextData<char>(cmd->length + 1);
                    VkDebugUtilsLabelEXT utilsLabel;
                    utilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                    utilsLabel.pNext = nullptr;
                    utilsLabel.pLabelName = label;
                    // Default color to black
                    utilsLabel.color[0] = 0.0;
                    utilsLabel.color[1] = 0.0;
                    utilsLabel.color[2] = 0.0;
                    utilsLabel.color[3] = 1.0;
                    device->fn.CmdInsertDebugUtilsLabelEXT(commands, &utilsLabel);
                } else {
                    SkipCommand(&mCommands, Command::InsertDebugMarker);
                }
                break;
            }

            case Command::PopDebugGroup: {
                if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
                    mCommands.NextCommand<PopDebugGroupCmd>();
                    device->fn.CmdEndDebugUtilsLabelEXT(commands);
                } else {
                    SkipCommand(&mCommands, Command::PopDebugGroup);
                }
                break;
            }

            case Command::PushDebugGroup: {
                if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
                    PushDebugGroupCmd* cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                    const char* label = mCommands.NextData<char>(cmd->length + 1);
                    VkDebugUtilsLabelEXT utilsLabel;
                    utilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                    utilsLabel.pNext = nullptr;
                    utilsLabel.pLabelName = label;
                    // Default color to black
                    utilsLabel.color[0] = 0.0;
                    utilsLabel.color[1] = 0.0;
                    utilsLabel.color[2] = 0.0;
                    utilsLabel.color[3] = 1.0;
                    device->fn.CmdBeginDebugUtilsLabelEXT(commands, &utilsLabel);
                } else {
                    SkipCommand(&mCommands, Command::PushDebugGroup);
                }
                break;
            }

            case Command::WriteTimestamp: {
                WriteTimestampCmd* cmd = mCommands.NextCommand<WriteTimestampCmd>();

                RecordWriteTimestampCmd(recordingContext, device, cmd->querySet.Get(),
                                        cmd->queryIndex, false, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
                break;
            }

            case Command::SetImmediates: {
                SetImmediatesCmd* cmd = mCommands.NextCommand<SetImmediatesCmd>();
                DAWN_ASSERT(cmd->size > 0);
                uint8_t* value = nullptr;
                value = mCommands.NextData<uint8_t>(cmd->size);
                state.immediates.SetImmediates(cmd->offset, value, cmd->size);
                break;
            }

            case Command::SetResourceTable: {
                SetResourceTableCmd* cmd = mCommands.NextCommand<SetResourceTableCmd>();
                state.descriptorSets.SetResourceTable(ToBackend(cmd->table.Get()));
                break;
            }

            default:
                DAWN_UNREACHABLE();
        }
    }

    // EndComputePass should have been called
    DAWN_UNREACHABLE();
}

MaybeError CommandBuffer::RecordRenderPass(CommandRecordingContext* recordingContext,
                                           BeginRenderPassCmd* renderPassCmd) {
    Device* device = ToBackend(GetDevice());
    VkCommandBuffer commands = recordingContext->commandBuffer;

    // Write timestamp at the beginning of render pass if it's set.
    // We've observed that this must be called before the render pass or the timestamps produced
    // are nonsensical on multiple Android devices.
    if (renderPassCmd->timestampWrites.beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
        RecordWriteTimestampCmd(recordingContext, device,
                                renderPassCmd->timestampWrites.querySet.Get(),
                                renderPassCmd->timestampWrites.beginningOfPassWriteIndex, true,
                                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    }

    DAWN_TRY(RecordBeginRenderPass(recordingContext, device, renderPassCmd));

    RenderPassState state(device->fn, recordingContext);

    // Set the default value for the dynamic state
    {
        device->fn.CmdSetLineWidth(commands, 1.0f);
        device->fn.CmdSetDepthBounds(commands, 0.0f, 1.0f);

        device->fn.CmdSetStencilReference(commands, VK_STENCIL_FRONT_AND_BACK, 0);

        float blendConstants[4] = {
            0.0f,
            0.0f,
            0.0f,
            0.0f,
        };
        device->fn.CmdSetBlendConstants(commands, blendConstants);

        // The viewport and scissor default to cover all of the attachments
        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = static_cast<float>(renderPassCmd->height);
        viewport.width = static_cast<float>(renderPassCmd->width);
        viewport.height = -static_cast<float>(renderPassCmd->height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        device->fn.CmdSetViewport(commands, 0, 1, &viewport);

        VkRect2D scissorRect;
        scissorRect.offset.x = renderPassCmd->renderArea.x;
        scissorRect.offset.y = renderPassCmd->renderArea.y;
        scissorRect.extent.width = renderPassCmd->renderArea.width;
        scissorRect.extent.height = renderPassCmd->renderArea.height;
        device->fn.CmdSetScissor(commands, 0, 1, &scissorRect);

        // Apply default frag depth
        state.immediates.SetClampFragDepth(0.0, 1.0);
    }


    // Tracks the number of commands that do significant GPU work (a draw or query write) this pass.
    uint32_t workCommandCount = 0;

    auto EncodeRenderBundleCommand = [&](CommandIterator* iter, Command type) -> MaybeError {
        switch (type) {
            case Command::Draw: {
                workCommandCount++;
                DrawCmd* draw = iter->NextCommand<DrawCmd>();

                DAWN_TRY(state.SyncAndRun([&](const VulkanFunctions& vk, VkCommandBuffer commands) {
                    vk.CmdDraw(commands, draw->vertexCount, draw->instanceCount, draw->firstVertex,
                               draw->firstInstance);
                }));
                break;
            }

            case Command::DrawIndexed: {
                workCommandCount++;
                DrawIndexedCmd* draw = iter->NextCommand<DrawIndexedCmd>();

                DAWN_TRY(state.SyncAndRun([&](const VulkanFunctions& vk, VkCommandBuffer commands) {
                    vk.CmdDrawIndexed(commands, draw->indexCount, draw->instanceCount,
                                      draw->firstIndex, draw->baseVertex, draw->firstInstance);
                }));
                break;
            }

            case Command::DrawIndirect: {
                workCommandCount++;
                DrawIndirectCmd* draw = iter->NextCommand<DrawIndirectCmd>();
                Buffer* buffer = ToBackend(draw->indirectBuffer.Get());

                DAWN_TRY(state.SyncAndRun([&](const VulkanFunctions& vk, VkCommandBuffer commands) {
                    vk.CmdDrawIndirect(commands, buffer->GetHandle(),
                                       static_cast<VkDeviceSize>(draw->indirectOffset), 1, 0);
                }));
                break;
            }

            case Command::DrawIndexedIndirect: {
                workCommandCount++;
                DrawIndexedIndirectCmd* draw = iter->NextCommand<DrawIndexedIndirectCmd>();
                Buffer* buffer = ToBackend(draw->indirectBuffer.Get());
                DAWN_ASSERT(buffer != nullptr);

                DAWN_TRY(state.SyncAndRun([&](const VulkanFunctions& vk, VkCommandBuffer commands) {
                    vk.CmdDrawIndexedIndirect(commands, buffer->GetHandle(),
                                              static_cast<VkDeviceSize>(draw->indirectOffset), 1,
                                              0);
                }));
                break;
            }

            case Command::MultiDrawIndirect: {
                workCommandCount++;
                MultiDrawIndirectCmd* cmd = iter->NextCommand<MultiDrawIndirectCmd>();

                Buffer* indirectBuffer = ToBackend(cmd->indirectBuffer.Get());
                DAWN_ASSERT(indirectBuffer != nullptr);

                // Count buffer is optional
                Buffer* countBuffer = ToBackend(cmd->drawCountBuffer.Get());

                DAWN_TRY(state.SyncAndRun([&](const VulkanFunctions& vk, VkCommandBuffer commands) {
                    if (countBuffer == nullptr) {
                        vk.CmdDrawIndirect(commands, indirectBuffer->GetHandle(),
                                           static_cast<VkDeviceSize>(cmd->indirectOffset),
                                           cmd->maxDrawCount, kDrawIndirectSize);
                    } else {
                        vk.CmdDrawIndirectCountKHR(commands, indirectBuffer->GetHandle(),
                                                   static_cast<VkDeviceSize>(cmd->indirectOffset),
                                                   countBuffer->GetHandle(),
                                                   static_cast<VkDeviceSize>(cmd->drawCountOffset),
                                                   cmd->maxDrawCount, kDrawIndirectSize);
                    }
                }));
                break;
            }
            case Command::MultiDrawIndexedIndirect: {
                workCommandCount++;
                MultiDrawIndexedIndirectCmd* cmd = iter->NextCommand<MultiDrawIndexedIndirectCmd>();

                Buffer* indirectBuffer = ToBackend(cmd->indirectBuffer.Get());
                DAWN_ASSERT(indirectBuffer != nullptr);

                // Count buffer is optional
                Buffer* countBuffer = ToBackend(cmd->drawCountBuffer.Get());

                DAWN_TRY(state.SyncAndRun([&](const VulkanFunctions& vk, VkCommandBuffer commands) {
                    if (countBuffer == nullptr) {
                        vk.CmdDrawIndexedIndirect(commands, indirectBuffer->GetHandle(),
                                                  static_cast<VkDeviceSize>(cmd->indirectOffset),
                                                  cmd->maxDrawCount, kDrawIndexedIndirectSize);
                    } else {
                        vk.CmdDrawIndexedIndirectCountKHR(
                            commands, indirectBuffer->GetHandle(),
                            static_cast<VkDeviceSize>(cmd->indirectOffset),
                            countBuffer->GetHandle(),
                            static_cast<VkDeviceSize>(cmd->drawCountOffset), cmd->maxDrawCount,
                            kDrawIndexedIndirectSize);
                    }
                }));

                break;
            }

            case Command::InsertDebugMarker: {
                if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
                    InsertDebugMarkerCmd* cmd = iter->NextCommand<InsertDebugMarkerCmd>();
                    const char* label = iter->NextData<char>(cmd->length + 1);
                    VkDebugUtilsLabelEXT utilsLabel;
                    utilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                    utilsLabel.pNext = nullptr;
                    utilsLabel.pLabelName = label;
                    // Default color to black
                    utilsLabel.color[0] = 0.0;
                    utilsLabel.color[1] = 0.0;
                    utilsLabel.color[2] = 0.0;
                    utilsLabel.color[3] = 1.0;
                    device->fn.CmdInsertDebugUtilsLabelEXT(commands, &utilsLabel);
                } else {
                    SkipCommand(iter, Command::InsertDebugMarker);
                }
                break;
            }

            case Command::PopDebugGroup: {
                if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
                    iter->NextCommand<PopDebugGroupCmd>();
                    device->fn.CmdEndDebugUtilsLabelEXT(commands);
                } else {
                    SkipCommand(iter, Command::PopDebugGroup);
                }
                break;
            }

            case Command::PushDebugGroup: {
                if (device->GetGlobalInfo().HasExt(InstanceExt::DebugUtils)) {
                    PushDebugGroupCmd* cmd = iter->NextCommand<PushDebugGroupCmd>();
                    const char* label = iter->NextData<char>(cmd->length + 1);
                    VkDebugUtilsLabelEXT utilsLabel;
                    utilsLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
                    utilsLabel.pNext = nullptr;
                    utilsLabel.pLabelName = label;
                    // Default color to black
                    utilsLabel.color[0] = 0.0;
                    utilsLabel.color[1] = 0.0;
                    utilsLabel.color[2] = 0.0;
                    utilsLabel.color[3] = 1.0;
                    device->fn.CmdBeginDebugUtilsLabelEXT(commands, &utilsLabel);
                } else {
                    SkipCommand(iter, Command::PushDebugGroup);
                }
                break;
            }

            case Command::SetBindGroup: {
                SetBindGroupCmd* cmd = iter->NextCommand<SetBindGroupCmd>();
                BindGroup* bindGroup = ToBackend(cmd->group.Get());
                uint32_t* dynamicOffsets = nullptr;
                if (cmd->dynamicOffsetCount > 0) {
                    dynamicOffsets = iter->NextData<uint32_t>(cmd->dynamicOffsetCount);
                }

                state.descriptorSets.OnSetBindGroup(cmd->index, bindGroup, cmd->dynamicOffsetCount,
                                                    dynamicOffsets);
                break;
            }

            case Command::SetIndexBuffer: {
                SetIndexBufferCmd* cmd = iter->NextCommand<SetIndexBufferCmd>();
                VkBuffer indexBuffer = ToBackend(cmd->buffer)->GetHandle();

                device->fn.CmdBindIndexBuffer(commands, indexBuffer, cmd->offset,
                                              VulkanIndexType(cmd->format));
                break;
            }

            case Command::SetRenderPipeline: {
                SetRenderPipelineCmd* cmd = iter->NextCommand<SetRenderPipelineCmd>();
                state.OnSetPipeline(ToBackend(cmd->pipeline).Get());
                break;
            }

            case Command::SetVertexBuffer: {
                SetVertexBufferCmd* cmd = iter->NextCommand<SetVertexBufferCmd>();
                VkBuffer buffer = ToBackend(cmd->buffer)->GetHandle();
                VkDeviceSize offset = static_cast<VkDeviceSize>(cmd->offset);

                device->fn.CmdBindVertexBuffers(commands, static_cast<uint8_t>(cmd->slot), 1,
                                                &*buffer, &offset);
                break;
            }

            case Command::SetImmediates: {
                SetImmediatesCmd* cmd = iter->NextCommand<SetImmediatesCmd>();
                DAWN_ASSERT(cmd->size > 0);
                uint8_t* value = nullptr;
                value = iter->NextData<uint8_t>(cmd->size);
                state.immediates.SetImmediates(cmd->offset, value, cmd->size);
                break;
            }

            case Command::SetResourceTable: {
                SetResourceTableCmd* cmd = iter->NextCommand<SetResourceTableCmd>();
                state.descriptorSets.SetResourceTable(ToBackend(cmd->table.Get()));
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

                // If no work-producing commands were executed during the render pass and the
                // VulkanAddWorkToEmptyResolvePass toggle is enabled, add a small amount of work
                // in the form of performing an occlusion query before ending the pass. This avoids
                // a driver bug that fails to resolve render targets in empty passes.
                if (workCommandCount == 0 &&
                    device->IsToggleEnabled(Toggle::VulkanAddWorkToEmptyResolvePass)) {
                    QuerySetBase* querySet = device->GetEmptyPassQuerySet();

                    // Ensure that the query was allocated. In the unlikely event that it wasn't
                    // (probably due to an OOM) empty render passes can't be safely completed using
                    // this workaround.
                    DAWN_INTERNAL_ERROR_IF(querySet->IsError(),
                                           "Unable to allocate internal empty pass query set.");

                    device->fn.CmdBeginQuery(commands, ToBackend(querySet)->GetHandle(), 0, 0);
                    device->fn.CmdEndQuery(commands, ToBackend(querySet)->GetHandle(), 0);
                }

                RecordEndRenderPass(recordingContext, device);

                // Write timestamp at the end of render pass if it's set.
                // We've observed that this must be called after the render pass ends or the
                // timestamps produced are nonsensical on multiple Android devices.
                if (renderPassCmd->timestampWrites.endOfPassWriteIndex !=
                    wgpu::kQuerySetIndexUndefined) {
                    RecordWriteTimestampCmd(recordingContext, device,
                                            renderPassCmd->timestampWrites.querySet.Get(),
                                            renderPassCmd->timestampWrites.endOfPassWriteIndex,
                                            true, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
                }

                return {};
            }

            case Command::SetBlendConstant: {
                SetBlendConstantCmd* cmd = mCommands.NextCommand<SetBlendConstantCmd>();
                const std::array<float, 4> blendConstants = ConvertToFloatColor(cmd->color);
                device->fn.CmdSetBlendConstants(commands, blendConstants.data());
                break;
            }

            case Command::SetStencilReference: {
                SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();
                device->fn.CmdSetStencilReference(commands, VK_STENCIL_FRONT_AND_BACK,
                                                  cmd->reference);
                break;
            }

            case Command::SetViewport: {
                SetViewportCmd* cmd = mCommands.NextCommand<SetViewportCmd>();
                VkViewport viewport;
                viewport.x = cmd->x;
                viewport.y = cmd->y + cmd->height;
                viewport.width = cmd->width;
                viewport.height = -cmd->height;
                viewport.minDepth = cmd->minDepth;
                viewport.maxDepth = cmd->maxDepth;

                // Vulkan disallows width = 0, but VK_KHR_maintenance1 which we require allows
                // height = 0 so use that to do an empty viewport.
                if (viewport.width == 0) {
                    viewport.height = 0;

                    // Set the viewport x range to a range that's always valid.
                    viewport.x = 0;
                    viewport.width = 1;
                }

                device->fn.CmdSetViewport(commands, 0, 1, &viewport);

                // Try applying the immediate data that contain min/maxDepth immediately. This can
                // be deferred if no pipeline is currently bound.
                state.immediates.SetClampFragDepth(viewport.minDepth, viewport.maxDepth);
                break;
            }

            case Command::SetScissorRect: {
                SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                VkRect2D rect;
                rect.offset.x = cmd->x;
                rect.offset.y = cmd->y;
                rect.extent.width = cmd->width;
                rect.extent.height = cmd->height;

                device->fn.CmdSetScissor(commands, 0, 1, &rect);
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
                workCommandCount++;
                BeginOcclusionQueryCmd* cmd = mCommands.NextCommand<BeginOcclusionQueryCmd>();

                device->fn.CmdBeginQuery(commands, ToBackend(cmd->querySet.Get())->GetHandle(),
                                         cmd->queryIndex, 0);
                break;
            }

            case Command::EndOcclusionQuery: {
                workCommandCount++;
                EndOcclusionQueryCmd* cmd = mCommands.NextCommand<EndOcclusionQueryCmd>();

                device->fn.CmdEndQuery(commands, ToBackend(cmd->querySet.Get())->GetHandle(),
                                       cmd->queryIndex);
                break;
            }

            case Command::WriteTimestamp: {
                workCommandCount++;
                WriteTimestampCmd* cmd = mCommands.NextCommand<WriteTimestampCmd>();

                RecordWriteTimestampCmd(recordingContext, device, cmd->querySet.Get(),
                                        cmd->queryIndex, true, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
                break;
            }

            default: {
                DAWN_TRY(EncodeRenderBundleCommand(&mCommands, type));
                break;
            }
        }
    }

    // EndRenderPass should have been called
    DAWN_UNREACHABLE();
}

}  // namespace dawn::native::vulkan
