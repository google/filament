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
#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/builtin_texture_helper_test.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::resolver {
namespace {

using ::testing::ElementsAre;
using ::testing::HasSubstr;
using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using ResolverValidationTest = ResolverTest;
using ResolverValidationDeathTest = ResolverValidationTest;

class FakeStmt final : public Castable<FakeStmt, ast::Statement> {
  public:
    FakeStmt(GenerationID pid, ast::NodeID nid, Source src) : Base(pid, nid, src) {}
    FakeStmt* Clone(ast::CloneContext&) const override { return nullptr; }
};

class FakeExpr final : public Castable<FakeExpr, ast::Expression> {
  public:
    FakeExpr(GenerationID pid, ast::NodeID nid, Source src) : Base(pid, nid, src) {}
    FakeExpr* Clone(ast::CloneContext&) const override { return nullptr; }
};

TEST_F(ResolverValidationTest, WorkgroupMemoryUsedInVertexStage) {
    GlobalVar(Source{{1, 2}}, "wg", ty.vec4<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("dst", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    auto* stmt = Assign(Expr("dst"), Expr(Source{{3, 4}}, "wg"));

    Func(Source{{9, 10}}, "f0", tint::Empty, ty.vec4<f32>(),
         Vector{
             stmt,
             Return(Expr("dst")),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(3:4 error: var with 'workgroup' address space cannot be used by vertex pipeline stage
1:2 note: variable is declared here)");
}

TEST_F(ResolverValidationTest, WorkgroupMemoryUsedInFragmentStage) {
    // var<workgroup> wg : vec4<f32>;
    // var<workgroup> dst : vec4<f32>;
    // fn f2(){ dst = wg; }
    // fn f1() { f2(); }
    // @fragment
    // fn f0() {
    //  f1();
    //}

    GlobalVar(Source{{1, 2}}, "wg", ty.vec4<f32>(), core::AddressSpace::kWorkgroup);
    GlobalVar("dst", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    auto* stmt = Assign(Expr("dst"), Expr(Source{{3, 4}}, "wg"));

    Func(Source{{5, 6}}, "f2", tint::Empty, ty.void_(), Vector{stmt});
    Func(Source{{7, 8}}, "f1", tint::Empty, ty.void_(),
         Vector{
             CallStmt(Call("f2")),
         });
    Func(Source{{9, 10}}, "f0", tint::Empty, ty.void_(), Vector{CallStmt(Call("f1"))},
         Vector{
             Stage(ast::PipelineStage::kFragment),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(3:4 error: var with 'workgroup' address space cannot be used by fragment pipeline stage
1:2 note: variable is declared here
5:6 note: called by function 'f2'
7:8 note: called by function 'f1'
9:10 note: called by entry point 'f0')");
}

TEST_F(ResolverValidationTest, RWStorageBufferUsedInVertexStage) {
    // @group(0) @binding(0) var<storage, read_write> v : vec4<f32>;
    // @vertex
    // fn main() -> @builtin(position) vec4f {
    //   return v;
    // }

    GlobalVar(Source{{1, 2}}, "v", ty.vec4<f32>(), core::AddressSpace::kStorage,
              core::Access::kReadWrite, Binding(0_a), Group(0_a));
    Func("main", tint::Empty, ty.vec4<f32>(),
         Vector{
             Return(Expr(Source{{3, 4}}, "v")),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(3:4 error: var with 'storage' address space and 'read_write' access mode cannot be used by vertex pipeline stage
1:2 note: variable is declared here)");
}

TEST_F(ResolverValidationTest, RWStorageTextureUsedInVertexStage) {
    auto type = ty.storage_texture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Uint,
                                   core::Access::kReadWrite);
    GlobalVar(Source{{1, 2}}, "v", type, Binding(0_a), Group(0_a));
    GlobalVar("res", ty.vec4<f32>(), core::AddressSpace::kPrivate);
    Func("main", tint::Empty, ty.vec4<f32>(),
         Vector{
             Assign(Phony(), Expr(Source{{3, 4}}, "v")),
             Return(Expr(Source{{3, 4}}, "res")),
         },
         Vector{
             Stage(ast::PipelineStage::kVertex),
         },
         Vector{
             Builtin(core::BuiltinValue::kPosition),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(3:4 error: storage texture with 'read_write' access mode cannot be used by vertex pipeline stage
1:2 note: variable is declared here)");
}

TEST_F(ResolverValidationDeathTest, UnhandledStmt) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.WrapInFunction(b.create<FakeStmt>());
            resolver::Resolve(b);
        },
        testing::HasSubstr(
            "internal compiler error: Switch() matched no cases. Type: tint::resolver::FakeStmt"));
}

TEST_F(ResolverValidationTest, Stmt_If_NonBool) {
    // if (1.23f) {}

    WrapInFunction(If(Expr(Source{{12, 34}}, 1.23_f), Block()));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: if statement condition must be bool, got f32)");
}

TEST_F(ResolverValidationTest, Stmt_ElseIf_NonBool) {
    // else if (1.23f) {}

    WrapInFunction(If(Expr(true), Block(), Else(If(Expr(Source{{12, 34}}, 1.23_f), Block()))));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(), R"(12:34 error: if statement condition must be bool, got f32)");
}

TEST_F(ResolverValidationDeathTest, Expr_ErrUnknownExprType) {
    EXPECT_DEATH_IF_SUPPORTED(
        {
            ProgramBuilder b;
            b.WrapInFunction(b.create<FakeExpr>());
            Resolver(&b, {}).Resolve();
        },
        testing::HasSubstr(
            "internal compiler error: Switch() matched no cases. Type: tint::resolver::FakeExpr"));
}

TEST_F(ResolverValidationTest, UsingUndefinedVariable_Fail) {
    // b = 2;

    auto* lhs = Expr(Source{{12, 34}}, "b");
    auto* rhs = Expr(2_i);
    auto* assign = Assign(lhs, rhs);
    WrapInFunction(assign);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved value 'b')");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableInBlockStatement_Fail) {
    // {
    //  b = 2;
    // }

    auto* lhs = Expr(Source{{12, 34}}, "b");
    auto* rhs = Expr(2_i);

    auto* body = Block(Assign(lhs, rhs));
    WrapInFunction(body);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved value 'b')");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableGlobalVariable_Pass) {
    // var global_var: f32 = 2.1;
    // fn my_func() {
    //   global_var = 3.14;
    //   return;
    // }

    GlobalVar("global_var", ty.f32(), core::AddressSpace::kPrivate, Expr(2.1_f));

    Func("my_func", tint::Empty, ty.void_(),
         Vector{
             Assign(Expr(Source{{12, 34}}, "global_var"), 3.14_f),
             Return(),
         });

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableInnerScope_Fail) {
    // {
    //   if (true) { var a : f32 = 2.0; }
    //   a = 3.14;
    // }
    auto* var = Var("a", ty.f32(), Expr(2_f));

    auto* cond = Expr(true);
    auto* body = Block(Decl(var));

    SetSource(Source{{12, 34}});
    auto* lhs = Expr(Source{{12, 34}}, "a");
    auto* rhs = Expr(3.14_f);

    auto* outer_body = Block(If(cond, body), Assign(lhs, rhs));

    WrapInFunction(outer_body);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved value 'a')");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableOuterScope_Pass) {
    // {
    //   var a : f32 = 2.0;
    //   if (true) { a = 3.14; }
    // }
    auto* var = Var("a", ty.f32(), Expr(2_f));

    auto* lhs = Expr(Source{{12, 34}}, "a");
    auto* rhs = Expr(3.14_f);

    auto* cond = Expr(true);
    auto* body = Block(Assign(lhs, rhs));

    auto* outer_body = Block(Decl(var), If(cond, body));

    WrapInFunction(outer_body);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableDifferentScope_Fail) {
    // {
    //  { var a : f32 = 2.0; }
    //  { a = 3.14; }
    // }
    auto* var = Var("a", ty.f32(), Expr(2_f));
    auto* first_body = Block(Decl(var));

    auto* lhs = Expr(Source{{12, 34}}, "a");
    auto* rhs = Expr(3.14_f);
    auto* second_body = Block(Assign(lhs, rhs));

    auto* outer_body = Block(first_body, second_body);

    WrapInFunction(outer_body);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: unresolved value 'a')");
}

TEST_F(ResolverValidationTest, AddressSpace_FunctionVariableWorkgroupClass) {
    auto* var = Var("var", ty.i32(), core::AddressSpace::kWorkgroup);

    Func("func", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
         });

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              "error: function-scope 'var' declaration must use 'function' address space");
}

TEST_F(ResolverValidationTest, AddressSpace_FunctionVariableI32) {
    auto* var = Var("s", ty.i32(), core::AddressSpace::kPrivate);

    Func("func", tint::Empty, ty.void_(),
         Vector{
             Decl(var),
         });

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              "error: function-scope 'var' declaration must use 'function' address space");
}

TEST_F(ResolverValidationTest, AddressSpace_SamplerExplicitAddressSpace) {
    auto t = ty.sampler(core::type::SamplerKind::kSampler);
    GlobalVar(Source{{12, 34}}, "var", t, core::AddressSpace::kPrivate, Binding(0_a), Group(0_a));

    EXPECT_FALSE(r()->Resolve());

    EXPECT_EQ(r()->error(),
              R"(12:34 error: variables of type 'sampler' must not specify an address space)");
}

TEST_F(ResolverValidationTest, AddressSpace_TextureExplicitAddressSpace) {
    auto t = ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32());
    GlobalVar(Source{{12, 34}}, "var", t, core::AddressSpace::kFunction, Binding(0_a), Group(0_a));

    EXPECT_FALSE(r()->Resolve()) << r()->error();

    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: variables of type 'texture_1d<f32>' must not specify an address space)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadChar) {
    GlobalVar("my_vec", ty.vec3<f32>(), core::AddressSpace::kPrivate);

    auto* mem = MemberAccessor("my_vec", Ident(Source{{{3, 3}, {3, 7}}}, "xyqz"));
    WrapInFunction(mem);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(3:5 error: invalid vector swizzle character)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_MixedChars) {
    GlobalVar("my_vec", ty.vec4<f32>(), core::AddressSpace::kPrivate);

    auto* mem = MemberAccessor("my_vec", Ident(Source{{{3, 3}, {3, 7}}}, "rgyw"));
    WrapInFunction(mem);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "3:3 error: invalid mixing of vector swizzle characters rgba with xyzw");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadLength) {
    GlobalVar("my_vec", ty.vec3<f32>(), core::AddressSpace::kPrivate);

    auto* mem = MemberAccessor("my_vec", Ident(Source{{{3, 3}, {3, 8}}}, "zzzzz"));
    WrapInFunction(mem);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(3:3 error: invalid vector swizzle size)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadIndex) {
    GlobalVar("my_vec", ty.vec2<f32>(), core::AddressSpace::kPrivate);

    auto* mem = MemberAccessor("my_vec", Ident(Source{{3, 3}}, "z"));
    WrapInFunction(mem);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(3:3 error: invalid vector swizzle member)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_BadParent) {
    // var param: vec4<f32>
    // let ret: f32 = *(&param).x;
    auto* param = Var("param", ty.vec4<f32>());

    auto* addressOf_expr = AddressOf(param);
    auto* accessor_expr = MemberAccessor(addressOf_expr, Ident(Source{{12, 34}}, "x"));
    auto* star_p = Deref(accessor_expr);
    auto* ret = Var("r", ty.f32(), star_p);
    WrapInFunction(Decl(param), Decl(ret));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: cannot dereference expression of type 'f32'");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_FuncGoodParent) {
    // fn func(p: ptr<function, vec4<f32>>) -> f32 {
    //     let x: f32 = (*p).z;
    //     return x;
    // }
    auto* p = Param("p", ty.ptr<function, vec4<f32>>());
    auto* star_p = Deref(p);
    auto* accessor_expr = MemberAccessor(star_p, "z");
    auto* x = Var("x", ty.f32(), accessor_expr);
    Func("func", Vector{p}, ty.f32(),
         Vector{
             Decl(x),
             Return(x),
         });
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_FuncBadParent) {
    // fn func(p: ptr<function, vec4<f32>>) -> f32 {
    //     let x: f32 = *p.z;
    //     return x;
    // }
    auto* p = Param("p", ty.ptr<function, vec4<f32>>());
    auto* accessor_expr = MemberAccessor(p, Ident(Source{{12, 34}}, "z"));
    auto* star_p = Deref(accessor_expr);
    auto* x = Var("x", ty.f32(), star_p);
    Func("func", Vector{p}, ty.f32(),
         Vector{
             Decl(x),
             Return(x),
         });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "error: cannot dereference expression of type 'f32'");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodyBeforeDeclAndAfterDecl_UsageInContinuing) {
    // loop  {
    //     continue; // Bypasses z decl
    //     var z : i32; // unreachable
    //
    //     continuing {
    //         z = 2;
    //     }
    // }

    auto error_loc = Source{{12, 34}};
    auto* body = Block(Continue(), Decl(error_loc, Var("z", ty.i32())));
    auto* continuing = Block(Assign(Expr("z"), 2_i));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 warning: code is unreachable
error: continue statement bypasses declaration of 'z'
note: identifier 'z' declared here
note: identifier 'z' referenced in continuing block here)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInLoopBodyAfterDecl_UsageInContinuing_InBlocks) {
    // loop  {
    //     if (false) { break; }
    //     var z : i32;
    //     {{{continue;}}}
    //     continue; // Ok
    //
    //     continuing {
    //         z = 2i;
    //     }
    // }

    auto* body = Block(If(false, Block(Break())),  //
                       Decl(Var("z", ty.i32())), Block(Block(Block(Continue()))));
    auto* continuing = Block(Assign(Expr("z"), 2_i));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageInContinuing) {
    // loop  {
    //     if (true) {
    //         continue; // Still bypasses z decl (if we reach here)
    //     }
    //     var z : i32;
    //     continuing {
    //         z = 2i;
    //     }
    // }

    auto cont_loc = Source{{12, 34}};
    auto decl_loc = Source{{56, 78}};
    auto ref_loc = Source{{90, 12}};
    auto* body =
        Block(If(Expr(true), Block(Continue(cont_loc))), Decl(Var(decl_loc, "z", ty.i32())));
    auto* continuing = Block(Assign(Expr(ref_loc, "z"), 2_i));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continue statement bypasses declaration of 'z'
56:78 note: identifier 'z' declared here
90:12 note: identifier 'z' referenced in continuing block here)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageInContinuingSubscope) {
    // loop  {
    //     if (true) {
    //         continue; // Still bypasses z decl (if we reach here)
    //     }
    //     var z : i32;
    //     continuing {
    //         if (true) {
    //             z = 2i; // Must fail even if z is in a sub-scope
    //         }
    //     }
    // }

    auto cont_loc = Source{{12, 34}};
    auto decl_loc = Source{{56, 78}};
    auto ref_loc = Source{{90, 12}};
    auto* body =
        Block(If(Expr(true), Block(Continue(cont_loc))), Decl(Var(decl_loc, "z", ty.i32())));

    auto* continuing = Block(If(Expr(true), Block(Assign(Expr(ref_loc, "z"), 2_i))));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continue statement bypasses declaration of 'z'
56:78 note: identifier 'z' declared here
90:12 note: identifier 'z' referenced in continuing block here)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageOutsideBlock) {
    // loop  {
    //     if (true) {
    //         continue; // bypasses z decl (if we reach here)
    //     }
    //     var z : i32;
    //     continuing {
    //         // Must fail even if z is used in an expression that isn't
    //         // directly contained inside a block.
    //         if (z < 2i) {
    //         }
    //     }
    // }

    auto cont_loc = Source{{12, 34}};
    auto decl_loc = Source{{56, 78}};
    auto ref_loc = Source{{90, 12}};
    auto* body =
        Block(If(Expr(true), Block(Continue(cont_loc))), Decl(Var(decl_loc, "z", ty.i32())));
    auto* compare =
        create<ast::BinaryExpression>(core::BinaryOp::kLessThan, Expr(ref_loc, "z"), Expr(2_i));
    auto* continuing = Block(If(compare, Block()));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continue statement bypasses declaration of 'z'
56:78 note: identifier 'z' declared here
90:12 note: identifier 'z' referenced in continuing block here)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageInContinuingLoop) {
    // loop  {
    //     if (true) {
    //         continue; // Still bypasses z decl (if we reach here)
    //     }
    //     var z : i32;
    //     continuing {
    //         loop {
    //             z = 2i; // Must fail even if z is in a sub-scope
    //         }
    //     }
    // }

    auto cont_loc = Source{{12, 34}};
    auto decl_loc = Source{{56, 78}};
    auto ref_loc = Source{{90, 12}};
    auto* body =
        Block(If(Expr(true), Block(Continue(cont_loc))), Decl(Var(decl_loc, "z", ty.i32())));

    auto* continuing = Block(Loop(Block(Assign(Expr(ref_loc, "z"), 2_i))));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continue statement bypasses declaration of 'z'
56:78 note: identifier 'z' declared here
90:12 note: identifier 'z' referenced in continuing block here)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodyBeforeDecl_UsageInNestedContinuingInBody) {
    // loop  {
    //     continue;
    //     var z : i32;
    //     loop {
    //         continue;
    //         continuing {
    //             z = 2i;
    //             break if true;
    //         }
    //     }
    //     continuing {
    //         break if true;
    //     }
    // }

    auto cont_loc = Source{{12, 34}};
    auto decl_loc = Source{{56, 78}};
    auto ref_loc = Source{{90, 12}};
    auto* nested_loop =
        Loop(Block(Continue()), Block(Assign(Expr(ref_loc, "z"), 2_i), BreakIf(true)));
    auto* body = Block(Continue(cont_loc), Decl(Var(decl_loc, "z", ty.i32())), nested_loop);
    auto* continuing = Block(BreakIf(true));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodyBeforeDecl_UsageInNestedContinuingInContinuing) {
    // loop  {
    //     continue;
    //     var z : i32;
    //     continuing {
    //         loop {
    //             continuing {
    //               z = 2i;
    //               break if true;
    //             }
    //         }
    //         break if true;
    //     }
    // }

    auto cont_loc = Source{{12, 34}};
    auto decl_loc = Source{{56, 78}};
    auto ref_loc = Source{{90, 12}};
    auto* body = Block(Continue(cont_loc), Decl(Var(decl_loc, "z", ty.i32())));
    auto* nested_loop = Loop(Block(), Block(Assign(Expr(ref_loc, "z"), 2_i), BreakIf(true)));
    auto* continuing = Block(nested_loop, BreakIf(true));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_FALSE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(),
              R"(warning: code is unreachable
12:34 error: continue statement bypasses declaration of 'z'
56:78 note: identifier 'z' declared here
90:12 note: identifier 'z' referenced in continuing block here)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuing) {
    // loop  {
    //     loop {
    //         if (true) { continue; } // OK: not part of the outer loop
    //         break;
    //     }
    //     var z : i32;
    //     break;
    //     continuing {
    //         z = 2i;
    //     }
    // }

    auto* inner_loop = Loop(Block(    //
        If(true, Block(Continue())),  //
        Break()));
    auto* body = Block(inner_loop,                //
                       Decl(Var("z", ty.i32())),  //
                       Break());
    auto* continuing = Block(Assign("z", 2_i));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuingSubscope) {
    // loop  {
    //     loop {
    //         if (true) { continue; } // OK: not part of the outer loop
    //         break;
    //     }
    //     var z : i32;
    //     break;
    //     continuing {
    //         if (true) {
    //             z = 2i;
    //         }
    //     }
    // }

    auto* inner_loop = Loop(Block(If(true, Block(Continue())),  //
                                  Break()));
    auto* body = Block(inner_loop,                //
                       Decl(Var("z", ty.i32())),  //
                       Break());
    auto* continuing = Block(If(Expr(true), Block(Assign("z", 2_i))));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuingLoop) {
    // loop  {
    //     loop {
    //         if (true) { continue; } // OK: not part of the outer loop
    //         break;
    //     }
    //     var z : i32;
    //     break;
    //     continuing {
    //         loop {
    //             z = 2i;
    //             break;
    //         }
    //     }
    // }

    auto* inner_loop = Loop(Block(If(true, Block(Continue())),  //
                                  Break()));
    auto* body = Block(inner_loop,                //
                       Decl(Var("z", ty.i32())),  //
                       Break());
    auto* continuing = Block(Loop(Block(Assign("z", 2_i),  //
                                        Break())));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_Loop_ContinueInLoopBodyAfterDecl_UsageInContinuing) {
    // loop  {
    //     var z : i32;
    //     if (true) { continue; }
    //     break;
    //     continuing {
    //         z = 2i;
    //     }
    // }

    auto error_loc = Source{{12, 34}};
    auto* body = Block(Decl(Var("z", ty.i32())), If(true, Block(Continue())),  //
                       Break());
    auto* continuing = Block(Assign(Expr(error_loc, "z"), 2_i));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverTest, Stmt_Loop_ReturnInContinuing_Direct) {
    // loop  {
    //   continuing {
    //     return;
    //   }
    // }

    WrapInFunction(Loop(  // loop
        Block(),          //   loop block
        Block(            //   loop continuing block
            Return(Source{{12, 34}}))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continuing blocks must not contain a return statement)");
}

TEST_F(ResolverTest, Stmt_Loop_ReturnInContinuing_Indirect) {
    // loop {
    //   if (false) { break; }
    //   continuing {
    //     loop {
    //       return;
    //     }
    //   }
    // }

    WrapInFunction(Loop(                   // outer loop
        Block(If(false, Block(Break()))),  //   outer loop block
        Block(Source{{56, 78}},            //   outer loop continuing block
              Loop(                        //     inner loop
                  Block(                   //       inner loop block
                      Return(Source{{12, 34}}))))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continuing blocks must not contain a return statement
56:78 note: see continuing block here)");
}

TEST_F(ResolverTest, Stmt_Loop_DiscardInContinuing_Direct) {
    // loop  {
    //   continuing {
    //     discard;
    //     breakif true;
    //   }
    // }

    Func("my_func", tint::Empty, ty.void_(),
         Vector{Loop(  // loop
             Block(),  //   loop block
             Block(    //   loop continuing block
                 Discard(Source{{12, 34}}), BreakIf(true)))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_Loop_ContinueInContinuing_Direct) {
    // loop  {
    //     continuing {
    //         continue;
    //     }
    // }

    WrapInFunction(Loop(         // loop
        Block(),                 //   loop block
        Block(Source{{56, 78}},  //   loop continuing block
              Continue(Source{{12, 34}}))));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continuing blocks must not contain a continue statement)");
}

TEST_F(ResolverTest, Stmt_Loop_ContinueInContinuing_Indirect) {
    // loop {
    //   if (false) { break; }
    //   continuing {
    //     loop {
    //       if (false) { break; }
    //       continue;
    //     }
    //   }
    // }

    WrapInFunction(Loop(                              // outer loop
        Block(                                        //   outer loop block
            If(false, Block(Break()))),               //     if (false) { break; }
        Block(                                        //   outer loop continuing block
            Loop(                                     //     inner loop
                Block(                                //       inner loop block
                    If(false, Block(Break())),        //          if (false) { break; }
                    Continue(Source{{12, 34}}))))));  //    continue

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_Loop_Continuing_BreakIf) {
    // loop  {
    //     continuing {
    //         break if true;
    //     }
    // }

    auto* body = Block();
    auto* continuing = Block(BreakIf(true));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_Loop_Continuing_BreakIf_Not_Last) {
    // loop  {
    //     var z : i32;
    //     continuing {
    //         break if true;
    //         z = 2i;
    //     }
    // }

    auto* body = Block(Decl(Var("z", ty.i32())));
    auto* continuing =
        Block(Source{{10, 9}}, BreakIf(Source{{12, 23}}, true), Assign(Expr("z"), 2_i));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:23 error: break-if must be the last statement in a continuing block
10:9 note: see continuing block here)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_Continuing_BreakIf_Duplicate) {
    // loop  {
    //     continuing {
    //         break if true;
    //         break if false;
    //     }
    // }

    auto* body = Block();
    auto* continuing = Block(Source{{10, 9}}, BreakIf(Source{{12, 23}}, true), BreakIf(false));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:23 error: break-if must be the last statement in a continuing block
10:9 note: see continuing block here)");
}

TEST_F(ResolverValidationTest, Stmt_Loop_Continuing_BreakIf_NonBool) {
    // loop  {
    //     continuing {
    //         break if 1i;
    //     }
    // }

    auto* body = Block();
    auto* continuing = Block(BreakIf(Expr(Source{{12, 23}}, 1_i)));
    auto* loop_stmt = Loop(body, continuing);
    WrapInFunction(loop_stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:23 error: break-if statement condition must be bool, got i32)");
}

TEST_F(ResolverTest, Stmt_ForLoop_ReturnInContinuing_Direct) {
    // for(;; return) {
    //   break;
    // }

    WrapInFunction(For(nullptr, nullptr, Return(Source{{12, 34}}),  //
                       Block(Break())));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continuing blocks must not contain a return statement)");
}

TEST_F(ResolverTest, Stmt_ForLoop_ReturnInContinuing_Indirect) {
    // for(;; loop { return }) {
    //   break;
    // }

    WrapInFunction(For(nullptr, nullptr,
                       Loop(Source{{56, 78}},                  //
                            Block(Return(Source{{12, 34}}))),  //
                       Block(Break())));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continuing blocks must not contain a return statement
56:78 note: see continuing block here)");
}

TEST_F(ResolverTest, Stmt_ForLoop_DiscardInContinuing_Direct) {
    // for(;; discard) {
    //   break;
    // }

    Func("my_func", tint::Empty, ty.void_(),
         Vector{For(nullptr, nullptr, Discard(Source{{12, 34}}),  //
                    Block(Break()))});

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_ForLoop_ContinueInContinuing_Direct) {
    // for(;; continue) {
    //   break;
    // }

    WrapInFunction(For(nullptr, nullptr, Continue(Source{{12, 34}}),  //
                       Block(Break())));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: continuing blocks must not contain a continue statement)");
}

TEST_F(ResolverTest, Stmt_ForLoop_ContinueInContinuing_Indirect) {
    // for(;; loop { if (false) { break; } continue }) {
    //   break;
    // }

    WrapInFunction(For(nullptr, nullptr,
                       Loop(                                    //
                           Block(If(false, Block(Break())),     //
                                 Continue(Source{{12, 34}}))),  //
                       Block(Break())));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_ForLoop_CondIsBoolRef) {
    // var cond : bool = true;
    // for (; cond; ) {
    // }

    auto* cond = Var("cond", ty.bool_(), Expr(true));
    WrapInFunction(Decl(cond), For(nullptr, "cond", nullptr, Block()));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_ForLoop_CondIsNotBool) {
    // for (; 1.0f; ) {
    // }

    WrapInFunction(For(nullptr, Expr(Source{{12, 34}}, 1_f), nullptr, Block()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: for-loop condition must be bool, got f32)");
}

TEST_F(ResolverTest, Stmt_While_CondIsBoolRef) {
    // var cond : bool = false;
    // while (cond) {
    // }

    auto* cond = Var("cond", ty.bool_(), Expr(false));
    WrapInFunction(Decl(cond), While("cond", Block()));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_While_CondIsNotBool) {
    // while (1.0f) {
    // }

    WrapInFunction(While(Expr(Source{{12, 34}}, 1_f), Block()));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: while condition must be bool, got f32)");
}

TEST_F(ResolverValidationTest, Stmt_ContinueInLoop) {
    WrapInFunction(Loop(Block(If(false, Block(Break())),  //
                              Continue(Source{{12, 34}}))));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_ContinueNotInLoop) {
    WrapInFunction(Continue(Source{{12, 34}}));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: continue statement must be in a loop)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInLoop) {
    WrapInFunction(Loop(Block(Break(Source{{12, 34}}))));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakInSwitch) {
    WrapInFunction(Loop(Block(Switch(Expr(1_i),               //
                                     Case(CaseSelector(1_i),  //
                                          Block(Break())),    //
                                     DefaultCase()),          //
                              Break())));                     //
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakInSwitchInContinuing) {
    // loop {
    //   continuing {
    //     switch(1) {
    //       default:
    //         break;
    //     }
    //   }
    // }

    auto* cont = Block(Switch(1_i, DefaultCase(Block(Break()))));

    WrapInFunction(Loop(Block(Break()), cont));
    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfTrueInContinuing) {
    auto* cont = Block(                           // continuing {
        If(true, Block(                           //   if(true) {
                     Break(Source{{12, 34}}))));  //     break;
                                                  //   }
                                                  // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseInContinuing) {
    auto* cont = Block(                      // continuing {
        If(true, Block(),                    //   if(true) {
           Else(Block(                       //   } else {
               Break(Source{{12, 34}})))));  //     break;
                                             //   }
                                             // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInContinuing) {
    auto* cont = Block(                   // continuing {
        Block(Break(Source{{12, 34}})));  //   break;
                                          // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfInIfInContinuing) {
    auto* cont = Block(                                      // continuing {
        If(true, Block(                                      //   if(true) {
                     If(Source{{56, 78}}, true,              //     if(true) {
                        Block(Break(Source{{12, 34}}))))));  //       break;
                                                             //     }
                                                             //   }
                                                             // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfTrueMultipleStmtsInContinuing) {
    auto* cont = Block(                             // continuing {
        If(true, Block(Source{{56, 78}},            //   if(true) {
                       Assign(Phony(), 1_i),        //     _ = 1i;
                       Break(Source{{12, 34}}))));  //     break;
                                                    //   }
                                                    // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseMultipleStmtsInContinuing) {
    auto* cont = Block(                             // continuing {
        If(true, Block(),                           //   if(true) {
           Else(Block(Source{{56, 78}},             //   } else {
                      Assign(Phony(), 1_i),         //     _ = 1i;
                      Break(Source{{12, 34}})))));  //     break;
                                                    //   }
                                                    // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseIfInContinuing) {
    auto* cont = Block(                                 // continuing {
        If(true, Block(),                               //   if(true) {
           Else(If(Source{{56, 78}}, Expr(true),        //   } else if (true) {
                   Block(Break(Source{{12, 34}}))))));  //     break;
                                                        //   }
                                                        // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfNonEmptyElseInContinuing) {
    auto* cont = Block(                          // continuing {
        If(true,                                 //   if(true) {
           Block(Break(Source{{12, 34}})),       //     break;
           Else(Block(Source{{56, 78}},          //   } else {
                      Assign(Phony(), 1_i)))));  //     _ = 1i;
                                                 //   }
                                                 // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseNonEmptyTrueInContinuing) {
    auto* cont = Block(                                    // continuing {
        If(true,                                           //   if(true) {
           Block(Source{{56, 78}}, Assign(Phony(), 1_i)),  //     _ = 1i;
           Else(Block(                                     //   } else {
               Break(Source{{12, 34}})))));                //     break;
                                                           //   }
                                                           // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfInContinuingNotLast) {
    auto* cont = Block(                      // continuing {
        If(Source{{56, 78}}, true,           //   if(true) {
           Block(Break(Source{{12, 34}}))),  //     break;
                                             //   }
        Assign(Phony(), 1_i));               //   _ = 1i;
                                             // }
    WrapInFunction(Loop(Block(), cont));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(12:34 error: 'break' must not be used to exit from a continuing block. Use 'break if' instead.)");
}

TEST_F(ResolverValidationTest, Stmt_BreakNotInLoopOrSwitch) {
    WrapInFunction(Break(Source{{12, 34}}));
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: break statement must be in a loop or switch case)");
}

TEST_F(ResolverValidationTest, StructMemberDuplicateName) {
    Structure("S", Vector{
                       Member(Source{{12, 34}}, "a", ty.i32()),
                       Member(Source{{56, 78}}, "a", ty.i32()),
                   });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: redefinition of 'a'\n12:34 note: previous definition "
              "is here");
}
TEST_F(ResolverValidationTest, StructMemberDuplicateNameDifferentTypes) {
    Structure("S", Vector{
                       Member(Source{{12, 34}}, "a", ty.bool_()),
                       Member(Source{{12, 34}}, "a", ty.vec3<f32>()),
                   });
    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: redefinition of 'a'\n12:34 note: previous definition "
              "is here");
}
TEST_F(ResolverValidationTest, StructMemberDuplicateNamePass) {
    Structure("S", Vector{
                       Member("a", ty.i32()),
                       Member("b", ty.f32()),
                   });
    Structure("S1", Vector{
                        Member("a", ty.i32()),
                        Member("b", ty.f32()),
                    });
    EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverValidationTest, NegativeStructMemberAlignAttribute) {
    Structure("S", Vector{
                       Member("a", ty.f32(), Vector{MemberAlign(Source{{12, 34}}, -2_i)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: '@align' value must be a positive, power-of-two integer)");
}

TEST_F(ResolverValidationTest, NonPOTStructMemberAlignAttribute) {
    Structure("S", Vector{
                       Member("a", ty.f32(), Vector{MemberAlign(Source{{12, 34}}, 3_i)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: '@align' value must be a positive, power-of-two integer)");
}

TEST_F(ResolverValidationTest, ZeroStructMemberAlignAttribute) {
    Structure("S", Vector{
                       Member("a", ty.f32(), Vector{MemberAlign(Source{{12, 34}}, 0_i)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: '@align' value must be a positive, power-of-two integer)");
}

TEST_F(ResolverValidationTest, ZeroStructMemberSizeAttribute) {
    Structure("S", Vector{
                       Member("a", ty.f32(), Vector{MemberSize(Source{{12, 34}}, 1_a)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(12:34 error: '@size' must be at least as big as the type's size (4))");
}

TEST_F(ResolverValidationTest, OffsetAndSizeAttribute) {
    Structure("S", Vector{
                       Member(Source{{12, 34}}, "a", ty.f32(),
                              Vector{MemberOffset(0_a), MemberSize(4_a)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@offset' cannot be used with '@align' or '@size')");
}

TEST_F(ResolverValidationTest, OffsetAndAlignAttribute) {
    Structure("S", Vector{
                       Member(Source{{12, 34}}, "a", ty.f32(),
                              Vector{MemberOffset(0_a), MemberAlign(4_i)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@offset' cannot be used with '@align' or '@size')");
}

TEST_F(ResolverValidationTest, OffsetAndAlignAndSizeAttribute) {
    Structure("S", Vector{
                       Member(Source{{12, 34}}, "a", ty.f32(),
                              Vector{MemberOffset(0_a), MemberAlign(4_i), MemberSize(4_a)}),
                   });

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: '@offset' cannot be used with '@align' or '@size')");
}

TEST_F(ResolverTest, Expr_Initializer_Cast_Pointer) {
    auto* vf = Var("vf", ty.f32());
    auto* c = Call(Source{{12, 34}}, ty.ptr<function, i32>(), ExprList(vf));
    auto* ip = Let("ip", ty.ptr<function, i32>(), c);
    WrapInFunction(Decl(vf), Decl(ip));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: type is not constructible)");
}

TEST_F(ResolverTest, I32_Overflow) {
    GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate, Expr(Source{{12, 24}}, 2147483648_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:24 error: value 2147483648 cannot be represented as 'i32')");
}

TEST_F(ResolverTest, I32_Underflow) {
    GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate, Expr(Source{{12, 24}}, -2147483649_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:24 error: value -2147483649 cannot be represented as 'i32')");
}

TEST_F(ResolverTest, U32_Overflow) {
    GlobalVar("v", ty.u32(), core::AddressSpace::kPrivate, Expr(Source{{12, 24}}, 4294967296_a));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:24 error: value 4294967296 cannot be represented as 'u32')");
}

TEST_F(ResolverValidationTest, ShiftLeft_I32_PartialEval_Valid) {
    auto* lhs = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* let = Let("res", Shl(lhs, Expr(31_u)));
    WrapInFunction(Decl(let));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, ShiftLeft_I32_PartialEval_Invalid) {
    auto* lhs = GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* let = Let("res", Shl(Source{{1, 2}}, lhs, Expr(32_u)));
    WrapInFunction(Decl(let));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: shift left value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ResolverValidationTest, ShiftRight_U32_PartialEval_Invalid) {
    auto* lhs = GlobalVar("v", ty.u32(), core::AddressSpace::kPrivate);
    auto* let = Let("res", Shr(Source{{1, 2}}, lhs, Expr(64_u)));
    WrapInFunction(Decl(let));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: shift right value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ResolverValidationTest, ShiftLeft_VecI32_PartialEval_Valid) {
    auto* lhs = GlobalVar("v", ty.vec3(ty.i32()), core::AddressSpace::kPrivate);
    auto* let = Let("res", Shl(lhs, Call<vec3<u32>>(0_u, 1_u, 2_u)));
    WrapInFunction(Decl(let));

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, ShiftLeft_VecI32_PartialEval_Invalid) {
    auto* lhs = GlobalVar("v", ty.vec3(ty.i32()), core::AddressSpace::kPrivate);
    auto* let = Let("res", Shl(Source{{1, 2}}, lhs, Call<vec3<u32>>(31_u, 32_u, 33_u)));
    WrapInFunction(Decl(let));

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: shift left value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ResolverValidationTest, ShiftLeft_I32_CompoundAssign_Valid) {
    GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* expr = CompoundAssign(Source{{1, 2}}, "v", 1_u, core::BinaryOp::kShiftLeft);
    WrapInFunction(expr);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, ShiftLeft_I32_CompoundAssign_Invalid) {
    GlobalVar("v", ty.i32(), core::AddressSpace::kPrivate);
    auto* expr = CompoundAssign(Source{{1, 2}}, "v", 64_u, core::BinaryOp::kShiftLeft);
    WrapInFunction(expr);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(1:2 error: shift left value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ResolverValidationTest, WorkgroupUniformLoad_ArraySize_NamedOverride) {
    // override size = 10u;
    // var<workgroup> a : array<u32, size>;
    auto* override = Override("size", Expr(10_u));
    auto* a = GlobalVar(Source{{7, 3}}, "a", ty.array(ty.u32(), override),
                        core::AddressSpace::kWorkgroup);

    auto* call = Call("workgroupUniformLoad", AddressOf(a));
    WrapInFunction(call);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: no matching call to 'workgroupUniformLoad(ptr<workgroup, array<u32, size>, read_write>)'

2 candidate functions:
 • 'workgroupUniformLoad(ptr<workgroup, T, read_write>  ✗ ) -> T' where:
      ✗  'T' is 'any concrete constructible type'
 • 'workgroupUniformLoad(ptr<workgroup, atomic<T>, read_write>  ✗ ) -> T' where:
      ✗  'T' is 'i32' or 'u32'
)");
}

TEST_F(ResolverValidationTest, WorkgroupUniformLoad_ArraySize_NamedConstant) {
    // const size = 10u;
    // var<workgroup> a : array<u32, size>;
    auto* const_exp = GlobalConst("size", Expr(10_u));
    auto* a = GlobalVar(Source{{7, 3}}, "a", ty.array(ty.u32(), const_exp),
                        core::AddressSpace::kWorkgroup);

    auto* call = Call("workgroupUniformLoad", AddressOf(a));
    WrapInFunction(call);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver

TINT_INSTANTIATE_TYPEINFO(tint::resolver::FakeStmt);
TINT_INSTANTIATE_TYPEINFO(tint::resolver::FakeExpr);
