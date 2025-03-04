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
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, EmitVariable) {
    auto* v = GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(var<private> a : f32;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_AddressSpace) {
    auto* v = GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(var<private> a : f32;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Access_Read) {
    auto* s = Structure("S", Vector{Member("a", ty.i32())});
    auto* v = GlobalVar("a", ty.Of(s), core::AddressSpace::kStorage, core::Access::kRead,
                        Binding(0_a), Group(0_a));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(@binding(0) @group(0) var<storage, read> a : S;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Access_ReadWrite) {
    auto* s = Structure("S", Vector{Member("a", ty.i32())});
    auto* v = GlobalVar("a", ty.Of(s), core::AddressSpace::kStorage, core::Access::kReadWrite,
                        Binding(0_a), Group(0_a));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(@binding(0) @group(0) var<storage, read_write> a : S;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Decorated) {
    auto* v =
        GlobalVar("a", ty.sampler(core::type::SamplerKind::kSampler), Group(1_a), Binding(2_a));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(@group(1) @binding(2) var a : sampler;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Initializer) {
    auto* v = GlobalVar("a", ty.f32(), core::AddressSpace::kPrivate, Expr(1_f));

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(var<private> a : f32 = 1.0f;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Let_Explicit) {
    auto* v = Let("a", ty.f32(), Expr(1_f));
    WrapInFunction(v);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(let a : f32 = 1.0f;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Let_Inferred) {
    auto* v = Let("a", Expr(1_f));
    WrapInFunction(v);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(let a = 1.0f;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Const_Explicit) {
    auto* v = Const("a", ty.f32(), Expr(1_f));
    WrapInFunction(v);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(const a : f32 = 1.0f;)");
}

TEST_F(WgslASTPrinterTest, EmitVariable_Const_Inferred) {
    auto* v = Const("a", Expr(1_f));
    WrapInFunction(v);

    ASTPrinter& gen = Build();

    StringStream out;
    gen.EmitVariable(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), R"(const a = 1.0f;)");
}

}  // namespace
}  // namespace tint::wgsl::writer
