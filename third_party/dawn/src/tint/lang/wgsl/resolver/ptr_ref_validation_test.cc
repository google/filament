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

#include "gmock/gmock.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct ResolverPtrRefValidationTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverPtrRefValidationTest, AddressOfLiteral) {
    // &1

    auto* expr = AddressOf(Expr(Source{{12, 34}}, 1_i));

    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: cannot take the address of value of type 'i32')");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfLet) {
    // let l : i32 = 1;
    // &l
    auto* l = Let("l", ty.i32(), Expr(1_i));
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "l"));

    WrapInFunction(l, expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: cannot take the address of 'let l')");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfConst) {
    // const c : i32 = 1;
    // &c
    auto* l = Const("c", ty.i32(), Expr(1_i));
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "c"));

    WrapInFunction(l, expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: cannot take the address of 'const c')");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfOverride) {
    // override c : i32;
    // &o
    Override("o", ty.i32());
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "o"));

    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: cannot take the address of 'override o')");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfParameter) {
    // fn F(p : i32) { _ = &p }
    // &F
    Func("F", Vector{Param("p", ty.i32())}, ty.void_(),
         Vector{
             Assign(Phony(), AddressOf(Expr(Source{{12, 34}}, "p"))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot take the address of parameter 'p')");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfHandle) {
    // @group(0) @binding(0) var t: texture_3d<f32>;
    // &t
    GlobalVar("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()), Group(0_a),
              Binding(0_a));
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "t"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot take the address of 'var t' in handle address space)");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfFunction) {
    // fn F() {}
    // &F
    Func("F", Empty, ty.void_(), Empty);
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "F"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot use function 'F' as value
note: function 'F' declared here
12:34 note: are you missing '()'?)");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfBuiltinFunction) {
    // &max
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "max"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot use builtin function 'max' as value
12:34 note: are you missing '()'?)");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfType) {
    // &i32
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "i32"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot use type 'i32' as value
12:34 note: are you missing '()'?)");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfTypeAlias) {
    // alias T = i32
    // &T
    Alias("T", ty.i32());
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "T"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot use type 'i32' as value
12:34 note: are you missing '()'?)");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfAccess) {
    // &read_write
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "read_write"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot use access 'read_write' as value)");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfAddressSpace) {
    // &handle
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "uniform"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot use address space 'uniform' as value)");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfUnresolvedValue) {
    // &position
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "position"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved value 'position')");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfTexelFormat) {
    // &rgba8snorm
    auto* expr = AddressOf(Expr(Source{{12, 34}}, "rgba8snorm"));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: cannot use texel format 'rgba8snorm' as value)");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfVectorComponent_MemberAccessor) {
    // var v : vec4<i32>;
    // &v.y
    auto* v = Var("v", ty.vec4<i32>());
    auto* expr = AddressOf(MemberAccessor(Source{{12, 34}}, "v", "y"));

    WrapInFunction(v, expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: cannot take the address of a vector component)");
}

TEST_F(ResolverPtrRefValidationTest, AddressOfVectorComponent_IndexAccessor) {
    // var v : vec4<i32>;
    // &v[2i]
    auto* v = Var("v", ty.vec4<i32>());
    auto* expr = AddressOf(IndexAccessor(Source{{12, 34}}, "v", 2_i));

    WrapInFunction(v, expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: cannot take the address of a vector component)");
}

TEST_F(ResolverPtrRefValidationTest, IndirectOfAddressOfHandle) {
    // @group(0) @binding(0) var t: texture_3d<f32>;
    // *&t
    GlobalVar("t", ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32()), Group(0_a),
              Binding(0_a));
    auto* expr = Deref(AddressOf(Expr(Source{{12, 34}}, "t")));
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: cannot take the address of 'var t' in handle address space)");
}

TEST_F(ResolverPtrRefValidationTest, DerefOfLiteral) {
    // *1

    auto* expr = Deref(Expr(Source{{12, 34}}, 1_i));

    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: cannot dereference expression of type 'i32')");
}

TEST_F(ResolverPtrRefValidationTest, DerefOfVar) {
    // var v : i32;
    // *v
    auto* v = Var("v", ty.i32());
    auto* expr = Deref(Expr(Source{{12, 34}}, "v"));

    WrapInFunction(v, expr);

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: cannot dereference expression of type 'i32')");
}

TEST_F(ResolverPtrRefValidationTest, InferredPtrAccessMismatch) {
    // struct Inner {
    //    arr: array<i32, 4u>;
    // }
    // struct S {
    //    inner: Inner;
    // }
    // @group(0) @binding(0) var<storage, read_write> s : S;
    // fn f() {
    //   let p : pointer<storage, i32> = &s.inner.arr[2i];
    // }
    auto* inner = Structure("Inner", Vector{Member("arr", ty.array<i32, 4>())});
    auto* buf = Structure("S", Vector{Member("inner", ty.Of(inner))});
    auto* var = GlobalVar("s", ty.Of(buf), core::AddressSpace::kStorage, core::Access::kReadWrite,
                          Binding(0_a), Group(0_a));

    auto* expr = IndexAccessor(MemberAccessor(MemberAccessor(var, "inner"), "arr"), 2_i);
    auto* ptr = Let(Source{{12, 34}}, "p", ty.ptr<storage, i32>(), AddressOf(expr));

    WrapInFunction(ptr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: cannot initialize 'let' of type "
              "'ptr<storage, i32, read>' with value of type "
              "'ptr<storage, i32, read_write>'");
}

}  // namespace
}  // namespace tint::resolver
