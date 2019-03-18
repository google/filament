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

#include "driver/vulkan/VulkanDisposer.h"

#include <utils/Panic.h>

namespace filament {
namespace driver {

void VulkanDisposer::createDisposable(Key resource, std::function<void()> destructor) noexcept {
    mDisposables[resource] = { 1, destructor };
}

void VulkanDisposer::addReference(Key resource) noexcept {
    ASSERT_POSTCONDITION(mDisposables[resource].refcount > 0, "Unexpected ref count.");
    ++mDisposables[resource].refcount;
}

void VulkanDisposer::removeReference(Key resource) noexcept {
    ASSERT_POSTCONDITION(mDisposables[resource].refcount > 0, "Unexpected ref count.");
    if (--mDisposables[resource].refcount == 0) {
        mGraveyard.emplace_back(std::move(mDisposables[resource]));
        mDisposables.erase(resource);
    }
}
void VulkanDisposer::acquire(Key resource, Set& resources) noexcept {
    auto iter = resources.find(resource);
    if (iter == resources.end()) {
        resources.insert(resource);
        addReference(resource);
    }
}

void VulkanDisposer::release(Set& resources) {
    for (auto resource : resources) {
        removeReference(resource);
    }
    resources.clear();
}

void VulkanDisposer::gc() noexcept {
    for (auto& ptr : mGraveyard) {
        ptr.destructor();
    }
    mGraveyard.clear();
}

void VulkanDisposer::reset() noexcept {
    gc();
    for (auto iter : mDisposables) {
        iter.second.destructor();
    }
    mDisposables.clear();
}

} // namespace filament
} // namespace driver
