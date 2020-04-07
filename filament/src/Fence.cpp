/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include "details/Fence.h"

#include "details/Engine.h"

#include <filament/Fence.h>

#include <utils/Panic.h>

namespace filament {

using namespace backend;

namespace details {


utils::Mutex FFence::sLock;
utils::Condition FFence::sCondition;

static const constexpr uint64_t PUMP_INTERVAL_MILLISECONDS = 1;

using ms = std::chrono::milliseconds;
using ns = std::chrono::nanoseconds;

FFence::FFence(FEngine& engine, Type type)
    : mEngine(engine), mFenceSignal(std::make_shared<FenceSignal>(type)) {
    DriverApi& driverApi = engine.getDriverApi();
    if (type == Type::HARD) {
        mFenceHandle = driverApi.createFence();
    }

    // we have to first wait for the fence to be signaled by the command stream
    auto& fs = mFenceSignal;
    driverApi.queueCommand([fs]() {
        fs->signal();
    });
}

void FFence::terminate(FEngine& engine) noexcept {
    FenceSignal * const fs = mFenceSignal.get();
    fs->signal(FenceSignal::DESTROYED);
    if (fs->mType == Type::HARD) {
        if (mFenceHandle) {
            FEngine::DriverApi& driver = engine.getDriverApi();
            driver.destroyFence(mFenceHandle);
            mFenceHandle.clear();
        }
    }
}

UTILS_NOINLINE
FenceStatus FFence::waitAndDestroy(FFence* fence, Mode mode) noexcept {
    assert(fence);
    FenceStatus status = fence->wait(mode, FENCE_WAIT_FOR_EVER);
    fence->mEngine.destroy(fence);
    return status;
}

UTILS_NOINLINE
FenceStatus FFence::wait(Mode mode, uint64_t timeout) noexcept {
    ASSERT_PRECONDITION(UTILS_HAS_THREADING || timeout == 0, "Non-zero timeout requires threads.");

    FEngine& engine = mEngine;

    if (mode == Mode::FLUSH) {
        engine.flush();
    }

    FenceSignal * const fs = mFenceSignal.get();

    FenceStatus status;

    if (UTILS_LIKELY(!engine.pumpPlatformEvents())) {
        status = fs->wait(timeout);
    } else {
        // Unfortunately, some platforms might force us to have sync points between the GL thread
        // and user thread. To prevent deadlock on these platforms, we chop up the waiting time into
        // polling and pumping the platform's event queue.
        const auto startTime = std::chrono::system_clock::now();
        while (true) {
            status = fs->wait(ns(ms(PUMP_INTERVAL_MILLISECONDS)).count());
            if (status != FenceStatus::TIMEOUT_EXPIRED) {
                break;
            }
            engine.pumpPlatformEvents();
            const auto elapsed = std::chrono::system_clock::now() - startTime;
            if (timeout != Fence::FENCE_WAIT_FOR_EVER && elapsed >= ns(timeout)) {
                break;
            }
        }
    }

    if (status != FenceStatus::CONDITION_SATISFIED) {
        return status;
    }

    if (fs->mType == Type::HARD) {
        // note: even if the driver doesn't support h/w fences, mFenceHandle will be valid
        status = engine.getDriverApi().wait(mFenceHandle, timeout);
    }
    return status;
}

UTILS_NOINLINE
void FFence::FenceSignal::signal(State s) noexcept {
    std::unique_lock<utils::Mutex> lock(FFence::sLock);
    mState = s;
    lock.unlock();
    FFence::sCondition.notify_all();
}

UTILS_NOINLINE
Fence::FenceStatus FFence::FenceSignal::wait(uint64_t timeout) noexcept {
    std::unique_lock<utils::Mutex> lock(FFence::sLock);
    while (mState == UNSIGNALED) {
        if (mState == DESTROYED) {
            return FenceStatus::ERROR;
        }
        if (timeout == FENCE_WAIT_FOR_EVER) {
            FFence::sCondition.wait(lock);
        } else {
            if (timeout == 0 ||
                    sCondition.wait_for(lock, ns(timeout)) == std::cv_status::timeout) {
                return FenceStatus::TIMEOUT_EXPIRED;
            }
        }
    }
    return FenceStatus::CONDITION_SATISFIED;
}

// ------------------------------------------------------------------------------------------------
} // namespace details

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

using namespace details;

FenceStatus Fence::waitAndDestroy(Fence* fence, Mode mode) {
    return FFence::waitAndDestroy(upcast(fence), mode);
}

FenceStatus Fence::wait(Mode mode, uint64_t timeout) {
    return upcast(this)->wait(mode, timeout);
}


} // namespace filament
