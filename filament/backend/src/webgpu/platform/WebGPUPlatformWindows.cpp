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

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstdint>
#include <vector>

#include <Windows.h>

/**
 * Windows OS specific implementation aspects of the WebGPU backend
 */

namespace filament::backend {

std::vector<wgpu::RequestAdapterOptions> WebGPUPlatform::getAdapterOptions() {
    constexpr std::array powerPreferences = {
        wgpu::PowerPreference::HighPerformance,
        wgpu::PowerPreference::LowPower };
    constexpr std::array backendTypes = {
        wgpu::BackendType::Vulkan,
        wgpu::BackendType::OpenGL,
        wgpu::BackendType::OpenGLES,
        wgpu::BackendType::D3D12,
        wgpu::BackendType::D3D11 };
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
    HWND window = static_cast<HWND>(nativeWindow);
    RECT windowRect;
    GetWindowRect(window, &windowRect);
    return wgpu::Extent2D{
        .width = static_cast<uint32_t>(windowRect.right - windowRect.left),
        .height = static_cast<uint32_t>(windowRect.bottom - windowRect.top)
    };
}

wgpu::Surface WebGPUPlatform::createSurface(void* nativeWindow, uint64_t /*flags*/) {
    // TODO verify this is necessary for Dawn implementation as well:
    // On (at least) NVIDIA drivers, the Vulkan implementation (specifically the call to
    // vkGetPhysicalDeviceSurfaceCapabilitiesKHR()) does not correctly handle the fact that
    // each native window has its own DPI_AWARENESS_CONTEXT, and erroneously uses the context
    // of the calling thread. As a workaround, we set the current thread's DPI_AWARENESS_CONTEXT
    // to that of the native window we've been given. This isn't a perfect solution, because an
    // application could create swap chains on multiple native windows with varying DPI-awareness,
    // but even then, at least one of the windows would be guaranteed to work correctly.
    SetThreadDpiAwarenessContext(GetWindowDpiAwarenessContext((HWND) nativeWindow));
    wgpu::SurfaceSourceWindowsHWND surfaceSourceWin{};
    surfaceSourceWin.hinstance = GetModuleHandle(nullptr);
    surfaceSourceWin.hwnd = nativeWindow;
    const wgpu::SurfaceDescriptor surfaceDescriptor{
        .nextInChain = &surfaceSourceWin,
        .label = "windows_surface"
    };
    wgpu::Surface surface = mInstance.CreateSurface(&surfaceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(surface.Get() != nullptr) << "Unable to create Windows-backed surface.";
    return surface;
}

}// namespace filament::backend
