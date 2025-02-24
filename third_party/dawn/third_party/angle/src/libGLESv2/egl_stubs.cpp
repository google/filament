//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// egl_stubs.cpp: Stubs for EGL entry points.
//

#include "libGLESv2/egl_stubs_autogen.h"

#include "common/angle_version_info.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/EGLSync.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Thread.h"
#include "libANGLE/queryutils.h"
#include "libANGLE/validationEGL.h"
#include "libGLESv2/global_state.h"

namespace egl
{
namespace
{

void ClipConfigs(const std::vector<const Config *> &filteredConfigs,
                 EGLConfig *outputConfigs,
                 EGLint configSize,
                 EGLint *numConfigs)
{
    EGLint resultSize = static_cast<EGLint>(filteredConfigs.size());
    if (outputConfigs)
    {
        resultSize = std::max(std::min(resultSize, configSize), 0);
        for (EGLint i = 0; i < resultSize; i++)
        {
            outputConfigs[i] = const_cast<Config *>(filteredConfigs[i]);
        }
    }
    *numConfigs = resultSize;
}
}  // anonymous namespace

EGLBoolean BindAPI(Thread *thread, EGLenum api)
{
    thread->setAPI(api);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean BindTexImage(Thread *thread, Display *display, egl::SurfaceID surfaceID, EGLint buffer)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglBindTexImage",
                                          GetDisplayIfValid(display), EGL_FALSE);

    gl::Context *context = thread->getContext();
    if (context && !context->isContextLost())
    {
        gl::TextureType type =
            egl_gl::EGLTextureTargetToTextureType(eglSurface->getTextureTarget());
        gl::Texture *textureObject = context->getTextureByType(type);
        ANGLE_EGL_TRY_RETURN(thread, eglSurface->bindTexImage(context, textureObject, buffer),
                             "eglBindTexImage", GetSurfaceIfValid(display, surfaceID), EGL_FALSE);
    }

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean ChooseConfig(Thread *thread,
                        Display *display,
                        const AttributeMap &attribMap,
                        EGLConfig *configs,
                        EGLint config_size,
                        EGLint *num_config)
{
    ClipConfigs(display->chooseConfig(attribMap), configs, config_size, num_config);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLint ClientWaitSync(Thread *thread,
                      Display *display,
                      SyncID syncID,
                      EGLint flags,
                      EGLTime timeout)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglClientWaitSync",
                                          GetDisplayIfValid(display), EGL_FALSE);
    gl::Context *currentContext = thread->getContext();
    EGLint syncStatus           = EGL_FALSE;
    Sync *syncObject            = display->getSync(syncID);
    ANGLE_EGL_TRY_RETURN(
        thread, syncObject->clientWait(display, currentContext, flags, timeout, &syncStatus),
        "eglClientWaitSync", GetSyncIfValid(display, syncID), EGL_FALSE);

    // When performing CPU wait through UnlockedTailCall we need to handle any error conditions
    if (egl::Display::GetCurrentThreadUnlockedTailCall()->any())
    {
        auto handleErrorStatus = [thread, display, syncID](void *result) {
            EGLint *eglResult = static_cast<EGLint *>(result);
            ASSERT(eglResult);
            if (*eglResult == EGL_FALSE)
            {
                thread->setError(egl::Error(EGL_BAD_ALLOC), "eglClientWaitSync",
                                 GetSyncIfValid(display, syncID));
            }
            else
            {
                thread->setSuccess();
            }
        };
        egl::Display::GetCurrentThreadUnlockedTailCall()->add(handleErrorStatus);
    }
    else
    {
        thread->setSuccess();
    }
    return syncStatus;
}

EGLBoolean CopyBuffers(Thread *thread,
                       Display *display,
                       egl::SurfaceID surfaceID,
                       EGLNativePixmapType target)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglCopyBuffers",
                                          GetDisplayIfValid(display), EGL_FALSE);
    UNIMPLEMENTED();  // FIXME

    thread->setSuccess();
    return 0;
}

EGLContext CreateContext(Thread *thread,
                         Display *display,
                         Config *configuration,
                         gl::ContextID sharedContextID,
                         const AttributeMap &attributes)
{
    gl::Context *sharedGLContext = display->getContext(sharedContextID);
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglCreateContext",
                                          GetDisplayIfValid(display), EGL_NO_CONTEXT);
    gl::Context *context = nullptr;
    ANGLE_EGL_TRY_RETURN(
        thread, display->createContext(configuration, sharedGLContext, attributes, &context),
        "eglCreateContext", GetDisplayIfValid(display), EGL_NO_CONTEXT);

    thread->setSuccess();
    return reinterpret_cast<EGLContext>(static_cast<uintptr_t>(context->id().value));
}

EGLImage CreateImage(Thread *thread,
                     Display *display,
                     gl::ContextID contextID,
                     EGLenum target,
                     EGLClientBuffer buffer,
                     const AttributeMap &attributes)
{
    gl::Context *context = display->getContext(contextID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglCreateImage",
                                          GetDisplayIfValid(display), EGL_FALSE);

    Image *image = nullptr;
    Error error  = display->createImage(context, target, buffer, attributes, &image);
    if (error.isError())
    {
        thread->setError(error, "eglCreateImage", GetDisplayIfValid(display));
        return EGL_NO_IMAGE;
    }

    thread->setSuccess();
    return reinterpret_cast<EGLImage>(static_cast<uintptr_t>(image->id().value));
}

EGLSurface CreatePbufferFromClientBuffer(Thread *thread,
                                         Display *display,
                                         EGLenum buftype,
                                         EGLClientBuffer buffer,
                                         Config *configuration,
                                         const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCreatePbufferFromClientBuffer",
                                          GetDisplayIfValid(display), EGL_NO_SURFACE);
    Surface *surface = nullptr;
    ANGLE_EGL_TRY_RETURN(thread,
                         display->createPbufferFromClientBuffer(configuration, buftype, buffer,
                                                                attributes, &surface),
                         "eglCreatePbufferFromClientBuffer", GetDisplayIfValid(display),
                         EGL_NO_SURFACE);

    return reinterpret_cast<EGLSurface>(static_cast<uintptr_t>(surface->id().value));
}

EGLSurface CreatePbufferSurface(Thread *thread,
                                Display *display,
                                Config *configuration,
                                const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCreatePbufferSurface", GetDisplayIfValid(display),
                                          EGL_NO_SURFACE);
    Surface *surface = nullptr;
    ANGLE_EGL_TRY_RETURN(thread, display->createPbufferSurface(configuration, attributes, &surface),
                         "eglCreatePbufferSurface", GetDisplayIfValid(display), EGL_NO_SURFACE);

    return reinterpret_cast<EGLSurface>(static_cast<uintptr_t>(surface->id().value));
}

EGLSurface CreatePixmapSurface(Thread *thread,
                               Display *display,
                               Config *configuration,
                               EGLNativePixmapType pixmap,
                               const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCreatePixmapSurface", GetDisplayIfValid(display),
                                          EGL_NO_SURFACE);
    Surface *surface = nullptr;
    ANGLE_EGL_TRY_RETURN(thread,
                         display->createPixmapSurface(configuration, pixmap, attributes, &surface),
                         "eglCreatePixmapSurface", GetDisplayIfValid(display), EGL_NO_SURFACE);

    thread->setSuccess();
    return reinterpret_cast<EGLSurface>(static_cast<uintptr_t>(surface->id().value));
}

EGLSurface CreatePlatformPixmapSurface(Thread *thread,
                                       Display *display,
                                       Config *configuration,
                                       void *pixmap,
                                       const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCreatePlatformPixmapSurface",
                                          GetDisplayIfValid(display), EGL_NO_SURFACE);
    Surface *surface                 = nullptr;
    EGLNativePixmapType nativePixmap = reinterpret_cast<EGLNativePixmapType>(pixmap);
    ANGLE_EGL_TRY_RETURN(
        thread, display->createPixmapSurface(configuration, nativePixmap, attributes, &surface),
        "eglCreatePlatformPixmapSurface", GetDisplayIfValid(display), EGL_NO_SURFACE);

    thread->setSuccess();
    return reinterpret_cast<EGLSurface>(static_cast<uintptr_t>(surface->id().value));
}

EGLSurface CreatePlatformWindowSurface(Thread *thread,
                                       Display *display,
                                       Config *configuration,
                                       void *win,
                                       const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCreatePlatformWindowSurface",
                                          GetDisplayIfValid(display), EGL_NO_SURFACE);
    Surface *surface                 = nullptr;
    EGLNativeWindowType nativeWindow = reinterpret_cast<EGLNativeWindowType>(win);
    ANGLE_EGL_TRY_RETURN(
        thread, display->createWindowSurface(configuration, nativeWindow, attributes, &surface),
        "eglCreatePlatformWindowSurface", GetDisplayIfValid(display), EGL_NO_SURFACE);

    return reinterpret_cast<EGLSurface>(static_cast<uintptr_t>(surface->id().value));
}

EGLSync CreateSync(Thread *thread, Display *display, EGLenum type, const AttributeMap &attributes)
{
    gl::Context *currentContext = thread->getContext();

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglCreateSync",
                                          GetDisplayIfValid(display), EGL_FALSE);
    Sync *syncObject = nullptr;
    ANGLE_EGL_TRY_RETURN(thread, display->createSync(currentContext, type, attributes, &syncObject),
                         "eglCreateSync", GetDisplayIfValid(display), EGL_NO_SYNC);

    thread->setSuccess();
    return reinterpret_cast<EGLSync>(static_cast<uintptr_t>(syncObject->id().value));
}

EGLSurface CreateWindowSurface(Thread *thread,
                               Display *display,
                               Config *configuration,
                               EGLNativeWindowType win,
                               const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCreateWindowSurface", GetDisplayIfValid(display),
                                          EGL_NO_SURFACE);

    Surface *surface = nullptr;
    ANGLE_EGL_TRY_RETURN(thread,
                         display->createWindowSurface(configuration, win, attributes, &surface),
                         "eglCreateWindowSurface", GetDisplayIfValid(display), EGL_NO_SURFACE);

    return reinterpret_cast<EGLSurface>(static_cast<uintptr_t>(surface->id().value));
}

EGLBoolean DestroyContext(Thread *thread, Display *display, gl::ContextID contextID)
{
    gl::Context *context = display->getContext(contextID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglDestroyContext",
                                          GetDisplayIfValid(display), EGL_FALSE);

    ScopedSyncCurrentContextFromThread scopedSyncCurrent(thread);

    ANGLE_EGL_TRY_RETURN(thread, display->destroyContext(thread, context), "eglDestroyContext",
                         GetContextIfValid(display, contextID), EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean DestroyImage(Thread *thread, Display *display, ImageID imageID)
{
    Image *img = display->getImage(imageID);
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglDestroyImage",
                                          GetDisplayIfValid(display), EGL_FALSE);
    display->destroyImage(img);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean DestroySurface(Thread *thread, Display *display, egl::SurfaceID surfaceID)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    // Workaround https://issuetracker.google.com/292285899
    // When destroying surface, if the surface
    // is still bound by the context of the current rendering
    // thread, release the surface by passing EGL_NO_SURFACE to eglMakeCurrent().
    if (display->getFrontendFeatures().uncurrentEglSurfaceUponSurfaceDestroy.enabled &&
        eglSurface->isCurrentOnAnyContext() &&
        (thread->getCurrentDrawSurface() == eglSurface ||
         thread->getCurrentReadSurface() == eglSurface))
    {
        SurfaceID drawSurface             = PackParam<SurfaceID>(EGL_NO_SURFACE);
        SurfaceID readSurface             = PackParam<SurfaceID>(EGL_NO_SURFACE);
        const gl::Context *currentContext = thread->getContext();
        const gl::ContextID contextID     = currentContext == nullptr
                                                ? PackParam<gl::ContextID>(EGL_NO_CONTEXT)
                                                : currentContext->id();

        // if surfaceless context is supported, only release the surface.
        if (display->getExtensions().surfacelessContext)
        {
            MakeCurrent(thread, display, drawSurface, readSurface, contextID);
        }
        else
        {
            // if surfaceless context is not supported, release the context, too.
            MakeCurrent(thread, display, drawSurface, readSurface,
                        PackParam<gl::ContextID>(EGL_NO_CONTEXT));
        }
    }

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglDestroySurface",
                                          GetDisplayIfValid(display), EGL_FALSE);

    ANGLE_EGL_TRY_RETURN(thread, display->destroySurface(eglSurface), "eglDestroySurface",
                         GetSurfaceIfValid(display, surfaceID), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean DestroySync(Thread *thread, Display *display, SyncID syncID)
{
    Sync *sync = display->getSync(syncID);
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglDestroySync",
                                          GetDisplayIfValid(display), EGL_FALSE);
    display->destroySync(sync);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean GetConfigAttrib(Thread *thread,
                           Display *display,
                           Config *configuration,
                           EGLint attribute,
                           EGLint *value)
{
    QueryConfigAttrib(configuration, attribute, value);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean GetConfigs(Thread *thread,
                      Display *display,
                      EGLConfig *configs,
                      EGLint config_size,
                      EGLint *num_config)
{
    ClipConfigs(display->getConfigs(AttributeMap()), configs, config_size, num_config);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLContext GetCurrentContext(Thread *thread)
{
    gl::Context *context = thread->getContext();

    thread->setSuccess();
    return reinterpret_cast<EGLContext>(context ? static_cast<uintptr_t>(context->id().value) : 0);
}

EGLDisplay GetCurrentDisplay(Thread *thread)
{
    thread->setSuccess();
    if (thread->getContext() != nullptr)
    {
        return thread->getContext()->getDisplay();
    }
    return EGL_NO_DISPLAY;
}

EGLSurface GetCurrentSurface(Thread *thread, EGLint readdraw)
{
    Surface *surface =
        (readdraw == EGL_READ) ? thread->getCurrentReadSurface() : thread->getCurrentDrawSurface();
    thread->setSuccess();
    if (surface)
    {
        return reinterpret_cast<EGLSurface>(static_cast<uintptr_t>(surface->id().value));
    }
    else
    {
        return EGL_NO_SURFACE;
    }
}

EGLDisplay GetDisplay(Thread *thread, EGLNativeDisplayType display_id)
{
    return Display::GetDisplayFromNativeDisplay(EGL_PLATFORM_ANGLE_ANGLE, display_id,
                                                AttributeMap());
}

EGLint GetError(Thread *thread)
{
    EGLint error = thread->getError();
    thread->setSuccess();
    return error;
}

EGLDisplay GetPlatformDisplay(Thread *thread,
                              EGLenum platform,
                              void *native_display,
                              const AttributeMap &attribMap)
{
    switch (platform)
    {
        case EGL_PLATFORM_ANGLE_ANGLE:
        case EGL_PLATFORM_GBM_KHR:
        case EGL_PLATFORM_WAYLAND_EXT:
        case EGL_PLATFORM_SURFACELESS_MESA:
        {
            return Display::GetDisplayFromNativeDisplay(
                platform, gl::bitCast<EGLNativeDisplayType>(native_display), attribMap);
        }
        case EGL_PLATFORM_DEVICE_EXT:
        {
            Device *eglDevice = static_cast<Device *>(native_display);
            return Display::GetDisplayFromDevice(eglDevice, attribMap);
        }
        default:
        {
            UNREACHABLE();
            return EGL_NO_DISPLAY;
        }
    }
}

EGLBoolean GetSyncAttrib(Thread *thread,
                         Display *display,
                         SyncID syncID,
                         EGLint attribute,
                         EGLAttrib *value)
{
    EGLint valueExt;
    ANGLE_EGL_TRY_RETURN(thread, GetSyncAttrib(display, syncID, attribute, &valueExt),
                         "eglGetSyncAttrib", GetSyncIfValid(display, syncID), EGL_FALSE);
    *value = valueExt;

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean Initialize(Thread *thread, Display *display, EGLint *major, EGLint *minor)
{
    ANGLE_EGL_TRY_RETURN(thread, display->initialize(), "eglInitialize", GetDisplayIfValid(display),
                         EGL_FALSE);

    if (major)
    {
        *major = kEglMajorVersion;
    }
    if (minor)
    {
        *minor = kEglMinorVersion;
    }

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean MakeCurrent(Thread *thread,
                       Display *display,
                       egl::SurfaceID drawSurfaceID,
                       egl::SurfaceID readSurfaceID,
                       gl::ContextID contextID)
{
    Surface *drawSurface = display->getSurface(drawSurfaceID);
    Surface *readSurface = display->getSurface(readSurfaceID);
    gl::Context *context = display->getContext(contextID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglMakeCurrent",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ScopedSyncCurrentContextFromThread scopedSyncCurrent(thread);

    Surface *previousDraw        = thread->getCurrentDrawSurface();
    Surface *previousRead        = thread->getCurrentReadSurface();
    gl::Context *previousContext = thread->getContext();

    // Only call makeCurrent if the context or surfaces have changed.
    if (previousDraw != drawSurface || previousRead != readSurface || previousContext != context)
    {
        ANGLE_EGL_TRY_RETURN(
            thread,
            display->makeCurrent(thread, previousContext, drawSurface, readSurface, context),
            "eglMakeCurrent", GetContextIfValid(display, contextID), EGL_FALSE);
    }

    thread->setSuccess();
    return EGL_TRUE;
}

EGLenum QueryAPI(Thread *thread)
{
    EGLenum API = thread->getAPI();

    thread->setSuccess();
    return API;
}

EGLBoolean QueryContext(Thread *thread,
                        Display *display,
                        gl::ContextID contextID,
                        EGLint attribute,
                        EGLint *value)
{
    gl::Context *context = display->getContext(contextID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglQueryContext",
                                          GetDisplayIfValid(display), EGL_FALSE);
    QueryContextAttrib(context, attribute, value);

    thread->setSuccess();
    return EGL_TRUE;
}

const char *QueryString(Thread *thread, Display *display, EGLint name)
{
    if (display)
    {
        ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglQueryString",
                                              GetDisplayIfValid(display), nullptr);
    }

    const char *result = nullptr;
    switch (name)
    {
        case EGL_CLIENT_APIS:
            result = display->getClientAPIString().c_str();
            break;
        case EGL_EXTENSIONS:
            if (display == EGL_NO_DISPLAY)
            {
                result = Display::GetClientExtensionString().c_str();
            }
            else
            {
                result = display->getExtensionString().c_str();
            }
            break;
        case EGL_VENDOR:
            result = display->getVendorString().c_str();
            break;
        case EGL_VERSION:
        {
            static const char *sVersionString =
                MakeStaticString(std::string("1.5 (ANGLE ") + angle::GetANGLEVersionString() + ")");
            result = sVersionString;
            break;
        }
        default:
            UNREACHABLE();
            break;
    }

    thread->setSuccess();
    return result;
}

EGLBoolean QuerySurface(Thread *thread,
                        Display *display,
                        egl::SurfaceID surfaceID,
                        EGLint attribute,
                        EGLint *value)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglQuerySurface",
                                          GetDisplayIfValid(display), EGL_FALSE);

    // Update GetContextLock_QuerySurface() switch accordingly to take a ContextMutex lock for
    // attributes that require current Context.
    const gl::Context *context;
    switch (attribute)
    {
        // EGL_BUFFER_AGE_EXT uses Context, so lock was taken in GetContextLock_QuerySurface().
        case EGL_BUFFER_AGE_EXT:
            context = thread->getContext();
            break;
        // Other attributes are not using Context, pass nullptr to be explicit about that.
        default:
            context = nullptr;
            break;
    }

    ANGLE_EGL_TRY_RETURN(thread, QuerySurfaceAttrib(display, context, eglSurface, attribute, value),
                         "eglQuerySurface", GetSurfaceIfValid(display, surfaceID), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean ReleaseTexImage(Thread *thread,
                           Display *display,
                           egl::SurfaceID surfaceID,
                           EGLint buffer)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglReleaseTexImage",
                                          GetDisplayIfValid(display), EGL_FALSE);
    gl::Context *context = thread->getContext();
    if (context && !context->isContextLost())
    {
        gl::Texture *texture = eglSurface->getBoundTexture();

        if (texture)
        {
            ANGLE_EGL_TRY_RETURN(thread, eglSurface->releaseTexImage(thread->getContext(), buffer),
                                 "eglReleaseTexImage", GetSurfaceIfValid(display, surfaceID),
                                 EGL_FALSE);
        }
    }
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean ReleaseThread(Thread *thread)
{
    ScopedSyncCurrentContextFromThread scopedSyncCurrent(thread);

    Surface *previousDraw        = thread->getCurrentDrawSurface();
    Surface *previousRead        = thread->getCurrentReadSurface();
    gl::Context *previousContext = thread->getContext();
    Display *previousDisplay     = thread->getDisplay();

    if (previousDisplay != EGL_NO_DISPLAY)
    {
        ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, previousDisplay->prepareForCall(),
                                              "eglReleaseThread",
                                              GetDisplayIfValid(previousDisplay), EGL_FALSE);
        // Only call makeCurrent if the context or surfaces have changed.
        if (previousDraw != EGL_NO_SURFACE || previousRead != EGL_NO_SURFACE ||
            previousContext != EGL_NO_CONTEXT)
        {
            ANGLE_EGL_TRY_RETURN(
                thread,
                previousDisplay->makeCurrent(thread, previousContext, nullptr, nullptr, nullptr),
                "eglReleaseThread", nullptr, EGL_FALSE);
        }
        ANGLE_EGL_TRY_RETURN(thread, previousDisplay->releaseThread(), "eglReleaseThread",
                             GetDisplayIfValid(previousDisplay), EGL_FALSE);
    }

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean SurfaceAttrib(Thread *thread,
                         Display *display,
                         egl::SurfaceID surfaceID,
                         EGLint attribute,
                         EGLint value)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglSurfaceAttrib",
                                          GetDisplayIfValid(display), EGL_FALSE);

    ANGLE_EGL_TRY_RETURN(thread, SetSurfaceAttrib(eglSurface, attribute, value), "eglSurfaceAttrib",
                         GetDisplayIfValid(display), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean SwapBuffers(Thread *thread, Display *display, egl::SurfaceID surfaceID)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglSwapBuffers",
                                          GetDisplayIfValid(display), EGL_FALSE);

    ANGLE_EGL_TRY_RETURN(thread, eglSurface->swap(thread->getContext()), "eglSwapBuffers",
                         GetSurfaceIfValid(display, surfaceID), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean SwapInterval(Thread *thread, Display *display, EGLint interval)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglSwapInterval",
                                          GetDisplayIfValid(display), EGL_FALSE);

    Surface *drawSurface        = static_cast<Surface *>(thread->getCurrentDrawSurface());
    const Config *surfaceConfig = drawSurface->getConfig();
    EGLint clampedInterval      = std::min(std::max(interval, surfaceConfig->minSwapInterval),
                                           surfaceConfig->maxSwapInterval);

    drawSurface->setRequestedSwapInterval(clampedInterval);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean Terminate(Thread *thread, Display *display)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglTerminate",
                                          GetDisplayIfValid(display), EGL_FALSE);

    ScopedSyncCurrentContextFromThread scopedSyncCurrent(thread);

    ANGLE_EGL_TRY_RETURN(thread, display->terminate(thread, Display::TerminateReason::Api),
                         "eglTerminate", GetDisplayIfValid(display), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean WaitClient(Thread *thread)
{
    Display *display = thread->getDisplay();
    if (display == nullptr)
    {
        // EGL spec says this about eglWaitClient -
        //    If there is no current context for the current rendering API,
        //    the function has no effect but still returns EGL_TRUE.
        return EGL_TRUE;
    }

    gl::Context *context = thread->getContext();

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglWaitClient",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, display->waitClient(context), "eglWaitClient",
                         GetContextIfValid(display, context->id()), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean WaitGL(Thread *thread)
{
    Display *display = thread->getDisplay();
    if (display == nullptr)
    {
        // EGL spec says this about eglWaitGL -
        //    eglWaitGL is ignored if there is no current EGL rendering context for OpenGL ES.
        return EGL_TRUE;
    }

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglWaitGL",
                                          GetDisplayIfValid(display), EGL_FALSE);

    // eglWaitGL like calling eglWaitClient with the OpenGL ES API bound. Since we only implement
    // OpenGL ES we can do the call directly.
    ANGLE_EGL_TRY_RETURN(thread, display->waitClient(thread->getContext()), "eglWaitGL",
                         GetDisplayIfValid(display), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean WaitNative(Thread *thread, EGLint engine)
{
    Display *display = thread->getDisplay();
    if (display == nullptr)
    {
        // EGL spec says this about eglWaitNative -
        //    eglWaitNative is ignored if there is no current EGL rendering context.
        return EGL_TRUE;
    }

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglWaitNative",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, display->waitNative(thread->getContext(), engine), "eglWaitNative",
                         GetThreadIfValid(thread), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean WaitSync(Thread *thread, Display *display, SyncID syncID, EGLint flags)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglWaitSync",
                                          GetDisplayIfValid(display), EGL_FALSE);
    gl::Context *currentContext = thread->getContext();
    Sync *syncObject            = display->getSync(syncID);
    ANGLE_EGL_TRY_RETURN(thread, syncObject->serverWait(display, currentContext, flags),
                         "eglWaitSync", GetSyncIfValid(display, syncID), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}
}  // namespace egl
