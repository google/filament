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
 */

#include "CallbackManager.h"

#include "DriverBase.h"

#include <utils/debug.h>
#include <utils/Mutex.h>

namespace filament::backend {

CallbackManager::CallbackManager(DriverBase& driver)
    : mDriver(driver), mCallbacks(1) {
}

CallbackManager::~CallbackManager() noexcept = default;

void CallbackManager::schedule(CallbackInfo const& callback) const noexcept {
    mDriver.scheduleCallback(callback.handler, callback.user, callback.func);
}

CallbackManager::CallbackInfo CallbackManager::takePendingCallback() noexcept {
    utils::LockGuard const lock(mLock);
    for (auto&& item: mCallbacks) {
        if (item.callback) {
            CallbackInfo const callback = item.callback;
            item.callback = {};
            return callback;
        }
    }
    return {};
}

void CallbackManager::terminate() noexcept {
    // Drain one callback at a time so that user callbacks are never scheduled with mLock held.
    // This is O(n^2) in the number of pending callbacks, but n is the number of outstanding
    // setCallback() groups (a handful at most) and this only runs at shutdown.
    // A concurrent setCallback() could in theory keep this loop alive; it can't happen because
    // terminate() and setCallback() are both only ever called from the driver thread.
    while (CallbackInfo const callback = takePendingCallback()) {
        schedule(callback);
    }
}

CallbackManager::Handle CallbackManager::get() const noexcept {
    utils::LockGuard const lock(mLock);
    return createCondition();
}

void CallbackManager::put(Handle& curr) noexcept {
    CallbackInfo callback;
    {
        utils::LockGuard const lock(mLock);
        callback = decrementAndCheck(curr);
    }
    if (callback) {
        schedule(callback);
    }
    curr = {};
}

void CallbackManager::setCallback(
        CallbackHandler* handler, CallbackHandler::Callback const func, void* user) {
    assert_invariant(func);
    CallbackInfo const callback{ handler, func, user };
    bool shouldSchedule = false;
    {
        utils::LockGuard const lock(mLock);
        shouldSchedule = setSlotCallback(callback);
    }
    if (shouldSchedule) {
        schedule(callback);
    }
}

} // namespace filament::backend
