// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_SERIALIZATION_SCHEMA_H_
#define SRC_DAWN_SERIALIZATION_SCHEMA_H_

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace schema {
// NOTE: This file must be included after files that define
// DAWN_REPLAY_SERIALIZABLE and DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA.
// Those macro are different for serialization and deserialization.

// TODO(crbug.com/413053623): Switch to protobufs or json. This is just to get
// stuff working. We'll switch to something else once we're sure of the data we
// want to capture.

using ObjectId = uint64_t;
// device is always 1
const ObjectId kDeviceId = 1;

// Use alias of std::array since the preprocessor doesn't consider < > to be parenthesis for the
// logic of skipping commas (so the end up splitting macro invocation arguments).
using FloatArray7 = std::array<float, 7>;
using Mat3x3 = std::array<float, 9>;
using Mat4x3 = std::array<float, 12>;

#define DAWN_REPLAY_ENUM_MEMBER(NAME) NAME,
#define DAWN_REPLAY_ENUM(NAME, MEMBERS) \
    enum class NAME : uint32_t { MEMBERS(DAWN_REPLAY_ENUM_MEMBER) };

#define DAWN_REPLAY_ENUM_WITH_INVALID(NAME, MEMBERS) \
    enum class NAME : uint32_t { Invalid = 0, MEMBERS(DAWN_REPLAY_ENUM_MEMBER) };

#define DAWN_REPLAY_OBJECT_TYPES(X) \
    X(BindGroup)                    \
    X(BindGroupLayout)              \
    X(Buffer)                       \
    X(CommandBuffer)                \
    X(ComputePipeline)              \
    X(Device)                       \
    X(ExternalTexture)              \
    X(PipelineLayout)               \
    X(QuerySet)                     \
    X(RenderBundle)                 \
    X(RenderPipeline)               \
    X(Sampler)                      \
    X(ShaderModule)                 \
    X(Texture)                      \
    X(TexelBufferView)              \
    X(TextureView)

#define DAWN_REPLAY_OBJECT_TYPES_ENUM(X) X(ObjectType, DAWN_REPLAY_OBJECT_TYPES)

DAWN_REPLAY_OBJECT_TYPES_ENUM(DAWN_REPLAY_ENUM_WITH_INVALID)

#define DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES(X) \
    X(BufferBinding)                                    \
    X(SamplerBinding)                                   \
    X(TextureBinding)                                   \
    X(TexelBufferBinding)                               \
    X(StorageTextureBinding)                            \
    X(ExternalTextureBinding)                           \
    X(StaticSamplerBindingInfo)                         \
    X(InputAttachmentBindingInfo)

#define DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES_ENUM(X) \
    X(BindGroupLayoutEntryType, DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES)

DAWN_REPLAY_BINDING_GROUP_LAYOUT_ENTRY_TYPES_ENUM(DAWN_REPLAY_ENUM_WITH_INVALID)

#define DAWN_REPLAY_COMMAND_BUFFER_COMMANDS(X) \
    X(BeginComputePass)                        \
    X(BeginOcclusionQuery)                     \
    X(BeginRenderPass)                         \
    X(ClearBuffer)                             \
    X(CopyBufferToBuffer)                      \
    X(CopyBufferToTexture)                     \
    X(CopyTextureToBuffer)                     \
    X(CopyTextureToTexture)                    \
    X(Dispatch)                                \
    X(DispatchIndirect)                        \
    X(Draw)                                    \
    X(DrawIndexed)                             \
    X(DrawIndexedIndirect)                     \
    X(DrawIndirect)                            \
    X(End)                                     \
    X(EndOcclusionQuery)                       \
    X(EndRenderPass)                           \
    X(ExecuteBundles)                          \
    X(InsertDebugMarker)                       \
    X(PopDebugGroup)                           \
    X(PushDebugGroup)                          \
    X(ResolveQuerySet)                         \
    X(SetBindGroup)                            \
    X(SetBlendConstant)                        \
    X(SetComputePipeline)                      \
    X(SetImmediates)                           \
    X(SetIndexBuffer)                          \
    X(SetRenderPipeline)                       \
    X(SetScissorRect)                          \
    X(SetStencilReference)                     \
    X(SetVertexBuffer)                         \
    X(SetViewport)                             \
    X(WriteBuffer)                             \
    X(WriteTimestamp)

#define DAWN_REPLAY_SHARED_PASS_COMMANDS(X) \
    X(SetBindGroup)                         \
    X(SetImmediates)

#define DAWN_REPLAY_DEBUG_COMMANDS(X) \
    X(PushDebugGroup)                 \
    X(InsertDebugMarker)              \
    X(PopDebugGroup)

#define DAWN_REPLAY_RENDER_COMMANDS(X)  \
    X(SetRenderPipeline)                \
    X(SetVertexBuffer)                  \
    X(SetIndexBuffer)                   \
    X(Draw)                             \
    X(DrawIndexed)                      \
    X(DrawIndirect)                     \
    X(DrawIndexedIndirect)              \
    DAWN_REPLAY_SHARED_PASS_COMMANDS(X) \
    DAWN_REPLAY_DEBUG_COMMANDS(X)

#define DAWN_REPLAY_RENDER_BUNDLE_COMMANDS(X) \
    DAWN_REPLAY_RENDER_COMMANDS(X)            \
    X(End)

#define DAWN_REPLAY_COMPUTE_PASS_COMMANDS(X) \
    X(SetComputePipeline)                    \
    X(Dispatch)                              \
    X(DispatchIndirect)                      \
    X(WriteTimestamp)                        \
    DAWN_REPLAY_SHARED_PASS_COMMANDS(X)      \
    DAWN_REPLAY_DEBUG_COMMANDS(X)            \
    X(End)

#define DAWN_REPLAY_RENDER_PASS_COMMANDS(X) \
    X(ExecuteBundles)                       \
    X(BeginOcclusionQuery)                  \
    X(EndOcclusionQuery)                    \
    X(SetBlendConstant)                     \
    X(SetScissorRect)                       \
    X(SetStencilReference)                  \
    X(SetViewport)                          \
    X(WriteTimestamp)                       \
    DAWN_REPLAY_RENDER_COMMANDS(X)          \
    X(End)

#define DAWN_REPLAY_ENCODER_CREATION_COMMANDS(X) \
    X(BeginComputePass)                          \
    X(BeginRenderPass)

#define DAWN_REPLAY_ENCODER_NON_CREATION_COMMANDS(X) \
    X(ClearBuffer)                                   \
    X(CopyBufferToBuffer)                            \
    X(CopyBufferToTexture)                           \
    X(CopyTextureToBuffer)                           \
    X(CopyTextureToTexture)                          \
    X(ResolveQuerySet)                               \
    X(WriteBuffer)                                   \
    X(WriteTimestamp)                                \
    DAWN_REPLAY_DEBUG_COMMANDS(X)                    \
    X(End)

#define DAWN_REPLAY_ENCODER_COMMANDS(X)      \
    DAWN_REPLAY_ENCODER_CREATION_COMMANDS(X) \
    DAWN_REPLAY_ENCODER_NON_CREATION_COMMANDS(X)

#define DAWN_REPLAY_COMMAND_BUFFER_COMMANDS_ENUM(X) \
    X(CommandBufferCommand, DAWN_REPLAY_COMMAND_BUFFER_COMMANDS)

DAWN_REPLAY_COMMAND_BUFFER_COMMANDS_ENUM(DAWN_REPLAY_ENUM_WITH_INVALID)

#define DAWN_REPLAY_EXPAND_RESOLVE_MODES(X) \
    X(Unused)                               \
    X(Disabled)                             \
    X(Enabled)

#define DAWN_REPLAY_EXPAND_RESOLVE_MODES_ENUM(X) \
    X(ExpandResolveMode, DAWN_REPLAY_EXPAND_RESOLVE_MODES)

DAWN_REPLAY_EXPAND_RESOLVE_MODES_ENUM(DAWN_REPLAY_ENUM)

#define DAWN_REPLAY_ROOT_COMMANDS(X) \
    X(CreateResource)                \
    X(QueueSubmit)                   \
    X(WriteBuffer)                   \
    X(WriteTexture)                  \
    X(SetLabel)                      \
    X(InitTexture)                   \
    X(End)

#define DAWN_REPLAY_ROOT_COMMANDS_ENUM(X) X(RootCommand, DAWN_REPLAY_ROOT_COMMANDS)

DAWN_REPLAY_ROOT_COMMANDS_ENUM(DAWN_REPLAY_ENUM_WITH_INVALID)

#undef DAWN_REPLAY_ENUM
#undef DAWN_REPLAY_GET_X_MACRO
#undef DAWN_REPLAY_ENUM_DEfAULT_MEMBER
#undef DAWN_REPLAY_ENUM_VALUE_MEMBER
#undef DAWN_REPLAY_ENUM_MEMBER

#define ORIGIN3D_MEMBER(X) \
    X(uint32_t, x)         \
    X(uint32_t, y)         \
    X(uint32_t, z)

DAWN_REPLAY_SERIALIZABLE(struct, Origin3D, ORIGIN3D_MEMBER){};

#define ORIGIN2D_MEMBER(X) \
    X(uint32_t, x)         \
    X(uint32_t, y)

DAWN_REPLAY_SERIALIZABLE(struct, Origin2D, ORIGIN2D_MEMBER){};

#define EXTENT3D_MEMBER(X) \
    X(uint32_t, width)     \
    X(uint32_t, height)    \
    X(uint32_t, depthOrArrayLayers)

DAWN_REPLAY_SERIALIZABLE(struct, Extent3D, EXTENT3D_MEMBER){};

#define EXTENT2D_MEMBER(X) \
    X(uint32_t, width)     \
    X(uint32_t, height)

DAWN_REPLAY_SERIALIZABLE(struct, Extent2D, EXTENT2D_MEMBER){};

#define COLOR_MEMBER(X) \
    X(double, r)        \
    X(double, g)        \
    X(double, b)        \
    X(double, a)

DAWN_REPLAY_SERIALIZABLE(struct, Color, COLOR_MEMBER){};

#define TEXTURE_COMPONENT_SWIZZLE_MEMBER(X) \
    X(wgpu::ComponentSwizzle, r)            \
    X(wgpu::ComponentSwizzle, g)            \
    X(wgpu::ComponentSwizzle, b)            \
    X(wgpu::ComponentSwizzle, a)

DAWN_REPLAY_SERIALIZABLE(struct, TextureComponentSwizzle, TEXTURE_COMPONENT_SWIZZLE_MEMBER){};

#define PIPELINE_CONSTANT_MEMBER(X) \
    X(std::string, name)            \
    X(double, value)

DAWN_REPLAY_SERIALIZABLE(struct, PipelineConstant, PIPELINE_CONSTANT_MEMBER){};

#define PROGRAMMABLE_STAGE_MEMBER(X) \
    X(ObjectId, moduleId)            \
    X(std::string, entryPoint)       \
    X(std::vector<PipelineConstant>, constants)

DAWN_REPLAY_SERIALIZABLE(struct, ProgrammableStage, PROGRAMMABLE_STAGE_MEMBER){};

#define VERTEX_ATTRIBUTE_MEMBER(X) \
    X(wgpu::VertexFormat, format)  \
    X(uint64_t, offset)            \
    X(uint32_t, shaderLocation)

DAWN_REPLAY_SERIALIZABLE(struct, VertexAttribute, VERTEX_ATTRIBUTE_MEMBER){};

#define VERTEX_BUFFER_LAYOUT_MEMBER(X) \
    X(uint64_t, arrayStride)           \
    X(wgpu::VertexStepMode, stepMode)  \
    X(std::vector<VertexAttribute>, attributes)

DAWN_REPLAY_SERIALIZABLE(struct, VertexBufferLayout, VERTEX_BUFFER_LAYOUT_MEMBER){};

#define VERTEX_STATE_MEMBER(X)    \
    X(ProgrammableStage, program) \
    X(std::vector<VertexBufferLayout>, buffers)

DAWN_REPLAY_SERIALIZABLE(struct, VertexState, VERTEX_STATE_MEMBER){};

#define PRIMITIVE_STATE_MEMBER(X)          \
    X(wgpu::PrimitiveTopology, topology)   \
    X(wgpu::IndexFormat, stripIndexFormat) \
    X(wgpu::FrontFace, frontFace)          \
    X(wgpu::CullMode, cullMode)            \
    X(bool, unclippedDepth)

DAWN_REPLAY_SERIALIZABLE(struct, PrimitiveState, PRIMITIVE_STATE_MEMBER){};

#define STENCIL_FACE_STATE_MEMBER(X)       \
    X(wgpu::CompareFunction, compare)      \
    X(wgpu::StencilOperation, failOp)      \
    X(wgpu::StencilOperation, depthFailOp) \
    X(wgpu::StencilOperation, passOp)

DAWN_REPLAY_SERIALIZABLE(struct, StencilFaceState, STENCIL_FACE_STATE_MEMBER){};

#define DEPTH_STENCIL_STATE_MEMBER(X)      \
    X(wgpu::TextureFormat, format)         \
    X(bool, depthWriteEnabled)             \
    X(wgpu::CompareFunction, depthCompare) \
    X(StencilFaceState, stencilFront)      \
    X(StencilFaceState, stencilBack)       \
    X(uint32_t, stencilReadMask)           \
    X(uint32_t, stencilWriteMask)          \
    X(int32_t, depthBias)                  \
    X(float, depthBiasSlopeScale)          \
    X(float, depthBiasClamp)

DAWN_REPLAY_SERIALIZABLE(struct, DepthStencilState, DEPTH_STENCIL_STATE_MEMBER){};

#define MULTISAMPLE_STATE_MEMBER(X) \
    X(uint32_t, count)              \
    X(uint32_t, mask)               \
    X(bool, alphaToCoverageEnabled)

DAWN_REPLAY_SERIALIZABLE(struct, MultisampleState, MULTISAMPLE_STATE_MEMBER){};

#define BLEND_COMPONENT_MEMBER(X)      \
    X(wgpu::BlendOperation, operation) \
    X(wgpu::BlendFactor, srcFactor)    \
    X(wgpu::BlendFactor, dstFactor)

DAWN_REPLAY_SERIALIZABLE(struct, BlendComponent, BLEND_COMPONENT_MEMBER){};

#define BLEND_STATE_MEMBER(X) \
    X(BlendComponent, color)  \
    X(BlendComponent, alpha)

DAWN_REPLAY_SERIALIZABLE(struct, BlendState, BLEND_STATE_MEMBER){};

#define COLOR_TARGET_STATE_MEMBER(X)   \
    X(wgpu::TextureFormat, format)     \
    X(BlendState, blend)               \
    X(wgpu::ColorWriteMask, writeMask) \
    X(ExpandResolveMode, expandResolveMode)

DAWN_REPLAY_SERIALIZABLE(struct, ColorTargetState, COLOR_TARGET_STATE_MEMBER){};

#define FRAGMENT_STATE_MEMBER(X)  \
    X(ProgrammableStage, program) \
    X(std::vector<ColorTargetState>, targets)

DAWN_REPLAY_SERIALIZABLE(struct, FragmentState, FRAGMENT_STATE_MEMBER){};

#define BIND_GROUP_LAYOUT_BINDING_MEMBER(X) \
    X(uint32_t, binding)                    \
    X(wgpu::ShaderStage, visibility)        \
    X(uint32_t, bindingArraySize)

DAWN_REPLAY_SERIALIZABLE(struct, BindGroupLayoutBinding, BIND_GROUP_LAYOUT_BINDING_MEMBER){};

#define BUFFER_BIND_GROUP_LAYOUT_MEMBER(X) \
    X(wgpu::BufferBindingType, type)       \
    X(uint64_t, minBindingSize)            \
    X(bool, hasDynamicOffset)

DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(BufferBinding, BUFFER_BIND_GROUP_LAYOUT_MEMBER){};

#define SAMPLER_BIND_GROUP_LAYOUT_MEMBER(X) X(wgpu::SamplerBindingType, type)

DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(SamplerBinding, SAMPLER_BIND_GROUP_LAYOUT_MEMBER){};

#define STORAGE_TEXTURE_BIND_GROUP_LAYOUT_MEMBER(X) \
    X(wgpu::TextureFormat, format)                  \
    X(wgpu::TextureViewDimension, viewDimension)    \
    X(wgpu::StorageTextureAccess, access)

DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(StorageTextureBinding,
                                          STORAGE_TEXTURE_BIND_GROUP_LAYOUT_MEMBER){};

#define TEXTURE_BIND_GROUP_LAYOUT_MEMBER(X)      \
    X(wgpu::TextureSampleType, sampleType)       \
    X(wgpu::TextureViewDimension, viewDimension) \
    X(bool, multisampled)

DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(TextureBinding, TEXTURE_BIND_GROUP_LAYOUT_MEMBER){};

#define TEXEL_BUFFER_BIND_GROUP_LAYOUT_MEMBER(X)
DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(TexelBufferBinding,
                                          TEXEL_BUFFER_BIND_GROUP_LAYOUT_MEMBER){};

#define EXTERNAL_TEXTURE_BIND_GROUP_LAYOUT_MEMBER(X)
DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(ExternalTextureBinding,
                                          EXTERNAL_TEXTURE_BIND_GROUP_LAYOUT_MEMBER){};

#define STATIC_SAMPLER_BIND_GROUP_LAYOUT_MEMBER(X)
DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(StaticSamplerBindingInfo,
                                          STATIC_SAMPLER_BIND_GROUP_LAYOUT_MEMBER){};

#define INPUT_ATTACHMENT_BIND_GROUP_LAYOUT_MEMBER(X)
DAWN_REPLAY_MAKE_BINDGROUP_LAYOUT_VARIANT(InputAttachmentBindingInfo,
                                          INPUT_ATTACHMENT_BIND_GROUP_LAYOUT_MEMBER){};

#define BIND_GROUP_LAYOUT_MEMBER(X) X(uint32_t, numEntries)

DAWN_REPLAY_SERIALIZABLE(struct, BindGroupLayout, BIND_GROUP_LAYOUT_MEMBER){};

#define PIPELINE_LAYOUT_MEMBER(X)                \
    X(std::vector<ObjectId>, bindGroupLayoutIds) \
    X(uint32_t, immediateSize)

DAWN_REPLAY_SERIALIZABLE(struct, PipelineLayout, PIPELINE_LAYOUT_MEMBER){};

#define BUFFER_CREATION_MEMBER(X) \
    X(uint64_t, size)             \
    X(wgpu::BufferUsage, usage)

DAWN_REPLAY_SERIALIZABLE(struct, Buffer, BUFFER_CREATION_MEMBER){};

#define SAMPLER_CREATION_MEMBER(X)          \
    X(wgpu::AddressMode, addressModeU)      \
    X(wgpu::AddressMode, addressModeV)      \
    X(wgpu::AddressMode, addressModeW)      \
    X(wgpu::FilterMode, magFilter)          \
    X(wgpu::FilterMode, minFilter)          \
    X(wgpu::MipmapFilterMode, mipmapFilter) \
    X(float, lodMinClamp)                   \
    X(float, lodMaxClamp)                   \
    X(wgpu::CompareFunction, compare)       \
    X(uint16_t, maxAnisotropy)

DAWN_REPLAY_SERIALIZABLE(struct, Sampler, SAMPLER_CREATION_MEMBER){};

#define RENDER_BUNDLE_CREATION_MEMBER(X)              \
    X(std::vector<wgpu::TextureFormat>, colorFormats) \
    X(wgpu::TextureFormat, depthStencilFormat)        \
    X(uint32_t, sampleCount)                          \
    X(bool, depthReadOnly)                            \
    X(bool, stencilReadOnly)

DAWN_REPLAY_SERIALIZABLE(struct, RenderBundle, RENDER_BUNDLE_CREATION_MEMBER){};

#define TEXTURE_CREATION_MEMBER(X)       \
    X(wgpu::TextureUsage, usage)         \
    X(wgpu::TextureDimension, dimension) \
    X(Extent3D, size)                    \
    X(wgpu::TextureFormat, format)       \
    X(uint32_t, mipLevelCount)           \
    X(uint32_t, sampleCount)             \
    X(std::vector<wgpu::TextureFormat>, viewFormats)

DAWN_REPLAY_SERIALIZABLE(struct, Texture, TEXTURE_CREATION_MEMBER){};

#define TEXTURE_VIEW_CREATION_MEMBER(X)      \
    X(ObjectId, textureId)                   \
    X(wgpu::TextureFormat, format)           \
    X(wgpu::TextureViewDimension, dimension) \
    X(uint32_t, baseMipLevel)                \
    X(uint32_t, mipLevelCount)               \
    X(uint32_t, baseArrayLayer)              \
    X(uint32_t, arrayLayerCount)             \
    X(wgpu::TextureAspect, aspect)           \
    X(wgpu::TextureUsage, usage)             \
    X(TextureComponentSwizzle, swizzle)

DAWN_REPLAY_SERIALIZABLE(struct, TextureView, TEXTURE_VIEW_CREATION_MEMBER){};

#define TEXEL_BUFFER_VIEW_CREATION_MEMBER(X) \
    X(wgpu::TextureFormat, format)           \
    X(uint64_t, offset)                      \
    X(uint64_t, size)

DAWN_REPLAY_SERIALIZABLE(struct, TexelBufferView, TEXEL_BUFFER_VIEW_CREATION_MEMBER){};

#define QUERYSET_CREATION_MEMBER(X) \
    X(wgpu::QueryType, type)        \
    X(uint32_t, count)

DAWN_REPLAY_SERIALIZABLE(struct, QuerySet, QUERYSET_CREATION_MEMBER){};

#define EXTERNAL_TEXTURE_CREATION_MEMBER(X)       \
    X(ObjectId, plane0Id)                         \
    X(ObjectId, plane1Id)                         \
    X(Origin2D, cropOrigin)                       \
    X(Extent2D, cropSize)                         \
    X(Extent2D, apparentSize)                     \
    X(bool, doYuvToRgbConversionOnly)             \
    X(Mat4x3, yuvToRgbConversionMatrix)           \
    X(FloatArray7, srcTransferFunctionParameters) \
    X(FloatArray7, dstTransferFunctionParameters) \
    X(Mat3x3, gamutConversionMatrix)              \
    X(bool, mirrored)                             \
    X(wgpu::ExternalTextureRotation, rotation)

DAWN_REPLAY_SERIALIZABLE(struct, ExternalTexture, EXTERNAL_TEXTURE_CREATION_MEMBER){};

// TODO(452840621): Make this use a chain instead of hard coded to WGSL only and handle other
// chained structs.
#define SHADER_MODULE_CREATION_MEMBER(X) X(std::string, code)

DAWN_REPLAY_SERIALIZABLE(struct, ShaderModule, SHADER_MODULE_CREATION_MEMBER){};

#define COMPUTE_PIPELINE_CREATION_MEMBER(X) \
    X(ObjectId, layoutId)                   \
    X(ProgrammableStage, compute)

DAWN_REPLAY_SERIALIZABLE(struct, ComputePipeline, COMPUTE_PIPELINE_CREATION_MEMBER){};

#define RENDER_PIPELINE_CREATION_MEMBER(X) \
    X(ObjectId, layoutId)                  \
    X(VertexState, vertex)                 \
    X(PrimitiveState, primitive)           \
    X(DepthStencilState, depthStencil)     \
    X(MultisampleState, multisample)       \
    X(FragmentState, fragment)

DAWN_REPLAY_SERIALIZABLE(struct, RenderPipeline, RENDER_PIPELINE_CREATION_MEMBER){};

#define BUFFER_BIND_GROUP_ENTRY_MEMBER(X) \
    X(ObjectId, bufferId)                 \
    X(uint64_t, offset)                   \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(BufferBinding, BUFFER_BIND_GROUP_ENTRY_MEMBER){};

#define SAMPLER_BIND_GROUP_ENTRY_MEMBER(X) X(ObjectId, samplerId)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(SamplerBinding, SAMPLER_BIND_GROUP_ENTRY_MEMBER){};

#define TEXTURE_BIND_GROUP_ENTRY_MEMBER(X) X(ObjectId, textureViewId)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(TextureBinding, TEXTURE_BIND_GROUP_ENTRY_MEMBER){};

#define STORAGE_TEXTURE_BIND_GROUP_ENTRY_MEMBER(X) X(ObjectId, textureViewId)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(StorageTextureBinding,
                                   STORAGE_TEXTURE_BIND_GROUP_ENTRY_MEMBER){};

#define TEXEL_BUFFER_BIND_GROUP_ENTRY_MEMBER(X) X(ObjectId, texelBufferViewId)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(TexelBufferBinding, TEXEL_BUFFER_BIND_GROUP_ENTRY_MEMBER){};

#define INPUT_ATTACHMENTS_BIND_GROUP_ENTRY_MEMBER(X) X(ObjectId, textureViewId)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(InputAttachmentBindingInfo,
                                   INPUT_ATTACHMENTS_BIND_GROUP_ENTRY_MEMBER){};

#define STATIC_SAMPLER_BIND_GROUP_ENTRY_MEMBER(X)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(StaticSamplerBindingInfo,
                                   STATIC_SAMPLER_BIND_GROUP_ENTRY_MEMBER){};

#define EXTERNAL_TEXTURE_BIND_GROUP_ENTRY_MEMBER(X) \
    X(ObjectId, externalTextureId)                  \
    X(ObjectId, textureViewId)

DAWN_REPLAY_MAKE_BINDGROUP_VARIANT(ExternalTextureBinding,
                                   EXTERNAL_TEXTURE_BIND_GROUP_ENTRY_MEMBER){};

#define BIND_GROUP_CREATION_MEMBER(X) \
    X(ObjectId, layoutId)             \
    X(uint32_t, numEntries)

DAWN_REPLAY_SERIALIZABLE(struct, BindGroup, BIND_GROUP_CREATION_MEMBER){};

#define LABELED_RESOURCE_MEMBER(X) \
    X(ObjectType, type)            \
    X(ObjectId, id)                \
    X(std::string, label)

DAWN_REPLAY_SERIALIZABLE(struct, LabeledResource, LABELED_RESOURCE_MEMBER){};

#define TEXEL_COPY_BUFFER_LAYOUT_MEMBER(X) \
    X(uint64_t, offset)                    \
    X(uint32_t, bytesPerRow)               \
    X(uint32_t, rowsPerImage)

DAWN_REPLAY_SERIALIZABLE(struct, TexelCopyBufferLayout, TEXEL_COPY_BUFFER_LAYOUT_MEMBER){};

#define TEXEL_COPY_BUFFER_INFO_MEMBER(X) \
    X(ObjectId, bufferId)                \
    X(TexelCopyBufferLayout, layout)

DAWN_REPLAY_SERIALIZABLE(struct, TexelCopyBufferInfo, TEXEL_COPY_BUFFER_INFO_MEMBER){};

#define TEXEL_COPY_TEXTURE_INFO_MEMBER(X) \
    X(ObjectId, textureId)                \
    X(uint32_t, mipLevel)                 \
    X(Origin3D, origin)                   \
    X(wgpu::TextureAspect, aspect)

DAWN_REPLAY_SERIALIZABLE(struct, TexelCopyTextureInfo, TEXEL_COPY_TEXTURE_INFO_MEMBER){};

#define TIMESTAMP_WRITES_MEMBER(X)         \
    X(ObjectId, querySetId)                \
    X(uint32_t, beginningOfPassWriteIndex) \
    X(uint32_t, endOfPassWriteIndex)

DAWN_REPLAY_SERIALIZABLE(struct, TimestampWrites, TIMESTAMP_WRITES_MEMBER){};

#define COLOR_ATTACHMENT_MEMBER(X) \
    X(ObjectId, viewId)            \
    X(uint32_t, depthSlice)        \
    X(ObjectId, resolveTargetId)   \
    X(wgpu::LoadOp, loadOp)        \
    X(wgpu::StoreOp, storeOp)      \
    X(Color, clearValue)

DAWN_REPLAY_SERIALIZABLE(struct, ColorAttachment, COLOR_ATTACHMENT_MEMBER){};

#define RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_MEMBER(X) \
    X(ObjectId, viewId)                                \
    X(wgpu::LoadOp, depthLoadOp)                       \
    X(wgpu::StoreOp, depthStoreOp)                     \
    X(float, depthClearValue)                          \
    X(bool, depthReadOnly)                             \
    X(wgpu::LoadOp, stencilLoadOp)                     \
    X(wgpu::StoreOp, stencilStoreOp)                   \
    X(uint32_t, stencilClearValue)                     \
    X(bool, stencilReadOnly)

DAWN_REPLAY_SERIALIZABLE(struct,
                         RenderPassDepthStencilAttachment,
                         RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_MEMBER){};

#define RESOLVE_RECT_MEMBER(X)  \
    X(uint32_t, colorOffsetX)   \
    X(uint32_t, colorOffsetY)   \
    X(uint32_t, resolveOffsetX) \
    X(uint32_t, resolveOffsetY) \
    X(uint32_t, width)          \
    X(uint32_t, height)

DAWN_REPLAY_SERIALIZABLE(struct, ResolveRect, RESOLVE_RECT_MEMBER){};

#define CREATE_RESOURCE_CMD_DATA_MEMBER(X) X(LabeledResource, resource)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(CreateResource, CREATE_RESOURCE_CMD_DATA_MEMBER){};

#define SET_LABEL_CMD_DATA_MEMBER(X) \
    X(ObjectId, id)                  \
    X(ObjectType, type)              \
    X(std::string, label)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(SetLabel, SET_LABEL_CMD_DATA_MEMBER){};

#define WRITE_BUFFER_CMD_DATA_MEMBER(X) \
    X(ObjectId, bufferId)               \
    X(uint64_t, bufferOffset)           \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(WriteBuffer, WRITE_BUFFER_CMD_DATA_MEMBER){};

#define WRITE_TEXTURE_CMD_DATA_MEMBER(X) \
    X(TexelCopyTextureInfo, destination) \
    X(TexelCopyBufferLayout, layout)     \
    X(Extent3D, size)                    \
    X(uint64_t, dataSize)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(WriteTexture, WRITE_TEXTURE_CMD_DATA_MEMBER){};

#define INIT_TEXTURE_CMD_DATA_MEMBER(X)  \
    X(TexelCopyTextureInfo, destination) \
    X(TexelCopyBufferLayout, layout)     \
    X(Extent3D, size)                    \
    X(uint64_t, dataSize)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(InitTexture, INIT_TEXTURE_CMD_DATA_MEMBER){};

#define QUEUE_SUBMIT_CMD_DATA_MEMBER(X) X(std::vector<ObjectId>, commandBuffers)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(QueueSubmit, QUEUE_SUBMIT_CMD_DATA_MEMBER){};

#define END_ROOT_CMD_DATA_MEMBER(X)

DAWN_REPLAY_MAKE_ROOT_CMD_AND_CMD_DATA(End, END_ROOT_CMD_DATA_MEMBER){};

#define COPY_BUFFER_TO_BUFFER_CMD_DATA_MEMBER(X) \
    X(ObjectId, srcBufferId)                     \
    X(uint64_t, srcOffset)                       \
    X(ObjectId, dstBufferId)                     \
    X(uint64_t, dstOffset)                       \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(CopyBufferToBuffer,
                                                 COPY_BUFFER_TO_BUFFER_CMD_DATA_MEMBER){};

#define COPY_BUFFER_TO_TEXTURE_CMD_DATA_MEMBER(X) \
    X(TexelCopyBufferInfo, source)                \
    X(TexelCopyTextureInfo, destination)          \
    X(Extent3D, copySize)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(CopyBufferToTexture,
                                                 COPY_BUFFER_TO_TEXTURE_CMD_DATA_MEMBER){};

#define COPY_TEXTURE_TO_BUFFER_CMD_DATA_MEMBER(X) \
    X(TexelCopyTextureInfo, source)               \
    X(TexelCopyBufferInfo, destination)           \
    X(Extent3D, copySize)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(CopyTextureToBuffer,
                                                 COPY_TEXTURE_TO_BUFFER_CMD_DATA_MEMBER){};

#define COPY_TEXTURE_TO_TEXTURE_CMD_DATA_MEMBER(X) \
    X(TexelCopyTextureInfo, source)                \
    X(TexelCopyTextureInfo, destination)           \
    X(Extent3D, copySize)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(CopyTextureToTexture,
                                                 COPY_TEXTURE_TO_TEXTURE_CMD_DATA_MEMBER){};

#define WRITE_BUFFER_CMD_ENCODER_DATA_MEMBER(X) \
    X(ObjectId, bufferId)                       \
    X(uint64_t, bufferOffset)                   \
    X(std::vector<uint8_t>, data)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(WriteBuffer,
                                                 WRITE_BUFFER_CMD_ENCODER_DATA_MEMBER){};

#define BEGIN_COMPUTE_PASS_CMD_DATA_MEMBER(X) \
    X(std::string, label)                     \
    X(TimestampWrites, timestampWrites)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(BeginComputePass,
                                                 BEGIN_COMPUTE_PASS_CMD_DATA_MEMBER){};

#define BEGIN_RENDER_PASS_CMD_DATA_MEMBER(X)                    \
    X(std::string, label)                                       \
    X(std::vector<ColorAttachment>, colorAttachments)           \
    X(RenderPassDepthStencilAttachment, depthStencilAttachment) \
    X(ObjectId, occlusionQuerySetId)                            \
    X(TimestampWrites, timestampWrites)                         \
    X(uint64_t, maxDrawCount)                                   \
    X(ResolveRect, resolveRect)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(BeginRenderPass,
                                                 BEGIN_RENDER_PASS_CMD_DATA_MEMBER){};

#define RESOLVE_QUERYSET_CMD_DATA_MEMBER(X) \
    X(ObjectId, querySetId)                 \
    X(uint32_t, firstQuery)                 \
    X(uint32_t, queryCount)                 \
    X(ObjectId, destinationId)              \
    X(uint64_t, destinationOffset)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(ResolveQuerySet,
                                                 RESOLVE_QUERYSET_CMD_DATA_MEMBER){};

#define SET_COMPUTE_PIPELINE_CMD_DATA_MEMBER(X) X(ObjectId, pipelineId)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetComputePipeline,
                                                 SET_COMPUTE_PIPELINE_CMD_DATA_MEMBER){};

#define SET_BIND_GROUP_CMD_DATA_MEMBER(X) \
    X(uint32_t, index)                    \
    X(ObjectId, bindGroupId)              \
    X(std::vector<uint32_t>, dynamicOffsets)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetBindGroup, SET_BIND_GROUP_CMD_DATA_MEMBER){};

#define SET_IMMEDIATES_CMD_DATA_MEMBER(X) \
    X(uint32_t, offset)                   \
    X(std::vector<uint8_t>, data)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetImmediates, SET_IMMEDIATES_CMD_DATA_MEMBER){};

#define SET_CLEAR_BUFFER_CMD_DATA_MEMBER(X) \
    X(ObjectId, bufferId)                   \
    X(uint64_t, offset)                     \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(ClearBuffer, SET_CLEAR_BUFFER_CMD_DATA_MEMBER){};

#define DISPATCH_CMD_DATA_MEMBER(X) \
    X(uint32_t, x)                  \
    X(uint32_t, y)                  \
    X(uint32_t, z)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(Dispatch, DISPATCH_CMD_DATA_MEMBER){};

#define DISPATCH_INDIRECT_CMD_DATA_MEMBER(X) \
    X(ObjectId, bufferId)                    \
    X(uint64_t, offset)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(DispatchIndirect,
                                                 DISPATCH_INDIRECT_CMD_DATA_MEMBER){};

#define SET_RENDER_PIPELINE_CMD_DATA_MEMBER(X) X(ObjectId, pipelineId)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetRenderPipeline,
                                                 SET_RENDER_PIPELINE_CMD_DATA_MEMBER){};

#define PUSH_DEBUG_GROUP_CMD_DATA_MEMBER(X) X(std::string, groupLabel)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(PushDebugGroup,
                                                 PUSH_DEBUG_GROUP_CMD_DATA_MEMBER){};

#define INSERT_DEBUG_MARKER_CMD_DATA_MEMBER(X) X(std::string, markerLabel)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(InsertDebugMarker,
                                                 INSERT_DEBUG_MARKER_CMD_DATA_MEMBER){};

#define BEGIN_OCCLUSION_QUERY_CMD_DATA_MEMBER(X) X(uint32_t, queryIndex)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(BeginOcclusionQuery,
                                                 BEGIN_OCCLUSION_QUERY_CMD_DATA_MEMBER){};

#define EXECUTE_BUNDLES_CMD_DATA_MEMBER(X) X(std::vector<ObjectId>, bundleIds)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(ExecuteBundles, EXECUTE_BUNDLES_CMD_DATA_MEMBER){};

#define END_CMD_DATA_MEMBER(X)
DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(End, END_CMD_DATA_MEMBER){};

#define POP_DEBUG_GROUP_CMD_DATA_MEMBER(X)
DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(PopDebugGroup, POP_DEBUG_GROUP_CMD_DATA_MEMBER){};

#define END_OCCLUSION_QUERY_CMD_DATA_MEMBER(X)
DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(EndOcclusionQuery,
                                                 END_OCCLUSION_QUERY_CMD_DATA_MEMBER){};

#define SET_VERTEX_BUFFER_CMD_DATA_MEMBER(X) \
    X(uint32_t, slot)                        \
    X(ObjectId, bufferId)                    \
    X(uint64_t, offset)                      \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetVertexBuffer,
                                                 SET_VERTEX_BUFFER_CMD_DATA_MEMBER){};

#define SET_INDEX_BUFFER_CMD_DATA_MEMBER(X) \
    X(ObjectId, bufferId)                   \
    X(wgpu::IndexFormat, format)            \
    X(uint64_t, offset)                     \
    X(uint64_t, size)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetIndexBuffer,
                                                 SET_INDEX_BUFFER_CMD_DATA_MEMBER){};

#define SET_BLEND_CONSTANT_CMD_DATA_MEMBER(X) X(Color, color)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetBlendConstant,
                                                 SET_BLEND_CONSTANT_CMD_DATA_MEMBER){};

#define SET_SCISSOR_RECT_CMD_DATA_MEMBER(X) \
    X(uint32_t, x)                          \
    X(uint32_t, y)                          \
    X(uint32_t, width)                      \
    X(uint32_t, height)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetScissorRect,
                                                 SET_SCISSOR_RECT_CMD_DATA_MEMBER){};

#define SET_STENCIL_REFERENCE_CMD_DATA_MEMBER(X) X(uint32_t, reference)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetStencilReference,
                                                 SET_STENCIL_REFERENCE_CMD_DATA_MEMBER){};

#define SET_VIEWPORT_CMD_DATA_MEMBER(X) \
    X(float, x)                         \
    X(float, y)                         \
    X(float, width)                     \
    X(float, height)                    \
    X(float, minDepth)                  \
    X(float, maxDepth)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(SetViewport, SET_VIEWPORT_CMD_DATA_MEMBER){};

#define DRAW_CMD_DATA_MEMBER(X) \
    X(uint32_t, vertexCount)    \
    X(uint32_t, instanceCount)  \
    X(uint32_t, firstVertex)    \
    X(uint32_t, firstInstance)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(Draw, DRAW_CMD_DATA_MEMBER){};

#define DRAW_INDEXED_CMD_DATA_MEMBER(X) \
    X(uint32_t, indexCount)             \
    X(uint32_t, instanceCount)          \
    X(uint32_t, firstIndex)             \
    X(int32_t, baseVertex)              \
    X(uint32_t, firstInstance)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(DrawIndexed, DRAW_INDEXED_CMD_DATA_MEMBER){};

#define DRAW_INDIRECT_CMD_DATA_MEMBER(X) \
    X(ObjectId, indirectBufferId)        \
    X(uint64_t, indirectOffset)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(DrawIndirect, DRAW_INDIRECT_CMD_DATA_MEMBER){};

#define DRAW_INDEXED_INDIRECT_CMD_DATA_MEMBER(X) \
    X(ObjectId, indirectBufferId)                \
    X(uint64_t, indirectOffset)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(DrawIndexedIndirect,
                                                 DRAW_INDIRECT_CMD_DATA_MEMBER){};

#define WRITE_TIMESTAMP_CMD_DATA_MEMBER(X) \
    X(ObjectId, querySetId)                \
    X(uint32_t, queryIndex)

DAWN_REPLAY_MAKE_COMMAND_BUFFER_CMD_AND_CMD_DATA(WriteTimestamp, WRITE_TIMESTAMP_CMD_DATA_MEMBER){};

}  // namespace schema

#endif  // SRC_DAWN_SERIALIZATION_SCHEMA_H_
