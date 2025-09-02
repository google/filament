// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/utils/containers/hashset.h"

#include <array>
#include <random>
#include <string>
#include <tuple>
#include <unordered_set>

#include "gmock/gmock.h"
#include "src/tint/utils/containers/predicates.h"

namespace tint {
namespace {

constexpr std::array kPrimes{
    2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,
    59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131,
    137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223,
    227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311,
    313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
};

TEST(Hashset, Empty) {
    Hashset<std::string, 8> set;
    EXPECT_EQ(set.Count(), 0u);
}

TEST(Hashset, InitializerConstructor) {
    Hashset<int, 8> set{1, 5, 7};
    EXPECT_EQ(set.Count(), 3u);
    EXPECT_TRUE(set.Contains(1));
    EXPECT_FALSE(set.Contains(3));
    EXPECT_TRUE(set.Contains(5));
    EXPECT_TRUE(set.Contains(7));
    EXPECT_FALSE(set.Contains(9));
}

TEST(Hashset, AddRemove) {
    Hashset<std::string, 8> set;
    EXPECT_TRUE(set.Add("hello"));
    EXPECT_EQ(set.Count(), 1u);
    EXPECT_TRUE(set.Contains("hello"));
    EXPECT_FALSE(set.Contains("world"));
    EXPECT_FALSE(set.Add("hello"));
    EXPECT_EQ(set.Count(), 1u);
    EXPECT_TRUE(set.Remove("hello"));
    EXPECT_EQ(set.Count(), 0u);
    EXPECT_FALSE(set.Contains("hello"));
    EXPECT_FALSE(set.Contains("world"));
}

TEST(Hashset, AddMany) {
    Hashset<int, 8> set;
    for (size_t i = 0; i < kPrimes.size(); i++) {
        int prime = kPrimes[i];
        ASSERT_TRUE(set.Add(prime)) << "i: " << i;
        ASSERT_FALSE(set.Add(prime)) << "i: " << i;
        ASSERT_EQ(set.Count(), i + 1);
    }
    ASSERT_EQ(set.Count(), kPrimes.size());
    for (int prime : kPrimes) {
        ASSERT_TRUE(set.Contains(prime)) << prime;
    }
}

TEST(Hashset, Iterator) {
    Hashset<std::string, 8> set;
    set.Add("one");
    set.Add("four");
    set.Add("three");
    set.Add("two");
    EXPECT_THAT(set, testing::UnorderedElementsAre("one", "two", "three", "four"));
}

TEST(Hashset, Vector) {
    Hashset<std::string, 8> set;
    set.Add("one");
    set.Add("four");
    set.Add("three");
    set.Add("two");
    auto vec = set.Vector();
    EXPECT_THAT(vec, testing::UnorderedElementsAre("one", "two", "three", "four"));
}

TEST(Hashset, Soak) {
    std::mt19937 rnd;
    std::unordered_set<std::string> reference;
    Hashset<std::string, 8> set;
    for (size_t i = 0; i < 1000000; i++) {
        std::string value = std::to_string(rnd() & 0x100);
        switch (rnd() % 5) {
            case 0: {  // Add
                auto expected = reference.emplace(value).second;
                ASSERT_EQ(set.Add(value), expected) << "i: " << i;
                ASSERT_TRUE(set.Contains(value)) << "i: " << i;
                break;
            }
            case 1: {  // Remove
                auto expected = reference.erase(value) != 0;
                ASSERT_EQ(set.Remove(value), expected) << "i: " << i;
                ASSERT_FALSE(set.Contains(value)) << "i: " << i;
                break;
            }
            case 2: {  // Contains
                auto expected = reference.count(value) != 0;
                ASSERT_EQ(set.Contains(value), expected) << "i: " << i;
                break;
            }
            case 3: {  // Copy / Move
                Hashset<std::string, 8> tmp(set);
                set = std::move(tmp);
                break;
            }
            case 4: {  // Clear
                reference.clear();
                set.Clear();
                ASSERT_TRUE(reference.empty());
                ASSERT_TRUE(set.IsEmpty());
                break;
            }
        }
    }
}

TEST(HashsetTest, Any) {
    Hashset<int, 8> set{1, 7, 5, 9};
    EXPECT_TRUE(set.Any(Eq(1)));
    EXPECT_FALSE(set.Any(Eq(2)));
    EXPECT_FALSE(set.Any(Eq(3)));
    EXPECT_FALSE(set.Any(Eq(4)));
    EXPECT_TRUE(set.Any(Eq(5)));
    EXPECT_FALSE(set.Any(Eq(6)));
    EXPECT_TRUE(set.Any(Eq(7)));
    EXPECT_FALSE(set.Any(Eq(8)));
    EXPECT_TRUE(set.Any(Eq(9)));
}

TEST(HashsetTest, All) {
    Hashset<int, 8> set{1, 7, 5, 9};
    EXPECT_FALSE(set.All(Ne(1)));
    EXPECT_TRUE(set.All(Ne(2)));
    EXPECT_TRUE(set.All(Ne(3)));
    EXPECT_TRUE(set.All(Ne(4)));
    EXPECT_FALSE(set.All(Ne(5)));
    EXPECT_TRUE(set.All(Ne(6)));
    EXPECT_FALSE(set.All(Ne(7)));
    EXPECT_TRUE(set.All(Ne(8)));
    EXPECT_FALSE(set.All(Ne(9)));
}

}  // namespace
}  // namespace tint
