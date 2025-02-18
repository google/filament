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

#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_If) {
    GlobalVar("cond", ty.bool_(), core::AddressSpace::kPrivate);

    auto* cond = Expr("cond");
    auto* body = Block(Return());
    auto* i = If(cond, body);
    WrapInFunction(i);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(i);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  if (cond) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_IfWithElseIf) {
    GlobalVar("cond", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("else_cond", ty.bool_(), core::AddressSpace::kPrivate);

    auto* else_cond = Expr("else_cond");
    auto* else_body = Block(Return());

    auto* cond = Expr("cond");
    auto* body = Block(Return());
    auto* i = If(cond, body, Else(If(else_cond, else_body)));
    WrapInFunction(i);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(i);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  if (cond) {
    return;
  } else if (else_cond) {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_IfWithElse) {
    GlobalVar("cond", ty.bool_(), core::AddressSpace::kPrivate);

    auto* else_body = Block(Return());

    auto* cond = Expr("cond");
    auto* body = Block(Return());
    auto* i = If(cond, body, Else(else_body));
    WrapInFunction(i);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(i);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  if (cond) {
    return;
  } else {
    return;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_IfWithMultiple) {
    GlobalVar("cond", ty.bool_(), core::AddressSpace::kPrivate);
    GlobalVar("else_cond", ty.bool_(), core::AddressSpace::kPrivate);

    auto* else_cond = Expr("else_cond");

    auto* else_body = Block(Return());

    auto* else_body_2 = Block(Return());

    auto* cond = Expr("cond");
    auto* body = Block(Return());
    auto* i = If(cond, body, Else(If(else_cond, else_body, Else(else_body_2))));
    WrapInFunction(i);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(i);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  if (cond) {
    return;
  } else if (else_cond) {
    return;
  } else {
    return;
  }
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
