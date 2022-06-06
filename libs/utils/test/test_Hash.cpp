/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include <utils/Hash.h>

using namespace utils;

TEST(HashTest, murmur) {
    const uint32_t seed = 1701;
    const uint32_t words[3] = { 0x12345678, 0x00c0ffee, 0xbadf000d};
    const uint32_t result1 = hash::murmur3(words, 1, seed);
    const uint32_t result2 = hash::murmur3(words, 3, seed);
    const uint32_t result3 = hash::murmur3(&words[2], 1, seed);

    EXPECT_NE(result1, result2);
    EXPECT_NE(result2, result3);

    // Ensure that the byte-based hash function gives consistent results,
    // regardless of alignment and surrounding bytes.

    const uint8_t bytes_a0[] = { 0x78, 0x56, 0x34, 0x12, 0xff, 0xaa, 0xbb };
    const uint8_t bytes_a1[] = { 0xcc, 0x78, 0x56, 0x34, 0x12, 0xdd, 0xee };
    const uint8_t bytes_a2[] = { 0xff, 0xaa, 0x78, 0x56, 0x34, 0x12, 0xbb };
    const uint8_t bytes_a3[] = { 0xcc, 0xdd, 0xee, 0x78, 0x56, 0x34, 0x12 };

    const uint32_t result4 = hash::murmurSlow(&bytes_a0[0], 4, seed);
    const uint32_t result5 = hash::murmurSlow(&bytes_a1[1], 4, seed);
    const uint32_t result6 = hash::murmurSlow(&bytes_a2[2], 4, seed);
    const uint32_t result7 = hash::murmurSlow(&bytes_a3[3], 4, seed);
    const uint32_t result8 = hash::murmurSlow(&bytes_a0[0], 5, seed);

    EXPECT_EQ(result1, result4); // <== passes only on little endian machines.
    EXPECT_EQ(result4, result5);
    EXPECT_EQ(result4, result6);
    EXPECT_EQ(result4, result7);
    EXPECT_NE(result4, result8);
}
