/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <cstddef>
#include <gtest/gtest.h>

#include <utility>
#include <vector>
#include <string>
#include <numeric>

#include "../src/compact.h"

using namespace filament;

// A simple struct to test non-trivially copyable types.
struct NonTrivial {
    int value;
    std::string str;

    NonTrivial(int const v, std::string s) : value(v), str(std::move(s)) {
    }

    // Make it non-trivially copyable
    NonTrivial(const NonTrivial& other) : value(other.value), str(other.str) {
    }

    NonTrivial& operator=(const NonTrivial& other) {
        value = other.value;
        str = other.str;
        return *this;
    }

    NonTrivial(NonTrivial&& other) noexcept : value(other.value), str(std::move(other.str)) {
    }

    NonTrivial& operator=(NonTrivial&& other) noexcept {
        value = other.value;
        str = std::move(other.str);
        return *this;
    }

    bool operator==(const NonTrivial& other) const {
        return value == other.value && str == other.str;
    }
};

TEST(CompactTest, EmptyRange) {
    std::vector<int> data;
    auto equivalent = [](int const a, int const b) { return a == b; };
    auto processor = [](auto, auto, auto) {
        FAIL() << "Processor should not be called on an empty range.";
    };
    auto const end = compact<4>(data.begin(), data.end(), equivalent, processor);
    EXPECT_EQ(end, data.end());
    EXPECT_TRUE(data.empty());
}

TEST(CompactTest, NoRuns) {
    std::vector data = { 1, 2, 3, 4, 5 };
    std::vector<int> const original_data = data;
    auto equivalent = [](int const a, int const b) { return a == b; };
    auto processor = [](auto, auto, auto) {
        FAIL() << "Processor should not be called when there are no runs.";
    };
    auto const end = compact<4>(data.begin(), data.end(), equivalent, processor);
    data.erase(end, data.end());
    EXPECT_EQ(data, original_data);
}

TEST(CompactTest, SingleRun) {
    std::vector data = { 1, 2, 2, 2, 3 };
    std::vector const expected = { 1, 2, 3 };
    auto equivalent = [](int const a, int const b) { return a == b; };
    int processor_calls = 0;
    auto processor = [&](auto dst, auto chunkBegin, auto chunkEnd) {
        processor_calls++;
        EXPECT_EQ(*dst, 2);
        EXPECT_EQ(std::distance(chunkBegin, chunkEnd), 3);
    };
    auto const end = compact<4>(data.begin(), data.end(), equivalent, processor);
    data.erase(end, data.end());
    EXPECT_EQ(data, expected);
    EXPECT_EQ(processor_calls, 1);
}

TEST(CompactTest, MultipleRuns) {
    std::vector data = { 1, 2, 2, 3, 4, 4, 4, 4, 5, 5 };
    std::vector const expected = { 1, 2, 3, 4, 5 };
    auto equivalent = [](int const a, int const b) { return a == b; };
    int processor_calls = 0;
    auto processor = [&](auto dst, auto chunkBegin, auto chunkEnd) {
        processor_calls++;
        if (*dst == 2) {
            EXPECT_EQ(std::distance(chunkBegin, chunkEnd), 2);
        } else if (*dst == 4) {
            EXPECT_EQ(std::distance(chunkBegin, chunkEnd), 4);
        } else if (*dst == 5) {
            EXPECT_EQ(std::distance(chunkBegin, chunkEnd), 2);
        } else {
            FAIL() << "Unexpected value in processor: " << *dst;
        }
    };
    auto const end = compact<4>(data.begin(), data.end(), equivalent, processor);
    data.erase(end, data.end());
    EXPECT_EQ(data, expected);
    EXPECT_EQ(processor_calls, 3);
}

TEST(CompactTest, FullRun) {
    std::vector data = { 7, 7, 7, 7, 7 };
    auto equivalent = [](int const a, int const b) { return a == b; };
    int processor_calls = 0;

    // With MaxRunSize = 4, the run of 5 is broken into a chunk of 4 and a chunk of 1.
    // The processor is called for the chunk of 4. The chunk of 1 is a single element, so no call.
    // The result should contain a representative for the chunk of 4, and the element from the chunk of 1.
    auto processor_chunked = [&](auto dst, auto chunkBegin, auto chunkEnd) {
        processor_calls++;
        EXPECT_EQ(*dst, 7);
        EXPECT_EQ(std::distance(chunkBegin, chunkEnd), 4);
    };
    auto const end_chunked = compact<4>(data.begin(), data.end(), equivalent, processor_chunked);
    std::vector const actual_chunked(data.begin(), end_chunked);
    std::vector const expected_chunked = { 7, 7 };
    EXPECT_EQ(actual_chunked, expected_chunked);
    EXPECT_EQ(processor_calls, 1);

    // With MaxRunSize > 5, the run is processed as a single chunk.
    data = { 7, 7, 7, 7, 7 };
    processor_calls = 0;
    auto processor_single_chunk = [&](auto dst, auto chunkBegin, auto chunkEnd) {
        processor_calls++;
        EXPECT_EQ(*dst, 7);
        EXPECT_EQ(std::distance(chunkBegin, chunkEnd), 5);
    };
    auto const end_single_chunk = compact<8>(data.begin(), data.end(), equivalent,
            processor_single_chunk);
    std::vector const actual_single_chunk(data.begin(), end_single_chunk);
    std::vector const expected_single_chunk = { 7 };
    EXPECT_EQ(actual_single_chunk, expected_single_chunk);
    EXPECT_EQ(processor_calls, 1);
}

TEST(CompactTest, Chunking) {
    std::vector data = { 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // 9 elements
    std::vector const expected = { 1, 1, 1 };
    auto equivalent = [](int const a, int const b) { return a == b; };
    int processor_calls = 0;
    std::vector<ptrdiff_t> chunk_sizes;
    auto processor = [&](auto, auto chunkBegin, auto chunkEnd) {
        processor_calls++;
        chunk_sizes.push_back(std::distance(chunkBegin, chunkEnd));
    };
    auto const end = compact<4>(data.begin(), data.end(), equivalent, processor);
    data.erase(end, data.end());
    EXPECT_EQ(data, expected);
    EXPECT_EQ(processor_calls, 2);
    ASSERT_EQ(chunk_sizes.size(), 2);
    EXPECT_EQ(chunk_sizes[0], 4);
    EXPECT_EQ(chunk_sizes[1], 4);
}

TEST(CompactTest, NonTrivialType) {
    using NT = NonTrivial;
    std::vector<NT> data;
    data.emplace_back(1, "one");
    data.emplace_back(2, "two");
    data.emplace_back(2, "two");
    data.emplace_back(3, "three");

    std::vector<NT> expected;
    expected.emplace_back(1, "one");
    expected.emplace_back(2, "two");
    expected.emplace_back(3, "three");

    auto equivalent = [](const NT& a, const NT& b) { return a.value == b.value && a.str == b.str; };
    int processor_calls = 0;
    auto processor = [&](auto dst, auto chunkBegin, auto chunkEnd) {
        processor_calls++;
        EXPECT_EQ(dst->value, 2);
        EXPECT_EQ(std::distance(chunkBegin, chunkEnd), 2);
    };

    auto const end = compact<4>(data.begin(), data.end(), equivalent, processor);
    data.erase(end, data.end());

    EXPECT_EQ(data.size(), expected.size());
    for (size_t i = 0; i < data.size(); ++i) {
        EXPECT_EQ(data[i], expected[i]);
    }
    EXPECT_EQ(processor_calls, 1);
}

TEST(CompactTest, ProcessorModification) {
    std::vector data = { 1, 2, 2, 2, 3 };
    std::vector const expected = { 1, 99, 3 };
    auto equivalent = [](int const a, int const b) { return a == b; };
    auto processor = [&](auto dst, auto, auto) {
        *dst = 99;
    };
    auto const end = compact<4>(data.begin(), data.end(), equivalent, processor);
    data.erase(end, data.end());
    EXPECT_EQ(data, expected);
}

TEST(CompactTest, IteratorStability) {
    std::vector data = { 10, 20, 20, 20, 30, 40, 40, 50 };
    std::vector const expected = { 10, 20, 30, 40, 50 };

    auto equivalent = [](int a, int b) { return a == b; };

    // Store pointers to the elements that are expected to be stable
    std::vector<int*> stable_pointers;
    stable_pointers.push_back(&data[0]); // 10
    // The '20' will be processed, its representative will be at data[1]
    // The '30' will be moved to data[2]
    // The '40' will be processed, its representative will be at data[3]
    // The '50' will be moved to data[4]

    int processor_calls = 0;
    auto processor = [&](auto dst, auto chunkBegin, auto chunkEnd) {
        processor_calls++;
        if (*dst == 20) {
            stable_pointers.push_back(&(*dst)); // Store pointer to the representative '20'
            EXPECT_EQ(std::distance(chunkBegin, chunkEnd), 3);
            *dst = 25; // Modify the representative value
        } else if (*dst == 40) {
            stable_pointers.push_back(&(*dst)); // Store pointer to the representative '40'
            EXPECT_EQ(std::distance(chunkBegin, chunkEnd), 2);
            *dst = 45; // Modify the representative value
        }
    };

    auto const end = compact<4>(data.begin(), data.end(), equivalent, processor);
    data.erase(end, data.end());

    // After compact, the data should be {10, 25, 30, 45, 50}
    std::vector const final_expected = { 10, 25, 30, 45, 50 };
    EXPECT_EQ(data, final_expected);
    EXPECT_EQ(processor_calls, 2);

    // Verify iterator stability by checking the values through the stored pointers
    ASSERT_EQ(stable_pointers.size(), 3);
    EXPECT_EQ(*stable_pointers[0], 10); // Original 10
    EXPECT_EQ(*stable_pointers[1], 25); // Modified 20
    EXPECT_EQ(*stable_pointers[2], 45); // Modified 40
}
