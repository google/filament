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

#include "dawn/native/Adapter.h"

#include <algorithm>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/PhysicalDevice.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {
namespace {
static constexpr DeviceDescriptor kDefaultDeviceDesc = {};
}  // anonymous namespace

AdapterBase::AdapterBase(InstanceBase* instance,
                         Ref<PhysicalDeviceBase> physicalDevice,
                         wgpu::FeatureLevel featureLevel,
                         const TogglesState& requiredAdapterToggles,
                         wgpu::PowerPreference powerPreference)
    : mInstance(instance),
      mPhysicalDevice(std::move(physicalDevice)),
      mFeatureLevel(featureLevel),
      mTogglesState(requiredAdapterToggles),
      mPowerPreference(powerPreference) {
    DAWN_ASSERT(mPhysicalDevice->SupportsFeatureLevel(featureLevel, mInstance.Get()));
    DAWN_ASSERT(mTogglesState.GetStage() == ToggleStage::Adapter);
    // Cache the supported features of this adapter. Note that with device toggles overriding, a
    // device created by this adapter may support features not in this set and vice versa.
    mSupportedFeatures = mPhysicalDevice->GetSupportedFeatures(mTogglesState);
    // Cache the limits of this adapter. UpdateLimits should be called when the adapter's
    // limits-related status changes, e.g. SetUseTieredLimits.
    UpdateLimits();
}

AdapterBase::~AdapterBase() = default;

void AdapterBase::SetUseTieredLimits(bool useTieredLimits) {
    mUseTieredLimits = useTieredLimits;
    UpdateLimits();
}

PhysicalDeviceBase* AdapterBase::GetPhysicalDevice() {
    return mPhysicalDevice.Get();
}

const PhysicalDeviceBase* AdapterBase::GetPhysicalDevice() const {
    return mPhysicalDevice.Get();
}

InstanceBase* AdapterBase::GetInstance() const {
    return mInstance.Get();
}

InstanceBase* AdapterBase::APIGetInstance() const {
    InstanceBase* instance = mInstance.Get();
    DAWN_ASSERT(instance != nullptr);
    instance->APIAddRef();
    return instance;
}

void AdapterBase::UpdateLimits() {
    mLimits = mPhysicalDevice->GetLimits();

    // Apply the tiered limits if needed.
    if (mUseTieredLimits) {
        mLimits = ApplyLimitTiers(std::move(mLimits));
    }
}

const CombinedLimits& AdapterBase::GetLimits() const {
    return mLimits;
}

wgpu::Status AdapterBase::APIGetLimits(Limits* limits) const {
    DAWN_ASSERT(limits != nullptr);
    UnpackedPtr<Limits> unpacked;
    if (mInstance->ConsumedError(ValidateAndUnpack(limits), &unpacked)) {
        return wgpu::Status::Error;
    }

    {
        wgpu::ChainedStructOut* originalChain = unpacked->nextInChain;
        **unpacked = mLimits.v1;
        // Recover origin chain.
        unpacked->nextInChain = originalChain;
    }

    if (auto* immediateDataLimits = unpacked.Get<DawnExperimentalImmediateDataLimits>()) {
        wgpu::ChainedStructOut* originalChain = immediateDataLimits->nextInChain;
        if (!mSupportedFeatures.IsEnabled(wgpu::FeatureName::ChromiumExperimentalImmediateData)) {
            // If immediate data features are not supported, return the default-initialized
            // DawnExperimentalImmediateDataLimits object, where maxImmediateDataByteSize is
            // WGPU_LIMIT_U32_UNDEFINED.
            *immediateDataLimits = DawnExperimentalImmediateDataLimits{};
        } else {
            // If adapter supports immediate data features, always return the valid immediate data
            // limits.
            *immediateDataLimits = mLimits.experimentalImmediateDataLimits;
        }

        // Recover origin chain.
        immediateDataLimits->nextInChain = originalChain;
    }

    if (auto* texelCopyBufferRowAlignmentLimits =
            unpacked.Get<DawnTexelCopyBufferRowAlignmentLimits>()) {
        wgpu::ChainedStructOut* originalChain = texelCopyBufferRowAlignmentLimits->nextInChain;
        if (!mSupportedFeatures.IsEnabled(wgpu::FeatureName::DawnTexelCopyBufferRowAlignment)) {
            // If the feature is not enabled, minTexelCopyBufferRowAlignment is default-initialized
            // to WGPU_LIMIT_U32_UNDEFINED.
            *texelCopyBufferRowAlignmentLimits = DawnTexelCopyBufferRowAlignmentLimits{};
        } else {
            *texelCopyBufferRowAlignmentLimits = mLimits.texelCopyBufferRowAlignmentLimits;
        }

        // Recover origin chain.
        texelCopyBufferRowAlignmentLimits->nextInChain = originalChain;
    }

    return wgpu::Status::Success;
}

wgpu::Status AdapterBase::APIGetInfo(AdapterInfo* info) const {
    DAWN_ASSERT(info != nullptr);

    UnpackedPtr<AdapterInfo> unpacked;
    if (mInstance->ConsumedError(ValidateAndUnpack(info), &unpacked)) {
        return wgpu::Status::Error;
    }

    bool hadError = false;
    if (unpacked.Get<AdapterPropertiesMemoryHeaps>() != nullptr &&
        !mSupportedFeatures.IsEnabled(wgpu::FeatureName::AdapterPropertiesMemoryHeaps)) {
        hadError |= mInstance->ConsumedError(
            DAWN_VALIDATION_ERROR("Feature AdapterPropertiesMemoryHeaps is not available."));
    }
    if (unpacked.Get<AdapterPropertiesD3D>() != nullptr &&
        !mSupportedFeatures.IsEnabled(wgpu::FeatureName::AdapterPropertiesD3D)) {
        hadError |= mInstance->ConsumedError(
            DAWN_VALIDATION_ERROR("Feature AdapterPropertiesD3D is not available."));
    }
    if (unpacked.Get<AdapterPropertiesVk>() != nullptr &&
        !mSupportedFeatures.IsEnabled(wgpu::FeatureName::AdapterPropertiesVk)) {
        hadError |= mInstance->ConsumedError(
            DAWN_VALIDATION_ERROR("Feature AdapterPropertiesVk is not available."));
    }
    if (unpacked.Get<AdapterPropertiesSubgroupMatrixConfigs>() != nullptr &&
        !mSupportedFeatures.IsEnabled(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix)) {
        hadError |= mInstance->ConsumedError(
            DAWN_VALIDATION_ERROR("Feature ChromiumExperimentalSubgroupMatrix is not available."));
    }
    if (hadError) {
        return wgpu::Status::Error;
    }

    if (auto* powerPreferenceDesc = unpacked.Get<DawnAdapterPropertiesPowerPreference>()) {
        powerPreferenceDesc->powerPreference = mPowerPreference;
    }
    if (auto* subgroupsProperties = unpacked.Get<AdapterPropertiesSubgroups>()) {
        // When the feature is *not* supported, these must be 4 and 128.
        // Set those defaults now, but a backend may override this.
        subgroupsProperties->subgroupMinSize = 4;
        subgroupsProperties->subgroupMaxSize = 128;
    }

    mPhysicalDevice->PopulateBackendProperties(unpacked);

    if (auto* subgroupsProperties = unpacked.Get<AdapterPropertiesSubgroups>()) {
        if (mPhysicalDevice->GetBackendType() == wgpu::BackendType::D3D12 &&
            mTogglesState.IsEnabled(Toggle::D3D12RelaxMinSubgroupSizeTo8)) {
            subgroupsProperties->subgroupMinSize =
                std::min(subgroupsProperties->subgroupMinSize, 8u);
        }
    }

    // Allocate space for all strings.
    size_t allocSize = mPhysicalDevice->GetVendorName().length() +
                       mPhysicalDevice->GetArchitectureName().length() +
                       mPhysicalDevice->GetName().length() +
                       mPhysicalDevice->GetDriverDescription().length();
    absl::Span<char> outBuffer{new char[allocSize], allocSize};

    auto AddString = [&](const std::string& in, StringView* out) {
        DAWN_ASSERT(in.length() <= outBuffer.length());
        memcpy(outBuffer.data(), in.data(), in.length());
        *out = {outBuffer.data(), in.length()};
        outBuffer = outBuffer.subspan(in.length());
    };

    AddString(mPhysicalDevice->GetVendorName(), &info->vendor);
    AddString(mPhysicalDevice->GetArchitectureName(), &info->architecture);
    AddString(mPhysicalDevice->GetName(), &info->device);
    AddString(mPhysicalDevice->GetDriverDescription(), &info->description);
    DAWN_ASSERT(outBuffer.empty());

    info->backendType = mPhysicalDevice->GetBackendType();
    info->adapterType = mPhysicalDevice->GetAdapterType();
    info->vendorID = mPhysicalDevice->GetVendorId();
    info->deviceID = mPhysicalDevice->GetDeviceId();
    info->subgroupMinSize = mPhysicalDevice->GetSubgroupMinSize();
    info->subgroupMaxSize = mPhysicalDevice->GetSubgroupMaxSize();

    if (mPhysicalDevice->GetBackendType() == wgpu::BackendType::D3D12 &&
        mTogglesState.IsEnabled(Toggle::D3D12RelaxMinSubgroupSizeTo8)) {
        info->subgroupMinSize = std::min(info->subgroupMinSize, 8u);
    }

    return wgpu::Status::Success;
}

void APIAdapterInfoFreeMembers(WGPUAdapterInfo info) {
    // This single delete is enough because everything is a single allocation.
    delete[] info.vendor.data;
}

void APIAdapterPropertiesMemoryHeapsFreeMembers(
    WGPUAdapterPropertiesMemoryHeaps memoryHeapProperties) {
    delete[] memoryHeapProperties.heapInfo;
}

void APIDawnDrmFormatCapabilitiesFreeMembers(WGPUDawnDrmFormatCapabilities capabilities) {
    delete[] capabilities.properties;
}

void APIAdapterPropertiesSubgroupMatrixConfigsFreeMembers(
    WGPUAdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs) {
    delete[] subgroupMatrixConfigs.configs;
}

bool AdapterBase::APIHasFeature(wgpu::FeatureName feature) const {
    return mSupportedFeatures.IsEnabled(feature);
}

void AdapterBase::APIGetFeatures(wgpu::SupportedFeatures* features) const {
    this->APIGetFeatures(reinterpret_cast<SupportedFeatures*>(features));
}

void AdapterBase::APIGetFeatures(SupportedFeatures* features) const {
    mSupportedFeatures.ToSupportedFeatures(features);
}

void APISupportedFeaturesFreeMembers(WGPUSupportedFeatures supportedFeatures) {
    delete[] supportedFeatures.features;
}

// TODO(https://crbug.com/dawn/2465) Could potentially re-implement via AllowSpontaneous async mode.
DeviceBase* AdapterBase::APICreateDevice(const DeviceDescriptor* descriptor) {
    if (descriptor == nullptr) {
        descriptor = &kDefaultDeviceDesc;
    }

    auto [lostEvent, result] = CreateDevice(descriptor);
    mInstance->GetEventManager()->TrackEvent(lostEvent);
    Ref<DeviceBase> device;
    if (mInstance->ConsumedError(std::move(result), &device)) {
        return nullptr;
    }
    return ReturnToAPI(std::move(device));
}

ResultOrError<Ref<DeviceBase>> AdapterBase::CreateDeviceInternal(
    const DeviceDescriptor* rawDescriptor,
    Ref<DeviceBase::DeviceLostEvent> lostEvent) {
    DAWN_ASSERT(rawDescriptor != nullptr);

    // Create device toggles state from required toggles descriptor and inherited adapter toggles
    // state.
    UnpackedPtr<DeviceDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(rawDescriptor));
    auto* deviceTogglesDesc = descriptor.Get<DawnTogglesDescriptor>();

    // Create device toggles state.
    TogglesState deviceToggles =
        TogglesState::CreateFromTogglesDescriptor(deviceTogglesDesc, ToggleStage::Device);
    deviceToggles.InheritFrom(mTogglesState);
    // Default toggles for all backend
    deviceToggles.Default(Toggle::LazyClearResourceOnFirstUse, true);
    deviceToggles.Default(Toggle::TimestampQuantization, true);
    if (mInstance->IsBackendValidationEnabled()) {
        deviceToggles.Default(Toggle::UseUserDefinedLabelsInBackend, true);
    }

    // Backend-specific forced and default device toggles
    mPhysicalDevice->SetupBackendDeviceToggles(mInstance->GetPlatform(), &deviceToggles);

    std::unordered_set<wgpu::FeatureName> requiredFeatureSet;
    for (uint32_t i = 0; i < descriptor->requiredFeatureCount; ++i) {
        requiredFeatureSet.insert(descriptor->requiredFeatures[i]);
    }

    // Validate all required features are supported by the adapter and suitable under device
    // toggles. Note that certain toggles in device toggles state may be overridden by user and
    // different from the adapter toggles state, and in this case a device may support features
    // that not supported by the adapter. We allow such toggles overriding for the convenience e.g.
    // creating a device for internal usage with AllowUnsafeAPI enabled from an adapter that
    // disabled AllowUnsafeAPIS.
    for (wgpu::FeatureName requiredFeature : requiredFeatureSet) {
        FeatureValidationResult result =
            mPhysicalDevice->ValidateFeatureSupportedWithToggles(requiredFeature, deviceToggles);
        DAWN_INVALID_IF(!result.success, "Invalid feature required: %s",
                        result.errorMessage.c_str());
    }
    // Validate features dependency.
    // TODO(349125474): Decide if this validation is needed, see
    // https://github.com/gpuweb/gpuweb/issues/4734 for detail.
    if (requiredFeatureSet.count(wgpu::FeatureName::SubgroupsF16) > 0) {
        DAWN_INVALID_IF((requiredFeatureSet.count(wgpu::FeatureName::Subgroups) == 0),
                        "Feature %s must be required together with feature %s.",
                        wgpu::FeatureName::SubgroupsF16, wgpu::FeatureName::Subgroups);
        DAWN_INVALID_IF(requiredFeatureSet.count(wgpu::FeatureName::ShaderF16) == 0,
                        "Feature %s must be required together with feature %s.",
                        wgpu::FeatureName::SubgroupsF16, wgpu::FeatureName::ShaderF16);
    }

    if (descriptor->requiredLimits != nullptr) {
        // Only consider limits in RequiredLimits structure, and currently no chained structure
        // supported.
        DAWN_INVALID_IF(descriptor->requiredLimits->nextInChain != nullptr,
                        "can not chain after requiredLimits.");

        Limits supportedLimits;
        wgpu::Status status = APIGetLimits(&supportedLimits);
        DAWN_ASSERT(status == wgpu::Status::Success);

        DAWN_TRY_CONTEXT(ValidateLimits(supportedLimits, *descriptor->requiredLimits),
                         "validating required limits");
    }

    return mPhysicalDevice->CreateDevice(this, descriptor, deviceToggles, std::move(lostEvent));
}

std::pair<Ref<DeviceBase::DeviceLostEvent>, ResultOrError<Ref<DeviceBase>>>
AdapterBase::CreateDevice(const DeviceDescriptor* descriptor) {
    DAWN_ASSERT(descriptor != nullptr);

    Ref<DeviceBase::DeviceLostEvent> lostEvent = DeviceBase::DeviceLostEvent::Create(descriptor);
    auto result = CreateDeviceInternal(descriptor, lostEvent);

    // Catch any errors to directly complete the device lost event with the error message.
    if (result.IsError()) {
        auto error = result.AcquireError();
        lostEvent->mReason = wgpu::DeviceLostReason::FailedCreation;
        lostEvent->mMessage = "Failed to create device:\n" + error->GetFormattedMessage();

        // When the device fails to initialize, we need to both promote the device ref to an
        // external ref to clean up resources, and drop it, so we acquire it in this scope.
        APIRef<DeviceBase> device;
        device.Acquire(ReturnToAPI(std::move(lostEvent->mDevice)));

        mInstance->GetEventManager()->SetFutureReady(lostEvent.Get());
        return {lostEvent, std::move(error)};
    }

    return {lostEvent, std::move(result)};
}

Future AdapterBase::APIRequestDevice(const DeviceDescriptor* descriptor,
                                     const WGPURequestDeviceCallbackInfo& callbackInfo) {
    struct RequestDeviceEvent final : public EventManager::TrackedEvent {
        WGPURequestDeviceCallback mCallback;
        raw_ptr<void> mUserdata1;
        raw_ptr<void> mUserdata2;

        WGPURequestDeviceStatus mStatus;
        Ref<DeviceBase> mDevice = nullptr;
        std::string mMessage;

        RequestDeviceEvent(const WGPURequestDeviceCallbackInfo& callbackInfo,
                           Ref<DeviceBase> device)
            : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                           TrackedEvent::Completed{}),
              mCallback(callbackInfo.callback),
              mUserdata1(callbackInfo.userdata1),
              mUserdata2(callbackInfo.userdata2),
              mStatus(WGPURequestDeviceStatus_Success),
              mDevice(std::move(device)) {}

        RequestDeviceEvent(const WGPURequestDeviceCallbackInfo& callbackInfo,
                           const std::string& message)
            : TrackedEvent(static_cast<wgpu::CallbackMode>(callbackInfo.mode),
                           TrackedEvent::Completed{}),
              mCallback(callbackInfo.callback),
              mUserdata1(callbackInfo.userdata1),
              mUserdata2(callbackInfo.userdata2),
              mStatus(WGPURequestDeviceStatus_Error),
              mMessage(message) {}

        ~RequestDeviceEvent() override { EnsureComplete(EventCompletionType::Shutdown); }

        void Complete(EventCompletionType completionType) override {
            if (completionType == EventCompletionType::Shutdown) {
                mStatus = WGPURequestDeviceStatus_CallbackCancelled;
                mDevice = nullptr;
                mMessage = "A valid external Instance reference no longer exists.";
            }
            mCallback(mStatus, ToAPI(ReturnToAPI(std::move(mDevice))), ToOutputStringView(mMessage),
                      mUserdata1.ExtractAsDangling(), mUserdata2.ExtractAsDangling());
        }
    };

    if (descriptor == nullptr) {
        descriptor = &kDefaultDeviceDesc;
    }

    FutureID futureID = kNullFutureID;
    auto [lostEvent, result] = CreateDevice(descriptor);
    if (result.IsSuccess()) {
        futureID = mInstance->GetEventManager()->TrackEvent(
            AcquireRef(new RequestDeviceEvent(callbackInfo, result.AcquireSuccess())));
    } else {
        futureID = mInstance->GetEventManager()->TrackEvent(AcquireRef(
            new RequestDeviceEvent(callbackInfo, result.AcquireError()->GetFormattedMessage())));
    }
    mInstance->GetEventManager()->TrackEvent(std::move(lostEvent));
    return {futureID};
}

wgpu::Status AdapterBase::APIGetFormatCapabilities(wgpu::TextureFormat format,
                                                   DawnFormatCapabilities* capabilities) {
    if (!mSupportedFeatures.IsEnabled(wgpu::FeatureName::DawnFormatCapabilities)) {
        [[maybe_unused]] bool hadError = mInstance->ConsumedError(
            DAWN_VALIDATION_ERROR("Feature DawnFormatCapabilities is not available."));
        return wgpu::Status::Error;
    }
    DAWN_ASSERT(capabilities != nullptr);

    UnpackedPtr<DawnFormatCapabilities> unpacked;
    if (mInstance->ConsumedError(ValidateAndUnpack(capabilities), &unpacked)) {
        return wgpu::Status::Error;
    }

    if (unpacked.Get<DawnDrmFormatCapabilities>() != nullptr &&
        !mSupportedFeatures.IsEnabled(wgpu::FeatureName::DawnDrmFormatCapabilities)) {
        [[maybe_unused]] bool hadError = mInstance->ConsumedError(
            DAWN_VALIDATION_ERROR("Feature DawnDrmFormatCapabilities is not available."));
        return wgpu::Status::Error;
    }

    mPhysicalDevice->PopulateBackendFormatCapabilities(format, unpacked);
    return wgpu::Status::Success;
}

const TogglesState& AdapterBase::GetTogglesState() const {
    return mTogglesState;
}

wgpu::FeatureLevel AdapterBase::GetFeatureLevel() const {
    return mFeatureLevel;
}

const std::string& AdapterBase::GetName() const {
    return mPhysicalDevice->GetName();
}

std::vector<Ref<AdapterBase>> SortAdapters(std::vector<Ref<AdapterBase>> adapters,
                                           const UnpackedPtr<RequestAdapterOptions>& options) {
    const bool highPerformance = options->powerPreference == wgpu::PowerPreference::HighPerformance;

    const auto ComputeAdapterTypeRank = [&](const Ref<AdapterBase>& a) {
        switch (a->GetPhysicalDevice()->GetAdapterType()) {
            case wgpu::AdapterType::DiscreteGPU:
                return highPerformance ? 0 : 1;
            case wgpu::AdapterType::IntegratedGPU:
                return highPerformance ? 1 : 0;
            case wgpu::AdapterType::CPU:
                return 2;
            case wgpu::AdapterType::Unknown:
                return 3;
        }
        DAWN_UNREACHABLE();
    };

    const auto ComputeBackendTypeRank = [](const Ref<AdapterBase>& a) {
        switch (a->GetPhysicalDevice()->GetBackendType()) {
            // Sort backends generally in order of Core -> Compat -> Testing,
            // while preferring OS-specific backends like Metal/D3D.
            case wgpu::BackendType::Metal:
            case wgpu::BackendType::D3D12:
                return 0;
            case wgpu::BackendType::Vulkan:
                return 1;
            case wgpu::BackendType::D3D11:
                return 2;
            case wgpu::BackendType::OpenGLES:
                return 3;
            case wgpu::BackendType::OpenGL:
                return 4;
            case wgpu::BackendType::WebGPU:
                return 5;
            case wgpu::BackendType::Null:
                return 6;
            case wgpu::BackendType::Undefined:
                break;
        }
        DAWN_UNREACHABLE();
    };

    std::sort(adapters.begin(), adapters.end(),
              [&](const Ref<AdapterBase>& a, const Ref<AdapterBase>& b) -> bool {
                  return std::tuple(ComputeAdapterTypeRank(a), ComputeBackendTypeRank(a)) <
                         std::tuple(ComputeAdapterTypeRank(b), ComputeBackendTypeRank(b));
              });

    return adapters;
}

}  // namespace dawn::native
