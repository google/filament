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

#include "src/dawn/node/binding/GPUQueue.h"

#include <cassert>
#include <limits>
#include <memory>
#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUCommandBuffer.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUQueue
////////////////////////////////////////////////////////////////////////////////
GPUQueue::GPUQueue(wgpu::Queue queue, std::shared_ptr<AsyncRunner> async)
    : queue_(std::move(queue)), async_(std::move(async)), label_("") {}

void GPUQueue::submit(Napi::Env env,
                      std::vector<interop::Interface<interop::GPUCommandBuffer>> commandBuffers) {
    std::vector<wgpu::CommandBuffer> bufs(commandBuffers.size());
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        bufs[i] = *commandBuffers[i].As<GPUCommandBuffer>();
    }
    Converter conv(env);
    uint32_t bufs_size;
    if (!conv(bufs_size, bufs.size())) {
        return;
    }
    queue_.Submit(bufs_size, bufs.data());
}

interop::Promise<void> GPUQueue::onSubmittedWorkDone(Napi::Env env) {
    auto ctx = std::make_unique<AsyncContext<void>>(env, PROMISE_INFO, async_);
    auto promise = ctx->promise;

    queue_.OnSubmittedWorkDone(
        wgpu::CallbackMode::AllowProcessEvents,
        [ctx = std::move(ctx)](wgpu::QueueWorkDoneStatus status, wgpu::StringView message) {
            if (status != wgpu::QueueWorkDoneStatus::Success) {
                Napi::Error::New(ctx->env, std::string(message)).ThrowAsJavaScriptException();
            }
            ctx->promise.Resolve();
        });

    return promise;
}

void GPUQueue::writeBuffer(Napi::Env env,
                           interop::Interface<interop::GPUBuffer> buffer,
                           interop::GPUSize64 bufferOffset,
                           interop::AllowSharedBufferSource data,
                           interop::GPUSize64 dataOffsetElements,
                           std::optional<interop::GPUSize64> sizeElements) {
    wgpu::Buffer buf = *buffer.As<GPUBuffer>();
    Converter::BufferSource src{};
    Converter conv(env);
    if (!conv(src, data)) {
        return;
    }

    // Note that in the JS semantics of WebGPU, writeBuffer works in number of elements of the
    // typed arrays.
    if (dataOffsetElements > uint64_t(src.size / src.bytesPerElement)) {
        binding::Errors::OperationError(env, "dataOffset is larger than data's size.")
            .ThrowAsJavaScriptException();
        return;
    }
    uint64_t dataOffset = dataOffsetElements * src.bytesPerElement;
    src.data = reinterpret_cast<uint8_t*>(src.data) + dataOffset;
    src.size -= dataOffset;

    // Size defaults to dataSize - dataOffset. Instead of computing in elements, we directly
    // use it in bytes, and convert the provided value, if any, in bytes.
    uint64_t size64 = uint64_t(src.size);
    if (sizeElements.has_value()) {
        if (sizeElements.value() > std::numeric_limits<uint64_t>::max() / src.bytesPerElement) {
            binding::Errors::OperationError(env, "size overflows.").ThrowAsJavaScriptException();
            return;
        }
        size64 = sizeElements.value() * src.bytesPerElement;
    }

    if (size64 > uint64_t(src.size)) {
        binding::Errors::OperationError(env, "size + dataOffset is larger than data's size.")
            .ThrowAsJavaScriptException();
        return;
    }

    if (size64 % 4 != 0) {
        binding::Errors::OperationError(env, "size is not a multiple of 4 bytes.")
            .ThrowAsJavaScriptException();
        return;
    }

    assert(size64 <= std::numeric_limits<size_t>::max());
    queue_.WriteBuffer(buf, bufferOffset, src.data, static_cast<size_t>(size64));
}

void GPUQueue::writeTexture(Napi::Env env,
                            interop::GPUTexelCopyTextureInfo destination,
                            interop::AllowSharedBufferSource data,
                            interop::GPUTexelCopyBufferLayout dataLayout,
                            interop::GPUExtent3D size) {
    wgpu::TexelCopyTextureInfo dst{};
    Converter::BufferSource src{};
    wgpu::TexelCopyBufferLayout layout{};
    wgpu::Extent3D sz{};
    Converter conv(env);
    if (!conv(dst, destination) ||    //
        !conv(src, data) ||           //
        !conv(layout, dataLayout) ||  //
        !conv(sz, size)) {
        return;
    }

    queue_.WriteTexture(&dst, src.data, src.size, &layout, &sz);
}

void GPUQueue::copyExternalImageToTexture(Napi::Env env,
                                          interop::GPUCopyExternalImageSourceInfo source,
                                          interop::GPUCopyExternalImageDestInfo destination,
                                          interop::GPUExtent3D copySize) {
    UNIMPLEMENTED(env);
}

std::string GPUQueue::getLabel(Napi::Env) {
    return label_;
}

void GPUQueue::setLabel(Napi::Env, std::string value) {
    queue_.SetLabel(value.c_str());
    label_ = value;
}

}  // namespace wgpu::binding
