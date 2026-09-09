// Copyright 2019 The Dawn & Tint Authors
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

#include "src/dawn/wire/client/Device.h"

#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "dawn/wire/client/ApiObjects_autogen.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/wire/client/Client.h"
#include "src/dawn/wire/client/EventManager.h"
#include "src/utils/assert.h"
#include "src/utils/log.h"

namespace dawn::wire::client {
namespace {

class PopErrorScopeEvent final : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::PopErrorScope;

    explicit PopErrorScopeEvent(const WGPUPopErrorScopeCallbackInfo& callbackInfo)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2) {}

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID,
                         WGPUPopErrorScopeStatus status,
                         WGPUErrorType errorType,
                         WGPUStringView message) {
        mStatus = status;
        mType = errorType;
        mMessage = ToString(message);
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUPopErrorScopeStatus_CallbackCancelled;
            mMessage = "";
        }
        if (mCallback) {
            mCallback(mStatus, mType, ToOutputStringView(mMessage), mUserdata1.ExtractAsDangling(),
                      mUserdata2.ExtractAsDangling());
        }
    }

    WGPUPopErrorScopeCallback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    WGPUPopErrorScopeStatus mStatus = {};
    WGPUErrorType mType = WGPUErrorType_NoError;
    std::string mMessage;
};

template <typename PipelineT, EventType Type, typename CallbackInfoT, typename CreateErrorCmdT>
class CreatePipelineEventBase : public TrackedEvent {
  public:
    // Export these types upwards for ease of use.
    using Pipeline = PipelineT;
    using CallbackInfo = CallbackInfoT;

    static constexpr EventType kType = Type;

    CreatePipelineEventBase(const CallbackInfo& callbackInfo,
                            Ref<Device> device,
                            Ref<Pipeline> pipeline,
                            WGPUStringView label)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2),
          mLabel(ToString(label)),
          mDevice(std::move(device)),
          mPipeline(std::move(pipeline)) {
        DAWN_ASSERT(mDevice != nullptr);
        DAWN_ASSERT(mPipeline != nullptr);
    }

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID,
                         WGPUCreatePipelineAsyncStatus status,
                         WGPUStringView message) {
        DAWN_ASSERT(mPipeline != nullptr);
        mStatus = status;
        mMessage = ToString(message);
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        auto userdata1 = mUserdata1.ExtractAsDangling();
        auto userdata2 = mUserdata2.ExtractAsDangling();

        if (mCallback == nullptr) {
            return;
        }

        if (completionType == EventCompletionType::Shutdown) {
            mStatus = WGPUCreatePipelineAsyncStatus_CallbackCancelled;
            mMessage = "A valid external Instance reference no longer exists.";
        } else if (mDevice->IsKnownLost()) {
            mPipeline =
                mDevice->CreateErrorPipeline<Pipeline, CreateErrorCmdT>(ToOutputStringView(mLabel));
            mStatus = WGPUCreatePipelineAsyncStatus_Success;
            mMessage = "";
        }

        mCallback(mStatus,
                  mStatus == WGPUCreatePipelineAsyncStatus_Success
                      ? ReturnToAPI(std::move(mPipeline))
                      : nullptr,
                  ToOutputStringView(mMessage), userdata1, userdata2);
    }

    using Callback = decltype(std::declval<CallbackInfo>().callback);
    Callback mCallback;
    raw_ptr<void> mUserdata1;
    raw_ptr<void> mUserdata2;

    WGPUCreatePipelineAsyncStatus mStatus = WGPUCreatePipelineAsyncStatus_Success;
    std::string mMessage;

    std::string mLabel;
    Ref<Device> mDevice;
    Ref<Pipeline> mPipeline;
};

using CreateComputePipelineEvent =
    CreatePipelineEventBase<ComputePipeline,
                            EventType::CreateComputePipeline,
                            WGPUCreateComputePipelineAsyncCallbackInfo,
                            DeviceCreateErrorComputePipelineCmd>;
using CreateRenderPipelineEvent = CreatePipelineEventBase<RenderPipeline,
                                                          EventType::CreateRenderPipeline,
                                                          WGPUCreateRenderPipelineAsyncCallbackInfo,
                                                          DeviceCreateErrorRenderPipelineCmd>;

}  // namespace

class Device::DeviceLostEvent : public TrackedEvent {
  public:
    static constexpr EventType kType = EventType::DeviceLost;

    DeviceLostEvent(const WGPUDeviceLostCallbackInfo& callbackInfo, Ref<Device> device)
        : TrackedEvent(callbackInfo.mode),
          mCallback(callbackInfo.callback),
          mUserdata1(callbackInfo.userdata1),
          mUserdata2(callbackInfo.userdata2),
          mDevice(std::move(device)) {
        DAWN_ASSERT(mDevice != nullptr);
    }

    EventType GetType() override { return kType; }

    WireResult ReadyHook(FutureID futureID, WGPUDeviceLostReason reason, WGPUStringView message) {
        mState.Use([&](auto state) {
            if (state->message.empty()) {
                state->reason = reason;
                state->message = ToString(message);
            }
        });
        return WireResult::Success;
    }

  private:
    void CompleteImpl(FutureID futureID, EventCompletionType completionType) override {
        WGPUDeviceLostReason reason;
        std::string message;

        mState.Use([&](auto state) {
            if (completionType == EventCompletionType::Shutdown) {
                state->reason = WGPUDeviceLostReason_CallbackCancelled;
                state->message = "A valid external Instance reference no longer exists.";
            }
            reason = state->reason;
            message = state->message;
        });

        // The uncaptured error and logging callbacks are spontaneous and must not be called
        // after we call the device lost's |mCallback| below, so we clear them and wait for them to
        // be no longer referenced before moving forwards.
        mDevice->mIsLost = true;
        mDevice->mCallbackInfos.Clear();

        void* userdata1 = mUserdata1.ExtractAsDangling();
        void* userdata2 = mUserdata2.ExtractAsDangling();

        if (mCallback != nullptr) {
            const auto device =
                reason != WGPUDeviceLostReason_FailedCreation ? ToAPI(mDevice.Get()) : nullptr;
            mCallback(&device, reason, ToOutputStringView(message), userdata1, userdata2);
        }
    }

    WGPUDeviceLostCallback mCallback = nullptr;
    raw_ptr<void> mUserdata1 = nullptr;
    raw_ptr<void> mUserdata2 = nullptr;

    struct State {
        WGPUDeviceLostReason reason;
        std::string message;
    };
    MutexProtected<State> mState;

    // Strong reference to the device so that when we call the callback we can pass the device.
    Ref<Device> mDevice;
};

Device::Device(const ObjectBaseParams& params,
               Ref<Instance> instance,
               Adapter* adapter,
               const DeviceDescriptor* descriptor)
    : RefCountedWithExternalCount<ObjectWithEventsBase>(params, std::move(instance)),
      mDeviceLostInfo(AcquireRef(
          new DeviceLostEvent(GetDeviceLostCallbackInfoOrDefault(ToAPI(descriptor)), this))),
      mCallbackInfos(ToAPI(descriptor)),
      mAdapter(adapter) {}

ObjectType Device::GetObjectType() const {
    return ObjectType::Device;
}

bool Device::IsDestroyed() const {
    return mIsDestroyed;
}

bool Device::IsKnownLost() const {
    return mIsLost;
}

Queue* Device::GetQueue() {
    // The queue is lazily created because if a Device is created by Reserve/Inject, we cannot send
    // the GetQueue message until it has been injected on the Server. It cannot happen immediately
    // on construction.
    if (mQueue == nullptr) {
        // Get the primary queue for this device.
        Client* client = GetClient();
        mQueue = client->Make<Queue>(GetInstance());

        DeviceGetQueueCmd cmd;
        cmd.self = ToAPI(this);
        cmd.result = mQueue->GetWireHandle(client);
        client->SerializeCommand(cmd);
    }
    return mQueue.Get();
}

const LimitsAndFeatures& Device::GetLimitsAndFeatures() const {
    return mLimitsAndFeatures;
}

void Device::WillDropLastExternalRef() {
    if (IsRegistered()) {
        APIDestroy();
    }
}

wgpu::Status Device::APIGetLimits(Limits* limits) const {
    return mLimitsAndFeatures.GetLimits(limits);
}

bool Device::APIHasFeature(wgpu::FeatureName feature) const {
    return mLimitsAndFeatures.HasFeature(feature);
}

void Device::APIGetFeatures(SupportedFeatures* features) const {
    mLimitsAndFeatures.ToSupportedFeatures(features);
}

wgpu::Status Device::APIGetAdapterInfo(AdapterInfo* adapterInfo) const {
    return mAdapter->APIGetInfo(adapterInfo);
}

void Device::SetLimits(const Limits* limits) {
    mLimitsAndFeatures.SetLimits(limits);
}

void Device::SetFeatures(Span<const wgpu::FeatureName> features) {
    mLimitsAndFeatures.SetFeatures(features);
}

void Device::HandleError(WGPUErrorType errorType, WGPUStringView message) {
    const auto device = ToAPI(this);
    mCallbackInfos.CallErrorCallback(&device, errorType, message);
}

void Device::HandleLogging(WGPULoggingType loggingType, WGPUStringView message) {
    mCallbackInfos.CallLoggingCallback(loggingType, message);
}

void Device::HandleDeviceLost(WGPUDeviceLostReason reason, WGPUStringView message) {
    FutureID futureID = APIGetLostFuture().id;
    auto wireStatus = GetEventManager().SetFutureReady<DeviceLostEvent>(futureID, reason, message);
    DAWN_CHECK(wireStatus == WireResult::Success);
}

Future Device::APIGetLostFuture() {
    // Lazily track the device lost event so that event ordering w.r.t RequestDevice is correct.
    if (const auto* e = std::get_if<Ref<TrackedEvent>>(&mDeviceLostInfo)) {
        Ref<TrackedEvent> event = *e;
        auto [futureID, _] = GetEventManager().TrackEvent(std::move(event));
        mDeviceLostInfo = futureID;
    }
    return {std::get<FutureID>(mDeviceLostInfo)};
}

void Device::APISetLoggingCallback(const WGPULoggingCallbackInfo& callbackInfo) {
    if (!mIsLost) {
        mCallbackInfos.SetLoggingCallbackInfo(callbackInfo);
    }
}

WireResult Client::DoDeviceLostCallback(ObjectId instanceId,
                                        WGPUFuture future,
                                        WGPUDeviceLostReason reason,
                                        WGPUStringView message) {
    return SetFutureReady<Device::DeviceLostEvent>(instanceId, future.id, reason, message);
}

Future Device::APIPopErrorScope(const WGPUPopErrorScopeCallbackInfo& callbackInfo) {
    Client* client = GetClient();
    auto [futureIDInternal, tracked] =
        GetEventManager().TrackEvent(AcquireRef(new PopErrorScopeEvent(callbackInfo)));
    if (!tracked) {
        return {futureIDInternal};
    }

    DevicePopErrorScopeCmd cmd;
    cmd.deviceId = GetWireHandle(client).id;
    cmd.instanceId = GetInstance()->GetWireHandle(client).id;
    cmd.future = {futureIDInternal};
    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

WireResult Client::DoDevicePopErrorScopeCallback(ObjectId instanceId,
                                                 WGPUFuture future,
                                                 WGPUPopErrorScopeStatus status,
                                                 WGPUErrorType errorType,
                                                 WGPUStringView message) {
    return SetFutureReady<PopErrorScopeEvent>(instanceId, future.id, status, errorType, message);
}

void Device::APIInjectError(wgpu::ErrorType type, StringView message) {
    DeviceInjectErrorCmd cmd;
    cmd.self = ToAPI(this);
    cmd.type = ToAPI(type);
    cmd.message = ToAPI(message);
    GetClient()->SerializeCommand(cmd);
}

Buffer* Device::APICreateBuffer(const BufferDescriptor* descriptor) {
    return Buffer::Create(this, descriptor);
}

Buffer* Device::APICreateErrorBuffer(const BufferDescriptor* descriptor) {
    return Buffer::CreateError(this, descriptor);
}

ResourceTable* Device::APICreateResourceTable(const ResourceTableDescriptor* descriptor) {
    return ResourceTable::Create(this, descriptor);
}

ShaderModule* Device::APICreateShaderModule(const ShaderModuleDescriptor* descriptor) {
    return ShaderModule::Create(this, descriptor);
}

Texture* Device::APICreateTexture(const TextureDescriptor* descriptor) {
    return Texture::Create(this, descriptor);
}

Texture* Device::APICreateErrorTexture(const TextureDescriptor* descriptor) {
    return Texture::CreateError(this, descriptor);
}

Adapter* Device::APIGetAdapter() const {
    Ref<Adapter> adapter = mAdapter;
    return ReturnToAPI2(std::move(adapter));
}

Queue* Device::APIGetQueue() {
    Ref<Queue> queue = GetQueue();
    return ReturnToAPI2(std::move(queue));
}

template <typename Event, typename Cmd, typename CallbackInfo, typename Descriptor>
Future Device::CreatePipelineAsync(Descriptor const* descriptor, const CallbackInfo& callbackInfo) {
    using Pipeline = typename Event::Pipeline;

    Client* client = GetClient();
    Ref<Pipeline> pipeline = client->Make<Pipeline>();
    auto [futureIDInternal, tracked] = GetEventManager().TrackEvent(AcquireRef(new Event(
        callbackInfo, this, pipeline, descriptor ? ToAPI(descriptor->label) : WGPUStringView{})));
    if (!tracked) {
        return {futureIDInternal};
    }

    Cmd cmd;
    cmd.deviceId = GetWireHandle(client).id;
    cmd.descriptor = ToAPI(descriptor);
    cmd.instanceId = GetInstance()->GetWireHandle(client).id;
    cmd.future = {futureIDInternal};
    cmd.pipelineObjectHandle = pipeline->GetWireHandle(client);

    client->SerializeCommand(cmd);
    return {futureIDInternal};
}

Future Device::APICreateComputePipelineAsync(
    const ComputePipelineDescriptor* descriptor,
    const WGPUCreateComputePipelineAsyncCallbackInfo& callbackInfo) {
    return CreatePipelineAsync<CreateComputePipelineEvent, DeviceCreateComputePipelineAsyncCmd>(
        descriptor, callbackInfo);
}

WireResult Client::DoDeviceCreateComputePipelineAsyncCallback(ObjectId instanceId,
                                                              WGPUFuture future,
                                                              WGPUCreatePipelineAsyncStatus status,
                                                              WGPUStringView message) {
    return SetFutureReady<CreateComputePipelineEvent>(instanceId, future.id, status, message);
}

Future Device::APICreateRenderPipelineAsync(
    const RenderPipelineDescriptor* descriptor,
    const WGPUCreateRenderPipelineAsyncCallbackInfo& callbackInfo) {
    return CreatePipelineAsync<CreateRenderPipelineEvent, DeviceCreateRenderPipelineAsyncCmd>(
        descriptor, callbackInfo);
}

WireResult Client::DoDeviceCreateRenderPipelineAsyncCallback(ObjectId instanceId,
                                                             WGPUFuture future,
                                                             WGPUCreatePipelineAsyncStatus status,
                                                             WGPUStringView message) {
    return SetFutureReady<CreateRenderPipelineEvent>(instanceId, future.id, status, message);
}

void Device::APIDestroy() {
    mIsDestroyed = true;
    HandleDeviceLost(WGPUDeviceLostReason_Destroyed, ToOutputStringView("Device was destroyed."));

    DeviceDestroyCmd cmd;
    cmd.self = ToAPI(this);
    GetClient()->SerializeCommand(cmd);
}

template <typename PipelineT, typename CmdT>
Ref<PipelineT> Device::CreateErrorPipeline(WGPUStringView label) {
    Client* client = GetClient();
    Ref<PipelineT> pipeline = client->Make<PipelineT>();

    CmdT cmd;
    cmd.self = ToAPI(this);
    cmd.label = label;
    cmd.result = pipeline->GetWireHandle(client);
    client->SerializeCommand(cmd);

    return pipeline;
}

template Ref<ComputePipeline>
Device::CreateErrorPipeline<ComputePipeline, DeviceCreateErrorComputePipelineCmd>(
    WGPUStringView label);
template Ref<RenderPipeline>
Device::CreateErrorPipeline<RenderPipeline, DeviceCreateErrorRenderPipelineCmd>(
    WGPUStringView label);

}  // namespace dawn::wire::client
