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

#ifndef TNT_FILAMENT_BACKEND_VULKANDISPOSER_H
#define TNT_FILAMENT_BACKEND_VULKANDISPOSER_H

#include <tsl/robin_map.h>

#include <functional>

namespace filament::backend {

// VulkanDisposer tracks resources (such as textures or vertex buffers) that need deferred
// destruction due to potential use by a Vulkan command buffer. Resources are represented with void*
// to allow callers to use any type of handle.
class VulkanDisposer {
public:
    using Key = const void*;

    // Adds the given resource to the disposer and sets its reference count to 1.
    void createDisposable(Key resource, std::function<void()> destructor) noexcept;

    // Decrements the reference count.
    void removeReference(Key resource) noexcept;

    // Increments the reference count and auto-decrements it after FRAMES_BEFORE_EVICTION frames.
    // This is used to indicate that the current command buffer has a reference to the resource.
    void acquire(Key resource) noexcept;

    // Invokes the destructor function for each disposable with a 0 refcount.
    void gc() noexcept;

    // Invokes the destructor function for all disposables, regardless of reference count.
    void terminate() noexcept;

private:
    struct Disposable {
        uint16_t refcount = 1;
        uint16_t remainingFrames = 0;
        std::function<void()> destructor = []() {};
    };
    tsl::robin_map<Key, Disposable> mDisposables;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANDISPOSER_H
