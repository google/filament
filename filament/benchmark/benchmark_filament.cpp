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


#include <filament/Box.h>
#include <filament/Frustum.h>
#include "details/Culler.h"

#include <utils/Allocator.h>

#include <vector>
#include <random>

using namespace filament;
using namespace filament::math;
using namespace utils;


class FilamentFixture : public benchmark::Fixture {
protected:
    static constexpr size_t BATCH_SIZE = 512;

    Frustum frustum{};
    std::vector<float3> boxesCenter;
    std::vector<float3> boxesExtent;
    std::vector<float4> spheres;
    Culler::result_type* UTILS_RESTRICT visibles = nullptr;


public:
    FilamentFixture() {

        std::default_random_engine gen; // NOLINT
        std::uniform_real_distribution<float> rand(-100.0f, 100.0f);

        const size_t batch = BATCH_SIZE;
        frustum = Frustum{ mat4f::perspective(45.0f, 1.0f, 0.1f, 100.0f) };

        boxesCenter.resize(batch);
        boxesExtent.resize(batch);
        spheres.resize(batch);
        for (size_t i = 0; i < batch; i++) {
            float4& sphere = spheres[i];
            float z = std::fabs(rand(gen));
            sphere.z = -z;
            sphere.x = rand(gen, std::uniform_real_distribution<float>::param_type{ -z, z });
            sphere.y = rand(gen, std::uniform_real_distribution<float>::param_type{ -z, z });
            sphere.w = rand(gen, std::uniform_real_distribution<float>::param_type{ 0.11f, 25.0f });

            boxesCenter[i] = sphere.xyz;
            boxesExtent[i] = {
                    rand(gen, std::uniform_real_distribution<float>::param_type{ 0.11f, 25.0f }),
                    rand(gen, std::uniform_real_distribution<float>::param_type{ 0.11f, 25.0f }),
                    rand(gen, std::uniform_real_distribution<float>::param_type{ 0.11f, 25.0f })
            };
        }

        visibles = (Culler::result_type*)utils::aligned_alloc(batch * sizeof(*visibles), 32);
    }

    ~FilamentFixture() override {
        utils::aligned_free(visibles);
    }
};

BENCHMARK_F(FilamentFixture, boxCulling)(benchmark::State& state) {
    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            Culler::Test::intersects(visibles, frustum, boxesCenter.data(), boxesExtent.data(), BATCH_SIZE);
        }
        benchmark::ClobberMemory();
        pc.stop();
        state.SetItemsProcessed(state.iterations() * BATCH_SIZE);
    }
}

BENCHMARK_F(FilamentFixture, sphereCulling)(benchmark::State& state) {
    {
        PerformanceCounters pc(state);
        for (auto _ : state) {
            Culler::Test::intersects(visibles, frustum, spheres.data(), BATCH_SIZE);
        }
        benchmark::ClobberMemory();
        pc.stop();
        state.SetItemsProcessed(state.iterations() * BATCH_SIZE);
    }
}
