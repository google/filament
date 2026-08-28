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
    p = la.alloc(1024, 1);

    // check we can allocate the whole block
    EXPECT_EQ(scratch, p);

    // check we can free everything and reallocate the whole block
    la.reset();
    p = la.alloc(1024, 1);
    EXPECT_EQ(scratch, p);

    // check we can rewind
    la.rewind(scratch + 512);
    p = la.alloc(512, 1);
    EXPECT_EQ(scratch + 512, p);

    // check we can't allocate more than the area size
    la.reset();
    p = la.alloc(1025, 1);
    EXPECT_EQ(nullptr, p);

    // check that after failure, we can allocate to the area size
    p = la.alloc(1024, 1);
    EXPECT_EQ(scratch, p);

    // check small allocations
    la.reset();
    p = la.alloc(1, 1);
    EXPECT_EQ(scratch, p);
    p = la.alloc(7, 1);
    EXPECT_EQ(scratch+1, p);
    p = la.alloc(8, 1);
    EXPECT_EQ(scratch+8, p);

    // check alignment
    la.alloc(1, 1);
    p = la.alloc(24, 32);
    EXPECT_NE(nullptr, p);
    EXPECT_EQ(0, uintptr_t(p) & 31);

    // now check that next allocation doesn't overlap previous one
    q = la.alloc(1, 1);
    EXPECT_EQ(uintptr_t(q), uintptr_t(p) + 24);

    // check free() of the top allocation
    la.reset();
    void* const a0 = la.alloc(64);
    EXPECT_EQ(scratch, a0);
    EXPECT_EQ(pointermath::add(scratch, 64), la.getCurrent());

    // freeing top allocation succeeds and rolls back
    EXPECT_TRUE(la.free(a0, 64));
    EXPECT_EQ(scratch, la.getCurrent());

    // reallocating reclaims the exact space
    void* const a1 = la.alloc(64);
    EXPECT_EQ(scratch, a1);

    // test top allocation free
    void* const b0 = la.alloc(128);
    EXPECT_EQ(pointermath::add(a1, 64 + 128), la.getCurrent());

    // freeing b0 (top) succeeds
    EXPECT_TRUE(la.free(b0, 128));
    EXPECT_EQ(pointermath::add(a1, 64), la.getCurrent());

    // reallocate and test freeing non-top (buried) allocation fails
    void* const b1 = la.alloc(128);
    void* const c1 = la.alloc(256);
    EXPECT_FALSE(la.free(b1, 128)); // b1 is buried under c1
    EXPECT_EQ(pointermath::add(c1, 256), la.getCurrent()); // current unchanged

    // test freeing with mismatched size fails
    EXPECT_FALSE(la.free(c1, 128)); // c1 is 256 bytes, not 128
    EXPECT_EQ(pointermath::add(c1, 256), la.getCurrent());

    // freeing c1 (top) succeeds
    EXPECT_TRUE(la.free(c1, 256));
    EXPECT_EQ(pointermath::add(b1, 128), la.getCurrent());

    // reset cleans up
    la.reset();
    EXPECT_EQ(scratch, la.getCurrent());
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

TEST(AllocatorTest, LeakDetectorNoLeaks) {
    using LeakArena = Arena<LinearAllocator, LockingPolicy::NoLock, TrackingPolicy::LeakDetector>;
    LeakArena arena("LeakArenaNoLeaks", 1024);

    {
        ArenaScope scope(arena);
        void* const p0 = arena.alloc(64);
        EXPECT_NE(nullptr, p0);
        EXPECT_EQ(1u, arena.getListener().getActiveAllocationCount());
        EXPECT_EQ(64u, arena.getListener().getActiveAllocationBytes());

        void* const p1 = arena.alloc(128);
        EXPECT_NE(nullptr, p1);
        EXPECT_EQ(2u, arena.getListener().getActiveAllocationCount());
        EXPECT_EQ(192u, arena.getListener().getActiveAllocationBytes());

        arena.free(p1, 128);
        EXPECT_EQ(1u, arena.getListener().getActiveAllocationCount());
        EXPECT_EQ(64u, arena.getListener().getActiveAllocationBytes());

        arena.free(p0, 64);
        EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());
        EXPECT_EQ(0u, arena.getListener().getActiveAllocationBytes());
    }

    EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationBytes());
}

TEST(AllocatorTest, LeakDetectorWithLeaksOnRewind) {
    using LeakArena = Arena<LinearAllocator, LockingPolicy::NoLock, TrackingPolicy::LeakDetector>;
    LeakArena arena("LeakArenaWithLeaks", 1024);

    {
        ArenaScope scope(arena);
        void* const p0 = arena.alloc(64);
        void* const p1 = arena.alloc(128);
        EXPECT_NE(nullptr, p0);
        EXPECT_NE(nullptr, p1);
        EXPECT_EQ(2u, arena.getListener().getActiveAllocationCount());
        EXPECT_EQ(192u, arena.getListener().getActiveAllocationBytes());

        auto const& active = arena.getListener().getActiveAllocations();
        EXPECT_NE(active.find(p0), active.end());
        EXPECT_NE(active.find(p1), active.end());
        EXPECT_EQ(64u, active.find(p0)->second.size);
        EXPECT_EQ(128u, active.find(p1)->second.size);
        EXPECT_GT(active.find(p0)->second.callstack.getFrameCount(), 0u);
    }

    // After ArenaScope exits and rewinds, leaks are logged and cleared
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationBytes());
}

TEST(AllocatorTest, LeakDetectorNestedScopes) {
    using LeakArena = Arena<LinearAllocator, LockingPolicy::NoLock, TrackingPolicy::LeakDetector>;
    LeakArena arena("LeakArenaNested", 1024);

    void* p0 = nullptr;
    {
        ArenaScope outerScope(arena);
        p0 = arena.alloc(64);
        EXPECT_NE(nullptr, p0);
        EXPECT_EQ(1u, arena.getListener().getActiveAllocationCount());

        {
            ArenaScope const innerScope(arena);
            void* const p1 = arena.alloc(128);
            EXPECT_NE(nullptr, p1);
            EXPECT_EQ(2u, arena.getListener().getActiveAllocationCount());
            EXPECT_EQ(192u, arena.getListener().getActiveAllocationBytes());
            // innerScope exits without freeing p1 -> p1 leaked on inner rewind
        }

        // After inner scope exits, only p0 remains active
        EXPECT_EQ(1u, arena.getListener().getActiveAllocationCount());
        EXPECT_EQ(64u, arena.getListener().getActiveAllocationBytes());
        auto const& active = arena.getListener().getActiveAllocations();
        EXPECT_NE(active.find(p0), active.end());

        arena.free(p0, 64);
        EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());
    }

    EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());
}

TEST(AllocatorTest, DebugAndLeakDetector) {
    using DebugLeakArena = Arena<LinearAllocator, LockingPolicy::NoLock, TrackingPolicy::DebugAndLeakDetector>;
    DebugLeakArena arena("DebugLeakArena", 1024);

    void* const p = arena.alloc(64);
    EXPECT_NE(nullptr, p);
    EXPECT_EQ(1u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(64u, arena.getListener().getActiveAllocationBytes());

    // Verify debug memory poisoning on alloc (0xeb)
    uint8_t const* const bytes = static_cast<uint8_t const*>(p);
    for (size_t i = 0; i < 64; ++i) {
        EXPECT_EQ(0xeb, bytes[i]);
    }

    arena.free(p, 64);
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());

    // Verify debug memory poisoning on free (0xef)
    for (size_t i = 0; i < 64; ++i) {
        EXPECT_EQ(0xef, bytes[i]);
    }
}

TEST(AllocatorTest, LeakDetectorReset) {
    using LeakArena = Arena<LinearAllocator, LockingPolicy::NoLock, TrackingPolicy::LeakDetector>;
    LeakArena arena("LeakArenaReset", 1024);

    arena.alloc(64);
    arena.alloc(128);
    EXPECT_EQ(2u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(192u, arena.getListener().getActiveAllocationBytes());

    arena.reset();
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationBytes());
}

TEST(AllocatorTest, LeakDetectorFreeSizeMismatch) {
    using LeakArena = Arena<LinearAllocator, LockingPolicy::NoLock, TrackingPolicy::LeakDetector>;
    LeakArena arena("LeakArenaSizeMismatch", 1024);

    void* const p = arena.alloc(64);
    EXPECT_NE(nullptr, p);
    EXPECT_EQ(1u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(64u, arena.getListener().getActiveAllocationBytes());

    // Free with mismatched size (32 bytes instead of 64) - will log warning and still remove
    arena.free(p, 32);
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationBytes());
}

TEST(AllocatorTest, HighWatermarkWastedBytes) {
    using HwArena = Arena<LinearAllocator, LockingPolicy::NoLock, TrackingPolicy::HighWatermark>;
    HwArena arena("HwArenaWasted", 1024);

    void* const p0 = arena.alloc(64);
    void* const p1 = arena.alloc(128);
    EXPECT_EQ(192u, arena.getListener().getHighWatermark());
    EXPECT_EQ(0u, arena.getListener().getWastedBytes());

    // Free p0 (buried under p1) -> logically freed, cannot be physically reclaimed -> wasted!
    arena.free(p0, 64);
    EXPECT_EQ(192u, arena.getListener().getHighWatermark());
    EXPECT_EQ(64u, arena.getListener().getWastedBytes());

    // Free p1 (at top) -> physically reclaimed -> not wasted
    arena.free(p1, 128);
    EXPECT_EQ(192u, arena.getListener().getHighWatermark());
    EXPECT_EQ(64u, arena.getListener().getWastedBytes());

    // Reset clears both high watermark current and wasted
    arena.reset();
    EXPECT_EQ(0u, arena.getListener().getWastedBytes());
}

TEST(AllocatorTest, CompositeTrackingPolicy) {
    using CustomComposite = TrackingPolicy::Composite<
            TrackingPolicy::HighWatermark,
            TrackingPolicy::Debug,
            TrackingPolicy::LeakDetector>;

    using CompArena = Arena<LinearAllocator, LockingPolicy::NoLock, CustomComposite>;
    CompArena arena("CompArena", 1024);

    void* const p0 = arena.alloc(64);
    EXPECT_NE(nullptr, p0);

    // Verify HighWatermark part
    EXPECT_EQ(64u, arena.getListener().getHighWatermark());
    EXPECT_EQ(0u, arena.getListener().getWastedBytes());

    // Verify LeakDetector part
    EXPECT_EQ(1u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(64u, arena.getListener().getActiveAllocationBytes());

    // Verify Debug part (memory poisoning)
    uint8_t const* const bytes0 = static_cast<uint8_t const*>(p0);
    for (size_t i = 0; i < 64; ++i) {
        EXPECT_EQ(0xeb, bytes0[i]);
    }

    void* const p1 = arena.alloc(128);
    EXPECT_NE(nullptr, p1);
    EXPECT_EQ(192u, arena.getListener().getHighWatermark());
    EXPECT_EQ(0u, arena.getListener().getWastedBytes());
    EXPECT_EQ(2u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(192u, arena.getListener().getActiveAllocationBytes());

    // Free p0 (buried under p1) -> logically freed, not physically freed -> wasted
    arena.free(p0, 64);
    EXPECT_EQ(1u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(128u, arena.getListener().getActiveAllocationBytes());
    EXPECT_EQ(64u, arena.getListener().getWastedBytes());
    EXPECT_EQ(64u, arena.getListener().get<TrackingPolicy::HighWatermark>().getWastedBytes());
    for (size_t i = 0; i < 64; ++i) {
        EXPECT_EQ(0xef, bytes0[i]);
    }

    // Free p1 (top allocation) -> physically reclaimed -> not added to wasted
    arena.free(p1, 128);
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationBytes());
    EXPECT_EQ(64u, arena.getListener().getWastedBytes());

    // Reset clears high watermark and wasted
    arena.reset();
    EXPECT_EQ(0u, arena.getListener().getWastedBytes());
    EXPECT_EQ(0u, arena.getListener().getActiveAllocationCount());
}

TEST(AllocatorTest, LinearAllocatorWithFallbackFree) {
    char scratch[256];
    LinearAllocatorWithFallback allocator(scratch, scratch + sizeof(scratch));

    // Multiple allocations within linear buffer
    void* const p0 = allocator.alloc(64);
    void* const p1 = allocator.alloc(128);
    EXPECT_EQ(scratch, p0);
    EXPECT_EQ(pointermath::add(scratch, 64), p1);
    EXPECT_FALSE(allocator.isHeapAllocation(p0));
    EXPECT_FALSE(allocator.isHeapAllocation(p1));
    EXPECT_EQ(pointermath::add(scratch, 192), allocator.getCurrent());

    // free() on buried linear allocation returns false
    EXPECT_FALSE(allocator.free(p0, 64));
    EXPECT_EQ(pointermath::add(scratch, 192), allocator.getCurrent());

    // Freeing top allocation in linear buffer succeeds
    EXPECT_TRUE(allocator.free(p1, 128));
    EXPECT_EQ(pointermath::add(scratch, 64), allocator.getCurrent());

    // Exceed remaining linear capacity (192 bytes remaining, request 200 bytes) -> fallback to heap
    void* const pHeap = allocator.alloc(200);
    EXPECT_NE(nullptr, pHeap);
    EXPECT_TRUE(allocator.isHeapAllocation(pHeap));

    // free() on heap allocation returns false (heap allocations are reclaimed on reset/destruction)
    EXPECT_FALSE(allocator.free(pHeap, 200));

    // reset() cleans up both heap and linear allocations
    allocator.reset();
    EXPECT_EQ(scratch, allocator.getCurrent());
}

TEST(AllocatorTest, ArenaAreaMoveSemantics) {
    using ParentArena = Arena<LinearAllocator, LockingPolicy::NoLock>;
    ParentArena parent("ParentArena", 1024);

    void* const initialCurrent = parent.getAllocator().getCurrent();

    {
        // Allocate an ArenaArea from the parent arena
        AreaPolicy::ArenaArea<ParentArena> area1(parent, 256);
        EXPECT_NE(nullptr, area1.begin());
        EXPECT_EQ(256u, area1.size());
        EXPECT_EQ(pointermath::add(initialCurrent, 256), parent.getAllocator().getCurrent());

        // Move construct area2 from area1
        AreaPolicy::ArenaArea<ParentArena> area2(std::move(area1));
        EXPECT_EQ(nullptr, area1.begin());
        EXPECT_EQ(0u, area1.size());
        EXPECT_NE(nullptr, area2.begin());
        EXPECT_EQ(256u, area2.size());

        // Parent current should still be at 256
        EXPECT_EQ(pointermath::add(initialCurrent, 256), parent.getAllocator().getCurrent());

        // Destructing moved-from area1 should be a no-op (verified when area2 is still in scope)
    }

    // After area2 destructs, parent allocation should be safely freed and current rolled back
    EXPECT_EQ(initialCurrent, parent.getAllocator().getCurrent());
}

TEST(AllocatorTest, LinearAllocatorStackLIFO) {
    char scratch[1024];
    LinearAllocator la(scratch, scratch + sizeof(scratch));

    // Allocate 5 blocks of varying sizes
    void* const p0 = la.alloc(64);
    void* const p1 = la.alloc(128);
    void* const p2 = la.alloc(32);
    void* const p3 = la.alloc(256);
    void* const p4 = la.alloc(48);

    EXPECT_EQ(scratch, p0);
    EXPECT_EQ(pointermath::add(scratch, 64), p1);
    EXPECT_EQ(pointermath::add(scratch, 192), p2);
    EXPECT_EQ(pointermath::add(scratch, 224), p3);
    EXPECT_EQ(pointermath::add(scratch, 480), p4);
    EXPECT_EQ(pointermath::add(scratch, 528), la.getCurrent());

    // Free in LIFO order (all within STACK_DEPTH = 8)
    EXPECT_TRUE(la.free(p4, 48));
    EXPECT_EQ(pointermath::add(scratch, 480), la.getCurrent());

    EXPECT_TRUE(la.free(p3, 256));
    EXPECT_EQ(pointermath::add(scratch, 224), la.getCurrent());

    EXPECT_TRUE(la.free(p2, 32));
    EXPECT_EQ(pointermath::add(scratch, 192), la.getCurrent());

    EXPECT_TRUE(la.free(p1, 128));
    EXPECT_EQ(pointermath::add(scratch, 64), la.getCurrent());

    EXPECT_TRUE(la.free(p0, 64));
    EXPECT_EQ(scratch, la.getCurrent());

    // Once empty, free() returns false
    EXPECT_FALSE(la.free(p0, 64));
    EXPECT_EQ(scratch, la.getCurrent());

    // Whole block can be reallocated
    void* const pAll = la.alloc(1024);
    EXPECT_EQ(scratch, pAll);
}

TEST(AllocatorTest, LinearAllocatorStackOverflowFree) {
    char scratch[2048];
    LinearAllocator la(scratch, scratch + sizeof(scratch));

    // Allocate 12 blocks (exceeding STACK_DEPTH = 8)
    constexpr size_t NUM_ALLOCS = 12;
    constexpr size_t BLOCK_SIZE = 64;
    void* ptrs[NUM_ALLOCS];
    for (size_t i = 0; i < NUM_ALLOCS; ++i) {
        ptrs[i] = la.alloc(BLOCK_SIZE);
        EXPECT_EQ(pointermath::add(scratch, i * BLOCK_SIZE), ptrs[i]);
    }
    EXPECT_EQ(pointermath::add(scratch, NUM_ALLOCS * BLOCK_SIZE), la.getCurrent());

    // Free the 8 most recent allocations in LIFO order (all should succeed)
    for (size_t i = 0; i < LinearAllocator::STACK_DEPTH; ++i) {
        size_t const idx = NUM_ALLOCS - 1 - i;
        EXPECT_TRUE(la.free(ptrs[idx], BLOCK_SIZE));
        EXPECT_EQ(pointermath::add(scratch, idx * BLOCK_SIZE), la.getCurrent());
    }

    // Now stack history has been exhausted (mCount == 0).
    // Attempting to free the 9th allocation (ptrs[3]) must return false.
    size_t const overflowIdx = NUM_ALLOCS - 1 - LinearAllocator::STACK_DEPTH; // index 3
    EXPECT_FALSE(la.free(ptrs[overflowIdx], BLOCK_SIZE));

    // Current pointer must remain untouched
    EXPECT_EQ(pointermath::add(scratch, (overflowIdx + 1) * BLOCK_SIZE), la.getCurrent());

    // Further older allocations also cannot be freed
    EXPECT_FALSE(la.free(ptrs[0], BLOCK_SIZE));
}

TEST(AllocatorTest, LinearAllocatorStackInterleaved) {
    char scratch[1024];
    LinearAllocator la(scratch, scratch + sizeof(scratch));

    // Allocate A, B, C
    void* const a = la.alloc(64);
    void* const b = la.alloc(64);
    void* const c = la.alloc(64);
    EXPECT_EQ(pointermath::add(scratch, 192), la.getCurrent());

    // Free C, B
    EXPECT_TRUE(la.free(c, 64));
    EXPECT_EQ(pointermath::add(scratch, 128), la.getCurrent());
    EXPECT_TRUE(la.free(b, 64));
    EXPECT_EQ(pointermath::add(scratch, 64), la.getCurrent());

    // Allocate D, E
    void* const d = la.alloc(128);
    void* const e = la.alloc(128);
    EXPECT_EQ(pointermath::add(scratch, 64), d);
    EXPECT_EQ(pointermath::add(scratch, 192), e);
    EXPECT_EQ(pointermath::add(scratch, 320), la.getCurrent());

    // Free E, D, then A
    EXPECT_TRUE(la.free(e, 128));
    EXPECT_EQ(pointermath::add(scratch, 192), la.getCurrent());
    EXPECT_TRUE(la.free(d, 128));
    EXPECT_EQ(pointermath::add(scratch, 64), la.getCurrent());
    EXPECT_TRUE(la.free(a, 64));
    EXPECT_EQ(scratch, la.getCurrent());
}
