/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <backend/platforms/PlatformMetal.h>

#include "MetalDriverFactory.h"

#include <utils/Log.h>

#import <Foundation/Foundation.h>

#include <atomic>

namespace filament::backend {

struct PlatformMetalImpl {
    id<MTLCommandQueue> mCommandQueue = nil;
    // read form driver thread, read/written to from client thread
    std::atomic<PlatformMetal::DrawableFailureBehavior> mDrawableFailureBehavior =
            PlatformMetal::DrawableFailureBehavior::PANIC;
};

Platform* createDefaultMetalPlatform() {
    return new PlatformMetal();
}

PlatformMetal::PlatformMetal() : pImpl(new PlatformMetalImpl) {}

PlatformMetal::~PlatformMetal() noexcept {
    delete pImpl;
}

Driver* PlatformMetal::createDriver(void* /*sharedContext*/, const Platform::DriverConfig& driverConfig) noexcept {
    return MetalDriverFactory::create(this, driverConfig);
}

id<MTLDevice> PlatformMetal::createDevice() noexcept {
    id<MTLDevice> result;

#if !defined(FILAMENT_IOS)
    const bool forceIntegrated =
            NSProcessInfo.processInfo.environment[@"FILAMENT_FORCE_INTEGRATED_GPU"] != nil;
    if (forceIntegrated) {
        // Find the first low power device, which is likely the integrated GPU.
        NSArray<id<MTLDevice>>* const devices = MTLCopyAllDevices();
        for (id<MTLDevice> device in devices) {
            if (device.isLowPower) {
                result = device;
                break;
            }
        }
    } else
#endif
    {
        result = MTLCreateSystemDefaultDevice();
    }

    utils::slog.i << "Selected physical device '"
                  << [result.name cStringUsingEncoding:NSUTF8StringEncoding] << "'"
                  << utils::io::endl;

    return result;
}

id<MTLCommandQueue> PlatformMetal::createCommandQueue(id<MTLDevice> device) noexcept {
    pImpl->mCommandQueue = [device newCommandQueue];
    pImpl->mCommandQueue.label = @"Filament";
    return pImpl->mCommandQueue;
}

id<MTLCommandBuffer> PlatformMetal::createAndEnqueueCommandBuffer() noexcept {
    id<MTLCommandBuffer> commandBuffer = [pImpl->mCommandQueue commandBuffer];
    [commandBuffer enqueue];
    return commandBuffer;
}

void PlatformMetal::setDrawableFailureBehavior(DrawableFailureBehavior behavior) noexcept {
    pImpl->mDrawableFailureBehavior = behavior;
}

PlatformMetal::DrawableFailureBehavior PlatformMetal::getDrawableFailureBehavior() const noexcept {
    return pImpl->mDrawableFailureBehavior;
}

} // namespace filament
