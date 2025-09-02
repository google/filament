// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/wire/client/Adapter.h"

#include <memory>
#include <string>
#include <utility>

#include "absl/types/span.h"  // TODO(343500108): Use std::span when we have C++20.
#include "dawn/common/Log.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/wire/client/Client.h"
#include "dawn/wire/client/webgpu.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {
namespace {

class RequestDeviceEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::RequestDevice;

    RequestDeviceEvent(const WGPURequestDeviceCallbackInfo& callbackInfo, Ref<Device> device)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2),
          mDevice(std::move(device)) {}

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID,
                         WGPURequestDeviceStatus status,
                         WGPUStringView message,
                         const WGPULimits* limits,
                         uint32_t featuresCount,
                         const WGPUFeatureName* features) {
        DAWN_ASSERT(mDevice != nullptr);
        mStatus = status;
        mMessage = ToString(message);
        if (status == WGPURequestDeviceStatus_Success) {
            mDevice->SetLimits(limits);
            mDevice->SetFeatures(features, featuresCount);
        }
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPURequestDeviceStatus_CallbackCancelled;
            mMessage = "A valid external Instance reference no longer exists.";
        }

        // Callback needs to happen before device lost handling to ensure resolution order.
        void* userdata1 = mUserdata1.ExtractAsDangling();
        void* userdata2 = mUserdata2.ExtractAsDangling();
        if (mCallback) {
            Ref<Device> device = mDevice;
            mCallback(mStatus,
                      mStatus == WGPURequestDeviceStatus_Success ? ReturnToAPI(std::move(device))
                                                                 : nullptr,
                      ToOutputStringView(mMessage), userdata1, userdata2);
        }

        if (mStatus != WGPURequestDeviceStatus_Success) {
            // If there was an error and we didn't return a device, we need to call the device lost
            // callback and reclaim the device allocation.
            if (mStatus == WGPURequestDeviceStatus_CallbackCancelled) {
                mDevice->HandleDeviceLost(
                    WGPUDeviceLostReason_CallbackCancelled,
                    ToOutputStringView("A valid external Instance reference no longer exists."));
            } else {
                mDevice->HandleDeviceLost(WGPUDeviceLostReason_FailedCreation,
                                          ToOutputStringView("Device failed at creation."));
            }
        }
    }

    WGPURequestDeviceCallback mCallback = nullptr;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    // Note that the message is optional because we want to return nullptr when it wasn't set
    // instead of a pointer to an empty string.
    WGPURequestDeviceStatus mStatus;
    std::string mMessage;

    // The device is created when we call RequestDevice. It is guaranteed to be alive
    // throughout the duration of a RequestDeviceEvent because the Event essentially takes
    // ownership of it until either an error occurs at which point the Event cleans it up, or it
    // returns the device to the user who then takes ownership as the Event goes away.
    Ref<Device> mDevice;
};

}  // anonymous namespace

ObjectType Adapter::GetObjectType() const {
    return ObjectType::Adapter;
}

WGPUStatus Adapter::APIGetLimits(WGPULimits* limits) const {
    return mLimitsAndFeatures.GetLimits(limits);
}

bool Adapter::APIHasFeature(WGPUFeatureName feature) const {
    return mLimitsAndFeatures.HasFeature(feature);
}

void Adapter::APIGetFeatures(WGPUSupportedFeatures* features) const {
    mLimitsAndFeatures.ToSupportedFeatures(features);
}

void Adapter::SetLimits(const WGPULimits* limits) {
    return mLimitsAndFeatures.SetLimits(limits);
}

void Adapter::SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount) {
    return mLimitsAndFeatures.SetFeatures(features, featuresCount);
}

void Adapter::SetInfo(const WGPUAdapterInfo* info) {
    mInfo = *info;

    // Deep copy the string pointed out by info. StringViews are all explicitly sized by the wire.
    mVendor = ToString(info->vendor);
    mInfo.vendor = ToOutputStringView(mVendor);
    mArchitecture = ToString(info->architecture);
    mInfo.architecture = ToOutputStringView(mArchitecture);
    mDeviceName = ToString(info->device);
    mInfo.device = ToOutputStringView(mDeviceName);
    mDescription = ToString(info->description);
    mInfo.description = ToOutputStringView(mDescription);

    mInfo.nextInChain = nullptr;

    // Loop through the chained struct.
    WGPUChainedStruct* chain = info->nextInChain;
    while (chain != nullptr) {
        switch (chain->sType) {
            case WGPUSType_AdapterPropertiesMemoryHeaps: {
                // Make a copy of the heap info in `mMemoryHeapInfo`.
                const auto* memoryHeapProperties =
                    reinterpret_cast<const WGPUAdapterPropertiesMemoryHeaps*>(chain);
                mMemoryHeapInfo = {
                    memoryHeapProperties->heapInfo,
                    memoryHeapProperties->heapInfo + memoryHeapProperties->heapCount};
                break;
            }
            case WGPUSType_AdapterPropertiesD3D: {
                auto* d3dProperties = reinterpret_cast<WGPUAdapterPropertiesD3D*>(chain);
                mD3DProperties.shaderModel = d3dProperties->shaderModel;
                break;
            }
            case WGPUSType_AdapterPropertiesVk: {
                auto* vkProperties = reinterpret_cast<WGPUAdapterPropertiesVk*>(chain);
                mVkProperties.driverVersion = vkProperties->driverVersion;
                break;
            }
            case WGPUSType_AdapterPropertiesSubgroupMatrixConfigs: {
                // Make a copy of the heap info in `mSubgroupMatrixConfigs`.
                const auto* subgroupMatrixConfigs =
                    reinterpret_cast<const WGPUAdapterPropertiesSubgroupMatrixConfigs*>(chain);
                mSubgroupMatrixConfigs = {
                    subgroupMatrixConfigs->configs,
                    subgroupMatrixConfigs->configs + subgroupMatrixConfigs->configCount};
                break;
            }
            case WGPUSType_DawnAdapterPropertiesPowerPreference: {
                auto* powerProperties =
                    reinterpret_cast<WGPUDawnAdapterPropertiesPowerPreference*>(chain);
                mPowerProperties.powerPreference = powerProperties->powerPreference;
                break;
            }
            default:
                DAWN_UNREACHABLE();
                break;
        }
        chain = chain->next;
    }
}

WGPUStatus Adapter::APIGetInfo(WGPUAdapterInfo* info) const {
    // Loop through the chained struct.
    WGPUChainedStruct* chain = info->nextInChain;
    while (chain != nullptr) {
        switch (chain->sType) {
            case WGPUSType_AdapterPropertiesMemoryHeaps: {
                // Copy `mMemoryHeapInfo` into a new allocation.
                auto* memoryHeapProperties =
                    reinterpret_cast<WGPUAdapterPropertiesMemoryHeaps*>(chain);
                size_t heapCount = mMemoryHeapInfo.size();
                auto* heapInfo = new WGPUMemoryHeapInfo[heapCount];
                memcpy(heapInfo, mMemoryHeapInfo.data(), sizeof(WGPUMemoryHeapInfo) * heapCount);
                // Write out the pointer and count to the heap properties out-struct.
                memoryHeapProperties->heapCount = heapCount;
                memoryHeapProperties->heapInfo = heapInfo;
                break;
            }
            case WGPUSType_AdapterPropertiesD3D: {
                auto* d3dProperties = reinterpret_cast<WGPUAdapterPropertiesD3D*>(chain);
                d3dProperties->shaderModel = mD3DProperties.shaderModel;
                break;
            }
            case WGPUSType_AdapterPropertiesVk: {
                auto* vkProperties = reinterpret_cast<WGPUAdapterPropertiesVk*>(chain);
                vkProperties->driverVersion = mVkProperties.driverVersion;
                break;
            }
            case WGPUSType_AdapterPropertiesSubgroupMatrixConfigs: {
                if (!APIHasFeature(WGPUFeatureName_ChromiumExperimentalSubgroupMatrix)) {
                    return WGPUStatus_Error;
                }

                // Copy `mSubgroupMatrixConfigs` into a new allocation.
                auto* subgroupMatrixConfigs =
                    reinterpret_cast<WGPUAdapterPropertiesSubgroupMatrixConfigs*>(chain);
                size_t configCount = mSubgroupMatrixConfigs.size();
                auto* configs = new WGPUSubgroupMatrixConfig[configCount];
                memcpy(configs, mSubgroupMatrixConfigs.data(),
                       sizeof(WGPUSubgroupMatrixConfig) * configCount);
                // Write out the pointer and count to the subgroup matrix configs out-struct.
                subgroupMatrixConfigs->configCount = configCount;
                subgroupMatrixConfigs->configs = configs;
                break;
            }
            case WGPUSType_DawnAdapterPropertiesPowerPreference: {
                auto* powerProperties =
                    reinterpret_cast<WGPUDawnAdapterPropertiesPowerPreference*>(chain);
                powerProperties->powerPreference = mPowerProperties.powerPreference;
                break;
            }
            default:
                break;
        }
        chain = chain->next;
    }

    *info = mInfo;

    // Allocate space for all strings.
    size_t allocSize =
        mVendor.length() + mArchitecture.length() + mDeviceName.length() + mDescription.length();
    absl::Span<char> outBuffer{new char[allocSize], allocSize};

    auto AddString = [&](const std::string& in, WGPUStringView* out) {
        DAWN_ASSERT(in.length() <= outBuffer.length());
        memcpy(outBuffer.data(), in.data(), in.length());
        *out = {outBuffer.data(), in.length()};
        outBuffer = outBuffer.subspan(in.length());
    };

    AddString(mVendor, &info->vendor);
    AddString(mArchitecture, &info->architecture);
    AddString(mDeviceName, &info->device);
    AddString(mDescription, &info->description);
    DAWN_ASSERT(outBuffer.empty());

    return WGPUStatus_Success;
}

WGPUFuture Adapter::APIRequestDevice(const WGPUDeviceDescriptor* descriptor,
                                     const WGPURequestDeviceCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    Ref<Device> device = client->Make<Device>(GetEventManagerHandle(), this, descriptor);
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(std::make_unique<RequestDeviceEvent>(callbackInfo, device));
    if (!tracked) {
        return {futureIDInternal};
    }

    // Ensure callbacks are not serialized as part of the command, as they cannot be passed between
    // processes.
    WGPUDeviceDescriptor wireDescriptor = {};
    if (descriptor) {
        wireDescriptor = *descriptor;
        wireDescriptor.deviceLostCallbackInfo = {};
        wireDescriptor.uncapturedErrorCallbackInfo = {};
    }

    AdapterRequestDeviceCmd cmd;
    cmd.adapterId = GetWireId();
    cmd.eventManagerHandle = GetEventManagerHandle();
    cmd.future = {futureIDInternal};
    cmd.deviceObjectHandle = device->GetWireHandle();
    cmd.deviceLostFuture = device->APIGetLostFuture();
    cmd.descriptor = &wireDescriptor;

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoAdapterRequestDeviceCallback(ObjectHandle eventManager,
                                                  WGPUFuture future,
                                                  WGPURequestDeviceStatus status,
                                                  WGPUStringView message,
                                                  const WGPULimits* limits,
                                                  uint32_t featuresCount,
                                                  const WGPUFeatureName* features) {
    return SetFutureReady<RequestDeviceEvent>(eventManager, future.id, status, message, limits,
                                              featuresCount, features);
}

WGPUInstance Adapter::APIGetInstance() const {
    dawn::ErrorLog() << "adapter.GetInstance not supported with dawn_wire.";
    return nullptr;
}

WGPUDevice Adapter::APICreateDevice(const WGPUDeviceDescriptor*) {
    dawn::ErrorLog() << "adapter.CreateDevice not supported with dawn_wire.";
    return nullptr;
}

WGPUStatus Adapter::APIGetFormatCapabilities(WGPUTextureFormat format,
                                             WGPUDawnFormatCapabilities* capabilities) {
    dawn::ErrorLog() << "adapter.GetFormatCapabilities not supported with dawn_wire.";
    return WGPUStatus_Error;
}

void APIFreeMembers(WGPUAdapterInfo info) {
    // This single delete is enough because everything is a single allocation.
    delete[] info.vendor.data;
}

void APIFreeMembers(WGPUAdapterPropertiesMemoryHeaps memoryHeapProperties) {
    delete[] memoryHeapProperties.heapInfo;
}

void APIFreeMembers(WGPUDawnDrmFormatCapabilities capabilities) {
    delete[] capabilities.properties;
}

void APIFreeMembers(WGPUSupportedFeatures supportedFeatures) {
    delete[] supportedFeatures.features;
}

void APIFreeMembers(WGPUAdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs) {
    delete[] subgroupMatrixConfigs.configs;
}

}  // namespace dawn::wire::client
