// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/PhysicalDevice.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

FeatureValidationResult::FeatureValidationResult() : success(true) {}
FeatureValidationResult::FeatureValidationResult(std::string errorMsg)
    : success(false), errorMessage(errorMsg) {}

PhysicalDeviceBase::PhysicalDeviceBase(wgpu::BackendType backend) : mBackend(backend) {}

PhysicalDeviceBase::~PhysicalDeviceBase() = default;

MaybeError PhysicalDeviceBase::Initialize() {
    DAWN_TRY_CONTEXT(InitializeImpl(), "initializing adapter (backend=%s)", mBackend);
    InitializeVendorArchitectureImpl();

    EnableFeature(Feature::DawnNative);
    EnableFeature(Feature::DawnInternalUsages);
    EnableFeature(Feature::ImplicitDeviceSynchronization);
    EnableFeature(Feature::DawnFormatCapabilities);
    InitializeSupportedFeaturesImpl();

    DAWN_TRY_CONTEXT(
        InitializeSupportedLimitsImpl(&mLimits),
        "gathering supported limits for \"%s\" - \"%s\" (vendorId=%#06x deviceId=%#06x "
        "backend=%s type=%s)",
        mName, mDriverDescription, mVendorId, mDeviceId, mBackend, mAdapterType);

    NormalizeLimits(&mLimits.v1);

    return {};
}

ResultOrError<Ref<DeviceBase>> PhysicalDeviceBase::CreateDevice(
    AdapterBase* adapter,
    const UnpackedPtr<DeviceDescriptor>& descriptor,
    const TogglesState& deviceToggles,
    Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    return CreateDeviceImpl(adapter, descriptor, deviceToggles, std::move(lostEvent));
}

void PhysicalDeviceBase::InitializeVendorArchitectureImpl() {
    mVendorName = gpu_info::GetVendorName(mVendorId);
    mArchitectureName = gpu_info::GetArchitectureName(mVendorId, mDeviceId);
}

uint32_t PhysicalDeviceBase::GetVendorId() const {
    return mVendorId;
}

uint32_t PhysicalDeviceBase::GetDeviceId() const {
    return mDeviceId;
}

const std::string& PhysicalDeviceBase::GetVendorName() const {
    return mVendorName;
}

const std::string& PhysicalDeviceBase::GetArchitectureName() const {
    return mArchitectureName;
}

const std::string& PhysicalDeviceBase::GetName() const {
    return mName;
}

const gpu_info::DriverVersion& PhysicalDeviceBase::GetDriverVersion() const {
    return mDriverVersion;
}

const std::string& PhysicalDeviceBase::GetDriverDescription() const {
    return mDriverDescription;
}

wgpu::AdapterType PhysicalDeviceBase::GetAdapterType() const {
    return mAdapterType;
}

wgpu::BackendType PhysicalDeviceBase::GetBackendType() const {
    return mBackend;
}

uint32_t PhysicalDeviceBase::GetSubgroupMinSize() const {
    return mSubgroupMinSize;
}

uint32_t PhysicalDeviceBase::GetSubgroupMaxSize() const {
    return mSubgroupMaxSize;
}

bool PhysicalDeviceBase::IsFeatureSupportedWithToggles(wgpu::FeatureName feature,
                                                       const TogglesState& toggles) const {
    return ValidateFeatureSupportedWithToggles(feature, toggles).success;
}

void PhysicalDeviceBase::GetDefaultLimitsForSupportedFeatureLevel(Limits* limits) const {
    // If the physical device does not support core then the defaults are compat defaults.
    GetDefaultLimits(limits, SupportsFeatureLevel(wgpu::FeatureLevel::Core, nullptr)
                                 ? wgpu::FeatureLevel::Core
                                 : wgpu::FeatureLevel::Compatibility);
}

FeaturesSet PhysicalDeviceBase::GetSupportedFeatures(const TogglesState& toggles) const {
    FeaturesSet supportedFeaturesWithToggles;
    // Iterate each PhysicalDevice's supported feature and check if it is supported with given
    // toggles
    for (Feature feature : IterateBitSet(mSupportedFeatures.featuresBitSet)) {
        if (IsFeatureSupportedWithToggles(ToAPI(feature), toggles)) {
            supportedFeaturesWithToggles.EnableFeature(feature);
        }
    }
    return supportedFeaturesWithToggles;
}

bool PhysicalDeviceBase::SupportsAllRequiredFeatures(
    const ityp::span<size_t, const wgpu::FeatureName>& features,
    const TogglesState& toggles) const {
    for (wgpu::FeatureName f : features) {
        if (!IsFeatureSupportedWithToggles(f, toggles)) {
            return false;
        }
    }
    return true;
}

const CombinedLimits& PhysicalDeviceBase::GetLimits() const {
    return mLimits;
}

void PhysicalDeviceBase::EnableFeature(Feature feature) {
    mSupportedFeatures.EnableFeature(feature);
}

FeatureValidationResult PhysicalDeviceBase::ValidateFeatureSupportedWithToggles(
    wgpu::FeatureName feature,
    const TogglesState& toggles) const {
    auto validateNameResult = ValidateFeatureName(feature);
    if (validateNameResult.IsError()) {
        return FeatureValidationResult(validateNameResult.AcquireError()->GetMessage());
    }

    if (!mSupportedFeatures.IsEnabled(feature)) {
        return FeatureValidationResult(
            absl::StrFormat("Requested feature %s is not supported.", feature));
    }

    const FeatureInfo* featureInfo = GetFeatureInfo(feature);
    // Experimental features are guarded by the AllowUnsafeAPIs toggle.
    if (featureInfo->featureState == FeatureInfo::FeatureState::Experimental) {
        // AllowUnsafeAPIs toggle is by default disabled if not explicitly enabled.
        if (!toggles.IsEnabled(Toggle::AllowUnsafeAPIs)) {
            return FeatureValidationResult(absl::StrFormat(
                "Feature %s is guarded by toggle allow_unsafe_apis.", featureInfo->name));
        }
    }

    // Do backend-specific validation.
    return ValidateFeatureSupportedWithTogglesImpl(feature, toggles);
}

void PhysicalDeviceBase::SetSupportedFeaturesForTesting(
    const std::vector<wgpu::FeatureName>& requiredFeatures) {
    mSupportedFeatures = {};
    for (wgpu::FeatureName f : requiredFeatures) {
        mSupportedFeatures.EnableFeature(f);
    }
}

MaybeError PhysicalDeviceBase::ResetInternalDeviceForTesting() {
    return ResetInternalDeviceForTestingImpl();
}

MaybeError PhysicalDeviceBase::ResetInternalDeviceForTestingImpl() {
    return DAWN_INTERNAL_ERROR(
        "ResetInternalDeviceForTesting should only be used with the D3D12 backend.");
}

void PhysicalDeviceBase::PopulateBackendFormatCapabilities(
    wgpu::TextureFormat format,
    UnpackedPtr<DawnFormatCapabilities>& capabilities) const {}

}  // namespace dawn::native
