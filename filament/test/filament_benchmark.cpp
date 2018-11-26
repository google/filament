/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <filament/Box.h>
#include <filament/Frustum.h>
#include "details/Culler.h"

#include <utils/Profiler.h>
#include <utils/compiler.h>
#include <math/fast.h>
#include <math/scalar.h>

#include <iostream>
#include <vector>
#include <random>

using namespace filament;
using namespace filament::details;
using namespace math;
using namespace utils;

UTILS_NOINLINE
void printResults(char const* name, size_t REPEAT, Profiler::Counters const& c) {
    std::cout << name << ":" << std::endl;
    std::cout << "instructions: " << c.getInstructions() / float(REPEAT) << std::endl;
    std::cout << "cycles:       " << c.getCpuCycles() / float(REPEAT) << std::endl;
    std::cout << "bpu misses:   " << c.getBranchMisses() / float(REPEAT) << " (" << c.getBranchMisses() << "/" << REPEAT << ")" << std::endl;
    std::cout << "CPI:          " << c.getCPI() << std::endl;
    std::cout << "" << std::endl;
}

template<typename T, size_t REPEAT = 2>
void benchmark(Profiler& p, const char* const name, T f) {
    p.start();

#pragma nounroll
    for (size_t j = 0; j < 2; j++) {
        p.reset();
#pragma nounroll
        for (size_t i = 0; i < REPEAT; i++) {
            f();
        }
    }

    p.stop();
    Profiler::Counters c = p.readCounters();
    printResults(name, REPEAT, c);
}

// ------------------------------------------------------------------------------------------------
int main() {

    std::mt19937 gen;
    std::uniform_real_distribution<float> rand(-100.0f, 100.0f);

    size_t batch = 512;
    Frustum frustum(mat4f::perspective(45.0f, 1.0f, 0.1f, 100.0f));


    std::vector<float3> boxesCenter(batch);
    std::vector<float3> boxesExtent(batch);
    std::vector<float4> spheres(batch);
    std::vector<float4> colors(batch);
    std::vector<float4> colors_results(batch);
    std::vector<half4> spheresHalf(batch);
    for (size_t i=0 ; i<batch ; i++) {
        float4& sphere = spheres[i];
        float4& color = colors[i];
        float z = std::fabs(rand(gen));
        sphere.z = -z;
        sphere.x = rand(gen, std::uniform_real_distribution<float>::param_type{-z, z});
        sphere.y = rand(gen, std::uniform_real_distribution<float>::param_type{-z, z});
        sphere.w = rand(gen, std::uniform_real_distribution<float>::param_type{0.11f, 25.0f});

        color.x = rand(gen, std::uniform_real_distribution<float>::param_type{0.0f, 1.0f});
        color.y = rand(gen, std::uniform_real_distribution<float>::param_type{0.0f, 1.0f});
        color.z = rand(gen, std::uniform_real_distribution<float>::param_type{0.0f, 1.0f});
        color.w = rand(gen, std::uniform_real_distribution<float>::param_type{0.0f, 1.0f});

        boxesCenter[i] = sphere.xyz;
        boxesExtent[i] = {
                rand(gen, std::uniform_real_distribution<float>::param_type{0.11f, 25.0f}),
                rand(gen, std::uniform_real_distribution<float>::param_type{0.11f, 25.0f}),
                rand(gen, std::uniform_real_distribution<float>::param_type{0.11f, 25.0f})
        };
    }

    Culler::result_type * __restrict__ visibles = nullptr;
    posix_memalign((void**)&visibles, 32, batch * sizeof(*visibles));

    Profiler p(Profiler::EV_CPU_CYCLES | Profiler::EV_BPU_MISSES);

    benchmark(p, "Box Culling Direct", [&]() {
        Culler::Test::intersects(visibles, frustum, boxesCenter.data(), boxesExtent.data(), batch);
    });

    size_t vb = 0;
    for (size_t i = 0; i < batch; i++) {
        vb = vb + (visibles[i] ? 1 : 0);
    }

    benchmark(p, "Sphere Culling Direct", [&]() {
        Culler::Test::intersects(visibles, frustum, spheres.data(), batch);
    });

    size_t vs = 0;
    for (size_t i = 0; i < batch; i++) {
        vs = vs + (visibles[i] ? 1 : 0);
    }

    std::cout << "visible boxes: " << vb << std::endl;
    std::cout << "visible spheres: " << vs << std::endl;
    std::cout << std::endl;

    free(visibles);


    benchmark(p, "cos", [&]() {
        for (size_t i = 0; i < batch; i++) {
            spheres[i].x = std::cos(spheres[i].x);
        }
    });

    benchmark(p, "fast::cos", [&]() {
        for (size_t i = 0; i < batch; i++) {
            spheres[i].x = math::fast::cos<float>(spheres[i].x);
        }
    });
    
    benchmark(p, "rsqrt", [&]() {
        for (size_t i = 0; i < batch; i++) {
            spheres[i].x = 1.0f / std::sqrt(spheres[i].x);
        }
    });

    benchmark(p, "fast::rsqrt", [&]() {
        for (size_t i = 0; i < batch; i++) {
            spheres[i].x = math::fast::isqrt(spheres[i].x);
        }
    });

    benchmark(p, "half4", [&]() {
        for (size_t i = 0; i < batch; i++) {
            spheresHalf[i] = half4(spheres[i]);
        }
    });

    benchmark(p, "half x 4", [&]() {
        for (size_t i = 0; i < batch; i++) {
            spheresHalf[i] = half4(
                    spheres[i].x,
                    spheres[i].y,
                    spheres[i].z,
                    spheres[i].w
            );
        }
    });

    benchmark(p, "pow(float4, 2.2f)", [&]() {
        for (size_t i = 0; i < batch; i++) {
            colors_results[i] = pow(colors[i], 2.2f);
        }
    });

    benchmark(p, "fast::pow(float4, 2.2f)", [&]() {
        for (size_t i = 0; i < batch; i++) {
            colors_results[i] = {
                    fast::pow(colors[i].x, 2.2f),
                    fast::pow(colors[i].y, 2.2f),
                    fast::pow(colors[i].z, 2.2f),
                    fast::pow(colors[i].w, 2.2f),
            };
        }
    });

    benchmark(p, "fast::pow2dot2(float4)", [&]() {
        for (size_t i = 0; i < batch; i++) {
            colors_results[i] = {
                    fast::pow2dot2(colors[i].x),
                    fast::pow2dot2(colors[i].y),
                    fast::pow2dot2(colors[i].z),
                    fast::pow2dot2(colors[i].w),
            };
        }
    });

    return 0;
}

// ------------------------------------------------------------------------------------------------
//
// Running benchmarks on Android
//
// make -j4 && adb push filament/filament/test/benchmark_filament /data/local/tmp
// adb shell /data/local/tmp/filament_benchmark
