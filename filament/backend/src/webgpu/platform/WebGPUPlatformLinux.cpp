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

#include <cstddef>
#include <cstdint>

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
#elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11)
    // TODO: we should allow for headless on Linux explicitly. Right now this is the headless path
    // (with no FILAMENT_SUPPORTS_XCB or FILAMENT_SUPPORTS_XLIB).
    #include <dlfcn.h>
    #if defined(FILAMENT_SUPPORTS_XCB)
        #include <xcb/xcb.h>
        namespace {
        typedef xcb_connection_t* (*XCB_CONNECT)(const char* displayname, int* screenp);
        }// namespace
    #endif
    #if defined(FILAMENT_SUPPORTS_XLIB)
        #include <X11/Xlib.h>
        namespace {
        typedef Display* (*X11_OPEN_DISPLAY)(const char*);
        }// namespace
    #endif
    static constexpr const char* LIBRARY_X11 = "libX11.so.6";
    namespace {
    struct XEnv {
    #if defined(FILAMENT_SUPPORTS_XCB)
        XCB_CONNECT xcbConnect;
        xcb_connection_t* connection;
    #endif
    #if defined(FILAMENT_SUPPORTS_XLIB)
        X11_OPEN_DISPLAY openDisplay;
        Display* display;
    #endif
        void* library = nullptr;
    } g_x11;
    }// namespace
#else
    #error Not a supported Linux or FeeBSD + WebGPU platform
#endif

/**
 * Linux OS specific implementation aspects of the WebGPU Backend
 */

namespace filament::backend {

wgpu::Surface WebGPUPlatform::createSurface(void* nativeWindow, uint64_t flags) {
    wgpu::Surface surface = nullptr;
#if defined(__linux__) && defined(FILAMENT_SUPPORTS_WAYLAND)
    wl* ptrval = reinterpret_cast<wl*>(nativeWindow);
    wgpu::SurfaceSourceWaylandSurface surfaceSourceWayland{};
    surfaceSourceWayland.display = ptrval->display;
    surfaceSourceWayland.surface = ptrval->surface;
    wgpu::SurfaceDescriptor surfaceDescriptor{
        .nextInChain = &surfaceSourceWayland,
        .label = "linux_wayland_surface"
    };
    surface = mInstance.CreateSurface(&surfaceDescriptor);
    FILAMENT_CHECK_POSTCONDITION(surface != nullptr)
            << "Unable to create Linux Wayland-backed surface.";
#elif defined(LINUX_OR_FREEBSD) && defined(FILAMENT_SUPPORTS_X11)
    if (g_x11.library == nullptr) {
        g_x11.library = dlopen(LIBRARY_X11, RTLD_LOCAL | RTLD_NOW);
        FILAMENT_CHECK_PRECONDITION(g_x11.library) << "Unable to open X11 library.";
        #if defined(FILAMENT_SUPPORTS_XCB)
            g_x11.xcbConnect = (XCB_CONNECT) dlsym(g_x11.library, "xcb_connect");
            int screen = 0;
            g_x11.connection = g_x11.xcbConnect(nullptr, &screen);
        #endif
        #if defined(FILAMENT_SUPPORTS_XLIB)
            g_x11.openDisplay = (X11_OPEN_DISPLAY) dlsym(g_x11.library, "XOpenDisplay");
            g_x11.display = g_x11.openDisplay(NULL);
            FILAMENT_CHECK_PRECONDITION(g_x11.display) << "Unable to open X11 display.";
        #endif
    }
    #if defined(FILAMENT_SUPPORTS_XCB) || defined(FILAMENT_SUPPORTS_XLIB)
        bool useXcb = false;
    #endif
    #if defined(FILAMENT_SUPPORTS_XCB)
        #if defined(FILAMENT_SUPPORTS_XLIB)
            useXcb = (flags & SWAP_CHAIN_CONFIG_ENABLE_XCB) != 0;
        #else
            useXcb = true;
        #endif
        if (useXcb) {
            wgpu::SurfaceSourceXCBWindow surfaceSourceXcb{};
            surfaceSourceXcb.connection = g_x11.connection;

            // TODO: this looks really wrong, please fix!!
            surfaceSourceXcb.window = *((uint32_t*) nativeWindow);
            wgpu::SurfaceDescriptor surfaceDescriptor{
                .nextInChain = &surfaceSourceXcb,
                .label = "linux_xcb_surface"
            };
            surface = mInstance.CreateSurface(&surfaceDescriptor);
            FILAMENT_CHECK_POSTCONDITION(surface != nullptr)
                    << "Unable to create Linux (or FreeBSD) XCB-backed surface.";
        }
    #endif
    #if defined(FILAMENT_SUPPORTS_XLIB)
        if (!useXcb) {
            wgpu::SurfaceSourceXlibWindow surfaceSourceXlib{};
            surfaceSourceXlib.display = g_x11.display;
            surfaceSourceXlib.window = reinterpret_cast<uint64_t>(nativeWindow);
            wgpu::SurfaceDescriptor surfaceDescriptor{
                .nextInChain = &surfaceSourceXlib,
                .label = "linux_xlib_surface"
            };
            surface = mInstance.CreateSurface(&surfaceDescriptor);
            FILAMENT_CHECK_POSTCONDITION(surface != nullptr)
                    << "Unable to create Linux (or FreeBSD) XLib-backed surface.";
        }
    #endif
    FILAMENT_CHECK_POSTCONDITION(surface != nullptr)
            << "Cannot create WebGPU X11 surface for Linux (or FreeBSD) OS "
               "(not built with support for XCB or XLIB?)";
#elif defined(__linux__)
    FILAMENT_CHECK_POSTCONDITION(surface != nullptr)
            << "Cannot create WebGPU surface for Linux (or FreeBSD) OS "
               "(not built with support for Wayland or X11?)";
#else
    FILAMENT_CHECK_POSTCONDITION(surface != nullptr)
            << "Not a supported (Linux) OS + WebGPU platform";
#endif
    return surface;
}

}// namespace filament::backend
