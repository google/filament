/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <utils/StructureOfArrays.h>
#include <math/vec4.h>

using namespace filament::math;
using namespace utils;

struct TestFloat4 : public float4 {
    using float4::float4;
    ~TestFloat4() {
        x = -1;
        y = -2;
        z = -3;
        w = -4;
    }
    friend bool operator < (TestFloat4 const& lhs, TestFloat4 const& rhs) noexcept {
        return any(lessThan(lhs, rhs));
    }
};

using SoA = utils::StructureOfArrays<float, double, TestFloat4>;

TEST(StructureOfArraysTest, Iterator) {
    SoA soa;
    soa.setCapacity(15);
    soa.resize(8);

    auto b = soa.begin();
    auto e = soa.end();

    EXPECT_LT(b, e);
    EXPECT_EQ(e - b, 8);
    EXPECT_EQ(b + 8, e);
    EXPECT_EQ(b, e - 8);

    auto first = *b;

    first.get<0>() = 3.0f;
    EXPECT_EQ(soa.elementAt<0>(0), 3.0f);

    (*(b+1)).get<0>() = 6.0f;
    EXPECT_EQ(soa.elementAt<0>(1), 6.0f);

    b[2].get<0>() = 7.0f;
    EXPECT_EQ(soa.elementAt<0>(2), 7.0f);

    swap(*b, *(b+1));
    EXPECT_EQ(soa.elementAt<0>(0), 6.0f);
    EXPECT_EQ(soa.elementAt<0>(1), 3.0f);

    swap(b[1], b[2]);
    EXPECT_EQ(soa.elementAt<0>(0), 6.0f);
    EXPECT_EQ(soa.elementAt<0>(1), 7.0f);
    EXPECT_EQ(soa.elementAt<0>(2), 3.0f);

    for (size_t i = 0; i < 8; i++) {
        soa.elementAt<0>(i) = (float)std::rand();
        soa.elementAt<1>(i) = soa.elementAt<0>(i) * 2;
        soa.elementAt<2>(i) = soa.elementAt<0>(i) * 4;
    }

    EXPECT_FALSE(std::is_sorted(soa.begin<0>(), soa.end<0>()));
    EXPECT_FALSE(std::is_sorted(soa.begin<1>(), soa.end<1>()));
    EXPECT_FALSE(std::is_sorted(soa.begin<2>(), soa.end<2>()));

    std::sort(b, e, [](auto const& lhs, auto const& rhs) {
        return lhs.template get<0>() < rhs.template get<0>();
    });

    EXPECT_TRUE(std::is_sorted(soa.begin<0>(), soa.end<0>()));
    EXPECT_TRUE(std::is_sorted(soa.begin<1>(), soa.end<1>()));
    EXPECT_TRUE(std::is_sorted(soa.begin<2>(), soa.end<2>()));
}

TEST(StructureOfArraysTest, Simple) {
    StructureOfArrays<float, double, TestFloat4> soa;
    TestFloat4 destroyedFloat4 = { -1, -2, -3, -4 };

    // use an odd capacity
    soa.setCapacity(15);
    soa.resize(8);

    for (size_t i = 0; i < 8; i++) {
        soa.elementAt<0>(i) = i;
        soa.elementAt<1>(i) = i * 2;
        soa.elementAt<2>(i) = i * 4;
    }

    size_t capacity = soa.capacity();

    // check that each array doesn't overlap the previous one
    EXPECT_TRUE((void*)soa.data<1>() >= (void*)(soa.data<0>() + capacity));
    EXPECT_TRUE((void*)soa.data<2>() >= (void*)(soa.data<1>() + capacity));

    // check that each array is aligned properly
    EXPECT_EQ(0, uintptr_t(soa.data<0>()) % alignof(float));
    EXPECT_EQ(0, uintptr_t(soa.data<1>()) % alignof(double));
    EXPECT_EQ(0, uintptr_t(soa.data<2>()) % alignof(float4));

    // check we can remove an element
    EXPECT_EQ(8, soa.size());
    soa.pop_back();
    EXPECT_EQ(7, soa.size());

    // check the destructor was called
    EXPECT_TRUE(soa.data<2>()[7] == destroyedFloat4);

    // check we can remove a few elements
    soa.resize(4);
    EXPECT_EQ(4, soa.size());

    // check the content hasn't changed
    for (size_t i = 0; i < 4; i++) {
        EXPECT_EQ(i    , soa.elementAt<0>(i));
        EXPECT_EQ(i * 2, soa.elementAt<1>(i));
        EXPECT_EQ(TestFloat4{ i * 4 }, soa.elementAt<2>(i));
    }

    // check we can add a few elements
    soa.resize(8);
    EXPECT_EQ(8, soa.size());

    // check the constructor was called
    EXPECT_TRUE(soa.data<2>()[7] == TestFloat4());

    // check the content hasn't changed
    for (size_t i = 0; i < 4; i++) {
        EXPECT_EQ(i    , soa.elementAt<0>(i));
        EXPECT_EQ(i * 2, soa.elementAt<1>(i));
        EXPECT_EQ(TestFloat4{ i * 4 }, soa.elementAt<2>(i));
    }


    // check we can change the capacity
    soa.setCapacity(31);
    EXPECT_EQ(31, soa.capacity());
    EXPECT_EQ(8, soa.size());

    // check that each array doesn't overlap the previous one
    EXPECT_TRUE((void*)soa.data<1>() >= (void*)(soa.data<0>() + capacity));
    EXPECT_TRUE((void*)soa.data<2>() >= (void*)(soa.data<1>() + capacity));

    // check that each array is aligned properly
    EXPECT_EQ(0, uintptr_t(soa.data<0>()) % alignof(float));
    EXPECT_EQ(0, uintptr_t(soa.data<1>()) % alignof(double));
    EXPECT_EQ(0, uintptr_t(soa.data<2>()) % alignof(float4));

    // check the content hasn't changed
    for (size_t i = 0; i < 4; i++) {
        EXPECT_EQ(i    , soa.elementAt<0>(i));
        EXPECT_EQ(i * 2, soa.elementAt<1>(i));
        EXPECT_EQ(TestFloat4{ i * 4 }, soa.elementAt<2>(i));
    }

    soa.push_back(0.0f, 1.0, destroyedFloat4);
    soa.push_back(0.0f, 1.0, std::move(destroyedFloat4));
}

TEST(StructureOfArraysTest, MoveOnly) {
    StructureOfArrays<float, std::unique_ptr<int32_t>> soa;
    soa.setCapacity(2);
    soa.push_back(1.0f, std::make_unique<int32_t>(1));
    soa.push_back(2.0f, std::make_unique<int32_t>(2));
    EXPECT_EQ(soa.size(), 2);
    EXPECT_EQ(*soa.elementAt<1>(0).get(), 1);
    EXPECT_EQ(*soa.elementAt<1>(1).get(), 2);
}

