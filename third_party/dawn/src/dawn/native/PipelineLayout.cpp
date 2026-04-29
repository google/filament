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

#include "dawn/native/PipelineLayout.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "absl/container/inlined_vector.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Math.h"
#include "dawn/common/Range.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/ShaderModule.h"

namespace dawn::native {

ResultOrError<UnpackedPtr<PipelineLayoutDescriptor>> ValidatePipelineLayoutDescriptor(
    DeviceBase* device,
    const PipelineLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken) {
    UnpackedPtr<PipelineLayoutDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    // A binding count that will be updated as we validate the various parts ot the pipeline layout.
    BindingCounts bindingCounts{};

    // Validation for any pixel local storage.
    if (auto* pls = unpacked.Get<PipelineLayoutPixelLocalStorage>()) {
        absl::InlinedVector<StorageAttachmentInfoForValidation, 4> attachments;
        for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
            const PipelineLayoutStorageAttachment& attachment = pls->storageAttachments[i];

            const Format* format;
            DAWN_TRY_ASSIGN_CONTEXT(format, device->GetInternalFormat(attachment.format),
                                    "validating storageAttachments[%i]", i);
            DAWN_INVALID_IF(!format->SupportsStorageAttachment(),
                            "storageAttachments[%i]'s format (%s) cannot be used with %s.", i,
                            format->format, wgpu::TextureUsage::StorageAttachment);

            attachments.push_back({attachment.offset, attachment.format});
        }

        DAWN_TRY(ValidatePLSInfo(device, pls->totalPixelLocalStorageSize,
                                 {attachments.data(), attachments.size()}));
    }

    // Validation for the resource table, if any.
    bool usesResourceTable = false;
    if (auto* rt = unpacked.Get<PipelineLayoutResourceTable>()) {
        DAWN_INVALID_IF(rt->usesResourceTable &&
                            !device->HasFeature(Feature::ChromiumExperimentalSamplingResourceTable),
                        "Resource table used without the %s feature enabled.",
                        wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable);
        usesResourceTable = rt->usesResourceTable;

        // Add to the limits the storage buffer that will be used for the availability data of the
        // resource table. Set a minimum binding size so as to not increment unverifiedBufferCount.
        BindGroupLayoutEntry availabilityEntry{
            .binding = 0,
            .visibility = kAllStages,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::ReadOnlyStorage,
                    .minBindingSize = 4,
                },
        };
        IncrementBindingCounts(&bindingCounts, Unpack(&availabilityEntry));
    }

    // Validation for the bind group layouts.
    if (usesResourceTable) {
        DAWN_INVALID_IF(descriptor->bindGroupLayoutCount + 1 > kMaxBindGroups,
                        "bindGroupLayoutCount (%i) + 1 for the resource table is larger than the "
                        "maximum allowed (%i).",
                        descriptor->bindGroupLayoutCount, kMaxBindGroups);
    } else {
        DAWN_INVALID_IF(descriptor->bindGroupLayoutCount > kMaxBindGroups,
                        "bindGroupLayoutCount (%i) is larger than the maximum allowed (%i).",
                        descriptor->bindGroupLayoutCount, kMaxBindGroups);
    }

    for (uint32_t i = 0; i < descriptor->bindGroupLayoutCount; ++i) {
        if (descriptor->bindGroupLayouts[i] == nullptr) {
            continue;
        }

        DAWN_TRY(device->ValidateObject(descriptor->bindGroupLayouts[i]));
        DAWN_INVALID_IF(descriptor->bindGroupLayouts[i]->GetPipelineCompatibilityToken() !=
                            pipelineCompatibilityToken,
                        "bindGroupLayouts[%i] (%s) is used to create a pipeline layout but it was "
                        "created as part of a pipeline's default layout.",
                        i, descriptor->bindGroupLayouts[i]);

        AccumulateBindingCounts(&bindingCounts, descriptor->bindGroupLayouts[i]
                                                    ->GetInternalBindGroupLayout()
                                                    ->GetValidationBindingCounts());
    }

    // Validate immediateSize.
    if (descriptor->immediateSize) {
        DAWN_INVALID_IF(!device->GetInstance()->HasFeature(
                            wgpu::WGSLLanguageFeatureName::ImmediateAddressSpace),
                        "ImmediateAddressSpace feature is not enabled");
        DAWN_INVALID_IF(!IsAligned(descriptor->immediateSize, kImmediateConstantElementByteSize),
                        "immediateSize (%i) is not a multiple of %i bytes.",
                        descriptor->immediateSize, kImmediateConstantElementByteSize);
        uint32_t maxImmediateSize = device->GetLimits().v1.maxImmediateSize;
        DAWN_INVALID_IF(descriptor->immediateSize > maxImmediateSize,
                        "immediateSize (%i) is larger than the maximum allowed (%i).",
                        descriptor->immediateSize, maxImmediateSize);
    }

    DAWN_TRY(ValidateBindingCounts(device->GetLimits(), bindingCounts, device->GetAdapter()));
    return unpacked;
}

StageAndDescriptor::StageAndDescriptor(SingleShaderStage shaderStage,
                                       ShaderModuleBase* module,
                                       StringView entryPoint,
                                       size_t constantCount,
                                       ConstantEntry const* constants)
    : shaderStage(shaderStage),
      module(module),
      entryPoint(module->ReifyEntryPointName(entryPoint, shaderStage).name),
      constantCount(constantCount),
      constants(constants) {}

// PipelineLayoutBase

PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                       const UnpackedPtr<PipelineLayoutDescriptor>& descriptor,
                                       ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label),
      mImmediateDataRangeByteSize(descriptor->immediateSize) {
    DAWN_ASSERT(descriptor->bindGroupLayoutCount <= kMaxBindGroups);

    // According to WebGPU SPEC of CreatePipelineLayout(), if bindGroupLayouts[i] is null or
    // bindGroupLayouts[i].[[descriptor]].entries is empty, treat bindGroupLayouts[i] as an
    // empty bind group layout. So here unspecified or null bind group layouts can be set to
    // `device->GetEmptyBindGroupLayout()`.
    mBindGroupLayouts.fill(device->GetEmptyBindGroupLayout());
    auto bgls = ityp::SpanFromUntyped<BindGroupIndex>(descriptor->bindGroupLayouts,
                                                      descriptor->bindGroupLayoutCount);
    for (auto [group, bgl] : Enumerate(bgls)) {
        // Keep the default empty bind group layouts for nullptr bind group layouts
        if (bgl == nullptr) {
            continue;
        }

        // Set the bind group layout even if it is empty to copy over the empty bind group layouts
        // that have a pipeline compatibility token.
        mBindGroupLayouts[group] = bgl;
        mMask.set(group, !bgl->IsEmpty());
    }

    // Gather the PLS information.
    if (auto* pls = descriptor.Get<PipelineLayoutPixelLocalStorage>()) {
        mHasPLS = true;
        mStorageAttachmentSlots = std::vector<wgpu::TextureFormat>(
            pls->totalPixelLocalStorageSize / kPLSSlotByteSize, wgpu::TextureFormat::Undefined);
        for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
            size_t slot = pls->storageAttachments[i].offset / kPLSSlotByteSize;
            mStorageAttachmentSlots[slot] = pls->storageAttachments[i].format;
        }
    }
    // Gather the resource table information.
    if (auto* rt = descriptor.Get<PipelineLayoutResourceTable>()) {
        mUsesResourceTable = rt->usesResourceTable;
    }
}

PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                       const UnpackedPtr<PipelineLayoutDescriptor>& descriptor)
    : PipelineLayoutBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

PipelineLayoutBase::PipelineLayoutBase(DeviceBase* device,
                                       ObjectBase::ErrorTag tag,
                                       StringView label)
    : ApiObjectBase(device, tag, label) {}

PipelineLayoutBase::~PipelineLayoutBase() = default;

void PipelineLayoutBase::DestroyImpl(DestroyReason reason) {
    Uncache();
}

// static
Ref<PipelineLayoutBase> PipelineLayoutBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new PipelineLayoutBase(device, ObjectBase::kError, label));
}

namespace {

// Helper function used to merge multiple TextureSampleTypes for the same binding together.
ResultOrError<wgpu::TextureSampleType> MostSpecificSampleTypeIfCompatible(
    wgpu::TextureSampleType a,
    wgpu::TextureSampleType b) {
    if (a == b) {
        return a;
    }

    // If a binding is UnknownFilterableFloat then the other one is more specific (the case where it
    // is also UnknownFilterableFloat is handled above and it keeps the same value as it is "as
    // specific").
    if (a == kUnknownFilterableFloatSampleType &&
        (b == wgpu::TextureSampleType::UnfilterableFloat || b == wgpu::TextureSampleType::Float)) {
        return b;
    }
    if (b == kUnknownFilterableFloatSampleType &&
        (a == wgpu::TextureSampleType::UnfilterableFloat || a == wgpu::TextureSampleType::Float)) {
        return a;
    }

    return DAWN_VALIDATION_ERROR("Texture sample types are not compatible (%s vs %s).", a, b);
}

// Helper function used to merge multiple SamplerBindingType for the same binding together.
ResultOrError<wgpu::SamplerBindingType> MostSpecificSamplerTypeIfCompatible(
    wgpu::SamplerBindingType a,
    wgpu::SamplerBindingType b) {
    if (a == b) {
        return a;
    }

    // If a binding is UnknownFiltering then the other one is more specific (the case where it is
    // also UnknownFiltering is handled above and it keeps the same value as it is "as specific").
    if (a == kUnknownFilteringSamplerBindingType &&
        (b == wgpu::SamplerBindingType::Filtering || b == wgpu::SamplerBindingType::NonFiltering)) {
        return b;
    }
    if (b == kUnknownFilteringSamplerBindingType &&
        (a == wgpu::SamplerBindingType::Filtering || a == wgpu::SamplerBindingType::NonFiltering)) {
        return b;
    }

    return DAWN_VALIDATION_ERROR("Sampler binding types are not compatible (%s vs %s).", a, b);
}

// Merges two entries at the same location, if they are allowed to be merged.
MaybeError MergeEntries(BindGroupLayoutEntry* modifiedEntry,
                        const BindGroupLayoutEntry& mergedEntry) {
    DAWN_ASSERT(modifiedEntry->binding == mergedEntry.binding);

    BindingInfoType modifiedType = GetBindingInfoType(modifiedEntry);
    BindingInfoType mergedType = GetBindingInfoType(&mergedEntry);
    DAWN_INVALID_IF(modifiedType != mergedType, "Binding types differ (%s vs %s).", modifiedType,
                    mergedType);

    // Use the OR of all the stages at which we find this binding.
    modifiedEntry->visibility |= mergedEntry.visibility;

    // Size binding_arrays to be the maximum of the required array sizes.
    modifiedEntry->bindingArraySize =
        std::max(modifiedEntry->bindingArraySize, mergedEntry.bindingArraySize);

    switch (mergedType) {
        case BindingInfoType::Buffer:
            DAWN_INVALID_IF(modifiedEntry->buffer.type != mergedEntry.buffer.type,
                            "Buffer binding types differs (%s vs. %s).", modifiedEntry->buffer.type,
                            mergedEntry.buffer.type);
            DAWN_INVALID_IF(
                modifiedEntry->buffer.hasDynamicOffset != mergedEntry.buffer.hasDynamicOffset,
                "Buffer dynamic offsets differs (%v vs. %v).",
                modifiedEntry->buffer.hasDynamicOffset, mergedEntry.buffer.hasDynamicOffset);

            // Use the max |minBufferBindingSize| we find.
            modifiedEntry->buffer.minBindingSize =
                std::max(modifiedEntry->buffer.minBindingSize, mergedEntry.buffer.minBindingSize);
            break;

        case BindingInfoType::Texture: {
            DAWN_INVALID_IF(
                modifiedEntry->texture.viewDimension != mergedEntry.texture.viewDimension,
                "Texture dimensions differs (%s vs. %s).", modifiedEntry->texture.viewDimension,
                mergedEntry.texture.viewDimension);
            DAWN_INVALID_IF(modifiedEntry->texture.multisampled != mergedEntry.texture.multisampled,
                            "Texture multisampled differs (%v vs. %v).",
                            modifiedEntry->texture.multisampled, mergedEntry.texture.multisampled);

            DAWN_TRY_ASSIGN(modifiedEntry->texture.sampleType,
                            MostSpecificSampleTypeIfCompatible(modifiedEntry->texture.sampleType,
                                                               mergedEntry.texture.sampleType));
            break;
        }

        case BindingInfoType::StorageTexture:
            DAWN_INVALID_IF(
                modifiedEntry->storageTexture.access != mergedEntry.storageTexture.access,
                "Storage texture accesses differs (%s vs. %s).",
                modifiedEntry->storageTexture.access, mergedEntry.storageTexture.access);
            DAWN_INVALID_IF(
                modifiedEntry->storageTexture.format != mergedEntry.storageTexture.format,
                "Storage texture formats differs (%s vs. %s).",
                modifiedEntry->storageTexture.format, mergedEntry.storageTexture.format);
            DAWN_INVALID_IF(modifiedEntry->storageTexture.viewDimension !=
                                mergedEntry.storageTexture.viewDimension,
                            "Storage texture dimensions differs (%s vs. %s).",
                            modifiedEntry->storageTexture.viewDimension,
                            mergedEntry.storageTexture.viewDimension);
            break;

        case BindingInfoType::Sampler:
            DAWN_TRY_ASSIGN(modifiedEntry->sampler.type,
                            MostSpecificSamplerTypeIfCompatible(modifiedEntry->sampler.type,
                                                                mergedEntry.sampler.type));
            break;

        case BindingInfoType::ExternalTexture:
            // Nothing to check or merge.
            break;

        // Types that cannot be defaulted (yet?)
        case BindingInfoType::StaticSampler:
        case BindingInfoType::TexelBuffer:
        case BindingInfoType::InputAttachment:
            DAWN_UNREACHABLE();
    }

    return {};
}

BindGroupLayoutEntry ConvertMetadataToEntry(
    std::vector<std::unique_ptr<wgpu::TexelBufferBindingLayout>>& texelBufferLayouts,
    const ShaderBindingInfo& shaderBinding,
    const ExternalTextureBindingLayout* externalTextureBindingEntry) {
    BindGroupLayoutEntry entry = {};
    entry.bindingArraySize = uint32_t(shaderBinding.arraySize);

    MatchVariant(
        shaderBinding.bindingInfo,
        [&](const BufferBindingInfo& bindingInfo) {
            entry.buffer.type = bindingInfo.type;
            entry.buffer.minBindingSize = bindingInfo.minBindingSize;
        },
        [&](const SamplerBindingInfo& bindingInfo) {
            entry.sampler.type = bindingInfo.type;
        },
        [&](const TextureBindingInfo& bindingInfo) {
            entry.texture.sampleType = bindingInfo.sampleType;
            entry.texture.viewDimension = bindingInfo.viewDimension;
            entry.texture.multisampled = bindingInfo.multisampled;
        },
        [&](const StorageTextureBindingInfo& bindingInfo) {
            entry.storageTexture.access = bindingInfo.access;
            entry.storageTexture.format = bindingInfo.format;
            entry.storageTexture.viewDimension = bindingInfo.viewDimension;
        },
        [&](const TexelBufferBindingInfo& bindingInfo) {
            auto layout = std::make_unique<wgpu::TexelBufferBindingLayout>();
            layout->format = bindingInfo.format;
            layout->access = bindingInfo.access;
            texelBufferLayouts.push_back(std::move(layout));
            entry.nextInChain = texelBufferLayouts.back().get();
        },
        [&](const ExternalTextureBindingInfo&) { entry.nextInChain = externalTextureBindingEntry; },
        [&](const InputAttachmentBindingInfo& bindingInfo) {
            entry.texture.sampleType = bindingInfo.sampleType;
            entry.texture.viewDimension = kInternalInputAttachmentDim;
        });

    return entry;
}

// Creates the BGL from the entries for a stage, checking it is valid.
ResultOrError<Ref<BindGroupLayoutBase>> CreateBGL(
    DeviceBase* device,
    absl::flat_hash_map<BindingNumber, BindGroupLayoutEntry> entries,
    PipelineCompatibilityToken pipelineCompatibilityToken,
    bool allowInternalBinding) {
    // Put all the values from the map in a vector
    std::vector<BindGroupLayoutEntry> entryVec;
    entryVec.reserve(entries.size());
    for (auto& [_, entry] : entries) {
        entryVec.push_back(entry);
    }

    // Create and validate the BGL
    BindGroupLayoutDescriptor desc = {};
    desc.entries = entryVec.data();
    desc.entryCount = entryVec.size();

    UnpackedPtr<BindGroupLayoutDescriptor> unpacked;
    if (device->IsValidationEnabled()) {
        DAWN_TRY_ASSIGN_CONTEXT(
            unpacked, ValidateBindGroupLayoutDescriptor(device, &desc, allowInternalBinding),
            "validating %s", &desc);
    } else {
        unpacked = Unpack(&desc);
    }
    return device->GetOrCreateBindGroupLayout(unpacked, pipelineCompatibilityToken);
}

// Resolves all the samplers with type kUnknownFilteringSamplerBindingType and all textures with
// sample type kUnknownFilterableFloatSampleType to concrete values.
void ResolveUnknownTypes(
    const std::vector<StageAndDescriptor>& stages,
    PerBindGroup<absl::flat_hash_map<BindingNumber, BindGroupLayoutEntry>>* entryData) {
    // Handle the constraint where an unknown sampler used with a non-filterable texture
    // (unfilterable-float, sint or uint) must be non-filtering. Note that unknown textures used
    // with samplers can only be changed to filterable floats in the rest of the resolving, so no
    // new constraints on samplers will be created after this.
    for (const StageAndDescriptor& stage : stages) {
        for (const auto& pair :
             stage.module->GetEntryPoint(stage.entryPoint).samplerAndNonSamplerTexturePairs) {
            if (pair.sampler == EntryPointMetadata::nonSamplerBindingPoint) {
                continue;
            }

            BindGroupLayoutEntry* s = &entryData->at(pair.sampler.group)[pair.sampler.binding];
            BindGroupLayoutEntry* t = &entryData->at(pair.texture.group)[pair.texture.binding];
            if (s->sampler.type != kUnknownFilteringSamplerBindingType) {
                continue;
            }

            // Pairs can reference external textures, they are always filterable.
            if (t->texture.sampleType == wgpu::TextureSampleType::BindingNotUsed) {
                DAWN_ASSERT(t->nextInChain != nullptr &&
                            t->nextInChain->sType == wgpu::SType::ExternalTextureBindingLayout);
                continue;
            }

            if (t->texture.sampleType != wgpu::TextureSampleType::Float &&
                t->texture.sampleType != kUnknownFilterableFloatSampleType) {
                s->sampler.type = wgpu::SamplerBindingType::NonFiltering;
            }
        }
    }
    // All the other unknown samplers have no specific constraints and are made filtering as that's
    // the least constraining for samplers that can be put in BindGroups.
    for (const StageAndDescriptor& stage : stages) {
        const EntryPointMetadata& metadata = stage.module->GetEntryPoint(stage.entryPoint);

        for (auto [group, groupBindings] : Enumerate(metadata.bindings)) {
            for (const auto& [bindingNumber, shaderBinding] : groupBindings) {
                BindGroupLayoutEntry* entry = &entryData->at(group)[bindingNumber];
                if (entry->sampler.type == kUnknownFilteringSamplerBindingType) {
                    entry->sampler.type = wgpu::SamplerBindingType::Filtering;
                }
            }
        }
    }

    // Handle the constraint where an unknown texture used with a filtering sampler must be a
    // filterable float.
    for (const StageAndDescriptor& stage : stages) {
        for (const auto& pair :
             stage.module->GetEntryPoint(stage.entryPoint).samplerAndNonSamplerTexturePairs) {
            if (pair.sampler == EntryPointMetadata::nonSamplerBindingPoint) {
                continue;
            }

            BindGroupLayoutEntry* s = &entryData->at(pair.sampler.group)[pair.sampler.binding];
            BindGroupLayoutEntry* t = &entryData->at(pair.texture.group)[pair.texture.binding];

            // Pairs can reference external textures, skip handling them.
            if (t->texture.sampleType == wgpu::TextureSampleType::BindingNotUsed) {
                DAWN_ASSERT(t->nextInChain != nullptr &&
                            t->nextInChain->sType == wgpu::SType::ExternalTextureBindingLayout);
                continue;
            }

            DAWN_ASSERT(s->sampler.type != kUnknownFilteringSamplerBindingType);
            if (t->texture.sampleType == kUnknownFilterableFloatSampleType &&
                s->sampler.type == wgpu::SamplerBindingType::Filtering) {
                t->texture.sampleType = wgpu::TextureSampleType::Float;
            }
        }
    }

    // All the other unknown textures have no specific constraints and are made unfilterable as
    // that's the least constraining for textures that can be put in BindGroups.
    for (const StageAndDescriptor& stage : stages) {
        const EntryPointMetadata& metadata = stage.module->GetEntryPoint(stage.entryPoint);

        for (auto [group, groupBindings] : Enumerate(metadata.bindings)) {
            for (const auto& [bindingNumber, shaderBinding] : groupBindings) {
                BindGroupLayoutEntry* entry = &entryData->at(group)[bindingNumber];
                if (entry->texture.sampleType == kUnknownFilterableFloatSampleType) {
                    entry->texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
                }
            }
        }
    }
}

}  // namespace

// static
ResultOrError<Ref<PipelineLayoutBase>> PipelineLayoutBase::CreateDefault(
    DeviceBase* device,
    std::vector<StageAndDescriptor> stages,
    bool allowInternalBinding) {
    DAWN_ASSERT(!stages.empty());

    // Does the trivial conversions from a ShaderBindingInfo to a BindGroupLayoutEntry
    std::vector<std::unique_ptr<wgpu::TexelBufferBindingLayout>> texelBufferLayouts;

    PipelineCompatibilityToken pipelineCompatibilityToken =
        device->GetNextPipelineCompatibilityToken();

    // Data which BindGroupLayoutDescriptor will point to for creation
    PerBindGroup<absl::flat_hash_map<BindingNumber, BindGroupLayoutEntry>> entryData = {};

    // External texture binding layouts are chained structs that are set as a pointer within
    // the bind group layout entry. We declare an entry here so that it can be used when needed
    // in each BindGroupLayoutEntry and so it can stay alive until the call to
    // GetOrCreateBindGroupLayout. Because ExternalTextureBindingLayout is an empty struct,
    // there's no issue with using the same struct multiple times.
    ExternalTextureBindingLayout externalTextureBindingLayout;

    bool usesResourceTable = false;
    uint32_t immediateDataRangeByteSize = 0;

    // Loops over all the reflected BindGroupLayoutEntries from shaders.
    for (const StageAndDescriptor& stage : stages) {
        const EntryPointMetadata& metadata = stage.module->GetEntryPoint(stage.entryPoint);

        // Check if at least one stage uses a resource table
        if (metadata.usesResourceTable) {
            usesResourceTable = true;
        }

        // TODO(dawn:1704): Find if we can usefully deduce the PLS for the pipeline layout.
        DAWN_INVALID_IF(
            metadata.usesPixelLocal,
            "Implicit layouts are not supported for entry-points using `pixel_local` blocks.");

        for (auto [group, groupBindings] : Enumerate(metadata.bindings)) {
            for (const auto& [bindingNumber, shaderBinding] : groupBindings) {
                // Create the BindGroupLayoutEntry
                BindGroupLayoutEntry entry = ConvertMetadataToEntry(
                    texelBufferLayouts, shaderBinding, &externalTextureBindingLayout);
                entry.binding = uint32_t(bindingNumber);
                entry.visibility = StageBit(stage.shaderStage);

                // Add it to our map of all entries, if there is an existing entry, then we
                // need to merge, if we can.
                const auto& [existingEntry, inserted] =
                    entryData[group].insert({bindingNumber, entry});
                if (!inserted) {
                    DAWN_TRY_CONTEXT(MergeEntries(&existingEntry->second, entry),
                                     "merging implicit bindings for @group(%u) @binding(%u).",
                                     group, bindingNumber);
                }
            }
        }

        // For render pipeline that might has vertex and fragment stages, it is possible that each
        // stage has their own immediate data variable shares the same immediate data block. Pick
        // the max size of immediate data variable from vertex and fragment stage as the
        // pipelineLayout immediate data block size.
        immediateDataRangeByteSize =
            std::max(immediateDataRangeByteSize, metadata.immediateDataRangeByteSize);
    }

    // Some sampler and texture bindings are created with an unknown sampler type / texture sample
    // type and must be resolved to concrete types based on which texture/sampler pairs are
    // statically used.
    ResolveUnknownTypes(stages, &entryData);

    // Create the bind group layouts, including the empty ones as all the bind group layouts should
    // be created with `pipelineCompatibilityToken` whether they are empty or not.
    PerBindGroup<Ref<BindGroupLayoutBase>> bindGroupLayouts = {};
    for (auto group : Range(kMaxBindGroupsTyped)) {
        DAWN_TRY_ASSIGN(bindGroupLayouts[group],
                        CreateBGL(device, std::move(entryData[group]), pipelineCompatibilityToken,
                                  allowInternalBinding));
    }

    // Create the deduced pipeline layout, validating if it is valid.
    PerBindGroup<BindGroupLayoutBase*> bgls = {};
    for (auto group : Range(kMaxBindGroupsTyped)) {
        bgls[group] = bindGroupLayouts[group].Get();
    }

    PipelineLayoutDescriptor desc = {};
    desc.bindGroupLayouts = bgls.data();
    desc.bindGroupLayoutCount = static_cast<uint32_t>(kMaxBindGroupsTyped);
    desc.immediateSize = immediateDataRangeByteSize;

    PipelineLayoutResourceTable resourceTable;
    if (usesResourceTable) {
        resourceTable.usesResourceTable = true;
        resourceTable.nextInChain = desc.nextInChain;
        desc.nextInChain = &resourceTable;

        // The resource table uses one BGL entry, so remove the last one, only if it's empty, to
        // make room for it. If it's not empty, this means kMaxBindGroups were referenced in the
        // shader, which will trigger a validation error in CreatePipelineLayout that too many BGLs
        // are used with the resource table.
        if (desc.bindGroupLayouts[desc.bindGroupLayoutCount - 1]->IsEmpty()) {
            desc.bindGroupLayoutCount--;
        }
    }

    Ref<PipelineLayoutBase> result;
    DAWN_TRY_ASSIGN(result, device->CreatePipelineLayout(&desc, pipelineCompatibilityToken));
    DAWN_ASSERT(!result->IsError());

    // Validate that the auto pipeline layout is compatible with the current pipeline.
    // Note: the currently specified rules can generate invalid default layouts.
    // Hopefully the spec will be updated to prevent this.
    // See: https://github.com/gpuweb/gpuweb/issues/4952
    for (const StageAndDescriptor& stage : stages) {
        const EntryPointMetadata& metadata = stage.module->GetEntryPoint(stage.entryPoint);
        DAWN_TRY(ValidateCompatibilityWithPipelineLayout(device, metadata, result.Get()));
    }

    return std::move(result);
}

ObjectType PipelineLayoutBase::GetType() const {
    return ObjectType::PipelineLayout;
}

const BindGroupLayoutBase* PipelineLayoutBase::GetFrontendBindGroupLayout(
    BindGroupIndex group) const {
    DAWN_ASSERT(!IsError());
    const BindGroupLayoutBase* bgl = mBindGroupLayouts[group].Get();
    DAWN_ASSERT(bgl != nullptr);
    return bgl;
}

BindGroupLayoutBase* PipelineLayoutBase::GetFrontendBindGroupLayout(BindGroupIndex group) {
    DAWN_ASSERT(!IsError());
    BindGroupLayoutBase* bgl = mBindGroupLayouts[group].Get();
    DAWN_ASSERT(bgl != nullptr);
    return bgl;
}

const BindGroupLayoutInternalBase* PipelineLayoutBase::GetBindGroupLayout(
    BindGroupIndex group) const {
    DAWN_ASSERT(!IsError());
    return GetFrontendBindGroupLayout(group)->GetInternalBindGroupLayout();
}

BindGroupLayoutInternalBase* PipelineLayoutBase::GetBindGroupLayout(BindGroupIndex group) {
    DAWN_ASSERT(!IsError());
    return GetFrontendBindGroupLayout(group)->GetInternalBindGroupLayout();
}

const BindGroupMask& PipelineLayoutBase::GetBindGroupLayoutsMask() const {
    DAWN_ASSERT(!IsError());
    return mMask;
}

bool PipelineLayoutBase::HasPixelLocalStorage() const {
    DAWN_ASSERT(!IsError());
    return mHasPLS;
}

const std::vector<wgpu::TextureFormat>& PipelineLayoutBase::GetStorageAttachmentSlots() const {
    DAWN_ASSERT(!IsError());
    return mStorageAttachmentSlots;
}

bool PipelineLayoutBase::HasAnyStorageAttachments() const {
    DAWN_ASSERT(!IsError());

    for (auto format : mStorageAttachmentSlots) {
        if (format != wgpu::TextureFormat::Undefined) {
            return true;
        }
    }
    return false;
}

bool PipelineLayoutBase::HasExternalTextures() const {
    DAWN_ASSERT(!IsError());

    for (BindGroupIndex g : mMask) {
        if (mBindGroupLayouts[g]->GetInternalBindGroupLayout()->GetExternalTextureCount() != 0) {
            return true;
        }
    }
    return false;
}

bool PipelineLayoutBase::HasAPIStaticSamplers() const {
    DAWN_ASSERT(!IsError());

    for (BindGroupIndex g : mMask) {
        if (mBindGroupLayouts[g]->GetInternalBindGroupLayout()->GetAPIStaticSamplerCount() != 0) {
            return true;
        }
    }
    return false;
}

BindGroupMask PipelineLayoutBase::InheritedGroupsMask(const PipelineLayoutBase* other) const {
    DAWN_ASSERT(!IsError());
    return {(1 << static_cast<uint32_t>(GroupsInheritUpTo(other))) - 1u};
}

BindGroupIndex PipelineLayoutBase::GroupsInheritUpTo(const PipelineLayoutBase* other) const {
    DAWN_ASSERT(!IsError());

    for (BindGroupIndex i(0); i < kMaxBindGroupsTyped; ++i) {
        if (!mMask[i] || mBindGroupLayouts[i].Get() != other->mBindGroupLayouts[i].Get()) {
            return i;
        }
    }
    return kMaxBindGroupsTyped;
}

size_t PipelineLayoutBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    recorder.Record(mMask);

    for (BindGroupIndex group : mMask) {
        recorder.Record(GetBindGroupLayout(group)->GetContentHash());
    }

    // Hash the PLS state
    recorder.Record(mHasPLS);
    for (wgpu::TextureFormat slotFormat : mStorageAttachmentSlots) {
        recorder.Record(slotFormat);
    }

    // Hash the immediate data range byte size
    recorder.Record(mImmediateDataRangeByteSize);

    // Hash the resource table state
    recorder.Record(mUsesResourceTable);

    return recorder.GetContentHash();
}

bool PipelineLayoutBase::EqualityFunc::operator()(const PipelineLayoutBase* a,
                                                  const PipelineLayoutBase* b) const {
    if (a->mMask != b->mMask) {
        return false;
    }

    for (BindGroupIndex group : a->mMask) {
        if (a->GetBindGroupLayout(group) != b->GetBindGroupLayout(group)) {
            return false;
        }
    }

    // Check PLS
    if (a->mHasPLS != b->mHasPLS) {
        return false;
    }
    if (a->mStorageAttachmentSlots.size() != b->mStorageAttachmentSlots.size()) {
        return false;
    }
    for (size_t i = 0; i < a->mStorageAttachmentSlots.size(); i++) {
        if (a->mStorageAttachmentSlots[i] != b->mStorageAttachmentSlots[i]) {
            return false;
        }
    }

    // Check immediate data range
    if (a->mImmediateDataRangeByteSize != b->mImmediateDataRangeByteSize) {
        return false;
    }

    // Check resource table
    if (a->mUsesResourceTable != b->mUsesResourceTable) {
        return false;
    }

    return true;
}

uint32_t PipelineLayoutBase::GetImmediateDataRangeByteSize() const {
    DAWN_ASSERT(!IsError());
    return mImmediateDataRangeByteSize;
}

bool PipelineLayoutBase::UsesResourceTable() const {
    DAWN_ASSERT(!IsError());
    return mUsesResourceTable;
}

}  // namespace dawn::native
