//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Fence11.h: Defines the rx::FenceNV11 and rx::Sync11 classes which implement rx::FenceNVImpl
// and rx::SyncImpl.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_FENCE11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_FENCE11_H_

#include "libANGLE/renderer/FenceNVImpl.h"
#include "libANGLE/renderer/SyncImpl.h"

namespace rx
{
class Renderer11;

class FenceNV11 : public FenceNVImpl
{
  public:
    explicit FenceNV11(Renderer11 *renderer);
    ~FenceNV11() override;

    void onDestroy(const gl::Context *context) override {}
    angle::Result set(const gl::Context *context, GLenum condition) override;
    angle::Result test(const gl::Context *context, GLboolean *outFinished) override;
    angle::Result finish(const gl::Context *context) override;

  private:
    template <class T>
    friend angle::Result FenceSetHelper(const gl::Context *context, T *fence);
    template <class T>
    friend angle::Result FenceTestHelper(const gl::Context *context,
                                         T *fence,
                                         bool flushCommandBuffer,
                                         GLboolean *outFinished);

    Renderer11 *mRenderer;
    ID3D11Query *mQuery;
};

class Sync11 : public SyncImpl
{
  public:
    explicit Sync11(Renderer11 *renderer);
    ~Sync11() override;

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
    template <class T>
    friend angle::Result FenceSetHelper(const gl::Context *context, T *fence);
    template <class T>
    friend angle::Result FenceTestHelper(const gl::Context *context,
                                         T *fence,
                                         bool flushCommandBuffer,
                                         GLboolean *outFinished);

    Renderer11 *mRenderer;
    ID3D11Query *mQuery;
    LONGLONG mCounterFrequency;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_FENCE11_H_
