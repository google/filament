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
#include "dawn/common/SlabAllocator.h"
#include "dawn/common/ityp_span.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/CachedObject.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {
// TODO(dawn:1082): Minor optimization to use BindingIndex instead of BindingNumber
struct ExternalTextureBindingExpansion {
    BindingNumber plane0;
    BindingNumber plane1;
    BindingNumber params;
};

using ExternalTextureBindingExpansionMap =
    absl::flat_hash_map<BindingNumber, ExternalTextureBindingExpansion>;

MaybeError ValidateBindGroupLayoutDescriptor(DeviceBase* device,
                                             const BindGroupLayoutDescriptor* descriptor,
                                             bool allowInternalBinding = false);

// Bindings are specified as a |BindingNumber| in the BindGroupLayoutDescriptor.
// These numbers may be arbitrary and sparse. Internally, Dawn packs these numbers
// into a packed range of |BindingIndex| integers.
class BindGroupLayoutInternalBase : public ApiObjectBase,
                                    public CachedObject,
                                    public ContentLessObjectCacheable<BindGroupLayoutInternalBase> {
  public:
    BindGroupLayoutInternalBase(DeviceBase* device,
                                const BindGroupLayoutDescriptor* descriptor,
                                ApiObjectBase::UntrackedByDeviceTag tag);
    BindGroupLayoutInternalBase(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor);
    ~BindGroupLayoutInternalBase() override;

    ObjectType GetType() const override;

    // A map from the BindingNumber to its packed BindingIndex.
    using BindingMap = std::map<BindingNumber, BindingIndex>;

    const BindingInfo& GetBindingInfo(BindingIndex bindingIndex) const;
    const BindingMap& GetBindingMap() const;
    bool HasBinding(BindingNumber bindingNumber) const;
    BindingIndex GetBindingIndex(BindingNumber bindingNumber) const;

    // Signals it's an appropriate time to free unused memory. BindGroupLayout implementations often
    // have SlabAllocator<BindGroup> that need an external signal.
    virtual void ReduceMemoryUsage();

    // Functions necessary for the unordered_set<BGLBase*>-based cache.
    size_t ComputeContentHash() override;

    struct EqualityFunc {
        bool operator()(const BindGroupLayoutInternalBase* a,
                        const BindGroupLayoutInternalBase* b) const;
    };

    bool IsEmpty() const;
    BindingIndex GetBindingCount() const;
    // Returns |BindingIndex| because buffers are packed at the front.
    BindingIndex GetBufferCount() const;
    // Returns |BindingIndex| because dynamic buffers are packed at the front.
    BindingIndex GetDynamicBufferCount() const;
    uint32_t GetDynamicStorageBufferCount() const;
    uint32_t GetUnverifiedBufferCount() const;
    uint32_t GetStaticSamplerCount() const;

    // Used to get counts and validate them in pipeline layout creation. It might not match the
    // actual number of bindings stored as external textures are expanded. Other getters should be
    // used to get the stored counts.
    const BindingCounts& GetValidationBindingCounts() const;

    // Used to specify unpacked external texture binding slots when transforming shader modules.
    const ExternalTextureBindingExpansionMap& GetExternalTextureBindingExpansionMap() const;

    uint32_t GetUnexpandedBindingCount() const;

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

    bool IsStorageBufferBinding(BindingIndex bindingIndex) const;

    // Returns a detailed string representation of the layout entries for use in error messages.
    std::string EntriesToString() const;

  protected:
    void DestroyImpl() override;

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

    ityp::vector<BindingIndex, BindingInfo> mBindingInfo;
    // Counts for how many such bindings are in mBindingInfo.
    uint32_t mUnverifiedBufferCount = 0;
    uint32_t mBufferCount = 0;
    uint32_t mDynamicBufferCount = 0;
    uint32_t mDynamicStorageBufferCount = 0;
    uint32_t mStaticSamplerCount = 0;

    // Map from BindGroupLayoutEntry.binding as BindingNumber to packed indices as BindingIndex.
    BindingMap mBindingMap;
    // Map from the BindingNumber of the ExternalTexture to the BindingNumber of the expansion.
    ExternalTextureBindingExpansionMap mExternalTextureBindingExpansionMap;

    BindingCounts mValidationBindingCounts = {};
    bool mNeedsCrossBindingValidation = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BINDGROUPLAYOUT_H_
