/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "PlatformEGL.h"

#include "OpenGLDriver.h"

#include <jni.h>

#include <assert.h>

#include <string>
#include <unordered_set>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/compiler.h>
#include <utils/Log.h>

#include "android/ExternalTextureManagerAndroid.h"
#include "android/ExternalStreamManagerAndroid.h"
#include "android/VirtualMachineEnv.h"

#include "OpenGLContext.h"
#include "OpenGLDriverFactory.h"

#include <android/api-level.h>
#include <sys/system_properties.h>

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
UTILS_PRIVATE PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR = {};
UTILS_PRIVATE PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR = {};
UTILS_PRIVATE PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR = {};
UTILS_PRIVATE PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC eglGetNativeClientBufferANDROID = {};
UTILS_PRIVATE PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = {};
UTILS_PRIVATE PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = {};
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
// Utilities
// ---------------------------------------------------------------------------------------------

static void logEglError(const char* name) noexcept {
    const char* err;
    switch (eglGetError()) {
        case EGL_NOT_INITIALIZED:       err = "EGL_NOT_INITIALIZED";    break;
        case EGL_BAD_ACCESS:            err = "EGL_BAD_ACCESS";         break;
        case EGL_BAD_ALLOC:             err = "EGL_BAD_ALLOC";          break;
        case EGL_BAD_ATTRIBUTE:         err = "EGL_BAD_ATTRIBUTE";      break;
        case EGL_BAD_CONTEXT:           err = "EGL_BAD_CONTEXT";        break;
        case EGL_BAD_CONFIG:            err = "EGL_BAD_CONFIG";         break;
        case EGL_BAD_CURRENT_SURFACE:   err = "EGL_BAD_CURRENT_SURFACE";break;
        case EGL_BAD_DISPLAY:           err = "EGL_BAD_DISPLAY";        break;
        case EGL_BAD_SURFACE:           err = "EGL_BAD_SURFACE";        break;
        case EGL_BAD_MATCH:             err = "EGL_BAD_MATCH";          break;
        case EGL_BAD_PARAMETER:         err = "EGL_BAD_PARAMETER";      break;
        case EGL_BAD_NATIVE_PIXMAP:     err = "EGL_BAD_NATIVE_PIXMAP";  break;
        case EGL_BAD_NATIVE_WINDOW:     err = "EGL_BAD_NATIVE_WINDOW";  break;
        case EGL_CONTEXT_LOST:          err = "EGL_CONTEXT_LOST";       break;
        default:                        err = "unknown";                break;
    }
    slog.e << name << " failed with " << err << io::endl;
}

static void clearGlError() noexcept {
    // clear GL error that may have been set by previous calls
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        slog.w << "Ignoring pending GL error " << io::hex << error << io::endl;
    }
}

class unordered_string_set : public std::unordered_set<std::string> {
public:
    bool has(const char* str) {
        return find(std::string(str)) != end();
    }
};

static unordered_string_set split(const char* spacedList) {
    unordered_string_set set;
    const char* current = spacedList;
    const char* head = current;
    do {
        head = strchr(current, ' ');
        std::string s(current, head ? head - current : strlen(current));
        if (s.length()) {
            set.insert(std::move(s));
        }
        current = head + 1;
    } while (head);
    return set;
}

// ---------------------------------------------------------------------------------------------

PlatformEGL::PlatformEGL() noexcept
        : mExternalStreamManager(ExternalStreamManagerAndroid::get()),
          mExternalTextureManager(ExternalTextureManagerAndroid::get()) {
}

Driver* PlatformEGL::createDriver(void* sharedContext) noexcept {
    char scratch[PROP_VALUE_MAX + 1];
    int length = __system_property_get("ro.build.version.release", scratch);
    int androidVersion = length >= 0 ? atoi(scratch) : 1;
    if (!androidVersion) {
        mOSVersion = 1000; // if androidVersion is 0, it means "future"
    } else {
        length = __system_property_get("ro.build.version.sdk", scratch);
        mOSVersion = length >= 0 ? atoi(scratch) : 1;
    }

    mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(mEGLDisplay != EGL_NO_DISPLAY);

    EGLint major, minor;
    EGLBoolean initialized = eglInitialize(mEGLDisplay, &major, &minor);
    if (UTILS_UNLIKELY(!initialized)) {
        slog.e << "eglInitialize failed" << io::endl;
        return nullptr;
    }

    importGLESExtensionsEntryPoints();

    auto extensions = split(eglQueryString(mEGLDisplay, EGL_EXTENSIONS));

    eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) eglGetProcAddress("eglCreateSyncKHR");
    eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) eglGetProcAddress("eglDestroySyncKHR");
    eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC) eglGetProcAddress("eglClientWaitSyncKHR");

    eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
    eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");

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

    EGLint configsCount;
    EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_RED_SIZE,    8,
            EGL_GREEN_SIZE,  8,
            EGL_BLUE_SIZE,   8,
            EGL_ALPHA_SIZE,  0, // reserved to set ALPHA_SIZE below
            EGL_RECORDABLE_ANDROID, 1,
            EGL_NONE
    };

    EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE, EGL_NONE, // reserved for EGL_CONTEXT_OPENGL_NO_ERROR_KHR below
            EGL_NONE
    };

    EGLint pbufferAttribs[] = {
            EGL_WIDTH,  1,
            EGL_HEIGHT, 1,
            EGL_NONE
    };

#ifdef NDEBUG
    // When we don't have a shared context and we're in release mode, we always activate the
    // EGL_KHR_create_context_no_error extension.
    if (!sharedContext && extensions.has("EGL_KHR_create_context_no_error")) {
        contextAttribs[2] = EGL_CONTEXT_OPENGL_NO_ERROR_KHR;
        contextAttribs[3] = EGL_TRUE;
    }
#endif

    EGLConfig eglConfig = nullptr;

    // find an opaque config
    if (!eglChooseConfig(mEGLDisplay, configAttribs, &mEGLConfig, 1, &configsCount)) {
        logEglError("eglChooseConfig");
        goto error;
    }

    if (configsCount == 0) {
      // warn and retry without EGL_RECORDABLE_ANDROID
      logEglError("eglChooseConfig(..., EGL_RECORDABLE_ANDROID) failed. Continuing without it.");
      configAttribs[10] = EGL_RECORDABLE_ANDROID;
      configAttribs[11] = EGL_DONT_CARE;
      if (!eglChooseConfig(mEGLDisplay, configAttribs, &mEGLConfig, 1, &configsCount) ||
              configsCount == 0) {
          logEglError("eglChooseConfig");
          goto error;
      }
    }

    // find a transparent config
    configAttribs[8] = EGL_ALPHA_SIZE;
    configAttribs[9] = 8;
    if (!eglChooseConfig(mEGLDisplay, configAttribs, &mEGLTransparentConfig, 1, &configsCount) ||
            (configAttribs[11] == EGL_DONT_CARE && configsCount == 0)) {
        logEglError("eglChooseConfig");
        goto error;
    }

    if (configsCount == 0) {
      // warn and retry without EGL_RECORDABLE_ANDROID
        logEglError("eglChooseConfig(..., EGL_RECORDABLE_ANDROID) failed. Continuing without it.");
      // this is not fatal
      configAttribs[10] = EGL_RECORDABLE_ANDROID;
      configAttribs[11] = EGL_DONT_CARE;
      if (!eglChooseConfig(mEGLDisplay, configAttribs, &mEGLTransparentConfig, 1, &configsCount) ||
              configsCount == 0) {
          logEglError("eglChooseConfig");
          goto error;
      }
    }

    if (!extensions.has("EGL_KHR_no_config_context")) {
        // if we have the EGL_KHR_no_config_context, we don't need to worry about the config
        // when creating the context, otherwise, we must always pick a transparent config.
        eglConfig = mEGLConfig = mEGLTransparentConfig;
    }

    // the pbuffer dummy surface is always created with a transparent surface because
    // either we have EGL_KHR_no_config_context and it doesn't matter, or we don't and
    // we must use a transparent surface
    mEGLDummySurface = eglCreatePbufferSurface(mEGLDisplay, mEGLTransparentConfig, pbufferAttribs);
    if (mEGLDummySurface == EGL_NO_SURFACE) {
        logEglError("eglCreatePbufferSurface");
        goto error;
    }

    mEGLContext = eglCreateContext(mEGLDisplay, eglConfig, (EGLContext)sharedContext, contextAttribs);
    if (mEGLContext == EGL_NO_CONTEXT && sharedContext &&
        extensions.has("EGL_KHR_create_context_no_error")) {
        // context creation could fail because of EGL_CONTEXT_OPENGL_NO_ERROR_KHR
        // not matching the sharedContext. Try with it.
        contextAttribs[2] = EGL_CONTEXT_OPENGL_NO_ERROR_KHR;
        contextAttribs[3] = EGL_TRUE;
        mEGLContext = eglCreateContext(mEGLDisplay, eglConfig, (EGLContext)sharedContext, contextAttribs);
    }
    if (UTILS_UNLIKELY(mEGLContext == EGL_NO_CONTEXT)) {
        // eglCreateContext failed
        logEglError("eglCreateContext");
        goto error;
    }

    if (!makeCurrent(mEGLDummySurface, mEGLDummySurface)) {
        // eglMakeCurrent failed
        logEglError("eglMakeCurrent");
        goto error;
    }

    initializeGlExtensions();

    // this is needed with older emulators/API levels on Android
    clearGlError();

    // success!!
    return OpenGLDriverFactory::create(this, sharedContext);

error:
    // if we're here, we've failed
    if (mEGLDummySurface) {
        eglDestroySurface(mEGLDisplay, mEGLDummySurface);
    }
    if (mEGLContext) {
        eglDestroyContext(mEGLDisplay, mEGLContext);
    }

    mEGLDummySurface = EGL_NO_SURFACE;
    mEGLContext = EGL_NO_CONTEXT;

    eglTerminate(mEGLDisplay);
    eglReleaseThread();

    return nullptr;
}

EGLBoolean PlatformEGL::makeCurrent(EGLSurface drawSurface, EGLSurface readSurface) noexcept {
    if (UTILS_UNLIKELY((drawSurface != mCurrentDrawSurface || readSurface != mCurrentReadSurface))) {
        mCurrentDrawSurface = drawSurface;
        mCurrentReadSurface = readSurface;
        return eglMakeCurrent(mEGLDisplay, drawSurface, readSurface, mEGLContext);
    }
    return EGL_TRUE;
}

void PlatformEGL::terminate() noexcept {
    eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(mEGLDisplay, mEGLDummySurface);
    eglDestroyContext(mEGLDisplay, mEGLContext);
    eglTerminate(mEGLDisplay);
    eglReleaseThread();
}

Platform::SwapChain* PlatformEGL::createSwapChain(
        void* nativeWindow, uint64_t& flags) noexcept {
    EGLSurface sur = eglCreateWindowSurface(mEGLDisplay,
            (flags & backend::SWAP_CHAIN_CONFIG_TRANSPARENT) ?
            mEGLTransparentConfig : mEGLConfig,
            (EGLNativeWindowType)nativeWindow, nullptr);

    if (UTILS_UNLIKELY(sur == EGL_NO_SURFACE)) {
        logEglError("eglCreateWindowSurface");
        return nullptr;
    }
    if (!eglSurfaceAttrib(mEGLDisplay, sur, EGL_SWAP_BEHAVIOR, EGL_BUFFER_DESTROYED)) {
        logEglError("eglSurfaceAttrib(..., EGL_SWAP_BEHAVIOR, EGL_BUFFER_DESTROYED)");
        // this is not fatal
    }
    // activate this for recording frame timestamps
    //if (!eglSurfaceAttrib(mEGLDisplay, sur, EGL_TIMESTAMPS_ANDROID, EGL_TRUE)) {
    //    logEglError("eglSurfaceAttrib(..., EGL_TIMESTAMPS_ANDROID, EGL_TRUE)");
    //    // this is not fatal
    //}
    return (SwapChain*)sur;
}

Platform::SwapChain* PlatformEGL::createSwapChain(
        uint32_t width, uint32_t height, uint64_t& flags) noexcept {

    EGLint attribs[] = {
            EGL_WIDTH, EGLint(width),
            EGL_HEIGHT, EGLint(height),
            EGL_NONE
    };

    EGLSurface sur = eglCreatePbufferSurface(mEGLDisplay,
                (flags & backend::SWAP_CHAIN_CONFIG_TRANSPARENT) ?
                mEGLTransparentConfig : mEGLConfig, attribs);

    if (UTILS_UNLIKELY(sur == EGL_NO_SURFACE)) {
        logEglError("eglCreatePbufferSurface");
        return nullptr;
    }
    return (SwapChain*)sur;
}

void PlatformEGL::destroySwapChain(Platform::SwapChain* swapChain) noexcept {
    EGLSurface sur = (EGLSurface) swapChain;
    if (sur != EGL_NO_SURFACE) {
        makeCurrent(mEGLDummySurface, mEGLDummySurface);
        eglDestroySurface(mEGLDisplay, sur);
    }
}

void PlatformEGL::makeCurrent(Platform::SwapChain* drawSwapChain,
                              Platform::SwapChain* readSwapChain) noexcept {
    EGLSurface drawSur = (EGLSurface) drawSwapChain;
    EGLSurface readSur = (EGLSurface) readSwapChain;
    if (drawSur != EGL_NO_SURFACE || readSur != EGL_NO_SURFACE) {
        makeCurrent(drawSur, readSur);
    }
}

void PlatformEGL::setPresentationTime(int64_t presentationTimeInNanosecond) noexcept {
    if (mCurrentDrawSurface != EGL_NO_SURFACE) {
        if (eglPresentationTimeANDROID) {
            eglPresentationTimeANDROID(
                    mEGLDisplay,
                    mCurrentDrawSurface,
                    presentationTimeInNanosecond);
        }
    }
}

void PlatformEGL::commit(Platform::SwapChain* swapChain) noexcept {
    EGLSurface sur = (EGLSurface) swapChain;
    if (sur != EGL_NO_SURFACE) {
        eglSwapBuffers(mEGLDisplay, sur);
    }
}

Platform::Fence* PlatformEGL::createFence() noexcept {
    Fence* f = nullptr;
#ifdef EGL_KHR_reusable_sync
    f = (Fence*) eglCreateSyncKHR(mEGLDisplay, EGL_SYNC_FENCE_KHR, nullptr);
#endif
    return f;
}

void PlatformEGL::destroyFence(Platform::Fence* fence) noexcept {
#ifdef EGL_KHR_reusable_sync
    EGLSyncKHR sync = (EGLSyncKHR) fence;
    if (sync != EGL_NO_SYNC_KHR) {
        eglDestroySyncKHR(mEGLDisplay, sync);
    }
#endif
}

backend::FenceStatus PlatformEGL::waitFence(
        Platform::Fence* fence, uint64_t timeout) noexcept {
#ifdef EGL_KHR_reusable_sync
    EGLSyncKHR sync = (EGLSyncKHR) fence;
    if (sync != EGL_NO_SYNC_KHR) {
        EGLint status = eglClientWaitSyncKHR(mEGLDisplay, sync, 0, (EGLTimeKHR)timeout);
        if (status == EGL_CONDITION_SATISFIED_KHR) {
            return FenceStatus::CONDITION_SATISFIED;
        }
        if (status == EGL_TIMEOUT_EXPIRED_KHR) {
            return FenceStatus::TIMEOUT_EXPIRED;
        }
    }
#endif
    return FenceStatus::ERROR;
}

Platform::Stream* PlatformEGL::createStream(void* nativeStream) noexcept {
    return mExternalStreamManager.acquire(static_cast<jobject>(nativeStream));
}

void PlatformEGL::destroyStream(Platform::Stream* stream) noexcept {
    mExternalStreamManager.release(stream);
}

void PlatformEGL::attach(Stream* stream, intptr_t tname) noexcept {
    mExternalStreamManager.attach(stream, tname);
}

void PlatformEGL::detach(Stream* stream) noexcept {
    mExternalStreamManager.detach(stream);
}

void PlatformEGL::updateTexImage(Stream* stream, int64_t* timestamp) noexcept {
    mExternalStreamManager.updateTexImage(stream, timestamp);
}

Platform::ExternalTexture* PlatformEGL::createExternalTextureStorage() noexcept {
    return mExternalTextureManager.create();
}

void PlatformEGL::reallocateExternalStorage(
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

void PlatformEGL::destroyExternalTextureStorage(
        Platform::ExternalTexture* externalTexture) noexcept {
    if (externalTexture) {
        mExternalTextureManager.destroy(externalTexture);
        if ((EGLImageKHR)externalTexture->image != EGL_NO_IMAGE_KHR) {
            eglDestroyImageKHR(mEGLDisplay, (EGLImageKHR)externalTexture->image);
            externalTexture->image = (uintptr_t)EGL_NO_IMAGE_KHR;
        }
    }
}

int PlatformEGL::getOSVersion() const noexcept {
    return mOSVersion;
}

void PlatformEGL::createExternalImageTexture(void* texture) noexcept {
    auto* t = (OpenGLDriver::GLTexture*) texture;
    glGenTextures(1, &t->gl.id);
    if (ext.OES_EGL_image_external_essl3) {
        t->gl.targetIndex = (uint8_t)
                OpenGLContext::getIndexForTextureTarget(t->gl.target = GL_TEXTURE_EXTERNAL_OES);
    }
}

void PlatformEGL::destroyExternalImage(void* texture) noexcept {
    auto* t = (OpenGLDriver::GLTexture*) texture;
    glDeleteTextures(1, &t->gl.id);
}

backend::AcquiredImage PlatformEGL::transformAcquiredImage(backend::AcquiredImage source) noexcept {
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

void PlatformEGL::initializeGlExtensions() noexcept {
    unordered_string_set glExtensions;
    GLint n;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    for (GLint i = 0; i < n; ++i) {
        const char * const extension = (const char*) glGetStringi(GL_EXTENSIONS, (GLuint)i);
        glExtensions.insert(extension);
    }
    ext.OES_EGL_image_external_essl3 = glExtensions.has("GL_OES_EGL_image_external_essl3");
}

// This must called when the library is loaded. We need this to get a reference to the global VM
void JNI_OnLoad(JavaVM* vm, void* reserved) {
    ::filament::VirtualMachineEnv::JNI_OnLoad(vm);
}

} // namespace filament

// ---------------------------------------------------------------------------------------------
