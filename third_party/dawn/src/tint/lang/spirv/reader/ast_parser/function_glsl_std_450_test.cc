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

#include "gmock/gmock.h"
#include "src/tint/lang/spirv/reader/ast_parser/function.h"
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::HasSubstr;

std::string Preamble() {
    return R"(
  OpCapability Shader
  %glsl = OpExtInstImport "GLSL.std.450"
  OpMemoryModel Logical GLSL450
  OpEntryPoint GLCompute %100 "main"
  OpExecutionMode %100 LocalSize 1 1 1

  OpName %u1 "u1"
  OpName %u2 "u2"
  OpName %u3 "u3"
  OpName %i1 "i1"
  OpName %i2 "i2"
  OpName %i3 "i3"
  OpName %f1 "f1"
  OpName %f2 "f2"
  OpName %f3 "f3"
  OpName %v2u1 "v2u1"
  OpName %v2u2 "v2u2"
  OpName %v2u3 "v2u3"
  OpName %v2i1 "v2i1"
  OpName %v2i2 "v2i2"
  OpName %v2i3 "v2i3"
  OpName %v2f1 "v2f1"
  OpName %v2f2 "v2f2"
  OpName %v2f3 "v2f3"
  OpName %v3f1 "v3f1"
  OpName %v3f2 "v3f2"
  OpName %v4f1 "v4f1"
  OpName %v4f2 "v4f2"
  OpName %m2x2f1 "m2x2f1"
  OpName %m3x3f1 "m3x3f1"
  OpName %m4x4f1 "m4x4f1"

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %uint_10 = OpConstant %uint 10
  %uint_15 = OpConstant %uint 15
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_35 = OpConstant %int 35
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
  %v3float = OpTypeVector %float 3
  %v4float = OpTypeVector %float 4
  %mat2v2float = OpTypeMatrix %v2float 2
  %mat3v3float = OpTypeMatrix %v3float 3
  %mat4v4float = OpTypeMatrix %v4float 4

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2uint_15_15 = OpConstantComposite %v2uint %uint_15 %uint_15
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2int_35_35 = OpConstantComposite %v2int %int_35 %int_35
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v2float_70_70 = OpConstantComposite %v2float %float_70 %float_70

  %v3float_50_60_70 = OpConstantComposite %v3float %float_50 %float_60 %float_70
  %v3float_60_70_50 = OpConstantComposite %v3float %float_60 %float_70 %float_50

  %v4float_50_50_50_50 = OpConstantComposite %v4float %float_50 %float_50 %float_50 %float_50

  %mat2v2float_50_60 = OpConstantComposite %mat2v2float %v2float_50_60 %v2float_50_60
  %mat3v3float_50_60_70 = OpConstantComposite %mat3v3float %v3float_50_60_70 %v3float_50_60_70 %v3float_50_60_70
  %mat4v4float_50_50_50_50 = OpConstantComposite %mat4v4float %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50 %v4float_50_50_50_50

  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  %u1 = OpCopyObject %uint %uint_10
  %u2 = OpCopyObject %uint %uint_15
  %u3 = OpCopyObject %uint %uint_20

  %i1 = OpCopyObject %int %int_30
  %i2 = OpCopyObject %int %int_35
  %i3 = OpCopyObject %int %int_40

  %f1 = OpCopyObject %float %float_50
  %f2 = OpCopyObject %float %float_60
  %f3 = OpCopyObject %float %float_70

  %v2u1 = OpCopyObject %v2uint %v2uint_10_20
  %v2u2 = OpCopyObject %v2uint %v2uint_20_10
  %v2u3 = OpCopyObject %v2uint %v2uint_15_15

  %v2i1 = OpCopyObject %v2int %v2int_30_40
  %v2i2 = OpCopyObject %v2int %v2int_40_30
  %v2i3 = OpCopyObject %v2int %v2int_35_35

  %v2f1 = OpCopyObject %v2float %v2float_50_60
  %v2f2 = OpCopyObject %v2float %v2float_60_50
  %v2f3 = OpCopyObject %v2float %v2float_70_70

  %v3f1 = OpCopyObject %v3float %v3float_50_60_70
  %v3f2 = OpCopyObject %v3float %v3float_60_70_50

  %v4f1 = OpCopyObject %v4float %v4float_50_50_50_50
  %v4f2 = OpCopyObject %v4float %v4f1

  %m2x2f1 = OpCopyObject %mat2v2float %mat2v2float_50_60
  %m3x3f1 = OpCopyObject %mat3v3float %mat3v3float_50_60_70
  %m4x4f1 = OpCopyObject %mat4v4float %mat4v4float_50_50_50_50
)";
}

struct GlslStd450Case {
    std::string opcode;
    std::string wgsl_func;
};
inline std::ostream& operator<<(std::ostream& out, GlslStd450Case c) {
    out << "GlslStd450Case(" << c.opcode << " " << c.wgsl_func << ")";
    return out;
}

// Nomenclature:
// Float = scalar float
// Floating = scalar float or vector-of-float
// Float3 = 3-element vector of float
// Int = scalar signed int
// Inting = scalar int or vector-of-int
// Uint = scalar unsigned int
// Uinting = scalar unsigned or vector-of-unsigned

using SpirvASTParserTest_GlslStd450_Float_Floating =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Float_FloatingFloating =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Floating_Floating =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Floating_FloatingFloating =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Floating_FloatingFloatingFloating =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Floating_FloatingInting =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Float3_Float3Float3 =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;

using SpirvASTParserTest_GlslStd450_Inting_Inting =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Inting_Inting_SignednessCoercing =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Inting_IntingInting =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Inting_IntingIntingInting =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Uinting_Uinting =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Uinting_UintingUinting =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;
using SpirvASTParserTest_GlslStd450_Uinting_UintingUintingUinting =
    SpirvASTParserTestBase<::testing::TestWithParam<GlslStd450Case>>;

TEST_P(SpirvASTParserTest_GlslStd450_Float_Floating, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                          GetParam().opcode + R"( %f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(f1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Float_Floating, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                          GetParam().opcode + R"( %v2f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2f1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Float_FloatingFloating, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                          GetParam().opcode + R"( %f1 %f2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(f1, f2);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Float_FloatingFloating, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                          GetParam().opcode + R"( %v2f1 %v2f2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2f1, v2f2);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Floating_Floating, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                          GetParam().opcode + R"( %f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(f1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Floating_Floating, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                          GetParam().opcode + R"( %v2f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2f1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Floating_FloatingFloating, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                          GetParam().opcode + R"( %f1 %f2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(f1, f2);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Floating_FloatingFloating, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                          GetParam().opcode + R"( %v2f1 %v2f2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2f1, v2f2);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Floating_FloatingFloatingFloating, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                          GetParam().opcode + R"( %f1 %f2 %f3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(f1, f2, f3);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Floating_FloatingFloatingFloating, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                          GetParam().opcode +
                          R"( %v2f1 %v2f2 %v2f3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2f1, v2f2, v2f3);"))
        << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Floating_FloatingInting, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl )" +
                          GetParam().opcode + R"( %f1 %i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(f1, i1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Floating_FloatingInting, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl )" +
                          GetParam().opcode +
                          R"( %v2f1 %v2i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2f1, v2i1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Float3_Float3Float3, Samples) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v3float %glsl )" +
                          GetParam().opcode +
                          R"( %v3f1 %v3f2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v3f1, v3f2);")) << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Float_Floating,
                         ::testing::Values(GlslStd450Case{"Length", "length"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Float_FloatingFloating,
                         ::testing::Values(GlslStd450Case{"Distance", "distance"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Floating_Floating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"Acos", "acos"},                //
                             {"Asin", "asin"},                //
                             {"Atan", "atan"},                //
                             {"Ceil", "ceil"},                //
                             {"Cos", "cos"},                  //
                             {"Cosh", "cosh"},                //
                             {"Degrees", "degrees"},          //
                             {"Exp", "exp"},                  //
                             {"Exp2", "exp2"},                //
                             {"FAbs", "abs"},                 //
                             {"FSign", "sign"},               //
                             {"Floor", "floor"},              //
                             {"Fract", "fract"},              //
                             {"InverseSqrt", "inverseSqrt"},  //
                             {"Log", "log"},                  //
                             {"Log2", "log2"},                //
                             {"Radians", "radians"},          //
                             {"Round", "round"},              //
                             {"RoundEven", "round"},          //
                             {"Sin", "sin"},                  //
                             {"Sinh", "sinh"},                //
                             {"Sqrt", "sqrt"},                //
                             {"Tan", "tan"},                  //
                             {"Tanh", "tanh"},                //
                             {"Trunc", "trunc"},              //
                         }));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Floating_FloatingFloating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"Atan2", "atan2"},
                             {"NMax", "max"},
                             {"NMin", "min"},
                             {"FMax", "max"},  // WGSL max promises more for NaN
                             {"FMin", "min"},  // WGSL min promises more for NaN
                             {"Pow", "pow"},
                             {"Step", "step"},
                         }));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Floating_FloatingInting,
                         ::testing::Values(GlslStd450Case{"Ldexp", "ldexp"}));
// For ldexp with unsigned second argument, see below.

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Float3_Float3Float3,
                         ::testing::Values(GlslStd450Case{"Cross", "cross"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Floating_FloatingFloatingFloating,
                         ::testing::ValuesIn(std::vector<GlslStd450Case>{
                             {"NClamp", "clamp"},
                             {"FClamp", "clamp"},  // WGSL FClamp promises more for NaN
                             {"Fma", "fma"},
                             {"FMix", "mix"},
                             {"SmoothStep", "smoothstep"}}));

TEST_P(SpirvASTParserTest_GlslStd450_Inting_Inting, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                          GetParam().opcode +
                          R"( %i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(i1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Inting_Inting_SignednessCoercing, Scalar_UnsignedArg) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                          GetParam().opcode +
                          R"( %u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(bitcast<i32>(u1));"))
        << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Inting_Inting_SignednessCoercing, Scalar_UnsignedResult) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                          GetParam().opcode +
                          R"( %i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<u32>(" + GetParam().wgsl_func + "(i1));"))
        << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Inting_Inting, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                          GetParam().opcode +
                          R"( %v2i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2i1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Inting_Inting_SignednessCoercing, Vector_UnsignedArg) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                          GetParam().opcode +
                          R"( %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(bitcast<vec2i>(v2u1));"))
        << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Inting_Inting_SignednessCoercing, Vector_UnsignedResult) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                          GetParam().opcode +
                          R"( %v2i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<vec2u>(" + GetParam().wgsl_func + "(v2i1));"))
        << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Inting_IntingInting, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                          GetParam().opcode +
                          R"( %i1 %i2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(i1, i2);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Inting_IntingInting, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                          GetParam().opcode +
                          R"( %v2i1 %v2i2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2i1, v2i2);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Inting_IntingIntingInting, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl )" +
                          GetParam().opcode +
                          R"( %i1 %i2 %i3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(i1, i2, i3);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Inting_IntingIntingInting, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2int %glsl )" +
                          GetParam().opcode +
                          R"( %v2i1 %v2i2 %v2i3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2i1, v2i2, v2i3);"))
        << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Inting_Inting,
                         ::testing::Values(GlslStd450Case{"SAbs", "abs"},
                                           GlslStd450Case{"FindILsb", "firstTrailingBit"},
                                           GlslStd450Case{"FindSMsb", "firstLeadingBit"},
                                           GlslStd450Case{"SSign", "sign"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Inting_Inting_SignednessCoercing,
                         ::testing::Values(GlslStd450Case{"SSign", "sign"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Inting_IntingInting,
                         ::testing::Values(GlslStd450Case{"SMax", "max"},
                                           GlslStd450Case{"SMin", "min"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Inting_IntingIntingInting,
                         ::testing::Values(GlslStd450Case{"SClamp", "clamp"}));

TEST_P(SpirvASTParserTest_GlslStd450_Uinting_Uinting, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                          GetParam().opcode +
                          R"( %u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(u1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Uinting_Uinting, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                          GetParam().opcode +
                          R"( %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2u1);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Uinting_UintingUinting, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                          GetParam().opcode + R"( %u1 %u2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(u1, u2);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Uinting_UintingUinting, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                          GetParam().opcode +
                          R"( %v2u1 %v2u2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2u1, v2u2);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Uinting_UintingUintingUinting, Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl )" +
                          GetParam().opcode + R"( %u1 %u2 %u3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(u1, u2, u3);")) << body;
}

TEST_P(SpirvASTParserTest_GlslStd450_Uinting_UintingUintingUinting, Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2uint %glsl )" +
                          GetParam().opcode +
                          R"( %v2u1 %v2u2 %v2u3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + GetParam().wgsl_func + "(v2u1, v2u2, v2u3);"))
        << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Uinting_Uinting,
                         ::testing::Values(GlslStd450Case{"FindILsb", "firstTrailingBit"},
                                           GlslStd450Case{"FindUMsb", "firstLeadingBit"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Uinting_UintingUinting,
                         ::testing::Values(GlslStd450Case{"UMax", "max"},
                                           GlslStd450Case{"UMin", "min"}));

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_Uinting_UintingUintingUinting,
                         ::testing::Values(GlslStd450Case{"UClamp", "clamp"}));

// Test Normalize.  WGSL does not have a scalar form of the normalize builtin.
// So we have to test it separately, as it does not fit the patterns tested
// above.

TEST_F(SpirvASTParserTest, Normalize_Scalar) {
    // Scalar normalize maps to sign.
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl Normalize %f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = sign(f1);")) << body;
}

TEST_F(SpirvASTParserTest, Normalize_Vector2) {
    // Scalar normalize always results in 1.0
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Normalize %v2f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = normalize(v2f1);")) << body;
}

TEST_F(SpirvASTParserTest, Normalize_Vector3) {
    // Scalar normalize always results in 1.0
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v3float %glsl Normalize %v3f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = normalize(v3f1);")) << body;
}

TEST_F(SpirvASTParserTest, Normalize_Vector4) {
    // Scalar normalize always results in 1.0
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v4float %glsl Normalize %v4f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = normalize(v4f1);")) << body;
}

// Check that we convert signedness of operands and result type.
// This is needed for each of the integer-based extended instructions.

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_SAbs) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl SAbs %u1
     %2 = OpExtInst %v2uint %glsl SAbs %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr(R"(let x_1 = bitcast<u32>(abs(bitcast<i32>(u1)));)")) << body;
    EXPECT_THAT(body, HasSubstr(R"(let x_2 = bitcast<vec2u>(abs(bitcast<vec2i>(v2u1)));)")) << body;
}

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_SMax) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl SMax %u1 %u2
     %2 = OpExtInst %v2uint %glsl SMax %v2u1 %v2u2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body,
                HasSubstr(R"(let x_1 = bitcast<u32>(max(bitcast<i32>(u1), bitcast<i32>(u2)));)"))
        << body;
    EXPECT_THAT(
        body,
        HasSubstr(R"(let x_2 = bitcast<vec2u>(max(bitcast<vec2i>(v2u1), bitcast<vec2i>(v2u2)));)"))
        << body;
}

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_SMin) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl SMin %u1 %u2
     %2 = OpExtInst %v2uint %glsl SMin %v2u1 %v2u2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body,
                HasSubstr(R"(let x_1 = bitcast<u32>(min(bitcast<i32>(u1), bitcast<i32>(u2)));)"))
        << body;
    EXPECT_THAT(
        body,
        HasSubstr(R"(let x_2 = bitcast<vec2u>(min(bitcast<vec2i>(v2u1), bitcast<vec2i>(v2u2)));)"))
        << body;
}

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_SClamp) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl SClamp %u1 %i2 %u3
     %2 = OpExtInst %v2uint %glsl SClamp %v2u1 %v2i2 %v2u3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(
        body,
        HasSubstr(R"(let x_1 = bitcast<u32>(clamp(bitcast<i32>(u1), i2, bitcast<i32>(u3)));)"))
        << body;
    EXPECT_THAT(
        body,
        HasSubstr(
            R"(let x_2 = bitcast<vec2u>(clamp(bitcast<vec2i>(v2u1), v2i2, bitcast<vec2i>(v2u3)));)"))
        << body;
}

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_UMax) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl UMax %i1 %i2
     %2 = OpExtInst %v2int %glsl UMax %v2i1 %v2i2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body,
                HasSubstr(R"(let x_1 = bitcast<i32>(max(bitcast<u32>(i1), bitcast<u32>(i2)));)"))
        << body;
    EXPECT_THAT(
        body,
        HasSubstr(R"(let x_2 = bitcast<vec2i>(max(bitcast<vec2u>(v2i1), bitcast<vec2u>(v2i2)));)"))
        << body;
}

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_UMin) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl UMin %i1 %i2
     %2 = OpExtInst %v2int %glsl UMin %v2i1 %v2i2
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body,
                HasSubstr(R"(let x_1 = bitcast<i32>(min(bitcast<u32>(i1), bitcast<u32>(i2)));)"))
        << body;
    EXPECT_THAT(
        body,
        HasSubstr(R"(let x_2 = bitcast<vec2i>(min(bitcast<vec2u>(v2i1), bitcast<vec2u>(v2i2)));)"))
        << body;
}

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_UClamp) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %int %glsl UClamp %i1 %u2 %i3
     %2 = OpExtInst %v2int %glsl UClamp %v2i1 %v2u2 %v2i3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(
        body,
        HasSubstr(R"(let x_1 = bitcast<i32>(clamp(bitcast<u32>(i1), u2, bitcast<u32>(i3)));)"))
        << body;
    EXPECT_THAT(
        body,
        HasSubstr(
            R"(let x_2 = bitcast<vec2i>(clamp(bitcast<vec2u>(v2i1), v2u2, bitcast<vec2u>(v2i3)));)"))
        << body;
}

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_FindILsb) {
    // Check conversion of:
    //   signed results to unsigned result to match first arg.
    //   unsigned results to signed result to match first arg.
    // This is the first extended instruction we've supported which goes both
    // ways.
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %uint %glsl FindILsb %i1
     %2 = OpExtInst %v2uint %glsl FindILsb %v2i1
     %3 = OpExtInst %int %glsl FindILsb %u1
     %4 = OpExtInst %v2int %glsl FindILsb %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr(R"(
let x_1 = bitcast<u32>(firstTrailingBit(i1));
let x_2 = bitcast<vec2u>(firstTrailingBit(v2i1));
let x_3 = bitcast<i32>(firstTrailingBit(u1));
let x_4 = bitcast<vec2i>(firstTrailingBit(v2u1));)"))
        << body;
}

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_FindSMsb) {
    // Check signedness conversion of arguments and results.
    //   SPIR-V signed arg -> keep it
    //      signed result -> keep it
    //      unsigned result -> cast result to unsigned
    //
    //   SPIR-V unsigned arg -> cast it to signed
    //      signed result -> keept it
    //      unsigned result -> cast result to unsigned
    const auto assembly = Preamble() + R"(
     ; signed arg
     ;    signed result
     %1 = OpExtInst %int %glsl FindSMsb %i1
     %2 = OpExtInst %v2int %glsl FindSMsb %v2i1

     ; signed arg
     ;    unsigned result
     %3 = OpExtInst %uint %glsl FindSMsb %i1
     %4 = OpExtInst %v2uint %glsl FindSMsb %v2i1

     ; unsigned arg
     ;    signed result
     %5 = OpExtInst %int %glsl FindSMsb %u1
     %6 = OpExtInst %v2int %glsl FindSMsb %v2u1

     ; unsigned arg
     ;    unsigned result
     %7 = OpExtInst %uint %glsl FindSMsb %u1
     %8 = OpExtInst %v2uint %glsl FindSMsb %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr(R"(
let x_1 = firstLeadingBit(i1);
let x_2 = firstLeadingBit(v2i1);
let x_3 = bitcast<u32>(firstLeadingBit(i1));
let x_4 = bitcast<vec2u>(firstLeadingBit(v2i1));
let x_5 = firstLeadingBit(bitcast<i32>(u1));
let x_6 = firstLeadingBit(bitcast<vec2i>(v2u1));
let x_7 = bitcast<u32>(firstLeadingBit(bitcast<i32>(u1)));
let x_8 = bitcast<vec2u>(firstLeadingBit(bitcast<vec2i>(v2u1)));
)")) << body;
}

TEST_F(SpirvASTParserTest, RectifyOperandsAndResult_FindUMsb) {
    // Check signedness conversion of arguments and results.
    //   SPIR-V signed arg -> cast arg to unsigned
    //      signed result -> cast result to signed
    //      unsigned result -> keep it
    //
    //   SPIR-V unsigned arg -> keep it
    //      signed result -> cast result to signed
    //      unsigned result -> keep it
    const auto assembly = Preamble() + R"(
     ; signed arg
     ;    signed result
     %1 = OpExtInst %int %glsl FindUMsb %i1
     %2 = OpExtInst %v2int %glsl FindUMsb %v2i1

     ; signed arg
     ;    unsigned result
     %3 = OpExtInst %uint %glsl FindUMsb %i1
     %4 = OpExtInst %v2uint %glsl FindUMsb %v2i1

     ; unsigned arg
     ;    signed result
     %5 = OpExtInst %int %glsl FindUMsb %u1
     %6 = OpExtInst %v2int %glsl FindUMsb %v2u1

     ; unsigned arg
     ;    unsigned result
     %7 = OpExtInst %uint %glsl FindUMsb %u1
     %8 = OpExtInst %v2uint %glsl FindUMsb %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr(R"(
let x_1 = bitcast<i32>(firstLeadingBit(bitcast<u32>(i1)));
let x_2 = bitcast<vec2i>(firstLeadingBit(bitcast<vec2u>(v2i1)));
let x_3 = firstLeadingBit(bitcast<u32>(i1));
let x_4 = firstLeadingBit(bitcast<vec2u>(v2i1));
let x_5 = bitcast<i32>(firstLeadingBit(u1));
let x_6 = bitcast<vec2i>(firstLeadingBit(v2u1));
let x_7 = firstLeadingBit(u1);
let x_8 = firstLeadingBit(v2u1);
)")) << body;
}

struct DataPackingCase {
    std::string opcode;
    std::string wgsl_func;
    uint32_t vec_size;
};

inline std::ostream& operator<<(std::ostream& out, DataPackingCase c) {
    out << "DataPacking(" << c.opcode << ")";
    return out;
}

using SpirvASTParserTest_GlslStd450_DataPacking =
    SpirvASTParserTestBase<::testing::TestWithParam<DataPackingCase>>;

TEST_P(SpirvASTParserTest_GlslStd450_DataPacking, Valid) {
    auto param = GetParam();
    const auto assembly = Preamble() + R"(
  %1 = OpExtInst %uint %glsl )" +
                          param.opcode + (param.vec_size == 2 ? " %v2f1" : " %v4f1") + R"(
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + param.wgsl_func + "(v" +
                                std::to_string(param.vec_size) + "f1);"))
        << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_DataPacking,
                         ::testing::ValuesIn(std::vector<DataPackingCase>{
                             {"PackSnorm4x8", "pack4x8snorm", 4},
                             {"PackUnorm4x8", "pack4x8unorm", 4},
                             {"PackSnorm2x16", "pack2x16snorm", 2},
                             {"PackUnorm2x16", "pack2x16unorm", 2},
                             {"PackHalf2x16", "pack2x16float", 2}}));

using SpirvASTParserTest_GlslStd450_DataUnpacking =
    SpirvASTParserTestBase<::testing::TestWithParam<DataPackingCase>>;

TEST_P(SpirvASTParserTest_GlslStd450_DataUnpacking, Valid) {
    auto param = GetParam();
    const auto assembly = Preamble() + R"(
  %1 = OpExtInst )" + (param.vec_size == 2 ? "%v2float" : "%v4float") +
                          std::string(" %glsl ") + param.opcode + R"( %u1
  OpReturn
  OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = " + param.wgsl_func + "(u1);")) << body;
}

INSTANTIATE_TEST_SUITE_P(Samples,
                         SpirvASTParserTest_GlslStd450_DataUnpacking,
                         ::testing::ValuesIn(std::vector<DataPackingCase>{
                             {"UnpackSnorm4x8", "unpack4x8snorm", 4},
                             {"UnpackUnorm4x8", "unpack4x8unorm", 4},
                             {"UnpackSnorm2x16", "unpack2x16snorm", 2},
                             {"UnpackUnorm2x16", "unpack2x16unorm", 2},
                             {"UnpackHalf2x16", "unpack2x16float", 2}}));

TEST_F(SpirvASTParserTest, GlslStd450_Refract_Scalar) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl Refract %f1 %f2 %f3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    const auto* expected = R"(let x_1 = refract(vec2f(f1, 0.0f), vec2f(f2, 0.0f), f3).x;)";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

TEST_F(SpirvASTParserTest, GlslStd450_Refract_Vector) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Refract %v2f1 %v2f2 %f3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    const auto* expected = R"(let x_1 = refract(v2f1, v2f2, f3);)";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

TEST_F(SpirvASTParserTest, GlslStd450_FaceForward_Scalar) {
    const auto assembly = Preamble() + R"(
     %99 = OpFAdd %float %f1 %f1 ; normal operand has only one use
     %1 = OpExtInst %float %glsl FaceForward %99 %f2 %f3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    // The %99 sum only has one use.  Ensure it is evaluated only once by
    // making a let-declaration for it, since it is the normal operand to
    // the builtin function, and code generation uses it twice.
    const auto* expected = R"(let x_1 = select(-(x_99), x_99, ((f2 * f3) < 0.0f));)";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

TEST_F(SpirvASTParserTest, GlslStd450_FaceForward_Vector) {
    const auto assembly = Preamble() + R"(
     %99 = OpFAdd %v2float %v2f1 %v2f1
     %1 = OpExtInst %v2float %glsl FaceForward %v2f1 %v2f2 %v2f3
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    const auto* expected = R"(let x_1 = faceForward(v2f1, v2f2, v2f3);)";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

TEST_F(SpirvASTParserTest, GlslStd450_Reflect_Scalar) {
    const auto assembly = Preamble() + R"(
     %98 = OpFAdd %float %f1 %f1 ; has only one use
     %99 = OpFAdd %float %f2 %f2 ; has only one use
     %1 = OpExtInst %float %glsl Reflect %98 %99
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    // The %99 sum only has one use.  Ensure it is evaluated only once by
    // making a let-declaration for it, since it is the normal operand to
    // the builtin function, and code generation uses it twice.
    const auto* expected = R"(let x_1 = (x_98 - (2.0f * (x_99 * (x_99 * x_98))));)";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

TEST_F(SpirvASTParserTest, GlslStd450_Reflect_Vector) {
    const auto assembly = Preamble() + R"(
     %98 = OpFAdd %v2float %v2f1 %v2f1
     %99 = OpFAdd %v2float %v2f2 %v2f2
     %1 = OpExtInst %v2float %glsl Reflect %98 %99
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    const auto* expected = R"(
let x_98 = (v2f1 + v2f1);
let x_99 = (v2f2 + v2f2);
let x_1 = reflect(x_98, x_99);
)";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

// For ldexp with signed second argument, see above.
TEST_F(SpirvASTParserTest, GlslStd450_Ldexp_Scalar_Float_Uint) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl Ldexp %f1 %u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    const auto* expected = "let x_1 = ldexp(f1, i32(u1));";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

TEST_F(SpirvASTParserTest, GlslStd450_Ldexp_Vector_Floatvec_Uintvec) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %v2float %glsl Ldexp %v2f1 %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    const auto* expected = "let x_1 = ldexp(v2f1, vec2i(v2u1));";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

using SpirvASTParserTest_GlslStd450_Determinant =
    SpirvASTParserTestBase<::testing::TestWithParam<std::string>>;
TEST_P(SpirvASTParserTest_GlslStd450_Determinant, Test) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %float %glsl Determinant %)" +
                          GetParam() + R"(
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);
    std::string expected = "let x_1 = determinant(" + GetParam() + ");";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}
INSTANTIATE_TEST_SUITE_P(Test,
                         SpirvASTParserTest_GlslStd450_Determinant,
                         ::testing::Values("m2x2f1", "m3x3f1", "m4x4f1"));

TEST_F(SpirvASTParserTest, GlslStd450_MatrixInverse_mat2x2) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %mat2v2float %glsl MatrixInverse %m2x2f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);

    std::string expected =
        "let s = (1.0f / determinant(m2x2f1));\n"
        "let x_1 = mat2x2f(vec2f((s * m2x2f1[1u][1u]), (-(s) * "
        "m2x2f1[0u][1u])), vec2f((-(s) * m2x2f1[1u][0u]), (s * m2x2f1[0u][0u])));";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

TEST_F(SpirvASTParserTest, GlslStd450_MatrixInverse_mat3x3) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %mat3v3float %glsl MatrixInverse %m3x3f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);

    std::string expected =
        "let s = (1.0f / determinant(m3x3f1));\n"
        "let x_1 = (s * mat3x3f(vec3f(((m3x3f1[1u][1u] * m3x3f1[2u][2u]) - "
        "(m3x3f1[1u][2u] * m3x3f1[2u][1u])), ((m3x3f1[0u][2u] * m3x3f1[2u][1u]) - (m3x3f1[0u][1u] "
        "* m3x3f1[2u][2u])), ((m3x3f1[0u][1u] * m3x3f1[1u][2u]) - (m3x3f1[0u][2u] * "
        "m3x3f1[1u][1u]))), vec3f(((m3x3f1[1u][2u] * m3x3f1[2u][0u]) - (m3x3f1[1u][0u] * "
        "m3x3f1[2u][2u])), ((m3x3f1[0u][0u] * m3x3f1[2u][2u]) - (m3x3f1[0u][2u] * "
        "m3x3f1[2u][0u])), ((m3x3f1[0u][2u] * m3x3f1[1u][0u]) - (m3x3f1[0u][0u] * "
        "m3x3f1[1u][2u]))), vec3f(((m3x3f1[1u][0u] * m3x3f1[2u][1u]) - (m3x3f1[1u][1u] * "
        "m3x3f1[2u][0u])), ((m3x3f1[0u][1u] * m3x3f1[2u][0u]) - (m3x3f1[0u][0u] * "
        "m3x3f1[2u][1u])), ((m3x3f1[0u][0u] * m3x3f1[1u][1u]) - (m3x3f1[0u][1u] * "
        "m3x3f1[1u][0u])))));";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

TEST_F(SpirvASTParserTest, GlslStd450_MatrixInverse_mat4x4) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %mat4v4float %glsl MatrixInverse %m4x4f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);

    std::string expected =
        "let s = (1.0f / determinant(m4x4f1));\n"
        "let x_1 = (s * mat4x4f(vec4f((((m4x4f1[1u][1u] * ((m4x4f1[2u][2u] * "
        "m4x4f1[3u][3u]) - (m4x4f1[2u][3u] * m4x4f1[3u][2u]))) - (m4x4f1[1u][2u] * "
        "((m4x4f1[2u][1u] * m4x4f1[3u][3u]) - (m4x4f1[2u][3u] * m4x4f1[3u][1u])))) + "
        "(m4x4f1[1u][3u] * ((m4x4f1[2u][1u] * m4x4f1[3u][2u]) - (m4x4f1[2u][2u] * "
        "m4x4f1[3u][1u])))), (((-(m4x4f1[0u][1u]) * ((m4x4f1[2u][2u] * m4x4f1[3u][3u]) - "
        "(m4x4f1[2u][3u] * m4x4f1[3u][2u]))) + (m4x4f1[0u][2u] * ((m4x4f1[2u][1u] * "
        "m4x4f1[3u][3u]) - (m4x4f1[2u][3u] * m4x4f1[3u][1u])))) - (m4x4f1[0u][3u] * "
        "((m4x4f1[2u][1u] * m4x4f1[3u][2u]) - (m4x4f1[2u][2u] * m4x4f1[3u][1u])))), "
        "(((m4x4f1[0u][1u] * ((m4x4f1[1u][2u] * m4x4f1[3u][3u]) - (m4x4f1[1u][3u] * "
        "m4x4f1[3u][2u]))) - (m4x4f1[0u][2u] * ((m4x4f1[1u][1u] * m4x4f1[3u][3u]) - "
        "(m4x4f1[1u][3u] * m4x4f1[3u][1u])))) + (m4x4f1[0u][3u] * ((m4x4f1[1u][1u] * "
        "m4x4f1[3u][2u]) - (m4x4f1[1u][2u] * m4x4f1[3u][1u])))), (((-(m4x4f1[0u][1u]) * "
        "((m4x4f1[1u][2u] * m4x4f1[2u][3u]) - (m4x4f1[1u][3u] * m4x4f1[2u][2u]))) + "
        "(m4x4f1[0u][2u] * ((m4x4f1[1u][1u] * m4x4f1[2u][3u]) - (m4x4f1[1u][3u] * "
        "m4x4f1[2u][1u])))) - (m4x4f1[0u][3u] * ((m4x4f1[1u][1u] * m4x4f1[2u][2u]) - "
        "(m4x4f1[1u][2u] * m4x4f1[2u][1u]))))), vec4f((((-(m4x4f1[1u][0u]) * ((m4x4f1[2u][2u] "
        "* m4x4f1[3u][3u]) - (m4x4f1[2u][3u] * m4x4f1[3u][2u]))) + (m4x4f1[1u][2u] * "
        "((m4x4f1[2u][0u] * m4x4f1[3u][3u]) - (m4x4f1[2u][3u] * m4x4f1[3u][0u])))) - "
        "(m4x4f1[1u][3u] * ((m4x4f1[2u][0u] * m4x4f1[3u][2u]) - (m4x4f1[2u][2u] * "
        "m4x4f1[3u][0u])))), (((m4x4f1[0u][0u] * ((m4x4f1[2u][2u] * m4x4f1[3u][3u]) - "
        "(m4x4f1[2u][3u] * m4x4f1[3u][2u]))) - (m4x4f1[0u][2u] * ((m4x4f1[2u][0u] * "
        "m4x4f1[3u][3u]) - (m4x4f1[2u][3u] * m4x4f1[3u][0u])))) + (m4x4f1[0u][3u] * "
        "((m4x4f1[2u][0u] * m4x4f1[3u][2u]) - (m4x4f1[2u][2u] * m4x4f1[3u][0u])))), "
        "(((-(m4x4f1[0u][0u]) * ((m4x4f1[1u][2u] * m4x4f1[3u][3u]) - (m4x4f1[1u][3u] * "
        "m4x4f1[3u][2u]))) + (m4x4f1[0u][2u] * ((m4x4f1[1u][0u] * m4x4f1[3u][3u]) - "
        "(m4x4f1[1u][3u] * m4x4f1[3u][0u])))) - (m4x4f1[0u][3u] * ((m4x4f1[1u][0u] * "
        "m4x4f1[3u][2u]) - (m4x4f1[1u][2u] * m4x4f1[3u][0u])))), (((m4x4f1[0u][0u] * "
        "((m4x4f1[1u][2u] * m4x4f1[2u][3u]) - (m4x4f1[1u][3u] * m4x4f1[2u][2u]))) - "
        "(m4x4f1[0u][2u] * ((m4x4f1[1u][0u] * m4x4f1[2u][3u]) - (m4x4f1[1u][3u] * "
        "m4x4f1[2u][0u])))) + (m4x4f1[0u][3u] * ((m4x4f1[1u][0u] * m4x4f1[2u][2u]) - "
        "(m4x4f1[1u][2u] * m4x4f1[2u][0u]))))), vec4f((((m4x4f1[1u][0u] * ((m4x4f1[2u][1u] * "
        "m4x4f1[3u][3u]) - (m4x4f1[2u][3u] * m4x4f1[3u][1u]))) - (m4x4f1[1u][1u] * "
        "((m4x4f1[2u][0u] * m4x4f1[3u][3u]) - (m4x4f1[2u][3u] * m4x4f1[3u][0u])))) + "
        "(m4x4f1[1u][3u] * ((m4x4f1[2u][0u] * m4x4f1[3u][1u]) - (m4x4f1[2u][1u] * "
        "m4x4f1[3u][0u])))), (((-(m4x4f1[0u][0u]) * ((m4x4f1[2u][1u] * m4x4f1[3u][3u]) - "
        "(m4x4f1[2u][3u] * m4x4f1[3u][1u]))) + (m4x4f1[0u][1u] * ((m4x4f1[2u][0u] * "
        "m4x4f1[3u][3u]) - (m4x4f1[2u][3u] * m4x4f1[3u][0u])))) - (m4x4f1[0u][3u] * "
        "((m4x4f1[2u][0u] * m4x4f1[3u][1u]) - (m4x4f1[2u][1u] * m4x4f1[3u][0u])))), "
        "(((m4x4f1[0u][0u] * ((m4x4f1[1u][1u] * m4x4f1[3u][3u]) - (m4x4f1[1u][3u] * "
        "m4x4f1[3u][1u]))) - (m4x4f1[0u][1u] * ((m4x4f1[1u][0u] * m4x4f1[3u][3u]) - "
        "(m4x4f1[1u][3u] * m4x4f1[3u][0u])))) + (m4x4f1[0u][3u] * ((m4x4f1[1u][0u] * "
        "m4x4f1[3u][1u]) - (m4x4f1[1u][1u] * m4x4f1[3u][0u])))), (((-(m4x4f1[0u][0u]) * "
        "((m4x4f1[1u][1u] * m4x4f1[2u][3u]) - (m4x4f1[1u][3u] * m4x4f1[2u][1u]))) + "
        "(m4x4f1[0u][1u] * ((m4x4f1[1u][0u] * m4x4f1[2u][3u]) - (m4x4f1[1u][3u] * "
        "m4x4f1[2u][0u])))) - (m4x4f1[0u][3u] * ((m4x4f1[1u][0u] * m4x4f1[2u][1u]) - "
        "(m4x4f1[1u][1u] * m4x4f1[2u][0u]))))), vec4f((((-(m4x4f1[1u][0u]) * ((m4x4f1[2u][1u] "
        "* m4x4f1[3u][2u]) - (m4x4f1[2u][2u] * m4x4f1[3u][1u]))) + (m4x4f1[1u][1u] * "
        "((m4x4f1[2u][0u] * m4x4f1[3u][2u]) - (m4x4f1[2u][2u] * m4x4f1[3u][0u])))) - "
        "(m4x4f1[1u][2u] * ((m4x4f1[2u][0u] * m4x4f1[3u][1u]) - (m4x4f1[2u][1u] * "
        "m4x4f1[3u][0u])))), (((m4x4f1[0u][0u] * ((m4x4f1[2u][1u] * m4x4f1[3u][2u]) - "
        "(m4x4f1[2u][2u] * m4x4f1[3u][1u]))) - (m4x4f1[0u][1u] * ((m4x4f1[2u][0u] * "
        "m4x4f1[3u][2u]) - (m4x4f1[2u][2u] * m4x4f1[3u][0u])))) + (m4x4f1[0u][2u] * "
        "((m4x4f1[2u][0u] * m4x4f1[3u][1u]) - (m4x4f1[2u][1u] * m4x4f1[3u][0u])))), "
        "(((-(m4x4f1[0u][0u]) * ((m4x4f1[1u][1u] * m4x4f1[3u][2u]) - (m4x4f1[1u][2u] * "
        "m4x4f1[3u][1u]))) + (m4x4f1[0u][1u] * ((m4x4f1[1u][0u] * m4x4f1[3u][2u]) - "
        "(m4x4f1[1u][2u] * m4x4f1[3u][0u])))) - (m4x4f1[0u][2u] * ((m4x4f1[1u][0u] * "
        "m4x4f1[3u][1u]) - (m4x4f1[1u][1u] * m4x4f1[3u][0u])))), (((m4x4f1[0u][0u] * "
        "((m4x4f1[1u][1u] * m4x4f1[2u][2u]) - (m4x4f1[1u][2u] * m4x4f1[2u][1u]))) - "
        "(m4x4f1[0u][1u] * ((m4x4f1[1u][0u] * m4x4f1[2u][2u]) - (m4x4f1[1u][2u] * "
        "m4x4f1[2u][0u])))) + (m4x4f1[0u][2u] * ((m4x4f1[1u][0u] * m4x4f1[2u][1u]) - "
        "(m4x4f1[1u][1u] * m4x4f1[2u][0u])))))));";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}

TEST_F(SpirvASTParserTest, GlslStd450_MatrixInverse_MultipleInScope) {
    const auto assembly = Preamble() + R"(
     %1 = OpExtInst %mat2v2float %glsl MatrixInverse %m2x2f1
     %2 = OpExtInst %mat2v2float %glsl MatrixInverse %m2x2f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    const auto body = test::ToString(p->program(), ast_body);

    std::string expected =
        "let s = (1.0f / determinant(m2x2f1));\n"
        "let x_1 = mat2x2f(vec2f((s * m2x2f1[1u][1u]), (-(s) * "
        "m2x2f1[0u][1u])), vec2f((-(s) * m2x2f1[1u][0u]), (s * m2x2f1[0u][0u])));\n"
        "let s_1 = (1.0f / determinant(m2x2f1));\n"
        "let x_2 = mat2x2f(vec2f((s_1 * m2x2f1[1u][1u]), (-(s_1) * "
        "m2x2f1[0u][1u])), vec2f((-(s_1) * m2x2f1[1u][0u]), (s_1 * m2x2f1[0u][0u])));";

    EXPECT_THAT(body, HasSubstr(expected)) << body;
}
}  // namespace
}  // namespace tint::spirv::reader::ast_parser
