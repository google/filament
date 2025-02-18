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

#include "src/tint/lang/wgsl/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/struct.h"

namespace tint::resolver {
namespace {

using ::testing::UnorderedElementsAre;
using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverPipelineStageUseTest = ResolverTest;

TEST_F(ResolverPipelineStageUseTest, UnusedStruct) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(0_a)})});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->PipelineStageUses().IsEmpty());
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsNonEntryPointParam) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(0_a)})});

    Func("foo", Vector{Param("param", ty.Of(s))}, ty.void_(), tint::Empty, tint::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->PipelineStageUses().IsEmpty());
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsNonEntryPointReturnType) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(0_a)})});

    Func("foo", tint::Empty, ty.Of(s), Vector{Return(Call(ty.Of(s), Expr(0_f)))}, tint::Empty);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_TRUE(sem->PipelineStageUses().IsEmpty());
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsVertexShaderParam) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(0_a)})});

    Func("main", Vector{Param("param", ty.Of(s))}, ty.vec4<f32>(),
         Vector{Return(Call<vec4<f32>>())}, Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{Builtin(core::BuiltinValue::kPosition)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(core::type::PipelineStageUsage::kVertexInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsVertexShaderReturnType) {
    auto* s = Structure(
        "S", Vector{Member("a", ty.vec4<f32>(), Vector{Builtin(core::BuiltinValue::kPosition)})});

    Func("main", tint::Empty, ty.Of(s), Vector{Return(Call(ty.Of(s)))},
         Vector{Stage(ast::PipelineStage::kVertex)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(core::type::PipelineStageUsage::kVertexOutput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsFragmentShaderParam) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(0_a)})});

    Func("main", Vector{Param("param", ty.Of(s))}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(core::type::PipelineStageUsage::kFragmentInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsFragmentShaderReturnType) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(0_a)})});

    Func("main", tint::Empty, ty.Of(s), Vector{Return(Call(ty.Of(s), Expr(0_f)))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(core::type::PipelineStageUsage::kFragmentOutput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsComputeShaderParam) {
    auto* s = Structure(
        "S",
        Vector{Member("a", ty.u32(), Vector{Builtin(core::BuiltinValue::kLocalInvocationIndex)})});

    Func("main", Vector{Param("param", ty.Of(s))}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(1_i)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(core::type::PipelineStageUsage::kComputeInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedMultipleStages) {
    auto* s = Structure(
        "S", Vector{Member("a", ty.vec4<f32>(), Vector{Builtin(core::BuiltinValue::kPosition)})});

    Func("vert_main", tint::Empty, ty.Of(s), Vector{Return(Call(ty.Of(s)))},
         Vector{Stage(ast::PipelineStage::kVertex)});

    Func("frag_main", Vector{Param("param", ty.Of(s))}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(core::type::PipelineStageUsage::kVertexOutput,
                                     core::type::PipelineStageUsage::kFragmentInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderParamViaAlias) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(0_a)})});
    auto* s_alias = Alias("S_alias", ty.Of(s));

    Func("main", Vector{Param("param", ty.Of(s_alias))}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(core::type::PipelineStageUsage::kFragmentInput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderParamLocationSet) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(3_a)})});

    Func("main", Vector{Param("param", ty.Of(s))}, ty.void_(), tint::Empty,
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    ASSERT_EQ(1u, sem->Members().Length());
    EXPECT_EQ(3u, sem->Members()[0]->Attributes().location);
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderReturnTypeViaAlias) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(0_a)})});
    auto* s_alias = Alias("S_alias", ty.Of(s));

    Func("main", tint::Empty, ty.Of(s_alias), Vector{Return(Call(ty.Of(s_alias), Expr(0_f)))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    EXPECT_THAT(sem->PipelineStageUses(),
                UnorderedElementsAre(core::type::PipelineStageUsage::kFragmentOutput));
}

TEST_F(ResolverPipelineStageUseTest, StructUsedAsShaderReturnTypeLocationSet) {
    auto* s = Structure("S", Vector{Member("a", ty.f32(), Vector{Location(3_a)})});

    Func("main", tint::Empty, ty.Of(s), Vector{Return(Call(ty.Of(s), Expr(0_f)))},
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = TypeOf(s)->As<core::type::Struct>();
    ASSERT_NE(sem, nullptr);
    ASSERT_EQ(1u, sem->Members().Length());
    EXPECT_EQ(3u, sem->Members()[0]->Attributes().location);
}

}  // namespace
}  // namespace tint::resolver
