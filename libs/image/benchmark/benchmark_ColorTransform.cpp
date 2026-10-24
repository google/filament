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

#include <image/ColorTransform.h>
#include <image/LinearImage.h>

#include <utils/compiler.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>

using namespace image;
using namespace filament::math;

namespace {

template<typename T>
std::unique_ptr<uint8_t[]> generateTestImage(size_t w, size_t h, size_t channels) {
    size_t const count = w * h * channels;
    auto data = std::make_unique<uint8_t[]>(count * sizeof(T));
    T* dst = reinterpret_cast<T*>(data.get());
    uint32_t state = 0x12345678u;
    for (size_t i = 0; i < count; ++i) {
        state = state * 1664525u + 1013904223u;
        if constexpr (sizeof(T) == 1) {
            dst[i] = static_cast<T>(state >> 24);
        } else {
            dst[i] = static_cast<T>(state >> 16);
        }
    }
    return data;
}

inline uint16_t bswap16(uint16_t v) noexcept {
    return static_cast<uint16_t>((v >> 8) | (v << 8));
}

} // anonymous namespace

// -------------------------------------------------------------------------------------------------
// 4-argument toLinear<T> and toLinearWithAlpha<T>
// -------------------------------------------------------------------------------------------------

template<typename T>
static void BM_toLinear(benchmark::State& state) {
    size_t const w = static_cast<size_t>(state.range(0));
    size_t const h = static_cast<size_t>(state.range(0));
    size_t const bpr = w * 3 * sizeof(T);
    auto src = generateTestImage<T>(w, h, 3);

    {
        PerformanceCounters pc(state);
        for (auto _ UTILS_UNUSED : state) {
            LinearImage img = toLinear<T>(w, h, bpr, src.get());
            benchmark::DoNotOptimize(img);
        }
    }
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(w * h));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(h * bpr));
}

template<typename T>
static void BM_toLinearWithAlpha(benchmark::State& state) {
    size_t const w = static_cast<size_t>(state.range(0));
    size_t const h = static_cast<size_t>(state.range(0));
    size_t const bpr = w * 4 * sizeof(T);
    auto src = generateTestImage<T>(w, h, 4);

    {
        PerformanceCounters pc(state);
        for (auto _ UTILS_UNUSED : state) {
            LinearImage img = toLinearWithAlpha<T>(w, h, bpr, src.get());
            benchmark::DoNotOptimize(img);
        }
    }
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(w * h));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(h * bpr));
}

// -------------------------------------------------------------------------------------------------
// 6-argument toLinear<T, PROCESS, TRANSFORM>
// -------------------------------------------------------------------------------------------------

template<typename T>
static void BM_toLinear_ProcIdentity_sRGB(benchmark::State& state) {
    size_t const w = static_cast<size_t>(state.range(0));
    size_t const h = static_cast<size_t>(state.range(0));
    size_t const bpr = w * 3 * sizeof(T);
    auto src = generateTestImage<T>(w, h, 3);

    {
        PerformanceCounters pc(state);
        for (auto _ UTILS_UNUSED : state) {
            LinearImage img = toLinear<T>(w, h, bpr, src,
                    [](T v) -> T { return v; },
                    sRGBToLinear<float3>);
            benchmark::DoNotOptimize(img);
        }
    }
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(w * h));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(h * bpr));
}

static void BM_toLinear_ProcBswap16_sRGB(benchmark::State& state) {
    size_t const w = static_cast<size_t>(state.range(0));
    size_t const h = static_cast<size_t>(state.range(0));
    size_t const bpr = w * 3 * sizeof(uint16_t);
    auto src = generateTestImage<uint16_t>(w, h, 3);

    {
        PerformanceCounters pc(state);
        for (auto _ UTILS_UNUSED : state) {
            LinearImage img = toLinear<uint16_t>(w, h, bpr, src,
                    [](uint16_t v) -> uint16_t { return bswap16(v); },
                    sRGBToLinear<float3>);
            benchmark::DoNotOptimize(img);
        }
    }
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(w * h));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(h * bpr));
}

static void BM_toLinear_ProcBswap16_Linear(benchmark::State& state) {
    size_t const w = static_cast<size_t>(state.range(0));
    size_t const h = static_cast<size_t>(state.range(0));
    size_t const bpr = w * 3 * sizeof(uint16_t);
    auto src = generateTestImage<uint16_t>(w, h, 3);

    {
        PerformanceCounters pc(state);
        for (auto _ UTILS_UNUSED : state) {
            LinearImage img = toLinear<uint16_t>(w, h, bpr, src,
                    [](uint16_t v) -> uint16_t { return bswap16(v); },
                    [](float3 const& c) -> float3 { return c; });
            benchmark::DoNotOptimize(img);
        }
    }
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(w * h));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(h * bpr));
}

// -------------------------------------------------------------------------------------------------
// 6-argument toLinearWithAlpha<T, PROCESS, TRANSFORM>
// -------------------------------------------------------------------------------------------------

template<typename T>
static void BM_toLinearWithAlpha_ProcIdentity_sRGB(benchmark::State& state) {
    size_t const w = static_cast<size_t>(state.range(0));
    size_t const h = static_cast<size_t>(state.range(0));
    size_t const bpr = w * 4 * sizeof(T);
    auto src = generateTestImage<T>(w, h, 4);

    {
        PerformanceCounters pc(state);
        for (auto _ UTILS_UNUSED : state) {
            LinearImage img = toLinearWithAlpha<T>(w, h, bpr, src,
                    [](T v) -> T { return v; },
                    sRGBToLinear<float4>);
            benchmark::DoNotOptimize(img);
        }
    }
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(w * h));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(h * bpr));
}

static void BM_toLinearWithAlpha_ProcBswap16_sRGB(benchmark::State& state) {
    size_t const w = static_cast<size_t>(state.range(0));
    size_t const h = static_cast<size_t>(state.range(0));
    size_t const bpr = w * 4 * sizeof(uint16_t);
    auto src = generateTestImage<uint16_t>(w, h, 4);

    {
        PerformanceCounters pc(state);
        for (auto _ UTILS_UNUSED : state) {
            LinearImage img = toLinearWithAlpha<uint16_t>(w, h, bpr, src,
                    [](uint16_t v) -> uint16_t { return bswap16(v); },
                    sRGBToLinear<float4>);
            benchmark::DoNotOptimize(img);
        }
    }
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(w * h));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(h * bpr));
}

static void BM_toLinearWithAlpha_ProcBswap16_Linear(benchmark::State& state) {
    size_t const w = static_cast<size_t>(state.range(0));
    size_t const h = static_cast<size_t>(state.range(0));
    size_t const bpr = w * 4 * sizeof(uint16_t);
    auto src = generateTestImage<uint16_t>(w, h, 4);

    {
        PerformanceCounters pc(state);
        for (auto _ UTILS_UNUSED : state) {
            LinearImage img = toLinearWithAlpha<uint16_t>(w, h, bpr, src,
                    [](uint16_t v) -> uint16_t { return bswap16(v); },
                    [](float4 const& c) -> float4 { return c; });
            benchmark::DoNotOptimize(img);
        }
    }
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(w * h));
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(h * bpr));
}

BENCHMARK_TEMPLATE(BM_toLinear, uint8_t)->RangeMultiplier(4)->Range(64, 1024);
BENCHMARK_TEMPLATE(BM_toLinear, uint16_t)->RangeMultiplier(4)->Range(64, 1024);
BENCHMARK_TEMPLATE(BM_toLinearWithAlpha, uint8_t)->RangeMultiplier(4)->Range(64, 1024);
BENCHMARK_TEMPLATE(BM_toLinearWithAlpha, uint16_t)->RangeMultiplier(4)->Range(64, 1024);

BENCHMARK_TEMPLATE(BM_toLinear_ProcIdentity_sRGB, uint8_t)->RangeMultiplier(4)->Range(64, 1024);
BENCHMARK_TEMPLATE(BM_toLinear_ProcIdentity_sRGB, uint16_t)->RangeMultiplier(4)->Range(64, 1024);
BENCHMARK(BM_toLinear_ProcBswap16_sRGB)->RangeMultiplier(4)->Range(64, 1024);
BENCHMARK(BM_toLinear_ProcBswap16_Linear)->RangeMultiplier(4)->Range(64, 1024);

BENCHMARK_TEMPLATE(BM_toLinearWithAlpha_ProcIdentity_sRGB, uint8_t)->RangeMultiplier(4)->Range(64, 1024);
BENCHMARK_TEMPLATE(BM_toLinearWithAlpha_ProcIdentity_sRGB, uint16_t)->RangeMultiplier(4)->Range(64, 1024);
BENCHMARK(BM_toLinearWithAlpha_ProcBswap16_sRGB)->RangeMultiplier(4)->Range(64, 1024);
BENCHMARK(BM_toLinearWithAlpha_ProcBswap16_Linear)->RangeMultiplier(4)->Range(64, 1024);

