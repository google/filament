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

#ifndef TNT_FILAMENT_DRIVER_PLATFORM_METAL_H
#define TNT_FILAMENT_DRIVER_PLATFORM_METAL_H

#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#import <Metal/Metal.h>

namespace filament {
namespace backend {

class MetalPlatform : public DefaultPlatform {
public:
    ~MetalPlatform() override;

    Driver* createDriver(void* sharedContext) noexcept override;
    int getOSVersion() const noexcept override { return 0; }

    virtual id<MTLDevice> createDevice() noexcept;
    virtual id<MTLCommandQueue> createCommandQueue(id<MTLDevice> device) noexcept;

    /**
     * Obtain a MTLCommandBuffer enqueued on this Platform's MTLCommandQueue. The command buffer is
     * guaranteed to execute before all subsequent command buffers.
     */
    id<MTLCommandBuffer> createAndEnqueueCommandBuffer() noexcept;

private:
    id<MTLCommandQueue> mCommandQueue = nil;

};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_PLATFORM_METAL_H
