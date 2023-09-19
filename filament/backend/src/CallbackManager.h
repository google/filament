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

#include <utils/Mutex.h>

#include <atomic>
#include <mutex>
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
 */
class CallbackManager {
    struct Callback {
        mutable std::atomic_int count{};
        CallbackHandler* handler = nullptr;
        CallbackHandler::Callback func = {};
        void* user = nullptr;
    };

    using Container = std::list<Callback>;

public:
    using Handle = Container::const_iterator;

    explicit CallbackManager(DriverBase& driver) noexcept;

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
    Container::const_iterator getCurrent() const noexcept {
        std::lock_guard const lock(mLock);
        return --mCallbacks.end();
    }

    Container::iterator allocateNewSlot() noexcept {
        std::lock_guard const lock(mLock);
        auto curr = --mCallbacks.end();
        mCallbacks.emplace_back();
        return curr;
    }
    void destroySlot(Container::const_iterator curr) noexcept {
        std::lock_guard const lock(mLock);
        mCallbacks.erase(curr);
    }

    DriverBase& mDriver;
    mutable utils::Mutex mLock;
    Container mCallbacks;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_CALLBACKMANAGER_H
