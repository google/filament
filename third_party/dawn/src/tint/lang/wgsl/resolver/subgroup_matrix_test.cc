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
    auto* var = GlobalVar("m", private_,
                          ty(kind.str(), params.el_ast(*this), u32(params.cols), u32(params.rows)));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(var)->UnwrapRef()->As<core::type::SubgroupMatrix>();
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
                             Case<u32, 8, 8>(core::SubgroupMatrixKind::kResult)));

TEST_F(ResolverSubgroupMatrixTest, SignedColumnCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* var = GlobalVar("left", private_, ty("subgroup_matrix_result", ty.f32(), 4_i, 2_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(var)->UnwrapRef()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->Columns(), 4u);
    EXPECT_EQ(m->Rows(), 2u);
}

TEST_F(ResolverSubgroupMatrixTest, SignedRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* var = GlobalVar("left", private_, ty("subgroup_matrix_result", ty.f32(), 4_u, 2_i));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* m = TypeOf(var)->UnwrapRef()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(m, nullptr);
    EXPECT_EQ(m->Columns(), 4u);
    EXPECT_EQ(m->Rows(), 2u);
}

TEST_F(ResolverSubgroupMatrixTest, DeclareTypeWithoutExtension) {
    GlobalVar("left", private_, ty("subgroup_matrix_result", ty.f32(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of 'subgroup_matrix_*' requires enabling extension 'chromium_experimental_subgroup_matrix')");
}

TEST_F(ResolverSubgroupMatrixTest, MissingTemplateArgs) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("left", private_, ty("subgroup_matrix_result"));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: expected '<' for 'subgroup_matrix_result')");
}

TEST_F(ResolverSubgroupMatrixTest, MissingColsAndRows) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("left", private_, ty("subgroup_matrix_result", ty.f32()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'subgroup_matrix_result' requires 3 template arguments)");
}

TEST_F(ResolverSubgroupMatrixTest, MissingRows) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("left", private_, ty("subgroup_matrix_result", ty.f32(), 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'subgroup_matrix_result' requires 3 template arguments)");
}

TEST_F(ResolverSubgroupMatrixTest, MissingType) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("left", private_, ty("subgroup_matrix_result", 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: 'subgroup_matrix_result' requires 3 template arguments)");
}

TEST_F(ResolverSubgroupMatrixTest, BadType) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("left", private_, ty("subgroup_matrix_result", ty.bool_(), 8_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: subgroup_matrix element type must be f32, f16, i32, or u32)");
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
    GlobalVar("left", private_, ty("subgroup_matrix_result", ty.f32(), 0_a, 8_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix column count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, NegativeColumnCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("left", private_, ty("subgroup_matrix_result", ty.f32(), -1_i, 8_a));

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
    GlobalVar("left", private_, ty("subgroup_matrix_result", ty.f32(), 8_a, 0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: subgroup matrix row count must be a constant positive integer)");
}

TEST_F(ResolverSubgroupMatrixTest, NegativeRowCount) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    GlobalVar("left", private_, ty("subgroup_matrix_result", ty.f32(), 8_a, -1_i));

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

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = GlobalVar("left", private_, ty("subgroup_matrix_left", ty.f32(), 2_u, 4_u));
    auto* right = GlobalVar("right", private_, ty("subgroup_matrix_right", ty.f32(), 8_u, 2_u));
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixMultiply, left, right);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto call_sem = Sem().Get(call)->As<sem::Call>();
    ASSERT_NE(call_sem, nullptr);
    auto* target = call_sem->Target()->As<sem::BuiltinFn>();
    ASSERT_NE(target, nullptr);
    EXPECT_EQ(target->Fn(), wgsl::BuiltinFn::kSubgroupMatrixMultiply);
    auto* result = target->ReturnType()->As<core::type::SubgroupMatrix>();
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->Kind(), core::SubgroupMatrixKind::kResult);
    EXPECT_EQ(result->Columns(), 8u);
    EXPECT_EQ(result->Rows(), 4u);
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply_MismatchDimensions) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = GlobalVar("left", private_, ty("subgroup_matrix_left", ty.f32(), 4_u, 2_u));
    auto* right = GlobalVar("right", private_, ty("subgroup_matrix_right", ty.f32(), 2_u, 8_u));
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixMultiply, left, right);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixMultiply)"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply_MismatchTypes) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = GlobalVar("left", private_, ty("subgroup_matrix_left", ty.u32(), 8_u, 8_u));
    auto* right = GlobalVar("right", private_, ty("subgroup_matrix_right", ty.i32(), 8_u, 8_u));
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixMultiply, left, right);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixMultiply)"));
}

TEST_F(ResolverSubgroupMatrixTest, SubgroupMatrixMultiply_MismatchKinds) {
    Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
    auto* left = GlobalVar("left", private_, ty("subgroup_matrix_left", ty.f32(), 8_u, 8_u));
    auto* right = GlobalVar("right", private_, ty("subgroup_matrix_right", ty.f32(), 8_u, 8_u));
    auto* call = Call(wgsl::BuiltinFn::kSubgroupMatrixMultiply, right, left);
    Func("foo", Empty, ty.void_(),
         Vector{
             Assign(Phony(), call),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_THAT(r()->error(),
                testing::HasSubstr(R"(error: no matching call to 'subgroupMatrixMultiply)"));
}

}  // namespace
}  // namespace tint::resolver
