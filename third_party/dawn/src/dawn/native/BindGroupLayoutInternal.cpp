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
#include <vector>

#include "dawn/common/BitSetIterator.h"
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
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

namespace {
MaybeError ValidateStorageTextureFormat(DeviceBase* device,
                                        wgpu::TextureFormat storageTextureFormat,
                                        wgpu::StorageTextureAccess access) {
    const Format* format = nullptr;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(storageTextureFormat));
    DAWN_ASSERT(format != nullptr);

    DAWN_INVALID_IF(!format->supportsStorageUsage,
                    "Texture format (%s) does not support storage textures.", storageTextureFormat);

    if (access == wgpu::StorageTextureAccess::ReadWrite) {
        DAWN_INVALID_IF(!format->supportsReadWriteStorageUsage,
                        "Texture format %s does not support storage texture access %s",
                        storageTextureFormat, wgpu::StorageTextureAccess::ReadWrite);
    }

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
    }

    if (entry->sampler.type != wgpu::SamplerBindingType::BindingNotUsed) {
        bindingMemberCount++;
        DAWN_TRY(ValidateSamplerBindingType(entry->sampler.type));
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
    }

    if (auto* staticSamplerBindingLayout = entry.Get<StaticSamplerBindingLayout>()) {
        bindingMemberCount++;

        DAWN_INVALID_IF(!device->HasFeature(Feature::StaticSamplers),
                        "Static samplers used without the %s feature enabled.",
                        wgpu::FeatureName::StaticSamplers);

        DAWN_TRY(device->ValidateObject(staticSamplerBindingLayout->sampler));

        if (staticSamplerBindingLayout->sampledTextureBinding == WGPU_LIMIT_U32_UNDEFINED) {
            DAWN_INVALID_IF(staticSamplerBindingLayout->sampler->IsYCbCr(),
                            "YCbCr static sampler requires a sampled texture binding");
        }
    }

    if (entry.Get<ExternalTextureBindingLayout>()) {
        bindingMemberCount++;
    }

    DAWN_INVALID_IF(bindingMemberCount == 0,
                    "BindGroupLayoutEntry had none of buffer, sampler, texture, "
                    "storageTexture, or externalTexture set");

    DAWN_INVALID_IF(bindingMemberCount != 1,
                    "BindGroupLayoutEntry had more than one of buffer, sampler, texture, "
                    "storageTexture, or externalTexture set");

    return {};
}

MaybeError ValidateStaticSamplersWithTextureBindings(
    DeviceBase* device,
    const BindGroupLayoutDescriptor* descriptor,
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

        DAWN_INVALID_IF(!bindingNumberToIndexMap.count(sampledTextureBinding),
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

BindGroupLayoutEntry CreateSampledTextureBindingForExternalTexture(uint32_t binding,
                                                                   wgpu::ShaderStage visibility) {
    BindGroupLayoutEntry entry;
    entry.binding = binding;
    entry.visibility = visibility;
    entry.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    entry.texture.multisampled = false;
    entry.texture.sampleType = wgpu::TextureSampleType::Float;
    return entry;
}

BindGroupLayoutEntry CreateUniformBindingForExternalTexture(uint32_t binding,
                                                            wgpu::ShaderStage visibility) {
    BindGroupLayoutEntry entry;
    entry.binding = binding;
    entry.visibility = visibility;
    entry.buffer.hasDynamicOffset = false;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    return entry;
}

// Helper struct to encapsulate additional backing BindGroupLayoutEntries created when expanding for
// UnpackedPtr types. Users shouldn't need to use the list of additional entries directly, instead
// they should use the list of unpacked pointers instead.
struct UnpackedExpandedBglEntries {
    // Backing memory for additional entries. Note that we use a list for pointer stability of the
    // create elements that are used for the UnpackedPtrs.
    std::list<BindGroupLayoutEntry> additionalEntries;
    std::vector<UnpackedPtr<BindGroupLayoutEntry>> unpackedEntries;
};

UnpackedExpandedBglEntries ExtractAndExpandBglEntries(
    const BindGroupLayoutDescriptor* descriptor,
    BindingCounts* bindingCounts,
    ExternalTextureBindingExpansionMap* externalTextureBindingExpansions,
    bool* needsCrossBindingValidation) {
    UnpackedExpandedBglEntries result;
    std::list<BindGroupLayoutEntry>& additionalEntries = result.additionalEntries;
    std::vector<UnpackedPtr<BindGroupLayoutEntry>>& expandedOutput = result.unpackedEntries;

    // When new bgl entries are created, we use binding numbers larger than
    // kMaxBindingsPerBindGroup to ensure there are no collisions.
    uint32_t nextOpenBindingNumberForNewEntry = kMaxBindingsPerBindGroup;
    for (uint32_t i = 0; i < descriptor->entryCount; i++) {
        UnpackedPtr<BindGroupLayoutEntry> entry = Unpack(&descriptor->entries[i]);

        // External textures are expanded from a texture_external into two sampled texture
        // bindings and one uniform buffer binding. The original binding number is used
        // for the first sampled texture.
        if (entry.Get<ExternalTextureBindingLayout>()) {
            for (SingleShaderStage stage : IterateStages(entry->visibility)) {
                // External textures are not fully implemented, which means that expanding
                // the external texture at this time will not occupy the same number of
                // binding slots as defined in the WebGPU specification. Here we prematurely
                // increment the binding counts for an additional sampled textures and a
                // sampler so that an external texture will occupy the correct number of
                // slots for correct validation of shader binding limits.
                // TODO(dawn:1082): Consider removing this and instead making a change to
                // the validation.
                constexpr uint32_t kUnimplementedSampledTexturesPerExternalTexture = 2;
                constexpr uint32_t kUnimplementedSamplersPerExternalTexture = 1;
                bindingCounts->perStage[stage].sampledTextureCount +=
                    kUnimplementedSampledTexturesPerExternalTexture;
                bindingCounts->perStage[stage].samplerCount +=
                    kUnimplementedSamplersPerExternalTexture;
            }

            dawn::native::ExternalTextureBindingExpansion bindingExpansion;

            BindGroupLayoutEntry plane0Entry =
                CreateSampledTextureBindingForExternalTexture(entry->binding, entry->visibility);
            bindingExpansion.plane0 = BindingNumber(plane0Entry.binding);
            expandedOutput.push_back(Unpack(&additionalEntries.emplace_back(plane0Entry)));

            BindGroupLayoutEntry plane1Entry = CreateSampledTextureBindingForExternalTexture(
                nextOpenBindingNumberForNewEntry++, entry->visibility);
            bindingExpansion.plane1 = BindingNumber(plane1Entry.binding);
            expandedOutput.push_back(Unpack(&additionalEntries.emplace_back(plane1Entry)));

            BindGroupLayoutEntry paramsEntry = CreateUniformBindingForExternalTexture(
                nextOpenBindingNumberForNewEntry++, entry->visibility);
            bindingExpansion.params = BindingNumber(paramsEntry.binding);
            expandedOutput.push_back(Unpack(&additionalEntries.emplace_back(paramsEntry)));

            externalTextureBindingExpansions->insert(
                {BindingNumber(entry->binding), bindingExpansion});
        } else {
            if (auto* staticSamplerBindingLayout = entry.Get<StaticSamplerBindingLayout>()) {
                if (staticSamplerBindingLayout->sampledTextureBinding != WGPU_LIMIT_U32_UNDEFINED) {
                    *needsCrossBindingValidation = true;
                }
            }

            expandedOutput.push_back(entry);
        }
    }
    return result;
}
}  // anonymous namespace

MaybeError ValidateBindGroupLayoutDescriptor(DeviceBase* device,
                                             const BindGroupLayoutDescriptor* descriptor,
                                             bool allowInternalBinding) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr");

    // Map of binding number to entry index.
    std::map<BindingNumber, uint32_t> bindingMap;
    BindingCounts bindingCounts = {};

    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        UnpackedPtr<BindGroupLayoutEntry> entry;
        DAWN_TRY_ASSIGN(entry, ValidateAndUnpack(&descriptor->entries[i]));
        BindingNumber bindingNumber = BindingNumber(entry->binding);

        DAWN_INVALID_IF(bindingNumber >= kMaxBindingsPerBindGroupTyped,
                        "Binding number (%u) exceeds the maxBindingsPerBindGroup limit (%u).",
                        uint32_t(bindingNumber), kMaxBindingsPerBindGroup);
        DAWN_INVALID_IF(bindingMap.count(bindingNumber) != 0,
                        "On entries[%u]: binding index (%u) was specified by a previous entry.", i,
                        entry->binding);

        DAWN_TRY_CONTEXT(ValidateBindGroupLayoutEntry(device, entry, allowInternalBinding),
                         "validating entries[%u]", i);

        IncrementBindingCounts(&bindingCounts, entry);

        bindingMap.insert({bindingNumber, i});
    }

    // Perform a second validation pass for static samplers. This is done after initial validation
    // as static samplers can have associated texture entries that need to be validated first.
    DAWN_TRY(ValidateStaticSamplersWithTextureBindings(device, descriptor, bindingMap));

    DAWN_TRY_CONTEXT(
        ValidateBindingCounts(device->GetLimits(), bindingCounts, device->GetAdapter()),
        "validating binding counts");

    return {};
}

namespace {

bool operator!=(const BindingInfo& a, const BindingInfo& b) {
    if (a.visibility != b.visibility || a.bindingLayout.index() != b.bindingLayout.index()) {
        return true;
    }

    return MatchVariant(
        a.bindingLayout,
        [&](const BufferBindingInfo& layoutA) -> bool {
            const BufferBindingInfo& layoutB = std::get<BufferBindingInfo>(b.bindingLayout);
            return layoutA.type != layoutB.type ||
                   layoutA.hasDynamicOffset != layoutB.hasDynamicOffset ||
                   layoutA.minBindingSize != layoutB.minBindingSize;
        },
        [&](const SamplerBindingInfo& layoutA) -> bool {
            const SamplerBindingInfo& layoutB = std::get<SamplerBindingInfo>(b.bindingLayout);
            return layoutA.type != layoutB.type;
        },
        [&](const StaticSamplerBindingInfo& layoutA) -> bool {
            const StaticSamplerBindingInfo& layoutB =
                std::get<StaticSamplerBindingInfo>(b.bindingLayout);
            return layoutA.sampler != layoutB.sampler;
        },
        [&](const TextureBindingInfo& layoutA) -> bool {
            const TextureBindingInfo& layoutB = std::get<TextureBindingInfo>(b.bindingLayout);
            return layoutA.sampleType != layoutB.sampleType ||
                   layoutA.viewDimension != layoutB.viewDimension ||
                   layoutA.multisampled != layoutB.multisampled;
        },
        [&](const StorageTextureBindingInfo& layoutA) -> bool {
            const StorageTextureBindingInfo& layoutB =
                std::get<StorageTextureBindingInfo>(b.bindingLayout);
            return layoutA.access != layoutB.access ||
                   layoutA.viewDimension != layoutB.viewDimension ||
                   layoutA.format != layoutB.format;
        },
        [&](const InputAttachmentBindingInfo& layoutA) -> bool {
            const InputAttachmentBindingInfo& layoutB =
                std::get<InputAttachmentBindingInfo>(b.bindingLayout);
            return layoutA.sampleType != layoutB.sampleType;
        });
}

bool IsBufferBinding(const UnpackedPtr<BindGroupLayoutEntry>& binding) {
    return binding->buffer.type != wgpu::BufferBindingType::BindingNotUsed;
}

bool BindingHasDynamicOffset(const UnpackedPtr<BindGroupLayoutEntry>& binding) {
    if (binding->buffer.type != wgpu::BufferBindingType::BindingNotUsed) {
        return binding->buffer.hasDynamicOffset;
    }
    return false;
}

BindingInfo CreateBindGroupLayoutInfo(const UnpackedPtr<BindGroupLayoutEntry>& binding) {
    BindingInfo bindingInfo;
    bindingInfo.binding = BindingNumber(binding->binding);
    bindingInfo.visibility = binding->visibility;

    if (binding->buffer.type != wgpu::BufferBindingType::BindingNotUsed) {
        bindingInfo.bindingLayout =
            BufferBindingInfo(binding->buffer.WithTrivialFrontendDefaults());
    } else if (binding->sampler.type != wgpu::SamplerBindingType::BindingNotUsed) {
        bindingInfo.bindingLayout =
            SamplerBindingInfo(binding->sampler.WithTrivialFrontendDefaults());
    } else if (binding->texture.sampleType != wgpu::TextureSampleType::BindingNotUsed) {
        auto textureBindingInfo =
            TextureBindingInfo(binding->texture.WithTrivialFrontendDefaults());
        if (binding->texture.viewDimension == kInternalInputAttachmentDim) {
            bindingInfo.bindingLayout = InputAttachmentBindingInfo(textureBindingInfo.sampleType);
        } else {
            bindingInfo.bindingLayout = textureBindingInfo;
        }
    } else if (binding->storageTexture.access != wgpu::StorageTextureAccess::BindingNotUsed) {
        bindingInfo.bindingLayout =
            StorageTextureBindingInfo(binding->storageTexture.WithTrivialFrontendDefaults());
    } else if (auto* staticSamplerBindingLayout = binding.Get<StaticSamplerBindingLayout>()) {
        bindingInfo.bindingLayout = StaticSamplerBindingInfo(*staticSamplerBindingLayout);
    } else {
        DAWN_UNREACHABLE();
    }

    return bindingInfo;
}

bool SortBindingsCompare(const UnpackedPtr<BindGroupLayoutEntry>& a,
                         const UnpackedPtr<BindGroupLayoutEntry>& b) {
    if (&a == &b) {
        return false;
    }

    const bool aIsBuffer = IsBufferBinding(a);
    const bool bIsBuffer = IsBufferBinding(b);
    if (aIsBuffer != bIsBuffer) {
        // Always place buffers first.
        return aIsBuffer;
    }

    if (aIsBuffer) {
        bool aHasDynamicOffset = BindingHasDynamicOffset(a);
        bool bHasDynamicOffset = BindingHasDynamicOffset(b);
        DAWN_ASSERT(bIsBuffer);
        if (aHasDynamicOffset != bHasDynamicOffset) {
            // Buffers with dynamic offsets should come before those without.
            // This makes it easy to iterate over the dynamic buffer bindings
            // [0, dynamicBufferCount) during validation.
            return aHasDynamicOffset;
        }
        if (aHasDynamicOffset) {
            DAWN_ASSERT(bHasDynamicOffset);
            DAWN_ASSERT(a->binding != b->binding);
            // Above, we ensured that dynamic buffers are first. Now, ensure that
            // dynamic buffer bindings are in increasing order. This is because dynamic
            // buffer offsets are applied in increasing order of binding number.
            return a->binding < b->binding;
        }
    }

    // This applies some defaults and gives us a single value to check for the binding type.
    BindingInfo aInfo = CreateBindGroupLayoutInfo(a);
    BindingInfo bInfo = CreateBindGroupLayoutInfo(b);

    // Sort by type.
    if (aInfo.bindingLayout.index() != bInfo.bindingLayout.index()) {
        return GetBindingInfoType(aInfo) < GetBindingInfoType(bInfo);
    }

    if (a->visibility != b->visibility) {
        return a->visibility < b->visibility;
    }

    switch (GetBindingInfoType(aInfo)) {
        case BindingInfoType::Buffer: {
            const auto& aLayout = std::get<BufferBindingInfo>(aInfo.bindingLayout);
            const auto& bLayout = std::get<BufferBindingInfo>(bInfo.bindingLayout);
            if (aLayout.minBindingSize != bLayout.minBindingSize) {
                return aLayout.minBindingSize < bLayout.minBindingSize;
            }
            break;
        }
        case BindingInfoType::Sampler: {
            const auto& aLayout = std::get<SamplerBindingInfo>(aInfo.bindingLayout);
            const auto& bLayout = std::get<SamplerBindingInfo>(bInfo.bindingLayout);
            if (aLayout.type != bLayout.type) {
                return aLayout.type < bLayout.type;
            }
            break;
        }
        case BindingInfoType::Texture: {
            const auto& aLayout = std::get<TextureBindingInfo>(aInfo.bindingLayout);
            const auto& bLayout = std::get<TextureBindingInfo>(bInfo.bindingLayout);
            if (aLayout.multisampled != bLayout.multisampled) {
                return aLayout.multisampled < bLayout.multisampled;
            }
            if (aLayout.viewDimension != bLayout.viewDimension) {
                return aLayout.viewDimension < bLayout.viewDimension;
            }
            if (aLayout.sampleType != bLayout.sampleType) {
                return aLayout.sampleType < bLayout.sampleType;
            }
            break;
        }
        case BindingInfoType::StorageTexture: {
            const auto& aLayout = std::get<StorageTextureBindingInfo>(aInfo.bindingLayout);
            const auto& bLayout = std::get<StorageTextureBindingInfo>(bInfo.bindingLayout);
            if (aLayout.access != bLayout.access) {
                return aLayout.access < bLayout.access;
            }
            if (aLayout.viewDimension != bLayout.viewDimension) {
                return aLayout.viewDimension < bLayout.viewDimension;
            }
            if (aLayout.format != bLayout.format) {
                return aLayout.format < bLayout.format;
            }
            break;
        }
        case BindingInfoType::StaticSampler: {
            const auto& aLayout = std::get<StaticSamplerBindingInfo>(aInfo.bindingLayout);
            const auto& bLayout = std::get<StaticSamplerBindingInfo>(bInfo.bindingLayout);
            if (aLayout.sampler != bLayout.sampler) {
                return aLayout.sampler < bLayout.sampler;
            }
            break;
        }
        case BindingInfoType::ExternalTexture:
            DAWN_UNREACHABLE();
            break;
        case BindingInfoType::InputAttachment: {
            const auto& aLayout = std::get<InputAttachmentBindingInfo>(aInfo.bindingLayout);
            const auto& bLayout = std::get<InputAttachmentBindingInfo>(bInfo.bindingLayout);
            if (aLayout.sampleType != bLayout.sampleType) {
                return aLayout.sampleType < bLayout.sampleType;
            }
            break;
        }
    }
    return a->binding < b->binding;
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
    const BindGroupLayoutDescriptor* descriptor,
    ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label), mUnexpandedBindingCount(descriptor->entryCount) {
    auto unpackedBindings = ExtractAndExpandBglEntries(descriptor, &mBindingCounts,
                                                       &mExternalTextureBindingExpansionMap,
                                                       &mNeedsCrossBindingValidation);
    auto& sortedBindings = unpackedBindings.unpackedEntries;

    std::sort(sortedBindings.begin(), sortedBindings.end(), SortBindingsCompare);

    for (uint32_t i = 0; i < sortedBindings.size(); ++i) {
        const UnpackedPtr<BindGroupLayoutEntry>& binding = sortedBindings[static_cast<uint32_t>(i)];
        mBindingInfo.push_back(CreateBindGroupLayoutInfo(binding));

        if (IsBufferBinding(binding)) {
            // Buffers must be contiguously packed at the start of the binding info.
            DAWN_ASSERT(GetBufferCount() == BindingIndex(i));
        }
        IncrementBindingCounts(&mBindingCounts, binding);

        const auto& [_, inserted] = mBindingMap.emplace(BindingNumber(binding->binding), i);
        DAWN_ASSERT(inserted);
    }
    DAWN_ASSERT(CheckBufferBindingsFirst({mBindingInfo.data(), GetBindingCount()}));
    DAWN_ASSERT(mBindingInfo.size() <= kMaxBindingsPerPipelineLayoutTyped);
}

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(
    DeviceBase* device,
    const BindGroupLayoutDescriptor* descriptor)
    : BindGroupLayoutInternalBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(DeviceBase* device,
                                                         ObjectBase::ErrorTag tag,
                                                         StringView label)
    : ApiObjectBase(device, tag, label) {}

BindGroupLayoutInternalBase::~BindGroupLayoutInternalBase() = default;

void BindGroupLayoutInternalBase::DestroyImpl() {
    Uncache();
}

ObjectType BindGroupLayoutInternalBase::GetType() const {
    return ObjectType::BindGroupLayoutInternal;
}

const BindingInfo& BindGroupLayoutInternalBase::GetBindingInfo(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(bindingIndex < mBindingInfo.size());
    return mBindingInfo[bindingIndex];
}

const BindGroupLayoutInternalBase::BindingMap& BindGroupLayoutInternalBase::GetBindingMap() const {
    DAWN_ASSERT(!IsError());
    return mBindingMap;
}

bool BindGroupLayoutInternalBase::HasBinding(BindingNumber bindingNumber) const {
    return mBindingMap.count(bindingNumber) != 0;
}

BindingIndex BindGroupLayoutInternalBase::GetBindingIndex(BindingNumber bindingNumber) const {
    DAWN_ASSERT(!IsError());
    const auto& it = mBindingMap.find(bindingNumber);
    DAWN_ASSERT(it != mBindingMap.end());
    return it->second;
}

void BindGroupLayoutInternalBase::ReduceMemoryUsage() {}

size_t BindGroupLayoutInternalBase::ComputeContentHash() {
    ObjectContentHasher recorder;

    // std::map is sorted by key, so two BGLs constructed in different orders
    // will still record the same.
    for (const auto [id, index] : mBindingMap) {
        recorder.Record(id, index);

        const BindingInfo& info = mBindingInfo[index];
        recorder.Record(info.visibility);

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
            [&](const StaticSamplerBindingInfo& layout) {
                recorder.Record(BindingInfoType::StaticSampler, layout.sampler->GetContentHash());
            },
            [&](const InputAttachmentBindingInfo& layout) {
                recorder.Record(BindingInfoType::InputAttachment, layout.sampleType);
            });
    }

    return recorder.GetContentHash();
}

bool BindGroupLayoutInternalBase::EqualityFunc::operator()(
    const BindGroupLayoutInternalBase* a,
    const BindGroupLayoutInternalBase* b) const {
    return a->IsLayoutEqual(b);
}

bool BindGroupLayoutInternalBase::IsEmpty() const {
    return mBindingInfo.empty();
}

BindingIndex BindGroupLayoutInternalBase::GetBindingCount() const {
    return mBindingInfo.size();
}

BindingIndex BindGroupLayoutInternalBase::GetBufferCount() const {
    return BindingIndex(mBindingCounts.bufferCount);
}

BindingIndex BindGroupLayoutInternalBase::GetDynamicBufferCount() const {
    // This is a binding index because dynamic buffers are packed at the front of the binding
    // info.
    return static_cast<BindingIndex>(mBindingCounts.dynamicStorageBufferCount +
                                     mBindingCounts.dynamicUniformBufferCount);
}

uint32_t BindGroupLayoutInternalBase::GetUnverifiedBufferCount() const {
    return mBindingCounts.unverifiedBufferCount;
}

uint32_t BindGroupLayoutInternalBase::GetStaticSamplerCount() const {
    return mBindingCounts.staticSamplerCount;
}

uint32_t BindGroupLayoutInternalBase::GetExternalTextureBindingCount() const {
    return mExternalTextureBindingExpansionMap.size();
}

const BindingCounts& BindGroupLayoutInternalBase::GetBindingCountInfo() const {
    return mBindingCounts;
}

const ExternalTextureBindingExpansionMap&
BindGroupLayoutInternalBase::GetExternalTextureBindingExpansionMap() const {
    return mExternalTextureBindingExpansionMap;
}

bool BindGroupLayoutInternalBase::NeedsCrossBindingValidation() const {
    return mNeedsCrossBindingValidation;
}

uint32_t BindGroupLayoutInternalBase::GetUnexpandedBindingCount() const {
    return mUnexpandedBindingCount;
}

bool BindGroupLayoutInternalBase::IsLayoutEqual(const BindGroupLayoutInternalBase* other) const {
    if (GetBindingCount() != other->GetBindingCount()) {
        return false;
    }
    for (BindingIndex i{0}; i < GetBindingCount(); ++i) {
        if (mBindingInfo[i] != other->mBindingInfo[i]) {
            return false;
        }
    }
    return mBindingMap == other->mBindingMap;
}

size_t BindGroupLayoutInternalBase::GetBindingDataSize() const {
    // | ------ buffer-specific ----------| ------------ object pointers -------------|
    // | --- offsets + sizes -------------| --------------- Ref<ObjectBase> ----------|
    // Followed by:
    // |---------buffer size array--------|
    // |-uint64_t[mUnverifiedBufferCount]-|
    size_t objectPointerStart = mBindingCounts.bufferCount * sizeof(BufferBindingData);
    DAWN_ASSERT(IsAligned(objectPointerStart, alignof(Ref<ObjectBase>)));
    size_t bufferSizeArrayStart = Align(
        objectPointerStart + mBindingCounts.totalCount * sizeof(Ref<ObjectBase>), sizeof(uint64_t));
    DAWN_ASSERT(IsAligned(bufferSizeArrayStart, alignof(uint64_t)));
    return bufferSizeArrayStart + mBindingCounts.unverifiedBufferCount * sizeof(uint64_t);
}

BindGroupLayoutInternalBase::BindingDataPointers
BindGroupLayoutInternalBase::ComputeBindingDataPointers(void* dataStart) const {
    BufferBindingData* bufferData = reinterpret_cast<BufferBindingData*>(dataStart);
    auto bindings = reinterpret_cast<Ref<ObjectBase>*>(bufferData + mBindingCounts.bufferCount);
    uint64_t* unverifiedBufferSizes = AlignPtr(
        reinterpret_cast<uint64_t*>(bindings + mBindingCounts.totalCount), sizeof(uint64_t));

    DAWN_ASSERT(IsPtrAligned(bufferData, alignof(BufferBindingData)));
    DAWN_ASSERT(IsPtrAligned(bindings, alignof(Ref<ObjectBase>)));
    DAWN_ASSERT(IsPtrAligned(unverifiedBufferSizes, alignof(uint64_t)));

    return {{bufferData, GetBufferCount()},
            {bindings, GetBindingCount()},
            {unverifiedBufferSizes, mBindingCounts.unverifiedBufferCount}};
}

bool BindGroupLayoutInternalBase::IsStorageBufferBinding(BindingIndex bindingIndex) const {
    DAWN_ASSERT(bindingIndex < GetBufferCount());
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

std::string BindGroupLayoutInternalBase::EntriesToString() const {
    std::string entries = "[";
    std::string sep = "";
    const BindGroupLayoutInternalBase::BindingMap& bindingMap = GetBindingMap();
    for (const auto [bindingNumber, bindingIndex] : bindingMap) {
        const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);
        entries += absl::StrFormat("%s%s", sep, bindingInfo);
        sep = ", ";
    }
    entries += "]";
    return entries;
}

}  // namespace dawn::native
