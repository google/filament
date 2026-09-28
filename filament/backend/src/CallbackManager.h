/*
 * Copyright (C) 2023 The Android Open Source Project
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
 *
 */

#ifndef TNT_FILAMENT_BACKEND_CALLBACKMANAGER_H
#define TNT_FILAMENT_BACKEND_CALLBACKMANAGER_H

#include <backend/CallbackHandler.h>

#include <utils/compiler.h>
#include <utils/Mutex.h>

#include <iterator>
#include <list>

namespace filament::backend {

class DriverBase;
class CallbackHandler;

/*
 * CallbackManager schedules user callbacks once all previous conditions are met.
 * A "Condition" is created by calling "get" and is met by calling "put". These
 * are typically called from different threads.
 * The callback is specified with "setCallback", which atomically creates a new set of
 * conditions to be met.
 *
 * Callbacks are scheduled outside of mLock, so no ordering is guaranteed between callbacks
 * belonging to different sets of conditions: two threads completing two sets can enqueue their
 * callbacks onto the driver in either order.
 */
class CallbackManager {
    struct CallbackInfo {
        CallbackHandler* handler = nullptr;
        CallbackHandler::Callback func = {};
        void* user = nullptr;
        explicit operator bool() const noexcept { return func != nullptr; }
    };

    // All fields in Callback are guarded by mLock.
    struct Callback {
        mutable int count = 0;
        CallbackInfo callback{};
    };

    using Container = std::list<Callback>;

public:
    using Handle = Container::const_iterator;

    explicit CallbackManager(DriverBase& driver);

    ~CallbackManager() noexcept;

    // Calls all the pending callbacks regardless of remaining conditions to be met. This is to
    // avoid leaking resources for instance. It also doesn't matter if the conditions are met
    // because we're shutting down.
    void terminate() noexcept;

    // creates a condition and get a handle for it
    Handle get() const noexcept;

    // Announces the specified condition is met. If a callback was specified and all conditions
    // prior to setting the callback are met, the callback is scheduled.
    void put(Handle& curr) noexcept;

    // Sets a callback to be called when all previously created (get) conditions are met (put).
    // If there were no conditions created, or they're all already met, the callback is scheduled
    // immediately.
    void setCallback(CallbackHandler* handler, CallbackHandler::Callback func, void* user);

private:
    Container::iterator allocateNewSlot() UTILS_REQUIRES(mLock) {
        Container::iterator const curr = std::prev(mCallbacks.end());
        mCallbacks.emplace_back();
        return curr;
    }

    void destroySlot(Container::const_iterator const curr) noexcept UTILS_REQUIRES(mLock) {
        mCallbacks.erase(curr);
    }

    Handle createCondition() const noexcept UTILS_REQUIRES(mLock) {
        Container::const_iterator const curr = std::prev(mCallbacks.end());
        curr->count++;
        return curr;
    }

    CallbackInfo decrementAndCheck(Handle const curr) noexcept UTILS_REQUIRES(mLock) {
        if (--curr->count == 0) {
            if (curr->callback) {
                auto const callback = curr->callback;
                destroySlot(curr);
                return callback;
            }
        }
        return {};
    }

    bool setSlotCallback(CallbackInfo const& callback) UTILS_REQUIRES(mLock) {
        Container::iterator const curr = allocateNewSlot();
        curr->callback = callback;
        if (curr->count == 0) {
            destroySlot(curr);
            return true;
        }
        return false;
    }

    // Claims the first pending callback, clearing it from its slot so that it can't be scheduled
    // twice. Returns an empty CallbackInfo when no pending callback is left. The slot itself is
    // kept: its conditions still hold outstanding Handles that must remain valid for put().
    // Unlike the helpers above, this acquires mLock itself, so that it can be used directly as a
    // loop condition; UTILS_EXCLUDES catches a caller that already holds mLock.
    CallbackInfo takePendingCallback() noexcept UTILS_EXCLUDES(mLock);

    // Hands a callback over to the driver. DriverBase::scheduleCallback() acquires the driver's
    // own locks, so this must never be called with mLock held; UTILS_EXCLUDES makes that a
    // compile-time error under -Wthread-safety.
    void schedule(CallbackInfo const& callback) const noexcept UTILS_EXCLUDES(mLock);

    DriverBase& mDriver;
    mutable utils::Mutex mLock;
    // mLock guards mCallbacks as well as the fields of all Callback elements.
    Container mCallbacks UTILS_GUARDED_BY(mLock);
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_CALLBACKMANAGER_H
