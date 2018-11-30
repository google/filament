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

#include <math/fast.h>

#include <cmath>

using namespace math;

template <typename T>
static void BM_trig(benchmark::State& state) {
    T f;
    state.SetLabel(T::label());
    std::vector<float> data(1024);
    for (size_t i = 0; i < data.size(); i++) {
        data[i] = float((float(i) / data.size()) * M_2_PI - M_PI);
    }

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
#pragma unroll(16)
            for (float v : data) {
                benchmark::DoNotOptimize(f(v));
            }
        }
        state.SetItemsProcessed(state.iterations() * data.size());
    }
}

template <typename T>
static void BM_func(benchmark::State& state) {
    T f;
    state.SetLabel(T::label());
    std::vector<float> data(1024);
    std::vector<float> res(1024);
    for (size_t i = 0; i < data.size(); i++) {
        data[i] = (float(i + 1) / (data.size() + 1)) * 1024;
    }

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
#pragma unroll(16)
            for (size_t i = 0, c = data.size(); i < c; i++) {
                res[i] = f(data[i]);
                benchmark::DoNotOptimize(res[i]);
            }
            benchmark::ClobberMemory();
        }
        state.SetItemsProcessed(state.iterations() * data.size());
    }
}

template <typename T>
static void BM_func_smp(benchmark::State& state) {
    T f;
    state.SetLabel(T::label());
    std::vector<float> data(1024);
    std::vector<float> res(1024);
    for (size_t i = 0; i < data.size(); i++) {
        data[i] = (float(i + 1) / (data.size() + 1)) * 1024;
    }

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
#pragma loop vectorize_width(4)
            for (size_t i = 0, c = data.size(); i < c; i++) {
                res[i] = f(data[i]);
            }
            benchmark::ClobberMemory();
            benchmark::DoNotOptimize(res);
        }
        state.SetItemsProcessed(state.iterations() * data.size());
    }
}

struct StdCos {
    float operator()(float v) { return std::cos(v); }
    static const char* label() { return "std::cos"; }
};
struct FastCos {
    float operator()(float v) { return fast::cos(v); }
    static const char* label() { return "fast::cos"; }
};
struct StdLog2 {
    float operator()(float v) { return std::log2(v); }
    static const char* label() { return "std::log2"; }
};
struct FastLog2 {
    float operator()(float v) { return fast::log2(v); }
    static const char* label() { return "fast::log2"; }
};
struct Rcp {
    float operator()(float v) { return 1.0f/v; }
    static const char* label() { return "1/x"; }
};
struct StdISqrt {
    float operator()(float v) { return 1.0f / std::sqrt(v); }
    static const char* label() { return "1/std::sqrt"; }
};
struct FastISqrt {
    float operator()(float v) { return fast::isqrt(v); }
    static const char* label() { return "fast::isqrt"; }
};
struct StdPow2dot2 {
    float operator()(float v) { return std::pow(v, 2.2f); }
    static const char* label() { return "std::pow(x, 2.2f)"; }
};
struct StdExp2dot2Log {
    float operator()(float v) { return std::exp(2.2f * std::log(v)); }
    static const char* label() { return "std::exp(2.2f * std::log(x))"; }
};
struct FastPow2dot2 {
    float operator()(float v) { return fast::pow2dot2(v); }
    static const char* label() { return "fast::pow2dot2"; }
};


BENCHMARK_TEMPLATE(BM_trig, StdCos);
BENCHMARK_TEMPLATE(BM_trig, FastCos);

BENCHMARK_TEMPLATE(BM_func, StdLog2);
BENCHMARK_TEMPLATE(BM_func, FastLog2);
BENCHMARK_TEMPLATE(BM_func_smp, FastLog2);
BENCHMARK_TEMPLATE(BM_func, Rcp);
BENCHMARK_TEMPLATE(BM_func_smp, Rcp);
BENCHMARK_TEMPLATE(BM_func, StdISqrt);
BENCHMARK_TEMPLATE(BM_func_smp, StdISqrt);
BENCHMARK_TEMPLATE(BM_func, FastISqrt);
BENCHMARK_TEMPLATE(BM_func_smp, FastISqrt);
BENCHMARK_TEMPLATE(BM_func, StdPow2dot2);
BENCHMARK_TEMPLATE(BM_func, StdExp2dot2Log);
BENCHMARK_TEMPLATE(BM_func_smp, StdExp2dot2Log);
BENCHMARK_TEMPLATE(BM_func, FastPow2dot2);
BENCHMARK_TEMPLATE(BM_func_smp, FastPow2dot2);
