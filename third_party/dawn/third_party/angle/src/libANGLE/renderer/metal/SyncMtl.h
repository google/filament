//
// Copyright (c) 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SyncMtl:
//    Defines the class interface for SyncMtl, implementing SyncImpl.
//

#ifndef LIBANGLE_RENDERER_METAL_SYNCMTL_H_
#define LIBANGLE_RENDERER_METAL_SYNCMTL_H_

#include <optional>

#include "libANGLE/renderer/EGLSyncImpl.h"
#include "libANGLE/renderer/FenceNVImpl.h"
#include "libANGLE/renderer/SyncImpl.h"
#include "libANGLE/renderer/metal/mtl_common.h"

namespace egl
{
class AttributeMap;
}

namespace rx
{

class ContextMtl;

namespace mtl
{
class SyncImpl;
}  // namespace mtl

class FenceNVMtl : public FenceNVImpl
{
  public:
    FenceNVMtl();
    ~FenceNVMtl() override;
    void onDestroy(const gl::Context *context) override;
    angle::Result set(const gl::Context *context, GLenum condition) override;
    angle::Result test(const gl::Context *context, GLboolean *outFinished) override;
    angle::Result finish(const gl::Context *context) override;

  private:
    std::unique_ptr<mtl::SyncImpl> mSync;
};

class SyncMtl : public SyncImpl
{
  public:
    SyncMtl();
    ~SyncMtl() override;

    void onDestroy(const gl::Context *context) override;

    angle::Result set(const gl::Context *context, GLenum condition, GLbitfield flags) override;
    angle::Result clientWait(const gl::Context *context,
                             GLbitfield flags,
                             GLuint64 timeout,
                             GLenum *outResult) override;
    angle::Result serverWait(const gl::Context *context,
                             GLbitfield flags,
                             GLuint64 timeout) override;
    angle::Result getStatus(const gl::Context *context, GLint *outResult) override;

  private:
    std::unique_ptr<mtl::SyncImpl> mSync;
};

class EGLSyncMtl final : public EGLSyncImpl
{
  public:
    EGLSyncMtl();
    ~EGLSyncMtl() override;

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
    egl::Error getStatus(const egl::Display *display, EGLint *outStatus) override;

    egl::Error copyMetalSharedEventANGLE(const egl::Display *display, void **result) const override;
    egl::Error dupNativeFenceFD(const egl::Display *display, EGLint *result) const override;

  private:
    mtl::AutoObjCPtr<id<MTLSharedEvent>> mSharedEvent;

    std::unique_ptr<mtl::SyncImpl> mSync;
};

}  // namespace rx

#endif
