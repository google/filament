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

#include "src/dawn/wire/client/Adapter.h"

#include <memory>
#include <string>
#include <utility>

#include "absl/types/span.h"  // TODO(343500108): Use std::span when we have C++20.
#include "dawn/wire/client/webgpu.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/wire/client/Client.h"
#include "src/utils/compiler.h"
#include "src/utils/log.h"

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
                         Span<const WGPUFeatureName> features) {
        DAWN_ASSERT(mDevice != nullptr);
        mStatus = status;
        mMessage = ToString(message);
        if (status == WGPURequestDeviceStatus_Success) {
            mDevice->SetLimits(FromAPI(limits));
            mDevice->SetFeatures(FromAPI(features));
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

wgpu::Status Adapter::APIGetLimits(Limits* limits) const {
    return mLimitsAndFeatures.GetLimits(limits);
}

bool Adapter::APIHasFeature(wgpu::FeatureName feature) const {
    return mLimitsAndFeatures.HasFeature(feature);
}

void Adapter::APIGetFeatures(SupportedFeatures* features) const {
    mLimitsAndFeatures.ToSupportedFeatures(features);
}

void Adapter::SetLimits(const Limits* limits) {
    return mLimitsAndFeatures.SetLimits(limits);
}

void Adapter::SetFeatures(Span<const wgpu::FeatureName> features) {
    return mLimitsAndFeatures.SetFeatures(features);
}

void Adapter::SetInfo(const AdapterInfo* info) {
    mInfo.backendType = info->backendType;
    mInfo.adapterType = info->adapterType;
    mInfo.vendorID = info->vendorID;
    mInfo.deviceID = info->deviceID;
    mInfo.subgroupMinSize = info->subgroupMinSize;
    mInfo.subgroupMaxSize = info->subgroupMaxSize;

    // Deep copy the string pointed out by info. StringViews are all explicitly sized by the wire.
    mVendor = info->vendor;
    mArchitecture = info->architecture;
    mDeviceName = info->device;
    mDescription = info->description;

    mInfo.nextInChain = nullptr;

    // Loop through the chained struct.
    for (const auto* chain = info->nextInChain; chain; chain = chain->nextInChain) {
        switch (chain->sType) {
            case wgpu::SType::AdapterPropertiesMemoryHeaps: {
                // Make a copy of the heap info in `mMemoryHeapInfo`.
                const auto* memoryHeapProperties =
                    reinterpret_cast<const AdapterPropertiesMemoryHeaps*>(chain);
                mMemoryHeapInfo.assign(memoryHeapProperties->heapInfo.begin(),
                                       memoryHeapProperties->heapInfo.end());
                break;
            }
            case wgpu::SType::AdapterPropertiesD3D: {
                const auto* d3dProperties = reinterpret_cast<const AdapterPropertiesD3D*>(chain);
                mD3DProperties.shaderModel = d3dProperties->shaderModel;
                break;
            }
            case wgpu::SType::AdapterPropertiesVk: {
                const auto* vkProperties = reinterpret_cast<const AdapterPropertiesVk*>(chain);
                mVkProperties.driverVersion = vkProperties->driverVersion;
                break;
            }
            case wgpu::SType::AdapterPropertiesSubgroupMatrixConfigs: {
                // Make a copy of the heap info in `mSubgroupMatrixConfigs`.
                const auto* subgroupMatrixConfigs =
                    reinterpret_cast<const AdapterPropertiesSubgroupMatrixConfigs*>(chain);
                mSubgroupMatrixConfigs.assign(subgroupMatrixConfigs->configs.begin(),
                                              subgroupMatrixConfigs->configs.end());
                break;
            }
            case wgpu::SType::DawnAdapterPropertiesPowerPreference: {
                const auto* powerProperties =
                    reinterpret_cast<const DawnAdapterPropertiesPowerPreference*>(chain);
                mPowerProperties.powerPreference = powerProperties->powerPreference;
                break;
            }
            default:
                DAWN_UNREACHABLE();
                break;
        }
    }
}

wgpu::Status Adapter::APIGetInfo(AdapterInfo* info) const {
    // Loop through the chained struct.
    for (auto* chain = info->nextInChain; chain; chain = chain->nextInChain) {
        switch (chain->sType) {
            case wgpu::SType::AdapterPropertiesMemoryHeaps: {
                // Copy `mMemoryHeapInfo` into a new allocation.
                auto* memoryHeapProperties = reinterpret_cast<AdapterPropertiesMemoryHeaps*>(chain);
                memoryHeapProperties->heapInfo = HeapArrayFrom(mMemoryHeapInfo).MoveToSpan();
                break;
            }
            case wgpu::SType::AdapterPropertiesD3D: {
                auto* d3dProperties = reinterpret_cast<AdapterPropertiesD3D*>(chain);
                d3dProperties->shaderModel = mD3DProperties.shaderModel;
                break;
            }
            case wgpu::SType::AdapterPropertiesVk: {
                auto* vkProperties = reinterpret_cast<AdapterPropertiesVk*>(chain);
                vkProperties->driverVersion = mVkProperties.driverVersion;
                break;
            }
            case wgpu::SType::AdapterPropertiesSubgroupMatrixConfigs: {
                if (!APIHasFeature(wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix)) {
                    return wgpu::Status::Error;
                }

                // Copy `mSubgroupMatrixConfigs` into a new allocation.
                auto* subgroupMatrixConfigs =
                    reinterpret_cast<AdapterPropertiesSubgroupMatrixConfigs*>(chain);
                subgroupMatrixConfigs->configs = HeapArrayFrom(mSubgroupMatrixConfigs).MoveToSpan();
                break;
            }
            case wgpu::SType::DawnAdapterPropertiesPowerPreference: {
                auto* powerProperties =
                    reinterpret_cast<DawnAdapterPropertiesPowerPreference*>(chain);
                powerProperties->powerPreference = mPowerProperties.powerPreference;
                break;
            }
            default:
                break;
        }
    }

    info->backendType = mInfo.backendType;
    info->adapterType = mInfo.adapterType;
    info->vendorID = mInfo.vendorID;
    info->deviceID = mInfo.deviceID;
    info->subgroupMinSize = mInfo.subgroupMinSize;
    info->subgroupMaxSize = mInfo.subgroupMaxSize;

    // Allocate space for all strings.
    size_t allocSize =
        mVendor.length() + mArchitecture.length() + mDeviceName.length() + mDescription.length();
    // SAFETY: The data in the buffer will be initialized by AddString below.
    Span<char> outBuffer = DAWN_UNSAFE_BUFFERS(HeapArray<char>::Uninit(allocSize).MoveToSpan());

    auto AddString = [&](const std::string& in, StringView* out) {
        DAWN_ASSERT(in.length() <= outBuffer.size());
        outBuffer.CopyPrefixFrom(in);
        *out = {outBuffer.data(), in.length()};
        outBuffer = outBuffer.subspan(in.length());
    };

    AddString(mVendor, &info->vendor);
    AddString(mArchitecture, &info->architecture);
    AddString(mDeviceName, &info->device);
    AddString(mDescription, &info->description);
    DAWN_ASSERT(outBuffer.empty());

    return wgpu::Status::Success;
}

Future Adapter::APIRequestDevice(const DeviceDescriptor* descriptor,
                                 const WGPURequestDeviceCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    Ref<Device> device = client->Make<Device>(GetInstance(), this, descriptor);
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(AcquireRef(new RequestDeviceEvent(callbackInfo, device)));
    if (!tracked) {
        return {futureIDInternal};
    }

    // Ensure callbacks are not serialized as part of the command, as they cannot be passed between
    // processes.
    DeviceDescriptor wireDescriptor = {};
    if (descriptor) {
        wireDescriptor = *descriptor;
        wireDescriptor.deviceLostCallbackInfo = {};
        wireDescriptor.uncapturedErrorCallbackInfo = {};
    }

    AdapterRequestDeviceCmd cmd;
    cmd.adapterId = GetWireHandle(client).id;
    cmd.instanceId = GetInstance()->GetWireHandle(client).id;
    cmd.future = {futureIDInternal};
    cmd.deviceObjectHandle = device->GetWireHandle(client);
    cmd.deviceLostFuture = ToAPI(device->APIGetLostFuture());
    cmd.descriptor = ToAPI(&wireDescriptor);

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoAdapterRequestDeviceCallback(ObjectId instanceId,
                                                  WGPUFuture future,
                                                  WGPURequestDeviceStatus status,
                                                  WGPUStringView message,
                                                  const WGPULimits* limits,
                                                  Span<const WGPUFeatureName> features) {
    return SetFutureReady<RequestDeviceEvent>(instanceId, future.id, status, message, limits,
                                              features);
}

Instance* Adapter::APIGetInstance() const {
    Ref<Instance> instance = GetInstance();
    return ReturnToAPI2(std::move(instance));
}

Device* Adapter::APICreateDevice(const DeviceDescriptor*) {
    dawn::ErrorLog() << "adapter.CreateDevice not supported with dawn_wire.";
    return nullptr;
}

wgpu::Status Adapter::APIGetFormatCapabilities(wgpu::TextureFormat format,
                                               DawnFormatCapabilities* capabilities) {
    dawn::ErrorLog() << "adapter.GetFormatCapabilities not supported with dawn_wire.";
    return wgpu::Status::Error;
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

void APISupportedFeaturesFreeMembers(WGPUSupportedFeatures supportedFeatures) {
    delete[] supportedFeatures.features;
}

void APIAdapterPropertiesSubgroupMatrixConfigsFreeMembers(
    WGPUAdapterPropertiesSubgroupMatrixConfigs subgroupMatrixConfigs) {
    delete[] subgroupMatrixConfigs.configs;
}

}  // namespace dawn::wire::client
