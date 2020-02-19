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
#include "OpenGLContext.h"
#include "OpenGLDriverFactory.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/compiler.h>
#include <utils/Log.h>

#include <assert.h>


using namespace utils;

namespace filament {
using namespace backend;

// The Android NDK doesn't exposes extensions, fake it with eglGetProcAddress
namespace glext {
UTILS_PRIVATE PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR = {};
UTILS_PRIVATE PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR = {};
UTILS_PRIVATE PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR = {};
UTILS_PRIVATE PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = {};
UTILS_PRIVATE PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = {};
}
using namespace glext;


// ---------------------------------------------------------------------------------------------
// Utilities
// ---------------------------------------------------------------------------------------------

void PlatformEGL::logEglError(const char* name) noexcept {
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

// ---------------------------------------------------------------------------------------------

PlatformEGL::PlatformEGL() noexcept = default;

Driver* PlatformEGL::createDriver(void* sharedContext) noexcept {
    mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(mEGLDisplay != EGL_NO_DISPLAY);

    EGLint major, minor;
    EGLBoolean initialized = eglInitialize(mEGLDisplay, &major, &minor);
    if (UTILS_UNLIKELY(!initialized)) {
        slog.e << "eglInitialize failed" << io::endl;
        return nullptr;
    }

    importGLESExtensionsEntryPoints();

    auto extensions = GLUtils::split(eglQueryString(mEGLDisplay, EGL_EXTENSIONS));

    eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) eglGetProcAddress("eglCreateSyncKHR");
    eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) eglGetProcAddress("eglDestroySyncKHR");
    eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC) eglGetProcAddress("eglClientWaitSyncKHR");

    eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
    eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");

    EGLint configsCount;
    EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_RED_SIZE,    8,
            EGL_GREEN_SIZE,  8,
            EGL_BLUE_SIZE,   8,
            EGL_ALPHA_SIZE,  0, // reserved to set ALPHA_SIZE below
            EGL_DEPTH_SIZE, 24,
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

void PlatformEGL::createExternalImageTexture(void* texture) noexcept {
    auto* t = (OpenGLDriver::GLTexture*) texture;
    glGenTextures(1, &t->gl.id);
    if (UTILS_LIKELY(ext.OES_EGL_image_external_essl3)) {
        t->gl.target = GL_TEXTURE_EXTERNAL_OES;
        t->gl.targetIndex = (uint8_t)
                OpenGLContext::getIndexForTextureTarget(GL_TEXTURE_EXTERNAL_OES);
    } else {
        // if texture external is not supported, revert to texture 2d
        t->gl.target = GL_TEXTURE_2D;
        t->gl.targetIndex = (uint8_t)
                OpenGLContext::getIndexForTextureTarget(GL_TEXTURE_2D);
    }
}

void PlatformEGL::destroyExternalImage(void* texture) noexcept {
    auto* t = (OpenGLDriver::GLTexture*) texture;
    glDeleteTextures(1, &t->gl.id);
}

void PlatformEGL::initializeGlExtensions() noexcept {
    GLUtils::unordered_string_set glExtensions;
    GLint n;
    glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    for (GLint i = 0; i < n; ++i) {
        const char * const extension = (const char*) glGetStringi(GL_EXTENSIONS, (GLuint)i);
        glExtensions.insert(extension);
    }
    ext.OES_EGL_image_external_essl3 = glExtensions.has("GL_OES_EGL_image_external_essl3");
}

} // namespace filament

// ---------------------------------------------------------------------------------------------
