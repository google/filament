// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/utils/containers/enum_set.h"

#include <vector>

#include "gmock/gmock.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint {
namespace {

using ::testing::ElementsAre;

enum class E { A = 0, B = 3, C = 7 };

StringStream& operator<<(StringStream& out, E e) {
    switch (e) {
        case E::A:
            return out << "A";
        case E::B:
            return out << "B";
        case E::C:
            return out << "C";
    }
    return out << "E(" << static_cast<uint32_t>(e) << ")";
}

TEST(EnumSetTest, ConstructEmpty) {
    EnumSet<E> set;
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
    EXPECT_TRUE(set.Empty());
}

TEST(EnumSetTest, ConstructWithSingle) {
    EnumSet<E> set(E::B);
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
    EXPECT_FALSE(set.Empty());
}

TEST(EnumSetTest, ConstructWithMultiple) {
    EnumSet<E> set(E::A, E::C);
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
    EXPECT_FALSE(set.Empty());
}

TEST(EnumSetTest, AssignSet) {
    EnumSet<E> set;
    set = EnumSet<E>(E::A, E::C);
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, AssignEnum) {
    EnumSet<E> set(E::A);
    set = E::B;
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, AddEnum) {
    EnumSet<E> set;
    set.Add(E::B);
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, RemoveEnum) {
    EnumSet<E> set(E::A, E::B);
    set.Remove(E::B);
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, AddEnums) {
    EnumSet<E> set;
    set.Add(E::B, E::C);
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, RemoveEnums) {
    EnumSet<E> set(E::A, E::B);
    set.Remove(E::C, E::B);
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, AddEnumSet) {
    EnumSet<E> set;
    set.Add(EnumSet<E>{E::B, E::C});
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, RemoveEnumSet) {
    EnumSet<E> set(E::A, E::B);
    set.Remove(EnumSet<E>{E::B, E::C});
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, Set) {
    EnumSet<E> set;
    set.Set(E::B);
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));

    set.Set(E::B, false);
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorPlusEnum) {
    EnumSet<E> set = EnumSet<E>{E::B} + E::C;
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorMinusEnum) {
    EnumSet<E> set = EnumSet<E>{E::A, E::B} - E::B;
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorPlusSet) {
    EnumSet<E> set = EnumSet<E>{E::B} + EnumSet<E>{E::B, E::C};
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_TRUE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorMinusSet) {
    EnumSet<E> set = EnumSet<E>{E::A, E::B} - EnumSet<E>{E::B, E::C};
    EXPECT_TRUE(set.Contains(E::A));
    EXPECT_FALSE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, OperatorAnd) {
    EnumSet<E> set = EnumSet<E>{E::A, E::B} & EnumSet<E>{E::B, E::C};
    EXPECT_FALSE(set.Contains(E::A));
    EXPECT_TRUE(set.Contains(E::B));
    EXPECT_FALSE(set.Contains(E::C));
}

TEST(EnumSetTest, EqualitySet) {
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) == EnumSet<E>(E::A, E::B));
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) == EnumSet<E>(E::A, E::C));
}

TEST(EnumSetTest, InequalitySet) {
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) != EnumSet<E>(E::A, E::B));
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) != EnumSet<E>(E::A, E::C));
}

TEST(EnumSetTest, EqualityEnum) {
    EXPECT_TRUE(EnumSet<E>(E::A) == E::A);
    EXPECT_FALSE(EnumSet<E>(E::B) == E::A);
    EXPECT_FALSE(EnumSet<E>(E::B) == E::C);
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) == E::A);
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) == E::B);
    EXPECT_FALSE(EnumSet<E>(E::A, E::B) == E::C);
}

TEST(EnumSetTest, InequalityEnum) {
    EXPECT_FALSE(EnumSet<E>(E::A) != E::A);
    EXPECT_TRUE(EnumSet<E>(E::B) != E::A);
    EXPECT_TRUE(EnumSet<E>(E::B) != E::C);
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) != E::A);
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) != E::B);
    EXPECT_TRUE(EnumSet<E>(E::A, E::B) != E::C);
}

TEST(EnumSetTest, HashCode) {
    EXPECT_EQ(EnumSet<E>(E::A, E::B).HashCode(), EnumSet<E>(E::A, E::B).HashCode());
}

TEST(EnumSetTest, Value) {
    EXPECT_EQ(EnumSet<E>().Value(), 0u);
    EXPECT_EQ(EnumSet<E>(E::A).Value(), 1u);
    EXPECT_EQ(EnumSet<E>(E::B).Value(), 8u);
    EXPECT_EQ(EnumSet<E>(E::C).Value(), 128u);
    EXPECT_EQ(EnumSet<E>(E::A, E::C).Value(), 129u);
}

TEST(EnumSetTest, Iterator) {
    auto set = EnumSet<E>(E::C, E::A);

    auto it = set.begin();
    EXPECT_EQ(*it, E::A);
    EXPECT_NE(it, set.end());
    ++it;
    EXPECT_EQ(*it, E::C);
    EXPECT_NE(it, set.end());
    ++it;
    EXPECT_EQ(it, set.end());
}

TEST(EnumSetTest, IteratorEmpty) {
    auto set = EnumSet<E>();
    EXPECT_EQ(set.begin(), set.end());
}

TEST(EnumSetTest, Loop) {
    auto set = EnumSet<E>(E::C, E::A);

    std::vector<E> seen;
    for (auto e : set) {
        seen.emplace_back(e);
    }

    EXPECT_THAT(seen, ElementsAre(E::A, E::C));
}

TEST(EnumSetTest, Ostream) {
    StringStream ss;
    ss << EnumSet<E>(E::A, E::C);
    EXPECT_EQ(ss.str(), "{A, C}");
}

}  // namespace
}  // namespace tint
