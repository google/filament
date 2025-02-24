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

#include "src/dawn/node/binding/GPUComputePassEncoder.h"

#include <utility>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/GPUBindGroup.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUComputePipeline.h"
#include "src/dawn/node/binding/GPUQuerySet.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUComputePassEncoder
////////////////////////////////////////////////////////////////////////////////
GPUComputePassEncoder::GPUComputePassEncoder(const wgpu::ComputePassDescriptor& desc,
                                             wgpu::ComputePassEncoder enc)
    : enc_(std::move(enc)), label_(CopyLabel(desc.label)) {}

void GPUComputePassEncoder::setPipeline(Napi::Env,
                                        interop::Interface<interop::GPUComputePipeline> pipeline) {
    enc_.SetPipeline(*pipeline.As<GPUComputePipeline>());
}

void GPUComputePassEncoder::dispatchWorkgroups(Napi::Env,
                                               interop::GPUSize32 workgroupCountX,
                                               interop::GPUSize32 workgroupCountY,
                                               interop::GPUSize32 workgroupCountZ) {
    enc_.DispatchWorkgroups(workgroupCountX, workgroupCountY, workgroupCountZ);
}

void GPUComputePassEncoder::dispatchWorkgroupsIndirect(
    Napi::Env,
    interop::Interface<interop::GPUBuffer> indirectBuffer,
    interop::GPUSize64 indirectOffset) {
    enc_.DispatchWorkgroupsIndirect(*indirectBuffer.As<GPUBuffer>(), indirectOffset);
}

void GPUComputePassEncoder::end(Napi::Env) {
    enc_.End();
}

void GPUComputePassEncoder::setBindGroup(
    Napi::Env env,
    interop::GPUIndex32 index,
    std::optional<interop::Interface<interop::GPUBindGroup>> bindGroup,
    std::vector<interop::GPUBufferDynamicOffset> dynamicOffsets) {
    Converter conv(env);

    wgpu::BindGroup bg{};
    uint32_t* offsets = nullptr;
    size_t num_offsets = 0;
    if (!conv(bg, bindGroup) || !conv(offsets, num_offsets, dynamicOffsets)) {
        return;
    }

    enc_.SetBindGroup(index, bg, num_offsets, offsets);
}

void GPUComputePassEncoder::setBindGroup(
    Napi::Env env,
    interop::GPUIndex32 index,
    std::optional<interop::Interface<interop::GPUBindGroup>> bindGroup,
    interop::Uint32Array dynamicOffsetsData,
    interop::GPUSize64 dynamicOffsetsDataStart,
    interop::GPUSize32 dynamicOffsetsDataLength) {
    Converter conv(env);

    wgpu::BindGroup bg{};
    if (!conv(bg, bindGroup)) {
        return;
    }

    if (dynamicOffsetsDataStart > dynamicOffsetsData.ElementLength()) {
        Napi::RangeError::New(env, "dynamicOffsetsDataStart is out of bound of dynamicOffsetData")
            .ThrowAsJavaScriptException();
        return;
    }

    if (dynamicOffsetsDataLength > dynamicOffsetsData.ElementLength() - dynamicOffsetsDataStart) {
        Napi::RangeError::New(env,
                              "dynamicOffsetsDataLength + dynamicOffsetsDataStart is out of "
                              "bound of dynamicOffsetData")
            .ThrowAsJavaScriptException();
        return;
    }

    enc_.SetBindGroup(index, bg, dynamicOffsetsDataLength,
                      dynamicOffsetsData.Data() + dynamicOffsetsDataStart);
}

void GPUComputePassEncoder::pushDebugGroup(Napi::Env, std::string groupLabel) {
    enc_.PushDebugGroup(groupLabel.c_str());
}

void GPUComputePassEncoder::popDebugGroup(Napi::Env) {
    enc_.PopDebugGroup();
}

void GPUComputePassEncoder::insertDebugMarker(Napi::Env, std::string markerLabel) {
    enc_.InsertDebugMarker(markerLabel.c_str());
}

std::string GPUComputePassEncoder::getLabel(Napi::Env) {
    return label_;
}

void GPUComputePassEncoder::setLabel(Napi::Env, std::string value) {
    enc_.SetLabel(std::string_view(value));
    label_ = value;
}

}  // namespace wgpu::binding
