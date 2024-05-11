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
#include <math/half.h>

#include <cmath>

using namespace filament::math;


struct Scalar{};
struct Vector{};


template <typename A, typename T>
static void BM_trig(benchmark::State& state) noexcept {
    T f;
    state.SetLabel(T::label());
    std::vector<float> data(1024);
    std::vector<typename T::result_type> res(1024);
    for (size_t i = 0; i < data.size(); i++) {
        data[i] = float((float(i) / data.size()) * F_2_PI - F_PI);
    }

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            for (size_t i = 0, c = data.size(); i < c; i++) {
                res[i] = f(data[i]);
            }
            benchmark::ClobberMemory();
            benchmark::DoNotOptimize(res);
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * data.size());
    }
}

UTILS_NOINLINE
static void init(std::vector<float>& v) noexcept {
    for (size_t i = 0; i < v.size(); i++) {
        v[i] = (float(i + 1) / (v.size() + 1)) * 1024;
    }
}

template <typename A, typename T>
static void BM_func(benchmark::State& state) noexcept {
    T f;
    state.SetLabel(T::label());
    std::vector<typename T::result_type> res(1024);
    std::vector<float> data(1024);
    init(data);

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            for (size_t i = 0, c = data.size(); i < c; i++) {
                res[i] = f(data[i]);
            }
            benchmark::ClobberMemory();
            benchmark::DoNotOptimize(res);
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * data.size());
    }
}

struct StdCos {
    using result_type = float;
    float operator()(float v) { return std::cos(v); }
    static const char* label() { return "std::cos"; }
};
struct FastCos {
    using result_type = float;
    float operator()(float v) { return fast::cos(v); }
    static const char* label() { return "fast::cos"; }
};
struct StdLog2 {
    using result_type = float;
    float operator()(float v) { return std::log2(v); }
    static const char* label() { return "std::log2"; }
};
struct FastLog2 {
    using result_type = float;
    float operator()(float v) { return fast::log2(v); }
    static const char* label() { return "fast::log2"; }
};
struct Rcp {
    using result_type = float;
    float operator()(float v) { return 1.0f/v; }
    static const char* label() { return "1/x"; }
};
struct StdISqrt {
    using result_type = float;
    float operator()(float v) { return 1.0f / std::sqrt(v); }
    static const char* label() { return "1/std::sqrt"; }
};
struct FastISqrt {
    using result_type = float;
    float operator()(float v) { return fast::isqrt(v); }
    static const char* label() { return "fast::isqrt"; }
};
struct StdPow2dot2 {
    using result_type = float;
    float operator()(float v) { return std::pow(v, 2.2f); }
    static const char* label() { return "std::pow(x, 2.2f)"; }
};
struct StdExp2dot2Log {
    using result_type = float;
    float operator()(float v) { return std::exp(2.2f * std::log(v)); }
    static const char* label() { return "std::exp(2.2f * std::log(x))"; }
};

struct Float16 {
    using result_type = half;
    half operator()(float v) { return half(v); }
    static const char* label() { return "half"; }
};


BENCHMARK_TEMPLATE(BM_trig, Scalar, StdCos);
BENCHMARK_TEMPLATE(BM_trig, Scalar, FastCos);
BENCHMARK_TEMPLATE(BM_trig, Vector, FastCos);

BENCHMARK_TEMPLATE(BM_func, Scalar, StdLog2);
BENCHMARK_TEMPLATE(BM_func, Scalar, FastLog2);
BENCHMARK_TEMPLATE(BM_func, Vector, FastLog2);

BENCHMARK_TEMPLATE(BM_func, Scalar, Rcp);
BENCHMARK_TEMPLATE(BM_func, Vector, Rcp);

BENCHMARK_TEMPLATE(BM_func, Scalar, StdISqrt);
BENCHMARK_TEMPLATE(BM_func, Vector, StdISqrt);
BENCHMARK_TEMPLATE(BM_func, Scalar, FastISqrt);
BENCHMARK_TEMPLATE(BM_func, Vector, FastISqrt);

BENCHMARK_TEMPLATE(BM_func, Scalar, StdPow2dot2);
BENCHMARK_TEMPLATE(BM_func, Scalar, StdExp2dot2Log);

BENCHMARK_TEMPLATE(BM_func, Scalar, Float16);
