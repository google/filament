// Copyright 2022 The Dawn & Tint Authors
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

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/ast/transform/helper_test.h"
#include "src/tint/lang/wgsl/ast/transform/hoist_to_decl_before.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/if_statement.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/statement.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::ast::transform {
namespace {

using HoistToDeclBeforeTest = ::testing::Test;

TEST_F(HoistToDeclBeforeTest, VarInit) {
    // fn f() {
    //     var a = 1;
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* var = b.Decl(b.Var("a", expr));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().Get(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kLet);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  let tint_symbol : i32 = 1i;
  var a = tint_symbol;
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, ForLoopInit) {
    // fn f() {
    //     for(var a = 1i; true; ) {
    //     }
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* s = b.For(b.Decl(b.Var("a", expr)), b.Expr(true), nullptr, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().Get(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kVar);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  {
    var tint_symbol : i32 = 1i;
    var a = tint_symbol;
    loop {
      if (!(true)) {
        break;
      }
      {
      }
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, ForLoopCond) {
    // fn f() {
    //     const a = true;
    //     for(; a; ) {
    //     }
    // }
    ProgramBuilder b;
    auto* var = b.Decl(b.Const("a", b.Expr(true)));
    auto* expr = b.Expr("a");
    auto* s = b.For(nullptr, expr, nullptr, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().GetVal(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kConst);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  const a = true;
  loop {
    const tint_symbol = a;
    if (!(tint_symbol)) {
      break;
    }
    {
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, ForLoopCont) {
    // fn f() {
    //     for(; true; var a = 1i) {
    //     }
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* s = b.For(nullptr, b.Expr(true), b.Decl(b.Var("a", expr)), b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().Get(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kLet);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  loop {
    if (!(true)) {
      break;
    }
    {
    }

    continuing {
      let tint_symbol : i32 = 1i;
      var a = tint_symbol;
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, WhileCond) {
    // fn f() {
    //     var a : bool;
    //     while(a) {
    //     }
    // }
    ProgramBuilder b;
    auto* var = b.Decl(b.Var("a", b.ty.bool_()));
    auto* expr = b.Expr("a");
    auto* s = b.While(expr, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().GetVal(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kVar);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  var a : bool;
  loop {
    var tint_symbol : bool = a;
    if (!(tint_symbol)) {
      break;
    }
    {
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, ElseIf) {
    // fn f() {
    //     const a = true;
    //     if (true) {
    //     } else if (a) {
    //     } else {
    //     }
    // }
    ProgramBuilder b;
    auto* var = b.Decl(b.Const("a", b.Expr(true)));
    auto* expr = b.Expr("a");
    auto* s = b.If(b.Expr(true), b.Block(),      //
                   b.Else(b.If(expr, b.Block(),  //
                               b.Else(b.Block()))));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().GetVal(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kConst);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  const a = true;
  if (true) {
  } else {
    const tint_symbol = a;
    if (tint_symbol) {
    } else {
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Array1D) {
    // fn f() {
    //     var a : array<i32, 10>;
    //     var b = a[0];
    // }
    ProgramBuilder b;
    auto* var1 = b.Decl(b.Var("a", b.ty.array<i32, 10>()));
    auto* expr = b.IndexAccessor("a", 0_i);
    auto* var2 = b.Decl(b.Var("b", expr));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var1, var2});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().Get(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kLet);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  var a : array<i32, 10u>;
  let tint_symbol : i32 = a[0i];
  var b = tint_symbol;
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Array2D) {
    // fn f() {
    //     var a : array<array<i32, 10>, 10>;
    //     var b = a[0][0];
    // }
    ProgramBuilder b;

    auto* var1 = b.Decl(b.Var("a", b.ty.array(b.ty.array<i32, 10>(), 10_i)));
    auto* expr = b.IndexAccessor(b.IndexAccessor("a", 0_i), 0_i);
    auto* var2 = b.Decl(b.Var("b", expr));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var1, var2});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().Get(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kVar);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  var a : array<array<i32, 10u>, 10i>;
  var tint_symbol : i32 = a[0i][0i];
  var b = tint_symbol;
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Prepare_ForLoopCond) {
    // fn f() {
    //     var a : bool;
    //     for(; a; ) {
    //     }
    // }
    ProgramBuilder b;
    auto* var = b.Decl(b.Var("a", b.ty.bool_()));
    auto* expr = b.Expr("a");
    auto* s = b.For(nullptr, expr, nullptr, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().GetVal(expr);
    hoistToDeclBefore.Prepare(sem_expr);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  var a : bool;
  loop {
    if (!(a)) {
      break;
    }
    {
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Prepare_ForLoopCont) {
    // fn f() {
    //     for(; true; var a = 1i) {
    //     }
    // }
    ProgramBuilder b;
    auto* expr = b.Expr(1_i);
    auto* s = b.For(nullptr, b.Expr(true), b.Decl(b.Var("a", expr)), b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().Get(expr);
    hoistToDeclBefore.Prepare(sem_expr);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  loop {
    if (!(true)) {
      break;
    }
    {
    }

    continuing {
      var a = 1i;
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Prepare_ElseIf) {
    // fn f() {
    //     var a : bool;
    //     if (true) {
    //     } else if (a) {
    //     } else {
    //     }
    // }
    ProgramBuilder b;
    auto* var = b.Decl(b.Var("a", b.ty.bool_()));
    auto* expr = b.Expr("a");
    auto* s = b.If(b.Expr(true), b.Block(),      //
                   b.Else(b.If(expr, b.Block(),  //
                               b.Else(b.Block()))));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().GetVal(expr);
    hoistToDeclBefore.Prepare(sem_expr);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  var a : bool;
  if (true) {
  } else {
    if (a) {
    } else {
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, InsertBefore_Block) {
    // fn foo() {
    // }
    // fn f() {
    //     var a = 1i;
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* before_stmt = ctx.src->Sem().Get(var);
    auto* new_stmt = ctx.dst->CallStmt(ctx.dst->Call("foo"));
    hoistToDeclBefore.InsertBefore(before_stmt, new_stmt);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  foo();
  var a = 1i;
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, InsertBefore_Block_Function) {
    // fn foo() {
    // }
    // fn f() {
    //     var a = 1i;
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* before_stmt = ctx.src->Sem().Get(var);
    hoistToDeclBefore.InsertBefore(before_stmt,
                                   [&] { return ctx.dst->CallStmt(ctx.dst->Call("foo")); });

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  foo();
  var a = 1i;
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, InsertBefore_ForLoopInit) {
    // fn foo() {
    // }
    // fn f() {
    //     for(var a = 1i; true;) {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    auto* s = b.For(var, b.Expr(true), nullptr, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* before_stmt = ctx.src->Sem().Get(var);
    auto* new_stmt = ctx.dst->CallStmt(ctx.dst->Call("foo"));
    hoistToDeclBefore.InsertBefore(before_stmt, new_stmt);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  {
    foo();
    var a = 1i;
    loop {
      if (!(true)) {
        break;
      }
      {
      }
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, InsertBefore_ForLoopInit_Function) {
    // fn foo() {
    // }
    // fn f() {
    //     for(var a = 1i; true;) {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    auto* s = b.For(var, b.Expr(true), nullptr, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* before_stmt = ctx.src->Sem().Get(var);
    hoistToDeclBefore.InsertBefore(before_stmt,
                                   [&] { return ctx.dst->CallStmt(ctx.dst->Call("foo")); });

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  {
    foo();
    var a = 1i;
    loop {
      if (!(true)) {
        break;
      }
      {
      }
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, InsertBefore_ForLoopCont) {
    // fn foo() {
    // }
    // fn f() {
    //     var a = 1i;
    //     for(; true; a+=1i) {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    auto* cont = b.CompoundAssign("a", b.Expr(1_i), core::BinaryOp::kAdd);
    auto* s = b.For(nullptr, b.Expr(true), cont, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* before_stmt = ctx.src->Sem().Get(cont->As<Statement>());
    auto* new_stmt = ctx.dst->CallStmt(ctx.dst->Call("foo"));
    hoistToDeclBefore.InsertBefore(before_stmt, new_stmt);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  var a = 1i;
  loop {
    if (!(true)) {
      break;
    }
    {
    }

    continuing {
      foo();
      a += 1i;
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, InsertBefore_ForLoopCont_Function) {
    // fn foo() {
    // }
    // fn f() {
    //     var a = 1i;
    //     for(; true; a+=1i) {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    auto* cont = b.CompoundAssign("a", b.Expr(1_i), core::BinaryOp::kAdd);
    auto* s = b.For(nullptr, b.Expr(true), cont, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* before_stmt = ctx.src->Sem().Get(cont->As<Statement>());
    hoistToDeclBefore.InsertBefore(before_stmt,
                                   [&] { return ctx.dst->CallStmt(ctx.dst->Call("foo")); });

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  var a = 1i;
  loop {
    if (!(true)) {
      break;
    }
    {
    }

    continuing {
      foo();
      a += 1i;
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, InsertBefore_ElseIf) {
    // fn foo() {
    // }
    // fn f() {
    //     var a : bool;
    //     if (true) {
    //     } else if (a) {
    //     } else {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.ty.bool_()));
    auto* elseif = b.If(b.Expr("a"), b.Block(), b.Else(b.Block()));
    auto* s = b.If(b.Expr(true), b.Block(),  //
                   b.Else(elseif));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* before_stmt = ctx.src->Sem().Get(elseif);
    auto* new_stmt = ctx.dst->CallStmt(ctx.dst->Call("foo"));
    hoistToDeclBefore.InsertBefore(before_stmt, new_stmt);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  var a : bool;
  if (true) {
  } else {
    foo();
    if (a) {
    } else {
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, InsertBefore_ElseIf_Function) {
    // fn foo() {
    // }
    // fn f() {
    //     var a : bool;
    //     if (true) {
    //     } else if (a) {
    //     } else {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.ty.bool_()));
    auto* elseif = b.If(b.Expr("a"), b.Block(), b.Else(b.Block()));
    auto* s = b.If(b.Expr(true), b.Block(),  //
                   b.Else(elseif));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* before_stmt = ctx.src->Sem().Get(elseif);
    hoistToDeclBefore.InsertBefore(before_stmt,
                                   [&] { return ctx.dst->CallStmt(ctx.dst->Call("foo")); });

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  var a : bool;
  if (true) {
  } else {
    foo();
    if (a) {
    } else {
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, AbstractArray_ToLet) {
    // fn f() {
    //     var a : array<f32, 1> = array(1);
    // }
    ProgramBuilder b;
    auto* expr = b.Call(b.ty("array"), b.Expr(1_a));
    auto* var = b.Decl(b.Var("a", b.ty.array(b.ty.f32(), 1_a), expr));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().Get(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kLet);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  let tint_symbol : array<f32, 1u> = array(1);
  var a : array<f32, 1> = tint_symbol;
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, AbstractArray_ToVar) {
    // fn f() {
    //     var a : array<f32, 1> = array(1);
    // }
    ProgramBuilder b;
    auto* expr = b.Call(b.ty("array"), b.Expr(1_a));
    auto* var = b.Decl(b.Var("a", b.ty.array(b.ty.f32(), 1_a), expr));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* sem_expr = ctx.src->Sem().Get(expr);
    hoistToDeclBefore.Add(sem_expr, expr, HoistToDeclBefore::VariableKind::kVar);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn f() {
  var tint_symbol : array<f32, 1u> = array(1);
  var a : array<f32, 1> = tint_symbol;
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Replace_Block) {
    // fn foo() {
    // }
    // fn f() {
    //     var a = 1i;
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* target_stmt = ctx.src->Sem().Get(var);
    auto* new_stmt = ctx.dst->CallStmt(ctx.dst->Call("foo"));
    hoistToDeclBefore.Replace(target_stmt, new_stmt);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  foo();
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Replace_Block_Function) {
    // fn foo() {
    // }
    // fn f() {
    //     var a = 1i;
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* target_stmt = ctx.src->Sem().Get(var);
    hoistToDeclBefore.Replace(target_stmt, [&] { return ctx.dst->CallStmt(ctx.dst->Call("foo")); });

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  foo();
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Replace_ForLoopInit) {
    // fn foo() {
    // }
    // fn f() {
    //     for(var a = 1i; true;) {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    auto* s = b.For(var, b.Expr(true), nullptr, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* target_stmt = ctx.src->Sem().Get(var);
    auto* new_stmt = ctx.dst->CallStmt(ctx.dst->Call("foo"));
    hoistToDeclBefore.Replace(target_stmt, new_stmt);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  {
    foo();
    loop {
      if (!(true)) {
        break;
      }
      {
      }
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Replace_ForLoopInit_Function) {
    // fn foo() {
    // }
    // fn f() {
    //     for(var a = 1i; true;) {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    auto* s = b.For(var, b.Expr(true), nullptr, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* target_stmt = ctx.src->Sem().Get(var);
    hoistToDeclBefore.Replace(target_stmt, [&] { return ctx.dst->CallStmt(ctx.dst->Call("foo")); });

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  {
    foo();
    loop {
      if (!(true)) {
        break;
      }
      {
      }
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Replace_ForLoopCont) {
    // fn foo() {
    // }
    // fn f() {
    //     var a = 1i;
    //     for(; true; a+=1i) {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    auto* cont = b.CompoundAssign("a", b.Expr(1_i), core::BinaryOp::kAdd);
    auto* s = b.For(nullptr, b.Expr(true), cont, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* target_stmt = ctx.src->Sem().Get(cont->As<Statement>());
    auto* new_stmt = ctx.dst->CallStmt(ctx.dst->Call("foo"));
    hoistToDeclBefore.Replace(target_stmt, new_stmt);

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  var a = 1i;
  loop {
    if (!(true)) {
      break;
    }
    {
    }

    continuing {
      foo();
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

TEST_F(HoistToDeclBeforeTest, Replace_ForLoopCont_Function) {
    // fn foo() {
    // }
    // fn f() {
    //     var a = 1i;
    //     for(; true; a+=1i) {
    //     }
    // }
    ProgramBuilder b;
    b.Func("foo", tint::Empty, b.ty.void_(), tint::Empty);
    auto* var = b.Decl(b.Var("a", b.Expr(1_i)));
    auto* cont = b.CompoundAssign("a", b.Expr(1_i), core::BinaryOp::kAdd);
    auto* s = b.For(nullptr, b.Expr(true), cont, b.Block());
    b.Func("f", tint::Empty, b.ty.void_(), Vector{var, s});

    Program original(resolver::Resolve(b));
    ProgramBuilder cloned_b;
    program::CloneContext ctx(&cloned_b, &original);

    HoistToDeclBefore hoistToDeclBefore(ctx);
    auto* target_stmt = ctx.src->Sem().Get(cont->As<Statement>());
    hoistToDeclBefore.Replace(target_stmt, [&] { return ctx.dst->CallStmt(ctx.dst->Call("foo")); });

    ctx.Clone();
    Program cloned(resolver::Resolve(cloned_b));

    auto* expect = R"(
fn foo() {
}

fn f() {
  var a = 1i;
  loop {
    if (!(true)) {
      break;
    }
    {
    }

    continuing {
      foo();
    }
  }
}
)";

    EXPECT_EQ(expect, str(cloned));
}

}  // namespace
}  // namespace tint::ast::transform
