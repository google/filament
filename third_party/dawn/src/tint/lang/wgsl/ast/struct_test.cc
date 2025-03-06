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

#include "src/tint/lang/wgsl/ast/struct.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"

namespace tint::ast {
namespace {

using AstStructTest = TestHelper;
using AstStructDeathTest = AstStructTest;

TEST_F(AstStructTest, Creation) {
    auto name = Sym("s");
    auto* s = Structure(name, tint::Vector{Member("a", ty.i32())});
    EXPECT_EQ(s->name->symbol, name);
    EXPECT_EQ(s->members.Length(), 1u);
    EXPECT_TRUE(s->attributes.IsEmpty());
    EXPECT_EQ(s->source.range.begin.line, 0u);
    EXPECT_EQ(s->source.range.begin.column, 0u);
    EXPECT_EQ(s->source.range.end.line, 0u);
    EXPECT_EQ(s->source.range.end.column, 0u);
}

TEST_F(AstStructDeathTest, Assert_Null_StructMember) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Structure(b.Sym("S"), tint::Vector{b.Member("a", b.ty.i32()), nullptr}, tint::Empty);
        },
        "internal compiler error");
}

TEST_F(AstStructDeathTest, Assert_Null_Attribute) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Structure(b.Sym("S"), tint::Vector{b.Member("a", b.ty.i32())},
                        tint::Vector<const Attribute*, 1>{nullptr});
        },
        "internal compiler error");
}

TEST_F(AstStructDeathTest, Assert_DifferentGenerationID_StructMember) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Structure(b1.Sym("S"), tint::Vector{b2.Member("a", b2.ty.i32())});
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
