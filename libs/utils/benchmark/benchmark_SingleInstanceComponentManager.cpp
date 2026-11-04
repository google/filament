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

#include <utils/compiler.h>
#include <utils/Entity.h>
#include <utils/PagedArenaBitset.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Slice.h>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

using namespace utils;

namespace {

struct BenchComponent {
    uint64_t a = 0;
    uint64_t b = 0;
};

class BenchComponentManager : public SingleInstanceComponentManager<BenchComponent> {
public:
    explicit BenchComponentManager(EntityManager& em, bool const amortizationSupported) noexcept
            : SingleInstanceComponentManager<BenchComponent>(
                      em, "BenchComponentManager", amortizationSupported) {}

    // Mirrors real Filament component managers (TransformManager, RenderableManager, LightManager,
    // CameraManager, NameComponentManager) when amortizationSupported is true.
    Instance create(Entity const e) {
        Entity zombie;
        if (UTILS_UNLIKELY(popPendingZombie(e, zombie))) {
            removeComponent(zombie);
        }
        return addComponent(e);
    }

    using SingleInstanceComponentManager<BenchComponent>::gc;
    using SingleInstanceComponentManager<BenchComponent>::catchupGarbage;
    using SingleInstanceComponentManager<BenchComponent>::swap;
    using SingleInstanceComponentManager<BenchComponent>::mPendingDestruction;
};

} // namespace

// -------------------------------------------------------------------------------------------------
// 1. popPendingZombie (Empty Queue Fast Path)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_PopPendingZombie_Empty(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, true);
    Entity const e = em.create();

    for (auto _ : state) {
        Entity zombie;
        bool const hit = cm.popPendingZombie(e, zombie);
        benchmark::DoNotOptimize(hit);
        benchmark::DoNotOptimize(zombie);
    }

    em.destroy(e);
}
BENCHMARK(BM_SICM_PopPendingZombie_Empty);

// -------------------------------------------------------------------------------------------------
// 2. popPendingZombie (Pending Queue Hit Path - fetchRemove vs operator[] + remove)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_PopPendingZombie_Hit(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, true);

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());

    for (auto _ : state) {
        state.PauseTiming();
        for (size_t i = 0; i < count; ++i) {
            cm.mPendingDestruction.add(entities[i].getId());
        }
        state.ResumeTiming();

        for (size_t i = 0; i < count; ++i) {
            Entity zombie;
            bool const hit = cm.popPendingZombie(entities[i], zombie);
            benchmark::DoNotOptimize(hit);
            benchmark::DoNotOptimize(zombie);
        }
    }
    state.SetItemsProcessed(state.iterations() * count);

    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_PopPendingZombie_Hit)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 3. addComponent (Already Existing Component - Single Lookup vs Double Lookup)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_AddComponent_Existing(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    for (size_t i = 0; i < count; ++i) {
        cm.addComponent(entities[i]);
    }

    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            auto const ci = cm.addComponent(entities[i]);
            benchmark::DoNotOptimize(ci);
        }
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponents(entities.data(), count);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_AddComponent_Existing)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 4. addComponent (New Components, Pre-warmed Capacity)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_AddComponent_New(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    Entity const anchor = em.create();
    cm.addComponent(anchor);

    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    // Pre-warm SoA and robin_map capacities
    for (size_t i = 0; i < count; ++i) {
        cm.addComponent(entities[i]);
    }
    cm.removeComponents(entities.data(), count);

    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            auto const ci = cm.addComponent(entities[i]);
            benchmark::DoNotOptimize(ci);
        }

        state.PauseTiming();
        cm.removeComponents(entities.data(), count);
        state.ResumeTiming();
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponent(anchor);
    em.destroy(anchor);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_AddComponent_New)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 5. Realistic Amortized Component Creation (create = popPendingZombie + addComponent)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_Create_Amortized(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, true);

    size_t const count = state.range(0);
    Entity const anchor = em.create();
    cm.create(anchor);

    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    // Pre-warm capacities
    for (size_t i = 0; i < count; ++i) {
        cm.create(entities[i]);
    }
    cm.removeComponents(entities.data(), count);

    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            auto const ci = cm.create(entities[i]);
            benchmark::DoNotOptimize(ci);
        }

        state.PauseTiming();
        cm.removeComponents(entities.data(), count);
        state.ResumeTiming();
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponent(anchor);
    em.destroy(anchor);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_Create_Amortized)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 6. Full Create + Remove Cycle (Zero PauseTiming Overhead, Amortized Manager)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_CreateAndRemoveCycle_Amortized(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, true);

    size_t const count = state.range(0);
    Entity const anchor = em.create();
    cm.create(anchor);

    std::vector<Entity> entities(count);
    em.create(count, entities.data());

    std::vector<Entity> shuffled = entities;
    std::default_random_engine rng(42);
    std::shuffle(shuffled.begin(), shuffled.end(), rng);

    // Pre-warm capacities
    for (size_t i = 0; i < count; ++i) {
        cm.create(entities[i]);
    }
    cm.removeComponents(shuffled.data(), count);

    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            cm.create(entities[i]);
        }
        cm.removeComponents(shuffled.data(), count);
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponent(anchor);
    em.destroy(anchor);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_CreateAndRemoveCycle_Amortized)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 7. removeComponents (Shuffled Removal with Swap-and-Pop + Sparse Hash Table)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_RemoveComponents_Shuffled(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    Entity const anchor = em.create();
    cm.addComponent(anchor);

    std::vector<Entity> entities(count);
    em.create(count, entities.data());

    std::vector<Entity> shuffled = entities;
    std::default_random_engine rng(42);
    std::shuffle(shuffled.begin(), shuffled.end(), rng);

    for (auto _ : state) {
        state.PauseTiming();
        for (size_t i = 0; i < count; ++i) {
            cm.addComponent(entities[i]);
        }
        state.ResumeTiming();

        cm.removeComponents(shuffled.data(), count);
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponent(anchor);
    em.destroy(anchor);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_RemoveComponents_Shuffled)->Arg(1024)->Arg(8192);

// -------------------------------------------------------------------------------------------------
// 8. removeComponents (Sparse Hash Table - erase_fast vs erase iterator advancement)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_RemoveComponents_Sparse(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    constexpr size_t kInitialCapacity = 16384;
    size_t const sparseCount = state.range(0);

    Entity const anchor = em.create();
    cm.addComponent(anchor);

    std::vector<Entity> warmEntities(kInitialCapacity);
    em.create(kInitialCapacity, warmEntities.data());
    for (size_t i = 0; i < kInitialCapacity; ++i) {
        cm.addComponent(warmEntities[i]);
    }
    cm.removeComponents(warmEntities.data(), kInitialCapacity);

    std::default_random_engine rng(12345);
    std::shuffle(warmEntities.begin(), warmEntities.end(), rng);
    std::vector<Entity> sparseEntities(
            warmEntities.begin(), warmEntities.begin() + sparseCount);

    for (auto _ : state) {
        state.PauseTiming();
        for (size_t i = 0; i < sparseCount; ++i) {
            cm.addComponent(sparseEntities[i]);
        }
        state.ResumeTiming();

        cm.removeComponents(sparseEntities.data(), sparseCount);
    }
    state.SetItemsProcessed(state.iterations() * sparseCount);

    cm.removeComponent(anchor);
    em.destroy(anchor);
    em.destroy(kInitialCapacity, warmEntities.data());
}
BENCHMARK(BM_SICM_RemoveComponents_Sparse)->Arg(256)->Arg(1024);

// -------------------------------------------------------------------------------------------------
// 9. getInstance / hasComponent (Hit Path, Shuffled Query Order)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_GetInstance_Hit(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    for (size_t i = 0; i < count; ++i) {
        cm.addComponent(entities[i]);
    }

    std::vector<Entity> queries = entities;
    std::default_random_engine rng(42);
    std::shuffle(queries.begin(), queries.end(), rng);

    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            auto const ci = cm.getInstance(queries[i]);
            benchmark::DoNotOptimize(ci);
        }
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponents(entities.data(), count);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_GetInstance_Hit)->Arg(1024)->Arg(8192);

// -------------------------------------------------------------------------------------------------
// 10. getInstance / hasComponent (Miss Path on Populated Manager)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_GetInstance_Miss(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    std::vector<Entity> activeEntities(count);
    std::vector<Entity> missingEntities(count);
    em.create(count, activeEntities.data());
    em.create(count, missingEntities.data());

    for (size_t i = 0; i < count; ++i) {
        cm.addComponent(activeEntities[i]);
    }

    std::default_random_engine rng(42);
    std::shuffle(missingEntities.begin(), missingEntities.end(), rng);

    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            bool const has = cm.hasComponent(missingEntities[i]);
            benchmark::DoNotOptimize(has);
        }
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponents(activeEntities.data(), count);
    em.destroy(count, activeEntities.data());
    em.destroy(count, missingEntities.data());
}
BENCHMARK(BM_SICM_GetInstance_Miss)->Arg(1024)->Arg(8192);

// -------------------------------------------------------------------------------------------------
// 11. notifyChange(Entity) with Registered Bitsets (Unfiltered + Filtered)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_NotifyChange_Single_Bitset(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    for (size_t i = 0; i < count; ++i) {
        cm.addComponent(entities[i]);
    }

    PagedArenaBitset unfilteredBitset;
    PagedArenaBitset filteredBitset;
    PagedArenaBitset filterMask;
    for (size_t i = 0; i < count; i += 2) {
        filterMask.add(entities[i].getId());
    }

    cm.registerBitset(&unfilteredBitset);
    cm.registerBitset(&filteredBitset, &filterMask);

    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            cm.notifyChange(entities[i]);
        }
        benchmark::DoNotOptimize(unfilteredBitset);
        benchmark::DoNotOptimize(filteredBitset);
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.unregisterBitset(&filteredBitset);
    cm.unregisterBitset(&unfilteredBitset);
    cm.removeComponents(entities.data(), count);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_NotifyChange_Single_Bitset)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 12. notifyChange(Slice<const Entity>) Batch Notification with Registered Bitsets
// -------------------------------------------------------------------------------------------------
static void BM_SICM_NotifyChange_Slice_Bitset(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    for (size_t i = 0; i < count; ++i) {
        cm.addComponent(entities[i]);
    }

    PagedArenaBitset unfilteredBitset;
    PagedArenaBitset filteredBitset;
    PagedArenaBitset filterMask;
    for (size_t i = 0; i < count; i += 2) {
        filterMask.add(entities[i].getId());
    }

    cm.registerBitset(&unfilteredBitset);
    cm.registerBitset(&filteredBitset, &filterMask);

    Slice<const Entity> const slice(entities.data(), count);
    for (auto _ : state) {
        cm.notifyChange(slice);
        benchmark::DoNotOptimize(unfilteredBitset);
        benchmark::DoNotOptimize(filteredBitset);
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.unregisterBitset(&filteredBitset);
    cm.unregisterBitset(&unfilteredBitset);
    cm.removeComponents(entities.data(), count);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_NotifyChange_Slice_Bitset)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 13. notifyChange + registerChangeCallback + flushNotifications
// -------------------------------------------------------------------------------------------------
static void BM_SICM_NotifyChange_Callbacks(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    for (size_t i = 0; i < count; ++i) {
        cm.addComponent(entities[i]);
    }

    size_t notifiedTotal = 0;
    int const token = 1;
    cm.registerChangeCallback(&token, [&notifiedTotal](Slice<const Entity> const dirty) {
        notifiedTotal += dirty.size();
    });

    for (auto _ : state) {
        for (size_t i = 0; i < count; ++i) {
            cm.notifyChange(entities[i]);
        }
        cm.flushNotifications();
        benchmark::DoNotOptimize(notifiedTotal);
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.unregisterChangeCallback(&token);
    cm.removeComponents(entities.data(), count);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_NotifyChange_Callbacks)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 14. catchupGarbage (Clean Frame Fast-Path Early Exit)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_CatchupGarbage_Clean(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    Entity const e = em.create();
    cm.addComponent(e);
    cm.catchupGarbage();

    for (auto _ : state) {
        cm.catchupGarbage();
    }

    cm.removeComponent(e);
    em.destroy(e);
}
BENCHMARK(BM_SICM_CatchupGarbage_Clean);

// -------------------------------------------------------------------------------------------------
// 15. catchupGarbage (Dirty Epoch Intersection & Pending Destruction Merge)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_CatchupGarbage_Dirty(benchmark::State& state) {
    size_t const count = state.range(0);
    constexpr size_t kBatchSize = 16;

    std::unique_ptr<EntityManagerImpl> em;
    std::unique_ptr<BenchComponentManager> cm;
    std::vector<Entity> entities(count);
    size_t batchIter = kBatchSize;

    for (auto _ : state) {
        state.PauseTiming();
        if (batchIter == kBatchSize) {
            cm.reset();
            em = std::make_unique<EntityManagerImpl>();
            cm = std::make_unique<BenchComponentManager>(*em, true);
            Entity const anchor = em->create();
            cm->addComponent(anchor);
            batchIter = 0;
        }
        em->create(count, entities.data());
        for (size_t i = 0; i < count; ++i) {
            cm->addComponent(entities[i]);
        }
        em->destroy(count, entities.data());
        em->advanceEpoch();
        ++batchIter;
        state.ResumeTiming();

        cm->catchupGarbage();

        state.PauseTiming();
        cm->mPendingDestruction.clear();
        cm->removeComponents(entities.data(), count);
        state.ResumeTiming();
    }
    state.SetItemsProcessed(state.iterations() * count);
}
BENCHMARK(BM_SICM_CatchupGarbage_Dirty)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 16. gc (Full Synchronous Eviction, amortizationSupported = false)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_GC_Full(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    Entity const anchor = em.create();
    cm.addComponent(anchor);

    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    for (size_t i = 0; i < count; ++i) {
        cm.addComponent(entities[i]);
    }
    cm.removeComponents(entities.data(), count);

    for (auto _ : state) {
        state.PauseTiming();
        for (size_t i = 0; i < count; ++i) {
            cm.addComponent(entities[i]);
            cm.mPendingDestruction.add(entities[i].getId());
        }
        state.ResumeTiming();

        cm.gc(&cm, &BenchComponentManager::removeComponents);
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponent(anchor);
    em.destroy(anchor);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_GC_Full)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 17. gc (Amortized Eviction in Batches of 50, amortizationSupported = true)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_GC_Amortized(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, true);

    size_t const count = state.range(0);
    Entity const anchor = em.create();
    cm.create(anchor);

    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    for (size_t i = 0; i < count; ++i) {
        cm.create(entities[i]);
    }
    cm.removeComponents(entities.data(), count);

    for (auto _ : state) {
        state.PauseTiming();
        for (size_t i = 0; i < count; ++i) {
            cm.create(entities[i]);
            cm.mPendingDestruction.add(entities[i].getId());
        }
        state.ResumeTiming();

        while (!cm.mPendingDestruction.empty()) {
            cm.gc(&cm, &BenchComponentManager::removeComponents);
        }
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponent(anchor);
    em.destroy(anchor);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_GC_Amortized)->Arg(1000)->Arg(4000);

// -------------------------------------------------------------------------------------------------
// 18. swap(Instance, Instance) (Internal SoA Entity Swap + Map Update)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_Swap(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    for (size_t i = 0; i < count; ++i) {
        cm.addComponent(entities[i]);
    }

    for (auto _ : state) {
        for (size_t i = 1; i <= count / 2; ++i) {
            cm.swap(BenchComponentManager::Instance(i),
                    BenchComponentManager::Instance(count - i + 1));
        }
    }
    state.SetItemsProcessed(state.iterations() * (count / 2));

    cm.removeComponents(entities.data(), count);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_Swap)->Arg(1024)->Arg(4096);

// -------------------------------------------------------------------------------------------------
// 19. SoA Iteration (Dense Component Slice Traversal)
// -------------------------------------------------------------------------------------------------
static void BM_SICM_SoAIteration(benchmark::State& state) {
    EntityManagerImpl em;
    BenchComponentManager cm(em, false);

    size_t const count = state.range(0);
    std::vector<Entity> entities(count);
    em.create(count, entities.data());
    for (size_t i = 0; i < count; ++i) {
        auto const ci = cm.addComponent(entities[i]);
        cm.elementAt<0>(ci) = { i, i * 2 };
    }

    for (auto _ : state) {
        uint64_t sum = 0;
        for (auto const& comp : cm.slice<0>()) {
            sum += comp.a + comp.b;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * count);

    cm.removeComponents(entities.data(), count);
    em.destroy(count, entities.data());
}
BENCHMARK(BM_SICM_SoAIteration)->Arg(1024)->Arg(8192);
