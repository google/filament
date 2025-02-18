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

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_Case) {
    auto* s =
        Switch(1_i, Case(CaseSelector(5_i), Block(create<ast::BreakStatement>())), DefaultCase());
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitCase(s->body[0]);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  case 5i: {
    break;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Case_MultipleSelectors) {
    auto* s = Switch(1_i,
                     Case(
                         Vector{
                             CaseSelector(5_i),
                             CaseSelector(6_i),
                         },
                         Block(create<ast::BreakStatement>())),
                     DefaultCase());
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitCase(s->body[0]);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  case 5i, 6i: {
    break;
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Case_Default) {
    auto* s = Switch(1_i, DefaultCase(Block(create<ast::BreakStatement>())));
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitCase(s->body[0]);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  default: {
    break;
  }
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
