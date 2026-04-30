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

#include <algorithm>
#include <limits>
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
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Sampler.h"
#include "dawn/native/TexelBufferView.h"
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
        case kInternalReadOnlyStorageBufferBinding:
            // This is needed for for some workarounds that read a buffer in shaders. The buffer
            // only needs kReadOnlyStorageBuffer usage in this case. Unlike the standard
            // wgpu::BufferBindingType::ReadOnlyStorage which requires the read-write Storage usage.
            // On some backends such as D3D11, using only kReadOnlyStorageBuffer usage could avoid
            // extra allocations.
            requiredUsage = kReadOnlyStorageBuffer;
            requiredBindingAlignment = device->GetLimits().v1.minStorageBufferOffsetAlignment;
            break;
        case wgpu::BufferBindingType::BindingNotUsed:
        case wgpu::BufferBindingType::Undefined:
            DAWN_UNREACHABLE();
    }

    DAWN_INVALID_IF(!IsAligned(static_cast<uint32_t>(entry.offset), requiredBindingAlignment),
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
                            DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter()->GetLimits().v1,
                                                        maxUniformBufferBindingSize, bindingSize));
            break;
        case wgpu::BufferBindingType::Storage:
        case wgpu::BufferBindingType::ReadOnlyStorage:
        case kInternalStorageBufferBinding:
        case kInternalReadOnlyStorageBufferBinding:
            maxStorageBufferBindingSize = device->GetLimits().v1.maxStorageBufferBindingSize;
            DAWN_INVALID_IF(bindingSize > maxStorageBufferBindingSize,
                            "Binding size (%u) of %s is larger than the maximum storage buffer "
                            "binding size (%u).%s",
                            bindingSize, entry.buffer, maxStorageBufferBindingSize,
                            DAWN_INCREASE_LIMIT_MESSAGE(device->GetAdapter()->GetLimits().v1,
                                                        maxStorageBufferBindingSize, bindingSize));
            break;
        case wgpu::BufferBindingType::BindingNotUsed:
        case wgpu::BufferBindingType::Undefined:
            DAWN_UNREACHABLE();
    }

    return {};
}

MaybeError ValidateTextureBindGroupEntry(const DeviceBase* device, const BindGroupEntry& entry) {
    DAWN_INVALID_IF(entry.textureView == nullptr, "Binding entry textureView not set.");

    DAWN_INVALID_IF(entry.sampler != nullptr || entry.buffer != nullptr,
                    "Expected only textureView to be set for binding entry.");

    DAWN_INVALID_IF(entry.nextInChain != nullptr, "nextInChain must be nullptr.");

    TextureViewBase* view = entry.textureView;
    DAWN_TRY(device->ValidateObject(view));

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
    TextureBase* texture = view->GetTexture();

    Aspect aspect = view->GetAspects();
    SampleTypeBit supportedTypes = texture->GetFormat().GetAspectInfo(aspect).supportedSampleTypes;

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

MaybeError ValidateTextureViewBindingUsedAsExternalTexture(DeviceBase* device,
                                                           const BindGroupEntry& entry) {
    // TODO(crbug.com/398752857): Error message should include that entry is neither an
    // ExternalTexture nor a TextureView when the layout contains an ExternalTexture entry.
    DAWN_TRY(ValidateTextureBindGroupEntry(device, entry));

    TextureViewBase* view = entry.textureView;
    TextureBase* texture = view->GetTexture();
    wgpu::TextureFormat format = view->GetFormat().format;

    DAWN_INVALID_IF(
        format != wgpu::TextureFormat::RGBA8Unorm && format != wgpu::TextureFormat::BGRA8Unorm &&
            format != wgpu::TextureFormat::RGBA16Float,
        "%s format (%s) is not %s, %s, or %s.", view, format, wgpu::TextureFormat::RGBA8Unorm,
        wgpu::TextureFormat::BGRA8Unorm, wgpu::TextureFormat::RGBA16Float);

    DAWN_INVALID_IF((view->GetUsage() & wgpu::TextureUsage::TextureBinding) == 0,
                    "%s usage (%s) doesn't include the required usage (%s)", view, view->GetUsage(),
                    wgpu::TextureUsage::TextureBinding);

    DAWN_INVALID_IF(view->GetDimension() != wgpu::TextureViewDimension::e2D,
                    "%s dimension (%s) is not 2D.", view, view->GetDimension());

    DAWN_INVALID_IF(view->GetLevelCount() > 1, "%s mip level count (%u) is not 1.", view,
                    view->GetLevelCount());

    DAWN_INVALID_IF(texture->GetSampleCount() != 1, "%s sample count (%u) is not 1.", texture,
                    texture->GetSampleCount());

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

    // TODO(450506641): Precompute allowed usages of texture views (including swizzle identity
    // check) instead of recomputing.
    DAWN_INVALID_IF(!view->IsSwizzleIdentity(), "Swizzle of %s must be identity.", view);

    if (!device->HasFlexibleTextureViews()) {
        DAWN_TRY(ValidateCompatibilityModeTextureViewArrayLayer(device, view, texture));
    }

    return {};
}

MaybeError ValidateTexelBufferBinding(DeviceBase* device,
                                      const UnpackedPtr<BindGroupEntry>& entry,
                                      const TexelBufferBindingInfo& layout,
                                      UsageValidationMode mode) {
    const TexelBufferBindingEntry* texelBufferEntry = entry.Get<TexelBufferBindingEntry>();
    DAWN_INVALID_IF(texelBufferEntry == nullptr, "Expected a texelBufferView.");

    DAWN_TRY(entry.ValidateSubset<TexelBufferBindingEntry>());
    DAWN_INVALID_IF(
        entry->buffer != nullptr || entry->sampler != nullptr || entry->textureView != nullptr,
        "Expected only texelBufferView to be set for binding entry.");

    DAWN_TRY(device->ValidateObject(texelBufferEntry->texelBufferView));

    BufferBase* buffer = texelBufferEntry->texelBufferView->GetBuffer();
    DAWN_TRY(ValidateCanUseAs(buffer, wgpu::BufferUsage::TexelBuffer));

    DAWN_INVALID_IF(texelBufferEntry->texelBufferView->GetFormat() != layout.format,
                    "Format (%s) of %s expected to be (%s).",
                    texelBufferEntry->texelBufferView->GetFormat(),
                    texelBufferEntry->texelBufferView, layout.format);

    if (layout.access == wgpu::TexelBufferAccess::ReadWrite) {
        DAWN_TRY(ValidateCanUseAs(buffer, wgpu::BufferUsage::Storage));
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

MaybeError ValidateExternalTextureBinding(DeviceBase* device,
                                          const UnpackedPtr<BindGroupEntry>& entry) {
    auto* externalTextureBindingEntry = entry.Get<ExternalTextureBindingEntry>();

    // It is possible to use texture views in lieu of external textures.
    if (externalTextureBindingEntry == nullptr) {
        return ValidateTextureViewBindingUsedAsExternalTexture(device, **entry);
    }

    DAWN_TRY(entry.ValidateSubset<ExternalTextureBindingEntry>());
    DAWN_INVALID_IF(
        entry->sampler != nullptr || entry->textureView != nullptr || entry->buffer != nullptr,
        "Expected only external texture to be set for binding entry.");
    return device->ValidateObject(externalTextureBindingEntry->externalTexture);
}

template <typename F>
void ForEachUnverifiedBufferBindingIndexImpl(const BindGroupLayoutInternalBase* bgl, F&& f) {
    uint32_t packedIndex = 0;
    for (BindingIndex bindingIndex : bgl->GetBufferIndices()) {
        const auto& bufferLayout =
            std::get<BufferBindingInfo>(bgl->GetBindingInfo(bindingIndex).bindingLayout);
        if (bufferLayout.minBindingSize == 0) {
            f(bindingIndex, packedIndex++);
        }
    }
}

MaybeError ValidateStaticSamplersWithSampledTextures(
    const UnpackedPtr<BindGroupDescriptor>& descriptor,
    const BindGroupLayoutInternalBase* layout) {
    // Cache the position of all the sampled texture in descriptor->entries to later validate them
    // against their static sampler (if they are used with the static sampler).
    absl::flat_hash_map<BindingIndex, uint32_t> textureIndexToEntryIndex;
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        APIBindingIndex apiIndex =
            layout->GetBindingMap().at(BindingNumber(descriptor->entries[i].binding));
        const auto& bindingInfo = layout->GetAPIBindingInfo(apiIndex);
        if (std::holds_alternative<TextureBindingInfo>(bindingInfo.bindingLayout)) {
            textureIndexToEntryIndex[layout->AsBindingIndex(apiIndex)] = i;
        }
    }

    // Gather the indices of YCbCr textures sampled by a static sampler.
    ityp::bitset<uint32_t, kMaxBindingsPerPipelineLayout> sampledYcbcrTextures;
    for (BindingIndex samplerIndex : layout->GetStaticSamplerIndices()) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(samplerIndex);
        const auto& staticSamplerLayout =
            std::get<StaticSamplerBindingInfo>(bindingInfo.bindingLayout);
        if (staticSamplerLayout.use != StaticSamplerUse::SingleTextureYCbCr) {
            continue;
        }

        uint32_t textureEntryIndex =
            textureIndexToEntryIndex.at(staticSamplerLayout.sampledTextureIndex);

        const SamplerBase* sampler = staticSamplerLayout.sampler.Get();
        const TextureViewBase* textureView = descriptor->entries[textureEntryIndex].textureView;

        // Compare static sampler and sampled textures to make sure they are compatible.
        if (sampler->IsYCbCr()) {
            DAWN_INVALID_IF(!textureView->IsYCbCr(),
                            "YCbCr static sampler %s at binding (%u) samples a non-YCbCr %s.",
                            sampler, bindingInfo.binding, textureView);

            // YCbCr views can be created without a YCbCrDescriptor but that means they can only be
            // used with ExternalTextures.
            DAWN_INVALID_IF(!textureView->HasYCbCrDescriptor(),
                            "YCbCr static sampler %s at binding (%u) samples a YCbCr %s with "
                            "implicit YCbCr info.",
                            sampler, bindingInfo.binding, textureView);

            // Filterability of YCbCr textures is per-object so we don't check with the sampleType
            // but instead check against the static sampler it will be used with.
            DAWN_INVALID_IF(sampler->IsFiltering() && !textureView->IsYCbCrFilterable(),
                            "YCbCr static sampler %s at binding (%u) is filtering but samples an "
                            "unfilterable YCbCr %s.",
                            sampler, bindingInfo.binding, textureView);

            sampledYcbcrTextures.set(textureEntryIndex);
        } else {
            DAWN_INVALID_IF(textureView->IsYCbCr(),
                            "Non-YCbCr static sampler at binding (%u) samples a YCbCr texture.",
                            bindingInfo.binding);
        }
    }

    // Validate that all YCbCr texture entries are sampled by a static sampler.
    const auto& bindingMap = layout->GetBindingMap();
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];
        const BindingInfo& bindingInfo =
            layout->GetAPIBindingInfo(bindingMap.at(BindingNumber(entry.binding)));
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

ResultOrError<UnpackedPtr<BindGroupDescriptor>> ValidateBindGroupDescriptor(
    DeviceBase* device,
    const BindGroupDescriptor* descriptorChain,
    UsageValidationMode mode) {
    UnpackedPtr<BindGroupDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(descriptorChain));

    DAWN_TRY(device->ValidateObject(descriptor->layout));
    BindGroupLayoutInternalBase* layout = descriptor->layout->GetInternalBindGroupLayout();
    const BindGroupLayoutInternalBase::BindingMap& bindingMap = layout->GetBindingMap();

    // Validate individual entries.
    bool needsCrossBindingValidation = layout->NeedsCrossBindingValidation();
    // TODO(https://issues.chromium.org/448578977): Use a more optimized type as 1000 bits on the
    // stack is a bit much.
    ityp::bitset<BindingNumber, kMaxBindingsPerBindGroup> bindingsSet;
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];
        BindingNumber binding = BindingNumber(entry.binding);

        // Check that the entry exists in the BGL and get its info.
        const auto& it = bindingMap.find(binding);
        DAWN_INVALID_IF(it == bindingMap.end(),
                        "In entries[%u], binding index %u not present in the bind group layout."
                        "\nExpected layout: %s",
                        i, binding, layout->EntriesToString());
        const BindingInfo& bindingInfo = layout->GetAPIBindingInfo(it->second);

        // Check for redundant entries.
        DAWN_INVALID_IF(bindingsSet[binding],
                        "In entries[%u], binding index %u already used by a previous entry", i,
                        binding);
        bindingsSet.set(binding);

        // Below this block we validate entries based on the bind group layout, in which
        // external textures have been expanded into their underlying contents. For this reason
        // we must identify external texture binding entries by checking the bind group entry
        // itself.
        // TODO(42240282): Store external textures in
        // BindGroupLayoutBase::BindingDataPointers::bindings so checking external textures can
        // be moved in the switch below.
        UnpackedPtr<BindGroupEntry> unpacked;
        DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(&entry));

        // Perform binding-type specific validation.
        DAWN_TRY_CONTEXT(
            MatchVariant(
                bindingInfo.bindingLayout,
                [&](const BufferBindingInfo& layout) -> MaybeError {
                    // TODO(dawn:1485): Validate buffer binding with usage validation mode.
                    return ValidateBufferBinding(device, entry, layout);
                },
                [&](const TextureBindingInfo& layout) -> MaybeError {
                    DAWN_TRY(ValidateSampledTextureBinding(device, entry, layout, mode));

                    if (entry.textureView->IsYCbCr()) {
                        // Need to validate that the YCbCr texture is statically sampled.
                        needsCrossBindingValidation = true;
                    }
                    return {};
                },
                [&](const StorageTextureBindingInfo& layout) -> MaybeError {
                    return ValidateStorageTextureBinding(device, entry, layout, mode);
                },
                [&](const TexelBufferBindingInfo& layout) -> MaybeError {
                    return ValidateTexelBufferBinding(device, unpacked, layout, mode);
                },
                [&](const SamplerBindingInfo& layout) -> MaybeError {
                    return ValidateSamplerBinding(device, entry, layout);
                },
                [&](const StaticSamplerBindingInfo& layout) -> MaybeError {
                    return DAWN_VALIDATION_ERROR("An entry is provided for a static sampler.");
                },
                [&](const ExternalTextureBindingInfo& layout) -> MaybeError {
                    return ValidateExternalTextureBinding(device, unpacked);
                },

                [](const InputAttachmentBindingInfo&) -> MaybeError {
                    // Internal use only. No validation.
                    return {};
                }),
            "validating entries[%u] against %s.", i, bindingInfo);
    }

    // Check that we have all the required entries.
    const uint32_t expectedEntryCount = layout->GetBindingCountForBindGroupCreation();

    DAWN_INVALID_IF(
        descriptor->entryCount != expectedEntryCount,
        "Number of entries (%u) did not match the expected number of entries (%u) for %s."
        "\nExpected layout: %s",
        descriptor->entryCount, expectedEntryCount, layout, layout->EntriesToString());

    // This should always be true because
    //  - numBindings has to match between the bind group and its layout.
    //  - Each binding must be set at most once
    //
    // We don't validate the equality because it wouldn't be possible to cover it with a test.
    DAWN_ASSERT(bindingsSet.count() == expectedEntryCount);

    if (needsCrossBindingValidation) {
        // This additional validation is only needed when there are static samplers used with a
        // single texture binding and/or there are YCbCr textures.
        DAWN_TRY(ValidateStaticSamplersWithSampledTextures(descriptor, layout));
    }

    return descriptor;
}

// BindGroup

BindGroupBase::BindGroupBase(DeviceBase* device,
                             const UnpackedPtr<BindGroupDescriptor>& descriptor,
                             void* bindingDataStart)
    : ApiObjectBase(device, descriptor->label),
      mLayout(descriptor->layout),
      mBindingData(GetLayout()->ComputeBindingDataPointers(bindingDataStart)) {
    GetObjectTrackingList()->Track(this);
}

MaybeError BindGroupBase::Initialize(const UnpackedPtr<BindGroupDescriptor>& descriptor) {
    BindGroupLayoutInternalBase* layout = GetLayout();

    for (BindingIndex i{0}; i < layout->GetBindingCount(); ++i) {
        // TODO(enga): Shouldn't be needed when bindings are tightly packed.
        // This is to fill Ref<ObjectBase> holes with nullptrs.
        new (&mBindingData.bindings[i]) Ref<ObjectBase>();
    }

    // Fill mBoundTextures with nullptr since these binding entries can possibly be bound without an
    // external texture.
    mBoundExternalTextures.resize(layout->GetExternalTextureCount(), nullptr);

    // Gather bindings.
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        UnpackedPtr<BindGroupEntry> entry = Unpack(&descriptor->entries[i]);
        BindingNumber binding = BindingNumber(entry->binding);
        APIBindingIndex apiBindingIndex = layout->GetAPIBindingIndex(binding);

        DAWN_TRY(MatchVariant(
            layout->GetAPIBindingInfo(apiBindingIndex).bindingLayout,
            [&](const BufferBindingInfo&) -> MaybeError {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
                mBindingData.bindings[bindingIndex] = entry->buffer;
                mBindingData.bufferData[bindingIndex].offset = entry->offset;
                uint64_t bufferSize = (entry->size == wgpu::kWholeSize)
                                          ? entry->buffer->GetSize() - entry->offset
                                          : entry->size;
                mBindingData.bufferData[bindingIndex].size = bufferSize;
                return {};
            },

            [&](const TextureBindingInfo&) -> MaybeError {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
                mBindingData.bindings[bindingIndex] = entry->textureView;
                return {};
            },
            [&](const StorageTextureBindingInfo&) -> MaybeError {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
                mBindingData.bindings[bindingIndex] = entry->textureView;
                return {};
            },
            [&](const InputAttachmentBindingInfo&) -> MaybeError {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
                mBindingData.bindings[bindingIndex] = entry->textureView;
                return {};
            },
            [&](const SamplerBindingInfo&) -> MaybeError {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
                mBindingData.bindings[bindingIndex] = entry->sampler;
                return {};
            },
            [&](const TexelBufferBindingInfo&) -> MaybeError {
                BindingIndex bindingIndex = layout->AsBindingIndex(apiBindingIndex);
                DAWN_ASSERT(mBindingData.bindings[bindingIndex] == nullptr);
                auto* texelBufferBindingEntry = entry.Get<TexelBufferBindingEntry>();
                mBindingData.bindings[bindingIndex] = texelBufferBindingEntry->texelBufferView;
                return {};
            },

            [&](const ExternalTextureBindingInfo& info) -> MaybeError {
                // Here we unpack external texture bindings into multiple additional bindings for
                // the external texture's contents. New binding locations previously determined in
                // the bind group layout are created in this bind group and filled with the external
                // texture's underlying resources.
                if (auto* externalTextureBindingEntry = entry.Get<ExternalTextureBindingEntry>()) {
                    const auto& externalTextureMap = layout->GetBoundExternalTextureMap();
                    mBoundExternalTextures[externalTextureMap.at(apiBindingIndex)] =
                        externalTextureBindingEntry->externalTexture;

                    DAWN_ASSERT(mBindingData.bindings[info.plane0] == nullptr);
                    mBindingData.bindings[info.plane0] =
                        externalTextureBindingEntry->externalTexture->GetTextureViews()[0];

                    DAWN_ASSERT(mBindingData.bindings[info.plane1] == nullptr);
                    mBindingData.bindings[info.plane1] =
                        externalTextureBindingEntry->externalTexture->GetTextureViews()[1];

                    DAWN_ASSERT(mBindingData.bindings[info.metadata] == nullptr);
                    mBindingData.bindings[info.metadata] =
                        externalTextureBindingEntry->externalTexture->GetParamsBuffer();
                    mBindingData.bufferData[info.metadata].offset = 0;
                    mBindingData.bufferData[info.metadata].size =
                        sizeof(dawn::native::ExternalTextureParams);
                    return {};
                }

                // If this is for a texture view that is used as an external texture, we need to
                // also provide placeholder for the second plane and a parameter buffer.
                DAWN_ASSERT(entry->textureView != nullptr);

                DAWN_ASSERT(mBindingData.bindings[info.plane0] == nullptr);
                mBindingData.bindings[info.plane0] = entry->textureView;

                DAWN_ASSERT(mBindingData.bindings[info.plane1] == nullptr);
                DAWN_TRY_ASSIGN(mBindingData.bindings[info.plane1],
                                GetDevice()->GetOrCreatePlaceholderTextureViewForExternalTexture());

                DAWN_ASSERT(mBindingData.bindings[info.metadata] == nullptr);
                Ref<BufferBase> paramsBuffer;
                DAWN_TRY_ASSIGN(paramsBuffer,
                                MakeParamsBufferForSimpleView(GetDevice(), entry->textureView));
                mBindingData.bindings[info.metadata] = paramsBuffer;
                mBindingData.bufferData[info.metadata].offset = 0;
                mBindingData.bufferData[info.metadata].size = paramsBuffer->GetSize();
                return {};
            },

            [](const StaticSamplerBindingInfo&) -> MaybeError { DAWN_UNREACHABLE(); }));
    }

    ForEachUnverifiedBufferBindingIndexImpl(layout,
                                            [&](BindingIndex bindingIndex, uint32_t packedIndex) {
                                                mBindingData.unverifiedBufferSizes[packedIndex] =
                                                    mBindingData.bufferData[bindingIndex].size;
                                            });

    DAWN_TRY(InitializeImpl());

    return {};
}

BindGroupBase::~BindGroupBase() = default;

void BindGroupBase::DestroyImpl(DestroyReason reason) {
    if (mLayout != nullptr) {
        DAWN_ASSERT(!IsError());
        for (BindingIndex i{0}; i < GetLayout()->GetBindingCount(); ++i) {
            mBindingData.bindings[i].~Ref<ObjectBase>();
        }
    }
}

BindGroupBase::BindGroupBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ApiObjectBase(device, tag, label), mBindingData() {}

// static
Ref<BindGroupBase> BindGroupBase::MakeError(DeviceBase* device, StringView label) {
    class ErrorBindGroupBase final : public BindGroupBase {
      public:
        explicit ErrorBindGroupBase(DeviceBase* device, StringView label)
            : BindGroupBase(device, ObjectBase::kError, label) {}
        MaybeError InitializeImpl() override { DAWN_UNREACHABLE(); }
    };

    return AcquireRef(new ErrorBindGroupBase(device, label));
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
    DAWN_ASSERT(mLayout != nullptr);
    return mLayout->GetInternalBindGroupLayout();
}

const BindGroupLayoutInternalBase* BindGroupBase::GetLayout() const {
    DAWN_ASSERT(mLayout != nullptr);
    return mLayout->GetInternalBindGroupLayout();
}

const ityp::span<uint32_t, uint64_t>& BindGroupBase::GetUnverifiedBufferSizes() const {
    DAWN_ASSERT(!IsError());
    return mBindingData.unverifiedBufferSizes;
}

BufferBase* BindGroupBase::GetBindingAsBuffer(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_ASSERT(bindingIndex < layout->GetBindingCount());
    DAWN_ASSERT(std::holds_alternative<BufferBindingInfo>(
        layout->GetBindingInfo(bindingIndex).bindingLayout));
    return static_cast<BufferBase*>(mBindingData.bindings[bindingIndex].Get());
}

SamplerBase* BindGroupBase::GetBindingAsSampler(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_ASSERT(bindingIndex < layout->GetBindingCount());
    DAWN_ASSERT(std::holds_alternative<SamplerBindingInfo>(
        layout->GetBindingInfo(bindingIndex).bindingLayout));
    return static_cast<SamplerBase*>(mBindingData.bindings[bindingIndex].Get());
}

TextureViewBase* BindGroupBase::GetBindingAsTextureView(BindingIndex bindingIndex) const {
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

BufferBinding BindGroupBase::GetBindingAsBufferBinding(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    return {GetBindingAsBuffer(bindingIndex), mBindingData.bufferData[bindingIndex].offset,
            mBindingData.bufferData[bindingIndex].size};
}

TexelBufferViewBase* BindGroupBase::GetBindingAsTexelBufferView(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutInternalBase* layout = GetLayout();
    DAWN_ASSERT(bindingIndex < layout->GetBindingCount());
    DAWN_ASSERT(std::holds_alternative<TexelBufferBindingInfo>(
        layout->GetBindingInfo(bindingIndex).bindingLayout));
    return static_cast<TexelBufferViewBase*>(mBindingData.bindings[bindingIndex].Get());
}

const std::vector<Ref<ExternalTextureBase>>& BindGroupBase::GetBoundExternalTextures() const {
    DAWN_ASSERT(!IsError());
    return mBoundExternalTextures;
}

Ref<ExternalTextureBase> BindGroupBase::GetBoundExternalTexture(
    APIBindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(GetLayout()->GetBoundExternalTextureMap().count(bindingIndex) == 1);

    size_t etIndex = GetLayout()->GetBoundExternalTextureMap().at(bindingIndex);
    return mBoundExternalTextures[etIndex];
}

void BindGroupBase::ForEachUnverifiedBufferBindingIndex(
    std::function<void(BindingIndex, uint32_t)> fn) const {
    ForEachUnverifiedBufferBindingIndexImpl(GetLayout(), fn);
}

}  // namespace dawn::native
