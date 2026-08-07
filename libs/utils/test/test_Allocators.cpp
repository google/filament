/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <utils/Allocator.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <bitset>
#include <functional>
#include <utility>
#include <vector>

using namespace utils;

TEST(AllocatorTest, LinearAllocator) {
    char scratch[1024];
    void* p = nullptr;
    void* q = nullptr;

    LinearAllocator la(scratch, scratch+sizeof(scratch));
    p = la.alloc(1024, 1, 0);

    // check we can allocate the whole block
    EXPECT_EQ(scratch, p);

    // check we can free everything and reallocate the whole block
    la.reset();
    p = la.alloc(1024, 1, 0);
    EXPECT_EQ(scratch, p);

    // check we can rewind
    la.rewind(scratch + 512);
    p = la.alloc(512, 1, 0);
    EXPECT_EQ(scratch + 512, p);

    // check we can't allocate more than the area size
    la.reset();
    p = la.alloc(1025, 1, 0);
    EXPECT_EQ(nullptr, p);

    // check that after failure, we can allocate to the area size
    p = la.alloc(1024, 1, 0);
    EXPECT_EQ(scratch, p);

    // check small allocations
    la.reset();
    p = la.alloc(1, 1, 0);
    EXPECT_EQ(scratch, p);
    p = la.alloc(7, 1, 0);
    EXPECT_EQ(scratch+1, p);
    p = la.alloc(8, 1, 0);
    EXPECT_EQ(scratch+8, p);

    // check alignment
    la.alloc(1, 1, 0);
    p = la.alloc(24, 32, 0);
    EXPECT_NE(nullptr, p);
    EXPECT_EQ(0, uintptr_t(p) & 31);

    // now check that next allocation doesn't overlap previous one
    q = la.alloc(1, 1, 0);
    EXPECT_EQ(uintptr_t(q), uintptr_t(p) + 24);

    // check alignment + offset
    la.alloc(3, 1, 0);
    p = la.alloc(sizeof(float)*4, 32, 4);
    EXPECT_EQ(0, uintptr_t(p) & 31);

    // now check that next allocation doesn't overlap previous one
    q = la.alloc(1, 1, 0);
    EXPECT_EQ(uintptr_t(q), uintptr_t(p) + sizeof(float)*4);
}


TEST(AllocatorTest, PoolAllocator) {
    char scratch[1024 + 31];
    void* p = nullptr;
    void* q = nullptr;
    std::bitset<16> used;

    // verify buffers have not been clobbered
    auto check = [](char const* p, int const v, size_t const s)->bool {
        for (size_t i = 0; i<s ; ++i) {
            if (p[i] != v) {
                return false;
            }
        }
        return true;
    };

    // pool of 64-bytes objects aligned on 32 bytes
    PoolAllocator<64, 32> pa(scratch, scratch+sizeof(scratch));
    void* const b = pointermath::align(scratch, 32, 0);

    // repeat the test multiple times
    for (size_t k=0 ; k<16 ; k++) {
        // make sure we can allocate exactly 16 of those objects
        for (size_t i = 0; i < 16; i++) {
            p = pa.alloc();
            EXPECT_NE(nullptr, p);
            EXPECT_EQ(0, uintptr_t(p) & 31);

            size_t const j = (uintptr_t(p) - uintptr_t(b)) / 64;
            //printf("%3d", j);
            memset(p, int(j + 1), 64);
        }
        //printf("\n");

        // check an extra one fails
        q = pa.alloc();
        EXPECT_EQ(nullptr, q);

        // check that buffers where not clobbered
        q = b;
        for (size_t i = 0; i < 16; i++) {
            EXPECT_TRUE(check((char const*)q, int(i + 1), 64));
            q = pointermath::add(q, 64);
        }

        // now free all our buffers
        used.set();
        q = b;
        for (size_t i = 0; i < 16; i++) {
            // use gray-coding so we don't free exactly linearly
            size_t const j = ((i^k) >> 1) ^ (i^k);
            p = pointermath::add(q, j * 64);
            pa.free(p);
            used[j] = false;
            if (j > 0 && used[j - 1]) {
                // check that the previous buffer didn't get clobbered
                EXPECT_TRUE(check((char const*) pointermath::add(p, -64), int(j - 1 + 1), 64));
            }
            if (j < 15 && used[j + 1]) {
                // check that the following buffer didn't get clobbered
                EXPECT_TRUE(check((char const*) pointermath::add(p, +64), int(j + 1 + 1), 64));
            }
        }
        EXPECT_FALSE(used.any());
    }
}


TEST(AllocatorTest, CppAllocator) {
    struct Tracking {
        Tracking() noexcept { }
        Tracking(const char* name, void const* base, size_t size) noexcept { }
        void onAlloc(void* p, size_t size, size_t alignment, size_t extra) {
            allocations.push_back(p);
        }
        void onFree(void* p, size_t) {
            auto const pos = std::find(allocations.begin(), allocations.end(), p);
            EXPECT_TRUE(pos != allocations.end());
        }
        std::vector<void*> allocations;
    };

    using CppArena = Arena<PoolAllocator<8, 8, sizeof(void*)>, LockingPolicy::NoLock, Tracking>;
    static int count = 0;
    count = 0;
    struct Foo {
        ~Foo() {
            ++count;
        }
        struct Tag {
            CppArena* arena;
        };

        void* operator new(size_t const size, CppArena& arena) {
            void* p = arena.alloc(size, alignof(Foo), sizeof(Tag));
            Tag* tag = static_cast<Tag*>(p) - 1;
            tag->arena = &arena;
            return p;
        }

        void operator delete(void* p, size_t s) {
            // don't do anything
            Tag const* tag = static_cast<Tag*>(p) - 1;
            tag->arena->free(p, s);
        }
        char dummy[8];
    };
    CppArena arena("CppArena", 1024);

    // check we can override operator new and use one of our allocator
    Foo* p0 = new(arena) Foo;
    EXPECT_NE(nullptr, p0);

    Foo* p1 = new(arena) Foo;
    EXPECT_NE(nullptr, p1);

    EXPECT_EQ(0, count);

    delete p0;
    EXPECT_EQ(1, count);

    delete p1;
    EXPECT_EQ(2, count);
}

TEST(AllocatorTest, ArenaScope) {
    using LinearArena = Arena<LinearAllocator, LockingPolicy::NoLock>;
    LinearArena arena("ArenaScopeTest", 1024);

    void* const initial = arena.getAllocator().getCurrent();

    {
        ArenaScope scope(arena);
        EXPECT_EQ(&arena, &scope.getArena());

        void* const p0 = arena.alloc(128);
        EXPECT_NE(nullptr, p0);
        EXPECT_EQ(pointermath::add(initial, 128), arena.getAllocator().getCurrent());

        {
            ArenaScope const nestedScope(arena);
            void* const p1 = arena.alloc(256);
            EXPECT_NE(nullptr, p1);
            EXPECT_EQ(pointermath::add(p1, 256), arena.getAllocator().getCurrent());
        }

        // After nested scope exits, current should be rewound to p0 + 128
        EXPECT_EQ(pointermath::add(initial, 128), arena.getAllocator().getCurrent());

        // We can allocate again from that rewind point
        void* const p2 = arena.alloc(64);
        EXPECT_NE(nullptr, p2);
        EXPECT_EQ(pointermath::add(p2, 64), arena.getAllocator().getCurrent());
    }

    // After outer scope exits, current should be rewound to initial
    EXPECT_EQ(initial, arena.getAllocator().getCurrent());

    // Check with another arena type that supports rewind, e.g. LinearAllocatorWithFallback
    using FallbackArena = Arena<LinearAllocatorWithFallback, LockingPolicy::NoLock>;
    FallbackArena fallbackArena("FallbackArenaScopeTest", 1024);
    void* const fallbackInitial = fallbackArena.getAllocator().getCurrent();

    {
        ArenaScope const fallbackScope(fallbackArena);
        void* const pf0 = fallbackArena.alloc(256);
        EXPECT_NE(nullptr, pf0);
        EXPECT_NE(fallbackInitial, fallbackArena.getAllocator().getCurrent());
    }

    EXPECT_EQ(fallbackInitial, fallbackArena.getAllocator().getCurrent());

    // Check with TrackingPolicy::Debug to verify active allocation count and bytes tracking
    using DebugArena = Arena<LinearAllocator, LockingPolicy::NoLock, TrackingPolicy::Debug>;
    DebugArena debugArena("DebugArenaScopeTest", 1024);
    EXPECT_EQ(0, debugArena.getListener().getActiveAllocationCount());
    EXPECT_EQ(0, debugArena.getListener().getActiveAllocationBytes());

    {
        ArenaScope debugScope(debugArena);
        void* const p = debugArena.alloc(128);
        EXPECT_NE(nullptr, p);
        EXPECT_EQ(1, debugArena.getListener().getActiveAllocationCount());
        EXPECT_EQ(128, debugArena.getListener().getActiveAllocationBytes());

        // Logical free balances the active count and bytes
        debugArena.free(p, 128);
        EXPECT_EQ(0, debugArena.getListener().getActiveAllocationCount());
        EXPECT_EQ(0, debugArena.getListener().getActiveAllocationBytes());
    }

    EXPECT_EQ(0, debugArena.getListener().getActiveAllocationCount());
    EXPECT_EQ(0, debugArena.getListener().getActiveAllocationBytes());
}

TEST(AllocatorTest, STLAllocator) {
    struct Tracking {
        Tracking() noexcept { }
        Tracking(const char* name, void const* base, size_t size) noexcept { }
        void onAlloc(void* p, size_t size, size_t alignment, size_t extra) {
            allocations.push_back(p);
        }
        void onFree(void* p, size_t) {
            auto const pos = std::find(allocations.begin(), allocations.end(), p);
            EXPECT_TRUE(pos != allocations.end());
            allocations.erase(pos);
        }
        void onLogicalFree(void* p, size_t size) {
            onFree(p, size);
        }
        void onReset() noexcept { }
        void onRewind(void const* addr) noexcept { }
        size_t getActiveAllocationCount() const noexcept { return allocations.size(); }
        size_t getActiveAllocationBytes() const noexcept { return 0; }
        std::vector<void*> allocations;
    };


    using Arena = Arena<LinearAllocator, LockingPolicy::NoLock, Tracking>;
    static_assert(detail::has_logical_free_v<Tracking>, "Tracking must have onLogicalFree");
    Arena arena("arena", 1204);
    Arena arena2("arena2", 1204);
    STLAllocator<int, Arena> allocator(arena);
    STLAllocator<int, Arena> allocator2(arena2);
    EXPECT_TRUE(allocator != allocator2);
    EXPECT_TRUE(allocator == allocator);

    STLAllocator<int, Arena>::rebind<char>::other charAllocator(arena);
    EXPECT_TRUE(allocator == charAllocator);

    STLAllocator<int, Arena> allocatorCopy(allocator);
    EXPECT_TRUE(allocator == allocatorCopy);

    STLAllocator<int, Arena> allocatorFromCharCopy(charAllocator);
    EXPECT_TRUE(allocatorFromCharCopy == charAllocator);


    {
        std::vector<int, STLAllocator<int, Arena>> vector(allocator);
        vector.push_back(1);
        EXPECT_GT(arena.getListener().allocations.size(), 0);
        vector.push_back(2);
        vector.push_back(3);
        vector.push_back(4);
        vector.clear();
    }

    EXPECT_EQ(0, arena.getListener().allocations.size());
}
