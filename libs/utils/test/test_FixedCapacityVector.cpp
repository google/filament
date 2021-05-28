/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <utils/FixedCapacityVector.h>

using namespace utils;

TEST(FixedCapacityVectorTest, Simple) {

    FixedCapacityVector<int> v;

    EXPECT_EQ(v.capacity(),0);
    EXPECT_EQ(v.size(), 0);
    EXPECT_TRUE(v.empty());

    v.reserve(16);

    EXPECT_EQ(v.capacity(),16);
    EXPECT_EQ(v.size(), 0);
    EXPECT_TRUE(v.empty());

    v.resize(8);

    EXPECT_EQ(v.capacity(), 16);
    EXPECT_EQ(v.size(), 8);
    EXPECT_FALSE(v.empty());

    v[0] = 0;
    v[1] = 1;
    v[2] = 2;
    v[3] = 3;

    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[1], 1);
    EXPECT_EQ(v[2], 2);
    EXPECT_EQ(v[3], 3);

    v.resize(16);

    EXPECT_EQ(v.capacity(), 16);
    EXPECT_EQ(v.size(), 16);
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[1], 1);
    EXPECT_EQ(v[2], 2);
    EXPECT_EQ(v[3], 3);

    v.resize(4);

    EXPECT_EQ(v.capacity(), 16);
    EXPECT_EQ(v.size(), 4);
    EXPECT_FALSE(v.empty());
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[1], 1);
    EXPECT_EQ(v[2], 2);
    EXPECT_EQ(v[3], 3);

#ifdef __EXCEPTIONS
    EXPECT_THROW( v.resize(17), PreconditionPanic );
#endif
}

TEST(FixedCapacityVectorTest, Constructors) {
    FixedCapacityVector<std::string> v_default;
    EXPECT_EQ(v_default.size(), 0);

    FixedCapacityVector<std::string> v_size(8);
    EXPECT_EQ(v_size.size(), 8);

    FixedCapacityVector<std::string> v_size_proto(4, "pixelflinger");
    EXPECT_EQ(v_size_proto.capacity(), 4);
    EXPECT_EQ(v_size_proto.size(), 4);
    EXPECT_EQ(v_size_proto[0], "pixelflinger");
    EXPECT_EQ(v_size_proto[1], "pixelflinger");
    EXPECT_EQ(v_size_proto[2], "pixelflinger");
    EXPECT_EQ(v_size_proto[3], "pixelflinger");

    FixedCapacityVector<std::string> v_copy(v_size_proto);
    EXPECT_EQ(v_size_proto.capacity(), 4);
    EXPECT_EQ(v_size_proto.size(), 4);
    EXPECT_EQ(v_size_proto[0], "pixelflinger");
    EXPECT_EQ(v_size_proto[1], "pixelflinger");
    EXPECT_EQ(v_size_proto[2], "pixelflinger");
    EXPECT_EQ(v_size_proto[3], "pixelflinger");
    EXPECT_EQ(v_copy.capacity(), 4);
    EXPECT_EQ(v_copy.size(), 4);
    EXPECT_EQ(v_copy[0], "pixelflinger");
    EXPECT_EQ(v_copy[1], "pixelflinger");
    EXPECT_EQ(v_copy[2], "pixelflinger");
    EXPECT_EQ(v_copy[3], "pixelflinger");

    FixedCapacityVector<std::string> v_move(std::move(v_size_proto));
    EXPECT_EQ(v_move.capacity(), 4);
    EXPECT_EQ(v_move.size(), 4);
    EXPECT_EQ(v_move[0], "pixelflinger");
    EXPECT_EQ(v_move[1], "pixelflinger");
    EXPECT_EQ(v_move[2], "pixelflinger");
    EXPECT_EQ(v_move[3], "pixelflinger");
    EXPECT_EQ(v_size_proto.capacity(), 0);
    EXPECT_EQ(v_size_proto.size(), 0);

    auto v_capacity = FixedCapacityVector<std::string>::with_capacity(4);
    EXPECT_EQ(v_capacity.capacity(), 4);
    EXPECT_EQ(v_capacity.size(), 0);
}

TEST(FixedCapacityVectorTest, Assignments) {
    FixedCapacityVector<std::string> v(4, "pixelflinger");
    FixedCapacityVector<std::string> v_assigned(10);
    FixedCapacityVector<std::string> v_move_assigned(10);

    v_assigned = v;
    EXPECT_EQ(v_assigned.capacity(), 4);
    EXPECT_EQ(v_assigned.size(), 4);
    EXPECT_EQ(v_assigned[0], "pixelflinger");
    EXPECT_EQ(v_assigned[1], "pixelflinger");
    EXPECT_EQ(v_assigned[2], "pixelflinger");
    EXPECT_EQ(v_assigned[3], "pixelflinger");
    EXPECT_EQ(v.capacity(), 4);
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], "pixelflinger");
    EXPECT_EQ(v[1], "pixelflinger");
    EXPECT_EQ(v[2], "pixelflinger");
    EXPECT_EQ(v[3], "pixelflinger");

    v_move_assigned = std::move(v);
    EXPECT_EQ(v_move_assigned.capacity(), 4);
    EXPECT_EQ(v_move_assigned.size(), 4);
    EXPECT_EQ(v_move_assigned[0], "pixelflinger");
    EXPECT_EQ(v_move_assigned[1], "pixelflinger");
    EXPECT_EQ(v_move_assigned[2], "pixelflinger");
    EXPECT_EQ(v_move_assigned[3], "pixelflinger");
}

TEST(FixedCapacityVectorTest, Access) {
    FixedCapacityVector<std::string> v(4);
    v[0] = "Hello";
    v[1] = "Filament";
    v[2] = "World";
    v[3] = "!";

    EXPECT_EQ(v[0], "Hello");
    EXPECT_EQ(v[1], "Filament");
    EXPECT_EQ(v[2], "World");
    EXPECT_EQ(v[3], "!");

    v[0] = "Bonjour";

    EXPECT_EQ(v[0], "Bonjour");
    EXPECT_EQ(v.front(), "Bonjour");
    EXPECT_EQ(v.back(), "!");

    v.back() = " !";
    EXPECT_EQ(v.back(), " !");
}

TEST(FixedCapacityVectorTest, Insertions) {
    FixedCapacityVector<std::string> v(4);
    v[0] = "Hello";
    v[1] = "Filament";
    v[2] = "World";
    v[3] = "!";

    v.reserve(8);

    EXPECT_EQ(v[0], "Hello");
    EXPECT_EQ(v[1], "Filament");
    EXPECT_EQ(v[2], "World");
    EXPECT_EQ(v[3], "!");

    v.push_back("Bonjour");

    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.back(), "Bonjour");

    v.emplace_back("le");

    EXPECT_EQ(v.size(), 6);
    EXPECT_EQ(v.back(), "le");

    v.pop_back();

    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.back(), "Bonjour");

    v.pop_back();

    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v.back(), "!");

    v.insert(v.end(), "Guten Tag");

    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.back(), "Guten Tag");

    v.insert(v.begin(), "header:");

    EXPECT_EQ(v.size(), 6);
    EXPECT_EQ(v[0], "header:");
    EXPECT_EQ(v[1], "Hello");
    EXPECT_EQ(v[2], "Filament");
    EXPECT_EQ(v[3], "World");
    EXPECT_EQ(v[4], "!");
    EXPECT_EQ(v[5], "Guten Tag");

    v.insert(v.begin(), v.back());

    EXPECT_EQ(v.size(), 7);
    EXPECT_EQ(v[0], "Guten Tag");
    EXPECT_EQ(v[1], "header:");
    EXPECT_EQ(v[2], "Hello");
    EXPECT_EQ(v[3], "Filament");
    EXPECT_EQ(v[4], "World");
    EXPECT_EQ(v[5], "!");
    EXPECT_EQ(v[6], "Guten Tag");

    v.insert(v.begin(), v.front());

    EXPECT_EQ(v.size(), 8);
    EXPECT_EQ(v[0], "Guten Tag");
    EXPECT_EQ(v[1], "Guten Tag");
    EXPECT_EQ(v[2], "header:");
    EXPECT_EQ(v[3], "Hello");
    EXPECT_EQ(v[4], "Filament");
    EXPECT_EQ(v[5], "World");
    EXPECT_EQ(v[6], "!");
    EXPECT_EQ(v[7], "Guten Tag");

    v.erase(v.begin(), v.begin() + 3);
    v.erase(v.end() - 1);

    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], "Hello");
    EXPECT_EQ(v[1], "Filament");
    EXPECT_EQ(v[2], "World");
    EXPECT_EQ(v[3], "!");

    v.clear();
    EXPECT_EQ(v.capacity(), 8);
    EXPECT_EQ(v.size(), 0);
}

