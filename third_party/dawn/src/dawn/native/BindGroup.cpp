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

#include "dawn/native/BindGroup.h"

#include <variant>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Assert.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Math.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/Texture.h"
#include "dawn/native/utils/WGPUHelpers.h"

namespace dawn::native {

namespace {

// Helper functions to perform binding-type specific validation

MaybeError ValidateBufferBinding(const DeviceBase* device,
                                 const BindGroupEntry& entry,
                                 const BufferBindingInfo& layout) {
    DAWN_INVALID_IF(entry.buffer == nullptr, "Binding entry buffer not set.");

    DAWN_INVALID_IF(entry.sampler != nullptr || entry.textureView != nullptr,
                    "Expected only buffer to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY(device->ValidateObject(entry.buffer));

    uint64_t bufferSize = entry.buffer->GetSize();

    // Handle wgpu::WholeSize, avoiding overflows.
    DAWN_INVALID_IF(entry.offset > bufferSize,
                    "Binding offset (%u) is larger than the size (%u) of %s.", entry.offset,
                    bufferSize, entry.buffer);

    uint64_t bindingSize =
        (entry.size == wgpu::kWholeSize) ? bufferSize - entry.offset : entry.size;

    DAWN_INVALID_IF(bindingSize > bufferSize,
                    "Binding size (%u) is larger than the size (%u) of %s.", bindingSize,
                    bufferSize, entry.buffer);

    DAWN_INVALID_IF(bindingSize == 0, "Binding size for %s is zero.", entry.buffer);

    // Note that no overflow can happen because we already checked that
    // bufferSize >= bindingSize
    DAWN_INVALID_IF(entry.offset > bufferSize - bindingSize,
                    "Binding range (offset: %u, size: %u) doesn't fit in the size (%u) of %s.",
                    entry.offset, bufferSize, bindingSize, entry.buffer);

    wgpu::BufferUsage requiredUsage;
    uint64_t requiredBindingAlignment;
    switch (layout.type) {
        case wgpu::BufferBindingType::Uniform:
            requiredUsage = wgpu::BufferUsage::Uniform;
            requiredBindingAlignment = device->GetLimits().v1.minUniformBufferOffsetAlignment;
            break;
        case wgpu::BufferBindingType::Storage:
        case wgpu::BufferBindingType::ReadOnlyStorage:
            requiredUsage = wgpu::BufferUsage::Storage;
            requiredBindingAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
            DAWN_INVALID_IF(
                bindingSize % 4 != 0,
                "Binding size (%u) of %s isn't a multiple of 4 when binding type is (%s).",
                bindingSize, entry.buffer, layout.type);
            break;
        case kInternalStorageBufferBinding:
            requiredUsage = kInternalStorageBuffer;
            requiredBindingAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
            break;
        case wgpu::BufferBindingType::BindingNotUsed:
        case wgpu::BufferBindingType::Undefined:
            DAWN_UNREACHABLE();
    }

    DAWN_INVALID_IF(!IsAligned(entry.offset, requiredBindingAlignment),
                    "Offset (%u) of %s does not satisfy the minimum %s alignment (%u).",
                    entry.offset, entry.buffer, layout.type, requiredBindingAlignment);

    DAWN_INVALID_IF(!(entry.buffer->GetInternalUsage() & requiredUsage),
                    "Binding usage (%s) of %s doesn't match expected usage (%s).",
                    entry.buffer->GetUsage(), entry.buffer, requiredUsage);

    DAWN_INVALID_IF(bindingSize < layout.minBindingSize,
                    "Binding size (%u) of %s is smaller than the minimum binding size (%u).",
                    bindingSize, entry.buffer, layout.minBindingSize);

    uint64_t maxUniformBufferBindingSize;
    uint64_t maxStorageBufferBindingSize;
    switch (layout.type) {
        case wgpu::BufferBindingType::Uniform:
            maxUniformBufferBindingSize = device->GetLimits().v1.maxUniformBufferBindingSize;
            DAWN_INVALID_IF(bindingSize > maxUniformBufferBindingSize,
                            "Binding size (%u) of %s is larger than the maximum uniform buffer "
                            "binding size (%u).%s",
                            bindingSize, entry.buffer, maxUniformBufferBindingSize,
                            DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter(),
                                                        maxUniformBufferBindingSize, bindingSize));
            break;
        case wgpu::BufferBindingType::Storage:
        case wgpu::BufferBindingType::ReadOnlyStorage:
        case kInternalStorageBufferBinding:
            maxStorageBufferBindingSize = device->GetLimits().v1.maxStorageBufferBindingSize;
            DAWN_INVALID_IF(bindingSize > maxStorageBufferBindingSize,
                            "Binding size (%u) of %s is larger than the maximum storage buffer "
                            "binding size (%u).%s",
                            bindingSize, entry.buffer, maxStorageBufferBindingSize,
                            DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter(),
                                                        maxStorageBufferBindingSize, bindingSize));
            break;
        case wgpu::BufferBindingType::BindingNotUsed:
        case wgpu::BufferBindingType::Undefined:
            DAWN_UNREACHABLE();
    }

    return {};
}

MaybeError ValidateTextureBindGroupEntry(DeviceBase* device, const BindGroupEntry& entry) {
    DAWN_INVALID_IF(entry.textureView == nullptr, "Binding entry textureView not set.");

    DAWN_INVALID_IF(entry.sampler != nullptr || entry.buffer != nullptr,
                    "Expected only textureView to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    TextureViewBase* view = entry.textureView;
    DAWN_TRY(device->ValidateObject(entry.textureView));

    Aspect aspect = view->GetAspects();
    DAWN_INVALID_IF(!HasOneBit(aspect), "Multiple aspects (%s) selected in %s.", aspect, view);

    return {};
}

MaybeError ValidateCompatibilityModeTextureViewArrayLayer(DeviceBase* device,
                                                          const TextureViewBase* view,
                                                          const TextureBase* texture) {
    DAWN_INVALID_IF(
        view->GetBaseArrayLayer() != 0 || view->GetLayerCount() != texture->GetArrayLayers(),
        "Texture binding uses %s with baseArrayLayer (%u) and arrayLayerCount (%u), but must use "
        "all (%u) layers of %s in compatibility mode.",
        view, view->GetBaseArrayLayer(), view->GetLayerCount(), texture->GetArrayLayers(), texture);

    return {};
}

MaybeError ValidateSampledTextureBinding(DeviceBase* device,
                                         const BindGroupEntry& entry,
                                         const TextureBindingInfo& layout,
                                         UsageValidationMode mode) {
    DAWN_TRY(ValidateTextureBindGroupEntry(device, entry));

    TextureViewBase* view = entry.textureView;

    Aspect aspect = view->GetAspects();
    DAWN_INVALID_IF(!HasOneBit(aspect), "Multiple aspects (%s) selected in %s.", aspect, view);

    TextureBase* texture = view->GetTexture();

    SampleTypeBit supportedTypes = texture->GetFormat().GetAspectInfo(aspect).supportedSampleTypes;
    if (supportedTypes == SampleTypeBit::External) {
        DAWN_ASSERT(texture->GetSharedResourceMemoryContents());
        supportedTypes =
            static_cast<SharedTextureMemoryContents*>(texture->GetSharedResourceMemoryContents())
                ->GetExternalFormatSupportedSampleTypes();
    }
    DAWN_TRY(ValidateCanUseAs(view, wgpu::TextureUsage::TextureBinding, mode));

    DAWN_INVALID_IF(texture->IsMultisampledTexture() != layout.multisampled,
                    "Sample count (%u) of %s doesn't match expectation (multisampled: %d).",
                    texture->GetSampleCount(), texture, layout.multisampled);

    SampleTypeBit requiredType;
    if (layout.sampleType == kInternalResolveAttachmentSampleType) {
        // If the binding's sample type is kInternalResolveAttachmentSampleType,
        // then the supported types must contain float.
        requiredType = SampleTypeBit::UnfilterableFloat;
    } else {
        requiredType = SampleTypeToSampleTypeBit(layout.sampleType);
    }

    DAWN_INVALID_IF(!(supportedTypes & requiredType),
                    "None of the supported sample types (%s) of %s match the expected sample "
                    "types (%s).",
                    supportedTypes, texture, requiredType);

    DAWN_INVALID_IF(entry.textureView->GetDimension() != layout.viewDimension,
                    "Dimension (%s) of %s doesn't match the expected dimension (%s).",
                    entry.textureView->GetDimension(), entry.textureView, layout.viewDimension);

    if (!device->HasFlexibleTextureViews()) {
        DAWN_INVALID_IF(
            view->GetDimension() != texture->GetCompatibilityTextureBindingViewDimension(),
            "Dimension (%s) of %s must match textureBindingViewDimension (%s) of "
            "%s in compatibility mode.",
            view->GetDimension(), view, texture->GetCompatibilityTextureBindingViewDimension(),
            texture);

        DAWN_TRY(ValidateCompatibilityModeTextureViewArrayLayer(device, view, texture));
    }

    return {};
}

MaybeError ValidateStorageTextureBinding(DeviceBase* device,
                                         const BindGroupEntry& entry,
                                         const StorageTextureBindingInfo& layout,
                                         UsageValidationMode mode) {
    DAWN_TRY(ValidateTextureBindGroupEntry(device, entry));

    TextureViewBase* view = entry.textureView;
    TextureBase* texture = view->GetTexture();

    DAWN_TRY(ValidateCanUseAs(view, wgpu::TextureUsage::StorageBinding, mode));

    DAWN_ASSERT(!texture->IsMultisampledTexture());

    DAWN_INVALID_IF(texture->GetFormat().format != layout.format,
                    "Format (%s) of %s expected to be (%s).", texture->GetFormat().format, texture,
                    layout.format);

    DAWN_INVALID_IF(view->GetDimension() != layout.viewDimension,
                    "Dimension (%s) of %s doesn't match the expected dimension (%s).",
                    view->GetDimension(), entry.textureView, layout.viewDimension);

    DAWN_INVALID_IF(view->GetLevelCount() != 1, "mipLevelCount (%u) of %s expected to be 1.",
                    view->GetLevelCount(), view);

    if (!device->HasFlexibleTextureViews()) {
        DAWN_TRY(ValidateCompatibilityModeTextureViewArrayLayer(device, view, texture));
    }

    return {};
}

MaybeError ValidateSamplerBinding(const DeviceBase* device,
                                  const BindGroupEntry& entry,
                                  const SamplerBindingInfo& layout) {
    DAWN_INVALID_IF(entry.sampler == nullptr, "Binding entry sampler not set.");

    DAWN_INVALID_IF(entry.sampler->IsYCbCr(),
                    "YCbCr sampler is incompatible with SamplerBindingLayout");

    DAWN_INVALID_IF(entry.textureView != nullptr || entry.buffer != nullptr,
                    "Expected only sampler to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY(device->ValidateObject(entry.sampler));

    switch (layout.type) {
        case wgpu::SamplerBindingType::NonFiltering:
            DAWN_INVALID_IF(entry.sampler->IsFiltering(),
                            "Filtering sampler %s is incompatible with non-filtering sampler "
                            "binding.",
                            entry.sampler);
            [[fallthrough]];
        case wgpu::SamplerBindingType::Filtering:
            DAWN_INVALID_IF(entry.sampler->IsComparison(),
                            "Comparison sampler %s is incompatible with non-comparison sampler "
                            "binding.",
                            entry.sampler);
            break;
        case wgpu::SamplerBindingType::Comparison:
            DAWN_INVALID_IF(!entry.sampler->IsComparison(),
                            "Non-comparison sampler %s is incompatible with comparison sampler "
                            "binding.",
                            entry.sampler);
            break;
        default:
            DAWN_UNREACHABLE();
            break;
    }

    return {};
}

MaybeError ValidateExternalTextureBinding(
    const DeviceBase* device,
    const BindGroupEntry& entry,
    const ExternalTextureBindingEntry* externalTextureBindingEntry,
    const ExternalTextureBindingExpansionMap& expansions) {
    DAWN_INVALID_IF(externalTextureBindingEntry == nullptr,
                    "Binding entry external texture not set.");

    DAWN_INVALID_IF(
        entry.sampler != nullptr || entry.textureView != nullptr || entry.buffer != nullptr,
        "Expected only external texture to be set for binding entry.");

    DAWN_INVALID_IF(expansions.find(BindingNumber(entry.binding)) == expansions.end(),
                    "External texture binding entry %u is not present in the bind group layout.",
                    entry.binding);

    DAWN_TRY(device->ValidateObject(externalTextureBindingEntry->externalTexture));

    return {};
}

template <typename F>
void ForEachUnverifiedBufferBindingIndexImpl(const BindGroupLayoutInternalBase* bgl, F&& f) {
    uint32_t packedIndex = 0;
    for (BindingIndex bindingIndex{0}; bindingIndex < bgl->GetBufferCount(); ++bindingIndex) {
        const auto* bufferLayout =
            std::get_if<BufferBindingInfo>(&bgl->GetBindingInfo(bindingIndex).bindingLayout);
        if (bufferLayout == nullptr || bufferLayout->minBindingSize == 0) {
            f(bindingIndex, packedIndex++);
        }
    }
}

MaybeError ValidateStaticSamplersWithSampledTextures(const BindGroupDescriptor* descriptor,
                                                     const BindGroupLayoutInternalBase* layout) {
    absl::flat_hash_map<BindingNumber, uint32_t> bindingNumberToEntryIndexMap;
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        bindingNumberToEntryIndexMap[BindingNumber(descriptor->entries[i].binding)] = i;
    }

    // Entry indices of YCbCr textures sampled by a static sampler.
    ityp::bitset<uint32_t, kMaxBindingsPerPipelineLayout> sampledYcbcrTextures;
    for (BindingIndex index{0}; index < layout->GetBindingCount(); ++index) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(index);
        auto* staticSamplerLayout =
            std::get_if<StaticSamplerBindingInfo>(&bindingInfo.bindingLayout);
        if (staticSamplerLayout && staticSamplerLayout->isUsedForSingleTextureBinding) {
            const SamplerBase* sampler = staticSamplerLayout->sampler.Get();

            uint32_t textureEntryIndex = bindingNumberToEntryIndexMap.at(
                BindingNumber(staticSamplerLayout->sampledTextureBinding));
            const TextureViewBase* textureView = descriptor->entries[textureEntryIndex].textureView;

            // Compare static sampler and sampled textures to make sure they are compatible.
            if (sampler->IsYCbCr()) {
                DAWN_INVALID_IF(!textureView->IsYCbCr(),
                                "YCbCr static sampler at binding (%u) samples a non-YCbCr texture.",
                                bindingInfo.binding);

                sampledYcbcrTextures.set(textureEntryIndex);
            } else {
                DAWN_INVALID_IF(textureView->IsYCbCr(),
                                "Non-YCbCr static sampler at binding (%u) samples a YCbCr texture.",
                                bindingInfo.binding);
            }
        }
    }

    // Validate that all YCbCr texture entries are sampled by a static sampler.
    const auto& bindingMap = layout->GetBindingMap();
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];
        const BindingInfo& bindingInfo =
            layout->GetBindingInfo(bindingMap.at(BindingNumber(entry.binding)));
        if (std::holds_alternative<TextureBindingInfo>(bindingInfo.bindingLayout) &&
            entry.textureView && entry.textureView->IsYCbCr()) {
            DAWN_INVALID_IF(!sampledYcbcrTextures.test(i),
                            "YCbCr texture at binding (%u) is not sampled by a static sampler.",
                            entry.binding);
        }
    }

    return {};
}

}  // anonymous namespace

MaybeError ValidateBindGroupDescriptor(DeviceBase* device,
                                       const BindGroupDescriptor* descriptor,
                                       UsageValidationMode mode) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr.");

    DAWN_TRY(device->ValidateObject(descriptor->layout));

    BindGroupLayoutInternalBase* layout = descriptor->layout->GetInternalBindGroupLayout();

    // NOTE: Static sampler layout bindings should not have bind group entries,
    // as the sampler is specified in the layout itself.
    const auto expectedBindingsCount =
        layout->GetUnexpandedBindingCount() - layout->GetStaticSamplerCount();

    DAWN_INVALID_IF(
        descriptor->entryCount != expectedBindingsCount,
        "Number of entries (%u) did not match the expected number of entries (%u) for %s."
        "\nExpected layout: %s",
        descriptor->entryCount, static_cast<uint32_t>(expectedBindingsCount), layout,
        layout->EntriesToString());

    const BindGroupLayoutInternalBase::BindingMap& bindingMap = layout->GetBindingMap();
    DAWN_ASSERT(bindingMap.size() <= kMaxBindingsPerPipelineLayout);

    bool needsCrossBindingValidation = layout->NeedsCrossBindingValidation();

    ityp::bitset<BindingIndex, kMaxBindingsPerPipelineLayout> bindingsSet;
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];

        const auto& it = bindingMap.find(BindingNumber(entry.binding));
        DAWN_INVALID_IF(it == bindingMap.end(),
                        "In entries[%u], binding index %u not present in the bind group layout."
                        "\nExpected layout: %s",
                        i, entry.binding, layout->EntriesToString());

        BindingIndex bindingIndex = it->second;
        DAWN_ASSERT(bindingIndex < layout->GetBindingCount());

        DAWN_INVALID_IF(bindingsSet[bindingIndex],
                        "In entries[%u], binding index %u already used by a previous entry", i,
                        entry.binding);

        bindingsSet.set(bindingIndex);

        // Below this block we validate entries based on the bind group layout, in which
        // external textures have been expanded into their underlying contents. For this reason
        // we must identify external texture binding entries by checking the bind group entry
        // itself.
        // TODO(dawn:1293): Store external textures in
        // BindGroupLayoutBase::BindingDataPointers::bindings so checking external textures can
        // be moved in the switch below.
        UnpackedPtr<BindGroupEntry> unpacked;
        DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(&entry));
        if (auto* externalTextureBindingEntry = unpacked.Get<ExternalTextureBindingEntry>()) {
            DAWN_TRY(
                ValidateExternalTextureBinding(device, entry, externalTextureBindingEntry,
                                               layout->GetExternalTextureBindingExpansionMap()));
            continue;
        } else {
            DAWN_INVALID_IF(
                layout->GetExternalTextureBindingExpansionMap().count(BindingNumber(entry.binding)),
                "entries[%u] is not an ExternalTexture when the layout contains an "
                "ExternalTexture entry.",
                i);
        }

        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);

        // Perform binding-type specific validation.
        DAWN_TRY(MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo& layout) -> MaybeError {
                // TODO(dawn:1485): Validate buffer binding with usage validation mode.
                DAWN_TRY_CONTEXT(ValidateBufferBinding(device, entry, layout),
                                 "validating entries[%u] as a Buffer."
                                 "\nExpected entry layout: %s",
                                 i, layout);
                return {};
            },
            [&](const TextureBindingInfo& layout) -> MaybeError {
                DAWN_TRY_CONTEXT(ValidateSampledTextureBinding(device, entry, layout, mode),
                                 "validating entries[%u] as a Sampled Texture."
                                 "\nExpected entry layout: %s",
                                 i, layout);
                if (entry.textureView->IsYCbCr()) {
                    // Need to validate that the YCbCr texture is statically sampled.
                    needsCrossBindingValidation = true;
                }

                return {};
            },
            [&](const StorageTextureBindingInfo& layout) -> MaybeError {
                DAWN_TRY_CONTEXT(ValidateStorageTextureBinding(device, entry, layout, mode),
                                 "validating entries[%u] as a Storage Texture."
                                 "\nExpected entry layout: %s",
                                 i, layout);
                return {};
            },
            [&](const SamplerBindingInfo& layout) -> MaybeError {
                DAWN_TRY_CONTEXT(ValidateSamplerBinding(device, entry, layout),
                                 "validating entries[%u] as a Sampler."
                                 "\nExpected entry layout: %s",
                                 i, layout);
                return {};
            },
            [&](const StaticSamplerBindingInfo& layout) -> MaybeError {
                return DAWN_VALIDATION_ERROR(
                    "entries[%u] is provided when the layout contains a static sampler for that "
                    "binding.",
                    i);
            },
            [](const InputAttachmentBindingInfo&) -> MaybeError {
                // Internal use only. No validation.
                return {};
            }));
    }

    // This should always be true because
    //  - numBindings has to match between the bind group and its layout.
    //  - Each binding must be set at most once
    //
    // We don't validate the equality because it wouldn't be possible to cover it with a test.
    DAWN_ASSERT(bindingsSet.count() == expectedBindingsCount);

    if (needsCrossBindingValidation) {
        // This additional validation is only needed when there are static samplers used with a
        // single texture binding and/or there are YCbCr textures.
        DAWN_TRY(ValidateStaticSamplersWithSampledTextures(descriptor, layout));
    }

    return {};
}

// BindGroup

BindGroupBase::BindGroupBase(DeviceBase* device,
                             const BindGroupDescriptor* descriptor,
                             void* bindingDataStart)
    : ApiObjectBase(device, descriptor->label),
      mLayout(descriptor->layout),
      mBindingData(GetLayout()->ComputeBindingDataPointers(bindingDataStart)) {
    BindGroupLayoutInternalBase* layout = GetLayout();

    for (BindingIndex i{0}; i < layout->GetBindingCount(); ++i) {
        // TODO(enga): Shouldn't be needed when bindings are tightly packed.
        // This is to fill Ref<ObjectBase> holes with nullptrs.
        new (&mBindingData.bindings[i]) Ref<ObjectBase>();
    }

    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        UnpackedPtr<BindGroupEntry> entry = Unpack(&descriptor->entries[i]);

        BindingIndex bindingIndex = layout->GetBindingIndex(BindingNumber(entry->binding));
        DAWN_ASSERT(bindingIndex < layout->GetBindingCount());

        // Only a single binding type should be set, so once we found it we can skip to the
        // next loop iteration.

        if (entry->buffer != nullptr) {
            DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
            mBindingData.bindings[bindingIndex] = entry->buffer;
            mBindingData.bufferData[bindingIndex].offset = entry->offset;
            uint64_t bufferSize = (entry->size == wgpu::kWholeSize)
                                      ? entry->buffer->GetSize() - entry->offset
                                      : entry->size;
            mBindingData.bufferData[bindingIndex].size = bufferSize;
            continue;
        }

        if (entry->textureView != nullptr) {
            DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
            mBindingData.bindings[bindingIndex] = entry->textureView;
            continue;
        }

        if (entry->sampler != nullptr) {
            DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
            mBindingData.bindings[bindingIndex] = entry->sampler;
            continue;
        }

        // Here we unpack external texture bindings into multiple additional bindings for the
        // external texture's contents. New binding locations previously determined in the bind
        // group layout are created in this bind group and filled with the external texture's
        // underlying resources.
        if (auto* externalTextureBindingEntry = entry.Get<ExternalTextureBindingEntry>()) {
            mBoundExternalTextures.push_back(externalTextureBindingEntry->externalTexture);

            ExternalTextureBindingExpansionMap expansions =
                layout->GetExternalTextureBindingExpansionMap();
            ExternalTextureBindingExpansionMap::iterator it =
                expansions.find(BindingNumber(entry->binding));

            DAWN_ASSERT(it != expansions.end());

            BindingIndex plane0BindingIndex = layout->GetBindingIndex(it->second.plane0);
            BindingIndex plane1BindingIndex = layout->GetBindingIndex(it->second.plane1);
            BindingIndex paramsBindingIndex = layout->GetBindingIndex(it->second.params);

            DAWN_ASSERT(mBindingData.bindings[plane0BindingIndex] == nullptr);

            mBindingData.bindings[plane0BindingIndex] =
                externalTextureBindingEntry->externalTexture->GetTextureViews()[0];

            DAWN_ASSERT(mBindingData.bindings[plane1BindingIndex] == nullptr);
            mBindingData.bindings[plane1BindingIndex] =
                externalTextureBindingEntry->externalTexture->GetTextureViews()[1];

            DAWN_ASSERT(mBindingData.bindings[paramsBindingIndex] == nullptr);
            mBindingData.bindings[paramsBindingIndex] =
                externalTextureBindingEntry->externalTexture->GetParamsBuffer();
            mBindingData.bufferData[paramsBindingIndex].offset = 0;
            mBindingData.bufferData[paramsBindingIndex].size =
                sizeof(dawn::native::ExternalTextureParams);

            continue;
        }
    }

    ForEachUnverifiedBufferBindingIndexImpl(layout,
                                            [&](BindingIndex bindingIndex, uint32_t packedIndex) {
                                                mBindingData.unverifiedBufferSizes[packedIndex] =
                                                    mBindingData.bufferData[bindingIndex].size;
                                            });

    GetObjectTrackingList()->Track(this);
}

BindGroupBase::~BindGroupBase() = default;

void BindGroupBase::DestroyImpl() {
    if (mLayout != nullptr) {
        DAWN_ASSERT(!IsError());
        for (BindingIndex i{0}; i < GetLayout()->GetBindingCount(); ++i) {
            mBindingData.bindings[i].~Ref<ObjectBase>();
        }
    }
}

void BindGroupBase::DeleteThis() {
    // Add another ref to the layout so that if this is the last ref, the layout
    // is destroyed after the bind group. The bind group is slab-allocated inside
    // memory owned by the layout (except for the null backend).
    Ref<BindGroupLayoutBase> layout = mLayout;
    ApiObjectBase::DeleteThis();
}

BindGroupBase::BindGroupBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ApiObjectBase(device, tag, label), mBindingData() {}

// static
Ref<BindGroupBase> BindGroupBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new BindGroupBase(device, ObjectBase::kError, label));
}

ObjectType BindGroupBase::GetType() const {
    return ObjectType::BindGroup;
}

BindGroupLayoutBase* BindGroupBase::GetFrontendLayout() {
    DAWN_ASSERT(!IsError());
    return mLayout.Get();
}

const BindGroupLayoutBase* BindGroupBase::GetFrontendLayout() const {
    DAWN_ASSERT(!IsError());
    return mLayout.Get();
}

BindGroupLayoutInternalBase* BindGroupBase::GetLayout() {
    DAWN_ASSERT(!IsError());
    return mLayout->GetInternalBindGroupLayout();
}

const BindGroupLayoutInternalBase* BindGroupBase::GetLayout() const {
    DAWN_ASSERT(!IsError());
    return mLayout->GetInternalBindGroupLayout();
}

const ityp::span<uint32_t, uint64_t>& BindGroupBase::GetUnverifiedBufferSizes() const {
    DAWN_ASSERT(!IsError());
    return mBindingData.unverifiedBufferSizes;
}

BufferBinding BindGroupBase::GetBindingAsBufferBinding(BindingIndex bindingIndex) {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_ASSERT(bindingIndex < layout->GetBindingCount());
    DAWN_ASSERT(std::holds_alternative<BufferBindingInfo>(
        layout->GetBindingInfo(bindingIndex).bindingLayout));
    BufferBase* buffer = static_cast<BufferBase*>(mBindingData.bindings[bindingIndex].Get());
    return {buffer, mBindingData.bufferData[bindingIndex].offset,
            mBindingData.bufferData[bindingIndex].size};
}

SamplerBase* BindGroupBase::GetBindingAsSampler(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_ASSERT(bindingIndex < layout->GetBindingCount());
    DAWN_ASSERT(std::holds_alternative<SamplerBindingInfo>(
        layout->GetBindingInfo(bindingIndex).bindingLayout));
    return static_cast<SamplerBase*>(mBindingData.bindings[bindingIndex].Get());
}

TextureViewBase* BindGroupBase::GetBindingAsTextureView(BindingIndex bindingIndex) {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_ASSERT(bindingIndex < layout->GetBindingCount());
    DAWN_ASSERT(std::holds_alternative<TextureBindingInfo>(
                    layout->GetBindingInfo(bindingIndex).bindingLayout) ||
                std::holds_alternative<StorageTextureBindingInfo>(
                    layout->GetBindingInfo(bindingIndex).bindingLayout) ||
                std::holds_alternative<InputAttachmentBindingInfo>(
                    layout->GetBindingInfo(bindingIndex).bindingLayout));
    return static_cast<TextureViewBase*>(mBindingData.bindings[bindingIndex].Get());
}

const std::vector<Ref<ExternalTextureBase>>& BindGroupBase::GetBoundExternalTextures() const {
    return mBoundExternalTextures;
}

void BindGroupBase::ForEachUnverifiedBufferBindingIndex(
    std::function<void(BindingIndex, uint32_t)> fn) const {
    ForEachUnverifiedBufferBindingIndexImpl(GetLayout(), fn);
}

}  // namespace dawn::native
