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

#include "src/dawn/node/binding/GPUDevice.h"

#include <cassert>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/Errors.h"
#include "src/dawn/node/binding/GPUAdapterInfo.h"
#include "src/dawn/node/binding/GPUBindGroup.h"
#include "src/dawn/node/binding/GPUBindGroupLayout.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUCommandBuffer.h"
#include "src/dawn/node/binding/GPUCommandEncoder.h"
#include "src/dawn/node/binding/GPUComputePipeline.h"
#include "src/dawn/node/binding/GPUPipelineLayout.h"
#include "src/dawn/node/binding/GPUQuerySet.h"
#include "src/dawn/node/binding/GPUQueue.h"
#include "src/dawn/node/binding/GPURenderBundleEncoder.h"
#include "src/dawn/node/binding/GPURenderPipeline.h"
#include "src/dawn/node/binding/GPUSampler.h"
#include "src/dawn/node/binding/GPUShaderModule.h"
#include "src/dawn/node/binding/GPUSupportedFeatures.h"
#include "src/dawn/node/binding/GPUSupportedLimits.h"
#include "src/dawn/node/binding/GPUTexture.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

namespace {

// Returns a string representation of the WGPULoggingType
const char* str(wgpu::LoggingType ty) {
    switch (ty) {
        case wgpu::LoggingType::Verbose:
            return "verbose";
        case wgpu::LoggingType::Info:
            return "info";
        case wgpu::LoggingType::Warning:
            return "warning";
        case wgpu::LoggingType::Error:
            return "error";
        default:
            return "unknown";
    }
}

// Returns a string representation of the wgpu::ErrorType
const char* str(wgpu::ErrorType ty) {
    switch (ty) {
        case wgpu::ErrorType::NoError:
            return "no error";
        case wgpu::ErrorType::Validation:
            return "validation";
        case wgpu::ErrorType::OutOfMemory:
            return "out of memory";
        case wgpu::ErrorType::Internal:
            return "internal";
        case wgpu::ErrorType::Unknown:
        default:
            return "unknown";
    }
}

// There's something broken with Node when attempting to write more than 65536 bytes to cout.
// Split the string up into writes of 4k chunks.
// Likely related: https://github.com/nodejs/node/issues/12921
void chunkedWrite(wgpu::StringView msg) {
    while (msg.length != 0) {
        int n;
        if (msg.length > 4096) {
            n = printf("%.4096s", msg.data);
        } else {
            n = printf("%.*s", static_cast<int>(msg.length), msg.data);
        }
        msg.data += n;
        msg.length -= n;
    }
}

std::optional<interop::Interface<interop::GPUError>>
createErrorFromWGPUError(Napi::Env env, wgpu::ErrorType type, wgpu::StringView message) {
    auto constructors = interop::ConstructorsFor(env);
    auto msg = Napi::String::New(env, std::string(message.data, message.length));

    switch (type) {
        case wgpu::ErrorType::NoError:
            return {};
        case wgpu::ErrorType::OutOfMemory:
            return interop::Interface<interop::GPUError>(
                constructors->GPUOutOfMemoryError_ctor.New({msg}));
        case wgpu::ErrorType::Validation:
            return interop::Interface<interop::GPUError>(
                constructors->GPUValidationError_ctor.New({msg}));
        case wgpu::ErrorType::Internal:
            return interop::Interface<interop::GPUError>(
                constructors->GPUInternalError_ctor.New({msg}));
        case wgpu::ErrorType::Unknown:
            // This error type is reserved for when translating an error type from a newer
            // implementation (e.g. the browser added a new error type) to another (e.g.
            // you're using an older version of Emscripten). It shouldn't happen in Dawn.
            break;
    }
    assert(false);
    return {};
}

Napi::Value createGPUPipelineError(Napi::Env env,
                                   wgpu::CreatePipelineAsyncStatus status,
                                   wgpu::StringView message) {
    Napi::Object pipeline_error_init = Napi::Object::New(env);
    const char* reason = "invalid";  // this is an illegal reason
    switch (status) {
        case wgpu::CreatePipelineAsyncStatus::InternalError:
            reason = "internal";
            break;
        case wgpu::CreatePipelineAsyncStatus::ValidationError:
            reason = "validation";
            break;
        case wgpu::CreatePipelineAsyncStatus::Success:
        case wgpu::CreatePipelineAsyncStatus::CallbackCancelled:
            break;
    }

    pipeline_error_init.Set("reason", reason);

    auto constructors = interop::ConstructorsFor(env);
    return constructors->GPUPipelineError_ctor.New(
        {Napi::String::New(env, std::string(message)), pipeline_error_init});
}

static std::mutex s_device_to_js_map_mutex_;
static std::unordered_map<WGPUDevice, GPUDevice*> s_device_to_js_map_;

GPUDevice* lookupGPUDeviceFromWGPUDevice(wgpu::Device device) {
    std::lock_guard<std::mutex> lock(s_device_to_js_map_mutex_);
    auto it = s_device_to_js_map_.find(device.Get());
    if (it != s_device_to_js_map_.end()) {
        return it->second;
    }
    return nullptr;
}

const char kUncapturedError[] = "uncapturederror";

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUDeviceLostInfo
////////////////////////////////////////////////////////////////////////////////
GPUDeviceLostInfo::GPUDeviceLostInfo(interop::GPUDeviceLostReason reason, std::string message)
    : reason_(reason), message_(message) {}

interop::GPUDeviceLostReason GPUDeviceLostInfo::getReason(Napi::Env env) {
    return reason_;
}

std::string GPUDeviceLostInfo::getMessage(Napi::Env) {
    return message_;
}

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUDevice
////////////////////////////////////////////////////////////////////////////////
GPUDevice::GPUDevice(Napi::Env env,
                     const wgpu::DeviceDescriptor& desc,
                     wgpu::Device device,
                     interop::Promise<interop::Interface<interop::GPUDeviceLostInfo>> lost_promise,
                     std::shared_ptr<AsyncRunner> async)
    : EventTarget(env),
      env_(env),
      device_(device),
      async_(async),
      lost_promise_(lost_promise),
      label_(CopyLabel(desc.label)) {
    device_.SetLoggingCallback([](wgpu::LoggingType type, wgpu::StringView message) {
        printf("%s:\n", str(type));
        chunkedWrite(message);
    });
    {
        std::lock_guard<std::mutex> lock(s_device_to_js_map_mutex_);
        s_device_to_js_map_.insert({device_.Get(), this});
    }
}

GPUDevice::~GPUDevice() {
    // A bit of a fudge to work around the fact that the CTS doesn't destroy GPU devices.
    // Without this, we'll get a 'Promise not resolved or rejected' fatal message as the
    // lost_promise_ is left hanging. We'll also not clean up any GPU objects before terminating the
    // process, which is not a good idea.
    if (!destroyed_) {
        lost_promise_.Discard();
        device_.Destroy();
        destroyed_ = true;
    }
    {
        std::lock_guard<std::mutex> lock(s_device_to_js_map_mutex_);
        s_device_to_js_map_.erase(device_.Get());
    }
}

void GPUDevice::handleUncapturedError(ErrorType type, wgpu::StringView message) {
    Napi::HandleScope scope(env_);

    auto error = createErrorFromWGPUError(env_, type, message);
    if (!error.has_value()) {
        fprintf(stderr,
                "GPUDevice::handleUncapturedError: Failed to create GPUError object for error type "
                "%s.\n",
                str(type));
        return;
    }

    Napi::Object event_init_dict = Napi::Object::New(env_);
    event_init_dict.Set("error", error.value());
    event_init_dict.Set("cancelable", Napi::Boolean::New(env_, true));

    auto constructors = interop::ConstructorsFor(env_);
    Napi::Object eventObj = constructors->GPUUncapturedErrorEvent_ctor.New(
        {Napi::String::New(env_, "uncapturederror"), event_init_dict});

    bool doDefault = dispatchEvent(env_, eventObj);
    if (doDefault) {
        printf("%s:\n", str(type));
        chunkedWrite(message);
    }
}

void GPUDevice::handleUncapturedErrorCallback(const wgpu::Device& device,
                                              ErrorType type,
                                              wgpu::StringView message) {
    auto gpuDevice = lookupGPUDeviceFromWGPUDevice(device.Get());
    gpuDevice->handleUncapturedError(type, message);
}

void GPUDevice::ForceLoss(wgpu::DeviceLostReason reason, const char* message) {
    if (lost_promise_.GetState() == interop::PromiseState::Pending) {
        lost_promise_.Resolve(interop::GPUDeviceLostInfo::Create<GPUDeviceLostInfo>(
            env_, interop::GPUDeviceLostReason::kUnknown, std::string(message)));
    }
    device_.ForceLoss(reason, message);
}

interop::Interface<interop::GPUSupportedFeatures> GPUDevice::getFeatures(Napi::Env env) {
    wgpu::SupportedFeatures features{};
    device_.GetFeatures(&features);
    return interop::GPUSupportedFeatures::Create<GPUSupportedFeatures>(env, env, features);
}

interop::Interface<interop::GPUSupportedLimits> GPUDevice::getLimits(Napi::Env env) {
    dawn::utils::ComboLimits limits;
    if (!device_.GetLimits(limits.GetLinked())) {
        Napi::Error::New(env, "failed to get device limits").ThrowAsJavaScriptException();
    }

    return interop::GPUSupportedLimits::Create<GPUSupportedLimits>(env, limits);
}

interop::Interface<interop::GPUAdapterInfo> GPUDevice::getAdapterInfo(Napi::Env env) {
    wgpu::AdapterInfo adapterInfo = {};
    device_.GetAdapterInfo(&adapterInfo);

    return interop::GPUAdapterInfo::Create<GPUAdapterInfo>(env, adapterInfo);
}

interop::Interface<interop::GPUQueue> GPUDevice::getQueue(Napi::Env env) {
    return interop::GPUQueue::Create<GPUQueue>(env, device_.GetQueue(), async_);
}

void GPUDevice::destroy(Napi::Env env) {
    if (lost_promise_.GetState() == interop::PromiseState::Pending) {
        lost_promise_.Resolve(interop::GPUDeviceLostInfo::Create<GPUDeviceLostInfo>(
            env_, interop::GPUDeviceLostReason::kDestroyed, "device was destroyed"));
    }
    device_.Destroy();
    destroyed_ = true;
}

interop::Interface<interop::GPUBuffer> GPUDevice::createBuffer(
    Napi::Env env,
    interop::GPUBufferDescriptor descriptor) {
    Converter conv(env);

    wgpu::BufferDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||
        !conv(desc.mappedAtCreation, descriptor.mappedAtCreation) ||
        !conv(desc.size, descriptor.size) || !conv(desc.usage, descriptor.usage)) {
        return {};
    }

    wgpu::Buffer dawnBuffer = device_.CreateBuffer(&desc);
    // Buffer creation may return nullptr if it fails to map at creation. Translate that to a
    // RangeError as required by the spec.
    if (dawnBuffer == nullptr) {
        assert(descriptor.mappedAtCreation);
        Napi::RangeError::New(env, "createBuffer failed to allocate a buffer mapped at creation.")
            .ThrowAsJavaScriptException();
        return {};
    }

    return interop::GPUBuffer::Create<GPUBuffer>(env, dawnBuffer, desc, device_, async_);
}

interop::Interface<interop::GPUTexture> GPUDevice::createTexture(
    Napi::Env env,
    interop::GPUTextureDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::TextureDescriptor desc{};
    if (!conv(desc.label, descriptor.label) || !conv(desc.usage, descriptor.usage) ||  //
        !conv(desc.size, descriptor.size) ||                                           //
        !conv(desc.dimension, descriptor.dimension) ||                                 //
        !conv(desc.mipLevelCount, descriptor.mipLevelCount) ||                         //
        !conv(desc.sampleCount, descriptor.sampleCount) ||                             //
        !conv(desc.format, descriptor.format) ||                                       //
        !conv(desc.viewFormats, desc.viewFormatCount, descriptor.viewFormats)) {
        return {};
    }

    wgpu::TextureBindingViewDimensionDescriptor texture_binding_view_dimension_desc{};
    wgpu::TextureViewDimension texture_binding_view_dimension;
    if (descriptor.textureBindingViewDimension.has_value() &&
        conv(texture_binding_view_dimension, descriptor.textureBindingViewDimension)) {
        texture_binding_view_dimension_desc.textureBindingViewDimension =
            texture_binding_view_dimension;
        desc.nextInChain =
            reinterpret_cast<wgpu::ChainedStruct*>(&texture_binding_view_dimension_desc);
    }

    return interop::GPUTexture::Create<GPUTexture>(env, device_, desc,
                                                   device_.CreateTexture(&desc));
}

interop::Interface<interop::GPUSampler> GPUDevice::createSampler(
    Napi::Env env,
    interop::GPUSamplerDescriptor descriptor) {
    Converter conv(env);

    wgpu::SamplerDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||                //
        !conv(desc.addressModeU, descriptor.addressModeU) ||  //
        !conv(desc.addressModeV, descriptor.addressModeV) ||  //
        !conv(desc.addressModeW, descriptor.addressModeW) ||  //
        !conv(desc.magFilter, descriptor.magFilter) ||        //
        !conv(desc.minFilter, descriptor.minFilter) ||        //
        !conv(desc.mipmapFilter, descriptor.mipmapFilter) ||  //
        !conv(desc.lodMinClamp, descriptor.lodMinClamp) ||    //
        !conv(desc.lodMaxClamp, descriptor.lodMaxClamp) ||    //
        !conv(desc.compare, descriptor.compare) ||            //
        !conv(desc.maxAnisotropy, descriptor.maxAnisotropy)) {
        return {};
    }
    return interop::GPUSampler::Create<GPUSampler>(env, desc, device_.CreateSampler(&desc));
}

interop::Interface<interop::GPUExternalTexture> GPUDevice::importExternalTexture(
    Napi::Env env,
    interop::GPUExternalTextureDescriptor descriptor) {
    UNIMPLEMENTED(env, {});
}

interop::Interface<interop::GPUBindGroupLayout> GPUDevice::createBindGroupLayout(
    Napi::Env env,
    interop::GPUBindGroupLayoutDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::BindGroupLayoutDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||
        !conv(desc.entries, desc.entryCount, descriptor.entries)) {
        return {};
    }

    return interop::GPUBindGroupLayout::Create<GPUBindGroupLayout>(
        env, desc, device_.CreateBindGroupLayout(&desc));
}

interop::Interface<interop::GPUPipelineLayout> GPUDevice::createPipelineLayout(
    Napi::Env env,
    interop::GPUPipelineLayoutDescriptor descriptor) {
    Converter conv(env);

    wgpu::PipelineLayoutDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||
        !conv(desc.bindGroupLayouts, desc.bindGroupLayoutCount, descriptor.bindGroupLayouts)) {
        return {};
    }

    return interop::GPUPipelineLayout::Create<GPUPipelineLayout>(
        env, desc, device_.CreatePipelineLayout(&desc));
}

interop::Interface<interop::GPUBindGroup> GPUDevice::createBindGroup(
    Napi::Env env,
    interop::GPUBindGroupDescriptor descriptor) {
    Converter conv(env);

    wgpu::BindGroupDescriptor desc{};
    if (!conv(desc.label, descriptor.label) || !conv(desc.layout, descriptor.layout) ||
        !conv(desc.entries, desc.entryCount, descriptor.entries)) {
        return {};
    }

    return interop::GPUBindGroup::Create<GPUBindGroup>(env, desc, device_.CreateBindGroup(&desc));
}

interop::Interface<interop::GPUShaderModule> GPUDevice::createShaderModule(
    Napi::Env env,
    interop::GPUShaderModuleDescriptor descriptor) {
    Converter conv(env);

    wgpu::ShaderSourceWGSL wgsl_desc{};
    wgpu::ShaderModuleDescriptor sm_desc{};
    if (!conv(wgsl_desc.code, descriptor.code) || !conv(sm_desc.label, descriptor.label)) {
        return {};
    }
    sm_desc.nextInChain = &wgsl_desc;

    // Special case for a source containing a \0. This should be an error instead of just truncating
    // the source.
    if (descriptor.code.find('\0') != std::string::npos) {
        return interop::GPUShaderModule::Create<GPUShaderModule>(
            env, sm_desc,
            device_.CreateErrorShaderModule(&sm_desc,
                                            "The WGSL shader contains an illegal character '\\0'"),
            async_);
    }

    return interop::GPUShaderModule::Create<GPUShaderModule>(
        env, sm_desc, device_.CreateShaderModule(&sm_desc), async_);
}

interop::Interface<interop::GPUComputePipeline> GPUDevice::createComputePipeline(
    Napi::Env env,
    interop::GPUComputePipelineDescriptor descriptor) {
    Converter conv(env);

    wgpu::ComputePipelineDescriptor desc{};
    if (!conv(desc, descriptor)) {
        return {};
    }

    return interop::GPUComputePipeline::Create<GPUComputePipeline>(
        env, desc, device_.CreateComputePipeline(&desc));
}

interop::Interface<interop::GPURenderPipeline> GPUDevice::createRenderPipeline(
    Napi::Env env,
    interop::GPURenderPipelineDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::RenderPipelineDescriptor desc{};
    if (!conv(desc, descriptor)) {
        return {};
    }

    return interop::GPURenderPipeline::Create<GPURenderPipeline>(
        env, desc, device_.CreateRenderPipeline(&desc));
}

interop::Promise<interop::Interface<interop::GPUComputePipeline>>
GPUDevice::createComputePipelineAsync(Napi::Env env,
                                      interop::GPUComputePipelineDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::ComputePipelineDescriptor desc{};
    if (!conv(desc, descriptor)) {
        return {env, interop::kUnusedPromise};
    }

    auto ctx = std::make_unique<AsyncContext<interop::Interface<interop::GPUComputePipeline>>>(
        env, PROMISE_INFO, async_);
    auto promise = ctx->promise;

    device_.CreateComputePipelineAsync(
        &desc, wgpu::CallbackMode::AllowProcessEvents,
        [ctx = std::move(ctx), label = CopyLabel(desc.label)](
            wgpu::CreatePipelineAsyncStatus status, wgpu::ComputePipeline pipeline,
            wgpu::StringView message) {
            switch (status) {
                case wgpu::CreatePipelineAsyncStatus::Success:
                    ctx->promise.Resolve(interop::GPUComputePipeline::Create<GPUComputePipeline>(
                        ctx->env, pipeline, label));
                    break;
                default:
                    ctx->promise.Reject(createGPUPipelineError(ctx->env, status, message));
                    break;
            }
        });

    return promise;
}

interop::Promise<interop::Interface<interop::GPURenderPipeline>>
GPUDevice::createRenderPipelineAsync(Napi::Env env,
                                     interop::GPURenderPipelineDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::RenderPipelineDescriptor desc{};
    if (!conv(desc, descriptor)) {
        return {env, interop::kUnusedPromise};
    }

    auto ctx = std::make_unique<AsyncContext<interop::Interface<interop::GPURenderPipeline>>>(
        env, PROMISE_INFO, async_);
    auto promise = ctx->promise;

    device_.CreateRenderPipelineAsync(
        &desc, wgpu::CallbackMode::AllowProcessEvents,
        [ctx = std::move(ctx), label = CopyLabel(desc.label)](
            wgpu::CreatePipelineAsyncStatus status, wgpu::RenderPipeline pipeline,
            wgpu::StringView message) {
            switch (status) {
                case wgpu::CreatePipelineAsyncStatus::Success:
                    ctx->promise.Resolve(interop::GPURenderPipeline::Create<GPURenderPipeline>(
                        ctx->env, pipeline, label));
                    break;
                default:
                    ctx->promise.Reject(createGPUPipelineError(ctx->env, status, message));
                    break;
            }
        });

    return promise;
}

interop::Interface<interop::GPUCommandEncoder> GPUDevice::createCommandEncoder(
    Napi::Env env,
    interop::GPUCommandEncoderDescriptor descriptor) {
    Converter conv(env, device_);
    wgpu::CommandEncoderDescriptor desc{};
    if (!conv(desc.label, descriptor.label)) {
        return {};
    }
    return interop::GPUCommandEncoder::Create<GPUCommandEncoder>(
        env, device_, desc, device_.CreateCommandEncoder(&desc));
}

interop::Interface<interop::GPURenderBundleEncoder> GPUDevice::createRenderBundleEncoder(
    Napi::Env env,
    interop::GPURenderBundleEncoderDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::RenderBundleEncoderDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||
        !conv(desc.colorFormats, desc.colorFormatCount, descriptor.colorFormats) ||
        !conv(desc.depthStencilFormat, descriptor.depthStencilFormat) ||
        !conv(desc.sampleCount, descriptor.sampleCount) ||
        !conv(desc.depthReadOnly, descriptor.depthReadOnly) ||
        !conv(desc.stencilReadOnly, descriptor.stencilReadOnly)) {
        return {};
    }

    return interop::GPURenderBundleEncoder::Create<GPURenderBundleEncoder>(
        env, desc, device_.CreateRenderBundleEncoder(&desc));
}

interop::Interface<interop::GPUQuerySet> GPUDevice::createQuerySet(
    Napi::Env env,
    interop::GPUQuerySetDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::QuerySetDescriptor desc{};
    if (!conv(desc.label, descriptor.label) || !conv(desc.type, descriptor.type) ||
        !conv(desc.count, descriptor.count)) {
        return {};
    }

    return interop::GPUQuerySet::Create<GPUQuerySet>(env, desc, device_.CreateQuerySet(&desc));
}

interop::Promise<interop::Interface<interop::GPUDeviceLostInfo>> GPUDevice::getLost(Napi::Env env) {
    return lost_promise_;
}

void GPUDevice::pushErrorScope(Napi::Env env, interop::GPUErrorFilter filter) {
    wgpu::ErrorFilter f;
    switch (filter) {
        case interop::GPUErrorFilter::kOutOfMemory:
            f = wgpu::ErrorFilter::OutOfMemory;
            break;
        case interop::GPUErrorFilter::kValidation:
            f = wgpu::ErrorFilter::Validation;
            break;
        case interop::GPUErrorFilter::kInternal:
            f = wgpu::ErrorFilter::Internal;
            break;
        default:
            Napi::Error::New(env, "unhandled GPUErrorFilter value").ThrowAsJavaScriptException();
            return;
    }
    device_.PushErrorScope(f);
}

interop::Promise<std::optional<interop::Interface<interop::GPUError>>> GPUDevice::popErrorScope(
    Napi::Env env) {
    auto ctx = std::make_unique<AsyncContext<std::optional<interop::Interface<interop::GPUError>>>>(
        env, PROMISE_INFO, async_);
    auto promise = ctx->promise;

    device_.PopErrorScope(
        wgpu::CallbackMode::AllowProcessEvents,
        [ctx = std::move(ctx)](wgpu::PopErrorScopeStatus status, wgpu::ErrorType type,
                               wgpu::StringView message) {
            auto env = ctx->env;
            switch (status) {
                case wgpu::PopErrorScopeStatus::Error:
                    // PopErrorScope itself failed, e.g. the error scope stack was empty.
                    ctx->promise.Reject(Errors::OperationError(env, std::string(message)));
                    return;
                case wgpu::PopErrorScopeStatus::CallbackCancelled:
                    // The instance has been dropped. Shouldn't happen except maybe during shutdown.
                    return;
                case wgpu::PopErrorScopeStatus::Success:
                    // This is the only case where `type` is set to a meaningful value.
                    break;
            }

            ctx->promise.Resolve(createErrorFromWGPUError(env, type, message));
        });

    return promise;
}

std::string GPUDevice::getLabel(Napi::Env) {
    return label_;
}

void GPUDevice::setLabel(Napi::Env, std::string value) {
    device_.SetLabel(std::string_view(value));
    label_ = value;
}

interop::EventHandler GPUDevice::getOnuncapturederror(Napi::Env env) {
    const RegisteredEventListener* listener = getAttributeRegisteredEventListener(kUncapturedError);
    return listener ? interop::EventHandler(listener->callback()) : interop::EventHandler();
}

void GPUDevice::setOnuncapturederror(Napi::Env env, interop::EventHandler value) {
    setAttributeEventListener(env, kUncapturedError, value);
}

}  // namespace wgpu::binding
