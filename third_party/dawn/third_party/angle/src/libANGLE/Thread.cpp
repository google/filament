//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Thread.cpp : Defines the Thread class which represents a global EGL thread.

#include "libANGLE/Thread.h"

#include "libANGLE/Context.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Display.h"
#include "libANGLE/Error.h"

namespace angle
{
#if defined(ANGLE_USE_ANDROID_TLS_SLOT)
bool gUseAndroidOpenGLTlsSlot = false;
#endif
}  // namespace angle

namespace egl
{
namespace
{
Debug *sDebug = nullptr;
}  // namespace

Thread::Thread()
    : mLabel(nullptr),
      mError(EGL_SUCCESS),
      mAPI(EGL_OPENGL_ES_API),
      mContext(static_cast<gl::Context *>(EGL_NO_CONTEXT))
{}

void Thread::setLabel(EGLLabelKHR label)
{
    mLabel = label;
}

EGLLabelKHR Thread::getLabel() const
{
    return mLabel;
}

void Thread::setSuccess()
{
    mError = EGL_SUCCESS;
}

void Thread::setError(EGLint error,
                      const char *command,
                      const LabeledObject *object,
                      const char *message)
{
    mError = error;
    if (error != EGL_SUCCESS && message)
    {
        EnsureDebugAllocated();
        sDebug->insertMessage(error, command, ErrorCodeToMessageType(error), getLabel(),
                              object ? object->getLabel() : nullptr, message);
    }
}

void Thread::setError(const Error &error, const char *command, const LabeledObject *object)
{
    mError = error.getCode();
    if (error.isError() && !error.getMessage().empty())
    {
        EnsureDebugAllocated();
        sDebug->insertMessage(error.getCode(), command, ErrorCodeToMessageType(error.getCode()),
                              getLabel(), object ? object->getLabel() : nullptr,
                              error.getMessage());
    }
}

EGLint Thread::getError() const
{
    return mError;
}

void Thread::setAPI(EGLenum api)
{
    mAPI = api;
}

EGLenum Thread::getAPI() const
{
    return mAPI;
}

void Thread::setCurrent(gl::Context *context)
{
    mContext = context;
    if (mContext)
    {
        ASSERT(mContext->getDisplay());
    }
}

Surface *Thread::getCurrentDrawSurface() const
{
    if (mContext)
    {
        return mContext->getCurrentDrawSurface();
    }
    return nullptr;
}

Surface *Thread::getCurrentReadSurface() const
{
    if (mContext)
    {
        return mContext->getCurrentReadSurface();
    }
    return nullptr;
}

gl::Context *Thread::getContext() const
{
    return mContext;
}

Display *Thread::getDisplay() const
{
    if (mContext)
    {
        return mContext->getDisplay();
    }
    return nullptr;
}

void EnsureDebugAllocated()
{
    // All EGL calls use a global lock, this is thread safe
    if (sDebug == nullptr)
    {
        sDebug = new Debug();
    }
}

void DeallocateDebug()
{
    SafeDelete(sDebug);
}

Debug *GetDebug()
{
    EnsureDebugAllocated();
    return sDebug;
}
}  // namespace egl
