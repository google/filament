// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/utils/containers/filtered_iterator.h"

#include <vector>

#include "gmock/gmock.h"

namespace tint {
namespace {

struct IsEven {
    bool operator()(int i) const { return (i & 1) == 0; }
};

struct IsOdd {
    bool operator()(int i) const { return (i & 1) != 0; }
};

struct Never {
    bool operator()(int) const { return false; }
};

struct Always {
    bool operator()(int) const { return true; }
};

TEST(FilteredIterableTest, Vector_Odds) {
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::vector<int> even;

    for (auto v : Filter<IsOdd>(vec)) {
        even.emplace_back(v);
    }

    ASSERT_THAT(even, testing::ElementsAre(1, 3, 5));
}

TEST(FilteredIterableTest, Vector_Evens) {
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::vector<int> even;

    for (auto v : Filter<IsEven>(vec)) {
        even.emplace_back(v);
    }

    ASSERT_THAT(even, testing::ElementsAre(2, 4));
}

TEST(FilteredIterableTest, Vector_None) {
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::vector<int> none;

    for (auto v : Filter<Never>(vec)) {
        none.emplace_back(v);
    }

    ASSERT_TRUE(none.empty());
}

TEST(FilteredIterableTest, Vector_All) {
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::vector<int> all;

    for (auto v : Filter<Always>(vec)) {
        all.emplace_back(v);
    }

    ASSERT_THAT(all, testing::ElementsAre(1, 2, 3, 4, 5));
}

}  // namespace
}  // namespace tint
