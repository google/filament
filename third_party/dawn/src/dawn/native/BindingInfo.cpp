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

#include "dawn/native/BindingInfo.h"

#include "dawn/common/MatchVariant.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Limits.h"
#include "dawn/native/Sampler.h"

namespace dawn::native {

BindingInfoType GetBindingInfoType(const BindingInfo& info) {
    return MatchVariant(
        info.bindingLayout,
        [](const BufferBindingInfo&) -> BindingInfoType { return BindingInfoType::Buffer; },
        [](const SamplerBindingInfo&) -> BindingInfoType { return BindingInfoType::Sampler; },
        [](const TextureBindingInfo&) -> BindingInfoType { return BindingInfoType::Texture; },
        [](const StorageTextureBindingInfo&) -> BindingInfoType {
            return BindingInfoType::StorageTexture;
        },
        [](const StaticSamplerBindingInfo&) -> BindingInfoType {
            return BindingInfoType::StaticSampler;
        },
        [](const InputAttachmentBindingInfo&) -> BindingInfoType {
            return BindingInfoType::InputAttachment;
        });
}

void IncrementBindingCounts(BindingCounts* bindingCounts,
                            const UnpackedPtr<BindGroupLayoutEntry>& entry) {
    bindingCounts->totalCount += 1;

    uint32_t PerStageBindingCounts::*perStageBindingCountMember = nullptr;

    if (entry->buffer.type != wgpu::BufferBindingType::BindingNotUsed) {
        ++bindingCounts->bufferCount;
        const BufferBindingLayout& buffer = entry->buffer;

        if (buffer.minBindingSize == 0) {
            ++bindingCounts->unverifiedBufferCount;
        }

        switch (buffer.type) {
            case wgpu::BufferBindingType::Uniform:
                if (buffer.hasDynamicOffset) {
                    ++bindingCounts->dynamicUniformBufferCount;
                }
                perStageBindingCountMember = &PerStageBindingCounts::uniformBufferCount;
                break;

            case wgpu::BufferBindingType::Storage:
            case kInternalStorageBufferBinding:
            case wgpu::BufferBindingType::ReadOnlyStorage:
                if (buffer.hasDynamicOffset) {
                    ++bindingCounts->dynamicStorageBufferCount;
                }
                perStageBindingCountMember = &PerStageBindingCounts::storageBufferCount;
                break;

            case wgpu::BufferBindingType::BindingNotUsed:
                // Can't get here due to the enclosing if statement.
            case wgpu::BufferBindingType::Undefined:
                DAWN_UNREACHABLE();
                break;
        }
    } else if (entry->sampler.type != wgpu::SamplerBindingType::BindingNotUsed) {
        perStageBindingCountMember = &PerStageBindingCounts::samplerCount;
    } else if (entry->texture.sampleType != wgpu::TextureSampleType::BindingNotUsed) {
        if (entry->texture.viewDimension == kInternalInputAttachmentDim) {
            // Internal use only.
            return;
        } else {
            perStageBindingCountMember = &PerStageBindingCounts::sampledTextureCount;
        }
    } else if (entry->storageTexture.access != wgpu::StorageTextureAccess::BindingNotUsed) {
        perStageBindingCountMember = &PerStageBindingCounts::storageTextureCount;
    } else if (entry.Get<ExternalTextureBindingLayout>()) {
        perStageBindingCountMember = &PerStageBindingCounts::externalTextureCount;
    } else if (entry.Get<StaticSamplerBindingLayout>()) {
        ++bindingCounts->staticSamplerCount;
        perStageBindingCountMember = &PerStageBindingCounts::staticSamplerCount;
    }

    DAWN_ASSERT(perStageBindingCountMember != nullptr);
    for (SingleShaderStage stage : IterateStages(entry->visibility)) {
        ++(bindingCounts->perStage[stage].*perStageBindingCountMember);
    }
}

void AccumulateBindingCounts(BindingCounts* bindingCounts, const BindingCounts& rhs) {
    bindingCounts->totalCount += rhs.totalCount;
    bindingCounts->bufferCount += rhs.bufferCount;
    bindingCounts->unverifiedBufferCount += rhs.unverifiedBufferCount;
    bindingCounts->dynamicUniformBufferCount += rhs.dynamicUniformBufferCount;
    bindingCounts->dynamicStorageBufferCount += rhs.dynamicStorageBufferCount;

    for (SingleShaderStage stage : IterateStages(kAllStages)) {
        bindingCounts->perStage[stage].sampledTextureCount +=
            rhs.perStage[stage].sampledTextureCount;
        bindingCounts->perStage[stage].samplerCount += rhs.perStage[stage].samplerCount;
        bindingCounts->perStage[stage].storageBufferCount += rhs.perStage[stage].storageBufferCount;
        bindingCounts->perStage[stage].storageTextureCount +=
            rhs.perStage[stage].storageTextureCount;
        bindingCounts->perStage[stage].uniformBufferCount += rhs.perStage[stage].uniformBufferCount;
        bindingCounts->perStage[stage].externalTextureCount +=
            rhs.perStage[stage].externalTextureCount;
        bindingCounts->perStage[stage].staticSamplerCount += rhs.perStage[stage].staticSamplerCount;
    }
}

MaybeError ValidateBindingCounts(const CombinedLimits& limits,
                                 const BindingCounts& bindingCounts,
                                 const AdapterBase* adapter) {
    uint32_t maxDynamicUniformBuffersPerPipelineLayout =
        limits.v1.maxDynamicUniformBuffersPerPipelineLayout;
    DAWN_INVALID_IF(
        bindingCounts.dynamicUniformBufferCount > maxDynamicUniformBuffersPerPipelineLayout,
        "The number of dynamic uniform buffers (%u) exceeds the maximum per-pipeline-layout "
        "limit (%u).%s",
        bindingCounts.dynamicUniformBufferCount, maxDynamicUniformBuffersPerPipelineLayout,
        DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxDynamicUniformBuffersPerPipelineLayout,
                                    bindingCounts.dynamicUniformBufferCount));

    uint32_t maxDynamicStorageBuffersPerPipelineLayout =
        limits.v1.maxDynamicStorageBuffersPerPipelineLayout;
    DAWN_INVALID_IF(
        bindingCounts.dynamicStorageBufferCount > maxDynamicStorageBuffersPerPipelineLayout,
        "The number of dynamic storage buffers (%u) exceeds the maximum per-pipeline-layout "
        "limit (%u).%s",
        bindingCounts.dynamicStorageBufferCount, maxDynamicStorageBuffersPerPipelineLayout,
        DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxDynamicStorageBuffersPerPipelineLayout,
                                    bindingCounts.dynamicStorageBufferCount));

    uint32_t maxSampledTexturesPerShaderStage = limits.v1.maxSampledTexturesPerShaderStage;
    uint32_t maxSamplersPerShaderStage = limits.v1.maxSamplersPerShaderStage;
    uint32_t maxStorageBuffersInFragmentStage = limits.v1.maxStorageBuffersInFragmentStage;
    uint32_t maxStorageBuffersInVertexStage = limits.v1.maxStorageBuffersInVertexStage;
    uint32_t maxStorageBuffersPerShaderStage = limits.v1.maxStorageBuffersPerShaderStage;
    uint32_t maxUniformBuffersPerShaderStage = limits.v1.maxUniformBuffersPerShaderStage;
    uint32_t maxStorageTexturesInFragmentStage = limits.v1.maxStorageTexturesInFragmentStage;
    uint32_t maxStorageTexturesInVertexStage = limits.v1.maxStorageTexturesInVertexStage;
    uint32_t maxStorageTexturesPerShaderStage = limits.v1.maxStorageTexturesPerShaderStage;
    for (SingleShaderStage stage : IterateStages(kAllStages)) {
        uint32_t sampledTextureCount = bindingCounts.perStage[stage].sampledTextureCount;
        DAWN_INVALID_IF(sampledTextureCount > maxSampledTexturesPerShaderStage,
                        "The number of sampled textures (%u) in the %s stage exceeds the maximum "
                        "per-stage limit (%u).%s",
                        sampledTextureCount, stage, maxSampledTexturesPerShaderStage,
                        DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxSampledTexturesPerShaderStage,
                                                    sampledTextureCount));

        uint32_t externalTextureCount = bindingCounts.perStage[stage].externalTextureCount;
        uint32_t sampledTextureAndExternalTextureCount =
            sampledTextureCount + (externalTextureCount * kSampledTexturesPerExternalTexture);
        DAWN_INVALID_IF(
            sampledTextureAndExternalTextureCount > maxSampledTexturesPerShaderStage,
            "The combination of sampled textures (%u) and external textures (%u × %u) in the %s "
            "stage exceeds the maximum per-stage limit (%u).%s",
            sampledTextureCount, externalTextureCount, kSampledTexturesPerExternalTexture, stage,
            maxSampledTexturesPerShaderStage,
            DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxSampledTexturesPerShaderStage,
                                        sampledTextureCount));

        uint32_t samplerCount = bindingCounts.perStage[stage].samplerCount;
        // TODO(crbug.com/dawn/2463): Account for static samplers here.
        DAWN_INVALID_IF(
            samplerCount > maxSamplersPerShaderStage,
            "The number of samplers (%u) in the %s stage exceeds the maximum per-stage limit "
            "(%u).%s",
            samplerCount, stage, maxSamplersPerShaderStage,
            DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxSamplersPerShaderStage, samplerCount));

        uint32_t samplerAndExternalTextureCount =
            samplerCount + (externalTextureCount * kSamplersPerExternalTexture);
        // TODO(crbug.com/dawn/2463): Account for static samplers here.
        DAWN_INVALID_IF(
            samplerAndExternalTextureCount > maxSamplersPerShaderStage,
            "The combination of samplers (%u) and external textures (%u × %u) in the %s stage "
            "exceeds the maximum per-stage limit (%u).%s",
            samplerCount, externalTextureCount, kSamplersPerExternalTexture, stage,
            maxSamplersPerShaderStage,
            DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxSamplersPerShaderStage,
                                        samplerAndExternalTextureCount));

        uint32_t storageBufferCount = bindingCounts.perStage[stage].storageBufferCount;
        DAWN_INVALID_IF(
            storageBufferCount > maxStorageBuffersPerShaderStage,
            "The number of storage buffers (%u) in the %s stage exceeds the maximum per-stage "
            "limit (%u).%s",
            storageBufferCount, stage, maxStorageBuffersPerShaderStage,
            DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxStorageBuffersPerShaderStage,
                                        storageBufferCount));

        uint32_t storageTextureCount = bindingCounts.perStage[stage].storageTextureCount;
        DAWN_INVALID_IF(
            storageTextureCount > maxStorageTexturesPerShaderStage,
            "The number of storage textures (%u) in the %s stage exceeds the maximum per-stage "
            "limit (%u).%s",
            storageTextureCount, stage, maxStorageTexturesPerShaderStage,
            DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxStorageTexturesPerShaderStage,
                                        storageTextureCount));

        uint32_t uniformBufferCount = bindingCounts.perStage[stage].uniformBufferCount;
        DAWN_INVALID_IF(
            uniformBufferCount > maxUniformBuffersPerShaderStage,
            "The number of uniform buffers (%u) in the %s stage exceeds the maximum per-stage "
            "limit (%u).%s",
            uniformBufferCount, stage, maxUniformBuffersPerShaderStage,
            DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxUniformBuffersPerShaderStage,
                                        uniformBufferCount));

        uint32_t uniformBuffersAndExternalTextureCount =
            uniformBufferCount + (externalTextureCount * kUniformsPerExternalTexture);
        DAWN_INVALID_IF(
            uniformBuffersAndExternalTextureCount > maxUniformBuffersPerShaderStage,
            "The combination of uniform buffers (%u) and external textures (%u × %u) in the %s "
            "stage exceeds the maximum per-stage limit (%u).%s",
            uniformBufferCount, externalTextureCount, kUniformsPerExternalTexture, stage,
            maxUniformBuffersPerShaderStage,
            DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxUniformBuffersPerShaderStage,
                                        uniformBuffersAndExternalTextureCount));

        switch (stage) {
            case SingleShaderStage::Fragment:
                DAWN_INVALID_IF(storageBufferCount > maxStorageBuffersInFragmentStage,
                                "number of storage buffers used in fragment stage (%u) exceeds "
                                "maxStorageBuffersInFragmentStage (%u).%s",
                                storageBufferCount, maxStorageBuffersInFragmentStage,
                                DAWN_INCREASE_LIMIT_MESSAGE(
                                    adapter, maxStorageBuffersInFragmentStage, storageBufferCount));
                DAWN_INVALID_IF(
                    storageTextureCount > maxStorageTexturesInFragmentStage,
                    "number of storage textures used in fragment stage (%u) exceeds "
                    "maxStorageTexturesInFragmentStage (%u).%s",
                    storageTextureCount, maxStorageTexturesInFragmentStage,
                    DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxStorageTexturesInFragmentStage,
                                                storageTextureCount));
                break;
            case SingleShaderStage::Vertex:
                DAWN_INVALID_IF(storageBufferCount > maxStorageBuffersInVertexStage,
                                "number of storage buffers used in vertex stage (%u) exceeds "
                                "maxStorageBuffersInVertexStage (%u).%s",
                                storageBufferCount, maxStorageBuffersInVertexStage,
                                DAWN_INCREASE_LIMIT_MESSAGE(adapter, maxStorageBuffersInVertexStage,
                                                            storageBufferCount));
                DAWN_INVALID_IF(storageTextureCount > maxStorageTexturesInVertexStage,
                                "number of storage textures used in vertex stage (%u) exceeds "
                                "maxStorageTexturesInVertexStage (%u).%s",
                                storageTextureCount, maxStorageTexturesInVertexStage,
                                DAWN_INCREASE_LIMIT_MESSAGE(
                                    adapter, maxStorageTexturesInVertexStage, storageTextureCount));
                break;
            default:
                break;
        }
    }

    return {};
}

BufferBindingInfo::BufferBindingInfo() = default;

BufferBindingInfo::BufferBindingInfo(const BufferBindingLayout& apiLayout)
    : type(apiLayout.type),
      minBindingSize(apiLayout.minBindingSize),
      hasDynamicOffset(apiLayout.hasDynamicOffset) {}

TextureBindingInfo::TextureBindingInfo() {}

TextureBindingInfo::TextureBindingInfo(const TextureBindingLayout& apiLayout)
    : sampleType(apiLayout.sampleType),
      viewDimension(apiLayout.viewDimension),
      multisampled(apiLayout.multisampled) {}

StorageTextureBindingInfo::StorageTextureBindingInfo() = default;

StorageTextureBindingInfo::StorageTextureBindingInfo(const StorageTextureBindingLayout& apiLayout)
    : format(apiLayout.format), viewDimension(apiLayout.viewDimension), access(apiLayout.access) {}

SamplerBindingInfo::SamplerBindingInfo() = default;

SamplerBindingInfo::SamplerBindingInfo(const SamplerBindingLayout& apiLayout)
    : type(apiLayout.type) {}

StaticSamplerBindingInfo::StaticSamplerBindingInfo(const StaticSamplerBindingLayout& apiLayout)
    : sampler(apiLayout.sampler),
      sampledTextureBinding(BindingNumber{apiLayout.sampledTextureBinding}),
      isUsedForSingleTextureBinding(apiLayout.sampledTextureBinding < WGPU_LIMIT_U32_UNDEFINED) {}

InputAttachmentBindingInfo::InputAttachmentBindingInfo() = default;
InputAttachmentBindingInfo::InputAttachmentBindingInfo(wgpu::TextureSampleType sampleType)
    : sampleType(sampleType) {}

}  // namespace dawn::native
