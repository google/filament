//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// QueryVk.cpp:
//    Implements the class methods for QueryVk.
//

#include "libANGLE/renderer/vulkan/QueryVk.h"
#include "libANGLE/Context.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/TransformFeedbackVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

#include "common/debug.h"

namespace rx
{

namespace
{
struct QueryReleaseHelper
{
    void operator()(vk::QueryHelper &&query) { queryPool->freeQuery(contextVk, &query); }

    ContextVk *contextVk;
    vk::DynamicQueryPool *queryPool;
};

bool IsRenderPassQuery(ContextVk *contextVk, gl::QueryType type)
{
    switch (type)
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
        case gl::QueryType::PrimitivesGenerated:
            return true;
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            return contextVk->getFeatures().supportsTransformFeedbackExtension.enabled;
        default:
            return false;
    }
}

bool IsEmulatedTransformFeedbackQuery(ContextVk *contextVk, gl::QueryType type)
{
    return type == gl::QueryType::TransformFeedbackPrimitivesWritten &&
           contextVk->getFeatures().emulateTransformFeedback.enabled;
}

bool IsPrimitivesGeneratedQueryShared(ContextVk *contextVk)
{
    return !contextVk->getFeatures().supportsPrimitivesGeneratedQuery.enabled &&
           !contextVk->getFeatures().supportsPipelineStatisticsQuery.enabled;
}

QueryVk *GetShareQuery(ContextVk *contextVk, gl::QueryType type)
{
    QueryVk *shareQuery = nullptr;

    // If the primitives generated query has its own dedicated Vulkan query, there's no sharing.
    if (!IsPrimitivesGeneratedQueryShared(contextVk))
    {
        return nullptr;
    }

    switch (type)
    {
        case gl::QueryType::PrimitivesGenerated:
            shareQuery = contextVk->getActiveRenderPassQuery(
                gl::QueryType::TransformFeedbackPrimitivesWritten);
            break;
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            shareQuery = contextVk->getActiveRenderPassQuery(gl::QueryType::PrimitivesGenerated);
            break;
        default:
            break;
    }

    return shareQuery;
}

// When a render pass starts/ends, onRenderPassStart/End  is called for all active queries.  For
// shared queries, the one that is called first would actually manage the query helper begin/end and
// allocation, and the one that follows would share it.  PrimitivesGenerated and
// TransformFeedbackPrimitivesWritten share queries, and the former is processed first.
QueryVk *GetOnRenderPassStartEndShareQuery(ContextVk *contextVk, gl::QueryType type)
{
    static_assert(
        gl::QueryType::PrimitivesGenerated < gl::QueryType::TransformFeedbackPrimitivesWritten,
        "incorrect assumption about the order in which queries are started in a render pass");

    if (type != gl::QueryType::TransformFeedbackPrimitivesWritten ||
        !IsPrimitivesGeneratedQueryShared(contextVk))
    {
        return nullptr;
    }

    // For TransformFeedbackPrimitivesWritten, return the already-processed PrimitivesGenerated
    // share query.
    return contextVk->getActiveRenderPassQuery(gl::QueryType::PrimitivesGenerated);
}
}  // anonymous namespace

QueryVk::QueryVk(gl::QueryType type)
    : QueryImpl(type),
      mTransformFeedbackPrimitivesDrawn(0),
      mCachedResult(0),
      mCachedResultValid(false)
{}

QueryVk::~QueryVk() = default;

angle::Result QueryVk::allocateQuery(ContextVk *contextVk)
{
    ASSERT(!mQueryHelper.isReferenced());
    mQueryHelper.setUnreferenced(new vk::RefCounted<vk::QueryHelper>);

    // When used with multiview, render pass queries write as many queries as the number of views.
    // Render pass queries are always allocated at the beginning of the render pass, so the number
    // of views is known at this time.
    uint32_t queryCount = 1;
    if (IsRenderPassQuery(contextVk, mType))
    {
        ASSERT(contextVk->hasActiveRenderPass());
        queryCount = std::max(contextVk->getCurrentViewCount(), 1u);
    }

    return contextVk->getQueryPool(mType)->allocateQuery(contextVk, &mQueryHelper.get(),
                                                         queryCount);
}

void QueryVk::assignSharedQuery(QueryVk *shareQuery)
{
    ASSERT(!mQueryHelper.isReferenced());
    ASSERT(shareQuery->mQueryHelper.isReferenced());
    mQueryHelper.copyUnreferenced(shareQuery->mQueryHelper);
}

void QueryVk::releaseQueries(ContextVk *contextVk)
{
    ASSERT(!IsEmulatedTransformFeedbackQuery(contextVk, mType));

    vk::DynamicQueryPool *queryPool = contextVk->getQueryPool(mType);

    // Free the main query
    if (mQueryHelper.isReferenced())
    {
        QueryReleaseHelper releaseHelper = {contextVk, queryPool};
        mQueryHelper.resetAndRelease(&releaseHelper);
    }
    // Free the secondary query used to emulate TimeElapsed
    queryPool->freeQuery(contextVk, &mQueryHelperTimeElapsedBegin);

    // Free any stashed queries used to support queries that start and stop with the render pass.
    releaseStashedQueries(contextVk);
}

void QueryVk::releaseStashedQueries(ContextVk *contextVk)
{
    vk::DynamicQueryPool *queryPool = contextVk->getQueryPool(mType);

    for (vk::Shared<vk::QueryHelper> &query : mStashedQueryHelpers)
    {
        ASSERT(query.isReferenced());

        QueryReleaseHelper releaseHelper = {contextVk, queryPool};
        query.resetAndRelease(&releaseHelper);
    }
    mStashedQueryHelpers.clear();
}

void QueryVk::onDestroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    if (!IsEmulatedTransformFeedbackQuery(contextVk, mType))
    {
        releaseQueries(contextVk);
    }
}

void QueryVk::stashQueryHelper()
{
    ASSERT(mQueryHelper.isReferenced());
    mStashedQueryHelpers.push_back(std::move(mQueryHelper));
    ASSERT(!mQueryHelper.isReferenced());
}

angle::Result QueryVk::onRenderPassStart(ContextVk *contextVk)
{
    ASSERT(IsRenderPassQuery(contextVk, mType));

    // If there is a query helper already, stash it and allocate a new one for this render pass.
    if (mQueryHelper.isReferenced())
    {
        stashQueryHelper();
    }

    QueryVk *shareQuery = GetOnRenderPassStartEndShareQuery(contextVk, mType);

    if (shareQuery)
    {
        assignSharedQuery(shareQuery);

        // shareQuery has already started the query.
        return angle::Result::Continue;
    }

    ANGLE_TRY(allocateQuery(contextVk));
    return mQueryHelper.get().beginRenderPassQuery(contextVk);
}

void QueryVk::onRenderPassEnd(ContextVk *contextVk)
{
    ASSERT(IsRenderPassQuery(contextVk, mType));

    QueryVk *shareQuery = GetOnRenderPassStartEndShareQuery(contextVk, mType);

    // If present, share query has already taken care of ending the query.
    // The query may not be referenced if it's a transform feedback query that was never resumed due
    // to transform feedback being paused when the render pass was broken.
    if (shareQuery == nullptr && mQueryHelper.isReferenced())
    {
        mQueryHelper.get().endRenderPassQuery(contextVk);
    }
}

angle::Result QueryVk::accumulateStashedQueryResult(ContextVk *contextVk, vk::QueryResult *result)
{
    for (vk::Shared<vk::QueryHelper> &query : mStashedQueryHelpers)
    {
        vk::QueryResult v(getQueryResultCount(contextVk));
        ANGLE_TRY(query.get().getUint64Result(contextVk, &v));
        *result += v;
    }
    releaseStashedQueries(contextVk);
    return angle::Result::Continue;
}

angle::Result QueryVk::setupBegin(ContextVk *contextVk)
{
    if (IsRenderPassQuery(contextVk, mType))
    {
        // Clean up query helpers from the previous begin/end call on the same query.  Only
        // necessary for in-render-pass queries.  The other queries can reuse query helpers as they
        // are able to reset it ouside the render pass where they are recorded.
        if (mQueryHelper.isReferenced())
        {
            releaseQueries(contextVk);
        }

        // If either of TransformFeedbackPrimitivesWritten or PrimitivesGenerated queries are
        // already active when the other one is begun, we have to switch to a new query helper (if
        // in render pass), and have them share the query helper from here on.

        // If this is a transform feedback query, see if the other transform feedback query is
        // already active.
        QueryVk *shareQuery = GetShareQuery(contextVk, mType);

        // If so, make the other query stash its results and continue with a new query helper.
        if (contextVk->hasActiveRenderPass())
        {
            if (shareQuery)
            {
                // This serves the following scenario (TF = TransformFeedbackPrimitivesWritten, PG =
                // PrimitivesGenerated):
                //
                // - TF starts <-- QueryHelper1 starts
                // - Draw
                // - PG starts <-- QueryHelper1 stashed in TF, TF starts QueryHelper2,
                //                                             PG shares QueryHelper2
                // - Draw
                shareQuery->onRenderPassEnd(contextVk);
                shareQuery->stashQueryHelper();
                ANGLE_TRY(shareQuery->allocateQuery(contextVk));

                // Share the query helper with the other transform feedback query.  After
                // |setupBegin()| returns, they query helper is started on behalf of the shared
                // query.
                assignSharedQuery(shareQuery);
            }
        }
        else
        {
            // Keep the query helper unallocated.  When the render pass starts, a new one
            // will be allocated / shared.
            return angle::Result::Continue;
        }
    }

    // If no query helper, create a new one.
    if (!mQueryHelper.isReferenced())
    {
        ANGLE_TRY(allocateQuery(contextVk));
    }

    return angle::Result::Continue;
}

angle::Result QueryVk::begin(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    // Ensure that we start with the right RenderPass when we begin a new query.
    if (contextVk->getState().isDrawFramebufferBindingDirty())
    {
        ANGLE_TRY(contextVk->flushCommandsAndEndRenderPass(
            RenderPassClosureReason::FramebufferBindingChange));
    }

    mCachedResultValid = false;

    // Transform feedback query is handled by a CPU-calculated value when emulated.
    if (IsEmulatedTransformFeedbackQuery(contextVk, mType))
    {
        ASSERT(!contextVk->getFeatures().supportsTransformFeedbackExtension.enabled);
        mTransformFeedbackPrimitivesDrawn = 0;

        return angle::Result::Continue;
    }

    ANGLE_TRY(setupBegin(contextVk));

    switch (mType)
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
        case gl::QueryType::PrimitivesGenerated:
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            ANGLE_TRY(contextVk->beginRenderPassQuery(this));
            break;
        case gl::QueryType::Timestamp:
            ANGLE_TRY(mQueryHelper.get().beginQuery(contextVk));
            break;
        case gl::QueryType::TimeElapsed:
            // Note: TimeElapsed is implemented by using two Timestamp queries and taking the diff.
            if (!mQueryHelperTimeElapsedBegin.valid())
            {
                // Note that timestamp queries are not allowed with multiview, so query count is
                // always 1.
                ANGLE_TRY(contextVk->getQueryPool(mType)->allocateQuery(
                    contextVk, &mQueryHelperTimeElapsedBegin, 1));
            }

            ANGLE_TRY(mQueryHelperTimeElapsedBegin.flushAndWriteTimestamp(contextVk));
            break;
        default:
            UNREACHABLE();
            break;
    }

    return angle::Result::Continue;
}

angle::Result QueryVk::end(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    // Transform feedback query is handled by a CPU-calculated value when emulated.
    if (IsEmulatedTransformFeedbackQuery(contextVk, mType))
    {
        ASSERT(contextVk->getFeatures().emulateTransformFeedback.enabled);
        mCachedResult = mTransformFeedbackPrimitivesDrawn;

        // There could be transform feedback in progress, so add the primitives drawn so far
        // from the current transform feedback object.
        gl::TransformFeedback *transformFeedback =
            context->getState().getCurrentTransformFeedback();
        if (transformFeedback)
        {
            mCachedResult += transformFeedback->getPrimitivesDrawn();
        }
        mCachedResultValid = true;

        return angle::Result::Continue;
    }

    switch (mType)
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
        case gl::QueryType::PrimitivesGenerated:
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
        {
            QueryVk *shareQuery = GetShareQuery(contextVk, mType);
            ASSERT(shareQuery == nullptr || &mQueryHelper.get() == &shareQuery->mQueryHelper.get());

            ANGLE_TRY(contextVk->endRenderPassQuery(this));

            // If another query shares its query helper with this one, its query has just ended!
            // Make it stash its query and create a new one so it can continue.
            if (shareQuery && shareQuery->mQueryHelper.isReferenced())
            {
                // This serves the following scenario (TF = TransformFeedbackPrimitivesWritten, PG =
                // PrimitivesGenerated):
                //
                // - TF starts <-- QueryHelper1 starts
                // - PG starts <-- PG shares QueryHelper1
                // - Draw
                // - TF ends   <-- Results = QueryHelper1,
                //                 QueryHelper1 stashed in PG, PG starts QueryHelper2
                // - Draw
                // - PG ends   <-- Results = QueryHelper1 + QueryHelper2
                if (contextVk->hasActiveRenderPass())
                {
                    ANGLE_TRY(shareQuery->onRenderPassStart(contextVk));
                }
            }
            break;
        }
        case gl::QueryType::Timestamp:
            ANGLE_TRY(mQueryHelper.get().endQuery(contextVk));
            break;
        case gl::QueryType::TimeElapsed:
            ANGLE_TRY(mQueryHelper.get().flushAndWriteTimestamp(contextVk));
            break;
        default:
            UNREACHABLE();
            break;
    }

    return angle::Result::Continue;
}

angle::Result QueryVk::queryCounter(const gl::Context *context)
{
    ASSERT(mType == gl::QueryType::Timestamp);
    ContextVk *contextVk = vk::GetImpl(context);

    mCachedResultValid = false;

    if (!mQueryHelper.isReferenced())
    {
        ANGLE_TRY(allocateQuery(contextVk));
    }

    return mQueryHelper.get().flushAndWriteTimestamp(contextVk);
}

bool QueryVk::isCurrentlyInUse(vk::Renderer *renderer) const
{
    ASSERT(mQueryHelper.isReferenced());
    return !renderer->hasResourceUseFinished(mQueryHelper.get().getResourceUse());
}

angle::Result QueryVk::finishRunningCommands(ContextVk *contextVk)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    // Caller already made sure query has been submitted.
    if (!renderer->hasResourceUseFinished(mQueryHelper.get().getResourceUse()))
    {
        ANGLE_TRY(renderer->finishResourceUse(contextVk, mQueryHelper.get().getResourceUse()));
    }

    // Since mStashedQueryHelpers are older than mQueryHelper, these must also finished.
    for (vk::Shared<vk::QueryHelper> &query : mStashedQueryHelpers)
    {
        ASSERT(renderer->hasResourceUseFinished(query.get().getResourceUse()));
    }
    return angle::Result::Continue;
}

angle::Result QueryVk::getResult(const gl::Context *context, bool wait)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "QueryVk::getResult");

    if (mCachedResultValid)
    {
        return angle::Result::Continue;
    }

    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    // Support the pathological case where begin/end is called on a render pass query but without
    // any render passes in between.  In this case, the query helper is never allocated.
    if (!mQueryHelper.isReferenced())
    {
        ASSERT(IsRenderPassQuery(contextVk, mType));
        mCachedResult      = 0;
        mCachedResultValid = true;
        return angle::Result::Continue;
    }

    // glGetQueryObject* requires an implicit flush of the command buffers to guarantee execution in
    // finite time.
    // Note regarding time-elapsed: end should have been called after begin, so flushing when end
    // has pending work should flush begin too.
    // We only need to check mQueryHelper, not mStashedQueryHelper, since they are always in order.
    if (contextVk->hasUnsubmittedUse(mQueryHelper.get()))
    {
        ANGLE_TRY(contextVk->flushAndSubmitCommands(nullptr, nullptr,
                                                    RenderPassClosureReason::GetQueryResult));

        ASSERT(contextVk->getRenderer()->hasResourceUseSubmitted(
            mQueryHelperTimeElapsedBegin.getResourceUse()));
        ASSERT(
            contextVk->getRenderer()->hasResourceUseSubmitted(mQueryHelper.get().getResourceUse()));
    }

    // If the command buffer this query is being written to is still in flight and uses
    // vkCmdResetQueryPool, its reset command may not have been performed by the GPU yet.  To avoid
    // a race condition in this case, wait for the batch to finish first before querying (or return
    // not-ready if not waiting).
    if (isCurrentlyInUse(renderer) &&
        (!renderer->getFeatures().supportsHostQueryReset.enabled ||
         renderer->getFeatures().forceWaitForSubmissionToCompleteForQueryResult.enabled))
    {
        // The query might appear busy because there was no check for completed commands
        // recently. Do that now and see if the query is still busy.  If the application is
        // looping until the query results become available, there wouldn't be any forward
        // progress without this.
        ANGLE_TRY(renderer->checkCompletedCommandsAndCleanup(contextVk));

        if (isCurrentlyInUse(renderer))
        {
            if (!wait)
            {
                return angle::Result::Continue;
            }
            ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_HIGH,
                                  "GPU stall due to waiting on uncompleted query");

            // Assert that the work has been sent to the GPU
            ASSERT(!contextVk->hasUnsubmittedUse(mQueryHelper.get()));
            ANGLE_TRY(finishRunningCommands(contextVk));
        }
    }

    // If its a render pass query, the current query helper must have commands recorded (i.e. it's
    // not a newly allocated query with the actual queries all stashed).  If this is not respected
    // and !wait, |mQueryHelper.get().getUint64ResultNonBlocking()| will tell that the result is
    // readily available, which may not be true.  The subsequent calls to |getUint64Result()| on the
    // stashed queries will incur a wait that is not desired by the application.
    ASSERT(!IsRenderPassQuery(contextVk, mType) || mQueryHelper.get().hasSubmittedCommands());

    vk::QueryResult result(getQueryResultCount(contextVk));

    if (wait)
    {
        ANGLE_TRY(mQueryHelper.get().getUint64Result(contextVk, &result));
        ANGLE_TRY(accumulateStashedQueryResult(contextVk, &result));
    }
    else
    {
        bool available = false;
        ANGLE_TRY(mQueryHelper.get().getUint64ResultNonBlocking(contextVk, &result, &available));
        if (!available)
        {
            // If the results are not ready, do nothing.  mCachedResultValid remains false.
            return angle::Result::Continue;
        }
        ANGLE_TRY(accumulateStashedQueryResult(contextVk, &result));
    }

    double timestampPeriod = renderer->getPhysicalDeviceProperties().limits.timestampPeriod;

    // Fix up the results to what OpenGL expects.
    switch (mType)
    {
        case gl::QueryType::AnySamples:
        case gl::QueryType::AnySamplesConservative:
            // OpenGL query result in these cases is binary
            mCachedResult = !!result.getResult(vk::QueryResult::kDefaultResultIndex);
            break;
        case gl::QueryType::Timestamp:
            mCachedResult = static_cast<uint64_t>(
                result.getResult(vk::QueryResult::kDefaultResultIndex) * timestampPeriod);
            break;
        case gl::QueryType::TimeElapsed:
        {
            vk::QueryResult timeElapsedBegin(1);

            // Since the result of the end query of time-elapsed is already available, the
            // result of begin query must be available too.
            ANGLE_TRY(mQueryHelperTimeElapsedBegin.getUint64Result(contextVk, &timeElapsedBegin));

            uint64_t delta = result.getResult(vk::QueryResult::kDefaultResultIndex) -
                             timeElapsedBegin.getResult(vk::QueryResult::kDefaultResultIndex);
            mCachedResult = static_cast<uint64_t>(delta * timestampPeriod);
            break;
        }
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            mCachedResult =
                result.getResult(IsPrimitivesGeneratedQueryShared(contextVk)
                                     ? vk::QueryResult::kTransformFeedbackPrimitivesWrittenIndex
                                     : vk::QueryResult::kDefaultResultIndex);
            break;
        case gl::QueryType::PrimitivesGenerated:
            mCachedResult = result.getResult(IsPrimitivesGeneratedQueryShared(contextVk)
                                                 ? vk::QueryResult::kPrimitivesGeneratedIndex
                                                 : vk::QueryResult::kDefaultResultIndex);
            break;
        default:
            UNREACHABLE();
            break;
    }

    mCachedResultValid = true;
    return angle::Result::Continue;
}
angle::Result QueryVk::getResult(const gl::Context *context, GLint *params)
{
    ANGLE_TRY(getResult(context, true));
    *params = static_cast<GLint>(mCachedResult);
    return angle::Result::Continue;
}

angle::Result QueryVk::getResult(const gl::Context *context, GLuint *params)
{
    ANGLE_TRY(getResult(context, true));
    *params = static_cast<GLuint>(mCachedResult);
    return angle::Result::Continue;
}

angle::Result QueryVk::getResult(const gl::Context *context, GLint64 *params)
{
    ANGLE_TRY(getResult(context, true));
    *params = static_cast<GLint64>(mCachedResult);
    return angle::Result::Continue;
}

angle::Result QueryVk::getResult(const gl::Context *context, GLuint64 *params)
{
    ANGLE_TRY(getResult(context, true));
    *params = mCachedResult;
    return angle::Result::Continue;
}

angle::Result QueryVk::isResultAvailable(const gl::Context *context, bool *available)
{
    ANGLE_TRY(getResult(context, false));
    *available = mCachedResultValid;

    return angle::Result::Continue;
}

void QueryVk::onTransformFeedbackEnd(GLsizeiptr primitivesDrawn)
{
    mTransformFeedbackPrimitivesDrawn += primitivesDrawn;
}

uint32_t QueryVk::getQueryResultCount(ContextVk *contextVk) const
{
    switch (mType)
    {
        case gl::QueryType::PrimitivesGenerated:
            return IsPrimitivesGeneratedQueryShared(contextVk) ? 2 : 1;
        case gl::QueryType::TransformFeedbackPrimitivesWritten:
            return 2;
        default:
            return 1;
    }
}
}  // namespace rx
