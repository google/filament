// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_INTEGERTYPES_H_
#define SRC_DAWN_NATIVE_INTEGERTYPES_H_

#include <cstdint>

#include "dawn/common/Constants.h"
#include "dawn/common/TypedInteger.h"

namespace dawn::ityp {
template <typename Index, typename Value, size_t Size>
class array;

template <typename Index, size_t N>
class bitset;
}  // namespace dawn::ityp

// This files creates a number of integer types using ityp to represent the zoo of indices used all
// over the codebase, so that the semantic of numbers is clear from the types, but also so it is
// harder to use the wrong type to index into array, bitsets and other containers.
//
// In addition various container type aliases are declared so that they have a consistent name
// everywhere and don't need explicit sizing with the kMaxStuff constants. Respective ityp::
// headers still need to be #included in the files using them though.

namespace dawn::native {

// Binding numbers in the shader and BindGroup/BindGroupLayoutDescriptors
using BindingNumber = TypedInteger<struct BindingNumberT, uint32_t>;
constexpr BindingNumber kMaxBindingsPerBindGroupTyped = BindingNumber(kMaxBindingsPerBindGroup);

// Binding numbers get mapped to a packed range of indices
using BindingIndex = TypedInteger<struct BindingIndexT, uint32_t>;

// Bind group indinces represent the index in the SetBindGroup, the index in
// wgpu::PipelineLayoutDescriptor::bindGroupLayouts and friends.
using BindGroupIndex = TypedInteger<struct BindGroupIndexT, uint32_t>;
constexpr BindGroupIndex kMaxBindGroupsTyped = BindGroupIndex(kMaxBindGroups);

using BindGroupMask = ityp::bitset<BindGroupIndex, kMaxBindGroups>;
template <typename Value>
using PerBindGroup = ityp::array<BindGroupIndex, Value, kMaxBindGroups>;

// Immediate data constant index get mapped to a packed range of indices
using ImmediateConstantIndex = TypedInteger<struct ImmediateConstantIndexT, uint32_t>;
constexpr ImmediateConstantIndex kMaxImmediateConstantIndexTyped =
    ImmediateConstantIndex(kMaxImmediateConstantsPerPipeline);

using ImmediateConstantMask =
    ityp::bitset<ImmediateConstantIndex, kMaxImmediateConstantsPerPipeline>;

// Color attachment indices represent the index in the wgpu::FragmentState::targets array, the
// wgpu::RenderPassDescriptor::colorAttachments arry and other similar arrays.
using ColorAttachmentIndex = TypedInteger<struct ColorAttachmentIndexT, uint8_t>;
constexpr ColorAttachmentIndex kMaxColorAttachmentsTyped =
    ColorAttachmentIndex(kMaxColorAttachments);

using ColorAttachmentMask = ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments>;
template <typename Value>
using PerColorAttachment = ityp::array<ColorAttachmentIndex, Value, kMaxColorAttachments>;

// Vertex buffer slots represent the `slot` passed in calls to SetVertexBuffer or the index in the
// wgpu::VertexState::vertexBuffers array.
using VertexBufferSlot = TypedInteger<struct VertexBufferSlotT, uint8_t>;
constexpr VertexBufferSlot kMaxVertexBuffersTyped = VertexBufferSlot(kMaxVertexBuffers);

using VertexBufferMask = ityp::bitset<VertexBufferSlot, kMaxVertexBuffers>;
template <typename Value>
using PerVertexBuffer = ityp::array<VertexBufferSlot, Value, kMaxVertexBuffers>;

// Vertex attribute locations represent the "shaderLocation" in wgpu::VertexAttribute.
using VertexAttributeLocation = TypedInteger<struct VertexAttributeLocationT, uint8_t>;
constexpr VertexAttributeLocation kMaxVertexAttributesTyped =
    VertexAttributeLocation(kMaxVertexAttributes);

using VertexAttributeMask = ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes>;
template <typename Value>
using PerVertexAttribute = ityp::array<VertexAttributeLocation, Value, kMaxVertexAttributes>;

// Serials are 64bit integers that are incremented by one each time to produce unique values.
// Some serials (like queue serials) are compared numerically to know which one is before
// another, while some serials are only checked for equality. We call serials only checked
// for equality IDs.

// Buffer mapping requests are stored outside of the buffer while they are being processed and
// cannot be invalidated. Instead they are associated with an ID, and when a map request is
// finished, the mapping callback is fired only if its ID matches the ID if the last request
// that was sent.
using MapRequestID = TypedInteger<struct MapRequestIDT, uint64_t>;

// The type for the WebGPU API fence serial values.
using FenceAPISerial = TypedInteger<struct FenceAPISerialT, uint64_t>;

// A serial used to watch the progression of GPU execution on a queue, each time operations
// that need to be followed individually are scheduled for execution on a queue, the serial
// is incremented by one. This way to know if something is done executing, we just need to
// compare its serial with the currently completed serial.
using ExecutionSerial = TypedInteger<struct QueueSerialT, uint64_t>;
constexpr ExecutionSerial kMaxExecutionSerial = ExecutionSerial(~uint64_t(0));
constexpr ExecutionSerial kBeginningOfGPUTime = ExecutionSerial(0);

// An identifier that indicates which Pipeline a BindGroupLayout is compatible with. Pipelines
// created with a default layout will produce BindGroupLayouts with a non-zero compatibility
// token, which prevents them (and any BindGroups created with them) from being used with any
// other pipelines.
using PipelineCompatibilityToken = TypedInteger<struct PipelineCompatibilityTokenT, uint64_t>;
constexpr PipelineCompatibilityToken kExplicitPCT = PipelineCompatibilityToken(0);

using Nanoseconds = TypedInteger<struct NanosecondsT, uint64_t>;

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_INTEGERTYPES_H_
