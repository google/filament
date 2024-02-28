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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_TIMERQUERY_H
#define TNT_FILAMENT_BACKEND_OPENGL_TIMERQUERY_H

#include "OpenGLDriver.h"

#include <utils/Condition.h>
#include <utils/Mutex.h>

#include <thread>
#include <vector>

namespace filament::backend {

class OpenGLPlatform;
class OpenGLTimerQueryInterface;

/*
 * We need two implementation of timer queries (only elapsed time), because
 * on some gpu disjoint_timer_query/arb_timer_query is much less accurate than
 * using fences.
 *
 * These classes implement the various strategies...
 */


class OpenGLTimerQueryFactory {
    static bool mGpuTimeSupported;
public:
    static OpenGLTimerQueryInterface* init(
            OpenGLPlatform& platform, OpenGLDriver& driver) noexcept;

    static bool isGpuTimeSupported() noexcept {
        return mGpuTimeSupported;
    }
};

class OpenGLTimerQueryInterface {
protected:
    using GLTimerQuery = OpenGLDriver::GLTimerQuery;
    using clock = std::chrono::steady_clock;

public:
    virtual ~OpenGLTimerQueryInterface();
    virtual void createTimerQuery(GLTimerQuery* query) = 0;
    virtual void destroyTimerQuery(GLTimerQuery* query) = 0;
    virtual void beginTimeElapsedQuery(GLTimerQuery* query) = 0;
    virtual void endTimeElapsedQuery(GLTimerQuery* query) = 0;

    static TimerQueryResult getTimerQueryValue(GLTimerQuery* tq, uint64_t* elapsedTime) noexcept;
};

#if defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_EXT_disjoint_timer_query)

class TimerQueryNative : public OpenGLTimerQueryInterface {
public:
    explicit TimerQueryNative(OpenGLDriver& driver);
    ~TimerQueryNative() override;
private:
    void createTimerQuery(GLTimerQuery* query) override;
    void destroyTimerQuery(GLTimerQuery* query) override;
    void beginTimeElapsedQuery(GLTimerQuery* query) override;
    void endTimeElapsedQuery(GLTimerQuery* query) override;
    OpenGLDriver& mDriver;
};

#endif

class OpenGLTimerQueryFence : public OpenGLTimerQueryInterface {
public:
    explicit OpenGLTimerQueryFence(OpenGLPlatform& platform);
    ~OpenGLTimerQueryFence() override;
private:
    using Job = std::function<void()>;
    void createTimerQuery(GLTimerQuery* query) override;
    void destroyTimerQuery(GLTimerQuery* query) override;
    void beginTimeElapsedQuery(GLTimerQuery* tq) override;
    void endTimeElapsedQuery(GLTimerQuery* tq) override;

    void enqueue(Job&& job);
    template<typename CALLABLE, typename ... ARGS>
    void push(CALLABLE&& func, ARGS&& ... args) {
        enqueue(Job(std::bind(std::forward<CALLABLE>(func), std::forward<ARGS>(args)...)));
    }

    OpenGLPlatform& mPlatform;
    std::thread mThread;
    mutable utils::Mutex mLock;
    mutable utils::Condition mCondition;
    std::vector<Job> mQueue;
    bool mExitRequested = false;
};

class TimerQueryFallback : public OpenGLTimerQueryInterface {
public:
    explicit TimerQueryFallback();
    ~TimerQueryFallback() override;
private:
    void createTimerQuery(GLTimerQuery* query) override;
    void destroyTimerQuery(GLTimerQuery* query) override;
    void beginTimeElapsedQuery(GLTimerQuery* query) override;
    void endTimeElapsedQuery(GLTimerQuery* query) override;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_TIMERQUERY_H
