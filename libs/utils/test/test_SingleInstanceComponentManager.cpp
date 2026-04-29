#include <gtest/gtest.h>
#include <utils/SingleInstanceComponentManager.h>
#include <utils/EntityManager.h>
#include <utils/Invocable.h>
#include <utils/Slice.h>
#include <vector>

using namespace utils;

// A dummy component for testing
struct DummyComponent {
    int value = 0;
};

class TestManager : public SingleInstanceComponentManager<DummyComponent> {
public:
    using SingleInstanceComponentManager::SingleInstanceComponentManager;
    
    void setValue(Instance i, int v) {
        if (elementAt<0>(i).value != v) {
            elementAt<0>(i).value = v;
            notifyChange(getEntity(i));
        }
    }
};

TEST(SingleInstanceComponentManagerTest, BatchCallbackTest) {
    EntityManager& em = EntityManager::get();
    TestManager manager;
    
    std::vector<Entity> notifiedEntities;
    
    auto callback = [&](Slice<const Entity> entities) {
        for (auto e : entities) {
            notifiedEntities.push_back(e);
        }
    };
    
    manager.registerChangeCallback(&manager, std::move(callback));
    
    // Test addComponent (should be batched, not flushed yet)
    Entity e1 = em.create();
    auto i1 = manager.addComponent(e1);
    
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
    TestManager manager;
    
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
    TestManager manager;
    
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
