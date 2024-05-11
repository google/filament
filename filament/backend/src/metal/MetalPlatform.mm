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

#include "MetalPlatform.h"

#include "MetalDriverFactory.h"

#include <utils/Log.h>

#import <Foundation/Foundation.h>

namespace filament::backend {

Platform* createDefaultMetalPlatform() {
    return new MetalPlatform();
}

MetalPlatform::~MetalPlatform() = default;

Driver* MetalPlatform::createDriver(void* /*sharedContext*/, const Platform::DriverConfig& driverConfig) noexcept {
    return MetalDriverFactory::create(this, driverConfig);
}

id<MTLDevice> MetalPlatform::createDevice() noexcept {
    id<MTLDevice> result;

#if !defined(IOS)
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

id<MTLCommandQueue> MetalPlatform::createCommandQueue(id<MTLDevice> device) noexcept {
    mCommandQueue = [device newCommandQueue];
    mCommandQueue.label = @"Filament";
    return mCommandQueue;
}

id<MTLCommandBuffer> MetalPlatform::createAndEnqueueCommandBuffer() noexcept {
    id<MTLCommandBuffer> commandBuffer = [mCommandQueue commandBuffer];
    [commandBuffer enqueue];
    return commandBuffer;
}

} // namespace filament
