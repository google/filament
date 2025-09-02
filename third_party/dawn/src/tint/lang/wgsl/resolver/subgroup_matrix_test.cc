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

#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"

#include "gmock/gmock.h"

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
        Alias("m", ty(kind.str(), params.el_ast(*this), u32(params.cols), u32(params.rows)));

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
    auto* alias = Alias("left", ty("subgroup_matrix_result", ty.f32(), 4_i, 2_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(alias)->UnwrapRef()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->Columns(), 4u);
    EXPECT_EQ(m->Rows(), 2u);
}

TEST_F(ResolverSubgroupMatrixTest, SignedRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* alias = Alias("left", ty("subgroup_matrix_result", ty.f32(), 4_u, 2_i));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(alias)->UnwrapRef()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->Columns(), 4u);
    EXPECT_EQ(m->Rows(), 2u);
}

TEST_F(ResolverSubgroupMatrixTest, DeclareTypeWithoutExtension) {
    Alias("left", ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of 'subgroup_matrix_*' requires enabling extension 'chromium_experimental_subgroup_matrix')");
}

TEST_F(ResolverSubgroupMatrixTest, MissingTemplateArgs) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty("subgroup_matrix_result"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: expected '<' for 'subgroup_matrix_result')");
}

TEST_F(ResolverSubgroupMatrixTest, MissingColsAndRows) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty("subgroup_matrix_result", ty.f32()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'subgroup_matrix_result' requires 3 template arguments)");
}

TEST_F(ResolverSubgroupMatrixTest, MissingRows) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty("subgroup_matrix_result", ty.f32(), 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'subgroup_matrix_result' requires 3 template arguments)");
}

TEST_F(ResolverSubgroupMatrixTest, MissingType) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty("subgroup_matrix_result", 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'subgroup_matrix_result' requires 3 template arguments)");
}

TEST_F(ResolverSubgroupMatrixTest, BadType) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty("subgroup_matrix_result", ty.bool_(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup_matrix element type must be f32, f16, i32, u32, i8 or u8)");
}

TEST_F(ResolverSubgroupMatrixTest, NonConstantColumnCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("cols", ty.u32(), Expr(8_a))),
             Decl(Var("left", ty("subgroup_matrix_result", ty.f32(), "cols", 8_a))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix column count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, ZeroColumnCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty("subgroup_matrix_result", ty.f32(), 0_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix column count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, NegativeColumnCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty("subgroup_matrix_result", ty.f32(), -1_i, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix column count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, NonConstantRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("rows", ty.u32(), Expr(8_a))),
             Decl(Var("left", ty("subgroup_matrix_result", ty.f32(), 8_a, "rows"))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix row count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, ZeroRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty("subgroup_matrix_result", ty.f32(), 8_a, 0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix row count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, NegativeRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Alias("left", ty("subgroup_matrix_result", ty.f32(), 8_a, -1_i));

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
    auto* construct = Call(ty.array(matrix, 4_a));
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
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.f32(), 8_a),
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
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.u32(), 8_a),
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
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.i32(), 8_a),
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
    auto* buffer = GlobalVar("buffer", storage, read_write, ty.array(ty.u32(), 8_a),
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
    auto* buffer =
        GlobalVar("buffer", storage, ty.array(ty.f32(), 8_a), Vector{Group(0_u), Binding(0_u)});
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
    auto* buffer =
        GlobalVar("buffer", storage, ty.array(ty.u32(), 8_a), Vector{Group(0_u), Binding(0_u)});
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
    auto* buffer =
        GlobalVar("buffer", storage, ty.array(ty.f32(), 8_a), Vector{Group(0_u), Binding(0_u)});
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
    auto* buffer =
        GlobalVar("buffer", storage, ty.array(ty.i32(), 8_a), Vector{Group(0_u), Binding(0_u)});
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
    auto* buffer =
        GlobalVar("buffer", storage, ty.array(ty.u32(), 8_a), Vector{Group(0_u), Binding(0_u)});
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

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.f32(), 2_u, 4_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.f32(), 8_u, 2_u));
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
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.i8(), 2_u, 4_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.i8(), 8_u, 2_u));
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
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.u8(), 2_u, 4_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.u8(), 8_u, 2_u));
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
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.f32(), 2_u, 4_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.f32(), 8_u, 2_u));
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
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.f32(), 4_u, 2_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.f32(), 2_u, 8_u));
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
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.u32(), 8_u, 8_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.i32(), 8_u, 8_u));
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
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.f32(), 8_u, 8_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.f32(), 8_u, 8_u));
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
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.f32(), 2_u, 4_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.f32(), 8_u, 2_u));
    auto* acc = Var("acc", function, ty("subgroup_matrix_result", ty.f32(), 8_u, 4_u));
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
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.i8(), 2_u, 4_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.i8(), 8_u, 2_u));
    auto* acc = Var("acc", function, ty("subgroup_matrix_result", ty.i32(), 8_u, 4_u));
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
    auto* left = Var("left", function, ty("subgroup_matrix_left", ty.u8(), 2_u, 4_u));
    auto* right = Var("right", function, ty("subgroup_matrix_right", ty.u8(), 8_u, 2_u));
    auto* acc = Var("acc", function, ty("subgroup_matrix_result", ty.u32(), 8_u, 4_u));
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
             Decl(Let("result", ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a),
                      Call(Ident("subgroup_matrix_result", ty.f32(), 8_a, 8_a)))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, FunctionVar_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("result", function, ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, PrivateVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", private_, ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'private' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, StorageVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", storage, ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a), Group(0_a),
              Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'storage' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, UniformVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", uniform, ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a), Group(0_a),
              Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'uniform' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, WorkgroupVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", workgroup, ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'workgroup' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, FunctionVar_ArrayElement_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto matrix_type = ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a);
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("result", function, ty.array(matrix_type, 8_a))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, WorkgroupVar_ArrayElement_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("result", workgroup, ty.array(ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a), 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: subgroup matrix types cannot be declared in the 'workgroup' address space)"));
}

TEST_F(ResolverSubgroupMatrixTest, FunctionVar_StructMember_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    auto* s = Structure("S", Vector{
                                 Member("m", ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a)),
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
                                 Member("m", ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a)),
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
    GlobalConst("result", ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a),
                Call(Ident("subgroup_matrix_result", ty.f32(), 8_a, 8_a)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(
        r()->error(),
        testing::HasSubstr(
            R"(error: const initializer requires a const-expression, but expression is a runtime-expression)"));
}

TEST_F(ResolverSubgroupMatrixTest, OverrideVar_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Override("result", ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a));

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
             Param("result", ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a)),
         },
         ty.void_(), Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, FunctionParameter_FunctionPointer_Valid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo",
         Vector{
             Param("result", ty.ptr<function>(ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a))),
         },
         ty.void_(), Empty);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, FunctionParameter_WorkgroupPointer_Invalid) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func("foo",
         Vector{
             Param("result", ty.ptr<workgroup>(ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a))),
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
    Func("foo", Empty, ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a),
         Vector{
             Return(Call(Ident("subgroup_matrix_result", ty.f32(), 8_a, 8_a))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

// Using the subgroup_matrix_uniformity diagnostic rule without the extension should succeed.
TEST_F(ResolverSubgroupMatrixTest, UseSubgroupUniformityRuleWithoutExtension) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kOff, "chromium", "subgroup_matrix_uniformity");
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverSubgroupMatrixTest, FragmentShader_FunctionVar) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    Func(
        "foo", Empty, ty.void_(),
        Vector{
            Decl(Var("result", ty(Source({12, 34}), "subgroup_matrix_result", ty.f32(), 8_u, 8_u))),
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
                      ty.array(ty(Source({12, 34}), "subgroup_matrix_result", ty.f32(), 8_u, 8_u),
                               4_a))),
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: subgroup matrix type cannot be used in fragment pipeline stage)");
}

TEST_F(ResolverSubgroupMatrixTest, FragmentShader_FunctionVarInStruct) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);

    Structure("S", Vector{
                       Member("m", ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a)),
                   });
    Func("foo", Empty, ty.void_(),
         Vector{
             Decl(Var("result", ty(Expr(Ident(Source({12, 34}), "S"))))),
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
    auto* buffer =
        GlobalVar("buffer", storage, ty.array(ty.f32(), 8_a), Vector{Group(0_u), Binding(0_u)});
    auto* call = Call(Source({12, 34}),
                      Ident(wgsl::BuiltinFn::kSubgroupMatrixLoad,
                            ty.subgroup_matrix(core::SubgroupMatrixKind::kLeft, ty.f32(), 8u, 8u)),
                      AddressOf(buffer), 0_u, false, 32_u);
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

}  // namespace
}  // namespace tint::resolver
