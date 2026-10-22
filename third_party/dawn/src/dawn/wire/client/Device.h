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
#include <optional>
#include <variant>

#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ApiObjects_autogen.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/LinkedList.h"
#include "src/dawn/common/RefCountedWithExternalCount.h"
#include "src/dawn/common/WGPUDeviceCallbackInfos.h"
#include "src/dawn/wire/client/LimitsAndFeatures.h"
#include "src/dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class Client;
class Queue;

class Device final : public RefCountedWithExternalCount<ObjectWithEventsBase> {
  public:
    Device(const ObjectBaseParams& params,
           Ref<Instance> instance,
           Adapter* adapter,
           const DeviceDescriptor* descriptor);

    ObjectType GetObjectType() const override;

    void SetLimits(const Limits* limits);
    void SetFeatures(Span<const wgpu::FeatureName> features);

    bool IsDestroyed() const;
    bool IsKnownLost() const;
    Queue* GetQueue();
    const LimitsAndFeatures& GetLimitsAndFeatures() const;

    void HandleError(WGPUErrorType errorType, WGPUStringView message);
    void HandleLogging(WGPULoggingType loggingType, WGPUStringView message);
    void HandleDeviceLost(WGPUDeviceLostReason reason, WGPUStringView message);
    class DeviceLostEvent;

    // WebGPU API
    void APISetLoggingCallback(const WGPULoggingCallbackInfo& callbackInfo);
    void APIInjectError(wgpu::ErrorType type, StringView message);
    Future APIPopErrorScope(const WGPUPopErrorScopeCallbackInfo& callbackInfo);

    template <typename PipelineT, typename CmdT>
    Ref<PipelineT> CreateErrorPipeline(WGPUStringView label);

    Buffer* APICreateBuffer(const BufferDescriptor* descriptor);
    Buffer* APICreateErrorBuffer(const BufferDescriptor* descriptor);
    Future APICreateComputePipelineAsync(
        const ComputePipelineDescriptor* descriptor,
        const WGPUCreateComputePipelineAsyncCallbackInfo& callbackInfo);
    Future APICreateRenderPipelineAsync(
        const RenderPipelineDescriptor* descriptor,
        const WGPUCreateRenderPipelineAsyncCallbackInfo& callbackInfo);
    ResourceTable* APICreateResourceTable(const ResourceTableDescriptor* descriptor);
    ShaderModule* APICreateShaderModule(const ShaderModuleDescriptor* descriptor);
    Texture* APICreateTexture(const TextureDescriptor* descriptor);
    Texture* APICreateErrorTexture(const TextureDescriptor* descriptor);

    wgpu::Status APIGetLimits(Limits* limits) const;
    Future APIGetLostFuture();
    bool APIHasFeature(wgpu::FeatureName feature) const;
    void APIGetFeatures(SupportedFeatures* features) const;
    wgpu::Status APIGetAdapterInfo(AdapterInfo* info) const;
    Adapter* APIGetAdapter() const;
    Queue* APIGetQueue();

    void APIDestroy();

  private:
    void WillDropLastExternalRef() override;
    template <typename Event,
              typename Cmd,
              typename CallbackInfo = typename Event::CallbackInfo,
              typename Descriptor = decltype(std::declval<Cmd>().descriptor)>
    Future CreatePipelineAsync(Descriptor const* descriptor, const CallbackInfo& callbackInfo);

    LimitsAndFeatures mLimitsAndFeatures;
    std::variant<Ref<TrackedEvent>, FutureID> mDeviceLostInfo;

    WGPUDeviceCallbackInfos mCallbackInfos;

    Ref<Adapter> mAdapter;
    Ref<Queue> mQueue;

    // Note that we differentiate between destroyed and lost in that destroyed is a client-side
    // state that is immediately set once `APIDestroy()` is called, whereas lost is a server-side
    // state that is updated only once our lost callback has been completed. The destroyed state is,
    // as of writing, only really needed for buffer mapping because device.Destroy() is supposed to
    // explicitly unmap all buffers, but don't currently handle that exactly in the wire, so the
    // destroyed state is used to help simulate that. Pretty much everything else should be relying
    // on the lost state.
    bool mIsDestroyed = false;
    bool mIsLost = false;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_DEVICE_H_
