// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_FORWARD_H_
#define SRC_DAWN_NATIVE_FORWARD_H_

#include <cstdint>

namespace dawn {
template <typename T>
class Ref;
}  // namespace dawn

namespace dawn::native {

enum class ObjectType : uint32_t;

class AdapterBase;
class BindGroupBase;
class BindGroupLayoutBase;
class BindGroupLayoutInternalBase;
class BufferBase;
class SharedBufferMemoryBase;
class ComputePipelineBase;
class CommandBufferBase;
class CommandEncoder;
class ComputePassEncoder;
class ExternalTextureBase;
class SharedTextureMemoryBase;
class InstanceBase;
class PhysicalDeviceBase;
class PipelineBase;
class PipelineCacheBase;
class PipelineLayoutBase;
class QuerySetBase;
class QueueBase;
class RenderBundleBase;
class RenderBundleEncoder;
class RenderPassEncoder;
class RenderPipelineBase;
class ResourceHeapBase;
class SamplerBase;
class SharedFenceBase;
class Surface;
class ShaderModuleBase;
class SwapChainBase;
class TextureBase;
class TextureViewBase;

class DeviceBase;

template <typename T>
class PerStage;

struct Format;

template <typename T>
class UnpackedPtr;

// Aliases for frontend-only types.
using CommandEncoderBase = CommandEncoder;
using ComputePassEncoderBase = ComputePassEncoder;
using RenderBundleEncoderBase = RenderBundleEncoder;
using RenderPassEncoderBase = RenderPassEncoder;
using SurfaceBase = Surface;

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_FORWARD_H_
