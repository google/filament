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

#include "Culler.h"
#include "PerformanceCounters.h"

#include <filament/Box.h>
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/Frustum.h>
#include <filament/ToneMapper.h>

#include <utils/Allocator.h>
#include <utils/FixedCapacityVector.h>

#include <benchmark/benchmark.h>

#include <random>
#include <vector>

using namespace filament;
using namespace filament::math;
using namespace utils;


class FilamentCullingFixture : public benchmark::Fixture {
protected:
    static constexpr size_t BATCH_SIZE = 512;

    Frustum frustum{};
    std::vector<float3> boxesCenter;
    std::vector<float3> boxesExtent;
    std::vector<float4> spheres;
    Culler::result_type* UTILS_RESTRICT visibles = nullptr;


public:
    FilamentCullingFixture() {

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

    ~FilamentCullingFixture() override {
        aligned_free(visibles);
    }
};

BENCHMARK_F(FilamentCullingFixture, boxCulling)(benchmark::State& state) {
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

BENCHMARK_F(FilamentCullingFixture, sphereCulling)(benchmark::State& state) {
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

class ColorGradingFixture : public benchmark::Fixture {
protected:
    Engine* engine = nullptr;

public:
    static constexpr size_t kMaxAccumulatedLuts = 100;

    void SetUp(const benchmark::State& state) override {
        Engine::Config config;
        config.commandBufferSizeMB = 8;
        engine = Engine::Builder()
            .backend(Engine::Backend::NOOP)
            .config(&config)
            .build();
    }

    void TearDown(const benchmark::State& state) override {
        Engine::destroy(&engine);
    }
};

BENCHMARK_F(ColorGradingFixture, lutGenerationDefault)(benchmark::State& state) {
    {
        ColorGrading::Builder builder;
        std::vector<ColorGrading*> cgs;
        cgs.reserve(kMaxAccumulatedLuts);
        PerformanceCounters pc(state);
        for (auto _ : state) {
            cgs.push_back(builder.build(*engine));
        }
        benchmark::ClobberMemory();
        pc.stop();
        for (ColorGrading* cg : cgs) {
            engine->destroy(cg);
        }
        engine->flush();
        state.SetItemsProcessed(state.iterations() * 32 * 32 * 32);
    }
}

BENCHMARK_F(ColorGradingFixture, lutGenerationWithAdjustments)(benchmark::State& state) {
    {
        ColorGrading::Builder builder;
        builder.format(ColorGrading::LutFormat::FLOAT)
               .exposure(0.5f)
               .contrast(1.1f)
               .saturation(1.05f);
        std::vector<ColorGrading*> cgs;
        cgs.reserve(kMaxAccumulatedLuts);
        PerformanceCounters pc(state);
        for (auto _ : state) {
            cgs.push_back(builder.build(*engine));
        }
        benchmark::ClobberMemory();
        pc.stop();
        for (ColorGrading* cg : cgs) {
            engine->destroy(cg);
        }
        engine->flush();
        state.SetItemsProcessed(state.iterations() * 32 * 32 * 32);
    }
}

BENCHMARK_F(ColorGradingFixture, lutGenerationWithAdjustmentsInteger)(benchmark::State& state) {
    {
        ColorGrading::Builder builder;
        builder.format(ColorGrading::LutFormat::INTEGER)
               .exposure(0.5f)
               .contrast(1.1f)
               .saturation(1.05f);
        std::vector<ColorGrading*> cgs;
        cgs.reserve(kMaxAccumulatedLuts);
        PerformanceCounters pc(state);
        for (auto _ : state) {
            cgs.push_back(builder.build(*engine));
        }
        benchmark::ClobberMemory();
        pc.stop();
        for (ColorGrading* cg : cgs) {
            engine->destroy(cg);
        }
        engine->flush();
        state.SetItemsProcessed(state.iterations() * 32 * 32 * 32);
    }
}

BENCHMARK_F(ColorGradingFixture, lutGenerationAdvanced32)(benchmark::State& state) {
    {
        ColorGrading::Builder builder;
        builder.format(ColorGrading::LutFormat::INTEGER)
               .dimensions(32)
               .exposure(0.5f)
               .contrast(1.1f)
               .saturation(1.05f)
               .nightAdaptation(0.5f)
               .luminanceScaling(true)
               .gamutMapping(true);
        std::vector<ColorGrading*> cgs;
        cgs.reserve(kMaxAccumulatedLuts);
        PerformanceCounters pc(state);
        for (auto _ : state) {
            cgs.push_back(builder.build(*engine));
        }
        benchmark::ClobberMemory();
        pc.stop();
        for (ColorGrading* cg : cgs) {
            engine->destroy(cg);
        }
        engine->flush();
        state.SetItemsProcessed(state.iterations() * 32 * 32 * 32);
    }
}


BENCHMARK_F(ColorGradingFixture, lutGenerationUltraQuality)(benchmark::State& state) {
    {
        ColorGrading::Builder builder;
        builder.quality(ColorGrading::QualityLevel::ULTRA)
               .exposure(0.5f)
               .contrast(1.1f);
        std::vector<ColorGrading*> cgs;
        cgs.reserve(kMaxAccumulatedLuts);
        PerformanceCounters pc(state);
        for (auto _ : state) {
            cgs.push_back(builder.build(*engine));
        }
        benchmark::ClobberMemory();
        pc.stop();
        for (ColorGrading* cg : cgs) {
            engine->destroy(cg);
        }
        engine->flush();
        state.SetItemsProcessed(state.iterations() * 64 * 64 * 64);
    }
}

BENCHMARK_F(ColorGradingFixture, lutGenerationCustomLutBaseline)(benchmark::State& state) {
    {
        ColorGrading::Builder builder;
        builder.format(ColorGrading::LutFormat::INTEGER)
               .dimensions(32)
               .toneMapping(ColorGrading::ToneMapping::ACES);
        std::vector<ColorGrading*> cgs;
        cgs.reserve(kMaxAccumulatedLuts);
        PerformanceCounters pc(state);
        for (auto _ : state) {
            cgs.push_back(builder.build(*engine));
        }
        benchmark::ClobberMemory();
        pc.stop();
        for (ColorGrading* cg : cgs) {
            engine->destroy(cg);
        }
        engine->flush();
        state.SetItemsProcessed(state.iterations() * 32 * 32 * 32);
    }
}

BENCHMARK_F(ColorGradingFixture, lutGenerationWithCustomLut)(benchmark::State& state) {
    {
        utils::FixedCapacityVector<math::float3> lut(32 * 32 * 32, math::float3{0.5f, 0.5f, 0.5f});
        ColorGrading::Builder builder;
        builder.format(ColorGrading::LutFormat::INTEGER)
               .dimensions(32)
               .toneMapping(ColorGrading::ToneMapping::ACES)
               .customLut(std::move(lut), 32);
        std::vector<ColorGrading*> cgs;
        cgs.reserve(kMaxAccumulatedLuts);
        PerformanceCounters pc(state);
        for (auto _ : state) {
            cgs.push_back(builder.build(*engine));
        }
        benchmark::ClobberMemory();
        pc.stop();
        for (ColorGrading* cg : cgs) {
            engine->destroy(cg);
        }
        engine->flush();
        state.SetItemsProcessed(state.iterations() * 32 * 32 * 32);
    }
}

BENCHMARK_F(ColorGradingFixture, lutGeneration1DLDR)(benchmark::State& state) {
    {
        ColorGrading::Builder builder;
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
        builder.toneMapping(ColorGrading::ToneMapping::LINEAR);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
        std::vector<ColorGrading*> cgs;
        cgs.reserve(kMaxAccumulatedLuts);
        PerformanceCounters pc(state);
        for (auto _ : state) {
            cgs.push_back(builder.build(*engine));
        }
        benchmark::ClobberMemory();
        pc.stop();
        for (ColorGrading* cg : cgs) {
            engine->destroy(cg);
        }
        engine->flush();
        state.SetItemsProcessed(state.iterations() * 512);
    }
}

BENCHMARK_F(ColorGradingFixture, lutGeneration1DHDR)(benchmark::State& state) {
    {
        ColorGrading::Builder builder;
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
        builder.toneMapping(ColorGrading::ToneMapping::FILMIC);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
        std::vector<ColorGrading*> cgs;
        cgs.reserve(kMaxAccumulatedLuts);
        PerformanceCounters pc(state);
        for (auto _ : state) {
            cgs.push_back(builder.build(*engine));
        }
        benchmark::ClobberMemory();
        pc.stop();
        for (ColorGrading* cg : cgs) {
            engine->destroy(cg);
        }
        engine->flush();
        state.SetItemsProcessed(state.iterations() * 512);
    }
}

template <typename ToneMapperType>
void benchmarkToneMapper(Engine* engine, benchmark::State& state) {
    ToneMapperType tm;
    ColorGrading::Builder builder;
    builder.format(ColorGrading::LutFormat::INTEGER)
           .dimensions(32)
           .toneMapper(&tm);
    std::vector<ColorGrading*> cgs;
    cgs.reserve(ColorGradingFixture::kMaxAccumulatedLuts);
    PerformanceCounters pc(state);
    for (auto _ : state) {
        cgs.push_back(builder.build(*engine));
    }
    benchmark::ClobberMemory();
    pc.stop();
    for (ColorGrading* cg : cgs) {
        engine->destroy(cg);
    }
    engine->flush();
    state.SetItemsProcessed(state.iterations() * 32 * 32 * 32);
}

BENCHMARK_F(ColorGradingFixture, lutGenerationLinear)(benchmark::State& state) {
    benchmarkToneMapper<LinearToneMapper>(engine, state);
}
BENCHMARK_F(ColorGradingFixture, lutGenerationACES)(benchmark::State& state) {
    benchmarkToneMapper<ACESToneMapper>(engine, state);
}
BENCHMARK_F(ColorGradingFixture, lutGenerationACESLegacy)(benchmark::State& state) {
    benchmarkToneMapper<ACESLegacyToneMapper>(engine, state);
}
BENCHMARK_F(ColorGradingFixture, lutGenerationFilmic)(benchmark::State& state) {
    benchmarkToneMapper<FilmicToneMapper>(engine, state);
}
BENCHMARK_F(ColorGradingFixture, lutGenerationPBRNeutral)(benchmark::State& state) {
    benchmarkToneMapper<PBRNeutralToneMapper>(engine, state);
}
BENCHMARK_F(ColorGradingFixture, lutGenerationGT7)(benchmark::State& state) {
    benchmarkToneMapper<GT7ToneMapper>(engine, state);
}
BENCHMARK_F(ColorGradingFixture, lutGenerationAgx)(benchmark::State& state) {
    benchmarkToneMapper<AgxToneMapper>(engine, state);
}
BENCHMARK_F(ColorGradingFixture, lutGenerationGeneric)(benchmark::State& state) {
    benchmarkToneMapper<GenericToneMapper>(engine, state);
}
BENCHMARK_F(ColorGradingFixture, lutGenerationDisplayRange)(benchmark::State& state) {
    benchmarkToneMapper<DisplayRangeToneMapper>(engine, state);
}
