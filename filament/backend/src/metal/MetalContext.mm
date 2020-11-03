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

#include <utils/Panic.h>

namespace filament {
namespace backend {
namespace metal {

id<MTLCommandBuffer> getPendingCommandBuffer(MetalContext* context) {
    if (context->pendingCommandBuffer) {
        return context->pendingCommandBuffer;
    }
    context->pendingCommandBuffer = [context->commandQueue commandBuffer];
    // It's safe for this block to capture the context variable. MetalDriver::terminate will ensure
    // all frames and their completion handlers finish before context is deallocated.
    [context->pendingCommandBuffer addCompletedHandler:^(id <MTLCommandBuffer> buffer) {
        context->resourceTracker.clearResources((__bridge void*) buffer);
    }];
    ASSERT_POSTCONDITION(context->pendingCommandBuffer, "Could not obtain command buffer.");
    return context->pendingCommandBuffer;
}

void submitPendingCommands(MetalContext* context) {
    if (!context->pendingCommandBuffer) {
        return;
    }
    assert(context->pendingCommandBuffer.status != MTLCommandBufferStatusCommitted);
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

} // namespace metal
} // namespace backend
} // namespace filament
