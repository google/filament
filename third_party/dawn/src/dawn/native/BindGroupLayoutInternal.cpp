// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/BindGroupLayoutInternal.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "dawn/common/Enumerator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Error.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/TexelBufferView.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::native {

namespace {

bool TextureFormatSupportStorageAccess(const Format& format, wgpu::StorageTextureAccess access) {
    switch (access) {
        case wgpu::StorageTextureAccess::ReadOnly:
            return format.SupportsReadOnlyStorageUsage();
        case wgpu::StorageTextureAccess::WriteOnly:
            return format.SupportsWriteOnlyStorageUsage();
        case wgpu::StorageTextureAccess::ReadWrite:
            return format.SupportsReadWriteStorageUsage();
        default:
            DAWN_UNREACHABLE();
    }
}

MaybeError ValidateStorageTextureFormat(DeviceBase* device,
                                        wgpu::TextureFormat storageTextureFormat,
                                        wgpu::StorageTextureAccess access) {
    const Format* format = nullptr;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(storageTextureFormat));
    DAWN_ASSERT(format != nullptr);

    DAWN_INVALID_IF(!TextureFormatSupportStorageAccess(*format, access),
                    "Texture format %s does not support storage texture access %s.",
                    storageTextureFormat, access);

    return {};
}

MaybeError ValidateStorageTextureViewDimension(wgpu::TextureViewDimension dimension) {
    switch (dimension) {
        case wgpu::TextureViewDimension::Cube:
        case wgpu::TextureViewDimension::CubeArray:
            return DAWN_VALIDATION_ERROR("%s texture views cannot be used as storage textures.",
                                         dimension);

        case wgpu::TextureViewDimension::e1D:
        case wgpu::TextureViewDimension::e2D:
        case wgpu::TextureViewDimension::e2DArray:
        case wgpu::TextureViewDimension::e3D:
            return {};

        case wgpu::TextureViewDimension::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MaybeError ValidateBindGroupLayoutEntry(DeviceBase* device,
                                        const UnpackedPtr<BindGroupLayoutEntry>& entry,
                                        bool allowInternalBinding) {
    DAWN_TRY(ValidateShaderStage(entry->visibility));

    uint32_t arraySize = std::max(1u, entry->bindingArraySize);

    int bindingMemberCount = 0;

    if (entry->buffer.type != wgpu::BufferBindingType::BindingNotUsed) {
        bindingMemberCount++;
        const BufferBindingLayout& buffer = entry->buffer;

        // The kInternalStorageBufferBinding is used internally and not a value
        // in wgpu::BufferBindingType.
        if (buffer.type == kInternalStorageBufferBinding ||
            buffer.type == kInternalReadOnlyStorageBufferBinding) {
            DAWN_INVALID_IF(!allowInternalBinding, "Internal binding types are disallowed");
        } else {
            DAWN_TRY(ValidateBufferBindingType(buffer.type));
        }

        if (buffer.type == wgpu::BufferBindingType::Storage ||
            buffer.type == kInternalStorageBufferBinding) {
            DAWN_INVALID_IF(
                entry->visibility & wgpu::ShaderStage::Vertex,
                "Read-write storage buffer binding is used with a visibility (%s) that contains %s "
                "(note that read-only storage buffer bindings are allowed).",
                entry->visibility, wgpu::ShaderStage::Vertex);
        }

        // TODO(393558555): Support bindingArraySize > 1 for non-dynamic buffers.
        DAWN_INVALID_IF(arraySize > 1,
                        "bindingArraySize (%u) > 1 for a buffer binding is not implemented yet.",
                        arraySize);
    }

    if (entry->sampler.type != wgpu::SamplerBindingType::BindingNotUsed) {
        bindingMemberCount++;
        DAWN_TRY(ValidateSamplerBindingType(entry->sampler.type));

        // TODO(393558555): Support bindingArraySize > 1 for samplers.
        DAWN_INVALID_IF(arraySize > 1,
                        "bindingArraySize (%u) > 1 for a sampler binding is not implemented yet.",
                        arraySize);
    }

    if (entry->texture.sampleType != wgpu::TextureSampleType::BindingNotUsed) {
        bindingMemberCount++;
        const TextureBindingLayout& texture = entry->texture;
        // The kInternalResolveAttachmentSampleType is used internally and not a value
        // in wgpu::TextureSampleType.
        switch (texture.sampleType) {
            case kInternalResolveAttachmentSampleType:
                if (allowInternalBinding) {
                    break;
                }
                // should return validation error.
                [[fallthrough]];
            default:
                DAWN_TRY(ValidateTextureSampleType(texture.sampleType));
                break;
        }

        // viewDimension defaults to 2D if left undefined, needs validation otherwise.
        wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D;
        if (texture.viewDimension != wgpu::TextureViewDimension::Undefined) {
            switch (texture.viewDimension) {
                case kInternalInputAttachmentDim:
                    if (allowInternalBinding) {
                        break;
                    }
                    // should return validation error.
                    [[fallthrough]];
                default:
                    DAWN_TRY(ValidateTextureViewDimension(texture.viewDimension));
            }
            viewDimension = texture.viewDimension;
        }

        DAWN_INVALID_IF(texture.multisampled && viewDimension != wgpu::TextureViewDimension::e2D,
                        "View dimension (%s) for a multisampled texture bindings was not %s.",
                        viewDimension, wgpu::TextureViewDimension::e2D);

        DAWN_INVALID_IF(
            texture.multisampled && texture.sampleType == wgpu::TextureSampleType::Float,
            "Sample type for multisampled texture binding was %s.", wgpu::TextureSampleType::Float);
    }

    if (entry->storageTexture.access != wgpu::StorageTextureAccess::BindingNotUsed) {
        bindingMemberCount++;
        const StorageTextureBindingLayout& storageTexture = entry->storageTexture;
        DAWN_TRY(ValidateStorageTextureAccess(storageTexture.access));
        DAWN_TRY(
            ValidateStorageTextureFormat(device, storageTexture.format, storageTexture.access));

        // viewDimension defaults to 2D if left undefined, needs validation otherwise.
        if (storageTexture.viewDimension != wgpu::TextureViewDimension::Undefined) {
            DAWN_TRY(ValidateTextureViewDimension(storageTexture.viewDimension));
            DAWN_TRY(ValidateStorageTextureViewDimension(storageTexture.viewDimension));
        }

        switch (storageTexture.access) {
            case wgpu::StorageTextureAccess::ReadOnly:
                break;
            case wgpu::StorageTextureAccess::ReadWrite:
            case wgpu::StorageTextureAccess::WriteOnly:
                DAWN_INVALID_IF(entry->visibility & wgpu::ShaderStage::Vertex,
                                "Storage texture binding with %s is used with a visibility (%s) "
                                "that contains %s.",
                                storageTexture.access, entry->visibility,
                                wgpu::ShaderStage::Vertex);
                break;
            default:
                DAWN_UNREACHABLE();
        }

        // TODO(393558555): Support bindingArraySize > 1 for storage textures.
        DAWN_INVALID_IF(
            arraySize > 1,
            "bindingArraySize (%u) > 1 for a storage texture binding is not implemented yet.",
            arraySize);
    }

    if (auto* staticSamplerBindingLayout = entry.Get<StaticSamplerBindingLayout>()) {
        bindingMemberCount++;

        DAWN_INVALID_IF(!device->HasFeature(Feature::StaticSamplers),
                        "Static samplers used without the %s feature enabled.",
                        wgpu::FeatureName::StaticSamplers);

        DAWN_TRY(device->ValidateObject(staticSamplerBindingLayout->sampler));
        DAWN_INVALID_IF(arraySize > 1,
                        "BindGroupLayoutEntry bindingArraySize (%u) > 1 for a static "
                        "sampler entry.",
                        arraySize);

        if (staticSamplerBindingLayout->sampledTextureBinding == WGPU_LIMIT_U32_UNDEFINED) {
            DAWN_INVALID_IF(staticSamplerBindingLayout->sampler->IsYCbCr(),
                            "YCbCr static sampler requires a sampled texture binding");
        }
    }

    if (auto* texelBufferLayout = entry.Get<TexelBufferBindingLayout>()) {
        bindingMemberCount++;
        DAWN_INVALID_IF(!device->AreTexelBuffersEnabled(), "%s is not enabled.",
                        wgpu::WGSLLanguageFeatureName::TexelBuffers);

        DAWN_TRY(ValidateTexelBufferAccess(texelBufferLayout->access));

        // TODO(393558555): Support bindingArraySize > 1 for texel buffers.
        DAWN_INVALID_IF(
            arraySize > 1,
            "bindingArraySize (%u) > 1 for a storage texture binding is not implemented yet.",
            arraySize);

        DAWN_INVALID_IF(entry->visibility & wgpu::ShaderStage::Vertex &&
                            texelBufferLayout->access != wgpu::TexelBufferAccess::ReadOnly,
                        "Vertex visibility requires read-only texel buffer access.");

        const Format* format;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(texelBufferLayout->format));
        DAWN_INVALID_IF(!IsFormatSupportedForTexelBuffer(format->format),
                        "Texel buffer layout format (%s) is not allowed for texel buffers.",
                        format->format);
    }

    if (entry.Has<ExternalTextureBindingLayout>()) {
        bindingMemberCount++;
        DAWN_INVALID_IF(arraySize > 1,
                        "BindGroupLayoutEntry bindingArraySize (%u) > 1 for an "
                        "external texture entry.",
                        arraySize);
    }

    DAWN_INVALID_IF(bindingMemberCount == 0,
                    "BindGroupLayoutEntry had none of buffer, sampler, texture, "
                    "storageTexture, texelBuffer, or externalTexture set");

    DAWN_INVALID_IF(bindingMemberCount != 1,
                    "BindGroupLayoutEntry had more than one of buffer, sampler, texture, "
                    "storageTexture, texelBuffer, or externalTexture set");

    DAWN_INVALID_IF(
        arraySize > 1 && entry->texture.sampleType == wgpu::TextureSampleType::BindingNotUsed,
        "Entry that is not a sampled texture has an bindingArraySize (%u) > 1.", arraySize);

    return {};
}

MaybeError ValidateStaticSamplersWithTextureBindings(
    DeviceBase* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor,
    const std::map<BindingNumber, uint32_t>& bindingNumberToIndexMap) {
    // Map of texture binding number to static sampler binding number.
    std::map<BindingNumber, BindingNumber> textureToStaticSamplerBindingMap;

    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        UnpackedPtr<BindGroupLayoutEntry> entry = Unpack(&descriptor->entries[i]);
        auto* staticSamplerLayout = entry.Get<StaticSamplerBindingLayout>();
        if (!staticSamplerLayout ||
            staticSamplerLayout->sampledTextureBinding == WGPU_LIMIT_U32_UNDEFINED) {
            continue;
        }

        BindingNumber samplerBinding(entry->binding);
        BindingNumber sampledTextureBinding(staticSamplerLayout->sampledTextureBinding);

        bool inserted =
            textureToStaticSamplerBindingMap.insert({sampledTextureBinding, samplerBinding}).second;
        DAWN_INVALID_IF(!inserted,
                        "For static sampler binding (%u) the sampled texture binding (%u) is "
                        "already bound to a static sampler at binding (%u).",
                        samplerBinding, sampledTextureBinding,
                        textureToStaticSamplerBindingMap[sampledTextureBinding]);

        DAWN_INVALID_IF(!bindingNumberToIndexMap.contains(sampledTextureBinding),
                        "For static sampler binding (%u) the sampled texture binding (%u) is not a "
                        "valid binding number.",
                        samplerBinding, sampledTextureBinding);

        auto& textureEntry = descriptor->entries[bindingNumberToIndexMap.at(sampledTextureBinding)];
        DAWN_INVALID_IF(textureEntry.texture.sampleType == wgpu::TextureSampleType::BindingNotUsed,
                        "For static sampler binding (%u) the sampled texture binding (%u) is not a "
                        "texture binding.",
                        samplerBinding, sampledTextureBinding);
    }

    return {};
}

}  // anonymous namespace

ResultOrError<UnpackedPtr<BindGroupLayoutDescriptor>> ValidateBindGroupLayoutDescriptor(
    DeviceBase* device,
    const BindGroupLayoutDescriptor* descriptorChain,
    bool allowInternalBinding) {
    UnpackedPtr<BindGroupLayoutDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(descriptorChain));

    // A running total of the number of bindings used by the layout.
    BindingCounts bindingCounts = {};

    // Map of binding number to entry index.
    std::map<BindingNumber, uint32_t> bindingMap;

    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        UnpackedPtr<BindGroupLayoutEntry> entry;
        DAWN_TRY_ASSIGN(entry, ValidateAndUnpack(&descriptor->entries[i]));

        BindingNumber bindingNumber = BindingNumber(entry->binding);
        DAWN_INVALID_IF(
            bindingNumber >= kMaxBindingsPerBindGroupTyped,
            "On entries[%u]: binding number (%u) exceeds the maxBindingsPerBindGroup limit (%u).",
            i, bindingNumber, kMaxBindingsPerBindGroup);

        BindingNumber arraySize{1};
        if (entry->bindingArraySize > 1) {
            arraySize = BindingNumber(entry->bindingArraySize);

            DAWN_INVALID_IF(device->IsToggleEnabled(Toggle::DisableBindGroupLayoutEntryArraySize),
                            "On entries[%u]: use of bindingArraySize > 1 is disabled.", i);
            DAWN_INVALID_IF(!device->IsToggleEnabled(Toggle::AllowUnsafeAPIs),
                            "On entries[%u]: use of bindingArraySize > 1 is currently unsafe.", i);

            DAWN_INVALID_IF(arraySize > kMaxBindingsPerBindGroupTyped - bindingNumber,
                            "On entries[%u]: binding (%u) + arraySize (%u) is %u which is larger "
                            "than maxBindingsPerBindGroup (%u).",
                            i, arraySize, bindingNumber,
                            uint32_t(arraySize) + uint32_t(bindingNumber),
                            kMaxBindingsPerBindGroupTyped);
        }

        // Check that the same binding is not set twice. bindingNumber + arraySize cannot overflow
        // as they are both smaller than kMaxBindingsPerBindGroupTyped.
        static_assert(kMaxBindingsPerBindGroup < std::numeric_limits<uint32_t>::max() / 2);
        for (BindingNumber usedBinding : Range(bindingNumber, bindingNumber + arraySize)) {
            DAWN_INVALID_IF(bindingMap.contains(usedBinding),
                            "On entries[%u]: binding index (%u) was specified by a previous entry.",
                            i, entry->binding);
            bindingMap.insert({usedBinding, i});
        }

        DAWN_TRY_CONTEXT(ValidateBindGroupLayoutEntry(device, entry, allowInternalBinding),
                         "validating entries[%u]", i);

        IncrementBindingCounts(&bindingCounts, entry);
    }

    // Perform a second validation pass for static samplers. This is done after initial validation
    // as static samplers can have associated texture entries that need to be validated first.
    DAWN_TRY(ValidateStaticSamplersWithTextureBindings(device, descriptor, bindingMap));

    DAWN_TRY_CONTEXT(
        ValidateBindingCounts(device->GetLimits(), bindingCounts, device->GetAdapter()),
        "validating binding counts");

    return descriptor;
}

namespace {

BindingInfo CreateSampledTextureBindingForExternalTexture(BindingNumber binding,
                                                          wgpu::ShaderStage visibility) {
    return {
        .binding = binding,
        .visibility = visibility,
        .bindingLayout = TextureBindingInfo{{
            .sampleType = wgpu::TextureSampleType::Float,
            .viewDimension = wgpu::TextureViewDimension::e2D,
            .multisampled = false,
        }},
    };
}

BindingInfo CreateUniformBindingForExternalTexture(BindingNumber binding,
                                                   wgpu::ShaderStage visibility) {
    return {
        .binding = binding,
        .visibility = visibility,
        .bindingLayout = BufferBindingInfo{{
            .type = wgpu::BufferBindingType::Uniform,
            .minBindingSize = 0,
            .hasDynamicOffset = false,
        }},
    };
}

BindingInfo CreateStaticSamplerBindingForExternalTexture(const DeviceBase* device,
                                                         BindingNumber binding,
                                                         wgpu::ShaderStage visibility) {
    return {
        .binding = binding,
        .visibility = visibility,
        .bindingLayout =
            StaticSamplerBindingInfo{
                .sampler = device->GetPlaceholderSampler(),
                .use = StaticSamplerUse::InternalForExternalTexture,
            },
    };
}

BindingInfo ConvertToBindingInfo(const UnpackedPtr<BindGroupLayoutEntry>& binding) {
    BindingInfo bindingInfo;
    bindingInfo.binding = BindingNumber(binding->binding);
    bindingInfo.visibility = binding->visibility;
    bindingInfo.arraySize = BindingIndex(std::max(1u, binding->bindingArraySize));

    if (binding->buffer.type != wgpu::BufferBindingType::BindingNotUsed) {
        bindingInfo.bindingLayout = BufferBindingInfo::From(binding->buffer);
    } else if (binding->sampler.type != wgpu::SamplerBindingType::BindingNotUsed) {
        bindingInfo.bindingLayout = SamplerBindingInfo::From(binding->sampler);
    } else if (binding->texture.sampleType != wgpu::TextureSampleType::BindingNotUsed) {
        auto textureBindingInfo = TextureBindingInfo::From(binding->texture);
        if (binding->texture.viewDimension == kInternalInputAttachmentDim) {
            bindingInfo.bindingLayout = InputAttachmentBindingInfo{{textureBindingInfo.sampleType}};
        } else {
            bindingInfo.bindingLayout = textureBindingInfo;
        }
    } else if (binding->storageTexture.access != wgpu::StorageTextureAccess::BindingNotUsed) {
        bindingInfo.bindingLayout = StorageTextureBindingInfo::From(binding->storageTexture);
    } else if (auto* texelBufferLayout = binding.Get<TexelBufferBindingLayout>()) {
        bindingInfo.bindingLayout = TexelBufferBindingInfo::From(*texelBufferLayout);
    } else if (auto* staticSamplerBindingLayout = binding.Get<StaticSamplerBindingLayout>()) {
        // The sampledTextureIndex will be filled later, after reordering of bindings.
        bindingInfo.bindingLayout = StaticSamplerBindingInfo::From(*staticSamplerBindingLayout);
    } else if (binding.Has<ExternalTextureBindingLayout>()) {
        // The BindingIndex members are filled later once we know the order of bindings.
        bindingInfo.bindingLayout = ExternalTextureBindingInfo{};
    } else {
        DAWN_UNREACHABLE();
    }

    return bindingInfo;
}

bool SortBindingsCompare(const BindingInfo& a, const BindingInfo& b) {
    if (&a == &b) {
        return false;
    }

    // Buffers with dynamic offsets come first and then the rest of the buffers. Other bindings are
    // only grouped by types. This is to make it easier and faster to handle them.
    auto TypeOrder = [](const BindingInfo& info) {
        return MatchVariant(
            info.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                return layout.hasDynamicOffset ? BindingTypeOrder_DynamicBuffer
                                               : BindingTypeOrder_RegularBuffer;
            },
            [&](const TextureBindingInfo&) { return BindingTypeOrder_SampledTexture; },
            [&](const StorageTextureBindingInfo&) { return BindingTypeOrder_StorageTexture; },
            [&](const SamplerBindingInfo&) { return BindingTypeOrder_RegularSampler; },
            [&](const StaticSamplerBindingInfo&) { return BindingTypeOrder_StaticSampler; },
            [&](const TexelBufferBindingInfo&) { return BindingTypeOrder_TexelBuffer; },
            [&](const InputAttachmentBindingInfo&) { return BindingTypeOrder_InputAttachment; },
            [&](const ExternalTextureBindingInfo&) { return BindingTypeOrder_ExternalTexture; });
    };

    auto aOrder = TypeOrder(a);
    auto bOrder = TypeOrder(b);
    if (aOrder != bOrder) {
        return aOrder < bOrder;
    }

    // Afterwards sort the bindings by binding number. This is necessary because dynamic buffers
    // are applied in order of increasing binding number in SetBindGroup. It also ensures that
    // bindings for binding arrays stay contiguous as that's required by backends.
    return a.binding < b.binding;
}

// This function handles the conversion of the API format for each binding info to Dawn's internal
// representation of them. This is also where the ExternalTextures are replaced and expanded in the
// various bindings that are used internally in Dawn. Arrays are also expanded to individual
// bindings here.
struct ExpandedBindingInfo {
    BindGroupLayoutInternalBase::BindingMap apiBindingMap;
    ityp::vector<BindingIndex, BindingInfo> entries;
};
ExpandedBindingInfo ConvertAndExpandBGLEntries(
    const DeviceBase* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor) {
    // When new BGL entries are created, we use binding numbers decreasing from the max uint32_t
    // to ensure there are no collisions and that validation will prevent using these BindingNumbers
    // when creating a bindgroup (so there is no risk of applications injecting their own buffer for
    // the metadata for example).
    BindingNumber nextOpenBindingNumberForNewEntry = std::numeric_limits<BindingNumber>::max();

    ityp::vector<BindingIndex, BindingInfo> entries;
    absl::flat_hash_set<BindingNumber> internalEntries;

    // Keep track of the BindingNumbers for additional bindings for external textures, it is used
    // below to link the ExternalTextureBindingInfo to the BindingIndex of the additional bindings.
    struct ExternalTextureExpansion {
        BindingNumber plane0;
        BindingNumber plane1;
        BindingNumber metadata;
        std::optional<BindingNumber> staticSampler;
    };
    absl::flat_hash_map<BindingNumber, ExternalTextureExpansion> externalTextureExpansions;

    // Likewise keep track of the "single texture binding" for static samplers so that we can link
    // StaticSamplerBindingInfo::sampledTextureIndex to the BindingIndex post reordering.
    absl::flat_hash_map<BindingNumber, BindingNumber> staticSamplerToSingleTextureBinding;

    for (uint32_t i = 0; i < descriptor->entryCount; i++) {
        UnpackedPtr<BindGroupLayoutEntry> entry = Unpack(&descriptor->entries[i]);

        // External textures are expanded with additional bindings:
        //  - Two sampled texture bindings and one uniform buffer
        //  - (optionally) a static sampler that may be used to sample plane0.
        // The external texture is still added to the entries to be used in validation and to know
        // where the additional bindings are located.
        if (entry.Has<ExternalTextureBindingLayout>()) {
            DAWN_ASSERT(entry->bindingArraySize <= 1);

            BindingInfo plane0Entry = CreateSampledTextureBindingForExternalTexture(
                nextOpenBindingNumberForNewEntry--, entry->visibility);
            entries.push_back(plane0Entry);
            internalEntries.insert(plane0Entry.binding);

            BindingInfo plane1Entry = CreateSampledTextureBindingForExternalTexture(
                nextOpenBindingNumberForNewEntry--, entry->visibility);
            entries.push_back(plane1Entry);
            internalEntries.insert(plane1Entry.binding);

            BindingInfo metadataEntry = CreateUniformBindingForExternalTexture(
                nextOpenBindingNumberForNewEntry--, entry->visibility);
            entries.push_back(metadataEntry);
            internalEntries.insert(metadataEntry.binding);

            // External textures may alternatively use a static sampler to sample plane0, add it and
            // ensure its sampledTextureIndex is remapped.
            std::optional<BindingNumber> staticSamplerBinding = {};
            if (device->NeedsStaticSamplerForExternalTexture()) {
                BindingInfo staticSamplerEntry = CreateStaticSamplerBindingForExternalTexture(
                    device, nextOpenBindingNumberForNewEntry--, entry->visibility);
                entries.push_back(staticSamplerEntry);
                internalEntries.insert(staticSamplerEntry.binding);
                staticSamplerBinding = staticSamplerEntry.binding;

                staticSamplerToSingleTextureBinding.insert(
                    {staticSamplerEntry.binding, plane0Entry.binding});
            }

            externalTextureExpansions.insert({BindingNumber(entry->binding),
                                              {
                                                  .plane0 = BindingNumber(plane0Entry.binding),
                                                  .plane1 = BindingNumber(plane1Entry.binding),
                                                  .metadata = BindingNumber(metadataEntry.binding),
                                                  .staticSampler = staticSamplerBinding,
                                              }});
        }

        if (auto* staticSampler = entry.Get<StaticSamplerBindingLayout>()) {
            if (staticSampler->sampledTextureBinding < WGPU_LIMIT_U32_UNDEFINED) {
                staticSamplerToSingleTextureBinding.insert(
                    {BindingNumber(entry->binding),
                     BindingNumber(staticSampler->sampledTextureBinding)});
            }
        }

        // Add one BindingInfo per element of the array with increasing indexInArray for backends to
        // know which element it is when they need it, but also with increasing BindingNumber as the
        // array takes consecutive binding numbers on the API side.
        BindingInfo info = ConvertToBindingInfo(entry);
        for (BindingIndex indexInArray : Range(info.arraySize)) {
            info.indexInArray = indexInArray;
            entries.push_back(info);
            info.binding++;
        }
    }

    // Reorder bindings internally and compute the complete BindingNumber->BindingIndex map.
    std::sort(entries.begin(), entries.end(), SortBindingsCompare);

    absl::flat_hash_map<BindingNumber, BindingIndex> fullBindingMap;
    for (const auto [i, binding] : Enumerate(entries)) {
        const auto& [_, inserted] = fullBindingMap.emplace(binding.binding, i);
        DAWN_ASSERT(inserted);
    }

    // Store the location of expanded entries in ExternalTexture layouts.
    for (const auto& [etBindingNumber, expansion] : externalTextureExpansions) {
        std::optional<BindingIndex> staticSamplerBinding = {};
        if (expansion.staticSampler) {
            staticSamplerBinding = {fullBindingMap[expansion.staticSampler.value()]};
        }

        entries[fullBindingMap[etBindingNumber]].bindingLayout = ExternalTextureBindingInfo{{
            .metadata = fullBindingMap[expansion.metadata],
            .plane0 = fullBindingMap[expansion.plane0],
            .plane1 = fullBindingMap[expansion.plane1],
            .staticSampler = staticSamplerBinding,
        }};
    }

    // Store the location of the single texture binding in the StaticSamplerBindingInfo.
    for (auto [samplerNumber, textureNumber] : staticSamplerToSingleTextureBinding) {
        auto& samplerBindingInfo = std::get<StaticSamplerBindingInfo>(
            entries[fullBindingMap[samplerNumber]].bindingLayout);
        samplerBindingInfo.sampledTextureIndex = fullBindingMap[textureNumber];
    }

    // Now build the result.
    ExpandedBindingInfo result;

    // Build the user-facing binding map.
    for (const auto [i, binding] : Enumerate(entries)) {
        if (internalEntries.contains(binding.binding)) {
            continue;
        }

        APIBindingIndex index = APIBindingIndex(uint32_t(i));
        const auto& [_, inserted] = result.apiBindingMap.emplace(binding.binding, index);
        DAWN_ASSERT(inserted);
    }

    result.entries = std::move(entries);
    return result;
}

// This is a utility function to help DAWN_ASSERT that the BGL-binding comparator places buffers
// first.
bool CheckBufferBindingsFirst(ityp::span<BindingIndex, const BindingInfo> bindings) {
    BindingIndex lastBufferIndex{0};
    BindingIndex firstNonBufferIndex = std::numeric_limits<BindingIndex>::max();
    for (auto [i, binding] : Enumerate(bindings)) {
        if (std::holds_alternative<BufferBindingInfo>(binding.bindingLayout)) {
            lastBufferIndex = std::max(i, lastBufferIndex);
        } else {
            firstNonBufferIndex = std::min(i, firstNonBufferIndex);
        }
    }

    // If there are no buffers, then |lastBufferIndex| is initialized to 0 and
    // |firstNonBufferIndex| gets set to 0.
    return firstNonBufferIndex >= lastBufferIndex;
}

}  // namespace

// BindGroupLayoutInternalBase

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(
    DeviceBase* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor,
    ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label) {
    ExpandedBindingInfo unpackedBindings = ConvertAndExpandBGLEntries(device, descriptor);
    mBindingInfo = std::move(unpackedBindings.entries);
    mBindingMap = std::move(unpackedBindings.apiBindingMap);

    DAWN_ASSERT(CheckBufferBindingsFirst({mBindingInfo.data(), GetBindingCount()}));

    // Compute various counts of expanded bindings and other metadata.
    std::array<BindingIndex, BindingTypeOrder_Count + 1> counts{};
    for (const auto& binding : mBindingInfo) {
        MatchVariant(
            binding.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                if (layout.minBindingSize == 0) {
                    mUnverifiedBufferCount++;
                }
                if (layout.hasDynamicOffset) {
                    counts[BindingTypeOrder_DynamicBuffer]++;
                    switch (layout.type) {
                        case wgpu::BufferBindingType::Storage:
                        case kInternalStorageBufferBinding:
                        case kInternalReadOnlyStorageBufferBinding:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                            mDynamicStorageBufferCount++;
                            break;

                        case wgpu::BufferBindingType::Uniform:
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            break;
                    }
                } else {
                    counts[BindingTypeOrder_RegularBuffer]++;
                }
            },
            [&](const TextureBindingInfo&) { counts[BindingTypeOrder_SampledTexture]++; },
            [&](const StorageTextureBindingInfo&) { counts[BindingTypeOrder_StorageTexture]++; },
            [&](const SamplerBindingInfo&) { counts[BindingTypeOrder_RegularSampler]++; },
            [&](const StaticSamplerBindingInfo& layout) {
                counts[BindingTypeOrder_StaticSampler]++;
                if (layout.use == StaticSamplerUse::SingleTextureYCbCr) {
                    mNeedsCrossBindingValidation = true;
                }
            },
            [&](const TexelBufferBindingInfo&) { counts[BindingTypeOrder_TexelBuffer]++; },
            [&](const InputAttachmentBindingInfo&) { counts[BindingTypeOrder_InputAttachment]++; },
            [&](const ExternalTextureBindingInfo&) { counts[BindingTypeOrder_ExternalTexture]++; });
    }

    // Populate map of binding to ordered index to access BindGroup::mBoundExternalTextures.
    size_t externalTextureIndex = 0;
    for (const auto& [bindingNumber, apiBindingIndex] : mBindingMap) {
        const BindingInfo& info = GetAPIBindingInfo(apiBindingIndex);
        if (std::holds_alternative<ExternalTextureBindingInfo>(info.bindingLayout)) {
            mBoundExternalTextureMap.emplace(apiBindingIndex, externalTextureIndex++);
        }
    }

    // Do a prefix sum to store the start offset of each binding type.
    BindingIndex sum{0};
    for (auto [type, count] : Enumerate(counts)) {
        mBindingTypeStart[type] = sum;
        sum += count;
    }

    // Recompute the number of bindings of each type from the descriptor since that is used for
    // validation of the pipeline layout.
    for (uint32_t i = 0; i < descriptor->entryCount; i++) {
        UnpackedPtr<BindGroupLayoutEntry> entry = Unpack(&descriptor->entries[i]);
        IncrementBindingCounts(&mValidationBindingCounts, entry);
    }
}

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(
    DeviceBase* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor)
    : BindGroupLayoutInternalBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(DeviceBase* device,
                                                         ObjectBase::ErrorTag tag,
                                                         StringView label)
    : ApiObjectBase(device, tag, label) {}

BindGroupLayoutInternalBase::~BindGroupLayoutInternalBase() = default;

void BindGroupLayoutInternalBase::DestroyImpl(DestroyReason reason) {
    Uncache();
}

ObjectType BindGroupLayoutInternalBase::GetType() const {
    return ObjectType::BindGroupLayoutInternal;
}

const BindingInfo& BindGroupLayoutInternalBase::GetBindingInfo(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    // Assert that this is an internal binding.
    DAWN_ASSERT(bindingIndex < GetBindingCount());
    return mBindingInfo[bindingIndex];
}

const BindingInfo& BindGroupLayoutInternalBase::GetAPIBindingInfo(
    APIBindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    BindingIndex index = BindingIndex(uint32_t(bindingIndex));
    DAWN_ASSERT(index < mBindingInfo.size());
    // Assert this is a user-facing binding and not an private internal binding.
    DAWN_ASSERT(mBindingMap.contains(mBindingInfo[index].binding));
    return mBindingInfo[index];
}

const BindGroupLayoutInternalBase::BindingMap& BindGroupLayoutInternalBase::GetBindingMap() const {
    DAWN_ASSERT(!IsError());
    return mBindingMap;
}

APIBindingIndex BindGroupLayoutInternalBase::GetAPIBindingIndex(BindingNumber bindingNumber) const {
    DAWN_ASSERT(!IsError());
    const auto& it = mBindingMap.find(bindingNumber);
    DAWN_ASSERT(it != mBindingMap.end());
    return it->second;
}

const BindGroupLayoutInternalBase::BoundExternalTextureMap&
BindGroupLayoutInternalBase::GetBoundExternalTextureMap() const {
    DAWN_ASSERT(!IsError());
    return mBoundExternalTextureMap;
}

BindingIndex BindGroupLayoutInternalBase::AsBindingIndex(APIBindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    // Assert this is a user-facing binding and not a private internal binding, and that it
    // represents an internal bindings.
    BindingIndex index = BindingIndex(uint32_t(bindingIndex));
    DAWN_ASSERT(index < GetBindingCount());
    DAWN_ASSERT(mBindingMap.contains(mBindingInfo[index].binding));
    return index;
}

void BindGroupLayoutInternalBase::ReduceMemoryUsage() {}

size_t BindGroupLayoutInternalBase::ComputeContentHash() {
    ObjectContentHasher recorder;

    // std::map is sorted by key, so two BGLs constructed in different orders
    // will still record the same.
    for (const auto [id, index] : mBindingMap) {
        recorder.Record(id, index);
    }

    for (const auto& info : mBindingInfo) {
        recorder.Record(info.visibility);
        recorder.Record(info.arraySize);
        recorder.Record(info.indexInArray);

        MatchVariant(
            info.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                recorder.Record(BindingInfoType::Buffer, layout.hasDynamicOffset, layout.type,
                                layout.minBindingSize);
            },
            [&](const SamplerBindingInfo& layout) {
                recorder.Record(BindingInfoType::Sampler, layout.type);
            },
            [&](const TextureBindingInfo& layout) {
                recorder.Record(BindingInfoType::Texture, layout.sampleType, layout.viewDimension,
                                layout.multisampled);
            },
            [&](const StorageTextureBindingInfo& layout) {
                recorder.Record(BindingInfoType::StorageTexture, layout.access, layout.format,
                                layout.viewDimension);
            },
            [&](const TexelBufferBindingInfo& layout) {
                recorder.Record(BindingInfoType::TexelBuffer, layout.format, layout.access);
            },
            [&](const StaticSamplerBindingInfo& layout) {
                recorder.Record(BindingInfoType::StaticSampler, layout.sampler->GetContentHash());
            },
            [&](const InputAttachmentBindingInfo& layout) {
                recorder.Record(BindingInfoType::InputAttachment, layout.sampleType);
            },
            [&](const ExternalTextureBindingInfo& layout) {
                recorder.Record(BindingInfoType::ExternalTexture, layout.metadata, layout.plane0,
                                layout.plane1);
            });
    }

    return recorder.GetContentHash();
}

bool BindGroupLayoutInternalBase::EqualityFunc::operator()(
    const BindGroupLayoutInternalBase* a,
    const BindGroupLayoutInternalBase* b) const {
    if (a->GetBindingCount() != b->GetBindingCount()) {
        return false;
    }
    for (BindingIndex i{0}; i < a->GetBindingCount(); ++i) {
        if (a->mBindingInfo[i] != b->mBindingInfo[i]) {
            return false;
        }
    }
    if (a->mBindingMap != b->mBindingMap) {
        return false;
    }
    return true;
}

bool BindGroupLayoutInternalBase::IsEmpty() const {
    DAWN_ASSERT(!IsError());
    return mBindingInfo.empty();
}

BindingIndex BindGroupLayoutInternalBase::GetBindingCount() const {
    DAWN_ASSERT(!IsError());
    return GetBindingTypeStart(BindingTypeOrder_ExternalTexture);
}

BindingIndex BindGroupLayoutInternalBase::GetDynamicBufferCount() const {
    DAWN_ASSERT(!IsError());
    return GetBindingTypeEnd(BindingTypeOrder_DynamicBuffer) -
           GetBindingTypeStart(BindingTypeOrder_DynamicBuffer);
}

uint32_t BindGroupLayoutInternalBase::GetDynamicStorageBufferCount() const {
    DAWN_ASSERT(!IsError());
    return mDynamicStorageBufferCount;
}

uint32_t BindGroupLayoutInternalBase::GetUnverifiedBufferCount() const {
    DAWN_ASSERT(!IsError());
    return mUnverifiedBufferCount;
}

uint32_t BindGroupLayoutInternalBase::GetAPIStaticSamplerCount() const {
    DAWN_ASSERT(!IsError());
    return mValidationBindingCounts.staticSamplerCount;
}

uint32_t BindGroupLayoutInternalBase::GetStaticSamplerCount() const {
    DAWN_ASSERT(!IsError());
    return uint32_t(GetBindingTypeEnd(BindingTypeOrder_StaticSampler) -
                    GetBindingTypeStart(BindingTypeOrder_StaticSampler));
}

uint32_t BindGroupLayoutInternalBase::GetExternalTextureCount() const {
    DAWN_ASSERT(!IsError());
    return uint32_t(GetBindingTypeEnd(BindingTypeOrder_ExternalTexture) -
                    GetBindingTypeStart(BindingTypeOrder_ExternalTexture));
}

const BindingCounts& BindGroupLayoutInternalBase::GetValidationBindingCounts() const {
    DAWN_ASSERT(!IsError());
    return mValidationBindingCounts;
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetDynamicBufferIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_DynamicBuffer),
                 GetBindingTypeEnd(BindingTypeOrder_DynamicBuffer));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetBufferIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_DynamicBuffer),
                 GetBindingTypeEnd(BindingTypeOrder_RegularBuffer));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetStorageTextureIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_StorageTexture),
                 GetBindingTypeEnd(BindingTypeOrder_StorageTexture));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetTexelBufferIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_TexelBuffer),
                 GetBindingTypeEnd(BindingTypeOrder_TexelBuffer));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetSampledTextureIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_SampledTexture),
                 GetBindingTypeEnd(BindingTypeOrder_SampledTexture));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetTextureIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_SampledTexture),
                 GetBindingTypeEnd(BindingTypeOrder_InputAttachment));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetSamplerIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_StaticSampler),
                 GetBindingTypeEnd(BindingTypeOrder_RegularSampler));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetStaticSamplerIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_StaticSampler),
                 GetBindingTypeEnd(BindingTypeOrder_StaticSampler));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetNonStaticSamplerIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_RegularSampler),
                 GetBindingTypeEnd(BindingTypeOrder_RegularSampler));
}

BeginEndRange<BindingIndex> BindGroupLayoutInternalBase::GetInputAttachmentIndices() const {
    return Range(GetBindingTypeStart(BindingTypeOrder_InputAttachment),
                 GetBindingTypeEnd(BindingTypeOrder_InputAttachment));
}

BeginEndRange<APIBindingIndex> BindGroupLayoutInternalBase::GetExternalTextureIndices() const {
    // Cast the result of GetBindingType* as mBindingTypeStart works for ExternalTextures as well
    // but they are the only binding type that should be indexed with APIBindingIndex.
    return Range(APIBindingIndex{uint32_t{GetBindingTypeStart(BindingTypeOrder_ExternalTexture)}},
                 APIBindingIndex{uint32_t{GetBindingTypeEnd(BindingTypeOrder_ExternalTexture)}});
}

bool BindGroupLayoutInternalBase::NeedsCrossBindingValidation() const {
    DAWN_ASSERT(!IsError());
    return mNeedsCrossBindingValidation;
}

uint32_t BindGroupLayoutInternalBase::GetBindingCountForBindGroupCreation() const {
    DAWN_ASSERT(!IsError());
    return mValidationBindingCounts.totalCount - mValidationBindingCounts.staticSamplerCount;
}

size_t BindGroupLayoutInternalBase::GetBindingDataSize() const {
    DAWN_ASSERT(!IsError());
    // | ------ buffer-specific ----------| ------------ object pointers -------------|
    // | --- offsets + sizes -------------| --------------- Ref<ObjectBase> ----------|
    // Followed by:
    // |---------buffer size array--------|
    // |-uint64_t[mUnverifiedBufferCount]-|
    const size_t bufferCount = size_t(GetBindingTypeEnd(BindingTypeOrder_RegularBuffer));
    const size_t bindingCount = size_t(mBindingInfo.size());

    size_t objectPointerStart = bufferCount * sizeof(BufferBindingData);
    DAWN_ASSERT(IsAligned(objectPointerStart, alignof(Ref<ObjectBase>)));
    size_t bufferSizeArrayStart =
        Align(objectPointerStart + bindingCount * sizeof(Ref<ObjectBase>), sizeof(uint64_t));
    DAWN_ASSERT(IsAligned(bufferSizeArrayStart, alignof(uint64_t)));
    return bufferSizeArrayStart + mUnverifiedBufferCount * sizeof(uint64_t);
}

BindGroupLayoutInternalBase::BindingDataPointers
BindGroupLayoutInternalBase::ComputeBindingDataPointers(void* dataStart) const {
    const size_t bufferCount = size_t(GetBindingTypeEnd(BindingTypeOrder_RegularBuffer));
    const size_t bindingCount = size_t(mBindingInfo.size());

    BufferBindingData* bufferData = reinterpret_cast<BufferBindingData*>(dataStart);
    auto bindings = reinterpret_cast<Ref<ObjectBase>*>(bufferData + bufferCount);
    uint64_t* unverifiedBufferSizes =
        AlignPtr(reinterpret_cast<uint64_t*>(bindings + bindingCount), sizeof(uint64_t));

    DAWN_ASSERT(IsPtrAligned(bufferData, alignof(BufferBindingData)));
    DAWN_ASSERT(IsPtrAligned(bindings, alignof(Ref<ObjectBase>)));
    DAWN_ASSERT(IsPtrAligned(unverifiedBufferSizes, alignof(uint64_t)));

    return {{bufferData, GetBindingTypeEnd(BindingTypeOrder_RegularBuffer)},
            {bindings, GetBindingCount()},
            {unverifiedBufferSizes, mUnverifiedBufferCount}};
}

bool BindGroupLayoutInternalBase::IsStorageBufferBinding(BindingIndex bindingIndex) const {
    switch (std::get<BufferBindingInfo>(GetBindingInfo(bindingIndex).bindingLayout).type) {
        case wgpu::BufferBindingType::Uniform:
            return false;
        case kInternalStorageBufferBinding:
        case kInternalReadOnlyStorageBufferBinding:
        case wgpu::BufferBindingType::Storage:
        case wgpu::BufferBindingType::ReadOnlyStorage:
            return true;
        case wgpu::BufferBindingType::BindingNotUsed:
        case wgpu::BufferBindingType::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

bool BindGroupLayoutInternalBase::IsExternalTextureBinding(APIBindingIndex bindingIndex) const {
    return std::holds_alternative<ExternalTextureBindingInfo>(
        GetAPIBindingInfo(bindingIndex).bindingLayout);
}

std::string BindGroupLayoutInternalBase::EntriesToString() const {
    std::string entries = "[";
    std::string sep = "";
    const BindGroupLayoutInternalBase::BindingMap& bindingMap = GetBindingMap();
    for (const auto [bindingNumber, bindingIndex] : bindingMap) {
        const BindingInfo& bindingInfo = GetAPIBindingInfo(bindingIndex);
        entries += absl::StrFormat("%s%s", sep, bindingInfo);
        sep = ", ";
    }
    entries += "]";
    return entries;
}

BindingIndex BindGroupLayoutInternalBase::GetBindingTypeStart(BindingTypeOrder type) const {
    return mBindingTypeStart[type];
}

BindingIndex BindGroupLayoutInternalBase::GetBindingTypeEnd(BindingTypeOrder type) const {
    return mBindingTypeStart[BindingTypeOrder(static_cast<uint32_t>(type) + 1)];
}

}  // namespace dawn::native
