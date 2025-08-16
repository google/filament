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

#include "WebGPUPipelineCache.h"

#include "WebGPUConstants.h"
#include "WebGPUTextureHelpers.h"
#include "WebGPUVertexBufferInfo.h"

#include <backend/DriverEnums.h>
#include <backend/TargetBufferInfo.h>

#include <utils/BitmaskEnum.h>
#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <cstring>

namespace filament::backend {

namespace {

[[nodiscard]] constexpr uint8_t toUint8(const bool value) { return value ? 1 : 0; }

[[nodiscard]] constexpr wgpu::PrimitiveTopology toWebGPU(const PrimitiveType primitiveType) {
    switch (primitiveType) {
        case PrimitiveType::POINTS:         return wgpu::PrimitiveTopology::PointList;
        case PrimitiveType::LINES:          return wgpu::PrimitiveTopology::LineList;
        case PrimitiveType::LINE_STRIP:     return wgpu::PrimitiveTopology::LineStrip;
        case PrimitiveType::TRIANGLES:      return wgpu::PrimitiveTopology::TriangleList;
        case PrimitiveType::TRIANGLE_STRIP: return wgpu::PrimitiveTopology::TriangleStrip;
    }
}

[[nodiscard]] constexpr wgpu::CullMode toWebGPU(const CullingMode cullMode) {
    switch (cullMode) {
        case CullingMode::NONE:  return wgpu::CullMode::None;
        case CullingMode::FRONT: return wgpu::CullMode::Front;
        case CullingMode::BACK:  return wgpu::CullMode::Back;
        case CullingMode::FRONT_AND_BACK:
            // WebGPU does not support culling both front and back faces simultaneously.
            FILAMENT_CHECK_POSTCONDITION(false)
                    << "WebGPU does not support CullingMode::FRONT_AND_BACK";
            return wgpu::CullMode::Undefined;
    }
}

[[nodiscard]] constexpr wgpu::CompareFunction toWebGPU(const SamplerCompareFunc compareFunction) {
    switch (compareFunction) {
        case SamplerCompareFunc::LE: return wgpu::CompareFunction::LessEqual;
        case SamplerCompareFunc::GE: return wgpu::CompareFunction::GreaterEqual;
        case SamplerCompareFunc::L:  return wgpu::CompareFunction::Less;
        case SamplerCompareFunc::G:  return wgpu::CompareFunction::Greater;
        case SamplerCompareFunc::E:  return wgpu::CompareFunction::Equal;
        case SamplerCompareFunc::NE: return wgpu::CompareFunction::NotEqual;
        case SamplerCompareFunc::A:  return wgpu::CompareFunction::Always;
        case SamplerCompareFunc::N:  return wgpu::CompareFunction::Never;
    }
}

[[nodiscard]] constexpr wgpu::StencilOperation toWebGPU(const StencilOperation stencilOp) {
    switch (stencilOp) {
        case StencilOperation::KEEP:      return wgpu::StencilOperation::Keep;
        case StencilOperation::ZERO:      return wgpu::StencilOperation::Zero;
        case StencilOperation::REPLACE:   return wgpu::StencilOperation::Replace;
        case StencilOperation::INCR:      return wgpu::StencilOperation::IncrementClamp;
        case StencilOperation::INCR_WRAP: return wgpu::StencilOperation::IncrementWrap;
        case StencilOperation::DECR:      return wgpu::StencilOperation::DecrementClamp;
        case StencilOperation::DECR_WRAP: return wgpu::StencilOperation::DecrementWrap;
        case StencilOperation::INVERT:    return wgpu::StencilOperation::Invert;
    }
}

[[nodiscard]] constexpr wgpu::BlendOperation toWebGPU(const BlendEquation blendOp) {
    switch (blendOp) {
        case BlendEquation::ADD:              return wgpu::BlendOperation::Add;
        case BlendEquation::SUBTRACT:         return wgpu::BlendOperation::Subtract;
        case BlendEquation::REVERSE_SUBTRACT: return wgpu::BlendOperation::ReverseSubtract;
        case BlendEquation::MIN:              return wgpu::BlendOperation::Min;
        case BlendEquation::MAX:              return wgpu::BlendOperation::Max;
    }
}

[[nodiscard]] constexpr wgpu::BlendFactor toWebGPU(const BlendFunction blendFunction) {
    switch (blendFunction) {
        case BlendFunction::ZERO:                return wgpu::BlendFactor::Zero;
        case BlendFunction::ONE:                 return wgpu::BlendFactor::One;
        case BlendFunction::SRC_COLOR:           return wgpu::BlendFactor::Src;
        case BlendFunction::ONE_MINUS_SRC_COLOR: return wgpu::BlendFactor::OneMinusSrc;
        case BlendFunction::DST_COLOR:           return wgpu::BlendFactor::Dst;
        case BlendFunction::ONE_MINUS_DST_COLOR: return wgpu::BlendFactor::OneMinusDst;
        case BlendFunction::SRC_ALPHA:           return wgpu::BlendFactor::SrcAlpha;
        case BlendFunction::ONE_MINUS_SRC_ALPHA: return wgpu::BlendFactor::OneMinusSrcAlpha;
        case BlendFunction::DST_ALPHA:           return wgpu::BlendFactor::DstAlpha;
        case BlendFunction::ONE_MINUS_DST_ALPHA: return wgpu::BlendFactor::OneMinusDstAlpha;
        case BlendFunction::SRC_ALPHA_SATURATE:  return wgpu::BlendFactor::SrcAlphaSaturated;
    }
}

}  // namespace

WebGPUPipelineCache::WebGPUPipelineCache(wgpu::Device const& device)
    : mDevice{ device } {}

wgpu::RenderPipeline const& WebGPUPipelineCache::getOrCreateRenderPipeline(
        RenderPipelineRequest const& request) {
    RenderPipelineKey key{};
    populateKey(request, key);
    if (auto iterator{ mRenderPipelines.find(key) }; iterator != mRenderPipelines.end()) {
        RenderPipelineCacheEntry& entry{ iterator.value() };
        entry.lastUsedFrameCount = mFrameCount;
        return entry.pipeline;
    }
    const wgpu::RenderPipeline pipeline{ createRenderPipeline(request) };
    mRenderPipelines.emplace(key, RenderPipelineCacheEntry{
                                      .pipeline = pipeline,
                                      .lastUsedFrameCount = mFrameCount,
                                  });
    return mRenderPipelines[key].pipeline;
}

void WebGPUPipelineCache::onFrameEnd() {
    ++mFrameCount;
    removeExpiredPipelines();
}

void WebGPUPipelineCache::populateKey(RenderPipelineRequest const& request,
        RenderPipelineKey& outKey) {
    outKey.vertexShaderModuleHandle =
            request.vertexShaderModule ? request.vertexShaderModule.Get() : nullptr;
    outKey.fragmentShaderModuleHandle =
            request.fragmentShaderModule ? request.fragmentShaderModule.Get() : nullptr;
    outKey.pipelineLayoutHandle = request.pipelineLayout ? request.pipelineLayout.Get() : nullptr;
    outKey.depthBias = static_cast<int32_t>(request.polygonOffset.constant);
    outKey.depthBiasSlopeScale = request.polygonOffset.slope;
    outKey.primitiveType = request.primitiveType;
    outKey.stencilFrontCompare = request.stencilState.front.stencilFunc;
    outKey.stencilFrontFailOperation = request.stencilState.front.stencilOpStencilFail;
    outKey.stencilFrontDepthFailOperation = request.stencilState.front.stencilOpDepthFail;
    outKey.stencilFrontPassOperation = request.stencilState.front.stencilOpDepthStencilPass;
    outKey.stencilWrite = toUint8(request.stencilState.stencilWrite);
    outKey.stencilFrontReadMask = request.stencilState.front.readMask;
    outKey.stencilFrontWriteMask = request.stencilState.front.writeMask;
    outKey.stencilBackCompare = request.stencilState.back.stencilFunc;
    outKey.stencilBackFailOperation = request.stencilState.back.stencilOpStencilFail;
    outKey.stencilBackDepthFailOperation = request.stencilState.back.stencilOpDepthFail;
    outKey.stencilBackPassOperation = request.stencilState.back.stencilOpDepthStencilPass;
    outKey.cullingMode = request.rasterState.culling;
    outKey.inverseFrontFaces = toUint8(request.rasterState.inverseFrontFaces);
    outKey.depthWriteEnabled = toUint8(request.rasterState.depthWrite);
    outKey.depthCompare = request.rasterState.depthFunc;
    outKey.depthClamp = toUint8(request.rasterState.depthClamp);
    outKey.colorWrite = toUint8(request.rasterState.colorWrite);
    outKey.alphaToCoverageEnabled = toUint8(request.rasterState.alphaToCoverage);
    outKey.colorBlendOperation = request.rasterState.blendEquationRGB;
    outKey.colorBlendSourceFactor = request.rasterState.blendFunctionSrcRGB;
    outKey.colorBlendDestinationFactor = request.rasterState.blendFunctionDstRGB;
    outKey.alphaBlendOperation = request.rasterState.blendEquationAlpha;
    outKey.alphaBlendSourceFactor = request.rasterState.blendFunctionSrcAlpha;
    outKey.alphaBlendDestinationFactor = request.rasterState.blendFunctionDstAlpha;
    outKey.targetRenderFlags = request.targetRenderFlags;
    outKey.multisampleCount = request.multisampleCount;
    outKey.depthStencilFormat = request.depthStencilFormat;
    assert_invariant(request.colorFormatCount <= MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
    outKey.colorFormatCount = request.colorFormatCount;
    for (size_t colorIndex{ 0 }; colorIndex < request.colorFormatCount; colorIndex++) {
        outKey.colorFormats[colorIndex] = request.colorFormats[colorIndex];
    }
    // vertex buffers...
    for (WebGPUVertexBufferInfo::WebGPUSlotBindingInfo const& vertexBufferSlot:
            request.vertexBufferSlots) {
        assert_invariant(vertexBufferSlot.sourceBufferIndex < MAX_VERTEX_BUFFER_COUNT);
        outKey.vertexBuffers[vertexBufferSlot.sourceBufferIndex].stride = vertexBufferSlot.stride;
        outKey.vertexBuffers[vertexBufferSlot.sourceBufferIndex].offset =
                vertexBufferSlot.bufferOffset;
    }
    // vertex attributes...
    uint8_t currentAttributeIndex{ 0 };
    for (size_t bufferIndex{ 0 }; bufferIndex < request.vertexBufferSlots.size(); bufferIndex++) {
        wgpu::VertexBufferLayout const& vertexBufferLayout{
            request.vertexBufferLayouts[bufferIndex]
        };
        for (size_t attributeIndex{ 0 }; attributeIndex < vertexBufferLayout.attributeCount;
                attributeIndex++) {
            assert_invariant(attributeIndex < MAX_VERTEX_ATTRIBUTE_COUNT);
            wgpu::VertexAttribute const& vertexAttribute{
                vertexBufferLayout.attributes[attributeIndex]
            };
            outKey.vertexAttributes[currentAttributeIndex].bufferIndex = bufferIndex;
            outKey.vertexAttributes[currentAttributeIndex].offset =
                    static_cast<uint8_t>(vertexAttribute.offset);
            outKey.vertexAttributes[currentAttributeIndex].shaderLocation =
                    static_cast<uint8_t>(vertexAttribute.shaderLocation);
            outKey.vertexAttributes[currentAttributeIndex].format = vertexAttribute.format;
            currentAttributeIndex++;
        }
    }
}

wgpu::RenderPipeline WebGPUPipelineCache::createRenderPipeline(
        RenderPipelineRequest const& request) {
    assert_invariant(request.vertexShaderModule);
    wgpu::DepthStencilState depthStencilState{};
    const bool requestedDepth{ any(request.targetRenderFlags & TargetBufferFlags::DEPTH) };
    const bool requestedStencil{ any(request.targetRenderFlags & TargetBufferFlags::STENCIL) };
    const bool depthOrStencilRequested{ requestedDepth || requestedStencil };

    if (depthOrStencilRequested) {
        FILAMENT_CHECK_PRECONDITION(request.depthStencilFormat != wgpu::TextureFormat::Undefined)
                << "Depth or Stencil requested for pipeline, but depthStencilFormat is "
                   "wgpu::TextureFormat::Undefined.";
        depthStencilState.format = request.depthStencilFormat;
        if (requestedDepth) {
            assert_invariant(hasDepth(depthStencilState.format));
            depthStencilState.depthWriteEnabled = request.rasterState.depthWrite;
            depthStencilState.depthCompare = toWebGPU(request.rasterState.depthFunc);
            depthStencilState.depthBias = static_cast<int32_t>(request.polygonOffset.constant);
            depthStencilState.depthBiasSlopeScale = request.polygonOffset.slope;
            depthStencilState.depthBiasClamp = 0.0f;
        } else {
            depthStencilState.depthWriteEnabled = false;
            depthStencilState.depthCompare = wgpu::CompareFunction::Undefined;
            depthStencilState.depthBias = 0;
            depthStencilState.depthBiasSlopeScale = 0.0f;
            depthStencilState.depthBiasClamp = 0.0f;
        }
        if (requestedStencil) {
            assert_invariant(hasStencil(depthStencilState.format));
            depthStencilState.stencilFront = {
                .compare = toWebGPU(request.stencilState.front.stencilFunc),
                .failOp = toWebGPU(request.stencilState.front.stencilOpStencilFail),
                .depthFailOp = toWebGPU(request.stencilState.front.stencilOpDepthFail),
                .passOp = toWebGPU(request.stencilState.front.stencilOpDepthStencilPass),
            };
            depthStencilState.stencilBack = {
                .compare = toWebGPU(request.stencilState.back.stencilFunc),
                .failOp = toWebGPU(request.stencilState.back.stencilOpStencilFail),
                .depthFailOp = toWebGPU(request.stencilState.back.stencilOpDepthFail),
                .passOp = toWebGPU(request.stencilState.back.stencilOpDepthStencilPass),
            };
            // TODO: should we also consider the back readMask and writeMask?
            depthStencilState.stencilReadMask = request.stencilState.front.readMask;
            depthStencilState.stencilWriteMask =
                    request.stencilState.stencilWrite ? request.stencilState.front.writeMask : 0u;
        } else {
            depthStencilState.stencilFront.compare = wgpu::CompareFunction::Undefined;
            depthStencilState.stencilFront.failOp = wgpu::StencilOperation::Keep;
            depthStencilState.stencilFront.depthFailOp = wgpu::StencilOperation::Keep;
            depthStencilState.stencilFront.passOp = wgpu::StencilOperation::Keep;
            depthStencilState.stencilBack = depthStencilState.stencilFront;
            depthStencilState.stencilReadMask = 0;
            depthStencilState.stencilWriteMask = 0;
        }
    }
    wgpu::RenderPipelineDescriptor pipelineDescriptor{
        .label = wgpu::StringView(request.label.c_str_safe()),
        .layout = request.pipelineLayout,
        .vertex = {
            .module = request.vertexShaderModule,
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
            .bufferCount = request.vertexBufferSlots.size(),
            .buffers = request.vertexBufferLayouts,
        },
        .primitive = {
            .topology = toWebGPU(request.primitiveType),
            // TODO should we assume some constant format here or is there a way to get
            //      this from PipelineState somehow or elsewhere?
            //      Perhaps, cache/assert format from index buffers as they are requested?
            .stripIndexFormat = wgpu::IndexFormat::Undefined,
            .frontFace = request.rasterState.inverseFrontFaces ? wgpu::FrontFace::CW : wgpu::FrontFace::CCW,
            .cullMode = toWebGPU(request.rasterState.culling),
            // TODO no depth clamp in WebGPU supported directly. unclippedDepth is close, so we are
            //      starting there
            .unclippedDepth = !request.rasterState.depthClamp &&
                              mDevice.HasFeature(wgpu::FeatureName::DepthClipControl),
        },
        .depthStencil = depthOrStencilRequested ? &depthStencilState: nullptr,
        .multisample = {
            .count = request.multisampleCount,
            .mask = 0xFFFFFFFF,
            .alphaToCoverageEnabled = (request.multisampleCount > 1) && request.rasterState.alphaToCoverage
        },
        .fragment = nullptr // will add below if fragment module is included
    };
    // TODO:
    if (pipelineDescriptor.primitive.topology == wgpu::PrimitiveTopology::LineStrip ||
            pipelineDescriptor.primitive.topology == wgpu::PrimitiveTopology::TriangleStrip) {
        PANIC_POSTCONDITION("stripIndexFormat must be set for strip topologies. "
                            "This needs to be plumbed through from the RenderPrimitive.");
    }

    wgpu::FragmentState fragmentState = {};
    const wgpu::BlendState blendState {
        .color = {
            .operation = toWebGPU(request.rasterState.blendEquationRGB),
            .srcFactor = toWebGPU(request.rasterState.blendFunctionSrcRGB),
            .dstFactor = toWebGPU(request.rasterState.blendFunctionDstRGB)
        },
        .alpha = {
            .operation = toWebGPU(request.rasterState.blendEquationAlpha),
            .srcFactor = toWebGPU(request.rasterState.blendFunctionSrcAlpha),
            .dstFactor = toWebGPU(request.rasterState.blendFunctionDstAlpha)
        }
    };
    // According to the WebGPU spec, a pipeline cannot have a fragment stage with zero color
    // targets. This situation can arise in Filament during depth-only passes (like shadow map
    // generation) if the material variant still includes a fragment shader.
    //
    // To handle this, we check if any color targets are configured for this pipeline. If not, we
    // create a pipeline *without* a fragment stage. This makes the pipeline valid for a
    // depth-only pass, allowing depth writes to proceed correctly.
    if (request.fragmentShaderModule != nullptr && request.colorFormatCount > 0) {
        fragmentState.module = request.fragmentShaderModule;
        fragmentState.entryPoint = "main";
        // see the comment about constants for the vertex state, as the same reasoning applies
        // here
        fragmentState.constantCount = 0;
        fragmentState.constants = nullptr;
        fragmentState.targetCount = request.colorFormatCount;
        std::array<wgpu::ColorTargetState, MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT> colorTargets {};
        assert_invariant(fragmentState.targetCount <= MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT);
        for (size_t targetIndex = 0; targetIndex < fragmentState.targetCount; targetIndex++) {
            wgpu::ColorTargetState& colorTarget = colorTargets[targetIndex];
            colorTarget.format = request.colorFormats[targetIndex];
            colorTarget.blend = request.rasterState.hasBlending() ? &blendState : nullptr;
            colorTarget.writeMask = request.rasterState.colorWrite ? wgpu::ColorWriteMask::All
                                                                   : wgpu::ColorWriteMask::None;
        }
        fragmentState.targets = colorTargets.data();
        pipelineDescriptor.fragment = &fragmentState;
    }
    const wgpu::RenderPipeline pipeline{ mDevice.CreateRenderPipeline(&pipelineDescriptor) };
    FILAMENT_CHECK_POSTCONDITION(pipeline)
            << "Failed to create render pipeline for " << pipelineDescriptor.label;
    return pipeline;
}

bool WebGPUPipelineCache::RenderPipelineKeyEqual::operator()(RenderPipelineKey const& key1,
        RenderPipelineKey const& key2) const {
    return 0 == memcmp(reinterpret_cast<void const*>(&key1), reinterpret_cast<void const*>(&key2),
                        sizeof(key1));
}

void WebGPUPipelineCache::removeExpiredPipelines() {
    using Iterator = decltype(mRenderPipelines)::const_iterator;
    for (Iterator iterator{ mRenderPipelines.begin() }; iterator != mRenderPipelines.end();) {
        RenderPipelineCacheEntry const& entry{ iterator.value() };
        if (mFrameCount > (entry.lastUsedFrameCount +
                                  FILAMENT_WEBGPU_RENDER_PIPELINE_EXPIRATION_IN_FRAME_COUNT)) {
            // pipeline expired...
            iterator = mRenderPipelines.erase(iterator);
        } else {
            // pipeline not yet expired...
            ++iterator;
        }
    }
}

}  //namespace filament::backend
