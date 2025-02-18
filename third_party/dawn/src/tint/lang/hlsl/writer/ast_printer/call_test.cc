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
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

using HlslASTPrinterTest_Call = TestHelper;

TEST_F(HlslASTPrinterTest_Call, EmitExpression_Call_WithoutParams) {
    Func("my_func", tint::Empty, ty.f32(), Vector{Return(1.23_f)});

    auto* call = Call("my_func");
    WrapInFunction(call);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "my_func()");
}

TEST_F(HlslASTPrinterTest_Call, EmitExpression_Call_WithParams) {
    Func("my_func",
         Vector{
             Param(Sym(), ty.f32()),
             Param(Sym(), ty.f32()),
         },
         ty.f32(), Vector{Return(1.23_f)});
    GlobalVar("param1", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("param2", ty.f32(), core::AddressSpace::kPrivate);

    auto* call = Call("my_func", "param1", "param2");
    WrapInFunction(call);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, call)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "my_func(param1, param2)");
}

TEST_F(HlslASTPrinterTest_Call, EmitStatement_Call) {
    Func("my_func",
         Vector{
             Param(Sym(), ty.f32()),
             Param(Sym(), ty.f32()),
         },
         ty.void_(), tint::Empty, tint::Empty);
    GlobalVar("param1", ty.f32(), core::AddressSpace::kPrivate);
    GlobalVar("param2", ty.f32(), core::AddressSpace::kPrivate);

    auto* call = CallStmt(Call("my_func", "param1", "param2"));
    WrapInFunction(call);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    ASSERT_TRUE(gen.EmitStatement(call)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), "  my_func(param1, param2);\n");
}

}  // namespace
}  // namespace tint::hlsl::writer
