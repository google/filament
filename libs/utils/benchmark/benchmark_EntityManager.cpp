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

#include "../src/EntityManagerImpl.h"

#include <utils/SingleInstanceComponentManager.h>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

using namespace utils;
template <typename T>
class TestManagerBase : public SingleInstanceComponentManager<T> {
public:
    explicit TestManagerBase(EntityManager& em) noexcept
            : SingleInstanceComponentManager<T>(em, "TestManager", false) {}

    using SingleInstanceComponentManager<T>::gc;

    void removeComponents(Entity const* entities, size_t const count) noexcept {
        for (size_t i = 0; i < count; ++i) {
            SingleInstanceComponentManager<T>::removeComponent(entities[i]);
        }
    }
};
// ==================================================================================================
// Part 1: Core Entity Lifecycle (Single-Threaded)
// ==================================================================================================

static void BM_EntityManager_CreateSingular(benchmark::State& state) {
    EntityManagerImpl em;
    size_t const count = state.range(0);
    std::vector<Entity> entities(count);

    for (auto _ : state) {
        state.PauseTiming();
        em.advanceEpoch();
        em.reclaimSafeEpochs();
        state.ResumeTiming();

        for (size_t i = 0; i < count; ++i) {
            entities[i] = em.create();
        }

        state.PauseTiming();
        em.destroy(count, entities.data());
        em.flushNotifications();
        state.ResumeTiming();
    }
}
BENCHMARK(BM_EntityManager_CreateSingular)->Range(8, 8192);

static void BM_EntityManager_CreateBatched(benchmark::State& state) {
    EntityManagerImpl em;
    size_t const count = state.range(0);
    std::vector<Entity> entities(count);

    for (auto _ : state) {
        state.PauseTiming();
        em.advanceEpoch();
        em.reclaimSafeEpochs();
        state.ResumeTiming();

        em.create(count, entities.data());

        state.PauseTiming();
        em.destroy(count, entities.data());
        em.flushNotifications();
        state.ResumeTiming();
    }
}
BENCHMARK(BM_EntityManager_CreateBatched)->Range(8, 8192);

static void BM_EntityManager_DestroySingular(benchmark::State& state) {
    EntityManagerImpl em;
    size_t const count = state.range(0);
    std::vector<Entity> entities(count);

    for (auto _ : state) {
        state.PauseTiming();
        em.create(count, entities.data());
        state.ResumeTiming();

        for (size_t i = 0; i < count; ++i) {
            em.destroy(entities[i]);
        }
        em.flushNotifications();

        state.PauseTiming();
        em.advanceEpoch();
        em.reclaimSafeEpochs();
        state.ResumeTiming();
    }
}
BENCHMARK(BM_EntityManager_DestroySingular)->Range(8, 8192);

static void BM_EntityManager_DestroyBatched(benchmark::State& state) {
    EntityManagerImpl em;
    size_t const count = state.range(0);
    std::vector<Entity> entities(count);

    for (auto _ : state) {
        state.PauseTiming();
        em.create(count, entities.data());
        state.ResumeTiming();

        em.destroy(count, entities.data());
        em.flushNotifications();

        state.PauseTiming();
        em.advanceEpoch();
        em.reclaimSafeEpochs();
        state.ResumeTiming();
    }
}
BENCHMARK(BM_EntityManager_DestroyBatched)->Range(8, 8192);

// ==================================================================================================
// Part 2: Component Management & JIT Synchronization (Single-Threaded)
// ==================================================================================================

static void BM_ComponentManager_AddComponent(benchmark::State& state) {
    EntityManagerImpl em;
    struct DummyComponent { float x; };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());

    for (auto _ : state) {
        state.PauseTiming();
        TestManager cm(em);
        state.ResumeTiming();

        for (size_t i = 0; i < count; ++i) {
            cm.addComponent(entities[i]);
        }

        state.PauseTiming();
        cm.suspend();
        state.ResumeTiming();
    }

    em.destroy(count, entities.data());
}
BENCHMARK(BM_ComponentManager_AddComponent)->Range(8, 8192);

static void BM_ComponentManager_CatchupGarbage(benchmark::State& state) {
    EntityManagerImpl em;
    struct DummyComponent { float x; };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());

    for (auto _ : state) {
        state.PauseTiming();
        TestManager cm(em);
        for (size_t i = 0; i < count; ++i) {
            cm.addComponent(entities[i]);
        }
        for (size_t i = 0; i < count; i += 2) {
            em.destroy(entities[i]);
        }
        em.flushNotifications();
        em.advanceEpoch();
        state.ResumeTiming();

        cm.gc(&cm, &TestManager::removeComponent);

        state.PauseTiming();
        cm.suspend();
        em.reclaimSafeEpochs();
        state.ResumeTiming();
    }

    em.destroy(count, entities.data());
}
BENCHMARK(BM_ComponentManager_CatchupGarbage)->Range(8, 8192);

static void BM_ComponentManager_GCLargeScale(benchmark::State& state) {
    EntityManagerImpl em;
    struct DummyComponent { float x; };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);

    for (auto _ : state) {
        state.PauseTiming();
        em.create(count, entities.data());
        TestManager cm(em);
        for (size_t i = 0; i < count; ++i) {
            cm.addComponent(entities[i]);
        }
        em.destroy(count, entities.data());
        em.flushNotifications();
        em.advanceEpoch();
        em.reclaimSafeEpochs();
        state.ResumeTiming();

        cm.gc(&cm, &TestManager::removeComponents);
    }
}
BENCHMARK(BM_ComponentManager_GCLargeScale)->Range(64, 8192);

static void BM_ComponentManager_Defragmentation(benchmark::State& state) {
    EntityManagerImpl em;
    struct DummyComponent { float x; };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());

    for (auto _ : state) {
        state.PauseTiming();
        TestManager cm(em);
        for (size_t i = 0; i < count; ++i) {
            cm.addComponent(entities[i]);
        }
        std::default_random_engine rng(1337);
        std::vector<Entity> toDestroy;
        for (size_t i = 0; i < count; ++i) {
            if (rng() % 4 == 0) {
                toDestroy.push_back(entities[i]);
            }
        }
        em.destroy(toDestroy.size(), toDestroy.data());
        em.flushNotifications();
        em.advanceEpoch();
        state.ResumeTiming();

        cm.gc(&cm, &TestManager::removeComponent);

        state.PauseTiming();
        cm.suspend();
        em.reclaimSafeEpochs();
        state.ResumeTiming();
    }

    em.destroy(count, entities.data());
}
BENCHMARK(BM_ComponentManager_Defragmentation)->Range(8, 8192);

// ==================================================================================================
// Part 3: The Garbage Collector Orchestrator (Single-Threaded)
// ==================================================================================================

static void BM_EBR_AdvanceAndReclaim(benchmark::State& state) {
    size_t const count = state.range(0);
    std::vector<Entity> entities(count);

    for (auto _ : state) {
        state.PauseTiming();
        EntityManagerImpl em;
        std::atomic<uint64_t> dummyWatermark{em.getLatestEpochID()};
        em.registerWatermark(&dummyWatermark, "DummyWatermark");

        em.create(count, entities.data());
        em.destroy(count, entities.data());
        em.flushNotifications();
        state.ResumeTiming();

        em.advanceEpoch();
        dummyWatermark.store(em.getLatestEpochID(), std::memory_order_release);
        em.reclaimSafeEpochs();

        state.PauseTiming();
        em.unregisterWatermark(&dummyWatermark);
        state.ResumeTiming();
    }
}
BENCHMARK(BM_EBR_AdvanceAndReclaim)->Range(8, 8192);

// ==================================================================================================
// Part 4: Multithreaded Contention Provers
// ==================================================================================================

static void BM_MT_Contention_Create_Vs_Destroy(benchmark::State& state) {
    EntityManagerImpl em;
    constexpr size_t NUM = 10000;
    std::vector<Entity> preallocated(NUM);
    em.create(NUM, preallocated.data());

    for (auto _ : state) {
        state.PauseTiming();
        std::atomic<bool> active{true};
        state.ResumeTiming();

        std::thread t1([&em, &active]() {
            constexpr size_t BATCH = 128;
            std::vector<Entity> local(BATCH);
            while (active.load(std::memory_order_relaxed)) {
                em.create(BATCH, local.data());
                for (auto e : local) {
                    if (e) em.destroy(e);
                }
            }
        });

        std::thread t2([&em, &preallocated, &active]() {
            constexpr size_t BATCH = 128;
            while (active.load(std::memory_order_relaxed)) {
                for (size_t i = 0; i < NUM; i += BATCH) {
                    Entity localBatch[BATCH];
                    std::copy_n(&preallocated[i], BATCH, localBatch);
                    em.destroy(BATCH, localBatch);
                    em.create(BATCH, &preallocated[i]);
                }
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        active.store(false, std::memory_order_relaxed);

        t1.join();
        t2.join();

        state.PauseTiming();
        em.advanceEpoch();
        em.reclaimSafeEpochs();
        state.ResumeTiming();
    }

    em.destroy(NUM, preallocated.data());
}
BENCHMARK(BM_MT_Contention_Create_Vs_Destroy);

static void BM_MT_Contention_Systems_Vs_GC(benchmark::State& state) {
    EntityManagerImpl em;
    struct DummyComponent { float x; };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    for (auto _ : state) {
        state.PauseTiming();
        TestManager cm(em);
        std::atomic<bool> active{true};
        state.ResumeTiming();

        std::thread t1([&em, &cm, &active]() {
            constexpr size_t BATCH = 128;
            std::vector<Entity> local(BATCH);
            while (active.load(std::memory_order_relaxed)) {
                em.create(BATCH, local.data());
                for (auto e : local) {
                    cm.addComponent(e);
                }
                em.destroy(BATCH, local.data());
                em.flushNotifications();
                cm.gc(&cm, &TestManager::removeComponent);
            }
        });

        std::thread t2([&em, &active]() {
            while (active.load(std::memory_order_relaxed)) {
                em.advanceEpoch();
                std::this_thread::sleep_for(std::chrono::microseconds(500));
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        active.store(false, std::memory_order_relaxed);

        t1.join();
        t2.join();

        state.PauseTiming();
        cm.suspend();
        em.reclaimSafeEpochs();
        state.ResumeTiming();
    }
}
BENCHMARK(BM_MT_Contention_Systems_Vs_GC);

static void BM_PagedArenaBitset_PopSetBits(benchmark::State& state) {
    size_t const count = state.range(0);
    for (auto _ : state) {
        state.PauseTiming();
        PagedArenaBitset bitset;
        for (uint32_t i = 0; i < count; ++i) {
            bitset.add(i * 10);
        }
        state.ResumeTiming();

        uint32_t processedCount = 0;
        bitset.popSetBits([&](uint32_t bit) {
            processedCount++;
            if (processedCount == count / 2) {
                return false; // Stop halfway
            }
            return true; // Pop
        });

        state.PauseTiming();
        uint32_t remainderCount = 0;
        bitset.popSetBits([&](uint32_t bit) {
            remainderCount++;
            return true;
        });
        state.ResumeTiming();
    }
}
BENCHMARK(BM_PagedArenaBitset_PopSetBits)->Range(8, 8192);

