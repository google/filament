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

#include <utils/Allocator.h>
#include <utils/compiler.h>
#include <utils/Mutex.h>

#include <benchmark/benchmark.h>

using namespace utils;


class Allocators : public benchmark::Fixture {
public:
    Allocators();
    ~Allocators() override;

protected:
    struct alignas(64) Payload {};

    static_assert(sizeof(Payload) == 64, "Payload must be 64 bytes");

    utils::Arena<utils::ObjectPoolAllocator<Payload>, LockingPolicy::NoLock> mPoolAllocatorNoLock;
    utils::Arena<utils::ObjectPoolAllocator<Payload>, std::mutex> mPoolAllocatorStdMutex;
    utils::Arena<utils::ObjectPoolAllocator<Payload>, utils::Mutex> mPoolAllocatorUtilsMutex;
    utils::Arena<utils::ThreadSafeObjectPoolAllocator<Payload>, LockingPolicy::NoLock> mPoolAllocatorAtomic;
};

static constexpr size_t POOL_ITEM_COUNT = 4096;

Allocators::Allocators()
        : mPoolAllocatorNoLock("nolock", POOL_ITEM_COUNT * sizeof(Payload)),
          mPoolAllocatorStdMutex("std::mutex", POOL_ITEM_COUNT * sizeof(Payload)),
          mPoolAllocatorUtilsMutex("utils::Mutex", POOL_ITEM_COUNT * sizeof(Payload)),
          mPoolAllocatorAtomic("atomic", POOL_ITEM_COUNT * sizeof(Payload)) {
}

Allocators::~Allocators() = default;

BENCHMARK_F(Allocators, poolAllocator_nolock)(benchmark::State& state) {
    auto& pool = mPoolAllocatorNoLock;
    PerformanceCounters pc(state);
    for (auto _ : state) {
        Payload* p = pool.alloc<Payload>(1);
        pool.free(p);
    }
}

BENCHMARK_DEFINE_F(Allocators, poolAllocator_std_mutex)(benchmark::State& state) {
    auto& pool = mPoolAllocatorStdMutex;
    PerformanceCounters pc(state);
    for (auto _ : state) {
        Payload* p = pool.alloc<Payload>(1);
        pool.free(p);
    }
}

BENCHMARK_DEFINE_F(Allocators, poolAllocator_utils_mutex)(benchmark::State& state) {
    auto& pool = mPoolAllocatorUtilsMutex;
    PerformanceCounters pc(state);
    for (auto _ : state) {
        Payload* p = pool.alloc<Payload>(1);
        pool.free(p);
    }
}

BENCHMARK_DEFINE_F(Allocators, poolAllocator_atomic)(benchmark::State& state) {
    auto& pool = mPoolAllocatorAtomic;
    PerformanceCounters pc(state);
    for (auto _ : state) {
        Payload* p = pool.alloc<Payload>(1);
        pool.free(p);
    }
}

BENCHMARK_REGISTER_F(Allocators, poolAllocator_std_mutex)
        ->ThreadRange(1, 4)
        ->Threads(benchmark::CPUInfo::Get().num_cpus * 2);

BENCHMARK_REGISTER_F(Allocators, poolAllocator_utils_mutex)
        ->ThreadRange(1, 4)
        ->Threads(benchmark::CPUInfo::Get().num_cpus * 2);

BENCHMARK_REGISTER_F(Allocators, poolAllocator_atomic)
        ->ThreadRange(1, 4)
        ->Threads(benchmark::CPUInfo::Get().num_cpus * 2);
