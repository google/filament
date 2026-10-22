// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/parser/helper_test.h"

namespace tint::spirv::reader {
namespace {

struct BinaryCase {
    std::string spirv_type;
    std::string spirv_opcode;
    std::string ir_type;
    std::string ir;
};
std::string PrintBuiltinCase(testing::TestParamInfo<BinaryCase> bc) {
    return bc.param.spirv_opcode + "_" + bc.param.spirv_type;
}

using BinaryTest = SpirvParserTestWithParam<BinaryCase>;

TEST_P(BinaryTest, All) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %f16 = OpTypeFloat 16
        %f32 = OpTypeFloat 32
      %vec3i = OpTypeVector %i32 3
      %vec4u = OpTypeVector %u32 4
      %vec3h = OpTypeVector %f16 3
      %vec4f = OpTypeVector %f32 4
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %)" +
                  params.spirv_type + " %" + params.spirv_type + " %" + params.spirv_type + R"(
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %)" +
                  params.spirv_type + " " + R"( None %fn_type
        %lhs = OpFunctionParameter %)" +
                  params.spirv_type + " " + R"(
        %rhs = OpFunctionParameter %)" +
                  params.spirv_type + " " + R"(
  %foo_start = OpLabel
     %result = )" +
                  params.spirv_opcode + R"( %)" + params.spirv_type + " " + R"( %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
%2 = func(%3:)" + params.ir_type +
                  ", %4:" + params.ir_type + "):" + params.ir_type + R"( {
  $B2: {
    )" + params.ir +
                  R"(
    ret %5
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvParser,
    BinaryTest,
    testing::Values(
        // OpFAdd
        BinaryCase{"f16", "OpFAdd", "f16", "%5:f16 = add %3, %4"},
        BinaryCase{"f32", "OpFAdd", "f32", "%5:f32 = add %3, %4"},
        BinaryCase{"vec3h", "OpFAdd", "vec3<f16>", "%5:vec3<f16> = add %3, %4"},
        BinaryCase{"vec4f", "OpFAdd", "vec4<f32>", "%5:vec4<f32> = add %3, %4"},

        // OpFSub
        BinaryCase{"f16", "OpFSub", "f16", "%5:f16 = sub %3, %4"},
        BinaryCase{"f32", "OpFSub", "f32", "%5:f32 = sub %3, %4"},
        BinaryCase{"vec3h", "OpFSub", "vec3<f16>", "%5:vec3<f16> = sub %3, %4"},
        BinaryCase{"vec4f", "OpFSub", "vec4<f32>", "%5:vec4<f32> = sub %3, %4"},

        // OpFMul
        BinaryCase{"f16", "OpFMul", "f16", "%5:f16 = mul %3, %4"},
        BinaryCase{"f32", "OpFMul", "f32", "%5:f32 = mul %3, %4"},
        BinaryCase{"vec3h", "OpFMul", "vec3<f16>", "%5:vec3<f16> = mul %3, %4"},
        BinaryCase{"vec4f", "OpFMul", "vec4<f32>", "%5:vec4<f32> = mul %3, %4"},

        // OpFDiv
        BinaryCase{"f16", "OpFDiv", "f16", "%5:f16 = div %3, %4"},
        BinaryCase{"f32", "OpFDiv", "f32", "%5:f32 = div %3, %4"},
        BinaryCase{"vec3h", "OpFDiv", "vec3<f16>", "%5:vec3<f16> = div %3, %4"},
        BinaryCase{"vec4f", "OpFDiv", "vec4<f32>", "%5:vec4<f32> = div %3, %4"},

        // OpFRem
        BinaryCase{"f16", "OpFRem", "f16", "%5:f16 = mod %3, %4"},
        BinaryCase{"f32", "OpFRem", "f32", "%5:f32 = mod %3, %4"},
        BinaryCase{"vec3h", "OpFRem", "vec3<f16>", "%5:vec3<f16> = mod %3, %4"},
        BinaryCase{"vec4f", "OpFRem", "vec4<f32>", "%5:vec4<f32> = mod %3, %4"},

        // OpIAdd
        BinaryCase{"i32", "OpIAdd", "i32", "%5:i32 = spirv.add<i32> %3, %4"},
        BinaryCase{"u32", "OpIAdd", "u32", "%5:u32 = spirv.add<u32> %3, %4"},
        BinaryCase{"vec3i", "OpIAdd", "vec3<i32>", "%5:vec3<i32> = spirv.add<i32> %3, %4"},
        BinaryCase{"vec4u", "OpIAdd", "vec4<u32>", "%5:vec4<u32> = spirv.add<u32> %3, %4"},

        // OpISub
        BinaryCase{"i32", "OpISub", "i32", "%5:i32 = spirv.sub<i32> %3, %4"},
        BinaryCase{"u32", "OpISub", "u32", "%5:u32 = spirv.sub<u32> %3, %4"},
        BinaryCase{"vec3i", "OpISub", "vec3<i32>", "%5:vec3<i32> = spirv.sub<i32> %3, %4"},
        BinaryCase{"vec4u", "OpISub", "vec4<u32>", "%5:vec4<u32> = spirv.sub<u32> %3, %4"},

        // OpIMul
        BinaryCase{"i32", "OpIMul", "i32", "%5:i32 = spirv.mul<i32> %3, %4"},
        BinaryCase{"u32", "OpIMul", "u32", "%5:u32 = spirv.mul<u32> %3, %4"},
        BinaryCase{"vec3i", "OpIMul", "vec3<i32>", "%5:vec3<i32> = spirv.mul<i32> %3, %4"},
        BinaryCase{"vec4u", "OpIMul", "vec4<u32>", "%5:vec4<u32> = spirv.mul<u32> %3, %4"},

        // OpSDiv
        BinaryCase{"i32", "OpSDiv", "i32", "%5:i32 = spirv.s_div<i32> %3, %4"},
        BinaryCase{"u32", "OpSDiv", "u32", "%5:u32 = spirv.s_div<u32> %3, %4"},
        BinaryCase{"vec3i", "OpSDiv", "vec3<i32>", "%5:vec3<i32> = spirv.s_div<i32> %3, %4"},
        BinaryCase{"vec4u", "OpSDiv", "vec4<u32>", "%5:vec4<u32> = spirv.s_div<u32> %3, %4"},

        // OpSMod
        BinaryCase{"i32", "OpSMod", "i32", "%5:i32 = spirv.s_mod<i32> %3, %4"},
        BinaryCase{"u32", "OpSMod", "u32", "%5:u32 = spirv.s_mod<u32> %3, %4"},
        BinaryCase{"vec3i", "OpSMod", "vec3<i32>", "%5:vec3<i32> = spirv.s_mod<i32> %3, %4"},
        BinaryCase{"vec4u", "OpSMod", "vec4<u32>", "%5:vec4<u32> = spirv.s_mod<u32> %3, %4"},

        // OpSRem
        BinaryCase{"i32", "OpSRem", "i32", "%5:i32 = spirv.s_mod<i32> %3, %4"},
        BinaryCase{"u32", "OpSRem", "u32", "%5:u32 = spirv.s_mod<u32> %3, %4"},
        BinaryCase{"vec3i", "OpSRem", "vec3<i32>", "%5:vec3<i32> = spirv.s_mod<i32> %3, %4"},
        BinaryCase{"vec4u", "OpSRem", "vec4<u32>", "%5:vec4<u32> = spirv.s_mod<u32> %3, %4"},

        // OpUDiv
        BinaryCase{"u32", "OpUDiv", "u32", "%5:u32 = div %3, %4"},
        BinaryCase{"vec4u", "OpUDiv", "vec4<u32>", "%5:vec4<u32> = div %3, %4"},

        // OpUMod
        BinaryCase{"u32", "OpUMod", "u32", "%5:u32 = mod %3, %4"},
        BinaryCase{"vec4u", "OpUMod", "vec4<u32>", "%5:vec4<u32> = mod %3, %4"}),
    PrintBuiltinCase);

struct VectorMatTimesCase {
    std::string lhs_type;
    std::string rhs_type;
    std::string res_type;
    std::string spirv_opcode;
    std::string ir;
};
std::string PrintVectorMatTimesCase(testing::TestParamInfo<VectorMatTimesCase> bc) {
    return bc.param.spirv_opcode + "_" + bc.param.lhs_type + "_" + bc.param.rhs_type;
}

using VectorScalarTest = SpirvParserTestWithParam<VectorMatTimesCase>;

TEST_P(VectorScalarTest, All) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f16 = OpTypeFloat 16
        %f32 = OpTypeFloat 32
      %vec3h = OpTypeVector %f16 3
      %vec3f = OpTypeVector %f32 3
    %mat3x3f = OpTypeMatrix %vec3f 3
    %mat3x3h = OpTypeMatrix %vec3h 3
    %ep_type = OpTypeFunction %void
    %fn_type = OpTypeFunction %)" +
                  params.res_type + " %" + params.lhs_type + " %" + params.rhs_type + R"(
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd

        %foo = OpFunction %)" +
                  params.res_type + " " + R"( None %fn_type
        %lhs = OpFunctionParameter %)" +
                  params.lhs_type + " " + R"(
        %rhs = OpFunctionParameter %)" +
                  params.rhs_type + " " + R"(
  %foo_start = OpLabel
     %result = )" +
                  params.spirv_opcode + R"( %)" + params.res_type + " " + R"( %lhs %rhs
               OpReturnValue %result
               OpFunctionEnd
)",
              R"(
  $B2: {
    )" + params.ir +
                  R"(
    ret %5
  }
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvParser,
    VectorScalarTest,
    testing::Values(
        // OpVectorTimesScalar
        VectorMatTimesCase{"vec3h", "f16", "vec3h", "OpVectorTimesScalar",
                           "%5:vec3<f16> = mul %3, %4"},
        VectorMatTimesCase{"vec3f", "f32", "vec3f", "OpVectorTimesScalar",
                           "%5:vec3<f32> = mul %3, %4"},

        // OpMatrixTimesScalar
        VectorMatTimesCase{"mat3x3h", "f16", "mat3x3h", "OpMatrixTimesScalar",
                           "%5:mat3x3<f16> = mul %3, %4"},
        VectorMatTimesCase{"mat3x3f", "f32", "mat3x3f", "OpMatrixTimesScalar",
                           "%5:mat3x3<f32> = mul %3, %4"},

        // OpMatrixTimesVector
        VectorMatTimesCase{"mat3x3h", "vec3h", "vec3h", "OpMatrixTimesVector",
                           "%5:vec3<f16> = mul %3, %4"},
        VectorMatTimesCase{"mat3x3f", "vec3f", "vec3f", "OpMatrixTimesVector",
                           "%5:vec3<f32> = mul %3, %4"},

        // OpVectorTimesMatrix
        VectorMatTimesCase{"vec3h", "mat3x3h", "vec3h", "OpVectorTimesMatrix",
                           "%5:vec3<f16> = mul %3, %4"},
        VectorMatTimesCase{"vec3f", "mat3x3f", "vec3f", "OpVectorTimesMatrix",
                           "%5:vec3<f32> = mul %3, %4"},

        // OpMatrixTimesMatrix
        VectorMatTimesCase{"mat3x3h", "mat3x3h", "mat3x3h", "OpMatrixTimesMatrix",
                           "%5:mat3x3<f16> = mul %3, %4"},
        VectorMatTimesCase{"mat3x3f", "mat3x3f", "mat3x3f", "OpMatrixTimesMatrix",
                           "%5:mat3x3<f32> = mul %3, %4"}),
    PrintVectorMatTimesCase);

using BinaryMixedSignTest = SpirvParserTestWithParam<BinaryCase>;

TEST_P(BinaryMixedSignTest, Scalar_Signed_SignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
     %u32_10 = OpConstant %u32 10
     %i32_20 = OpConstant %i32 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %i32 %i32_20 %u32_10
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir +
                  R"(<i32> 20i, 10u
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Scalar_Signed_UnsignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
     %u32_10 = OpConstant %u32 10
     %i32_20 = OpConstant %i32 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %i32 %u32_10 %i32_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir +
                  R"(<i32> 10u, 20i
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Scalar_Signed_UnsignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
     %u32_10 = OpConstant %u32 10
     %u32_30 = OpConstant %u32 30
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %i32 %u32_10 %u32_30
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
                  params.ir +
                  R"(<i32> 10u, 30u
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Scalar_Unsigned_SignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
     %u32_10 = OpConstant %u32 10
     %i32_20 = OpConstant %i32 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %u32 %i32_20 %u32_10
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir +
                  R"(<u32> 20i, 10u
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Scalar_Unsigned_UnsignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
     %u32_10 = OpConstant %u32 10
     %i32_20 = OpConstant %i32 20
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %u32 %u32_10 %i32_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir +
                  R"(<u32> 10u, 20i
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Scalar_Unsigned_SignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
     %i32_10 = OpConstant %i32 10
     %i32_30 = OpConstant %i32 30
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %u32 %i32_10 %i32_30
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
                  params.ir +
                  R"(<u32> 10i, 30i
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Vector_Signed_SignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
      %v2i32 = OpTypeVector %i32 2
      %v2u32 = OpTypeVector %u32 2
     %u32_10 = OpConstant %u32 10
     %u32_20 = OpConstant %u32 20
     %i32_50 = OpConstant %i32 50
     %i32_60 = OpConstant %i32 60
  %v2u32_10_20 = OpConstantComposite %v2u32 %u32_10 %u32_20
  %v2i32_50_60 = OpConstantComposite %v2i32 %i32_50 %i32_60
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %v2i32 %v2i32_50_60 %v2u32_10_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir +
                  R"(<i32> vec2<i32>(50i, 60i), vec2<u32>(10u, 20u)
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Vector_Signed_UnsignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
      %v2i32 = OpTypeVector %i32 2
      %v2u32 = OpTypeVector %u32 2
     %u32_10 = OpConstant %u32 10
     %u32_20 = OpConstant %u32 20
     %i32_50 = OpConstant %i32 50
     %i32_60 = OpConstant %i32 60
  %v2u32_10_20 = OpConstantComposite %v2u32 %u32_10 %u32_20
  %v2i32_50_60 = OpConstantComposite %v2i32 %i32_50 %i32_60
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %v2i32 %v2u32_10_20 %v2i32_50_60
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir +
                  R"(<i32> vec2<u32>(10u, 20u), vec2<i32>(50i, 60i)
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Vector_Signed_UnsignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
      %v2i32 = OpTypeVector %i32 2
      %v2u32 = OpTypeVector %u32 2
     %u32_10 = OpConstant %u32 10
     %u32_20 = OpConstant %u32 20
  %v2u32_10_20 = OpConstantComposite %v2u32 %u32_10 %u32_20
  %v2u32_20_10 = OpConstantComposite %v2u32 %u32_20 %u32_10
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %v2i32 %v2u32_10_20 %v2u32_20_10
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.)" +
                  params.ir +
                  R"(<i32> vec2<u32>(10u, 20u), vec2<u32>(20u, 10u)
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Vector_Unsigned_SignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
      %v2i32 = OpTypeVector %i32 2
      %v2u32 = OpTypeVector %u32 2
     %u32_10 = OpConstant %u32 10
     %u32_20 = OpConstant %u32 20
     %i32_50 = OpConstant %i32 50
     %i32_60 = OpConstant %i32 60
  %v2u32_10_20 = OpConstantComposite %v2u32 %u32_10 %u32_20
  %v2i32_50_60 = OpConstantComposite %v2i32 %i32_50 %i32_60
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %v2u32 %v2i32_50_60 %v2u32_10_20
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir +
                  R"(<u32> vec2<i32>(50i, 60i), vec2<u32>(10u, 20u)
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Vector_Unsigned_UnsignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
      %v2i32 = OpTypeVector %i32 2
      %v2u32 = OpTypeVector %u32 2
     %u32_10 = OpConstant %u32 10
     %u32_20 = OpConstant %u32 20
     %i32_50 = OpConstant %i32 50
     %i32_60 = OpConstant %i32 60
  %v2u32_10_20 = OpConstantComposite %v2u32 %u32_10 %u32_20
  %v2i32_50_60 = OpConstantComposite %v2i32 %i32_50 %i32_60
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %v2u32 %v2u32_10_20 %v2i32_50_60
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir +
                  R"(<u32> vec2<u32>(10u, 20u), vec2<i32>(50i, 60i)
    ret
  }
}
)");
}

TEST_P(BinaryMixedSignTest, Vector_Unsigned_SignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
      %v2i32 = OpTypeVector %i32 2
      %v2u32 = OpTypeVector %u32 2
     %i32_50 = OpConstant %i32 50
     %i32_60 = OpConstant %i32 60
  %v2i32_50_60 = OpConstantComposite %v2i32 %i32_50 %i32_60
  %v2i32_60_50 = OpConstantComposite %v2i32 %i32_60 %i32_50
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = )" +
                  params.spirv_opcode +
                  R"( %v2u32 %v2i32_50_60 %v2i32_60_50
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.)" +
                  params.ir +
                  R"(<u32> vec2<i32>(50i, 60i), vec2<i32>(60i, 50i)
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         BinaryMixedSignTest,
                         testing::Values(BinaryCase{"", "OpIAdd", "", "add"},
                                         BinaryCase{"", "OpISub", "", "sub"},
                                         BinaryCase{"", "OpIMul", "", "mul"},
                                         BinaryCase{"", "OpSDiv", "", "s_div"},
                                         BinaryCase{"", "OpSMod", "", "s_mod"},
                                         BinaryCase{"", "OpSRem", "", "s_mod"}));

TEST_F(SpirvParserTest, FMod_Scalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %f16 = OpTypeFloat 16
      %v2f32 = OpTypeVector %f32 2
      %v2f16 = OpTypeVector %f16 2
        %one = OpConstant %f32 1
      %eight = OpConstant %f16 8
      %v2one = OpConstantComposite %v2f32 %one %one
    %v2eight = OpConstantComposite %v2f16 %eight %eight
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = OpFMod %f32 %one %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.f_mod 1.0f, 1.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FMod_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %f16 = OpTypeFloat 16
      %v2f32 = OpTypeVector %f32 2
      %v2f16 = OpTypeVector %f16 2
        %one = OpConstant %f32 1
      %eight = OpConstant %f16 8
      %v2one = OpConstantComposite %v2f32 %one %one
    %v2eight = OpConstantComposite %v2f16 %eight %eight
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               %1 = OpFMod %v2f16 %v2eight %v2eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f16> = spirv.f_mod vec2<f16>(8.0h), vec2<f16>(8.0h)
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
