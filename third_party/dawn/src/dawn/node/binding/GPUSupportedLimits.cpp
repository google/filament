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

#include "src/dawn/node/binding/GPUSupportedLimits.h"

#include <utility>

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUSupportedLimits
////////////////////////////////////////////////////////////////////////////////

GPUSupportedLimits::GPUSupportedLimits(wgpu::SupportedLimits limits) : limits_(std::move(limits)) {
    // Unpack the reported limits.
    for (wgpu::ChainedStructOut* chain = limits_.nextInChain; chain; chain = chain->nextInChain) {
        if (chain->sType == wgpu::SType::DawnExperimentalSubgroupLimits) {
            subgroup_limits_ = *static_cast<wgpu::DawnExperimentalSubgroupLimits*>(chain);
            // Clear to prevent using invalid pointer.
            subgroup_limits_->nextInChain = nullptr;
            break;
        }
    }
    // Clear to prevent using invalid pointer.
    limits_.nextInChain = nullptr;
}

uint32_t GPUSupportedLimits::getMaxTextureDimension1D(Napi::Env) {
    return limits_.limits.maxTextureDimension1D;
}

uint32_t GPUSupportedLimits::getMaxTextureDimension2D(Napi::Env) {
    return limits_.limits.maxTextureDimension2D;
}

uint32_t GPUSupportedLimits::getMaxTextureDimension3D(Napi::Env) {
    return limits_.limits.maxTextureDimension3D;
}

uint32_t GPUSupportedLimits::getMaxTextureArrayLayers(Napi::Env) {
    return limits_.limits.maxTextureArrayLayers;
}

uint32_t GPUSupportedLimits::getMaxBindGroups(Napi::Env) {
    return limits_.limits.maxBindGroups;
}

uint32_t GPUSupportedLimits::getMaxBindGroupsPlusVertexBuffers(Napi::Env) {
    return limits_.limits.maxBindGroupsPlusVertexBuffers;
}

uint32_t GPUSupportedLimits::getMaxBindingsPerBindGroup(Napi::Env) {
    return limits_.limits.maxBindingsPerBindGroup;
}

uint32_t GPUSupportedLimits::getMaxDynamicUniformBuffersPerPipelineLayout(Napi::Env) {
    return limits_.limits.maxDynamicUniformBuffersPerPipelineLayout;
}

uint32_t GPUSupportedLimits::getMaxDynamicStorageBuffersPerPipelineLayout(Napi::Env) {
    return limits_.limits.maxDynamicStorageBuffersPerPipelineLayout;
}

uint32_t GPUSupportedLimits::getMaxSampledTexturesPerShaderStage(Napi::Env) {
    return limits_.limits.maxSampledTexturesPerShaderStage;
}

uint32_t GPUSupportedLimits::getMaxSamplersPerShaderStage(Napi::Env) {
    return limits_.limits.maxSamplersPerShaderStage;
}

uint32_t GPUSupportedLimits::getMaxStorageBuffersPerShaderStage(Napi::Env) {
    return limits_.limits.maxStorageBuffersPerShaderStage;
}

uint32_t GPUSupportedLimits::getMaxStorageTexturesPerShaderStage(Napi::Env) {
    return limits_.limits.maxStorageTexturesPerShaderStage;
}

uint32_t GPUSupportedLimits::getMaxUniformBuffersPerShaderStage(Napi::Env) {
    return limits_.limits.maxUniformBuffersPerShaderStage;
}

uint64_t GPUSupportedLimits::getMaxUniformBufferBindingSize(Napi::Env) {
    return limits_.limits.maxUniformBufferBindingSize;
}

uint64_t GPUSupportedLimits::getMaxStorageBufferBindingSize(Napi::Env) {
    return limits_.limits.maxStorageBufferBindingSize;
}

uint32_t GPUSupportedLimits::getMinUniformBufferOffsetAlignment(Napi::Env) {
    return limits_.limits.minUniformBufferOffsetAlignment;
}

uint32_t GPUSupportedLimits::getMinStorageBufferOffsetAlignment(Napi::Env) {
    return limits_.limits.minStorageBufferOffsetAlignment;
}

uint32_t GPUSupportedLimits::getMaxVertexBuffers(Napi::Env) {
    return limits_.limits.maxVertexBuffers;
}

uint64_t GPUSupportedLimits::getMaxBufferSize(Napi::Env) {
    return limits_.limits.maxBufferSize;
}

uint32_t GPUSupportedLimits::getMaxVertexAttributes(Napi::Env) {
    return limits_.limits.maxVertexAttributes;
}

uint32_t GPUSupportedLimits::getMaxVertexBufferArrayStride(Napi::Env) {
    return limits_.limits.maxVertexBufferArrayStride;
}

uint32_t GPUSupportedLimits::getMaxInterStageShaderVariables(Napi::Env) {
    return limits_.limits.maxInterStageShaderVariables;
}

uint32_t GPUSupportedLimits::getMaxColorAttachments(Napi::Env) {
    return limits_.limits.maxColorAttachments;
}

uint32_t GPUSupportedLimits::getMaxColorAttachmentBytesPerSample(Napi::Env) {
    return limits_.limits.maxColorAttachmentBytesPerSample;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupStorageSize(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupStorageSize;
}

uint32_t GPUSupportedLimits::getMaxComputeInvocationsPerWorkgroup(Napi::Env) {
    return limits_.limits.maxComputeInvocationsPerWorkgroup;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupSizeX(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupSizeX;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupSizeY(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupSizeY;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupSizeZ(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupSizeZ;
}

uint32_t GPUSupportedLimits::getMaxComputeWorkgroupsPerDimension(Napi::Env) {
    return limits_.limits.maxComputeWorkgroupsPerDimension;
}

std::variant<uint32_t, interop::UndefinedType>
GPUSupportedLimits::getMaxStorageBuffersInFragmentStage(Napi::Env) {
    return std::variant<uint32_t, interop::UndefinedType>(
        limits_.limits.maxStorageBuffersInFragmentStage);
}

std::variant<uint32_t, interop::UndefinedType>
GPUSupportedLimits::getMaxStorageTexturesInFragmentStage(Napi::Env) {
    return std::variant<uint32_t, interop::UndefinedType>(
        limits_.limits.maxStorageTexturesInFragmentStage);
}

std::variant<uint32_t, interop::UndefinedType>
GPUSupportedLimits::getMaxStorageBuffersInVertexStage(Napi::Env) {
    return std::variant<uint32_t, interop::UndefinedType>(
        limits_.limits.maxStorageBuffersInVertexStage);
}

std::variant<uint32_t, interop::UndefinedType>
GPUSupportedLimits::getMaxStorageTexturesInVertexStage(Napi::Env) {
    return std::variant<uint32_t, interop::UndefinedType>(
        limits_.limits.maxStorageTexturesInVertexStage);
}

}  // namespace wgpu::binding
