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

#include <utils/compiler.h>
#include <utils/PagedArenaBitset.h>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

using namespace utils;

// this is the batch used for in-place benchmarks. It needs to be high enough to mask the overhead of the
// bitset copy (needed for the benchmark) and low-enough that we don't exhaust the L3 cache.
const size_t kInPlaceBatchSize = 100;

template<typename T>
T clone_helper(const T& val) {
    return val.clone();
}

template<typename Policy>
void BM_bitset_Insert(benchmark::State& state) {
    Policy set;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 100000};
    std::uniform_int_distribution<uint32_t> ndGen{0, 0};
    std::vector<uint32_t> bits(count);
    size_t idx = 0;
    while (idx < count) {
        uint32_t const base = ndBase(gen);
        uint32_t const g = ndGen(gen);
        size_t const batchSize = std::min<size_t>(count - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            bits[idx++] = (g << 17) | ((base + j) & 0x1FFFF);
        }
    }

    for (auto _ : state) {
        for (auto b : bits) {
            set.add(b);
        }
        state.PauseTiming();
        set.clear();
        state.ResumeTiming();
    }
}

template<typename Policy>
void BM_bitset_Intersect(benchmark::State& state) {
    Policy set1;
    Policy set2;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    size_t idx = 0;
    while (idx < overlapCount) {
        uint32_t const base = ndBase(gen);
        size_t const batchSize = std::min<size_t>(overlapCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
            set2.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 50000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 100000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set2.add(base + j);
        }
        idx += batchSize;
    }

    for (auto _ : state) {
        Policy temp = Policy::intersect(set1, set2);
        benchmark::DoNotOptimize(temp);
    }
}

template<typename Policy>
void BM_bitset_Difference(benchmark::State& state) {
    Policy set1;
    Policy set2;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    size_t idx = 0;
    while (idx < overlapCount) {
        uint32_t const base = ndBase(gen);
        size_t const batchSize = std::min<size_t>(overlapCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
            set2.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 50000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 100000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set2.add(base + j);
        }
        idx += batchSize;
    }

    for (auto _ : state) {
        Policy temp = Policy::difference(set1, set2);
        benchmark::DoNotOptimize(temp);
    }
}

template<typename Policy>
void BM_bitset_IntersectInPlace(benchmark::State& state) {
    Policy set1;
    Policy set2;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    size_t idx = 0;
    while (idx < overlapCount) {
        uint32_t const base = ndBase(gen);
        size_t const batchSize = std::min<size_t>(overlapCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
            set2.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 50000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 100000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set2.add(base + j);
        }
        idx += batchSize;
    }

    std::vector<Policy> temps(kInPlaceBatchSize);
    // Pre-allocate once outside the loop to bypass Scudo entirely
    for (auto& t : temps) {
        t.copyFrom(set1);
    }

    for (auto _ : state) {
        state.PauseTiming();
        for (auto& t : temps) {
            t.copyFrom(set1);
        }
        state.ResumeTiming();

        for (auto& t : temps) {
            benchmark::DoNotOptimize(t);
            t.intersect(set2);
            benchmark::ClobberMemory();
        }
    }
    state.SetItemsProcessed(state.iterations() * kInPlaceBatchSize);
}

template<typename Policy>
void BM_bitset_IntersectFragmented(benchmark::State& state) {
    size_t const numPages = state.range(0);
    size_t const totalBits = 4096;
    size_t const bitsPerPage = totalBits / numPages;
    size_t const stride = 1U << 17;

    Policy set1;
    Policy set2;

    for (size_t i = 0; i < numPages; ++i) {
        uint32_t const regionStart = i * stride;
        for (size_t j = 0; j < bitsPerPage; ++j) {
            set1.add(regionStart + j);
        }
        set2.add(regionStart);
    }

    std::vector<Policy> temps(kInPlaceBatchSize);
    // Pre-allocate once outside the loop to bypass Scudo entirely
    for (auto& t : temps) {
        t.copyFrom(set1);
    }

    for (auto _ : state) {
        state.PauseTiming();
        for (auto& t : temps) {
            t.copyFrom(set1);
        }
        state.ResumeTiming();

        for (auto& t : temps) {
            benchmark::DoNotOptimize(t);
            t.intersect(set2);
            benchmark::ClobberMemory();
        }
    }
    state.SetItemsProcessed(state.iterations() * kInPlaceBatchSize);
}

template<typename Policy>
void BM_bitset_DifferenceInPlace(benchmark::State& state) {
    Policy set1;
    Policy set2;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    size_t idx = 0;
    while (idx < overlapCount) {
        uint32_t const base = ndBase(gen);
        size_t const batchSize = std::min<size_t>(overlapCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
            set2.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 50000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 100000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set2.add(base + j);
        }
        idx += batchSize;
    }

    std::vector<Policy> temps(kInPlaceBatchSize);
    // Pre-allocate once outside the loop to bypass Scudo entirely
    for (auto& t : temps) {
        t.copyFrom(set1);
    }

    for (auto _ : state) {
        state.PauseTiming();
        for (auto& t : temps) {
            t.copyFrom(set1);
        }
        state.ResumeTiming();

        for (auto& t : temps) {
            benchmark::DoNotOptimize(t);
            t.difference(set2);
            benchmark::ClobberMemory();
        }
    }
    state.SetItemsProcessed(state.iterations() * kInPlaceBatchSize);
}

template<typename Policy>
void BM_bitset_IntersectMulti2(benchmark::State& state) {
    Policy set1;
    Policy set2;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    uint32_t const base = 1000;

    // Overlap
    for (size_t j = 0; j < overlapCount; ++j) {
        set1.add(base + j);
        set2.add(base + j);
    }

    size_t idx = 0;
    while (idx < disjointCount) {
        uint32_t const base1 = ndBase(gen) + 50000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base1 + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base2 = ndBase(gen) + 100000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set2.add(base2 + j);
        }
        idx += batchSize;
    }

    for (auto _ : state) {
        Policy temp = Policy::intersect(set1, set2);
        benchmark::DoNotOptimize(temp);
    }
}

template<typename Policy>
void BM_bitset_IntersectMulti6(benchmark::State& state) {
    std::array<Policy, 6> sets;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    uint32_t const base = 1000;

    for (size_t i = 0; i < 6; ++i) {
        // Overlap
        for (size_t j = 0; j < overlapCount; ++j) {
            sets[i].add(base + j);
        }

        // Disjoint
        size_t idx = 0;
        while (idx < disjointCount) {
            uint32_t const baseI = ndBase(gen) + 50000 + i * 50000;
            size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
            for (size_t j = 0; j < batchSize; ++j) {
                sets[i].add(baseI + j);
            }
            idx += batchSize;
        }
    }

    for (auto _ : state) {
        Policy temp = Policy::intersect(sets[0], sets[1], sets[2], sets[3], sets[4], sets[5]);
        benchmark::DoNotOptimize(temp);
    }
}

template<typename Policy>
void BM_bitset_IntersectChained6(benchmark::State& state) {
    std::array<Policy, 6> sets;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    uint32_t const base = 1000;

    for (size_t i = 0; i < 6; ++i) {
        // Overlap
        for (size_t j = 0; j < overlapCount; ++j) {
            sets[i].add(base + j);
        }

        // Disjoint
        size_t idx = 0;
        while (idx < disjointCount) {
            uint32_t const baseI = ndBase(gen) + 50000 + i * 50000;
            size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
            for (size_t j = 0; j < batchSize; ++j) {
                sets[i].add(baseI + j);
            }
            idx += batchSize;
        }
    }

    for (auto _ : state) {
        Policy r = Policy::intersect(sets[0], sets[1]);
        r.intersect(sets[2]);
        r.intersect(sets[3]);
        r.intersect(sets[4]);
        r.intersect(sets[5]);
        benchmark::DoNotOptimize(r);
    }
}

template<typename Policy>
void setupFilterBenchmark(std::array<Policy, 6>& sets) {
    std::default_random_engine gen{123};

    // 1. Sparse Filter (sets[0])
    // 100 random bits spread across 27-bit domain
    std::uniform_int_distribution<uint32_t> dist27{0, (1U << 27) - 1};
    for (int i = 0; i < 100; ++i) {
        sets[0].add(dist27(gen));
    }

    // 2. Dense Noise (sets[1] to sets[5])
    // 70% density by filling random pages!
    std::uniform_real_distribution<double> distProb{0.0, 1.0};
    for (size_t i = 1; i < 6; ++i) {
        for (uint32_t j = 0; j < (1U << 27); j += 4096) {
            if (distProb(gen) < 0.7) {
                for (uint32_t k = 0; k < 4096; ++k) {
                    sets[i].add(j + k);
                }
            }
        }
    }

    // Ensure they have at least some bits in common!
    std::vector<uint32_t> filterBits;
    sets[0].extractTo(filterBits);
    for (size_t i = 0; i < std::min<size_t>(10, filterBits.size()); ++i) {
        uint32_t bit = filterBits[i];
        for (size_t j = 1; j < 6; ++j) {
            sets[j].add(bit);
        }
    }
}

template<typename Policy>
void BM_bitset_IntersectMulti6_Filter(benchmark::State& state) {
    std::array<Policy, 6> sets;
    setupFilterBenchmark(sets);

    for (auto _ : state) {
        Policy temp = Policy::intersect(sets[0], sets[1], sets[2], sets[3], sets[4], sets[5]);
        benchmark::DoNotOptimize(temp);
    }
}

template<typename Policy>
void BM_bitset_IntersectChained6_Filter_Best(benchmark::State& state) {
    std::array<Policy, 6> sets;
    setupFilterBenchmark(sets);

    for (auto _ : state) {
        Policy r = Policy::intersect(sets[0], sets[1]);
        r.intersect(sets[2]);
        r.intersect(sets[3]);
        r.intersect(sets[4]);
        r.intersect(sets[5]);
        benchmark::DoNotOptimize(r);
    }
}

template<typename Policy>
void BM_bitset_IntersectChained6_Filter_Worst(benchmark::State& state) {
    std::array<Policy, 6> sets;
    setupFilterBenchmark(sets);

    for (auto _ : state) {
        Policy r = Policy::intersect(sets[1], sets[2]);
        r.intersect(sets[3]);
        r.intersect(sets[4]);
        r.intersect(sets[5]);
        r.intersect(sets[0]);
        benchmark::DoNotOptimize(r);
    }
}

template<typename Policy>
void BM_bitset_Iteration(benchmark::State& state) {
    Policy set;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 100000};
    std::uniform_int_distribution<uint32_t> ndGen{0, 0};
    size_t idx = 0;
    while (idx < count) {
        uint32_t const base = ndBase(gen);
        uint32_t const g = ndGen(gen);
        size_t const batchSize = std::min<size_t>(count - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set.add((g << 17) | ((base + j) & 0x1FFFF));
        }
        idx += batchSize;
    }

    for (auto _ : state) {
        size_t sum = 0;
        set.forEachSetBit([&sum](uint32_t bit) {
            sum += bit;
        });
        benchmark::DoNotOptimize(sum);
    }
}

template<>
void BM_bitset_Iteration<PagedArenaBitset>(benchmark::State& state) {
    PagedArenaBitset set;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 100000};
    std::uniform_int_distribution<uint32_t> ndGen{0, 0};
    size_t idx = 0;
    while (idx < count) {
        uint32_t const base = ndBase(gen);
        uint32_t const g = ndGen(gen);
        size_t const batchSize = std::min<size_t>(count - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set.add((g << 17) | ((base + j) & 0x1FFFF));
        }
        idx += batchSize;
    }

    std::vector<uint32_t> buffer;
    for (auto _ : state) {
        set.extractTo(buffer);
        size_t sum = 0;
        for (uint32_t const bit : buffer) {
            sum += bit;
        }
        benchmark::DoNotOptimize(sum);
    }
}

template<typename Policy>
void BM_bitset_Merge(benchmark::State& state) {
    Policy set1;
    Policy set2;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    size_t idx = 0;
    while (idx < overlapCount) {
        uint32_t const base = ndBase(gen);
        size_t const batchSize = std::min<size_t>(overlapCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
            set2.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 50000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 100000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set2.add(base + j);
        }
        idx += batchSize;
    }

    for (auto _ : state) {
        Policy temp = Policy::merge(set1, set2);
        benchmark::DoNotOptimize(temp);
    }
}

template<typename Policy>
void BM_bitset_MergeInPlace(benchmark::State& state) {
    Policy set1;
    Policy set2;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    size_t idx = 0;
    while (idx < overlapCount) {
        uint32_t const base = ndBase(gen);
        size_t const batchSize = std::min<size_t>(overlapCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
            set2.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 50000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set1.add(base + j);
        }
        idx += batchSize;
    }

    idx = 0;
    while (idx < disjointCount) {
        uint32_t const base = ndBase(gen) + 100000;
        size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
        for (size_t j = 0; j < batchSize; ++j) {
            set2.add(base + j);
        }
        idx += batchSize;
    }

    for (auto _ : state) {
        state.PauseTiming();
        Policy temp = set1.clone(); // Clone to avoid accumulating!
        state.ResumeTiming();
        temp.merge(set2);
        benchmark::DoNotOptimize(temp);
    }
}

template<typename Policy>
void BM_bitset_Merge6(benchmark::State& state) {
    std::array<Policy, 6> sets;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    for (size_t i = 0; i < 6; ++i) {
        // Overlap
        for (size_t j = 0; j < overlapCount; ++j) {
            sets[i].add(1000 + j);
        }

        // Disjoint
        size_t idx = 0;
        while (idx < disjointCount) {
            uint32_t const baseI = ndBase(gen) + 50000 + i * 50000;
            size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
            for (size_t j = 0; j < batchSize; ++j) {
                sets[i].add(baseI + j);
            }
            idx += batchSize;
        }
    }

    for (auto _ : state) {
        Policy temp = Policy::merge(sets[0], sets[1], sets[2], sets[3], sets[4], sets[5]);
        benchmark::DoNotOptimize(temp);
    }
}

template<typename Policy>
void BM_bitset_IntersectSize6(benchmark::State& state) {
    std::array<Policy, 6> sets;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    for (size_t i = 0; i < 6; ++i) {
        // Overlap
        for (size_t j = 0; j < overlapCount; ++j) {
            sets[i].add(1000 + j);
        }

        // Disjoint
        size_t idx = 0;
        while (idx < disjointCount) {
            uint32_t const baseI = ndBase(gen) + 50000 + i * 50000;
            size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
            for (size_t j = 0; j < batchSize; ++j) {
                sets[i].add(baseI + j);
            }
            idx += batchSize;
        }
    }

    for (auto _ : state) {
        uint32_t size = Policy::intersectSize(sets[0], sets[1], sets[2], sets[3], sets[4], sets[5]);
        benchmark::DoNotOptimize(size);
    }
}

template<typename Policy>
void BM_bitset_MergeSize6(benchmark::State& state) {
    std::array<Policy, 6> sets;
    size_t const count = state.range(0);
    std::default_random_engine gen{123};
    std::uniform_int_distribution<uint32_t> ndBase{0, 50000};

    size_t const overlapCount = std::max<size_t>(count / 100, 1);
    size_t const disjointCount = count - overlapCount;

    for (size_t i = 0; i < 6; ++i) {
        // Overlap
        for (size_t j = 0; j < overlapCount; ++j) {
            sets[i].add(1000 + j);
        }

        // Disjoint
        size_t idx = 0;
        while (idx < disjointCount) {
            uint32_t const baseI = ndBase(gen) + 50000 + i * 50000;
            size_t const batchSize = std::min<size_t>(disjointCount - idx, 100);
            for (size_t j = 0; j < batchSize; ++j) {
                sets[i].add(baseI + j);
            }
            idx += batchSize;
        }
    }

    for (auto _ : state) {
        uint32_t size = Policy::mergeSize(sets[0], sets[1], sets[2], sets[3], sets[4], sets[5]);
        benchmark::DoNotOptimize(size);
    }
}

// Registrations
#define REGISTER_BENCHMARKS(Type) \
    BENCHMARK_TEMPLATE(BM_bitset_Insert, Type)->Range(10, 100000); \
    BENCHMARK_TEMPLATE(BM_bitset_Intersect, Type)->Range(10, 100000); \
    BENCHMARK_TEMPLATE(BM_bitset_Difference, Type)->Range(10, 100000); \
    BENCHMARK_TEMPLATE(BM_bitset_IntersectInPlace, Type)->Range(10, 100000)->Iterations(10); \
    BENCHMARK_TEMPLATE(BM_bitset_DifferenceInPlace, Type)->Range(10, 100000)->Iterations(10); \
    BENCHMARK_TEMPLATE(BM_bitset_Iteration, Type)->Range(10, 100000);

REGISTER_BENCHMARKS(PagedArenaBitset)

BENCHMARK_TEMPLATE(BM_bitset_Merge, PagedArenaBitset)->Range(10, 100000);
BENCHMARK_TEMPLATE(BM_bitset_MergeInPlace, PagedArenaBitset)->Range(10, 100000)->Iterations(10);
BENCHMARK_TEMPLATE(BM_bitset_Merge6, PagedArenaBitset)->Range(10, 100000);
BENCHMARK_TEMPLATE(BM_bitset_IntersectSize6, PagedArenaBitset)->Range(10, 100000);
BENCHMARK_TEMPLATE(BM_bitset_MergeSize6, PagedArenaBitset)->Range(10, 100000);

BENCHMARK_TEMPLATE(BM_bitset_IntersectMulti2, PagedArenaBitset)->Range(10, 100000);
BENCHMARK_TEMPLATE(BM_bitset_IntersectMulti6, PagedArenaBitset)->Range(10, 100000);
BENCHMARK_TEMPLATE(BM_bitset_IntersectChained6, PagedArenaBitset)->Range(10, 100000);
BENCHMARK_TEMPLATE(BM_bitset_IntersectMulti6_Filter, PagedArenaBitset);
BENCHMARK_TEMPLATE(BM_bitset_IntersectChained6_Filter_Best, PagedArenaBitset);
BENCHMARK_TEMPLATE(BM_bitset_IntersectChained6_Filter_Worst, PagedArenaBitset);
BENCHMARK_TEMPLATE(BM_bitset_IntersectFragmented, PagedArenaBitset)->RangeMultiplier(2)->Range(1, 64)->Iterations(10);
;
