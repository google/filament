/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_ASYNCHELPERS_H
#define TNT_FILAMENT_DETAILS_ASYNCHELPERS_H

#include "private/backend/Driver.h"

#include <backend/CallbackHandler.h>

#include <functional>

namespace filament {

using namespace utils;

// This acts as an adapter that bridges a user-provided callback with the specific requirements of
// the Filament callback system.
// E.g.
//   The callback that users expect => std::function<void(Texture*, void*)>
//   Filament callback system requirement => CallbackHandler::Callback* callback, void* user
template<typename T>
class CallbackAdapter {
public:
    using UserCallback = std::function<void(T*, void*)>;

    template<typename... Args>
    static CallbackAdapter<T>* make(Args&&... args) {
        return new (std::nothrow) CallbackAdapter<T>(std::forward<Args>(args)...);
    }

    CallbackAdapter(UserCallback&& callback, const T* param, void* custom)
        : mUserCallback(std::move(callback)), mUserParam1(param), mUserParam2(custom) {}

    static void func(void* user) {
        auto* const thisObject = static_cast<CallbackAdapter*>(user);
        thisObject->mUserCallback(const_cast<T*>(thisObject->mUserParam1), thisObject->mUserParam2);
        delete thisObject;
    }

private:
    UserCallback mUserCallback;
    const T* mUserParam1;
    void* mUserParam2;
};

// This specialized class tracks multiple asynchronous operations. Before invoking the user-provided
// callback, it waits for all tasks to complete. Users are required to manually update the countdown
// for each async call they make.
template<typename T>
class CountdownCallbackHandler : public backend::CallbackHandler {
public:
    using UserCallback = std::function<void(T*, void*)>;
    using CountdownCompleteCallback = std::function<void()>;

    template<typename... Args>
    static CountdownCallbackHandler<T>* make(Args&&... args) {
        return new (std::nothrow) CountdownCallbackHandler<T>(std::forward<Args>(args)...);
    }

    CountdownCallbackHandler(CallbackHandler* handler, UserCallback&& userCallback, T* userParam1,
            void* userParam2, CountdownCompleteCallback&& onCountdownComplete, backend::Driver* driver)
            : mUserHandler(handler),
              mUserCallback(std::move(userCallback)),
              mUserParam1(userParam1),
              mUserParam2(userParam2),
              mCountdownCompleteCallback(std::move(onCountdownComplete)),
              mDriver(driver) {
    }

    // 1. This method serves as a custom, intermediate handler that is called by the
    // "ServiceThread" in DriverBase. Its primary purpose is to manage the countdown
    // mechanism before the *REAL* user's callback is executed.
    void post(void* user, Callback callback) override {
        // 2. Calls the static method `countdownCallback(user)` to manage the internal
        // counter for pending asynchronous operations.
        callback(user);
    }

    // 3. Decreases the count of pending asynchronous operations. When the counter hits zero,
    // the final callback is scheduled with the *REAL* user's handler to execute the *REAL*
    // user's callback.
    static void countdownCallback(void* user) {
        auto* thisObject = static_cast<CountdownCallbackHandler*>(user);
        if (thisObject->decreaseCountdown()) {
            if (thisObject->mCountdownCompleteCallback) {
                thisObject->mCountdownCompleteCallback();
            }
            // 4. Schedules the final callback to execute the *REAL* user's callback.
            thisObject->mDriver->scheduleCallback(thisObject->mUserHandler, user,
                    &CountdownCallbackHandler::invokeUserCallback);
        }
    }

    // 5. This method is called via the *REAL* user's handler. Executes the *REAL* user's
    // callback.
    static void invokeUserCallback(void* user) {
        auto* thisObject = static_cast<CountdownCallbackHandler*>(user);
        // 6. Executes the *REAL* user's callback.
        if (thisObject->mUserCallback) {
            thisObject->mUserCallback(thisObject->mUserParam1, thisObject->mUserParam2);
        }
        delete thisObject;
    }

    // Increase the countdown. Users are required to call this for each async call they make.
    uint32_t increaseCountdown() {
        // `std::memory_order_relaxed` should be sufficient because no other variables need to be
        // visible to other threads in a strict sequence.
        auto prev = mCount.fetch_add(1, std::memory_order_relaxed);
        return prev + 1;
    }

private:
    // Reduces the countdown and returns true upon reaching zero.
    bool decreaseCountdown() {
        // `std::memory_order_acq_rel` is recommended here to ensure all member fields (mUserHandler,
        // mCountdownCompleteCallback, and others) are visible to this `ServiceThread` before using
        // them.
        auto prev = mCount.fetch_sub(1, std::memory_order_acq_rel);
        return prev == 1;
    }

    std::atomic_uint32_t mCount = 0;
    CallbackHandler* mUserHandler = nullptr;
    UserCallback mUserCallback;
    T* mUserParam1 = nullptr;
    void* mUserParam2 = nullptr;
    CountdownCompleteCallback mCountdownCompleteCallback;
    backend::Driver* mDriver = nullptr;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_ASYNCHELPERS_H
