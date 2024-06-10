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

#include <backend/DriverEnums.h>

#include "DriverBase.h"

#include <utils/Condition.h>
#include <utils/Mutex.h>

#include "gl_headers.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include <stdint.h>

namespace filament::backend {

class OpenGLPlatform;
class OpenGLContext;
class OpenGLDriver;
class TimerQueryFactoryInterface;

struct GLTimerQuery : public HwTimerQuery {
    struct State {
        struct {
            GLuint query;
        } gl;
        int64_t then{};
        std::atomic<int64_t> elapsed{};
    };
    std::shared_ptr<State> state;
};

/*
 * We need two implementation of timer queries (only elapsed time), because
 * on some gpu disjoint_timer_query/arb_timer_query is much less accurate than
 * using fences.
 *
 * These classes implement the various strategies...
 */

class TimerQueryFactory {
    static bool mGpuTimeSupported;
public:
    static TimerQueryFactoryInterface* init(
            OpenGLPlatform& platform, OpenGLContext& context) noexcept;

    static bool isGpuTimeSupported() noexcept {
        return mGpuTimeSupported;
    }
};

class TimerQueryFactoryInterface {
protected:
    using GLTimerQuery = filament::backend::GLTimerQuery;
    using clock = std::chrono::steady_clock;

public:
    virtual ~TimerQueryFactoryInterface();
    virtual void createTimerQuery(GLTimerQuery* query) = 0;
    virtual void destroyTimerQuery(GLTimerQuery* query) = 0;
    virtual void beginTimeElapsedQuery(GLTimerQuery* query) = 0;
    virtual void endTimeElapsedQuery(OpenGLDriver& driver, GLTimerQuery* query) = 0;

    static TimerQueryResult getTimerQueryValue(GLTimerQuery* tq, uint64_t* elapsedTime) noexcept;
};

#if defined(BACKEND_OPENGL_VERSION_GL) || defined(GL_EXT_disjoint_timer_query)

class TimerQueryNativeFactory final : public TimerQueryFactoryInterface {
public:
    explicit TimerQueryNativeFactory(OpenGLContext& context);
    ~TimerQueryNativeFactory() override;
private:
    void createTimerQuery(GLTimerQuery* query) override;
    void destroyTimerQuery(GLTimerQuery* query) override;
    void beginTimeElapsedQuery(GLTimerQuery* query) override;
    void endTimeElapsedQuery(OpenGLDriver& driver, GLTimerQuery* query) override;
    OpenGLContext& mContext;
};

#endif

class TimerQueryFenceFactory final : public TimerQueryFactoryInterface {
public:
    explicit TimerQueryFenceFactory(OpenGLPlatform& platform);
    ~TimerQueryFenceFactory() override;
private:
    using Job = std::function<void()>;
    using Container = std::vector<Job>;

    void createTimerQuery(GLTimerQuery* query) override;
    void destroyTimerQuery(GLTimerQuery* query) override;
    void beginTimeElapsedQuery(GLTimerQuery* tq) override;
    void endTimeElapsedQuery(OpenGLDriver& driver, GLTimerQuery* tq) override;

    void push(Job&& job);

    OpenGLPlatform& mPlatform;
    std::thread mThread;
    mutable utils::Mutex mLock;
    mutable utils::Condition mCondition;
    Container mQueue;
    bool mExitRequested = false;
};

class TimerQueryFallbackFactory final : public TimerQueryFactoryInterface {
public:
    explicit TimerQueryFallbackFactory();
    ~TimerQueryFallbackFactory() override;
private:
    void createTimerQuery(GLTimerQuery* query) override;
    void destroyTimerQuery(GLTimerQuery* query) override;
    void beginTimeElapsedQuery(GLTimerQuery* query) override;
    void endTimeElapsedQuery(OpenGLDriver& driver, GLTimerQuery* query) override;
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_OPENGL_TIMERQUERY_H
