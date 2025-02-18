// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_FORWARD_H_
#define SRC_DAWN_NATIVE_D3D11_FORWARD_H_

#include "dawn/native/ToBackend.h"

namespace dawn::native::d3d11 {

class BindGroup;
class BindGroupLayout;
class Buffer;
class CommandBuffer;
class ComputePipeline;
class Device;
class Heap;
class PhysicalDevice;
class PipelineCache;
class PipelineLayout;
class QuerySet;
class Queue;
class RenderPipeline;
class Sampler;
class ShaderModule;
class SharedFence;
class SharedTextureMemory;
class SwapChain;
class Texture;
class TextureView;

struct D3D11BackendTraits {
    using BindGroupType = BindGroup;
    using BindGroupLayoutType = BindGroupLayout;
    using BufferType = Buffer;
    using CommandBufferType = CommandBuffer;
    using ComputePipelineType = ComputePipeline;
    using DeviceType = Device;
    using PhysicalDeviceType = PhysicalDevice;
    using PipelineCacheType = PipelineCache;
    using PipelineLayoutType = PipelineLayout;
    using QuerySetType = QuerySet;
    using QueueType = Queue;
    using RenderPipelineType = RenderPipeline;
    using ResourceHeapType = Heap;
    using SamplerType = Sampler;
    using ShaderModuleType = ShaderModule;
    using SharedFenceType = SharedFence;
    using SharedTextureMemoryType = SharedTextureMemory;
    using SwapChainType = SwapChain;
    using TextureType = Texture;
    using TextureViewType = TextureView;
};

template <typename T>
auto ToBackend(T&& common) -> decltype(ToBackendBase<D3D11BackendTraits>(common)) {
    return ToBackendBase<D3D11BackendTraits>(common);
}

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_FORWARD_H_
