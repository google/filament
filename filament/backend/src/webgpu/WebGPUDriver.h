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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUDRIVER_H
#define TNT_FILAMENT_BACKEND_WEBGPUDRIVER_H

#include <backend/platforms/WebGPUPlatform.h>

#include "DriverBase.h"
#include "private/backend/Dispatcher.h"
#include "private/backend/Driver.h"
#include <backend/DriverEnums.h>

#include <utils/compiler.h>

#include <webgpu/webgpu_cpp.h>

#include <cstdint>

namespace filament::backend {

/**
 * WebGPU backend (driver) implementation
 */
class WebGPUDriver final : public DriverBase {
public:
    ~WebGPUDriver() noexcept override;

    [[nodiscard]] Dispatcher getDispatcher() const noexcept final;
    [[nodiscard]] static Driver* create(WebGPUPlatform& platform) noexcept;

private:
    explicit WebGPUDriver(WebGPUPlatform& platform) noexcept;
    [[nodiscard]] ShaderModel getShaderModel() const noexcept final;
    [[nodiscard]] ShaderLanguage getShaderLanguage() const noexcept final;

    // the platform (e.g. OS) specific aspects of the WebGPU backend are strictly only
    // handled in the WebGPUPlatform
    WebGPUPlatform& mPlatform;
    wgpu::Surface mSurface = nullptr;
    wgpu::Adapter mAdapter = nullptr;
    wgpu::Device mDevice = nullptr;
    wgpu::Queue mQueue = nullptr;
    uint64_t mNextFakeHandle = 1;

    /*
     * Driver interface
     */

    template<typename T>
    friend class ConcreteDispatcher;

#define DECL_DRIVER_API(methodName, paramsDecl, params)                                            \
    UTILS_ALWAYS_INLINE inline void methodName(paramsDecl);

#define DECL_DRIVER_API_SYNCHRONOUS(RetType, methodName, paramsDecl, params)                       \
    RetType methodName(paramsDecl) override;

#define DECL_DRIVER_API_RETURN(RetType, methodName, paramsDecl, params)                            \
    RetType methodName##S() noexcept override;                                                     \
    UTILS_ALWAYS_INLINE inline void methodName##R(RetType, paramsDecl);

#include "private/backend/DriverAPI.inc"
};

}// namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_WEBGPUDRIVER_H
