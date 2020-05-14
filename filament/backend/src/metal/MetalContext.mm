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
        textureDescriptor.width = context->currentSurface->getSurfaceWidth();
        textureDescriptor.height = context->currentSurface->getSurfaceHeight();
        // Specify MTLTextureUsageShaderRead so the headless surface can be blitted from.
        textureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if defined(IOS)
        textureDescriptor.storageMode = MTLStorageModeShared;
#else
        textureDescriptor.storageMode = MTLStorageModeManaged;
#endif
        context->headlessDrawable = [context->device newTextureWithDescriptor:textureDescriptor];
        return context->headlessDrawable;
    }

    context->currentDrawable = [context->currentSurface->getLayer() nextDrawable];

    if (context->frameFinishedCallback) {
        id<CAMetalDrawable> drawable = context->currentDrawable;
        backend::FrameFinishedCallback callback = context->frameFinishedCallback;
        void* userData = context->frameFinishedUserData;
        // This block strongly captures drawable to keep it alive until the handler executes.
        [getPendingCommandBuffer(context) addScheduledHandler:^(id<MTLCommandBuffer> cb) {
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

id<MTLTexture> acquireDepthTexture(MetalContext* context) {
    if (context->currentDepthTexture) {
        // If the surface size has changed, we'll need to allocate a new depth texture.
        if (context->currentDepthTexture.width != context->currentSurface->getSurfaceWidth() ||
            context->currentDepthTexture.height != context->currentSurface->getSurfaceHeight()) {
            context->currentDepthTexture = nil;
        } else {
            return context->currentDepthTexture;
        }
    }

    const MTLPixelFormat depthFormat =
#if defined(IOS)
            MTLPixelFormatDepth32Float;
#else
    context->device.depth24Stencil8PixelFormatSupported ?
            MTLPixelFormatDepth24Unorm_Stencil8 : MTLPixelFormatDepth32Float;
#endif

    const NSUInteger width = context->currentSurface->getSurfaceWidth();
    const NSUInteger height = context->currentSurface->getSurfaceHeight();
    MTLTextureDescriptor* descriptor =
            [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:depthFormat
                                                               width:width
                                                              height:height
                                                           mipmapped:NO];
    descriptor.usage = MTLTextureUsageRenderTarget;
    descriptor.resourceOptions = MTLResourceStorageModePrivate;

    context->currentDepthTexture = [context->device newTextureWithDescriptor:descriptor];

    return context->currentDepthTexture;
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
