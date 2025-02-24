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
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/lang/wgsl/sem/if_statement.h"
#include "src/tint/lang/wgsl/sem/loop_statement.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/while_statement.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

using ResolverCompoundStatementTest = ResolverTest;

TEST_F(ResolverCompoundStatementTest, FunctionBlock) {
    // fn F() {
    //   var x : 32;
    // }
    auto* stmt = Decl(Var("x", ty.i32()));
    auto* f = Func("F", tint::Empty, ty.void_(), Vector{stmt});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* s = Sem().Get(stmt);
    ASSERT_NE(s, nullptr);
    ASSERT_NE(s->Block(), nullptr);
    ASSERT_TRUE(s->Block()->Is<sem::FunctionBlockStatement>());
    EXPECT_EQ(s->Block(), s->FindFirstParent<sem::BlockStatement>());
    EXPECT_EQ(s->Block(), s->FindFirstParent<sem::FunctionBlockStatement>());
    EXPECT_EQ(s->Function()->Declaration(), f);
    EXPECT_EQ(s->Block()->Parent(), nullptr);
}

TEST_F(ResolverCompoundStatementTest, Block) {
    // fn F() {
    //   {
    //     var x : 32;
    //   }
    // }
    auto* stmt = Decl(Var("x", ty.i32()));
    auto* block = Block(stmt);
    auto* f = Func("F", tint::Empty, ty.void_(), Vector{block});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* s = Sem().Get(block);
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(s->Is<sem::BlockStatement>());
        EXPECT_EQ(s, s->Block());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
    }
    {
        auto* s = Sem().Get(stmt);
        ASSERT_NE(s, nullptr);
        ASSERT_NE(s->Block(), nullptr);
        EXPECT_EQ(s->Block(), s->FindFirstParent<sem::BlockStatement>());
        EXPECT_EQ(s->Block()->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        ASSERT_TRUE(s->Block()->Parent()->Is<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Function()->Declaration(), f);
        EXPECT_EQ(s->Block()->Parent()->Parent(), nullptr);
    }
}

TEST_F(ResolverCompoundStatementTest, Loop) {
    // fn F() {
    //   loop {
    //     break;
    //     continuing {
    //       stmt;
    //     }
    //   }
    // }
    auto* brk = Break();
    auto* stmt = Ignore(1_i);
    auto* loop = Loop(Block(brk), Block(stmt));
    auto* f = Func("F", tint::Empty, ty.void_(), Vector{loop});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* s = Sem().Get(loop);
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(s->Is<sem::LoopStatement>());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
    }
    {
        auto* s = Sem().Get(brk);
        ASSERT_NE(s, nullptr);
        ASSERT_NE(s->Block(), nullptr);
        EXPECT_EQ(s->Parent(), s->Block());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::LoopBlockStatement>());

        EXPECT_EQ(s->Parent()->Parent(), s->FindFirstParent<sem::LoopStatement>());
        EXPECT_TRUE(Is<sem::LoopStatement>(s->Parent()->Parent()));

        EXPECT_EQ(s->Parent()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_TRUE(Is<sem::FunctionBlockStatement>(s->Parent()->Parent()->Parent()));

        EXPECT_EQ(s->Function()->Declaration(), f);

        EXPECT_EQ(s->Parent()->Parent()->Parent()->Parent(), nullptr);
    }
    {
        auto* s = Sem().Get(stmt);
        ASSERT_NE(s, nullptr);
        ASSERT_NE(s->Block(), nullptr);
        EXPECT_EQ(s->Parent(), s->Block());

        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::LoopContinuingBlockStatement>());
        EXPECT_TRUE(Is<sem::LoopContinuingBlockStatement>(s->Parent()));

        EXPECT_EQ(s->Parent()->Parent(), s->FindFirstParent<sem::LoopBlockStatement>());
        EXPECT_TRUE(Is<sem::LoopBlockStatement>(s->Parent()->Parent()));

        EXPECT_EQ(s->Parent()->Parent()->Parent(), s->FindFirstParent<sem::LoopStatement>());
        EXPECT_TRUE(Is<sem::LoopStatement>(s->Parent()->Parent()->Parent()));

        EXPECT_EQ(s->Parent()->Parent()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_TRUE(Is<sem::FunctionBlockStatement>(s->Parent()->Parent()->Parent()->Parent()));
        EXPECT_EQ(s->Function()->Declaration(), f);

        EXPECT_EQ(s->Parent()->Parent()->Parent()->Parent()->Parent(), nullptr);
    }
}

TEST_F(ResolverCompoundStatementTest, Loop_EmptyContinuing) {
    // fn F() {
    //   loop {
    //     break;
    //     continuing {
    //     }
    //   }
    // }
    auto* brk = Break();
    auto* loop = Loop(Block(brk), Block());
    Func("F", tint::Empty, ty.void_(), Vector{loop});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* s = Sem().Get(loop);
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(s->Is<sem::LoopStatement>());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
    }
    {
        auto* s = Sem().Get(loop->continuing);
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(Is<sem::LoopContinuingBlockStatement>(s));
        EXPECT_TRUE(Is<sem::LoopStatement>(s->Parent()->Parent()));
    }
}

TEST_F(ResolverCompoundStatementTest, ForLoop) {
    // fn F() {
    //   for (var i : u32; true; i = i + 1u) {
    //     return;
    //   }
    // }
    auto* init = Decl(Var("i", ty.u32()));
    auto* cond = Expr(true);
    auto* cont = Assign("i", Add("i", 1_u));
    auto* stmt = Return();
    auto* body = Block(stmt);
    auto* for_ = For(init, cond, cont, body);
    auto* f = Func("F", tint::Empty, ty.void_(), Vector{for_});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* s = Sem().Get(for_);
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(s->Is<sem::ForLoopStatement>());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
    }
    {
        auto* s = Sem().Get(init);
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::ForLoopStatement>());
        EXPECT_TRUE(Is<sem::ForLoopStatement>(s->Parent()));
        EXPECT_EQ(s->Block(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_TRUE(Is<sem::FunctionBlockStatement>(s->Parent()->Parent()));
    }
    {  // Condition expression's statement is the for-loop itself
        auto* e = Sem().Get(cond);
        ASSERT_NE(e, nullptr);
        auto* s = e->Stmt();
        ASSERT_NE(s, nullptr);
        ASSERT_TRUE(Is<sem::ForLoopStatement>(s));
        ASSERT_NE(s->Parent(), nullptr);
        EXPECT_EQ(s->Parent(), s->Block());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_TRUE(Is<sem::FunctionBlockStatement>(s->Block()));
    }
    {
        auto* s = Sem().Get(cont);
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::ForLoopStatement>());
        EXPECT_TRUE(Is<sem::ForLoopStatement>(s->Parent()));
        EXPECT_EQ(s->Block(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_TRUE(Is<sem::FunctionBlockStatement>(s->Parent()->Parent()));
    }
    {
        auto* s = Sem().Get(stmt);
        ASSERT_NE(s, nullptr);
        ASSERT_NE(s->Block(), nullptr);
        EXPECT_EQ(s->Parent(), s->Block());
        EXPECT_EQ(s->Block(), s->FindFirstParent<sem::LoopBlockStatement>());
        EXPECT_TRUE(Is<sem::ForLoopStatement>(s->Parent()->Parent()));
        EXPECT_EQ(s->Block()->Parent(), s->FindFirstParent<sem::ForLoopStatement>());
        ASSERT_TRUE(Is<sem::FunctionBlockStatement>(s->Block()->Parent()->Parent()));
        EXPECT_EQ(s->Block()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Function()->Declaration(), f);
        EXPECT_EQ(s->Block()->Parent()->Parent()->Parent(), nullptr);
    }
}

TEST_F(ResolverCompoundStatementTest, While) {
    // fn F() {
    //   while (true) {
    //     return;
    //   }
    // }
    auto* cond = Expr(true);
    auto* stmt = Return();
    auto* body = Block(stmt);
    auto* while_ = While(cond, body);
    auto* f = Func("W", tint::Empty, ty.void_(), Vector{while_});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* s = Sem().Get(while_);
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(Sem().Get(body)->Parent(), s);
        EXPECT_TRUE(s->Is<sem::WhileStatement>());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
    }
    {  // Condition expression's statement is the while itself
        auto* e = Sem().Get(cond);
        ASSERT_NE(e, nullptr);
        auto* s = e->Stmt();
        ASSERT_NE(s, nullptr);
        ASSERT_TRUE(Is<sem::WhileStatement>(s));
        ASSERT_NE(s->Parent(), nullptr);
        EXPECT_EQ(s->Parent(), s->Block());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_TRUE(Is<sem::FunctionBlockStatement>(s->Block()));
    }
    {
        auto* s = Sem().Get(stmt);
        ASSERT_NE(s, nullptr);
        ASSERT_NE(s->Block(), nullptr);
        EXPECT_EQ(s->Parent(), s->Block());
        EXPECT_EQ(s->Block(), s->FindFirstParent<sem::LoopBlockStatement>());
        EXPECT_TRUE(Is<sem::WhileStatement>(s->Parent()->Parent()));
        EXPECT_EQ(s->Block()->Parent(), s->FindFirstParent<sem::WhileStatement>());
        ASSERT_TRUE(Is<sem::FunctionBlockStatement>(s->Block()->Parent()->Parent()));
        EXPECT_EQ(s->Block()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Function()->Declaration(), f);
        EXPECT_EQ(s->Block()->Parent()->Parent()->Parent(), nullptr);
    }
}

TEST_F(ResolverCompoundStatementTest, If) {
    // fn F() {
    //   if (cond_a) {
    //     stat_a;
    //   } else if (cond_b) {
    //     stat_b;
    //   } else {
    //     stat_c;
    //   }
    // }

    auto* cond_a = Expr(true);
    auto* stmt_a = Ignore(1_i);
    auto* cond_b = Expr(true);
    auto* stmt_b = Ignore(1_i);
    auto* stmt_c = Ignore(1_i);
    auto* if_stmt = If(cond_a, Block(stmt_a), Else(If(cond_b, Block(stmt_b), Else(Block(stmt_c)))));
    WrapInFunction(if_stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* s = Sem().Get(if_stmt);
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(s->Is<sem::IfStatement>());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
    }
    {
        auto* e = Sem().Get(cond_a);
        ASSERT_NE(e, nullptr);
        auto* s = e->Stmt();
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(s->Is<sem::IfStatement>());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
    }
    {
        auto* s = Sem().Get(stmt_a);
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::BlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
        EXPECT_EQ(s->Parent()->Parent(), s->FindFirstParent<sem::IfStatement>());
        EXPECT_EQ(s->Parent()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
    }
    {
        auto* e = Sem().Get(cond_b);
        ASSERT_NE(e, nullptr);
        auto* s = e->Stmt();
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(s->Is<sem::IfStatement>());
        EXPECT_EQ(s->Parent(), s->Parent()->FindFirstParent<sem::IfStatement>());
        EXPECT_EQ(s->Parent()->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Parent()->Parent(), s->Block());
    }
    {
        auto* s = Sem().Get(stmt_b);
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::BlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
        auto* elseif = s->FindFirstParent<sem::IfStatement>();
        EXPECT_EQ(s->Parent()->Parent(), elseif);
        EXPECT_EQ(s->Parent()->Parent()->Parent(),
                  elseif->Parent()->FindFirstParent<sem::IfStatement>());
        EXPECT_EQ(s->Parent()->Parent()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
    }
    {
        auto* s = Sem().Get(stmt_c);
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::BlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
        auto* elseif = s->FindFirstParent<sem::IfStatement>();
        EXPECT_EQ(s->Parent()->Parent(), elseif);
        EXPECT_EQ(s->Parent()->Parent()->Parent(),
                  elseif->Parent()->FindFirstParent<sem::IfStatement>());
        EXPECT_EQ(s->Parent()->Parent()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
    }
}

TEST_F(ResolverCompoundStatementTest, Switch) {
    // fn F() {
    //   switch (expr) {
    //     case 1i: {
    //        stmt_a;
    //     }
    //     case 2i: {
    //        stmt_b;
    //     }
    //     default: {
    //        stmt_c;
    //     }
    //   }
    // }

    auto* expr = Expr(5_i);
    auto* stmt_a = Ignore(1_i);
    auto* stmt_b = Ignore(1_i);
    auto* stmt_c = Ignore(1_i);
    auto* swi = Switch(expr, Case(CaseSelector(1_i), Block(stmt_a)),
                       Case(CaseSelector(2_i), Block(stmt_b)), DefaultCase(Block(stmt_c)));
    WrapInFunction(swi);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    {
        auto* s = Sem().Get(swi);
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(s->Is<sem::SwitchStatement>());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
    }
    {
        auto* e = Sem().Get(expr);
        ASSERT_NE(e, nullptr);
        auto* s = e->Stmt();
        ASSERT_NE(s, nullptr);
        EXPECT_TRUE(s->Is<sem::SwitchStatement>());
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::FunctionBlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
    }
    {
        auto* s = Sem().Get(stmt_a);
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::BlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
        EXPECT_EQ(s->Parent()->Parent(), s->FindFirstParent<sem::CaseStatement>());
        EXPECT_EQ(s->Parent()->Parent()->Parent(), s->FindFirstParent<sem::SwitchStatement>());
        EXPECT_EQ(s->Parent()->Parent()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
    }
    {
        auto* s = Sem().Get(stmt_b);
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::BlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
        EXPECT_EQ(s->Parent()->Parent(), s->FindFirstParent<sem::CaseStatement>());
        EXPECT_EQ(s->Parent()->Parent()->Parent(), s->FindFirstParent<sem::SwitchStatement>());
        EXPECT_EQ(s->Parent()->Parent()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
    }
    {
        auto* s = Sem().Get(stmt_c);
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(s->Parent(), s->FindFirstParent<sem::BlockStatement>());
        EXPECT_EQ(s->Parent(), s->Block());
        EXPECT_EQ(s->Parent()->Parent(), s->FindFirstParent<sem::CaseStatement>());
        EXPECT_EQ(s->Parent()->Parent()->Parent(), s->FindFirstParent<sem::SwitchStatement>());
        EXPECT_EQ(s->Parent()->Parent()->Parent()->Parent(),
                  s->FindFirstParent<sem::FunctionBlockStatement>());
    }
}

}  // namespace
}  // namespace tint::resolver
