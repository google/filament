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

#ifndef SRC_DAWN_NODE_BINDING_GPUDEVICE_H_
#define SRC_DAWN_NODE_BINDING_GPUDEVICE_H_

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <string>

#include "src/dawn/node/binding/AsyncRunner.h"
#include "src/dawn/node/binding/EventTarget.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {
// GPUDeviceLostInfo is an implementation of interop::GPUDeviceLostInfo that wraps the members.
class GPUDeviceLostInfo final : public interop::GPUDeviceLostInfo {
  public:
    GPUDeviceLostInfo(interop::GPUDeviceLostReason reason, std::string message);

    interop::GPUDeviceLostReason getReason(Napi::Env env) override;
    std::string getMessage(Napi::Env) override;

  private:
    interop::GPUDeviceLostReason reason_;
    std::string message_;
};

// GPUDevice is an implementation of interop::GPUDevice that wraps a wgpu::Device.
class GPUDevice final : public interop::GPUDevice, EventTarget {
  public:
    GPUDevice(Napi::Env env,
              const wgpu::DeviceDescriptor& desc,
              wgpu::Device device,
              interop::Promise<interop::Interface<interop::GPUDeviceLostInfo>> lost_promise,
              std::shared_ptr<AsyncRunner> async);
    ~GPUDevice() override;

    void ForceLoss(wgpu::DeviceLostReason reason, const char* message);

    // interop::GPUDevice interface compliance
    interop::Interface<interop::GPUSupportedFeatures> getFeatures(Napi::Env) override;
    interop::Interface<interop::GPUSupportedLimits> getLimits(Napi::Env) override;
    interop::Interface<interop::GPUAdapterInfo> getAdapterInfo(Napi::Env) override;
    interop::Interface<interop::GPUQueue> getQueue(Napi::Env env) override;
    void destroy(Napi::Env) override;
    interop::Interface<interop::GPUBuffer> createBuffer(
        Napi::Env env,
        interop::GPUBufferDescriptor descriptor) override;
    interop::Interface<interop::GPUTexture> createTexture(
        Napi::Env,
        interop::GPUTextureDescriptor descriptor) override;
    interop::Interface<interop::GPUSampler> createSampler(
        Napi::Env,
        interop::GPUSamplerDescriptor descriptor) override;
    interop::Interface<interop::GPUExternalTexture> importExternalTexture(
        Napi::Env,
        interop::GPUExternalTextureDescriptor descriptor) override;
    interop::Interface<interop::GPUBindGroupLayout> createBindGroupLayout(
        Napi::Env,
        interop::GPUBindGroupLayoutDescriptor descriptor) override;
    interop::Interface<interop::GPUPipelineLayout> createPipelineLayout(
        Napi::Env,
        interop::GPUPipelineLayoutDescriptor descriptor) override;
    interop::Interface<interop::GPUBindGroup> createBindGroup(
        Napi::Env,
        interop::GPUBindGroupDescriptor descriptor) override;
    interop::Interface<interop::GPUShaderModule> createShaderModule(
        Napi::Env,
        interop::GPUShaderModuleDescriptor descriptor) override;
    interop::Interface<interop::GPUComputePipeline> createComputePipeline(
        Napi::Env,
        interop::GPUComputePipelineDescriptor descriptor) override;
    interop::Interface<interop::GPURenderPipeline> createRenderPipeline(
        Napi::Env,
        interop::GPURenderPipelineDescriptor descriptor) override;
    interop::Promise<interop::Interface<interop::GPUComputePipeline>> createComputePipelineAsync(
        Napi::Env env,
        interop::GPUComputePipelineDescriptor descriptor) override;
    interop::Promise<interop::Interface<interop::GPURenderPipeline>> createRenderPipelineAsync(
        Napi::Env env,
        interop::GPURenderPipelineDescriptor descriptor) override;
    interop::Interface<interop::GPUCommandEncoder> createCommandEncoder(
        Napi::Env env,
        interop::GPUCommandEncoderDescriptor descriptor) override;
    interop::Interface<interop::GPURenderBundleEncoder> createRenderBundleEncoder(
        Napi::Env,
        interop::GPURenderBundleEncoderDescriptor descriptor) override;
    interop::Interface<interop::GPUQuerySet> createQuerySet(
        Napi::Env,
        interop::GPUQuerySetDescriptor descriptor) override;
    interop::Promise<interop::Interface<interop::GPUDeviceLostInfo>> getLost(
        Napi::Env env) override;
    void pushErrorScope(Napi::Env, interop::GPUErrorFilter filter) override;
    interop::Promise<std::optional<interop::Interface<interop::GPUError>>> popErrorScope(
        Napi::Env env) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;
    interop::EventHandler getOnuncapturederror(Napi::Env) override;
    void setOnuncapturederror(Napi::Env, interop::EventHandler value) override;

    void handleUncapturedError(ErrorType type, wgpu::StringView message);
    static void handleUncapturedErrorCallback(const wgpu::Device& device,
                                              ErrorType type,
                                              wgpu::StringView message);

  private:
    Napi::Env env_;
    wgpu::Device device_;
    std::shared_ptr<AsyncRunner> async_;

    // This promise's JS object lives as long as the device because it is stored in .lost
    // of the wrapper JS object.
    interop::Promise<interop::Interface<interop::GPUDeviceLostInfo>> lost_promise_;
    std::string label_;

    bool destroyed_ = false;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUDEVICE_H_
