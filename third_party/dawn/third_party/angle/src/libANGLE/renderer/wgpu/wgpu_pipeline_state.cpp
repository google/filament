//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/wgpu/wgpu_pipeline_state.h"

#include "common/aligned_memory.h"
#include "common/hash_utils.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{
namespace webgpu
{
// Can pack the index format into 1 bit since it has 2 values and Undefined is not used.
static_assert(static_cast<uint32_t>(wgpu::IndexFormat::Uint32) == 2U,
              "Max wgpu::IndexFormat is not 2");
static_assert(static_cast<uint32_t>(wgpu::IndexFormat::Undefined) == 0,
              "wgpu::IndexFormat::Undefined unexpected value");
constexpr uint32_t PackIndexFormat(wgpu::IndexFormat unpackedFormat)
{
    ASSERT(static_cast<uint32_t>(unpackedFormat) > 0);
    return static_cast<uint32_t>(unpackedFormat) - 1;
}

constexpr wgpu::IndexFormat UnpackIndexFormat(uint32_t packedIndexFormat)
{
    return static_cast<wgpu::IndexFormat>(packedIndexFormat + 1);
}

// Can pack the front face into 1 bit since it has 2 values and Undefined is not used.
static_assert(static_cast<uint32_t>(wgpu::FrontFace::CW) == 2U, "Max wgpu::FrontFace is not 2");
static_assert(static_cast<uint32_t>(wgpu::FrontFace::Undefined) == 0,
              "wgpu::FrontFace::Undefined unexpected value");
constexpr uint32_t PackFrontFace(wgpu::FrontFace unpackedFrontFace)
{
    ASSERT(static_cast<uint32_t>(unpackedFrontFace) > 0);
    return static_cast<uint32_t>(unpackedFrontFace) - 1;
}

constexpr wgpu::FrontFace UnpackFrontFace(uint32_t packedFrontFace)
{
    return static_cast<wgpu::FrontFace>(packedFrontFace + 1);
}

PackedVertexAttribute::PackedVertexAttribute()
{
    memset(this, 0, sizeof(PackedVertexAttribute));
}

// GraphicsPipelineDesc implementation.
RenderPipelineDesc::RenderPipelineDesc()
{
    (void)mPad0;
    memset(this, 0, sizeof(RenderPipelineDesc));
}

RenderPipelineDesc::~RenderPipelineDesc() = default;

RenderPipelineDesc::RenderPipelineDesc(const RenderPipelineDesc &other)
{
    *this = other;
}

RenderPipelineDesc &RenderPipelineDesc::operator=(const RenderPipelineDesc &other)
{
    memcpy(this, &other, sizeof(*this));
    return *this;
}

bool RenderPipelineDesc::setPrimitiveMode(gl::PrimitiveMode primitiveMode,
                                          gl::DrawElementsType indexTypeOrInvalid)
{
    bool changed = false;

    wgpu::PrimitiveTopology topology = gl_wgpu::GetPrimitiveTopology(primitiveMode);
    if (mPrimitiveState.topology != static_cast<uint8_t>(topology))
    {
        SetBitField(mPrimitiveState.topology, topology);
        changed = true;
    }

    uint32_t indexFormat = webgpu::IsStripPrimitiveTopology(topology) &&
                                   indexTypeOrInvalid != gl::DrawElementsType::InvalidEnum
                               ? PackIndexFormat(gl_wgpu::GetIndexFormat(indexTypeOrInvalid))
                               : 0;
    if (mPrimitiveState.stripIndexFormat != static_cast<uint8_t>(indexFormat))
    {
        SetBitField(mPrimitiveState.stripIndexFormat, indexFormat);
        changed = true;
    }

    return changed;
}

void RenderPipelineDesc::setFrontFace(GLenum frontFace)
{
    SetBitField(mPrimitiveState.frontFace, PackFrontFace(gl_wgpu::GetFrontFace(frontFace)));
}

void RenderPipelineDesc::setCullMode(gl::CullFaceMode cullMode, bool cullFaceEnabled)
{
    SetBitField(mPrimitiveState.cullMode, gl_wgpu::GetCullMode(cullMode, cullFaceEnabled));
}

void RenderPipelineDesc::setColorWriteMask(size_t colorIndex, bool r, bool g, bool b, bool a)
{
    PackedColorTargetState &colorTarget = mColorTargetStates[colorIndex];
    SetBitField(colorTarget.writeMask, gl_wgpu::GetColorWriteMask(r, g, b, a));
}

bool RenderPipelineDesc::setVertexAttribute(size_t attribIndex, PackedVertexAttribute &newAttrib)
{
    PackedVertexAttribute &currentAttrib = mVertexAttributes[attribIndex];
    if (memcmp(&currentAttrib, &newAttrib, sizeof(PackedVertexAttribute)) == 0)
    {
        return false;
    }

    memcpy(&currentAttrib, &newAttrib, sizeof(PackedVertexAttribute));
    return true;
}

bool RenderPipelineDesc::setColorAttachmentFormat(size_t colorIndex, wgpu::TextureFormat format)
{
    if (mColorTargetStates[colorIndex].format == static_cast<uint8_t>(format))
    {
        return false;
    }

    SetBitField(mColorTargetStates[colorIndex].format, format);
    return true;
}

bool RenderPipelineDesc::setDepthStencilAttachmentFormat(wgpu::TextureFormat format)
{
    if (mDepthStencilState.format == static_cast<uint8_t>(format))
    {
        return false;
    }

    SetBitField(mDepthStencilState.format, format);
    return true;
}

bool RenderPipelineDesc::setDepthFunc(wgpu::CompareFunction compareFunc)
{
    if (mDepthStencilState.depthCompare == static_cast<uint8_t>(compareFunc))
    {
        return false;
    }
    SetBitField(mDepthStencilState.depthCompare, compareFunc);
    return true;
}

bool RenderPipelineDesc::setStencilFrontFunc(wgpu::CompareFunction compareFunc)
{
    if (mDepthStencilState.stencilFrontCompare == static_cast<uint8_t>(compareFunc))
    {
        return false;
    }
    SetBitField(mDepthStencilState.stencilFrontCompare, compareFunc);
    return true;
}

bool RenderPipelineDesc::setStencilFrontOps(wgpu::StencilOperation failOp,
                                            wgpu::StencilOperation depthFailOp,
                                            wgpu::StencilOperation passOp)
{
    if (mDepthStencilState.stencilFrontFailOp == static_cast<uint8_t>(failOp) &&
        mDepthStencilState.stencilFrontDepthFailOp == static_cast<uint8_t>(depthFailOp) &&
        mDepthStencilState.stencilFrontPassOp == static_cast<uint8_t>(passOp))
    {
        return false;
    }
    SetBitField(mDepthStencilState.stencilFrontFailOp, failOp);
    SetBitField(mDepthStencilState.stencilFrontDepthFailOp, depthFailOp);
    SetBitField(mDepthStencilState.stencilFrontPassOp, passOp);
    return true;
}

bool RenderPipelineDesc::setStencilBackFunc(wgpu::CompareFunction compareFunc)
{
    if (mDepthStencilState.stencilBackCompare == static_cast<uint8_t>(compareFunc))
    {
        return false;
    }
    SetBitField(mDepthStencilState.stencilBackCompare, compareFunc);
    return true;
}

bool RenderPipelineDesc::setStencilBackOps(wgpu::StencilOperation failOp,
                                           wgpu::StencilOperation depthFailOp,
                                           wgpu::StencilOperation passOp)
{
    if (mDepthStencilState.stencilBackFailOp == static_cast<uint8_t>(failOp) &&
        mDepthStencilState.stencilBackDepthFailOp == static_cast<uint8_t>(depthFailOp) &&
        mDepthStencilState.stencilBackPassOp == static_cast<uint8_t>(passOp))
    {
        return false;
    }
    SetBitField(mDepthStencilState.stencilBackFailOp, failOp);
    SetBitField(mDepthStencilState.stencilBackDepthFailOp, depthFailOp);
    SetBitField(mDepthStencilState.stencilBackPassOp, passOp);
    return true;
}

bool RenderPipelineDesc::setStencilReadMask(uint8_t readMask)
{

    if (mDepthStencilState.stencilReadMask == readMask)
    {
        return false;
    }
    mDepthStencilState.stencilReadMask = readMask;
    return true;
}

bool RenderPipelineDesc::setStencilWriteMask(uint8_t writeMask)
{
    if (mDepthStencilState.stencilWriteMask == writeMask)
    {
        return false;
    }
    mDepthStencilState.stencilWriteMask = writeMask;
    return true;
}

size_t RenderPipelineDesc::hash() const
{
    return angle::ComputeGenericHash(this, sizeof(*this));
}

angle::Result RenderPipelineDesc::createPipeline(ContextWgpu *context,
                                                 const wgpu::PipelineLayout &pipelineLayout,
                                                 const gl::ShaderMap<wgpu::ShaderModule> &shaders,
                                                 wgpu::RenderPipeline *pipelineOut) const
{
    wgpu::RenderPipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pipelineLayout;

    pipelineDesc.vertex.module        = shaders[gl::ShaderType::Vertex];
    pipelineDesc.vertex.entryPoint    = "wgslMain";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants     = nullptr;
    pipelineDesc.vertex.bufferCount   = 0;
    pipelineDesc.vertex.buffers       = nullptr;

    pipelineDesc.primitive.topology =
        static_cast<wgpu::PrimitiveTopology>(mPrimitiveState.topology);
    if (webgpu::IsStripPrimitiveTopology(pipelineDesc.primitive.topology))
    {
        pipelineDesc.primitive.stripIndexFormat =
            UnpackIndexFormat(mPrimitiveState.stripIndexFormat);
    }
    else
    {
        pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    }
    pipelineDesc.primitive.frontFace = UnpackFrontFace(mPrimitiveState.frontFace);
    pipelineDesc.primitive.cullMode  = static_cast<wgpu::CullMode>(mPrimitiveState.cullMode);

    size_t attribCount = 0;
    gl::AttribArray<wgpu::VertexBufferLayout> vertexBuffers;
    gl::AttribArray<wgpu::VertexAttribute> vertexAttribs;

    for (PackedVertexAttribute packedAttrib : mVertexAttributes)
    {
        if (!packedAttrib.enabled)
        {
            continue;
        }

        wgpu::VertexAttribute &newAttribute = vertexAttribs[attribCount];
        newAttribute.format                 = static_cast<wgpu::VertexFormat>(packedAttrib.format);
        newAttribute.offset                 = packedAttrib.offset;
        newAttribute.shaderLocation         = packedAttrib.shaderLocation;

        wgpu::VertexBufferLayout &newBufferLayout = vertexBuffers[attribCount];
        newBufferLayout.arrayStride               = packedAttrib.stride;
        newBufferLayout.attributeCount            = 1;
        newBufferLayout.attributes                = &newAttribute;

        attribCount++;
    }

    pipelineDesc.vertex.bufferCount = attribCount;
    pipelineDesc.vertex.buffers     = vertexBuffers.data();

    wgpu::FragmentState fragmentState;
    std::array<wgpu::ColorTargetState, gl::IMPLEMENTATION_MAX_DRAW_BUFFERS> colorTargets;
    std::array<wgpu::BlendState, gl::IMPLEMENTATION_MAX_DRAW_BUFFERS> blendStates;
    if (shaders[gl::ShaderType::Fragment])
    {
        fragmentState.module        = shaders[gl::ShaderType::Fragment];
        fragmentState.entryPoint    = "wgslMain";
        fragmentState.constantCount = 0;
        fragmentState.constants     = nullptr;

        size_t colorTargetCount = 0;
        for (size_t colorTargetIndex = 0; colorTargetIndex < gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
             ++colorTargetIndex)
        {
            const webgpu::PackedColorTargetState &packedColorTarget =
                mColorTargetStates[colorTargetIndex];
            wgpu::ColorTargetState &outputColorTarget = colorTargets[colorTargetIndex];

            outputColorTarget.format = static_cast<wgpu::TextureFormat>(packedColorTarget.format);
            if (packedColorTarget.blendEnabled)
            {
                blendStates[colorTargetIndex].color.srcFactor =
                    static_cast<wgpu::BlendFactor>(packedColorTarget.colorBlendSrcFactor);
                blendStates[colorTargetIndex].color.dstFactor =
                    static_cast<wgpu::BlendFactor>(packedColorTarget.colorBlendDstFactor);
                blendStates[colorTargetIndex].color.operation =
                    static_cast<wgpu::BlendOperation>(packedColorTarget.colorBlendOp);

                blendStates[colorTargetIndex].alpha.srcFactor =
                    static_cast<wgpu::BlendFactor>(packedColorTarget.alphaBlendSrcFactor);
                blendStates[colorTargetIndex].alpha.dstFactor =
                    static_cast<wgpu::BlendFactor>(packedColorTarget.alphaBlendDstFactor);
                blendStates[colorTargetIndex].alpha.operation =
                    static_cast<wgpu::BlendOperation>(packedColorTarget.alphaBlendOp);
            }

            outputColorTarget.writeMask =
                static_cast<wgpu::ColorWriteMask>(packedColorTarget.writeMask);

            if (outputColorTarget.format != wgpu::TextureFormat::Undefined)
            {
                colorTargetCount = colorTargetIndex + 1;
            }
        }
        fragmentState.targetCount = colorTargetCount;
        fragmentState.targets     = colorTargets.data();

        pipelineDesc.fragment = &fragmentState;
    }

    wgpu::DepthStencilState depthStencilState;
    if (static_cast<wgpu::TextureFormat>(mDepthStencilState.format) !=
        wgpu::TextureFormat::Undefined)
    {
        const webgpu::PackedDepthStencilState &packedDepthStencilState = mDepthStencilState;

        depthStencilState.format = static_cast<wgpu::TextureFormat>(packedDepthStencilState.format);
        depthStencilState.depthWriteEnabled =
            static_cast<bool>(packedDepthStencilState.depthWriteEnabled);
        depthStencilState.depthCompare =
            static_cast<wgpu::CompareFunction>(packedDepthStencilState.depthCompare);

        depthStencilState.stencilFront.compare =
            static_cast<wgpu::CompareFunction>(packedDepthStencilState.stencilFrontCompare);
        depthStencilState.stencilFront.failOp =
            static_cast<wgpu::StencilOperation>(packedDepthStencilState.stencilFrontFailOp);
        depthStencilState.stencilFront.depthFailOp =
            static_cast<wgpu::StencilOperation>(packedDepthStencilState.stencilFrontDepthFailOp);
        depthStencilState.stencilFront.passOp =
            static_cast<wgpu::StencilOperation>(packedDepthStencilState.stencilFrontPassOp);

        depthStencilState.stencilBack.compare =
            static_cast<wgpu::CompareFunction>(packedDepthStencilState.stencilBackCompare);
        depthStencilState.stencilBack.failOp =
            static_cast<wgpu::StencilOperation>(packedDepthStencilState.stencilBackFailOp);
        depthStencilState.stencilBack.depthFailOp =
            static_cast<wgpu::StencilOperation>(packedDepthStencilState.stencilBackDepthFailOp);
        depthStencilState.stencilBack.passOp =
            static_cast<wgpu::StencilOperation>(packedDepthStencilState.stencilBackPassOp);

        depthStencilState.stencilReadMask  = packedDepthStencilState.stencilReadMask;
        depthStencilState.stencilWriteMask = packedDepthStencilState.stencilWriteMask;

        depthStencilState.depthBias           = packedDepthStencilState.depthBias;
        depthStencilState.depthBiasSlopeScale = packedDepthStencilState.depthBiasSlopeScalef;
        depthStencilState.depthBiasClamp      = packedDepthStencilState.depthBiasClamp;

        pipelineDesc.depthStencil = &depthStencilState;
    }

    wgpu::Device device = context->getDevice();
    ANGLE_WGPU_SCOPED_DEBUG_TRY(context, *pipelineOut = device.CreateRenderPipeline(&pipelineDesc));

    return angle::Result::Continue;
}

bool operator==(const RenderPipelineDesc &lhs, const RenderPipelineDesc &rhs)
{
    return memcmp(&lhs, &rhs, sizeof(RenderPipelineDesc)) == 0;
}

// PipelineCache implementation.
PipelineCache::PipelineCache()  = default;
PipelineCache::~PipelineCache() = default;

angle::Result PipelineCache::getRenderPipeline(ContextWgpu *context,
                                               const RenderPipelineDesc &desc,
                                               const wgpu::PipelineLayout &pipelineLayout,
                                               const gl::ShaderMap<wgpu::ShaderModule> &shaders,
                                               wgpu::RenderPipeline *pipelineOut)
{
    auto iter = mRenderPipelines.find(desc);
    if (iter != mRenderPipelines.end())
    {
        *pipelineOut = iter->second;
        return angle::Result::Continue;
    }

    ANGLE_TRY(desc.createPipeline(context, pipelineLayout, shaders, pipelineOut));
    mRenderPipelines.insert(std::make_pair(desc, *pipelineOut));

    return angle::Result::Continue;
}

}  // namespace webgpu

}  // namespace rx
