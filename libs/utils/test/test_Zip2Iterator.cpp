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

#include <gtest/gtest.h>

#include <utils/Zip2Iterator.h>
#include <utils/StructureOfArrays.h>

#include <algorithm>
#include <array>
#include <cstdlib>

#include <stdlib.h>

using namespace utils;

TEST(Zip2IteratorTest, sort) {
    using Array = std::array<int, 10>;
    Array values;
    Array keys;

    std::generate(keys.begin(), keys.end(), std::rand);
    std::generate(values.begin(), values.end(), [&values, &keys, n=0]() mutable { return keys[n++]/2; });

    EXPECT_FALSE(std::is_sorted(keys.begin(), keys.end()));
    EXPECT_FALSE(std::is_sorted(values.begin(), values.end()));

    Zip2Iterator<Array::iterator, Array::iterator> b = { values.begin(), keys.begin() };
    Zip2Iterator<Array::iterator, Array::iterator> e = b + values.size();
    std::sort(b, e, [](auto const& lhs, auto const& rhs) { return lhs.second < rhs.second; });

    EXPECT_TRUE(std::is_sorted(keys.begin(), keys.end()));
    EXPECT_TRUE(std::is_sorted(values.begin(), values.end()));
}

enum {
    LIGHT_INSTANCE,
};

using LightSoa = utils::StructureOfArrays<
    int
>;

TEST(Zip2IteratorTest, soa) {
    LightSoa lightData;

    lightData.push_back(3);
    lightData.push_back(2);
    lightData.push_back(1);

    float distances[3] = {1.00f, 0.5f, 0.25f};

    EXPECT_EQ(lightData.size(), 3);

    Zip2Iterator<LightSoa::iterator, float*> z = { lightData.begin(), distances };

    // To sort lightData by distance, we just need to swap the first and third elements.
    // MSVC's std::sort does something akin to the following:

    // using _BidIt = Zip2Iterator<LightSoa::iterator, float*>;
    // const _BidIt _First = z;
    // const _BidIt _Last = z + 2;

    // std::iter_value_t<_BidIt> _Val = std::move(*_First);
    // *_First = std::move(*_Last);
    // *_Last = std::move(_Val);

    std::sort(z, z + lightData.size(),
          [](auto const& lhs, auto const& rhs) { return lhs.second < rhs.second; });

    auto const* instances = lightData.data<LIGHT_INSTANCE>();

    EXPECT_EQ(distances[0], 0.25f);
    EXPECT_EQ(distances[1], 0.5f);
    EXPECT_EQ(distances[2], 1.0f);

    EXPECT_EQ(instances[0], 1);
    EXPECT_EQ(instances[1], 2);
    EXPECT_EQ(instances[2], 3);
}
