/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <backend/platforms/PlatformOSMesa.h>

#include <utils/Log.h>
#include <utils/Panic.h>

#include <dlfcn.h>
#include <memory>

#if defined(__linux__)
// This is to ensure that linking during compilation will not fail even if
// OSMesaGetProcAddress is not linked.
__attribute__((weak)) OSMESAproc OSMesaGetProcAddress(char const*);
#endif

namespace filament::backend {

using namespace backend;

namespace {

using BackingType = GLfloat;
#define BACKING_GL_TYPE GL_FLOAT

struct OSMesaSwapchain {
    OSMesaSwapchain(uint32_t width, uint32_t height)
        : width(width),
          height(height),
          buffer(new uint8_t[width * height * 4 * sizeof(BackingType)]) {}

    uint32_t width = 0;
    uint32_t height = 0;
    std::unique_ptr<uint8_t[]> buffer;
};

struct OSMesaAPI {
private:
    using CreateContextAttribsFunc = OSMesaContext (*)(const int *, OSMesaContext);
    using DestroyContextFunc = GLboolean (*)(OSMesaContext);
    using MakeCurrentFunc = GLboolean (*)(OSMesaContext ctx, void* buffer, GLenum type,
            GLsizei width, GLsizei height);
    using GetProcAddressFunc = OSMESAproc (*)(const char* funcName);

public:
    CreateContextAttribsFunc fOSMesaCreateContextAttribs;
    DestroyContextFunc fOSMesaDestroyContext;
    MakeCurrentFunc fOSMesaMakeCurrent;
    GetProcAddressFunc fOSMesaGetProcAddress;

    OSMesaAPI() {
        static constexpr char const* libraryNames[] = {
#if defined(__linux__)
            "libOSMesa.so",
            "libosmesa.so",
#elif defined(__APPLE__)
            "libOSMesa.dylib",
#endif
        };
        for (char const* libName: libraryNames) {
            mLib = dlopen(libName, RTLD_GLOBAL | RTLD_NOW);
            if (mLib) {
                break;
            }
        }
        if (mLib) {
            // Loading from a libosmesa.os
            fOSMesaGetProcAddress = (GetProcAddressFunc) dlsym(mLib, "OSMesaGetProcAddress");
        }
#if defined(__linux__)
        else {
            // Filament is built into a .so
            fOSMesaGetProcAddress = (GetProcAddressFunc) dlsym(RTLD_LOCAL, "OSMesaGetProcAddress");
        }
        if (!fOSMesaGetProcAddress) {
            // Statically linking osmesa
            fOSMesaGetProcAddress = OSMesaGetProcAddress;
        }
#endif // __linux__

        FILAMENT_CHECK_PRECONDITION(fOSMesaGetProcAddress)
                << "Unable to link against libOSMesa to create a software GL context";

        fOSMesaCreateContextAttribs =
                (CreateContextAttribsFunc) fOSMesaGetProcAddress("OSMesaCreateContextAttribs");
        fOSMesaDestroyContext = (DestroyContextFunc) fOSMesaGetProcAddress("OSMesaDestroyContext");
        fOSMesaMakeCurrent = (MakeCurrentFunc) fOSMesaGetProcAddress("OSMesaMakeCurrent");
    }

    ~OSMesaAPI() {
        if (mLib) {
            dlclose(mLib);
        }
    }
private:
    void* mLib = nullptr;
};

}// anonymous namespace

Driver* PlatformOSMesa::createDriver(void* sharedGLContext,
        const DriverConfig& driverConfig) noexcept {

    OSMesaAPI* api = new OSMesaAPI();
    mOsMesaApi = api;

    static constexpr int attribs[] = {
        OSMESA_FORMAT, GL_RGBA,
        OSMESA_DEPTH_BITS, 24,
        OSMESA_STENCIL_BITS, 8,
        OSMESA_ACCUM_BITS, 0,
        OSMESA_PROFILE, OSMESA_CORE_PROFILE,
        0,
    };

    FILAMENT_CHECK_PRECONDITION(sharedGLContext == nullptr)
            << "shared GL context is not supported with PlatformOSMesa";
    mContext = api->fOSMesaCreateContextAttribs(attribs, NULL);

    // We need to do a no-op makecurrent here so that the context will be in a correct state before
    // any GL calls.
    auto chain = createSwapChain(1, 1, 0);
    makeCurrent(ContextType::UNPROTECTED, chain, nullptr);
    destroySwapChain(chain);

    int result = bluegl::bind();
    FILAMENT_CHECK_POSTCONDITION(!result) << "Unable to load OpenGL entry points.";

    return OpenGLPlatform::createDefaultDriver(this, sharedGLContext, driverConfig);
}

void PlatformOSMesa::terminate() noexcept {
    OSMesaAPI* api = (OSMesaAPI*) mOsMesaApi;
    api->fOSMesaDestroyContext(mContext);
    delete api;
    mOsMesaApi = nullptr;

    bluegl::unbind();
}

Platform::SwapChain* PlatformOSMesa::createSwapChain(void* nativeWindow, uint64_t flags) noexcept {
    FILAMENT_CHECK_POSTCONDITION(false) << "Cannot create non-headless swapchain";
    return (SwapChain*) nativeWindow;
}

Platform::SwapChain* PlatformOSMesa::createSwapChain(uint32_t width, uint32_t height,
        uint64_t flags) noexcept {
    OSMesaSwapchain* swapchain = new OSMesaSwapchain(width, height);
    return (SwapChain*) swapchain;
}

void PlatformOSMesa::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
    OSMesaSwapchain* impl = (OSMesaSwapchain*) swapChain;
    delete impl;
}

bool PlatformOSMesa::makeCurrent(ContextType type, SwapChain* drawSwapChain,
        SwapChain* readSwapChain) {
    OSMesaAPI* api = (OSMesaAPI*) mOsMesaApi;
    OSMesaSwapchain* impl = (OSMesaSwapchain*) drawSwapChain;

    auto result = api->fOSMesaMakeCurrent(mContext, (BackingType*) impl->buffer.get(),
            BACKING_GL_TYPE, impl->width, impl->height);
    FILAMENT_CHECK_POSTCONDITION(result == GL_TRUE) << "OSMesaMakeCurrent failed!";

    return true;
}

void PlatformOSMesa::commit(Platform::SwapChain* swapChain) noexcept {
    // No-op since we are not scanning out to a display.
}

} // namespace filament::backend
