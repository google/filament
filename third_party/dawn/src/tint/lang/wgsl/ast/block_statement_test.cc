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

#include "gmock/gmock.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"

namespace tint::ast {
namespace {

using BlockStatementTest = TestHelper;
using BlockStatementDeathTest = BlockStatementTest;

TEST_F(BlockStatementTest, Creation) {
    auto* d = create<DiscardStatement>();
    auto* ptr = d;

    auto* b = create<BlockStatement>(tint::Vector{d}, tint::Empty);

    ASSERT_EQ(b->statements.Length(), 1u);
    EXPECT_EQ(b->statements[0], ptr);
    EXPECT_EQ(b->attributes.Length(), 0u);
}

TEST_F(BlockStatementTest, Creation_WithSource) {
    auto* b = create<BlockStatement>(Source{Source::Location{20, 2}}, tint::Empty, tint::Empty);
    auto src = b->source;
    EXPECT_EQ(src.range.begin.line, 20u);
    EXPECT_EQ(src.range.begin.column, 2u);
}

TEST_F(BlockStatementTest, Creation_WithAttributes) {
    auto* d = create<DiscardStatement>();
    auto* ptr = d;

    auto* attr1 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "foo");
    auto* attr2 = DiagnosticAttribute(wgsl::DiagnosticSeverity::kOff, "bar");
    auto* b = create<BlockStatement>(tint::Vector{d}, tint::Vector{attr1, attr2});

    ASSERT_EQ(b->statements.Length(), 1u);
    EXPECT_EQ(b->statements[0], ptr);
    EXPECT_THAT(b->attributes, testing::ElementsAre(attr1, attr2));
}

TEST_F(BlockStatementTest, IsBlock) {
    auto* b = create<BlockStatement>(tint::Empty, tint::Empty);
    EXPECT_TRUE(b->Is<BlockStatement>());
}

TEST_F(BlockStatementDeathTest, Assert_Null_Statement) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.create<BlockStatement>(tint::Vector<const Statement*, 1>{nullptr}, tint::Empty);
        },
        "internal compiler error");
}

TEST_F(BlockStatementDeathTest, Assert_DifferentGenerationID_Statement) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.create<BlockStatement>(tint::Vector{b2.create<DiscardStatement>()}, tint::Empty);
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
