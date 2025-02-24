//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Query9.cpp: Defines the rx::Query9 class which implements rx::QueryImpl.

#include "libANGLE/renderer/d3d/d3d9/Query9.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/d3d9/Context9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"

#include <GLES2/gl2ext.h>

namespace rx
{
Query9::Query9(Renderer9 *renderer, gl::QueryType type)
    : QueryImpl(type),
      mGetDataAttemptCount(0),
      mResult(GL_FALSE),
      mQueryFinished(false),
      mRenderer(renderer),
      mQuery(nullptr)
{}

Query9::~Query9()
{
    SafeRelease(mQuery);
}

angle::Result Query9::begin(const gl::Context *context)
{
    Context9 *context9 = GetImplAs<Context9>(context);

    D3DQUERYTYPE d3dQueryType = gl_d3d9::ConvertQueryType(getType());
    if (mQuery == nullptr)
    {
        HRESULT result = mRenderer->getDevice()->CreateQuery(d3dQueryType, &mQuery);
        ANGLE_TRY_HR(context9, result, "Internal query creation failed");
    }

    if (d3dQueryType != D3DQUERYTYPE_EVENT)
    {
        HRESULT result = mQuery->Issue(D3DISSUE_BEGIN);
        ASSERT(SUCCEEDED(result));
        ANGLE_TRY_HR(context9, result, "Failed to begin internal query");
    }

    return angle::Result::Continue;
}

angle::Result Query9::end(const gl::Context *context)
{
    Context9 *context9 = GetImplAs<Context9>(context);
    ASSERT(mQuery);

    HRESULT result = mQuery->Issue(D3DISSUE_END);
    ASSERT(SUCCEEDED(result));
    ANGLE_TRY_HR(context9, result, "Failed to end internal query");
    mQueryFinished = false;
    mResult        = GL_FALSE;

    return angle::Result::Continue;
}

angle::Result Query9::queryCounter(const gl::Context *context)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
}

template <typename T>
angle::Result Query9::getResultBase(Context9 *context9, T *params)
{
    while (!mQueryFinished)
    {
        ANGLE_TRY(testQuery(context9));

        if (!mQueryFinished)
        {
            Sleep(0);
        }
    }

    ASSERT(mQueryFinished);
    *params = static_cast<T>(mResult);
    return angle::Result::Continue;
}

angle::Result Query9::getResult(const gl::Context *context, GLint *params)
{
    return getResultBase(GetImplAs<Context9>(context), params);
}

angle::Result Query9::getResult(const gl::Context *context, GLuint *params)
{
    return getResultBase(GetImplAs<Context9>(context), params);
}

angle::Result Query9::getResult(const gl::Context *context, GLint64 *params)
{
    return getResultBase(GetImplAs<Context9>(context), params);
}

angle::Result Query9::getResult(const gl::Context *context, GLuint64 *params)
{
    return getResultBase(GetImplAs<Context9>(context), params);
}

angle::Result Query9::isResultAvailable(const gl::Context *context, bool *available)
{
    ANGLE_TRY(testQuery(GetImplAs<Context9>(context)));
    *available = mQueryFinished;
    return angle::Result::Continue;
}

angle::Result Query9::testQuery(Context9 *context9)
{
    if (!mQueryFinished)
    {
        ASSERT(mQuery);

        HRESULT result = S_OK;
        switch (getType())
        {
            case gl::QueryType::AnySamples:
            case gl::QueryType::AnySamplesConservative:
            {
                DWORD numPixels = 0;
                result          = mQuery->GetData(&numPixels, sizeof(numPixels), D3DGETDATA_FLUSH);
                if (result == S_OK)
                {
                    mQueryFinished = true;
                    mResult        = (numPixels > 0) ? GL_TRUE : GL_FALSE;
                }
                break;
            }

            case gl::QueryType::CommandsCompleted:
            {
                BOOL completed = FALSE;
                result         = mQuery->GetData(&completed, sizeof(completed), D3DGETDATA_FLUSH);
                if (result == S_OK)
                {
                    mQueryFinished = true;
                    mResult        = (completed == TRUE) ? GL_TRUE : GL_FALSE;
                }
                break;
            }

            default:
                UNREACHABLE();
                break;
        }

        if (!mQueryFinished)
        {
            ANGLE_TRY_HR(context9, result, "Failed to test get query result");

            mGetDataAttemptCount++;
            bool checkDeviceLost =
                (mGetDataAttemptCount % kPollingD3DDeviceLostCheckFrequency) == 0;
            if (checkDeviceLost && mRenderer->testDeviceLost())
            {
                ANGLE_TRY_HR(context9, D3DERR_DEVICELOST,
                             "Failed to test get query result, device is lost");
            }
        }
    }

    return angle::Result::Continue;
}

}  // namespace rx
