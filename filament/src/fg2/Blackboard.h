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

#ifndef TNT_FILAMENT_FG2_BLACKBOARD_H
#define TNT_FILAMENT_FG2_BLACKBOARD_H

#include <fg2/FrameGraphId.h>

#include <utils/CString.h>

#include <unordered_map>

namespace filament {

class Blackboard {
    using Container = std::unordered_map<utils::StaticString, FrameGraphHandle>;

public:
    Blackboard() noexcept;
    ~Blackboard() noexcept;

    FrameGraphHandle& operator [](utils::StaticString const& name) noexcept;

    void put(utils::StaticString const& name, FrameGraphHandle handle) noexcept;

    template<typename T>
    FrameGraphId<T> get(utils::StaticString&& name) const noexcept {
        return static_cast<FrameGraphId<T>>(getHandle(std::forward<utils::StaticString>(name)));
    }

    void remove(utils::StaticString const& name) noexcept;

private:
    FrameGraphHandle getHandle(utils::StaticString const& name) const noexcept;
    Container mMap;
};

} // namespace filament


#endif //TNT_FILAMENT_FG2_BLACKBOARD_H
