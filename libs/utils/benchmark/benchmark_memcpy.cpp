/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "PerformanceCounters.h"

#include <benchmark/benchmark.h>

#include <utils/memalign.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

// ------------------------------------------------------------------------------------------------

#ifdef __linux__
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>
#endif

// Returns true if successful, false if failed
static bool pinThreadToCore(int core_id) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    // 0 = current thread
    int const result = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    return (result == 0);
#else
    (void)core_id;
    return false;
#endif
}

// ------------------------------------------------------------------------------------------------

static constexpr size_t ALIGNMENT = 16 << 10;

template<class T, size_t Align>
struct AlignedAllocator {
    using value_type = T;

    template<class U>
    struct rebind {
        using other = AlignedAllocator<U, Align>;
    };

    AlignedAllocator() noexcept = default;

    template<class U>
    explicit AlignedAllocator(const AlignedAllocator<U, Align>&) noexcept {}

    T* allocate(size_t const n) {
        size_t const bytes = n * sizeof(T);
        // std::aligned_alloc requires size to be a multiple of alignment.
        size_t const aligned_bytes = ((bytes + Align - 1) / Align) * Align;
        void* ptr = utils::aligned_alloc(aligned_bytes, Align);
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, size_t) noexcept {
        utils::aligned_free(p);
    }
};

template<typename T>
using PageAlignedVector = std::vector<T, AlignedAllocator<T, ALIGNMENT>>;

// ------------------------------------------------------------------------------------------------

static void BM_memcpy(benchmark::State& state) {
    // pinThreadToCore(7);
    int64_t const size = state.range(0);
    PageAlignedVector<char> src(size);
    PageAlignedVector<char> dst(size);

    // make all these pages resident
    memset(src.data(), 0, size);
    memset(dst.data(), 0, size);
    benchmark::ClobberMemory();

    {
        // PerformanceCounters const pc(state);
        for (auto _: state) {
            memcpy(dst.data(), src.data(), size);
            benchmark::DoNotOptimize(dst);
            benchmark::DoNotOptimize(src);
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

static void BM_memset(benchmark::State& state) {
    //pinThreadToCore(7);
    int64_t const size = state.range(0);
    PageAlignedVector<char> src(size);

    // make all these pages resident
    memset(src.data(), 0, size);
    benchmark::ClobberMemory();

    {
        // PerformanceCounters const pc(state);
        for (auto _: state) {
            memset(src.data(), 0, size);
            benchmark::DoNotOptimize(src);
        }
    }
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

BENCHMARK(BM_memcpy)
    ->DenseRange( 4<<10, 128<<10,   8<<10)
    ->DenseRange(128<<10,  4<<20, 128<<10)
    ->DenseRange(  4<<20, 16<<20,   1<<20)
    ->DenseRange( 16<<20, 32<<20,   2<<20)
    ;

BENCHMARK(BM_memset)
    ->DenseRange( 4<<10, 128<<10,   8<<10)
    ->DenseRange(128<<10,  4<<20, 128<<10)
    ->DenseRange(  4<<20, 16<<20,   1<<20)
    ->DenseRange( 16<<20, 32<<20,   2<<20)
    ;
