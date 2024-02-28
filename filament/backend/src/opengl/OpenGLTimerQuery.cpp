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

#include <backend/platforms/OpenGLPlatform.h>

#include <utils/compiler.h>
#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Systrace.h>
#include <utils/debug.h>

namespace filament::backend {

using namespace backend;
using namespace GLUtils;

// ------------------------------------------------------------------------------------------------

bool OpenGLTimerQueryFactory::mGpuTimeSupported = false;

OpenGLTimerQueryInterface* OpenGLTimerQueryFactory::init(
        OpenGLPlatform& platform, OpenGLDriver& driver) noexcept {
    (void)driver;

    OpenGLTimerQueryInterface* impl;

#if defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_EXT_disjoint_timer_query)
    auto& context = driver.getContext();
    if (context.ext.EXT_disjoint_timer_query) {
        // timer queries are available
        if (context.bugs.dont_use_timer_query && platform.canCreateFence()) {
            // however, they don't work well, revert to using fences if we can.
            impl = new(std::nothrow) OpenGLTimerQueryFence(platform);
        } else {
            impl = new(std::nothrow) TimerQueryNative(driver);
        }
        mGpuTimeSupported = true;
    } else
#endif
    if (platform.canCreateFence()) {
        // no timer queries, but we can use fences
        impl = new(std::nothrow) OpenGLTimerQueryFence(platform);
        mGpuTimeSupported = true;
    } else {
        // no queries, no fences -- that's a problem
        impl = new(std::nothrow) TimerQueryFallback();
        mGpuTimeSupported = false;
    }
    return impl;
}

// ------------------------------------------------------------------------------------------------

OpenGLTimerQueryInterface::~OpenGLTimerQueryInterface() = default;

// This is a backend synchronous call
TimerQueryResult OpenGLTimerQueryInterface::getTimerQueryValue(
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

TimerQueryNative::TimerQueryNative(OpenGLDriver& driver)
        : mDriver(driver) {
}

TimerQueryNative::~TimerQueryNative() = default;

void TimerQueryNative::createTimerQuery(GLTimerQuery* tq) {
    if (UTILS_UNLIKELY(!tq->state)) {
        tq->state = std::make_shared<GLTimerQuery::State>();
    }
    mDriver.getContext().procs.genQueries(1u, &tq->state->gl.query);
    CHECK_GL_ERROR(utils::slog.e)
}

void TimerQueryNative::destroyTimerQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
    mDriver.getContext().procs.deleteQueries(1u, &tq->state->gl.query);
    CHECK_GL_ERROR(utils::slog.e)
}

void TimerQueryNative::beginTimeElapsedQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
    tq->state->elapsed.store(int64_t(TimerQueryResult::NOT_READY));
    mDriver.getContext().procs.beginQuery(GL_TIME_ELAPSED, tq->state->gl.query);
    CHECK_GL_ERROR(utils::slog.e)
}

void TimerQueryNative::endTimeElapsedQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
    auto& context = mDriver.getContext();
    context.procs.endQuery(GL_TIME_ELAPSED);
    CHECK_GL_ERROR(utils::slog.e)

    std::weak_ptr<GLTimerQuery::State> const weak = tq->state;

    mDriver.runEveryNowAndThen([&context, weak]() -> bool {
        auto state = weak.lock();
        if (state) {
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
        } else {
            state->elapsed.store(int64_t(TimerQueryResult::ERROR), std::memory_order_relaxed);
        }
        return true;
    });
}

#endif

// ------------------------------------------------------------------------------------------------

OpenGLTimerQueryFence::OpenGLTimerQueryFence(OpenGLPlatform& platform)
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

OpenGLTimerQueryFence::~OpenGLTimerQueryFence() {
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

void OpenGLTimerQueryFence::enqueue(OpenGLTimerQueryFence::Job&& job) {
    std::unique_lock<utils::Mutex> const lock(mLock);
    mQueue.push_back(std::forward<Job>(job));
    mCondition.notify_one();
}

void OpenGLTimerQueryFence::createTimerQuery(GLTimerQuery* tq) {
    if (UTILS_UNLIKELY(!tq->state)) {
        tq->state = std::make_shared<GLTimerQuery::State>();
    }
}

void OpenGLTimerQueryFence::destroyTimerQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
}

void OpenGLTimerQueryFence::beginTimeElapsedQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
    tq->state->elapsed.store(0);

    Platform::Fence* fence = mPlatform.createFence();
    std::weak_ptr<GLTimerQuery::State> const weak = tq->state;

    // FIXME: this implementation of beginTimeElapsedQuery is usually wrong; it ends up
    //    measuring the current CPU time because the fence signals immediately (usually there is
    //    no work on the GPU at this point). We could workaround this by sending a small glClear
    //    on a dummy target for instance, or somehow latch the begin time at the next renderpass
    //    start.

    push([&platform = mPlatform, fence, weak]() {
        auto state = weak.lock();
        if (state) {
            platform.waitFence(fence, FENCE_WAIT_FOR_EVER);
            int64_t const then = clock::now().time_since_epoch().count();
            state->elapsed.store(-then, std::memory_order_relaxed);
            SYSTRACE_CONTEXT();
            SYSTRACE_ASYNC_BEGIN("OpenGLTimerQueryFence", intptr_t(state.get()));
        }
        platform.destroyFence(fence);
    });
}

void OpenGLTimerQueryFence::endTimeElapsedQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
    Platform::Fence* fence = mPlatform.createFence();
    std::weak_ptr<GLTimerQuery::State> const weak = tq->state;

    push([&platform = mPlatform, fence, weak]() {
        auto state = weak.lock();
        if (state) {
            platform.waitFence(fence, FENCE_WAIT_FOR_EVER);
            int64_t const now = clock::now().time_since_epoch().count();
            int64_t const then = state->elapsed.load(std::memory_order_relaxed);
            assert_invariant(then < 0);
            state->elapsed.store(now + then, std::memory_order_relaxed);
            SYSTRACE_CONTEXT();
            SYSTRACE_ASYNC_END("OpenGLTimerQueryFence", intptr_t(state.get()));
        }
        platform.destroyFence(fence);
    });
}

// ------------------------------------------------------------------------------------------------

TimerQueryFallback::TimerQueryFallback() = default;

TimerQueryFallback::~TimerQueryFallback() = default;

void TimerQueryFallback::createTimerQuery(GLTimerQuery* tq) {
    if (UTILS_UNLIKELY(!tq->state)) {
        tq->state = std::make_shared<GLTimerQuery::State>();
    }
}

void TimerQueryFallback::destroyTimerQuery(GLTimerQuery* tq) {
    assert_invariant(tq->state);
}

void TimerQueryFallback::beginTimeElapsedQuery(OpenGLTimerQueryInterface::GLTimerQuery* tq) {
    assert_invariant(tq->state);
    // this implementation measures the CPU time, but we have no h/w support
    int64_t const then = clock::now().time_since_epoch().count();
    tq->state->elapsed.store(-then, std::memory_order_relaxed);
}

void TimerQueryFallback::endTimeElapsedQuery(OpenGLTimerQueryInterface::GLTimerQuery* tq) {
    assert_invariant(tq->state);
    // this implementation measures the CPU time, but we have no h/w support
    int64_t const now = clock::now().time_since_epoch().count();
    int64_t const then = tq->state->elapsed.load(std::memory_order_relaxed);
    assert_invariant(then < 0);
    tq->state->elapsed.store(now + then, std::memory_order_relaxed);
}

} // namespace filament::backend
