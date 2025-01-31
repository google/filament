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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_PLATFORMMETAL_H
#define TNT_FILAMENT_BACKEND_PRIVATE_PLATFORMMETAL_H

#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#import <Metal/Metal.h>

namespace filament::backend {

struct PlatformMetalImpl;

class PlatformMetal final : public Platform {
public:
    PlatformMetal();
    ~PlatformMetal() noexcept override;

    Driver* createDriver(void* sharedContext, const Platform::DriverConfig& driverConfig) noexcept override;
    int getOSVersion() const noexcept override { return 0; }

    /**
     * Obtain the preferred Metal device object for the backend to use.
     *
     * On desktop platforms, there may be multiple GPUs suitable for rendering, and this method is
     * free to decide which one to use. On mobile systems with a single GPU, implementations should
     * simply return the result of MTLCreateSystemDefaultDevice();
     */
    virtual id<MTLDevice> createDevice() noexcept;

    /**
     * Create a command submission queue on the Metal device object.
     *
     * @param device The device which was returned from createDevice()
     */
    virtual id<MTLCommandQueue> createCommandQueue(id<MTLDevice> device) noexcept;

    /**
     * Obtain a MTLCommandBuffer enqueued on this Platform's MTLCommandQueue. The command buffer is
     * guaranteed to execute before all subsequent command buffers created either by Filament, or
     * further calls to this method.
     */
    id<MTLCommandBuffer> createAndEnqueueCommandBuffer() noexcept;

    /**
     * The action to take if a Drawable cannot be acquired.
     *
     * Each frame rendered requires a CAMetalDrawable texture, which is presented on-screen at the
     * completion of each frame. These are limited and provided round-robin style by the system.
     */
    enum class DrawableFailureBehavior : uint8_t {
        /**
         * Terminates the application and reports an error message (default).
         */
        PANIC,
        /*
         * Aborts execution of the current frame. The Metal backend will attempt to acquire a new
         * drawable at the next frame.
         */
        ABORT_FRAME
    };
    void setDrawableFailureBehavior(DrawableFailureBehavior behavior) noexcept;
    DrawableFailureBehavior getDrawableFailureBehavior() const noexcept;

private:
    PlatformMetalImpl* pImpl = nullptr;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_PLATFORMMETAL_H
