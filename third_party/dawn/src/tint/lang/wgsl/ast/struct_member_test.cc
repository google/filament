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

#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::ast {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
using StructMemberTest = TestHelper;
using StructMemberDeathTest = StructMemberTest;

TEST_F(StructMemberTest, Creation) {
    auto* st = Member("a", ty.i32(), tint::Vector{MemberSize(4_a)});
    CheckIdentifier(st->name, "a");
    CheckIdentifier(st->type, "i32");
    EXPECT_EQ(st->attributes.Length(), 1u);
    EXPECT_TRUE(st->attributes[0]->Is<StructMemberSizeAttribute>());
    EXPECT_EQ(st->source.range.begin.line, 0u);
    EXPECT_EQ(st->source.range.begin.column, 0u);
    EXPECT_EQ(st->source.range.end.line, 0u);
    EXPECT_EQ(st->source.range.end.column, 0u);
}

TEST_F(StructMemberTest, CreationWithSource) {
    auto* st = Member(Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 8}}}, "a",
                      ty.i32());
    CheckIdentifier(st->name, "a");
    CheckIdentifier(st->type, "i32");
    EXPECT_EQ(st->attributes.Length(), 0u);
    EXPECT_EQ(st->source.range.begin.line, 27u);
    EXPECT_EQ(st->source.range.begin.column, 4u);
    EXPECT_EQ(st->source.range.end.line, 27u);
    EXPECT_EQ(st->source.range.end.column, 8u);
}

TEST_F(StructMemberDeathTest, Assert_Null_Name) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Member(static_cast<Identifier*>(nullptr), b.ty.i32());
        },
        "internal compiler error");
}

TEST_F(StructMemberDeathTest, Assert_Null_Type) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Member("a", Type{});
        },
        "internal compiler error");
}

TEST_F(StructMemberDeathTest, Assert_Null_Attribute) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Member("a", b.ty.i32(), tint::Vector{b.MemberSize(4_a), nullptr});
        },
        "internal compiler error");
}

TEST_F(StructMemberDeathTest, Assert_DifferentGenerationID_Symbol) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Member(b2.Sym("a"), b1.ty.i32(), tint::Vector{b1.MemberSize(4_a)});
        },
        "internal compiler error");
}

TEST_F(StructMemberDeathTest, Assert_DifferentGenerationID_Attribute) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Member("a", b1.ty.i32(), tint::Vector{b2.MemberSize(4_a)});
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
