#include <utils/EntityManager.h>
#include <utils/Invocable.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/Slice.h>

#include <gtest/gtest.h>

#include <vector>

using namespace utils;

// A dummy component for testing
struct DummyComponent {
    int value = 0;
};

class TestManager : public SingleInstanceComponentManager<DummyComponent> {
public:
    explicit TestManager(EntityManager& em) noexcept : SingleInstanceComponentManager<DummyComponent>(em, "TestManager", false) {}

    void setValue(Instance i, int v) {
        if (elementAt<0>(i).value != v) {
            elementAt<0>(i).value = v;
            notifyChange(getEntity(i));
        }
    }
};

TEST(SingleInstanceComponentManagerTest, BatchCallbackTest) {
    EntityManager& em = EntityManager::get();
    TestManager manager(em);

    std::vector<Entity> notifiedEntities;

    auto callback = [&](Slice<const Entity> entities) {
        for (auto e : entities) {
            notifiedEntities.push_back(e);
        }
    };

    manager.registerChangeCallback(&manager, std::move(callback));

    // Test addComponent (should be batched, not flushed yet)
    Entity e1 = em.create();
    manager.addComponent(e1);

    EXPECT_TRUE(notifiedEntities.empty());

    // Manual flush
    manager.flushNotifications();
    EXPECT_EQ(notifiedEntities.size(), 1);
    EXPECT_EQ(notifiedEntities[0], e1);

    notifiedEntities.clear();

    // Test auto-flush when full (16 elements)
    std::vector<Entity> entities;
    for (int i = 0; i < 16; ++i) {
        Entity e = em.create();
        entities.push_back(e);
        manager.addComponent(e);
    }

    // The 16th add should have triggered a flush!
    EXPECT_EQ(notifiedEntities.size(), 16);
    if constexpr (SingleInstanceComponentManagerBase::USE_SORTED_DIRTY_ARRAY) {
        std::sort(entities.begin(), entities.end());
    }
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(notifiedEntities[i], entities[i]);
    }

    notifiedEntities.clear();

    // Test change check in setValue
    auto i_last = manager.getInstance(entities.back());
    manager.setValue(i_last, 42); // Value changed
    manager.flushNotifications();
    EXPECT_EQ(notifiedEntities.size(), 1);
    EXPECT_EQ(notifiedEntities[0], entities.back());

    notifiedEntities.clear();

    manager.setValue(i_last, 42); // Value not changed
    manager.flushNotifications();
    EXPECT_TRUE(notifiedEntities.empty()); // Should not notify

    notifiedEntities.clear();

    // Test de-duplication in notifyChange
    manager.setValue(i_last, 43);
    manager.setValue(i_last, 44); // Modifying same entity again!
    manager.flushNotifications();
    EXPECT_EQ(notifiedEntities.size(), 1); // Should only appear ONCE in the batch!

    manager.unregisterChangeCallback(&manager);

    // Cleanup
    for (auto e : entities) {
        manager.removeComponent(e);
        em.destroy(e);
    }
    manager.removeComponent(e1);
    em.destroy(e1);
}

TEST(SingleInstanceComponentManagerTest, MultiCallbackTest) {
    EntityManager& em = EntityManager::get();
    TestManager manager(em);

    int count1 = 0;
    int count2 = 0;
    auto cb1 = [&](Slice<const Entity>) { count1++; };
    auto cb2 = [&](Slice<const Entity>) { count2++; };

    int token1 = 1;
    int token2 = 2;
    manager.registerChangeCallback(&token1, std::move(cb1));
    manager.registerChangeCallback(&token2, std::move(cb2));

    Entity e1 = em.create();
    manager.addComponent(e1);
    manager.flushNotifications();

    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);

    manager.unregisterChangeCallback(&token1);

    Entity e2 = em.create();
    manager.addComponent(e2);
    manager.flushNotifications();

    EXPECT_EQ(count1, 1); // Should not increase
    EXPECT_EQ(count2, 2); // Should increase

    manager.unregisterChangeCallback(&token2);

    em.destroy(e1);
    em.destroy(e2);
}

TEST(SingleInstanceComponentManagerTest, DuplicateCallbackTest) {
    EntityManager& em = EntityManager::get();
    TestManager manager(em);

    int count = 0;

    // We must create two separate lambdas because Invocable is move-only
    auto cb1 = [&](Slice<const Entity>) { count++; };
    auto cb2 = [&](Slice<const Entity>) { count++; };

    int token = 1;
    manager.registerChangeCallback(&token, std::move(cb1));
    manager.registerChangeCallback(&token, std::move(cb2));

    Entity e = em.create();
    manager.addComponent(e);
    manager.flushNotifications();

    EXPECT_EQ(count, 2); // Both should be called

    manager.unregisterChangeCallback(&token);

    Entity e2 = em.create();
    manager.addComponent(e2);
    manager.flushNotifications();

    EXPECT_EQ(count, 2); // Should not increase after unregister

    em.destroy(e);
    em.destroy(e2);
}

TEST(SingleInstanceComponentManagerTest, TryAddAndRemoveWithCallback) {
    EntityManager& em = EntityManager::get();
    TestManager manager(em);

    Entity const e1 = em.create();
    Entity const e2 = em.create();
    Entity const e3 = em.create();
    Entity const eMissing = em.create();

    // 1. tryAddComponent on new entities returns { ci, true }
    auto const [i1, added1] = manager.tryAddComponent(e1);
    EXPECT_TRUE(added1);
    EXPECT_EQ(i1, 1u);
    manager.setValue(i1, 100);

    auto const [i2, added2] = manager.tryAddComponent(e2);
    EXPECT_TRUE(added2);
    EXPECT_EQ(i2, 2u);
    manager.setValue(i2, 200);

    auto const [i3, added3] = manager.tryAddComponent(e3);
    EXPECT_TRUE(added3);
    EXPECT_EQ(i3, 3u);
    manager.setValue(i3, 300);

    // 2. tryAddComponent on an existing entity returns { existingInstance, false }
    // and preserves existing component payload
    auto const [i2Dup, added2Dup] = manager.tryAddComponent(e2);
    EXPECT_FALSE(added2Dup);
    EXPECT_EQ(i2Dup, i2);
    EXPECT_EQ(manager.elementAt<0>(i2).value, 200);
    EXPECT_EQ(manager.getComponentCount(), 3u);

    // 3. removeComponentWithCallback on a missing entity does not invoke callback and returns {0, 0}
    bool missingCalled = false;
    auto const missingRes = manager.removeComponentWithCallback(eMissing, [&](TestManager::Instance) {
        missingCalled = true;
    });
    EXPECT_FALSE(missingCalled);
    EXPECT_EQ(missingRes.removed, 0u);
    EXPECT_EQ(missingRes.moved, 0u);

    // 4. removeComponentWithCallback on e1 (slot 1) invokes preRemove(1) BEFORE moving e3 (slot 3)
    int observedPreSwapValue = -1;
    TestManager::Instance observedPreSwapInstance = 0;
    auto const rem1 = manager.removeComponentWithCallback(e1, [&](TestManager::Instance const ci) {
        observedPreSwapInstance = ci;
        observedPreSwapValue = manager.elementAt<0>(ci).value;
    });
    EXPECT_EQ(observedPreSwapInstance, 1u);
    EXPECT_EQ(observedPreSwapValue, 100);
    EXPECT_EQ(rem1.removed, 1u);
    EXPECT_EQ(rem1.moved, 3u);
    // e3 should now reside at slot 1 with value 300
    EXPECT_EQ(manager.getInstance(e3), 1u);
    EXPECT_EQ(manager.elementAt<0>(1u).value, 300);

    // 5. removeComponentsWithCallback on a batch (including already-removed e1 and missing eMissing)
    std::vector<int> batchRemovedValues;
    Entity const batch[] = { e1, e2, eMissing, e3 };
    manager.removeComponentsWithCallback(batch, 4, [&](TestManager::Instance const ci) {
        batchRemovedValues.push_back(manager.elementAt<0>(ci).value);
    });
    ASSERT_EQ(batchRemovedValues.size(), 2u);
    EXPECT_EQ(batchRemovedValues[0], 200);
    EXPECT_EQ(batchRemovedValues[1], 300);
    EXPECT_TRUE(manager.empty());

    Entity const all[] = { e1, e2, e3, eMissing };
    em.destroy(4, all);
}
