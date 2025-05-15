/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "OpenGLTimerQuery.h"

#include "GLUtils.h"
#include "OpenGLDriver.h"

#include <backend/Platform.h>
#include <backend/platforms/OpenGLPlatform.h>
#include <backend/DriverEnums.h>

#include <private/utils/Tracing.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Mutex.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <new>
#include <utility>

#include <stdint.h>

namespace filament::backend {

using namespace backend;
using namespace GLUtils;

class OpenGLDriver;

// ------------------------------------------------------------------------------------------------

bool TimerQueryFactory::mGpuTimeSupported = false;

TimerQueryFactoryInterface* TimerQueryFactory::init(
        OpenGLPlatform& platform, OpenGLContext& context) noexcept {
    (void)context;

    TimerQueryFactoryInterface* impl = nullptr;

#if defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_EXT_disjoint_timer_query)
    if (context.ext.EXT_disjoint_timer_query) {
        // timer queries are available
        if (context.bugs.dont_use_timer_query && platform.canCreateFence()) {
            // however, they don't work well, revert to using fences if we can.
            impl = new(std::nothrow) TimerQueryFenceFactory(platform);
        } else {
            impl = new(std::nothrow) TimerQueryNativeFactory(context);
        }
        mGpuTimeSupported = true;
    } else
#endif
    if (platform.canCreateFence()) {
        // no timer queries, but we can use fences
        impl = new(std::nothrow) TimerQueryFenceFactory(platform);
        mGpuTimeSupported = true;
    } else {
        // no queries, no fences -- that's a problem
        impl = new(std::nothrow) TimerQueryFallbackFactory();
        mGpuTimeSupported = false;
    }
    assert_invariant(impl);
    return impl;
}

// ------------------------------------------------------------------------------------------------

TimerQueryFactoryInterface::~TimerQueryFactoryInterface() = default;

// This is a backend synchronous call
TimerQueryResult TimerQueryFactoryInterface::getTimerQueryValue(
        GLTimerQuery* tq, uint64_t* elapsedTime) noexcept {
    if (UTILS_LIKELY(tq->state)) {
        int64_t const elapsed = tq->state->elapsed.load(std::memory_order_relaxed);
        if (elapsed > 0) {
            *elapsedTime = elapsed;
            return TimerQueryResult::AVAILABLE;
        }
        return TimerQueryResult(elapsed);
    }
    return TimerQueryResult::ERROR;
}

// ------------------------------------------------------------------------------------------------

#if defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_EXT_disjoint_timer_query)

TimerQueryNativeFactory::TimerQueryNativeFactory(OpenGLContext& context)
        : mContext(context) {
}

TimerQueryNativeFactory::~TimerQueryNativeFactory() = default;

void TimerQueryNativeFactory::createTimerQuery(GLTimerQuery* tq) {
    assert_invariant(!tq->state);

    tq->state = std::make_shared<GLTimerQuery::State>();
    mContext.procs.genQueries(1u, &tq->state->gl.query);
    CHECK_GL_ERROR(utils::slog.e)
}

void TimerQueryNativeFactory::destroyTimerQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);

    mContext.procs.deleteQueries(1u, &tq->state->gl.query);
    CHECK_GL_ERROR(utils::slog.e)

    tq->state.reset();
}

void TimerQueryNativeFactory::beginTimeElapsedQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);

    tq->state->elapsed.store(int64_t(TimerQueryResult::NOT_READY), std::memory_order_relaxed);
    mContext.procs.beginQuery(GL_TIME_ELAPSED, tq->state->gl.query);
    CHECK_GL_ERROR(utils::slog.e)
}

void TimerQueryNativeFactory::endTimeElapsedQuery(OpenGLDriver& driver, GLTimerQuery* tq) {
    assert_invariant(tq->state);

    mContext.procs.endQuery(GL_TIME_ELAPSED);
    CHECK_GL_ERROR(utils::slog.e)

    std::weak_ptr<GLTimerQuery::State> const weak = tq->state;

    driver.runEveryNowAndThen([&context = mContext, weak]() -> bool {
        auto state = weak.lock();
        if (!state) {
            // The timer query state has been destroyed on the way, very likely due to the IBL
            // prefilter context destruction. We still return true to get this element removed from
            // the query list.
            return true;
        }

        GLuint available = 0;
        context.procs.getQueryObjectuiv(state->gl.query, GL_QUERY_RESULT_AVAILABLE, &available);
        CHECK_GL_ERROR(utils::slog.e)
        if (!available) {
            // we need to try this one again later
            return false;
        }
        GLuint64 elapsedTime = 0;
        // we won't end-up here if we're on ES and don't have GL_EXT_disjoint_timer_query
        context.procs.getQueryObjectui64v(state->gl.query, GL_QUERY_RESULT, &elapsedTime);
        state->elapsed.store((int64_t)elapsedTime, std::memory_order_relaxed);

        return true;
    });
}

#endif

// ------------------------------------------------------------------------------------------------

TimerQueryFenceFactory::TimerQueryFenceFactory(OpenGLPlatform& platform)
        : mPlatform(platform) {
    mQueue.reserve(2);
    mThread = std::thread([this]() {
        utils::JobSystem::setThreadName("OpenGLTimerQueryFence");
        utils::JobSystem::setThreadPriority(utils::JobSystem::Priority::URGENT_DISPLAY);
        auto& queue = mQueue;
        bool exitRequested;
        do {
            std::unique_lock<utils::Mutex> lock(mLock);
            mCondition.wait(lock, [this, &queue]() -> bool {
                return mExitRequested || !queue.empty();
            });
            exitRequested = mExitRequested;
            if (!queue.empty()) {
                Job const job(queue.front());
                queue.erase(queue.begin());
                lock.unlock();
                job();
            }
        } while (!exitRequested);
    });
}

TimerQueryFenceFactory::~TimerQueryFenceFactory() {
    assert_invariant(mQueue.empty());
    if (mThread.joinable()) {
        std::unique_lock<utils::Mutex> lock(mLock);
        mExitRequested = true;
        mCondition.notify_one();
        lock.unlock();
        if (mThread.joinable()) {
            mThread.join();
        }
    }
}

void TimerQueryFenceFactory::push(TimerQueryFenceFactory::Job&& job) {
    std::unique_lock<utils::Mutex> const lock(mLock);
    mQueue.push_back(std::move(job));
    mCondition.notify_one();
}

void TimerQueryFenceFactory::createTimerQuery(GLTimerQuery* tq) {
    assert_invariant(!tq->state);
    tq->state = std::make_shared<GLTimerQuery::State>();
}

void TimerQueryFenceFactory::destroyTimerQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
    tq->state.reset();
}

void TimerQueryFenceFactory::beginTimeElapsedQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
    tq->state->elapsed.store(int64_t(TimerQueryResult::NOT_READY), std::memory_order_relaxed);

    std::weak_ptr<GLTimerQuery::State> const weak = tq->state;

    // FIXME: this implementation of beginTimeElapsedQuery is usually wrong; it ends up
    //    measuring the current CPU time because the fence signals immediately (usually there is
    //    no work on the GPU at this point). We could workaround this by sending a small glClear
    //    on a dummy target for instance, or somehow latch the begin time at the next renderpass
    //    start.

    push([&platform = mPlatform, fence = mPlatform.createFence(), weak]() {
        auto state = weak.lock();
        if (state) {
            platform.waitFence(fence, FENCE_WAIT_FOR_EVER);
            state->then = clock::now().time_since_epoch().count();
            FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT);
            FILAMENT_TRACING_ASYNC_BEGIN(FILAMENT_TRACING_CATEGORY_FILAMENT, "OpenGLTimerQueryFence", intptr_t(state.get()));
        }
        platform.destroyFence(fence);
    });
}

void TimerQueryFenceFactory::endTimeElapsedQuery(OpenGLDriver&, GLTimerQuery* tq) {
    assert_invariant(tq->state);
    std::weak_ptr<GLTimerQuery::State> const weak = tq->state;

    push([&platform = mPlatform, fence = mPlatform.createFence(), weak]() {
        auto state = weak.lock();
        if (state) {
            platform.waitFence(fence, FENCE_WAIT_FOR_EVER);
            int64_t const now = clock::now().time_since_epoch().count();
            state->elapsed.store(now - state->then, std::memory_order_relaxed);
            FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT);
            FILAMENT_TRACING_ASYNC_END(FILAMENT_TRACING_CATEGORY_FILAMENT, "OpenGLTimerQueryFence", intptr_t(state.get()));
        }
        platform.destroyFence(fence);
    });
}

// ------------------------------------------------------------------------------------------------

TimerQueryFallbackFactory::TimerQueryFallbackFactory() = default;

TimerQueryFallbackFactory::~TimerQueryFallbackFactory() = default;

void TimerQueryFallbackFactory::createTimerQuery(GLTimerQuery* tq) {
    assert_invariant(!tq->state);
    tq->state = std::make_shared<GLTimerQuery::State>();
}

void TimerQueryFallbackFactory::destroyTimerQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
    tq->state.reset();
}

void TimerQueryFallbackFactory::beginTimeElapsedQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
    // this implementation measures the CPU time, but we have no h/w support
    tq->state->then = clock::now().time_since_epoch().count();
    tq->state->elapsed.store(int64_t(TimerQueryResult::NOT_READY), std::memory_order_relaxed);
}

void TimerQueryFallbackFactory::endTimeElapsedQuery(OpenGLDriver&, GLTimerQuery* tq) {
    assert_invariant(tq->state);
    // this implementation measures the CPU time, but we have no h/w support
    int64_t const now = clock::now().time_since_epoch().count();
    tq->state->elapsed.store(now - tq->state->then, std::memory_order_relaxed);
}

} // namespace filament::backend
