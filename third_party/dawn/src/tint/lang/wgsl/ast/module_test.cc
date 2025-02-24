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
#include "src/tint/lang/wgsl/ast/helper_test.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::ast {
namespace {

using ModuleTest = TestHelper;
using ModuleDeathTest = ModuleTest;

TEST_F(ModuleTest, Creation) {
    EXPECT_EQ(resolver::Resolve(*this).AST().Functions().Length(), 0u);
}

TEST_F(ModuleTest, LookupFunction) {
    auto* func = Func("main", {}, ty.f32(), {});

    Program program(std::move(*this));
    EXPECT_EQ(func, program.AST().Functions().Find(program.Symbols().Get("main")));
}

TEST_F(ModuleTest, LookupFunctionMissing) {
    Program program(std::move(*this));
    EXPECT_EQ(nullptr, program.AST().Functions().Find(program.Symbols().Get("Missing")));
}

TEST_F(ModuleDeathTest, Assert_Null_GlobalVariable) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder builder;
            builder.AST().AddGlobalVariable(nullptr);
        },
        "internal compiler error");
}

TEST_F(ModuleDeathTest, Assert_Null_TypeDecl) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder builder;
            builder.AST().AddTypeDecl(nullptr);
        },
        "internal compiler error");
}

TEST_F(ModuleDeathTest, Assert_DifferentGenerationID_Function) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.AST().AddFunction(b2.create<Function>(b2.Ident("func"), tint::Empty, b2.ty.f32(),
                                                     b2.Block(), tint::Empty, tint::Empty));
        },
        "internal compiler error");
}

TEST_F(ModuleDeathTest, Assert_DifferentGenerationID_GlobalVariable) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.AST().AddGlobalVariable(b2.Var("var", b2.ty.i32(), core::AddressSpace::kPrivate));
        },
        "internal compiler error");
}

TEST_F(ModuleDeathTest, Assert_Null_Function) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder builder;
            builder.AST().AddFunction(nullptr);
        },
        "internal compiler error");
}

TEST_F(ModuleTest, CloneOrder) {
    // Create a program with a function, alias decl and var decl.
    Program p = [] {
        ProgramBuilder b;
        b.Func("F", {}, b.ty.void_(), {});
        b.Alias("A", b.ty.u32());
        b.GlobalVar("V", b.ty.i32(), core::AddressSpace::kPrivate);
        return resolver::Resolve(b);
    }();

    // Clone the program, using ReplaceAll() to create new module-scope
    // declarations. We want to test that these are added just before the
    // declaration that triggered the ReplaceAll().
    ProgramBuilder cloned;
    program::CloneContext ctx(&cloned, &p);
    ctx.ReplaceAll([&](const Function*) -> const Function* {
        ctx.dst->Alias("inserted_before_F", cloned.ty.u32());
        return nullptr;
    });
    ctx.ReplaceAll([&](const ast::Alias*) -> const ast::Alias* {
        ctx.dst->Alias("inserted_before_A", cloned.ty.u32());
        return nullptr;
    });
    ctx.ReplaceAll([&](const Variable*) -> const Variable* {
        ctx.dst->Alias("inserted_before_V", cloned.ty.u32());
        return nullptr;
    });
    ctx.Clone();

    auto& decls = cloned.AST().GlobalDeclarations();
    ASSERT_EQ(decls.Length(), 6u);
    EXPECT_TRUE(decls[1]->Is<Function>());
    EXPECT_TRUE(decls[3]->Is<ast::Alias>());
    EXPECT_TRUE(decls[5]->Is<Variable>());

    ASSERT_TRUE(decls[0]->Is<ast::Alias>());
    ASSERT_TRUE(decls[2]->Is<ast::Alias>());
    ASSERT_TRUE(decls[4]->Is<ast::Alias>());

    ASSERT_EQ(decls[0]->As<ast::Alias>()->name->symbol.Name(), "inserted_before_F");
    ASSERT_EQ(decls[2]->As<ast::Alias>()->name->symbol.Name(), "inserted_before_A");
    ASSERT_EQ(decls[4]->As<ast::Alias>()->name->symbol.Name(), "inserted_before_V");
}

TEST_F(ModuleTest, Directives) {
    auto* enable_1 = Enable(wgsl::Extension::kF16);
    auto* diagnostic_1 = DiagnosticDirective(wgsl::DiagnosticSeverity::kWarning, "foo");
    auto* enable_2 = Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
    auto* diagnostic_2 = DiagnosticDirective(wgsl::DiagnosticSeverity::kOff, "bar");

    Program program(std::move(*this));
    EXPECT_THAT(program.AST().GlobalDeclarations(), ::testing::ContainerEq(tint::Vector{
                                                        enable_1,
                                                        diagnostic_1,
                                                        enable_2,
                                                        diagnostic_2,
                                                    }));
    EXPECT_THAT(program.AST().Enables(), ::testing::ContainerEq(tint::Vector{
                                             enable_1,
                                             enable_2,
                                         }));
    EXPECT_THAT(program.AST().DiagnosticDirectives(), ::testing::ContainerEq(tint::Vector{
                                                          diagnostic_1,
                                                          diagnostic_2,
                                                      }));
}

}  // namespace
}  // namespace tint::ast
