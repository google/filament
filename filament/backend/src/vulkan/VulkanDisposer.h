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

#ifndef TNT_FILAMENT_DRIVER_VULKANDISPOSER_H
#define TNT_FILAMENT_DRIVER_VULKANDISPOSER_H

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include <functional>
#include <memory>
#include <vector>

namespace filament {
namespace backend {

// VulkanDisposer tracks resources (such as textures or vertex buffers) that need deferred
// destruction due to potential use by one or more reference holders. An example of a reference
// holder is an active Vulkan command buffer. Resources are represented with void* to allow callers
// to use any type of handle. Reference holders (e.g. VulkanCommandBuffer) have an associated
// robin_set of resource handles.
class VulkanDisposer {
public:
    using Key = const void*;
    using Set = tsl::robin_set<Key>;

    // Adds the given resource to the disposer and sets its reference count to 1.
    void createDisposable(Key resource, std::function<void()> destructor) noexcept;

    // Increments the reference count.
    void addReference(Key resource) noexcept;

    // Decrements the reference count and moves it to the graveyard if it becomes 0.
    void removeReference(Key resource) noexcept;

    // If the given resource is not in the given set, then it gets added to the set and its
    // reference count is incremented.
    void acquire(Key resource, Set& resources) noexcept;

    // Decrements the reference count for all resources in the set, then clears it.
    void release(Set& resources);

    // Invokes the destructor function for each disposable in the graveyard.
    void gc() noexcept;

    // Invokes the destructor function for all disposables, regardless of reference count.
    void reset() noexcept;

private:
    struct Disposable {
        size_t refcount = 1;
        std::function<void()> destructor;
    };
    tsl::robin_map<Key, Disposable> mDisposables;
    std::vector<Disposable> mGraveyard;
};

} // namespace filament
} // namespace backend

#endif // TNT_FILAMENT_DRIVER_VULKANDISPOSER_H
