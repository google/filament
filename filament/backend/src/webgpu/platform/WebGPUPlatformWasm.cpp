/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <backend/platforms/WebGPUPlatformWasm.h>

#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>
#include <emscripten/html5.h>

#include <vector>
#include <cstdint>

extern "C" WGPUDevice emscripten_webgpu_get_device(void);

namespace filament::backend {

wgpu::Device WebGPUPlatformWasm::sDevice = nullptr;

std::vector<wgpu::RequestAdapterOptions> WebGPUPlatformWasm::getAdapterOptions() {
    std::vector<wgpu::RequestAdapterOptions> requests;
    requests.reserve(2);
    requests.emplace_back(wgpu::RequestAdapterOptions{
        .powerPreference = wgpu::PowerPreference::HighPerformance,
        .forceFallbackAdapter = false,
        .backendType = wgpu::BackendType::WebGPU,
    });
    requests.emplace_back(wgpu::RequestAdapterOptions{
        .powerPreference = wgpu::PowerPreference::LowPower,
        .forceFallbackAdapter = false,
        .backendType = wgpu::BackendType::WebGPU,
    });
    return requests;
}

wgpu::Adapter WebGPUPlatformWasm::requestAdapter(wgpu::Surface const& surface) { return nullptr; }

wgpu::Device WebGPUPlatformWasm::requestDevice(wgpu::Adapter const& adapter) {
    if (sDevice == nullptr) {
        WGPUDevice device = emscripten_webgpu_get_device();
        FILAMENT_CHECK_POSTCONDITION(device != nullptr)
                << "WebGPU device not initialized. Call Filament.initWebGPU() first.";
        sDevice = wgpu::Device::Acquire(device);
    }
    return sDevice;
}

wgpu::Extent2D WebGPUPlatformWasm::getSurfaceExtent(void* nativeWindow) const {
    const char* selector = static_cast<const char*>(nativeWindow);
    int width = 0;
    int height = 0;
    emscripten_get_canvas_element_size(selector, &width, &height);
    return wgpu::Extent2D{
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
    };
}

wgpu::Surface WebGPUPlatformWasm::createSurface(void* nativeWindow, uint64_t /*flags*/) {
    wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvasSource{};
    canvasSource.selector = static_cast<const char*>(nativeWindow);
    const wgpu::SurfaceDescriptor surfaceDescriptor{
        .nextInChain = &canvasSource,
        .label = "wasm_surface",
    };
    wgpu::Surface surface = mInstance.CreateSurface(&surfaceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(surface.Get() != nullptr)
            << "Unable to create Wasm-backed surface.";
    return surface;
}

} // namespace filament::backend
