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
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;

std::string Preamble() {
    return R"(
    OpCapability Shader
    OpCapability Sampled1D
    OpCapability Image1D
    OpCapability StorageImageExtendedFormats
    OpCapability ImageQuery
    OpCapability Float16
    OpMemoryModel Logical Simple
  )";
}

std::string FragMain() {
    return R"(
    OpEntryPoint Fragment %main "main" ; assume no IO
    OpExecutionMode %main OriginUpperLeft
  )";
}

std::string MainBody() {
    return R"(
    %main = OpFunction %void None %voidfn
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd
  )";
}

std::string CommonTypes() {
    return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %bool = OpTypeBool
    %float = OpTypeFloat 32
    %half = OpTypeFloat 16
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1

    %v2int = OpTypeVector %int 2
    %v3int = OpTypeVector %int 3
    %v4int = OpTypeVector %int 4
    %v2uint = OpTypeVector %uint 2
    %v3uint = OpTypeVector %uint 3
    %v4uint = OpTypeVector %uint 4
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4
    %v2half = OpTypeVector %half 2
    %v3half = OpTypeVector %half 3
    %v4half = OpTypeVector %half 4

    %true = OpConstantTrue %bool
    %false = OpConstantFalse %bool

    %int_1 = OpConstant %int 1
    %int_m5 = OpConstant %int -5
    %int_min = OpConstant %int 0x80000000
    %int_max = OpConstant %int 0x7fffffff
    %uint_0 = OpConstant %uint 0
    %uint_max = OpConstant %uint 0xffffffff

    %float_minus_5 = OpConstant %float -5
    %float_half = OpConstant %float 0.5
    %float_ten = OpConstant %float 10
    %half_minus_5 = OpConstant %half -5
    %half_half = OpConstant %half 0.5
    %half_ten = OpConstant %half 10
  )";
}

struct ConstantCase {
    std::string spirv_type;
    std::string spirv_value;
    std::string wgsl_value;
};
inline std::ostream& operator<<(std::ostream& out, const ConstantCase& c) {
    out << "ConstantCase(" << c.spirv_type << ", " << c.spirv_value << ", " << c.wgsl_value << ")";
    return out;
}

using SpvParserConstantTest = SpirvASTParserTestBase<::testing::TestWithParam<ConstantCase>>;

TEST_P(SpvParserConstantTest, ReturnValue) {
    const auto spirv_type = GetParam().spirv_type;
    const auto spirv_value = GetParam().spirv_value;
    const auto wgsl_value = GetParam().wgsl_value;
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %fty = OpTypeFunction )" +
                          spirv_type + R"(

     %200 = OpFunction )" +
                          spirv_type + R"( None %fty
     %fentry = OpLabel
     OpReturnValue )" + spirv_value +
                          R"(
     OpFunctionEnd
     )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->Parse());
    auto fe = p->function_emitter(200);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_TRUE(p->error().empty());
    const auto got = test::ToString(p->program(), fe.ast_body());
    const auto expect = std::string("return ") + wgsl_value + std::string(";\n");

    EXPECT_EQ(got, expect);
}

INSTANTIATE_TEST_SUITE_P(Scalars,
                         SpvParserConstantTest,
                         ::testing::ValuesIn(std::vector<ConstantCase>{
                             {"%bool", "%true", "true"},
                             {"%bool", "%false", "false"},
                             {"%int", "%int_1", "1i"},
                             {"%int", "%int_m5", "-5i"},
                             {"%int", "%int_min", "i32(-2147483648)"},
                             {"%int", "%int_max", "2147483647i"},
                             {"%uint", "%uint_0", "0u"},
                             {"%uint", "%uint_max", "4294967295u"},
                             {"%float", "%float_minus_5", "-5.0f"},
                             {"%float", "%float_half", "0.5f"},
                             {"%float", "%float_ten", "10.0f"},
                             {"%half", "%half_minus_5", "-5.0h"},
                             {"%half", "%half_half", "0.5h"},
                             {"%half", "%half_ten", "10.0h"}}));

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
