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

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/lang/wgsl/sem/if_statement.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/lang/wgsl/sem/while_statement.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class ResolverBehaviorTest : public ResolverTest {
  protected:
    void SetUp() override {
        // Create a function called 'Next' which returns an i32, and has the behavior of {Return},
        // which when called, will have the behavior {Next}.
        // It contains a discard statement, which should not affect the behavior analysis or any
        // related validation.
        Func("Next", tint::Empty, ty.i32(),
             Vector{
                 If(true, Block(Discard())),
                 Return(1_i),
             });
    }
};

TEST_F(ResolverBehaviorTest, ExprBinaryOp_LHS) {
    auto* stmt = Decl(Var("lhs", ty.i32(), Add(Call("Next"), 1_i)));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprBinaryOp_RHS) {
    auto* stmt = Decl(Var("lhs", ty.i32(), Add(1_i, Call("Next"))));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprBitcastOp) {
    auto* stmt = Decl(Var("lhs", ty.u32(), Bitcast<u32>(Call("Next"))));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprIndex_Arr) {
    Func("ArrayDiscardOrNext", tint::Empty, ty.array<i32, 4>(),
         Vector{
             If(true, Block(Discard())),
             Return(Call<array<i32, 4>>()),
         });

    auto* stmt = Decl(Var("lhs", ty.i32(), IndexAccessor(Call("ArrayDiscardOrNext"), 1_i)));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprIndex_Idx) {
    auto* stmt = Decl(Var("lhs", ty.i32(), IndexAccessor("arr", Call("Next"))));

    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("arr", ty.array<i32, 4>())),  //
             stmt,
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprUnaryOp) {
    auto* stmt = Decl(Var(
        "lhs", ty.i32(), create<ast::UnaryOpExpression>(core::UnaryOp::kComplement, Call("Next"))));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtAssign) {
    auto* stmt = Assign("lhs", "rhs");
    WrapInFunction(Decl(Var("lhs", ty.i32())),  //
                   Decl(Var("rhs", ty.i32())),  //
                   stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtAssign_LHSDiscardOrNext) {
    auto* stmt = Assign(IndexAccessor("lhs", Call("Next")), 1_i);

    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.array<i32, 4>())),  //
             stmt,
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtAssign_RHSDiscardOrNext) {
    auto* stmt = Assign("lhs", Call("Next"));

    Func("F", tint::Empty, ty.void_(),
         Vector{
             Decl(Var("lhs", ty.i32())),  //
             stmt,
         },
         Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtBlockEmpty) {
    auto* stmt = Block();
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtBlockSingleStmt) {
    auto* stmt = Block(Return());

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kReturn);
}

TEST_F(ResolverBehaviorTest, StmtCallReturn) {
    Func("f", tint::Empty, ty.void_(), Vector{Return()});
    auto* stmt = CallStmt(Call("f"));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtCallFuncDiscard) {
    Func("f", tint::Empty, ty.void_(), Vector{Discard()});
    auto* stmt = CallStmt(Call("f"));

    Func("g", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtCallFuncMayDiscard) {
    auto* stmt = For(Decl(Var("v", ty.i32(), Call("Next"))), nullptr, nullptr, Block(Break()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtBreak) {
    auto* stmt = Break();
    WrapInFunction(Loop(Block(stmt)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kBreak);
}

TEST_F(ResolverBehaviorTest, StmtContinue) {
    auto* stmt = Continue();
    WrapInFunction(Loop(Block(If(true, Block(Break())),  //
                              stmt)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kContinue);
}

TEST_F(ResolverBehaviorTest, StmtDiscard) {
    auto* stmt = Discard();

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtForLoopEmpty_NoExit) {
    auto* stmt = For(Source{{12, 34}}, nullptr, nullptr, nullptr, Block());
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: for-loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtForLoopBreak) {
    auto* stmt = For(nullptr, nullptr, nullptr, Block(Break()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtForLoopContinue_NoExit) {
    auto* stmt = For(Source{{12, 34}}, nullptr, nullptr, nullptr, Block(Continue()));
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: for-loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtForLoopDiscard) {
    auto* stmt = For(nullptr, nullptr, nullptr, Block(Discard(), Break()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtForLoopReturn) {
    auto* stmt = For(nullptr, nullptr, nullptr, Block(Return()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kReturn);
}

TEST_F(ResolverBehaviorTest, StmtForLoopBreak_InitCallFuncMayDiscard) {
    auto* stmt = For(Decl(Var("v", ty.i32(), Call("Next"))), nullptr, nullptr, Block(Break()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtForLoopEmpty_InitCallFuncMayDiscard) {
    auto* stmt = For(Decl(Var("v", ty.i32(), Call("Next"))), nullptr, nullptr, Block(Break()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtForLoopEmpty_CondTrue) {
    auto* stmt = For(nullptr, true, nullptr, Block());
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtForLoopEmpty_CondCallFuncMayDiscard) {
    auto* stmt = For(nullptr, Equal(Call("Next"), 1_i), nullptr, Block());

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtWhileBreak) {
    auto* stmt = While(Expr(true), Block(Break()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtWhileDiscard) {
    auto* stmt = While(Expr(true), Block(Discard()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtWhileReturn) {
    auto* stmt = While(Expr(true), Block(Return()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kReturn, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtWhileEmpty_CondTrue) {
    auto* stmt = While(Expr(true), Block());
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtWhileEmpty_CondCallFuncMayDiscard) {
    auto* stmt = While(Equal(Call("Next"), 1_i), Block());

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenEmptyBlock) {
    auto* stmt = If(true, Block());
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenDiscard) {
    auto* stmt = If(true, Block(Discard()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenEmptyBlock_ElseDiscard) {
    auto* stmt = If(true, Block(), Else(Block(Discard())));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenDiscard_ElseDiscard) {
    auto* stmt = If(true, Block(Discard()), Else(Block(Discard())));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtIfCallFuncMayDiscard_ThenEmptyBlock) {
    auto* stmt = If(Equal(Call("Next"), 1_i), Block());

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenEmptyBlock_ElseCallFuncMayDiscard) {
    auto* stmt = If(true, Block(),  //
                    Else(If(Equal(Call("Next"), 1_i), Block())));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtLetDecl) {
    auto* stmt = Decl(Let("v", ty.i32(), Expr(1_i)));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtLetDecl_RHSDiscardOrNext) {
    auto* stmt = Decl(Let("lhs", ty.i32(), Call("Next")));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtLoopEmpty_NoExit) {
    auto* stmt = Loop(Source{{12, 34}}, Block());
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtLoopBreak) {
    auto* stmt = Loop(Block(Break()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtLoopContinue_NoExit) {
    auto* stmt = Loop(Source{{12, 34}}, Block(Continue()));
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtLoopDiscard) {
    auto* stmt = Loop(Block(Discard(), Break()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtLoopReturn) {
    auto* stmt = Loop(Block(Return()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kReturn);
}

TEST_F(ResolverBehaviorTest, StmtLoopEmpty_ContEmpty_NoExit) {
    auto* stmt = Loop(Source{{12, 34}}, Block(), Block());
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtLoopEmpty_BreakIf) {
    auto* stmt = Loop(Block(), Block(BreakIf(true)));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtReturn) {
    auto* stmt = Return();
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kReturn);
}

TEST_F(ResolverBehaviorTest, StmtReturn_DiscardOrNext) {
    auto* stmt = Return(Call("Next"));

    Func("F", tint::Empty, ty.i32(), Vector{stmt});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kReturn));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondTrue_DefaultEmpty) {
    auto* stmt = Switch(1_i, DefaultCase(Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_DefaultEmpty) {
    auto* stmt = Switch(1_i, DefaultCase(Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_DefaultDiscard) {
    auto* stmt = Switch(1_i, DefaultCase(Block(Discard())));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_DefaultReturn) {
    auto* stmt = Switch(1_i, DefaultCase(Block(Return())));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kReturn);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Empty_DefaultEmpty) {
    auto* stmt = Switch(1_i, Case(CaseSelector(0_i), Block()), DefaultCase(Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Empty_DefaultDiscard) {
    auto* stmt = Switch(1_i, Case(CaseSelector(0_i), Block()), DefaultCase(Block(Discard())));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Empty_DefaultReturn) {
    auto* stmt = Switch(1_i, Case(CaseSelector(0_i), Block()), DefaultCase(Block(Return())));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext, sem::Behavior::kReturn));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Discard_DefaultEmpty) {
    auto* stmt = Switch(1_i, Case(CaseSelector(0_i), Block(Discard())), DefaultCase(Block()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Discard_DefaultDiscard) {
    auto* stmt =
        Switch(1_i, Case(CaseSelector(0_i), Block(Discard())), DefaultCase(Block(Discard())));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Discard_DefaultReturn) {
    auto* stmt =
        Switch(1_i, Case(CaseSelector(0_i), Block(Discard())), DefaultCase(Block(Return())));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kReturn, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Discard_Case1Return_DefaultEmpty) {
    auto* stmt = Switch(1_i,                                        //
                        Case(CaseSelector(0_i), Block(Discard())),  //
                        Case(CaseSelector(1_i), Block(Return())),   //
                        DefaultCase(Block()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext, sem::Behavior::kReturn));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondCallFuncMayDiscard_DefaultEmpty) {
    auto* stmt = Switch(Call("Next"), DefaultCase(Block()));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtVarDecl) {
    auto* stmt = Decl(Var("v", ty.i32()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtVarDecl_RHSDiscardOrNext) {
    auto* stmt = Decl(Var("lhs", ty.i32(), Call("Next")));

    Func("F", tint::Empty, ty.void_(), Vector{stmt}, Vector{Stage(ast::PipelineStage::kFragment)});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

}  // namespace
}  // namespace tint::resolver
