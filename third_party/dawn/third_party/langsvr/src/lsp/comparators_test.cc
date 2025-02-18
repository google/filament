// Copyright 2024 The langsvr Authors
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

#include "include/langsvr/lsp/comparators.h"
#include "langsvr/lsp/lsp.h"
#include "langsvr/lsp/printer.h"

#include "gmock/gmock.h"

namespace langsvr::lsp {
namespace {

TEST(ComparatorsTest, Position) {
    const Position pos_1_1{1, 1};
    const Position pos_1_2{1, 2};
    const Position pos_2_1{2, 1};
    const Position pos_2_2{2, 2};

    EXPECT_EQ(Compare(pos_1_1, pos_1_1), 0);
    EXPECT_EQ(Compare(pos_1_1, pos_1_2), -1);
    EXPECT_EQ(Compare(pos_1_2, pos_1_1), 1);

    EXPECT_TRUE(pos_1_1 == pos_1_1);
    EXPECT_FALSE(pos_1_1 != pos_1_1);
    EXPECT_FALSE(pos_1_1 < pos_1_1);
    EXPECT_FALSE(pos_1_1 > pos_1_1);
    EXPECT_TRUE(pos_1_1 <= pos_1_1);
    EXPECT_TRUE(pos_1_1 >= pos_1_1);

    EXPECT_FALSE(pos_1_1 == pos_1_2);
    EXPECT_TRUE(pos_1_1 != pos_1_2);
    EXPECT_TRUE(pos_1_1 < pos_1_2);
    EXPECT_FALSE(pos_1_1 > pos_1_2);
    EXPECT_TRUE(pos_1_1 <= pos_1_2);
    EXPECT_FALSE(pos_1_1 >= pos_1_2);

    EXPECT_FALSE(pos_1_2 == pos_1_1);
    EXPECT_TRUE(pos_1_2 != pos_1_1);
    EXPECT_FALSE(pos_1_2 < pos_1_1);
    EXPECT_TRUE(pos_1_2 > pos_1_1);
    EXPECT_FALSE(pos_1_2 <= pos_1_1);
    EXPECT_TRUE(pos_1_2 >= pos_1_1);

    EXPECT_FALSE(pos_1_1 == pos_2_1);
    EXPECT_TRUE(pos_1_1 != pos_2_1);
    EXPECT_TRUE(pos_1_1 < pos_2_1);
    EXPECT_FALSE(pos_1_1 > pos_2_1);
    EXPECT_TRUE(pos_1_1 <= pos_2_1);
    EXPECT_FALSE(pos_1_1 >= pos_2_1);

    EXPECT_FALSE(pos_2_1 == pos_1_1);
    EXPECT_TRUE(pos_2_1 != pos_1_1);
    EXPECT_FALSE(pos_2_1 < pos_1_1);
    EXPECT_TRUE(pos_2_1 > pos_1_1);
    EXPECT_FALSE(pos_2_1 <= pos_1_1);
    EXPECT_TRUE(pos_2_1 >= pos_1_1);

    std::array positions = {
        pos_2_1,
        pos_1_2,
        pos_1_1,
        pos_2_2,
    };
    std::sort(positions.begin(), positions.end());
    std::array positions_sorted = {
        pos_1_1,
        pos_1_2,
        pos_2_1,
        pos_2_2,
    };
    EXPECT_EQ(positions, positions_sorted);
}

TEST(ComparatorsTest, ContainsExclusive) {
    const Position pos_1_1{1, 1};
    const Position pos_1_2{1, 2};
    const Position pos_2_1{2, 1};
    const Position pos_2_2{2, 2};

    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_1_1}, pos_1_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_1_1}, pos_1_2));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_1_1}, pos_2_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_1_1}, pos_2_2));

    EXPECT_TRUE(ContainsExclusive(Range{pos_1_1, pos_1_2}, pos_1_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_1_2}, pos_1_2));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_1_2}, pos_2_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_1_2}, pos_2_2));

    EXPECT_TRUE(ContainsExclusive(Range{pos_1_1, pos_2_1}, pos_1_1));
    EXPECT_TRUE(ContainsExclusive(Range{pos_1_1, pos_2_1}, pos_1_2));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_2_1}, pos_2_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_2_1}, pos_2_2));

    EXPECT_TRUE(ContainsExclusive(Range{pos_1_1, pos_2_2}, pos_1_1));
    EXPECT_TRUE(ContainsExclusive(Range{pos_1_1, pos_2_2}, pos_1_2));
    EXPECT_TRUE(ContainsExclusive(Range{pos_1_1, pos_2_2}, pos_2_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_1, pos_2_2}, pos_2_2));

    EXPECT_FALSE(ContainsExclusive(Range{pos_1_2, pos_1_2}, pos_1_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_2, pos_1_2}, pos_1_2));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_2, pos_1_2}, pos_2_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_2, pos_1_2}, pos_2_2));

    EXPECT_FALSE(ContainsExclusive(Range{pos_1_2, pos_2_1}, pos_1_1));
    EXPECT_TRUE(ContainsExclusive(Range{pos_1_2, pos_2_1}, pos_1_2));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_2, pos_2_1}, pos_2_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_2, pos_2_1}, pos_2_2));

    EXPECT_FALSE(ContainsExclusive(Range{pos_1_2, pos_2_2}, pos_1_1));
    EXPECT_TRUE(ContainsExclusive(Range{pos_1_2, pos_2_2}, pos_1_2));
    EXPECT_TRUE(ContainsExclusive(Range{pos_1_2, pos_2_2}, pos_2_1));
    EXPECT_FALSE(ContainsExclusive(Range{pos_1_2, pos_2_2}, pos_2_2));
}

TEST(ComparatorsTest, ContainsInclusive) {
    const Position pos_1_1{1, 1};
    const Position pos_1_2{1, 2};
    const Position pos_2_1{2, 1};
    const Position pos_2_2{2, 2};

    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_1_1}, pos_1_1));
    EXPECT_FALSE(ContainsInclusive(Range{pos_1_1, pos_1_1}, pos_1_2));
    EXPECT_FALSE(ContainsInclusive(Range{pos_1_1, pos_1_1}, pos_2_1));
    EXPECT_FALSE(ContainsInclusive(Range{pos_1_1, pos_1_1}, pos_2_2));

    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_1_2}, pos_1_1));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_1_2}, pos_1_2));
    EXPECT_FALSE(ContainsInclusive(Range{pos_1_1, pos_1_2}, pos_2_1));
    EXPECT_FALSE(ContainsInclusive(Range{pos_1_1, pos_1_2}, pos_2_2));

    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_2_1}, pos_1_1));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_2_1}, pos_1_2));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_2_1}, pos_2_1));
    EXPECT_FALSE(ContainsInclusive(Range{pos_1_1, pos_2_1}, pos_2_2));

    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_2_2}, pos_1_1));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_2_2}, pos_1_2));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_2_2}, pos_2_1));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_1, pos_2_2}, pos_2_2));

    EXPECT_FALSE(ContainsInclusive(Range{pos_1_2, pos_1_2}, pos_1_1));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_2, pos_1_2}, pos_1_2));
    EXPECT_FALSE(ContainsInclusive(Range{pos_1_2, pos_1_2}, pos_2_1));
    EXPECT_FALSE(ContainsInclusive(Range{pos_1_2, pos_1_2}, pos_2_2));

    EXPECT_FALSE(ContainsInclusive(Range{pos_1_2, pos_2_1}, pos_1_1));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_2, pos_2_1}, pos_1_2));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_2, pos_2_1}, pos_2_1));
    EXPECT_FALSE(ContainsInclusive(Range{pos_1_2, pos_2_1}, pos_2_2));

    EXPECT_FALSE(ContainsInclusive(Range{pos_1_2, pos_2_2}, pos_1_1));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_2, pos_2_2}, pos_1_2));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_2, pos_2_2}, pos_2_1));
    EXPECT_TRUE(ContainsInclusive(Range{pos_1_2, pos_2_2}, pos_2_2));
}

}  // namespace
}  // namespace langsvr::lsp
