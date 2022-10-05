/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FG_FRAMEGRAPHID_H
#define TNT_FILAMENT_FG_FRAMEGRAPHID_H

#include <stdint.h>
#include <limits>
#include <utility>

namespace filament {

template<typename T>
class FrameGraphId;

class Blackboard;
class FrameGraph;
class FrameGraphResources;
class PassNode;
class ResourceNode;

/** A handle on a resource */
class FrameGraphHandle {
public:
    using Index = uint16_t;
    using Version = uint16_t;

private:
    template<typename T>
    friend class FrameGraphId;

    friend class Blackboard;
    friend class FrameGraph;
    friend class FrameGraphResources;
    friend class PassNode;
    friend class ResourceNode;

    // private ctor -- this cannot be constructed by users
    FrameGraphHandle() noexcept = default;
    explicit FrameGraphHandle(Index index) noexcept : index(index) {}

    // index to the resource handle
    static constexpr uint16_t UNINITIALIZED = std::numeric_limits<Index>::max();
    uint16_t index = UNINITIALIZED;     // index to a ResourceSlot
    Version version = 0;

public:
    FrameGraphHandle(FrameGraphHandle const& rhs) noexcept = default;

    FrameGraphHandle& operator=(FrameGraphHandle const& rhs) noexcept = default;

    bool isInitialized() const noexcept { return index != UNINITIALIZED; }

    operator bool() const noexcept { return isInitialized(); }

    void clear() noexcept { index = UNINITIALIZED; version = 0; }

    bool operator < (const FrameGraphHandle& rhs) const noexcept {
        return index < rhs.index;
    }

    bool operator == (const FrameGraphHandle& rhs) const noexcept {
        return (index == rhs.index);
    }

    bool operator != (const FrameGraphHandle& rhs) const noexcept {
        return !operator==(rhs);
    }
};

/** A typed handle on a resource */
template<typename RESOURCE>
class FrameGraphId : public FrameGraphHandle {
public:
    using FrameGraphHandle::FrameGraphHandle;
    FrameGraphId() {}
    explicit FrameGraphId(FrameGraphHandle r) : FrameGraphHandle(r) { }
};

} // namespace filament

#endif //TNT_FILAMENT_FG_FRAMEGRAPHID_H
