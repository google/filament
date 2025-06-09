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

#include "WebGPUProgram.h"
#include "WebGPURenderTarget.h"
#include "WebGPUVertexBufferInfo.h"

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

bool hasStencilAspect(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::Stencil8:
        case wgpu::TextureFormat::Depth24PlusStencil8:
        case wgpu::TextureFormat::Depth32FloatStencil8:
            return true;
        default:
            return false;
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
        WebGPUProgram const& program, WebGPUVertexBufferInfo const& vertexBufferInfo,
        wgpu::PipelineLayout const& layout, RasterState const& rasterState,
        StencilState const& stencilState, PolygonOffset const& polygonOffset,
        const PrimitiveType primitiveType, std::vector<wgpu::TextureFormat> const& colorFormats,
        const wgpu::TextureFormat depthFormat, const uint8_t samplesCount) {
    assert_invariant(program.vertexShaderModule);
    wgpu::DepthStencilState depthStencilState{};
    if (depthFormat != wgpu::TextureFormat::Undefined) {
        depthStencilState.format = depthFormat;
        depthStencilState.depthWriteEnabled = rasterState.depthWrite;
        depthStencilState.depthCompare = toWebGPU(rasterState.depthFunc);
        depthStencilState.stencilFront = {
            .compare = toWebGPU(stencilState.front.stencilFunc),
            .failOp = toWebGPU(stencilState.front.stencilOpStencilFail),
            .depthFailOp = toWebGPU(stencilState.front.stencilOpDepthFail),
            .passOp = toWebGPU(stencilState.front.stencilOpDepthStencilPass),
        };
        depthStencilState.stencilBack = {
            .compare = toWebGPU(stencilState.back.stencilFunc),
            .failOp = toWebGPU(stencilState.back.stencilOpStencilFail),
            .depthFailOp = toWebGPU(stencilState.back.stencilOpDepthFail),
            .passOp = toWebGPU(stencilState.back.stencilOpDepthStencilPass),
        };
        depthStencilState.stencilReadMask =
                stencilState.front.readMask; // Use front face's comparison mask for read mask
        depthStencilState.stencilWriteMask = stencilState.stencilWrite ? 0xFFFFFFFF : 0u;
        depthStencilState.depthBias = static_cast<int32_t>(polygonOffset.constant);
        depthStencilState.depthBiasSlopeScale = polygonOffset.slope;
        depthStencilState.depthBiasClamp = 0.0f;

        if (!hasStencilAspect(depthFormat)) {
            depthStencilState.stencilFront.compare = wgpu::CompareFunction::Always;
            depthStencilState.stencilFront.failOp = wgpu::StencilOperation::Keep;
            depthStencilState.stencilFront.depthFailOp = wgpu::StencilOperation::Keep;
            depthStencilState.stencilFront.passOp = wgpu::StencilOperation::Keep;
            depthStencilState.stencilBack =
                    depthStencilState.stencilFront; // Keep back and front consistent
            depthStencilState.stencilReadMask = 0;
            depthStencilState.stencilWriteMask = 0;
        }
    }

    std::stringstream pipelineLabelStream;
    pipelineLabelStream << program.name.c_str() << " pipeline";
    const auto pipelineLabel = pipelineLabelStream.str();
    wgpu::RenderPipelineDescriptor pipelineDescriptor{
        .label = wgpu::StringView(pipelineLabel),
        .layout = layout,
        .vertex = { .module = program.vertexShaderModule,
            .entryPoint = "main",
            // we do not use WebGPU's override constants due to 2 limitations
            // (at least at the time of write this):
            // 1. they cannot be used for the size of an array, which is needed
            // 2. if we pass the WebGPU API (CPU-side) constants not referenced in the
            //    shader WebGPU fails. This is a problem with how Filament is designed,
            //    where certain constants may be optimized out of the shader based
            //    on build configuration, etc.
            //
            // to bypass these problems, we do not use override constants in the
            // WebGPU backend, instead replacing placeholder constants in the shader
            // text before creating the shader module (essentially implementing
            // override constants ourselves)
            .constantCount = 0,
            .constants = nullptr,
            .bufferCount = vertexBufferInfo.getVertexBufferLayoutCount(),
            .buffers = vertexBufferInfo.getVertexBufferLayouts()
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
        .depthStencil = depthFormat != wgpu::TextureFormat::Undefined ? &depthStencilState: nullptr,
        .multisample = {
            .count = samplesCount,
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
        // see the comment about constants for the vertex state, as the same reasoning applies
        // here
        fragmentState.constantCount = 0,
        fragmentState.constants = nullptr,
        fragmentState.targetCount = colorFormats.size();
        fragmentState.targets = colorTargets.data();
        assert_invariant(fragmentState.targetCount <= MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
        // We expect a fragment shader implies at least one color target if it outputs color.
        // This should be guaranteed by the caller ensuring colorFormats is not empty.
        // However, this fails on shadowtest.cpp, TODO investigate why
        // assert_invariant(fragmentState.targetCount > 0);
        for (size_t targetIndex = 0; targetIndex < fragmentState.targetCount; targetIndex++) {
            auto& colorTarget = colorTargets[targetIndex];
            colorTarget.format = colorFormats[targetIndex];
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
