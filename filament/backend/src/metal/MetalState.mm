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

id<MTLRenderPipelineState> PipelineStateCreator::operator()(id<MTLDevice> device,
        const MetalPipelineState& state) noexcept {
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
            vertex.attributes[i].bufferIndex = attribute.buffer;
            vertex.attributes[i].offset = attribute.offset;
        }
    }

    for (uint32_t i = 0; i < LOGICAL_VERTEX_BUFFER_COUNT; i++) {
        if (vertexDescription.layouts[i].stride > 0) {
            const auto& layout = vertexDescription.layouts[i];
            vertex.layouts[i].stride = layout.stride;
            vertex.layouts[i].stepFunction = layout.step;
            if (layout.step == MTLVertexStepFunctionConstant) {
                vertex.layouts[i].stepRate = 0;
            }
        }
    }

    descriptor.vertexDescriptor = vertex;

    // Color attachments
    for (size_t i = 0; i < 4; i++) {
        if (state.colorAttachmentPixelFormat[i] == MTLPixelFormatInvalid) {
            continue;
        }

        descriptor.colorAttachments[i].pixelFormat = state.colorAttachmentPixelFormat[i];
        descriptor.colorAttachments[i].writeMask =
                state.colorWrite ? MTLColorWriteMaskAll : MTLColorWriteMaskNone;

        const auto& bs = state.blendState;
        descriptor.colorAttachments[i].blendingEnabled = bs.blendingEnabled;
        descriptor.colorAttachments[i].alphaBlendOperation = bs.alphaBlendOperation;
        descriptor.colorAttachments[i].rgbBlendOperation = bs.rgbBlendOperation;
        descriptor.colorAttachments[i].destinationAlphaBlendFactor = bs.destinationAlphaBlendFactor;
        descriptor.colorAttachments[i].destinationRGBBlendFactor = bs.destinationRGBBlendFactor;
        descriptor.colorAttachments[i].sourceAlphaBlendFactor = bs.sourceAlphaBlendFactor;
        descriptor.colorAttachments[i].sourceRGBBlendFactor = bs.sourceRGBBlendFactor;
    }

    // Depth attachment
    descriptor.depthAttachmentPixelFormat = state.depthAttachmentPixelFormat;

    // Stencil attachment
    descriptor.stencilAttachmentPixelFormat = state.stencilAttachmentPixelFormat;

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
    depthStencilDescriptor.depthCompareFunction = state.depthCompare;
    depthStencilDescriptor.depthWriteEnabled = BOOL(state.depthWriteEnabled);

    // Front-facing stencil.
    MTLStencilDescriptor* frontStencilDescriptor = [MTLStencilDescriptor new];
    frontStencilDescriptor.stencilCompareFunction = state.front.stencilCompare;
    frontStencilDescriptor.stencilFailureOperation = state.front.stencilOperationStencilFail;
    frontStencilDescriptor.depthFailureOperation = state.front.stencilOperationDepthFail;
    frontStencilDescriptor.depthStencilPassOperation = state.front.stencilOperationDepthStencilPass;
    frontStencilDescriptor.readMask = state.front.readMask;
    frontStencilDescriptor.writeMask = state.stencilWriteEnabled ? state.front.writeMask : 0x0;
    depthStencilDescriptor.frontFaceStencil = frontStencilDescriptor;

    // Back-facing stencil.
    MTLStencilDescriptor* backStencilDescriptor = [MTLStencilDescriptor new];
    backStencilDescriptor.stencilCompareFunction = state.back.stencilCompare;
    backStencilDescriptor.stencilFailureOperation = state.back.stencilOperationStencilFail;
    backStencilDescriptor.depthFailureOperation = state.back.stencilOperationDepthFail;
    backStencilDescriptor.depthStencilPassOperation = state.back.stencilOperationDepthStencilPass;
    backStencilDescriptor.readMask = state.back.readMask;
    backStencilDescriptor.writeMask = state.stencilWriteEnabled ? state.back.writeMask : 0x0;
    depthStencilDescriptor.backFaceStencil = backStencilDescriptor;

    return [device newDepthStencilStateWithDescriptor:depthStencilDescriptor];
}

id<MTLSamplerState> SamplerStateCreator::operator()(id<MTLDevice> device,
        const SamplerState& state) noexcept {
    SamplerParams params = state.samplerParams;
    MTLSamplerDescriptor* samplerDescriptor = [MTLSamplerDescriptor new];
    samplerDescriptor.minFilter = getFilter(params.filterMin);
    samplerDescriptor.magFilter = getFilter(params.filterMag);
    samplerDescriptor.mipFilter = getMipFilter(params.filterMin);
    samplerDescriptor.sAddressMode = getAddressMode(params.wrapS);
    samplerDescriptor.tAddressMode = getAddressMode(params.wrapT);
    samplerDescriptor.rAddressMode = getAddressMode(params.wrapR);
    samplerDescriptor.maxAnisotropy = 1u << params.anisotropyLog2;
    samplerDescriptor.compareFunction =
            params.compareMode == SamplerCompareMode::NONE ?
                MTLCompareFunctionNever : getCompareFunction(params.compareFunc);
    samplerDescriptor.supportArgumentBuffers = YES;

#if defined(IOS)
    // Older Apple devices (and the simulator) don't support setting a comparison function in
    // MTLSamplerDescriptor.
    // In practice, this means shadows are not supported when running in the simulator.
    if (![device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1]) {
        utils::slog.w << "Warning: sample comparison not supported by this GPU" << utils::io::endl;
        samplerDescriptor.compareFunction = MTLCompareFunctionNever;
    }
#endif

    return [device newSamplerStateWithDescriptor:samplerDescriptor];
}

id<MTLArgumentEncoder> ArgumentEncoderCreator::operator()(id<MTLDevice> device,
        const ArgumentEncoderState &state) noexcept {
    const auto& textureTypes = state.textureTypes;
    const auto& count = textureTypes.size();
    assert_invariant(count > 0);

    // Metal has separate data types for textures versus samplers, so the argument buffer layout
    // alternates between texture and sampler, i.e.:
    // textureA
    // samplerA
    // textureB
    // samplerB
    // etc
    NSMutableArray<MTLArgumentDescriptor*>* arguments =
            [NSMutableArray arrayWithCapacity:(count * 2)];
    for (size_t i = 0; i < count; i++) {
        MTLArgumentDescriptor* textureArgument = [MTLArgumentDescriptor argumentDescriptor];
        textureArgument.index = i * 2 + 0;
        textureArgument.dataType = MTLDataTypeTexture;
        textureArgument.textureType = textureTypes[i];
        textureArgument.access = MTLArgumentAccessReadOnly;
        [arguments addObject:textureArgument];

        MTLArgumentDescriptor* samplerArgument = [MTLArgumentDescriptor argumentDescriptor];
        samplerArgument.index = i * 2 + 1;
        samplerArgument.dataType = MTLDataTypeSampler;
        textureArgument.access = MTLArgumentAccessReadOnly;
        [arguments addObject:samplerArgument];
    }

    return [device newArgumentEncoderWithArguments:arguments];
}

} // namespace backend
} // namespace filament
