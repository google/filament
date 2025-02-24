// Copyright 2020 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

using HlslASTPrinterTest_Loop = TestHelper;

TEST_F(HlslASTPrinterTest_Loop, Emit_Loop) {
    auto* body = Block(Break());
    auto* continuing = Block();
    auto* l = Loop(body, continuing);

    Func("F", tint::Empty, ty.void_(), Vector{l}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(l)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  while (true) {
    break;
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_LoopWithContinuing) {
    Func("a_statement", {}, ty.void_(), {});

    auto* body = Block(Break());
    auto* continuing = Block(CallStmt(Call("a_statement")));
    auto* l = Loop(body, continuing);

    Func("F", tint::Empty, ty.void_(), Vector{l}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(l)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  while (true) {
    break;
    {
      a_statement();
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_LoopWithContinuing_BreakIf) {
    Func("a_statement", {}, ty.void_(), {});

    auto* body = Block(Break());
    auto* continuing = Block(CallStmt(Call("a_statement")), BreakIf(true));
    auto* l = Loop(body, continuing);

    Func("F", tint::Empty, ty.void_(), Vector{l}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(l)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  while (true) {
    break;
    {
      a_statement();
      if (true) { break; }
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_LoopNestedWithContinuing) {
    Func("a_statement", {}, ty.void_(), {});

    GlobalVar("lhs", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("rhs", ty.f32(), core::AddressSpace::kPrivate);

    auto* body = Block(Break());
    auto* continuing = Block(CallStmt(Call("a_statement")));
    auto* inner = Loop(body, continuing);

    body = Block(inner);

    auto* lhs = Expr("lhs");
    auto* rhs = Expr("rhs");

    continuing = Block(Assign(lhs, rhs), BreakIf(true));

    auto* outer = Loop(body, continuing);

    Func("F", tint::Empty, ty.void_(), Vector{outer}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(outer)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  while (true) {
    while (true) {
      break;
      {
        a_statement();
      }
    }
    {
      lhs = rhs;
      if (true) { break; }
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_LoopWithVarUsedInContinuing) {
    // loop {
    //   var lhs : f32 = 2.5;
    //   var other : f32;
    //   break;
    //   continuing {
    //     lhs = rhs
    //   }
    // }

    GlobalVar("rhs", ty.f32(), core::AddressSpace::kPrivate);

    auto* body = Block(Decl(Var("lhs", ty.f32(), Expr(2.5_f))),  //
                       Decl(Var("other", ty.f32())),             //
                       Break());

    auto* continuing = Block(Assign("lhs", "rhs"));
    auto* outer = Loop(body, continuing);
    WrapInFunction(outer);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(outer)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  while (true) {
    float lhs = 2.5f;
    float other = 0.0f;
    break;
    {
      lhs = rhs;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_ForLoop) {
    // for(; ; ) {
    //   return;
    // }

    auto* f = For(nullptr, nullptr, nullptr, Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  {
    for(; ; ) {
      return;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_ForLoopWithSimpleInit) {
    // for(var i : i32; ; ) {
    //   return;
    // }

    auto* f = For(Decl(Var("i", ty.i32())), nullptr, nullptr, Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  {
    for(int i = 0; ; ) {
      return;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_ForLoopWithMultiStmtInit) {
    // let t = true;
    // for(var b = t && false; ; ) {
    //   return;
    // }

    auto* t = Let("t", Expr(true));
    auto* multi_stmt = LogicalAnd(t, false);
    auto* f = For(Decl(Var("b", multi_stmt)), nullptr, nullptr, Block(Return()));
    WrapInFunction(t, f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  {
    bool tint_tmp = t;
    if (tint_tmp) {
      tint_tmp = false;
    }
    bool b = (tint_tmp);
    for(; ; ) {
      return;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_ForLoopWithSimpleCond) {
    // for(; true; ) {
    //   return;
    // }

    auto* f = For(nullptr, true, nullptr, Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  {
    for(; true; ) {
      return;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_ForLoopWithMultiStmtCond) {
    // let t = true;
    // for(; t && false; ) {
    //   return;
    // }

    Func("a_statement", {}, ty.void_(), {});
    auto* t = Let("t", Expr(true));
    auto* multi_stmt = LogicalAnd(t, false);
    auto* f = For(nullptr, multi_stmt, nullptr, Block(CallStmt(Call("a_statement"))));
    WrapInFunction(t, f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  {
    while (true) {
      bool tint_tmp = t;
      if (tint_tmp) {
        tint_tmp = false;
      }
      if (!((tint_tmp))) { break; }
      a_statement();
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_ForLoopWithSimpleCont) {
    // for(; ; i = i + 1i) {
    //   return;
    // }

    auto* v = Decl(Var("i", ty.i32()));
    auto* f = For(nullptr, nullptr, Assign("i", Add("i", 1_i)), Block(Return()));
    WrapInFunction(v, f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  {
    for(; ; i = (i + 1)) {
      return;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_ForLoopWithMultiStmtCont) {
    // let t = true;
    // for(; ; i = t && false) {
    //   return;
    // }

    auto* t = Let("t", Expr(true));
    auto* multi_stmt = LogicalAnd(t, false);
    auto* v = Decl(Var("i", ty.bool_()));
    auto* f = For(nullptr, nullptr, Assign("i", multi_stmt),  //
                  Block(Return()));
    WrapInFunction(t, v, f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  {
    while (true) {
      return;
      bool tint_tmp = t;
      if (tint_tmp) {
        tint_tmp = false;
      }
      i = (tint_tmp);
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_ForLoopWithSimpleInitCondCont) {
    // for(var i : i32; true; i = i + 1i) {
    //   return;
    // }

    auto* f = For(Decl(Var("i", ty.i32())), true, Assign("i", Add("i", 1_i)), Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  {
    for(int i = 0; true; i = (i + 1)) {
      return;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_ForLoopWithMultiStmtInitCondCont) {
    // let t = true;
    // for(var i = t && false; t && false; i = t && false) {
    //   return;
    // }

    auto* t = Let("t", Expr(true));
    auto* multi_stmt_a = LogicalAnd(t, false);
    auto* multi_stmt_b = LogicalAnd(t, false);
    auto* multi_stmt_c = LogicalAnd(t, false);

    auto* f = For(Decl(Var("i", multi_stmt_a)), multi_stmt_b, Assign("i", multi_stmt_c),  //
                  Block(Return()));
    WrapInFunction(t, f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  {
    bool tint_tmp = t;
    if (tint_tmp) {
      tint_tmp = false;
    }
    bool i = (tint_tmp);
    while (true) {
      bool tint_tmp_1 = t;
      if (tint_tmp_1) {
        tint_tmp_1 = false;
      }
      if (!((tint_tmp_1))) { break; }
      return;
      bool tint_tmp_2 = t;
      if (tint_tmp_2) {
        tint_tmp_2 = false;
      }
      i = (tint_tmp_2);
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_While) {
    // while(true) {
    //   return;
    // }

    auto* f = While(Expr(true), Block(Return()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  while(true) {
    return;
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_While_WithContinue) {
    // while(true) {
    //   continue;
    // }

    auto* f = While(Expr(true), Block(Continue()));
    WrapInFunction(f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  while(true) {
    continue;
  }
)");
}

TEST_F(HlslASTPrinterTest_Loop, Emit_WhileWithMultiStmtCond) {
    // let t = true;
    // while(t && false) {
    //   return;
    // }

    Func("a_statement", {}, ty.void_(), {});

    auto* t = Let("t", Expr(true));
    auto* multi_stmt = LogicalAnd(t, false);
    auto* f = While(multi_stmt, Block(CallStmt(Call("a_statement"))));
    WrapInFunction(t, f);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(f)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  while (true) {
    bool tint_tmp = t;
    if (tint_tmp) {
      tint_tmp = false;
    }
    if (!((tint_tmp))) { break; }
    a_statement();
  }
)");
}

}  // namespace
}  // namespace tint::hlsl::writer
