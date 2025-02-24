//
// Copyright (c) 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QueryMtl.mm:
//    Defines the class interface for QueryMtl, implementing QueryImpl.
//

#include "libANGLE/renderer/metal/QueryMtl.h"

#include "libANGLE/renderer/metal/ContextMtl.h"

namespace rx
{
QueryMtl::QueryMtl(gl::QueryType type) : QueryImpl(type) {}

QueryMtl::~QueryMtl() {}

void QueryMtl::onDestroy(const gl::Context *context)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    if (!getAllocatedVisibilityOffsets().empty())
    {
        contextMtl->onOcclusionQueryDestroy(context, this);
    }
    mVisibilityResultBuffer = nullptr;
}

angle::Result QueryMtl::begin(const gl::Context *context)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    switch (getType())
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
            if (!mVisibilityResultBuffer)
            {
                // Allocate buffer
                ANGLE_TRY(mtl::Buffer::MakeBuffer(contextMtl, mtl::kOcclusionQueryResultSize,
                                                  nullptr, &mVisibilityResultBuffer));

                ANGLE_MTL_OBJC_SCOPE
                {
                    mVisibilityResultBuffer->get().label =
                        [NSString stringWithFormat:@"QueryMtl=%p", this];
                }
            }

            ANGLE_TRY(contextMtl->onOcclusionQueryBegin(context, this));
            break;
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            mTransformFeedbackPrimitivesDrawn = 0;
            break;
        case gl::QueryType::TimeElapsed:
        {
            // End any command buffer being encoded, to get a clean boundary for beginning
            // measurement.
            contextMtl->flushCommandBuffer(mtl::NoWait);
            mtl::CommandQueue &queue = contextMtl->getDisplay()->cmdQueue();
            if (mTimeElapsedEntry != 0)
            {
                queue.deleteTimeElapsedEntry(mTimeElapsedEntry);
                mTimeElapsedEntry = 0;
            }
            mTimeElapsedEntry = queue.allocateTimeElapsedEntry();
            queue.setActiveTimeElapsedEntry(mTimeElapsedEntry);
            break;
        }
        default:
            UNIMPLEMENTED();
            break;
    }

    return angle::Result::Continue;
}
angle::Result QueryMtl::end(const gl::Context *context)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    switch (getType())
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
            contextMtl->onOcclusionQueryEnd(context, this);
            break;
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            onTransformFeedbackEnd(context);
            break;
        case gl::QueryType::TimeElapsed:
        {
            // End any command buffer being encoded, to get a clean boundary for ending measurement.
            contextMtl->flushCommandBuffer(mtl::NoWait);
            mtl::CommandQueue &queue = contextMtl->getDisplay()->cmdQueue();
            queue.setActiveTimeElapsedEntry(0);
            break;
        }
        default:
            UNIMPLEMENTED();
            break;
    }
    return angle::Result::Continue;
}
angle::Result QueryMtl::queryCounter(const gl::Context *context)
{
    UNIMPLEMENTED();
    return angle::Result::Continue;
}

template <typename T>
angle::Result QueryMtl::waitAndGetResult(const gl::Context *context, T *params)
{
    ASSERT(params);
    ContextMtl *contextMtl = mtl::GetImpl(context);
    switch (getType())
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
        {
            ASSERT(mVisibilityResultBuffer);
            if (mVisibilityResultBuffer->hasPendingWorks(contextMtl))
            {
                contextMtl->flushCommandBuffer(mtl::NoWait);
            }
            // map() will wait for the pending GPU works to finish
            const uint8_t *visibilityResultBytes = mVisibilityResultBuffer->mapReadOnly(contextMtl);
            uint64_t queryResult;
            memcpy(&queryResult, visibilityResultBytes, sizeof(queryResult));
            mVisibilityResultBuffer->unmap(contextMtl);

            *params = queryResult ? GL_TRUE : GL_FALSE;
        }
        break;
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            *params = static_cast<T>(mTransformFeedbackPrimitivesDrawn);
            break;
        case gl::QueryType::TimeElapsed:
        {
            ASSERT(mTimeElapsedEntry != 0);
            mtl::CommandQueue &queue = contextMtl->getDisplay()->cmdQueue();
            if (!queue.isTimeElapsedEntryComplete(mTimeElapsedEntry))
            {
                contextMtl->flushCommandBuffer(mtl::WaitUntilFinished);
            }
            ASSERT(queue.isTimeElapsedEntryComplete(mTimeElapsedEntry));
            double nanos    = queue.getTimeElapsedEntryInSeconds(mTimeElapsedEntry) * 1e9;
            uint64_t result = static_cast<uint64_t>(nanos);
            *params         = static_cast<T>(result);
            break;
        }
        default:
            UNIMPLEMENTED();
            break;
    }
    return angle::Result::Continue;
}

angle::Result QueryMtl::isResultAvailable(const gl::Context *context, bool *available)
{
    ASSERT(available);
    ContextMtl *contextMtl = mtl::GetImpl(context);
    // glGetQueryObjectuiv implicitly flush any pending works related to the query
    switch (getType())
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
            ASSERT(mVisibilityResultBuffer);
            if (mVisibilityResultBuffer->hasPendingWorks(contextMtl))
            {
                contextMtl->flushCommandBuffer(mtl::NoWait);
            }

            *available = !mVisibilityResultBuffer->isBeingUsedByGPU(contextMtl);
            break;
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            *available = true;
            break;
        case gl::QueryType::TimeElapsed:
            *available =
                contextMtl->getDisplay()->cmdQueue().isTimeElapsedEntryComplete(mTimeElapsedEntry);
            break;
        default:
            UNIMPLEMENTED();
            break;
    }
    return angle::Result::Continue;
}

angle::Result QueryMtl::getResult(const gl::Context *context, GLint *params)
{
    return waitAndGetResult(context, params);
}
angle::Result QueryMtl::getResult(const gl::Context *context, GLuint *params)
{
    return waitAndGetResult(context, params);
}
angle::Result QueryMtl::getResult(const gl::Context *context, GLint64 *params)
{
    return waitAndGetResult(context, params);
}
angle::Result QueryMtl::getResult(const gl::Context *context, GLuint64 *params)
{
    return waitAndGetResult(context, params);
}

void QueryMtl::resetVisibilityResult(ContextMtl *contextMtl)
{
    // Occlusion query buffer must be allocated in QueryMtl::begin
    ASSERT(mVisibilityResultBuffer);

    // Fill the query's buffer with zeros
    auto blitEncoder = contextMtl->getBlitCommandEncoder();
    blitEncoder->fillBuffer(mVisibilityResultBuffer, NSMakeRange(0, mtl::kOcclusionQueryResultSize),
                            0);
    mVisibilityResultBuffer->syncContent(contextMtl, blitEncoder);
}

void QueryMtl::onTransformFeedbackEnd(const gl::Context *context)
{
    gl::TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
    if (transformFeedback)
    {
        mTransformFeedbackPrimitivesDrawn += transformFeedback->getPrimitivesDrawn();
    }
}

void QueryMtl::onContextMakeCurrent(const gl::Context *context)
{
    // At present this should only be called for time elapsed queries.
    ASSERT(getType() == gl::QueryType::TimeElapsed);
    ContextMtl *contextMtl = mtl::GetImpl(context);
    contextMtl->getDisplay()->cmdQueue().setActiveTimeElapsedEntry(mTimeElapsedEntry);
}

void QueryMtl::onContextUnMakeCurrent(const gl::Context *context)
{
    // At present this should only be called for time elapsed queries.
    ASSERT(getType() == gl::QueryType::TimeElapsed);
    ContextMtl *contextMtl = mtl::GetImpl(context);
    contextMtl->getDisplay()->cmdQueue().setActiveTimeElapsedEntry(0);
}

}  // namespace rx
