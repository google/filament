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
    using CreateContextFunc = OSMesaContext (*)(GLenum format, OSMesaContext);
    using DestroyContextFunc = GLboolean (*)(OSMesaContext);
    using MakeCurrentFunc = GLboolean (*)(OSMesaContext ctx, void* buffer, GLenum type,
            GLsizei width, GLsizei height);
    using GetProcAddressFunc = OSMESAproc (*)(const char* funcName);

public:
    CreateContextFunc OSMesaCreateContext;
    DestroyContextFunc OSMesaDestroyContext;
    MakeCurrentFunc OSMesaMakeCurrent;
    GetProcAddressFunc OSMesaGetProcAddress;

    OSMesaAPI() {
        constexpr char const* libraryNames[] = {"libOSMesa.so", "libosmesa.so"};
        for (char const* libName: libraryNames) {
            mLib = dlopen(libName, RTLD_GLOBAL | RTLD_NOW);
            if (mLib) {
                break;
            }
        }
        FILAMENT_CHECK_PRECONDITION(mLib)
                << "Unable to dlopen libOSMesa to create a software GL context";

        OSMesaGetProcAddress = (GetProcAddressFunc) dlsym(mLib, "OSMesaGetProcAddress");

        OSMesaCreateContext = (CreateContextFunc) OSMesaGetProcAddress("OSMesaCreateContext");
        OSMesaDestroyContext =
                (DestroyContextFunc) OSMesaGetProcAddress("OSMesaDestroyContext");
        OSMesaMakeCurrent = (MakeCurrentFunc) OSMesaGetProcAddress("OSMesaMakeCurrent");
    }

    ~OSMesaAPI() {
        dlclose(mLib);
    }
private:
    void* mLib = nullptr;
};

}// anonymous namespace

Driver* PlatformOSMesa::createDriver(void* const sharedGLContext,
        const DriverConfig& driverConfig) noexcept {
    OSMesaAPI* api = new OSMesaAPI();
    mOsMesaApi = api;

    FILAMENT_CHECK_PRECONDITION(sharedGLContext == nullptr)
            << "shared GL context is not supported with PlatformOSMesa";
    mContext = api->OSMesaCreateContext(GL_RGBA, NULL);

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
    api->OSMesaDestroyContext(mContext);
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
        SwapChain* readSwapChain) noexcept {
    OSMesaAPI* api = (OSMesaAPI*) mOsMesaApi;
    OSMesaSwapchain* impl = (OSMesaSwapchain*) drawSwapChain;

    auto result = api->OSMesaMakeCurrent(mContext, (BackingType*) impl->buffer.get(),
            BACKING_GL_TYPE, impl->width, impl->height);
    FILAMENT_CHECK_POSTCONDITION(result == GL_TRUE) << "OSMesaMakeCurrent failed!";

    return true;
}

void PlatformOSMesa::commit(Platform::SwapChain* swapChain) noexcept {
    // No-op since we are not scanning out to a display.
}

} // namespace filament::backend
