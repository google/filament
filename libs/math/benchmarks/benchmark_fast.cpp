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

#include <math/fast.h>
#include <math/half.h>
#include <math/mat4.h>
#include <math/scalar.h>
#include <math/vec3.h>

#include <benchmark/benchmark.h>

#include <cmath>
#include <vector>

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

template <typename T>
UTILS_NOINLINE
static void initMatrices(std::vector<details::TMat44<T>>& matrices,
        std::vector<T>& determinants) noexcept {
    using M4 = details::TMat44<T>;
    using V3 = details::TVec3<T>;
    for (size_t i = 0, c = matrices.size(); i < c; i++) {
        T const angle = T(i + 1) * T(0.05);
        M4 const t = M4::translation(V3{ T(i) * T(0.1), T(i) * T(-0.2), T(i) * T(0.3) });
        M4 const r = M4::eulerZYX(angle, angle * T(0.7), angle * T(-0.3));
        M4 const s = M4::scaling(V3{ T(1.0) + T(i % 5) * T(0.25),
                                     T(1.2) + T(i % 7) * T(0.15),
                                     T(0.8) + T(i % 3) * T(0.35) });
        matrices[i] = t * r * s;
        determinants[i] = det(matrices[i]);
    }
}

template <typename T, typename Op>
static void BM_mat4Inverse(benchmark::State& state) noexcept {
    using M4 = details::TMat44<T>;
    Op op;
    state.SetLabel(Op::label());
    std::vector<M4> data(1024);
    std::vector<T> determinants(1024);
    std::vector<M4> res(1024);
    initMatrices(data, determinants);

    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            for (size_t i = 0, c = data.size(); i < c; i++) {
                res[i] = op(data[i], determinants[i]);
            }
            benchmark::ClobberMemory();
            benchmark::DoNotOptimize(res);
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * data.size());
    }
}

template <typename T>
struct GaussJordanInverse4 {
    using M4 = details::TMat44<T>;
    M4 operator()(M4 const& m, T) const noexcept {
        return details::matrix::gaussJordanInverse(m);
    }
    static const char* label() { return "gaussJordanInverse(m)"; }
};

template <typename T>
struct FastInverse4 {
    using M4 = details::TMat44<T>;
    M4 operator()(M4 const& m, T) const noexcept {
        return inverse(m);
    }
    static const char* label() { return "inverse(m)"; }
};

template <typename T>
struct FastInverse4WithPrecomputedDet {
    using M4 = details::TMat44<T>;
    M4 operator()(M4 const& m, T d) const noexcept {
        return inverse(m, d);
    }
    static const char* label() { return "inverse(m, det)"; }
};

template <typename T>
struct DetAndGaussJordanInverse4 {
    using M4 = details::TMat44<T>;
    M4 operator()(M4 const& m, T) const noexcept {
        if (UTILS_VERY_LIKELY(det(m) != T(0))) {
            return details::matrix::gaussJordanInverse(m);
        }
        return M4{};
    }
    static const char* label() { return "det(m) + gaussJordanInverse(m)"; }
};

template <typename T>
struct DetAndFastInverse4WithDet {
    using M4 = details::TMat44<T>;
    M4 operator()(M4 const& m, T) const noexcept {
        T const d = det(m);
        if (UTILS_VERY_LIKELY(d != T(0))) {
            return inverse(m, d);
        }
        return M4{};
    }
    static const char* label() { return "det(m) + inverse(m, d)"; }
};

BENCHMARK_TEMPLATE(BM_mat4Inverse, float, GaussJordanInverse4<float>);
BENCHMARK_TEMPLATE(BM_mat4Inverse, float, FastInverse4<float>);
BENCHMARK_TEMPLATE(BM_mat4Inverse, float, FastInverse4WithPrecomputedDet<float>);
BENCHMARK_TEMPLATE(BM_mat4Inverse, float, DetAndGaussJordanInverse4<float>);
BENCHMARK_TEMPLATE(BM_mat4Inverse, float, DetAndFastInverse4WithDet<float>);

BENCHMARK_TEMPLATE(BM_mat4Inverse, double, GaussJordanInverse4<double>);
BENCHMARK_TEMPLATE(BM_mat4Inverse, double, FastInverse4<double>);
BENCHMARK_TEMPLATE(BM_mat4Inverse, double, FastInverse4WithPrecomputedDet<double>);
BENCHMARK_TEMPLATE(BM_mat4Inverse, double, DetAndGaussJordanInverse4<double>);
BENCHMARK_TEMPLATE(BM_mat4Inverse, double, DetAndFastInverse4WithDet<double>);
