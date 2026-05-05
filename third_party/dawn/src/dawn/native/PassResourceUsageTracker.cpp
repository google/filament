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

#include "dawn/native/PassResourceUsageTracker.h"

#include <utility>

#include "dawn/common/MatchVariant.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/Format.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/TexelBufferView.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

SyncScopeUsageTracker::SyncScopeUsageTracker() = default;

SyncScopeUsageTracker::SyncScopeUsageTracker(SyncScopeUsageTracker&&) = default;

SyncScopeUsageTracker::~SyncScopeUsageTracker() = default;

SyncScopeUsageTracker& SyncScopeUsageTracker::operator=(SyncScopeUsageTracker&&) = default;

void SyncScopeUsageTracker::BufferUsedAs(BufferBase* buffer,
                                         wgpu::BufferUsage usage,
                                         wgpu::ShaderStage shaderStages) {
    // absl::flat_hash_map's operator[] will create a new element using the default constructor
    // if the key didn't exist before.
    BufferSyncInfo& bufferSyncInfo = mBufferSyncInfos[buffer];

    bufferSyncInfo.usage |= usage;
    bufferSyncInfo.shaderStages |= shaderStages;
}

void SyncScopeUsageTracker::TextureViewUsedAs(TextureViewBase* view,
                                              wgpu::TextureUsage usage,
                                              wgpu::ShaderStage shaderStages) {
    TextureRangeUsedAs(view->GetTexture(), view->GetSubresourceRange(), usage, shaderStages);
}

void SyncScopeUsageTracker::TextureRangeUsedAs(TextureBase* texture,
                                               const SubresourceRange& range,
                                               wgpu::TextureUsage usage,
                                               wgpu::ShaderStage shaderStages) {
    // Get or create a new TextureSubresourceSyncInfo for that texture (initially filled with
    // wgpu::TextureUsage::None and WGPUShaderStage_None)
    auto it = mTextureSyncInfos.try_emplace(
        texture, texture->GetFormat().aspects, texture->GetArrayLayers(),
        texture->GetNumMipLevels(),
        TextureSyncInfo{wgpu::TextureUsage::None, wgpu::ShaderStage::None});
    TextureSubresourceSyncInfo& textureSyncInfo = it.first->second;

    textureSyncInfo.Update(
        range, [usage, shaderStages](const SubresourceRange&, TextureSyncInfo* storedSyncInfo) {
            storedSyncInfo->usage |= usage;
            storedSyncInfo->shaderStages |= shaderStages;
        });
}

void SyncScopeUsageTracker::MergeTextureUsage(TextureBase* texture,
                                              const TextureSubresourceSyncInfo& textureSyncInfo) {
    // Get or create a new TextureSubresourceSyncInfo for that texture (initially filled with
    // wgpu::TextureUsage::None and WGPUShaderStage_None)
    auto it = mTextureSyncInfos.try_emplace(
        texture, texture->GetFormat().aspects, texture->GetArrayLayers(),
        texture->GetNumMipLevels(),
        TextureSyncInfo{wgpu::TextureUsage::None, wgpu::ShaderStage::None});
    TextureSubresourceSyncInfo& passTextureSyncInfo = it.first->second;

    passTextureSyncInfo.Merge(
        textureSyncInfo, [](const SubresourceRange&, TextureSyncInfo* storedSyncInfo,
                            const TextureSyncInfo& addedSyncInfo) {
            DAWN_ASSERT((addedSyncInfo.usage & wgpu::TextureUsage::RenderAttachment) == 0);
            storedSyncInfo->usage |= addedSyncInfo.usage;
            storedSyncInfo->shaderStages |= addedSyncInfo.shaderStages;
        });
}

void SyncScopeUsageTracker::MergeResourceUsages(const SyncScopeResourceUsage& usages) {
    for (uint32_t i = 0; i < usages.buffers.size(); ++i) {
        BufferUsedAs(usages.buffers[i], usages.bufferSyncInfos[i].usage,
                     usages.bufferSyncInfos[i].shaderStages);
    }

    for (uint32_t i = 0; i < usages.textures.size(); ++i) {
        MergeTextureUsage(usages.textures[i], usages.textureSyncInfos[i]);
    }

    for (ExternalTextureBase* t : usages.externalTextures) {
        mExternalTextureUsages.insert(t);
    }

    for (ResourceTableBase* t : usages.usedResourceTables) {
        mUsedResourceTables.insert(t);
    }
}

void SyncScopeUsageTracker::AddBindGroup(BindGroupBase* group) {
    const auto* layout = group->GetLayout();

    for (BindingIndex i : layout->GetBufferIndices()) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(i);
        const BufferBindingInfo& bufferInfo =
            std::get<BufferBindingInfo>(bindingInfo.bindingLayout);

        wgpu::BufferUsage usage;
        switch (bufferInfo.type) {
            case wgpu::BufferBindingType::Uniform:
                usage = wgpu::BufferUsage::Uniform;
                break;
            case wgpu::BufferBindingType::Storage:
                usage = wgpu::BufferUsage::Storage;
                break;
            case kInternalStorageBufferBinding:
                usage = kInternalStorageBuffer;
                break;
            case wgpu::BufferBindingType::ReadOnlyStorage:
            case kInternalReadOnlyStorageBufferBinding:
                usage = kReadOnlyStorageBuffer;
                break;
            case wgpu::BufferBindingType::BindingNotUsed:
            case wgpu::BufferBindingType::Undefined:
                DAWN_UNREACHABLE();
        }

        BufferBase* buffer = group->GetBindingAsBuffer(i);
        BufferUsedAs(buffer, usage, bindingInfo.visibility);
    }

    for (BindingIndex i : layout->GetSampledTextureIndices()) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(i);
        const TextureBindingInfo& textureInfo =
            std::get<TextureBindingInfo>(bindingInfo.bindingLayout);

        wgpu::TextureUsage usage;
        switch (textureInfo.sampleType) {
            case kInternalResolveAttachmentSampleType:
                usage = kResolveAttachmentLoadingUsage;
                break;
            default:
                usage = wgpu::TextureUsage::TextureBinding;
                break;
        }

        TextureViewBase* view = group->GetBindingAsTextureView(i);
        TextureViewUsedAs(view, usage, bindingInfo.visibility);
    }

    for (BindingIndex i : layout->GetStorageTextureIndices()) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(i);
        const StorageTextureBindingInfo& textureInfo =
            std::get<StorageTextureBindingInfo>(bindingInfo.bindingLayout);

        wgpu::TextureUsage usage;
        switch (textureInfo.access) {
            case wgpu::StorageTextureAccess::WriteOnly:
                usage = kWriteOnlyStorageTexture;
                break;
            case wgpu::StorageTextureAccess::ReadWrite:
                usage = wgpu::TextureUsage::StorageBinding;
                break;
            case wgpu::StorageTextureAccess::ReadOnly:
                usage = kReadOnlyStorageTexture;
                break;
            case wgpu::StorageTextureAccess::BindingNotUsed:
            case wgpu::StorageTextureAccess::Undefined:
                DAWN_UNREACHABLE();
        }

        TextureViewBase* view = group->GetBindingAsTextureView(i);
        TextureViewUsedAs(view, usage, bindingInfo.visibility);
    }

    for (BindingIndex i : layout->GetTexelBufferIndices()) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(i);
        const TexelBufferBindingInfo& texelInfo =
            std::get<TexelBufferBindingInfo>(bindingInfo.bindingLayout);

        wgpu::BufferUsage usage = wgpu::BufferUsage::None;
        switch (texelInfo.access) {
            case wgpu::TexelBufferAccess::ReadOnly:
                usage = kReadOnlyTexelBuffer;
                break;
            case wgpu::TexelBufferAccess::ReadWrite:
                usage = wgpu::BufferUsage::Storage;
                break;
            case wgpu::TexelBufferAccess::Undefined:
                DAWN_UNREACHABLE();
        }

        BufferBase* buffer = group->GetBindingAsTexelBufferView(i)->GetBuffer();
        BufferUsedAs(buffer, usage, bindingInfo.visibility);
    }

    for (const Ref<ExternalTextureBase>& externalTexture : group->GetBoundExternalTextures()) {
        // The ref of bound external texture can be null.
        if (externalTexture) {
            mExternalTextureUsages.insert(externalTexture.Get());
        }
    }
}

void SyncScopeUsageTracker::AddResourceTableUsage(ResourceTableBase* table) {
    mUsedResourceTables.insert(table);
}

SyncScopeResourceUsage SyncScopeUsageTracker::AcquireSyncScopeUsage() {
    SyncScopeResourceUsage result;
    result.buffers.reserve(mBufferSyncInfos.size());
    result.bufferSyncInfos.reserve(mBufferSyncInfos.size());
    result.textures.reserve(mTextureSyncInfos.size());
    result.textureSyncInfos.reserve(mTextureSyncInfos.size());

    for (auto& [buffer, syncInfo] : mBufferSyncInfos) {
        result.buffers.push_back(buffer);
        result.bufferSyncInfos.push_back(std::move(syncInfo));
    }
    mBufferSyncInfos.clear();

    for (auto& [texture, syncInfo] : mTextureSyncInfos) {
        result.textures.push_back(texture);
        result.textureSyncInfos.push_back(std::move(syncInfo));
    }
    mTextureSyncInfos.clear();

    for (auto* const it : mExternalTextureUsages) {
        result.externalTextures.push_back(it);
    }
    mExternalTextureUsages.clear();

    for (auto* const it : mUsedResourceTables) {
        result.usedResourceTables.push_back(it);
    }
    mUsedResourceTables.clear();

    return result;
}

ComputePassResourceUsageTracker::ComputePassResourceUsageTracker() = default;

ComputePassResourceUsageTracker::~ComputePassResourceUsageTracker() = default;

void ComputePassResourceUsageTracker::AddDispatch(SyncScopeResourceUsage scope) {
    mUsage.dispatchUsages.push_back(std::move(scope));
}

void ComputePassResourceUsageTracker::AddReferencedBuffer(BufferBase* buffer) {
    mUsage.referencedBuffers.insert(buffer);
}

void ComputePassResourceUsageTracker::AddReferencedResourceTable(ResourceTableBase* table) {
    mUsage.referencedResourceTables.insert(table);
}

void ComputePassResourceUsageTracker::AddResourcesReferencedByBindGroup(BindGroupBase* group) {
    const auto* layout = group->GetLayout();

    for (BindingIndex i : layout->GetBufferIndices()) {
        mUsage.referencedBuffers.insert(group->GetBindingAsBuffer(i));
    }

    for (BindingIndex i : layout->GetTextureIndices()) {
        mUsage.referencedTextures.insert(group->GetBindingAsTextureView(i)->GetTexture());
    }

    for (BindingIndex i : layout->GetTexelBufferIndices()) {
        mUsage.referencedBuffers.insert(group->GetBindingAsTexelBufferView(i)->GetBuffer());
    }

    for (const Ref<ExternalTextureBase>& externalTexture : group->GetBoundExternalTextures()) {
        // The ref of bound external texture can be null.
        if (externalTexture) {
            mUsage.referencedExternalTextures.insert(externalTexture.Get());
        }
    }
}

ComputePassResourceUsage ComputePassResourceUsageTracker::AcquireResourceUsage() {
    return std::move(mUsage);
}

RenderPassResourceUsageTracker::RenderPassResourceUsageTracker() = default;

RenderPassResourceUsageTracker::RenderPassResourceUsageTracker(RenderPassResourceUsageTracker&&) =
    default;

RenderPassResourceUsageTracker::~RenderPassResourceUsageTracker() = default;

RenderPassResourceUsageTracker& RenderPassResourceUsageTracker::operator=(
    RenderPassResourceUsageTracker&&) = default;

RenderPassResourceUsage RenderPassResourceUsageTracker::AcquireResourceUsage() {
    RenderPassResourceUsage result;
    *static_cast<SyncScopeResourceUsage*>(&result) = AcquireSyncScopeUsage();

    result.querySets.reserve(mQueryAvailabilities.size());
    result.queryAvailabilities.reserve(mQueryAvailabilities.size());

    for (auto& it : mQueryAvailabilities) {
        result.querySets.push_back(it.first);
        result.queryAvailabilities.push_back(std::move(it.second));
    }

    mQueryAvailabilities.clear();

    return result;
}

void RenderPassResourceUsageTracker::TrackQueryAvailability(QuerySetBase* querySet,
                                                            uint32_t queryIndex) {
    // The query availability only needs to be tracked again on render passes for checking
    // query overwrite on render pass and resetting query sets on the Vulkan backend.
    DAWN_ASSERT(querySet != nullptr);

    // Gets the iterator for that querySet or create a new vector of bool set to false
    // if the querySet wasn't registered.
    auto it = mQueryAvailabilities.try_emplace(querySet, querySet->GetQueryCount()).first;
    it->second[queryIndex] = true;
}

const QueryAvailabilityMap& RenderPassResourceUsageTracker::GetQueryAvailabilityMap() const {
    return mQueryAvailabilities;
}

}  // namespace dawn::native
