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

#include "fg/Blackboard.h"

#include <string_view>

namespace filament {

Blackboard::Blackboard() noexcept = default;

Blackboard::~Blackboard() noexcept = default;

FrameGraphHandle Blackboard::getHandle(std::string_view const name) const noexcept {
    auto it = mMap.find(name);
    if (it != mMap.end()) {
        return it->second;
    }
    return {};
}

FrameGraphHandle& Blackboard::operator [](std::string_view const name) noexcept {
    auto[pos, _] = mMap.insert_or_assign(name, FrameGraphHandle{});
    return pos->second;
}

void Blackboard::put(std::string_view const name, FrameGraphHandle const handle) noexcept {
    operator[](name) = handle;
}


void Blackboard::remove(std::string_view const name) noexcept {
    mMap.erase(name);
}

} // namespace filament
