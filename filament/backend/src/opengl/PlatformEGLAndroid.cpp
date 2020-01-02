/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "PlatformEGLAndroid.h"

#include "OpenGLDriver.h"
#include "OpenGLContext.h"

#include "android/ExternalTextureManagerAndroid.h"
#include "android/ExternalStreamManagerAndroid.h"
#include "android/VirtualMachineEnv.h"

#include <android/api-level.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/compiler.h>
#include <utils/Log.h>

#include <sys/system_properties.h>

#include <assert.h>

#include <jni.h>


// We require filament to be built with a API 19 toolchain, before that, OpenGLES 3.0 didn't exist
// Actually, OpenGL ES 3.0 was added to API 18, but API 19 is the better target and
// the minimum for Jetpack at the time of this comment.
#if __ANDROID_API__ < 19
#   error "__ANDROID_API__ must be at least 19"
#endif

using namespace utils;

namespace filament {
using namespace backend;

// The Android NDK doesn't exposes extensions, fake it with eglGetProcAddress
namespace glext {

extern PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR;
extern PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR;
extern PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR;
extern PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
extern PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;

UTILS_PRIVATE PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC eglGetNativeClientBufferANDROID = {};
UTILS_PRIVATE PFNEGLPRESENTATIONTIMEANDROIDPROC eglPresentationTimeANDROID = {};
UTILS_PRIVATE PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC eglGetCompositorTimingSupportedANDROID = {};
UTILS_PRIVATE PFNEGLGETCOMPOSITORTIMINGANDROIDPROC eglGetCompositorTimingANDROID = {};
UTILS_PRIVATE PFNEGLGETNEXTFRAMEIDANDROIDPROC eglGetNextFrameIdANDROID = {};
UTILS_PRIVATE PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC eglGetFrameTimestampSupportedANDROID = {};
UTILS_PRIVATE PFNEGLGETFRAMETIMESTAMPSANDROIDPROC eglGetFrameTimestampsANDROID = {};
}
using namespace glext;

using EGLStream = Platform::Stream;

// ---------------------------------------------------------------------------------------------

PlatformEGLAndroid::PlatformEGLAndroid() noexcept
        : PlatformEGL(),
          mExternalStreamManager(ExternalStreamManagerAndroid::get()),
          mExternalTextureManager(ExternalTextureManagerAndroid::get()) {

    char scratch[PROP_VALUE_MAX + 1];
    int length = __system_property_get("ro.build.version.release", scratch);
    int androidVersion = length >= 0 ? atoi(scratch) : 1;
    if (!androidVersion) {
        mOSVersion = 1000; // if androidVersion is 0, it means "future"
    } else {
        length = __system_property_get("ro.build.version.sdk", scratch);
        mOSVersion = length >= 0 ? atoi(scratch) : 1;
    }
}

backend::Driver* PlatformEGLAndroid::createDriver(void* sharedContext) noexcept {
    backend::Driver* driver = PlatformEGL::createDriver(sharedContext);
    auto extensions = GLUtils::split(eglQueryString(mEGLDisplay, EGL_EXTENSIONS));

    eglGetNativeClientBufferANDROID = (PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC) eglGetProcAddress("eglGetNativeClientBufferANDROID");

    if (extensions.has("EGL_ANDROID_presentation_time")) {
        eglPresentationTimeANDROID = (PFNEGLPRESENTATIONTIMEANDROIDPROC)eglGetProcAddress(
                "eglPresentationTimeANDROID");
    }

    if (extensions.has("EGL_ANDROID_get_frame_timestamps")) {
        eglGetCompositorTimingSupportedANDROID = (PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC)eglGetProcAddress(
                "eglGetCompositorTimingSupportedANDROID");
        eglGetCompositorTimingANDROID = (PFNEGLGETCOMPOSITORTIMINGANDROIDPROC)eglGetProcAddress(
                "eglGetCompositorTimingANDROID");
        eglGetNextFrameIdANDROID = (PFNEGLGETNEXTFRAMEIDANDROIDPROC)eglGetProcAddress(
                "eglGetNextFrameIdANDROID");
        eglGetFrameTimestampSupportedANDROID = (PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC)eglGetProcAddress(
                "eglGetFrameTimestampSupportedANDROID");
        eglGetFrameTimestampsANDROID = (PFNEGLGETFRAMETIMESTAMPSANDROIDPROC)eglGetProcAddress(
                "eglGetFrameTimestampsANDROID");
    }

    return driver;
}

void PlatformEGLAndroid::setPresentationTime(int64_t presentationTimeInNanosecond) noexcept {
    if (mCurrentDrawSurface != EGL_NO_SURFACE) {
        if (eglPresentationTimeANDROID) {
            eglPresentationTimeANDROID(
                    mEGLDisplay,
                    mCurrentDrawSurface,
                    presentationTimeInNanosecond);
        }
    }
}

Platform::Stream* PlatformEGLAndroid::createStream(void* nativeStream) noexcept {
    return mExternalStreamManager.acquire(static_cast<jobject>(nativeStream));
}

void PlatformEGLAndroid::destroyStream(Platform::Stream* stream) noexcept {
    mExternalStreamManager.release(stream);
}

void PlatformEGLAndroid::attach(Stream* stream, intptr_t tname) noexcept {
    mExternalStreamManager.attach(stream, tname);
}

void PlatformEGLAndroid::detach(Stream* stream) noexcept {
    mExternalStreamManager.detach(stream);
}

void PlatformEGLAndroid::updateTexImage(Stream* stream, int64_t* timestamp) noexcept {
    mExternalStreamManager.updateTexImage(stream, timestamp);
}

Platform::ExternalTexture* PlatformEGLAndroid::createExternalTextureStorage() noexcept {
    return mExternalTextureManager.create();
}

void PlatformEGLAndroid::reallocateExternalStorage(
        Platform::ExternalTexture* externalTexture,
        uint32_t w, uint32_t h, backend::TextureFormat format) noexcept {
    if (externalTexture) {
        if ((EGLImageKHR)externalTexture->image != EGL_NO_IMAGE_KHR) {
            eglDestroyImageKHR(mEGLDisplay, (EGLImageKHR)externalTexture->image);
            externalTexture->image = (uintptr_t)EGL_NO_IMAGE_KHR;
        }

        mExternalTextureManager.reallocate(externalTexture, w, h, format,
                AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE);

        auto ets = (ExternalTextureManagerAndroid::ExternalTexture*)externalTexture;
        EGLClientBuffer clientBuffer;
        if (ets->hardwareBuffer) {
            clientBuffer = eglGetNativeClientBufferANDROID(ets->hardwareBuffer);
            if (UTILS_UNLIKELY(!clientBuffer)) {
                logEglError("eglGetNativeClientBufferANDROID");
                return;
            }
        } else if (ets->clientBuffer) {
            clientBuffer = (EGLClientBuffer)ets->clientBuffer;
        } else {
            // woops, reallocate failed.
            return;
        }

        const EGLint attr[] = { EGL_NONE };
        EGLImageKHR image = eglCreateImageKHR(mEGLDisplay,
                EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, attr);
        if (UTILS_UNLIKELY(!image)) {
            logEglError("eglCreateImageKHR");
        }
        ets->image = (uintptr_t)image;
    }
}

void PlatformEGLAndroid::destroyExternalTextureStorage(
        Platform::ExternalTexture* externalTexture) noexcept {
    if (externalTexture) {
        mExternalTextureManager.destroy(externalTexture);
        if ((EGLImageKHR)externalTexture->image != EGL_NO_IMAGE_KHR) {
            eglDestroyImageKHR(mEGLDisplay, (EGLImageKHR)externalTexture->image);
            externalTexture->image = (uintptr_t)EGL_NO_IMAGE_KHR;
        }
    }
}

int PlatformEGLAndroid::getOSVersion() const noexcept {
    return mOSVersion;
}

backend::AcquiredImage PlatformEGLAndroid::transformAcquiredImage(backend::AcquiredImage source) noexcept {
    void* const hwbuffer = source.image;
    const backend::StreamCallback userCallback = source.callback;
    void* const userData = source.userData;

    // Convert the AHardwareBuffer to EGLImage.
    EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID((const AHardwareBuffer*) hwbuffer);
    if (!clientBuffer) {
        slog.e << "Unable to get EGLClientBuffer from AHardwareBuffer." << io::endl;
        return {};
    }
    // Note that this cannot be used to stream protected video (for now) because we do not set EGL_PROTECTED_CONTENT_EXT.
    EGLint attrs[] = { EGL_NONE, EGL_NONE };
    EGLImageKHR eglImage = eglCreateImageKHR(mEGLDisplay, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, attrs);
    if (eglImage == EGL_NO_IMAGE_KHR) {
        slog.e << "eglCreateImageKHR returned no image." << io::endl;
        return {};
    }

    // Destroy the EGLImage before invoking the user's callback.
    struct Closure {
        void* image;
        backend::StreamCallback callback;
        void* userData;
        EGLDisplay display;
    };
    Closure* closure = new Closure();
    closure->callback = userCallback;
    closure->image = hwbuffer;
    closure->userData = userData;
    closure->display = mEGLDisplay;
    auto patchedCallback = [](void* image, void* userdata) {
        Closure* closure = (Closure*) userdata;
        if (eglDestroyImageKHR(closure->display, (EGLImageKHR) image) == EGL_FALSE) {
            slog.e << "eglDestroyImageKHR failed." << io::endl;
        }
        closure->callback(closure->image, closure->userData);
        delete closure;
    };

    return {eglImage, patchedCallback, closure};
}

// This must called when the library is loaded. We need this to get a reference to the global VM
void JNI_OnLoad(JavaVM* vm, void* reserved) {
    ::filament::VirtualMachineEnv::JNI_OnLoad(vm);
}

} // namespace filament

// ---------------------------------------------------------------------------------------------
