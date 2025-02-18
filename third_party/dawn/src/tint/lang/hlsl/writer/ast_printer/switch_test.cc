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

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

using HlslASTPrinterTest_Switch = TestHelper;

TEST_F(HlslASTPrinterTest_Switch, Emit_Switch) {
    GlobalVar("cond", ty.i32(), core::AddressSpace::kPrivate);
    auto* s = Switch(                             //
        Expr("cond"),                             //
        Case(CaseSelector(5_i), Block(Break())),  //
        DefaultCase());
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  switch(cond) {
    case 5: {
      break;
    }
    default: {
      break;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Switch, Emit_Switch_MixedDefault) {
    GlobalVar("cond", ty.i32(), core::AddressSpace::kPrivate);
    auto* s = Switch(  //
        Expr("cond"),  //
        Case(Vector{CaseSelector(5_i), DefaultCaseSelector()}, Block(Break())));
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  switch(cond) {
    case 5:
    default: {
      break;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Switch, Emit_Switch_OnlyDefaultCase_NoSideEffectsCondition) {
    // var<private> cond : i32;
    // var<private> a : i32;
    // fn test() {
    //   switch(cond) {
    //     default: {
    //       a = 42;
    //     }
    //   }
    // }
    GlobalVar("cond", ty.i32(), core::AddressSpace::kPrivate);
    GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);
    auto* s = Switch(  //
        Expr("cond"),  //
        DefaultCase(Block(Assign(Expr("a"), Expr(42_i)))));
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  do {
    a = 42;
  } while (false);
)");
}

TEST_F(HlslASTPrinterTest_Switch, Emit_Switch_OnlyDefaultCase_SideEffectsCondition) {
    // var<private> global : i32;
    // fn bar() -> i32 {
    //   global = 84;
    //   return global;
    // }
    //
    // var<private> a : i32;
    // fn test() {
    //   switch(bar()) {
    //     default: {
    //       a = 42;
    //     }
    //   }
    // }
    GlobalVar("global", ty.i32(), core::AddressSpace::kPrivate);
    Func("bar", {}, ty.i32(),
         Vector{                               //
                Assign("global", Expr(84_i)),  //
                Return("global")});

    GlobalVar("a", ty.i32(), core::AddressSpace::kPrivate);
    auto* s = Switch(  //
        Call("bar"),   //
        DefaultCase(Block(Assign(Expr("a"), Expr(42_i)))));
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  bar();
  do {
    a = 42;
  } while (false);
)");
}

}  // namespace
}  // namespace tint::hlsl::writer
