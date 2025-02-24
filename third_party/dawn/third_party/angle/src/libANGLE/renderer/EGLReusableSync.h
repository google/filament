//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EGL_KHR_reusable_sync

#ifndef LIBANGLE_RENDERER_EGLREUSABLESYNC_H_
#define LIBANGLE_RENDERER_EGLREUSABLESYNC_H_

#include "libANGLE/AttributeMap.h"
#include "libANGLE/renderer/EGLSyncImpl.h"

#include "common/angleutils.h"

#include <condition_variable>

namespace rx
{

class ReusableSync final : public EGLSyncImpl
{
  public:
    ReusableSync();
    ~ReusableSync() override;

    void onDestroy(const egl::Display *display) override;

    egl::Error initialize(const egl::Display *display,
                          const gl::Context *context,
                          EGLenum type,
                          const egl::AttributeMap &attribs) override;
    egl::Error clientWait(const egl::Display *display,
                          const gl::Context *context,
                          EGLint flags,
                          EGLTime timeout,
                          EGLint *outResult) override;
    egl::Error serverWait(const egl::Display *display,
                          const gl::Context *context,
                          EGLint flags) override;
    egl::Error signal(const egl::Display *display,
                      const gl::Context *context,
                      EGLint mode) override;
    egl::Error getStatus(const egl::Display *display, EGLint *outStatus) override;

  private:
    EGLint mStatus;
    std::condition_variable mCondVar;
    std::unique_lock<std::mutex> mMutex;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_EGLREUSABLESYNC_H_
