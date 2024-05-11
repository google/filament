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

#include <utils/algorithm.h>

#include <array>

#include <stdint.h>

using namespace utils;

template <typename T>
static inline T count_leading_zeros(T v) noexcept {
    return utils::clz(v);
}

template <typename T>
static inline T count_trailing_zeros(T v) noexcept {
    return utils::ctz(v);
}

TEST(AlgorithmTest, details_clz) {
    for (uint64_t i = 1, j = 63; j < 64; i *= 2, j--) {
        EXPECT_EQ(j, details::clz(i));
        EXPECT_EQ(j, details::clz(i|1));
    }
    for (uint32_t i = 1, j = 31; j < 32; i *= 2, j--) {
        EXPECT_EQ(j, details::clz(i));
        EXPECT_EQ(j, details::clz(i|1));
    }
}

TEST(AlgorithmTest, clz) {
    for (uint64_t i = 1, j = 63; j < 64; i *= 2, j--) {
        EXPECT_EQ(j, clz(i));
        EXPECT_EQ(j, clz(i|1));
        EXPECT_EQ(j, details::clz(i));
        EXPECT_EQ(j, details::clz(i|1));
        EXPECT_EQ(j, count_leading_zeros(i));
    }
    for (uint32_t i = 1, j = 31; j < 32; i *= 2, j--) {
        EXPECT_EQ(j, clz(i));
        EXPECT_EQ(j, clz(i|1));
        EXPECT_EQ(j, details::clz(i));
        EXPECT_EQ(j, details::clz(i|1));
        EXPECT_EQ(j, count_leading_zeros(i));
    }
    for (uint16_t i = 1, j = 15; j < 16; i *= 2, j--) {
        EXPECT_EQ(j, clz(i));
        EXPECT_EQ(j, clz(uint16_t(i|1)));
        EXPECT_EQ(j, details::clz(i));
        EXPECT_EQ(j, details::clz(uint16_t(i|1)));
        EXPECT_EQ(j, count_leading_zeros(i));
    }
    for (uint8_t i = 1, j = 7; j < 8; i *= 2, j--) {
        EXPECT_EQ(j, clz(i));
        EXPECT_EQ(j, clz(uint8_t(i|1)));
        EXPECT_EQ(j, details::clz(i));
        EXPECT_EQ(j, details::clz(uint8_t(i|1)));
        EXPECT_EQ(j, count_leading_zeros(i));
    }
}

TEST(AlgorithmTest, details_ctz) {
    for (uint64_t i = 1, j = 0; j < 64; i *= 2, j++) {
        EXPECT_EQ(j, details::ctz(i));
    }
    for (uint32_t i = 1, j = 0; j < 32; i *= 2, j++) {
        EXPECT_EQ(j, details::ctz(i));
    }
}

TEST(AlgorithmTest, ctz) {
    for (uint64_t i = 1, j = 0; j < 64; i *= 2, j++) {
        EXPECT_EQ(j, ctz(i));
        EXPECT_EQ(j, details::ctz(i));
        EXPECT_EQ(j, count_trailing_zeros(i));
    }
    for (uint32_t i = 1, j = 0; j < 32; i *= 2, j++) {
        EXPECT_EQ(j, ctz(i));
        EXPECT_EQ(j, details::ctz(i));
        EXPECT_EQ(j, count_trailing_zeros(i));
    }
    for (uint16_t i = 1, j = 0; j < 16; i *= 2, j++) {
        EXPECT_EQ(j, ctz(i));
        EXPECT_EQ(j, details::ctz(i));
        EXPECT_EQ(j, count_trailing_zeros(i));
    }
    for (uint8_t i = 1, j = 0; j < 8; i *= 2, j++) {
        EXPECT_EQ(j, ctz(i));
        EXPECT_EQ(j, details::ctz(i));
        EXPECT_EQ(j, count_trailing_zeros(i));
    }
}

TEST(AlgorithmTest, details_popcount) {
    EXPECT_EQ(0, details::popcount(0u));
    EXPECT_EQ(0, details::popcount(0lu));
    EXPECT_EQ(0, details::popcount(0llu));

    EXPECT_EQ(sizeof(int)*8, details::popcount(-1u));
    EXPECT_EQ(sizeof(long)*8, details::popcount(-1lu));
    EXPECT_EQ(sizeof(long long)*8, details::popcount(-1llu));

    EXPECT_EQ(8, details::popcount(uint8_t(-1)));
    EXPECT_EQ(32, details::popcount(uint32_t(-1)));
    EXPECT_EQ(64, details::popcount(uint64_t(-1)));

    EXPECT_EQ(4, details::popcount(uint8_t(0x55)));
    EXPECT_EQ(16, details::popcount(uint32_t(0x55555555)));
    EXPECT_EQ(32, details::popcount(uint64_t(0x5555555555555555)));

    for (uint64_t i = 1, j = 63; j < 64; i *= 2, j--) {
        EXPECT_EQ(1, details::popcount(i));
    }
    for (uint32_t i = 1, j = 31; j < 32; i *= 2, j--) {
        EXPECT_EQ(1, details::popcount(i));
    }
    for (uint8_t i = 1, j = 7; j < 8; i *= 2, j--) {
        EXPECT_EQ(1, details::popcount(i));
    }
}

TEST(AlgorithmTest, popcount) {
    EXPECT_EQ(0, popcount(0u));
    EXPECT_EQ(0, popcount(0lu));
    EXPECT_EQ(0, popcount(0llu));

    EXPECT_EQ(sizeof(int)*8, popcount(-1u));
    EXPECT_EQ(sizeof(long)*8, popcount(-1lu));
    EXPECT_EQ(sizeof(long long)*8, popcount(-1llu));

    EXPECT_EQ(8, popcount(uint8_t(-1)));
    EXPECT_EQ(32, popcount(uint32_t(-1)));
    EXPECT_EQ(64, popcount(uint64_t(-1)));

    EXPECT_EQ(4, popcount(uint8_t(0x55)));
    EXPECT_EQ(16, popcount(uint32_t(0x55555555)));
    EXPECT_EQ(32, popcount(uint64_t(0x5555555555555555)));

    for (uint64_t i = 1, j = 63; i < 64; i *= 2, j--) {
        EXPECT_EQ(1, popcount(i));
    }
    for (uint32_t i = 1, j = 31; j < 32; i *= 2, j--) {
        EXPECT_EQ(1, popcount(i));
    }
    for (uint8_t i = 1, j = 7; j < 8; i *= 2, j--) {
        EXPECT_EQ(1, popcount(i));
    }
}

TEST(AlgorithmTest, Partition) {
    int* r;
    int array[8] = { 2, 5, 4, 8, 9, 9, 9, 9 };
    r = utils::partition_point(std::begin(array), std::end(array), [](int i) { return i < 9; });
    EXPECT_EQ(4, r - std::begin(array));

    r = utils::partition_point(std::begin(array), std::end(array), [](int i) { return i < 10; });
    EXPECT_EQ(std::end(array), r);

    r = utils::partition_point(std::begin(array), std::end(array), [](int i) { return i < 2; });
    EXPECT_EQ(std::begin(array), r);

    int array7[7] = { 2, 5, 4, 8, 9, 9, 9 };
    r = utils::partition_point(std::begin(array7), std::end(array7), [](int i) { return i < 9; });
    EXPECT_EQ(4, r - std::begin(array7));

    int array9[9] = { 2, 5, 4, 8, 9, 9, 9, 9, 9 };
    r = utils::partition_point(std::begin(array9), std::end(array9), [](int i) { return i < 9; });
    EXPECT_EQ(4, r - std::begin(array9));
}
