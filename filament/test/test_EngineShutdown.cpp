/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "CommandStreamDispatcher.h"
#include "MockDriver.h"

#include "details/AsyncHelpers.h"

#include <filament/Engine.h>

#include <private/backend/Driver.h>

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>
#include <backend/Platform.h>

#include <utils/CString.h>
#include <utils/Mutex.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <utility>
#include <vector>

using namespace filament;
using namespace filament::backend;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

namespace {

// What the test looks at once the Engine, and the driver it owns, are both gone.
struct ShutdownTrace {
    std::atomic_int userCallbacksInvoked{ 0 };
    std::atomic<AsyncCallStatus> lastStatus{ AsyncCallStatus::COMPLETED };
    // -1 means the driver was never destroyed.
    std::atomic_int callbacksLeftAtDestruction{ -1 };
};

using ShutdownCountdown = CountdownCallbackHandler<int>;

// A driver that queues the callbacks scheduled without a handler instead of dropping them like
// MockDriver does, with DriverBase's semantics for a build with no ServiceThread -- which is where
// a callback scheduling another callback is reachable: purge() dispatches the batch that was queued
// when it was called, purgeAll() keeps going until the queue stays empty.
class QueueingDriver : public MockDriver {
public:
    explicit QueueingDriver(ShutdownTrace* trace)
            : mTrace(trace) {
        // Enough of a driver for Engine::Builder::build() to get through its validation.
        ON_CALL(*this, getFeatureLevel()).WillByDefault(Return(FeatureLevel::FEATURE_LEVEL_1));
        ON_CALL(*this, isTextureFormatSupported(_)).WillByDefault(Return(true));
        ON_CALL(*this, isRenderTargetFormatSupported(_)).WillByDefault(Return(true));
        ON_CALL(*this, isTextureFormatMipmappable(_)).WillByDefault(Return(true));
        ON_CALL(*this, isTextureFormatFilterable(_)).WillByDefault(Return(true));
        ON_CALL(*this, getMaxTextureSize(_)).WillByDefault(Return(2048));
        ON_CALL(*this, isAsynchronousModeEnabled()).WillByDefault(Return(true));

        // terminate() runs on the backend thread after the engine's last flush, so anything
        // scheduled from here has only FEngine::shutdown()'s final purge left to run it. That is
        // when a JobWorker being torn down reports its canceled asynchronous calls.
        ON_CALL(*this, terminate())
                .WillByDefault(Invoke(this, &QueueingDriver::reportCanceledCreation));
    }

    ~QueueingDriver() override {
        utils::LockGuard const lock(mLock);
        mTrace->callbacksLeftAtDestruction.store(int(mCallbacks.size()), std::memory_order_release);
    }

    // With no ServiceThread, DriverBase queues (user, callback) and invokes the callback directly
    // at purge time, bypassing `handler`. Same here.
    void scheduleCallback(CallbackHandler*, void* user,
            CallbackHandler::Callback callback) override {
        utils::LockGuard const lock(mLock);
        mCallbacks.emplace_back(user, callback);
    }

    void purge() noexcept override {
        dispatchQueuedCallbacks();
    }

    void purgeAll() noexcept override {
        while (dispatchQueuedCallbacks()) {
        }
    }

    Dispatcher getDispatcher() const noexcept override {
        return ConcreteDispatcher<QueueingDriver>::make();
    }

private:
    // The box DriverBase::scheduleAsyncCallback() uses to carry a status through a callback
    // signature that doesn't have one.
    struct StatusBox {
        AsyncCallback callback;
        void* user;
        AsyncCallStatus status;
    };

    static void invokeBoxed(void* data) {
        auto* const box = static_cast<StatusBox*>(data);
        box->callback(box->user, box->status);
        delete box;
    }

    // An asynchronous creation canceled by teardown, in its real shape: the countdown handler is
    // notified from the callback queue and, reaching zero, hands the user's callback back to that
    // same queue from inside the dispatch. Dispatching the queue once leaves the user callback
    // behind: it never runs, and the handler it would have deleted leaks.
    void reportCanceledCreation() {
        auto* const countdown = ShutdownCountdown::make(
                /* handler */ nullptr,
                ShutdownCountdown::UserCallback(
                        [trace = mTrace](int*, void*, AsyncCallStatus const status) {
                            trace->lastStatus.store(status, std::memory_order_release);
                            trace->userCallbacksInvoked.fetch_add(1, std::memory_order_acq_rel);
                        }),
                /* userParam1 */ nullptr, /* userParam2 */ nullptr,
                ShutdownCountdown::CountdownCompleteCallback{}, this);
        countdown->increaseCountdown();
        scheduleCallback(/* handler */ countdown,
                new StatusBox{ &ShutdownCountdown::countdownCallback, countdown,
                    AsyncCallStatus::CANCELED },
                &invokeBoxed);
    }

    // Dispatches what is queued so far, with no lock held. Returns false if there was nothing.
    bool dispatchQueuedCallbacks() noexcept {
        decltype(mCallbacks) callbacks;
        {
            utils::LockGuard const lock(mLock);
            if (mCallbacks.empty()) {
                return false;
            }
            std::swap(callbacks, mCallbacks);
        }
        for (auto& item: callbacks) {
            item.second(item.first);
        }
        return true;
    }

    mutable utils::Mutex mLock;
    std::vector<std::pair<void*, CallbackHandler::Callback>> mCallbacks UTILS_GUARDED_BY(mLock);
    ShutdownTrace* mTrace;
};

class QueueingPlatform : public Platform {
public:
    explicit QueueingPlatform(ShutdownTrace* trace)
            : mTrace(trace) {}

    int getOSVersion() const noexcept override { return 0; }

    utils::CString getDeviceInfo(DeviceInfoType, Driver*) const override { return {}; }

    Driver* createDriver(void*, const Platform::DriverConfig&) override {
        // The Engine takes ownership and deletes it in the FEngine destructor.
        return new ::testing::NiceMock<QueueingDriver>(mTrace);
    }

private:
    ShutdownTrace* mTrace;
};

} // namespace

TEST(EngineShutdownTest, DrainsUserCallbacksScheduledWhileTheQueueIsBeingDispatched) {
    // Shutting the engine down is the last chance to run the callbacks the user is still waiting
    // on. A callback scheduled by one that is being dispatched -- the shape every canceled
    // asynchronous creation takes, through CountdownCallbackHandler -- has no later purge to pick
    // it up, so the final purge has to keep draining until the queue stays empty.
    ShutdownTrace trace;
    QueueingPlatform platform(&trace);

    Engine* engine = Engine::Builder()
            .backend(Backend::NOOP)
            .platform(&platform)
            .build();
    ASSERT_NE(nullptr, engine);

    Engine::destroy(&engine);

    EXPECT_EQ(1, trace.userCallbacksInvoked.load(std::memory_order_acquire))
            << "shutting down the engine must run the callbacks its own teardown schedules";
    EXPECT_EQ(AsyncCallStatus::CANCELED, trace.lastStatus.load(std::memory_order_acquire))
            << "a creation canceled by teardown must be reported as canceled";
    EXPECT_EQ(0, trace.callbacksLeftAtDestruction.load(std::memory_order_acquire))
            << "the driver was destroyed with callbacks still queued";
}
