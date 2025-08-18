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

#ifndef SRC_DAWN_WIRE_CLIENT_DEVICE_H_
#define SRC_DAWN_WIRE_CLIENT_DEVICE_H_

#include <webgpu/webgpu.h>

#include <memory>

#include "dawn/common/LinkedList.h"
#include "dawn/common/RefCountedWithExternalCount.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ApiObjects_autogen.h"
#include "dawn/wire/client/LimitsAndFeatures.h"
#include "dawn/wire/client/ObjectBase.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::wire::client {

class Client;
class Queue;

class Device final : public RefCountedWithExternalCount<ObjectWithEventsBase> {
  public:
    Device(const ObjectBaseParams& params,
           const ObjectHandle& eventManagerHandle,
           Adapter* adapter,
           const WGPUDeviceDescriptor* descriptor);

    ObjectType GetObjectType() const override;

    void SetLimits(const WGPULimits* limits);
    void SetFeatures(const WGPUFeatureName* features, uint32_t featuresCount);

    bool IsAlive() const;

    void HandleError(WGPUErrorType errorType, WGPUStringView message);
    void HandleLogging(WGPULoggingType loggingType, WGPUStringView message);
    void HandleDeviceLost(WGPUDeviceLostReason reason, WGPUStringView message);
    class DeviceLostEvent;

    // WebGPU API
    void APISetLoggingCallback(const WGPULoggingCallbackInfo& callbackInfo);
    void APIInjectError(WGPUErrorType type, WGPUStringView message);
    WGPUFuture APIPopErrorScope(const WGPUPopErrorScopeCallbackInfo& callbackInfo);

    WGPUBuffer APICreateBuffer(const WGPUBufferDescriptor* descriptor);
    WGPUBuffer APICreateErrorBuffer(const WGPUBufferDescriptor* descriptor);
    WGPUFuture APICreateComputePipelineAsync(
        WGPUComputePipelineDescriptor const* descriptor,
        const WGPUCreateComputePipelineAsyncCallbackInfo& callbackInfo);
    WGPUFuture APICreateRenderPipelineAsync(
        WGPURenderPipelineDescriptor const* descriptor,
        const WGPUCreateRenderPipelineAsyncCallbackInfo& callbackInfo);

    WGPUStatus APIGetLimits(WGPULimits* limits) const;
    WGPUFuture APIGetLostFuture();
    bool APIHasFeature(WGPUFeatureName feature) const;
    void APIGetFeatures(WGPUSupportedFeatures* features) const;
    WGPUStatus APIGetAdapterInfo(WGPUAdapterInfo* info) const;
    WGPUAdapter APIGetAdapter() const;
    WGPUQueue APIGetQueue();

    void APIDestroy();

  private:
    void WillDropLastExternalRef() override;
    template <typename Event,
              typename Cmd,
              typename CallbackInfo = typename Event::CallbackInfo,
              typename Descriptor = decltype(std::declval<Cmd>().descriptor)>
    WGPUFuture CreatePipelineAsync(Descriptor const* descriptor, const CallbackInfo& callbackInfo);

    LimitsAndFeatures mLimitsAndFeatures;

    struct DeviceLostInfo {
        FutureID futureID = kNullFutureID;
        std::unique_ptr<TrackedEvent> event = nullptr;
    };
    DeviceLostInfo mDeviceLostInfo;

    WGPUUncapturedErrorCallbackInfo mUncapturedErrorCallbackInfo =
        WGPU_UNCAPTURED_ERROR_CALLBACK_INFO_INIT;
    WGPULoggingCallbackInfo mLoggingCallbackInfo = WGPU_LOGGING_CALLBACK_INFO_INIT;

    Ref<Adapter> mAdapter;
    Ref<Queue> mQueue;
    bool mIsAlive = true;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_DEVICE_H_
