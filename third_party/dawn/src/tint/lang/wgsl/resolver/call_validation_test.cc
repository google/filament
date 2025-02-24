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
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverCallValidationTest = ResolverTest;

TEST_F(ResolverCallValidationTest, TooFewArgs) {
    Func("foo",
         Vector{
             Param(Sym(), ty.i32()),
             Param(Sym(), ty.f32()),
         },
         ty.void_(),
         Vector{
             Return(),
         });
    auto* call = Call(Source{{12, 34}}, "foo", 1_i);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: too few arguments in call to 'foo', expected 2, got 1");
}

TEST_F(ResolverCallValidationTest, TooManyArgs) {
    Func("foo",
         Vector{
             Param(Sym(), ty.i32()),
             Param(Sym(), ty.f32()),
         },
         ty.void_(),
         Vector{
             Return(),
         });
    auto* call = Call(Source{{12, 34}}, "foo", 1_i, 1_f, 1_f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: too many arguments in call to 'foo', expected 2, got 3");
}

TEST_F(ResolverCallValidationTest, MismatchedArgs) {
    Func("foo",
         Vector{
             Param(Sym(), ty.i32()),
             Param(Sym(), ty.f32()),
         },
         ty.void_(),
         Vector{
             Return(),
         });
    auto* call = Call("foo", Expr(Source{{12, 34}}, true), 1_f);
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: type mismatch for argument 1 in call to 'foo', "
              "expected 'i32', got 'bool'");
}

TEST_F(ResolverCallValidationTest, UnusedRetval) {
    // fn func() -> f32 { return 1.0; }
    // fn main() {func(); return; }

    Func("func", tint::Empty, ty.f32(),
         Vector{
             Return(Expr(1_f)),
         },
         tint::Empty);

    Func("main", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Source{{12, 34}}, Call("func")),
             Return(),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, PointerArgument_VariableIdentExpr) {
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   var z: i32 = 1i;
    //   foo(&z);
    // }
    auto* param = Param("p", ty.ptr<function, i32>());
    Func("foo", Vector{param}, ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("z", ty.i32(), Expr(1_i))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, Expr("z")))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, PointerArgument_LetIdentExpr) {
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   let z: i32 = 1i;
    //   foo(&z);
    // }
    auto* param = Param("p", ty.ptr<function, i32>());
    Func("foo", Vector{param}, ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("z", ty.i32(), Expr(1_i))),
             CallStmt(Call("foo", AddressOf(Expr(Source{{12, 34}}, "z")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of 'let z'");
}

TEST_F(ResolverCallValidationTest,
       PointerArgument_AddressOfFunctionMember_WithoutUnrestrictedPointerParameters) {
    // struct S { m: i32; };
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   var v : S;
    //   foo(&v.m);
    // }
    auto* S = Structure("S", Vector{
                                 Member("m", ty.i32()),
                             });
    auto* param = Param("p", ty.ptr<function, i32>());
    Func("foo", Vector{param}, ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.Of(S))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, MemberAccessor("v", "m")))),
         });

    Resolver r{this, wgsl::AllowedFeatures{}};

    EXPECT_FALSE(r.Resolve());
    EXPECT_EQ(
        r.error(),
        R"(12:34 error: arguments of pointer type must not point to a subset of the originating variable)");
}

TEST_F(ResolverCallValidationTest,
       PointerArgument_AddressOfFunctionMember_WithUnrestrictedPointerParameters) {
    // struct S { m: i32; };
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   var v : S;
    //   foo(&v.m);
    // }

    auto* S = Structure("S", Vector{
                                 Member("m", ty.i32()),
                             });
    auto* param = Param("p", ty.ptr<function, i32>());
    Func("foo", Vector{param}, ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.Of(S))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, MemberAccessor("v", "m")))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}
TEST_F(ResolverCallValidationTest, PointerArgument_AddressOfLetMember) {
    // struct S { m: i32; };
    // fn foo(p: ptr<function, i32>) {}
    // fn main() {
    //   let v: S = S();
    //   foo(&v.m);
    // }
    auto* S = Structure("S", Vector{
                                 Member("m", ty.i32()),
                             });
    auto* param = Param("p", ty.ptr<function, i32>());
    Func("foo", Vector{param}, ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("v", ty.Of(S), Call(ty.Of(S)))),
             CallStmt(Call("foo", AddressOf(MemberAccessor(Source{{12, 34}}, "v", "m")))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: cannot take the address of value of type 'i32'");
}

TEST_F(ResolverCallValidationTest, PointerArgument_FunctionParam) {
    // fn foo(p: ptr<function, i32>) {}
    // fn bar(p: ptr<function, i32>) {
    // foo(p);
    // }
    Func("foo",
         Vector{
             Param("p", ty.ptr<function, i32>()),
         },
         ty.void_(), tint::Empty);
    Func("bar",
         Vector{
             Param("p", ty.ptr<function, i32>()),
         },
         ty.void_(),
         Vector{
             CallStmt(Call("foo", Expr("p"))),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, PointerArgument_FunctionParamWithMain) {
    // fn foo(p: ptr<function, i32>) {}
    // fn bar(p: ptr<function, i32>) {
    // foo(p);
    // }
    // @fragment
    // fn main() {
    //   var v: i32;
    //   bar(&v);
    // }
    Func("foo",
         Vector{
             Param("p", ty.ptr<function, i32>()),
         },
         ty.void_(), tint::Empty);
    Func("bar",
         Vector{
             Param("p", ty.ptr<function, i32>()),
         },
         ty.void_(),
         Vector{
             CallStmt(Call("foo", "p")),
         });
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.i32(), Expr(1_i))),
             CallStmt(Call("foo", AddressOf("v"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, LetPointer) {
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: i32;
    //   let p: ptr<function, i32> = &v;
    //   x(p);
    // }
    Func("x",
         Vector{
             Param("p", ty.ptr<function, i32>()),
         },
         ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.i32())),
             Decl(Let("p", ty.ptr<function, i32>(), AddressOf("v"))),
             CallStmt(Call("x", "p")),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, LetPointerPrivate) {
    // fn foo(p : ptr<private, i32>) {}
    // var v : i32;
    // @fragment
    // fn main() {
    //   let p : ptr<private, i32> = &v;
    //   foo(p);
    // }
    Func("foo",
         Vector{
             Param("p", ty.ptr<private_, i32>()),
         },
         ty.void_(), tint::Empty);
    GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Let("p", ty.ptr<private_, i32>(), AddressOf("v"))),
             CallStmt(Call("foo", Expr(Source{{12, 34}}, "p"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest, LetPointer_NotWholeVar_WithoutUnrestrictedPointerParameters) {
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p: ptr<function, i32> = &(v[0]);
    //   x(p);
    // }
    Func("foo",
         Vector{
             Param("p", ty.ptr<function, i32>()),
         },
         ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p", ty.ptr<function, i32>(), AddressOf(IndexAccessor("v", 0_a)))),
             CallStmt(Call("foo", Expr(Source{{12, 34}}, "p"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    Resolver r(this, wgsl::AllowedFeatures{});
    EXPECT_FALSE(r.Resolve());
    EXPECT_EQ(r.error(),
              "12:34 error: arguments of pointer type must not point to a subset of the "
              "originating variable");
}

TEST_F(ResolverCallValidationTest, LetPointer_NotWholeVar_WithUnrestrictedPointerParameters) {
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p: ptr<function, i32> = &(v[0]);
    //   x(p);
    // }

    Func("foo",
         Vector{
             Param("p", ty.ptr<function, i32>()),
         },
         ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p", ty.ptr<function, i32>(), AddressOf(IndexAccessor("v", 0_a)))),
             CallStmt(Call("foo", Expr(Source{{12, 34}}, "p"))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverCallValidationTest, ComplexPointerChain) {
    // fn foo(p : ptr<function, array<i32, 4>>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p1 = &v;
    //   let p2 = p1;
    //   let p3 = &*p2;
    //   foo(&*p);
    // }
    Func("foo",
         Vector{
             Param("p", ty.ptr<function, array<i32, 4>>()),
         },
         ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p1", AddressOf("v"))),
             Decl(Let("p2", Expr("p1"))),
             Decl(Let("p3", AddressOf(Deref("p2")))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, Deref("p3")))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverCallValidationTest,
       ComplexPointerChain_NotWholeVar_WithoutUnrestrictedPointerParameters) {
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p1 = &v;
    //   let p2 = p1;
    //   let p3 = &(*p2)[0];
    //   foo(&*p);
    // }
    Func("foo",
         Vector{
             Param("p", ty.ptr<function, i32>()),
         },
         ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p1", AddressOf("v"))),
             Decl(Let("p2", Expr("p1"))),
             Decl(Let("p3", AddressOf(IndexAccessor(Deref("p2"), 0_a)))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, Deref("p3")))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    Resolver r{this, {}};
    EXPECT_FALSE(r.Resolve());
    EXPECT_EQ(r.error(),
              "12:34 error: arguments of pointer type must not point to a subset of the "
              "originating variable");
}

TEST_F(ResolverCallValidationTest,
       ComplexPointerChain_NotWholeVar_WithUnrestrictedPointerParameters) {
    // fn foo(p : ptr<function, i32>) {}
    // @fragment
    // fn main() {
    //   var v: array<i32, 4>;
    //   let p1 = &v;
    //   let p2 = p1;
    //   let p3 = &(*p2)[0];
    //   foo(&*p);
    // }

    Func("foo",
         Vector{
             Param("p", ty.ptr<function, i32>()),
         },
         ty.void_(), tint::Empty);
    Func("main", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", ty.array<i32, 4>())),
             Decl(Let("p1", AddressOf("v"))),
             Decl(Let("p2", Expr("p1"))),
             Decl(Let("p3", AddressOf(IndexAccessor(Deref("p2"), 0_a)))),
             CallStmt(Call("foo", AddressOf(Source{{12, 34}}, Deref("p3")))),
         },
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverCallValidationTest, MustUseFunction) {
    Func(Source{{56, 78}}, "fn_must_use", tint::Empty, ty.i32(), Vector{Return(1_i)},
         Vector{MustUse()});
    Func("f", tint::Empty, ty.void_(), Vector{CallStmt(Call(Source{{12, 34}}, "fn_must_use"))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: ignoring return value of function 'fn_must_use' annotated with '@must_use'
56:78 note: function 'fn_must_use' declared here)");
}

TEST_F(ResolverCallValidationTest, MustUseBuiltin) {
    Func("f", tint::Empty, ty.void_(), Vector{CallStmt(Call(Source{{12, 34}}, "max", 1_a, 2_a))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: ignoring return value of builtin 'max'");
}

TEST_F(ResolverCallValidationTest, UnexpectedFunctionTemplateArgs) {
    // fn a() {}
    // fn b() {
    //   a<i32>();
    // }
    Func(Source{{56, 78}}, "a", tint::Empty, ty.void_(), tint::Empty);
    Func("b", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call(Ident(Source{{12, 34}}, "a", "i32"))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: function 'a' does not take template arguments
56:78 note: function 'a' declared here)");
}

TEST_F(ResolverCallValidationTest, UnexpectedBuiltinTemplateArgs) {
    // fn f() {
    //   min<i32>(1, 2);
    // }
    Func("f", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("v", Call(Source{{12, 34}}, Ident("min", "i32"), 1_a, 2_a))),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: no matching call to 'min<i32>(abstract-int, abstract-int)'

2 candidate functions:
 • 'min(T  ✓ , T  ✓ ) -> T' where:
      ✗  overload expects 0 template arguments, call passed 1 argument
      ✓  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
 • 'min(vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'abstract-float', 'abstract-int', 'f32', 'i32', 'u32' or 'f16'
)");
}

}  // namespace
}  // namespace tint::resolver
