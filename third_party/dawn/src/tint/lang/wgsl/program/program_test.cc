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
#include "src/tint/lang/wgsl/ast/return_statement.h"

namespace tint {
namespace {

using ProgramTest = ast::TestHelper;
using ProgramDeathTest = ProgramTest;

TEST_F(ProgramTest, Unbuilt) {
    Program program;
    EXPECT_FALSE(program.IsValid());
}

TEST_F(ProgramTest, Creation) {
    Program program(std::move(*this));
    EXPECT_EQ(program.AST().Functions().Length(), 0u);
}

TEST_F(ProgramTest, EmptyIsValid) {
    Program program(std::move(*this));
    EXPECT_TRUE(program.IsValid());
}

TEST_F(ProgramTest, IDsAreUnique) {
    Program program_a(ProgramBuilder{});
    Program program_b(ProgramBuilder{});
    Program program_c(ProgramBuilder{});
    EXPECT_NE(program_a.ID(), program_b.ID());
    EXPECT_NE(program_b.ID(), program_c.ID());
    EXPECT_NE(program_c.ID(), program_a.ID());
}

TEST_F(ProgramTest, Assert_GlobalVariable) {
    GlobalVar("var", ty.f32(), core::AddressSpace::kPrivate);

    Program program(std::move(*this));
    EXPECT_TRUE(program.IsValid());
}

TEST_F(ProgramDeathTest, Assert_NullGlobalVariable) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.AST().AddGlobalVariable(nullptr);
        },
        "internal compiler error");
}

TEST_F(ProgramDeathTest, Assert_NullTypeDecl) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.AST().AddTypeDecl(nullptr);
        },
        "internal compiler error");
}

TEST_F(ProgramDeathTest, Assert_Null_Function) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.AST().AddFunction(nullptr);
        },
        "internal compiler error");
}

TEST_F(ProgramTest, DiagnosticsMove) {
    Diagnostics().AddError(Source{}) << "an error message";

    Program program_a(std::move(*this));
    EXPECT_FALSE(program_a.IsValid());
    EXPECT_EQ(program_a.Diagnostics().Count(), 1u);
    EXPECT_EQ(program_a.Diagnostics().NumErrors(), 1u);
    EXPECT_EQ(program_a.Diagnostics().begin()->message.Plain(), "an error message");

    Program program_b(std::move(program_a));
    EXPECT_FALSE(program_b.IsValid());
    EXPECT_EQ(program_b.Diagnostics().Count(), 1u);
    EXPECT_EQ(program_b.Diagnostics().NumErrors(), 1u);
    EXPECT_EQ(program_b.Diagnostics().begin()->message.Plain(), "an error message");
}

TEST_F(ProgramTest, ReuseMovedFromVariable) {
    Program a(std::move(*this));
    EXPECT_TRUE(a.IsValid());

    Program b = std::move(a);
    EXPECT_TRUE(b.IsValid());

    a = std::move(b);
    EXPECT_TRUE(a.IsValid());
}

}  // namespace
}  // namespace tint
