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

#include "dawn/native/Sampler.h"

#include <cmath>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

MaybeError ValidateSamplerDescriptor(DeviceBase* device, const SamplerDescriptor* descriptor) {
    DAWN_INVALID_IF(std::isnan(descriptor->lodMinClamp) || std::isnan(descriptor->lodMaxClamp),
                    "LOD clamp bounds [%f, %f] contain a NaN.", descriptor->lodMinClamp,
                    descriptor->lodMaxClamp);

    DAWN_INVALID_IF(descriptor->lodMinClamp < 0 || descriptor->lodMaxClamp < 0,
                    "LOD clamp bounds [%f, %f] contain contain a negative number.",
                    descriptor->lodMinClamp, descriptor->lodMaxClamp);

    DAWN_INVALID_IF(descriptor->lodMinClamp > descriptor->lodMaxClamp,
                    "LOD min clamp (%f) is larger than the max clamp (%f).",
                    descriptor->lodMinClamp, descriptor->lodMaxClamp);

    if (descriptor->maxAnisotropy > 1) {
        DAWN_INVALID_IF(descriptor->minFilter != wgpu::FilterMode::Linear ||
                            descriptor->magFilter != wgpu::FilterMode::Linear ||
                            descriptor->mipmapFilter != wgpu::MipmapFilterMode::Linear,
                        "One of minFilter (%s), magFilter (%s) or mipmapFilter (%s) is not %s "
                        "while using anisotropic filter (maxAnisotropy is %f)",
                        descriptor->minFilter, descriptor->magFilter, descriptor->mipmapFilter,
                        wgpu::FilterMode::Linear, descriptor->maxAnisotropy);
    } else if (descriptor->maxAnisotropy == 0u) {
        return DAWN_VALIDATION_ERROR("Max anisotropy (%f) is less than 1.",
                                     descriptor->maxAnisotropy);
    }

    DAWN_TRY(ValidateFilterMode(descriptor->minFilter));
    DAWN_TRY(ValidateFilterMode(descriptor->magFilter));
    DAWN_TRY(ValidateMipmapFilterMode(descriptor->mipmapFilter));
    DAWN_TRY(ValidateAddressMode(descriptor->addressModeU));
    DAWN_TRY(ValidateAddressMode(descriptor->addressModeV));
    DAWN_TRY(ValidateAddressMode(descriptor->addressModeW));
    DAWN_TRY(ValidateCompareFunction(descriptor->compare));

    UnpackedPtr<SamplerDescriptor> unpacked = Unpack(descriptor);
    if (unpacked.Get<YCbCrVkDescriptor>()) {
        DAWN_INVALID_IF(!device->HasFeature(Feature::YCbCrVulkanSamplers), "%s is not enabled.",
                        wgpu::FeatureName::YCbCrVulkanSamplers);
    }

    return {};
}

// SamplerBase

SamplerBase::SamplerBase(DeviceBase* device,
                         const SamplerDescriptor* descriptor,
                         ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label),
      mAddressModeU(descriptor->addressModeU),
      mAddressModeV(descriptor->addressModeV),
      mAddressModeW(descriptor->addressModeW),
      mMagFilter(descriptor->magFilter),
      mMinFilter(descriptor->minFilter),
      mMipmapFilter(descriptor->mipmapFilter),
      mLodMinClamp(descriptor->lodMinClamp),
      mLodMaxClamp(descriptor->lodMaxClamp),
      mCompareFunction(descriptor->compare),
      mMaxAnisotropy(descriptor->maxAnisotropy) {
    if (auto* yCbCrVkDescriptor = Unpack(descriptor).Get<YCbCrVkDescriptor>()) {
        mIsYCbCr = true;
        mYCbCrVkDescriptor = *yCbCrVkDescriptor;
        mYCbCrVkDescriptor.nextInChain = nullptr;
    }
}

SamplerBase::SamplerBase(DeviceBase* device, const SamplerDescriptor* descriptor)
    : SamplerBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

SamplerBase::SamplerBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ApiObjectBase(device, tag, label) {}

SamplerBase::~SamplerBase() = default;

void SamplerBase::DestroyImpl() {
    Uncache();
}

// static
Ref<SamplerBase> SamplerBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new SamplerBase(device, ObjectBase::kError, label));
}

ObjectType SamplerBase::GetType() const {
    return ObjectType::Sampler;
}

bool SamplerBase::IsComparison() const {
    return mCompareFunction != wgpu::CompareFunction::Undefined;
}

bool SamplerBase::IsFiltering() const {
    return mMinFilter == wgpu::FilterMode::Linear || mMagFilter == wgpu::FilterMode::Linear ||
           mMipmapFilter == wgpu::MipmapFilterMode::Linear;
}

bool SamplerBase::IsYCbCr() const {
    return mIsYCbCr;
}

YCbCrVkDescriptor SamplerBase::GetYCbCrVkDescriptor() const {
    DAWN_ASSERT(IsYCbCr());
    return mYCbCrVkDescriptor;
}

size_t SamplerBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    // NOTE: We always hash the state of `mYCbCrVkDescriptor` to avoid splitting
    // this code into two separate Record() calls, which would be error-prone
    // when future state is added. If the client did not pass in a YCbCr
    // descriptor, `mIsYCbCr` will be false and the YCbCr descriptor will have
    // default values. The use of `mIsYCbCr` here differentiates that case from
    // the case of the client passing in a YCbCr descriptor holding all default
    // values.
    recorder.Record(
        mAddressModeU, mAddressModeV, mAddressModeW, mMagFilter, mMinFilter, mMipmapFilter,
        mLodMinClamp, mLodMaxClamp, mCompareFunction, mMaxAnisotropy, mIsYCbCr,
        mYCbCrVkDescriptor.vkFormat, mYCbCrVkDescriptor.vkYCbCrModel,
        mYCbCrVkDescriptor.vkYCbCrRange, mYCbCrVkDescriptor.vkComponentSwizzleRed,
        mYCbCrVkDescriptor.vkComponentSwizzleGreen, mYCbCrVkDescriptor.vkComponentSwizzleBlue,
        mYCbCrVkDescriptor.vkComponentSwizzleAlpha, mYCbCrVkDescriptor.vkXChromaOffset,
        mYCbCrVkDescriptor.vkYChromaOffset, mYCbCrVkDescriptor.vkChromaFilter,
        mYCbCrVkDescriptor.forceExplicitReconstruction, mYCbCrVkDescriptor.externalFormat);
    return recorder.GetContentHash();
}

bool SamplerBase::EqualityFunc::operator()(const SamplerBase* a, const SamplerBase* b) const {
    if (a == b) {
        return true;
    }

    DAWN_ASSERT(!std::isnan(a->mLodMinClamp));
    DAWN_ASSERT(!std::isnan(b->mLodMinClamp));
    DAWN_ASSERT(!std::isnan(a->mLodMaxClamp));
    DAWN_ASSERT(!std::isnan(b->mLodMaxClamp));

    // NOTE: For simplicity, we always check the state of the YCbCr descriptor.
    // If the client did not pass in a YCbCr descriptor, `mIsYCbCr` will be
    // false and the YCbCr descriptor will have default values. The use of
    // `mIsYCbCr` here differentiates that case from the case of the client
    // passing in a YCbCr descriptor holding all default values.
    return a->mAddressModeU == b->mAddressModeU && a->mAddressModeV == b->mAddressModeV &&
           a->mAddressModeW == b->mAddressModeW && a->mMagFilter == b->mMagFilter &&
           a->mMinFilter == b->mMinFilter && a->mMipmapFilter == b->mMipmapFilter &&
           a->mLodMinClamp == b->mLodMinClamp && a->mLodMaxClamp == b->mLodMaxClamp &&
           a->mCompareFunction == b->mCompareFunction && a->mMaxAnisotropy == b->mMaxAnisotropy &&
           a->mIsYCbCr == b->mIsYCbCr &&
           a->mYCbCrVkDescriptor.vkFormat == b->mYCbCrVkDescriptor.vkFormat &&
           a->mYCbCrVkDescriptor.vkYCbCrModel == b->mYCbCrVkDescriptor.vkYCbCrModel &&
           a->mYCbCrVkDescriptor.vkYCbCrRange == b->mYCbCrVkDescriptor.vkYCbCrRange &&
           a->mYCbCrVkDescriptor.vkComponentSwizzleRed ==
               b->mYCbCrVkDescriptor.vkComponentSwizzleRed &&
           a->mYCbCrVkDescriptor.vkComponentSwizzleGreen ==
               b->mYCbCrVkDescriptor.vkComponentSwizzleGreen &&
           a->mYCbCrVkDescriptor.vkComponentSwizzleBlue ==
               b->mYCbCrVkDescriptor.vkComponentSwizzleBlue &&
           a->mYCbCrVkDescriptor.vkComponentSwizzleAlpha ==
               b->mYCbCrVkDescriptor.vkComponentSwizzleAlpha &&
           a->mYCbCrVkDescriptor.vkXChromaOffset == b->mYCbCrVkDescriptor.vkXChromaOffset &&
           a->mYCbCrVkDescriptor.vkYChromaOffset == b->mYCbCrVkDescriptor.vkYChromaOffset &&
           a->mYCbCrVkDescriptor.vkChromaFilter == b->mYCbCrVkDescriptor.vkChromaFilter &&
           a->mYCbCrVkDescriptor.forceExplicitReconstruction ==
               b->mYCbCrVkDescriptor.forceExplicitReconstruction &&
           a->mYCbCrVkDescriptor.externalFormat == b->mYCbCrVkDescriptor.externalFormat;
}

}  // namespace dawn::native
