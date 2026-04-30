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

#ifndef SRC_DAWN_NATIVE_BINDGROUPLAYOUTINTERNAL_H_
#define SRC_DAWN_NATIVE_BINDGROUPLAYOUTINTERNAL_H_

#include <algorithm>
#include <bitset>
#include <map>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Constants.h"
#include "dawn/common/ContentLessObjectCacheable.h"
#include "dawn/common/Range.h"
#include "dawn/common/SlabAllocator.h"
#include "dawn/common/ityp_span.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/CachedObject.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

ResultOrError<UnpackedPtr<BindGroupLayoutDescriptor>> ValidateBindGroupLayoutDescriptor(
    DeviceBase* device,
    const BindGroupLayoutDescriptor* descriptor,
    bool allowInternalBinding = false);

// In the BindGroupLayout, entries are sorted by type for more efficient lookup and iteration.
// This enum is the order that's used and can also be used to index various ranges of entries.
// The enum is public so that helper function can use it during creation of the BindGroupLayout,
// but the order is not meant to be used anywhere else. Use the accessors on the BindGroupLayout for
// logic that relies on the packing or the order.
enum BindingTypeOrder : uint32_t {
    // Buffers
    BindingTypeOrder_DynamicBuffer,
    BindingTypeOrder_RegularBuffer,
    // Textures
    BindingTypeOrder_SampledTexture,
    BindingTypeOrder_StorageTexture,
    BindingTypeOrder_InputAttachment,
    // Samplers
    BindingTypeOrder_StaticSampler,
    BindingTypeOrder_RegularSampler,
    // Texel Buffers
    BindingTypeOrder_TexelBuffer,
    // Start of entries that are expanded in the frontend and aren't actually stored in the bind
    // groups.
    BindingTypeOrder_ExternalTexture,
    BindingTypeOrder_Count,
};

// BindGroupLayout stores the information passed in the BindGroupLayoutDescriptor but processes it
// in various ways to make it more efficient to use internally and to add internal bindings used to
// implement WebGPU feature that don't exist in backend APIs.
//
// Storing information in hashmap<BindingNumber, T> would be inefficient because these numbers may
// be sparse. Instead the are compacted into vectors, and reordered using |BindingTypeOrder| to make
// it efficient to iterate over all the bindings of a same kind. Indexing the packed bindings is
// done with |BindingIndex| or |APIBindingIndex| (see below for the explanation).
//
// In some cases bindings need to be expanded because a single BGLEntry can match multiple
// BGEntries when bindingArraySize > 1. To make handling more regular, a fake BGLEntry is created
// for each array element such that most code doesn't need to be aware of bindingArraySize.
//
// We also need to have private bindings that cannot be set by the users: ExternalTextures add
// additional bindings for their inner workings which are private to Dawn. Conversely
// ExternalTexture is a pure frontend object and doesn't exist in backends, so dawn::native must
// mostly be unaware about it. This is where |BindingIndex| and |APIBindingIndex| are different:
//
//  - |APIBindingIndex| are user-facing bindings and cannot be used to access private bindings. It
//  is used in code for BindGroup validation and opertations and when reflecting/validating WGSL
//  bind points.
//  - |BindingIndex| are Dawn-facing bindings where ExternalTexture shouldn't be accessed and
//  instead can be used to access internal bindings.
//
// Internally both |APIBindingIndex| and |BindingIndex| are used to access the same vector, but the
// types are used to force uses of different BindGroupLayout accessors that ASSERT the invariant
// above.
class BindGroupLayoutInternalBase : public ApiObjectBase,
                                    public CachedObject,
                                    public ContentLessObjectCacheable<BindGroupLayoutInternalBase> {
  public:
    BindGroupLayoutInternalBase(DeviceBase* device,
                                const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor,
                                ApiObjectBase::UntrackedByDeviceTag tag);
    BindGroupLayoutInternalBase(DeviceBase* device,
                                const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor);
    ~BindGroupLayoutInternalBase() override;

    ObjectType GetType() const override;

    // A map from the BindingNumber to its packed APIBindingIndex.
    using BindingMap = std::map<BindingNumber, APIBindingIndex>;

    // Getters for bindings
    const BindingInfo& GetBindingInfo(BindingIndex bindingIndex) const;
    const BindingInfo& GetAPIBindingInfo(APIBindingIndex bindingIndex) const;
    const BindingMap& GetBindingMap() const;
    BindingIndex AsBindingIndex(APIBindingIndex index) const;
    APIBindingIndex GetAPIBindingIndex(BindingNumber bindingNumber) const;

    // Map used to convert APIBindingIndex to indices in BindGroupBase::GetBoundExternalTextures.
    using BoundExternalTextureMap = absl::flat_hash_map<APIBindingIndex, size_t>;
    const BoundExternalTextureMap& GetBoundExternalTextureMap() const;

    // Returns the number of internal bindings, excluding things like ExternalTexture.
    BindingIndex GetBindingCount() const;
    // Returns |BindingIndex| because dynamic buffers are packed at the front.
    BindingIndex GetDynamicBufferCount() const;
    uint32_t GetDynamicStorageBufferCount() const;
    uint32_t GetUnverifiedBufferCount() const;
    uint32_t GetAPIStaticSamplerCount() const;
    uint32_t GetStaticSamplerCount() const;
    bool IsStorageBufferBinding(BindingIndex bindingIndex) const;
    bool IsExternalTextureBinding(APIBindingIndex bindingIndex) const;

    uint32_t GetExternalTextureCount() const;

    // Returns the exact ranges of indices that contains specific binding types.
    BeginEndRange<BindingIndex> GetDynamicBufferIndices() const;
    BeginEndRange<BindingIndex> GetBufferIndices() const;
    BeginEndRange<BindingIndex> GetStorageTextureIndices() const;
    BeginEndRange<BindingIndex> GetTexelBufferIndices() const;
    BeginEndRange<BindingIndex> GetSampledTextureIndices() const;
    BeginEndRange<BindingIndex> GetTextureIndices() const;
    BeginEndRange<BindingIndex> GetSamplerIndices() const;
    BeginEndRange<BindingIndex> GetStaticSamplerIndices() const;
    BeginEndRange<BindingIndex> GetNonStaticSamplerIndices() const;
    BeginEndRange<BindingIndex> GetInputAttachmentIndices() const;
    BeginEndRange<APIBindingIndex> GetExternalTextureIndices() const;

    // Functions necessary for the unordered_set<BGLBase*>-based cache.
    size_t ComputeContentHash() override;

    struct EqualityFunc {
        bool operator()(const BindGroupLayoutInternalBase* a,
                        const BindGroupLayoutInternalBase* b) const;
    };

    bool IsEmpty() const;
    // Used to get counts and validate them in pipeline layout creation. It might not match the
    // actual number of bindings stored as external textures are expanded. Other getters should be
    // used to get the stored counts.
    const BindingCounts& GetValidationBindingCounts() const;

    // Returns the number of bindings that's expected in the BindGroupDescriptor for BindGroups
    // created from this layout.
    uint32_t GetBindingCountForBindGroupCreation() const;

    bool NeedsCrossBindingValidation() const;

    struct BufferBindingData {
        uint64_t offset;
        uint64_t size;
    };

    struct BindingDataPointers {
        ityp::span<BindingIndex, BufferBindingData> const bufferData = {};
        ityp::span<BindingIndex, Ref<ObjectBase>> const bindings = {};
        ityp::span<uint32_t, uint64_t> const unverifiedBufferSizes = {};
    };

    // Compute the amount of space / alignment required to store bindings for a bind group of
    // this layout.
    size_t GetBindingDataSize() const;
    static constexpr size_t GetBindingDataAlignment() {
        static_assert(alignof(Ref<ObjectBase>) <= alignof(BufferBindingData));
        return alignof(BufferBindingData);
    }

    BindingDataPointers ComputeBindingDataPointers(void* dataStart) const;

    // Returns a detailed string representation of the layout entries for use in error messages.
    std::string EntriesToString() const;

    // Signals it's an appropriate time to free unused memory. BindGroupLayout implementations often
    // have SlabAllocator<BindGroup> that need an external signal.
    virtual void ReduceMemoryUsage();

  protected:
    void DestroyImpl(DestroyReason reason) override;

    template <typename BindGroup>
    SlabAllocator<BindGroup> MakeFrontendBindGroupAllocator(size_t size) {
        return SlabAllocator<BindGroup>(
            size,                                                                        // bytes
            Align(sizeof(BindGroup), GetBindingDataAlignment()) + GetBindingDataSize(),  // size
            std::max(alignof(BindGroup), GetBindingDataAlignment())  // alignment
        );
    }

  private:
    BindGroupLayoutInternalBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    // The entries with arbitrary BindingNumber are repacked into a compact BindingIndex range.
    ityp::vector<BindingIndex, BindingInfo> mBindingInfo;

    // Keep a list of the start indices for each kind of binding. Then (exclusive) end of a range
    // of bindings is the start of the next range. (that's why we use count + 1 entry, to have the
    // "end" of the last binding type)
    BindingIndex GetBindingTypeStart(BindingTypeOrder type) const;
    BindingIndex GetBindingTypeEnd(BindingTypeOrder type) const;
    std::array<BindingIndex, BindingTypeOrder_Count + 1> mBindingTypeStart;

    // Additional counts for types of bindings.
    uint32_t mUnverifiedBufferCount = 0;
    uint32_t mDynamicStorageBufferCount = 0;

    // Map from BindGroupLayoutEntry.binding as BindingNumber to packed indices as BindingIndex.
    // TODO(https://issues.chromium.org/448578977): Use a more optimized map type.
    BindingMap mBindingMap;

    // Map from APIBindingIndex of External Texture to its index in
    // BindGroupBase::mBoundExternalTextures.
    BoundExternalTextureMap mBoundExternalTextureMap;

    BindingCounts mValidationBindingCounts = {};
    bool mNeedsCrossBindingValidation = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BINDGROUPLAYOUT_H_
