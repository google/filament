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

namespace filament::backend {

CallbackManager::CallbackManager(DriverBase& driver) noexcept
    : mDriver(driver), mCallbacks(1) {
}

CallbackManager::~CallbackManager() noexcept = default;

void CallbackManager::terminate() noexcept {
    for (auto&& item: mCallbacks) {
        if (item.func) {
            mDriver.scheduleCallback(
                    item.handler, item.user, item.func);
        }
    }
}

CallbackManager::Handle CallbackManager::get() const noexcept {
    Container::const_iterator const curr = getCurrent();
    curr->count.fetch_add(1);
    return curr;
}

void CallbackManager::put(Handle& curr) noexcept {
    if (curr->count.fetch_sub(1) == 1) {
        if (curr->func) {
            mDriver.scheduleCallback(
                    curr->handler, curr->user, curr->func);
            destroySlot(curr);
        }
    }
    curr = {};
}

void CallbackManager::setCallback(
        CallbackHandler* handler, CallbackHandler::Callback func, void* user) {
    assert_invariant(func);
    Container::iterator const curr = allocateNewSlot();
    curr->handler = handler;
    curr->func = func;
    curr->user = user;
    if (curr->count == 0) {
        mDriver.scheduleCallback(
                curr->handler, curr->user, curr->func);
        destroySlot(curr);
    }
}

} // namespace filament::backend
