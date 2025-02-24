//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// egl_ext_stubs.cpp: Stubs for EXT extension entry points.
//

#include "libGLESv2/egl_ext_stubs_autogen.h"

#include "libANGLE/Device.h"
#include "libANGLE/Display.h"
#include "libANGLE/EGLSync.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Thread.h"
#include "libANGLE/entry_points_utils.h"
#include "libANGLE/queryutils.h"
#include "libANGLE/renderer/DisplayImpl.h"
#include "libANGLE/validationEGL.h"
#include "libANGLE/validationEGL_autogen.h"
#include "libGLESv2/global_state.h"

namespace egl
{
EGLint ClientWaitSyncKHR(Thread *thread,
                         Display *display,
                         SyncID syncID,
                         EGLint flags,
                         EGLTimeKHR timeout)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglClientWaitSyncKHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    gl::Context *currentContext = thread->getContext();
    EGLint syncStatus           = EGL_FALSE;
    Sync *sync                  = display->getSync(syncID);
    ANGLE_EGL_TRY_RETURN(thread,
                         sync->clientWait(display, currentContext, flags, timeout, &syncStatus),
                         "eglClientWaitSyncKHR", GetSyncIfValid(display, syncID), EGL_FALSE);

    // When performing CPU wait through UnlockedTailCall we need to handle any error conditions
    if (egl::Display::GetCurrentThreadUnlockedTailCall()->any())
    {
        auto handleErrorStatus = [thread, display, syncID](void *result) {
            EGLint *eglResult = static_cast<EGLint *>(result);
            ASSERT(eglResult);
            if (*eglResult == EGL_FALSE)
            {
                thread->setError(egl::Error(EGL_BAD_ALLOC), "eglClientWaitSyncKHR",
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

EGLImageKHR CreateImageKHR(Thread *thread,
                           Display *display,
                           gl::ContextID contextID,
                           EGLenum target,
                           EGLClientBuffer buffer,
                           const AttributeMap &attributes)
{
    gl::Context *context = display->getContext(contextID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglCreateImageKHR",
                                          GetDisplayIfValid(display), EGL_NO_IMAGE);

    Image *image = nullptr;
    ANGLE_EGL_TRY_RETURN(thread, display->createImage(context, target, buffer, attributes, &image),
                         "", GetDisplayIfValid(display), EGL_NO_IMAGE);

    thread->setSuccess();
    return reinterpret_cast<EGLImage>(static_cast<uintptr_t>(image->id().value));
}

EGLClientBuffer CreateNativeClientBufferANDROID(Thread *thread, const AttributeMap &attribMap)
{
    EGLClientBuffer eglClientBuffer = nullptr;
    ANGLE_EGL_TRY_RETURN(thread,
                         egl::Display::CreateNativeClientBuffer(attribMap, &eglClientBuffer),
                         "eglCreateNativeClientBufferANDROID", nullptr, nullptr);

    thread->setSuccess();
    return eglClientBuffer;
}

EGLSurface CreatePlatformPixmapSurfaceEXT(Thread *thread,
                                          Display *display,
                                          Config *configPacked,
                                          void *native_pixmap,
                                          const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCreatePlatformPixmapSurfaceEXT",
                                          GetDisplayIfValid(display), EGL_NO_SURFACE);
    thread->setError(EGL_BAD_DISPLAY, "eglCreatePlatformPixmapSurfaceEXT",
                     GetDisplayIfValid(display), "CreatePlatformPixmapSurfaceEXT unimplemented.");
    return EGL_NO_SURFACE;
}

EGLSurface CreatePlatformWindowSurfaceEXT(Thread *thread,
                                          Display *display,
                                          Config *configPacked,
                                          void *native_window,
                                          const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCreatePlatformWindowSurfaceEXT",
                                          GetDisplayIfValid(display), EGL_NO_SURFACE);
    Surface *surface = nullptr;

    // In X11, eglCreatePlatformWindowSurfaceEXT expects the native_window argument to be a pointer
    // to a Window while the EGLNativeWindowType for X11 is its actual value.
    // https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_platform_x11.txt
    void *actualNativeWindow =
        display->getImplementation()->getWindowSystem() == angle::NativeWindowSystem::X11
            ? *reinterpret_cast<void **>(native_window)
            : native_window;
    EGLNativeWindowType nativeWindow = reinterpret_cast<EGLNativeWindowType>(actualNativeWindow);

    ANGLE_EGL_TRY_RETURN(
        thread, display->createWindowSurface(configPacked, nativeWindow, attributes, &surface),
        "eglCreatePlatformWindowSurfaceEXT", GetDisplayIfValid(display), EGL_NO_SURFACE);

    return reinterpret_cast<EGLSurface>(static_cast<uintptr_t>(surface->id().value));
}

EGLStreamKHR CreateStreamKHR(Thread *thread, Display *display, const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglCreateStreamKHR",
                                          GetDisplayIfValid(display), EGL_NO_STREAM_KHR);
    Stream *stream;
    ANGLE_EGL_TRY_RETURN(thread, display->createStream(attributes, &stream), "eglCreateStreamKHR",
                         GetDisplayIfValid(display), EGL_NO_STREAM_KHR);

    thread->setSuccess();
    return static_cast<EGLStreamKHR>(stream);
}

EGLSyncKHR CreateSyncKHR(Thread *thread,
                         Display *display,
                         EGLenum type,
                         const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglCreateSyncKHR",
                                          GetDisplayIfValid(display), EGL_NO_SYNC);
    egl::Sync *syncObject = nullptr;
    ANGLE_EGL_TRY_RETURN(thread,
                         display->createSync(thread->getContext(), type, attributes, &syncObject),
                         "eglCreateSyncKHR", GetDisplayIfValid(display), EGL_NO_SYNC);

    thread->setSuccess();
    return reinterpret_cast<EGLSync>(static_cast<uintptr_t>(syncObject->id().value));
}

EGLint DebugMessageControlKHR(Thread *thread,
                              EGLDEBUGPROCKHR callback,
                              const AttributeMap &attributes)
{
    Debug *debug = GetDebug();
    debug->setCallback(callback, attributes);

    thread->setSuccess();
    return EGL_SUCCESS;
}

EGLBoolean DestroyImageKHR(Thread *thread, Display *display, egl::ImageID imageID)
{
    Image *img = display->getImage(imageID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglDestroyImageKHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    display->destroyImage(img);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean DestroyStreamKHR(Thread *thread, Display *display, Stream *streamObject)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglDestroyStreamKHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    display->destroyStream(streamObject);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean DestroySyncKHR(Thread *thread, Display *display, SyncID syncID)
{
    Sync *sync = display->getSync(syncID);
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglDestroySync",
                                          GetDisplayIfValid(display), EGL_FALSE);
    display->destroySync(sync);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLint DupNativeFenceFDANDROID(Thread *thread, Display *display, SyncID syncID)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglDupNativeFenceFDANDROID", GetDisplayIfValid(display),
                                          EGL_NO_NATIVE_FENCE_FD_ANDROID);
    EGLint result    = EGL_NO_NATIVE_FENCE_FD_ANDROID;
    Sync *syncObject = display->getSync(syncID);
    ANGLE_EGL_TRY_RETURN(thread, syncObject->dupNativeFenceFD(display, &result),
                         "eglDupNativeFenceFDANDROID", GetSyncIfValid(display, syncID),
                         EGL_NO_NATIVE_FENCE_FD_ANDROID);

    thread->setSuccess();
    return result;
}

EGLClientBuffer GetNativeClientBufferANDROID(Thread *thread, const struct AHardwareBuffer *buffer)
{
    thread->setSuccess();
    return egl::Display::GetNativeClientBuffer(buffer);
}

EGLDisplay GetPlatformDisplayEXT(Thread *thread,
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
            return egl::Display::GetDisplayFromNativeDisplay(
                platform, gl::bitCast<EGLNativeDisplayType>(native_display), attribMap);
        }
        case EGL_PLATFORM_DEVICE_EXT:
        {
            Device *eglDevice = static_cast<Device *>(native_display);
            return egl::Display::GetDisplayFromDevice(eglDevice, attribMap);
        }
        default:
        {
            UNREACHABLE();
            return EGL_NO_DISPLAY;
        }
    }
}

EGLBoolean GetSyncAttribKHR(Thread *thread,
                            Display *display,
                            SyncID syncObject,
                            EGLint attribute,
                            EGLint *value)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglGetSyncAttrib",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, GetSyncAttrib(display, syncObject, attribute, value),
                         "eglGetSyncAttrib", GetSyncIfValid(display, syncObject), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLint LabelObjectKHR(Thread *thread,
                      Display *display,
                      ObjectType objectTypePacked,
                      EGLObjectKHR object,
                      EGLLabelKHR label)
{
    LabeledObject *labeledObject =
        GetLabeledObjectIfValid(thread, display, objectTypePacked, object);
    ASSERT(labeledObject != nullptr);
    labeledObject->setLabel(label);

    thread->setSuccess();
    return EGL_SUCCESS;
}

EGLBoolean PostSubBufferNV(Thread *thread,
                           Display *display,
                           SurfaceID surfaceID,
                           EGLint x,
                           EGLint y,
                           EGLint width,
                           EGLint height)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglPostSubBufferNV",
                                          GetDisplayIfValid(display), EGL_FALSE);
    Error error = eglSurface->postSubBuffer(thread->getContext(), x, y, width, height);
    if (error.isError())
    {
        thread->setError(error, "eglPostSubBufferNV", GetSurfaceIfValid(display, surfaceID));
        return EGL_FALSE;
    }

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean PresentationTimeANDROID(Thread *thread,
                                   Display *display,
                                   SurfaceID surfaceID,
                                   EGLnsecsANDROID time)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglPresentationTimeANDROID", GetDisplayIfValid(display),
                                          EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, eglSurface->setPresentationTime(time),
                         "eglPresentationTimeANDROID", GetSurfaceIfValid(display, surfaceID),
                         EGL_FALSE);

    return EGL_TRUE;
}

EGLBoolean GetCompositorTimingSupportedANDROID(Thread *thread,
                                               Display *display,
                                               SurfaceID surfaceID,
                                               CompositorTiming nameInternal)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    thread->setSuccess();
    return eglSurface->getSupportedCompositorTimings().test(nameInternal);
}

EGLBoolean GetCompositorTimingANDROID(Thread *thread,
                                      Display *display,
                                      SurfaceID surfaceID,
                                      EGLint numTimestamps,
                                      const EGLint *names,
                                      EGLnsecsANDROID *values)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglGetCompositorTimingANDROIDD",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, eglSurface->getCompositorTiming(numTimestamps, names, values),
                         "eglGetCompositorTimingANDROIDD", GetSurfaceIfValid(display, surfaceID),
                         EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean GetNextFrameIdANDROID(Thread *thread,
                                 Display *display,
                                 SurfaceID surfaceID,
                                 EGLuint64KHR *frameId)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglGetNextFrameIdANDROID", GetDisplayIfValid(display),
                                          EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, eglSurface->getNextFrameId(frameId), "eglGetNextFrameIdANDROID",
                         GetSurfaceIfValid(display, surfaceID), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean GetFrameTimestampSupportedANDROID(Thread *thread,
                                             Display *display,
                                             SurfaceID surfaceID,
                                             Timestamp timestampInternal)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglQueryTimestampSupportedANDROID",
                                          GetDisplayIfValid(display), EGL_FALSE);
    thread->setSuccess();
    return eglSurface->getSupportedTimestamps().test(timestampInternal);
}

EGLBoolean GetFrameTimestampsANDROID(Thread *thread,
                                     Display *display,
                                     SurfaceID surfaceID,
                                     EGLuint64KHR frameId,
                                     EGLint numTimestamps,
                                     const EGLint *timestamps,
                                     EGLnsecsANDROID *values)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglGetFrameTimestampsANDROID",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(
        thread, eglSurface->getFrameTimestamps(frameId, numTimestamps, timestamps, values),
        "eglGetFrameTimestampsANDROID", GetSurfaceIfValid(display, surfaceID), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean QueryDebugKHR(Thread *thread, EGLint attribute, EGLAttrib *value)
{
    Debug *debug = GetDebug();
    switch (attribute)
    {
        case EGL_DEBUG_MSG_CRITICAL_KHR:
        case EGL_DEBUG_MSG_ERROR_KHR:
        case EGL_DEBUG_MSG_WARN_KHR:
        case EGL_DEBUG_MSG_INFO_KHR:
            *value = debug->isMessageTypeEnabled(FromEGLenum<MessageType>(attribute)) ? EGL_TRUE
                                                                                      : EGL_FALSE;
            break;
        case EGL_DEBUG_CALLBACK_KHR:
            *value = reinterpret_cast<EGLAttrib>(debug->getCallback());
            break;

        default:
            UNREACHABLE();
    }

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean QueryDeviceAttribEXT(Thread *thread, Device *dev, EGLint attribute, EGLAttrib *value)
{
    ANGLE_EGL_TRY_RETURN(thread, dev->getAttribute(attribute, value), "eglQueryDeviceAttribEXT",
                         GetDeviceIfValid(dev), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

const char *QueryDeviceStringEXT(Thread *thread, Device *dev, EGLint name)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, dev->getOwningDisplay()->prepareForCall(),
                                          "eglQueryDeviceStringEXT",
                                          GetDisplayIfValid(dev->getOwningDisplay()), EGL_FALSE);
    const char *result;
    switch (name)
    {
        case EGL_EXTENSIONS:
            result = dev->getExtensionString().c_str();
            break;
        case EGL_DRM_DEVICE_FILE_EXT:
        case EGL_DRM_RENDER_NODE_FILE_EXT:
            result = dev->getDeviceString(name).c_str();
            break;
        default:
            thread->setError(EglBadDevice(), "eglQueryDeviceStringEXT", GetDeviceIfValid(dev));
            return nullptr;
    }

    thread->setSuccess();
    return result;
}

EGLBoolean QueryDisplayAttribEXT(Thread *thread,
                                 Display *display,
                                 EGLint attribute,
                                 EGLAttrib *value)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglQueryDisplayAttribEXT", GetDisplayIfValid(display),
                                          EGL_FALSE);
    *value = display->queryAttrib(attribute);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean QueryStreamKHR(Thread *thread,
                          Display *display,
                          Stream *streamObject,
                          EGLenum attribute,
                          EGLint *value)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglQueryStreamKHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    switch (attribute)
    {
        case EGL_STREAM_STATE_KHR:
            *value = streamObject->getState();
            break;
        case EGL_CONSUMER_LATENCY_USEC_KHR:
            *value = streamObject->getConsumerLatency();
            break;
        case EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR:
            *value = streamObject->getConsumerAcquireTimeout();
            break;
        default:
            UNREACHABLE();
    }

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean QueryStreamu64KHR(Thread *thread,
                             Display *display,
                             Stream *streamObject,
                             EGLenum attribute,
                             EGLuint64KHR *value)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglQueryStreamu64KHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    switch (attribute)
    {
        case EGL_PRODUCER_FRAME_KHR:
            *value = streamObject->getProducerFrame();
            break;
        case EGL_CONSUMER_FRAME_KHR:
            *value = streamObject->getConsumerFrame();
            break;
        default:
            UNREACHABLE();
    }

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean QuerySurfacePointerANGLE(Thread *thread,
                                    Display *display,
                                    SurfaceID surfaceID,
                                    EGLint attribute,
                                    void **value)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglQuerySurfacePointerANGLE", GetDisplayIfValid(display),
                                          EGL_FALSE);
    Error error = eglSurface->querySurfacePointerANGLE(attribute, value);
    if (error.isError())
    {
        thread->setError(error, "eglQuerySurfacePointerANGLE",
                         GetSurfaceIfValid(display, surfaceID));
        return EGL_FALSE;
    }

    thread->setSuccess();
    return EGL_TRUE;
}

void SetBlobCacheFuncsANDROID(Thread *thread,
                              Display *display,
                              EGLSetBlobFuncANDROID set,
                              EGLGetBlobFuncANDROID get)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(), "eglSetBlobCacheFuncsANDROID",
                                   GetDisplayIfValid(display));
    thread->setSuccess();
    display->setBlobCacheFuncs(set, get);
}

EGLBoolean SignalSyncKHR(Thread *thread, Display *display, SyncID syncID, EGLenum mode)
{
    gl::Context *currentContext = thread->getContext();
    Sync *syncObject            = display->getSync(syncID);
    ANGLE_EGL_TRY_RETURN(thread, syncObject->signal(display, currentContext, mode),
                         "eglSignalSyncKHR", GetSyncIfValid(display, syncID), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean StreamAttribKHR(Thread *thread,
                           Display *display,
                           Stream *streamObject,
                           EGLenum attribute,
                           EGLint value)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglStreamAttribKHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    switch (attribute)
    {
        case EGL_CONSUMER_LATENCY_USEC_KHR:
            streamObject->setConsumerLatency(value);
            break;
        case EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR:
            streamObject->setConsumerAcquireTimeout(value);
            break;
        default:
            UNREACHABLE();
    }

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean StreamConsumerAcquireKHR(Thread *thread, Display *display, Stream *streamObject)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglStreamConsumerAcquireKHR", GetDisplayIfValid(display),
                                          EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, streamObject->consumerAcquire(thread->getContext()),
                         "eglStreamConsumerAcquireKHR", GetStreamIfValid(display, streamObject),
                         EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean StreamConsumerGLTextureExternalKHR(Thread *thread,
                                              Display *display,
                                              Stream *streamObject)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglStreamConsumerGLTextureExternalKHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(
        thread, streamObject->createConsumerGLTextureExternal(AttributeMap(), thread->getContext()),
        "eglStreamConsumerGLTextureExternalKHR", GetStreamIfValid(display, streamObject),
        EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean StreamConsumerGLTextureExternalAttribsNV(Thread *thread,
                                                    Display *display,
                                                    Stream *streamObject,
                                                    const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglStreamConsumerGLTextureExternalAttribsNV",
                                          GetDisplayIfValid(display), EGL_FALSE);

    gl::Context *context = gl::GetValidGlobalContext();
    ANGLE_EGL_TRY_RETURN(thread, streamObject->createConsumerGLTextureExternal(attributes, context),
                         "eglStreamConsumerGLTextureExternalAttribsNV",
                         GetStreamIfValid(display, streamObject), EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean StreamConsumerReleaseKHR(Thread *thread, Display *display, Stream *streamObject)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglStreamConsumerReleaseKHR", GetDisplayIfValid(display),
                                          EGL_FALSE);

    gl::Context *context = gl::GetValidGlobalContext();
    ANGLE_EGL_TRY_RETURN(thread, streamObject->consumerRelease(context),
                         "eglStreamConsumerReleaseKHR", GetStreamIfValid(display, streamObject),
                         EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean SwapBuffersWithDamageKHR(Thread *thread,
                                    Display *display,
                                    SurfaceID surfaceID,
                                    const EGLint *rects,
                                    EGLint n_rects)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglSwapBuffersWithDamageKHR", GetDisplayIfValid(display),
                                          EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, eglSurface->swapWithDamage(thread->getContext(), rects, n_rects),
                         "eglSwapBuffersWithDamageKHR", GetSurfaceIfValid(display, surfaceID),
                         EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

void LockVulkanQueueANGLE(Thread *thread, egl::Display *display)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(), "eglLockVulkanQueueANGLE",
                                   GetDisplayIfValid(display));
    display->lockVulkanQueue();
    thread->setSuccess();
}

void UnlockVulkanQueueANGLE(Thread *thread, egl::Display *display)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(), "eglUnlockVulkanQueueANGLE",
                                   GetDisplayIfValid(display));
    display->unlockVulkanQueue();
    thread->setSuccess();
}

EGLBoolean PrepareSwapBuffersANGLE(Thread *thread, Display *display, SurfaceID surfaceID)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglPrepareSwapBuffersANGLE", GetDisplayIfValid(display),
                                          EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, eglSurface->prepareSwap(thread->getContext()),
                         "eglPrepareSwapBuffersANGLE", eglSurface, EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLint WaitSyncKHR(Thread *thread, Display *display, SyncID syncID, EGLint flags)
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

EGLDeviceEXT CreateDeviceANGLE(Thread *thread,
                               EGLint device_type,
                               void *native_device,
                               const EGLAttrib *attrib_list)
{
    Device *device = nullptr;
    ANGLE_EGL_TRY_RETURN(thread, Device::CreateDevice(device_type, native_device, &device),
                         "eglCreateDeviceANGLE", GetThreadIfValid(thread), EGL_NO_DEVICE_EXT);

    thread->setSuccess();
    return device;
}

EGLBoolean ReleaseDeviceANGLE(Thread *thread, Device *dev)
{
    SafeDelete(dev);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean CreateStreamProducerD3DTextureANGLE(Thread *thread,
                                               Display *display,
                                               Stream *streamObject,
                                               const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCreateStreamProducerD3DTextureANGLE",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, streamObject->createProducerD3D11Texture(attributes),
                         "eglCreateStreamProducerD3DTextureANGLE",
                         GetStreamIfValid(display, streamObject), EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean StreamPostD3DTextureANGLE(Thread *thread,
                                     Display *display,
                                     Stream *streamObject,
                                     void *texture,
                                     const AttributeMap &attributes)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglStreamPostD3DTextureANGLE",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, streamObject->postD3D11Texture(texture, attributes),
                         "eglStreamPostD3DTextureANGLE", GetStreamIfValid(display, streamObject),
                         EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean GetMscRateANGLE(Thread *thread,
                           Display *display,
                           SurfaceID surfaceID,
                           EGLint *numerator,
                           EGLint *denominator)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglGetMscRateANGLE",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, eglSurface->getMscRate(numerator, denominator),
                         "eglGetMscRateANGLE", GetSurfaceIfValid(display, surfaceID), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean GetSyncValuesCHROMIUM(Thread *thread,
                                 Display *display,
                                 SurfaceID surfaceID,
                                 EGLuint64KHR *ust,
                                 EGLuint64KHR *msc,
                                 EGLuint64KHR *sbc)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglGetSyncValuesCHROMIUM", GetDisplayIfValid(display),
                                          EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, eglSurface->getSyncValues(ust, msc, sbc),
                         "eglGetSyncValuesCHROMIUM", GetSurfaceIfValid(display, surfaceID),
                         EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLint ProgramCacheGetAttribANGLE(Thread *thread, Display *display, EGLenum attrib)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglProgramCacheGetAttribANGLE",
                                          GetDisplayIfValid(display), 0);
    thread->setSuccess();
    return display->programCacheGetAttrib(attrib);
}

void ProgramCacheQueryANGLE(Thread *thread,
                            Display *display,
                            EGLint index,
                            void *key,
                            EGLint *keysize,
                            void *binary,
                            EGLint *binarysize)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(), "eglProgramCacheQueryANGLE",
                                   GetDisplayIfValid(display));
    ANGLE_EGL_TRY(thread, display->programCacheQuery(index, key, keysize, binary, binarysize),
                  "eglProgramCacheQueryANGLE", GetDisplayIfValid(display));

    thread->setSuccess();
}

void ProgramCachePopulateANGLE(Thread *thread,
                               Display *display,
                               const void *key,
                               EGLint keysize,
                               const void *binary,
                               EGLint binarysize)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(),
                                   "eglProgramCachePopulateANGLE", GetDisplayIfValid(display));
    ANGLE_EGL_TRY(thread, display->programCachePopulate(key, keysize, binary, binarysize),
                  "eglProgramCachePopulateANGLE", GetDisplayIfValid(display));

    thread->setSuccess();
}

EGLint ProgramCacheResizeANGLE(Thread *thread, Display *display, EGLint limit, EGLint mode)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglProgramCacheResizeANGLE", GetDisplayIfValid(display),
                                          0);
    thread->setSuccess();
    return display->programCacheResize(limit, mode);
}

const char *QueryStringiANGLE(Thread *thread, Display *display, EGLint name, EGLint index)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglQueryStringiANGLE",
                                          GetDisplayIfValid(display), nullptr);
    thread->setSuccess();
    return display->queryStringi(name, index);
}

EGLBoolean SwapBuffersWithFrameTokenANGLE(Thread *thread,
                                          Display *display,
                                          SurfaceID surfaceID,
                                          EGLFrameTokenANGLE frametoken)
{
    Surface *eglSurface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglSwapBuffersWithFrameTokenANGLE",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, eglSurface->swapWithFrameToken(thread->getContext(), frametoken),
                         "eglSwapBuffersWithFrameTokenANGLE", GetDisplayIfValid(display),
                         EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

void ReleaseHighPowerGPUANGLE(Thread *thread, Display *display, gl::ContextID contextID)
{
    gl::Context *context = display->getContext(contextID);
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(), "eglReleaseHighPowerGPUANGLE",
                                   GetDisplayIfValid(display));
    ANGLE_EGL_TRY(thread, context->releaseHighPowerGPU(), "eglReleaseHighPowerGPUANGLE",
                  GetDisplayIfValid(display));

    thread->setSuccess();
}

void ReacquireHighPowerGPUANGLE(Thread *thread, Display *display, gl::ContextID contextID)
{
    gl::Context *context = display->getContext(contextID);
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(),
                                   "eglReacquireHighPowerGPUANGLE", GetDisplayIfValid(display));
    ANGLE_EGL_TRY(thread, context->reacquireHighPowerGPU(), "eglReacquireHighPowerGPUANGLE",
                  GetDisplayIfValid(display));

    thread->setSuccess();
}

void HandleGPUSwitchANGLE(Thread *thread, Display *display)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(), "eglHandleGPUSwitchANGLE",
                                   GetDisplayIfValid(display));
    ANGLE_EGL_TRY(thread, display->handleGPUSwitch(), "eglHandleGPUSwitchANGLE",
                  GetDisplayIfValid(display));

    thread->setSuccess();
}

void ForceGPUSwitchANGLE(Thread *thread, Display *display, EGLint gpuIDHigh, EGLint gpuIDLow)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(), "eglForceGPUSwitchANGLE",
                                   GetDisplayIfValid(display));
    ANGLE_EGL_TRY(thread, display->forceGPUSwitch(gpuIDHigh, gpuIDLow), "eglForceGPUSwitchANGLE",
                  GetDisplayIfValid(display));

    thread->setSuccess();
}

void WaitUntilWorkScheduledANGLE(Thread *thread, Display *display)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(),
                                   "eglWaitUntilWorkScheduledANGLE", GetDisplayIfValid(display));
    ANGLE_EGL_TRY(thread, display->waitUntilWorkScheduled(), "eglWaitUntilWorkScheduledANGLE",
                  GetDisplayIfValid(display));

    thread->setSuccess();
}

EGLBoolean QueryDisplayAttribANGLE(Thread *thread,
                                   Display *display,
                                   EGLint attribute,
                                   EGLAttrib *value)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglQueryDisplayAttribEXT", GetDisplayIfValid(display),
                                          EGL_FALSE);
    *value = display->queryAttrib(attribute);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean LockSurfaceKHR(Thread *thread,
                          egl::Display *display,
                          SurfaceID surfaceID,
                          const AttributeMap &attributes)
{
    Surface *surface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglLockSurfaceKHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, surface->lockSurfaceKHR(display, attributes), "eglLockSurfaceKHR",
                         GetSurfaceIfValid(display, surfaceID), EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean UnlockSurfaceKHR(Thread *thread, egl::Display *display, SurfaceID surfaceID)
{
    Surface *surface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglUnlockSurfaceKHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, surface->unlockSurfaceKHR(display), "eglQuerySurface64KHR",
                         GetSurfaceIfValid(display, surfaceID), EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean QuerySurface64KHR(Thread *thread,
                             egl::Display *display,
                             SurfaceID surfaceID,
                             EGLint attribute,
                             EGLAttribKHR *value)
{
    Surface *surface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(), "eglQuerySurface64KHR",
                                          GetDisplayIfValid(display), EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(
        thread, QuerySurfaceAttrib64KHR(display, thread->getContext(), surface, attribute, value),
        "eglQuerySurface64KHR", GetSurfaceIfValid(display, surfaceID), EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean ExportVkImageANGLE(Thread *thread,
                              egl::Display *display,
                              egl::ImageID imageID,
                              void *vk_image,
                              void *vk_image_create_info)
{
    Image *image = display->getImage(imageID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglExportVkImageANGLE", GetDisplayIfValid(display),
                                          EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, image->exportVkImage(vk_image, vk_image_create_info),
                         "eglExportVkImageANGLE", GetImageIfValid(display, imageID), EGL_FALSE);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean SetDamageRegionKHR(Thread *thread,
                              egl::Display *display,
                              SurfaceID surfaceID,
                              EGLint *rects,
                              EGLint n_rects)
{
    Surface *surface = display->getSurface(surfaceID);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglSetDamageRegionKHR", GetDisplayIfValid(display),
                                          EGL_FALSE);
    surface->setDamageRegion(rects, n_rects);

    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean QueryDmaBufFormatsEXT(Thread *thread,
                                 egl::Display *display,
                                 EGLint max_formats,
                                 EGLint *formats,
                                 EGLint *num_formats)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglQueryDmaBufFormatsEXT", GetDisplayIfValid(display),
                                          EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread, display->queryDmaBufFormats(max_formats, formats, num_formats),
                         "eglQueryDmaBufFormatsEXT", GetDisplayIfValid(display), EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

EGLBoolean QueryDmaBufModifiersEXT(Thread *thread,
                                   egl::Display *display,
                                   EGLint format,
                                   EGLint max_modifiers,
                                   EGLuint64KHR *modifiers,
                                   EGLBoolean *external_only,
                                   EGLint *num_modifiers)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglQueryDmaBufModifiersEXT", GetDisplayIfValid(display),
                                          EGL_FALSE);
    ANGLE_EGL_TRY_RETURN(thread,
                         display->queryDmaBufModifiers(format, max_modifiers, modifiers,
                                                       external_only, num_modifiers),
                         "eglQueryDmaBufModifiersEXT", GetDisplayIfValid(display), EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

void *CopyMetalSharedEventANGLE(Thread *thread, Display *display, SyncID syncID)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglCopyMetalSharedEventANGLE",
                                          GetDisplayIfValid(display), nullptr);
    void *result     = nullptr;
    Sync *syncObject = display->getSync(syncID);
    ANGLE_EGL_TRY_RETURN(thread, syncObject->copyMetalSharedEventANGLE(display, &result),
                         "eglCopyMetalSharedEventANGLE", GetSyncIfValid(display, syncID), nullptr);

    thread->setSuccess();
    return result;
}

void AcquireExternalContextANGLE(Thread *thread, egl::Display *display, SurfaceID drawAndReadPacked)
{
    Surface *eglSurface = display->getSurface(drawAndReadPacked);

    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(),
                                   "eglAcquireExternalContextANGLE", GetDisplayIfValid(display));
    ANGLE_EGL_TRY(thread, thread->getContext()->acquireExternalContext(eglSurface),
                  "eglAcquireExternalContextANGLE", GetDisplayIfValid(display));

    thread->setSuccess();
}

void ReleaseExternalContextANGLE(Thread *thread, egl::Display *display)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL(thread, display->prepareForCall(),
                                   "eglReleaseExternalContextANGLE", GetDisplayIfValid(display));
    ANGLE_EGL_TRY(thread, thread->getContext()->releaseExternalContext(),
                  "eglReleaseExternalContextANGLE", GetDisplayIfValid(display));

    thread->setSuccess();
}

void SetValidationEnabledANGLE(Thread *thread, EGLBoolean validationState)
{
    SetEGLValidationEnabled(validationState != EGL_FALSE);
    thread->setSuccess();
}

EGLBoolean QuerySupportedCompressionRatesEXT(Thread *thread,
                                             egl::Display *display,
                                             egl::Config *configPacked,
                                             const EGLAttrib *attrib_list,
                                             EGLint *rates,
                                             EGLint rate_size,
                                             EGLint *num_rates)
{
    ANGLE_EGL_TRY_PREPARE_FOR_CALL_RETURN(thread, display->prepareForCall(),
                                          "eglQuerySupportedCompressionRatesEXT",
                                          GetDisplayIfValid(display), EGL_FALSE);

    const AttributeMap &attributes = PackParam<const AttributeMap &>((const EGLint *)attrib_list);
    ANGLE_EGL_TRY_RETURN(thread,
                         display->querySupportedCompressionRates(configPacked, attributes, rates,
                                                                 rate_size, num_rates),
                         "eglQuerySupportedCompressionRatesEXT", GetDisplayIfValid(display),
                         EGL_FALSE);
    thread->setSuccess();
    return EGL_TRUE;
}

}  // namespace egl
