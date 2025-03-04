/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <gtest/gtest.h>

#include <utils/Allocator.h>
#include <utils/compiler.h>
#include <private/backend/HandleAllocator.h>
#include "utils/Panic.h"

using namespace filament::backend;

// FIXME: consider making this constant non-private so we can use it in tests.
static constexpr uint32_t HANDLE_HEAP_FLAG = 0x80000000u;
static constexpr size_t POOL_SIZE_BYTES = 8 * 1024U * 1024U;
// NOTE: actual count may be lower due to alignment requirements
constexpr size_t const POOL_HANDLE_COUNT = POOL_SIZE_BYTES / (32 + 96 + 136); // 31775

// This must match HandleAllocatorGL, so its implementation is present on all platforms.
#define HandleAllocatorTest  HandleAllocator<32,  96, 136>    // ~4520 / pool / MiB

struct MyHandle {
};

struct Concrete : public MyHandle {
    uint8_t data[32];
};

#if GTEST_HAS_EXCEPTIONS
#define EXPECT_THROW_IF_ENABLED EXPECT_THROW
#else
#define EXPECT_THROW_IF_ENABLED(args...)
#endif

TEST(HandlesTest, useAfterFreePool) {
    HandleAllocatorTest allocator("Test Handles", POOL_SIZE_BYTES);

    Handle<MyHandle> handleA = allocator.allocate<Concrete>();
    allocator.deallocate(handleA);

    Handle<MyHandle> handleB = allocator.allocate<Concrete>();

    EXPECT_THROW_IF_ENABLED({
        Concrete* actualHandleA = allocator.handle_cast<Concrete*>(handleA);
    }, utils::PostconditionPanic);
}

TEST(HandlesTest, useAfterFreeHeap) {
    HandleAllocatorTest allocator("Test Handles", POOL_SIZE_BYTES);

    // Use up all the non-heap handles.
    for (int i = 0; i < POOL_HANDLE_COUNT; i++) {
        Handle<MyHandle> handle = allocator.allocate<Concrete>();
    }

    // This one is guaranteed to be a heap handle.
    Handle<MyHandle> handleA = allocator.allocate<Concrete>();
    EXPECT_TRUE(handleA.getId() & HANDLE_HEAP_FLAG);
    allocator.deallocate(handleA);

    Handle<MyHandle> handleB = allocator.allocate<Concrete>();

    // This should assert:
    EXPECT_THROW_IF_ENABLED({
        Concrete* actualHandleA = allocator.handle_cast<Concrete*>(handleA);
    }, utils::PostconditionPanic);

    // simulate a "corrupted" handle
    Handle<MyHandle> corruptedHandle { HANDLE_HEAP_FLAG | 0x123456 };

    EXPECT_THROW_IF_ENABLED({
        Concrete* actualHandleA = allocator.handle_cast<Concrete*>(corruptedHandle);
    }, utils::PostconditionPanic);
}

TEST(HandlesTest, isValid) {
    HandleAllocatorTest allocator("Test Handles", POOL_SIZE_BYTES);
    Handle<MyHandle> poolHandle = allocator.allocate<Concrete>();
    EXPECT_TRUE((poolHandle.getId() & HANDLE_HEAP_FLAG) == 0u);

    // Use up all the non-heap handles.
    for (int i = 0; i < POOL_HANDLE_COUNT; i++) {
        Handle<MyHandle> handle = allocator.allocate<Concrete>();
    }

    // This one is guaranteed to be a heap handle.
    Handle<MyHandle> heapHandle = allocator.allocate<Concrete>();
    EXPECT_TRUE(heapHandle.getId() & HANDLE_HEAP_FLAG);

    EXPECT_TRUE(allocator.is_valid(poolHandle));
    EXPECT_TRUE(allocator.is_valid(heapHandle));

    {
        // null handles are invalid
        Handle<MyHandle> handle;
        EXPECT_FALSE(allocator.is_valid(handle));
    }
    {
        Handle<MyHandle> handle { HANDLE_HEAP_FLAG | 0x123456 };
        EXPECT_FALSE(allocator.is_valid(handle));
    }
}
