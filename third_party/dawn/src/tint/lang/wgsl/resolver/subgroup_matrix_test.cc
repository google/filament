// Copyright 2025 The Dawn & Tint Authors
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
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverSubgroupMatrixTest = ResolverTest;

struct SubgroupMatrixTypeCase {
    core::SubgroupMatrixKind kind;
    builder::ast_type_func_ptr el_ast;
    builder::sem_type_func_ptr el_sem;
    uint32_t cols;
    uint32_t rows;
};

template <typename T, uint32_t C, uint32_t R>
SubgroupMatrixTypeCase Case(core::SubgroupMatrixKind kind) {
    return SubgroupMatrixTypeCase{kind, builder::DataType<T>::AST, builder::DataType<T>::Sem, C, R};
}

using ResolverSubgroupMatrixParamTest = ResolverTestWithParam<SubgroupMatrixTypeCase>;

TEST_P(ResolverSubgroupMatrixParamTest, DeclareType) {
    Enable(wgsl::Extension::kF16);
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    auto params = GetParam();

    StringStream kind;
    kind << "subgroup_matrix_" << ToString(params.kind);
    auto* alias =
        Alias("m", ty.AsType(kind.str(), params.el_ast(*this), u32(params.cols), u32(params.rows)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(alias)->UnwrapRef()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->Kind(), params.kind);
    EXPECT_EQ(m->Type(), params.el_sem(*this));
    EXPECT_EQ(m->Columns(), params.cols);
    EXPECT_EQ(m->Rows(), params.rows);
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         ResolverSubgroupMatrixParamTest,
                         testing::Values(
                             // Test different matrix kinds and dimensions.
                             Case<f32, 4, 2>(core::SubgroupMatrixKind::kLeft),
                             Case<f32, 2, 4>(core::SubgroupMatrixKind::kRight),
                             Case<f32, 8, 8>(core::SubgroupMatrixKind::kResult),

                             // Test different element types.
                             Case<f16, 8, 8>(core::SubgroupMatrixKind::kResult),
                             Case<i32, 8, 8>(core::SubgroupMatrixKind::kResult),
                             Case<u32, 8, 8>(core::SubgroupMatrixKind::kResult),
                             Case<i8, 8, 8>(core::SubgroupMatrixKind::kResult),
                             Case<u8, 8, 8>(core::SubgroupMatrixKind::kResult)));

TEST_F(ResolverSubgroupMatrixTest, SignedColumnCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* alias = Alias("left", ty.subgroup_matrix_result(ty.f32(), 4_i, 2_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(alias)->UnwrapRef()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->Columns(), 4u);
    EXPECT_EQ(m->Rows(), 2u);
}

TEST_F(ResolverSubgroupMatrixTest, SignedRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* alias = Alias("left", ty.subgroup_matrix_result(ty.f32(), 4_u, 2_i));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(alias)->UnwrapRef()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->Columns(), 4u);
    EXPECT_EQ(m->Rows(), 2u);
}

TEST_F(ResolverSubgroupMatrixTest, DeclareTypeWithoutExtension) {
    Alias("left", ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of 'subgroup_matrix_*' requires enabling extension 'chromium_experimental_subgroup_matrix')");
}

TEST_F(ResolverSubgroupMatrixTest, MissingTemplateArgs) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty.AsType("subgroup_matrix_result"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: expected '<' for 'subgroup_matrix_result')");
}

TEST_F(ResolverSubgroupMatrixTest, MissingColsAndRows) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty.AsType("subgroup_matrix_result", ty.f32()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'subgroup_matrix_result' requires 3 template arguments)");
}

TEST_F(ResolverSubgroupMatrixTest, MissingRows) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty.AsType("subgroup_matrix_result", ty.f32(), 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'subgroup_matrix_result' requires 3 template arguments)");
}

TEST_F(ResolverSubgroupMatrixTest, MissingType) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty.AsType("subgroup_matrix_result", 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'subgroup_matrix_result' requires 3 template arguments)");
}

TEST_F(ResolverSubgroupMatrixTest, BadType) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty.subgroup_matrix_result(ty.bool_(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup_matrix element type must be f32, f16, i32, u32, i8 or u8)");
}

TEST_F(ResolverSubgroupMatrixTest, NonConstantColumnCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("cols", ty.u32(), Expr(8_a))),
             Decl(Var("left", ty.AsType("subgroup_matrix_result", ty.f32(), "cols", 8_a))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix column count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, ZeroColumnCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty.subgroup_matrix_result(ty.f32(), 0_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix column count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, NegativeColumnCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty.AsType("subgroup_matrix_result", ty.f32(), -1_i, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix column count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, NonConstantRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("rows", ty.u32(), Expr(8_a))),
             Decl(Var("left", ty.AsType("subgroup_matrix_result", ty.f32(), 8_a, "rows"))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix row count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, ZeroRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty.subgroup_matrix_result(ty.f32(), 8_a, 0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix row count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, NegativeRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty.AsType("subgroup_matrix_result", ty.f32(), 8_a, -1_i));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix row count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, ZeroValueConstructor) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* call = Call(Ident("subgroup_matrix_result", ty.f32(), 8_a, 8_a));
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(target, nullptr);
    EXPECT_TRUE(target->ReturnType()->Is<core::type::SubgroupMatrix>());
    EXPECT_EQ(target->Parameters().Length(), 0u);
    EXPECT_EQ(target->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverSubgroupMatrixTest, ZeroValueConstructor_InArray) {
    // _ = array<subgroup_matrix_result<f32, 8, 8>, 4>();
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto matrix = ty.subgroup_matrix(core::SubgroupMatrixKind::kResult, ty.f32(), 8u, 8u);
    auto* construct = Call(ty.array(matrix, Expr(4_a)));
    WrapInFunction(Assign(Phony(), construct));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(construct)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverSubgroupMatrixTest, ZeroValueConstructor_InStruct) {
    // struct S { m : subgroup_matrix_result<f32, 8, 8> }
    // _ = S();
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Structure(
        "S",
        Vector{
            Member("m", ty.subgroup_matrix(core::SubgroupMatrixKind::kResult, ty.f32(), 8u, 8u)),
        });
    auto* construct = Call("S");
    WrapInFunction(Assign(Phony(), construct));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(Sem().Get(construct)->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverSubgroupMatrixTest, SingleValueConstructor) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* call = Call(Ident("subgroup_matrix_result", ty.f32(), 8_a, 8_a), 1_a);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(target, nullptr);
    EXPECT_TRUE(target->ReturnType()->Is<core::type::SubgroupMatrix>());
    EXPECT_EQ(target->Parameters().Length(), 1u);
    EXPECT_EQ(target->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverSubgroupMatrixTest, SingleValueConstructor_U8_Abstract) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* call = Call(Ident("subgroup_matrix_result", ty.u8(), 8_a, 8_a), 1_a);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(target, nullptr);
    EXPECT_TRUE(target->ReturnType()->Is<core::type::SubgroupMatrix>());
    EXPECT_EQ(target->Parameters().Length(), 1u);
    EXPECT_EQ(target->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverSubgroupMatrixTest, SingleValueConstructor_U8_U32) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* call = Call(Ident("subgroup_matrix_result", ty.u8(), 8_a, 8_a), 1_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::ValueConstructor>();
    ASSERT_NE(target, nullptr);
    EXPECT_TRUE(target->ReturnType()->Is<core::type::SubgroupMatrix>());
    EXPECT_EQ(target->Parameters().Length(), 1u);
    EXPECT_EQ(target->Stage(), core::EvaluationStage::kRuntime);
}

TEST_F(ResolverSubgroupMatrixTest, ConstructorTooManyArgs) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* call = Call(Ident("subgroup_matrix_result", ty.f32(), 8_a, 8_a), 1_f, 2_f);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup_matrix constructor can only have zero or one elements)");
}

TEST_F(ResolverSubgroupMatrixTest, ConstructorWrongType) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* call = Call(Ident("subgroup_matrix_result", ty.u32(), 8_a, 8_a), 1_f);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: 'f32' cannot be used to construct a subgroup matrix of 'u32')");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.f32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixStore, AddressOf(buffer), 0_u,
                      Call(ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8u, 8u)),
                      false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             CallStmt(call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixStore);
    EXPECT_TRUE(target->IsSubgroupMatrix());
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_MismatchedType) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.u32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixStore, AddressOf(buffer), 0_u,
                      Call(ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.i32(), 8u, 8u)),
                      false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             CallStmt(call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixStore)"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_i8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.i32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixStore, AddressOf(buffer), 0_u,
                      Call(ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.i8(), 8u, 8u)),
                      false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             CallStmt(call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixStore);
    EXPECT_TRUE(target->IsSubgroupMatrix());
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_u8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.u32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixStore, AddressOf(buffer), 0_u,
                      Call(ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.u8(), 8u, 8u)),
                      false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             CallStmt(call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixStore);
    EXPECT_TRUE(target->IsSubgroupMatrix());
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, ty.array(ty.f32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixLoad,
                            ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8u, 8u)),
                      AddressOf(buffer), 0_u, false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixLoad);
    EXPECT_TRUE(target->ReturnType()->Is<core::type::SubgroupMatrix>());
    EXPECT_TRUE(target->IsSubgroupMatrix());
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad_MismatchedType) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, ty.array(ty.u32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixLoad,
                            ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.i32(), 8u, 8u)),
                      AddressOf(buffer), 0_u, false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixLoad)"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad_MissingTemplateArg) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, ty.array(ty.f32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixLoad, AddressOf(buffer), 0_u, false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixLoad)"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad_i8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, ty.array(ty.i32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixLoad,
                            ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.i8(), 8u, 8u)),
                      AddressOf(buffer), 0_u, false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixLoad);
    EXPECT_TRUE(target->ReturnType()->Is<core::type::SubgroupMatrix>());
    EXPECT_TRUE(target->IsSubgroupMatrix());
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad_u8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, ty.array(ty.u32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixLoad,
                            ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.u8(), 8u, 8u)),
                      AddressOf(buffer), 0_u, false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixLoad);
    EXPECT_TRUE(target->ReturnType()->Is<core::type::SubgroupMatrix>());
    EXPECT_TRUE(target->IsSubgroupMatrix());
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad_OOBOffset) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, ty.array(ty.f32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixLoad,
                            ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8u, 8u)),
                      AddressOf(buffer), 8_u, false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr("error: the offset argument of subgroupMatrixLoad (8) is out of "
                                   "bounds of the array type of size 8"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad_ArrayStrideTooSmall) {
    EXPECT_ERROR(
        R"(
enable chromium_experimental_subgroup_matrix;
enable f16;
@group(0) @binding(0) var<storage> in : array<f16>;
fn foo(stride : u32) {
  _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, col_major>(&in, 0, stride);
})",
        R"(input.wgsl:6:7 error: the stride of the array (2 bytes) must be greater than or equal to the matrix element size (4 bytes)
  _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, col_major>(&in, 0, stride);
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_OOBOffset) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.f32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixStore, AddressOf(buffer), 12_u,
                      Call(ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8u, 8u)),
                      false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             CallStmt(call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr("error: the offset argument of subgroupMatrixStore (12) is out "
                                   "of bounds of the array type of size 8"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_i8_i32_InBoundsOffset) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.i32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixStore, AddressOf(buffer), 12_u,
                      Call(ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.i8(), 8u, 8u)),
                      false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             CallStmt(call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_i8_i32_OOBOffset) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.i32(), Expr(8_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixStore, AddressOf(buffer), 32_u,
                      Call(ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.i8(), 8u, 8u)),
                      false, 32_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             CallStmt(call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr("error: the offset argument of subgroupMatrixStore (32) is out "
                                   "of bounds of the array type of size 32"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_ArrayStrideTooSmall) {
    EXPECT_ERROR(
        R"(
enable chromium_experimental_subgroup_matrix;
enable f16;
@group(0) @binding(0) var<storage, read_write> out : array<f16>;
fn foo(stride : u32) {
  let m = subgroup_matrix_left<f32, 8, 8>();
  _ = subgroupMatrixStore<col_major>(&out, 0, m, stride);
})",
        R"(input.wgsl:7:7 error: the stride of the array (2 bytes) must be greater than or equal to the matrix element size (4 bytes)
  _ = subgroupMatrixStore<col_major>(&out, 0, m, stride);
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.f32(), 2_u, 4_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.f32(), 8_u, 2_u));
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixMultiply, ty.f32()), left, right);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixMultiply);
    EXPECT_TRUE(target->IsSubgroupMatrix());
    auto* result = target->ReturnType()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->Kind(), core::SubgroupMatrixKind::kResult);
    EXPECT_EQ(result->Columns(), 8u);
    EXPECT_EQ(result->Rows(), 4u);
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply_i8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.i8(), 2_u, 4_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.i8(), 8_u, 2_u));
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixMultiply, ty.i32()), left, right);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixMultiply);
    EXPECT_TRUE(target->IsSubgroupMatrix());
    auto* result = target->ReturnType()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->Kind(), core::SubgroupMatrixKind::kResult);
    EXPECT_EQ(result->Columns(), 8u);
    EXPECT_EQ(result->Rows(), 4u);
    EXPECT_TRUE(result->Type()->Is<core::type::I32>());
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply_u8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.u8(), 2_u, 4_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.u8(), 8_u, 2_u));
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixMultiply, ty.u32()), left, right);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixMultiply);
    EXPECT_TRUE(target->IsSubgroupMatrix());
    auto* result = target->ReturnType()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->Kind(), core::SubgroupMatrixKind::kResult);
    EXPECT_EQ(result->Columns(), 8u);
    EXPECT_EQ(result->Rows(), 4u);
    EXPECT_TRUE(result->Type()->Is<core::type::U32>());
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply_MissingTemplateArg) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.f32(), 2_u, 4_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.f32(), 8_u, 2_u));
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixMultiply, left, right);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixMultiply)"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply_MismatchDimensions) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.f32(), 4_u, 2_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.f32(), 2_u, 8_u));
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixMultiply, ty.f32()), left, right);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixMultiply)"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply_MismatchTypes) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.u32(), 8_u, 8_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.i32(), 8_u, 8_u));
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixMultiply, ty.f32()), left, right);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixMultiply)"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply_MismatchKinds) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.f32(), 8_u, 8_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.f32(), 8_u, 8_u));
    auto* call = Call(Ident(wgsl::BuiltinFn::kSubgroupMatrixMultiply, ty.f32()), right, left);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixMultiply)"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiplyAccumulate) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.f32(), 2_u, 4_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.f32(), 8_u, 2_u));
    auto* acc = Var("acc", function, ty.subgroup_matrix_result(ty.f32(), 8_u, 4_u));
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Decl(acc),
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixMultiplyAccumulate);
    EXPECT_TRUE(target->IsSubgroupMatrix());
    auto* result = target->ReturnType()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->Kind(), core::SubgroupMatrixKind::kResult);
    EXPECT_EQ(result->Columns(), 8u);
    EXPECT_EQ(result->Rows(), 4u);
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiplyAccumulate_i8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.i8(), 2_u, 4_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.i8(), 8_u, 2_u));
    auto* acc = Var("acc", function, ty.subgroup_matrix_result(ty.i32(), 8_u, 4_u));
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Decl(acc),
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixMultiplyAccumulate);
    EXPECT_TRUE(target->IsSubgroupMatrix());
    auto* result = target->ReturnType()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->Kind(), core::SubgroupMatrixKind::kResult);
    EXPECT_EQ(result->Columns(), 8u);
    EXPECT_EQ(result->Rows(), 4u);
    EXPECT_TRUE(result->Type()->Is<core::type::I32>());
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiplyAccumulate_u8) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty.subgroup_matrix_left(ty.u8(), 2_u, 4_u));
    auto* right = Var("right", function, ty.subgroup_matrix_right(ty.u8(), 8_u, 2_u));
    auto* acc = Var("acc", function, ty.subgroup_matrix_result(ty.u32(), 8_u, 4_u));
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixMultiplyAccumulate, left, right, acc);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(left),
             Decl(right),
             Decl(acc),
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixMultiplyAccumulate);
    EXPECT_TRUE(target->IsSubgroupMatrix());
    auto* result = target->ReturnType()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->Kind(), core::SubgroupMatrixKind::kResult);
    EXPECT_EQ(result->Columns(), 8u);
    EXPECT_EQ(result->Rows(), 4u);
    EXPECT_TRUE(result->Type()->Is<core::type::U32>());
}

TEST_F(ResolverSubgroupMatrixTest, Let_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Let("result", ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a),
                      Call(Ident("subgroup_matrix_result", ty.f32(), 8_a, 8_a)))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, FunctionVar_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("result", function, ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, PrivateVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", private_, ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'private' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, StorageVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", storage, ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a), Group(0_a),
              Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'storage' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, UniformVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", uniform, ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a), Group(0_a),
              Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'uniform' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, WorkgroupVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", workgroup, ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'workgroup' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, FunctionVar_ArrayElement_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto matrix_type = ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("result", function, ty.array(matrix_type, Expr(8_a)))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, WorkgroupVar_ArrayElement_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", workgroup,
              ty.array(ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a), Expr(8_a)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'workgroup' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, FunctionVar_StructMember_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    auto* s = Structure("S", Vector{
                                 Member("m", ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a)),
                             });
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("result", function, ty.Of(s))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, WorkgroupVar_StructMember_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    auto* s = Structure("S", Vector{
                                 Member("m", ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a)),
                             });
    GlobalVar("result", workgroup, ty.Of(s));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'workgroup' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, ConstVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalConst("result", ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a),
                Call(Ident("subgroup_matrix_result", ty.f32(), 8_a, 8_a)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: const initializer requires a const-expression, but expression is a runtime-expression)"));
}

TEST_F(ResolverSubgroupMatrixTest, OverrideVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Override("result", ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup_matrix_result<f32, 8, 8> cannot be used as the type of a 'override')"));
}

TEST_F(ResolverSubgroupMatrixTest, FunctionParameter_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo",
         Vector{
             Param("result", ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a)),
         },
         ty.void_(), Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, FunctionParameter_FunctionPointer_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo",
         Vector{
             Param("result", ty.ptr<function>(ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a))),
         },
         ty.void_(), Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, FunctionParameter_WorkgroupPointer_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo",
         Vector{
             Param("result", ty.ptr<workgroup>(ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a))),
         },
         ty.void_(), Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'workgroup' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, ReturnType_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a),
         Vector{
             Return(Call(Ident("subgroup_matrix_result", ty.f32(), 8_a, 8_a))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// Using the subgroup_matrix_uniformity diagnostic rule without the extension should succeed.
TEST_F(ResolverSubgroupMatrixTest, UseSubgroupUniformityRuleWithoutExtension) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kOff,
                        DiagnosticRuleName("chromium", "subgroup_matrix_uniformity"));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, FragmentShader_FunctionVar) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("result", ty.subgroup_matrix_result(Source({12, 34}), ty.f32(), 8_u, 8_u))),
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: subgroup matrix type cannot be used in fragment pipeline stage)");
}

TEST_F(ResolverSubgroupMatrixTest, FragmentShader_FunctionVarInArray) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("result",
                      ty.array(ty.subgroup_matrix_result(Source({12, 34}), ty.f32(), 8_u, 8_u),
                               Expr(4_a)))),
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: subgroup matrix type cannot be used in fragment pipeline stage)");
}

TEST_F(ResolverSubgroupMatrixTest, FragmentShader_FunctionVarInStruct) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    Structure("S", Vector{
                       Member("m", ty.subgroup_matrix_result(ty.f32(), 8_a, 8_a)),
                   });
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("result", ty.AsType(Expr(Ident(Source({12, 34}), "S"))))),
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: subgroup matrix type cannot be used in fragment pipeline stage)");
}

TEST_F(ResolverSubgroupMatrixTest, FragmentShader_Constructor) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(),
                    Call(Ident(Source({12, 34}), "subgroup_matrix_result", ty.f32(), 8_a, 8_a))),
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: subgroup matrix type cannot be used in fragment pipeline stage)");
}

TEST_F(ResolverSubgroupMatrixTest, FragmentShader_SubgroupMatrixLoad) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* buffer = GlobalVar("buffer", storage, ty.array(ty.f32(), Expr(64_a)),
                             Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(Source({12, 34}),
                      Ident(wgsl::BuiltinFn::kSubgroupMatrixLoad,
                            ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8u, 8u),
                            core::Majorness::kRowMajor),
                      AddressOf(buffer), 0_u, 8_u);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: built-in cannot be used by fragment pipeline stage)");
}

TEST_F(ResolverSubgroupMatrixTest, VertexShader_IndirectUse) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(),
                    Call(Ident(Source({12, 34}), "subgroup_matrix_result", ty.f32(), 8_a, 8_a))),
         });

    Func("main", Empty, ty.vec4<f32>(),
         Vector{
             CallStmt(Call("foo")),
             Return(Call(ty.vec4<f32>())),
         },
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{Builtin(core::BuiltinValue::kPosition)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: subgroup matrix type cannot be used in vertex pipeline stage
note: called by function 'foo'
note: called by entry point 'main')");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad_Deprecated) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* v = GlobalVar("v", storage, ty.array(ty.f32()), Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(Source({12, 34}),
                      Ident(wgsl::BuiltinFn::kSubgroupMatrixLoad,
                            ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8u, 8u)),
                      AddressOf(v), 0_u, true, 8_u);

    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });
    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 warning: use of deprecated builtin)");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_Deprecated) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* v =
        GlobalVar("v", storage, read_write, ty.array(ty.f32()), Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(
        Source({12, 34}), Ident(wgsl::BuiltinFn::kSubgroupMatrixStore), AddressOf(v), 0_u,
        Call(ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8u, 8u)), false, 8_u);

    Func("foo", Empty, ty.void_(), Vector{CallStmt(call)});
    EXPECT_TRUE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 warning: use of deprecated builtin)");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad_TemplateMajorness) {
    ExpectSuccess(R"(
enable chromium_experimental_subgroup_matrix;
enable f16;

@group(0) @binding(0) var<storage> in : buffer;

var<workgroup> v : array<f16, 128 * 128>;
override o : u32;
var<workgroup> v2 : array<f32, o>;
fn foo(offset : u32, stride : u32) {
  _ = subgroupMatrixLoad<subgroup_matrix_left<f16, 8, 8>, col_major>(&v, offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<f16, 8, 8>, row_major>(&v, offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(&v, offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(&v2, offset, stride);

  _ = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major>(bufferView<array<f16>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major>(bufferView<array<f32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major>(bufferView<array<u32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major>(bufferView<array<i32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major>(bufferView<array<vec3i>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u8, 8, 8>, col_major>(bufferView<array<vec4f>>(&in, 0), offset, stride);

  _ = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, row_major>(bufferView<array<f16>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, row_major>(bufferView<array<f32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, row_major>(bufferView<array<u32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, row_major>(bufferView<array<i32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, row_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, row_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, row_major>(bufferView<array<vec3i>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i8, 8, 8>, row_major>(bufferView<array<vec4f>>(&in, 0), offset, stride);

  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(bufferView<array<f16>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(bufferView<array<f32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(bufferView<array<u32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(bufferView<array<i32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(bufferView<array<vec3i>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f16, 8, 8>, col_major>(bufferView<array<vec4f>>(&in, 0), offset, stride);

  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major>(bufferView<array<f32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major>(bufferView<array<u32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major>(bufferView<array<i32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major>(bufferView<array<vec3i>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, row_major>(bufferView<array<vec4f>>(&in, 0), offset, stride);

  _ = subgroupMatrixLoad<subgroup_matrix_right<i32, 8, 8>, col_major>(bufferView<array<f32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i32, 8, 8>, col_major>(bufferView<array<u32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i32, 8, 8>, col_major>(bufferView<array<i32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i32, 8, 8>, col_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i32, 8, 8>, col_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i32, 8, 8>, col_major>(bufferView<array<vec3i>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_right<i32, 8, 8>, col_major>(bufferView<array<vec4f>>(&in, 0), offset, stride);

  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>, row_major>(bufferView<array<f32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>, row_major>(bufferView<array<u32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>, row_major>(bufferView<array<i32>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>, row_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>, row_major>(bufferView<array<vec2u>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>, row_major>(bufferView<array<vec3i>>(&in, 0), offset, stride);
  _ = subgroupMatrixLoad<subgroup_matrix_result<f32, 8, 8>, row_major>(bufferView<array<vec4f>>(&in, 0), offset, stride);
})");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_TemplateMajorness) {
    ExpectSuccess(R"(
enable chromium_experimental_subgroup_matrix;
enable f16;

@group(0) @binding(0) var<storage, read_write> v : array<f32>;
@group(0) @binding(0) var<storage, read_write> out : buffer;
override o : u32;
var<workgroup> v2 : array<f32, o>;
fn foo(offset : u32, stride : u32) {
  subgroupMatrixStore<row_major>(&v, offset, subgroup_matrix_result<f32, 8, 8>(), stride);
  subgroupMatrixStore<col_major>(&v, offset, subgroup_matrix_left<f32, 8, 8>(), stride);
  subgroupMatrixStore<row_major>(&v, offset, subgroup_matrix_right<f32, 8, 8>(), stride);
  subgroupMatrixStore<row_major>(&v2, offset, subgroup_matrix_right<f32, 8, 8>(), stride);

  let m_u8 = subgroup_matrix_left<u8, 8, 8>();
  subgroupMatrixStore<col_major>(bufferView<array<f16>>(&out, 0), offset, m_u8, stride);
  subgroupMatrixStore<col_major>(bufferView<array<f32>>(&out, 0), offset, m_u8, stride);
  subgroupMatrixStore<col_major>(bufferView<array<u32>>(&out, 0), offset, m_u8, stride);
  subgroupMatrixStore<col_major>(bufferView<array<i32>>(&out, 0), offset, m_u8, stride);
  subgroupMatrixStore<col_major>(bufferView<array<vec2u>>(&out, 0), offset, m_u8, stride);
  subgroupMatrixStore<col_major>(bufferView<array<vec2u>>(&out, 0), offset, m_u8, stride);
  subgroupMatrixStore<col_major>(bufferView<array<vec3i>>(&out, 0), offset, m_u8, stride);
  subgroupMatrixStore<col_major>(bufferView<array<vec4f>>(&out, 0), offset, m_u8, stride);

  let m_i8 = subgroup_matrix_right<i8, 8, 8>();
  subgroupMatrixStore<row_major>(bufferView<array<f16>>(&out, 0), offset, m_i8, stride);
  subgroupMatrixStore<row_major>(bufferView<array<f32>>(&out, 0), offset, m_i8, stride);
  subgroupMatrixStore<row_major>(bufferView<array<u32>>(&out, 0), offset, m_i8, stride);
  subgroupMatrixStore<row_major>(bufferView<array<i32>>(&out, 0), offset, m_i8, stride);
  subgroupMatrixStore<row_major>(bufferView<array<vec2u>>(&out, 0), offset, m_i8, stride);
  subgroupMatrixStore<row_major>(bufferView<array<vec2u>>(&out, 0), offset, m_i8, stride);
  subgroupMatrixStore<row_major>(bufferView<array<vec3i>>(&out, 0), offset, m_i8, stride);
  subgroupMatrixStore<row_major>(bufferView<array<vec4f>>(&out, 0), offset, m_i8, stride);

  let m_f16 = subgroup_matrix_result<f16, 8, 8>();
  subgroupMatrixStore< col_major>(bufferView<array<f16>>(&out, 0), offset, m_f16, stride);
  subgroupMatrixStore< col_major>(bufferView<array<f32>>(&out, 0), offset, m_f16, stride);
  subgroupMatrixStore< col_major>(bufferView<array<u32>>(&out, 0), offset, m_f16, stride);
  subgroupMatrixStore< col_major>(bufferView<array<i32>>(&out, 0), offset, m_f16, stride);
  subgroupMatrixStore< col_major>(bufferView<array<vec2u>>(&out, 0), offset, m_f16, stride);
  subgroupMatrixStore< col_major>(bufferView<array<vec2u>>(&out, 0), offset, m_f16, stride);
  subgroupMatrixStore< col_major>(bufferView<array<vec3i>>(&out, 0), offset, m_f16, stride);
  subgroupMatrixStore< col_major>(bufferView<array<vec4f>>(&out, 0), offset, m_f16, stride);

  let m_u32 = subgroup_matrix_left<u32, 8, 8>();
  subgroupMatrixStore< row_major>(bufferView<array<f32>>(&out, 0), offset, m_u32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<u32>>(&out, 0), offset, m_u32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<i32>>(&out, 0), offset, m_u32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<vec2u>>(&out, 0), offset, m_u32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<vec2u>>(&out, 0), offset, m_u32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<vec3i>>(&out, 0), offset, m_u32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<vec4f>>(&out, 0), offset, m_u32, stride);

  let m_i32 = subgroup_matrix_right<i32, 8, 8>();
  subgroupMatrixStore< col_major>(bufferView<array<f32>>(&out, 0), offset, m_i32, stride);
  subgroupMatrixStore< col_major>(bufferView<array<u32>>(&out, 0), offset, m_i32, stride);
  subgroupMatrixStore< col_major>(bufferView<array<i32>>(&out, 0), offset, m_i32, stride);
  subgroupMatrixStore< col_major>(bufferView<array<vec2u>>(&out, 0), offset, m_i32, stride);
  subgroupMatrixStore< col_major>(bufferView<array<vec2u>>(&out, 0), offset, m_i32, stride);
  subgroupMatrixStore< col_major>(bufferView<array<vec3i>>(&out, 0), offset, m_i32, stride);
  subgroupMatrixStore< col_major>(bufferView<array<vec4f>>(&out, 0), offset, m_i32, stride);

  let m_f32 = subgroup_matrix_left<f32, 8, 8>();
  subgroupMatrixStore< row_major>(bufferView<array<f32>>(&out, 0), offset, m_f32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<u32>>(&out, 0), offset, m_f32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<i32>>(&out, 0), offset, m_f32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<vec2u>>(&out, 0), offset, m_f32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<vec2u>>(&out, 0), offset, m_f32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<vec3i>>(&out, 0), offset, m_f32, stride);
  subgroupMatrixStore< row_major>(bufferView<array<vec4f>>(&out, 0), offset, m_f32, stride);
})");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixLoad_TemplateMajorness_BadType) {
    ExpectError(
        R"(
enable chromium_experimental_subgroup_matrix;

var<workgroup> v : array<f32, 128 * 128>;
fn foo(offset : u32, stride : u32) {
  _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, f32>(&v, offset, stride);
})",
        R"(input.wgsl:6:7 error: no matching call to 'subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, f32>(ptr<workgroup, array<f32, 16384>, read_write>, u32, u32)'

8 candidate functions:
 • 'subgroupMatrixLoad<T  ✓ , Majorness  ✗ >(ptr<AS, array<AE[, N]>, AM>  ✓ , offset: O  ✓ , stride: S  ✓ ) -> T' where:
      ✓  'T' is 'subgroup_matrix<K, E, C, R>'
      ✓  'E' is 'f32', 'f16', 'u32', 'i32', 'u8' or 'i8'
      ✓  'AE' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AS' is 'workgroup' or 'storage'
      ✓  'AM' is 'read' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'
 • 'subgroupMatrixLoad<T  ✓ , Majorness  ✗ >(ptr<AS, array<vecN<AE>[, N]>, AM>  ✗ , offset: O  ✓ , stride: S  ✓ ) -> T' where:
      ✓  'T' is 'subgroup_matrix<K, E, C, R>'
      ✓  'E' is 'f32', 'f16', 'u32', 'i32', 'u8' or 'i8'
      ✗  'AE' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'read' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'
 • 'subgroupMatrixLoad<T  ✓ >(ptr<AS, array<E, AC>, AM>  ✓ , offset: O  ✓ , col_major: bool  ✗ , stride: S  ✗ ) -> T' where:
      ✓  'T' is 'subgroup_matrix<K, E, C, R>'
      ✓  'E' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AS' is 'workgroup' or 'storage'
      ✓  'AM' is 'read' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixLoad<T  ✓ >(ptr<storage, array<E>, AM>  ✗ , offset: O  ✓ , col_major: bool  ✗ , stride: S  ✗ ) -> T' where:
      ✓  'T' is 'subgroup_matrix<K, E, C, R>'
      ✓  'E' is 'f32', 'i32', 'u32' or 'f16'
      ✗  'AM' is 'read' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixLoad<T  ✗ >(ptr<storage, array<i32>, AM>  ✗ , offset: O  ✓ , col_major: bool  ✗ , stride: S  ✗ ) -> T' where:
      ✗  'T' is 'subgroup_matrix<K, E, C, R>'
      ✗  'E' is 'i8'
      ✗  'AM' is 'read' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixLoad<T  ✗ >(ptr<storage, array<u32>, AM>  ✗ , offset: O  ✓ , col_major: bool  ✗ , stride: S  ✗ ) -> T' where:
      ✗  'T' is 'subgroup_matrix<K, E, C, R>'
      ✗  'E' is 'u8'
      ✗  'AM' is 'read' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixLoad<T  ✗ >(ptr<AS, array<i32, AC>, AM>  ✗ , offset: O  ✓ , col_major: bool  ✗ , stride: S  ✗ ) -> T' where:
      ✗  'T' is 'subgroup_matrix<K, E, C, R>'
      ✗  'E' is 'i8'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'read' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixLoad<T  ✗ >(ptr<AS, array<u32, AC>, AM>  ✗ , offset: O  ✓ , col_major: bool  ✗ , stride: S  ✗ ) -> T' where:
      ✗  'T' is 'subgroup_matrix<K, E, C, R>'
      ✗  'E' is 'u8'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'read' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'

  _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 8>, f32>(&v, offset, stride);
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_TemplateMajorness_BadEnum) {
    ExpectError(R"(
enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> v : array<f32>;
fn foo(offset : u32, stride : u32) {
  subgroupMatrixStore<read_write>(&v, offset, subgroup_matrix_result<f32, 8, 8>(), stride);
})",
                R"(input.wgsl:6:23 error: Unexpected template kind
  subgroupMatrixStore<read_write>(&v, offset, subgroup_matrix_result<f32, 8, 8>(), stride);
                      ^^^^^^^^^^

input.wgsl:6:3 error: no matching call to 'subgroupMatrixStore<<invalid-type>>(ptr<storage, array<f32>, read_write>, u32, subgroup_matrix_result<f32, 8, 8>, u32)'

8 candidate functions:
 • 'subgroupMatrixStore<Majorness  ✗ >(ptr<AS, array<AE[, N]>, AM>  ✓ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✓ , stride: S  ✓ )' where:
      ✓  'E' is 'f32', 'f16', 'u32', 'i32', 'u8' or 'i8'
      ✓  'AE' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AS' is 'workgroup' or 'storage'
      ✓  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore<Majorness  ✗ >(ptr<AS, array<vecN<AE>[, N]>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✓ , stride: S  ✓ )' where:
      ✓  'E' is 'f32', 'f16', 'u32', 'i32', 'u8' or 'i8'
      ✗  'AE' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<storage, array<E>, AM>  ✓ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✓ , col_major: bool  ✗ , stride: S  ✗ )' where:
      ✓  'E' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<AS, array<E, AC>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✓ , col_major: bool  ✗ , stride: S  ✗ )' where:
      ✓  'E' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<storage, array<i32>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✗ , col_major: bool  ✗ , stride: S  ✗ )' where:
      ✗  'E' is 'i8'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<storage, array<u32>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✗ , col_major: bool  ✗ , stride: S  ✗ )' where:
      ✗  'E' is 'u8'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<AS, array<i32, AC>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✗ , col_major: bool  ✗ , stride: S  ✗ )' where:
      ✗  'E' is 'i8'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<AS, array<u32, AC>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✗ , col_major: bool  ✗ , stride: S  ✗ )' where:
      ✗  'E' is 'u8'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'

  subgroupMatrixStore<read_write>(&v, offset, subgroup_matrix_result<f32, 8, 8>(), stride);
  ^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixStore_TemplateMajorness_BadEnum_CloseParams) {
    ExpectError(R"(
enable chromium_experimental_subgroup_matrix;

@group(0) @binding(0) var<storage, read_write> v : array<f32>;
fn foo(offset : u32, stride : u32) {
  subgroupMatrixStore<read_write>(&v, offset, subgroup_matrix_result<f32, 8, 8>(), true, stride);
})",
                R"(input.wgsl:6:23 error: Unexpected template kind
  subgroupMatrixStore<read_write>(&v, offset, subgroup_matrix_result<f32, 8, 8>(), true, stride);
                      ^^^^^^^^^^

input.wgsl:6:3 error: no matching call to 'subgroupMatrixStore<<invalid-type>>(ptr<storage, array<f32>, read_write>, u32, subgroup_matrix_result<f32, 8, 8>, bool, u32)'

8 candidate functions:
 • 'subgroupMatrixStore<Majorness  ✗ >(ptr<AS, array<AE[, N]>, AM>  ✓ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✓ , stride: S  ✗ )' where:
      ✓  'E' is 'f32', 'f16', 'u32', 'i32', 'u8' or 'i8'
      ✓  'AE' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AS' is 'workgroup' or 'storage'
      ✓  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore<Majorness  ✗ >(ptr<AS, array<vecN<AE>[, N]>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✓ , stride: S  ✗ )' where:
      ✓  'E' is 'f32', 'f16', 'u32', 'i32', 'u8' or 'i8'
      ✗  'AE' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✗  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<storage, array<E>, AM>  ✓ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✓ , col_major: bool  ✓ , stride: S  ✓ )' where:
      ✗  overload expects 0 template arguments, call passed 1 argument
      ✓  'E' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<AS, array<E, AC>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✓ , col_major: bool  ✓ , stride: S  ✓ )' where:
      ✓  'E' is 'f32', 'i32', 'u32' or 'f16'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<storage, array<i32>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✗ , col_major: bool  ✓ , stride: S  ✓ )' where:
      ✗  'E' is 'i8'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<storage, array<u32>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✗ , col_major: bool  ✓ , stride: S  ✓ )' where:
      ✗  'E' is 'u8'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<AS, array<i32, AC>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✗ , col_major: bool  ✓ , stride: S  ✓ )' where:
      ✗  'E' is 'i8'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'
 • 'subgroupMatrixStore(ptr<AS, array<u32, AC>, AM>  ✗ , offset: O  ✓ , subgroup_matrix<K, E, C, R>  ✗ , col_major: bool  ✓ , stride: S  ✓ )' where:
      ✗  'E' is 'u8'
      ✓  'AS' is 'workgroup' or 'storage'
      ✗  'AM' is 'write' or 'read_write'
      ✓  'O' is 'i32' or 'u32'
      ✓  'S' is 'i32' or 'u32'

  subgroupMatrixStore<read_write>(&v, offset, subgroup_matrix_result<f32, 8, 8>(), true, stride);
  ^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, Load_ColMajor_StrideNegative) {
    ExpectError(
        R"(
enable chromium_experimental_subgroup_matrix;
@group(0) @binding(0) var<storage> in : array<u32>;
fn foo() {
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 16>, col_major>(&in, 0, -1);
})",
        R"(input.wgsl:5:79 error: the stride argument of subgroupMatrixLoad must be non-negative
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 16>, col_major>(&in, 0, -1);
                                                                              ^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, Load_ColMajor_StrideLessThanMinStride_U32) {
    ExpectError(
        R"(
enable chromium_experimental_subgroup_matrix;
@group(0) @binding(0) var<storage> in : array<u32>;
fn foo() {
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 16>, col_major>(&in, 0, 15);
})",
        R"(input.wgsl:5:79 error: the stride argument (15, 60 bytes) of subgroupMatrixLoad must be greater than the minimum stride (64 bytes)
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 16>, col_major>(&in, 0, 15);
                                                                              ^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, Store_RowMajor_OffsetNegative) {
    ExpectError(
        R"(
enable chromium_experimental_subgroup_matrix;
@group(0) @binding(0) var<storage, read_write> out : array<u32>;
fn foo(m : subgroup_matrix_result<u8, 8, 8>, stride: u32) {
  subgroupMatrixStore<row_major>(&out, -1, m, stride);
})",
        R"(input.wgsl:5:40 error: the offset argument of subgroupMatrixStore must be non-negative
  subgroupMatrixStore<row_major>(&out, -1, m, stride);
                                       ^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, Store_RowMajor_StrideLessThanMinStride_U8) {
    ExpectError(
        R"(
enable chromium_experimental_subgroup_matrix;
@group(0) @binding(0) var<storage, read_write> out : array<u32>;
fn foo(m : subgroup_matrix_result<u8, 8, 8>) {
  subgroupMatrixStore<row_major>(&out, 0, m, 1);
})",
        R"(input.wgsl:5:46 error: the stride argument (1, 4 bytes) of subgroupMatrixStore must be greater than the minimum stride (8 bytes)
  subgroupMatrixStore<row_major>(&out, 0, m, 1);
                                             ^
)");
}

TEST_F(ResolverSubgroupMatrixTest, Load_ColMajor_PointerTooSmall_F32) {
    ExpectError(
        R"(
enable chromium_experimental_subgroup_matrix;
@group(0) @binding(0) var<storage> in : array<f32, 127>;
fn foo(offset: u32, stride: u32) {
  _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 16>, col_major>(&in, offset, stride);
})",
        R"(input.wgsl:5:7 error: the pointer operand of subgroupMatrixLoad is too small (508 bytes) for the matrix access (512 bytes)
  _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 16>, col_major>(&in, offset, stride);
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, Load_ColMajor_PointerTooSmall_F32_ConstOffset) {
    ExpectError(
        R"(
enable chromium_experimental_subgroup_matrix;
@group(0) @binding(0) var<storage> in : array<f32, 128>;
fn foo(stride: u32) {
  _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 16>, col_major>(&in, 1, stride);
})",
        R"(input.wgsl:5:7 error: the pointer operand of subgroupMatrixLoad is too small (512 bytes) for the matrix access (516 bytes)
  _ = subgroupMatrixLoad<subgroup_matrix_left<f32, 8, 16>, col_major>(&in, 1, stride);
      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, Store_RowMajor_PointerTooSmall_F16_ConstStride) {
    ExpectError(
        R"(
enable f16;
enable chromium_experimental_subgroup_matrix;
@group(0) @binding(0) var<storage, read_write> out : array<f16, 64>;
fn foo(m : subgroup_matrix_right<f16, 8, 8>, offset: u32) {
  subgroupMatrixStore<row_major>(&out, offset, m, 9);
})",
        R"(input.wgsl:6:3 error: the pointer operand of subgroupMatrixStore is too small (128 bytes) for the matrix access (142 bytes)
  subgroupMatrixStore<row_major>(&out, offset, m, 9);
  ^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, Store_RowMajor_PointerTooSmall_U8_ConstOffsetAndStride) {
    ExpectError(
        R"(
enable chromium_experimental_subgroup_matrix;
@group(0) @binding(0) var<storage, read_write> out : array<u32, 63>;
fn foo(m : subgroup_matrix_right<u8, 16, 8>) {
  subgroupMatrixStore<row_major>(&out, 5, m, 8);
})",
        R"(input.wgsl:5:3 error: the pointer operand of subgroupMatrixStore is too small (252 bytes) for the matrix access (260 bytes)
  subgroupMatrixStore<row_major>(&out, 5, m, 8);
  ^^^^^^^^^^^^^^^^^^^
)");
}

TEST_F(ResolverSubgroupMatrixTest, Load_RuntimeArray_Workgroup) {
    EXPECT_SUCCESS(R"(
enable chromium_experimental_subgroup_matrix;
var<workgroup> v : buffer<1024>;
fn foo(offset: u32, stride: u32) {
  let view = bufferView<array<u32>>(&v, 0);
  _ = subgroupMatrixLoad<subgroup_matrix_left<u32, 8, 8>, col_major>(view, offset, stride);
})");
}

TEST_F(ResolverSubgroupMatrixTest, Store_RuntimeArray_Workgroup) {
    EXPECT_SUCCESS(R"(
enable chromium_experimental_subgroup_matrix;
var<workgroup> v : buffer<1024>;
fn foo(m : subgroup_matrix_left<u32, 8, 8>, offset: u32, stride: u32) {
  let view = bufferArrayView<array<u32>>(&v, 0, 1024);
  subgroupMatrixStore<row_major>(view, offset, m, stride);
})");
}

}  // namespace
}  // namespace tint::resolver
