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

#include "src/tint/utils/containers/map.h"

#include <unordered_map>

#include "gtest/gtest.h"

namespace tint {
namespace {

TEST(Lookup, Test) {
    std::unordered_map<int, int> map;
    map.emplace(10, 1);
    EXPECT_EQ(Lookup(map, 10, 0), 1);    // exists, with if_missing
    EXPECT_EQ(Lookup(map, 10), 1);       // exists, without if_missing
    EXPECT_EQ(Lookup(map, 20, 50), 50);  // missing, with if_missing
    EXPECT_EQ(Lookup(map, 20), 0);       // missing, without if_missing
}

TEST(GetOrAddTest, NewKey) {
    std::unordered_map<int, int> map;
    EXPECT_EQ(GetOrAdd(map, 1, [&] { return 2; }), 2);
    EXPECT_EQ(map.size(), 1u);
    EXPECT_EQ(map[1], 2);
}

TEST(GetOrAddTest, ExistingKey) {
    std::unordered_map<int, int> map;
    map[1] = 2;
    bool called = false;
    EXPECT_EQ(GetOrAdd(map, 1,
                       [&] {
                           called = true;
                           return -2;
                       }),
              2);
    EXPECT_EQ(called, false);
    EXPECT_EQ(map.size(), 1u);
    EXPECT_EQ(map[1], 2);
}

}  // namespace
}  // namespace tint
