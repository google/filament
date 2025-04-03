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

#include "src/tint/lang/hlsl/writer/writer.h"

namespace tint::hlsl::writer {
namespace {

using HlslASTPrinterTest = TestHelper;

TEST_F(HlslASTPrinterTest, InvalidProgram) {
    Diagnostics().AddError(Source{}) << "make the program invalid";
    ASSERT_FALSE(IsValid());
    auto program = resolver::Resolve(*this);
    ASSERT_FALSE(program.IsValid());
    auto result = Generate(program, Options{});
    EXPECT_NE(result, Success);
    EXPECT_EQ(result.Failure().reason, "error: make the program invalid");
}

TEST_F(HlslASTPrinterTest, UnsupportedExtension) {
    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumInternalRelaxedUniformLayout);

    ASTPrinter& gen = Build();

    ASSERT_FALSE(gen.Generate());
    EXPECT_EQ(
        gen.Diagnostics().Str(),
        R"(12:34 error: HLSL backend does not support extension 'chromium_internal_relaxed_uniform_layout')");
}

TEST_F(HlslASTPrinterTest, RequiresDirective) {
    Require(wgsl::LanguageFeature::kReadonlyAndReadwriteStorageTextures);

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), "");
}

TEST_F(HlslASTPrinterTest, Generate) {
    Func("my_func", {}, ty.void_(), {});

    ASTPrinter& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(void my_func() {
}
)");
}

struct HlslBuiltinData {
    core::BuiltinValue builtin;
    const char* attribute_name;
};
inline std::ostream& operator<<(std::ostream& out, HlslBuiltinData data) {
    StringStream str;
    str << data.builtin;
    out << str.str();
    return out;
}
using HlslBuiltinConversionTest = TestParamHelper<HlslBuiltinData>;
TEST_P(HlslBuiltinConversionTest, Emit) {
    auto params = GetParam();
    ASTPrinter& gen = Build();

    EXPECT_EQ(gen.builtin_to_attribute(params.builtin), std::string(params.attribute_name));
}
INSTANTIATE_TEST_SUITE_P(
    HlslASTPrinterTest,
    HlslBuiltinConversionTest,
    testing::Values(HlslBuiltinData{core::BuiltinValue::kPosition, "SV_Position"},
                    HlslBuiltinData{core::BuiltinValue::kVertexIndex, "SV_VertexID"},
                    HlslBuiltinData{core::BuiltinValue::kInstanceIndex, "SV_InstanceID"},
                    HlslBuiltinData{core::BuiltinValue::kFrontFacing, "SV_IsFrontFace"},
                    HlslBuiltinData{core::BuiltinValue::kFragDepth, "SV_Depth"},
                    HlslBuiltinData{core::BuiltinValue::kLocalInvocationId, "SV_GroupThreadID"},
                    HlslBuiltinData{core::BuiltinValue::kLocalInvocationIndex, "SV_GroupIndex"},
                    HlslBuiltinData{core::BuiltinValue::kGlobalInvocationId, "SV_DispatchThreadID"},
                    HlslBuiltinData{core::BuiltinValue::kWorkgroupId, "SV_GroupID"},
                    HlslBuiltinData{core::BuiltinValue::kSampleIndex, "SV_SampleIndex"},
                    HlslBuiltinData{core::BuiltinValue::kSampleMask, "SV_Coverage"}));

}  // namespace
}  // namespace tint::hlsl::writer
