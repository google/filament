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

#include "MetalContext.h"

#include "MetalHandles.h"

#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>

#include <utility>

namespace filament {
namespace backend {

void initializeSupportedGpuFamilies(MetalContext* context) {
    auto& highestSupportedFamily = context->highestSupportedGpuFamily;

    assert_invariant(context->device);
    id<MTLDevice> device = context->device;

    highestSupportedFamily.common = 0u;
    highestSupportedFamily.apple = 0u;
    highestSupportedFamily.mac = 0u;

    if (@available(iOS 13.0, *)) {
        if ([device supportsFamily:MTLGPUFamilyApple7]) {
            highestSupportedFamily.apple = 7;
        } else if ([device supportsFamily:MTLGPUFamilyApple6]) {
            highestSupportedFamily.apple = 6;
        } else if ([device supportsFamily:MTLGPUFamilyApple5]) {
            highestSupportedFamily.apple = 5;
        } else if ([device supportsFamily:MTLGPUFamilyApple4]) {
            highestSupportedFamily.apple = 4;
        } else if ([device supportsFamily:MTLGPUFamilyApple3]) {
            highestSupportedFamily.apple = 3;
        } else if ([device supportsFamily:MTLGPUFamilyApple2]) {
            highestSupportedFamily.apple = 2;
        } else if ([device supportsFamily:MTLGPUFamilyApple1]) {
            highestSupportedFamily.apple = 1;
        }

        if ([device supportsFamily:MTLGPUFamilyCommon3]) {
            highestSupportedFamily.common = 3;
        } else if ([device supportsFamily:MTLGPUFamilyCommon2]) {
            highestSupportedFamily.common = 2;
        } else if ([device supportsFamily:MTLGPUFamilyCommon1]) {
            highestSupportedFamily.common = 1;
        }

        if ([device supportsFamily:MTLGPUFamilyMac2]) {
            highestSupportedFamily.mac = 2;
        } else if ([device supportsFamily:MTLGPUFamilyMac1]) {
            highestSupportedFamily.mac = 1;
        }
    } else {
#if TARGET_OS_IOS
        using FeatureSet = std::pair<MTLFeatureSet, uint8_t>;
        auto testFeatureSets = [device] (const auto& featureSets,
                uint8_t& outHighestSupported) {
            for (const auto& set : featureSets) {
                if ([device supportsFeatureSet:set.first]) {
                    outHighestSupported = set.second;
                    break;
                }
            }
        };

        // Apple GPUs
        auto appleFeatureSets = utils::FixedCapacityVector<FeatureSet>::with_capacity(5);
        if (@available(iOS 12.0, *)) {
            appleFeatureSets.emplace_back(MTLFeatureSet_iOS_GPUFamily5_v1, 5u);
        }
        appleFeatureSets.emplace_back(MTLFeatureSet_iOS_GPUFamily4_v1, 4u);
        appleFeatureSets.emplace_back(MTLFeatureSet_iOS_GPUFamily3_v2, 3u);
        appleFeatureSets.emplace_back(MTLFeatureSet_iOS_GPUFamily2_v4, 2u);
        appleFeatureSets.emplace_back(MTLFeatureSet_iOS_GPUFamily1_v4, 1u);

        testFeatureSets(appleFeatureSets, highestSupportedFamily.apple);
#endif
    }
}

id<MTLCommandBuffer> getPendingCommandBuffer(MetalContext* context) {
    if (context->pendingCommandBuffer) {
        return context->pendingCommandBuffer;
    }
    context->pendingCommandBuffer = [context->commandQueue commandBuffer];
    // It's safe for this block to capture the context variable. MetalDriver::terminate will ensure
    // all frames and their completion handlers finish before context is deallocated.
    [context->pendingCommandBuffer addCompletedHandler:^(id <MTLCommandBuffer> buffer) {
        context->resourceTracker.clearResources((__bridge void*) buffer);
        
        auto errorCode = (MTLCommandBufferError)buffer.error.code;
        if (@available(macOS 11.0, *)) {
            if (errorCode == MTLCommandBufferErrorMemoryless) {
                utils::slog.w << "Metal: memoryless geometry limit reached. "
                        "Continuing with private storage mode." << utils::io::endl;
                context->memorylessLimitsReached = true;
            }
        }
    }];
    ASSERT_POSTCONDITION(context->pendingCommandBuffer, "Could not obtain command buffer.");
    return context->pendingCommandBuffer;
}

void submitPendingCommands(MetalContext* context) {
    if (!context->pendingCommandBuffer) {
        return;
    }
    assert_invariant(context->pendingCommandBuffer.status != MTLCommandBufferStatusCommitted);
    [context->pendingCommandBuffer commit];
    context->pendingCommandBuffer = nil;
}

id<MTLTexture> getOrCreateEmptyTexture(MetalContext* context) {
    if (context->emptyTexture) {
        return context->emptyTexture;
    }

    MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor new];
    textureDescriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
    textureDescriptor.width = 1;
    textureDescriptor.height = 1;
    id<MTLTexture> texture = [context->device newTextureWithDescriptor:textureDescriptor];

    MTLRegion region = {
        { 0, 0, 0 },    // MTLOrigin
        { 1, 1, 1 }     // MTLSize
    };
    uint8_t imageData[4] = {0, 0, 0, 0};
    [texture replaceRegion:region mipmapLevel:0 withBytes:imageData bytesPerRow:4];

    context->emptyTexture = texture;

    return context->emptyTexture;
}

bool isInRenderPass(MetalContext* context) {
    return context->currentRenderPassEncoder != nil;
}

void MetalPushConstantBuffer::setPushConstant(PushConstantVariant value, uint8_t index) {
    if (mPushConstants.size() <= index) {
        mPushConstants.resize(index + 1);
        mDirty = true;
    }
    if (UTILS_LIKELY(mPushConstants[index] != value)) {
        mDirty = true;
        mPushConstants[index] = value;
    }
}

void MetalPushConstantBuffer::setBytes(id<MTLCommandEncoder> encoder, ShaderStage stage) {
    constexpr size_t PUSH_CONSTANT_SIZE_BYTES = 4;
    constexpr size_t PUSH_CONSTANT_BUFFER_INDEX = 26;

    static char buffer[MAX_PUSH_CONSTANT_COUNT * PUSH_CONSTANT_SIZE_BYTES];
    assert_invariant(mPushConstants.size() <= MAX_PUSH_CONSTANT_COUNT);

    size_t bufferSize = PUSH_CONSTANT_SIZE_BYTES * mPushConstants.size();
    for (size_t i = 0; i < mPushConstants.size(); i++) {
        const auto& constant = mPushConstants[i];
        std::visit(
                [i](auto arg) {
                    if constexpr (std::is_same_v<decltype(arg), bool>) {
                        // bool push constants are converted to uints in MSL.
                        // We must ensure we write all the bytes for boolean values to work
                        // correctly.
                        uint32_t boolAsUint = arg ? 0x00000001 : 0x00000000;
                        *(reinterpret_cast<uint32_t*>(buffer + PUSH_CONSTANT_SIZE_BYTES * i)) =
                                boolAsUint;
                    } else {
                        *(decltype(arg)*)(buffer + PUSH_CONSTANT_SIZE_BYTES * i) = arg;
                    }
                },
                constant);
    }

    switch (stage) {
        case ShaderStage::VERTEX:
            [(id<MTLRenderCommandEncoder>)encoder setVertexBytes:buffer
                                                          length:bufferSize
                                                         atIndex:PUSH_CONSTANT_BUFFER_INDEX];
            break;
        case ShaderStage::FRAGMENT:
            [(id<MTLRenderCommandEncoder>)encoder setFragmentBytes:buffer
                                                            length:bufferSize
                                                           atIndex:PUSH_CONSTANT_BUFFER_INDEX];
            break;
        case ShaderStage::COMPUTE:
            [(id<MTLComputeCommandEncoder>)encoder setBytes:buffer
                                                     length:bufferSize
                                                    atIndex:PUSH_CONSTANT_BUFFER_INDEX];
            break;
    }

    mDirty = false;
}

void MetalPushConstantBuffer::clear() {
    mPushConstants.clear();
    mDirty = false;
}

} // namespace backend
} // namespace filament
