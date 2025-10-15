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

#include <backend/platforms/PlatformEGLAndroid.h>

#include "opengl/GLUtils.h"

#include <backend/AcquiredImage.h>
#include <backend/DriverEnums.h>
#include <backend/Platform.h>
#include <backend/platforms/OpenGLPlatform.h>
#include <backend/platforms/PlatformEGL.h>

#include <private/backend/BackendUtilsAndroid.h>
#include <private/backend/VirtualMachineEnv.h>

#include "AndroidNativeWindow.h"
#include "ExternalStreamManagerAndroid.h"

#include <android/api-level.h>
#include <android/hardware_buffer.h>

#include <utils/android/PerformanceHintManager.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/ostream.h>

#include <math/mat3.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <sys/system_properties.h>

#include <jni.h>

#include <array>
#include <chrono>
#include <new>
#include <string_view>

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

UTILS_PRIVATE PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC eglGetNativeClientBufferANDROID{}; // NOLINT(*-use-internal-linkage)
UTILS_PRIVATE PFNEGLPRESENTATIONTIMEANDROIDPROC eglPresentationTimeANDROID{}; // NOLINT(*-use-internal-linkage)
UTILS_PRIVATE PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC eglGetCompositorTimingSupportedANDROID{}; // NOLINT(*-use-internal-linkage)
UTILS_PRIVATE PFNEGLGETCOMPOSITORTIMINGANDROIDPROC eglGetCompositorTimingANDROID{}; // NOLINT(*-use-internal-linkage)
UTILS_PRIVATE PFNEGLGETNEXTFRAMEIDANDROIDPROC eglGetNextFrameIdANDROID{}; // NOLINT(*-use-internal-linkage)
UTILS_PRIVATE PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC eglGetFrameTimestampSupportedANDROID{}; // NOLINT(*-use-internal-linkage)
UTILS_PRIVATE PFNEGLGETFRAMETIMESTAMPSANDROIDPROC eglGetFrameTimestampsANDROID{}; // NOLINT(*-use-internal-linkage)
UTILS_PRIVATE PFNEGLDUPNATIVEFENCEFDANDROIDPROC eglDupNativeFenceFDANDROID{}; // NOLINT(*-use-internal-linkage)
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
        : mExternalStreamManager(ExternalStreamManagerAndroid::create()) {
    mOSVersion = android_get_device_api_level();
    if (mOSVersion < 0) {
        mOSVersion = __ANDROID_API_FUTURE__;
    }
}

PlatformEGLAndroid::~PlatformEGLAndroid() noexcept {
}

void PlatformEGLAndroid::terminate() noexcept {
    ExternalStreamManagerAndroid::destroy(&mExternalStreamManager);
    PlatformEGL::terminate();
}

static constexpr std::string_view kNativeWindowInvalidMsg =
        "ANativeWindow is invalid. It probably has been destroyed. EGL surface = ";

bool PlatformEGLAndroid::makeCurrent(ContextType const type,
        SwapChain* drawSwapChain,
        SwapChain* readSwapChain) {

    // fast & safe path
    if (UTILS_LIKELY(!mAssertNativeWindowIsValid)) {
        return PlatformEGL::makeCurrent(type, drawSwapChain, readSwapChain);
    }

    SwapChainEGL const* const dsc = static_cast<SwapChainEGL const*>(drawSwapChain);
    // anw can be nullptr if we're using a pbuffer surface
    if (dsc->nativeWindow) {
        auto [err, valid] = NativeWindow::isValid(dsc->nativeWindow);
        FILAMENT_CHECK_POSTCONDITION(!err && valid) << kNativeWindowInvalidMsg << dsc->sur;
    }
    return PlatformEGL::makeCurrent(type, drawSwapChain, readSwapChain);
}

void PlatformEGLAndroid::beginFrame(
        int64_t const monotonic_clock_ns,
        int64_t refreshIntervalNs,
        uint32_t const frameId) noexcept {
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

void PlatformEGLAndroid::preCommit() noexcept {
    if (mPerformanceHintSession.isValid()) {
        auto const actualWorkDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                clock::now() - mStartTimeOfActualWork);
        mPerformanceHintSession.reportActualWorkDuration(actualWorkDuration.count());
    }
    PlatformEGL::preCommit();
}

Driver* PlatformEGLAndroid::createDriver(void* sharedContext,
        const DriverConfig& driverConfig) {

    // the refresh rate default value doesn't matter, we change it later
    int32_t const tid = gettid();
    mPerformanceHintSession = PerformanceHintManager::Session{
            mPerformanceHintManager, &tid, 1, 16'666'667 };

    Driver* driver = PlatformEGL::createDriver(sharedContext, driverConfig);
    auto const extensions = GLUtils::split(eglQueryString(getEglDisplay(), EGL_EXTENSIONS));

    ext.egl.ANDROID_presentation_time = extensions.has("EGL_ANDROID_presentation_time");
    ext.egl.ANDROID_get_frame_timestamps = extensions.has("EGL_ANDROID_get_frame_timestamps");
    ext.egl.ANDROID_native_fence_sync = extensions.has("EGL_ANDROID_native_fence_sync");

    eglGetNativeClientBufferANDROID =
            PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC(eglGetProcAddress(
                    "eglGetNativeClientBufferANDROID"));

    if (ext.egl.ANDROID_presentation_time) {
        eglGetNativeClientBufferANDROID =
                PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC(eglGetProcAddress(
                        "eglGetNativeClientBufferANDROID"));
    }

    if (ext.egl.ANDROID_get_frame_timestamps) {
        eglGetCompositorTimingSupportedANDROID =
                PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC(eglGetProcAddress(
                        "eglGetCompositorTimingSupportedANDROID"));
        eglGetCompositorTimingANDROID =
                PFNEGLGETCOMPOSITORTIMINGANDROIDPROC(eglGetProcAddress(
                        "eglGetCompositorTimingANDROID"));
        eglGetNextFrameIdANDROID =
                PFNEGLGETNEXTFRAMEIDANDROIDPROC(eglGetProcAddress(
                        "eglGetNextFrameIdANDROID"));
        eglGetFrameTimestampSupportedANDROID =
                PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC(eglGetProcAddress(
                        "eglGetFrameTimestampSupportedANDROID"));
        eglGetFrameTimestampsANDROID =
                PFNEGLGETFRAMETIMESTAMPSANDROIDPROC(eglGetProcAddress(
                        "eglGetFrameTimestampsANDROID"));
    }

    if (ext.egl.ANDROID_native_fence_sync) {
        eglDupNativeFenceFDANDROID =
                PFNEGLDUPNATIVEFENCEFDANDROIDPROC(eglGetProcAddress(
                        "eglDupNativeFenceFDANDROID"));
    } else {
        LOG(ERROR) << "EGL_ANDROID_native_fence_sync not supported!";
    }

    mAssertNativeWindowIsValid = driverConfig.assertNativeWindowIsValid;

    return driver;
}

PlatformEGLAndroid::ExternalImageEGLAndroid::~ExternalImageEGLAndroid() {
    if (__builtin_available(android 26, *)) {
        if (aHardwareBuffer) {
            AHardwareBuffer_release(aHardwareBuffer);
        }
    }
}

bool PlatformEGLAndroid::isCompositorTimingSupported() const noexcept {
    return true;
}

bool PlatformEGLAndroid::queryCompositorTiming(SwapChain const* swapchain,
        CompositorTiming* outCompositorTiming) const noexcept {
    if (!swapchain) {
        return false;
    }

    if (UTILS_LIKELY(ext.egl.ANDROID_get_frame_timestamps)) {
        EGLSurface sur = static_cast<SwapChainEGL const *>(swapchain)->sur;
        if (sur == EGL_NO_SURFACE) {
            return false;
        }

        std::array<EGLnsecsANDROID, 3> values;
        constexpr std::array<EGLint, 3> names{
            EGL_COMPOSITE_DEADLINE_ANDROID,
            EGL_COMPOSITE_INTERVAL_ANDROID,
            EGL_COMPOSITE_TO_PRESENT_LATENCY_ANDROID
        };
        EGLBoolean const success = eglGetCompositorTimingANDROID(getEglDisplay(), sur,
                names.size(), names.data(), values.data());
        if (!success) {
            return false;
        }
        outCompositorTiming->compositeDeadline = values[0];
        outCompositorTiming->compositeInterval = values[1];
        outCompositorTiming->compositeToPresentLatency = values[2];
        return true;
    }

    // fallback to private APIs
    auto const anw = static_cast<SwapChainEGL const *>(swapchain)->nativeWindow;
    int const status = NativeWindow::getCompositorTiming(anw,
            &outCompositorTiming->compositeDeadline,
            &outCompositorTiming->compositeInterval,
            &outCompositorTiming->compositeToPresentLatency);
    if (status == 0) {
        return true;
    }

    return PlatformEGL::queryCompositorTiming(swapchain, outCompositorTiming);
}

Platform::SwapChain* PlatformEGLAndroid::createSwapChain(void* nativeWindow, uint64_t const flags) {
    auto* const sc = new(std::nothrow) SwapChainEGLAndroid(*this, nativeWindow, flags);
    return sc;
}

Platform::SwapChain* PlatformEGLAndroid::createSwapChain(
        uint32_t const width, uint32_t const height, uint64_t const flags) {
    auto* const sc = new(std::nothrow) SwapChainEGLAndroid(*this, width, height, flags);
    return sc;
}

void PlatformEGLAndroid::destroySwapChain(SwapChain* swapChain) noexcept {
    if (swapChain) {
        SwapChainEGLAndroid* const sc = static_cast<SwapChainEGLAndroid*>(swapChain);
        sc->terminate(*this);
        delete sc;
    }
}

Platform::ExternalImageHandle PlatformEGLAndroid::createExternalImage(
        AHardwareBuffer const* buffer, bool const sRGB) noexcept {
    if (__builtin_available(android 26, *)) {
        auto* const p = new (std::nothrow) ExternalImageEGLAndroid;
        auto const hardwareBuffer = const_cast<AHardwareBuffer*>(buffer);
        AHardwareBuffer_acquire(hardwareBuffer);
        p->aHardwareBuffer = hardwareBuffer;
        p->sRGB = sRGB;
        AHardwareBuffer_Desc hardwareBufferDescription = {};
        AHardwareBuffer_describe(hardwareBuffer, &hardwareBufferDescription);
        p->height = hardwareBufferDescription.height;
        p->width = hardwareBufferDescription.width;
        auto const textureFormat = mapToFilamentFormat(hardwareBufferDescription.format, sRGB);
        p->format = textureFormat;
        p->usage = mapToFilamentUsage(hardwareBufferDescription.usage, textureFormat);
        return ExternalImageHandle{ p };
    }

    return ExternalImageHandle{};
}

PlatformEGLAndroid::ExternalImageDescAndroid PlatformEGLAndroid::getExternalImageDesc(
        ExternalImageHandle externalImage) noexcept {
    auto const* const eglExternalImage =
            static_cast<ExternalImageEGLAndroid const*>(externalImage.get());
    ExternalImageDescAndroid metadata = {};
    if (!eglExternalImage) {
        return metadata;
    }
    metadata.height = eglExternalImage->height;
    metadata.width = eglExternalImage->width;
    metadata.format = eglExternalImage->format;
    metadata.usage = eglExternalImage->usage;
    return metadata;
}

bool PlatformEGLAndroid::setExternalImage(ExternalImageHandleRef externalImage,
        UTILS_UNUSED_IN_RELEASE ExternalTexture* texture) noexcept {
    auto const* const eglExternalImage =
            static_cast<ExternalImageEGLAndroid const*>(externalImage.get());
    if (eglExternalImage->aHardwareBuffer) {
        return setImage(eglExternalImage, texture);
    }
    // not a AHardwareBuffer, fallback to the inherited version
    return PlatformEGL::setExternalImage(externalImage, texture);
}

OpenGLPlatform::ExternalTexture* PlatformEGLAndroid::createExternalImageTexture() noexcept {
    ExternalTextureAndroid* outTexture = new (std::nothrow) ExternalTextureAndroid{};
    glGenTextures(1, &outTexture->id);
    return outTexture;
}

void PlatformEGLAndroid::destroyExternalImageTexture(ExternalTexture* texture) noexcept {
    ExternalTextureAndroid const* outTexture = static_cast<ExternalTextureAndroid*>(texture);
    glDeleteTextures(1, &texture->id);

    if (outTexture->eglImage != EGL_NO_IMAGE) {
        eglDestroyImageKHR(eglGetCurrentDisplay(), outTexture->eglImage);
    }
    delete outTexture;
}

bool PlatformEGLAndroid::setImage(ExternalImageEGLAndroid const* eglExternalImage,
        UTILS_UNUSED_IN_RELEASE ExternalTexture* texture) noexcept {
    AHardwareBuffer const* hardwareBuffer = eglExternalImage->aHardwareBuffer;

    // Get the EGL client buffer from AHardwareBuffer
    EGLClientBuffer const clientBuffer = eglGetNativeClientBufferANDROID(hardwareBuffer);
    EGLint imageAttrs[] = {
        EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
        EGL_NONE, EGL_NONE,  // Reserve space
        EGL_NONE, EGL_NONE,  // Reserve space
        EGL_NONE             // Ensure the list always ends with EGL_NONE
    };
    int attrIndex = 2;
    if (eglExternalImage->sRGB) {
        imageAttrs[attrIndex++] = EGL_GL_COLORSPACE;
        imageAttrs[attrIndex++] = EGL_GL_COLORSPACE_SRGB;
    }

    if (static_cast<bool>(eglExternalImage->usage & TextureUsage::PROTECTED)) {
        imageAttrs[attrIndex++] = EGL_PROTECTED_CONTENT_EXT;
        imageAttrs[attrIndex++] = EGL_TRUE;
    }
    // Create an EGLImage from the client buffer
    EGLImageKHR const eglImage = eglCreateImageKHR(eglGetCurrentDisplay(), EGL_NO_CONTEXT,
            EGL_NATIVE_BUFFER_ANDROID, clientBuffer, imageAttrs);
    if (eglImage == EGL_NO_IMAGE_KHR) {
        // Handle error
        LOG(ERROR) << "Failed to create EGL image";
        glDeleteTextures(1, &texture->id);
        return false;
    }

    // Create and bind the OpenGL texture
    GLint prevActiveTexture, prevTexture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &prevActiveTexture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(texture->target, texture->id);
    GLenum error = glGetError();
    if (UTILS_UNLIKELY(error != GL_NO_ERROR)) {
        LOG(ERROR) << "Error after glBindTexture: " << error;
        glDeleteTextures(1, &texture->id);
        eglDestroyImageKHR(eglGetCurrentDisplay(), eglImage);
        glActiveTexture(prevActiveTexture);
        glBindTexture(GL_TEXTURE_2D, prevTexture);
        return false;
    }
    glEGLImageTargetTexture2DOES(texture->target, static_cast<GLeglImageOES>(eglImage));
    error = glGetError();
    if (UTILS_UNLIKELY(error != GL_NO_ERROR)) {
        LOG(ERROR) << "Error after glEGLImageTargetTexture2DOES: " << error;
        glDeleteTextures(1, &texture->id);
        eglDestroyImageKHR(eglGetCurrentDisplay(), eglImage);
        glActiveTexture(prevActiveTexture);
        glBindTexture(GL_TEXTURE_2D, prevTexture);
        return false;
    }
    ExternalTextureAndroid* outTexture = static_cast<ExternalTextureAndroid*>(texture);

    // Make sure to destroy the previous binded image, to avoid leaking memory
    if (outTexture->eglImage != EGL_NO_IMAGE) {
        eglDestroyImageKHR(eglGetCurrentDisplay(), outTexture->eglImage);
    }

    outTexture->eglImage = eglImage;

    glActiveTexture(prevActiveTexture);
    glBindTexture(GL_TEXTURE_2D, prevTexture);
    return true;
}

void PlatformEGLAndroid::setPresentationTime(int64_t const presentationTimeInNanosecond) noexcept {
    EGLSurface const currentDrawSurface = eglGetCurrentSurface(EGL_DRAW);
    if (currentDrawSurface != EGL_NO_SURFACE) {
        if (UTILS_UNLIKELY(ext.egl.ANDROID_presentation_time)) {
            eglPresentationTimeANDROID(
                    getEglDisplay(),
                    currentDrawSurface,
                    presentationTimeInNanosecond);
        }
    }
}

Platform::Stream* PlatformEGLAndroid::createStream(void* nativeStream) noexcept {
    return mExternalStreamManager.acquire(static_cast<jobject>(nativeStream));
}

void PlatformEGLAndroid::destroyStream(Stream* stream) noexcept {
    mExternalStreamManager.release(stream);
}

Platform::Sync* PlatformEGLAndroid::createSync() noexcept {
    auto const sync = eglCreateSyncKHR(getEglDisplay(), EGL_SYNC_NATIVE_FENCE_ANDROID, nullptr);
    return new(std::nothrow) SyncEGLAndroid{ .sync = sync };
}

bool PlatformEGLAndroid::convertSyncToFd(Sync* sync, int* fd) noexcept {
    assert_invariant(sync && fd);

    if (UTILS_UNLIKELY(!ext.egl.ANDROID_native_fence_sync)) {
        return false;
    }

    SyncEGLAndroid const& eglSync = static_cast<SyncEGLAndroid&>(*sync);
    *fd = eglDupNativeFenceFDANDROID(getEglDisplay(), eglSync.sync);
    // In the case where there was no native FD, -1 is returned. Return false
    // to indicate there was an error in this case.
    if (*fd == EGL_NO_NATIVE_FENCE_FD_ANDROID) {
        LOG(ERROR) << "Failed to convert sync to fd: " << eglGetError();
        return false;
    }
    return true;
}

void PlatformEGLAndroid::destroySync(Sync* sync) noexcept {
    assert_invariant(sync);
    SyncEGLAndroid const& eglSync = static_cast<SyncEGLAndroid&>(*sync);
    eglDestroySyncKHR(getEglDisplay(), eglSync.sync);
    delete sync;
}

void PlatformEGLAndroid::attach(Stream* stream, intptr_t const tname) noexcept {
    mExternalStreamManager.attach(stream, tname);
}

void PlatformEGLAndroid::detach(Stream* stream) noexcept {
    mExternalStreamManager.detach(stream);
}

void PlatformEGLAndroid::updateTexImage(Stream* stream, int64_t* timestamp) noexcept {
    mExternalStreamManager.updateTexImage(stream, timestamp);
}

math::mat3f PlatformEGLAndroid::getTransformMatrix(Stream* stream) noexcept {
    return mExternalStreamManager.getTransformMatrix(stream);
}

int PlatformEGLAndroid::getOSVersion() const noexcept {
    return mOSVersion;
}

AcquiredImage PlatformEGLAndroid::transformAcquiredImage(AcquiredImage const source) noexcept {
    // Convert the AHardwareBuffer to EGLImage.
    AHardwareBuffer const* const pHardwareBuffer = (const AHardwareBuffer*)source.image;

    EGLClientBuffer const clientBuffer = eglGetNativeClientBufferANDROID(pHardwareBuffer);
    if (!clientBuffer) {
        LOG(ERROR) << "Unable to get EGLClientBuffer from AHardwareBuffer.";
        return {};
    }

    Config attributes;

    if (__builtin_available(android 26, *)) {
        AHardwareBuffer_Desc desc;
        AHardwareBuffer_describe(pHardwareBuffer, &desc);
        bool const isProtectedContent =
                desc.usage & AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
        if (isProtectedContent) {
            attributes[EGL_PROTECTED_CONTENT_EXT] = EGL_TRUE;
        }
    }

    EGLDisplay const dpy = getEglDisplay();
    EGLImageKHR const eglImage = eglCreateImageKHR(dpy,
            EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, attributes.data());
    if (eglImage == EGL_NO_IMAGE_KHR) {
        LOG(ERROR) << "eglCreateImageKHR returned no image.";
        return {};
    }

    // Destroy the EGLImage before invoking the user's callback.
    struct Closure {
        Closure(AcquiredImage const& acquiredImage, EGLDisplay const display)
                : acquiredImage(acquiredImage), display(display) {}
        AcquiredImage acquiredImage;
        EGLDisplay display;
    };
    Closure* const closure = new(std::nothrow) Closure(source, dpy);
    auto patchedCallback = [](void* image, void* userdata) {
        Closure const* const closure = static_cast<Closure*>(userdata);
        if (eglDestroyImageKHR(closure->display, EGLImageKHR(image)) == EGL_FALSE) {
            LOG(ERROR) << "eglDestroyImageKHR failed.";
        }
        closure->acquiredImage.callback(closure->acquiredImage.image, closure->acquiredImage.userData);
        delete closure;
    };

    return { eglImage, patchedCallback, closure, source.handler };
}

// ---------------------------------------------------------------------------------------------
// PlatformEGLAndroid::SwapChainEGLAndroid

PlatformEGLAndroid::SwapChainEGLAndroid::SwapChainEGLAndroid(PlatformEGLAndroid const& platform,
        void* nativeWindow, uint64_t const flags)
    : SwapChainEGL(platform, nativeWindow, flags) {
    if (UTILS_LIKELY(platform.ext.egl.ANDROID_get_frame_timestamps)) {
        if (sur != EGL_NO_SURFACE) {
            // we ignore the result, it doesn't matter much if it fails
            eglSurfaceAttrib(platform.getEglDisplay(), sur, EGL_TIMESTAMPS_ANDROID, EGL_TRUE);
        }
    } else {
        NativeWindow::enableFrameTimestamps(EGLNativeWindowType(nativeWindow), true);
    }
}

PlatformEGLAndroid::SwapChainEGLAndroid::SwapChainEGLAndroid(PlatformEGLAndroid const& platform,
                uint32_t const width, uint32_t const height, uint64_t const flags)
    : SwapChainEGL(platform, width, height, flags) {
}

void PlatformEGLAndroid::SwapChainEGLAndroid::terminate(PlatformEGLAndroid& platform) {
    SwapChainEGL::terminate(platform);
}

} // namespace filament::backend
