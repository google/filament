//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Query9.h: Defines the rx::Query9 class which implements rx::QueryImpl.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_QUERY9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_QUERY9_H_

#include "libANGLE/renderer/QueryImpl.h"

namespace rx
{
class Context9;
class Renderer9;

class Query9 : public QueryImpl
{
  public:
    Query9(Renderer9 *renderer, gl::QueryType type);
    ~Query9() override;

    angle::Result begin(const gl::Context *context) override;
    angle::Result end(const gl::Context *context) override;
    angle::Result queryCounter(const gl::Context *context) override;
    angle::Result getResult(const gl::Context *context, GLint *params) override;
    angle::Result getResult(const gl::Context *context, GLuint *params) override;
    angle::Result getResult(const gl::Context *context, GLint64 *params) override;
    angle::Result getResult(const gl::Context *context, GLuint64 *params) override;
    angle::Result isResultAvailable(const gl::Context *context, bool *available) override;

  private:
    angle::Result testQuery(Context9 *context9);

    template <typename T>
    angle::Result getResultBase(Context9 *context9, T *params);

    unsigned int mGetDataAttemptCount;
    GLuint64 mResult;
    bool mQueryFinished;

    Renderer9 *mRenderer;
    IDirect3DQuery9 *mQuery;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_QUERY9_H_
