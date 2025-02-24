//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// global_state.h : Defines functions for querying the thread-local GL and EGL state.

#ifndef LIBGLESV2_GLOBALSTATE_H_
#define LIBGLESV2_GLOBALSTATE_H_

#include "libANGLE/Context.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Display.h"
#include "libANGLE/GlobalMutex.h"
#include "libANGLE/Thread.h"
#include "libANGLE/features.h"
#include "libANGLE/validationEGL.h"

#if defined(ANGLE_PLATFORM_APPLE) || (ANGLE_PLATFORM_ANDROID) || defined(ANGLE_USE_ANDROID_TLS_SLOT)
#    include "common/tls.h"
#endif

#include <mutex>

namespace egl
{
class Debug;
class Thread;

#if defined(ANGLE_PLATFORM_APPLE) || defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
extern Thread *GetCurrentThreadTLS();
extern void SetCurrentThreadTLS(Thread *thread);
#else
extern thread_local Thread *gCurrentThread;
#endif

gl::Context *GetGlobalLastContext();
void SetGlobalLastContext(gl::Context *context);
Thread *GetCurrentThread();
Debug *GetDebug();

void SetEGLValidationEnabled(bool enabled);
bool IsEGLValidationEnabled();

// Sync the current context from Thread to global state.
class [[nodiscard]] ScopedSyncCurrentContextFromThread
{
  public:
    ScopedSyncCurrentContextFromThread(egl::Thread *thread);
    ~ScopedSyncCurrentContextFromThread();

  private:
    egl::Thread *const mThread;
};

// Tries to lock "ContextMutex" of the Context current to the "thread".
ANGLE_INLINE ScopedContextMutexLock TryLockCurrentContext(Thread *thread)
{
    ASSERT(kIsContextMutexEnabled);
    gl::Context *context = thread->getContext();
    return context != nullptr ? ScopedContextMutexLock(context->getContextMutex())
                              : ScopedContextMutexLock();
}

// Tries to lock "ContextMutex" of the Context with "contextID" if it is valid.
ANGLE_INLINE ScopedContextMutexLock TryLockContext(Display *display, gl::ContextID contextID)
{
    ASSERT(kIsContextMutexEnabled);
    gl::Context *context = GetContextIfValid(display, contextID);
    return context != nullptr ? ScopedContextMutexLock(context->getContextMutex())
                              : ScopedContextMutexLock();
}

// Locks "ContextMutex" of the "context" and then tries to merge it with the "ContextMutex" of the
// Image with "imageID" if it is valid.
ANGLE_INLINE ScopedContextMutexLock LockAndTryMergeContextMutexes(gl::Context *context,
                                                                  ImageID imageID)
{
    ASSERT(kIsContextMutexEnabled);
    ASSERT(context->getDisplay() != nullptr);
    ScopedContextMutexLock lock(context->getContextMutex());
    const Image *image = context->getDisplay()->getImage(imageID);
    if (image != nullptr)
    {
        ContextMutex *imageMutex = image->getContextMutex();
        if (imageMutex != nullptr)
        {
            ContextMutex::Merge(&context->getContextMutex(), imageMutex);
        }
    }
    return lock;
}

#if !defined(ANGLE_ENABLE_CONTEXT_MUTEX)
#    define ANGLE_EGL_SCOPED_CONTEXT_LOCK(EP, THREAD, ...)
#else
#    define ANGLE_EGL_SCOPED_CONTEXT_LOCK(EP, THREAD, ...) \
        egl::ScopedContextMutexLock shareContextLock = GetContextLock_##EP(THREAD, ##__VA_ARGS__)
#endif

}  // namespace egl

#define ANGLE_SCOPED_GLOBAL_LOCK() egl::ScopedGlobalEGLMutexLock globalMutexLock
#if ANGLE_CAPTURE_ENABLED
#    define ANGLE_SCOPED_GLOBAL_EGL_AND_EGL_SYNC_LOCK() \
        egl::ScopedGlobalEGLMutexLock globalMutexLock
#else
#    define ANGLE_SCOPED_GLOBAL_EGL_AND_EGL_SYNC_LOCK() \
        egl::ScopedGlobalEGLMutexLock globalMutexLock;  \
        egl::ScopedGlobalEGLSyncObjectMutexLock globalEGLSyncObjectMutexLock
#endif

#if ANGLE_CAPTURE_ENABLED
#    define ANGLE_SCOPED_GLOBAL_EGL_SYNC_LOCK() egl::ScopedGlobalEGLMutexLock globalMutexLock
#else
#    define ANGLE_SCOPED_GLOBAL_EGL_SYNC_LOCK() \
        egl::ScopedGlobalEGLSyncObjectMutexLock globalEGLSyncObjectMutexLock
#endif

namespace gl
{
ANGLE_INLINE Context *GetGlobalContext()
{
#if defined(ANGLE_PLATFORM_APPLE) || defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
    egl::Thread *currentThread = egl::GetCurrentThreadTLS();
#else
    egl::Thread *currentThread = egl::gCurrentThread;
#endif
    ASSERT(currentThread);
    return currentThread->getContext();
}

ANGLE_INLINE Context *GetValidGlobalContext()
{
#if defined(ANGLE_USE_ANDROID_TLS_SLOT)
    // TODO: Replace this branch with a compile time flag (http://anglebug.com/42263361)
    if (angle::gUseAndroidOpenGLTlsSlot)
    {
        return static_cast<gl::Context *>(ANGLE_ANDROID_GET_GL_TLS()[angle::kAndroidOpenGLTlsSlot]);
    }
#endif

#if defined(ANGLE_PLATFORM_APPLE) || defined(ANGLE_USE_STATIC_THREAD_LOCAL_VARIABLES)
    return GetCurrentValidContextTLS();
#else
    return gCurrentValidContext;
#endif
}

// Generate a context lost error on the context if it is non-null and lost.
void GenerateContextLostErrorOnContext(Context *context);
void GenerateContextLostErrorOnCurrentGlobalContext();

#if defined(ANGLE_FORCE_CONTEXT_CHECK_EVERY_CALL)
// TODO(b/177574181): This should be handled in a backend-specific way.
// if previous context different from current context, dirty all state
static ANGLE_INLINE void DirtyContextIfNeeded(Context *context)
{
    if (context && context != egl::GetGlobalLastContext())
    {
        context->dirtyAllState();
        SetGlobalLastContext(context);
    }
}

#endif

#if !defined(ANGLE_ENABLE_SHARE_CONTEXT_LOCK)
#    define SCOPED_SHARE_CONTEXT_LOCK(context)
#    define SCOPED_EGL_IMAGE_SHARE_CONTEXT_LOCK(context, imageID) ANGLE_SCOPED_GLOBAL_LOCK()
#else
#    if defined(ANGLE_FORCE_CONTEXT_CHECK_EVERY_CALL)
#        define SCOPED_SHARE_CONTEXT_LOCK(context)          \
            egl::ScopedGlobalEGLMutexLock shareContextLock; \
            DirtyContextIfNeeded(context)
#        define SCOPED_EGL_IMAGE_SHARE_CONTEXT_LOCK(context, imageID) \
            SCOPED_SHARE_CONTEXT_LOCK(context)
#    elif !defined(ANGLE_ENABLE_CONTEXT_MUTEX)
#        define SCOPED_SHARE_CONTEXT_LOCK(context) \
            egl::ScopedOptionalGlobalMutexLock shareContextLock(context->isShared())
#        define SCOPED_EGL_IMAGE_SHARE_CONTEXT_LOCK(context, imageID) ANGLE_SCOPED_GLOBAL_LOCK()
#    else
#        define SCOPED_SHARE_CONTEXT_LOCK(context) \
            egl::ScopedContextMutexLock shareContextLock(context->getContextMutex())
#        define SCOPED_EGL_IMAGE_SHARE_CONTEXT_LOCK(context, imageID) \
            ANGLE_SCOPED_GLOBAL_LOCK();                               \
            egl::ScopedContextMutexLock shareContextLock =            \
                egl::LockAndTryMergeContextMutexes(context, imageID)
#    endif
#endif

}  // namespace gl

#endif  // LIBGLESV2_GLOBALSTATE_H_
