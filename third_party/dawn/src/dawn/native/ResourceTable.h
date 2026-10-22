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

#ifndef SRC_DAWN_NATIVE_RESOURCETABLE_H_
#define SRC_DAWN_NATIVE_RESOURCETABLE_H_

#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/functional/function_ref.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/WeakRefSupport.h"
#include "src/dawn/common/ityp_vector.h"
#include "src/dawn/native/Error.h"
#include "src/dawn/native/Forward.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/ObjectBase.h"
#include "src/dawn/native/Sampler.h"
#include "src/dawn/native/dawn_platform.h"

namespace tint {
enum class ResourceType : uint32_t;
}  // namespace tint

namespace dawn::native {

MaybeError ValidateResourceTableDescriptor(const DeviceBase* device,
                                           const ResourceTableDescriptor* descriptor);

// ResourceTableBase implements the frontend tracking for GPUResourceTable, a sparse array of
// heterogeneous resources that can be accessed in shaders. It needs logic for multiple aspects:
//
//  - Its content at a slot can only be updated when known unused, and part of the validation for
//    the mutation needs to run immediately to match what would happen on the WebGPU "content
//    timeline".
//  - Error resource table still need to work for that "content-timeline" validation.
//  - A metadata buffer is kept up to date, that tells the shader-side validation if and how a slot
//    may be accessed. Tint enum values are used since Tint is the place where shader-side
//    validation is implemented..
//  - The updates of resources in slots and of the metadata buffer are batched.
//  - Default resources are inserted at the end of the table, in a way invisible to the
//    application, which means that there are two different sizes for the table (the API visible
//    size and the real one).
class ResourceTableBase : public ApiObjectBase, public WeakRefSupport<ResourceTableBase> {
  public:
    static Ref<ResourceTableBase> MakeError(DeviceBase* device,
                                            const ResourceTableDescriptor* descriptor);

    ObjectType GetType() const override;

    // Return the API visible size that was passed in the descriptor (or 0 for destroyed tables).
    ResourceTableSlot GetAPISize() const;
    // Return the size, taking into account space needed for default resources. Most code outside
    // validation should use this getter for the size.
    ResourceTableSlot GetSizeWithDefaultResources() const;

    BufferBase* GetMetadataBuffer() const;
    bool IsDestroyed() const;
    MaybeError ValidateCanUseInSubmitNow() const;

    // Dawn API
    void APIDestroy();
    wgpu::Status APIUpdate(uint32_t slot, const BindingResource* resource);
    uint32_t APIInsert(const BindingResource* resource);
    wgpu::Status APIRemove(uint32_t slot);
    uint32_t APIGetSize() const;

    // Computes the tint::ResourceType that should be in the metadata buffer for the resource.
    static tint::ResourceType ComputeTypeId(
        const std::variant<std::monostate, Ref<TextureViewBase>, Ref<SamplerBase>>& resource);

    // Notify the table of a texture state change (destroyed, begin/end access)
    void OnTextureStateChange(TextureBase* texture);
    void OnTextureDestroyed(TextureBase* texture);

  protected:
    ResourceTableBase(DeviceBase* device, const ResourceTableDescriptor* descriptor);

    MaybeError InitializeBase();
    void DestroyImpl(DestroyReason reason) override;

    // Methods that mutate the state of resources in the table. They keep track of the necessary
    // metadata buffer updates required for dynamic type checks in the shader to match what's in the
    // table. `contents` can contain no resources, this is useful to mark the slot used even when an
    // error happens, to match what client-side validation would do.
    void Update(ResourceTableSlot slot, const BindingResource* contents);
    void Remove(ResourceTableSlot slot);

    // Performs the steps for wgpu::ResourceTable::Update that are after the validation returning a
    // synchronous error. This is to allow factoring the code between APIUpdate and APIInsert.
    void UpdateWithDeviceValidation(ResourceTableSlot slot,
                                    const BindingResource* resource,
                                    std::string_view methodName);

    // The ApplyUpdateFn takes all the batched updates that need to be applied before uses of the
    // ResourceTable (since the last call to ApplyDirtySlotUpdatesWith or creation of the
    // ResourceTable).
    struct MetadataUpdate {
        ResourceTableSlot slot{0u};  // Slot index to update
        uint32_t offset = 0;        // Byte offset resource array
        uint32_t data = 0;          // tint::ResourceType in the low 16 bits
    };
    struct ResourceDiff {
        using Resource = std::variant<std::monostate, Ref<TextureViewBase>, Ref<SamplerBase>>;
        ResourceTableSlot slot = ResourceTableSlot(0u);
        Resource removed;  // Resource removed from 'slot', if any
        Resource added;    // Resource added to 'slot', if any
    };
    using ApplyUpdateFn = MaybeError(const std::vector<MetadataUpdate>& metadataUpdates,
                                     const std::vector<ResourceDiff>& resourceDiffs,
                                     const absl::flat_hash_set<TextureBase*>& texturesToTransition);

    // Wrapper for the code that handles dirty slot updates in the backend so we can run code right
    // after it that ignores unnecessarily dirty textures added while transitioning them for this
    // table.
    MaybeError ApplyDirtySlotUpdatesWith(const absl::flat_hash_set<TextureBase*>& writableTextures,
                                         absl::FunctionRef<ApplyUpdateFn> ApplyUpdate);

  private:
    ResourceTableBase(DeviceBase* device,
                      const ResourceTableDescriptor* descriptor,
                      ObjectBase::ErrorTag tag);

    bool IsValidSlot(ResourceTableSlot slot) const;

    // Helper method that does the bulk of the shared work between Update and Remove.
    void SetEntry(ResourceTableSlot slot, const BindingResource* contents);

    // Helper method that must be called when anything in the SlotState changes (except
    // `availableAfter`), so that the slot updates are included in the next batch of updates.
    void MarkStateDirty(ResourceTableSlot slot);

    // Functions used to compute all the updates needed to apply the latest state for this table.
    struct Updates {
        std::vector<MetadataUpdate> metadataUpdates;
        std::vector<ResourceDiff> resourceDiffs;
        absl::flat_hash_set<TextureBase*> texturesToTransition;
        absl::flat_hash_set<TextureBase*> texturesDirtyAfterUpdate;
    };
    // Fills only texturesToTransition and texturesDirtyAfterUpdate.
    Updates MakeResourcesVisibleExcept(const absl::flat_hash_set<TextureBase*>& writableTextures);
    // Fills the whole Updates structure.
    Updates AcquireDirtySlotUpdates(const absl::flat_hash_set<TextureBase*>& writableTextures);

    ResourceTableSlot mAPISize = ResourceTableSlot(0u);
    bool mDestroyed = false;

    // Buffer that contains a WGSL metadata struct of the following shape:
    //
    // struct Metadata {
    //     arrayLength: u32,  //  Doesn't include the default resources
    //     slots: array<u32>,  // One entry per slot, including slots for default resources
    // }
    Ref<BufferBase> mMetadataBuffer;

    struct SlotState {
        using Resource = std::variant<std::monostate, Ref<TextureViewBase>, Ref<SamplerBase>>;
        // The last bound resource
        Resource lastResource;
        // The currently bound resource
        Resource resource;

        // Matches the value of the Tint enum for type IDs but kept as u32 to keep usage of Tint
        // headers local.
        tint::ResourceType typeId = tint::ResourceType(0);
        ExecutionSerial availableAfter = kBeginningOfGPUTime;
        bool dirty = false;
        bool resourceDirty = false;  // resourceDirty implies dirty.
        bool visible = false;        // Applies to textures
    };
    ityp::vector<ResourceTableSlot, SlotState> mSlots;

    // The list of slots that need to be updated before the next use of the resource table.
    std::vector<ResourceTableSlot> mDirtySlots;

    // The state of textures in the table
    struct TextureState {
        bool visible = false;
        // std::vector<ResourceTableSlot> slots;
        absl::flat_hash_set<ResourceTableSlot> slots;
    };
    absl::flat_hash_map<TextureBase*, TextureState> mTextureState;

    // Textures that are "dirty" for some reason, e.g. destroyed, access change, r/w change. Handled
    // by MakeResourcesVisibleExcept.
    absl::flat_hash_set<TextureBase*> mDirtyStateTextures;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RESOURCETABLE_H_
