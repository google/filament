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

#include <backend/platforms/WebGPUPlatform.h>

#include <utils/Panic.h>

#include <android/native_window.h>
#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <vector>

/**
 * Android OS specific implementation aspects of the WebGPU backend
 */

namespace filament::backend {

std::vector<wgpu::RequestAdapterOptions> WebGPUPlatform::getAdapterOptions() {
    constexpr std::array powerPreferences = {
        wgpu::PowerPreference::HighPerformance,
        wgpu::PowerPreference::LowPower };
    constexpr std::array backendTypes = { wgpu::BackendType::Vulkan, wgpu::BackendType::OpenGLES };
    constexpr std::array forceFallbackAdapters = { false, true };
    constexpr size_t totalCombinations =
            powerPreferences.size() * backendTypes.size() * forceFallbackAdapters.size();
    std::vector<wgpu::RequestAdapterOptions> requests;
    requests.reserve(totalCombinations);
    for (auto powerPreference: powerPreferences) {
        for (auto backendType: backendTypes) {
            for (auto forceFallbackAdapter: forceFallbackAdapters) {
                requests.emplace_back(
                        wgpu::RequestAdapterOptions{
                            .powerPreference = powerPreference,
                            .forceFallbackAdapter = forceFallbackAdapter,
                            .backendType = backendType });
            }
        }
    }
    return requests;
}

wgpu::Extent2D WebGPUPlatform::getSurfaceExtent(void* nativeWindow) const {
    ANativeWindow* window = static_cast<ANativeWindow*>(nativeWindow);
    return wgpu::Extent2D{
        .width = static_cast<uint32_t>(ANativeWindow_getWidth(window)),
        .height = static_cast<uint32_t>(ANativeWindow_getHeight(window))
    };
}

wgpu::Surface WebGPUPlatform::createSurface(void* nativeWindow, uint64_t /*flags*/) {
    wgpu::SurfaceSourceAndroidNativeWindow surfaceSourceAndroidWindow{};
    surfaceSourceAndroidWindow.window = nativeWindow;
    wgpu::SurfaceDescriptor surfaceDescriptor{
        .nextInChain = &surfaceSourceAndroidWindow,
        .label = "android_surface"
    };
    wgpu::Surface surface = mInstance.CreateSurface(&surfaceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(surface != nullptr) << "Unable to create Android-backed surface.";
    return surface;
}

}// namespace filament::backend
