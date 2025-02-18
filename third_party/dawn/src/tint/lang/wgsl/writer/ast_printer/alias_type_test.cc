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

TEST_F(WgslASTPrinterTest, EmitAlias_F32) {
    auto* alias = Alias("a", ty.f32());

    ASTPrinter& gen = Build();
    gen.EmitTypeDecl(alias);

    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(alias a = f32;
)");
}

TEST_F(WgslASTPrinterTest, EmitTypeDecl_Struct) {
    auto* s = Structure("A", Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.i32()),
                             });

    auto* alias = Alias("B", ty.Of(s));

    ASTPrinter& gen = Build();
    gen.EmitTypeDecl(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());

    gen.EmitTypeDecl(alias);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(struct A {
  a : f32,
  b : i32,
}
alias B = A;
)");
}

TEST_F(WgslASTPrinterTest, EmitAlias_ToStruct) {
    auto* s = Structure("A", Vector{
                                 Member("a", ty.f32()),
                                 Member("b", ty.i32()),
                             });

    auto* alias = Alias("B", ty.Of(s));

    ASTPrinter& gen = Build();

    gen.EmitTypeDecl(alias);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(alias B = A;
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
