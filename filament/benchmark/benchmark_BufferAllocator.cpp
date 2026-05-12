/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "../src/details/BufferAllocator.h"

#include <array>
#include <cstddef>
#include <utility>

using namespace filament;

namespace {

using AllocationId = BufferAllocator::AllocationId;
using allocation_size_t = BufferAllocator::allocation_size_t;

static constexpr allocation_size_t SLOT_SIZE = 256;
static constexpr allocation_size_t SLOT_COUNT = 8192;
static constexpr allocation_size_t TOTAL_SIZE = SLOT_SIZE * SLOT_COUNT;
static constexpr size_t BATCH_SIZE = 512;

void retireValid(BufferAllocator& allocator, AllocationId id) {
    if (BufferAllocator::isValid(id)) {
        allocator.retire(id);
    }
}

} // anonymous namespace

class BufferAllocatorFixture : public benchmark::Fixture {};

BENCHMARK_F(BufferAllocatorFixture, allocateRetire)(benchmark::State& state) {
    auto const size = static_cast<allocation_size_t>(state.range(0));
    BufferAllocator allocator(TOTAL_SIZE, SLOT_SIZE);

    PerformanceCounters pc(state);
    for (auto _ : state) {
        auto const [id, offset] = allocator.allocate(size);
        benchmark::DoNotOptimize(id);
        benchmark::DoNotOptimize(offset);
        retireValid(allocator, id);
    }
    benchmark::ClobberMemory();
    pc.stop();
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(BufferAllocatorFixture, allocateGpuRetireRelease)(benchmark::State& state) {
    auto const size = static_cast<allocation_size_t>(state.range(0));
    BufferAllocator allocator(TOTAL_SIZE, SLOT_SIZE);

    PerformanceCounters pc(state);
    for (auto _ : state) {
        auto const [id, offset] = allocator.allocate(size);
        benchmark::DoNotOptimize(offset);
        allocator.acquireGpu(id);
        allocator.retire(id);
        allocator.releaseGpu(id);
    }
    benchmark::ClobberMemory();
    pc.stop();
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(BufferAllocatorFixture, allocateBatchAndRetire)(benchmark::State& state) {
    auto const size = static_cast<allocation_size_t>(state.range(0));
    BufferAllocator allocator(TOTAL_SIZE, SLOT_SIZE);
    std::array<AllocationId, BATCH_SIZE> ids{};

    PerformanceCounters pc(state);
    for (auto _ : state) {
        for (AllocationId& id : ids) {
            auto const [newId, offset] = allocator.allocate(size);
            benchmark::DoNotOptimize(offset);
            id = newId;
        }
        for (auto iter = ids.rbegin(); iter != ids.rend(); ++iter) {
            retireValid(allocator, *iter);
        }
    }
    benchmark::ClobberMemory();
    pc.stop();
    state.SetItemsProcessed(state.iterations() * BATCH_SIZE);
}

BENCHMARK_F(BufferAllocatorFixture, allocateFragmented)(benchmark::State& state) {
    auto const size = static_cast<allocation_size_t>(state.range(0));
    BufferAllocator allocator(TOTAL_SIZE, SLOT_SIZE);
    std::array<AllocationId, BATCH_SIZE> ids{};

    for (AllocationId& id : ids) {
        id = allocator.allocate(SLOT_SIZE).first;
    }
    for (size_t i = 0; i < ids.size(); i += 2) {
        allocator.retire(ids[i]);
    }

    PerformanceCounters pc(state);
    for (auto _ : state) {
        auto const [id, offset] = allocator.allocate(size);
        benchmark::DoNotOptimize(offset);
        retireValid(allocator, id);
    }
    benchmark::ClobberMemory();
    pc.stop();
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(BufferAllocatorFixture, reset)(benchmark::State& state) {
    BufferAllocator allocator(TOTAL_SIZE, SLOT_SIZE);

    PerformanceCounters pc(state);
    for (auto _ : state) {
        allocator.reset(TOTAL_SIZE);
    }
    benchmark::ClobberMemory();
    pc.stop();
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_REGISTER_F(BufferAllocatorFixture, allocateRetire)
        ->RangeMultiplier(2)
        ->Range(16, 4096);

BENCHMARK_REGISTER_F(BufferAllocatorFixture, allocateGpuRetireRelease)
        ->RangeMultiplier(2)
        ->Range(16, 4096);

BENCHMARK_REGISTER_F(BufferAllocatorFixture, allocateBatchAndRetire)
        ->RangeMultiplier(2)
        ->Range(16, 4096);

BENCHMARK_REGISTER_F(BufferAllocatorFixture, allocateFragmented)
        ->RangeMultiplier(2)
        ->Range(16, SLOT_SIZE);

BENCHMARK_REGISTER_F(BufferAllocatorFixture, reset);
