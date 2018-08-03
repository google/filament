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

#include <gtest/gtest.h>

#include <memory>

#include "../src/EntityManagerImpl.h"
#include <utils/NameComponentManager.h>

using namespace utils;


TEST(EntityTest, Simple) {

    EntityManagerImpl em;
    Entity entities[16];


    // check that uninitialized Entities behave properly.
    // they're all the "null" Entity and they're not alive.
    for (auto const& e : entities) {
        EXPECT_TRUE(e.isNull());
        EXPECT_FALSE(em.isAlive(e));
    }

    em.create(16, entities);

    // after creation, check that all Entities are NOT the null entity
    // and are not alive.
    for (auto const& e : entities) {
        EXPECT_FALSE(e.isNull());
        EXPECT_TRUE(em.isAlive(e));
    }

    for (size_t i=0 ; i<16 ; i++) {
        auto& e = entities[i];
        EXPECT_EQ(EntityManagerImpl::makeIdentity(0, i+1), e.getId());
    }


    // after destruction, check that the destroyed entities are still NOT the null Entity
    // but are now dead (not alive).
    em.destroy(8, entities);
    for (size_t i=0 ; i<8 ; i++) {
        auto& e = entities[i];
        EXPECT_FALSE(e.isNull());
        EXPECT_FALSE(em.isAlive(e));
    }

    // and make sure the ones that were not deleted, didn't change state
    for (size_t i=8 ; i<16 ; i++) {
        auto& e = entities[i];
        EXPECT_FALSE(e.isNull());
        EXPECT_TRUE(em.isAlive(e));
    }

    // make sure that allocating more entities doesn't reuse old indices
    Entity more[8];
    em.create(8, more);
    for (size_t i=0 ; i<8 ; i++) {
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

    cm.gc(em);
}
