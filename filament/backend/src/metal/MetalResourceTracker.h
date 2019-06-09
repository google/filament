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

#ifndef TNT_METALRESOURCETRACKER_H
#define TNT_METALRESOURCETRACKER_H

#include <tsl/robin_map.h>
#include <tsl/robin_set.h>

#include <functional>
#include <mutex>

namespace filament {
namespace backend {
namespace metal {

/**
 * MetalResourceTracker is a simple utility that maps individual command buffers to a set of
 * associated resources.
 */
class MetalResourceTracker {
public:
    using CommandBuffer = void*;
    using Resource = const void*;
    using ResourceDeleter = std::function<void(Resource)>;

    /**
     * Associate the given resource with a command buffer. When clearResources is called, the given
     * deleter will be called for each resource.
     * @return true, if this is the first time tracking the resource.
     */
    bool trackResource(CommandBuffer buffer, Resource resource, ResourceDeleter deleter);

    /**
     * Calls the deleter for each resource associated with the command buffer, and removes them from
     * tracking.
     */
    void clearResources(CommandBuffer buffer);

private:
    struct ResourceEntry {
        Resource resource;
        ResourceDeleter deleter;

        bool operator==(const ResourceEntry& rhs) const noexcept {
            return resource == rhs.resource;
        }
    };

    struct ResourceEntryHash {
        size_t operator()(const ResourceEntry& entry) const noexcept {
            return std::hash<Resource>{}(entry.resource);
        }
    };

    using ResourcesKey = CommandBuffer;
    using Resources = tsl::robin_set<ResourceEntry, ResourceEntryHash>;
    tsl::robin_map<ResourcesKey, Resources> mResources;

    // Synchronizes access to the map.
    // trackResource and clearResources may be called on separate threads (the engine thread and a
    // Metal callback thread, for example).
    std::mutex mMutex;
};

} // namespace metal
} // namespace backend
} // namespace filament

#endif //TNT_METALRESOURCETRACKER_H
