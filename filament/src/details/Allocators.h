/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_ALLOCATORS_H
#define TNT_FILAMENT_DETAILS_ALLOCATORS_H

#include <utils/Allocator.h>

namespace filament {

// per render pass allocations
// Froxelization needs about 1 MiB. Command buffer needs about 1 MiB.
static constexpr size_t CONFIG_PER_RENDER_PASS_ARENA_SIZE    = 2 * 1024 * 1024;

// size of the high-level draw commands buffer (comes from the per-render pass allocator)
static constexpr size_t CONFIG_PER_FRAME_COMMANDS_SIZE = 1 * 1024 * 1024;

// size of a command-stream buffer (comes from mmap -- not the per-engine arena)
static constexpr size_t CONFIG_MIN_COMMAND_BUFFERS_SIZE = 1 * 1024 * 1024;
static constexpr size_t CONFIG_COMMAND_BUFFERS_SIZE     = 3 * CONFIG_MIN_COMMAND_BUFFERS_SIZE;

#ifndef NDEBUG

// on Debug builds, HeapAllocatorArena needs LockingPolicy::Mutex because it uses a
// TrackingPolicy, which needs to be synchronized.
using HeapAllocatorArena = utils::Arena<
        utils::HeapAllocator,
        utils::LockingPolicy::Mutex,
        utils::TrackingPolicy::DebugAndHighWatermark>;

using LinearAllocatorArena = utils::Arena<
        utils::LinearAllocator,
        utils::LockingPolicy::NoLock,
        utils::TrackingPolicy::DebugAndHighWatermark>;

#else

// on Release builds, HeapAllocatorArena doesn't need a LockingPolicy because HeapAllocator is
// intrinsically synchronized as it relies on heap allocations (i.e.: malloc/free)
using HeapAllocatorArena = utils::Arena<
        utils::HeapAllocator,
        utils::LockingPolicy::NoLock>;

using LinearAllocatorArena = utils::Arena<
        utils::LinearAllocator,
        utils::LockingPolicy::NoLock>;

#endif

using ArenaScope = utils::ArenaScope<LinearAllocatorArena>;

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_ALLOCATORS_H
