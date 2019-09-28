/*
 * Copyright (C) 2018 The Android Open Source Project
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


#include "ExternalTextureManagerAndroid.h"

#include <utils/api_level.h>
#include <utils/compiler.h>
#include <utils/Log.h>

#include <dlfcn.h>

using namespace utils;

namespace filament {
using namespace backend;

template <typename T>
static void loadSymbol(T*& pfn, const char *symbol) noexcept {
    pfn = (T*)dlsym(RTLD_DEFAULT, symbol);
}

// ------------------------------------------------------------------------------------------------

namespace ndk {
/*
 * This mimics the GraphicBuffer class for pre-API 26
 */
template<typename NATIVE_TYPE, typename REF>
struct ANativeObjectBase : public NATIVE_TYPE, public REF {
};

struct android_native_base_t {
    unsigned int magic;
    unsigned int version;
    void* reserved[4];
    void (* incRef)(struct android_native_base_t* base);
    void (* decRef)(struct android_native_base_t* base);
};

struct ANativeWindowBuffer {
    struct android_native_base_t common;
    unsigned int width;
    unsigned int height;
    unsigned int stride;
    unsigned int format;
    unsigned int usage_deprecated;
    uintptr_t layerCount;
    void* reserved[1];
    const void* handle;
    uint64_t usage;
    void* reserved_proc[8 - (sizeof(uint64_t) / sizeof(void*))];
};

struct RefBase {
    virtual ~RefBase() = default;
    void* ref;
};

struct GraphicBuffer : public ANativeObjectBase<ANativeWindowBuffer, RefBase> {
};
struct GraphicBufferWrapper {
    GraphicBuffer* graphicBuffer;
};
} // namespace ndk

// ------------------------------------------------------------------------------------------------
using namespace ndk;

struct EGLExternalTexture : public ExternalTextureManagerAndroid::ExternalTexture {
    GraphicBufferWrapper* graphicBufferWrapper = nullptr;
};

ExternalTextureManagerAndroid& ExternalTextureManagerAndroid::get() noexcept {
    static ExternalTextureManagerAndroid instance;
    return instance;
}

// called on gl thread
ExternalTextureManagerAndroid::ExternalTextureManagerAndroid() noexcept
        : mVm(VirtualMachineEnv::get()) {
    // if we compile for API 26 (Oreo) and above, we're guaranteed to have AHardwareBuffer
    // in all other cases, we need to get them at runtime.
#ifndef PLATFORM_HAS_HARDWAREBUFFER
    // the following dlsym() calls don't work on API 19
    if (api_level() >= 21) {
        loadSymbol(AHardwareBuffer_allocate, "AHardwareBuffer_allocate");
        loadSymbol(AHardwareBuffer_release, "AHardwareBuffer_release");
    }
#endif
}

// not quite sure on which thread this is going to be called
ExternalTextureManagerAndroid::~ExternalTextureManagerAndroid() noexcept {
#ifndef PLATFORM_HAS_HARDWAREBUFFER
    if (mGraphicBufferClass) {
        JNIEnv* env = VirtualMachineEnv::getThreadEnvironment();
        if (env) {
            env->DeleteGlobalRef(mGraphicBufferClass);
        }
    }
#endif
}

// called on gl thread
backend::Platform::ExternalTexture* ExternalTextureManagerAndroid::create() noexcept {
#ifndef PLATFORM_HAS_HARDWAREBUFFER
    if (!AHardwareBuffer_allocate) {
        // initialize java stuff on-demand
        if (!mGraphicBufferClass) {
            JNIEnv* env = mVm.getEnvironment();
            mGraphicBufferClass = env->FindClass("android/view/GraphicBuffer");
            mGraphicBuffer_nCreateGraphicBuffer = env->GetStaticMethodID(
                    mGraphicBufferClass, "nCreateGraphicBuffer", "(IIII)J");
            mGraphicBuffer_nDestroyGraphicBuffer = env->GetStaticMethodID(
                    mGraphicBufferClass, "nDestroyGraphicBuffer", "(J)V");

            mGraphicBufferClass = static_cast<jclass>(env->NewGlobalRef(mGraphicBufferClass));
        }
    }
#endif
    EGLExternalTexture* ets = new EGLExternalTexture;
    return ets;
}

// called on app thread
void ExternalTextureManagerAndroid::reallocate(
        backend::Platform::ExternalTexture* ets, uint32_t w, uint32_t h,
        backend::TextureFormat format, uint64_t usage) noexcept {
    destroyStorage(ets);
    alloc(ets, w, h, format, usage);
}

// called on gl thread
void ExternalTextureManagerAndroid::destroy(backend::Platform::ExternalTexture* ets) noexcept {
    destroyStorage(ets);
    delete static_cast<EGLExternalTexture*>(ets);
}

// called on app thread
void ExternalTextureManagerAndroid::alloc(
        backend::Platform::ExternalTexture* t,
        uint32_t w, uint32_t h, TextureFormat format, uint64_t usage) noexcept {

    EGLExternalTexture* ets = static_cast<EGLExternalTexture*>(t);

    AHardwareBuffer_Desc desc = { w, h, 1, 0, usage, 0, 0, 0 };
    switch (format) {
        case TextureFormat::RGB8:
            // don't use R8G8B8 here because some drivers produce garbled images
            desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM;
            break;
        case TextureFormat::RGBA8:
            desc.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
            break;
        case TextureFormat::RGB565:
            desc.format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
            break;
        case TextureFormat::RGB10_A2:
            desc.format = AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM;
            break;
        default:
            slog.e << "Unsupported format " << (int)format << ", use RGBA8 or RGB8 only."
                   << io::endl;
            return;
    }

#ifndef PLATFORM_HAS_HARDWAREBUFFER
    if (AHardwareBuffer_allocate) {
#endif
        // allocate new storage...
        AHardwareBuffer* buffer = nullptr;
        if (AHardwareBuffer_allocate(&desc, &buffer) < 0) {
            slog.e << "AHardwareBuffer_allocate() failed" << io::endl;
            return;
        }
        ets->hardwareBuffer = buffer;
#ifndef PLATFORM_HAS_HARDWAREBUFFER
    } else {
        // note: This is called on the application thread (not the GL thread)
        JNIEnv* env = VirtualMachineEnv::getThreadEnvironment();
        if (!env) {
            return; // this should not happen
        }

        jlong graphicBufferWrapperJni = env->CallStaticLongMethod(mGraphicBufferClass,
                mGraphicBuffer_nCreateGraphicBuffer, w, h, desc.format, usage);
        VirtualMachineEnv::handleException(env);

        if (graphicBufferWrapperJni) {
            GraphicBuffer* const gb = ((GraphicBufferWrapper*)graphicBufferWrapperJni)->graphicBuffer;
            ets->graphicBufferWrapper = (GraphicBufferWrapper*)graphicBufferWrapperJni;
            ets->clientBuffer = static_cast<ANativeWindowBuffer*>(gb);
        }
    }
#endif
}

// called on gl thread
void ExternalTextureManagerAndroid::destroyStorage(backend::Platform::ExternalTexture* t) noexcept {
        EGLExternalTexture* ets = static_cast<EGLExternalTexture*>(t);

#ifndef PLATFORM_HAS_HARDWAREBUFFER
    if (AHardwareBuffer_allocate) {
#endif
        // destroy the current storage if any
        if (ets->hardwareBuffer) {
            AHardwareBuffer_release(ets->hardwareBuffer);
            ets->hardwareBuffer = nullptr;
        }

#ifndef PLATFORM_HAS_HARDWAREBUFFER
    } else {
        if (ets->graphicBufferWrapper) {
            JNIEnv* env = VirtualMachineEnv::get().getEnvironment();
            env->CallStaticVoidMethod(mGraphicBufferClass,
                    mGraphicBuffer_nDestroyGraphicBuffer, (jlong)ets->graphicBufferWrapper);
            VirtualMachineEnv::handleException(env);
            ets->graphicBufferWrapper = nullptr;
        }
    }
#endif
}

} // namespace filament

