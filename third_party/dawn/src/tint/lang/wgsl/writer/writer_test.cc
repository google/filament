// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/writer/writer.h"

#include <ostream>
#include <string>
#include <string_view>

#include "gtest/gtest.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/program_options.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"
#include "src/tint/utils/text/string.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::wgsl::writer {
namespace {

/// Class used for IR to Program tests
class WgslIRWriterTest : public core::ir::IRTestHelper {
  public:
    struct Result {
        /// The resulting WGSL
        std::string wgsl;
        /// The resulting AST
        std::string ast;
        /// The resulting IR before raising
        std::string ir_pre_raise;
        /// The resulting IR after raising
        std::string ir_post_raise;
        /// The resulting error
        std::string err;
        /// The expected WGSL
        std::string expected;
    };

    /// @returns the WGSL generated from the IR
    Result Run(std::string_view expected_wgsl) {
        Result result;

        result.ir_pre_raise = str();

        if (auto res = tint::wgsl::writer::Raise(mod); res != Success) {
            result.err = res.Failure().reason;
            return result;
        }

        result.ir_post_raise = str();

        writer::ProgramOptions program_options;
        program_options.allowed_features = AllowedFeatures::Everything();
        auto output_program = wgsl::writer::IRToProgram(mod, program_options);
        if (!output_program.IsValid()) {
            result.err = output_program.Diagnostics().Str();
            result.ast = Program::printer(output_program);
            return result;
        }

        auto output = wgsl::writer::Generate(output_program);
        if (output != Success) {
            std::stringstream ss;
            ss << "wgsl::Generate() errored: " << output.Failure();
            result.err = ss.str();
            result.ast = Program::printer(output_program);
            return result;
        }

        result.expected = tint::TrimSpace(expected_wgsl);
        if (!result.expected.empty()) {
            result.expected = "\n" + result.expected + "\n";
        }

        result.wgsl = std::string(tint::TrimSpace(output->wgsl));
        if (!result.wgsl.empty()) {
            result.wgsl = "\n" + result.wgsl + "\n";
        }

        return result;
    }
};

std::ostream& operator<<(std::ostream& o, const WgslIRWriterTest::Result& res) {
    if (!res.err.empty()) {
        o << "============================\n"
          << "== Error                  ==\n"
          << "============================\n"
          << res.err << "\n\n";
    }
    if (!res.ir_pre_raise.empty()) {
        o << "============================\n"
          << "== IR (pre-raise)         ==\n"
          << "============================\n"
          << res.ir_pre_raise << "\n\n";
    }
    if (!res.ir_post_raise.empty()) {
        o << "============================\n"
          << "== IR (post-raise)        ==\n"
          << "============================\n"
          << res.ir_post_raise << "\n\n";
    }
    if (!res.ast.empty()) {
        o << "============================\n"
          << "== AST                    ==\n"
          << "============================\n"
          << res.ast << "\n\n";
    }
    return o;
}

#define RUN_TEST(EXPECTED)                               \
    do {                                                 \
        if (auto res = Run(EXPECTED); res.err.empty()) { \
            EXPECT_EQ(res.expected, res.wgsl) << res;    \
        } else {                                         \
            FAIL() << res;                               \
        }                                                \
    } while (false)

TEST_F(WgslIRWriterTest, NameConflict_NamedBeforeUnnamed_ModuleScope) {
    b.Append(mod.root_block, [&] {
        b.Var<private_, u32>("v");
        b.Var<private_, u32>();
    });

    RUN_TEST(R"(
var<private> v : u32;

var<private> v_1 : u32;
)");
}

TEST_F(WgslIRWriterTest, NameConflict_NamedBeforeUnnamed_FunctionScope) {
    auto* fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var<function, u32>("v");
        b.Var<function, u32>();
        b.Return(fn);
    });

    RUN_TEST(R"(
fn f() {
  var v : u32;
  var v_1 : u32;
}
)");
}

TEST_F(WgslIRWriterTest, NameConflict_UnnamedBeforeNamed_ModuleScope) {
    b.Append(mod.root_block, [&] {
        b.Var<private_, u32>();
        b.Var<private_, u32>("v");
    });

    RUN_TEST(R"(
var<private> v_1 : u32;

var<private> v : u32;
)");
}

TEST_F(WgslIRWriterTest, NameConflict_UnnamedBeforeNamed_FunctionScope) {
    auto* fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var<function, u32>();
        b.Var<function, u32>("v");
        b.Return(fn);
    });

    RUN_TEST(R"(
fn f() {
  var v_1 : u32;
  var v : u32;
}
)");
}

////////////////////////////////////////////////////////////////////////////////
// Short-circuiting binary ops
////////////////////////////////////////////////////////////////////////////////
TEST_F(WgslIRWriterTest, ShortCircuit_And_Param_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(pa);
        if_->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if_->True(), [&] { b.ExitIf(if_, pb); });
        b.Append(if_->False(), [&] { b.ExitIf(if_, false); });

        b.Return(fn, if_);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool) -> bool {
  return (a && b);
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Param_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, pb); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });

        auto* if2 = b.If(if1);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, pc); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a && b) && c);
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Param_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] {
            auto* if2 = b.If(pb);
            if2->SetResult(b.InstructionResult(ty.bool_()));
            b.Append(if2->True(), [&] { b.ExitIf(if2, pc); });
            b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

            b.ExitIf(if1, if2);
        });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });
        b.Return(fn, if1);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return (a && (b && c));
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Let_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(pa);
        if_->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if_->True(), [&] { b.ExitIf(if_, pb); });
        b.Append(if_->False(), [&] { b.ExitIf(if_, false); });

        mod.SetName(if_, "l");
        b.Return(fn, if_);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool) -> bool {
  let l = (a && b);
  return l;
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Let_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, pb); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });

        auto* if2 = b.If(if1);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, pc); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

        mod.SetName(if2, "l");
        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = ((a && b) && c);
  return l;
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Let_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] {
            auto* if2 = b.If(pb);
            if2->SetResult(b.InstructionResult(ty.bool_()));
            b.Append(if2->True(), [&] { b.ExitIf(if2, pc); });
            b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

            b.ExitIf(if1, if2);
        });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });

        mod.SetName(if1, "l");
        b.Return(fn, if1);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = (a && (b && c));
  return l;
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Call_2) {
    auto* fn_a = b.Function("a", ty.bool_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, true); });

    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn = b.Function("f", ty.bool_());

    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(b.Call(ty.bool_(), fn_a));
        if_->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if_->True(), [&] { b.ExitIf(if_, b.Call(ty.bool_(), fn_b)); });
        b.Append(if_->False(), [&] { b.ExitIf(if_, false); });

        b.Return(fn, if_);
    });

    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn f() -> bool {
  return (a() && b());
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Call_3_ab_c) {
    auto* fn_a = b.Function("a", ty.bool_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, true); });

    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn_c = b.Function("c", ty.bool_());
    b.Append(fn_c->Block(), [&] { b.Return(fn_c, true); });

    auto* fn = b.Function("f", ty.bool_());

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(b.Call(ty.bool_(), fn_a));
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, b.Call(ty.bool_(), fn_b)); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });

        auto* if2 = b.If(if1);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, b.Call(ty.bool_(), fn_c)); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return ((a() && b()) && c());
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Call_3_a_bc) {
    auto* fn_a = b.Function("a", ty.bool_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, true); });

    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn_c = b.Function("c", ty.bool_());
    b.Append(fn_c->Block(), [&] { b.Return(fn_c, true); });

    auto* fn = b.Function("f", ty.bool_());

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(b.Call(ty.bool_(), fn_a));
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] {
            auto* if2 = b.If(b.Call(ty.bool_(), fn_b));
            if2->SetResult(b.InstructionResult(ty.bool_()));
            b.Append(if2->True(), [&] { b.ExitIf(if2, b.Call(ty.bool_(), fn_c)); });
            b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

            b.ExitIf(if1, if2);
        });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });

        b.Return(fn, if1);
    });

    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return (a() && (b() && c()));
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Param_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(pa);
        if_->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if_->True(), [&] { b.ExitIf(if_, true); });
        b.Append(if_->False(), [&] { b.ExitIf(if_, pb); });

        b.Return(fn, if_);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool) -> bool {
  return (a || b);
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Param_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, pb); });

        auto* if2 = b.If(if1);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, pc); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return ((a || b) || c);
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Param_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] {
            auto* if2 = b.If(pb);
            if2->SetResult(b.InstructionResult(ty.bool_()));
            b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
            b.Append(if2->False(), [&] { b.ExitIf(if2, pc); });

            b.ExitIf(if1, if2);
        });

        b.Return(fn, if1);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  return (a || (b || c));
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Let_2) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    fn->SetParams({pa, pb});

    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(pa);
        if_->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if_->True(), [&] { b.ExitIf(if_, true); });
        b.Append(if_->False(), [&] { b.ExitIf(if_, pb); });

        mod.SetName(if_, "l");
        b.Return(fn, if_);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool) -> bool {
  let l = (a || b);
  return l;
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Let_3_ab_c) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, pb); });

        auto* if2 = b.If(if1);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, pc); });

        mod.SetName(if2, "l");
        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = ((a || b) || c);
  return l;
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Let_3_a_bc) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pb = b.FunctionParam("b", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] {
            auto* if2 = b.If(pb);
            if2->SetResult(b.InstructionResult(ty.bool_()));
            b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
            b.Append(if2->False(), [&] { b.ExitIf(if2, pc); });

            b.ExitIf(if1, if2);
        });

        mod.SetName(if1, "l");
        b.Return(fn, if1);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let l = (a || (b || c));
  return l;
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Call_2) {
    auto* fn_a = b.Function("a", ty.bool_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, true); });

    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn = b.Function("f", ty.bool_());

    b.Append(fn->Block(), [&] {
        auto* if_ = b.If(b.Call(ty.bool_(), fn_a));
        if_->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if_->True(), [&] { b.ExitIf(if_, true); });
        b.Append(if_->False(), [&] { b.ExitIf(if_, b.Call(ty.bool_(), fn_b)); });

        b.Return(fn, if_);
    });

    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn f() -> bool {
  return (a() || b());
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Call_3_ab_c) {
    auto* fn_a = b.Function("a", ty.bool_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, true); });

    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn_c = b.Function("c", ty.bool_());
    b.Append(fn_c->Block(), [&] { b.Return(fn_c, true); });

    auto* fn = b.Function("f", ty.bool_());

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(b.Call(ty.bool_(), fn_a));
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, b.Call(ty.bool_(), fn_b)); });

        auto* if2 = b.If(if1);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, b.Call(ty.bool_(), fn_c)); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return ((a() || b()) || c());
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Call_3_a_bc) {
    auto* fn_a = b.Function("a", ty.bool_());
    b.Append(fn_a->Block(), [&] { b.Return(fn_a, true); });

    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn_c = b.Function("c", ty.bool_());
    b.Append(fn_c->Block(), [&] { b.Return(fn_c, true); });

    auto* fn = b.Function("f", ty.bool_());

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(b.Call(ty.bool_(), fn_a));
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] {
            auto* if2 = b.If(b.Call(ty.bool_(), fn_b));
            if2->SetResult(b.InstructionResult(ty.bool_()));
            b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
            b.Append(if2->False(), [&] { b.ExitIf(if2, b.Call(ty.bool_(), fn_c)); });

            b.ExitIf(if1, if2);
        });

        b.Return(fn, if1);
    });

    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  return (a() || (b() || c()));
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Mixed) {
    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn_d = b.Function("d", ty.bool_());
    b.Append(fn_d->Block(), [&] { b.Return(fn_d, true); });

    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam("a", ty.bool_());
    auto* pc = b.FunctionParam("c", ty.bool_());
    fn->SetParams({pa, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pa);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, b.Call(ty.bool_(), fn_b)); });

        auto* if2 = b.If(if1);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] {
            auto* if3 = b.If(pc);
            if3->SetResult(b.InstructionResult(ty.bool_()));
            b.Append(if3->True(), [&] { b.ExitIf(if3, true); });
            b.Append(if3->False(), [&] { b.ExitIf(if3, b.Call(ty.bool_(), fn_d)); });

            b.ExitIf(if2, if3);
        });
        b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn b() -> bool {
  return true;
}

fn d() -> bool {
  return true;
}

fn f(a : bool, c : bool) -> bool {
  return ((a || b()) && (c || d()));
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_ParamCallParam_a_bc_EarlyEval) {
    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pc});

    b.Append(fn->Block(), [&] {
        // 'b() && c' is evaluated before 'a'.
        auto* if1 = b.If(b.Call(ty.bool_(), fn_b));
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, pc); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });

        auto* if2 = b.If(pa);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, if1); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, false); });
        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn b() -> bool {
  return true;
}

fn f(a : bool, c : bool) -> bool {
  let v = (b() && c);
  return (a && v);
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Call_3_a_bc_EarlyEval) {
    auto* fn_a = b.Function("a", ty.bool_());

    b.Append(fn_a->Block(), [&] { b.Return(fn_a, true); });

    auto* fn_b = b.Function("b", ty.bool_());

    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn_c = b.Function("c", ty.bool_());

    b.Append(fn_c->Block(), [&] { b.Return(fn_c, true); });

    auto* fn = b.Function("f", ty.bool_());

    b.Append(fn->Block(), [&] {
        // 'b() && c()' is evaluated before 'a()'.
        auto* if1 = b.If(b.Call(ty.bool_(), fn_b));
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, b.Call(ty.bool_(), fn_c)); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });

        auto* if2 = b.If(b.Call(ty.bool_(), fn_a));
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, if1); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  let v = (b() && c());
  return (a() && v);
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_And_Param_3_a_bc_EarlyEval) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pb);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, pc); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, false); });

        auto* if2 = b.If(pa);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, if1); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, false); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let v = (b && c);
  return (a && v);
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_ParamCallParam_a_bc_EarlyEval) {
    auto* fn_b = b.Function("b", ty.bool_());
    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pc});

    b.Append(fn->Block(), [&] {
        // 'b() && c' is evaluated before 'a'.
        auto* if1 = b.If(b.Call(ty.bool_(), fn_b));
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, pc); });
        auto* v = b.Let("v", if1);

        auto* if2 = b.If(pa);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, v); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn b() -> bool {
  return true;
}

fn f(a : bool, c : bool) -> bool {
  let v = (b() || c);
  return (a || v);
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Call_3_a_bc_EarlyEval) {
    auto* fn_a = b.Function("a", ty.bool_());

    b.Append(fn_a->Block(), [&] { b.Return(fn_a, true); });

    auto* fn_b = b.Function("b", ty.bool_());

    b.Append(fn_b->Block(), [&] { b.Return(fn_b, true); });

    auto* fn_c = b.Function("c", ty.bool_());

    b.Append(fn_c->Block(), [&] { b.Return(fn_c, true); });

    auto* fn = b.Function("f", ty.bool_());

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(b.Call(ty.bool_(), fn_b));
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, b.Call(ty.bool_(), fn_c)); });
        auto* v = b.Let("v", if1);

        auto* if2 = b.If(b.Call(ty.bool_(), fn_a));
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, v); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn a() -> bool {
  return true;
}

fn b() -> bool {
  return true;
}

fn c() -> bool {
  return true;
}

fn f() -> bool {
  let v = (b() || c());
  return (a() || v);
}
)");
}

TEST_F(WgslIRWriterTest, ShortCircuit_Or_Param_3_a_bc_EarlyEval) {
    auto* fn = b.Function("f", ty.bool_());
    auto* pa = b.FunctionParam(ty.bool_());
    auto* pb = b.FunctionParam(ty.bool_());
    auto* pc = b.FunctionParam(ty.bool_());
    mod.SetName(pa, "a");
    mod.SetName(pb, "b");
    mod.SetName(pc, "c");
    fn->SetParams({pa, pb, pc});

    b.Append(fn->Block(), [&] {
        auto* if1 = b.If(pb);
        if1->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if1->True(), [&] { b.ExitIf(if1, true); });
        b.Append(if1->False(), [&] { b.ExitIf(if1, pc); });

        auto* if2 = b.If(pa);
        if2->SetResult(b.InstructionResult(ty.bool_()));
        b.Append(if2->True(), [&] { b.ExitIf(if2, true); });
        b.Append(if2->False(), [&] { b.ExitIf(if2, if1); });

        b.Return(fn, if2);
    });

    RUN_TEST(R"(
fn f(a : bool, b : bool, c : bool) -> bool {
  let v = (b || c);
  return (a || v);
}
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
