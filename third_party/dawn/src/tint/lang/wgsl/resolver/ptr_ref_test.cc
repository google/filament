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

#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/load.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct ResolverPtrRefTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverPtrRefTest, AddressOf) {
    // var v : i32;
    // &v

    auto* v = Var("v", ty.i32());
    auto* expr = AddressOf(v);

    WrapInFunction(v, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(expr)->Is<core::type::Pointer>());
    EXPECT_TRUE(TypeOf(expr)->As<core::type::Pointer>()->StoreType()->Is<core::type::I32>());
    EXPECT_EQ(TypeOf(expr)->As<core::type::Pointer>()->AddressSpace(),
              core::AddressSpace::kFunction);
}

TEST_F(ResolverPtrRefTest, AddressOfThenDeref) {
    // var v : i32;
    // *(&v)

    auto* v = Var("v", ty.i32());
    auto* expr = Deref(AddressOf(v));

    WrapInFunction(v, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(ref->Type()->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
}

TEST_F(ResolverPtrRefTest, DefaultPtrAddressSpace) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class

    auto* buf = Structure("S", Vector{Member("m", ty.i32())});
    auto* function = Var("f", ty.i32());
    auto* private_ = GlobalVar("p", ty.i32(), core::AddressSpace::kPrivate);
    auto* workgroup = GlobalVar("w", ty.i32(), core::AddressSpace::kWorkgroup);
    auto* uniform =
        GlobalVar("ub", ty.Of(buf), core::AddressSpace::kUniform, Binding(0_a), Group(0_a));
    auto* storage =
        GlobalVar("sb", ty.Of(buf), core::AddressSpace::kStorage, Binding(1_a), Group(0_a));

    auto* function_ptr =
        Let("f_ptr", ty.ptr(core::AddressSpace::kFunction, ty.i32()), AddressOf(function));
    auto* private_ptr =
        Let("p_ptr", ty.ptr(core::AddressSpace::kPrivate, ty.i32()), AddressOf(private_));
    auto* workgroup_ptr =
        Let("w_ptr", ty.ptr(core::AddressSpace::kWorkgroup, ty.i32()), AddressOf(workgroup));
    auto* uniform_ptr =
        Let("ub_ptr", ty.ptr(core::AddressSpace::kUniform, ty.Of(buf)), AddressOf(uniform));
    auto* storage_ptr =
        Let("sb_ptr", ty.ptr(core::AddressSpace::kStorage, ty.Of(buf)), AddressOf(storage));

    WrapInFunction(function, function_ptr, private_ptr, workgroup_ptr, uniform_ptr, storage_ptr);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    ASSERT_TRUE(TypeOf(function_ptr)->Is<core::type::Pointer>())
        << "function_ptr is " << TypeOf(function_ptr)->TypeInfo().name;
    ASSERT_TRUE(TypeOf(private_ptr)->Is<core::type::Pointer>())
        << "private_ptr is " << TypeOf(private_ptr)->TypeInfo().name;
    ASSERT_TRUE(TypeOf(workgroup_ptr)->Is<core::type::Pointer>())
        << "workgroup_ptr is " << TypeOf(workgroup_ptr)->TypeInfo().name;
    ASSERT_TRUE(TypeOf(uniform_ptr)->Is<core::type::Pointer>())
        << "uniform_ptr is " << TypeOf(uniform_ptr)->TypeInfo().name;
    ASSERT_TRUE(TypeOf(storage_ptr)->Is<core::type::Pointer>())
        << "storage_ptr is " << TypeOf(storage_ptr)->TypeInfo().name;

    EXPECT_EQ(TypeOf(function_ptr)->As<core::type::Pointer>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(private_ptr)->As<core::type::Pointer>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(workgroup_ptr)->As<core::type::Pointer>()->Access(), core::Access::kReadWrite);
    EXPECT_EQ(TypeOf(uniform_ptr)->As<core::type::Pointer>()->Access(), core::Access::kRead);
    EXPECT_EQ(TypeOf(storage_ptr)->As<core::type::Pointer>()->Access(), core::Access::kRead);
}

TEST_F(ResolverPtrRefTest, ArrayIndexAccessorViaDerefPointer) {
    // var a : array<i32, 3>;
    // let p = &a;
    // (*p)[0]

    auto* v = Var("v", ty.array<i32, 3>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = IndexAccessor(Deref(p), 0_i);

    WrapInFunction(v, p, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(ref->Type()->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
}

TEST_F(ResolverPtrRefTest, ArrayIndexAccessorViaPointer) {
    // var a : array<i32, 3>;
    // let p = &a;
    // p[0]

    auto* v = Var("v", ty.array<i32, 3>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = IndexAccessor(p, 0_i);
    WrapInFunction(v, p, expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(ref->Type()->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
}

TEST_F(ResolverPtrRefTest, ArrayIndexAccessorViaPointer_FeatureDisallowed) {
    // var a : array<i32, 3>;
    // let p = &a;
    // p[0]

    auto* v = Var("v", ty.array<i32, 3>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = IndexAccessor(Source{Source::Location{12, 34}}, p, 0_i);
    WrapInFunction(v, p, expr);

    auto resolver = Resolver(this, {});
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: pointer composite access requires the pointer_composite_access "
              "language feature, which is not allowed in the current environment");
}

TEST_F(ResolverPtrRefTest, VectorIndexAccessorViaDerefPointer) {
    // var a : vec3<i32>;
    // let p = &a;
    // (*p)[0]

    auto* v = Var("v", ty.vec3<i32>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = IndexAccessor(Deref(p), 0_i);
    WrapInFunction(v, p, expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(ref->Type()->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
}

TEST_F(ResolverPtrRefTest, VectorIndexAccessorViaPointer) {
    // var a : vec3<i32>;
    // let p = &a;
    // p[0]

    auto* v = Var("v", ty.vec3<i32>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = IndexAccessor(p, 0_i);
    WrapInFunction(v, p, expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(ref->Type()->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
}

TEST_F(ResolverPtrRefTest, VectorIndexAccessorViaPointer_FeatureDisallowed) {
    // var a : vec3<i32>;
    // let p = &a;
    // p[0]

    auto* v = Var("v", ty.vec3<i32>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = IndexAccessor(Source{Source::Location{12, 34}}, p, 0_i);
    WrapInFunction(v, p, expr);

    auto resolver = Resolver(this, {});
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: pointer composite access requires the pointer_composite_access "
              "language feature, which is not allowed in the current environment");
}

TEST_F(ResolverPtrRefTest, VectorMemberAccessorViaDerefPointer) {
    // var a : vec3<i32>;
    // let p = &a;
    // (*p).x

    auto* v = Var("v", ty.vec3<i32>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = MemberAccessor(Deref(p), "x");
    WrapInFunction(v, p, expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(ref->Type()->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
}

TEST_F(ResolverPtrRefTest, VectorMemberAccessorViaPointer) {
    // var a : vec3<i32>;
    // let p = &a;
    // p.x

    auto* v = Var("v", ty.vec3<i32>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = MemberAccessor(p, "x");
    WrapInFunction(v, p, expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(ref->Type()->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
}

TEST_F(ResolverPtrRefTest, VectorMemberAccessorViaPointer_FeatureDisallowed) {
    // var a : vec3<i32>;
    // let p = &a;
    // p.x

    auto* v = Var("v", ty.vec3<i32>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = MemberAccessor(Source{Source::Location{12, 34}}, p, "x");
    WrapInFunction(v, p, expr);

    auto resolver = Resolver(this, {});
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: pointer composite access requires the pointer_composite_access "
              "language feature, which is not allowed in the current environment");
}

TEST_F(ResolverPtrRefTest, MatrixIndexAccessorViaDerefPointer) {
    // var a : mat2x3<f32>;
    // let p = &a;
    // (*p)[0]

    auto* v = Var("v", ty.mat2x3<f32>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = IndexAccessor(Deref(p), 0_i);
    WrapInFunction(v, p, expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    auto* vec = ref->Type()->As<core::type::Reference>()->StoreType()->As<core::type::Vector>();
    ASSERT_TRUE(vec);
    EXPECT_EQ(vec->Elements().count, 3u);
    EXPECT_TRUE(vec->Elements().type->Is<core::type::F32>());
}

TEST_F(ResolverPtrRefTest, MatrixIndexAccessorViaPointer) {
    // var a : mat2x3<f32>;
    // let p = &a;
    // p[0]

    auto* v = Var("v", ty.mat2x3<f32>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = IndexAccessor(p, 0_i);
    WrapInFunction(v, p, expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    auto* vec = ref->Type()->As<core::type::Reference>()->StoreType()->As<core::type::Vector>();
    ASSERT_TRUE(vec);
    EXPECT_EQ(vec->Elements().count, 3u);
    EXPECT_TRUE(vec->Elements().type->Is<core::type::F32>());
}

TEST_F(ResolverPtrRefTest, MatrixIndexAccessorViaPointer_FeatureDisallowed) {
    // var a : mat2x3<f32>;
    // let p = &a;
    // p[0]

    auto* v = Var("v", ty.mat2x3<f32>());
    auto* p = Let("p", AddressOf(v));
    auto* expr = IndexAccessor(Source{Source::Location{12, 34}}, p, 0_i);
    WrapInFunction(v, p, expr);

    auto resolver = Resolver(this, {});
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: pointer composite access requires the pointer_composite_access "
              "language feature, which is not allowed in the current environment");
}

TEST_F(ResolverPtrRefTest, StructMemberAccessorViaDerefPointer) {
    // struct S { a : i32, }
    // var a : S;
    // let p = &a;
    // (*p).a

    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                             });
    auto* v = Var("v", ty.Of(s));
    auto* p = Let("p", AddressOf(v));
    auto* expr = MemberAccessor(Deref(p), "a");
    WrapInFunction(v, p, expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(ref->Type()->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
}

TEST_F(ResolverPtrRefTest, StructMemberAccessorViaPointer) {
    // struct S { a : i32, }
    // var a : S;
    // let p = &a;
    // p.a

    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                             });
    auto* v = Var("v", ty.Of(s));
    auto* p = Let("p", AddressOf(v));
    auto* expr = MemberAccessor(p, "a");
    WrapInFunction(v, p, expr);
    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* load = Sem().Get<sem::Load>(expr);
    ASSERT_NE(load, nullptr);

    auto* ref = load->Source();
    ASSERT_NE(ref, nullptr);

    ASSERT_TRUE(ref->Type()->Is<core::type::Reference>());
    EXPECT_TRUE(ref->Type()->As<core::type::Reference>()->StoreType()->Is<core::type::I32>());
}

TEST_F(ResolverPtrRefTest, StructMemberAccessorViaPointer_FeatureDisallowed) {
    // struct S { a : i32, }
    // var a : S;
    // let p = &a;
    // p.a

    auto* s = Structure("S", Vector{
                                 Member("a", ty.i32()),
                             });
    auto* v = Var("v", ty.Of(s));
    auto* p = Let("p", AddressOf(v));
    auto* expr = MemberAccessor(Source{Source::Location{12, 34}}, p, "a");
    WrapInFunction(v, p, expr);

    auto resolver = Resolver(this, {});
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: pointer composite access requires the pointer_composite_access "
              "language feature, which is not allowed in the current environment");
}

}  // namespace
}  // namespace tint::resolver
