/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "PlatformEGLHeadless.h"

#include "opengl/OpenGLDriver.h"
#include "opengl/OpenGLContext.h"
#include "opengl/OpenGLDriverFactory.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Panic.h>

using namespace utils;

namespace filament {
using namespace backend;

namespace glext {
extern PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR;
extern PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR;
extern PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR;
extern PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
extern PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
}
using namespace glext;

// ---------------------------------------------------------------------------------------------

PlatformEGLHeadless::PlatformEGLHeadless() noexcept
        : PlatformEGL() {
}

backend::Driver* PlatformEGLHeadless::createDriver(void* sharedContext) noexcept {
    EGLBoolean bindAPI = eglBindAPI(EGL_OPENGL_API);
    if (UTILS_UNLIKELY(!bindAPI)) {
        slog.e << "eglBindAPI EGL_OPENGL_API failed" << io::endl;
        return nullptr;
    }
    int bindBlueGL = bluegl::bind();
    if (UTILS_UNLIKELY(bindBlueGL != 0)) {
        slog.e << "bluegl bind failed" << io::endl;
        return nullptr;
    }

    // Copied from the base class and modified slightly. Should be cleaned up/improved later.
    mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert_invariant(mEGLDisplay != EGL_NO_DISPLAY);

    EGLint major, minor;
    EGLBoolean initialized = eglInitialize(mEGLDisplay, &major, &minor);

    if (!initialized) {
      EGLDeviceEXT eglDevice;
      EGLint numDevices;

      PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
              (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");
      if (eglQueryDevicesEXT != NULL) {
          eglQueryDevicesEXT(1, &eglDevice, &numDevices);
          if(auto* getPlatformDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
              eglGetProcAddress("eglGetPlatformDisplay"))) {
            mEGLDisplay = getPlatformDisplay(EGL_PLATFORM_DEVICE_EXT, eglDevice, 0);
            initialized = eglInitialize(mEGLDisplay, &major, &minor);
          }
      }
    }

    if (UTILS_UNLIKELY(!initialized)) {
        slog.e << "eglInitialize failed" << io::endl;
        return nullptr;
    }

    auto extensions = GLUtils::split(eglQueryString(mEGLDisplay, EGL_EXTENSIONS));

    eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) eglGetProcAddress("eglCreateSyncKHR");
    eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) eglGetProcAddress("eglDestroySyncKHR");
    eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC) eglGetProcAddress("eglClientWaitSyncKHR");

    eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
    eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");

    EGLint configsCount;

    EGLint configAttribs[] = {
           EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
           EGL_RED_SIZE, 8,
           EGL_GREEN_SIZE, 8,
           EGL_BLUE_SIZE, 8,
           EGL_ALPHA_SIZE,  0,
           EGL_DEPTH_SIZE, 32,
           EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
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

    // fallback to a 24-bit depth buffer
    if (configsCount == 0) {
        configAttribs[10] = EGL_DEPTH_SIZE;
        configAttribs[11] = 24;

        if (!eglChooseConfig(mEGLDisplay, configAttribs, &mEGLConfig, 1, &configsCount)) {
            logEglError("eglChooseConfig");
            goto error;
        }
    }

    // find a transparent config
    configAttribs[8] = EGL_ALPHA_SIZE;
    configAttribs[9] = 8;
    if (!eglChooseConfig(mEGLDisplay, configAttribs, &mEGLTransparentConfig, 1, &configsCount) ||
            (configAttribs[13] == EGL_DONT_CARE && configsCount == 0)) {
        logEglError("eglChooseConfig");
        goto error;
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

} // namespace filament

// ---------------------------------------------------------------------------------------------
