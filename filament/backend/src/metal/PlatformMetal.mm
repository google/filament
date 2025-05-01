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
#include <backend/platforms/PlatformMetal-ObjC.h>

#include "MetalDriverFactory.h"

#include <absl/log/log.h>

#import <Foundation/Foundation.h>

#include <atomic>
#include <mutex>

namespace filament::backend {

struct PlatformMetalImpl {
    std::mutex mLock;   // locks mDevice and mCommandQueue
    id<MTLDevice> mDevice = nil;
    id<MTLCommandQueue> mCommandQueue = nil;

    // read form driver thread, read/written to from client thread
    std::atomic<PlatformMetal::DrawableFailureBehavior> mDrawableFailureBehavior =
            PlatformMetal::DrawableFailureBehavior::PANIC;

    // These methods must be called with mLock held
    void createDeviceImpl(MetalDevice& outDevice);
    void createCommandQueueImpl(MetalDevice& device, MetalCommandQueue& outCommandQueue);
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


bool PlatformMetal::initialize() noexcept {
    std::lock_guard<std::mutex> lock(pImpl->mLock);

    MetalDevice device{};
    pImpl->createDeviceImpl(device);
    if (device.device == nil) {
        return false;
    }

    MetalCommandQueue commandQueue{};
    pImpl->createCommandQueueImpl(device, commandQueue);
    if (commandQueue.commandQueue == nil) {
        return false;
    }

    return true;
}

void PlatformMetal::createDevice(MetalDevice& outDevice) noexcept {
    std::lock_guard<std::mutex> lock(pImpl->mLock);
    pImpl->createDeviceImpl(outDevice);
}

void PlatformMetal::createCommandQueue(
        MetalDevice& device, MetalCommandQueue& outCommandQueue) noexcept {
    std::lock_guard<std::mutex> lock(pImpl->mLock);
    pImpl->createCommandQueueImpl(device, outCommandQueue);
}

void PlatformMetal::createAndEnqueueCommandBuffer(MetalCommandBuffer& outCommandBuffer) noexcept {
    std::lock_guard<std::mutex> lock(pImpl->mLock);
    id<MTLCommandBuffer> commandBuffer = [pImpl->mCommandQueue commandBuffer];
    [commandBuffer enqueue];
    outCommandBuffer.commandBuffer = commandBuffer;
}

void PlatformMetal::setDrawableFailureBehavior(DrawableFailureBehavior behavior) noexcept {
    pImpl->mDrawableFailureBehavior = behavior;
}

PlatformMetal::DrawableFailureBehavior PlatformMetal::getDrawableFailureBehavior() const noexcept {
    return pImpl->mDrawableFailureBehavior;
}

// -------------------------------------------------------------------------------------------------

void PlatformMetalImpl::createDeviceImpl(MetalDevice& outDevice) {
    if (mDevice) {
        outDevice.device = mDevice;
        return;
    }

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

    LOG(INFO) << "Selected physical device '"
              << [result.name cStringUsingEncoding:NSUTF8StringEncoding] << "'";

    outDevice.device = result;
    mDevice = result;
}

void PlatformMetalImpl::createCommandQueueImpl(MetalDevice& device, MetalCommandQueue& outCommandQueue) {
    if (mCommandQueue) {
        outCommandQueue.commandQueue = mCommandQueue;
        return;
    }
    mCommandQueue = [device.device newCommandQueue];
    mCommandQueue.label = @"Filament";
    outCommandQueue.commandQueue = mCommandQueue;
}

} // namespace filament
