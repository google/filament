/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "MetalState.h"

#include "MetalEnums.h"

namespace filament {
namespace backend {
namespace metal {

id<MTLRenderPipelineState> PipelineStateCreator::operator()(id<MTLDevice> device,
        const PipelineState& state) noexcept {
    MTLRenderPipelineDescriptor* descriptor = [MTLRenderPipelineDescriptor new];

    // Shader Functions
    descriptor.vertexFunction = state.vertexFunction;
    descriptor.fragmentFunction = state.fragmentFunction;

    // Vertex attributes
    MTLVertexDescriptor* vertex = [MTLVertexDescriptor vertexDescriptor];

    const auto& vertexDescription = state.vertexDescription;

    for (uint32_t i = 0; i < MAX_VERTEX_ATTRIBUTE_COUNT; i++) {
        if (vertexDescription.attributes[i].format > MTLVertexFormatInvalid) {
            const auto& attribute = vertexDescription.attributes[i];
            vertex.attributes[i].format = attribute.format;
            vertex.attributes[i].bufferIndex = VERTEX_BUFFER_START + attribute.buffer;
            vertex.attributes[i].offset = attribute.offset;
        }
    }

    for (uint32_t i = 0; i < VERTEX_BUFFER_COUNT; i++) {
        if (vertexDescription.layouts[i].stride > 0) {
            const auto& layout = vertexDescription.layouts[i];
            vertex.layouts[VERTEX_BUFFER_START + i].stride = layout.stride;
            vertex.layouts[VERTEX_BUFFER_START + i].stepFunction = layout.step;
            if (layout.step == MTLVertexStepFunctionConstant) {
                vertex.layouts[VERTEX_BUFFER_START + i].stepRate = 0;
            }
        }
    }

    descriptor.vertexDescriptor = vertex;

    // Color attachments
    descriptor.colorAttachments[0].pixelFormat = state.colorAttachmentPixelFormat;

    const auto& bs = state.blendState;
    descriptor.colorAttachments[0].blendingEnabled = bs.blendingEnabled;
    descriptor.colorAttachments[0].alphaBlendOperation = bs.alphaBlendOperation;
    descriptor.colorAttachments[0].rgbBlendOperation = bs.rgbBlendOperation;
    descriptor.colorAttachments[0].destinationAlphaBlendFactor = bs.destinationAlphaBlendFactor;
    descriptor.colorAttachments[0].destinationRGBBlendFactor = bs.destinationRGBBlendFactor;
    descriptor.colorAttachments[0].sourceAlphaBlendFactor = bs.sourceAlphaBlendFactor;
    descriptor.colorAttachments[0].sourceRGBBlendFactor = bs.sourceRGBBlendFactor;

    // Depth attachment
    descriptor.depthAttachmentPixelFormat = state.depthAttachmentPixelFormat;

    // MSAA
    descriptor.rasterSampleCount = state.sampleCount;

    NSError* error = nullptr;
    id<MTLRenderPipelineState> pipeline = [device newRenderPipelineStateWithDescriptor:descriptor
                                                                                 error:&error];
    if (error) {
        auto description = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
        utils::slog.e << description << utils::io::endl;
    }
    ASSERT_POSTCONDITION(error == nil, "Could not create Metal pipeline state.");

    return pipeline;
}

id<MTLDepthStencilState> DepthStateCreator::operator()(id<MTLDevice> device,
        const DepthStencilState& state) noexcept {
    MTLDepthStencilDescriptor* depthStencilDescriptor = [MTLDepthStencilDescriptor new];
    depthStencilDescriptor.depthCompareFunction = state.compareFunction;
    depthStencilDescriptor.depthWriteEnabled = state.depthWriteEnabled;
    return [device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
}

id<MTLSamplerState> SamplerStateCreator::operator()(id<MTLDevice> device,
        const backend::SamplerParams& state) noexcept {
    MTLSamplerDescriptor* samplerDescriptor = [MTLSamplerDescriptor new];
    samplerDescriptor.minFilter = getFilter(state.filterMin);
    samplerDescriptor.magFilter = getFilter(state.filterMag);
    samplerDescriptor.mipFilter = getMipFilter(state.filterMin);
    samplerDescriptor.sAddressMode = getAddressMode(state.wrapS);
    samplerDescriptor.tAddressMode = getAddressMode(state.wrapT);
    samplerDescriptor.rAddressMode = getAddressMode(state.wrapR);
    samplerDescriptor.maxAnisotropy = 1u << state.anisotropyLog2;
    samplerDescriptor.compareFunction =
            state.compareMode == SamplerCompareMode::NONE ?
                MTLCompareFunctionNever : getCompareFunction(state.compareFunc);
    return [device newSamplerStateWithDescriptor:samplerDescriptor];
}

} // namespace metal
} // namespace backend
} // namespace filament
