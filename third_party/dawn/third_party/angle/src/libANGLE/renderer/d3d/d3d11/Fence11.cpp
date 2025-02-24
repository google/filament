//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Fence11.cpp: Defines the rx::FenceNV11 and rx::Sync11 classes which implement
// rx::FenceNVImpl and rx::SyncImpl.

#include "libANGLE/renderer/d3d/d3d11/Fence11.h"

#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"

namespace rx
{

//
// Template helpers for set and test operations.
//

template <class FenceClass>
angle::Result FenceSetHelper(const gl::Context *context, FenceClass *fence)
{
    if (!fence->mQuery)
    {
        D3D11_QUERY_DESC queryDesc;
        queryDesc.Query     = D3D11_QUERY_EVENT;
        queryDesc.MiscFlags = 0;

        Context11 *context11 = GetImplAs<Context11>(context);
        HRESULT result = fence->mRenderer->getDevice()->CreateQuery(&queryDesc, &fence->mQuery);
        ANGLE_TRY_HR(context11, result, "Failed to create event query");
    }

    fence->mRenderer->getDeviceContext()->End(fence->mQuery);
    return angle::Result::Continue;
}

template <class FenceClass>
angle::Result FenceTestHelper(const gl::Context *context,
                              FenceClass *fence,
                              bool flushCommandBuffer,
                              GLboolean *outFinished)
{
    ASSERT(fence->mQuery);

    UINT getDataFlags = (flushCommandBuffer ? 0 : D3D11_ASYNC_GETDATA_DONOTFLUSH);

    Context11 *context11 = GetImplAs<Context11>(context);
    HRESULT result =
        fence->mRenderer->getDeviceContext()->GetData(fence->mQuery, nullptr, 0, getDataFlags);
    ANGLE_TRY_HR(context11, result, "Failed to get query data");

    ASSERT(result == S_OK || result == S_FALSE);
    *outFinished = ((result == S_OK) ? GL_TRUE : GL_FALSE);
    return angle::Result::Continue;
}

//
// FenceNV11
//

FenceNV11::FenceNV11(Renderer11 *renderer) : FenceNVImpl(), mRenderer(renderer), mQuery(nullptr) {}

FenceNV11::~FenceNV11()
{
    SafeRelease(mQuery);
}

angle::Result FenceNV11::set(const gl::Context *context, GLenum condition)
{
    return FenceSetHelper(context, this);
}

angle::Result FenceNV11::test(const gl::Context *context, GLboolean *outFinished)
{
    return FenceTestHelper(context, this, true, outFinished);
}

angle::Result FenceNV11::finish(const gl::Context *context)
{
    GLboolean finished = GL_FALSE;

    int loopCount = 0;
    while (finished != GL_TRUE)
    {
        loopCount++;
        ANGLE_TRY(FenceTestHelper(context, this, true, &finished));

        bool checkDeviceLost = (loopCount % kPollingD3DDeviceLostCheckFrequency) == 0;
        if (checkDeviceLost && mRenderer->testDeviceLost())
        {
            ANGLE_TRY_HR(GetImplAs<Context11>(context), DXGI_ERROR_DRIVER_INTERNAL_ERROR,
                         "Device was lost while querying result of an event query.");
        }

        std::this_thread::yield();
    }

    return angle::Result::Continue;
}

//
// Sync11
//

// Important note on accurate timers in Windows:
//
// QueryPerformanceCounter has a few major issues, including being 10x as expensive to call
// as timeGetTime on laptops and "jumping" during certain hardware events.
//
// See the comments at the top of the Chromium source file "chromium/src/base/time/time_win.cc"
//   https://code.google.com/p/chromium/codesearch#chromium/src/base/time/time_win.cc
//
// We still opt to use QPC. In the present and moving forward, most newer systems will not suffer
// from buggy implementations.

Sync11::Sync11(Renderer11 *renderer) : SyncImpl(), mRenderer(renderer), mQuery(nullptr)
{
    LARGE_INTEGER counterFreqency = {};
    BOOL success                  = QueryPerformanceFrequency(&counterFreqency);
    ASSERT(success);

    mCounterFrequency = counterFreqency.QuadPart;
}

Sync11::~Sync11()
{
    SafeRelease(mQuery);
}

angle::Result Sync11::set(const gl::Context *context, GLenum condition, GLbitfield flags)
{
    ASSERT(condition == GL_SYNC_GPU_COMMANDS_COMPLETE && flags == 0);
    return FenceSetHelper(context, this);
}

angle::Result Sync11::clientWait(const gl::Context *context,
                                 GLbitfield flags,
                                 GLuint64 timeout,
                                 GLenum *outResult)
{
    ASSERT(outResult);

    bool flushCommandBuffer = ((flags & GL_SYNC_FLUSH_COMMANDS_BIT) != 0);

    *outResult = GL_WAIT_FAILED;

    GLboolean result = GL_FALSE;
    ANGLE_TRY(FenceTestHelper(context, this, flushCommandBuffer, &result));

    if (result == GL_TRUE)
    {
        *outResult = GL_ALREADY_SIGNALED;
        return angle::Result::Continue;
    }

    if (timeout == 0)
    {
        *outResult = GL_TIMEOUT_EXPIRED;
        return angle::Result::Continue;
    }

    LARGE_INTEGER currentCounter = {};
    BOOL success                 = QueryPerformanceCounter(&currentCounter);
    ASSERT(success);

    LONGLONG timeoutInSeconds = static_cast<LONGLONG>(timeout / 1000000000ull);
    LONGLONG endCounter       = currentCounter.QuadPart + mCounterFrequency * timeoutInSeconds;

    // Extremely unlikely, but if mCounterFrequency is large enough, endCounter can wrap
    if (endCounter < currentCounter.QuadPart)
    {
        endCounter = MAXLONGLONG;
    }

    int loopCount = 0;
    while (currentCounter.QuadPart < endCounter && !result)
    {
        loopCount++;
        std::this_thread::yield();
        success = QueryPerformanceCounter(&currentCounter);
        ASSERT(success);

        *outResult = GL_WAIT_FAILED;

        ANGLE_TRY(FenceTestHelper(context, this, flushCommandBuffer, &result));

        bool checkDeviceLost = (loopCount % kPollingD3DDeviceLostCheckFrequency) == 0;
        if (checkDeviceLost && mRenderer->testDeviceLost())
        {
            *outResult = GL_WAIT_FAILED;
            ANGLE_TRY_HR(GetImplAs<Context11>(context), DXGI_ERROR_DRIVER_INTERNAL_ERROR,
                         "Device was lost while querying result of an event query.");
        }
    }

    if (currentCounter.QuadPart >= endCounter)
    {
        *outResult = GL_TIMEOUT_EXPIRED;
    }
    else
    {
        *outResult = GL_CONDITION_SATISFIED;
    }

    return angle::Result::Continue;
}

angle::Result Sync11::serverWait(const gl::Context *context, GLbitfield flags, GLuint64 timeout)
{
    // Because our API is currently designed to be called from a single thread, we don't need to do
    // extra work for a server-side fence. GPU commands issued after the fence is created will
    // always be processed after the fence is signaled.
    return angle::Result::Continue;
}

angle::Result Sync11::getStatus(const gl::Context *context, GLint *outResult)
{
    GLboolean result = GL_FALSE;

    // The spec does not specify any way to report errors during the status test (e.g. device
    // lost) so we report the fence is unblocked in case of error or signaled.
    *outResult = GL_SIGNALED;
    ANGLE_TRY(FenceTestHelper(context, this, false, &result));

    *outResult = (result ? GL_SIGNALED : GL_UNSIGNALED);
    return angle::Result::Continue;
}

}  // namespace rx
