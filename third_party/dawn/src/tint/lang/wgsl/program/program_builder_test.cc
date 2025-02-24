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

#include "src/tint/lang/wgsl/program/program_builder.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

using ProgramBuilderTest = testing::Test;

TEST_F(ProgramBuilderTest, IDsAreUnique) {
    Program program_a(ProgramBuilder{});
    Program program_b(ProgramBuilder{});
    Program program_c(ProgramBuilder{});
    EXPECT_NE(program_a.ID(), program_b.ID());
    EXPECT_NE(program_b.ID(), program_c.ID());
    EXPECT_NE(program_c.ID(), program_a.ID());
}

TEST_F(ProgramBuilderTest, WrapDoesntAffectInner) {
    Program inner([] {
        ProgramBuilder builder;
        auto ty = builder.ty.f32();
        builder.Func("a", {}, ty, {}, {});
        return builder;
    }());

    ASSERT_EQ(inner.AST().Functions().Length(), 1u);
    ASSERT_TRUE(inner.Symbols().Get("a").IsValid());
    ASSERT_FALSE(inner.Symbols().Get("b").IsValid());

    ProgramBuilder outer = ProgramBuilder::Wrap(inner);

    ASSERT_EQ(inner.AST().Functions().Length(), 1u);
    ASSERT_EQ(outer.AST().Functions().Length(), 1u);
    EXPECT_EQ(inner.AST().Functions()[0], outer.AST().Functions()[0]);
    EXPECT_TRUE(inner.Symbols().Get("a").IsValid());
    EXPECT_EQ(inner.Symbols().Get("a"), outer.Symbols().Get("a"));
    EXPECT_TRUE(inner.Symbols().Get("a").IsValid());
    EXPECT_TRUE(outer.Symbols().Get("a").IsValid());
    EXPECT_FALSE(inner.Symbols().Get("b").IsValid());
    EXPECT_FALSE(outer.Symbols().Get("b").IsValid());

    auto ty = outer.ty.f32();
    outer.Func("b", {}, ty, {}, {});

    ASSERT_EQ(inner.AST().Functions().Length(), 1u);
    ASSERT_EQ(outer.AST().Functions().Length(), 2u);
    EXPECT_EQ(inner.AST().Functions()[0], outer.AST().Functions()[0]);
    EXPECT_EQ(outer.AST().Functions()[1]->name->symbol, outer.Symbols().Get("b"));
    EXPECT_EQ(inner.Symbols().Get("a"), outer.Symbols().Get("a"));
    EXPECT_TRUE(inner.Symbols().Get("a").IsValid());
    EXPECT_TRUE(outer.Symbols().Get("a").IsValid());
    EXPECT_FALSE(inner.Symbols().Get("b").IsValid());
    EXPECT_TRUE(outer.Symbols().Get("b").IsValid());
}

}  // namespace
}  // namespace tint
