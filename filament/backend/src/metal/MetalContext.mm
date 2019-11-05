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

id<CAMetalDrawable> acquireDrawable(MetalContext* context) {
    if (!context->currentDrawable) {
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
