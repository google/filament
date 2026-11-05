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

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <vector>

using namespace image;

namespace {

constexpr size_t WIDTH = 512;
constexpr size_t HEIGHT = 512;

template<typename T>
std::vector<T> createSourceBuffer(size_t w, size_t h, size_t channels) {
    std::vector<T> src(w * h * channels);
    for (size_t i = 0; i < src.size(); ++i) {
        src[i] = static_cast<T>((i * 4099 + 1319) & 0xffff);
    }
    return src;
}

LinearImage createSourceLinearImage(size_t w, size_t h, size_t channels) {
    LinearImage img(w, h, channels);
    float* p = img.getPixelRef(0, 0);
    const size_t total = w * h * channels;
    for (size_t i = 0; i < total; ++i) {
        p[i] = float((i * 97 + 31) % 1000) * 0.01f;
    }
    return img;
}

using TransformFn = LinearImage (*)(size_t, size_t, size_t, const uint8_t*);

template<typename T, size_t CHANNELS, TransformFn Fn>
void BM_ColorTransform(benchmark::State& state) {
    const std::vector<T> src = createSourceBuffer<T>(WIDTH, HEIGHT, CHANNELS);
    const size_t bpr = WIDTH * CHANNELS * sizeof(T);
    const uint8_t* srcBytes = reinterpret_cast<const uint8_t*>(src.data());

    PerformanceCounters pc(state);
    for (UTILS_UNUSED auto _ : state) {
        LinearImage dst = Fn(WIDTH, HEIGHT, bpr, srcBytes);
        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }
    pc.stop();
    state.SetItemsProcessed(state.iterations() * WIDTH * HEIGHT);
}

template<size_t CHANNELS>
void BM_fromLinearToRGB_10_11_11_REV(benchmark::State& state) {
    const LinearImage img = createSourceLinearImage(WIDTH, HEIGHT, CHANNELS);

    PerformanceCounters pc(state);
    for (UTILS_UNUSED auto _ : state) {
        auto dst = fromLinearToRGB_10_11_11_REV(img);
        benchmark::DoNotOptimize(dst);
        benchmark::ClobberMemory();
    }
    pc.stop();
    state.SetItemsProcessed(state.iterations() * WIDTH * HEIGHT);
}

} // anonymous namespace

BENCHMARK(BM_fromLinearToRGB_10_11_11_REV<3>)->Name("fromLinearToRGB_10_11_11_REV_RGB");
BENCHMARK(BM_fromLinearToRGB_10_11_11_REV<4>)->Name("fromLinearToRGB_10_11_11_REV_RGBA");

BENCHMARK(BM_ColorTransform<uint8_t,  4, toLinearWithAlpha<uint8_t>>)->Name("toLinearWithAlpha_u8");
BENCHMARK(BM_ColorTransform<uint16_t, 4, toLinearWithAlpha<uint16_t>>)->Name("toLinearWithAlpha_u16");

BENCHMARK(BM_ColorTransform<uint8_t,  4, asLinearWithAlpha<uint8_t>>)->Name("asLinearWithAlpha_u8");
BENCHMARK(BM_ColorTransform<uint16_t, 4, asLinearWithAlpha<uint16_t>>)->Name("asLinearWithAlpha_u16");

BENCHMARK(BM_ColorTransform<uint8_t,  3, toLinear<uint8_t>>)->Name("toLinear_u8");
BENCHMARK(BM_ColorTransform<uint16_t, 3, toLinear<uint16_t>>)->Name("toLinear_u16");

BENCHMARK(BM_ColorTransform<uint8_t,  3, asLinear<uint8_t>>)->Name("asLinear_u8");
BENCHMARK(BM_ColorTransform<uint16_t, 3, asLinear<uint16_t>>)->Name("asLinear_u16");

BENCHMARK(BM_ColorTransform<uint16_t, 4, toLinearWithAlphaNtohs<uint16_t>>)->Name("toLinearWithAlphaNtohs_u16");
BENCHMARK(BM_ColorTransform<uint16_t, 4, asLinearWithAlphaNtohs<uint16_t>>)->Name("asLinearWithAlphaNtohs_u16");

BENCHMARK(BM_ColorTransform<uint16_t, 3, toLinearNtohs<uint16_t>>)->Name("toLinearNtohs_u16");
BENCHMARK(BM_ColorTransform<uint16_t, 3, asLinearNtohs<uint16_t>>)->Name("asLinearNtohs_u16");
