// Copyright 2020 The Dawn & Tint Authors  //
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

std::string CommonTypes() {
    return R"(
  %void = OpTypeVoid
  %voidfn = OpTypeFunction %void

  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %float = OpTypeFloat 32

  %uint_10 = OpConstant %uint 10
  %uint_20 = OpConstant %uint 20
  %int_10 = OpConstant %int 10
  %int_20 = OpConstant %int 20
  %int_30 = OpConstant %int 30
  %int_40 = OpConstant %int 40
  %float_50 = OpConstant %float 50
  %float_60 = OpConstant %float 60

  %ptr_uint = OpTypePointer Function %uint
  %ptr_int = OpTypePointer Function %int
  %ptr_float = OpTypePointer Function %float

  %v2uint = OpTypeVector %uint 2
  %v2int = OpTypeVector %int 2
  %v2float = OpTypeVector %float 2

  %v2uint_10_20 = OpConstantComposite %v2uint %uint_10 %uint_20
  %v2uint_20_10 = OpConstantComposite %v2uint %uint_20 %uint_10
  %v2int_30_40 = OpConstantComposite %v2int %int_30 %int_40
  %v2int_40_30 = OpConstantComposite %v2int %int_40 %int_30
  %v2float_50_60 = OpConstantComposite %v2float %float_50 %float_60
  %v2float_60_50 = OpConstantComposite %v2float %float_60 %float_50
)";
}

std::string SimplePreamble() {
    return R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint Fragment %100 "main"
  OpExecutionMode %100 OriginUpperLeft
)" + CommonTypes();
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
        return "bitcast<vec2i(vec2u(10u, 20u))";
    }
    if (assembly == "v2float_50_60") {
        return "vec2f(50.0, 60.0))";
    }
    if (assembly == "v2float_60_50") {
        return "vec2f(60.0, 50.0))";
    }
    return "bad case";
}

using SpvUnaryBitTest = SpirvASTParserTestBase<::testing::Test>;

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

using SpvBinaryBitTest = SpirvASTParserTestBase<::testing::TestWithParam<BinaryData>>;
using SpvBinaryBitTestBasic = SpirvASTParserTestBase<::testing::Test>;

TEST_P(SpvBinaryBitTest, EmitExpression) {
    const auto assembly = SimplePreamble() + R"(
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
    EXPECT_THAT(test::ToString(p->program(), ast_body), HasSubstr(ss.str())) << assembly;
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

using SpvBinaryBitGeneralTest = SpirvASTParserTestBase<::testing::TestWithParam<BinaryDataGeneral>>;

TEST_P(SpvBinaryBitGeneralTest, EmitExpression) {
    const auto assembly = SimplePreamble() + R"(
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
    EXPECT_TRUE(fe.EmitBody()) << p->error() << assembly;
    StringStream ss;
    ss << "let x_1 = " << GetParam().expected << ";\nreturn;\n";
    auto ast_body = fe.ast_body();
    auto got = test::ToString(p->program(), ast_body);
    EXPECT_THAT(got, HasSubstr(ss.str())) << "got:\n" << got << assembly;
}

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_ShiftLeftLogical_Arg2Unsigned,
    SpvBinaryBitTest,
    ::testing::Values(
        // uint uint -> uint
        BinaryData{"uint", "uint_10", "OpShiftLeftLogical", "uint_20", "u32", "10u", "<<", "20u"},
        // int, uint -> int
        BinaryData{"int", "int_30", "OpShiftLeftLogical", "uint_20", "i32", "30i", "<<", "20u"},
        // v2uint v2uint -> v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpShiftLeftLogical", "v2uint_20_10", "vec2u",
                   AstFor("v2uint_10_20"), "<<", AstFor("v2uint_20_10")},
        // v2int, v2uint -> v2int
        BinaryData{"v2int", "v2int_30_40", "OpShiftLeftLogical", "v2uint_20_10", "vec2i",
                   AstFor("v2int_30_40"), "<<", AstFor("v2uint_20_10")}));

INSTANTIATE_TEST_SUITE_P(
    // WGSL requires second operand to be unsigned, so insert bitcasts
    SpirvASTParserTest_ShiftLeftLogical_Arg2Signed,
    SpvBinaryBitGeneralTest,
    ::testing::Values(
        // int, int -> int
        BinaryDataGeneral{"int", "int_30", "OpShiftLeftLogical", "int_40", "i32",
                          "(30i << bitcast<u32>(40i))"},
        // uint, int -> uint
        BinaryDataGeneral{"uint", "uint_10", "OpShiftLeftLogical", "int_40", "u32",
                          "(10u << bitcast<u32>(40i))"},
        // v2uint, v2int -> v2uint
        BinaryDataGeneral{"v2uint", "v2uint_10_20", "OpShiftLeftLogical", "v2uint_20_10", "vec2u",
                          "(vec2u(10u, 20u) << vec2u(20u, 10u))"},
        // v2int, v2int -> v2int
        BinaryDataGeneral{"v2int", "v2int_30_40", "OpShiftLeftLogical", "v2int_40_30", "vec2i",
                          "(vec2i(30i, 40i) << bitcast<vec2u>(vec2i(40i, 30i)))"}));

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_ShiftLeftLogical_BitcastResult,
                         SpvBinaryBitGeneralTest,
                         ::testing::Values(
                             // int, int -> uint
                             BinaryDataGeneral{"uint", "int_30", "OpShiftLeftLogical", "uint_10",
                                               "u32", "bitcast<u32>((30i << 10u))"},
                             // v2uint, v2int -> v2uint
                             BinaryDataGeneral{
                                 "v2uint", "v2int_30_40", "OpShiftLeftLogical", "v2uint_20_10",
                                 "vec2u", "bitcast<vec2u>((vec2i(30i, 40i) << vec2u(20u, 10u)))"}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_ShiftRightLogical_Arg2Unsigned,
    SpvBinaryBitGeneralTest,
    ::testing::Values(
        // uint, uint -> uint
        BinaryDataGeneral{"uint", "uint_10", "OpShiftRightLogical", "uint_20", "u32",
                          "(10u >> 20u)"},
        // int, uint -> int
        BinaryDataGeneral{"int", "int_30", "OpShiftRightLogical", "uint_20", "i32",
                          "bitcast<i32>((bitcast<u32>(30i) >> 20u))"},
        // v2uint, v2uint -> v2uint
        BinaryDataGeneral{"v2uint", "v2uint_10_20", "OpShiftRightLogical", "v2uint_20_10", "vec2u",
                          "(vec2u(10u, 20u) >> vec2u(20u, 10u))"},
        // v2int, v2uint -> v2int
        BinaryDataGeneral{
            "v2int", "v2int_30_40", "OpShiftRightLogical", "v2uint_10_20", "vec2i",
            R"(bitcast<vec2i>((bitcast<vec2u>(vec2i(30i, 40i)) >> vec2u(10u, 20u))))"}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_ShiftRightLogical_Arg2Signed,
    SpvBinaryBitGeneralTest,
    ::testing::Values(
        // uint, int -> uint
        BinaryDataGeneral{"uint", "uint_10", "OpShiftRightLogical", "int_30", "u32",
                          "(10u >> bitcast<u32>(30i))"},
        // int, int -> int
        BinaryDataGeneral{"int", "int_30", "OpShiftRightLogical", "int_40", "i32",
                          "bitcast<i32>((bitcast<u32>(30i) >> bitcast<u32>(40i)))"},
        // v2uint, v2int -> v2uint
        BinaryDataGeneral{"v2uint", "v2uint_10_20", "OpShiftRightLogical", "v2int_30_40", "vec2u",
                          "(vec2u(10u, 20u) >> bitcast<vec2u>(vec2i(30i, 40i)))"},
        // v2int, v2int -> v2int
        BinaryDataGeneral{
            "v2int", "v2int_40_30", "OpShiftRightLogical", "v2int_30_40", "vec2i",
            R"(bitcast<vec2i>((bitcast<vec2u>(vec2i(40i, 30i)) >> bitcast<vec2u>(vec2i(30i, 40i)))))"}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_ShiftRightLogical_BitcastResult,
    SpvBinaryBitGeneralTest,
    ::testing::Values(
        // uint, uint -> int
        BinaryDataGeneral{"int", "uint_20", "OpShiftRightLogical", "uint_10", "i32",
                          "bitcast<i32>((20u >> 10u))"},
        // v2uint, v2uint -> v2int
        BinaryDataGeneral{"v2int", "v2uint_10_20", "OpShiftRightLogical", "v2uint_20_10", "vec2i",
                          R"(bitcast<vec2i>((vec2u(10u, 20u) >> vec2u(20u, 10u))))"}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_ShiftRightArithmetic_Arg2Unsigned,
    SpvBinaryBitGeneralTest,
    ::testing::Values(
        // uint, uint -> uint
        BinaryDataGeneral{"uint", "uint_10", "OpShiftRightArithmetic", "uint_20", "u32",
                          "bitcast<u32>((bitcast<i32>(10u) >> 20u))"},
        // int, uint -> int
        BinaryDataGeneral{"int", "int_30", "OpShiftRightArithmetic", "uint_10", "i32",
                          "(30i >> 10u)"},
        // v2uint, v2uint -> v2uint
        BinaryDataGeneral{
            "v2uint", "v2uint_10_20", "OpShiftRightArithmetic", "v2uint_20_10", "vec2u",
            R"(bitcast<vec2u>((bitcast<vec2i>(vec2u(10u, 20u)) >> vec2u(20u, 10u))))"},
        // v2int, v2uint -> v2int
        BinaryDataGeneral{"v2int", "v2int_40_30", "OpShiftRightArithmetic", "v2uint_20_10", "vec2i",
                          "(vec2i(40i, 30i) >> vec2u(20u, 10u))"}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_ShiftRightArithmetic_Arg2Signed,
    SpvBinaryBitGeneralTest,
    ::testing::Values(
        // uint, int -> uint
        BinaryDataGeneral{"uint", "uint_10", "OpShiftRightArithmetic", "int_30", "u32",
                          "bitcast<u32>((bitcast<i32>(10u) >> bitcast<u32>(30i)))"},
        // int, int -> int
        BinaryDataGeneral{"int", "int_30", "OpShiftRightArithmetic", "int_40", "i32",
                          "(30i >> bitcast<u32>(40i))"},
        // v2uint, v2int -> v2uint
        BinaryDataGeneral{
            "v2uint", "v2uint_10_20", "OpShiftRightArithmetic", "v2int_30_40", "vec2u",
            R"(bitcast<vec2u>((bitcast<vec2i>(vec2u(10u, 20u)) >> bitcast<vec2u>(vec2i(30i, 40i)))))"},
        // v2int, v2int -> v2int
        BinaryDataGeneral{"v2int", "v2int_40_30", "OpShiftRightArithmetic", "v2int_30_40", "vec2i",
                          "(vec2i(40i, 30i) >> bitcast<vec2u>(vec2i(30i, 40i)))"}));

INSTANTIATE_TEST_SUITE_P(SpirvASTParserTest_ShiftRightArithmetic_BitcastResult,
                         SpvBinaryBitGeneralTest,
                         ::testing::Values(
                             // int, uint -> uint
                             BinaryDataGeneral{"uint", "int_30", "OpShiftRightArithmetic",
                                               "uint_10", "u32", "bitcast<u32>((30i >> 10u))"},
                             // v2int, v2uint -> v2uint
                             BinaryDataGeneral{
                                 "v2uint", "v2int_30_40", "OpShiftRightArithmetic", "v2uint_20_10",
                                 "vec2u", "bitcast<vec2u>((vec2i(30i, 40i) >> vec2u(20u, 10u)))"}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_BitwiseAnd,
    SpvBinaryBitTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpBitwiseAnd", "uint_20", "u32", "10u", "&", "20u"},
        // Both int
        BinaryData{"int", "int_30", "OpBitwiseAnd", "int_40", "i32", "30i", "&", "40i"},
        // TODO(crbug.com/tint/678): Resolver fails on vector bitwise operations
        // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpBitwiseAnd", "v2uint_20_10", "vec2u",
                   AstFor("v2uint_10_20"), "&", AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpBitwiseAnd", "v2int_40_30", "vec2i",
                   AstFor("v2int_30_40"), "&", AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_BitwiseAnd_MixedSignedness,
    SpvBinaryBitGeneralTest,
    ::testing::Values(
        // Mixed, uint <- int uint
        BinaryDataGeneral{"uint", "int_30", "OpBitwiseAnd", "uint_10", "u32",
                          "bitcast<u32>((30i & bitcast<i32>(10u)))"},
        // Mixed, int <- int uint
        BinaryDataGeneral{"int", "int_30", "OpBitwiseAnd", "uint_10", "i32",
                          "(30i & bitcast<i32>(10u))"},
        // Mixed, uint <- uint int
        BinaryDataGeneral{"uint", "uint_10", "OpBitwiseAnd", "int_30", "u32",
                          "(10u & bitcast<u32>(30i))"},
        // Mixed, int <- uint uint
        BinaryDataGeneral{"int", "uint_20", "OpBitwiseAnd", "uint_10", "i32",
                          "bitcast<i32>((20u & 10u))"},
        // Mixed, returning v2uint
        BinaryDataGeneral{"v2uint", "v2int_30_40", "OpBitwiseAnd", "v2uint_10_20", "vec2u",
                          R"(bitcast<vec2u>((vec2i(30i, 40i) & bitcast<vec2i>(vec2u(10u, 20u)))))"},
        // Mixed, returning v2int
        BinaryDataGeneral{
            "v2int", "v2uint_10_20", "OpBitwiseAnd", "v2int_40_30", "vec2i",
            R"(bitcast<vec2i>((vec2u(10u, 20u) & bitcast<vec2u>(vec2i(40i, 30i)))))"}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_BitwiseOr,
    SpvBinaryBitTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpBitwiseOr", "uint_20", "u32", "10u", "|", "20u"},
        // Both int
        BinaryData{"int", "int_30", "OpBitwiseOr", "int_40", "i32", "30i", "|", "40i"},
        // TODO(crbug.com/tint/678): Resolver fails on vector bitwise operations
        // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpBitwiseOr", "v2uint_20_10", "vec2u",
                   AstFor("v2uint_10_20"), "|", AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpBitwiseOr", "v2int_40_30", "vec2i",
                   AstFor("v2int_30_40"), "|", AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_BitwiseOr_MixedSignedness,
    SpvBinaryBitGeneralTest,
    ::testing::Values(
        // Mixed, uint <- int uint
        BinaryDataGeneral{"uint", "int_30", "OpBitwiseOr", "uint_10", "u32",
                          "bitcast<u32>((30i | bitcast<i32>(10u)))"},
        // Mixed, int <- int uint
        BinaryDataGeneral{"int", "int_30", "OpBitwiseOr", "uint_10", "i32",
                          "(30i | bitcast<i32>(10u))"},
        // Mixed, uint <- uint int
        BinaryDataGeneral{"uint", "uint_10", "OpBitwiseOr", "int_30", "u32",
                          "(10u | bitcast<u32>(30i))"},
        // Mixed, int <- uint uint
        BinaryDataGeneral{"int", "uint_20", "OpBitwiseOr", "uint_10", "i32",
                          "bitcast<i32>((20u | 10u))"},
        // Mixed, returning v2uint
        BinaryDataGeneral{"v2uint", "v2int_30_40", "OpBitwiseOr", "v2uint_10_20", "vec2u",
                          R"(bitcast<vec2u>((vec2i(30i, 40i) | bitcast<vec2i>(vec2u(10u, 20u)))))"},
        // Mixed, returning v2int
        BinaryDataGeneral{
            "v2int", "v2uint_10_20", "OpBitwiseOr", "v2int_40_30", "vec2i",
            R"(bitcast<vec2i>((vec2u(10u, 20u) | bitcast<vec2u>(vec2i(40i, 30i)))))"}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_BitwiseXor,
    SpvBinaryBitTest,
    ::testing::Values(
        // Both uint
        BinaryData{"uint", "uint_10", "OpBitwiseXor", "uint_20", "u32", "10u", "^", "20u"},
        // Both int
        BinaryData{"int", "int_30", "OpBitwiseXor", "int_40", "i32", "30i", "^", "40i"},
        // TODO(crbug.com/tint/678): Resolver fails on vector bitwise operations
        // Both v2uint
        BinaryData{"v2uint", "v2uint_10_20", "OpBitwiseXor", "v2uint_20_10", "vec2u",
                   AstFor("v2uint_10_20"), "^", AstFor("v2uint_20_10")},
        // Both v2int
        BinaryData{"v2int", "v2int_30_40", "OpBitwiseXor", "v2int_40_30", "vec2i",
                   AstFor("v2int_30_40"), "^", AstFor("v2int_40_30")}));

INSTANTIATE_TEST_SUITE_P(
    SpirvASTParserTest_BitwiseXor_MixedSignedness,
    SpvBinaryBitGeneralTest,
    ::testing::Values(
        // Mixed, uint <- int uint
        BinaryDataGeneral{"uint", "int_30", "OpBitwiseXor", "uint_10", "u32",
                          "bitcast<u32>((30i ^ bitcast<i32>(10u)))"},
        // Mixed, int <- int uint
        BinaryDataGeneral{"int", "int_30", "OpBitwiseXor", "uint_10", "i32",
                          "(30i ^ bitcast<i32>(10u))"},
        // Mixed, uint <- uint int
        BinaryDataGeneral{"uint", "uint_10", "OpBitwiseXor", "int_30", "u32",
                          "(10u ^ bitcast<u32>(30i))"},
        // Mixed, int <- uint uint
        BinaryDataGeneral{"int", "uint_20", "OpBitwiseXor", "uint_10", "i32",
                          "bitcast<i32>((20u ^ 10u))"},
        // Mixed, returning v2uint
        BinaryDataGeneral{"v2uint", "v2int_30_40", "OpBitwiseXor", "v2uint_10_20", "vec2u",
                          R"(bitcast<vec2u>((vec2i(30i, 40i) ^ bitcast<vec2i>(vec2u(10u, 20u)))))"},
        // Mixed, returning v2int
        BinaryDataGeneral{
            "v2int", "v2uint_10_20", "OpBitwiseXor", "v2int_40_30", "vec2i",
            R"(bitcast<vec2i>((vec2u(10u, 20u) ^ bitcast<vec2u>(vec2i(40i, 30i)))))"}));

TEST_F(SpvUnaryBitTest, Not_Int_Int) {
    const auto assembly = SimplePreamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %int %int_30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = ~(30i);"));
}

TEST_F(SpvUnaryBitTest, Not_Int_Uint) {
    const auto assembly = SimplePreamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %int %uint_10
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<i32>(~(10u));"));
}

TEST_F(SpvUnaryBitTest, Not_Uint_Int) {
    const auto assembly = SimplePreamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %uint %int_30
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<u32>(~(30i));"));
}

TEST_F(SpvUnaryBitTest, Not_Uint_Uint) {
    const auto assembly = SimplePreamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %uint %uint_10
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = ~(10u);"));
}

TEST_F(SpvUnaryBitTest, Not_SignedVec_SignedVec) {
    const auto assembly = SimplePreamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %v2int %v2int_30_40
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = ~(vec2i(30i, 40i));"));
}

TEST_F(SpvUnaryBitTest, Not_SignedVec_UnsignedVec) {
    const auto assembly = SimplePreamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %v2int %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<vec2i>(~(vec2u(10u, 20u)));"));
}

TEST_F(SpvUnaryBitTest, Not_UnsignedVec_SignedVec) {
    const auto assembly = SimplePreamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %v2uint %v2int_30_40
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<vec2u>(~(vec2i(30i, 40i)));"));
}
TEST_F(SpvUnaryBitTest, Not_UnsignedVec_UnsignedVec) {
    const auto assembly = SimplePreamble() + R"(
     %100 = OpFunction %void None %voidfn
     %entry = OpLabel
     %1 = OpNot %v2uint %v2uint_10_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = ~(vec2u(10u, 20u));"));
}

std::string BitTestPreamble() {
    return R"(
  OpCapability Shader
  %glsl = OpExtInstImport "GLSL.std.450"
  OpMemoryModel Logical GLSL450
  OpEntryPoint GLCompute %100 "main"
  OpExecutionMode %100 LocalSize 1 1 1

  OpName %u1 "u1"
  OpName %i1 "i1"
  OpName %v2u1 "v2u1"
  OpName %v2i1 "v2i1"

)" + CommonTypes() +
           R"(

  %100 = OpFunction %void None %voidfn
  %entry = OpLabel

  %u1 = OpCopyObject %uint %uint_10
  %i1 = OpCopyObject %int %int_30
  %v2u1 = OpCopyObject %v2uint %v2uint_10_20
  %v2i1 = OpCopyObject %v2int %v2int_30_40
)";
}

TEST_F(SpvUnaryBitTest, BitCount_Uint_Uint) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitCount %uint %u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = countOneBits(u1);")) << body;
}

TEST_F(SpvUnaryBitTest, BitCount_Uint_Int) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitCount %uint %i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<u32>(countOneBits(i1));")) << body;
}

TEST_F(SpvUnaryBitTest, BitCount_Int_Uint) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitCount %int %u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<i32>(countOneBits(u1));")) << body;
}

TEST_F(SpvUnaryBitTest, BitCount_Int_Int) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitCount %int %i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = countOneBits(i1);")) << body;
}

TEST_F(SpvUnaryBitTest, BitCount_UintVector_UintVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitCount %v2uint %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = countOneBits(v2u1);")) << body;
}

TEST_F(SpvUnaryBitTest, BitCount_UintVector_IntVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitCount %v2uint %v2i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<vec2u>(countOneBits(v2i1));")) << body;
}

TEST_F(SpvUnaryBitTest, BitCount_IntVector_UintVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitCount %v2int %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = bitcast<vec2i>(countOneBits(v2u1));")) << body;
}

TEST_F(SpvUnaryBitTest, BitCount_IntVector_IntVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitCount %v2int %v2i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = countOneBits(v2i1);")) << body;
}

TEST_F(SpvUnaryBitTest, BitReverse_Uint_Uint) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitReverse %uint %u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = reverseBits(u1);")) << body;
}

TEST_F(SpvUnaryBitTest, BitReverse_Uint_Int) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitReverse %uint %i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("Expected Base Type to be equal to Result Type: BitReverse"));
}

TEST_F(SpvUnaryBitTest, BitReverse_Int_Uint) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitReverse %int %u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("Expected Base Type to be equal to Result Type: BitReverse"));
}

TEST_F(SpvUnaryBitTest, BitReverse_Int_Int) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitReverse %int %i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = reverseBits(i1);")) << body;
}

TEST_F(SpvUnaryBitTest, BitReverse_UintVector_UintVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitReverse %v2uint %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = reverseBits(v2u1);")) << body;
}

TEST_F(SpvUnaryBitTest, BitReverse_UintVector_IntVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitReverse %v2uint %v2i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("Expected Base Type to be equal to Result Type: BitReverse"));
}

TEST_F(SpvUnaryBitTest, BitReverse_IntVector_UintVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitReverse %v2int %v2u1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->Parse());
    EXPECT_FALSE(p->success());
    EXPECT_THAT(p->error(), HasSubstr("Expected Base Type to be equal to Result Type: BitReverse"));
}

TEST_F(SpvUnaryBitTest, BitReverse_IntVector_IntVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitReverse %v2int %v2i1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = reverseBits(v2i1);")) << body;
}

TEST_F(SpvUnaryBitTest, InsertBits_Int) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldInsert %int %int_30 %int_40 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = insertBits(30i, 40i, 10u, 20u);")) << body;
}

TEST_F(SpvUnaryBitTest, InsertBits_Int_SignedOffsetAndCount) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldInsert %int %int_30 %int_40 %int_10 %int_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = insertBits(30i, 40i, u32(10i), u32(20i));")) << body;
}

TEST_F(SpvUnaryBitTest, InsertBits_IntVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldInsert %v2int %v2int_30_40 %v2int_40_30 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr(R"(let x_1 = insertBits(x_28, vec2i(40i, 30i), 10u, 20u);)"))
        << body;
}

TEST_F(SpvUnaryBitTest, InsertBits_IntVector_SignedOffsetAndCount) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldInsert %v2int %v2int_30_40 %v2int_40_30 %int_10 %int_20
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
                HasSubstr(R"(let x_1 = insertBits(x_28, vec2i(40i, 30i), u32(10i), u32(20i));)"))
        << body;
}

TEST_F(SpvUnaryBitTest, InsertBits_Uint) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldInsert %uint %uint_20 %uint_10 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = insertBits(20u, 10u, 10u, 20u);")) << body;
}

TEST_F(SpvUnaryBitTest, InsertBits_Uint_SignedOffsetAndCount) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldInsert %uint %uint_20 %uint_10 %int_10 %int_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = insertBits(20u, 10u, u32(10i), u32(20i));")) << body;
}

TEST_F(SpvUnaryBitTest, InsertBits_UintVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldInsert %v2uint %v2uint_10_20 %v2uint_20_10 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr(R"(let x_1 = insertBits(x_26, vec2u(20u, 10u), 10u, 20u);)"))
        << body;
}

TEST_F(SpvUnaryBitTest, InsertBits_UintVector_SignedOffsetAndCount) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldInsert %v2uint %v2uint_10_20 %v2uint_20_10 %int_10 %int_20
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
                HasSubstr(R"(let x_1 = insertBits(x_26, vec2u(20u, 10u), u32(10i), u32(20i));)"))
        << body;
}

TEST_F(SpvUnaryBitTest, ExtractBits_Int) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldSExtract %int %int_30 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = extractBits(30i, 10u, 20u);")) << body;
}

TEST_F(SpvUnaryBitTest, ExtractBits_Int_SignedOffsetAndCount) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldSExtract %int %int_30 %int_10 %int_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = extractBits(30i, u32(10i), u32(20i));")) << body;
}

TEST_F(SpvUnaryBitTest, ExtractBits_IntVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldSExtract %v2int %v2int_30_40 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = extractBits(x_28, 10u, 20u);")) << body;
}

TEST_F(SpvUnaryBitTest, ExtractBits_IntVector_SignedOffsetAndCount) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldSExtract %v2int %v2int_30_40 %int_10 %int_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = extractBits(x_28, u32(10i), u32(20i));")) << body;
}

TEST_F(SpvUnaryBitTest, ExtractBits_Uint) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldUExtract %uint %uint_20 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = extractBits(20u, 10u, 20u);")) << body;
}

TEST_F(SpvUnaryBitTest, ExtractBits_Uint_SignedOffsetAndCount) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldUExtract %uint %uint_20 %int_10 %int_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = extractBits(20u, u32(10i), u32(20i));")) << body;
}

TEST_F(SpvUnaryBitTest, ExtractBits_UintVector) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldUExtract %v2uint %v2uint_10_20 %uint_10 %uint_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = extractBits(x_26, 10u, 20u);")) << body;
}

TEST_F(SpvUnaryBitTest, ExtractBits_UintVector_SignedOffsetAndCount) {
    const auto assembly = BitTestPreamble() + R"(
     %1 = OpBitFieldUExtract %v2uint %v2uint_10_20 %int_10 %int_20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModuleExceptFunctions());
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    auto ast_body = fe.ast_body();
    auto body = test::ToString(p->program(), ast_body);
    EXPECT_THAT(body, HasSubstr("let x_1 = extractBits(x_26, u32(10i), u32(20i));")) << body;
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
