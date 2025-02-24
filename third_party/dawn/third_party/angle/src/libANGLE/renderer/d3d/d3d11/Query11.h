//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Query11.h: Defines the rx::Query11 class which implements rx::QueryImpl.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_QUERY11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_QUERY11_H_

#include <deque>

#include "libANGLE/renderer/QueryImpl.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"

namespace rx
{
class Context11;
class Renderer11;

class Query11 : public QueryImpl
{
  public:
    Query11(Renderer11 *renderer, gl::QueryType type);
    ~Query11() override;

    angle::Result begin(const gl::Context *context) override;
    angle::Result end(const gl::Context *context) override;
    angle::Result queryCounter(const gl::Context *context) override;
    angle::Result getResult(const gl::Context *context, GLint *params) override;
    angle::Result getResult(const gl::Context *context, GLuint *params) override;
    angle::Result getResult(const gl::Context *context, GLint64 *params) override;
    angle::Result getResult(const gl::Context *context, GLuint64 *params) override;
    angle::Result isResultAvailable(const gl::Context *context, bool *available) override;

    angle::Result pause(Context11 *context11);
    angle::Result resume(Context11 *context11);

  private:
    struct QueryState final : private angle::NonCopyable
    {
        QueryState();
        ~QueryState();

        unsigned int getDataAttemptCount;

        d3d11::Query query;
        d3d11::Query beginTimestamp;
        d3d11::Query endTimestamp;
        bool finished;
    };

    angle::Result flush(Context11 *context11, bool force);
    angle::Result testQuery(Context11 *context11, QueryState *queryState);

    template <typename T>
    angle::Result getResultBase(Context11 *context11, T *params);

    GLuint64 mResult;
    GLuint64 mResultSum;

    Renderer11 *mRenderer;

    std::unique_ptr<QueryState> mActiveQuery;
    std::deque<std::unique_ptr<QueryState>> mPendingQueries;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_QUERY11_H_
