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

#ifndef TNT_FILAMENT_FG_RESOURCENODE_H
#define TNT_FILAMENT_FG_RESOURCENODE_H

#include <limits>
#include <stdint.h>

namespace filament {
namespace fg {

struct PassNode;
class ResourceEntryBase;

struct ResourceNode { // 24
    ResourceNode(ResourceEntryBase* resource, uint8_t version) noexcept
            : resource(resource), version(version) {}

    ResourceNode(ResourceNode const&) = delete;
    ResourceNode(ResourceNode&&) noexcept = default;
    ResourceNode& operator=(ResourceNode const&) = delete;

    // updated during compile()
    ResourceEntryBase* resource;    // actual (aliased) resource data
    PassNode* writer = nullptr;     // writer to this node
    uint32_t readerCount = 0;       // # of passes reading from this resource

    // constants
    const uint8_t version;          // version of the resource when the node was created
};

} // namespace fg
} // namespace filament

#endif //TNT_FILAMENT_FG_RESOURCENODE_H
