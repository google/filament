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

#include <backend/DriverEnums.h>

#include <utils/Panic.h>

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#if defined(__linux__) || defined(__FreeBSD__)
    #define LINUX_OR_FREEBSD 1
#endif

// Platform specific includes and defines
#if defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
    #include <dlfcn.h>
    namespace {
    typedef struct _wl {
        struct wl_display* display;
        struct wl_surface* surface;
        uint32_t width;
        uint32_t height;
    } wl;
    }// namespace
#elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11) && defined(FILAMENT_SUPPORTS_XLIB)
    // TODO: we should allow for headless on Linux explicitly. Right now headless path doesn't work
    #include <dlfcn.h>
    #include <X11/Xlib.h>

    static constexpr const char* LIBRARY_X11 { "libX11.so.6" };

    namespace {
        typedef Display* (*X11_OPEN_DISPLAY)(const char*);
        struct XEnv final {
            X11_OPEN_DISPLAY openDisplay;
            Display* display { nullptr };
            void* library { nullptr };
        } g_x11;
    } // namespace
#elif defined(FILAMENT_SUPPORTS_OSMESA)
    // No need
#else
    #error Not a supported Linux or FeeBSD + WebGPU platform
#endif

/**
 * Linux OS specific implementation aspects of the WebGPU Backend
 */

namespace filament::backend {

std::vector<wgpu::RequestAdapterOptions> WebGPUPlatform::getAdapterOptions() {
    constexpr std::array powerPreferences = {
        wgpu::PowerPreference::HighPerformance,
        wgpu::PowerPreference::LowPower };
    constexpr std::array backendTypes = {
        wgpu::BackendType::Vulkan,
        wgpu::BackendType::OpenGL,
        wgpu::BackendType::OpenGLES };
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
    auto surfaceExtent = wgpu::Extent2D{};
#if defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
    wl* ptrval = reinterpret_cast<wl*>(nativeWindow);
    surfaceExtent.width = ptrval->width;
    surfaceExtent.height = ptrval->height;
    FILAMENT_CHECK_POSTCONDITION(surfaceExtent.width != 0 && surfaceExtent.height != 0)
            << "Unable to get window size for Linux Wayland-backed surface.";
#elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11) && defined (FILAMENT_SUPPORTS_XLIB)
    if (g_x11.library == nullptr) {
        g_x11.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
        FILAMENT_CHECK_PRECONDITION(g_x11.library) << "Unable to open X11 library.";
    }
    if (g_x11.openDisplay == nullptr) {
        g_x11.openDisplay = reinterpret_cast<X11_OPEN_DISPLAY>(dlsym(g_x11.library, "XOpenDisplay"));
        g_x11.display = g_x11.openDisplay(NULL);
        FILAMENT_CHECK_PRECONDITION(g_x11.display) << "Unable to open X11 display.";
    }
    Window window { reinterpret_cast<Window const>(nativeWindow) };
    XWindowAttributes windowAttributes;
    Status ok { XGetWindowAttributes(g_x11.display, window, &windowAttributes) };
    FILAMENT_CHECK_PRECONDITION(ok != 0) << " XGetWindowAttributes failed";
    surfaceExtent.width = static_cast<uint32_t>(windowAttributes.width);
    surfaceExtent.height = static_cast<uint32_t>(windowAttributes.height);

    FILAMENT_CHECK_POSTCONDITION(surfaceExtent.width != 0 && surfaceExtent.height != 0)
            << "Cannot get window surface size for X11 surface for Linux (or FreeBSD) OS "
               "(not built with support for XLIB?)";
#else
    FILAMENT_CHECK_POSTCONDITION(surfaceExtent.width != 0 && surfaceExtent.height != 0)
            << "Not a supported (Linux) OS + WebGPU platform";
#endif
    return surfaceExtent;
}

wgpu::Surface WebGPUPlatform::createSurface(void* nativeWindow, uint64_t /*flags*/) {
    wgpu::Surface surface = nullptr;
#if defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
    wl* ptrval = reinterpret_cast<wl*>(nativeWindow);
    wgpu::SurfaceSourceWaylandSurface surfaceSourceWayland{};
    surfaceSourceWayland.display = ptrval->display;
    surfaceSourceWayland.surface = ptrval->surface;
    wgpu::SurfaceDescriptor const surfaceDescriptor {
        .nextInChain = &surfaceSourceWayland,
        .label = "linux_wayland_surface"
    };
    surface = mInstance.CreateSurface(&surfaceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(surface != nullptr)
            << "Unable to create Linux Wayland-backed surface.";
#elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11) && defined(FILAMENT_SUPPORTS_XLIB)
    if (g_x11.library == nullptr) {
        g_x11.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
        FILAMENT_CHECK_PRECONDITION(g_x11.library) << "Unable to open X11 library.";
    }
    if (g_x11.openDisplay == nullptr) {
        g_x11.openDisplay = (X11_OPEN_DISPLAY) dlsym(g_x11.library, "XOpenDisplay");
        g_x11.display = g_x11.openDisplay(NULL);
        FILAMENT_CHECK_PRECONDITION(g_x11.display) << "Unable to open X11 display.";
    }
    wgpu::SurfaceSourceXlibWindow surfaceSourceXlib{};
    surfaceSourceXlib.display = g_x11.display;
    surfaceSourceXlib.window = reinterpret_cast<Window const>(nativeWindow);
    wgpu::SurfaceDescriptor const surfaceDescriptor {
        .nextInChain = &surfaceSourceXlib,
        .label = "linux_xlib_surface"
    };
    surface = mInstance.CreateSurface(&surfaceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(surface != nullptr)
            << "Unable to create Linux (or FreeBSD) XLib-backed surface.";
#else
    FILAMENT_CHECK_POSTCONDITION(surface != nullptr)
            << "Not a supported (Linux) OS + WebGPU platform";
#endif
    return surface;
}

}// namespace filament::backend
