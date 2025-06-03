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

#include <utils/Logger.h>

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
    if (UTILS_UNLIKELY(pipeline == nil)) {
        NSString *errorMessage =
            [NSString stringWithFormat:@"Could not create Metal pipeline state: %@",
                error ? error.localizedDescription : @"unknown error"];
        auto description = [errorMessage cStringUsingEncoding:NSUTF8StringEncoding];
        LOG(ERROR) << description;
        [[NSException exceptionWithName:@"MetalRenderPipelineFailure"
                                 reason:errorMessage
                               userInfo:nil] raise];
    }
    FILAMENT_CHECK_POSTCONDITION(error == nil) << "Could not create Metal pipeline state.";

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

#if defined(FILAMENT_IOS)
    // Older Apple devices (and the simulator) don't support setting a comparison function in
    // MTLSamplerDescriptor.
    // In practice, this means shadows are not supported when running in the simulator.
    if (![device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1]) {
        LOG(WARNING) << "Warning: sample comparison not supported by this GPU";
        samplerDescriptor.compareFunction = MTLCompareFunctionNever;
    }
#endif

    return [device newSamplerStateWithDescriptor:samplerDescriptor];
}

id<MTLArgumentEncoder> ArgumentEncoderCreator::operator()(id<MTLDevice> device,
        const ArgumentEncoderState &state) noexcept {
    const auto& textureTypes = state.textureTypes;
    const auto& textureCount = textureTypes.size();
    const auto& bufferCount = state.bufferCount;
    assert_invariant(textureCount > 0);

    // Metal has separate data types for textures versus samplers, so the argument buffer layout
    // alternates between texture and sampler, i.e.:
    // buffer0
    // buffer1
    // textureA
    // samplerA
    // textureB
    // samplerB
    // etc
    NSMutableArray<MTLArgumentDescriptor*>* arguments =
            [NSMutableArray arrayWithCapacity:(bufferCount + textureCount * 2)];
    size_t i = 0;
    for (size_t j = 0; j < bufferCount; j++) {
        MTLArgumentDescriptor* bufferArgument = [MTLArgumentDescriptor argumentDescriptor];
        bufferArgument.index = i++;
        bufferArgument.dataType = MTLDataTypePointer;
        bufferArgument.access = MTLArgumentAccessReadOnly;
        [arguments addObject:bufferArgument];
    }

    for (size_t j = 0; j < textureCount; j++) {
        MTLArgumentDescriptor* textureArgument = [MTLArgumentDescriptor argumentDescriptor];
        textureArgument.index = i++;
        textureArgument.dataType = MTLDataTypeTexture;
        textureArgument.textureType = textureTypes[i];
        textureArgument.access = MTLArgumentAccessReadOnly;
        [arguments addObject:textureArgument];

        MTLArgumentDescriptor* samplerArgument = [MTLArgumentDescriptor argumentDescriptor];
        samplerArgument.index = i++;
        samplerArgument.dataType = MTLDataTypeSampler;
        textureArgument.access = MTLArgumentAccessReadOnly;
        [arguments addObject:samplerArgument];
    }

    return [device newArgumentEncoderWithArguments:arguments];
}

template <NSUInteger N, ShaderStage stage>
void MetalBufferBindings<N, stage>::setBuffer(const id<MTLBuffer> buffer, NSUInteger offset, NSUInteger index) {
    assert_invariant(offset + 1 <= N);

    if (mBuffers[index] != buffer) {
        mBuffers[index] = buffer;
        mDirtyBuffers.set(index);
    }

    if (mOffsets[index] != offset) {
        mOffsets[index] = offset;
        mDirtyOffsets.set(index);
    }
}

template <NSUInteger N, ShaderStage stage>
void MetalBufferBindings<N, stage>::bindBuffers(
        id<MTLCommandEncoder> encoder, NSUInteger startIndex) {
    if (mDirtyBuffers.none() && mDirtyOffsets.none()) {
        return;
    }

    utils::bitset8 onlyOffsetDirty = mDirtyOffsets & ~mDirtyBuffers;
    onlyOffsetDirty.forEachSetBit([&](size_t i) {
        if constexpr (stage == ShaderStage::VERTEX) {
            [(id<MTLRenderCommandEncoder>)encoder setVertexBufferOffset:mOffsets[i]
                                                                atIndex:i + startIndex];
        } else if constexpr (stage == ShaderStage::FRAGMENT) {
            [(id<MTLRenderCommandEncoder>)encoder setFragmentBufferOffset:mOffsets[i]
                                                                  atIndex:i + startIndex];
        } else if constexpr (stage == ShaderStage::COMPUTE) {
            [(id<MTLComputeCommandEncoder>)encoder setBufferOffset:mOffsets[i]
                                                           atIndex:i + startIndex];
        }
    });
    mDirtyOffsets.reset();

    mDirtyBuffers.forEachSetBit([&](size_t i) {
        if constexpr (stage == ShaderStage::VERTEX) {
            [(id<MTLRenderCommandEncoder>)encoder setVertexBuffer:mBuffers[i]
                                                           offset:mOffsets[i]
                                                          atIndex:i + startIndex];
        } else if constexpr (stage == ShaderStage::FRAGMENT) {
            [(id<MTLRenderCommandEncoder>)encoder setFragmentBuffer:mBuffers[i]
                                                             offset:mOffsets[i]
                                                            atIndex:i + startIndex];
        } else if constexpr (stage == ShaderStage::COMPUTE) {
            [(id<MTLComputeCommandEncoder>)encoder setBuffer:mBuffers[i]
                                                      offset:mOffsets[i]
                                                     atIndex:i + startIndex];
        }
    });
    mDirtyBuffers.reset();
}

template class MetalBufferBindings<MAX_DESCRIPTOR_SET_COUNT, ShaderStage::VERTEX>;
template class MetalBufferBindings<MAX_DESCRIPTOR_SET_COUNT, ShaderStage::FRAGMENT>;
template class MetalBufferBindings<MAX_DESCRIPTOR_SET_COUNT, ShaderStage::COMPUTE>;

} // namespace backend
} // namespace filament
