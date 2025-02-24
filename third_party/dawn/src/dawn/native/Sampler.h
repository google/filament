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

#ifndef SRC_DAWN_NATIVE_SAMPLER_H_
#define SRC_DAWN_NATIVE_SAMPLER_H_

#include "dawn/common/ContentLessObjectCacheable.h"
#include "dawn/native/CachedObject.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;

MaybeError ValidateSamplerDescriptor(DeviceBase* device, const SamplerDescriptor* descriptor);

class SamplerBase : public ApiObjectBase,
                    public CachedObject,
                    public ContentLessObjectCacheable<SamplerBase> {
  public:
    SamplerBase(DeviceBase* device,
                const SamplerDescriptor* descriptor,
                ApiObjectBase::UntrackedByDeviceTag tag);
    SamplerBase(DeviceBase* device, const SamplerDescriptor* descriptor);
    ~SamplerBase() override;

    static Ref<SamplerBase> MakeError(DeviceBase* device, StringView label);

    ObjectType GetType() const override;

    bool IsComparison() const;
    bool IsFiltering() const;
    bool IsYCbCr() const;
    // Valid to call only if `IsYCbCr()` is true.
    YCbCrVkDescriptor GetYCbCrVkDescriptor() const;

    // Functions necessary for the unordered_set<SamplerBase*>-based cache.
    size_t ComputeContentHash() override;

    struct EqualityFunc {
        bool operator()(const SamplerBase* a, const SamplerBase* b) const;
    };

    uint16_t GetMaxAnisotropy() const { return mMaxAnisotropy; }

  protected:
    void DestroyImpl() override;

  private:
    SamplerBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label);

    // TODO(cwallez@chromium.org): Store a crypto hash of the items instead?
    wgpu::AddressMode mAddressModeU;
    wgpu::AddressMode mAddressModeV;
    wgpu::AddressMode mAddressModeW;
    wgpu::FilterMode mMagFilter;
    wgpu::FilterMode mMinFilter;
    wgpu::MipmapFilterMode mMipmapFilter;
    float mLodMinClamp;
    float mLodMaxClamp;
    wgpu::CompareFunction mCompareFunction;
    uint16_t mMaxAnisotropy;
    bool mIsYCbCr = false;
    YCbCrVkDescriptor mYCbCrVkDescriptor = {};
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SAMPLER_H_
