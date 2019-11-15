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

void presentDrawable(bool presentFrame, void* user) {
    // CFBridgingRelease here is used to balance the CFBridgingRetain inside of acquireDrawable.
    id<CAMetalDrawable> drawable = (id<CAMetalDrawable>) CFBridgingRelease(user);
    if (presentFrame) {
        [drawable present];
    }
    // The drawable will be released here when the "drawable" variable goes out of scope.
}

id<MTLTexture> acquireDrawable(MetalContext* context) {
    if (context->currentDrawable) {
        return context->currentDrawable.texture;
    }
    if (context->currentSurface->isHeadless()) {
        if (context->headlessDrawable) {
            return context->headlessDrawable;
        }
        // For headless surfaces we construct a "fake" drawable, which is simply a renderable
        // texture.
        MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor new];
        textureDescriptor.pixelFormat = MTLPixelFormatBGRA8Unorm;
        textureDescriptor.width = context->currentSurface->surfaceWidth;
        textureDescriptor.height = context->currentSurface->surfaceHeight;
        textureDescriptor.usage = MTLTextureUsageRenderTarget;
#if defined(IOS)
        textureDescriptor.storageMode = MTLStorageModeShared;
#else
        textureDescriptor.storageMode = MTLStorageModeManaged;
#endif
        context->headlessDrawable = [context->device newTextureWithDescriptor:textureDescriptor];
        return context->headlessDrawable;
    }

    context->currentDrawable = [context->currentSurface->layer nextDrawable];

    if (context->frameFinishedCallback) {
        id<CAMetalDrawable> drawable = context->currentDrawable;
        backend::FrameFinishedCallback callback = context->frameFinishedCallback;
        void* userData = context->frameFinishedUserData;
        // This block strongly captures drawable to keep it alive until the handler executes.
        [context->currentCommandBuffer addScheduledHandler:^(id<MTLCommandBuffer> cb) {
            // CFBridgingRetain is used here to give the drawable a +1 retain count before
            // casting it to a void*.
            PresentCallable callable(presentDrawable, (void*) CFBridgingRetain(drawable));
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                callback(callable, userData);
            });
        }];
    }
    ASSERT_POSTCONDITION(context->currentDrawable != nil, "Could not obtain drawable.");
    return context->currentDrawable.texture;
}

id<MTLCommandBuffer> acquireCommandBuffer(MetalContext* context) {
    id<MTLCommandBuffer> commandBuffer = [context->commandQueue commandBuffer];
    ASSERT_POSTCONDITION(commandBuffer != nil, "Could not obtain command buffer.");
    context->currentCommandBuffer = commandBuffer;
    return commandBuffer;
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

} // namespace metal
} // namespace backend
} // namespace filament
