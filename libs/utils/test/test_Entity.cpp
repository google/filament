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

#include "../src/EntityManagerImpl.h"

#include <utils/NameComponentManager.h>

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

using namespace utils;
template<typename T>
class TestManagerBase : public SingleInstanceComponentManager<T> {
public:
    explicit TestManagerBase(EntityManager& em) noexcept
            : SingleInstanceComponentManager<T>(em, "TestManager", false) {}

    PagedArenaBitset const& getEntities() const noexcept { return this->mEntities; }
    PagedArenaBitset const& getPendingDestruction() const noexcept {
        return this->mPendingDestruction;
    }

    using SingleInstanceComponentManager<T>::gc;

    void removeComponents(Entity const* entities, size_t const count) noexcept {
        for (size_t i = 0; i < count; ++i) {
            SingleInstanceComponentManager<T>::removeComponent(entities[i]);
        }
    }
};

TEST(EntityTest, Simple) {

    EntityManagerImpl em;
    Entity entities[16];


    // check that uninitialized Entities behave properly.
    // they're all the "null" Entity and they're not alive.
    for (auto const& e: entities) {
        EXPECT_TRUE(e.isNull());
        EXPECT_FALSE(em.isAlive(e));
    }

    em.create(16, entities);

    // after creation, check that all Entities are NOT the null entity
    // and are not alive.
    for (auto const& e: entities) {
        EXPECT_FALSE(e.isNull());
        EXPECT_TRUE(em.isAlive(e));
    }

    for (size_t i = 0; i < 16; i++) {
        auto& e = entities[i];
        EXPECT_EQ(EntityManagerImpl::makeIdentity(0, i + 1), e.getId());
    }


    // after destruction, check that the destroyed entities are still NOT the null Entity
    // but are now dead (not alive).
    em.destroy(8, entities);
    for (size_t i = 0; i < 8; i++) {
        auto& e = entities[i];
        EXPECT_FALSE(e.isNull());
        EXPECT_FALSE(em.isAlive(e));
    }

    // and make sure the ones that were not deleted, didn't change state
    for (size_t i = 8; i < 16; i++) {
        auto& e = entities[i];
        EXPECT_FALSE(e.isNull());
        EXPECT_TRUE(em.isAlive(e));
    }

    // make sure that allocating more entities doesn't reuse old indices
    Entity more[8];
    em.create(8, more);
    for (size_t i = 0; i < 8; i++) {
        auto& e = entities[i];
        // make sure the new ones are valid/alive
        EXPECT_FALSE(more[i].isNull());
        EXPECT_TRUE(em.isAlive(more[i]));
        // make sure they're different from the ones we already have
        // (we do that by checking they're all bigger than the last entity we created)
        EXPECT_GT(more[i].getId(), entities[15].getId());
    }
    em.destroy(8, more);
}

TEST(EntityTest, Free) {
    EntityManagerImpl em;
    Entity entities[1024];
    em.create(1024, entities);
    em.destroy(1024, entities);
    em.advanceEpoch();

    // now we have 1024 entities in the free list, we're going to start reusing them
    // using a new generation

    Entity e = em.create();
    // generation=1, index=1
    EXPECT_EQ(EntityManagerImpl::makeIdentity(1, 1), e.getId());
}

TEST(EntityTest, Lots) {
    EntityManagerImpl em;
    std::unique_ptr<Entity[]> entities(new Entity[EntityManager::getMaxEntityCount()]);
    size_t n = EntityManager::getMaxEntityCount();
    em.create(n, entities.get());

    // we can't allocate a new one at this point
    Entity e = em.create();
    EXPECT_TRUE(e.isNull());

    // see that we can free and allocate
    em.destroy(entities[0]);
    em.advanceEpoch();
    e = em.create();
    EXPECT_FALSE(e.isNull());
    EXPECT_TRUE(em.isAlive(e));
    EXPECT_NE(e.getId(), entities[0].getId());
    entities[0] = e;

    // and that we're stuck again
    e = em.create();
    EXPECT_TRUE(e.isNull());

    // and free everything
    em.destroy(n, entities.get());
    em.advanceEpoch();
    for (size_t i = 0; i < EntityManager::getMaxEntityCount(); i++) {
        EXPECT_FALSE(em.isAlive(entities[i]));
    }

    // at this point, we should be getting indices from the free-list exclusively
}


TEST(EntityTest, NameComponent) {

    EntityManagerImpl em;
    NameComponentManager cm(em);

    Entity entities[8];
    em.create(8, entities);

    cm.addComponent(entities[0]);
    EXPECT_TRUE(cm.hasComponent(entities[0]));

    cm.addComponent(entities[1]);
    EXPECT_TRUE(cm.hasComponent(entities[0]));
    EXPECT_TRUE(cm.hasComponent(entities[1]));


    auto i0 = cm.getInstance(entities[0]);
    EXPECT_EQ(1, i0.asValue());

    auto i1 = cm.getInstance(entities[1]);
    EXPECT_EQ(2, i1.asValue());

    auto i2 = cm.getInstance(entities[2]);
    EXPECT_EQ(0, i2.asValue());

    cm.setName(i0, "i0");
    EXPECT_STREQ("i0", cm.getName(i0));

    cm.setName(i1, "i1");
    EXPECT_STREQ("i0", cm.getName(i0));
    EXPECT_STREQ("i1", cm.getName(i1));

    cm.removeComponent(entities[0]);

    i0 = cm.getInstance(entities[0]);
    EXPECT_EQ(0, i0.asValue());
    i1 = cm.getInstance(entities[1]);
    EXPECT_EQ(1, i1.asValue());
    i2 = cm.getInstance(entities[2]);
    EXPECT_EQ(0, i0.asValue());

    EXPECT_STREQ("i1", cm.getName(i1));

    em.destroy(8, entities);

    cm.gc();
}

TEST(EntityTest, Callback) {
    EntityManagerImpl em;
    Entity entities[20];
    em.create(20, entities);

    std::vector<Entity> notifiedEntities;
    auto callback = [&](Slice<const Entity> slice) {
        for (auto e: slice) {
            notifiedEntities.push_back(e);
        }
    };

    em.registerChangeCallback(&em, std::move(callback));

    // Test small batch (<= 16)
    em.destroy(10, entities);
    em.flushNotifications();
    EXPECT_EQ(notifiedEntities.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(notifiedEntities[i], entities[i]);
    }

    notifiedEntities.clear();

    // Test large batch (> 16)
    Entity largeBatch[17];
    em.create(17, largeBatch);
    em.destroy(17, largeBatch);
    em.flushNotifications();
    EXPECT_EQ(notifiedEntities.size(), 17);
    for (int i = 0; i < 17; ++i) {
        EXPECT_EQ(notifiedEntities[i], largeBatch[i]);
    }

    em.unregisterChangeCallback(&em);

    // Cleanup remaining
    em.destroy(10, entities + 10);
}

TEST(EntityTest, EpochBasedReclamation) {
    auto& em = static_cast<EntityManagerImpl&>(EntityManager::get());

    struct DummyComponent {
        float x;
    };

    struct TestComponentManager : public SingleInstanceComponentManager<DummyComponent> {
        explicit TestComponentManager(EntityManager& em) noexcept
                : SingleInstanceComponentManager<DummyComponent>(em, "TestComponentManager") {}
        using SingleInstanceComponentManager::gc;
    };

    TestComponentManager cm(em);

    constexpr size_t NUM_ENTITIES = 1024;
    std::vector<Entity> entities(NUM_ENTITIES);
    em.create(NUM_ENTITIES, entities.data());

    for (auto e: entities) {
        cm.addComponent(e);
    }

    em.advanceEpoch();

    for (auto e: entities) {
        em.destroy(e);
    }

    em.advanceEpoch();

    cm.gc(&cm, &TestComponentManager::removeComponent);

    for (auto e: entities) {
        EXPECT_FALSE(cm.hasComponent(e));
    }

    cm.suspend();

    em.reclaimSafeEpochs();

    Entity reused = em.create();
    EXPECT_EQ(EntityManagerImpl::getIndex(reused), 1);

    // Cleanup
    em.destroy(reused);
}

TEST(EntityTest, EntityManager_DoubleDestroyIsSafelyIgnored) {
    EntityManagerImpl em;
    Entity A = em.create();

    em.destroy(A);
    EXPECT_FALSE(em.isAlive(A));

#if !defined(NDEBUG) && defined(GTEST_HAS_DEATH_TEST)
    // Under Debug, the second destroy triggers an assertion failure (death test)
    EXPECT_DEATH(em.destroy(A), ".*");
#else
    // Under Release, the second destroy is silently and safely ignored
    em.destroy(A);
    em.advanceEpoch();
    em.reclaimSafeEpochs();

    // Allocate again: expects a fresh Index 2 (Generation 0) due to MIN_FREE_INDICES threshold
    Entity B = em.create();
    EXPECT_EQ(EntityManagerImpl::getIndex(B), 2);
    EXPECT_EQ(EntityManagerImpl::getGeneration(B), 0);
    em.destroy(B);
#endif
}

TEST(EntityTest, EntityManager_EBR_DelaysRecyclingUntilSafe) {
    EntityManagerImpl em;
    std::atomic<uint64_t> w1{ 1 };
    em.registerWatermark(&w1, "DummyWatermark");

    constexpr size_t NUM = 1024;
    std::vector<Entity> A(NUM);
    em.create(NUM, A.data());

    for (size_t i = 0; i < NUM; ++i) {
        EXPECT_EQ(EntityManagerImpl::getIndex(A[i]), i + 1);
    }

    em.destroy(NUM, A.data());
    em.flushNotifications();
    em.advanceEpoch(); // Current epoch becomes 2, A is sealed in epoch 1

    em.reclaimSafeEpochs(); // Reclaim safe epochs. w1 is still 1, so epoch 1 is NOT safe!

    std::vector<Entity> B(NUM);
    em.create(NUM, B.data());
    // None of the new ones should be index 1, because it's held hostage!
    for (size_t i = 0; i < NUM; ++i) {
        EXPECT_NE(EntityManagerImpl::getIndex(B[i]), 1);
    }

    w1.store(2, std::memory_order_release); // Update watermark past epoch 1
    em.reclaimSafeEpochs();                 // Reclaim safe epochs. w1 is now 2, so epoch 1 is safe!

    std::vector<Entity> C(NUM);
    em.create(NUM, C.data());
    // Now index 1 should be reused with bumped generation 1!
    EXPECT_EQ(EntityManagerImpl::getIndex(C[0]), 1);
    EXPECT_EQ(EntityManagerImpl::getGeneration(C[0]), 1);

    em.unregisterWatermark(&w1);
    em.destroy(NUM, B.data());
    em.destroy(NUM, C.data());
}

TEST(EntityTest, EntityManager_ConcurrentCreateAndDestroy) {
    EntityManagerImpl em;
    constexpr size_t NUM_ENTITIES = 5000;
    std::vector<Entity> preallocated(NUM_ENTITIES);
    em.create(NUM_ENTITIES, preallocated.data());

    std::atomic<bool> active{ true };

    std::thread t1([&em, &active]() {
        std::vector<Entity> localCreated(NUM_ENTITIES);
        while (active.load(std::memory_order_relaxed)) {
            em.create(NUM_ENTITIES, localCreated.data());
            for (auto e: localCreated) {
                if (e) {
                    em.destroy(e);
                }
            }
        }
    });

    std::thread t2([&em, &preallocated, &active]() {
        while (active.load(std::memory_order_relaxed)) {
            for (size_t i = 0; i < NUM_ENTITIES; i += 10) {
                Entity localBatch[10];
                for (size_t j = 0; j < 10; ++j) {
                    localBatch[j] = preallocated[i + j];
                }
                // destroy them concurrently
                em.destroy(10, localBatch);
                // recreate them immediately to keep the slots populated
                em.create(10, &preallocated[i]);
            }
        }
    });

    std::thread t3([&em, &active]() {
        for (size_t i = 0; i < 100; ++i) {
            em.advanceEpoch();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        active.store(false, std::memory_order_relaxed);
    });

    t1.join();
    t2.join();
    t3.join();

    // Final cleanup to unblock EBR
    em.advanceEpoch();
    em.reclaimSafeEpochs();

    // Verify we can still successfully allocate
    Entity finalCheck = em.create();
    EXPECT_TRUE(finalCheck);
    em.destroy(finalCheck);
}

TEST(EntityTest, EntityManager_EBR_ConcurrentTimelineStress) {
    EntityManagerImpl em;
    std::atomic<bool> active{ true };

    // Thread 1: Generates garbage rapidly
    std::thread t1([&em, &active]() {
        std::vector<Entity> ents(128);
        while (active.load(std::memory_order_relaxed)) {
            em.create(128, ents.data());
            em.destroy(128, ents.data());
            std::this_thread::yield();
        }
    });

    // Thread 2: Advances the timeline (Allocates from bitset pool)
    std::thread t2([&em, &active]() {
        while (active.load(std::memory_order_relaxed)) {
            em.advanceEpoch();
            std::this_thread::yield();
        }
    });

    // Thread 3: Reclaims safe epochs (Returns to bitset pool)
    std::thread t3([&em, &active]() {
        while (active.load(std::memory_order_relaxed)) {
            em.reclaimSafeEpochs();
            std::this_thread::yield();
        }
    });

    // Thread 4: Registers/Unregisters watermarks (Also returns to pool and moves safe threshold)
    std::thread t4([&em, &active]() {
        std::atomic<uint64_t> watermark{ 0 };
        while (active.load(std::memory_order_relaxed)) {
            em.registerWatermark(&watermark, "StressWatermark");
            watermark.store(em.getLatestEpochID(), std::memory_order_relaxed);
            em.unregisterWatermark(&watermark);
            std::this_thread::yield();
        }
    });

    // Let them hammer the pools and locks for a brief moment
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    active.store(false, std::memory_order_relaxed);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    // Final cleanup
    em.advanceEpoch();
    em.reclaimSafeEpochs();
}

TEST(EntityTest, ComponentManager_ZombieSafelyMovedDuringDefragmentation) {
    EntityManagerImpl em;
    struct DummyComponent {
        int payload;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    TestManager cm(em);
    Entity A = em.create();
    Entity B = em.create();
    Entity C = em.create();

    auto iA = cm.addComponent(A);
    auto iB = cm.addComponent(B);
    auto iC = cm.addComponent(C);

    cm.elementAt<0>(iA).payload = 100;
    cm.elementAt<0>(iB).payload = 200;
    cm.elementAt<0>(iC).payload = 300;

    // Logically destroy C. It's now a Zombie inside cm's active array!
    em.destroy(C);

    // Manually remove component A. This will trigger a swap-and-pop compaction,
    // swapping C (the last element at index 3) into C's new home at index 1!
    cm.removeComponent(A);

    // C should be swapped into index 1 (which was A's index)
    EXPECT_EQ(cm.getInstance(C), 1);
    EXPECT_EQ(cm.elementAt<0>(1).payload, 300); // C's payload should be at index 1!

    cm.suspend();
    em.destroy(A);
    em.destroy(B);
}

TEST(EntityTest, ComponentManager_CatchupGarbageRemovesDeadEntities) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    TestManager cm(em);
    Entity A = em.create();
    cm.addComponent(A);

    EXPECT_TRUE(cm.hasComponent(A));

    em.destroy(A);
    // State hasn't been swept yet, so it's still theoretically alive in the component manager!
    EXPECT_TRUE(cm.hasComponent(A));

    em.advanceEpoch();
    // Trigger GC sweep
    cm.gc(&cm, &TestManager::removeComponent);

    // State is now cleanly caught up!
    EXPECT_FALSE(cm.hasComponent(A));

    cm.suspend();
}

TEST(EntityTest, ComponentManager_MoveSemanticsRebindWatermark) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    TestManager M1(em);
    constexpr size_t NUM = 1024;
    std::vector<Entity> A(NUM);
    em.create(NUM, A.data());
    for (auto e: A) {
        M1.addComponent(e);
    }

    em.destroy(NUM, A.data());
    em.flushNotifications();
    em.advanceEpoch(); // A is sealed in epoch 1

    // Move construct M2 from M1. This atomically rebinds the watermark!
    TestManager M2(std::move(M1));

    em.reclaimSafeEpochs(); // Reclaim safe epochs. M2's watermark is still at 1, holding it
                            // hostage!

    std::vector<Entity> B(NUM);
    em.create(NUM, B.data());
    for (size_t i = 0; i < NUM; ++i) {
        EXPECT_NE(EntityManagerImpl::getIndex(B[i]), 1);
    }

    M2.suspend();           // Suspend M2 (unregisters watermark)
    em.reclaimSafeEpochs(); // Reclaim safe epochs again.

    std::vector<Entity> C(NUM);
    em.create(NUM, C.data());
    // Now A[0]'s index 1 should be successfully recycled!
    EXPECT_EQ(EntityManagerImpl::getIndex(C[0]), 1);

    em.destroy(NUM, B.data());
    em.destroy(NUM, C.data());
}

TEST(EntityTest, Legacy_AliveEntitiesReturns32BitIDs) {
    EntityManagerImpl em;
    constexpr size_t NUM = 1024;
    std::vector<Entity> A(NUM);
    em.create(NUM, A.data());
    auto const firstId = A[0].getId();
    EXPECT_EQ(EntityManagerImpl::getIndex(A[0]), 1);
    EXPECT_EQ(EntityManagerImpl::getGeneration(A[0]), 0);

    em.destroy(NUM, A.data());
    em.flushNotifications();
    em.advanceEpoch();
    em.reclaimSafeEpochs(); // A is recycled

    std::vector<Entity> B(NUM);
    em.create(NUM, B.data());
    // The first element of B inherits A[0]'s index but has generation 1
    EXPECT_EQ(EntityManagerImpl::getIndex(B[0]), 1);
    EXPECT_EQ(EntityManagerImpl::getGeneration(B[0]), 1);

    PagedArenaBitset alive = em.getAliveEntities();

    EXPECT_TRUE(alive[B[0].getId()]);
    EXPECT_FALSE(alive[firstId]);

    em.destroy(NUM, B.data());
}

TEST(EntityTest, LegacyManager_LogicalDeathIsImmediateDespiteEBR) {
    EntityManagerImpl em;
    std::atomic<uint64_t> w1{ 1 };
    em.registerWatermark(&w1, "DummyWatermark");

    Entity A = em.create();
    EXPECT_TRUE(em.isAlive(A));
    size_t const initialCount = em.getEntityCount();

    em.destroy(A);
    em.flushNotifications();

    // Even though A is stuck in the timeline (EBR holds it hostage),
    // it MUST be logically dead to observers immediately!
    EXPECT_FALSE(em.isAlive(A));
    EXPECT_EQ(em.getEntityCount(), initialCount - 1);

    em.unregisterWatermark(&w1);
}

TEST(EntityTest, LegacyManager_ImmuneToRecycledIndexCollisions) {
    EntityManagerImpl em;
    constexpr size_t NUM = 1024;
    std::vector<Entity> entities(NUM);
    em.create(NUM, entities.data());

    Entity A = entities[0];
    auto const indexA = EntityManagerImpl::getIndex(A);
    auto const genA = EntityManagerImpl::getGeneration(A);
    EXPECT_EQ(indexA, 1);
    EXPECT_EQ(genA, 0);

    // Simulate legacy manager storing A
    Entity storedA = A;

    em.destroy(NUM, entities.data());
    em.flushNotifications();
    em.advanceEpoch();
    em.reclaimSafeEpochs(); // Recycled!

    std::vector<Entity> entities2(NUM);
    em.create(NUM, entities2.data());
    Entity B = entities2[0];

    // B reuses index 1 but has generation 1
    EXPECT_EQ(EntityManagerImpl::getIndex(B), 1);
    EXPECT_EQ(EntityManagerImpl::getGeneration(B), 1);

    // The legacy manager wakes up and queries storedA
    EXPECT_FALSE(em.isAlive(storedA));     // Immune! Stored pointer is dead!
    EXPECT_NE(storedA.getId(), B.getId()); // Mathematically isolated!

    em.destroy(NUM, entities2.data());
}

TEST(EntityTest, ComponentManager_DestructorReleasesWatermark) {
    EntityManagerImpl em;
    constexpr size_t NUM = 1024;
    auto const firstIndex = [&em]() -> uint32_t {
        struct DummyComponent {
            float x;
        };
        struct TestManager : public TestManagerBase<DummyComponent> {
            using TestManagerBase::TestManagerBase;
        };

        std::vector<Entity> A(NUM);
        em.create(NUM, A.data());
        auto const index = EntityManagerImpl::getIndex(A[0]);
        EXPECT_EQ(index, 1);

        {
            TestManager cm(em);
            cm.addComponent(A[0]);

            em.destroy(NUM, A.data());
            em.flushNotifications();
        } // Scope closed! TestManager's destructor runs, unregistering its watermark!

        return index;
    }();

    em.advanceEpoch();
    em.reclaimSafeEpochs(); // Reclaim safe epochs. Since the manager is destroyed, nothing should
                            // block it!

    std::vector<Entity> B(NUM);
    em.create(NUM, B.data());

    // Index 1 should be successfully recycled!
    EXPECT_EQ(EntityManagerImpl::getIndex(B[0]), firstIndex);

    em.destroy(NUM, B.data());
}

TEST(EntityTest, ComponentManager_TOCTOU_WatermarkRaceCondition) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;

        struct CustomDestroyContext {
            EntityManagerImpl* pEm;
            Entity entityB;
        };

        void customDestroy(Entity const* entities, size_t const count,
                CustomDestroyContext& ctx) noexcept {
            for (size_t i = 0; i < count; ++i) {
                ctx.pEm->destroy(ctx.entityB);
                ctx.pEm->flushNotifications();
                ctx.pEm->advanceEpoch(); // Global epoch advances from 1 to 2! Seals B in epoch 1.
            }
        }
    };

    TestManager cm(em);
    Entity A = em.create();
    Entity B = em.create();
    cm.addComponent(A);
    cm.addComponent(B);

    // A is destroyed in epoch 0
    em.destroy(A);
    em.flushNotifications();

    // Advance epoch so A is sealed in epoch 0. Active epoch is 1.
    em.advanceEpoch();

    // Call gc to clean up A.
    // Inside the custom destructor callback, we simulate the TOCTOU race by advancing the epoch to
    // 2 and destroying B in epoch 1!
    TestManager::CustomDestroyContext ctx{ &em, B };
    cm.gc(&cm, &TestManager::customDestroy, ctx);

    // Now B has been destroyed in epoch 1, and the global epoch is 2.
    // Due to the TOCTOU race, gc() stored 2 in mMyWatermark at the end of A's sweep.
    // In the next gc sweep, B's destruction in epoch 1 will be completely skipped!

    // Let's trigger GC again
    em.advanceEpoch(); // global epoch = 3
    em.reclaimSafeEpochs();
    cm.gc(&cm, &TestManager::removeComponent);

    // If the TOCTOU race bug is present, B was skipped and is still active in mEntities (a ghost
    // object!) This assertion will FAIL under the bug, proving the existence of the race!
    EXPECT_FALSE(cm.hasComponent(B));
}

TEST(EntityTest, ComponentManager_CatchupGarbageExtractsDeadEntities) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::catchupGarbage;
        using TestManagerBase::TestManagerBase;
    };

    TestManager cm(em);
    Entity A = em.create();
    Entity B = em.create();
    cm.addComponent(A);
    cm.addComponent(B);

    em.destroy(A);
    em.flushNotifications();
    em.advanceEpoch(); // seals Epoch 0 containing A

    EXPECT_TRUE(cm.hasComponent(A));
    EXPECT_TRUE(cm.getPendingDestruction().empty());

    cm.catchupGarbage();

    EXPECT_FALSE(cm.getEntities()[A.getId()]);
    EXPECT_TRUE(cm.getEntities()[B.getId()]);
    EXPECT_TRUE(cm.getPendingDestruction()[A.getId()]);
    EXPECT_EQ(cm.getWatermark(), em.getLatestEpochID() - 1);

    em.destroy(B);
}

TEST(EntityTest, ComponentManager_CatchupGarbageLeapsOverCleanFrames) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::catchupGarbage;
        using TestManagerBase::TestManagerBase;
    };

    TestManager cm(em);
    Entity dummy = em.create();
    cm.addComponent(dummy);

    // Loop 5 times calling em.advanceEpoch() without destroying anything
    for (size_t i = 0; i < 5; ++i) {
        em.advanceEpoch();
    }

    cm.catchupGarbage();

    EXPECT_TRUE(cm.getPendingDestruction().empty());
    EXPECT_EQ(cm.getWatermark(), 5); // leaps forward to currentEpoch - 1 (which is 5)

    em.destroy(dummy);
}

TEST(EntityTest, ComponentManager_CatchupGarbageNeverSkipsActiveEpoch) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::catchupGarbage;
        using TestManagerBase::TestManagerBase;
    };

    TestManager cm(em);
    Entity dummy = em.create();
    cm.addComponent(dummy);

    em.destroy(dummy);
    em.flushNotifications();
    // Crucial: Do NOT call em.advanceEpoch(). Dummy's destruction is unsealed in Epoch 1.

    cm.catchupGarbage();

    EXPECT_TRUE(cm.getPendingDestruction().empty());
    EXPECT_EQ(cm.getWatermark(), 0); // Watermark stays behind at 0!

    em.advanceEpoch();
}

TEST(EntityTest, ComponentManager_GCLargeScaleEviction) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    TestManager cm(em);
    constexpr size_t NUM = 1024;
    std::vector<Entity> entities(NUM);
    em.create(NUM, entities.data());

    for (auto e: entities) {
        cm.addComponent(e);
    }

    em.destroy(NUM, entities.data());
    em.flushNotifications();
    em.advanceEpoch();
    em.reclaimSafeEpochs();

    // Verify all components are physically active prior to gc()
    EXPECT_EQ(cm.getComponentCount(), NUM);

    cm.gc(&cm, &TestManager::removeComponents);

    // Verify all components were cleanly and physically purged
    EXPECT_EQ(cm.getComponentCount(), 0);
}


TEST(EntityTest, PagedArenaBitset_PopSetBits) {
    PagedArenaBitset bitset;
    for (uint32_t i = 0; i < 10; ++i) {
        bitset.add(i * 10);
    }
    EXPECT_EQ(bitset.size(), 10);

    uint32_t processedCount = 0;
    bitset.popSetBits([&](uint32_t bit) {
        processedCount++;
        if (processedCount == 5) {
            return false; // Stop!
        }
        return true; // Pop
    });

    EXPECT_EQ(processedCount, 5);
    EXPECT_EQ(bitset.size(), 6); // 10 originally - 4 popped successfully = 6 remaining!

    // Process the remainder
    uint32_t remainderCount = 0;
    bitset.popSetBits([&](uint32_t bit) {
        remainderCount++;
        return true;
    });

    EXPECT_EQ(remainderCount, 6);
    EXPECT_TRUE(bitset.empty());
}

TEST(EntityTest, ComponentManager_AmortizedGC) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public SingleInstanceComponentManager<DummyComponent> {
        explicit TestManager(EntityManager& em) noexcept
                : SingleInstanceComponentManager<DummyComponent>(em, "TestManager", true) {}
        using SingleInstanceComponentManager<DummyComponent>::gc;
    };

    TestManager cm(em);

    std::vector<Entity> entities(10);
    em.create(10, entities.data());

    for (auto e: entities) {
        cm.addComponent(e);
    }

    em.advanceEpoch();

    for (auto e: entities) {
        em.destroy(e);
    }

    em.advanceEpoch();

    // GC exactly 3 components this frame
    cm.gc(&cm, 3, &TestManager::removeComponent);

    uint32_t aliveCount = 0;
    for (auto e: entities) {
        if (cm.hasComponent(e)) {
            aliveCount++;
        }
    }
    EXPECT_EQ(aliveCount, 7);

    // GC the remaining 7
    cm.gc(&cm, 10, &TestManager::removeComponent);

    aliveCount = 0;
    for (auto e: entities) {
        if (cm.hasComponent(e)) {
            aliveCount++;
        }
    }
    EXPECT_EQ(aliveCount, 0);
}

TEST(EntityTest, ComponentManager_AutomatedLeapForward_LeapsStaticManagers) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    TestManager cm(em);
    Entity A = em.create();
    Entity B = em.create();
    cm.addComponent(A);

    em.destroy(B);
    em.flushNotifications();
    em.advanceEpoch(); // Seals Epoch 0 containing B (which doesn't intersect cm)

    // Verify cm's watermark is automatically caught up to Epoch 1 without calling gc/catchup!
    EXPECT_EQ(cm.getWatermark(), 1);

    em.destroy(A);
}

TEST(EntityTest, ComponentManager_AutomatedLeapForward_BlocksOnIntersection) {
    EntityManagerImpl em;
    struct DummyComponent {
        float x;
    };
    struct TestManager : public TestManagerBase<DummyComponent> {
        using TestManagerBase::TestManagerBase;
    };

    TestManager cm(em);
    Entity A = em.create();
    cm.addComponent(A);

    em.destroy(A);
    em.flushNotifications();
    em.advanceEpoch(); // Seals Epoch 0 containing A (which intersects cm)

    // Verify cm's watermark is BLOCKED at 0 to prevent ID recycling before gc runs
    EXPECT_EQ(cm.getWatermark(), 0);

    // Call gc to purge A
    cm.gc(&cm, &TestManager::removeComponent);

    // Now it should be advanced to Epoch 1
    EXPECT_EQ(cm.getWatermark(), 1);
}
