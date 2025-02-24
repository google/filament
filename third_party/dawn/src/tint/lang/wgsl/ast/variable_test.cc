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

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using VariableTest = TestHelper;
using VariableDeathTest = VariableTest;

TEST_F(VariableTest, Creation) {
    auto* v = Var("my_var", ty.i32(), core::AddressSpace::kFunction);

    CheckIdentifier(v->name, "my_var");
    CheckIdentifier(v->declared_address_space, "function");
    EXPECT_EQ(v->declared_access, nullptr);
    CheckIdentifier(v->type, "i32");
    EXPECT_EQ(v->source.range.begin.line, 0u);
    EXPECT_EQ(v->source.range.begin.column, 0u);
    EXPECT_EQ(v->source.range.end.line, 0u);
    EXPECT_EQ(v->source.range.end.column, 0u);
}

TEST_F(VariableTest, CreationWithSource) {
    auto* v = Var(Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 5}}}, "i",
                  ty.f32(), core::AddressSpace::kPrivate, tint::Empty);

    CheckIdentifier(v->name, "i");
    CheckIdentifier(v->declared_address_space, "private");
    CheckIdentifier(v->type, "f32");
    EXPECT_EQ(v->source.range.begin.line, 27u);
    EXPECT_EQ(v->source.range.begin.column, 4u);
    EXPECT_EQ(v->source.range.end.line, 27u);
    EXPECT_EQ(v->source.range.end.column, 5u);
}

TEST_F(VariableTest, CreationEmpty) {
    auto* v = Var(Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 7}}}, "a_var",
                  ty.i32(), core::AddressSpace::kWorkgroup, tint::Empty);

    CheckIdentifier(v->name, "a_var");
    CheckIdentifier(v->declared_address_space, "workgroup");
    CheckIdentifier(v->type, "i32");
    EXPECT_EQ(v->source.range.begin.line, 27u);
    EXPECT_EQ(v->source.range.begin.column, 4u);
    EXPECT_EQ(v->source.range.end.line, 27u);
    EXPECT_EQ(v->source.range.end.column, 7u);
}

TEST_F(VariableDeathTest, Assert_Null_Name) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.Var(static_cast<Identifier*>(nullptr), b.ty.i32());
        },
        "internal compiler error");
}

TEST_F(VariableDeathTest, Assert_DifferentGenerationID_Symbol) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Var(b2.Sym("x"), b1.ty.f32());
        },
        "internal compiler error");
}

TEST_F(VariableDeathTest, Assert_DifferentGenerationID_Initializer) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Var("x", b1.ty.f32(), b2.Expr(1.2_f));
        },
        "internal compiler error");
}

TEST_F(VariableTest, WithAttributes) {
    auto* var = Var("my_var", ty.i32(), core::AddressSpace::kFunction, Location(1_u),
                    Builtin(core::BuiltinValue::kPosition), Id(1200_u));

    auto& attributes = var->attributes;
    EXPECT_TRUE(ast::HasAttribute<LocationAttribute>(attributes));
    EXPECT_TRUE(ast::HasAttribute<BuiltinAttribute>(attributes));
    EXPECT_TRUE(ast::HasAttribute<IdAttribute>(attributes));

    auto* location = GetAttribute<LocationAttribute>(attributes);
    ASSERT_NE(nullptr, location);
    ASSERT_NE(nullptr, location->expr);
    EXPECT_TRUE(location->expr->Is<IntLiteralExpression>());
}

TEST_F(VariableTest, HasBindingPoint_BothProvided) {
    auto* var = Var("my_var", ty.i32(), core::AddressSpace::kFunction, Binding(2_a), Group(1_a));
    EXPECT_TRUE(var->HasBindingPoint());
}

TEST_F(VariableTest, HasBindingPoint_NeitherProvided) {
    auto* var = Var("my_var", ty.i32(), core::AddressSpace::kFunction, tint::Empty);
    EXPECT_FALSE(var->HasBindingPoint());
}

TEST_F(VariableTest, HasBindingPoint_MissingGroupAttribute) {
    auto* var = Var("my_var", ty.i32(), core::AddressSpace::kFunction, Binding(2_a));
    EXPECT_FALSE(var->HasBindingPoint());
}

TEST_F(VariableTest, HasBindingPoint_MissingBindingAttribute) {
    auto* var = Var("my_var", ty.i32(), core::AddressSpace::kFunction, Group(1_a));
    EXPECT_FALSE(var->HasBindingPoint());
}

}  // namespace
}  // namespace tint::ast
