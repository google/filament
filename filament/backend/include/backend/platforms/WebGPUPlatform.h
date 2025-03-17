/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_PLATFORMS_WEBGPUPLATFORM_H
#define TNT_FILAMENT_BACKEND_PLATFORMS_WEBGPUPLATFORM_H

#include <backend/Platform.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

/**
 * A Platform interface, handling the environment-specific concerns, e.g. OS, for creating a WebGPU
 * driver (backend).
 */
class WebGPUPlatform final : public Platform {
public:
    WebGPUPlatform();
    ~WebGPUPlatform() override = default;

    [[nodiscard]] int getOSVersion() const noexcept final { return 0; }

    [[nodiscard]] wgpu::Instance& getInstance() noexcept { return mInstance; }

    // either returns a valid surface or panics
    [[nodiscard]] wgpu::Surface createSurface(void* nativeWindow, uint64_t flags);
    // either returns a valid adapter or panics
    [[nodiscard]] wgpu::Adapter requestAdapter(wgpu::Surface const& surface);
    // either returns a valid device or panics
    [[nodiscard]] wgpu::Device requestDevice(wgpu::Adapter const& adapter);

protected:
    [[nodiscard]] Driver* createDriver(void* sharedContext,
            const Platform::DriverConfig& driverConfig) noexcept override;

private:
    // we may consider having the driver own this in the future
    wgpu::Instance mInstance;
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_PLATFORMS_WEBGPUPLATFORM_H
