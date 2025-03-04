// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/sem/helper_test.h"

#include "src/tint/lang/wgsl/sem/module.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::sem {
namespace {

class DiagnosticSeverityTest : public TestHelper {
  protected:
    /// Create a program with two functions, setting the severity for "chromium.unreachable_code"
    /// using an attribute. Test that we correctly track the severity of the filter for the
    /// functions and the statements with them.
    /// @param global_severity the global severity of the "chromium.unreachable_code" filter
    void Run(wgsl::DiagnosticSeverity global_severity) {
        // @diagnostic(off, chromium.unreachable_code)
        // fn foo() {
        //   @diagnostic(info, chromium.unreachable_code) {
        //     @diagnostic(error, chromium.unreachable_code)
        //     if (true) @diagnostic(warning, chromium.unreachable_code) {
        //       return;
        //     } else if (false) {
        //       return;
        //     } else @diagnostic(info, chromium.unreachable_code) {
        //       return;
        //     }
        //     return;
        //
        //     @diagnostic(error, chromium.unreachable_code)
        //     switch (42) @diagnostic(off, chromium.unreachable_code) {
        //       case 0 @diagnostic(warning, chromium.unreachable_code) {
        //         return;
        //       }
        //       default {
        //         return;
        //       }
        //     }
        //
        //     @diagnostic(error, chromium.unreachable_code)
        //     for (var i = 0; false; i++) @diagnostic(warning, chromium.unreachable_code) {
        //       return;
        //     }
        //
        //     @diagnostic(warning, chromium.unreachable_code)
        //     loop @diagnostic(off, chromium.unreachable_code) {
        //       return;
        //       continuing @diagnostic(info, chromium.unreachable_code) {
        //         break if true;
        //       }
        //     }
        //
        //     @diagnostic(error, chromium.unreachable_code)
        //     while (false) @diagnostic(warning, chromium.unreachable_code) {
        //       return;
        //     }
        //   }
        // }
        //
        // fn bar() {
        //   return;
        // }
        auto rule = wgsl::ChromiumDiagnosticRule::kUnreachableCode;
        auto func_severity = wgsl::DiagnosticSeverity::kOff;
        auto block_severity = wgsl::DiagnosticSeverity::kInfo;
        auto if_severity = wgsl::DiagnosticSeverity::kError;
        auto if_body_severity = wgsl::DiagnosticSeverity::kWarning;
        auto else_body_severity = wgsl::DiagnosticSeverity::kInfo;
        auto switch_severity = wgsl::DiagnosticSeverity::kError;
        auto switch_body_severity = wgsl::DiagnosticSeverity::kOff;
        auto case_severity = wgsl::DiagnosticSeverity::kWarning;
        auto for_severity = wgsl::DiagnosticSeverity::kError;
        auto for_body_severity = wgsl::DiagnosticSeverity::kWarning;
        auto loop_severity = wgsl::DiagnosticSeverity::kWarning;
        auto loop_body_severity = wgsl::DiagnosticSeverity::kOff;
        auto continuing_severity = wgsl::DiagnosticSeverity::kInfo;
        auto while_severity = wgsl::DiagnosticSeverity::kError;
        auto while_body_severity = wgsl::DiagnosticSeverity::kWarning;
        auto attr = [&](auto severity) {
            return tint::Vector{DiagnosticAttribute(severity, "chromium", "unreachable_code")};
        };

        auto* return_foo_if = Return();
        auto* return_foo_elseif = Return();
        auto* return_foo_else = Return();
        auto* return_foo_block = Return();
        auto* return_foo_case = Return();
        auto* return_foo_default = Return();
        auto* return_foo_for = Return();
        auto* return_foo_loop = Return();
        auto* return_foo_while = Return();
        auto* breakif_foo_continuing = BreakIf(Expr(true));
        auto* else_stmt = Block(tint::Vector{return_foo_else}, attr(else_body_severity));
        auto* elseif = If(Expr(false), Block(return_foo_elseif), Else(else_stmt));
        auto* if_foo = If(Expr(true), Block(tint::Vector{return_foo_if}, attr(if_body_severity)),
                          Else(elseif), attr(if_severity));
        auto* case_stmt =
            Case(CaseSelector(0_a), Block(tint::Vector{return_foo_case}, attr(case_severity)));
        auto* default_stmt = DefaultCase(Block(return_foo_default));
        auto* swtch = Switch(42_a, tint::Vector{case_stmt, default_stmt}, attr(switch_severity),
                             attr(switch_body_severity));
        auto* fl =
            For(Decl(Var("i", ty.i32())), false, Increment("i"),
                Block(tint::Vector{return_foo_for}, attr(for_body_severity)), attr(for_severity));
        auto* l = Loop(Block(tint::Vector{return_foo_loop}, attr(loop_body_severity)),
                       Block(tint::Vector{breakif_foo_continuing}, attr(continuing_severity)),
                       attr(loop_severity));
        auto* wl = While(false, Block(tint::Vector{return_foo_while}, attr(while_body_severity)),
                         attr(while_severity));
        auto* block_1 =
            Block(tint::Vector{if_foo, return_foo_block, swtch, fl, l, wl}, attr(block_severity));
        auto* func_attr = DiagnosticAttribute(func_severity, "chromium", "unreachable_code");
        auto* foo = Func("foo", {}, ty.void_(), tint::Vector{block_1}, tint::Vector{func_attr});

        auto* return_bar = Return();
        auto* bar = Func("bar", {}, ty.void_(), tint::Vector{return_bar});

        auto p = Build();
        EXPECT_TRUE(p.IsValid()) << p.Diagnostics();

        EXPECT_EQ(p.Sem().DiagnosticSeverity(foo, rule), func_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(block_1, rule), block_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(if_foo, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(if_foo->condition, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(if_foo->body, rule), if_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_if, rule), if_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(elseif, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(elseif->condition, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(elseif->body, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_elseif, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(else_stmt, rule), else_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_else, rule), else_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(swtch, rule), switch_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(swtch->condition, rule), switch_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(case_stmt, rule), switch_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(case_stmt->body, rule), case_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_case, rule), case_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(default_stmt, rule), switch_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_default, rule), switch_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl, rule), while_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl->initializer, rule), for_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl->condition, rule), for_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl->continuing, rule), for_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl->body, rule), for_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_for, rule), for_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(l, rule), loop_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(l->body, rule), loop_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(l->continuing, rule), continuing_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(breakif_foo_continuing, rule), continuing_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_loop, rule), loop_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(wl, rule), while_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(wl->condition, rule), while_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(wl->body, rule), while_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_while, rule), while_body_severity);

        EXPECT_EQ(p.Sem().DiagnosticSeverity(bar, rule), global_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_bar, rule), global_severity);
    }
};

TEST_F(DiagnosticSeverityTest, WithDirective) {
    DiagnosticDirective(wgsl::DiagnosticSeverity::kError, "chromium", "unreachable_code");
    Run(wgsl::DiagnosticSeverity::kError);
}

TEST_F(DiagnosticSeverityTest, WithoutDirective) {
    Run(wgsl::DiagnosticSeverity::kWarning);
}

}  // namespace
}  // namespace tint::sem
