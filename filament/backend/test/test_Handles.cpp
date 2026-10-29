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

#include <private/backend/HandleAllocator.h>

#include <utils/Allocator.h>
#include <utils/compiler.h>
#include <utils/Panic.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <deque>
#include <random>
#include <thread>
#include <vector>

#include <stddef.h>
#include <stdint.h>

using namespace filament::backend;

// FIXME: consider making this constant non-private so we can use it in tests.
static constexpr uint32_t HANDLE_HEAP_FLAG = 0x80000000u;
static constexpr size_t POOL_SIZE_BYTES = 8 * 1024U * 1024U;
// The allocator must be an instantiation that exists in the backend library:
// HandleAllocatorGL normally, or HandleAllocatorMTL on Metal-only builds
// (e.g. an iOS build with the OpenGL backend disabled).
#if defined(FILAMENT_SUPPORTS_OPENGL)
#define HandleAllocatorTest  HandleAllocatorGL
#elif defined(FILAMENT_SUPPORTS_METAL)
#define HandleAllocatorTest  HandleAllocatorMTL
#elif defined(FILAMENT_SUPPORTS_VULKAN)
#define HandleAllocatorTest  HandleAllocatorVK
#elif defined(FILAMENT_SUPPORTS_WEBGPU)
#define HandleAllocatorTest  HandleAllocatorWGPU
#else
#error test_Handles requires at least one backend
#endif
struct MyHandle {
};

struct Concrete : public MyHandle {
    uint8_t data[32];
};

struct ConcreteMedium : public MyHandle {
    uint8_t data[64];
};

struct ConcreteLarge : public MyHandle {
    uint8_t data[184];
};

// NOTE: actual count may be lower due to alignment requirements
constexpr size_t const POOL_HANDLE_COUNT =
        POOL_SIZE_BYTES / HandleAllocatorTest::bucketSizesSum;

#if GTEST_HAS_EXCEPTIONS
#define EXPECT_THROW_IF_ENABLED EXPECT_THROW
#else
#define EXPECT_THROW_IF_ENABLED(args...)
#endif

TEST(HandlesTest, useAfterFreePool) {
    HandleAllocatorTest allocator("Test Handles", POOL_SIZE_BYTES);

    Handle<MyHandle> handleA = allocator.allocate<Concrete>();
    allocator.deallocate<Concrete>(handleA);

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
    allocator.deallocate<Concrete>(handleA);

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

TEST(HandlesTest, bucketIsolationNoFragmentation) {
    constexpr size_t ARENA_BYTES = 256 * 1024u;
    HandleAllocatorTest allocator("Fragmentation Test Handles", ARENA_BYTES);

    // 1. Burst-allocate Small (P0) handles until the P0 pool is completely exhausted and spills to
    // the heap.
    std::vector<Handle<MyHandle>> smallBurst;
    for (;;) {
        Handle<MyHandle> h = allocator.allocate<Concrete>();
        bool const isHeap = (h.getId() & HANDLE_HEAP_FLAG) != 0u;
        smallBurst.push_back(h);
        if (isHeap) {
            break;
        }
    }
    ASSERT_GT(smallBurst.size(), 100u);

    // 2. Even while P0 is 100% full, P2 has its own dedicated partition and must still allocate
    // in-pool rather than spilling to the heap.
    Handle<MyHandle> hLargeWhileP0Full = allocator.allocate<ConcreteLarge>();
    EXPECT_EQ(0u, hLargeWhileP0Full.getId() & HANDLE_HEAP_FLAG);
    auto* const pLargeFirst =
            reinterpret_cast<char*>(allocator.handle_cast<ConcreteLarge*>(hLargeWhileP0Full));
    allocator.deallocate<ConcreteLarge>(hLargeWhileP0Full);

    // 3. Free the entire P0 burst. Because each bucket has its own partition, P0's freed slots
    // cannot fragment or cannibalize P2 capacity: allocating Large handles must succeed in-pool
    // and reuse the recycled P2 slot first.
    for (Handle<MyHandle> h : smallBurst) {
        allocator.deallocate<Concrete>(h);
    }

    std::vector<Handle<MyHandle>> largeHandles;
    for (size_t i = 0; i < 64; ++i) {
        Handle<MyHandle> h = allocator.allocate<ConcreteLarge>();
        EXPECT_EQ(0u, h.getId() & HANDLE_HEAP_FLAG);
        largeHandles.push_back(h);
    }
    EXPECT_EQ(pLargeFirst,
            reinterpret_cast<char*>(allocator.handle_cast<ConcreteLarge*>(largeHandles.front())));

    for (Handle<MyHandle> h : largeHandles) {
        allocator.deallocate<ConcreteLarge>(h);
    }
}

namespace {

enum class SizeClass : uint8_t {
    SMALL = 0,
    MEDIUM = 1,
    LARGE = 2
};

struct LiveEntry {
    Handle<MyHandle> handle;
    SizeClass cls;
    char* ptr;
    size_t size;
    uint32_t seed;
};

uint8_t patternByte(uint32_t seed, size_t byteIndex) noexcept {
    uint32_t const x = seed * 2654435761u + static_cast<uint32_t>(byteIndex) * 97u;
    return static_cast<uint8_t>((x >> 24) ^ (x >> 8) ^ x);
}

void fillPattern(uint8_t* data, size_t size, uint32_t seed) noexcept {
    for (size_t i = 0; i < size; ++i) {
        data[i] = patternByte(seed, i);
    }
}

void verifyPattern(uint8_t const* data, size_t size, uint32_t seed) {
    for (size_t i = 0; i < size; ++i) {
        ASSERT_EQ(patternByte(seed, i), data[i]);
    }
}

LiveEntry allocateTracked(HandleAllocatorTest& allocator, SizeClass cls, uint32_t seed) {
    LiveEntry entry{};
    entry.cls = cls;
    entry.seed = seed;
    switch (cls) {
        case SizeClass::SMALL: {
            entry.handle = allocator.allocate<Concrete>();
            auto* const p = allocator.handle_cast<Concrete*>(entry.handle);
            fillPattern(p->data, sizeof(p->data), seed);
            entry.ptr = reinterpret_cast<char*>(p);
            entry.size = sizeof(Concrete);
            break;
        }
        case SizeClass::MEDIUM: {
            entry.handle = allocator.allocate<ConcreteMedium>();
            auto* const p = allocator.handle_cast<ConcreteMedium*>(entry.handle);
            fillPattern(p->data, sizeof(p->data), seed);
            entry.ptr = reinterpret_cast<char*>(p);
            entry.size = sizeof(ConcreteMedium);
            break;
        }
        case SizeClass::LARGE: {
            entry.handle = allocator.allocate<ConcreteLarge>();
            auto* const p = allocator.handle_cast<ConcreteLarge*>(entry.handle);
            fillPattern(p->data, sizeof(p->data), seed);
            entry.ptr = reinterpret_cast<char*>(p);
            entry.size = sizeof(ConcreteLarge);
            break;
        }
    }
    return entry;
}

void verifyTracked(HandleAllocatorTest& allocator, LiveEntry const& entry) {
    Handle<MyHandle> h = entry.handle;
    ASSERT_TRUE(allocator.is_valid(h));
    switch (entry.cls) {
        case SizeClass::SMALL: {
            auto* const p = allocator.handle_cast<Concrete*>(entry.handle);
            ASSERT_EQ(entry.ptr, reinterpret_cast<char*>(p));
            verifyPattern(p->data, sizeof(p->data), entry.seed);
            break;
        }
        case SizeClass::MEDIUM: {
            auto* const p = allocator.handle_cast<ConcreteMedium*>(entry.handle);
            ASSERT_EQ(entry.ptr, reinterpret_cast<char*>(p));
            verifyPattern(p->data, sizeof(p->data), entry.seed);
            break;
        }
        case SizeClass::LARGE: {
            auto* const p = allocator.handle_cast<ConcreteLarge*>(entry.handle);
            ASSERT_EQ(entry.ptr, reinterpret_cast<char*>(p));
            verifyPattern(p->data, sizeof(p->data), entry.seed);
            break;
        }
    }
}

void deallocateTracked(HandleAllocatorTest& allocator, LiveEntry& entry) {
    verifyTracked(allocator, entry);
    Handle<MyHandle> staleCopy = entry.handle;
    switch (entry.cls) {
        case SizeClass::SMALL:
            allocator.deallocate<Concrete>(entry.handle);
            break;
        case SizeClass::MEDIUM:
            allocator.deallocate<ConcreteMedium>(entry.handle);
            break;
        case SizeClass::LARGE:
            allocator.deallocate<ConcreteLarge>(entry.handle);
            break;
    }
    EXPECT_FALSE(allocator.is_valid(staleCopy));
}

template<typename Container>
void verifyNoOverlapsAndIntactPayloads(HandleAllocatorTest& allocator, Container const& live) {
    struct Interval {
        char const* begin;
        char const* end;
    };
    std::vector<Interval> intervals;
    intervals.reserve(live.size());
    for (LiveEntry const& e : live) {
        verifyTracked(allocator, e);
        intervals.push_back({ e.ptr, e.ptr + e.size });
    }
    std::sort(intervals.begin(), intervals.end(), [](Interval const& a, Interval const& b) {
        return a.begin < b.begin;
    });
    for (size_t i = 1; i < intervals.size(); ++i) {
        ASSERT_LE(intervals[i - 1].end, intervals[i].begin);
    }
}

} // anonymous namespace

TEST(HandlesTest, allocationPatternsAndPayloadIntegrity) {
    // Use a 256 KiB arena so we can quickly exercise both in-pool patterns and exact exhaustion.
    constexpr size_t ARENA_BYTES = 256 * 1024u;
    HandleAllocatorTest allocator("Pattern Test Handles", ARENA_BYTES);
    uint32_t nextSeed = 1u;

    // 1. FIFO Queue Churn across mixed size classes.
    {
        std::deque<LiveEntry> fifo;
        for (size_t i = 0; i < 60; ++i) {
            SizeClass const cls = static_cast<SizeClass>(i % 3);
            fifo.push_back(allocateTracked(allocator, cls, nextSeed++));
        }
        verifyNoOverlapsAndIntactPayloads(allocator, fifo);

        for (size_t step = 0; step < 180; ++step) {
            LiveEntry oldest = fifo.front();
            fifo.pop_front();
            deallocateTracked(allocator, oldest);

            SizeClass const cls = static_cast<SizeClass>((step + 1) % 3);
            fifo.push_back(allocateTracked(allocator, cls, nextSeed++));
        }
        verifyNoOverlapsAndIntactPayloads(allocator, fifo);

        while (!fifo.empty()) {
            LiveEntry e = fifo.front();
            fifo.pop_front();
            deallocateTracked(allocator, e);
        }
    }

    // 2. LIFO Stack Churn across mixed size classes.
    {
        std::vector<LiveEntry> stack;
        for (size_t wave = 0; wave < 4; ++wave) {
            for (size_t i = 0; i < 30; ++i) {
                SizeClass const cls = static_cast<SizeClass>((i + wave) % 3);
                stack.push_back(allocateTracked(allocator, cls, nextSeed++));
            }
            verifyNoOverlapsAndIntactPayloads(allocator, stack);

            // Pop half in LIFO order and verify LIFO reuse on the next wave.
            for (size_t i = 0; i < 15; ++i) {
                LiveEntry top = stack.back();
                stack.pop_back();
                deallocateTracked(allocator, top);
            }
            verifyNoOverlapsAndIntactPayloads(allocator, stack);
        }
        while (!stack.empty()) {
            LiveEntry top = stack.back();
            stack.pop_back();
            deallocateTracked(allocator, top);
        }
    }

    // 3. Swiss-Cheese (Alternating Holes) & 16-Generation Age Wrap-Around.
    {
        std::vector<LiveEntry> grid;
        grid.reserve(90);
        for (size_t i = 0; i < 90; ++i) {
            SizeClass const cls = static_cast<SizeClass>(i % 3);
            grid.push_back(allocateTracked(allocator, cls, nextSeed++));
        }
        verifyNoOverlapsAndIntactPayloads(allocator, grid);

        // Free every even index, leaving odd indices live as canaries around each freed hole.
        std::vector<LiveEntry> oddCanaries;
        oddCanaries.reserve(45);
        for (size_t i = 0; i < grid.size(); ++i) {
            if (i % 2 == 0) {
                deallocateTracked(allocator, grid[i]);
            } else {
                oddCanaries.push_back(grid[i]);
            }
        }
        // Freed nodes now hold FreeList::Node::next pointers; odd canary payloads must be intact.
        verifyNoOverlapsAndIntactPayloads(allocator, oddCanaries);

        // Cycle a single LARGE slot through all 16 4-bit age generations (wrap-around) while
        // surrounded by live canary slots.
        LiveEntry probe = allocateTracked(allocator, SizeClass::LARGE, nextSeed++);
        Handle<MyHandle> const initialGenerationHandle = probe.handle;
        char* const probePtr = probe.ptr;
        deallocateTracked(allocator, probe);

        for (size_t gen = 1; gen < 16; ++gen) {
            LiveEntry cycled = allocateTracked(allocator, SizeClass::LARGE, nextSeed++);
            ASSERT_EQ(probePtr, cycled.ptr);
            // Generations 1..15 must not match generation 0's 4-bit age tag.
            EXPECT_FALSE(allocator.is_valid(const_cast<Handle<MyHandle>&>(initialGenerationHandle)));
            deallocateTracked(allocator, cycled);
        }
        // Generation 16 wraps the 4-bit age back to initialGenerationHandle's age.
        LiveEntry wrapped = allocateTracked(allocator, SizeClass::LARGE, nextSeed++);
        ASSERT_EQ(probePtr, wrapped.ptr);
        EXPECT_TRUE(allocator.is_valid(const_cast<Handle<MyHandle>&>(initialGenerationHandle)));
        deallocateTracked(allocator, wrapped);

        // Verify all 45 canary slots survived the 16 age wrap-around cycles unscathed.
        verifyNoOverlapsAndIntactPayloads(allocator, oddCanaries);
        for (LiveEntry& e : oddCanaries) {
            deallocateTracked(allocator, e);
        }
    }

    // 4. Deterministic Pseudo-Random Churn (5,000 steps across all 3 size classes).
    {
        std::mt19937 rng(0xBADC0FFEu);
        std::vector<LiveEntry> active;
        active.reserve(128);

        for (size_t step = 0; step < 5000; ++step) {
            bool const doAlloc = active.empty() || (active.size() < 120 && (rng() % 100) < 55);
            if (doAlloc) {
                SizeClass const cls = static_cast<SizeClass>(rng() % 3);
                LiveEntry e = allocateTracked(allocator, cls, nextSeed++);
                ASSERT_EQ(0u, e.handle.getId() & HANDLE_HEAP_FLAG);
                active.push_back(e);
            } else {
                size_t const idx = rng() % active.size();
                deallocateTracked(allocator, active[idx]);
                active[idx] = active.back();
                active.pop_back();
            }
            if (step % 500 == 0) {
                verifyNoOverlapsAndIntactPayloads(allocator, active);
            }
        }
        verifyNoOverlapsAndIntactPayloads(allocator, active);
        for (LiveEntry& e : active) {
            deallocateTracked(allocator, e);
        }
    }

    // 5. Exhaustion -> Heap Spillover -> Pool Recovery while mCur == mEnd.
    {
        std::vector<LiveEntry> poolEntries;
        std::vector<LiveEntry> heapEntries;

        // Allocate mixed sizes until all three freelists and any remaining tail space in
        // [mCur, mEnd) are completely exhausted for every size class.
        bool exhausted[3] = { false, false, false };
        for (size_t i = 0; !(exhausted[0] && exhausted[1] && exhausted[2]); ++i) {
            SizeClass const cls = static_cast<SizeClass>(i % 3);
            if (exhausted[static_cast<size_t>(cls)]) {
                continue;
            }
            LiveEntry e = allocateTracked(allocator, cls, nextSeed++);
            if (e.handle.getId() & HANDLE_HEAP_FLAG) {
                exhausted[static_cast<size_t>(cls)] = true;
                heapEntries.push_back(e);
            } else {
                poolEntries.push_back(e);
            }
        }
        ASSERT_GT(poolEntries.size(), 100u);

        // Spill additional handles of all three sizes onto the system heap.
        for (size_t i = 0; i < 12; ++i) {
            SizeClass const cls = static_cast<SizeClass>(i % 3);
            LiveEntry e = allocateTracked(allocator, cls, nextSeed++);
            EXPECT_NE(0u, e.handle.getId() & HANDLE_HEAP_FLAG);
            heapEntries.push_back(e);
        }

        // Find and free one pool handle of each size class while mCur is exhausted.
        for (SizeClass targetCls : { SizeClass::SMALL, SizeClass::MEDIUM, SizeClass::LARGE }) {
            auto it = std::find_if(poolEntries.begin(), poolEntries.end(),
                    [targetCls](LiveEntry const& e) { return e.cls == targetCls; });
            ASSERT_NE(poolEntries.end(), it);
            char* const expectedRecycledPtr = it->ptr;
            deallocateTracked(allocator, *it);
            poolEntries.erase(it);

            // Re-allocating the same size class must immediately succeed as a POOL handle (not
            // heap) by popping from the corresponding recycled FreeList.
            LiveEntry recovered = allocateTracked(allocator, targetCls, nextSeed++);
            EXPECT_EQ(0u, recovered.handle.getId() & HANDLE_HEAP_FLAG);
            EXPECT_EQ(expectedRecycledPtr, recovered.ptr);
            poolEntries.push_back(recovered);
        }

        verifyNoOverlapsAndIntactPayloads(allocator, poolEntries);
        verifyNoOverlapsAndIntactPayloads(allocator, heapEntries);

        // Drain all pool and heap handles so ~HandleAllocator does not panic on leaks.
        for (LiveEntry& e : poolEntries) {
            deallocateTracked(allocator, e);
        }
        for (LiveEntry& e : heapEntries) {
            deallocateTracked(allocator, e);
        }
    }
}

TEST(HandlesTest, multithreadedStress) {
    // Size the arena so 4 threads concurrently hit both the in-pool fast path and heap fallback.
    constexpr size_t ARENA_BYTES = 64 * 1024u;
    constexpr size_t NUM_THREADS = 4;
    constexpr size_t ITERATIONS = 1500;

    HandleAllocatorTest allocator("MT Stress Handles", ARENA_BYTES);
    std::atomic<bool> start{ false };
    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for (size_t tid = 0; tid < NUM_THREADS; ++tid) {
        threads.emplace_back([&, tid]() {
            std::mt19937 rng(static_cast<uint32_t>(0x12345678u + tid * 997u));
            std::vector<LiveEntry> held;
            held.reserve(96);
            uint32_t seedBase = static_cast<uint32_t>((tid + 1) << 20);

            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (size_t iter = 0; iter < ITERATIONS; ++iter) {
                bool const doAlloc = held.empty() || (held.size() < 96 && (rng() & 1u) != 0u);
                if (doAlloc) {
                    SizeClass const cls = static_cast<SizeClass>(rng() % 3);
                    held.push_back(allocateTracked(allocator, cls, seedBase++));
                } else {
                    size_t const idx = rng() % held.size();
                    deallocateTracked(allocator, held[idx]);
                    held[idx] = held.back();
                    held.pop_back();
                }
            }

            for (LiveEntry& e : held) {
                deallocateTracked(allocator, e);
            }
        });
    }

    start.store(true, std::memory_order_release);
    for (auto& t : threads) {
        t.join();
    }
}
