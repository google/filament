#define GL_GLEXT_PROTOTYPES 1
#define EGL_EGL_PROTOTYPES 1
#include "PlatformEGL_ANGLE.h"

#include "OpenGLDriverFactory.h"

#include <utils/debug.h>

#include <EGL/eglext_angle.h>

using namespace utils;

namespace filament {
using namespace backend;


PlatformEGL_ANGLE::PlatformEGL_ANGLE(EGLDisplay display) noexcept {
    mEGLDisplay = display;
    assert_invariant(mEGLDisplay != EGL_NO_DISPLAY);
}

backend::Driver* PlatformEGL_ANGLE::createDriver(void* sharedContext) noexcept {
    EGLint major, minor;
    EGLBoolean initialized = eglInitialize(mEGLDisplay, &major, &minor);
    if (UTILS_UNLIKELY(!initialized)) {
        slog.e << "eglInitialize failed" << io::endl;
        return nullptr;
    }

    auto extensions = initializeExtensions();
    
    EGLint configsCount;
    EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,            //  0
        EGL_RED_SIZE,    8,                                     //  2
        EGL_GREEN_SIZE,  8,                                     //  4
        EGL_BLUE_SIZE,   8,                                     //  6
        EGL_ALPHA_SIZE,  0,                                     //  8 : reserved to set ALPHA_SIZE below
        EGL_DEPTH_SIZE, 24,                                     // 10
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
        goto errorANGLE;
    }

    // find a transparent config
    configAttribs[8] = EGL_ALPHA_SIZE;
    configAttribs[9] = 8;
    if (!eglChooseConfig(mEGLDisplay, configAttribs, &mEGLTransparentConfig, 1, &configsCount)) {
        logEglError("eglChooseConfig");
        goto errorANGLE;
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
        goto errorANGLE;
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
        goto errorANGLE;
    }

    if (!makeCurrent(mEGLDummySurface, mEGLDummySurface)) {
        // eglMakeCurrent failed
        logEglError("eglMakeCurrent");
        goto errorANGLE;
    }

    initializeGlExtensions();

    // success!!
    return OpenGLDriverFactory::create(this, sharedContext);

errorANGLE:
    // if we're here, we've failed
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
