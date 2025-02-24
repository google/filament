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

#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/array.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

class ResolverOverrideTest : public ResolverTest {
  protected:
    /// Verify that the AST node `var` was resolved to an overridable constant
    /// with an ID equal to `id`.
    /// @param var the overridable constant AST node
    /// @param id the expected constant ID
    void ExpectOverrideId(const ast::Variable* var, uint16_t id) {
        auto* sem = Sem().Get<sem::GlobalVariable>(var);
        ASSERT_NE(sem, nullptr);
        EXPECT_EQ(sem->Declaration(), var);
        EXPECT_TRUE(sem->Declaration()->Is<ast::Override>());
        EXPECT_EQ(sem->Attributes().override_id->value, id);
        EXPECT_FALSE(sem->ConstantValue());
    }
};

TEST_F(ResolverOverrideTest, NonOverridable) {
    auto* a = GlobalConst("a", ty.f32(), Expr(1_f));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_a = Sem().Get<sem::GlobalVariable>(a);
    ASSERT_NE(sem_a, nullptr);
    EXPECT_EQ(sem_a->Declaration(), a);
    EXPECT_FALSE(sem_a->Declaration()->Is<ast::Override>());
    EXPECT_TRUE(sem_a->ConstantValue());
}

TEST_F(ResolverOverrideTest, WithId) {
    auto* a = Override("a", ty.f32(), Expr(1_f), Id(7_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ExpectOverrideId(a, 7u);
}

TEST_F(ResolverOverrideTest, WithoutId) {
    auto* a = Override("a", ty.f32(), Expr(1_f));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ExpectOverrideId(a, 0u);
}

TEST_F(ResolverOverrideTest, WithAndWithoutIds) {
    Enable(wgsl::Extension::kF16);

    auto* a = Override("a", ty.f32(), Expr(1_f));
    auto* b = Override("b", ty.f16(), Expr(1_h));
    auto* c = Override("c", ty.i32(), Expr(1_i), Id(2_u));
    auto* d = Override("d", ty.u32(), Expr(1_u), Id(4_u));
    auto* e = Override("e", ty.f32(), Expr(1_f));
    auto* f = Override("f", ty.f32(), Expr(1_f), Id(1_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    // Verify that constant id allocation order is deterministic.
    ExpectOverrideId(a, 0u);
    ExpectOverrideId(b, 3u);
    ExpectOverrideId(c, 2u);
    ExpectOverrideId(d, 4u);
    ExpectOverrideId(e, 5u);
    ExpectOverrideId(f, 1u);
}

TEST_F(ResolverOverrideTest, DuplicateIds) {
    Override("a", ty.f32(), Expr(1_f), Id(Source{{12, 34}}, 7_u));
    Override("b", ty.f32(), Expr(1_f), Id(Source{{56, 78}}, 7_u));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(56:78 error: '@id' values must be unique
12:34 note: a override with an ID of 7 was previously declared here)");
}

TEST_F(ResolverOverrideTest, IdTooLarge) {
    Override("a", ty.f32(), Expr(1_f), Id(Source{{12, 34}}, 65536_u));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), "12:34 error: '@id' value must be between 0 and 65535");
}

TEST_F(ResolverOverrideTest, TransitiveReferences_DirectUse) {
    auto* a = Override("a", ty.f32());
    auto* b = Override("b", ty.f32(), Expr(1_f));
    Override("unused", ty.f32(), Expr(1_f));
    auto* func = Func("foo", tint::Empty, ty.void_(),
                      Vector{
                          Assign(Phony(), "a"),
                          Assign(Phony(), "b"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto& refs = Sem().Get(func)->TransitivelyReferencedGlobals();
    ASSERT_EQ(refs.Length(), 2u);
    EXPECT_EQ(refs[0], Sem().Get(a));
    EXPECT_EQ(refs[1], Sem().Get(b));
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaOverrideInit) {
    auto* a = Override("a", ty.f32());
    auto* b = Override("b", ty.f32(), Mul(2_a, "a"));
    Override("unused", ty.f32(), Expr(1_f));
    auto* func = Func("foo", tint::Empty, ty.void_(),
                      Vector{
                          Assign(Phony(), "b"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto refs = Sem().Get(b)->TransitivelyReferencedOverrides();
        ASSERT_EQ(refs.Length(), 1u);
        EXPECT_EQ(refs[0], Sem().Get(a));
    }

    {
        auto refs = Sem().Get(func)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaPrivateInit) {
    auto* a = Override("a", ty.f32());
    auto* b = GlobalVar("b", core::AddressSpace::kPrivate, ty.f32(), Mul(2_a, "a"));
    Override("unused", ty.f32(), Expr(1_f));
    auto* func = Func("foo", tint::Empty, ty.void_(),
                      Vector{
                          Assign(Phony(), "b"),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto refs = Sem().Get<sem::GlobalVariable>(b)->TransitivelyReferencedOverrides();
        ASSERT_EQ(refs.Length(), 1u);
        EXPECT_EQ(refs[0], Sem().Get(a));
    }

    {
        auto& refs = Sem().Get(func)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaAttribute) {
    auto* a = Override("a", ty.i32());
    auto* b = Override("b", ty.i32(), Mul(2_a, "a"));
    Override("unused", ty.i32(), Expr(1_a));
    auto* func = Func("foo", tint::Empty, ty.void_(),
                      Vector{
                          Assign(Phony(), "b"),
                      },
                      Vector{
                          Stage(ast::PipelineStage::kCompute),
                          WorkgroupSize(Mul(2_a, "b")),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto& refs = Sem().Get(func)->TransitivelyReferencedGlobals();
    ASSERT_EQ(refs.Length(), 2u);
    EXPECT_EQ(refs[0], Sem().Get(b));
    EXPECT_EQ(refs[1], Sem().Get(a));
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaArraySize) {
    auto* a = Override("a", ty.i32());
    auto* b = Override("b", ty.i32(), Mul(2_a, "a"));
    auto* arr = GlobalVar("arr", core::AddressSpace::kWorkgroup, ty.array(ty.i32(), Mul(2_a, "b")));
    Override("unused", ty.i32(), Expr(1_a));
    auto* func = Func("foo", tint::Empty, ty.void_(),
                      Vector{
                          Assign(IndexAccessor("arr", 0_a), 42_a),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get<sem::GlobalVariable>(arr);
    ASSERT_NE(global, nullptr);
    auto* arr_ty = global->Type()->UnwrapRef()->As<sem::Array>();
    ASSERT_NE(arr_ty, nullptr);

    {
        auto refs = global->TransitivelyReferencedOverrides();
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }
    {
        auto refs = arr_ty->TransitivelyReferencedOverrides();
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }
    {
        auto refs = Sem().Get(func)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 3u);
        EXPECT_EQ(refs[0], Sem().Get(arr));
        EXPECT_EQ(refs[1], Sem().Get(b));
        EXPECT_EQ(refs[2], Sem().Get(a));
    }
}

TEST_F(ResolverOverrideTest, TransitiveReferences_ViaArraySize_Alias) {
    auto* a = Override("a", ty.i32());
    auto* b = Override("b", ty.i32(), Mul(2_a, "a"));
    Alias("arr_ty", ty.array(ty.i32(), Mul(2_a, "b")));
    auto* arr = GlobalVar("arr", core::AddressSpace::kWorkgroup, ty("arr_ty"));
    Override("unused", ty.i32(), Expr(1_a));
    auto* func = Func("foo", tint::Empty, ty.void_(),
                      Vector{
                          Assign(IndexAccessor("arr", 0_a), 42_a),
                      });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* global = Sem().Get<sem::GlobalVariable>(arr);
    ASSERT_NE(global, nullptr);
    auto* arr_ty = global->Type()->UnwrapRef()->As<sem::Array>();
    ASSERT_NE(arr_ty, nullptr);

    {
        auto refs = global->TransitivelyReferencedOverrides();
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }
    {
        auto refs = arr_ty->TransitivelyReferencedOverrides();
        ASSERT_EQ(refs.Length(), 2u);
        EXPECT_EQ(refs[0], Sem().Get(b));
        EXPECT_EQ(refs[1], Sem().Get(a));
    }
    {
        auto refs = Sem().Get(func)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 3u);
        EXPECT_EQ(refs[0], Sem().Get(arr));
        EXPECT_EQ(refs[1], Sem().Get(b));
        EXPECT_EQ(refs[2], Sem().Get(a));
    }
}

TEST_F(ResolverOverrideTest, TransitiveReferences_MultipleEntryPoints) {
    auto* a = Override("a", ty.i32());
    auto* b1 = Override("b1", ty.i32(), Mul(2_a, "a"));
    auto* b2 = Override("b2", ty.i32(), Mul(2_a, "a"));
    auto* c1 = Override("c1", ty.i32());
    auto* c2 = Override("c2", ty.i32());
    auto* d = Override("d", ty.i32());
    Alias("arr_ty1", ty.array(ty.i32(), Mul("b1", "c1")));
    Alias("arr_ty2", ty.array(ty.i32(), Mul("b2", "c2")));
    auto* arr1 = GlobalVar("arr1", core::AddressSpace::kWorkgroup, ty("arr_ty1"));
    auto* arr2 = GlobalVar("arr2", core::AddressSpace::kWorkgroup, ty("arr_ty2"));
    Override("unused", ty.i32(), Expr(1_a));
    auto* func1 = Func("foo1", tint::Empty, ty.void_(),
                       Vector{
                           Assign(IndexAccessor("arr1", 0_a), 42_a),
                       },
                       Vector{
                           Stage(ast::PipelineStage::kCompute),
                           WorkgroupSize(Mul(2_a, "d")),
                       });
    auto* func2 = Func("foo2", tint::Empty, ty.void_(),
                       Vector{
                           Assign(IndexAccessor("arr2", 0_a), 42_a),
                       },
                       Vector{
                           Stage(ast::PipelineStage::kCompute),
                           WorkgroupSize(64_a),
                       });

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    {
        auto& refs = Sem().Get(func1)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 5u);
        EXPECT_EQ(refs[0], Sem().Get(d));
        EXPECT_EQ(refs[1], Sem().Get(arr1));
        EXPECT_EQ(refs[2], Sem().Get(b1));
        EXPECT_EQ(refs[3], Sem().Get(a));
        EXPECT_EQ(refs[4], Sem().Get(c1));
    }

    {
        auto& refs = Sem().Get(func2)->TransitivelyReferencedGlobals();
        ASSERT_EQ(refs.Length(), 4u);
        EXPECT_EQ(refs[0], Sem().Get(arr2));
        EXPECT_EQ(refs[1], Sem().Get(b2));
        EXPECT_EQ(refs[2], Sem().Get(a));
        EXPECT_EQ(refs[3], Sem().Get(c2));
    }
}

}  // namespace
}  // namespace tint::resolver
