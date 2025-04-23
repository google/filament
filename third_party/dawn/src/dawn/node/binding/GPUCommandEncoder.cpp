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

#include "src/dawn/node/binding/GPUCommandEncoder.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/GPU.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUCommandBuffer.h"
#include "src/dawn/node/binding/GPUComputePassEncoder.h"
#include "src/dawn/node/binding/GPUQuerySet.h"
#include "src/dawn/node/binding/GPURenderPassEncoder.h"
#include "src/dawn/node/binding/GPUTexture.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUCommandEncoder
////////////////////////////////////////////////////////////////////////////////
GPUCommandEncoder::GPUCommandEncoder(wgpu::Device device,
                                     const wgpu::CommandEncoderDescriptor& desc,
                                     wgpu::CommandEncoder enc)
    : device_(std::move(device)), enc_(std::move(enc)), label_(CopyLabel(desc.label)) {}

interop::Interface<interop::GPURenderPassEncoder> GPUCommandEncoder::beginRenderPass(
    Napi::Env env,
    interop::GPURenderPassDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::RenderPassDescriptor desc{};
    wgpu::RenderPassMaxDrawCount maxDrawCountDesc{};
    desc.nextInChain = &maxDrawCountDesc;

    if (!conv(desc.colorAttachments, desc.colorAttachmentCount, descriptor.colorAttachments) ||
        !conv(desc.depthStencilAttachment, descriptor.depthStencilAttachment) ||
        !conv(desc.label, descriptor.label) ||
        !conv(desc.occlusionQuerySet, descriptor.occlusionQuerySet) ||
        !conv(desc.timestampWrites, descriptor.timestampWrites) ||
        !conv(maxDrawCountDesc.maxDrawCount, descriptor.maxDrawCount)) {
        return {};
    }

    return interop::GPURenderPassEncoder::Create<GPURenderPassEncoder>(env, desc,
                                                                       enc_.BeginRenderPass(&desc));
}

interop::Interface<interop::GPUComputePassEncoder> GPUCommandEncoder::beginComputePass(
    Napi::Env env,
    interop::GPUComputePassDescriptor descriptor) {
    Converter conv(env, device_);

    wgpu::ComputePassDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||
        !conv(desc.timestampWrites, descriptor.timestampWrites)) {
        return {};
    }
    return interop::GPUComputePassEncoder::Create<GPUComputePassEncoder>(
        env, desc, enc_.BeginComputePass(&desc));
}

void GPUCommandEncoder::clearBuffer(Napi::Env env,
                                    interop::Interface<interop::GPUBuffer> buffer,
                                    interop::GPUSize64 offset,
                                    std::optional<interop::GPUSize64> size) {
    Converter conv(env);

    wgpu::Buffer b{};
    uint64_t s = wgpu::kWholeSize;
    if (!conv(b, buffer) ||  //
        !conv(s, size)) {
        return;
    }

    enc_.ClearBuffer(b, offset, s);
}

void GPUCommandEncoder::copyBufferToBuffer(Napi::Env env,
                                           interop::Interface<interop::GPUBuffer> source,
                                           interop::Interface<interop::GPUBuffer> destination,
                                           std::optional<interop::GPUSize64> size) {
    copyBufferToBuffer(env, source, 0, destination, 0, size);
}

void GPUCommandEncoder::copyBufferToBuffer(Napi::Env env,
                                           interop::Interface<interop::GPUBuffer> source,
                                           interop::GPUSize64 sourceOffset,
                                           interop::Interface<interop::GPUBuffer> destination,
                                           interop::GPUSize64 destinationOffset,
                                           std::optional<interop::GPUSize64> size) {
    Converter conv(env);

    wgpu::Buffer src{};
    wgpu::Buffer dst{};
    if (!conv(src, source) ||  //
        !conv(dst, destination)) {
        return;
    }

    // Underflow in the size calculation is acceptable because a GPU validation
    // error will be fired if the resulting size is a very large positive
    // integer. The offset is validated to be less than the buffer size before
    // we compute the remaining size in the buffer.
    uint64_t rangeSize = size.has_value() ? size.value().value : (src.GetSize() - sourceOffset);
    uint64_t s = wgpu::kWholeSize;
    if (!conv(s, rangeSize)) {
        return;
    }

    enc_.CopyBufferToBuffer(src, sourceOffset, dst, destinationOffset, s);
}

void GPUCommandEncoder::copyBufferToTexture(Napi::Env env,
                                            interop::GPUTexelCopyBufferInfo source,
                                            interop::GPUTexelCopyTextureInfo destination,
                                            interop::GPUExtent3D copySize) {
    Converter conv(env);

    wgpu::TexelCopyBufferInfo src{};
    wgpu::TexelCopyTextureInfo dst{};
    wgpu::Extent3D size{};
    if (!conv(src, source) ||       //
        !conv(dst, destination) ||  //
        !conv(size, copySize)) {
        return;
    }

    enc_.CopyBufferToTexture(&src, &dst, &size);
}

void GPUCommandEncoder::copyTextureToBuffer(Napi::Env env,
                                            interop::GPUTexelCopyTextureInfo source,
                                            interop::GPUTexelCopyBufferInfo destination,
                                            interop::GPUExtent3D copySize) {
    Converter conv(env);

    wgpu::TexelCopyTextureInfo src{};
    wgpu::TexelCopyBufferInfo dst{};
    wgpu::Extent3D size{};
    if (!conv(src, source) ||       //
        !conv(dst, destination) ||  //
        !conv(size, copySize)) {
        return;
    }

    enc_.CopyTextureToBuffer(&src, &dst, &size);
}

void GPUCommandEncoder::copyTextureToTexture(Napi::Env env,
                                             interop::GPUTexelCopyTextureInfo source,
                                             interop::GPUTexelCopyTextureInfo destination,
                                             interop::GPUExtent3D copySize) {
    Converter conv(env);

    wgpu::TexelCopyTextureInfo src{};
    wgpu::TexelCopyTextureInfo dst{};
    wgpu::Extent3D size{};
    if (!conv(src, source) ||       //
        !conv(dst, destination) ||  //
        !conv(size, copySize)) {
        return;
    }

    enc_.CopyTextureToTexture(&src, &dst, &size);
}

void GPUCommandEncoder::pushDebugGroup(Napi::Env, std::string groupLabel) {
    enc_.PushDebugGroup(groupLabel.c_str());
}

void GPUCommandEncoder::popDebugGroup(Napi::Env) {
    enc_.PopDebugGroup();
}

void GPUCommandEncoder::insertDebugMarker(Napi::Env, std::string markerLabel) {
    enc_.InsertDebugMarker(markerLabel.c_str());
}

void GPUCommandEncoder::writeTimestamp(Napi::Env env,
                                       interop::Interface<interop::GPUQuerySet> querySet,
                                       interop::GPUSize32 queryIndex) {
    Napi::TypeError::New(
        env, "writeTimestamp is no longer supported with the 'timestamp-query' feature.")
        .ThrowAsJavaScriptException();
    return;
}

void GPUCommandEncoder::resolveQuerySet(Napi::Env env,
                                        interop::Interface<interop::GPUQuerySet> querySet,
                                        interop::GPUSize32 firstQuery,
                                        interop::GPUSize32 queryCount,
                                        interop::Interface<interop::GPUBuffer> destination,
                                        interop::GPUSize64 destinationOffset) {
    Converter conv(env);

    wgpu::QuerySet q{};
    uint32_t f = 0;
    uint32_t c = 0;
    wgpu::Buffer b{};
    uint64_t o = 0;

    if (!conv(q, querySet) ||     //
        !conv(f, firstQuery) ||   //
        !conv(c, queryCount) ||   //
        !conv(b, destination) ||  //
        !conv(o, destinationOffset)) {
        return;
    }

    enc_.ResolveQuerySet(q, f, c, b, o);
}

interop::Interface<interop::GPUCommandBuffer> GPUCommandEncoder::finish(
    Napi::Env env,
    interop::GPUCommandBufferDescriptor descriptor) {
    Converter conv(env);
    wgpu::CommandBufferDescriptor desc{};
    if (!conv(desc.label, descriptor.label)) {
        return {};
    }
    return interop::GPUCommandBuffer::Create<GPUCommandBuffer>(env, desc, enc_.Finish(&desc));
}

std::string GPUCommandEncoder::getLabel(Napi::Env) {
    return label_;
}

void GPUCommandEncoder::setLabel(Napi::Env, std::string value) {
    enc_.SetLabel(std::string_view(value));
    label_ = value;
}

}  // namespace wgpu::binding
