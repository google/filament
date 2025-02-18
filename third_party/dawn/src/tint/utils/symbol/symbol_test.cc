// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/utils/symbol/symbol.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

using SymbolTest = testing::Test;

TEST_F(SymbolTest, ToStr) {
    Symbol sym(1, GenerationID::New(), "");
    EXPECT_EQ("$1", sym.to_str());
}

TEST_F(SymbolTest, CopyAssign) {
    Symbol sym1(1, GenerationID::New(), "");
    Symbol sym2;

    EXPECT_FALSE(sym2.IsValid());
    sym2 = sym1;
    EXPECT_TRUE(sym2.IsValid());
    EXPECT_EQ(sym2, sym1);
}

TEST_F(SymbolTest, Comparison) {
    auto generation_id = GenerationID::New();
    Symbol sym1(1, generation_id, "1");
    Symbol sym2(2, generation_id, "2");
    Symbol sym3(1, generation_id, "3");

    EXPECT_TRUE(sym1 == sym3);
    EXPECT_FALSE(sym1 != sym3);
    EXPECT_FALSE(sym1 == sym2);
    EXPECT_TRUE(sym1 != sym2);
    EXPECT_FALSE(sym3 == sym2);
    EXPECT_TRUE(sym3 != sym2);
}

}  // namespace
}  // namespace tint
