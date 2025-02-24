//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_WGPU_PIPELINE_STATE_H_
#define LIBANGLE_RENDERER_WGPU_PIPELINE_STATE_H_

#include <dawn/webgpu_cpp.h>
#include <stdint.h>
#include <limits>

#include "libANGLE/Constants.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"

#include "common/PackedEnums.h"

namespace rx
{
class ContextWgpu;

namespace webgpu
{
ANGLE_ENABLE_STRUCT_PADDING_WARNINGS

constexpr uint32_t kPrimitiveTopologyBitCount = 3;
constexpr uint32_t kIndexFormatBitCount       = 1;
constexpr uint32_t kFrontFaceBitCount         = 1;
constexpr uint32_t kCullModeBitCount          = 2;

struct PackedPrimitiveState final
{
    uint8_t topology : kPrimitiveTopologyBitCount;
    uint8_t stripIndexFormat : kIndexFormatBitCount;
    uint8_t frontFace : kFrontFaceBitCount;
    uint8_t cullMode : kCullModeBitCount;
    uint8_t pad0 : 1;
};

constexpr size_t kPackedPrimitiveStateSize = sizeof(PackedPrimitiveState);
static_assert(kPackedPrimitiveStateSize == 1, "Size mismatch");

constexpr uint32_t kTextureFormatBitCount  = 19;
constexpr uint32_t kColorWriteMaskBitCount = 4;
constexpr uint32_t kBlendFactorBitCount    = 5;
constexpr uint32_t kBlendOperationBitCount = 3;

struct PackedColorTargetState final
{
    uint32_t format : kTextureFormatBitCount;
    uint32_t blendEnabled : 1;
    uint32_t colorBlendSrcFactor : kBlendFactorBitCount;
    uint32_t colorBlendDstFactor : kBlendFactorBitCount;
    uint32_t pad0 : 2;
    uint32_t colorBlendOp : kBlendOperationBitCount;
    uint32_t alphaBlendSrcFactor : kBlendFactorBitCount;
    uint32_t alphaBlendDstFactor : kBlendFactorBitCount;
    uint32_t alphaBlendOp : kBlendOperationBitCount;
    uint32_t writeMask : kColorWriteMaskBitCount;
    uint32_t pad1 : 12;
};

constexpr size_t kPackedColorTargetStateSize = sizeof(PackedColorTargetState);
static_assert(kPackedColorTargetStateSize == 8, "Size mismatch");

constexpr uint32_t kCompareFunctionBitCount  = 4;
constexpr uint32_t kStencilOperationBitCount = 4;

struct PackedDepthStencilState final
{
    uint32_t format : kTextureFormatBitCount;

    uint32_t depthWriteEnabled : 1;
    uint32_t depthCompare : kCompareFunctionBitCount;

    uint32_t stencilFrontCompare : kCompareFunctionBitCount;
    uint32_t stencilFrontFailOp : kStencilOperationBitCount;
    uint32_t stencilFrontDepthFailOp : kStencilOperationBitCount;
    uint32_t stencilFrontPassOp : kStencilOperationBitCount;

    uint32_t stencilBackCompare : kCompareFunctionBitCount;
    uint32_t stencilBackFailOp : kStencilOperationBitCount;
    uint32_t stencilBackDepthFailOp : kStencilOperationBitCount;
    uint32_t stencilBackPassOp : kStencilOperationBitCount;

    uint32_t pad0 : 8;

    uint8_t stencilReadMask;
    uint8_t stencilWriteMask;

    uint8_t pad1[2];

    int32_t depthBias;
    float depthBiasSlopeScalef;
    float depthBiasClamp;
};

constexpr size_t kPackedDepthStencilStateSize = sizeof(PackedDepthStencilState);
static_assert(kPackedDepthStencilStateSize == 24, "Size mismatch");

constexpr uint32_t kVertexFormatBitCount = 5;

// A maximum offset of 4096 covers almost every Vulkan driver on desktop (80%) and mobile (99%). The
// next highest values to meet native drivers are 16 bits or 32 bits.
constexpr uint32_t kAttributeOffsetMaxBits = 15;

// In WebGPU, the maxVertexBufferArrayStride will be at least 2048.
constexpr uint32_t kVertexAttributeStrideBits = 16;

struct PackedVertexAttribute final
{
    PackedVertexAttribute();

    uint16_t offset : kAttributeOffsetMaxBits;
    uint16_t enabled : 1;
    uint8_t format : kVertexFormatBitCount;
    uint8_t pad1 : 3;
    uint8_t shaderLocation;
    uint16_t stride : kVertexAttributeStrideBits;
};

constexpr size_t kPackedVertexAttributeSize = sizeof(PackedVertexAttribute);
static_assert(kPackedVertexAttributeSize == 6, "Size mismatch");

class RenderPipelineDesc final
{
  public:
    RenderPipelineDesc();
    ~RenderPipelineDesc();
    RenderPipelineDesc(const RenderPipelineDesc &other);
    RenderPipelineDesc &operator=(const RenderPipelineDesc &other);

    // Returns true if the pipeline description has changed

    bool setPrimitiveMode(gl::PrimitiveMode primitiveMode, gl::DrawElementsType indexTypeOrInvalid);

    void setFrontFace(GLenum frontFace);
    void setCullMode(gl::CullFaceMode cullMode, bool cullFaceEnabled);
    void setColorWriteMask(size_t colorIndex, bool r, bool g, bool b, bool a);

    bool setVertexAttribute(size_t attribIndex, PackedVertexAttribute &newAttrib);
    bool setColorAttachmentFormat(size_t colorIndex, wgpu::TextureFormat format);
    bool setDepthStencilAttachmentFormat(wgpu::TextureFormat format);
    bool setDepthFunc(wgpu::CompareFunction compareFunc);
    bool setStencilFrontFunc(wgpu::CompareFunction compareFunc);
    bool setStencilFrontOps(wgpu::StencilOperation failOp,
                            wgpu::StencilOperation depthFailOp,
                            wgpu::StencilOperation passOp);
    bool setStencilBackFunc(wgpu::CompareFunction compareFunc);
    bool setStencilBackOps(wgpu::StencilOperation failOp,
                           wgpu::StencilOperation depthFailOp,
                           wgpu::StencilOperation passOp);

    bool setStencilReadMask(uint8_t readeMask);
    bool setStencilWriteMask(uint8_t writeMask);

    size_t hash() const;

    angle::Result createPipeline(ContextWgpu *context,
                                 const wgpu::PipelineLayout &pipelineLayout,
                                 const gl::ShaderMap<wgpu::ShaderModule> &shaders,
                                 wgpu::RenderPipeline *pipelineOut) const;

  private:
    PackedVertexAttribute mVertexAttributes[gl::MAX_VERTEX_ATTRIBS];
    PackedColorTargetState mColorTargetStates[gl::IMPLEMENTATION_MAX_DRAW_BUFFERS];
    PackedDepthStencilState mDepthStencilState;
    PackedPrimitiveState mPrimitiveState;
    uint8_t mPad0[3];
};

constexpr size_t kRenderPipelineDescSize = sizeof(RenderPipelineDesc);
static_assert(kRenderPipelineDescSize % 4 == 0,
              "RenderPipelineDesc size must be a multiple of 4 bytes.");

bool operator==(const RenderPipelineDesc &lhs, const RenderPipelineDesc &rhs);

ANGLE_DISABLE_STRUCT_PADDING_WARNINGS

}  // namespace webgpu
}  // namespace rx

// Introduce std::hash for the above classes.
namespace std
{
template <>
struct hash<rx::webgpu::RenderPipelineDesc>
{
    size_t operator()(const rx::webgpu::RenderPipelineDesc &key) const { return key.hash(); }
};
}  // namespace std

namespace rx
{
namespace webgpu
{

class PipelineCache final
{
  public:
    PipelineCache();
    ~PipelineCache();

    angle::Result getRenderPipeline(ContextWgpu *context,
                                    const RenderPipelineDesc &desc,
                                    const wgpu::PipelineLayout &pipelineLayout,
                                    const gl::ShaderMap<wgpu::ShaderModule> &shaders,
                                    wgpu::RenderPipeline *pipelineOut);

  private:
    std::unordered_map<RenderPipelineDesc, wgpu::RenderPipeline> mRenderPipelines;
};

}  // namespace webgpu

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_PIPELINE_STATE_H_
