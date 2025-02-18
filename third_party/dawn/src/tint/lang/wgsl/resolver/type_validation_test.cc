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

#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

#include "gmock/gmock.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <typename T>
using alias = builder::alias<T>;
template <typename T>
using alias1 = builder::alias1<T>;
template <typename T>
using alias2 = builder::alias2<T>;
template <typename T>
using alias3 = builder::alias3<T>;

class ResolverTypeValidationTest : public resolver::TestHelper, public testing::Test {};

TEST_F(ResolverTypeValidationTest, VariableDeclNoInitializer_Pass) {
    // {
    // var a :i32;
    // a = 2;
    // }
    auto* var = Var("a", ty.i32());
    auto* lhs = Expr("a");
    auto* rhs = Expr(2_i);

    auto* body = Block(Decl(var), Assign(Source{Source::Location{12, 34}}, lhs, rhs));

    WrapInFunction(body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_NE(TypeOf(lhs), nullptr);
    ASSERT_NE(TypeOf(rhs), nullptr);
}

TEST_F(ResolverTypeValidationTest, GlobalOverrideNoInitializer_Pass) {
    // @id(0) override a :i32;
    Override(Source{{12, 34}}, "a", ty.i32(), Id(0_u));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableWithAddressSpace_Pass) {
    // var<private> global_var: f32;
    GlobalVar(Source{{12, 34}}, "global_var", ty.f32(), core::AddressSpace::kPrivate);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalConstNoAddressSpace_Pass) {
    // const global_const: f32 = f32();
    GlobalConst(Source{{12, 34}}, "global_const", ty.f32(), Call<f32>());

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableUnique_Pass) {
    // var global_var0 : f32 = 0.1;
    // var global_var1 : i32 = 0;

    GlobalVar("global_var0", ty.f32(), core::AddressSpace::kPrivate, Expr(0.1_f));

    GlobalVar(Source{{12, 34}}, "global_var1", ty.f32(), core::AddressSpace::kPrivate, Expr(1_f));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, GlobalVariableFunctionVariableNotUnique_Pass) {
    // fn my_func() {
    //   var a: f32 = 2.0;
    // }
    // var a: f32 = 2.1;

    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("a", ty.f32(), Expr(2_f))),
         });

    GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate, Expr(2.1_f));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierInnerScope_Pass) {
    // {
    // if (true) { var a : f32 = 2.0; }
    // var a : f32 = 3.14;
    // }
    auto* var = Var("a", ty.f32(), Expr(2_f));

    auto* cond = Expr(true);
    auto* body = Block(Decl(var));

    auto* var_a_float = Var("a", ty.f32(), Expr(3.1_f));

    auto* outer_body = Block(If(cond, body), Decl(Source{{12, 34}}, var_a_float));

    WrapInFunction(outer_body);

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierInnerScopeBlock_Pass) {
    // {
    //  { var a : f32; }
    //  var a : f32;
    // }
    auto* var_inner = Var("a", ty.f32());
    auto* inner = Block(Decl(Source{{12, 34}}, var_inner));

    auto* var_outer = Var("a", ty.f32());
    auto* outer_body = Block(inner, Decl(var_outer));

    WrapInFunction(outer_body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RedeclaredIdentifierDifferentFunctions_Pass) {
    // func0 { var a : f32 = 2.0; return; }
    // func1 { var a : f32 = 3.0; return; }
    auto* var0 = Var("a", ty.f32(), Expr(2_f));

    auto* var1 = Var("a", ty.f32(), Expr(1_f));

    Func("func0", tint::Empty, ty.void_(),
         Vector{
             Decl(Source{{12, 34}}, var0),
             Return(),
         });

    Func("func1", tint::Empty, ty.void_(),
         Vector{
             Decl(Source{{13, 34}}, var1),
             Return(),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_AIntLiteral_Pass) {
    // var<private> a : array<f32, 4>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 4_a)), core::AddressSpace::kPrivate);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnsignedLiteral_Pass) {
    // var<private> a : array<f32, 4u>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 4_u)), core::AddressSpace::kPrivate);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedLiteral_Pass) {
    // var<private> a : array<f32, 4i>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 4_i)), core::AddressSpace::kPrivate);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnsignedConst_Pass) {
    // const size = 4u;
    // var<private> a : array<f32, size>;
    GlobalConst("size", Expr(4_u));
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
              core::AddressSpace::kPrivate);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedConst_Pass) {
    // const size = 4i;
    // var<private> a : array<f32, size>;
    GlobalConst("size", Expr(4_i));
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
              core::AddressSpace::kPrivate);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_AIntLiteral_Zero) {
    // var<private> a : array<f32, 0>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 0_a)), core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array count (0) must be greater than 0");
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnsignedLiteral_Zero) {
    // var<private> a : array<f32, 0u>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 0_u)), core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array count (0) must be greater than 0");
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedLiteral_Zero) {
    // var<private> a : array<f32, 0i>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 0_i)), core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array count (0) must be greater than 0");
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedLiteral_Negative) {
    // var<private> a : array<f32, -10i>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, -10_i)), core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array count (-10) must be greater than 0");
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnsignedConst_Zero) {
    // const size = 0u;
    // var<private> a : array<f32, size>;
    GlobalConst("size", Expr(0_u));
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array count (0) must be greater than 0");
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedConst_Zero) {
    // const size = 0i;
    // var<private> a : array<f32, size>;
    GlobalConst("size", Expr(0_i));
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array count (0) must be greater than 0");
}

TEST_F(ResolverTypeValidationTest, ArraySize_SignedConst_Negative) {
    // const size = -10i;
    // var<private> a : array<f32, size>;
    GlobalConst("size", Expr(-10_i));
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: array count (-10) must be greater than 0");
}

TEST_F(ResolverTypeValidationTest, ArraySize_FloatLiteral) {
    // var<private> a : array<f32, 10.0>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 10_f)), core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: array count must evaluate to a constant integer expression, but is type "
        "'f32'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_IVecLiteral) {
    // var<private> a : array<f32, vec2<i32>(10, 10)>;
    GlobalVar("a", ty.array(ty.f32(), Call(Source{{12, 34}}, ty.vec2<i32>(), 10_i, 10_i)),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: array count must evaluate to a constant integer expression, but is type "
        "'vec2<i32>'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_FloatConst) {
    // const size = 10.0;
    // var<private> a : array<f32, size>;
    GlobalConst("size", Expr(10_f));
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: array count must evaluate to a constant integer expression, but is type "
        "'f32'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_IVecConst) {
    // const size = vec2<i32>(100, 100);
    // var<private> a : array<f32, size>;
    GlobalConst("size", Call<vec2<i32>>(100_i, 100_i));
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        "12:34 error: array count must evaluate to a constant integer expression, but is type "
        "'vec2<i32>'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnderElementCountLimit) {
    // var<private> a : array<f32, 65535>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 65535_a)),
              core::AddressSpace::kPrivate);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_OverElementCountLimit) {
    // var<private> a : array<f32, 65536>;
    GlobalVar(Source{{56, 78}}, "a", ty.array(Source{{12, 34}}, ty.f32(), Expr(65536_a)),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: array count (65536) must be less than 65536
56:78 note: while instantiating 'var' a)");
}

TEST_F(ResolverTypeValidationTest, ArraySize_StorageBufferLargeArray) {
    // var<storage> a : array<f32, 65536>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 65536_a)),
              core::AddressSpace::kStorage, Vector{Binding(0_u), Group(0_u)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_NestedStorageBufferLargeArray) {
    // Struct S {
    //  a : array<f32, 65536>,
    // }
    // var<storage> a : S;
    Structure("S",
              Vector{Member(Source{{12, 34}}, "a", ty.array(Source{{12, 20}}, ty.f32(), 65536_a))});
    GlobalVar("a", ty(Source{{12, 30}}, "S"), core::AddressSpace::kStorage,
              Vector{Binding(0_u), Group(0_u)});
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_TooBig_ImplicitStride) {
    // struct S {
    //   @offset(800000) a : f32
    // }
    // var<private> a : array<S, 65535>;
    Structure("S", Vector{Member(Source{{12, 34}}, "a", ty.f32(), Vector{MemberOffset(800000_a)})});
    GlobalVar("a", ty.array(ty(Source{{12, 30}}, "S"), Expr(Source{{12, 34}}, 65535_a)),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: array byte size (0xc34f7cafc) must not exceed 0xffffffff bytes");
}

TEST_F(ResolverTypeValidationTest, ArraySize_TooBig_ExplicitStride) {
    // var<private> a : @stride(8000000) array<f32, 65535>;
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, 65535_a), Vector{Stride(8000000)}),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: array byte size (0x7a1185ee00) must not exceed 0xffffffff bytes");
}

TEST_F(ResolverTypeValidationTest, ArraySize_NamedOverride_PrivateVar) {
    // override size = 10i;
    // var<private> a : array<f32, size>;
    Override("size", Expr(10_i));
    GlobalVar("a", ty.array(Source{{12, 34}}, ty.f32(), "size"), core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'array' with an 'override' element count can only be used as the store "
              "type of a 'var<workgroup>'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_NamedOverride_InArray) {
    // override size = 10i;
    // var<workgroup> a : array<array<f32, size>, 4>;
    Override("size", Expr(10_i));
    GlobalVar("a", ty.array(ty.array(Source{{12, 34}}, ty.f32(), "size"), 4_a),
              core::AddressSpace::kWorkgroup);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'array' with an 'override' element count can only be used as the store "
              "type of a 'var<workgroup>'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_NamedOverride_InStruct) {
    // override size = 10i;
    // struct S {
    //   a : array<f32, size>
    // };
    Override("size", Expr(10_i));
    Structure("S", Vector{Member("a", ty.array(Source{{12, 34}}, ty.f32(), "size"))});
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'array' with an 'override' element count can only be used as the store "
              "type of a 'var<workgroup>'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_NamedOverride_FunctionVar_Explicit) {
    // override size = 10i;
    // fn f() {
    //   var a : array<f32, size>;
    // }
    Override("size", Expr(10_i));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("a", ty.array(Source{{12, 34}}, ty.f32(), "size"))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'array' with an 'override' element count can only be used as the store "
              "type of a 'var<workgroup>'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_NamedOverride_FunctionLet_Explicit) {
    // override size = 10i;
    // fn f() {
    //   var a : array<f32, size>;
    // }
    Override("size", Expr(10_i));
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("a", ty.array(Source{{12, 34}}, ty.f32(), "size"))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'array' with an 'override' element count can only be used as the store "
              "type of a 'var<workgroup>'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_NamedOverride_FunctionVar_Implicit) {
    // override size = 10i;
    // var<workgroup> w : array<f32, size>;
    // fn f() {
    //   var a = w;
    // }
    Override("size", Expr(10_i));
    GlobalVar("w", ty.array(ty.f32(), "size"), core::AddressSpace::kWorkgroup);
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("a", Expr(Source{{12, 34}}, "w"))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'array' with an 'override' element count can only be used as the store "
              "type of a 'var<workgroup>'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_NamedOverride_FunctionLet_Implicit) {
    // override size = 10i;
    // var<workgroup> w : array<f32, size>;
    // fn f() {
    //   let a = w;
    // }
    Override("size", Expr(10_i));
    GlobalVar("w", ty.array(ty.f32(), "size"), core::AddressSpace::kWorkgroup);
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("a", Expr(Source{{12, 34}}, "w"))),
         });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: 'array' with an 'override' element count can only be used as the store "
              "type of a 'var<workgroup>'");
}

TEST_F(ResolverTypeValidationTest, ArraySize_UnnamedOverride_Equivalence) {
    // override size = 10i;
    // var<workgroup> a : array<f32, size + 1>;
    // var<workgroup> b : array<f32, size + 1>;
    // fn f() {
    //   a = b;
    // }
    Override("size", Expr(10_i));
    GlobalVar("a", ty.array(ty.f32(), Add("size", 1_i)), core::AddressSpace::kWorkgroup);
    GlobalVar("b", ty.array(ty.f32(), Add("size", 1_i)), core::AddressSpace::kWorkgroup);
    WrapInFunction(Assign(Source{{12, 34}}, "a", "b"));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: cannot assign 'array<f32, [unnamed override-expression]>' to 'array<f32, [unnamed override-expression]>')");
}

TEST_F(ResolverTypeValidationTest, ArraySize_NamedOverride_Param) {
    // override size = 10i;
    // fn f(a : array<f32, size>) {
    // }
    Override("size", Expr(10_i));
    Func("f", Vector{Param("a", ty.array(Source{{12, 34}}, ty.f32(), "size"))}, ty.void_(),
         tint::Empty);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: type of function parameter must be constructible");
}

TEST_F(ResolverTypeValidationTest, ArraySize_NamedOverride_ReturnType) {
    // override size = 10i;
    // fn f() -> array<f32, size> {
    // }
    Override("size", Expr(10_i));
    Func("f", tint::Empty, ty.array(Source{{12, 34}}, ty.f32(), "size"), tint::Empty);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: function return type must be a constructible type");
}

TEST_F(ResolverTypeValidationTest, ArraySize_Workgroup_Overridable) {
    // override size = 10i;
    // var<workgroup> a : array<f32, size>;
    Override("size", Expr(10_i));
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
              core::AddressSpace::kWorkgroup);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_ModuleVar) {
    // var<private> size : i32 = 10i;
    // var<private> a : array<f32, size>;
    GlobalVar("size", ty.i32(), Expr(10_i), core::AddressSpace::kPrivate);
    GlobalVar("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: 'var size' cannot be referenced at module-scope
note: 'var size' declared here)");
}

TEST_F(ResolverTypeValidationTest, ArraySize_FunctionConst) {
    // {
    //   const size = 10;
    //   var a : array<f32, size>;
    // }
    auto* size = Const("size", Expr(10_i));
    auto* a = Var("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")));
    WrapInFunction(size, a);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArraySize_FunctionLet) {
    // {
    //   let size = 10;
    //   var a : array<f32, size>;
    // }
    auto* size = Let("size", Expr(10_i));
    auto* a = Var("a", ty.array(ty.f32(), Expr(Source{{12, 34}}, "size")));
    WrapInFunction(size, a);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: array count must evaluate to a constant integer expression or override "
              "variable");
}

TEST_F(ResolverTypeValidationTest, ArraySize_ComplexExpr) {
    // var a : array<f32, i32(4i)>;
    auto* a = Var("a", ty.array(ty.f32(), Call(Source{{12, 34}}, ty.i32(), 4_i)));
    WrapInFunction(a);
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInFunction_Fail) {
    /// @vertex
    // fn func() { var a : array<i32>; }

    auto* var = Var(Source{{56, 78}}, "a", ty.array(Source{{12, 34}}, ty.i32()));

    Func("func", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating 'var' a)");
}

TEST_F(ResolverTypeValidationTest, PtrType_ArrayIncomplete) {
    // fn f(l: ptr<function, array>) {}

    Func("f",
         Vector{
             Param("l", ty.ptr(function, ty(Source{{12, 34}}, "array"))),
         },
         ty.void_(), Empty);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'array'");
}

TEST_F(ResolverTypeValidationTest, Struct_Member_VectorIncomplete) {
    // struct S {
    //   a: vec3;
    // };

    Structure("S", Vector{
                       Member("a", ty.vec3<Infer>(Source{{12, 34}})),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'vec3'");
}

TEST_F(ResolverTypeValidationTest, Struct_Member_MatrixIncomplete) {
    // struct S {
    //   a: mat3x3;
    // };
    Structure("S", Vector{
                       Member("a", ty.mat3x3<Infer>(Source{{12, 34}})),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'mat3x3'");
}

TEST_F(ResolverTypeValidationTest, Struct_TooBig) {
    // Struct Bar {
    //   a: array<f32, 10000>;
    // }
    // struct Foo {
    //   a: array<Bar, 65535>;
    //   b: array<Bar, 65535>;
    // }

    Structure(Source{{10, 34}}, "Bar", Vector{Member("a", ty.array<f32, 10000>())});
    Structure(Source{{12, 34}}, "Foo",
              Vector{Member("a", ty.array(ty(Source{{12, 30}}, "Bar"), Expr(65535_a))),
                     Member("b", ty.array(ty("Bar"), Expr(65535_a)))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: struct size (0x1387ec780) must not exceed 0xffffffff bytes");
}

TEST_F(ResolverTypeValidationTest, Struct_MemberOffset_TooBig) {
    // struct Foo {
    //   @size(2147483647) a: array<f32, 65535>;
    //   b: f32;
    //   c: f32;
    // };

    Structure("Foo", Vector{
                         Member("z", ty.f32(), Vector{MemberSize(2147483647_a)}),
                         Member("y", ty.f32(), Vector{MemberSize(2147483647_a)}),
                         Member(Source{{12, 34}}, "a", ty.array<f32, 65535>()),
                         Member("c", ty.f32()),
                     });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: struct member offset (0x100000000) must not exceed 0xffffffff bytes");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsLast_Pass) {
    // struct Foo {
    //   vf: f32;
    //   rt: array<f32>;
    // };

    Structure("Foo", Vector{
                         Member("vf", ty.f32()),
                         Member("rt", ty.array<f32>()),
                     });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInArray) {
    // struct Foo {
    //   rt : array<array<f32>, 4u>;
    // };

    Structure("Foo", Vector{
                         Member("rt", ty.array(ty.array(Source{{12, 34}}, ty.f32()), 4_u)),
                     });

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: an array element type cannot contain a runtime-sized array");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInStructInArray) {
    // struct Foo {
    //   rt : array<f32>;
    // };
    // var<private> a : array<Foo, 4>;

    Structure("Foo", Vector{Member("rt", ty.array<f32>())});
    GlobalVar("v", ty.array(ty(Source{{12, 34}}, "Foo"), 4_u), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: an array element type cannot contain a runtime-sized array");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayInStructInStruct) {
    // struct Foo {
    //   rt : array<f32>;
    // };
    // struct Outer {
    //   inner : Foo;
    // };

    auto* foo = Structure("Foo", Vector{
                                     Member("rt", ty.array<f32>()),
                                 });
    Structure("Outer", Vector{
                           Member(Source{{12, 34}}, "inner", ty.Of(foo)),
                       });

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: a struct that contains a runtime array cannot be nested inside another "
              "struct");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayIsNotLast_Fail) {
    // struct Foo {
    //   rt: array<f32>;
    //   vf: f32;
    // };

    Structure("Foo", Vector{
                         Member(Source{{12, 34}}, "rt", ty.array<f32>()),
                         Member("vf", ty.f32()),
                     });

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime arrays may only appear as the last member of a struct)");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsGlobalVariable) {
    GlobalVar(Source{{56, 78}}, "g", ty.array(Source{{12, 34}}, ty.i32()),
              core::AddressSpace::kPrivate);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsLocalVariable) {
    auto* v = Var(Source{{56, 78}}, "g", ty.array(Source{{12, 34}}, ty.i32()));
    WrapInFunction(v);

    ASSERT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating 'var' g)");
}

TEST_F(ResolverTypeValidationTest, RuntimeArrayAsParameter_Fail) {
    // fn func(a : array<u32>) {}
    // @vertex fn main() {}

    auto* param = Param(Source{{56, 78}}, "a", ty.array(Source{{12, 34}}, ty.i32()));

    Func("func", Vector{param}, ty.void_(),
         Vector{
             Return(),
         });

    Func("main", tint::Empty, ty.void_(),
         Vector{
             Return(),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         });

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating parameter a)");
}

TEST_F(ResolverTypeValidationTest, PtrToPtr_Fail) {
    // fn func(a : ptr<workgroup, ptr<workgroup, u32>>) {}
    auto* param = Param("a", ty.ptr(workgroup, ty.ptr<workgroup, u32>(Source{{12, 34}})));

    Func("func", Vector{param}, ty.void_(),
         Vector{
             Return(),
         });

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: ptr<workgroup, u32, read_write> cannot be used as the store type of a pointer)");
}

TEST_F(ResolverTypeValidationTest, PtrToRuntimeArrayAsPointerParameter_Fail) {
    // fn func(a : ptr<workgroup, array<u32>>) {}

    auto* param = Param("a", ty.ptr(Source{{56, 78}}, core::AddressSpace::kWorkgroup,
                                    ty.array(Source{{12, 34}}, ty.i32())));

    Func("func", Vector{param}, ty.void_(),
         Vector{
             Return(),
         });

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating ptr<workgroup, array<i32>, read_write>)");
}

TEST_F(ResolverTypeValidationTest, PtrToRuntimeArrayAsParameter_Fail) {
    // fn func(a : ptr<workgroup, array<u32>>) {}

    auto* param = Param(Source{{56, 78}}, "a", ty.array(Source{{12, 34}}, ty.i32()));

    Func("func", Vector{param}, ty.void_(),
         Vector{
             Return(),
         });

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 error: runtime-sized arrays can only be used in the <storage> address space
56:78 note: while instantiating parameter a)");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsNotLast_Fail) {
    // type RTArr = array<u32>;
    // struct s {
    //  b: RTArr;
    //  a: u32;
    //}

    auto* alias = Alias("RTArr", ty.array<u32>());
    Structure("s", Vector{
                       Member(Source{{12, 34}}, "b", ty.Of(alias)),
                       Member("a", ty.u32()),
                   });

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              "12:34 error: runtime arrays may only appear as the last member of a struct");
}

TEST_F(ResolverTypeValidationTest, AliasRuntimeArrayIsLast_Pass) {
    // type RTArr = array<u32>;
    // struct s {
    //  a: u32;
    //  b: RTArr;
    //}

    auto* alias = Alias("RTArr", ty.array<u32>());
    Structure("s", Vector{
                       Member("a", ty.u32()),
                       Member("b", ty.Of(alias)),
                   });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTypeValidationTest, ArrayOfNonStorableType) {
    auto tex_ty = ty.sampled_texture(Source{{12, 34}}, core::type::TextureDimension::k2d, ty.f32());
    GlobalVar("arr", ty.array(tex_ty, 4_i), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: texture_2d<f32> cannot be used as an element type of an array");
}

TEST_F(ResolverTypeValidationTest, ArrayOfNonStorableTypeWithStride) {
    auto ptr_ty = ty.ptr<uniform, u32>(Source{{12, 34}});
    GlobalVar("arr", ty.array(ptr_ty, 4_i, Vector{Stride(16)}), core::AddressSpace::kPrivate);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: ptr<uniform, u32, read> cannot be used as an element type of an array");
}

namespace GetCanonicalTests {
struct Params {
    builder::ast_type_func_ptr create_ast_type;
    builder::sem_type_func_ptr create_sem_type;
};

template <typename T>
constexpr Params ParamsFor() {
    return Params{DataType<T>::AST, DataType<T>::Sem};
}

static constexpr Params cases[] = {
    ParamsFor<bool>(),
    ParamsFor<alias<bool>>(),
    ParamsFor<alias1<alias<bool>>>(),

    ParamsFor<vec3<f32>>(),
    ParamsFor<alias<vec3<f32>>>(),
    ParamsFor<alias1<alias<vec3<f32>>>>(),

    ParamsFor<vec3<alias<f32>>>(),
    ParamsFor<alias1<vec3<alias<f32>>>>(),
    ParamsFor<alias2<alias1<vec3<alias<f32>>>>>(),
    ParamsFor<alias3<alias2<vec3<alias1<alias<f32>>>>>>(),

    ParamsFor<mat3x3<alias<f32>>>(),
    ParamsFor<alias1<mat3x3<alias<f32>>>>(),
    ParamsFor<alias2<alias1<mat3x3<alias<f32>>>>>(),
    ParamsFor<alias3<alias2<mat3x3<alias1<alias<f32>>>>>>(),

    ParamsFor<alias1<alias<bool>>>(),
    ParamsFor<alias1<alias<vec3<f32>>>>(),
    ParamsFor<alias1<alias<mat3x3<f32>>>>(),
};

using CanonicalTest = ResolverTestWithParam<Params>;
TEST_P(CanonicalTest, All) {
    auto& params = GetParam();

    ast::Type type = params.create_ast_type(*this);

    auto* var = Var("v", type);
    auto* expr = Expr("v");
    WrapInFunction(var, expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* got = TypeOf(expr)->UnwrapRef();
    auto* expected = params.create_sem_type(*this);

    EXPECT_EQ(got, expected) << "got:      " << FriendlyName(got) << "\n"
                             << "expected: " << FriendlyName(expected) << "\n";
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest, CanonicalTest, testing::ValuesIn(cases));

}  // namespace GetCanonicalTests

namespace SampledTextureTests {

using SampledTextureDimensionTest = ResolverTestWithParam<core::type::TextureDimension>;
TEST_P(SampledTextureDimensionTest, All) {
    auto& params = GetParam();
    GlobalVar(Source{{12, 34}}, "a", ty.sampled_texture(params, ty.i32()), Group(0_a),
              Binding(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         SampledTextureDimensionTest,
                         testing::Values(  //
                             core::type::TextureDimension::k1d,
                             core::type::TextureDimension::k2d,
                             core::type::TextureDimension::k2dArray,
                             core::type::TextureDimension::k3d,
                             core::type::TextureDimension::kCube,
                             core::type::TextureDimension::kCubeArray));

using MultisampledTextureDimensionTest = ResolverTestWithParam<core::type::TextureDimension>;
TEST_P(MultisampledTextureDimensionTest, All) {
    auto& params = GetParam();
    GlobalVar("a", ty.multisampled_texture(Source{{12, 34}}, params, ty.i32()), Group(0_a),
              Binding(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         MultisampledTextureDimensionTest,
                         testing::Values(core::type::TextureDimension::k2d));

struct TypeParams {
    builder::ast_type_func_ptr type_func;
    bool is_valid;
};

template <typename T>
constexpr TypeParams TypeParamsFor(bool is_valid) {
    return TypeParams{DataType<T>::AST, is_valid};
}

static constexpr TypeParams type_cases[] = {
    TypeParamsFor<bool>(false),
    TypeParamsFor<i32>(true),
    TypeParamsFor<u32>(true),
    TypeParamsFor<f32>(true),
    TypeParamsFor<f16>(false),

    TypeParamsFor<alias<bool>>(false),
    TypeParamsFor<alias<i32>>(true),
    TypeParamsFor<alias<u32>>(true),
    TypeParamsFor<alias<f32>>(true),
    TypeParamsFor<alias<f16>>(false),

    TypeParamsFor<vec3<f32>>(false),
    TypeParamsFor<mat3x3<f32>>(false),
    TypeParamsFor<mat3x3<f16>>(false),

    TypeParamsFor<alias<vec3<f32>>>(false),
    TypeParamsFor<alias<mat3x3<f32>>>(false),
    TypeParamsFor<alias<mat3x3<f16>>>(false),
};

using SampledTextureTypeTest = ResolverTestWithParam<TypeParams>;
TEST_P(SampledTextureTypeTest, All) {
    auto& params = GetParam();
    Enable(wgsl::Extension::kF16);
    GlobalVar("a",
              ty.sampled_texture(Source{{12, 34}}, core::type::TextureDimension::k2d,
                                 params.type_func(*this)),
              Group(0_a), Binding(0_a));

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "12:34 error: texture_2d<type>: type must be f32, i32 or u32");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         SampledTextureTypeTest,
                         testing::ValuesIn(type_cases));

using MultisampledTextureTypeTest = ResolverTestWithParam<TypeParams>;
TEST_P(MultisampledTextureTypeTest, All) {
    auto& params = GetParam();
    Enable(wgsl::Extension::kF16);
    GlobalVar("a",
              ty.multisampled_texture(Source{{12, 34}}, core::type::TextureDimension::k2d,
                                      params.type_func(*this)),
              Group(0_a), Binding(0_a));

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: texture_multisampled_2d<type>: type must be f32, i32 or u32");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         MultisampledTextureTypeTest,
                         testing::ValuesIn(type_cases));

}  // namespace SampledTextureTests

namespace StorageTextureTests {
struct DimensionParams {
    const char* name;
    bool is_valid;
};

static constexpr DimensionParams Dimension_cases[] = {
    DimensionParams{"texture_storage_1d", true},
    DimensionParams{"texture_storage_2d", true},
    DimensionParams{"texture_storage_2d_array", true},
    DimensionParams{"texture_storage_3d", true},
    DimensionParams{"texture_storage_cube", false},
    DimensionParams{"texture_storage_cube_array", false}};

using StorageTextureDimensionTest = ResolverTestWithParam<DimensionParams>;
TEST_P(StorageTextureDimensionTest, All) {
    // @group(0) @binding(0)
    // var a : texture_storage_*<r32uint, write>;
    auto& params = GetParam();

    auto st = ty(Source{{12, 34}}, params.name, tint::ToString(core::TexelFormat::kR32Uint),
                 tint::ToString(core::Access::kWrite));

    GlobalVar("a", st, Group(0_a), Binding(0_a));

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(), "12:34 error: unresolved type '" + std::string(params.name) + "'");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         StorageTextureDimensionTest,
                         testing::ValuesIn(Dimension_cases));

struct FormatParams {
    core::TexelFormat format;
    bool is_valid;
};

static constexpr FormatParams format_cases[] = {FormatParams{core::TexelFormat::kBgra8Unorm, true},
                                                FormatParams{core::TexelFormat::kR32Float, true},
                                                FormatParams{core::TexelFormat::kR32Sint, true},
                                                FormatParams{core::TexelFormat::kR32Uint, true},
                                                FormatParams{core::TexelFormat::kRg32Float, true},
                                                FormatParams{core::TexelFormat::kRg32Sint, true},
                                                FormatParams{core::TexelFormat::kRg32Uint, true},
                                                FormatParams{core::TexelFormat::kRgba16Float, true},
                                                FormatParams{core::TexelFormat::kRgba16Sint, true},
                                                FormatParams{core::TexelFormat::kRgba16Uint, true},
                                                FormatParams{core::TexelFormat::kRgba32Float, true},
                                                FormatParams{core::TexelFormat::kRgba32Sint, true},
                                                FormatParams{core::TexelFormat::kRgba32Uint, true},
                                                FormatParams{core::TexelFormat::kRgba8Sint, true},
                                                FormatParams{core::TexelFormat::kRgba8Snorm, true},
                                                FormatParams{core::TexelFormat::kRgba8Uint, true},
                                                FormatParams{core::TexelFormat::kRgba8Unorm, true}};

using StorageTextureFormatTest = ResolverTestWithParam<FormatParams>;
TEST_P(StorageTextureFormatTest, All) {
    auto& params = GetParam();
    // @group(0) @binding(0)
    // var a : texture_storage_1d<*, write>;
    // @group(0) @binding(1)
    // var b : texture_storage_2d<*, write>;
    // @group(0) @binding(2)
    // var c : texture_storage_2d_array<*, write>;
    // @group(0) @binding(3)
    // var d : texture_storage_3d<*, write>;

    auto st_a = ty.storage_texture(Source{{12, 34}}, core::type::TextureDimension::k1d,
                                   params.format, core::Access::kWrite);
    GlobalVar("a", st_a, Group(0_a), Binding(0_a));

    ast::Type st_b =
        ty.storage_texture(core::type::TextureDimension::k2d, params.format, core::Access::kWrite);
    GlobalVar("b", st_b, Group(0_a), Binding(1_a));

    ast::Type st_c = ty.storage_texture(core::type::TextureDimension::k2dArray, params.format,
                                        core::Access::kWrite);
    GlobalVar("c", st_c, Group(0_a), Binding(2_a));

    ast::Type st_d =
        ty.storage_texture(core::type::TextureDimension::k3d, params.format, core::Access::kWrite);
    GlobalVar("d", st_d, Group(0_a), Binding(3_a));

    if (params.is_valid) {
        EXPECT_TRUE(r()->Resolve()) << r()->error();
    } else {
        EXPECT_FALSE(r()->Resolve());
        EXPECT_EQ(r()->error(),
                  "12:34 error: image format must be one of the texel formats specified for "
                  "storage textures in https://gpuweb.github.io/gpuweb/wgsl/#texel-formats");
    }
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         StorageTextureFormatTest,
                         testing::ValuesIn(format_cases));

using StorageTextureAccessTest = ResolverTest;

TEST_F(StorageTextureAccessTest, MissingTemplates) {
    // @group(0) @binding(0)
    // var a : texture_storage_1d<r32uint>;

    auto st = ty(Source{{12, 34}}, "texture_storage_1d");

    GlobalVar("a", st, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: expected '<' for 'texture_storage_1d'");
}

TEST_F(StorageTextureAccessTest, MissingAccess_Fail) {
    // @group(0) @binding(0)
    // var a : texture_storage_1d<r32uint>;

    auto st = ty(Source{{12, 34}}, "texture_storage_1d", "r32uint");

    GlobalVar("a", st, Group(0_a), Binding(0_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: 'texture_storage_1d' requires 2 template arguments)");
}

TEST_F(StorageTextureAccessTest, WriteOnlyAccess_Pass) {
    // @group(0) @binding(0)
    // var a : texture_storage_1d<r32uint, write>;

    auto st = ty.storage_texture(core::type::TextureDimension::k1d, core::TexelFormat::kR32Uint,
                                 core::Access::kWrite);

    GlobalVar("a", st, Group(0_a), Binding(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StorageTextureAccessTest, ReadOnlyAccess_Pass) {
    // @group(0) @binding(0)
    // var a : texture_storage_1d<r32uint, read>;

    auto st = ty.storage_texture(Source{{12, 34}}, core::type::TextureDimension::k1d,
                                 core::TexelFormat::kR32Uint, core::Access::kRead);

    GlobalVar("a", st, Group(0_a), Binding(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StorageTextureAccessTest, ReadOnlyAccess_FeatureDisallowed) {
    // @group(0) @binding(0)
    // var a : texture_storage_1d<r32uint, read>;

    auto st = ty.storage_texture(Source{{12, 34}}, core::type::TextureDimension::k1d,
                                 core::TexelFormat::kR32Uint, core::Access::kRead);

    GlobalVar("a", st, Group(0_a), Binding(0_a));

    auto resolver = Resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: read-only storage textures require the "
              "readonly_and_readwrite_storage_textures language feature, which is not allowed in "
              "the current environment");
}

TEST_F(StorageTextureAccessTest, RWAccess_Pass) {
    // @group(0) @binding(0)
    // var a : texture_storage_1d<r32uint, read_write>;

    auto st = ty.storage_texture(Source{{12, 34}}, core::type::TextureDimension::k1d,
                                 core::TexelFormat::kR32Uint, core::Access::kReadWrite);

    GlobalVar("a", st, Group(0_a), Binding(0_a));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(StorageTextureAccessTest, RWAccess_FeatureDisallowed) {
    // @group(0) @binding(0)
    // var a : texture_storage_1d<r32uint, read_write>;

    auto st = ty.storage_texture(Source{{12, 34}}, core::type::TextureDimension::k1d,
                                 core::TexelFormat::kR32Uint, core::Access::kReadWrite);

    GlobalVar("a", st, Group(0_a), Binding(0_a));

    Resolver resolver{this, wgsl::AllowedFeatures{}};
    EXPECT_FALSE(resolver.Resolve());
    EXPECT_EQ(resolver.error(),
              "12:34 error: read-write storage textures require the "
              "readonly_and_readwrite_storage_textures language feature, which is not allowed in "
              "the current environment");
}

}  // namespace StorageTextureTests

namespace MatrixTests {
struct Params {
    uint32_t columns;
    uint32_t rows;
    builder::ast_type_func_ptr elem_ty;
};

template <typename T>
constexpr Params ParamsFor(uint32_t columns, uint32_t rows) {
    return Params{columns, rows, DataType<T>::AST};
}

using ValidMatrixTypes = ResolverTestWithParam<Params>;
TEST_P(ValidMatrixTypes, Okay) {
    // enable f16;
    // var a : matNxM<EL_TY>;
    auto& params = GetParam();

    Enable(wgsl::Extension::kF16);

    ast::Type el_ty = params.elem_ty(*this);

    GlobalVar("a", ty.mat(el_ty, params.columns, params.rows), core::AddressSpace::kPrivate);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         ValidMatrixTypes,
                         testing::Values(ParamsFor<f32>(2, 2),
                                         ParamsFor<f32>(2, 3),
                                         ParamsFor<f32>(2, 4),
                                         ParamsFor<f32>(3, 2),
                                         ParamsFor<f32>(3, 3),
                                         ParamsFor<f32>(3, 4),
                                         ParamsFor<f32>(4, 2),
                                         ParamsFor<f32>(4, 3),
                                         ParamsFor<f32>(4, 4),
                                         ParamsFor<alias<f32>>(4, 2),
                                         ParamsFor<alias<f32>>(4, 3),
                                         ParamsFor<alias<f32>>(4, 4),
                                         ParamsFor<f16>(2, 2),
                                         ParamsFor<f16>(2, 3),
                                         ParamsFor<f16>(2, 4),
                                         ParamsFor<f16>(3, 2),
                                         ParamsFor<f16>(3, 3),
                                         ParamsFor<f16>(3, 4),
                                         ParamsFor<f16>(4, 2),
                                         ParamsFor<f16>(4, 3),
                                         ParamsFor<f16>(4, 4),
                                         ParamsFor<alias<f16>>(4, 2),
                                         ParamsFor<alias<f16>>(4, 3),
                                         ParamsFor<alias<f16>>(4, 4)));

using InvalidMatrixElementTypes = ResolverTestWithParam<Params>;
TEST_P(InvalidMatrixElementTypes, InvalidElementType) {
    // enable f16;
    // var a : matNxM<EL_TY>;
    auto& params = GetParam();

    Enable(wgsl::Extension::kF16);

    ast::Type el_ty = params.elem_ty(*this);

    GlobalVar("a", ty.mat(Source{{12, 34}}, el_ty, params.columns, params.rows),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: matrix element type must be 'f32' or 'f16'");
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         InvalidMatrixElementTypes,
                         testing::Values(ParamsFor<bool>(4, 2),
                                         ParamsFor<i32>(4, 3),
                                         ParamsFor<u32>(4, 4),
                                         ParamsFor<vec2<f32>>(2, 2),
                                         ParamsFor<vec2<f16>>(2, 2),
                                         ParamsFor<vec3<i32>>(2, 3),
                                         ParamsFor<vec4<u32>>(2, 4),
                                         ParamsFor<mat2x2<f32>>(3, 2),
                                         ParamsFor<mat3x3<f32>>(3, 3),
                                         ParamsFor<mat4x4<f32>>(3, 4),
                                         ParamsFor<mat2x2<f16>>(3, 2),
                                         ParamsFor<mat3x3<f16>>(3, 3),
                                         ParamsFor<mat4x4<f16>>(3, 4),
                                         ParamsFor<array<f32, 2>>(4, 2),
                                         ParamsFor<array<f16, 2>>(4, 2)));
}  // namespace MatrixTests

namespace VectorTests {
struct Params {
    uint32_t width;
    builder::ast_type_func_ptr elem_ty;
};

template <typename T>
constexpr Params ParamsFor(uint32_t width) {
    return Params{width, DataType<T>::AST};
}

using ValidVectorTypes = ResolverTestWithParam<Params>;
TEST_P(ValidVectorTypes, Okay) {
    // enable f16;
    // var a : vecN<EL_TY>;
    auto& params = GetParam();

    Enable(wgsl::Extension::kF16);

    GlobalVar("a", ty.vec(params.elem_ty(*this), params.width), core::AddressSpace::kPrivate);
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         ValidVectorTypes,
                         testing::Values(ParamsFor<bool>(2),
                                         ParamsFor<f32>(2),
                                         ParamsFor<f16>(2),
                                         ParamsFor<i32>(2),
                                         ParamsFor<u32>(2),
                                         ParamsFor<bool>(3),
                                         ParamsFor<f32>(3),
                                         ParamsFor<f16>(3),
                                         ParamsFor<i32>(3),
                                         ParamsFor<u32>(3),
                                         ParamsFor<bool>(4),
                                         ParamsFor<f32>(4),
                                         ParamsFor<f16>(4),
                                         ParamsFor<i32>(4),
                                         ParamsFor<u32>(4),
                                         ParamsFor<alias<bool>>(4),
                                         ParamsFor<alias<f32>>(4),
                                         ParamsFor<alias<f16>>(4),
                                         ParamsFor<alias<i32>>(4),
                                         ParamsFor<alias<u32>>(4)));

using InvalidVectorElementTypes = ResolverTestWithParam<Params>;
TEST_P(InvalidVectorElementTypes, InvalidElementType) {
    // enable f16;
    // var a : vecN<EL_TY>;
    auto& params = GetParam();

    Enable(wgsl::Extension::kF16);

    GlobalVar("a", ty.vec(Source{{12, 34}}, params.elem_ty(*this), params.width),
              core::AddressSpace::kPrivate);
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: vector element type must be 'bool', 'f32', 'f16', 'i32' or 'u32'");
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         InvalidVectorElementTypes,
                         testing::Values(ParamsFor<vec2<f32>>(2),
                                         ParamsFor<vec3<i32>>(2),
                                         ParamsFor<vec4<u32>>(2),
                                         ParamsFor<mat2x2<f32>>(2),
                                         ParamsFor<mat3x3<f16>>(2),
                                         ParamsFor<mat4x4<f32>>(2),
                                         ParamsFor<array<f32, 2>>(2)));
}  // namespace VectorTests

namespace BuiltinTypeAliasTests {
struct Params {
    const char* alias;
    builder::ast_type_func_ptr type;
};

template <typename T>
constexpr Params Case(const char* alias) {
    return Params{alias, DataType<T>::AST};
}

using BuiltinTypeAliasTest = ResolverTestWithParam<Params>;
TEST_P(BuiltinTypeAliasTest, CheckEquivalent) {
    // enable f16;
    // var aliased : vecTN;
    // var explicit : vecN<T>;
    // explicit = aliased;
    auto& params = GetParam();

    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Var("aliased", ty(params.alias))),
                   Decl(Var("explicit", params.type(*this))),  //
                   Assign("explicit", "aliased"));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
TEST_P(BuiltinTypeAliasTest, Construct) {
    // enable f16;
    // var v : vecN<T> = vecTN();
    auto& params = GetParam();

    Enable(wgsl::Extension::kF16);

    WrapInFunction(Decl(Var("v", params.type(*this), Call(params.alias))));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
INSTANTIATE_TEST_SUITE_P(ResolverTypeValidationTest,
                         BuiltinTypeAliasTest,
                         testing::Values(Case<mat2x2<f32>>("mat2x2f"),
                                         Case<mat2x3<f32>>("mat2x3f"),
                                         Case<mat2x4<f32>>("mat2x4f"),
                                         Case<mat3x2<f32>>("mat3x2f"),
                                         Case<mat3x3<f32>>("mat3x3f"),
                                         Case<mat3x4<f32>>("mat3x4f"),
                                         Case<mat4x2<f32>>("mat4x2f"),
                                         Case<mat4x3<f32>>("mat4x3f"),
                                         Case<mat4x4<f32>>("mat4x4f"),
                                         Case<mat2x2<f16>>("mat2x2h"),
                                         Case<mat2x3<f16>>("mat2x3h"),
                                         Case<mat2x4<f16>>("mat2x4h"),
                                         Case<mat3x2<f16>>("mat3x2h"),
                                         Case<mat3x3<f16>>("mat3x3h"),
                                         Case<mat3x4<f16>>("mat3x4h"),
                                         Case<mat4x2<f16>>("mat4x2h"),
                                         Case<mat4x3<f16>>("mat4x3h"),
                                         Case<mat4x4<f16>>("mat4x4h"),
                                         Case<vec2<f32>>("vec2f"),
                                         Case<vec3<f32>>("vec3f"),
                                         Case<vec4<f32>>("vec4f"),
                                         Case<vec2<f16>>("vec2h"),
                                         Case<vec3<f16>>("vec3h"),
                                         Case<vec4<f16>>("vec4h"),
                                         Case<vec2<i32>>("vec2i"),
                                         Case<vec3<i32>>("vec3i"),
                                         Case<vec4<i32>>("vec4i"),
                                         Case<vec2<u32>>("vec2u"),
                                         Case<vec3<u32>>("vec3u"),
                                         Case<vec4<u32>>("vec4u")));

}  // namespace BuiltinTypeAliasTests

namespace TypeDoesNotTakeTemplateArgs {

using ResolverUntemplatedTypeUsedWithTemplateArgs = ResolverTestWithParam<const char*>;

TEST_P(ResolverUntemplatedTypeUsedWithTemplateArgs, Builtin_UseWithTemplateArgs) {
    // enable f16;
    // var<private> v : f32<true>;

    Enable(wgsl::Extension::kF16);
    GlobalVar("v", core::AddressSpace::kPrivate, ty(Source{{12, 34}}, GetParam(), true));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: type '" + std::string(GetParam()) +
                                "' does not take template arguments");
}

TEST_P(ResolverUntemplatedTypeUsedWithTemplateArgs, BuiltinAlias_UseWithTemplateArgs) {
    // enable f16;
    // alias A = f32;
    // var<private> v : A<true>;

    Enable(wgsl::Extension::kF16);
    Alias(Source{{56, 78}}, "A", ty(GetParam()));
    GlobalVar("v", core::AddressSpace::kPrivate, ty(Source{{12, 34}}, "A", true));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: type 'A' does not take template arguments
56:78 note: 'alias A' declared here)");
}

INSTANTIATE_TEST_SUITE_P(BuiltinTypes,
                         ResolverUntemplatedTypeUsedWithTemplateArgs,
                         testing::Values("bool",
                                         "f16",
                                         "f32",
                                         "i32",
                                         "u32",
                                         "mat2x2f",
                                         "mat2x2h",
                                         "mat2x3f",
                                         "mat2x3h",
                                         "mat2x4f",
                                         "mat2x4h",
                                         "mat3x2f",
                                         "mat3x2h",
                                         "mat3x3f",
                                         "mat3x3h",
                                         "mat3x4f",
                                         "mat3x4h",
                                         "mat4x2f",
                                         "mat4x2h",
                                         "mat4x3f",
                                         "mat4x3h",
                                         "mat4x4f",
                                         "mat4x4h",
                                         "vec2f",
                                         "vec2h",
                                         "vec2i",
                                         "vec2u",
                                         "vec3f",
                                         "vec3h",
                                         "vec3i",
                                         "vec3u",
                                         "vec4f",
                                         "vec4h",
                                         "vec4i",
                                         "vec4u"));

TEST_F(ResolverUntemplatedTypeUsedWithTemplateArgs, Struct_Type) {
    // struct S {
    //   i: i32;
    // };
    // var<private> v : S<true>;

    Structure(Source{{56, 78}}, "S", Vector{Member("i", ty.i32())});
    GlobalVar("v", core::AddressSpace::kPrivate, ty(Source{{12, 34}}, "S", true));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: type 'S' does not take template arguments
56:78 note: 'struct S' declared here)");
}

TEST_F(ResolverUntemplatedTypeUsedWithTemplateArgs, Struct_Ctor) {
    // struct S { a : i32 }
    // var<private> v = S<true>();

    Structure("S", Vector{Member("a", ty.i32())});
    GlobalVar("v", core::AddressSpace::kPrivate, Call(ty(Source{{12, 34}}, "S", true)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: type 'S' does not take template arguments
note: 'struct S' declared here)");
}

TEST_F(ResolverUntemplatedTypeUsedWithTemplateArgs, AliasedArray_Type) {
    // alias A = array<i32, 4u>
    // var<private> v : A<true>;

    Alias("A", ty.array<i32, 4>());
    GlobalVar("v", core::AddressSpace::kPrivate, ty(Source{{12, 34}}, "A", true));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: type 'A' does not take template arguments
note: 'alias A' declared here)");
}

TEST_F(ResolverUntemplatedTypeUsedWithTemplateArgs, AliasedArray_Ctor) {
    // alias A = array<i32, 4u>
    // var<private> v = A<true>();

    Alias("A", ty.array<i32, 4>());
    GlobalVar("v", core::AddressSpace::kPrivate, Call(ty(Source{{12, 34}}, "A", true)));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: type 'A' does not take template arguments
note: 'alias A' declared here)");
}

}  // namespace TypeDoesNotTakeTemplateArgs

}  // namespace
}  // namespace tint::resolver
