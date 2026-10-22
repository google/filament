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

#include "src/dawn/wire/client/Instance.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <utility>

#include "dawn/wire/client/ApiObjects_autogen.h"
#include "dawn/wire/client/webgpu.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/common/WGSLFeatureMapping.h"
#include "src/dawn/wire/client/Client.h"
#include "src/dawn/wire/client/EventManager.h"
#include "src/utils/compiler.h"
#include "src/utils/log.h"
#include "tint/tint.h"

namespace dawn::wire::client {
namespace {

class RequestAdapterEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::RequestAdapter;

    RequestAdapterEvent(const WGPURequestAdapterCallbackInfo& callbackInfo, Ref<Adapter> adapter)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2),
          mAdapter(std::move(adapter)) {}

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID,
                         WGPURequestAdapterStatus status,
                         WGPUStringView message,
                         const WGPUAdapterInfo* info,
                         const WGPULimits* limits,
                         Span<const WGPUFeatureName> features) {
        DAWN_ASSERT(mAdapter != nullptr);
        mStatus = status;
        mMessage = ToString(message);
        if (status == WGPURequestAdapterStatus_Success) {
            mAdapter->SetInfo(FromAPI(info));
            mAdapter->SetLimits(FromAPI(limits));
            mAdapter->SetFeatures(FromAPI(features));
        }
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPURequestAdapterStatus_CallbackCancelled;
            mMessage = "A valid external Instance reference no longer exists.";
        }

        void* userdata1 = mUserdata1.ExtractAsDangling();
        void* userdata2 = mUserdata2.ExtractAsDangling();
        if (mCallback) {
            mCallback(mStatus,
                      mStatus == WGPURequestAdapterStatus_Success ? ReturnToAPI(std::move(mAdapter))
                                                                  : nullptr,
                      ToOutputStringView(mMessage), userdata1, userdata2);
        }
    }

    WGPURequestAdapterCallback mCallback = nullptr;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    // Note that the message is optional because we want to return nullptr when it wasn't set
    // instead of a pointer to an empty string.
    WGPURequestAdapterStatus mStatus;
    std::string mMessage;

    // The adapter is created when we call RequestAdapter(F). It is guaranteed to be alive
    // throughout the duration of a RequestAdapterEvent because the Event essentially takes
    // ownership of it until either an error occurs at which point the Event cleans it up, or it
    // returns the adapter to the user who then takes ownership as the Event goes away.
    Ref<Adapter> mAdapter;
};

wgpu::WGSLLanguageFeatureName ToWGPUWGSLLanguageFeature(tint::wgsl::LanguageFeature f) {
    switch (f) {
#define CASE(WgslName, WgpuName)                \
    case tint::wgsl::LanguageFeature::WgslName: \
        return wgpu::WGSLLanguageFeatureName::WgpuName;
        DAWN_FOREACH_WGSL_FEATURE(CASE)
#undef CASE
        case tint::wgsl::LanguageFeature::kUndefined:
            DAWN_UNREACHABLE();
    }
}

}  // anonymous namespace

static constexpr auto kSupportedFeatures =
    std::array<wgpu::InstanceFeatureName, 1>{wgpu::InstanceFeatureName::TimedWaitAny};

// Instance

Instance::Instance(const ObjectBaseParams& params)
    : ObjectBase(params), mEventManager(std::make_unique<EventManager>(0)) {}

ObjectType Instance::GetObjectType() const {
    return ObjectType::Instance;
}

EventManager& Instance::GetEventManager() const {
    DAWN_ASSERT(mEventManager != nullptr);
    return *mEventManager;
}

WireResult Instance::Initialize(const InstanceDescriptor* descriptor) {
    size_t timedWaitAnyMaxCount = 0;
    if (descriptor != nullptr) {
        bool enabledTimedWaitAny = false;
        for (auto feature : descriptor->requiredFeatures) {
            if (std::find(kSupportedFeatures.begin(), kSupportedFeatures.end(), feature) ==
                kSupportedFeatures.end()) {
                dawn::ErrorLog() << "Wire client doesn't support WGPUInstanceFeatureName("
                                 << ToAPI(feature) << ")";
                return WireResult::FatalError;
            }
            if (feature == wgpu::InstanceFeatureName::TimedWaitAny) {
                enabledTimedWaitAny = true;
            }
        }

        if (enabledTimedWaitAny) {
            if (descriptor->requiredLimits) {
                timedWaitAnyMaxCount = descriptor->requiredLimits->timedWaitAnyMaxCount;
            }
            timedWaitAnyMaxCount = std::max(timedWaitAnyMaxCount, kTimedWaitAnyMaxCountDefault);
        }

        if (descriptor->requiredLimits) {
            if (descriptor->requiredLimits->nextInChain != nullptr) {
                dawn::ErrorLog() << "Wire client doesn't support any WGPUInstanceLimits extensions";
                return WireResult::FatalError;
            }
            if (!enabledTimedWaitAny && descriptor->requiredLimits->timedWaitAnyMaxCount > 0) {
                dawn::ErrorLog() << "Wire client doesn't support non-zero timedWaitAnyMaxCount if "
                                    "WGPUInstanceFeatureName_TimedWaitAny is not enabled.";
                return WireResult::FatalError;
            }
        }

        const DawnWireWGSLControl* wgslControl = nullptr;
        const DawnWGSLBlocklist* wgslBlocklist = nullptr;
        for (const auto* chain = descriptor->nextInChain; chain; chain = chain->nextInChain) {
            switch (chain->sType) {
                case wgpu::SType::DawnWireWGSLControl:
                    wgslControl = reinterpret_cast<const DawnWireWGSLControl*>(chain);
                    break;
                case wgpu::SType::DawnWGSLBlocklist:
                    wgslBlocklist = reinterpret_cast<const DawnWGSLBlocklist*>(chain);
                    break;
                default:
                    // TODO(https://crbug.com/526537254): Use the cpp_print helpers once we
                    // deduplicate some of the autogen code w.r.t native.
                    dawn::ErrorLog() << "Wire client instance doesn't support InstanceDescriptor "
                                        "extension structure with sType ("
                                     << static_cast<uint32_t>(chain->sType) << ")";
                    return WireResult::FatalError;
            }
        }

        GatherWGSLFeatures(wgslControl, wgslBlocklist);
    }

    mEventManager = std::make_unique<EventManager>(timedWaitAnyMaxCount);

    return WireResult::Success;
}

Future Instance::APIRequestAdapter(const RequestAdapterOptions* options,
                                   const WGPURequestAdapterCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    Ref<Adapter> adapter = client->Make<Adapter>(this);
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(AcquireRef(new RequestAdapterEvent(callbackInfo, adapter)));
    if (!tracked) {
        return {futureIDInternal};
    }

    InstanceRequestAdapterCmd cmd;
    cmd.instanceId = GetWireHandle(client).id;
    cmd.future = {futureIDInternal};
    cmd.adapterObjectHandle = adapter->GetWireHandle(client);
    cmd.options = ToAPI(options);

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoInstanceRequestAdapterCallback(ObjectId instanceId,
                                                    WGPUFuture future,
                                                    WGPURequestAdapterStatus status,
                                                    WGPUStringView message,
                                                    const WGPUAdapterInfo* info,
                                                    const WGPULimits* limits,
                                                    Span<const WGPUFeatureName> features) {
    return SetFutureReady<RequestAdapterEvent>(instanceId, future.id, status, message, info, limits,
                                               features);
}

void Instance::APIProcessEvents() {
    GetEventManager().ProcessPollEvents();
}

wgpu::WaitStatus Instance::APIWaitAny(Span<FutureWaitInfo> infos, uint64_t timeoutNS) {
    return GetEventManager().WaitAny(infos, timeoutNS);
}

void Instance::GatherWGSLFeatures(const DawnWireWGSLControl* wgslControl,
                                  const DawnWGSLBlocklist* wgslBlocklist) {
    DawnWireWGSLControl defaultWgslControl{};
    if (wgslControl == nullptr) {
        wgslControl = &defaultWgslControl;
    }

    for (auto wgslFeature : tint::wgsl::kAllLanguageFeatures) {
        // Skip over testing features if we don't have the toggle to expose them.
        if (!wgslControl->enableTesting) {
            switch (wgslFeature) {
                case tint::wgsl::LanguageFeature::kChromiumTestingUnimplemented:
                case tint::wgsl::LanguageFeature::kChromiumTestingUnsafeExperimental:
                case tint::wgsl::LanguageFeature::kChromiumTestingExperimental:
                case tint::wgsl::LanguageFeature::kChromiumTestingShippedWithKillswitch:
                case tint::wgsl::LanguageFeature::kChromiumTestingShipped:
                    continue;
                default:
                    break;
            }
        }

        // Expose the feature depending on its status and wgslControl.
        bool enable = false;
        switch (tint::wgsl::GetLanguageFeatureStatus(wgslFeature)) {
            case tint::wgsl::FeatureStatus::kUnknown:
            case tint::wgsl::FeatureStatus::kUnimplemented:
                enable = false;
                break;

            case tint::wgsl::FeatureStatus::kUnsafeExperimental:
                enable = wgpu::Bool(wgslControl->enableUnsafe);
                break;
            case tint::wgsl::FeatureStatus::kExperimental:
                enable = wgpu::Bool(wgslControl->enableExperimental);
                break;

            case tint::wgsl::FeatureStatus::kShippedWithKillswitch:
            case tint::wgsl::FeatureStatus::kShipped:
                enable = true;
                break;
        }

        if (enable && wgslFeature != tint::wgsl::LanguageFeature::kUndefined) {
            mWGSLFeatures.emplace(ToWGPUWGSLLanguageFeature(wgslFeature));
        }
    }

    // Remove blocklisted features.
    if (wgslBlocklist != nullptr) {
        for (const char* name : wgslBlocklist->blocklistedFeatures) {
            tint::wgsl::LanguageFeature tintFeature = tint::wgsl::ParseLanguageFeature(name);
            if (tintFeature == tint::wgsl::LanguageFeature::kUndefined) {
                // Ignore unknown features in the blocklist.
                continue;
            }
            if (tint::wgsl::GetLanguageFeatureStatus(tintFeature) !=
                tint::wgsl::FeatureStatus::kShipped) {
                mWGSLFeatures.erase(ToWGPUWGSLLanguageFeature(tintFeature));
            }
        }
    }
}

bool Instance::APIHasWGSLLanguageFeature(wgpu::WGSLLanguageFeatureName feature) const {
    return mWGSLFeatures.contains(feature);
}

void Instance::APIGetWGSLLanguageFeatures(SupportedWGSLLanguageFeatures* features) const {
    DAWN_ASSERT(features != nullptr);
    features->features = HeapArrayFrom(mWGSLFeatures).MoveToSpan();
}

Surface* Instance::APICreateSurface(const SurfaceDescriptor* desc) const {
    dawn::ErrorLog() << "Instance::CreateSurface is not supported in the wire. Use "
                        "dawn::wire::client::WireClient::InjectSurface instead.";
    return nullptr;
}

void APISupportedWGSLLanguageFeaturesFreeMembers(
    WGPUSupportedWGSLLanguageFeatures supportedFeatures) {
    delete[] supportedFeatures.features;
}

void APISupportedInstanceFeaturesFreeMembers(WGPUSupportedInstanceFeatures supportedFeatures) {
    // Nothing to do, supportedFeatures.features is statically allocated.
}

}  // namespace dawn::wire::client

// Free-standing API functions

DAWN_WIRE_EXPORT WGPUStatus wgpuDawnWireClientGetInstanceLimits(WGPUInstanceLimits* limits) {
    DAWN_ASSERT(limits != nullptr);
    if (limits->nextInChain != nullptr) {
        dawn::ErrorLog() << "Wire client doesn't support any WGPUInstanceLimits extensions";
        return WGPUStatus_Error;
    }

    limits->timedWaitAnyMaxCount = std::numeric_limits<size_t>::max();
    return WGPUStatus_Success;
}

DAWN_WIRE_EXPORT WGPUBool wgpuDawnWireClientHasInstanceFeature(WGPUInstanceFeatureName feature) {
    return std::find(dawn::wire::client::kSupportedFeatures.begin(),
                     dawn::wire::client::kSupportedFeatures.end(),
                     dawn::wire::client::FromAPI(feature)) !=
           dawn::wire::client::kSupportedFeatures.end();
}

DAWN_WIRE_EXPORT void wgpuDawnWireClientGetInstanceFeatures(
    WGPUSupportedInstanceFeatures* features) {
    DAWN_ASSERT(features != nullptr);

    dawn::wire::client::FromAPI(features)->features = dawn::wire::client::kSupportedFeatures;
}

DAWN_WIRE_EXPORT WGPUInstance
wgpuDawnWireClientCreateInstance(WGPUInstanceDescriptor const* descriptor) {
    // Not implemented. Wire currently must be created from an existing server side instance.
    DAWN_UNREACHABLE();
    return nullptr;
}
