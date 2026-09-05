// Copyright 2025 The Dawn & Tint Authors
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

#include "src/dawn/native/ResourceTable.h"

#include <utility>

#include "src/dawn/common/Enumerator.h"
#include "src/dawn/common/MatchVariant.h"
#include "src/dawn/common/Range.h"
#include "src/dawn/native/Buffer.h"
#include "src/dawn/native/Device.h"
#include "src/dawn/native/Queue.h"
#include "src/dawn/native/ResourceTableDefaultResources.h"
#include "src/dawn/native/Texture.h"
#include "src/utils/compiler.h"
#include "src/utils/numeric.h"
#include "tint/tint.h"

namespace dawn::native {

namespace {

// Helper to retrieve a Ref<T> out of a variant. Returns an empty Ref if the variant doesn't hold
// Ref<T>, otherwise it returns the variant value.
template <typename T, typename Variant>
Ref<T> GetRef(Variant&& variant) {
    auto* p = std::get_if<Ref<T>>(&variant);
    if (!p) {
        return {};
    }
    return *p;
}

MaybeError ValidateBindingResource(const DeviceBase* device, const BindingResource* resource) {
    DAWN_INVALID_IF(resource->nextInChain != nullptr, "nextInChain is not null.");

    uint32_t resourceCount = uint32_t{resource->buffer != nullptr} +
                             uint32_t{resource->textureView != nullptr} +
                             uint32_t{resource->sampler != nullptr};
    DAWN_INVALID_IF(resourceCount != 1,
                    "%i resources are specified (when there must be exactly 1).", resourceCount);

    if (resource->buffer != nullptr) {
        // TODO(https://issues.chromium.org/473444515): Support buffers in FullResourceTable.
        return DAWN_VALIDATION_ERROR("Buffers are not supported.");
    } else if (TextureViewBase* view = resource->textureView) {
        // TODO(https://issues.chromium.org/473444515): Support texel buffers in FullResourceTable.
        DAWN_TRY(device->ValidateObject(view));

        Aspect aspect = view->GetAspects();
        DAWN_INVALID_IF(!HasOneBit(aspect),
                        "Multiple aspects (%s) selected in %s. Expected only 1.", aspect, view);

        // TODO(https://issues.chromium.org/473444515): Support storage textures in
        // FullResourceTable
        DAWN_INVALID_IF(
            (view->GetUsage() & kTextureViewOnlyUsages) != wgpu::TextureUsage::TextureBinding,
            "%s's usages (%s) are not exactly %s.", view, view->GetUsage() & kTextureViewOnlyUsages,
            wgpu::TextureUsage::TextureBinding);

        DAWN_INVALID_IF(view->IsYCbCr(), "%s is YCbCr.", view);
    } else if (SamplerBase* sampler = resource->sampler) {
        DAWN_TRY(device->ValidateObject(sampler));
        DAWN_INVALID_IF(sampler->IsYCbCr(), "%s is YCbCr.", sampler);
    } else {
        DAWN_UNREACHABLE();
    }

    return {};
}

}  // anonymous namespace

MaybeError ValidateResourceTableDescriptor(const DeviceBase* device,
                                           const ResourceTableDescriptor* descriptor) {
    DAWN_CHECK(descriptor);

    DAWN_INVALID_IF(!device->HasFeature(Feature::ChromiumExperimentalSamplingResourceTable),
                    "Resource table used without the %s feature enabled.",
                    wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable);

    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain is not nullptr.");

    return {};
}

ResourceTableBase::ResourceTableBase(DeviceBase* device, const ResourceTableDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label), mAPISize(ResourceTableSlot(descriptor->size)) {
    mSlots.resize(mAPISize + ResourceTableDefaultResources::GetCount());
    // This checks that the default SlotState constructor used in the resize operation will
    // initialize with the typeId of an empty slot.
    DAWN_ASSERT(ComputeTypeId({}) == SlotState{}.typeId);

    GetObjectTrackingList()->Track(this);
}

ResourceTableBase::ResourceTableBase(DeviceBase* device,
                                     const ResourceTableDescriptor* descriptor,
                                     ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label) {
    // Create the vector of SlotState even for an error resource table because we need to do state
    // tracking used for the validation of synchronous errors. However skip creating it for tables
    // above the limit because that's a special error case caught on the content-timeline as well.
    if (descriptor->size <= kMaxResourceTableSize) {
        mAPISize = ResourceTableSlot(descriptor->size);
        mSlots.resize(mAPISize);
    } else {
        mDestroyed = true;
    }
}

// static
Ref<ResourceTableBase> ResourceTableBase::MakeError(DeviceBase* device,
                                                    const ResourceTableDescriptor* descriptor) {
    return AcquireRef(new ResourceTableBase(device, descriptor, ObjectBase::kError));
}

ObjectType ResourceTableBase::GetType() const {
    return ObjectType::ResourceTable;
}

ResourceTableSlot ResourceTableBase::GetAPISize() const {
    return mAPISize;
}

ResourceTableSlot ResourceTableBase::GetSizeWithDefaultResources() const {
    return mSlots.size();
}

BufferBase* ResourceTableBase::GetMetadataBuffer() const {
    DAWN_CHECK(!mDestroyed);
    return mMetadataBuffer.Get();
}

bool ResourceTableBase::IsDestroyed() const {
    return mDestroyed;
}

MaybeError ResourceTableBase::ValidateCanUseInSubmitNow() const {
    DAWN_CHECK(!IsError());
    DAWN_INVALID_IF(IsDestroyed(), "%s used while destroyed.", this);
    return {};
}

MaybeError ResourceTableBase::InitializeBase() {
    DeviceBase* device = GetDevice();

    // Create a storage buffer that will hold the shader-visible metadata for the dynamic array.
    uint32_t metadataArrayLength = uint32_t{GetSizeWithDefaultResources()};
    BufferDescriptor metadataDesc{
        .label = "resource table metadata",
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst,
        .size = sizeof(uint32_t) * (metadataArrayLength + 1),
        .mappedAtCreation = true,
    };
    DAWN_TRY_ASSIGN(mMetadataBuffer, device->CreateBuffer(&metadataDesc));

    // Initialize the metadata buffer with the arrayLength and a bunch of zeroes that correspond to
    // empty entries.
    DAWN_CHECK(uint32_t(tint::ResourceType::kEmpty) == 0);
    // TODO(https://crbug.com/435317394): We could rely on zero initialization if it is enabled, and
    // also apply the initial dirty slots in this mapping instead of on the first use of the
    // resource table.
    Span<uint32_t> data = mMetadataBuffer->GetMappedRangeSpan<uint32_t>();
    // Store APISize at element 0 in the metadata buffer, which will be used in the shader to index
    // default resources at APISize + resource type index.
    data[0] = uint32_t{mAPISize};
    std::ranges::fill(data.subspan(1), 0u);
    DAWN_TRY(mMetadataBuffer->Unmap());

    // Add the default resources at the end of the table.
    ResourceTableDefaultResources* defaultResources;
    DAWN_TRY_ASSIGN(defaultResources, device->GetOrCreateResourceTableDefaultsResource());

    for (auto [i, defaultResource] : Enumerate(defaultResources->GetResources())) {
        BindingResource entryContents;
        MatchVariant(
            defaultResource,
            [&](Ref<TextureViewBase> view) { entryContents.textureView = view.Get(); },
            [&](Ref<SamplerBase> sampler) { entryContents.sampler = sampler.Get(); });
        Update(mAPISize + i, &entryContents);
    }

    return {};
}

void ResourceTableBase::DestroyImpl(DestroyReason reason) {
    DAWN_CHECK(!mDestroyed);

    for (auto [texture, _] : mTextureState) {
        texture->RemoveResourceTableUse(this);
    }

    mSlots.clear();
    mDirtySlots.clear();
    mTextureState.clear();
    mDirtyStateTextures.clear();

    if (mMetadataBuffer != nullptr) {
        mMetadataBuffer->Destroy();
        mMetadataBuffer = nullptr;
    }

    mDestroyed = true;
}

void ResourceTableBase::APIDestroy() {
    // Handle error objects directly because Destroy() will skip calling DestroyImpl for them.
    if (IsError()) {
        mSlots.clear();
        mDirtySlots.clear();
        mTextureState.clear();
        mDirtyStateTextures.clear();
        mDestroyed = true;
    } else {
        Destroy();
    }
}

wgpu::Status ResourceTableBase::APIUpdate(uint32_t slotIn, const BindingResource* resource) {
    ResourceTableSlot slot = ResourceTableSlot(slotIn);
    if (!IsValidSlot(slot)) {
        return wgpu::Status::Error;
    }

    // Prevent replacing a slot that may be in use by the GPU.
    if (mSlots[slot].availableAfter > GetDevice()->GetQueue()->GetCompletedCommandSerial()) {
        return wgpu::Status::Error;
    }

    UpdateWithDeviceValidation(slot, resource, "Update");
    return wgpu::Status::Success;
}

uint32_t ResourceTableBase::APIInsert(const BindingResource* resource) {
    if (IsDestroyed()) {
        return wgpu::kInvalidBinding;
    }

    // TODO(https://crbug.com/435317394): This is O(n) in the number of slots. We could make it
    // O(logN) with a heap of the free slots that's maintained over time.
    ExecutionSerial completedSerial = GetDevice()->GetQueue()->GetCompletedCommandSerial();
    for (ResourceTableSlot slot : Range(mAPISize)) {
        if (mSlots[slot].availableAfter > completedSerial) {
            continue;
        }

        UpdateWithDeviceValidation(slot, resource, "Insert");
        return uint32_t{slot};
    }

    // No slot found, return the invalid binding.
    return wgpu::kInvalidBinding;
}

wgpu::Status ResourceTableBase::APIRemove(uint32_t slotIn) {
    ResourceTableSlot slot = ResourceTableSlot(slotIn);
    if (!IsValidSlot(slot)) {
        return wgpu::Status::Error;
    }

    // Always remove the slot, even if a validation error happens, so that we match client-side
    // validation.
    Remove(slot);

    [[maybe_unused]] bool error = GetDevice()->ConsumedError(
        GetDevice()->ValidateObject(this), "validating %s.Remove(%u)", this, slot);
    return wgpu::Status::Success;
}

uint32_t ResourceTableBase::APIGetSize() const {
    return uint32_t{mAPISize};
}

// static
tint::ResourceType ResourceTableBase::ComputeTypeId(
    const std::variant<std::monostate, Ref<TextureViewBase>, Ref<SamplerBase>>& resource) {
    return MatchVariant(
        resource, [&](std::monostate) { return tint::ResourceType::kEmpty; },
        [&](const Ref<SamplerBase>& sampler) {
            if (sampler->IsComparison()) {
                return tint::ResourceType::kSampler_comparison;
            }
            if (sampler->IsFiltering()) {
                return tint::ResourceType::kSampler_filtering;
            }
            return tint::ResourceType::kSampler_non_filtering;
        },
        [&](const Ref<TextureViewBase>& view) {
            const TextureBase* texture = view->GetTexture();
            if (texture->IsMultisampledTexture()) {
                DAWN_CHECK(view->GetDimension() == wgpu::TextureViewDimension::e2D);

                switch (view->GetAspects()) {
                    case Aspect::Color:
                        switch (view->GetFormat().GetAspectInfo(Aspect::Color).baseType) {
                            case TextureComponentType::Float:
                                return tint::ResourceType::kTextureMultisampled2d_f32;
                            case TextureComponentType::Uint:
                                return tint::ResourceType::kTextureMultisampled2d_u32;
                            case TextureComponentType::Sint:
                                return tint::ResourceType::kTextureMultisampled2d_i32;
                            default:
                                DAWN_UNREACHABLE();
                        }

                    case Aspect::Depth:
                        return tint::ResourceType::kTextureDepthMultisampled2d;
                    default:
                        DAWN_UNREACHABLE();
                }
            }

            if (view->GetAspects() == Aspect::Depth) {
                DAWN_CHECK(!texture->IsMultisampledTexture());

                switch (view->GetDimension()) {
                    case wgpu::TextureViewDimension::e2D:
                        return tint::ResourceType::kTextureDepth2d;
                    case wgpu::TextureViewDimension::e2DArray:
                        return tint::ResourceType::kTextureDepth2dArray;
                    case wgpu::TextureViewDimension::Cube:
                        return tint::ResourceType::kTextureDepthCube;
                    case wgpu::TextureViewDimension::CubeArray:
                        return tint::ResourceType::kTextureDepthCubeArray;
                    default:
                        DAWN_UNREACHABLE();
                }
            }

            const auto& aspect_info = view->GetFormat().GetAspectInfo(view->GetAspects());
            bool filterable = (aspect_info.supportedSampleTypes & SampleTypeBit::Float);
            switch (aspect_info.baseType) {
                case TextureComponentType::Float:
                    switch (view->GetDimension()) {
                        case wgpu::TextureViewDimension::e1D: {
                            if (filterable) {
                                return tint::ResourceType::kTexture1d_f32_filterable;
                            }
                            return tint::ResourceType::kTexture1d_f32_unfilterable;
                        }
                        case wgpu::TextureViewDimension::e2D: {
                            if (filterable) {
                                return tint::ResourceType::kTexture2d_f32_filterable;
                            }
                            return tint::ResourceType::kTexture2d_f32_unfilterable;
                        }
                        case wgpu::TextureViewDimension::e2DArray: {
                            if (filterable) {
                                return tint::ResourceType::kTexture2dArray_f32_filterable;
                            }
                            return tint::ResourceType::kTexture2dArray_f32_unfilterable;
                        }
                        case wgpu::TextureViewDimension::Cube: {
                            if (filterable) {
                                return tint::ResourceType::kTextureCube_f32_filterable;
                            }
                            return tint::ResourceType::kTextureCube_f32_unfilterable;
                        }
                        case wgpu::TextureViewDimension::CubeArray: {
                            if (filterable) {
                                return tint::ResourceType::kTextureCubeArray_f32_filterable;
                            }
                            return tint::ResourceType::kTextureCubeArray_f32_unfilterable;
                        }
                        case wgpu::TextureViewDimension::e3D: {
                            if (filterable) {
                                return tint::ResourceType::kTexture3d_f32_filterable;
                            }
                            return tint::ResourceType::kTexture3d_f32_unfilterable;
                        }
                        default:
                            DAWN_UNREACHABLE();
                    }
                case TextureComponentType::Uint:
                    switch (view->GetDimension()) {
                        case wgpu::TextureViewDimension::e1D:
                            return tint::ResourceType::kTexture1d_u32;
                        case wgpu::TextureViewDimension::e2D:
                            return tint::ResourceType::kTexture2d_u32;
                        case wgpu::TextureViewDimension::e2DArray:
                            return tint::ResourceType::kTexture2dArray_u32;
                        case wgpu::TextureViewDimension::Cube:
                            return tint::ResourceType::kTextureCube_u32;
                        case wgpu::TextureViewDimension::CubeArray:
                            return tint::ResourceType::kTextureCubeArray_u32;
                        case wgpu::TextureViewDimension::e3D:
                            return tint::ResourceType::kTexture3d_u32;
                        default:
                            DAWN_UNREACHABLE();
                    }
                case TextureComponentType::Sint:
                    switch (view->GetDimension()) {
                        case wgpu::TextureViewDimension::e1D:
                            return tint::ResourceType::kTexture1d_i32;
                        case wgpu::TextureViewDimension::e2D:
                            return tint::ResourceType::kTexture2d_i32;
                        case wgpu::TextureViewDimension::e2DArray:
                            return tint::ResourceType::kTexture2dArray_i32;
                        case wgpu::TextureViewDimension::Cube:
                            return tint::ResourceType::kTextureCube_i32;
                        case wgpu::TextureViewDimension::CubeArray:
                            return tint::ResourceType::kTextureCubeArray_i32;
                        case wgpu::TextureViewDimension::e3D:
                            return tint::ResourceType::kTexture3d_i32;
                        default:
                            DAWN_UNREACHABLE();
                    }
                default:
                    DAWN_UNREACHABLE();
            }
        });

    DAWN_UNREACHABLE();
}

bool ResourceTableBase::IsValidSlot(ResourceTableSlot slot) const {
    // Some validation is required to return a synchronous error. It needs to be able to run even on
    // error ResourceTables because it must act the same way as an implementation running on top of
    // the wire client-side and doesn't know if objects are errors or not.
    return !mDestroyed && slot < mAPISize;
}

void ResourceTableBase::Update(ResourceTableSlot slot, const BindingResource* contents) {
    DAWN_ASSERT(mSlots[slot].availableAfter <=
                GetDevice()->GetQueue()->GetCompletedCommandSerial());
    DAWN_CHECK(mSlots[slot].typeId == tint::ResourceType::kEmpty);
    mSlots[slot].availableAfter = kMaxExecutionSerial;
    SetEntry(slot, contents);
}

void ResourceTableBase::Remove(ResourceTableSlot slot) {
    // Prevent all accesses to the slot which means it will be possible to update it once all
    // current GPU work is finished.
    mSlots[slot].availableAfter = GetDevice()->GetQueue()->GetLastSubmittedCommandSerial();

    // Set the entry to be empty, which will unlink previously set resources.
    BindingResource nothing = {};
    SetEntry(slot, &nothing);
}

void ResourceTableBase::UpdateWithDeviceValidation(ResourceTableSlot slot,
                                                   const BindingResource* resource,
                                                   std::string_view methodName) {
    // Perform validation that produces a validation error, but unconditionally mark the slot as
    // used since we need to match client-side validation that doesn't perform these checks.
    if (GetDevice()->ConsumedError(  //
            ([&]() -> MaybeError {
                DAWN_TRY(GetDevice()->ValidateObject(this));
                return ValidateBindingResource(GetDevice(), resource);
            })(),
            "validating %s.%s()", this, methodName)) {
        BindingResource nothing = {};
        Update(slot, &nothing);
    } else {
        Update(slot, resource);
    }
}

void ResourceTableBase::SetEntry(ResourceTableSlot slot, const BindingResource* contents) {
    DAWN_CHECK(contents->buffer == nullptr);
    SlotState& state = mSlots[slot];

    // Check the current state. If it's already set to the input value, early out.
    if (auto currView = GetRef<TextureViewBase>(state.resource)) {
        if (currView == contents->textureView) {
            return;
        }

        if (currView != nullptr) {
            // Remove from mTextureState
            TextureBase* currTexture = currView->GetTexture();
            auto& slots = mTextureState[currTexture].slots;
            [[maybe_unused]] const bool erased = slots.erase(slot);
            DAWN_ASSERT(erased);
            // If it's the last slot, remove the entry
            if (slots.empty()) {
                mTextureState.erase(currTexture);
                // Also remove the mapping of texture to this table
                currTexture->RemoveResourceTableUse(this);
            }
        }
    } else if (auto sampler = GetRef<SamplerBase>(state.resource)) {
        if (sampler == contents->sampler) {
            return;
        }
    }

    // Update to new state
    state.resource = {};
    state.visible = false;
    // Note: state.lastResource is only updated by AcquireDirtySlotUpdates

    if (TextureViewBase* inputView = contents->textureView) {
        TextureBase* inputTexture = inputView->GetTexture();
        // Add to mTextureState
        auto [iter, added] = mTextureState.try_emplace(inputTexture);
        TextureState& textureState = iter->second;
        if (added) {
            textureState.visible = !inputTexture->IsDestroyed() && inputTexture->HasAccess();
            // Add the mapping to this table on the texture
            inputTexture->AddResourceTableUse(this);
            // Make it dirty to ensure it gets transitioned
            mDirtyStateTextures.insert(inputTexture);
        }
        [[maybe_unused]] const bool inserted = textureState.slots.insert(slot).second;
        DAWN_ASSERT(inserted);

        state.resource = inputView;
        state.visible = textureState.visible;
    } else if (SamplerBase* sampler = contents->sampler) {
        state.resource = sampler;
    }

    state.typeId = ComputeTypeId(state.resource);
    state.resourceDirty = true;
    MarkStateDirty(slot);
}

MaybeError ResourceTableBase::ApplyDirtySlotUpdatesWith(
    const absl::flat_hash_set<TextureBase*>& writableTextures,
    absl::FunctionRef<ApplyUpdateFn> ApplyUpdate) {
    Updates updates = AcquireDirtySlotUpdates(writableTextures);

    DAWN_TRY(
        ApplyUpdate(updates.metadataUpdates, updates.resourceDiffs, updates.texturesToTransition));

    // The handling of updates.texturesToTransition may cause memory barriers that mark them
    // dirty again, when they have just been put in the correct state. Ignore this fake dirtying
    // by clearing mDirtyStateTextures.
#if defined(DAWN_ENABLE_ASSERTS)
    for (const auto& texture : mDirtyStateTextures) {
        DAWN_ASSERT(updates.texturesToTransition.contains(texture));
    }
#endif
    mDirtyStateTextures = std::move(updates.texturesDirtyAfterUpdate);
    return {};
}

ResourceTableBase::Updates ResourceTableBase::MakeResourcesVisibleExcept(
    const absl::flat_hash_set<TextureBase*>& writableTextures) {
    // This function uses mDirtyStateTextures and writableTextures to figure out visibility for each
    // texture in the table. Along with returning the set of textures to transition, this function
    // also updates the visibility flag in mTextureState so that SetEntry can set the right
    // visibility on newly added textures.
    Updates updates;

    auto HandleDirtyTexture = [&](TextureBase* texture) {
        // The texture may not be in mTextureState if, for example, it was destroyed.
        auto iter = mTextureState.find(texture);
        if (iter == mTextureState.end()) {
            return;
        }
        auto& textureState = iter->second;

        // Update visible flag
        bool visible =
            !texture->IsDestroyed() && texture->HasAccess() && !writableTextures.contains(texture);

        if (visible) {
            updates.texturesToTransition.insert(texture);
        }

        if (textureState.visible != visible) {
            // Used in SetEntry to update textureState.slots with this visibility
            textureState.visible = visible;

            // Update visibility of all slots that contain this texture
            for (auto slot : textureState.slots) {
                SlotState& state = mSlots[slot];
                DAWN_ASSERT(state.visible != visible);
                state.visible = visible;
                MarkStateDirty(slot);
            }
        }
    };

    // First handle the textures that have been dirtied since last draw/dispatch
    for (TextureBase* texture : mDirtyStateTextures) {
        HandleDirtyTexture(texture);
    }
    // Now that we've handled them, clear
    mDirtyStateTextures.clear();

    // Now process writable textures, hiding any that are in the table.
    // We also make sure to add them to texturesDirtyAfterUpdate so that if they're not writable
    // next call, we unhide them.
    for (TextureBase* texture : writableTextures) {
        if (mTextureState.contains(texture)) {
            // We may be recomputing visibility unnecessarily, but it will be possible to optimize,
            // and it will get more complex once we support resource state.
            HandleDirtyTexture(texture);
            updates.texturesDirtyAfterUpdate.insert(texture);
        }
    }

    return updates;
}

ResourceTableBase::Updates ResourceTableBase::AcquireDirtySlotUpdates(
    const absl::flat_hash_set<TextureBase*>& writableTextures) {
    DAWN_CHECK(!mDestroyed);

    Updates updates = MakeResourcesVisibleExcept(writableTextures);

    for (ResourceTableSlot dirtySlot : mDirtySlots) {
        SlotState& state = mSlots[dirtySlot];
        DAWN_CHECK(state.dirty);
        state.dirty = false;

        // Set the value in the table to the type id. If the resource requires pinning, we only set
        // the type id if it's pinned, else we clear it.
        tint::ResourceType effectiveType = state.typeId;
        if (std::holds_alternative<Ref<TextureViewBase>>(state.resource)) {
            effectiveType = state.visible ? state.typeId : tint::ResourceType::kEmpty;
        }

        // Add the update for the metadata buffer.
        // Add 1 because the 0th element holds the table size (APISize).
        size_t offset = sizeof(uint32_t) * (uint32_t(dirtySlot) + 1);
        updates.metadataUpdates.push_back({
            .slot = dirtySlot,
            .offset = uint32_t(offset),
            .data = uint32_t(effectiveType),
        });

        // Compute whether a resource update is needed and skip adding it if unnecessary.
        if (!state.resourceDirty) {
            continue;
        }
        state.resourceDirty = false;

        ResourceDiff update{
            .slot = dirtySlot,
            .removed = state.lastResource,
            .added = state.resource,
        };

        // At least one of removed and added must be set
        DAWN_ASSERT(!(std::holds_alternative<std::monostate>(update.removed) &&
                      std::holds_alternative<std::monostate>(update.added)));

        updates.resourceDiffs.push_back(update);

        // Update the last resource for future slot updates
        state.lastResource = state.resource;
    }

    mDirtySlots.clear();
    return updates;
}

void ResourceTableBase::MarkStateDirty(ResourceTableSlot slot) {
    if (!mSlots[slot].dirty) {
        mDirtySlots.push_back(slot);
        mSlots[slot].dirty = true;
    }
}

void ResourceTableBase::OnTextureStateChange(TextureBase* texture) {
    DAWN_ASSERT(mTextureState.contains(texture));
    mDirtyStateTextures.insert(texture);
}

void ResourceTableBase::OnTextureDestroyed(TextureBase* texture) {
    auto iter = mTextureState.find(texture);
    DAWN_ASSERT(iter != mTextureState.end());
    // Make sure all slots for this texture are hidden
    for (auto slot : iter->second.slots) {
        SlotState& state = mSlots[slot];
        state.resource = std::monostate{};
        state.typeId = tint::ResourceType::kEmpty;
        state.visible = false;
        state.resourceDirty = true;
        MarkStateDirty(slot);
    }
    mTextureState.erase(iter);
}

}  // namespace dawn::native
