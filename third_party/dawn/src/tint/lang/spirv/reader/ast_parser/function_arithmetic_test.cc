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
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::HasSubstr;

std::string Preamble() {
    return R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint Fragment %100 "main"
  OpExecutionMode %100 OriginUpperLeft

  OpName %v2float_50_60 "v2float_50_60"
  OpName %v2float_60_50 "v2float_60_50"
  OpName %v3float_50_60_70 "v3float_50_60_70"
  OpName %v3float_60_70_50 "v3float_60_70_50"

  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %uint_10 = OpConstant %uint 10
  %uint_20 = OpConstant %uint 20
  %int_30 = OpConstant %int 30
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60
  %float_70 = OpConstant %float 70

  %ptr_uint = OpTypePointer Function %uint
  %ptr_int = OpTypePointer Function %int
  %ptr_float = OpTypePointer Function %float

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2
  %v3float = OpTypeVector %float 3

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
  %v3float_50_60_70 = OpConstantComposite %v3float %float_50 %float_60 %float_70
  %v3float_60_70_50 = OpConstantComposite %v3float %float_60 %float_70 %float_50

  %m2v2float = OpTypeMatrix %v2float 2
  %m2v3float = OpTypeMatrix %v3float 2
  %m3v2float = OpTypeMatrix %v2float 3
  %m2v2float_a = OpConstantComposite %m2v2float %v2float_50_60 %v2float_60_50
  %m2v2float_b = OpConstantComposite %m2v2float %v2float_60_50 %v2float_50_60
  %m3v2float_a = OpConstantComposite %m3v2float %v2float_50_60 %v2float_60_50 %v2float_50_60
  %m2v3float_a = OpConstantComposite %m2v3float %v3float_50_60_70 %v3float_60_70_50
)";
}

// Returns the AST dump for a given SPIR-V assembly constant.
std::string AstFor(std::string assembly) {
    if (assembly == "v2uint_10_20") {
        return "vec2u(10u, 20u)";
    }
    if (assembly == "v2uint_20_10") {
        return "vec2u(20u, 10u)";
    }
    if (assembly == "v2int_30_40") {
        return "vec2i(30i, 40i)";
    }
    if (assembly == "v2int_40_30") {
        return "vec2i(40i, 30i)";
    }
    if (assembly == "cast_int_v2uint_10_20") {
        return "bitcast<vec2i>(vec2u(10u, 20u))";
    }
    if (assembly == "cast_uint_v2int_40_30") {
        return "bitcast<vec2u>(vec2i(40i, 30i))";
    }
    if (assembly == "v2float_50_60") {
        return "v2float_50_60";
    }
    if (assembly == "v2float_60_50") {
        return "v2float_60_50";
    }
    return "bad case";
}

using SpvUnaryArithTest = SpirvASTParserTestBase<::testing::Test>;

TEST_F(SpvUnaryArithTest, SNegate_Int_Int) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSNegate %int %int_30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = -(30i);"));
}

TEST_F(SpvUnaryArithTest, SNegate_Int_Uint) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSNegate %int %uint_10
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = -(bitcast<i32>(10u));"));
}

TEST_F(SpvUnaryArithTest, SNegate_Uint_Int) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSNegate %uint %int_30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<u32>(-(30i));"));
}

TEST_F(SpvUnaryArithTest, SNegate_Uint_Uint) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSNegate %uint %uint_10
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<u32>(-(bitcast<i32>(10u)));"));
}

TEST_F(SpvUnaryArithTest, SNegate_SignedVec_SignedVec) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSNegate %v2int %v2int_30_40
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = -(vec2i(30i, 40i));"));
}

TEST_F(SpvUnaryArithTest, SNegate_SignedVec_UnsignedVec) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSNegate %v2int %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = -(bitcast<vec2i>(vec2u(10u, 20u)));"));
}

TEST_F(SpvUnaryArithTest, SNegate_UnsignedVec_SignedVec) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSNegate %v2uint %v2int_30_40
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<vec2u>(-(vec2i(30i, 40i)));"));
}

TEST_F(SpvUnaryArithTest, SNegate_UnsignedVec_UnsignedVec) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSNegate %v2uint %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr(R"(let x_1 = bitcast<vec2u>(-(bitcast<vec2i>(vec2u(10u, 20u))));)"));
}

TEST_F(SpvUnaryArithTest, FNegate_Scalar) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFNegate %float %float_50
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = -(50.0f);"));
}

TEST_F(SpvUnaryArithTest, FNegate_Vector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFNegate %v2float %v2float_50_60
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_1 = -(v2float_50_60);"));
}

struct BinaryData {
    const std::string res_type;
    const std::string lhs;
    const std::string op;
    const std::string rhs;
    const std::string ast_type;
    const std::string ast_lhs;
    const std::string ast_op;
    const std::string ast_rhs;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    out << "BinaryData{" << data.res_type << "," << data.lhs << "," << data.op << "," << data.rhs
        << "," << data.ast_type << "," << data.ast_lhs << "," << data.ast_op << "," << data.ast_rhs
        << "}";
    return out;
}

using SpvBinaryArithTest = SpirvASTParserTestBase<::testing::TestWithParam<BinaryData>>;
using SpvBinaryArithTestBasic = SpirvASTParserTestBase<::testing::Test>;

TEST_P(SpvBinaryArithTest, EmitExpression) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = )" + GetParam().op +
                          " %" + GetParam().res_type + " %" + GetParam().lhs + " %" +
                          GetParam().rhs + R"(
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    StringStream ss;
    ss << "let x_1 = (" << GetParam().ast_lhs << " " << GetParam().ast_op << " "
       << GetParam().ast_rhs << ");";
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(ss.str())) << "got:\n" << got << assembly;
}

// Use this when the result might have extra bitcasts on the outside.
struct BinaryDataGeneral {
    const std::string res_type;
    const std::string lhs;
    const std::string op;
    const std::string rhs;
    const std::string wgsl_type;
    const std::string expected;
};
inline std::ostream& operator<<(std::ostream& out, BinaryDataGeneral data) {
    out << "BinaryDataGeneral{" << data.res_type << "," << data.lhs << "," << data.op << ","
        << data.rhs << "," << data.wgsl_type << "," << data.expected << "}";
    return out;
}

using SpvBinaryArithGeneralTest =
    SpirvASTParserTestBase<::testing::TestWithParam<BinaryDataGeneral>>;

TEST_P(SpvBinaryArithGeneralTest, EmitExpression) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = )" + GetParam().op +
                          " %" + GetParam().res_type + " %" + GetParam().lhs + " %" +
                          GetParam().rhs + R"(
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    StringStream ss;
    ss << "let x_1 = " << GetParam().expected << ";";
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(ss.str())) << "got:\n" << got << assembly;
}

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_IAdd,
    SpvBinaryArithTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpIAdd", "uint_20", "u32", "10u", "+", "20u"},  // Both int
        BinaryData{"int", "int_30", "OpIAdd", "int_40", "i32", "30i", "+", "40i"},  // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpIAdd", "v2uint_20_10", "vec2u",
                   AstFor("v2uint_10_20"), "+", AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpIAdd", "v2int_40_30", "vec2i", AstFor("v2int_30_40"),
                   "+", AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_IAdd_MixedSignedness,
    SpvBinaryArithGeneralTest,
    ::testing::Values(
        // Mixed, uint <- int uint
        BinaryDataGeneral{"uint", "int_30", "OpIAdd", "uint_10", "u32",
                          "bitcast<u32>((30i + bitcast<i32>(10u)))"},
        // Mixed, int <- int uint
        BinaryDataGeneral{"int", "int_30", "OpIAdd", "uint_10", "i32", "(30i + bitcast<i32>(10u))"},
        // Mixed, uint <- uint int
        BinaryDataGeneral{"uint", "uint_10", "OpIAdd", "int_30", "u32",
                          "(10u + bitcast<u32>(30i))"},
        // Mixed, int <- uint uint
        BinaryDataGeneral{"int", "uint_20", "OpIAdd", "uint_10", "i32",
                          "bitcast<i32>((20u + 10u))"},
        // Mixed, returning v2uint
        BinaryDataGeneral{"v2uint", "v2int_30_40", "OpIAdd", "v2uint_10_20", "vec2u",
                          R"(bitcast<vec2u>((vec2i(30i, 40i) + bitcast<vec2i>(vec2u(10u, 20u)))))"},
        // Mixed, returning v2int
        BinaryDataGeneral{
            "v2int", "v2uint_10_20", "OpIAdd", "v2int_40_30", "vec2i",
            R"(bitcast<vec2i>((vec2u(10u, 20u) + bitcast<vec2u>(vec2i(40i, 30i)))))"}));

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_FAdd,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Scalar float
                             BinaryData{"float", "float_50", "OpFAdd", "float_60", "f32", "50.0f",
                                        "+", "60.0f"},  // Vector float
                             BinaryData{"v2float", "v2float_50_60", "OpFAdd", "v2float_60_50",
                                        "vec2f", AstFor("v2float_50_60"), "+",
                                        AstFor("v2float_60_50")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_ISub,
    SpvBinaryArithTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpISub", "uint_20", "u32", "10u", "-", "20u"},  // Both int
        BinaryData{"int", "int_30", "OpISub", "int_40", "i32", "30i", "-", "40i"},  // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpISub", "v2uint_20_10", "vec2u",
                   AstFor("v2uint_10_20"), "-", AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpISub", "v2int_40_30", "vec2i", AstFor("v2int_30_40"),
                   "-", AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_ISub_MixedSignedness,
    SpvBinaryArithGeneralTest,
    ::testing::Values(
        // Mixed, uint <- int uint
        BinaryDataGeneral{"uint", "int_30", "OpISub", "uint_10", "u32",
                          R"(bitcast<u32>((30i - bitcast<i32>(10u))))"},
        // Mixed, int <- int uint
        BinaryDataGeneral{"int", "int_30", "OpISub", "uint_10", "i32", "(30i - bitcast<i32>(10u))"},
        // Mixed, uint <- uint int
        BinaryDataGeneral{"uint", "uint_10", "OpISub", "int_30", "u32",
                          "(10u - bitcast<u32>(30i))"},
        // Mixed, int <- uint uint
        BinaryDataGeneral{"int", "uint_20", "OpISub", "uint_10", "i32",
                          "bitcast<i32>((20u - 10u))"},
        // Mixed, returning v2uint
        BinaryDataGeneral{"v2uint", "v2int_30_40", "OpISub", "v2uint_10_20", "vec2u",
                          R"(bitcast<vec2u>((vec2i(30i, 40i) - bitcast<vec2i>(vec2u(10u, 20u)))))"},
        // Mixed, returning v2int
        BinaryDataGeneral{
            "v2int", "v2uint_10_20", "OpISub", "v2int_40_30", "vec2i",
            R"(bitcast<vec2i>((vec2u(10u, 20u) - bitcast<vec2u>(vec2i(40i, 30i)))))"}));

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_FSub,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Scalar float
                             BinaryData{"float", "float_50", "OpFSub", "float_60", "f32", "50.0f",
                                        "-", "60.0f"},  // Vector float
                             BinaryData{"v2float", "v2float_50_60", "OpFSub", "v2float_60_50",
                                        "vec2f", AstFor("v2float_50_60"), "-",
                                        AstFor("v2float_60_50")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_IMul,
    SpvBinaryArithTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpIMul", "uint_20", "u32", "10u", "*", "20u"},  // Both int
        BinaryData{"int", "int_30", "OpIMul", "int_40", "i32", "30i", "*", "40i"},  // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpIMul", "v2uint_20_10", "vec2u",
                   AstFor("v2uint_10_20"), "*", AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpIMul", "v2int_40_30", "vec2i", AstFor("v2int_30_40"),
                   "*", AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_IMul_MixedSignedness,
    SpvBinaryArithGeneralTest,
    ::testing::Values(
        // Mixed, uint <- int uint
        BinaryDataGeneral{"uint", "int_30", "OpIMul", "uint_10", "u32",
                          "bitcast<u32>((30i * bitcast<i32>(10u)))"},
        // Mixed, int <- int uint
        BinaryDataGeneral{"int", "int_30", "OpIMul", "uint_10", "i32", "(30i * bitcast<i32>(10u))"},
        // Mixed, uint <- uint int
        BinaryDataGeneral{"uint", "uint_10", "OpIMul", "int_30", "u32",
                          "(10u * bitcast<u32>(30i))"},
        // Mixed, int <- uint uint
        BinaryDataGeneral{"int", "uint_20", "OpIMul", "uint_10", "i32",
                          "bitcast<i32>((20u * 10u))"},
        // Mixed, returning v2uint
        BinaryDataGeneral{"v2uint", "v2int_30_40", "OpIMul", "v2uint_10_20", "vec2u",
                          R"(bitcast<vec2u>((vec2i(30i, 40i) * bitcast<vec2i>(vec2u(10u, 20u)))))"},
        // Mixed, returning v2int
        BinaryDataGeneral{
            "v2int", "v2uint_10_20", "OpIMul", "v2int_40_30", "vec2i",
            R"(bitcast<vec2i>((vec2u(10u, 20u) * bitcast<vec2u>(vec2i(40i, 30i)))))"}));

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_FMul,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Scalar float
                             BinaryData{"float", "float_50", "OpFMul", "float_60", "f32", "50.0f",
                                        "*", "60.0f"},  // Vector float
                             BinaryData{"v2float", "v2float_50_60", "OpFMul", "v2float_60_50",
                                        "vec2f", AstFor("v2float_50_60"), "*",
                                        AstFor("v2float_60_50")}));

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_UDiv,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Both uint
                             BinaryData{"uint", "uint_10", "OpUDiv", "uint_20", "u32", "10u", "/",
                                        "20u"},  // Both v2uint
                             BinaryData{"v2uint", "v2uint_10_20", "OpUDiv", "v2uint_20_10", "vec2u",
                                        AstFor("v2uint_10_20"), "/", AstFor("v2uint_20_10")}));

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_SDiv,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Both int
                             BinaryData{"int", "int_30", "OpSDiv", "int_40", "i32", "30i", "/",
                                        "40i"},  // Both v2int
                             BinaryData{"v2int", "v2int_30_40", "OpSDiv", "v2int_40_30", "vec2i",
                                        AstFor("v2int_30_40"), "/", AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_SDiv_MixedSignednessOperands,
    SpvBinaryArithTest,
    ::testing::Values(
        // Mixed, returning int, second arg uint
        BinaryData{"int", "int_30", "OpSDiv", "uint_10", "i32", "30i", "/", "bitcast<i32>(10u)"},
        // Mixed, returning int, first arg uint
        BinaryData{"int", "uint_10", "OpSDiv", "int_30", "i32", "bitcast<i32>(10u)", "/",
                   "30i"},  // Mixed, returning v2int, first arg v2uint
        BinaryData{"v2int", "v2uint_10_20", "OpSDiv", "v2int_30_40", "vec2i",
                   AstFor("cast_int_v2uint_10_20"), "/", AstFor("v2int_30_40")},
        // Mixed, returning v2int, second arg v2uint
        BinaryData{"v2int", "v2int_30_40", "OpSDiv", "v2uint_10_20", "vec2i", AstFor("v2int_30_40"),
                   "/", AstFor("cast_int_v2uint_10_20")}));

TEST_F(SpvBinaryArithTestBasic, SDiv_Scalar_UnsignedResult) {
    // The WGSL signed division operator expects both operands to be signed
    // and the result is signed as well.
    // In this test SPIR-V demands an unsigned result, so we have to
    // wrap the result with an as-cast.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSDiv %uint %int_30 %int_40
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<u32>((30i / 40i));"));
}

TEST_F(SpvBinaryArithTestBasic, SDiv_Vector_UnsignedResult) {
    // The WGSL signed division operator expects both operands to be signed
    // and the result is signed as well.
    // In this test SPIR-V demands an unsigned result, so we have to
    // wrap the result with an as-cast.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSDiv %v2uint %v2int_30_40 %v2int_40_30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr(R"(let x_1 = bitcast<vec2u>((vec2i(30i, 40i) / vec2i(40i, 30i)));)"));
}

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_FDiv,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Scalar float
                             BinaryData{"float", "float_50", "OpFDiv", "float_60", "f32", "50.0f",
                                        "/", "60.0f"},  // Vector float
                             BinaryData{"v2float", "v2float_50_60", "OpFDiv", "v2float_60_50",
                                        "vec2f", AstFor("v2float_50_60"), "/",
                                        AstFor("v2float_60_50")}));

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_UMod,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Both uint
                             BinaryData{"uint", "uint_10", "OpUMod", "uint_20", "u32", "10u", "%",
                                        "20u"},  // Both v2uint
                             BinaryData{"v2uint", "v2uint_10_20", "OpUMod", "v2uint_20_10", "vec2u",
                                        AstFor("v2uint_10_20"), "%", AstFor("v2uint_20_10")}));

// For non-exceptional cases SPIR-V says:
//   Sign of result of OpSRem matches the sign of the *first* operand.
//        This is like WGSL % operator.
//   Sign of result of OpSMod matches the sign of the *second* operand.
//
// But then Vulkan says behaviour is undefined if either operand is negative.
// You may as well use OpUMod.

// Test OpSMod
INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_SMod,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Both int
                             BinaryData{"int", "int_30", "OpSMod", "int_40", "i32", "30i", "%",
                                        "40i"},  // Both v2int
                             BinaryData{"v2int", "v2int_30_40", "OpSMod", "v2int_40_30", "vec2i",
                                        AstFor("v2int_30_40"), "%", AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_SMod_MixedSignednessOperands,
    SpvBinaryArithTest,
    ::testing::Values(
        // Mixed, returning int, second arg uint
        BinaryData{"int", "int_30", "OpSMod", "uint_10", "i32", "30i", "%", "bitcast<i32>(10u)"},
        // Mixed, returning int, first arg uint
        BinaryData{"int", "uint_10", "OpSMod", "int_30", "i32", "bitcast<i32>(10u)", "%",
                   "30i"},  // Mixed, returning v2int, first arg v2uint
        BinaryData{"v2int", "v2uint_10_20", "OpSMod", "v2int_30_40", "vec2i",
                   AstFor("cast_int_v2uint_10_20"), "%", AstFor("v2int_30_40")},
        // Mixed, returning v2int, second arg v2uint
        BinaryData{"v2int", "v2int_30_40", "OpSMod", "v2uint_10_20", "vec2i", AstFor("v2int_30_40"),
                   "%", AstFor("cast_int_v2uint_10_20")}));

TEST_F(SpvBinaryArithTestBasic, SMod_Scalar_UnsignedResult) {
    // The WGSL signed modulus operator expects both operands to be signed
    // and the result is signed as well.
    // In this test SPIR-V demands an unsigned result, so we have to
    // wrap the result with an as-cast.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSMod %uint %int_30 %int_40
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<u32>((30i % 40i));"));
}

TEST_F(SpvBinaryArithTestBasic, SMod_Vector_UnsignedResult) {
    // The WGSL signed modulus operator expects both operands to be signed
    // and the result is signed as well.
    // In this test SPIR-V demands an unsigned result, so we have to
    // wrap the result with an as-cast.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSMod %v2uint %v2int_30_40 %v2int_40_30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr(R"(let x_1 = bitcast<vec2u>((vec2i(30i, 40i) % vec2i(40i, 30i)));)"));
}

// Test OpSRem
INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_SRem,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Both int
                             BinaryData{"int", "int_30", "OpSRem", "int_40", "i32", "30i", "%",
                                        "40i"},  // Both v2int
                             BinaryData{"v2int", "v2int_30_40", "OpSRem", "v2int_40_30", "vec2i",
                                        AstFor("v2int_30_40"), "%", AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_SRem_MixedSignednessOperands,
    SpvBinaryArithTest,
    ::testing::Values(
        // Mixed, returning int, second arg uint
        BinaryData{"int", "int_30", "OpSRem", "uint_10", "i32", "30i", "%", "bitcast<i32>(10u)"},
        // Mixed, returning int, first arg uint
        BinaryData{"int", "uint_10", "OpSRem", "int_30", "i32", "bitcast<i32>(10u)", "%",
                   "30i"},  // Mixed, returning v2int, first arg v2uint
        BinaryData{"v2int", "v2uint_10_20", "OpSRem", "v2int_30_40", "vec2i",
                   AstFor("cast_int_v2uint_10_20"), "%", AstFor("v2int_30_40")},
        // Mixed, returning v2int, second arg v2uint
        BinaryData{"v2int", "v2int_30_40", "OpSRem", "v2uint_10_20", "vec2i", AstFor("v2int_30_40"),
                   "%", AstFor("cast_int_v2uint_10_20")}));

TEST_F(SpvBinaryArithTestBasic, SRem_Scalar_UnsignedResult) {
    // The WGSL signed modulus operator expects both operands to be signed
    // and the result is signed as well.
    // In this test SPIR-V demands an unsigned result, so we have to
    // wrap the result with an as-cast.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSRem %uint %int_30 %int_40
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = bitcast<u32>((30i % 40i));"));
}

TEST_F(SpvBinaryArithTestBasic, SRem_Vector_UnsignedResult) {
    // The WGSL signed modulus operator expects both operands to be signed
    // and the result is signed as well.
    // In this test SPIR-V demands an unsigned result, so we have to
    // wrap the result with an as-cast.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpSRem %v2uint %v2int_30_40 %v2int_40_30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr(R"(let x_1 = bitcast<vec2u>((vec2i(30i, 40i) % vec2i(40i, 30i)));)"));
}

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_FRem,
                         SpvBinaryArithTest,
                         ::testing::Values(
                             // Scalar float
                             BinaryData{"float", "float_50", "OpFRem", "float_60", "f32", "50.0f",
                                        "%", "60.0f"},  // Vector float
                             BinaryData{"v2float", "v2float_50_60", "OpFRem", "v2float_60_50",
                                        "vec2f", AstFor("v2float_50_60"), "%",
                                        AstFor("v2float_60_50")}));

TEST_F(SpvBinaryArithTestBasic, FMod_Scalar) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFMod %float %float_50 %float_60
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = (50.0f - (60.0f * floor((50.0f / 60.0f))));"));
}

TEST_F(SpvBinaryArithTestBasic, FMod_Vector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFMod %v2float %v2float_50_60 %v2float_60_50
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_1 = (v2float_50_60 - (v2float_60_50 * "
                          "floor((v2float_50_60 / v2float_60_50))));"));
}

TEST_F(SpvBinaryArithTestBasic, VectorTimesScalar) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2float %v2float_50_60
     %2 = OpCopyObject %float %float_50
     %10 = OpVectorTimesScalar %v2float %1 %2
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_10 = (x_1 * x_2);"));
}

TEST_F(SpvBinaryArithTestBasic, MatrixTimesScalar) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %m2v2float %m2v2float_a
     %2 = OpCopyObject %float %float_50
     %10 = OpMatrixTimesScalar %m2v2float %1 %2
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_10 = (x_1 * x_2);"));
}

TEST_F(SpvBinaryArithTestBasic, VectorTimesMatrix) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %m2v2float %m2v2float_a
     %2 = OpCopyObject %v2float %v2float_50_60
     %10 = OpMatrixTimesVector %v2float %1 %2
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_10 = (x_1 * x_2);"));
}

TEST_F(SpvBinaryArithTestBasic, MatrixTimesVector) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %m2v2float %m2v2float_a
     %2 = OpCopyObject %v2float %v2float_50_60
     %10 = OpMatrixTimesVector %v2float %1 %2
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_10 = (x_1 * x_2);"));
}

TEST_F(SpvBinaryArithTestBasic, MatrixTimesMatrix) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %m2v2float %m2v2float_a
     %2 = OpCopyObject %m2v2float %m2v2float_b
     %10 = OpMatrixTimesMatrix %m2v2float %1 %2
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_10 = (x_1 * x_2);"));
}

TEST_F(SpvBinaryArithTestBasic, Dot) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %v2float %v2float_50_60
     %2 = OpCopyObject %v2float %v2float_60_50
     %3 = OpDot %float %1 %2
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr("let x_3 = dot(x_1, x_2);"));
}

TEST_F(SpvBinaryArithTestBasic, OuterProduct) {
    // OpOuterProduct is expanded to basic operations.
    // The operands, even if used once, are given their own const definitions.
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpFAdd %v3float %v3float_50_60_70 %v3float_50_60_70 ; column vector
     %2 = OpFAdd %v2float %v2float_60_50 %v2float_50_60 ; row vector
     %3 = OpOuterProduct %m2v3float %1 %2
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr("let x_3 = mat2x3f("
                               "vec3f((x_2.x * x_1.x), (x_2.x * x_1.y), (x_2.x * x_1.z)), "
                               "vec3f((x_2.y * x_1.x), (x_2.y * x_1.y), (x_2.y * x_1.z)));"))
        << got;
}

struct BuiltinData {
    const std::string spirv;
    const std::string wgsl;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinData data) {
    out << "OpData{" << data.spirv << "," << data.wgsl << "}";
    return out;
}
struct ArgAndTypeData {
    const std::string spirv_type;
    const std::string spirv_arg;
    const std::string ast_type;
};
inline std::ostream& operator<<(std::ostream& out, ArgAndTypeData data) {
    out << "ArgAndTypeData{" << data.spirv_type << "," << data.spirv_arg << "," << data.ast_type
        << "}";
    return out;
}

using SpvBinaryDerivativeTest =
    SpirvASTParserTestBase<::testing::TestWithParam<std::tuple<BuiltinData, ArgAndTypeData>>>;

TEST_P(SpvBinaryDerivativeTest, Derivatives) {
    auto& builtin = std::get<0>(GetParam());
    auto& arg = std::get<1>(GetParam());

    const auto assembly = R"(
     OpCapability DerivativeControl
)" + Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %)" +
                          arg.spirv_type + " %" + arg.spirv_arg + R"(
     %2 = )" + builtin.spirv +
                          " %" + arg.spirv_type + R"( %1
     OpReturn
     OpFunctionEnd
)";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << assembly << p->error();
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    EXPECT_THAT(test::ToString(p->program(), ast_body),
                HasSubstr("let x_2 = " + builtin.wgsl + "(x_1);"));
}

INSTANTIATE_TEST_SUITE_P(
    SpvBinaryDerivativeTest,
    SpvBinaryDerivativeTest,
    testing::Combine(::testing::Values(BuiltinData{"OpDPdx", "dpdx"},
                                       BuiltinData{"OpDPdy", "dpdy"},
                                       BuiltinData{"OpFwidth", "fwidth"},
                                       BuiltinData{"OpDPdxFine", "dpdxFine"},
                                       BuiltinData{"OpDPdyFine", "dpdyFine"},
                                       BuiltinData{"OpFwidthFine", "fwidthFine"},
                                       BuiltinData{"OpDPdxCoarse", "dpdxCoarse"},
                                       BuiltinData{"OpDPdyCoarse", "dpdyCoarse"},
                                       BuiltinData{"OpFwidthCoarse", "fwidthCoarse"}),
                     ::testing::Values(ArgAndTypeData{"float", "float_50", "f32"},
                                       ArgAndTypeData{"v2float", "v2float_50_60", "vec2f"},
                                       ArgAndTypeData{"v3float", "v3float_50_60_70", "vec3f"})));

TEST_F(SpvUnaryArithTest, Transpose_2x2) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %m2v2float %m2v2float_a
     %2 = OpTranspose %m2v2float %1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    const auto* expected = "let x_2 = transpose(x_1);";
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvUnaryArithTest, Transpose_2x3) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %m2v3float %m2v3float_a
     %2 = OpTranspose %m3v2float %1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    // Note, in the AST dump mat_2_3 means 2 rows and 3 columns.
    // So the column vectors have 2 elements.
    // That is,   %m3v2float is __mat_2_3f32.
    const auto* expected = "let x_2 = transpose(x_1);";
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

TEST_F(SpvUnaryArithTest, Transpose_3x2) {
    const auto assembly = Preamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpCopyObject %m3v2float %m3v2float_a
     %2 = OpTranspose %m2v3float %1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions()) << p->error() << "\n" << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    const auto* expected = "let x_2 = transpose(x_1);";
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(expected)) << got;
}

// TODO(dneto): OpSRem. Missing from WGSL
// https://github.com/gpuweb/gpuweb/issues/702

// TODO(dneto): OpFRem. Missing from WGSL
// https://github.com/gpuweb/gpuweb/issues/702

// TODO(dneto): OpIAddCarry
// TODO(dneto): OpISubBorrow
// TODO(dneto): OpUMulExtended
// TODO(dneto): OpSMulExtended

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
