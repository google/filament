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
#include <map>
#include <utility>

#include "absl/container/inlined_vector.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Enumerator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Numeric.h"
#include "dawn/common/Range.h"
#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Device.h"
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

    // Validation for any pixel local storage.
    if (auto* pls = unpacked.Get<PipelineLayoutPixelLocalStorage>()) {
        absl::InlinedVector<StorageAttachmentInfoForValidation, 4> attachments;
        for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
            const PipelineLayoutStorageAttachment& attachment = pls->storageAttachments[i];

            const Format* format;
            DAWN_TRY_ASSIGN_CONTEXT(format, device->GetInternalFormat(attachment.format),
                                    "validating storageAttachments[%i]", i);
            DAWN_INVALID_IF(!format->supportsStorageAttachment,
                            "storageAttachments[%i]'s format (%s) cannot be used with %s.", i,
                            format->format, wgpu::TextureUsage::StorageAttachment);

            attachments.push_back({attachment.offset, attachment.format});
        }

        DAWN_TRY(ValidatePLSInfo(device, pls->totalPixelLocalStorageSize,
                                 {attachments.data(), attachments.size()}));
    }

    DAWN_INVALID_IF(descriptor->bindGroupLayoutCount > kMaxBindGroups,
                    "bindGroupLayoutCount (%i) is larger than the maximum allowed (%i).",
                    descriptor->bindGroupLayoutCount, kMaxBindGroups);

    BindingCounts bindingCounts = {};
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

    DAWN_TRY(ValidateBindingCounts(device->GetLimits(), bindingCounts, device->GetAdapter()));

    // Validate immediateSize.
    if (descriptor->immediateSize) {
        uint32_t maxImmediateSize = device->GetLimits().v1.maxImmediateSize;
        DAWN_INVALID_IF(descriptor->immediateSize > maxImmediateSize,
                        "immediateSize (%i) is larger than the maximum allowed (%i).",
                        descriptor->immediateSize, maxImmediateSize);
    }

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

    BindingCounts bindingCounts = {};
    for (BindGroupIndex i : mMask) {
        AccumulateBindingCounts(
            &bindingCounts,
            mBindGroupLayouts[i]->GetInternalBindGroupLayout()->GetValidationBindingCounts());
    }
    mNumStorageBufferBindingsInVertexStage =
        bindingCounts.perStage[SingleShaderStage::Vertex].storageBufferCount;
    mNumStorageTextureBindingsInVertexStage =
        bindingCounts.perStage[SingleShaderStage::Vertex].storageTextureCount;
    mNumStorageBufferBindingsInFragmentStage =
        bindingCounts.perStage[SingleShaderStage::Fragment].storageBufferCount;
    mNumStorageTextureBindingsInFragmentStage =
        bindingCounts.perStage[SingleShaderStage::Fragment].storageTextureCount;
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

void PipelineLayoutBase::DestroyImpl() {
    Uncache();
}

// static
Ref<PipelineLayoutBase> PipelineLayoutBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new PipelineLayoutBase(device, ObjectBase::kError, label));
}

// static
ResultOrError<Ref<PipelineLayoutBase>> PipelineLayoutBase::CreateDefault(
    DeviceBase* device,
    std::vector<StageAndDescriptor> stages,
    bool allowInternalBinding) {
    using EntryData = BindGroupLayoutEntry;
    using EntryMap = absl::flat_hash_map<BindingNumber, EntryData>;

    // Merges two entries at the same location, if they are allowed to be merged.
    auto MergeEntries = [](EntryData* modifiedEntry, const EntryData& mergedEntry) -> MaybeError {
        // Visibility is excluded because we take the OR across stages.
        bool compatible =
            modifiedEntry->binding == mergedEntry.binding &&
            modifiedEntry->buffer.type == mergedEntry.buffer.type &&
            modifiedEntry->sampler.type == mergedEntry.sampler.type &&
            // Compatibility between these sample types is checked below.
            (modifiedEntry->texture.sampleType != wgpu::TextureSampleType::BindingNotUsed) ==
                (mergedEntry.texture.sampleType != wgpu::TextureSampleType::BindingNotUsed) &&
            modifiedEntry->storageTexture.access == mergedEntry.storageTexture.access;

        // Minimum buffer binding size excluded because we take the maximum seen across stages.
        if (modifiedEntry->buffer.type != wgpu::BufferBindingType::BindingNotUsed) {
            compatible = compatible && modifiedEntry->buffer.hasDynamicOffset ==
                                           mergedEntry.buffer.hasDynamicOffset;
        }

        if (modifiedEntry->texture.sampleType != wgpu::TextureSampleType::BindingNotUsed) {
            // Sample types are compatible if they are exactly equal,
            // or if the |modifiedEntry| is Float and the |mergedEntry| is UnfilterableFloat.
            // Note that the |mergedEntry| never has type Float. Texture bindings all start
            // as UnfilterableFloat and are promoted to Float if they are statically used with
            // a sampler.
            DAWN_ASSERT(mergedEntry.texture.sampleType != wgpu::TextureSampleType::Float);
            bool compatibleSampleTypes =
                modifiedEntry->texture.sampleType == mergedEntry.texture.sampleType ||
                (modifiedEntry->texture.sampleType == wgpu::TextureSampleType::Float &&
                 mergedEntry.texture.sampleType == wgpu::TextureSampleType::UnfilterableFloat);
            compatible =
                compatible && compatibleSampleTypes &&
                modifiedEntry->texture.viewDimension == mergedEntry.texture.viewDimension &&
                modifiedEntry->texture.multisampled == mergedEntry.texture.multisampled;
        }

        if (modifiedEntry->storageTexture.access != wgpu::StorageTextureAccess::BindingNotUsed) {
            compatible =
                compatible &&
                modifiedEntry->storageTexture.format == mergedEntry.storageTexture.format &&
                modifiedEntry->storageTexture.viewDimension ==
                    mergedEntry.storageTexture.viewDimension;
        }

        // Check if any properties are incompatible with existing entry
        // If compatible, we will merge some properties
        // TODO(dawn:563): Improve the error message by doing early-outs when bindings aren't
        // compatible instead of a single check at the end.
        if (!compatible) {
            return DAWN_VALIDATION_ERROR(
                "Duplicate binding in default pipeline layout initialization "
                "not compatible with previous declaration");
        }

        // Use the max |minBufferBindingSize| we find.
        modifiedEntry->buffer.minBindingSize =
            std::max(modifiedEntry->buffer.minBindingSize, mergedEntry.buffer.minBindingSize);

        // Use the OR of all the stages at which we find this binding.
        modifiedEntry->visibility |= mergedEntry.visibility;

        // Size binding_arrays to be the maximum of the required array sizes.
        modifiedEntry->bindingArraySize =
            std::max(modifiedEntry->bindingArraySize, mergedEntry.bindingArraySize);

        return {};
    };

    // Does the trivial conversions from a ShaderBindingInfo to a BindGroupLayoutEntry
    auto ConvertMetadataToEntry =
        [](const ShaderBindingInfo& shaderBinding,
           const ExternalTextureBindingLayout* externalTextureBindingEntry) -> EntryData {
        EntryData entry = {};
        entry.bindingArraySize = uint32_t(shaderBinding.arraySize);

        MatchVariant(
            shaderBinding.bindingInfo,
            [&](const BufferBindingInfo& bindingInfo) {
                entry.buffer.type = bindingInfo.type;
                entry.buffer.minBindingSize = bindingInfo.minBindingSize;
            },
            [&](const SamplerBindingInfo& bindingInfo) { entry.sampler.type = bindingInfo.type; },
            [&](const TextureBindingInfo& bindingInfo) {
                entry.texture.sampleType = bindingInfo.sampleType;
                entry.texture.viewDimension = bindingInfo.viewDimension;
                entry.texture.multisampled = bindingInfo.multisampled;

                // Default to UnfilterableFloat for texture_Nd<f32> as it will be promoted to Float
                // if it is used with a sampler.
                if (entry.texture.sampleType == wgpu::TextureSampleType::Float) {
                    entry.texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
                }
            },
            [&](const StorageTextureBindingInfo& bindingInfo) {
                entry.storageTexture.access = bindingInfo.access;
                entry.storageTexture.format = bindingInfo.format;
                entry.storageTexture.viewDimension = bindingInfo.viewDimension;
            },
            [&](const ExternalTextureBindingInfo&) {
                entry.nextInChain = externalTextureBindingEntry;
            },
            [&](const InputAttachmentBindingInfo& bindingInfo) {
                entry.texture.sampleType = bindingInfo.sampleType;
                entry.texture.viewDimension = kInternalInputAttachmentDim;
            });

        return entry;
    };

    // Creates the BGL from the entries for a stage, checking it is valid.
    auto CreateBGL = [](DeviceBase* device, EntryMap entries,
                        PipelineCompatibilityToken pipelineCompatibilityToken,
                        bool allowInternalBinding) -> ResultOrError<Ref<BindGroupLayoutBase>> {
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

        if (device->IsValidationEnabled()) {
            DAWN_TRY_CONTEXT(ValidateBindGroupLayoutDescriptor(device, &desc, allowInternalBinding),
                             "validating %s", &desc);
        }
        return device->GetOrCreateBindGroupLayout(&desc, pipelineCompatibilityToken);
    };

    DAWN_ASSERT(!stages.empty());

    PipelineCompatibilityToken pipelineCompatibilityToken =
        device->GetNextPipelineCompatibilityToken();

    // Data which BindGroupLayoutDescriptor will point to for creation
    PerBindGroup<EntryMap> entryData = {};

    // External texture binding layouts are chained structs that are set as a pointer within
    // the bind group layout entry. We declare an entry here so that it can be used when needed
    // in each BindGroupLayoutEntry and so it can stay alive until the call to
    // GetOrCreateBindGroupLayout. Because ExternalTextureBindingLayout is an empty struct,
    // there's no issue with using the same struct multiple times.
    ExternalTextureBindingLayout externalTextureBindingLayout;

    uint32_t immediateDataRangeByteSize = 0;

    // Loops over all the reflected BindGroupLayoutEntries from shaders.
    for (const StageAndDescriptor& stage : stages) {
        const EntryPointMetadata& metadata = stage.module->GetEntryPoint(stage.entryPoint);

        // TODO(dawn:1704): Find if we can usefully deduce the PLS for the pipeline layout.
        DAWN_INVALID_IF(
            metadata.usesPixelLocal,
            "Implicit layouts are not supported for entry-points using `pixel_local` blocks.");

        for (auto [group, groupBindings] : Enumerate(metadata.bindings)) {
            for (const auto& [bindingNumber, shaderBinding] : groupBindings) {
                // Create the BindGroupLayoutEntry
                EntryData entry =
                    ConvertMetadataToEntry(shaderBinding, &externalTextureBindingLayout);
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

        // Promote any Unfilterable textures used with a sampler to Filtering.
        for (const EntryPointMetadata::SamplerTexturePair& pair :
             metadata.samplerAndNonSamplerTexturePairs) {
            if (pair.sampler == EntryPointMetadata::nonSamplerBindingPoint) {
                continue;
            }
            BindGroupLayoutEntry* entry = &entryData[pair.texture.group][pair.texture.binding];
            if (entry->texture.sampleType == wgpu::TextureSampleType::UnfilterableFloat) {
                entry->texture.sampleType = wgpu::TextureSampleType::Float;
            }
        }

        // For render pipeline that might has vertex and
        // fragment stages, it is possible that each stage has their own immediate data variable
        // shares the same immediate data block. Pick the max size of immediate data variable from
        // vertex and fragment stage as the pipelineLayout immediate data block size.
        immediateDataRangeByteSize =
            std::max(immediateDataRangeByteSize, metadata.immediateDataRangeByteSize);
    }

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

    Ref<PipelineLayoutBase> result;
    DAWN_TRY_ASSIGN(result, device->CreatePipelineLayout(&desc, pipelineCompatibilityToken));
    DAWN_ASSERT(!result->IsError());

    // That the auto pipeline layout is compatible with the current pipeline.
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
    return GetFrontendBindGroupLayout(group)->GetInternalBindGroupLayout();
}

const BindGroupMask& PipelineLayoutBase::GetBindGroupLayoutsMask() const {
    DAWN_ASSERT(!IsError());
    return mMask;
}

bool PipelineLayoutBase::HasPixelLocalStorage() const {
    return mHasPLS;
}

const std::vector<wgpu::TextureFormat>& PipelineLayoutBase::GetStorageAttachmentSlots() const {
    return mStorageAttachmentSlots;
}

bool PipelineLayoutBase::HasAnyStorageAttachments() const {
    for (auto format : mStorageAttachmentSlots) {
        if (format != wgpu::TextureFormat::Undefined) {
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

    return true;
}

uint32_t PipelineLayoutBase::GetImmediateDataRangeByteSize() const {
    return mImmediateDataRangeByteSize;
}

uint32_t PipelineLayoutBase::GetNumStorageBufferBindingsInVertexStage() const {
    return mNumStorageBufferBindingsInVertexStage;
}

uint32_t PipelineLayoutBase::GetNumStorageTextureBindingsInVertexStage() const {
    return mNumStorageTextureBindingsInVertexStage;
}

uint32_t PipelineLayoutBase::GetNumStorageBufferBindingsInFragmentStage() const {
    return mNumStorageBufferBindingsInFragmentStage;
}

uint32_t PipelineLayoutBase::GetNumStorageTextureBindingsInFragmentStage() const {
    return mNumStorageTextureBindingsInFragmentStage;
}

}  // namespace dawn::native
