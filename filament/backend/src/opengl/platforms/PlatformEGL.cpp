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

#include <backend/platforms/PlatformEGL.h>

#include "opengl/GLUtils.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#if defined(__ANDROID__)
#include <sys/system_properties.h>
#endif

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/Panic.h>

#ifndef EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE
#   define EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE 0x3483
#endif

using namespace utils;

namespace filament::backend {
using namespace backend;

// The Android NDK doesn't expose extensions, fake it with eglGetProcAddress
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
    logEglError(name, eglGetError());
}

void PlatformEGL::logEglError(const char* name, EGLint error) noexcept {
    slog.e << name << " failed with " << getEglErrorName(error) << io::endl;
}

const char* PlatformEGL::getEglErrorName(EGLint error) noexcept {
    switch (error) {
        case EGL_NOT_INITIALIZED:       return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS:            return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC:             return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE:         return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONTEXT:           return "EGL_BAD_CONTEXT";
        case EGL_BAD_CONFIG:            return "EGL_BAD_CONFIG";
        case EGL_BAD_CURRENT_SURFACE:   return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY:           return "EGL_BAD_DISPLAY";
        case EGL_BAD_SURFACE:           return "EGL_BAD_SURFACE";
        case EGL_BAD_MATCH:             return "EGL_BAD_MATCH";
        case EGL_BAD_PARAMETER:         return "EGL_BAD_PARAMETER";
        case EGL_BAD_NATIVE_PIXMAP:     return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW:     return "EGL_BAD_NATIVE_WINDOW";
        case EGL_CONTEXT_LOST:          return "EGL_CONTEXT_LOST";
        default:                        return "unknown";
    }
}

void PlatformEGL::clearGlError() noexcept {
    // clear GL error that may have been set by previous calls
    GLenum const error = glGetError();
    if (error != GL_NO_ERROR) {
        slog.w << "Ignoring pending GL error " << io::hex << error << io::endl;
    }
}

// ---------------------------------------------------------------------------------------------

PlatformEGL::PlatformEGL() noexcept = default;

int PlatformEGL::getOSVersion() const noexcept {
    return 0;
}


Driver* PlatformEGL::createDriver(void* sharedContext, const Platform::DriverConfig& driverConfig) noexcept {
    mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert_invariant(mEGLDisplay != EGL_NO_DISPLAY);

    EGLint major, minor;
    EGLBoolean const initialized = eglInitialize(mEGLDisplay, &major, &minor);
    if (UTILS_UNLIKELY(!initialized)) {
        slog.e << "eglInitialize failed" << io::endl;
        return nullptr;
    }

#if defined(FILAMENT_IMPORT_ENTRY_POINTS)
    importGLESExtensionsEntryPoints();
#endif

    auto extensions = GLUtils::split(eglQueryString(mEGLDisplay, EGL_EXTENSIONS));
    ext.egl.ANDROID_recordable = extensions.has("EGL_ANDROID_recordable");
    ext.egl.KHR_gl_colorspace = extensions.has("EGL_KHR_gl_colorspace");
    ext.egl.KHR_create_context = extensions.has("EGL_KHR_create_context");
    ext.egl.KHR_no_config_context = extensions.has("EGL_KHR_no_config_context");
    ext.egl.KHR_surfaceless_context = extensions.has("KHR_surfaceless_context");
    if (ext.egl.KHR_create_context) {
        // KHR_create_context implies KHR_surfaceless_context for ES3.x contexts
        ext.egl.KHR_surfaceless_context = true;
    }

    eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) eglGetProcAddress("eglCreateSyncKHR");
    eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) eglGetProcAddress("eglDestroySyncKHR");
    eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC) eglGetProcAddress("eglClientWaitSyncKHR");

    eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
    eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");

    EGLint const pbufferAttribs[] = {
            EGL_WIDTH,  1,
            EGL_HEIGHT, 1,
            EGL_NONE
    };

#ifdef __ANDROID__
    bool requestES2Context = false;
    char property[PROP_VALUE_MAX];
    int const length = __system_property_get("debug.filament.es2", property);
    if (length > 0) {
        requestES2Context = bool(atoi(property));
    }
#else
    constexpr bool requestES2Context = false;
#endif

    // Request a ES2 context, devices that support ES3 will return an ES3 context
    Config contextAttribs = {
            { EGL_CONTEXT_CLIENT_VERSION, 2 },
    };

    // FOR TESTING ONLY, enforce the ES version we're asking for.
    // FIXME: we should check EGL_ANGLE_create_context_backwards_compatible, however, at least
    //        some versions of ANGLE don't advertise this extension but do support it.
    if (requestES2Context) {
        // TODO: is there a way to request the ANGLE driver if available?
        contextAttribs[EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE] = EGL_FALSE;
    }

#ifdef NDEBUG
    // When we don't have a shared context, and we're in release mode, we always activate the
    // EGL_KHR_create_context_no_error extension.
    if (!sharedContext && extensions.has("EGL_KHR_create_context_no_error")) {
        contextAttribs[EGL_CONTEXT_OPENGL_NO_ERROR_KHR] = EGL_TRUE;
    }
#endif

    // config use for creating the context
    EGLConfig eglConfig = EGL_NO_CONFIG_KHR;

    // find a config we can use if we don't have "EGL_KHR_no_config_context" and that we can use
    // for the dummy pbuffer surface.
    mEGLConfig = findSwapChainConfig(0);
    if (UTILS_UNLIKELY(mEGLConfig == EGL_NO_CONFIG_KHR)) {
        goto error; // error already logged
    }

    if (UTILS_UNLIKELY(!ext.egl.KHR_no_config_context)) {
        // if we don't have the EGL_KHR_no_config_context the context must be created with
        // the same config as the swapchain, so we have no choice but to create a
        // transparent config.
        eglConfig = mEGLConfig;
    }

    for (size_t tries = 0; tries < 3; tries++) {
        mEGLContext = eglCreateContext(mEGLDisplay, eglConfig,
                (EGLContext)sharedContext, contextAttribs.data());
        if (UTILS_LIKELY(mEGLContext != EGL_NO_CONTEXT)) {
            break;
        }

        GLint const error = eglGetError();
        if (error == EGL_BAD_ATTRIBUTE) {
            // ANGLE doesn't always advertise this extension, so we have to try
            contextAttribs.erase(EGL_CONTEXT_OPENGL_BACKWARDS_COMPATIBLE_ANGLE);
            continue;
        }
#ifdef NDEBUG
        else if (error == EGL_BAD_MATCH &&
                   sharedContext && extensions.has("EGL_KHR_create_context_no_error")) {
            // context creation could fail because of EGL_CONTEXT_OPENGL_NO_ERROR_KHR
            // not matching the sharedContext. Try with it.
            contextAttribs[EGL_CONTEXT_OPENGL_NO_ERROR_KHR] = EGL_TRUE;
            continue;
        }
#endif
        (void)error;
        break;
    }

    if (UTILS_UNLIKELY(mEGLContext == EGL_NO_CONTEXT)) {
        // eglCreateContext failed
        logEglError("eglCreateContext");
        goto error;
    }

    if (ext.egl.KHR_surfaceless_context) {
        // Adreno 306 driver advertises KHR_create_context but doesn't support passing
        // EGL_NO_SURFACE to eglMakeCurrent with a 3.0 context.
        if (UTILS_UNLIKELY(!eglMakeCurrent(mEGLDisplay,
                EGL_NO_SURFACE, EGL_NO_SURFACE, mEGLContext))) {
            if (eglGetError() == EGL_BAD_MATCH) {
                ext.egl.KHR_surfaceless_context = false;
            }
        }
    }

    if (UTILS_UNLIKELY(!ext.egl.KHR_surfaceless_context)) {
        // create the dummy surface, just for being able to make the context current.
        mEGLDummySurface = eglCreatePbufferSurface(mEGLDisplay, mEGLConfig, pbufferAttribs);
        if (UTILS_UNLIKELY(mEGLDummySurface == EGL_NO_SURFACE)) {
            logEglError("eglCreatePbufferSurface");
            goto error;
        }
    }

    if (UTILS_UNLIKELY(!makeCurrent(mEGLDummySurface, mEGLDummySurface))) {
        // eglMakeCurrent failed
        logEglError("eglMakeCurrent");
        goto error;
    }

    mContextAttribs = std::move(contextAttribs);

    initializeGlExtensions();

    // this is needed with older emulators/API levels on Android
    clearGlError();

    // success!!
    return OpenGLPlatform::createDefaultDriver(this, sharedContext, driverConfig);

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

bool PlatformEGL::isExtraContextSupported() const noexcept {
    return ext.egl.KHR_surfaceless_context;
}

void PlatformEGL::createContext(bool shared) {
    EGLConfig config = ext.egl.KHR_no_config_context ? EGL_NO_CONFIG_KHR : mEGLConfig;

    EGLContext context = eglCreateContext(mEGLDisplay, config,
            shared ? mEGLContext : EGL_NO_CONTEXT, mContextAttribs.data());

    if (UTILS_UNLIKELY(context == EGL_NO_CONTEXT)) {
        // eglCreateContext failed
        logEglError("eglCreateContext");
    }

    assert_invariant(context != EGL_NO_CONTEXT);

    eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context);

    mAdditionalContexts.push_back(context);
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
    eglMakeCurrent(mEGLDisplay, mEGLDummySurface, mEGLDummySurface, EGL_NO_CONTEXT);
    if (mEGLDummySurface) {
        eglDestroySurface(mEGLDisplay, mEGLDummySurface);
    }
    eglDestroyContext(mEGLDisplay, mEGLContext);
    for (auto context : mAdditionalContexts) {
        eglDestroyContext(mEGLDisplay, context);
    }
    eglTerminate(mEGLDisplay);
    eglReleaseThread();
}

EGLConfig PlatformEGL::findSwapChainConfig(uint64_t flags) const {
    // Find config that support ES3.
    EGLConfig config = EGL_NO_CONFIG_KHR;
    EGLint configsCount;
    Config configAttribs = {
            { EGL_RENDERABLE_TYPE,  EGL_OPENGL_ES2_BIT },
            { EGL_RED_SIZE,         8 },
            { EGL_GREEN_SIZE,       8 },
            { EGL_BLUE_SIZE,        8 },
            { EGL_ALPHA_SIZE,      (flags & SWAP_CHAIN_CONFIG_TRANSPARENT) ? 8 : 0 },
            { EGL_DEPTH_SIZE,      24 },
    };


    if (ext.egl.KHR_create_context) {
        configAttribs[EGL_RECORDABLE_ANDROID] |= EGL_OPENGL_ES3_BIT_KHR;
    }

    if (ext.egl.ANDROID_recordable) {
        configAttribs[EGL_RECORDABLE_ANDROID] = EGL_TRUE;
    }

    if (UTILS_UNLIKELY(
            !eglChooseConfig(mEGLDisplay, configAttribs.data(), &config, 1, &configsCount))) {
        logEglError("eglChooseConfig");
            return EGL_NO_CONFIG_KHR;
    }

    if (UTILS_UNLIKELY(configsCount == 0)) {
        if (ext.egl.ANDROID_recordable) {
            // warn and retry without EGL_RECORDABLE_ANDROID
            logEglError(
                    "eglChooseConfig(..., EGL_RECORDABLE_ANDROID) failed. Continuing without it.");
            configAttribs[EGL_RECORDABLE_ANDROID] = EGL_DONT_CARE;
            if (UTILS_UNLIKELY(
                    !eglChooseConfig(mEGLDisplay, configAttribs.data(), &config, 1, &configsCount)
                            || configsCount == 0)) {
                logEglError("eglChooseConfig");
                return EGL_NO_CONFIG_KHR;
            }
        } else {
            // we found zero config matching our request!
            logEglError("eglChooseConfig() didn't find any matching config!");
            return EGL_NO_CONFIG_KHR;
        }
    }
    return config;
}

bool PlatformEGL::isSRGBSwapChainSupported() const noexcept {
    return ext.egl.KHR_gl_colorspace;
}

Platform::SwapChain* PlatformEGL::createSwapChain(
        void* nativeWindow, uint64_t flags) noexcept {

    EGLConfig config = EGL_NO_CONFIG_KHR;
    if (UTILS_LIKELY(ext.egl.KHR_no_config_context)) {
        config = findSwapChainConfig(flags);
    } else {
        config = mEGLConfig;
    }

    if (UTILS_UNLIKELY(config == EGL_NO_CONFIG_KHR)) {
        return nullptr;
    }

    Config attribs;

    if (ext.egl.KHR_gl_colorspace) {
        if (flags & SWAP_CHAIN_CONFIG_SRGB_COLORSPACE) {
            attribs[EGL_GL_COLORSPACE_KHR] = EGL_GL_COLORSPACE_SRGB_KHR;
        }
    }

    EGLSurface sur = eglCreateWindowSurface(mEGLDisplay, config,
            (EGLNativeWindowType)nativeWindow, attribs.data());

    if (UTILS_UNLIKELY(sur == EGL_NO_SURFACE)) {
        logEglError("eglCreateWindowSurface");
        return nullptr;
    }

    // this is not fatal
    eglSurfaceAttrib(mEGLDisplay, sur, EGL_SWAP_BEHAVIOR, EGL_BUFFER_DESTROYED);

    return (SwapChain*)sur;
}

Platform::SwapChain* PlatformEGL::createSwapChain(
        uint32_t width, uint32_t height, uint64_t flags) noexcept {

    EGLConfig config = EGL_NO_CONFIG_KHR;
    if (UTILS_LIKELY(ext.egl.KHR_no_config_context)) {
        config = findSwapChainConfig(flags);
    } else {
        config = mEGLConfig;
    }

    if (UTILS_UNLIKELY(config == EGL_NO_CONFIG_KHR)) {
        return nullptr;
    }

    Config attribs = {
            { EGL_WIDTH,  EGLint(width) },
            { EGL_HEIGHT, EGLint(height) },
    };

    if (ext.egl.KHR_gl_colorspace) {
        if (flags & SWAP_CHAIN_CONFIG_SRGB_COLORSPACE) {
            attribs[EGL_GL_COLORSPACE_KHR] = EGL_GL_COLORSPACE_SRGB_KHR;
        }
    }

    EGLSurface sur = eglCreatePbufferSurface(mEGLDisplay, config, attribs.data());

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

bool PlatformEGL::canCreateFence() noexcept {
    return true;
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

FenceStatus PlatformEGL::waitFence(
        Platform::Fence* fence, uint64_t timeout) noexcept {
#ifdef EGL_KHR_reusable_sync
    EGLSyncKHR sync = (EGLSyncKHR) fence;
    if (sync != EGL_NO_SYNC_KHR) {
        EGLint const status = eglClientWaitSyncKHR(mEGLDisplay, sync, 0, (EGLTimeKHR)timeout);
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

OpenGLPlatform::ExternalTexture* PlatformEGL::createExternalImageTexture() noexcept {
    ExternalTexture* outTexture = new ExternalTexture{};
    glGenTextures(1, &outTexture->id);
    if (UTILS_LIKELY(ext.gl.OES_EGL_image_external_essl3)) {
        outTexture->target = GL_TEXTURE_EXTERNAL_OES;
    } else {
        // if texture external is not supported, revert to texture 2d
        outTexture->target = GL_TEXTURE_2D;
    }
    return outTexture;
}

void PlatformEGL::destroyExternalImage(ExternalTexture* texture) noexcept {
    glDeleteTextures(1, &texture->id);
    delete texture;
}

bool PlatformEGL::setExternalImage(void* externalImage,
        UTILS_UNUSED_IN_RELEASE ExternalTexture* texture) noexcept {
    if (UTILS_LIKELY(ext.gl.OES_EGL_image_external_essl3)) {
        assert_invariant(texture->target == GL_TEXTURE_EXTERNAL_OES);
        // the texture is guaranteed to be bound here.
#ifdef GL_OES_EGL_image
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES,
                static_cast<GLeglImageOES>(externalImage));
#endif
    }
    return true;
}

void PlatformEGL::initializeGlExtensions() noexcept {
    // We're guaranteed to be on an ES platform, since we're using EGL
    GLUtils::unordered_string_set glExtensions;
    const char* const extensions = (const char*)glGetString(GL_EXTENSIONS);
    glExtensions = GLUtils::split(extensions);
    ext.gl.OES_EGL_image_external_essl3 = glExtensions.has("GL_OES_EGL_image_external_essl3");
}

// ---------------------------------------------------------------------------------------------

PlatformEGL::Config::Config() = default;

PlatformEGL::Config::Config(std::initializer_list<std::pair<EGLint, EGLint>> list)
        : mConfig(list) {
    mConfig.emplace_back(EGL_NONE, EGL_NONE);
}

EGLint& PlatformEGL::Config::operator[](EGLint name) {
    auto pos = std::find_if(mConfig.begin(), mConfig.end(),
            [name](auto&& v) { return v.first == name; });
    if (pos == mConfig.end()) {
        mConfig.insert(pos - 1, { name, EGL_NONE });
        pos = mConfig.end() - 2;
    }
    return pos->second;
}

EGLint PlatformEGL::Config::operator[](EGLint name) const {
    auto pos = std::find_if(mConfig.begin(), mConfig.end(),
            [name](auto&& v) { return v.first == name; });
    assert_invariant(pos != mConfig.end());
    return pos->second;
}

void PlatformEGL::Config::erase(EGLint name) noexcept {
    if (name != EGL_NONE) {
        auto pos = std::find_if(mConfig.begin(), mConfig.end(),
                [name](auto&& v) { return v.first == name; });
        if (pos != mConfig.end()) {
            mConfig.erase(pos);
        }
    }
}

} // namespace filament::backend

// ---------------------------------------------------------------------------------------------
