//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SyncEGL.cpp: Implements the rx::SyncEGL class.

#include "libANGLE/renderer/gl/egl/SyncEGL.h"

#include "libANGLE/AttributeMap.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

namespace rx
{

SyncEGL::SyncEGL(const FunctionsEGL *egl) : mEGL(egl), mSync(EGL_NO_SYNC_KHR) {}

SyncEGL::~SyncEGL()
{
    ASSERT(mSync == EGL_NO_SYNC_KHR);
}

void SyncEGL::onDestroy(const egl::Display *display)
{
    if (mSync != EGL_NO_SYNC_KHR)
    {
        egl::Display::GetCurrentThreadUnlockedTailCall()->add(
            [egl = mEGL, sync = mSync](void *resultOut) {
                EGLBoolean result = egl->destroySyncKHR(sync);
                if (resultOut)
                {
                    // It's possible for resultOut to be null if this sync is being destructed as
                    // part of display destruction.
                    *static_cast<EGLBoolean *>(resultOut) = result;
                }
            });
        mSync = EGL_NO_SYNC_KHR;
    }
}

egl::Error SyncEGL::initialize(const egl::Display *display,
                               const gl::Context *context,
                               EGLenum type,
                               const egl::AttributeMap &attribs)
{
    ASSERT(type == EGL_SYNC_FENCE_KHR || type == EGL_SYNC_NATIVE_FENCE_ANDROID);

    constexpr size_t kAttribVectorSize = 3;
    angle::FixedVector<EGLint, kAttribVectorSize> nativeAttribs;
    if (type == EGL_SYNC_NATIVE_FENCE_ANDROID)
    {
        EGLint fenceFd =
            attribs.getAsInt(EGL_SYNC_NATIVE_FENCE_FD_ANDROID, EGL_NO_NATIVE_FENCE_FD_ANDROID);
        nativeAttribs.push_back(EGL_SYNC_NATIVE_FENCE_FD_ANDROID);
        nativeAttribs.push_back(fenceFd);
    }
    nativeAttribs.push_back(EGL_NONE);

    egl::Display::GetCurrentThreadUnlockedTailCall()->add(
        [egl = mEGL, &sync = mSync, type, attribs = nativeAttribs](void *resultOut) {
            sync = egl->createSyncKHR(type, attribs.data());

            // If sync creation failed, force the return value of eglCreateSync to EGL_NO_SYNC. This
            // won't delete this sync object but a driver error is unexpected at this point.
            if (sync == EGL_NO_SYNC_KHR)
            {
                ERR() << "eglCreateSync failed with " << gl::FmtHex(egl->getError());
                *static_cast<EGLSync *>(resultOut) = EGL_NO_SYNC_KHR;
            }
        });

    return egl::NoError();
}

egl::Error SyncEGL::clientWait(const egl::Display *display,
                               const gl::Context *context,
                               EGLint flags,
                               EGLTime timeout,
                               EGLint *outResult)
{
    ASSERT(mSync != EGL_NO_SYNC_KHR);

    // If we need to perform a CPU wait don't set the resultOut parameter passed into the
    // method, instead set the parameter passed into the unlocked tail call.
    egl::Display::GetCurrentThreadUnlockedTailCall()->add(
        [egl = mEGL, sync = mSync, flags, timeout](void *resultOut) {
            *static_cast<EGLint *>(resultOut) = egl->clientWaitSyncKHR(sync, flags, timeout);
        });

    return egl::NoError();
}

egl::Error SyncEGL::serverWait(const egl::Display *display,
                               const gl::Context *context,
                               EGLint flags)
{
    ASSERT(mSync != EGL_NO_SYNC_KHR);

    egl::Display::GetCurrentThreadUnlockedTailCall()->add(
        [egl = mEGL, sync = mSync, flags](void *resultOut) {
            *static_cast<EGLBoolean *>(resultOut) = egl->waitSyncKHR(sync, flags);
        });

    return egl::NoError();
}

egl::Error SyncEGL::getStatus(const egl::Display *display, EGLint *outStatus)
{
    ASSERT(mSync != EGL_NO_SYNC_KHR);
    EGLBoolean result = mEGL->getSyncAttribKHR(mSync, EGL_SYNC_STATUS_KHR, outStatus);

    if (result == EGL_FALSE)
    {
        return egl::Error(mEGL->getError(), "eglGetSyncAttribKHR with EGL_SYNC_STATUS_KHR failed");
    }

    return egl::NoError();
}

egl::Error SyncEGL::dupNativeFenceFD(const egl::Display *display, EGLint *result) const
{
    ASSERT(mSync != EGL_NO_SYNC_KHR);
    *result = mEGL->dupNativeFenceFDANDROID(mSync);
    if (*result == EGL_NO_NATIVE_FENCE_FD_ANDROID)
    {
        return egl::Error(mEGL->getError(), "eglDupNativeFenceFDANDROID failed");
    }

    return egl::NoError();
}

}  // namespace rx
