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

#include <utils/compiler.h>

#include <benchmark/benchmark.h>

#include <random>
#include <vector>

using namespace utils;


class BinarySearch : public benchmark::Fixture {
public:
     BinarySearch();
    ~BinarySearch() override;

protected:
    using value_type = uint64_t;
    std::vector<value_type> data;
    std::default_random_engine gen{123};
    std::uniform_int_distribution<value_type> nd;

    std::vector<value_type> prepareItems(size_t size);
};

BinarySearch::BinarySearch() {
    data.resize(2<<20);
    std::generate(data.begin(), data.end(), [&](){ return nd(gen); });
    std::sort(data.begin(), data.end());
}

BinarySearch::~BinarySearch() = default;

UTILS_NOINLINE
std::vector<BinarySearch::value_type> BinarySearch::prepareItems(size_t size) {
    auto first = data.begin();
    auto last = data.begin() + size;
    std::vector<value_type> indices(last - first);
    std::copy(first, last, indices.begin());
    std::default_random_engine gen{123};
    std::shuffle(indices.begin(), indices.end(), gen);
    return indices;
}

BENCHMARK_DEFINE_F(BinarySearch, linearSearch)(benchmark::State& state) {
    auto first = data.begin();
    auto last = data.begin() + state.range(0);
    std::vector<value_type> indices = prepareItems(state.range(0));
    value_type const* ip = indices.data();
    size_t i = 0;

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            auto item = ip[i++ % state.range(0)];
            auto const& pos = std::find(first, last, item);
            benchmark::DoNotOptimize(pos);
        }
    }
}

BENCHMARK_DEFINE_F(BinarySearch, stdLowerBound)(benchmark::State& state) {
    auto first = data.begin();
    auto last = data.begin() + state.range(0);
    std::vector<value_type> indices = prepareItems(state.range(0));
    value_type const* ip = indices.data();
    size_t i = 0;

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            auto item = ip[i++ % state.range(0)];
            auto const& pos = std::lower_bound(first, last, item);
            benchmark::DoNotOptimize(pos);
        }
    }
}

BENCHMARK_REGISTER_F(BinarySearch, linearSearch)->Range(2, 1<<20);
BENCHMARK_REGISTER_F(BinarySearch, stdLowerBound)->Range(2, 1<<20);
