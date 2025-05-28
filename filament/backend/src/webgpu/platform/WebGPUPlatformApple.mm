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
#include <utils/ostream.h>

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <vector>

// Platform specific includes and defines
#include <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

/**
 * Apple (Mac OS and IOS) specific implementation aspects of the WebGPU backend
 */

namespace filament::backend {

std::vector<wgpu::RequestAdapterOptions> WebGPUPlatform::getAdapterOptions() {
    constexpr std::array powerPreferences = {
        wgpu::PowerPreference::HighPerformance,
        wgpu::PowerPreference::LowPower };
    constexpr std::array backendTypes = { wgpu::BackendType::Metal };
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
    // Both IOS and MacOS expects CAMetalLayer.
    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*) nativeWindow;
    return wgpu::Extent2D{
        .width = static_cast<uint32_t>(metalLayer.drawableSize.width),
        .height = static_cast<uint32_t>(metalLayer.drawableSize.height)
    };
}

wgpu::Surface WebGPUPlatform::createSurface(void* nativeWindow, uint64_t /*flags*/) {
    wgpu::Surface surface = nullptr;
    // Both IOS and MacOS expects CAMetalLayer.
    CAMetalLayer* metalLayer = (__bridge CAMetalLayer*) nativeWindow;
    wgpu::SurfaceSourceMetalLayer surfaceSourceMetalLayer{};
    surfaceSourceMetalLayer.layer = (__bridge void*) metalLayer;
    wgpu::SurfaceDescriptor surfaceDescriptor = {
        .nextInChain = &surfaceSourceMetalLayer,
        .label = "metal_surface",
    };
    surface = mInstance.CreateSurface(&surfaceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(surface != nullptr) << "Unable to create Metal-backed surface.";
    return surface;
}

}// namespace filament::backend
