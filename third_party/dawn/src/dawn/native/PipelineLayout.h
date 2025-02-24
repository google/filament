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

#ifndef SRC_DAWN_NATIVE_PIPELINELAYOUT_H_
#define SRC_DAWN_NATIVE_PIPELINELAYOUT_H_

#include <array>
#include <bitset>
#include <string>
#include <vector>

#include "dawn/common/ContentLessObjectCacheable.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/CachedObject.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/dawn_platform.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn::native {

ResultOrError<UnpackedPtr<PipelineLayoutDescriptor>> ValidatePipelineLayoutDescriptor(
    DeviceBase*,
    const PipelineLayoutDescriptor* descriptor,
    PipelineCompatibilityToken pipelineCompatibilityToken = kExplicitPCT);

struct StageAndDescriptor {
    StageAndDescriptor(SingleShaderStage shaderStage,
                       ShaderModuleBase* module,
                       StringView entryPoint,
                       size_t constantCount,
                       ConstantEntry const* constants);

    SingleShaderStage shaderStage;
    raw_ptr<ShaderModuleBase> module;
    std::string entryPoint;
    size_t constantCount = 0u;

    // TODO(https://crbug.com/chromium/1521372): Investigate why this is assigned a dangling
    // pointer. Then rewrite it as a raw_ptr<T, AllowPtrArithmetic>.
    RAW_PTR_EXCLUSION ConstantEntry const* constants = nullptr;
};

class PipelineLayoutBase : public ApiObjectBase,
                           public CachedObject,
                           public ContentLessObjectCacheable<PipelineLayoutBase> {
  public:
    PipelineLayoutBase(DeviceBase* device,
                       const UnpackedPtr<PipelineLayoutDescriptor>& descriptor,
                       ApiObjectBase::UntrackedByDeviceTag tag);
    PipelineLayoutBase(DeviceBase* device, const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);
    ~PipelineLayoutBase() override;

    static Ref<PipelineLayoutBase> MakeError(DeviceBase* device, StringView label);
    static ResultOrError<Ref<PipelineLayoutBase>> CreateDefault(
        DeviceBase* device,
        std::vector<StageAndDescriptor> stages,
        bool allowInternalBinding);

    ObjectType GetType() const override;

    const BindGroupLayoutBase* GetFrontendBindGroupLayout(BindGroupIndex group) const;
    BindGroupLayoutBase* GetFrontendBindGroupLayout(BindGroupIndex group);
    const BindGroupLayoutInternalBase* GetBindGroupLayout(BindGroupIndex group) const;
    const BindGroupMask& GetBindGroupLayoutsMask() const;
    bool HasPixelLocalStorage() const;
    const std::vector<wgpu::TextureFormat>& GetStorageAttachmentSlots() const;
    bool HasAnyStorageAttachments() const;
    uint32_t GetNumStorageBufferBindingsInVertexStage() const;
    uint32_t GetNumStorageTextureBindingsInVertexStage() const;
    uint32_t GetNumStorageBufferBindingsInFragmentStage() const;
    uint32_t GetNumStorageTextureBindingsInFragmentStage() const;

    // Utility functions to compute inherited bind groups.
    // Returns the inherited bind groups as a mask.
    BindGroupMask InheritedGroupsMask(const PipelineLayoutBase* other) const;

    // Returns the index of the first incompatible bind group in the range
    // [0, kMaxBindGroups]
    BindGroupIndex GroupsInheritUpTo(const PipelineLayoutBase* other) const;

    // Functions necessary for the unordered_set<PipelineLayoutBase*>-based cache.
    size_t ComputeContentHash() override;

    struct EqualityFunc {
        bool operator()(const PipelineLayoutBase* a, const PipelineLayoutBase* b) const;
    };

    uint32_t GetImmediateDataRangeByteSize() const;

  protected:
    PipelineLayoutBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);
    void DestroyImpl() override;

    PerBindGroup<Ref<BindGroupLayoutBase>> mBindGroupLayouts;
    BindGroupMask mMask;
    bool mHasPLS = false;
    uint32_t mNumStorageBufferBindingsInVertexStage = 0;
    uint32_t mNumStorageTextureBindingsInVertexStage = 0;
    uint32_t mNumStorageBufferBindingsInFragmentStage = 0;
    uint32_t mNumStorageTextureBindingsInFragmentStage = 0;
    std::vector<wgpu::TextureFormat> mStorageAttachmentSlots;
    uint32_t mImmediateDataRangeByteSize = 0;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PIPELINELAYOUT_H_
