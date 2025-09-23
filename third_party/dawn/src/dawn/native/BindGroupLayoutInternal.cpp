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
#include "dawn/common/Range.h"
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
#include "dawn/platform/metrics/HistogramMacros.h"

namespace dawn::native {

namespace {

bool TextureFormatSupportStorageAccess(const Format& format, wgpu::StorageTextureAccess access) {
    switch (access) {
        case wgpu::StorageTextureAccess::ReadOnly:
            return format.supportsReadOnlyStorageUsage;
        case wgpu::StorageTextureAccess::WriteOnly:
            return format.supportsWriteOnlyStorageUsage;
        case wgpu::StorageTextureAccess::ReadWrite:
            return format.supportsReadWriteStorageUsage;
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

    // TODO(427681156): Remove this deprecation warning
    if (storageTextureFormat == wgpu::TextureFormat::BGRA8Unorm &&
        access == wgpu::StorageTextureAccess::ReadOnly) {
        DAWN_HISTOGRAM_BOOLEAN(device->GetPlatform(), "BGRA8UnormStorageTextureReadOnlyUsage",
                               true);
        device->EmitWarningOnce(
            "bgra8unorm with read-only access is deprecated. bgra8unorm only supports write-only "
            "access. Note: allowing this usage was a bug in Chrome. The spec disallows it as it is "
            "not portable.");
    }

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

    if (entry.Get<ExternalTextureBindingLayout>()) {
        bindingMemberCount++;
        DAWN_INVALID_IF(arraySize > 1,
                        "BindGroupLayoutEntry bindingArraySize (%u) > 1 for an "
                        "external texture entry.",
                        arraySize);
    }

    DAWN_INVALID_IF(bindingMemberCount == 0,
                    "BindGroupLayoutEntry had none of buffer, sampler, texture, "
                    "storageTexture, or externalTexture set");

    DAWN_INVALID_IF(bindingMemberCount != 1,
                    "BindGroupLayoutEntry had more than one of buffer, sampler, texture, "
                    "storageTexture, or externalTexture set");

    DAWN_INVALID_IF(
        arraySize > 1 && entry->texture.sampleType == wgpu::TextureSampleType::BindingNotUsed,
        "Entry that is not a sampled texture has an bindingArraySize (%u) > 1.", arraySize);

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
        DAWN_INVALID_IF(
            bindingNumber >= kMaxBindingsPerBindGroupTyped,
            "On entries[%u]: binding number (%u) exceeds the maxBindingsPerBindGroup limit (%u).",
            i, uint32_t(bindingNumber), kMaxBindingsPerBindGroup);

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

    return {};
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
    } else if (auto* staticSamplerBindingLayout = binding.Get<StaticSamplerBindingLayout>()) {
        bindingInfo.bindingLayout = StaticSamplerBindingInfo::From(*staticSamplerBindingLayout);
    } else {
        DAWN_UNREACHABLE();
    }

    return bindingInfo;
}

// This function handles the conversion of the API format for each binding info to Dawn's internal
// representation of them. This is also where the ExternalTextures are replaced and expanded in the
// various bindings that are used internally in Dawn. Arrays are also expanded to individual
// bindings here.
struct ExpandedBindingInfo {
    ityp::vector<BindingIndex, BindingInfo> entries;
    ExternalTextureBindingExpansionMap externalTextureBindingExpansions;
};
ExpandedBindingInfo ConvertAndExpandBGLEntries(const BindGroupLayoutDescriptor* descriptor) {
    ExpandedBindingInfo result;

    // When new bgl entries are created, we use binding numbers larger than kMaxBindingsPerBindGroup
    // to ensure there are no collisions.
    BindingNumber nextOpenBindingNumberForNewEntry = kMaxBindingsPerBindGroupTyped;
    for (uint32_t i = 0; i < descriptor->entryCount; i++) {
        UnpackedPtr<BindGroupLayoutEntry> entry = Unpack(&descriptor->entries[i]);

        // External textures are expanded from a texture_external into two sampled texture bindings
        // and one uniform buffer binding. The original binding number is used for the first sampled
        // texture.
        if (entry.Get<ExternalTextureBindingLayout>()) {
            DAWN_ASSERT(entry->bindingArraySize <= 1);
            dawn::native::ExternalTextureBindingExpansion bindingExpansion;

            BindingInfo plane0Entry = CreateSampledTextureBindingForExternalTexture(
                BindingNumber(entry->binding), entry->visibility);
            bindingExpansion.plane0 = BindingNumber(plane0Entry.binding);
            result.entries.push_back(plane0Entry);

            BindingInfo plane1Entry = CreateSampledTextureBindingForExternalTexture(
                nextOpenBindingNumberForNewEntry++, entry->visibility);
            bindingExpansion.plane1 = BindingNumber(plane1Entry.binding);
            result.entries.push_back(plane1Entry);

            BindingInfo paramsEntry = CreateUniformBindingForExternalTexture(
                nextOpenBindingNumberForNewEntry++, entry->visibility);
            bindingExpansion.params = BindingNumber(paramsEntry.binding);
            result.entries.push_back(paramsEntry);

            result.externalTextureBindingExpansions.insert(
                {BindingNumber(entry->binding), bindingExpansion});
            continue;
        }

        // Add one BindingInfo per element of the array with increasing indexInArray for backends to
        // know which element it is when they need it, but also with increasing BindingNumber as the
        // array takes consecutive binding numbers on the API side.
        BindingInfo info = ConvertToBindingInfo(entry);
        for (BindingIndex indexInArray : Range(info.arraySize)) {
            info.indexInArray = indexInArray;
            result.entries.push_back(info);
            info.binding++;
        }
    }
    return result;
}

bool SortBindingsCompare(const BindingInfo& a, const BindingInfo& b) {
    if (&a == &b) {
        return false;
    }

    // Buffers with dynamic offsets come first and then the rest of the buffers. Other bindings are
    // only grouped by types. This is to make it easier and faster to handle them.
    auto TypePriority = [](const BindingInfo& info) {
        return MatchVariant(
            info.bindingLayout,
            [&](const BufferBindingInfo& layout) { return layout.hasDynamicOffset ? 0 : 1; },
            [&](const TextureBindingInfo&) { return 2; },
            [&](const StorageTextureBindingInfo&) { return 3; },
            [&](const SamplerBindingInfo&) { return 4; },
            [&](const StaticSamplerBindingInfo&) { return 5; },
            [&](const InputAttachmentBindingInfo&) { return 6; });
    };

    auto aPriority = TypePriority(a);
    auto bPriority = TypePriority(b);
    if (aPriority != bPriority) {
        return aPriority < bPriority;
    }

    // Afterwards sort the bindings by binding number. This is necessary because dynamic buffers
    // are applied in order of increasing binding number in SetBindGroup.
    return a.binding < b.binding;
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
    : ApiObjectBase(device, descriptor->label) {
    ExpandedBindingInfo unpackedBindings = ConvertAndExpandBGLEntries(descriptor);
    mExternalTextureBindingExpansionMap =
        std::move(unpackedBindings.externalTextureBindingExpansions);
    mBindingInfo = std::move(unpackedBindings.entries);

    // Reorder bindings internally and compute the BindingNumber->BindingIndex map.
    std::sort(mBindingInfo.begin(), mBindingInfo.end(), SortBindingsCompare);
    for (const auto [i, binding] : Enumerate(mBindingInfo)) {
        const auto& [_, inserted] = mBindingMap.emplace(binding.binding, i);
        DAWN_ASSERT(inserted);
    }

    DAWN_ASSERT(CheckBufferBindingsFirst({mBindingInfo.data(), GetBindingCount()}));
    DAWN_ASSERT(mBindingInfo.size() <= kMaxBindingsPerPipelineLayoutTyped);

    // Compute various counts of expanded bindings and other metadata.
    for (const auto& binding : mBindingInfo) {
        MatchVariant(
            binding.bindingLayout,
            [&](const BufferBindingInfo& layout) {
                mBufferCount++;
                if (layout.minBindingSize == 0) {
                    mUnverifiedBufferCount++;
                }
                if (layout.hasDynamicOffset) {
                    mDynamicBufferCount++;
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
                }
            },
            [&](const TextureBindingInfo&) {}, [&](const StorageTextureBindingInfo&) {},
            [&](const SamplerBindingInfo&) {},
            [&](const StaticSamplerBindingInfo& layout) {
                mStaticSamplerCount++;
                if (layout.isUsedForSingleTextureBinding) {
                    mNeedsCrossBindingValidation = true;
                }
            },
            [&](const InputAttachmentBindingInfo&) {});
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
    DAWN_ASSERT(!IsError());
    return mBindingMap.contains(bindingNumber);
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
    if (a->GetBindingCount() != b->GetBindingCount()) {
        return false;
    }
    for (BindingIndex i{0}; i < a->GetBindingCount(); ++i) {
        if (a->mBindingInfo[i] != b->mBindingInfo[i]) {
            return false;
        }
    }
    return a->mBindingMap == b->mBindingMap;
}

bool BindGroupLayoutInternalBase::IsEmpty() const {
    DAWN_ASSERT(!IsError());
    return mBindingInfo.empty();
}

BindingIndex BindGroupLayoutInternalBase::GetBindingCount() const {
    DAWN_ASSERT(!IsError());
    return mBindingInfo.size();
}

BindingIndex BindGroupLayoutInternalBase::GetBufferCount() const {
    DAWN_ASSERT(!IsError());
    return BindingIndex(mBufferCount);
}

BindingIndex BindGroupLayoutInternalBase::GetDynamicBufferCount() const {
    DAWN_ASSERT(!IsError());
    return BindingIndex(mDynamicBufferCount);
}

uint32_t BindGroupLayoutInternalBase::GetDynamicStorageBufferCount() const {
    DAWN_ASSERT(!IsError());
    return mDynamicStorageBufferCount;
}

uint32_t BindGroupLayoutInternalBase::GetUnverifiedBufferCount() const {
    DAWN_ASSERT(!IsError());
    return mUnverifiedBufferCount;
}

uint32_t BindGroupLayoutInternalBase::GetStaticSamplerCount() const {
    DAWN_ASSERT(!IsError());
    return mStaticSamplerCount;
}

const BindingCounts& BindGroupLayoutInternalBase::GetValidationBindingCounts() const {
    DAWN_ASSERT(!IsError());
    return mValidationBindingCounts;
}

const ExternalTextureBindingExpansionMap&
BindGroupLayoutInternalBase::GetExternalTextureBindingExpansionMap() const {
    DAWN_ASSERT(!IsError());
    return mExternalTextureBindingExpansionMap;
}

bool BindGroupLayoutInternalBase::NeedsCrossBindingValidation() const {
    DAWN_ASSERT(!IsError());
    return mNeedsCrossBindingValidation;
}

uint32_t BindGroupLayoutInternalBase::GetUnexpandedBindingCount() const {
    DAWN_ASSERT(!IsError());
    return mValidationBindingCounts.totalCount;
}

size_t BindGroupLayoutInternalBase::GetBindingDataSize() const {
    DAWN_ASSERT(!IsError());
    // | ------ buffer-specific ----------| ------------ object pointers -------------|
    // | --- offsets + sizes -------------| --------------- Ref<ObjectBase> ----------|
    // Followed by:
    // |---------buffer size array--------|
    // |-uint64_t[mUnverifiedBufferCount]-|
    const uint64_t bindingCount = uint64_t(uint32_t(mBindingInfo.size()));
    size_t objectPointerStart = mBufferCount * sizeof(BufferBindingData);
    DAWN_ASSERT(IsAligned(objectPointerStart, alignof(Ref<ObjectBase>)));
    size_t bufferSizeArrayStart =
        Align(objectPointerStart + bindingCount * sizeof(Ref<ObjectBase>), sizeof(uint64_t));
    DAWN_ASSERT(IsAligned(bufferSizeArrayStart, alignof(uint64_t)));
    return bufferSizeArrayStart + mUnverifiedBufferCount * sizeof(uint64_t);
}

BindGroupLayoutInternalBase::BindingDataPointers
BindGroupLayoutInternalBase::ComputeBindingDataPointers(void* dataStart) const {
    const uint64_t bindingCount = uint64_t(uint32_t(mBindingInfo.size()));
    BufferBindingData* bufferData = reinterpret_cast<BufferBindingData*>(dataStart);
    auto bindings = reinterpret_cast<Ref<ObjectBase>*>(bufferData + mBufferCount);
    uint64_t* unverifiedBufferSizes =
        AlignPtr(reinterpret_cast<uint64_t*>(bindings + bindingCount), sizeof(uint64_t));

    DAWN_ASSERT(IsPtrAligned(bufferData, alignof(BufferBindingData)));
    DAWN_ASSERT(IsPtrAligned(bindings, alignof(Ref<ObjectBase>)));
    DAWN_ASSERT(IsPtrAligned(unverifiedBufferSizes, alignof(uint64_t)));

    return {{bufferData, GetBufferCount()},
            {bindings, GetBindingCount()},
            {unverifiedBufferSizes, mUnverifiedBufferCount}};
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
