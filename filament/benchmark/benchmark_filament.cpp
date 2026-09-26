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
#include "downcast.h"
#include "PerformanceCounters.h"

#include "details/Engine.h"

#include <filament/Box.h>
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/Frustum.h>
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/ToneMapper.h>
#include <filament/TransformManager.h>

#include <utils/Allocator.h>
#include <utils/compiler.h>
#include <utils/EntityManager.h>
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
    std::vector<float> box_cx, box_cy, box_cz;
    std::vector<float> box_ex, box_ey, box_ez;
    std::vector<float> sphere_cx, sphere_cy, sphere_cz, sphere_r;
    std::vector<float4> planes12;
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

        box_cx.resize(batch); box_cy.resize(batch); box_cz.resize(batch);
        box_ex.resize(batch); box_ey.resize(batch); box_ez.resize(batch);
        sphere_cx.resize(batch); sphere_cy.resize(batch); sphere_cz.resize(batch); sphere_r.resize(batch);

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

            box_cx[i] = boxesCenter[i].x;
            box_cy[i] = boxesCenter[i].y;
            box_cz[i] = boxesCenter[i].z;
            box_ex[i] = boxesExtent[i].x;
            box_ey[i] = boxesExtent[i].y;
            box_ez[i] = boxesExtent[i].z;

            sphere_cx[i] = sphere.x;
            sphere_cy[i] = sphere.y;
            sphere_cz[i] = sphere.z;
            sphere_r[i]  = sphere.w;
        }

        planes12.resize(12);
        for (size_t i = 0; i < 12; i++) {
            planes12[i] = float4(rand(gen), rand(gen), rand(gen), rand(gen));
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
        for (UTILS_UNUSED auto _ : state) {
            Culler::Test::intersects(visibles, frustum,
                    box_cx.data(), box_cy.data(), box_cz.data(),
                    box_ex.data(), box_ey.data(), box_ez.data(),
                    BATCH_SIZE);
        }
        benchmark::ClobberMemory();
        pc.stop();
        state.SetItemsProcessed(state.iterations() * BATCH_SIZE);
    }
}

BENCHMARK_F(FilamentCullingFixture, boxCulling12Planes)(benchmark::State& state) {
    {
        PerformanceCounters pc(state);
        for (auto _: state) {
            Culler::Test::intersects(visibles, planes12.data(), box_cx.data(), box_cy.data(),
                    box_cz.data(), box_ex.data(), box_ey.data(), box_ez.data(), BATCH_SIZE);
        }
        benchmark::ClobberMemory();
        pc.stop();
        state.SetItemsProcessed(state.iterations() * BATCH_SIZE);
    }
}

BENCHMARK_F(FilamentCullingFixture, sphereCulling)(benchmark::State& state) {
    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            Culler::Test::intersects(visibles, frustum,
                    sphere_cx.data(), sphere_cy.data(), sphere_cz.data(),
                    sphere_r.data(),
                    BATCH_SIZE);
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
        for (UTILS_UNUSED auto _: state) {
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
        for (UTILS_UNUSED auto _: state) {
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
        for (UTILS_UNUSED auto _: state) {
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
        for (UTILS_UNUSED auto _: state) {
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
        for (UTILS_UNUSED auto _: state) {
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
        for (UTILS_UNUSED auto _: state) {
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
        for (UTILS_UNUSED auto _: state) {
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
        for (UTILS_UNUSED auto _: state) {
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
        for (UTILS_UNUSED auto _: state) {
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

class EngineGcWorstCaseFixture : public benchmark::Fixture {
protected:
    Engine* engine = nullptr;
    EntityManager* em = nullptr;
    TransformManager* tcm = nullptr;
    LightManager* lcm = nullptr;
    RenderableManager* rcm = nullptr;

public:
    void SetUp(const benchmark::State& state) override {
        Engine::Config config;
        engine = Engine::Builder()
            .backend(Engine::Backend::NOOP)
            .config(&config)
            .build();
        em = &engine->getEntityManager();
        tcm = &engine->getTransformManager();
        lcm = &engine->getLightManager();
        rcm = &engine->getRenderableManager();
    }

    void TearDown(const benchmark::State& state) override {
        Engine::destroy(&engine);
    }
};

BENCHMARK_F(EngineGcWorstCaseFixture, worstCaseSequentialGc)(benchmark::State& state) {
    constexpr size_t TOTAL_ENTITIES = 10000;
    constexpr size_t DESTROYED_ENTITIES = 5000;

    std::vector<Entity> entities(TOTAL_ENTITIES);

    PerformanceCounters pc(state);
    for (auto _ : state) {
        state.PauseTiming();
        for (size_t i = 0; i < TOTAL_ENTITIES; ++i) {
            entities[i] = em->create();
            tcm->create(entities[i], 0, mat4f());
            LightManager::Builder(LightManager::Type::POINT).build(*engine, entities[i]);
            RenderableManager::Builder(1).boundingBox({{0,0,0},{1,1,1}}).build(*engine, entities[i]);
            engine->createCamera(entities[i]);
        }
        em->advanceEpoch();

        for (size_t i = 0; i < DESTROYED_ENTITIES; ++i) {
            em->destroy(entities[i]);
        }
        em->advanceEpoch();

        state.ResumeTiming();

        downcast(engine)->gc();

        state.PauseTiming();
        for (size_t i = DESTROYED_ENTITIES; i < TOTAL_ENTITIES; ++i) {
            em->destroy(entities[i]);
        }
        em->advanceEpoch();
        downcast(engine)->gc();
        entities.clear();
        entities.resize(TOTAL_ENTITIES);
    }
    pc.stop();
    state.SetItemsProcessed(state.iterations() * DESTROYED_ENTITIES);
}

class TransformManagerFixture : public benchmark::Fixture {
protected:
    Engine* engine = nullptr;
    EntityManager* em = nullptr;
    TransformManager* tcm = nullptr;
    utils::PagedArenaBitset dirtyEntities;

public:
    void SetUp(const benchmark::State& state) override {
        Engine::Config config;
        engine = Engine::Builder()
            .backend(Engine::Backend::NOOP)
            .config(&config)
            .build();
        em = &engine->getEntityManager();
        tcm = &engine->getTransformManager();
        downcast(tcm)->registerBitset(&dirtyEntities);
    }

    void TearDown(const benchmark::State& state) override {
        downcast(tcm)->unregisterBitset(&dirtyEntities);
        Engine::destroy(&engine);
    }
};

BENCHMARK_F(TransformManagerFixture, setTransformFlatRootNodes)(benchmark::State& state) {
    constexpr size_t COUNT = 1024;
    std::vector<Entity> entities(COUNT);
    std::vector<TransformManager::Instance> instances(COUNT);
    std::vector<mat4f> transformsA(COUNT);
    std::vector<mat4f> transformsB(COUNT);

    em->create(COUNT, entities.data());
    for (size_t i = 0; i < COUNT; ++i) {
        float const s = float(i + 1) * 0.01f;
        transformsA[i] = mat4f::translation(float3{ s, s * 2.0f, s * 3.0f }) *
                         mat4f::rotation(s, float3{ 0.0f, 1.0f, 0.0f });
        transformsB[i] = mat4f::translation(float3{ -s, s * 1.5f, -s * 2.5f }) *
                         mat4f::rotation(s * 0.5f, float3{ 0.0f, 1.0f, 0.0f });
        tcm->create(entities[i], {}, transformsA[i]);
        instances[i] = tcm->getInstance(entities[i]);
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();
    bool toggle = false;

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            auto const& transforms = toggle ? transformsA : transformsB;
            toggle = !toggle;
            for (size_t i = 0; i < COUNT; ++i) {
                tcm->setTransform(instances[i], transforms[i]);
            }
            downcast(tcm)->ensureWorldTransformsUpToDate();
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * COUNT);
    }

    for (size_t i = 0; i < COUNT; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(COUNT, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformParentedLeafNodes)(benchmark::State& state) {
    constexpr size_t COUNT = 1024;
    Entity rootEntity = em->create();
    tcm->create(rootEntity, {}, mat4f::translation(float3{ 10.0f, 20.0f, 30.0f }));
    TransformManager::Instance const rootInstance = tcm->getInstance(rootEntity);

    std::vector<Entity> entities(COUNT);
    std::vector<TransformManager::Instance> instances(COUNT);
    std::vector<mat4f> transformsA(COUNT);
    std::vector<mat4f> transformsB(COUNT);

    em->create(COUNT, entities.data());
    for (size_t i = 0; i < COUNT; ++i) {
        float const s = float(i + 1) * 0.01f;
        transformsA[i] = mat4f::translation(float3{ s, s * 2.0f, s * 3.0f }) *
                         mat4f::rotation(s, float3{ 0.0f, 1.0f, 0.0f });
        transformsB[i] = mat4f::translation(float3{ -s, s * 1.5f, -s * 2.5f }) *
                         mat4f::rotation(s * 0.5f, float3{ 0.0f, 1.0f, 0.0f });
        tcm->create(entities[i], rootInstance, transformsA[i]);
        instances[i] = tcm->getInstance(entities[i]);
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();
    bool toggle = false;

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            auto const& transforms = toggle ? transformsA : transformsB;
            toggle = !toggle;
            for (size_t i = 0; i < COUNT; ++i) {
                tcm->setTransform(instances[i], transforms[i]);
            }
            downcast(tcm)->ensureWorldTransformsUpToDate();
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * COUNT);
    }

    for (size_t i = 0; i < COUNT; ++i) {
        tcm->destroy(entities[i]);
    }
    tcm->destroy(rootEntity);
    em->destroy(COUNT, entities.data());
    em->destroy(rootEntity);
}

BENCHMARK_F(TransformManagerFixture, setTransformHierarchy)(benchmark::State& state) {
    // 4-ary tree of depth 5: 1 + 4 + 16 + 64 + 256 + 1024 = 1365 nodes
    constexpr size_t BRANCHING = 4;
    constexpr size_t TOTAL_NODES = 1 + 4 + 16 + 64 + 256 + 1024;
    constexpr size_t INTERNAL_NODES = 1 + 4 + 16 + 64 + 256;

    std::vector<Entity> entities(TOTAL_NODES);
    std::vector<TransformManager::Instance> instances(TOTAL_NODES);
    em->create(TOTAL_NODES, entities.data());

    mat4f const localMat = mat4f::translation(float3{ 1.0f, 2.0f, 3.0f }) *
                           mat4f::rotation(0.25f, float3{ 0.0f, 1.0f, 0.0f });

    tcm->create(entities[0], {}, localMat);
    instances[0] = tcm->getInstance(entities[0]);

    size_t nextIdx = 1;
    for (size_t p = 0; p < INTERNAL_NODES; ++p) {
        for (size_t b = 0; b < BRANCHING; ++b) {
            tcm->create(entities[nextIdx], instances[p], localMat);
            instances[nextIdx] = tcm->getInstance(entities[nextIdx]);
            ++nextIdx;
        }
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();

    mat4f const rootMatA = mat4f::translation(float3{ 5.0f, 6.0f, 7.0f });
    mat4f const rootMatB = mat4f::translation(float3{ -5.0f, -6.0f, -7.0f });
    bool toggle = false;

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            tcm->setTransform(instances[0], toggle ? rootMatA : rootMatB);
            downcast(tcm)->ensureWorldTransformsUpToDate();
            toggle = !toggle;
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * TOTAL_NODES);
    }

    for (size_t i = 0; i < TOTAL_NODES; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(TOTAL_NODES, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformFlatRootNodesAccurate)(benchmark::State& state) {
    tcm->setAccurateTranslationsEnabled(true);
    constexpr size_t COUNT = 1024;
    std::vector<Entity> entities(COUNT);
    std::vector<TransformManager::Instance> instances(COUNT);
    std::vector<mat4> transformsA(COUNT);
    std::vector<mat4> transformsB(COUNT);

    em->create(COUNT, entities.data());
    for (size_t i = 0; i < COUNT; ++i) {
        double const s = double(i + 1) * 0.01;
        transformsA[i] = mat4::translation(double3{ 100000.0 + s, -200000.0 + s * 2.0, 300000.0 + s * 3.0 }) *
                         mat4::rotation(s, double3{ 0.0, 1.0, 0.0 });
        transformsB[i] = mat4::translation(double3{ 100000.0 - s, -200000.0 + s * 1.5, 300000.0 - s * 2.5 }) *
                         mat4::rotation(s * 0.5, double3{ 0.0, 1.0, 0.0 });
        tcm->create(entities[i], {}, transformsA[i]);
        instances[i] = tcm->getInstance(entities[i]);
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();
    bool toggle = false;

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            auto const& transforms = toggle ? transformsA : transformsB;
            toggle = !toggle;
            for (size_t i = 0; i < COUNT; ++i) {
                tcm->setTransform(instances[i], transforms[i]);
            }
            downcast(tcm)->ensureWorldTransformsUpToDate();
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * COUNT);
    }

    for (size_t i = 0; i < COUNT; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(COUNT, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformParentedLeafNodesAccurate)(benchmark::State& state) {
    tcm->setAccurateTranslationsEnabled(true);
    constexpr size_t COUNT = 1024;
    Entity rootEntity = em->create();
    tcm->create(rootEntity, {}, mat4::translation(double3{ 100000.125, -200000.25, 300000.5 }));
    TransformManager::Instance const rootInstance = tcm->getInstance(rootEntity);

    std::vector<Entity> entities(COUNT);
    std::vector<TransformManager::Instance> instances(COUNT);
    std::vector<mat4> transformsA(COUNT);
    std::vector<mat4> transformsB(COUNT);

    em->create(COUNT, entities.data());
    for (size_t i = 0; i < COUNT; ++i) {
        double const s = double(i + 1) * 0.01;
        transformsA[i] = mat4::translation(double3{ s, s * 2.0, s * 3.0 }) *
                         mat4::rotation(s, double3{ 0.0, 1.0, 0.0 });
        transformsB[i] = mat4::translation(double3{ -s, s * 1.5, -s * 2.5 }) *
                         mat4::rotation(s * 0.5, double3{ 0.0, 1.0, 0.0 });
        tcm->create(entities[i], rootInstance, transformsA[i]);
        instances[i] = tcm->getInstance(entities[i]);
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();
    bool toggle = false;

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            auto const& transforms = toggle ? transformsA : transformsB;
            toggle = !toggle;
            for (size_t i = 0; i < COUNT; ++i) {
                tcm->setTransform(instances[i], transforms[i]);
            }
            downcast(tcm)->ensureWorldTransformsUpToDate();
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * COUNT);
    }

    for (size_t i = 0; i < COUNT; ++i) {
        tcm->destroy(entities[i]);
    }
    tcm->destroy(rootEntity);
    em->destroy(COUNT, entities.data());
    em->destroy(rootEntity);
}

BENCHMARK_F(TransformManagerFixture, setTransformHierarchyAccurate)(benchmark::State& state) {
    tcm->setAccurateTranslationsEnabled(true);
    // 4-ary tree of depth 5: 1 + 4 + 16 + 64 + 256 + 1024 = 1365 nodes
    constexpr size_t BRANCHING = 4;
    constexpr size_t TOTAL_NODES = 1 + 4 + 16 + 64 + 256 + 1024;
    constexpr size_t INTERNAL_NODES = 1 + 4 + 16 + 64 + 256;

    std::vector<Entity> entities(TOTAL_NODES);
    std::vector<TransformManager::Instance> instances(TOTAL_NODES);
    em->create(TOTAL_NODES, entities.data());

    mat4 const localMat = mat4::translation(double3{ 1.0001220703125, 2.000244140625, 3.00048828125 }) *
                          mat4::rotation(0.25, double3{ 0.0, 1.0, 0.0 });

    tcm->create(entities[0], {}, localMat);
    instances[0] = tcm->getInstance(entities[0]);

    size_t nextIdx = 1;
    for (size_t p = 0; p < INTERNAL_NODES; ++p) {
        for (size_t b = 0; b < BRANCHING; ++b) {
            tcm->create(entities[nextIdx], instances[p], localMat);
            instances[nextIdx] = tcm->getInstance(entities[nextIdx]);
            ++nextIdx;
        }
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();

    mat4 const rootMatA = mat4::translation(double3{ 100005.125, -200006.25, 300007.5 });
    mat4 const rootMatB = mat4::translation(double3{ -100005.125, 200006.25, -300007.5 });
    bool toggle = false;

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            tcm->setTransform(instances[0], toggle ? rootMatA : rootMatB);
            downcast(tcm)->ensureWorldTransformsUpToDate();
            toggle = !toggle;
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * TOTAL_NODES);
    }

    for (size_t i = 0; i < TOTAL_NODES; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(TOTAL_NODES, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformIdentical)(benchmark::State& state) {
    constexpr size_t COUNT = 1024;
    std::vector<Entity> entities(COUNT);
    std::vector<TransformManager::Instance> instances(COUNT);
    std::vector<mat4f> transforms(COUNT);

    em->create(COUNT, entities.data());
    for (size_t i = 0; i < COUNT; ++i) {
        float const s = float(i + 1) * 0.01f;
        transforms[i] = mat4f::translation(float3{ s, s * 2.0f, s * 3.0f }) *
                        mat4f::rotation(s, float3{ 0.0f, 1.0f, 0.0f });
        tcm->create(entities[i], {}, transforms[i]);
        instances[i] = tcm->getInstance(entities[i]);
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            for (size_t i = 0; i < COUNT; ++i) {
                tcm->setTransform(instances[i], transforms[i]);
            }
            downcast(tcm)->ensureWorldTransformsUpToDate();
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * COUNT);
    }

    for (size_t i = 0; i < COUNT; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(COUNT, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformHierarchyTopDown)(benchmark::State& state) {
    constexpr size_t BRANCHING = 4;
    constexpr size_t TOTAL_NODES = 1 + 4 + 16 + 64 + 256 + 1024;
    constexpr size_t INTERNAL_NODES = 1 + 4 + 16 + 64 + 256;

    std::vector<Entity> entities(TOTAL_NODES);
    std::vector<TransformManager::Instance> instances(TOTAL_NODES);
    em->create(TOTAL_NODES, entities.data());

    mat4f const matA = mat4f::translation(float3{ 1.0f, 2.0f, 3.0f }) *
                       mat4f::rotation(0.25f, float3{ 0.0f, 1.0f, 0.0f });
    mat4f const matB = mat4f::translation(float3{ -1.0f, -2.0f, -3.0f }) *
                       mat4f::rotation(0.5f, float3{ 0.0f, 1.0f, 0.0f });

    tcm->create(entities[0], {}, matA);
    instances[0] = tcm->getInstance(entities[0]);

    size_t nextIdx = 1;
    for (size_t p = 0; p < INTERNAL_NODES; ++p) {
        for (size_t b = 0; b < BRANCHING; ++b) {
            tcm->create(entities[nextIdx], instances[p], matA);
            instances[nextIdx] = tcm->getInstance(entities[nextIdx]);
            ++nextIdx;
        }
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();
    bool toggle = false;

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            mat4f const& m = toggle ? matA : matB;
            toggle = !toggle;
            for (size_t i = 0; i < TOTAL_NODES; ++i) {
                tcm->setTransform(instances[i], m);
            }
            downcast(tcm)->ensureWorldTransformsUpToDate();
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * TOTAL_NODES);
    }

    for (size_t i = 0; i < TOTAL_NODES; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(TOTAL_NODES, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformHierarchyTRS)(benchmark::State& state) {
    constexpr size_t BRANCHING = 4;
    constexpr size_t TOTAL_NODES = 1 + 4 + 16 + 64 + 256 + 1024;
    constexpr size_t INTERNAL_NODES = 1 + 4 + 16 + 64 + 256;

    std::vector<Entity> entities(TOTAL_NODES);
    std::vector<TransformManager::Instance> instances(TOTAL_NODES);
    em->create(TOTAL_NODES, entities.data());

    mat4f const matT = mat4f::translation(float3{ 1.0f, 2.0f, 3.0f });
    mat4f const matTR = matT * mat4f::rotation(0.25f, float3{ 0.0f, 1.0f, 0.0f });
    mat4f const matTRS = matTR * mat4f::scaling(float3{ 1.1f, 1.1f, 1.1f });

    tcm->create(entities[0], {}, matTRS);
    instances[0] = tcm->getInstance(entities[0]);

    size_t nextIdx = 1;
    for (size_t p = 0; p < INTERNAL_NODES; ++p) {
        for (size_t b = 0; b < BRANCHING; ++b) {
            tcm->create(entities[nextIdx], instances[p], matTRS);
            instances[nextIdx] = tcm->getInstance(entities[nextIdx]);
            ++nextIdx;
        }
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            tcm->setTransform(instances[0], matT);
            tcm->setTransform(instances[0], matTR);
            tcm->setTransform(instances[0], matTRS);
            downcast(tcm)->ensureWorldTransformsUpToDate();
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * TOTAL_NODES);
    }

    for (size_t i = 0; i < TOTAL_NODES; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(TOTAL_NODES, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformHierarchyTopDownTRS)(benchmark::State& state) {
    constexpr size_t BRANCHING = 4;
    constexpr size_t TOTAL_NODES = 1 + 4 + 16 + 64 + 256 + 1024;
    constexpr size_t INTERNAL_NODES = 1 + 4 + 16 + 64 + 256;

    std::vector<Entity> entities(TOTAL_NODES);
    std::vector<TransformManager::Instance> instances(TOTAL_NODES);
    em->create(TOTAL_NODES, entities.data());

    mat4f const matT = mat4f::translation(float3{ 1.0f, 2.0f, 3.0f });
    mat4f const matTR = matT * mat4f::rotation(0.25f, float3{ 0.0f, 1.0f, 0.0f });
    mat4f const matTRS = matTR * mat4f::scaling(float3{ 1.1f, 1.1f, 1.1f });

    tcm->create(entities[0], {}, matTRS);
    instances[0] = tcm->getInstance(entities[0]);

    size_t nextIdx = 1;
    for (size_t p = 0; p < INTERNAL_NODES; ++p) {
        for (size_t b = 0; b < BRANCHING; ++b) {
            tcm->create(entities[nextIdx], instances[p], matTRS);
            instances[nextIdx] = tcm->getInstance(entities[nextIdx]);
            ++nextIdx;
        }
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            for (size_t i = 0; i < TOTAL_NODES; ++i) {
                tcm->setTransform(instances[i], matT);
                tcm->setTransform(instances[i], matTR);
                tcm->setTransform(instances[i], matTRS);
            }
            downcast(tcm)->ensureWorldTransformsUpToDate();
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * TOTAL_NODES);
    }

    for (size_t i = 0; i < TOTAL_NODES; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(TOTAL_NODES, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformGetWorldTransformInterleavedFlat)(benchmark::State& state) {
    constexpr size_t COUNT = 1024;
    std::vector<Entity> entities(COUNT);
    std::vector<TransformManager::Instance> instances(COUNT);
    em->create(COUNT, entities.data());

    mat4f const matA = mat4f::translation(float3{ 1.0f, 2.0f, 3.0f }) *
                       mat4f::rotation(0.25f, float3{ 0.0f, 1.0f, 0.0f });
    mat4f const matB = mat4f::translation(float3{ -1.0f, -2.0f, -3.0f }) *
                       mat4f::rotation(0.5f, float3{ 0.0f, 1.0f, 0.0f });

    for (size_t i = 0; i < COUNT; ++i) {
        tcm->create(entities[i], {}, matA);
        instances[i] = tcm->getInstance(entities[i]);
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();
    bool toggle = false;

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            mat4f const& m = toggle ? matA : matB;
            toggle = !toggle;
            for (size_t i = 0; i < COUNT; ++i) {
                tcm->setTransform(instances[i], m);
                mat4f const& w = tcm->getWorldTransform(instances[i]);
                benchmark::DoNotOptimize(w);
            }
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * COUNT);
    }

    for (size_t i = 0; i < COUNT; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(COUNT, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformGetWorldTransformInterleavedHierarchy)(benchmark::State& state) {
    constexpr size_t BRANCHING = 4;
    constexpr size_t TOTAL_NODES = 1 + 4 + 16 + 64 + 256 + 1024;
    constexpr size_t INTERNAL_NODES = 1 + 4 + 16 + 64 + 256;

    std::vector<Entity> entities(TOTAL_NODES);
    std::vector<TransformManager::Instance> instances(TOTAL_NODES);
    em->create(TOTAL_NODES, entities.data());

    mat4f const matA = mat4f::translation(float3{ 1.0f, 2.0f, 3.0f }) *
                       mat4f::rotation(0.25f, float3{ 0.0f, 1.0f, 0.0f });
    mat4f const matB = mat4f::translation(float3{ -1.0f, -2.0f, -3.0f }) *
                       mat4f::rotation(0.5f, float3{ 0.0f, 1.0f, 0.0f });

    tcm->create(entities[0], {}, matA);
    instances[0] = tcm->getInstance(entities[0]);

    size_t nextIdx = 1;
    for (size_t p = 0; p < INTERNAL_NODES; ++p) {
        for (size_t b = 0; b < BRANCHING; ++b) {
            tcm->create(entities[nextIdx], instances[p], matA);
            instances[nextIdx] = tcm->getInstance(entities[nextIdx]);
            ++nextIdx;
        }
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();
    bool toggle = false;

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            mat4f const& m = toggle ? matA : matB;
            toggle = !toggle;
            for (size_t i = 0; i < TOTAL_NODES; ++i) {
                tcm->setTransform(instances[i], m);
                mat4f const& w = tcm->getWorldTransform(instances[i]);
                benchmark::DoNotOptimize(w);
            }
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * TOTAL_NODES);
    }

    for (size_t i = 0; i < TOTAL_NODES; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(TOTAL_NODES, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformTRSGetWorldTransformInterleavedFlat)(benchmark::State& state) {
    constexpr size_t COUNT = 1024;
    std::vector<Entity> entities(COUNT);
    std::vector<TransformManager::Instance> instances(COUNT);
    em->create(COUNT, entities.data());

    mat4f const matT = mat4f::translation(float3{ 1.0f, 2.0f, 3.0f });
    mat4f const matTR = matT * mat4f::rotation(0.25f, float3{ 0.0f, 1.0f, 0.0f });
    mat4f const matTRS = matTR * mat4f::scaling(float3{ 1.1f, 1.1f, 1.1f });

    for (size_t i = 0; i < COUNT; ++i) {
        tcm->create(entities[i], {}, matTRS);
        instances[i] = tcm->getInstance(entities[i]);
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            for (size_t i = 0; i < COUNT; ++i) {
                tcm->setTransform(instances[i], matT);
                tcm->setTransform(instances[i], matTR);
                tcm->setTransform(instances[i], matTRS);
                mat4f const& w = tcm->getWorldTransform(instances[i]);
                benchmark::DoNotOptimize(w);
            }
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * COUNT);
    }

    for (size_t i = 0; i < COUNT; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(COUNT, entities.data());
}

BENCHMARK_F(TransformManagerFixture, setTransformTRSGetWorldTransformInterleavedHierarchy)(benchmark::State& state) {
    constexpr size_t BRANCHING = 4;
    constexpr size_t TOTAL_NODES = 1 + 4 + 16 + 64 + 256 + 1024;
    constexpr size_t INTERNAL_NODES = 1 + 4 + 16 + 64 + 256;

    std::vector<Entity> entities(TOTAL_NODES);
    std::vector<TransformManager::Instance> instances(TOTAL_NODES);
    em->create(TOTAL_NODES, entities.data());

    mat4f const matT = mat4f::translation(float3{ 1.0f, 2.0f, 3.0f });
    mat4f const matTR = matT * mat4f::rotation(0.25f, float3{ 0.0f, 1.0f, 0.0f });
    mat4f const matTRS = matTR * mat4f::scaling(float3{ 1.1f, 1.1f, 1.1f });

    tcm->create(entities[0], {}, matTRS);
    instances[0] = tcm->getInstance(entities[0]);

    size_t nextIdx = 1;
    for (size_t p = 0; p < INTERNAL_NODES; ++p) {
        for (size_t b = 0; b < BRANCHING; ++b) {
            tcm->create(entities[nextIdx], instances[p], matTRS);
            instances[nextIdx] = tcm->getInstance(entities[nextIdx]);
            ++nextIdx;
        }
    }
    downcast(tcm)->ensureWorldTransformsUpToDate();

    {
        PerformanceCounters pc(state);
        for (UTILS_UNUSED auto _ : state) {
            for (size_t i = 0; i < TOTAL_NODES; ++i) {
                tcm->setTransform(instances[i], matT);
                tcm->setTransform(instances[i], matTR);
                tcm->setTransform(instances[i], matTRS);
                mat4f const& w = tcm->getWorldTransform(instances[i]);
                benchmark::DoNotOptimize(w);
            }
            benchmark::ClobberMemory();
        }
        pc.stop();
        state.SetItemsProcessed(state.iterations() * TOTAL_NODES);
    }

    for (size_t i = 0; i < TOTAL_NODES; ++i) {
        tcm->destroy(entities[i]);
    }
    em->destroy(TOTAL_NODES, entities.data());
}
