/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "VulkanDisposer.h"
#include "VulkanConstants.h"

#include <utils/debug.h>
#include <utils/Log.h>

namespace filament {
namespace backend {

// Always wait at least 3 frames after a DriverAPI-level resource has been destroyed for safe
// destruction, due to potential usage by outstanding command buffers and triple buffering.
static constexpr uint32_t FRAMES_BEFORE_EVICTION = VK_MAX_COMMAND_BUFFERS;

void VulkanDisposer::createDisposable(Key resource, std::function<void()> destructor) noexcept {
    mDisposables[resource].destructor = destructor;
}

void VulkanDisposer::removeReference(Key resource) noexcept {
    // Null can be passed in as a no-op, this is not an error.
    if (resource == nullptr) {
        return;
    }
    assert_invariant(mDisposables[resource].refcount > 0);
    --mDisposables[resource].refcount;
}

void VulkanDisposer::acquire(Key resource) noexcept {
    // It's fine to "acquire" a non-managed resource, it's just a no-op.
    if (resource == nullptr) {
        return;
    }
    auto iter = mDisposables.find(resource);
    if (iter == mDisposables.end()) {
        return;
    }
    Disposable& disposable = iter.value();
    assert_invariant(disposable.refcount > 0 && disposable.refcount < 65535);

    // If an auto-decrement is already in place, do not increase the ref count.
    if (disposable.remainingFrames == 0) {
        ++disposable.refcount;
    }

    disposable.remainingFrames = FRAMES_BEFORE_EVICTION;
}

void VulkanDisposer::gc() noexcept {
    // First decrement the frame count of all resources that were held by a command buffer.
    // If any of these reaches zero, decrement its reference count.
    for (auto iter = mDisposables.begin(); iter != mDisposables.end(); ++iter) {
        Disposable& disposable = iter.value();
        if (disposable.refcount > 0 && disposable.remainingFrames > 0) {
            if (--disposable.remainingFrames == 0) {
                removeReference(iter.key());
            }
        }
    }

    // Next, destroy all resources with a zero refcount.
    decltype(mDisposables) disposables;
    for (auto iter : mDisposables) {
        Disposable& disposable = iter.second;
        if (disposable.refcount == 0) {
            disposable.destructor();
        } else {
            disposables.insert({iter.first, disposable});
        }
    }
    disposables.swap(mDisposables);
}

void VulkanDisposer::reset() noexcept {
#ifndef NDEBUG
    utils::slog.i << mDisposables.size() << " disposables are outstanding." << utils::io::endl;
#endif
    for (auto iter : mDisposables) {
        iter.second.destructor();
    }
    mDisposables.clear();
}

} // namespace filament
} // namespace backend
