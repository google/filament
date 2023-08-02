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

OpenGLTimerQueryInterface::~OpenGLTimerQueryInterface() = default;

// ------------------------------------------------------------------------------------------------

#if defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_EXT_disjoint_timer_query)

TimerQueryNative::TimerQueryNative(OpenGLContext& context) : mContext(context) {
}

TimerQueryNative::~TimerQueryNative() = default;

void TimerQueryNative::beginTimeElapsedQuery(GLTimerQuery* query) {
    mContext.procs.beginQuery(GL_TIME_ELAPSED, query->gl.query);
    CHECK_GL_ERROR(utils::slog.e)
}

void TimerQueryNative::endTimeElapsedQuery(GLTimerQuery*) {
    mContext.procs.endQuery(GL_TIME_ELAPSED);
    CHECK_GL_ERROR(utils::slog.e)
}

bool TimerQueryNative::queryResultAvailable(GLTimerQuery* query) {
    GLuint available = 0;
    mContext.procs.getQueryObjectuiv(query->gl.query, GL_QUERY_RESULT_AVAILABLE, &available);
    CHECK_GL_ERROR(utils::slog.e)
    return available != 0;
}

uint64_t TimerQueryNative::queryResult(GLTimerQuery* query) {
    GLuint64 elapsedTime = 0;
    // we won't end-up here if we're on ES and don't have GL_EXT_disjoint_timer_query
    mContext.procs.getQueryObjectui64v(query->gl.query, GL_QUERY_RESULT, &elapsedTime);
    CHECK_GL_ERROR(utils::slog.e)
    return elapsedTime;
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
        mThread.join();
    }
}

void OpenGLTimerQueryFence::enqueue(OpenGLTimerQueryFence::Job&& job) {
    std::unique_lock<utils::Mutex> const lock(mLock);
    mQueue.push_back(std::forward<Job>(job));
    mCondition.notify_one();
}

void OpenGLTimerQueryFence::beginTimeElapsedQuery(GLTimerQuery* query) {
    if (UTILS_UNLIKELY(!query->gl.emulation)) {
        query->gl.emulation = std::make_shared<GLTimerQuery::State>();
    }
    query->gl.emulation->elapsed.store(0);
    Platform::Fence* fence = mPlatform.createFence();
    std::weak_ptr<GLTimerQuery::State> const weak = query->gl.emulation;
    uint32_t const cookie = query->gl.query;
    push([&platform = mPlatform, fence, weak, cookie]() {
        auto emulation = weak.lock();
        if (emulation) {
            platform.waitFence(fence, FENCE_WAIT_FOR_EVER);
            int64_t const then = clock::now().time_since_epoch().count();
            emulation->elapsed.store(-then, std::memory_order_relaxed);
            SYSTRACE_CONTEXT();
            SYSTRACE_ASYNC_BEGIN("OpenGLTimerQueryFence", cookie);
            (void)cookie;
        }
        platform.destroyFence(fence);
    });
}

void OpenGLTimerQueryFence::endTimeElapsedQuery(GLTimerQuery* query) {
    Platform::Fence* fence = mPlatform.createFence();
    std::weak_ptr<GLTimerQuery::State> const weak = query->gl.emulation;
    uint32_t const cookie = query->gl.query;
    push([&platform = mPlatform, fence, weak, cookie]() {
        auto emulation = weak.lock();
        if (emulation) {
            platform.waitFence(fence, FENCE_WAIT_FOR_EVER);
            int64_t const now = clock::now().time_since_epoch().count();
            int64_t const then = emulation->elapsed.load(std::memory_order_relaxed);
            assert_invariant(then < 0);
            emulation->elapsed.store(now + then, std::memory_order_relaxed);
            SYSTRACE_CONTEXT();
            SYSTRACE_ASYNC_END("OpenGLTimerQueryFence", cookie);
            (void)cookie;
        }
        platform.destroyFence(fence);
    });
}

bool OpenGLTimerQueryFence::queryResultAvailable(GLTimerQuery* query) {
    return query->gl.emulation->elapsed.load(std::memory_order_relaxed) > 0;
}

uint64_t OpenGLTimerQueryFence::queryResult(GLTimerQuery* query) {
    int64_t const result = query->gl.emulation->elapsed.load(std::memory_order_relaxed);
    return result > 0 ? result : 0;
}

// ------------------------------------------------------------------------------------------------

TimerQueryFallback::TimerQueryFallback() = default;

TimerQueryFallback::~TimerQueryFallback() = default;

void TimerQueryFallback::beginTimeElapsedQuery(OpenGLTimerQueryInterface::GLTimerQuery* query) {
    if (!query->gl.emulation) {
        query->gl.emulation = std::make_shared<GLTimerQuery::State>();
    }
    // this implementation clearly doesn't work at all, but we have no h/w support
    int64_t const then = clock::now().time_since_epoch().count();
    query->gl.emulation->elapsed.store(-then, std::memory_order_relaxed);
}

void TimerQueryFallback::endTimeElapsedQuery(OpenGLTimerQueryInterface::GLTimerQuery* query) {
    // this implementation clearly doesn't work at all, but we have no h/w support
    int64_t const now = clock::now().time_since_epoch().count();
    int64_t const then = query->gl.emulation->elapsed.load(std::memory_order_relaxed);
    assert_invariant(then < 0);
    query->gl.emulation->elapsed.store(now + then, std::memory_order_relaxed);
}

bool TimerQueryFallback::queryResultAvailable(OpenGLTimerQueryInterface::GLTimerQuery* query) {
    return query->gl.emulation->elapsed.load(std::memory_order_relaxed) > 0;
}

uint64_t TimerQueryFallback::queryResult(OpenGLTimerQueryInterface::GLTimerQuery* query) {
    int64_t const result = query->gl.emulation->elapsed.load(std::memory_order_relaxed);
    return result > 0 ? result : 0;
}

} // namespace filament::backend
