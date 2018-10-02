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

#include "driver/opengl/ContextManagerEGL.h"

#include <jni.h>

#include <assert.h>
#include <dlfcn.h>

#include <string>
#include <unordered_set>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/compiler.h>
#include <utils/Log.h>
#include <utils/ThreadLocal.h>

#include "driver/opengl/OpenGLDriver.h"

#include <android/api-level.h>
#include <android/ndk-version.h>
#include <sys/system_properties.h>

// We require filament to be built with a API 21 toolchain, before that, OpenGLES 3.0 didn't exist
#if __ANDROID_API__ < 21
#   error "__ANDROID_API__ must be at least 21"
#endif

#if __has_include(<android/hardware_buffer.h>)
#   include <android/hardware_buffer.h>
#endif

#if __has_include(<android/surface_texture.h>)
#   include <android/surface_texture.h>
#   include <android/surface_texture_jni.h>
#else
struct ASurfaceTexture;
typedef struct ASurfaceTexture ASurfaceTexture;
#endif

#if __ANDROID_API__ >= 26
#   define PLATFORM_HAS_HARDWAREBUFFER
#endif

using namespace utils;

namespace filament {
using namespace driver;

// The Android NDK doesn't exposes extensions, fake it with eglGetProcAddress
namespace glext {
UTILS_PRIVATE PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR;
UTILS_PRIVATE PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR;
UTILS_PRIVATE PFNEGLCLIENTWAITSYNCKHRPROC eglClientWaitSyncKHR;
UTILS_PRIVATE PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
UTILS_PRIVATE PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
UTILS_PRIVATE PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC eglGetNativeClientBufferANDROID;
UTILS_PRIVATE PFNEGLPRESENTATIONTIMEANDROIDPROC eglPresentationTimeANDROID;
}
using namespace glext;

using EGLStream = ExternalContext::Stream;

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

template <typename T>
static void loadSymbol(T*& pfn, const char *symbol) noexcept {
    pfn = (T*)dlsym(RTLD_DEFAULT, symbol);
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
// VirtualMachineEnv
// ---------------------------------------------------------------------------------------------

class VirtualMachineEnv {
public:
    static jint JNI_OnLoad(JavaVM* vm) noexcept;

    static VirtualMachineEnv& get() noexcept {
        // declaring this thread local, will ensure it's destroyed with the calling thread
        static UTILS_DECLARE_TLS(VirtualMachineEnv) instance;
        return instance;
    }

    static JNIEnv* getThreadEnvironment() noexcept {
        JNIEnv* env;
        assert(sVirtualMachine);
        if (sVirtualMachine->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
            return nullptr; // this should not happen
        }
        return env;
    }

    VirtualMachineEnv() noexcept : mVirtualMachine(sVirtualMachine) {
        // We're not initializing the JVM here -- but we could -- because most of the time
        // we don't need the jvm. Instead we do the initialization on first use. This means we could get
        // a nasty slow down the very first time, but we'll live with it for now.
    }

    ~VirtualMachineEnv() {
        if (mVirtualMachine) {
            mVirtualMachine->DetachCurrentThread();
        }
    }

    inline JNIEnv* getEnvironment() noexcept {
        assert(mVirtualMachine);
        JNIEnv* env = mJniEnv;
        if (UTILS_UNLIKELY(!env)) {
            return getEnvironmentSlow();
        }
        return env;
    }

    static void handleException(JNIEnv* const env) noexcept;

private:
    JNIEnv* getEnvironmentSlow() noexcept;
    static JavaVM* sVirtualMachine;
    JNIEnv* mJniEnv = nullptr;
    JavaVM* mVirtualMachine = nullptr;
};

JavaVM* VirtualMachineEnv::sVirtualMachine = nullptr;

// This is called when the library is loaded. We need this to get a reference to the global VM
UTILS_NOINLINE
jint VirtualMachineEnv::JNI_OnLoad(JavaVM* vm) noexcept {
    JNIEnv* env = nullptr;
    if (UTILS_UNLIKELY(vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)) {
        // this should not happen
        return -1;
    }
    sVirtualMachine = vm;
    return JNI_VERSION_1_6;
}

UTILS_NOINLINE
void VirtualMachineEnv::handleException(JNIEnv* const env) noexcept {
    if (UTILS_UNLIKELY(env->ExceptionCheck())) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

UTILS_NOINLINE
JNIEnv* VirtualMachineEnv::getEnvironmentSlow() noexcept {
    mVirtualMachine->AttachCurrentThread(&mJniEnv, nullptr);
    assert(mJniEnv);
    return  mJniEnv;
}

// ---------------------------------------------------------------------------------------------
// ExternalStreamManagerAndroid
// ---------------------------------------------------------------------------------------------

class ExternalStreamManagerAndroid {
public:
    ExternalStreamManagerAndroid() noexcept;
    static ExternalStreamManagerAndroid& get() noexcept;

    EGLStream* acquire(jobject surfaceTexture) noexcept;
    void release(EGLStream* stream) noexcept;
    void attach(EGLStream* stream, intptr_t tname) noexcept;
    void detach(EGLStream* stream) noexcept;
    void updateTexImage(EGLStream* stream) noexcept;

private:
    VirtualMachineEnv& mVm;
    JNIEnv* mJniEnv = nullptr;

    inline JNIEnv* getEnvironment() noexcept {
        JNIEnv* env = mJniEnv;
        if (UTILS_UNLIKELY(!env)) {
            return getEnvironmentSlow();
        }
        return env;
    }

    JNIEnv* getEnvironmentSlow() noexcept;

    jmethodID mSurfaceTextureClass_updateTexImage;
    jmethodID mSurfaceTextureClass_attachToGLContext;
    jmethodID mSurfaceTextureClass_detachFromGLContext;

    ASurfaceTexture* (*ASurfaceTexture_fromSurfaceTexture)(JNIEnv*, jobject);
    void (*ASurfaceTexture_release)(ASurfaceTexture*);
    int  (*ASurfaceTexture_attachToGLContext)(ASurfaceTexture*, uint32_t);
    int  (*ASurfaceTexture_detachFromGLContext)(ASurfaceTexture*);
    int  (*ASurfaceTexture_updateTexImage)(ASurfaceTexture*);
};

// ---------------------------------------------------------------------------------------------
// ExternalTextureManagerAndroid
// ---------------------------------------------------------------------------------------------

namespace ndk {
/*
 * This mimics the GraphicBuffer class for pre-API 26
 */
template <typename NATIVE_TYPE, typename REF>
struct ANativeObjectBase : public NATIVE_TYPE, public REF { };

struct android_native_base_t {
    unsigned int magic;
    unsigned int version;
    void* reserved[4];
    void (*incRef)(struct android_native_base_t* base);
    void (*decRef)(struct android_native_base_t* base);
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
struct GraphicBuffer : public ANativeObjectBase<ANativeWindowBuffer, RefBase> { };
struct GraphicBufferWrapper { GraphicBuffer* graphicBuffer; };
} // namespace ndk
using namespace ndk;

struct EGLExternalTexture : public ExternalContext::ExternalTexture {
    AHardwareBuffer* hardwareBuffer = nullptr;
    GraphicBufferWrapper* graphicBufferWrapper = nullptr;
};

class ExternalTextureManagerAndroid {
public:
    static ExternalTextureManagerAndroid& get() noexcept {
        static ExternalTextureManagerAndroid instance;
        return instance;
    }

    // called on gl thread
    ExternalTextureManagerAndroid() noexcept
            : mVm(VirtualMachineEnv::get()) {
        // if we compile for API 26 (Oreo) and above, we're guaranteed to have AHardwareBuffer
        // in all other cases, we need to get them at runtime.
#ifndef PLATFORM_HAS_HARDWAREBUFFER
        loadSymbol(AHardwareBuffer_allocate, "AHardwareBuffer_allocate");
        loadSymbol(AHardwareBuffer_release, "AHardwareBuffer_release");
#endif
    }

    // not quite sure on which thread this is going to be called
    ~ExternalTextureManagerAndroid() noexcept {
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
    EGLExternalTexture* create() noexcept {
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
        ets->image = (uintptr_t)EGL_NO_IMAGE_KHR;
        return ets;
    }

    // called on app thread
    void reallocate(EGLExternalTexture* ets, EGLDisplay dpy,
            uint32_t w, uint32_t h, TextureFormat format, uint64_t usage) noexcept {
        destroyStorage(ets, dpy);
        alloc(ets, dpy, w, h, format, usage);
    }

    // called on gl thread
    void destroy(EGLExternalTexture* ets, EGLDisplay dpy) noexcept {
        destroyStorage(ets, dpy);
        delete ets;
    }

private:
    // called on app thread
    void alloc(EGLExternalTexture* ets, EGLDisplay dpy,
            uint32_t w, uint32_t h, TextureFormat format, uint64_t usage) noexcept {
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

            EGLClientBuffer clientBuffer = eglGetNativeClientBufferANDROID(buffer);
            if (UTILS_UNLIKELY(!clientBuffer)) {
                logEglError("eglGetNativeClientBufferANDROID");
                return;
            }

            const EGLint attr[] = { EGL_NONE };
            EGLImageKHR image = eglCreateImageKHR(dpy,
                    EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, clientBuffer, attr);
            if (UTILS_UNLIKELY(!image)) {
                logEglError("eglCreateImageKHR");
                return;
            }

            ets->image = (uintptr_t)image;
            ets->hardwareBuffer = buffer;

#ifndef PLATFORM_HAS_HARDWAREBUFFER
        } else {
            // note: This is called on the application thread (not the GL thread)
            JNIEnv* env = VirtualMachineEnv::getThreadEnvironment();
            if (!env) {
                return; // this should not happen
            }

            jlong buffer = env->CallStaticLongMethod(mGraphicBufferClass,
                    mGraphicBuffer_nCreateGraphicBuffer, w, h, desc.format, usage);
            mVm.handleException(env);

            if (buffer) {
                GraphicBuffer* const gb = ((GraphicBufferWrapper*)buffer)->graphicBuffer;
                ANativeWindowBuffer* anwb = static_cast<ANativeWindowBuffer*>(gb);
                const EGLint attr[] = { EGL_NONE };
                EGLImageKHR image = eglCreateImageKHR(dpy,
                        EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID, (EGLClientBuffer)anwb, attr);
                if (UTILS_UNLIKELY(!image)) {
                    logEglError("eglCreateImageKHR");
                    return;
                }
                ets->image = (uintptr_t)image;
                ets->graphicBufferWrapper = (GraphicBufferWrapper*)buffer;
            }
        }
#endif
    }

    // called on gl thread
    void destroyStorage(EGLExternalTexture* ets, EGLDisplay dpy) noexcept {
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
                mVm.handleException(env);
                ets->graphicBufferWrapper = nullptr;
            }
        }
#endif

        if ((EGLImageKHR)ets->image != EGL_NO_IMAGE_KHR) {
            eglDestroyImageKHR(dpy, (EGLImageKHR)ets->image);
            ets->image = (uintptr_t)EGL_NO_IMAGE_KHR;
        }
    }

    VirtualMachineEnv& mVm;

#ifndef PLATFORM_HAS_HARDWAREBUFFER
    // if we compile for API 26 (Oreo) and above, we're guaranteed to have AHardwareBuffer
    // in all other cases, we need to get them at runtime.
    int (*AHardwareBuffer_allocate)(const AHardwareBuffer_Desc*, AHardwareBuffer**) = nullptr;
    void (*AHardwareBuffer_release)(AHardwareBuffer*) = nullptr;

    jclass mGraphicBufferClass = nullptr;
    jmethodID mGraphicBuffer_nCreateGraphicBuffer = nullptr;
    jmethodID mGraphicBuffer_nDestroyGraphicBuffer = nullptr;
#endif
};

// ---------------------------------------------------------------------------------------------

ContextManagerEGL::ContextManagerEGL() noexcept
        : mExternalStreamManager(ExternalStreamManagerAndroid::get()),
          mExternalTextureManager(ExternalTextureManagerAndroid::get()) {
}

std::unique_ptr<Driver> ContextManagerEGL::createDriver(void* const sharedGLContext) noexcept {
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

    auto extensions = split(eglQueryString(mEGLDisplay, EGL_EXTENSIONS));

    eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) eglGetProcAddress("eglCreateSyncKHR");
    eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) eglGetProcAddress("eglDestroySyncKHR");
    eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC) eglGetProcAddress("eglClientWaitSyncKHR");
    eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
    eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
    eglGetNativeClientBufferANDROID = (PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC) eglGetProcAddress("eglGetNativeClientBufferANDROID");
    eglPresentationTimeANDROID = (PFNEGLPRESENTATIONTIMEANDROIDPROC) eglGetProcAddress("eglPresentationTimeANDROID");

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
    if (!sharedGLContext && extensions.has("EGL_KHR_create_context_no_error")) {
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
      logEglError("eglChooseConfig failed to find opaque config with EGL_RECORDABLE_ANDROID.  Continuing without EGL_RECORDABLE_ANDROID.");
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
      logEglError("eglChooseConfig failed to find transparent config with EGL_RECORDABLE_ANDROID.  Continuing without EGL_RECORDABLE_ANDROID.");
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

    mEGLContext = eglCreateContext(mEGLDisplay, eglConfig, (EGLContext)sharedGLContext, contextAttribs);
    if (mEGLContext == EGL_NO_CONTEXT && sharedGLContext &&
        extensions.has("EGL_KHR_create_context_no_error")) {
        // context creation could fail because of EGL_CONTEXT_OPENGL_NO_ERROR_KHR
        // not matching the sharedContext. Try with it.
        contextAttribs[2] = EGL_CONTEXT_OPENGL_NO_ERROR_KHR;
        contextAttribs[3] = EGL_TRUE;
        mEGLContext = eglCreateContext(mEGLDisplay, eglConfig, (EGLContext)sharedGLContext, contextAttribs);
    }
    if (UTILS_UNLIKELY(mEGLContext == EGL_NO_CONTEXT)) {
        // eglCreateContext failed
        logEglError("eglCreateContext");
        goto error;
    }

    if (!makeCurrent(mEGLDummySurface)) {
        // eglMakeCurrent failed
        logEglError("eglMakeCurrent");
        goto error;
    }

    // success!!
    return OpenGLDriver::create(this, sharedGLContext);

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

EGLBoolean ContextManagerEGL::makeCurrent(EGLSurface surface) noexcept {
    if (UTILS_UNLIKELY((surface != mCurrentSurface))) {
        mCurrentSurface = surface;
        return eglMakeCurrent(mEGLDisplay, surface, surface, mEGLContext);
    }
    return EGL_TRUE;
}

void ContextManagerEGL::terminate() noexcept {
    eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(mEGLDisplay, mEGLDummySurface);
    eglDestroyContext(mEGLDisplay, mEGLContext);
    eglTerminate(mEGLDisplay);
    eglReleaseThread();
}

ExternalContext::SwapChain* ContextManagerEGL::createSwapChain(
        void* nativeWindow, uint64_t& flags) noexcept {
    EGLSurface sur = eglCreateWindowSurface(mEGLDisplay,
            (flags & driver::SWAP_CHAIN_CONFIG_TRANSPARENT) ? mEGLTransparentConfig : mEGLConfig,
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

void ContextManagerEGL::destroySwapChain(ExternalContext::SwapChain* swapChain) noexcept {
    EGLSurface sur = (EGLSurface) swapChain;
    if (sur != EGL_NO_SURFACE) {
        makeCurrent(mEGLDummySurface);
        eglDestroySurface(mEGLDisplay, sur);
    }
}

void ContextManagerEGL::makeCurrent(ExternalContext::SwapChain* swapChain) noexcept {
    EGLSurface sur = (EGLSurface) swapChain;
    if (sur != EGL_NO_SURFACE) {
        makeCurrent(sur);
    }
}

void ContextManagerEGL::setPresentationTime(long time) noexcept {
    if (mCurrentSurface != EGL_NO_SURFACE) {
      eglPresentationTimeANDROID(
                        mEGLDisplay,
                        mCurrentSurface,
                        time);
    }
}

void ContextManagerEGL::commit(ExternalContext::SwapChain* swapChain) noexcept {
    EGLSurface sur = (EGLSurface) swapChain;
    if (sur != EGL_NO_SURFACE) {
        eglSwapBuffers(mEGLDisplay, sur);
    }
}

ExternalContext::Fence* ContextManagerEGL::createFence() noexcept {
    Fence* f = nullptr;
#ifdef EGL_KHR_reusable_sync
    f = (Fence*) eglCreateSyncKHR(mEGLDisplay, EGL_SYNC_FENCE_KHR, nullptr);
#endif
    return f;
}

void ContextManagerEGL::destroyFence(ExternalContext::Fence* fence) noexcept {
#ifdef EGL_KHR_reusable_sync
    EGLSyncKHR sync = (EGLSyncKHR) fence;
    if (sync != EGL_NO_SYNC_KHR) {
        eglDestroySyncKHR(mEGLDisplay, sync);
    }
#endif
}

driver::FenceStatus ContextManagerEGL::waitFence(
        ExternalContext::Fence* fence, uint64_t timeout) noexcept {
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

ExternalContext::Stream* ContextManagerEGL::createStream(void* nativeStream) noexcept {
    return  mExternalStreamManager.acquire(static_cast<jobject>(nativeStream));
}

void ContextManagerEGL::destroyStream(ExternalContext::Stream* stream) noexcept {
    mExternalStreamManager.release(static_cast<EGLStream*>(stream));
}

void ContextManagerEGL::attach(Stream* stream, intptr_t tname) noexcept {
    mExternalStreamManager.attach(static_cast<EGLStream*>(stream), tname);
}

void ContextManagerEGL::detach(Stream* stream) noexcept {
    mExternalStreamManager.detach(static_cast<EGLStream*>(stream));
}

void ContextManagerEGL::updateTexImage(Stream* stream) noexcept {
    mExternalStreamManager.updateTexImage(static_cast<EGLStream*>(stream));
}

ExternalContext::ExternalTexture* ContextManagerEGL::createExternalTextureStorage() noexcept {
    return mExternalTextureManager.create();
}

void ContextManagerEGL::reallocateExternalStorage(
        ExternalContext::ExternalTexture* externalTexture,
        uint32_t w, uint32_t h, driver::TextureFormat format) noexcept {
    if (externalTexture) {
        EGLExternalTexture* ets = static_cast<EGLExternalTexture*>(externalTexture);
        mExternalTextureManager.reallocate(ets, mEGLDisplay, w, h, format,
                AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE);
    }
}

void ContextManagerEGL::destroyExternalTextureStorage(
        ExternalContext::ExternalTexture* externalTexture) noexcept {
    if (externalTexture) {
        EGLExternalTexture* ets = static_cast<EGLExternalTexture*>(externalTexture);
        mExternalTextureManager.destroy(ets, mEGLDisplay);
    }
}

int ContextManagerEGL::getOSVersion() const noexcept {
    return mOSVersion;
}

// ---------------------------------------------------------------------------------------------
// class ExternalStreamManagerAndroid
// ---------------------------------------------------------------------------------------------

ExternalStreamManagerAndroid& ExternalStreamManagerAndroid::get() noexcept {
    // declaring this thread local, will ensure it's destroyed with the calling thread
    static UTILS_DECLARE_TLS(ExternalStreamManagerAndroid) instance;
    return instance;
}

ExternalStreamManagerAndroid::ExternalStreamManagerAndroid() noexcept
    : mVm(VirtualMachineEnv::get()) {
    // We're not initializing the JVM here -- but we could -- because most of the time
    // we don't need the jvm. Instead we do the initialization on first use. This means we could get
    // a nasty slow down the very first time, but we'll live with it for now.

    loadSymbol(ASurfaceTexture_fromSurfaceTexture,  "ASurfaceTexture_fromSurfaceTexture");
    loadSymbol(ASurfaceTexture_release,             "ASurfaceTexture_release");
    loadSymbol(ASurfaceTexture_attachToGLContext,   "ASurfaceTexture_attachToGLContext");
    loadSymbol(ASurfaceTexture_detachFromGLContext, "ASurfaceTexture_detachFromGLContext");
    loadSymbol(ASurfaceTexture_updateTexImage,      "ASurfaceTexture_updateTexImage");
    if (ASurfaceTexture_fromSurfaceTexture) {
        slog.d << "Using ASurfaceTexture" << io::endl;
    }
}


UTILS_NOINLINE
JNIEnv* ExternalStreamManagerAndroid::getEnvironmentSlow() noexcept {
    JNIEnv * env = mVm.getEnvironment();
    mJniEnv = env;

    // we try to call directly the native version of updateTexImage() -- it's not 100% portable
    // but we mitigate this by falling back on the correct method if we don't find it.
    jclass SurfaceTextureClass = env->FindClass("android/graphics/SurfaceTexture");
    mSurfaceTextureClass_updateTexImage = env->GetMethodID(
            SurfaceTextureClass, "nativeUpdateTexImage", "()V");
    if (!mSurfaceTextureClass_updateTexImage) {
        mSurfaceTextureClass_updateTexImage = env->GetMethodID(
                SurfaceTextureClass, "updateTexImage", "()V");
    }

    mSurfaceTextureClass_attachToGLContext = env->GetMethodID(
            SurfaceTextureClass, "attachToGLContext", "(I)V");

    mSurfaceTextureClass_detachFromGLContext = env->GetMethodID(
            SurfaceTextureClass, "detachFromGLContext", "()V");

    return env;
}

static ASurfaceTexture* ASurfaceTexture_cast(EGLStream* stream) noexcept {
    return reinterpret_cast<ASurfaceTexture*>(static_cast<void*>(stream));
}

static jobject jobject_cast(EGLStream* stream) noexcept {
    return reinterpret_cast<jobject>(static_cast<void*>(stream));
}

EGLStream* ExternalStreamManagerAndroid::acquire(jobject surfaceTexture) noexcept {
    // note: This is called on the application thread (not the GL thread)
    JNIEnv* env = VirtualMachineEnv::getThreadEnvironment();
    if (!env) {
        return nullptr; // this should not happen
    }
    EGLStream* stream;
    if (ASurfaceTexture_fromSurfaceTexture) {
        stream = reinterpret_cast<EGLStream*>(ASurfaceTexture_fromSurfaceTexture(env, surfaceTexture));
    } else {
        stream = reinterpret_cast<EGLStream*>(env->NewGlobalRef(surfaceTexture));
    }
    return stream;
}

void ExternalStreamManagerAndroid::release(EGLStream* stream) noexcept {
    if (ASurfaceTexture_fromSurfaceTexture) {
        ASurfaceTexture_release(ASurfaceTexture_cast(stream));
    } else {
        JNIEnv* const env = getEnvironment();
        assert(env); // we should have called attach() by now
        env->DeleteGlobalRef(jobject_cast(stream));
    }
}

void ExternalStreamManagerAndroid::attach(EGLStream* stream, intptr_t tname) noexcept {
    if (ASurfaceTexture_fromSurfaceTexture) {
        // associate our GL texture to the SurfaceTexture
        ASurfaceTexture* const aSurfaceTexture = ASurfaceTexture_cast(stream);
        if (UTILS_UNLIKELY(ASurfaceTexture_attachToGLContext(aSurfaceTexture, (uint32_t)tname))) {
            // Unfortunately, before API 26 SurfaceTexture was always created in attached mode,
            // so attachToGLContext can fail. We consider this the unlikely case, because
            // this is how it should work.
            // So now we have to detach the surfacetexture from its texture
            ASurfaceTexture_detachFromGLContext(aSurfaceTexture);
            // and finally, try attaching again
            ASurfaceTexture_attachToGLContext(aSurfaceTexture, (uint32_t)tname);
        }
    } else {
        JNIEnv* const env = getEnvironment();
        assert(env); // we should have called attach() by now

        // associate our GL texture to the SurfaceTexture
        jobject const jSurfaceTexture = jobject_cast(stream);
        env->CallVoidMethod(jSurfaceTexture, mSurfaceTextureClass_attachToGLContext, (jint)tname);
        if (UTILS_UNLIKELY(env->ExceptionCheck())) {
            // Unfortunately, before API 26 SurfaceTexture was always created in attached mode,
            // so attachToGLContext can fail. We consider this the unlikely case, because
            // this is how it should work.
            env->ExceptionClear();

            // so now we have to detach the surfacetexture from its texture
            env->CallVoidMethod(jSurfaceTexture, mSurfaceTextureClass_detachFromGLContext);
            mVm.handleException(env);

            // and finally, try attaching again
            env->CallVoidMethod(jSurfaceTexture, mSurfaceTextureClass_attachToGLContext, (jint)tname);
            mVm.handleException(env);
        }
    }
}

void ExternalStreamManagerAndroid::detach(EGLStream* stream) noexcept {
    if (ASurfaceTexture_fromSurfaceTexture) {
        ASurfaceTexture_detachFromGLContext(ASurfaceTexture_cast(stream));
    } else {
        JNIEnv* const env = mVm.getEnvironment();
        assert(env); // we should have called attach() by now
        env->CallVoidMethod(jobject_cast(stream), mSurfaceTextureClass_detachFromGLContext);
        mVm.handleException(env);
    }
}

void ExternalStreamManagerAndroid::updateTexImage(EGLStream* stream) noexcept {
    if (ASurfaceTexture_fromSurfaceTexture) {
        ASurfaceTexture_updateTexImage(ASurfaceTexture_cast(stream));
    } else {
        JNIEnv* const env = mVm.getEnvironment();
        assert(env); // we should have called attach() by now
        env->CallVoidMethod(jobject_cast(stream), mSurfaceTextureClass_updateTexImage);
        mVm.handleException(env);
    }
}

// This must called when the library is loaded. We need this to get a reference to the global VM
void JNI_OnLoad(JavaVM* vm, void* reserved) {
    ::filament::VirtualMachineEnv::JNI_OnLoad(vm);
}

} // namespace filament

// ---------------------------------------------------------------------------------------------
