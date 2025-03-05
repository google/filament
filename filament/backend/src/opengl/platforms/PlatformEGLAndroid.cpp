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

#include <backend/AcquiredImage.h>
#include <backend/Platform.h>
#include <backend/platforms/PlatformEGL.h>
#include <backend/platforms/PlatformEGLAndroid.h>

#include <private/backend/VirtualMachineEnv.h>

#include "opengl/GLUtils.h"
#include "ExternalStreamManagerAndroid.h"

#include <android/api-level.h>
#include <android/native_window.h>
#include <android/hardware_buffer.h>

#include <utils/android/PerformanceHintManager.h>

#include <utils/compiler.h>
#include <utils/ostream.h>
#include <utils/Panic.h>
#include <utils/Log.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <sys/system_properties.h>

#include <jni.h>

#include <chrono>
#include <new>
#include <string_view>

#include <dlfcn.h>
#include <unistd.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// We require filament to be built with an API 19 toolchain, before that, OpenGLES 3.0 didn't exist
// Actually, OpenGL ES 3.0 was added to API 18, but API 19 is the better target and
// the minimum for Jetpack at the time of this comment.
#if __ANDROID_API__ < 19
#   error "__ANDROID_API__ must be at least 19"
#endif

using namespace utils;

namespace filament::backend {
using namespace backend;

// The Android NDK doesn't expose extensions, fake it with eglGetProcAddress
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

// ---------------------------------------------------------------------------------------------

PlatformEGLAndroid::InitializeJvmForPerformanceManagerIfNeeded::InitializeJvmForPerformanceManagerIfNeeded() {
    // PerformanceHintManager() needs the calling thread to be a Java thread; so we need
    // to attach this thread to the JVM before we initialize PerformanceHintManager.
    // This should be done in PerformanceHintManager(), but libutils doesn't have access to
    // VirtualMachineEnv.
    if (PerformanceHintManager::isSupported()) {
        (void)VirtualMachineEnv::get().getEnvironment();
    }
}

// ---------------------------------------------------------------------------------------------

PlatformEGLAndroid::PlatformEGLAndroid() noexcept
        : PlatformEGL(),
          mExternalStreamManager(ExternalStreamManagerAndroid::create()),
          mInitializeJvmForPerformanceManagerIfNeeded(),
          mPerformanceHintManager() {
    mOSVersion = android_get_device_api_level();
    if (mOSVersion < 0) {
        mOSVersion = __ANDROID_API_FUTURE__;
    }

    mNativeWindowLib = dlopen("libnativewindow.so", RTLD_LOCAL | RTLD_NOW);
    if (mNativeWindowLib) {
        ANativeWindow_getBuffersDefaultDataSpace =
                (int32_t(*)(ANativeWindow*))dlsym(mNativeWindowLib,
                        "ANativeWindow_getBuffersDefaultDataSpace");
    }
}

PlatformEGLAndroid::~PlatformEGLAndroid() noexcept {
    if (mNativeWindowLib) {
        dlclose(mNativeWindowLib);
    }
}

void PlatformEGLAndroid::terminate() noexcept {
    ExternalStreamManagerAndroid::destroy(&mExternalStreamManager);
    PlatformEGL::terminate();
}

static constexpr const std::string_view kNativeWindowInvalidMsg =
        "ANativeWindow is invalid. It probably has been destroyed. EGL surface = ";

bool PlatformEGLAndroid::makeCurrent(ContextType type,
        SwapChain* drawSwapChain,
        SwapChain* readSwapChain) noexcept {

    // fast & safe path
    if (UTILS_LIKELY(!mAssertNativeWindowIsValid)) {
        return PlatformEGL::makeCurrent(type, drawSwapChain, readSwapChain);
    }

    SwapChainEGL const* const dsc = static_cast<SwapChainEGL const*>(drawSwapChain);
    if (ANativeWindow_getBuffersDefaultDataSpace) {
        // anw can be nullptr if we're using a pbuffer surface
        if (UTILS_LIKELY(dsc->nativeWindow)) {
            // this a proxy of is_valid()
            auto result = ANativeWindow_getBuffersDefaultDataSpace(dsc->nativeWindow);
            FILAMENT_CHECK_POSTCONDITION(result >= 0) << kNativeWindowInvalidMsg << dsc->sur;
        }
    } else {
        // If we don't have ANativeWindow_getBuffersDefaultDataSpace, we revert to using the
        // private query() call.
        // Shadow version if the real ANativeWindow, so we can access the query() hook. Query
        // has existed since forever, probably Android 1.0.
        struct NativeWindow {
            // is valid query enum value
            enum { IS_VALID = 17 };
            uint64_t pad[18];
            int (* query)(ANativeWindow const*, int, int*);
        } const* pWindow = reinterpret_cast<NativeWindow const*>(dsc->nativeWindow);
        int isValid = 0;
        if (UTILS_LIKELY(pWindow->query)) { // just in case it's nullptr
            int const err = pWindow->query(dsc->nativeWindow, NativeWindow::IS_VALID, &isValid);
            if (UTILS_LIKELY(err >= 0)) { // in case the IS_VALID enum is not recognized
                // query call succeeded
                FILAMENT_CHECK_POSTCONDITION(isValid) << kNativeWindowInvalidMsg << dsc->sur;
            }
        }
    }
    return PlatformEGL::makeCurrent(type, drawSwapChain, readSwapChain);
}

void PlatformEGLAndroid::beginFrame(
        int64_t monotonic_clock_ns,
        int64_t refreshIntervalNs,
        uint32_t frameId) noexcept {
    if (mPerformanceHintSession.isValid()) {
        if (refreshIntervalNs <= 0) {
            // we're not provided with a target time, assume 16.67ms
            refreshIntervalNs = 16'666'667;
        }
        mStartTimeOfActualWork = clock::time_point(std::chrono::nanoseconds(monotonic_clock_ns));
        mPerformanceHintSession.updateTargetWorkDuration(refreshIntervalNs);
    }
    PlatformEGL::beginFrame(monotonic_clock_ns, refreshIntervalNs, frameId);
}

void backend::PlatformEGLAndroid::preCommit() noexcept {
    if (mPerformanceHintSession.isValid()) {
        auto const actualWorkDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                clock::now() - mStartTimeOfActualWork);
        mPerformanceHintSession.reportActualWorkDuration(actualWorkDuration.count());
    }
    PlatformEGL::preCommit();
}

Driver* PlatformEGLAndroid::createDriver(void* sharedContext,
        const Platform::DriverConfig& driverConfig) noexcept {

    // the refresh rate default value doesn't matter, we change it later
    int32_t const tid = gettid();
    mPerformanceHintSession = PerformanceHintManager::Session{
            mPerformanceHintManager, &tid, 1, 16'666'667 };

    Driver* driver = PlatformEGL::createDriver(sharedContext, driverConfig);
    auto extensions = GLUtils::split(eglQueryString(mEGLDisplay, EGL_EXTENSIONS));

    eglGetNativeClientBufferANDROID = (PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC) eglGetProcAddress(
            "eglGetNativeClientBufferANDROID");

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

    mAssertNativeWindowIsValid = driverConfig.assertNativeWindowIsValid;

    return driver;
}

PlatformEGLAndroid::ExternalImageEGLAndroid::~ExternalImageEGLAndroid() = default;

Platform::ExternalImageHandle PlatformEGLAndroid::createExternalImage(AHardwareBuffer const* buffer, bool sRGB) noexcept {
    auto* const p = new(std::nothrow) ExternalImageEGLAndroid;
    p->aHardwareBuffer = const_cast<AHardwareBuffer*>(buffer);
    p->sRGB = sRGB;
    return ExternalImageHandle{ p };
}

bool PlatformEGLAndroid::setExternalImage(ExternalImageHandleRef externalImage,
        UTILS_UNUSED_IN_RELEASE ExternalTexture* texture) noexcept {
    auto const* const eglExternalImage = static_cast<ExternalImageEGLAndroid const*>(externalImage.get());
    if (eglExternalImage->aHardwareBuffer) {
        // TODO: implement PlatformEGLAndroid::setExternalImage w/ AHardwareBuffer
        return true;
    }
    // not a AHardwareBuffer, fallback to the inherited version
    return PlatformEGL::setExternalImage(externalImage, texture);
}

void PlatformEGLAndroid::setPresentationTime(int64_t presentationTimeInNanosecond) noexcept {
    EGLSurface currentDrawSurface = eglGetCurrentSurface(EGL_DRAW);
    if (currentDrawSurface != EGL_NO_SURFACE) {
        if (eglPresentationTimeANDROID) {
            eglPresentationTimeANDROID(
                    mEGLDisplay,
                    currentDrawSurface,
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

void PlatformEGLAndroid::getTransformMatrix(Stream* stream, math::mat3f* uvTransform) noexcept {
    mExternalStreamManager.getTransformMatrix(stream, uvTransform);
}

int PlatformEGLAndroid::getOSVersion() const noexcept {
    return mOSVersion;
}

AcquiredImage PlatformEGLAndroid::transformAcquiredImage(AcquiredImage source) noexcept {
    // Convert the AHardwareBuffer to EGLImage.
    AHardwareBuffer const* const pHardwareBuffer = (const AHardwareBuffer*)source.image;

    EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(pHardwareBuffer);
    if (!clientBuffer) {
        slog.e << "Unable to get EGLClientBuffer from AHardwareBuffer." << io::endl;
        return {};
    }

    PlatformEGL::Config attributes;

    if (__builtin_available(android 26, *)) {
        AHardwareBuffer_Desc desc;
        AHardwareBuffer_describe(pHardwareBuffer, &desc);
        bool const isProtectedContent =
                desc.usage & AHardwareBuffer_UsageFlags::AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
        if (isProtectedContent) {
            attributes[EGL_PROTECTED_CONTENT_EXT] = EGL_TRUE;
        }
    }

    EGLImageKHR eglImage = eglCreateImageKHR(mEGLDisplay,
            EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, attributes.data());
    if (eglImage == EGL_NO_IMAGE_KHR) {
        slog.e << "eglCreateImageKHR returned no image." << io::endl;
        return {};
    }

    // Destroy the EGLImage before invoking the user's callback.
    struct Closure {
        Closure(AcquiredImage const& acquiredImage, EGLDisplay display)
                : acquiredImage(acquiredImage), display(display) {}
        AcquiredImage acquiredImage;
        EGLDisplay display;
    };
    Closure* closure = new(std::nothrow) Closure(source, mEGLDisplay);
    auto patchedCallback = [](void* image, void* userdata) {
        Closure* closure = (Closure*)userdata;
        if (eglDestroyImageKHR(closure->display, (EGLImageKHR) image) == EGL_FALSE) {
            slog.e << "eglDestroyImageKHR failed." << io::endl;
        }
        closure->acquiredImage.callback(closure->acquiredImage.image, closure->acquiredImage.userData);
        delete closure;
    };

    return { eglImage, patchedCallback, closure, source.handler };
}

} // namespace filament::backend

// ---------------------------------------------------------------------------------------------
