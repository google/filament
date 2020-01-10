/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef TNT_FILAMENT_FG_BLACKBOARD_H
#define TNT_FILAMENT_FG_BLACKBOARD_H

#include "FrameGraphHandle.h"

#include <utils/CString.h>

#include <tsl/robin_map.h>

namespace filament {

class Blackboard {
    using Container = tsl::robin_map<utils::StaticString, FrameGraphHandle>;

public:
    auto& operator [](utils::StaticString const& name) noexcept {
        return mMap.insert_or_assign(name, FrameGraphHandle{}).first.value();
    }

    template<typename T>
    void put(utils::StaticString const& name, FrameGraphId<T> handle) noexcept {
        mMap.insert_or_assign(name, handle);
    }

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

#endif //TNT_FILAMENT_FG_BLACKBOARD_H
