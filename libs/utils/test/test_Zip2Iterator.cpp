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

#if !defined(WIN32)
    Zip2Iterator<Array::iterator, Array::iterator> b = { values.begin(), keys.begin() };
    Zip2Iterator<Array::iterator, Array::iterator> e = b + values.size();
    std::sort(b, e, [](auto const& lhs, auto const& rhs) { return lhs.second < rhs.second; });

    EXPECT_TRUE(std::is_sorted(keys.begin(), keys.end()));
    EXPECT_TRUE(std::is_sorted(values.begin(), values.end()));
#endif
}
