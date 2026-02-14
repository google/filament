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

#include <utils/GenerationalArena.h>

#include <gtest/gtest.h>

using namespace utils;

namespace {

struct Foo {
    int x;
    int y;
    Foo(int x, int y) : x(x), y(y) {}
};

struct DtorCounter {
    static int count;
    int id;
    DtorCounter(int id) : id(id) {}
    ~DtorCounter() { count++; }
};
int DtorCounter::count = 0;

} // namespace

TEST(GenerationalArenaTest, AllocationAndGet) {
    GenerationalArena<Foo> arena(10);

    auto h1 = arena.allocate(1, 2);

    Foo* f1 = arena.get(h1);
    ASSERT_NE(f1, nullptr);
    EXPECT_EQ(f1->x, 1);
    EXPECT_EQ(f1->y, 2);

    auto h2 = arena.allocate(3, 4);
    EXPECT_NE(h1, h2);

    Foo* f2 = arena.get(h2);
    ASSERT_NE(f2, nullptr);
    EXPECT_EQ(f2->x, 3);
    EXPECT_EQ(f2->y, 4);

    const GenerationalArena<Foo>& constArena = arena;
    const Foo* cf1 = constArena.get(h1);
    ASSERT_NE(cf1, nullptr);
    EXPECT_EQ(cf1->x, 1);
}

TEST(GenerationalArenaTest, FreeAndReuse) {
    GenerationalArena<Foo> arena(2);

    auto h1 = arena.allocate(10, 20);
    auto h2 = arena.allocate(30, 40);

    arena.free(h1);
    EXPECT_EQ(arena.get(h1), nullptr);
    EXPECT_NE(arena.get(h2), nullptr);

    // Reuse slot 1
    auto h3 = arena.allocate(50, 60);
    // Index should match h1 (0)
    EXPECT_EQ(h3.index, h1.index);
    // Generation should be different
    EXPECT_NE(h3.generation, h1.generation);

    Foo* f3 = arena.get(h3);
    ASSERT_NE(f3, nullptr);
    EXPECT_EQ(f3->x, 50);

    // Old handle should not work
    EXPECT_EQ(arena.get(h1), nullptr);
}

TEST(GenerationalArenaTest, DoubleFreeProtection) {
    GenerationalArena<Foo> arena(10);
    auto h1 = arena.allocate(1, 2);

    arena.free(h1);
    // Double free should be safe (no-op)
    arena.free(h1);

    // Allocate should still work correctly
    auto h2 = arena.allocate(3, 4);
}

TEST(GenerationalArenaTest, DestructorCalls) {
    DtorCounter::count = 0;
    {
        GenerationalArena<DtorCounter> arena(10);
        auto h1 = arena.allocate(1);
        auto h2 = arena.allocate(2);
        auto h3 = arena.allocate(3);

         // Destroys object 2. count becomes 1.
        arena.free(h2);
        EXPECT_EQ(DtorCounter::count, 1);
    }
    // Should destroy h1 and h3.
    EXPECT_EQ(DtorCounter::count, 3);
}

TEST(GenerationalArenaTest, InvalidGenerationAccess) {
    GenerationalArena<Foo> arena(10);
    auto h1 = arena.allocate(1, 2);

    GenerationalArena<Foo>::Handle invalidGeneration =
            { h1.generation + 1, h1.index };
    EXPECT_EQ(arena.get(invalidGeneration), nullptr);

}

#ifdef GTEST_HAS_DEATH_TEST
TEST(GenerationalArenaTest, InvalidIndexAccess) {
    GenerationalArena<Foo> arena(10);
    auto h1 = arena.allocate(1, 2);

    GenerationalArena<Foo>::Handle invalidIndex =
            { h1.generation, h1.index + 100 };
    ASSERT_DEATH(arena.get(invalidIndex), "");
}
#endif
