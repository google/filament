/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "WebGPUPipelineCreation.h"

#include "WebGPUHandles.h"

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <utils/Panic.h>
#include <utils/debug.h>

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <sstream>

namespace filament::backend {

namespace {

constexpr wgpu::PrimitiveTopology toWebGPU(PrimitiveType primitiveType) {
    switch (primitiveType) {
        case PrimitiveType::POINTS:
            return wgpu::PrimitiveTopology::PointList;
        case PrimitiveType::LINES:
            return wgpu::PrimitiveTopology::LineList;
        case PrimitiveType::LINE_STRIP:
            return wgpu::PrimitiveTopology::LineStrip;
        case PrimitiveType::TRIANGLES:
            return wgpu::PrimitiveTopology::TriangleList;
        case PrimitiveType::TRIANGLE_STRIP:
            return wgpu::PrimitiveTopology::TriangleStrip;
    }
}

constexpr wgpu::CullMode toWebGPU(CullingMode cullMode) {
    switch (cullMode) {
        case CullingMode::NONE:
            return wgpu::CullMode::None;
        case CullingMode::FRONT:
            return wgpu::CullMode::Front;
        case CullingMode::BACK:
            return wgpu::CullMode::Back;
        case CullingMode::FRONT_AND_BACK:
            // no WegGPU equivalent of front and back
            FILAMENT_CHECK_POSTCONDITION(false)
                    << "WebGPU does not support CullingMode::FRONT_AND_BACK";
            return wgpu::CullMode::Undefined;
    }
}

constexpr wgpu::CompareFunction toWebGPU(SamplerCompareFunc compareFunction) {
    switch (compareFunction) {
        case SamplerCompareFunc::LE:
            return wgpu::CompareFunction::LessEqual;
        case SamplerCompareFunc::GE:
            return wgpu::CompareFunction::GreaterEqual;
        case SamplerCompareFunc::L:
            return wgpu::CompareFunction::Less;
        case SamplerCompareFunc::G:
            return wgpu::CompareFunction::Greater;
        case SamplerCompareFunc::E:
            return wgpu::CompareFunction::Equal;
        case SamplerCompareFunc::NE:
            return wgpu::CompareFunction::NotEqual;
        case SamplerCompareFunc::A:
            return wgpu::CompareFunction::Always;
        case SamplerCompareFunc::N:
            return wgpu::CompareFunction::Never;
    }
}

constexpr wgpu::StencilOperation toWebGPU(StencilOperation stencilOp) {
    switch (stencilOp) {
        case StencilOperation::KEEP:
            return wgpu::StencilOperation::Keep;
        case StencilOperation::ZERO:
            return wgpu::StencilOperation::Zero;
        case StencilOperation::REPLACE:
            return wgpu::StencilOperation::Replace;
        case StencilOperation::INCR:
            return wgpu::StencilOperation::IncrementClamp;
        case StencilOperation::INCR_WRAP:
            return wgpu::StencilOperation::IncrementWrap;
        case StencilOperation::DECR:
            return wgpu::StencilOperation::DecrementClamp;
        case StencilOperation::DECR_WRAP:
            return wgpu::StencilOperation::DecrementWrap;
        case StencilOperation::INVERT:
            return wgpu::StencilOperation::Invert;
    }
}

constexpr wgpu::BlendOperation toWebGPU(BlendEquation blendOp) {
    switch (blendOp) {
        case BlendEquation::ADD:
            return wgpu::BlendOperation::Add;
        case BlendEquation::SUBTRACT:
            return wgpu::BlendOperation::Subtract;
        case BlendEquation::REVERSE_SUBTRACT:
            return wgpu::BlendOperation::ReverseSubtract;
        case BlendEquation::MIN:
            return wgpu::BlendOperation::Min;
        case BlendEquation::MAX:
            return wgpu::BlendOperation::Max;
    }
}

constexpr wgpu::BlendFactor toWebGPU(BlendFunction blendFunction) {
    switch (blendFunction) {
        case BlendFunction::ZERO:
            return wgpu::BlendFactor::Zero;
        case BlendFunction::ONE:
            return wgpu::BlendFactor::One;
        case BlendFunction::SRC_COLOR:
            return wgpu::BlendFactor::Src;
        case BlendFunction::ONE_MINUS_SRC_COLOR:
            return wgpu::BlendFactor::OneMinusSrc;
        case BlendFunction::DST_COLOR:
            return wgpu::BlendFactor::Dst;
        case BlendFunction::ONE_MINUS_DST_COLOR:
            return wgpu::BlendFactor::OneMinusDst;
        case BlendFunction::SRC_ALPHA:
            return wgpu::BlendFactor::SrcAlpha;
        case BlendFunction::ONE_MINUS_SRC_ALPHA:
            return wgpu::BlendFactor::OneMinusSrcAlpha;
        case BlendFunction::DST_ALPHA:
            return wgpu::BlendFactor::DstAlpha;
        case BlendFunction::ONE_MINUS_DST_ALPHA:
            return wgpu::BlendFactor::OneMinusDstAlpha;
        case BlendFunction::SRC_ALPHA_SATURATE:
            return wgpu::BlendFactor::SrcAlphaSaturated;
    }
}

}// namespace

wgpu::RenderPipeline createWebGPURenderPipeline(wgpu::Device const& device,
        WGPUProgram const& program, WGPUVertexBufferInfo const& vertexBufferInfo,
        wgpu::PipelineLayout const& layout, RasterState const& rasterState,
        StencilState const& stencilState, PolygonOffset const& polygonOffset,
        PrimitiveType primitiveType, wgpu::TextureFormat colorFormat,
        wgpu::TextureFormat depthFormat) {
    assert_invariant(program.vertexShaderModule);
    const wgpu::DepthStencilState depthStencilState {
        .format = depthFormat,
        .depthWriteEnabled = rasterState.depthWrite,
        .depthCompare = toWebGPU(rasterState.depthFunc),
        .stencilFront = {
            .compare = toWebGPU(stencilState.front.stencilFunc),
            .failOp = toWebGPU(stencilState.front.stencilOpStencilFail),
            .depthFailOp = toWebGPU(stencilState.front.stencilOpDepthFail),
            .passOp = toWebGPU(stencilState.front.stencilOpDepthStencilPass),
        },
        .stencilBack = {
            .compare = toWebGPU(stencilState.back.stencilFunc),
            .failOp = toWebGPU(stencilState.back.stencilOpStencilFail),
            .depthFailOp = toWebGPU(stencilState.back.stencilOpDepthFail),
            .passOp = toWebGPU(stencilState.back.stencilOpDepthStencilPass),
        },
        .stencilReadMask = 0,
        .stencilWriteMask = stencilState.stencilWrite ? 0xFFFFFFFF : 0,
        .depthBias = static_cast<int32_t>(polygonOffset.constant),
        .depthBiasSlopeScale = polygonOffset.slope,
        .depthBiasClamp = 0.0f
    };
    std::stringstream pipelineLabelStream;
    pipelineLabelStream << program.name.c_str() << " pipeline";
    const auto pipelineLabel = pipelineLabelStream.str();
    wgpu::RenderPipelineDescriptor pipelineDescriptor{
        .label = wgpu::StringView(pipelineLabel),
        .layout = layout,
        .vertex = {
            .module = program.vertexShaderModule,
            .entryPoint = "main",
            .constantCount = program.constants.size(),
            .constants = program.constants.data(),
            .bufferCount = vertexBufferInfo.getVertexBufferLayoutSize(),
            .buffers = vertexBufferInfo.getVertexBufferLayout()
        },
        .primitive = {
            .topology = toWebGPU(primitiveType),
            // TODO should we assume some constant format here or is there a way to get
            //      this from PipelineState somehow or elsewhere?
            //      Perhaps, cache/assert format from index buffers as they are requested?
            .stripIndexFormat = wgpu::IndexFormat::Undefined,
            .frontFace = rasterState.inverseFrontFaces ? wgpu::FrontFace::CW : wgpu::FrontFace::CCW,
            .cullMode = toWebGPU(rasterState.culling),
            // TODO no depth clamp in WebGPU supported directly. unclippedDepth is close, so we are
            //      starting there
            .unclippedDepth = !rasterState.depthClamp &&
                              device.HasFeature(wgpu::FeatureName::DepthClipControl)
        },
        .depthStencil = &depthStencilState,
        .multisample = {
            .count = 1, // TODO need to get this from the render target
            .mask = 0xFFFFFFFF,
            .alphaToCoverageEnabled = rasterState.alphaToCoverage
        },
        .fragment = nullptr // will add below if fragment module is included
    };
    wgpu::FragmentState fragmentState = {};
    std::array<wgpu::ColorTargetState, MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT> colorTargets {};
    const wgpu::BlendState blendState {
        .color = {
            .operation = toWebGPU(rasterState.blendEquationRGB),
            .srcFactor = toWebGPU(rasterState.blendFunctionSrcRGB),
            .dstFactor = toWebGPU(rasterState.blendFunctionDstRGB)
        },
        .alpha = {
            .operation = toWebGPU(rasterState.blendEquationAlpha),
            .srcFactor = toWebGPU(rasterState.blendFunctionSrcAlpha),
            .dstFactor = toWebGPU(rasterState.blendFunctionDstAlpha)
        }
    };
    if (program.fragmentShaderModule != nullptr) {
        fragmentState.module = program.fragmentShaderModule;
        fragmentState.entryPoint = "main";
        fragmentState.constantCount = program.constants.size(),
        fragmentState.constants = program.constants.data(),
        fragmentState.targetCount = 1; // TODO need to get this from the render target
        assert_invariant(fragmentState.targetCount <= MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
        for (size_t targetIndex = 0; targetIndex < fragmentState.targetCount; targetIndex++) {
            auto& colorTarget = colorTargets[targetIndex];
            colorTarget.format = colorFormat;
            colorTarget.blend = rasterState.hasBlending() ? &blendState : nullptr;
            colorTarget.writeMask =
                    rasterState.colorWrite ? wgpu::ColorWriteMask::All : wgpu::ColorWriteMask::None;
        }
        pipelineDescriptor.fragment = &fragmentState;
    }
    const wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);
    FILAMENT_CHECK_POSTCONDITION(pipeline)
            << "Failed to create render pipeline for " << pipelineDescriptor.label;
    return pipeline;
}

}// namespace filament::backend
