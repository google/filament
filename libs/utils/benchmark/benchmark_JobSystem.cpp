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

#include <utils/JobSystem.h>
#include <utils/compiler.h>

#include <benchmark/benchmark.h>

using namespace utils;


static void emptyJob(void*, JobSystem&, JobSystem::Job*) {
}

static void BM_JobSystem(benchmark::State& state) {
    JobSystem js;
    js.adopt();

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            js.runAndWait(js.create(nullptr, &emptyJob));
        }
    }
    state.SetItemsProcessed((int64_t)state.iterations());

    js.emancipate();
}

static void BM_JobSystemAsChildren4k(benchmark::State& state) {
    JobSystem js;
    js.adopt();

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            auto root = js.create(nullptr, &emptyJob);
            for (size_t i = 0; i < 4095; i++) {
                js.run(js.create(root, &emptyJob));
            }
            js.runAndWait(root);
        }
    }
    state.SetItemsProcessed((int64_t)state.iterations() * 4096);

    js.emancipate();
}

static void BM_JobSystemParallelFor(benchmark::State& state) {
    JobSystem js;
    js.adopt();

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            auto job = jobs::parallel_for(js, nullptr, 0, 4096,
                    [](uint32_t start, uint32_t count) { }, jobs::CountSplitter<1>());
            js.runAndWait(job);
        }
    }
    state.SetItemsProcessed((int64_t)state.iterations() * 4096);

    js.emancipate();
}


BENCHMARK(BM_JobSystem);
BENCHMARK(BM_JobSystemAsChildren4k);
BENCHMARK(BM_JobSystemParallelFor);
