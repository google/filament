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

#ifndef TNT_FILAMENT_FG_DETAILS_UTILITIES_H
#define TNT_FILAMENT_FG_DETAILS_UTILITIES_H

#include "Allocators.h"

#include <vector>
#include <memory>

namespace filament {

class FrameGraph;

template<typename T, typename ARENA>
struct Deleter {
    ARENA* arena = nullptr;
    Deleter(ARENA& arena) noexcept: arena(&arena) {} // NOLINT
    void operator()(T* object) noexcept { arena->destroy(object); }
};

template<typename T, typename ARENA> using UniquePtr = std::unique_ptr<T, Deleter<T, ARENA>>;
template<typename T> using Allocator = utils::STLAllocator<T, LinearAllocatorArena>;
template<typename T> using Vector = std::vector<T, Allocator<T>>; // 32 bytes

} // namespace filament

#endif // TNT_FILAMENT_FG_DETAILS_UTILITIES_H
