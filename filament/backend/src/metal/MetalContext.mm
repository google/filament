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
    id<CAMetalDrawable> drawable = (id<CAMetalDrawable>) user;
    if (presentFrame) {
        [drawable present];
    }
    [drawable release];
}

id<CAMetalDrawable> acquireDrawable(MetalContext* context) {
    if (!context->currentDrawable) {
        // The drawable is retained here and will be released either:
        // 1. in MetalDriver::commit
        // 2. in the presentDrawable function, when the client calls the PresentCallable
        context->currentDrawable = [[context->currentSurface->layer nextDrawable] retain];

        if (context->frameFinishedCallback) {
            id<CAMetalDrawable> drawable = context->currentDrawable;
            backend::FrameFinishedCallback callback = context->frameFinishedCallback;
            void* userData = context->frameFinishedUserData;
            [context->currentCommandBuffer addScheduledHandler:^(id<MTLCommandBuffer> cb) {
                PresentCallable callable(presentDrawable, (void*) drawable);
                dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                    callback(callable, userData);
                });
            }];
        }
    }
    ASSERT_POSTCONDITION(context->currentDrawable != nil, "Could not obtain drawable.");
    return context->currentDrawable;
}

id<MTLCommandBuffer> acquireCommandBuffer(MetalContext* context) {
    id<MTLCommandBuffer> commandBuffer = [context->commandQueue commandBuffer];
    ASSERT_POSTCONDITION(commandBuffer != nil, "Could not obtain command buffer.");
    context->currentCommandBuffer = commandBuffer;
    return commandBuffer;
}

} // namespace metal
} // namespace backend
} // namespace filament
