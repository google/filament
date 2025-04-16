// Copyright 2025 The Dawn & Tint Authors
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

struct SpirvLogicalParam {
    std::string spv_name;
    std::string wgsl_name;
};
[[maybe_unused]] inline std::ostream& operator<<(std::ostream& out, SpirvLogicalParam c) {
    out << c.spv_name;
    return out;
}

using SpirvParser_FOrdLogicalTest = SpirvParserTestWithParam<SpirvLogicalParam>;
TEST_P(SpirvParser_FOrdLogicalTest, Scalar) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
        %one = OpConstant %f32 1
        %two = OpConstant %f32 2
     %v2bool = OpTypeVector %bool 2
    %v2float = OpTypeVector %f32 2
      %v2one = OpConstantComposite %v2float %one %one
      %v2two = OpConstantComposite %v2float %two %two
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpFOrd)" +
                  params.spv_name + R"( %bool %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = )" + params.wgsl_name +
                  R"( 1.0f, 2.0f
    ret
  }
}
)");
}

TEST_P(SpirvParser_FOrdLogicalTest, Vector) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
        %one = OpConstant %f32 1
        %two = OpConstant %f32 2
     %v2bool = OpTypeVector %bool 2
    %v2float = OpTypeVector %f32 2
      %v2one = OpConstantComposite %v2float %one %one
      %v2two = OpConstantComposite %v2float %two %two
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpFOrd)" +
                  params.spv_name + R"( %v2bool %v2one %v2two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = )" +
                  params.wgsl_name + R"( vec2<f32>(1.0f), vec2<f32>(2.0f)
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParserTest,
                         SpirvParser_FOrdLogicalTest,
                         testing::Values(SpirvLogicalParam{"Equal", "eq"},
                                         SpirvLogicalParam{"NotEqual", "neq"},
                                         SpirvLogicalParam{"GreaterThan", "gt"},
                                         SpirvLogicalParam{"GreaterThanEqual", "gte"},
                                         SpirvLogicalParam{"LessThan", "lt"},
                                         SpirvLogicalParam{"LessThanEqual", "lte"}));

using SpirvParser_FUnordLogicalTest = SpirvParserTestWithParam<SpirvLogicalParam>;
TEST_P(SpirvParser_FUnordLogicalTest, FOrd_Scalar) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
        %one = OpConstant %f32 1
        %two = OpConstant %f32 2
     %v2bool = OpTypeVector %bool 2
    %v2float = OpTypeVector %f32 2
      %v2one = OpConstantComposite %v2float %one %one
      %v2two = OpConstantComposite %v2float %two %two
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpFUnord)" +
                  params.spv_name + R"( %bool %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = )" + params.wgsl_name +
                  R"( 1.0f, 2.0f
    %3:bool = not %2
    ret
  }
}
)");
}

TEST_P(SpirvParser_FUnordLogicalTest, FOrd_Vector) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
        %one = OpConstant %f32 1
        %two = OpConstant %f32 2
     %v2bool = OpTypeVector %bool 2
    %v2float = OpTypeVector %f32 2
      %v2one = OpConstantComposite %v2float %one %one
      %v2two = OpConstantComposite %v2float %two %two
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpFUnord)" +
                  params.spv_name + R"( %v2bool %v2one %v2two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = )" +
                  params.wgsl_name + R"( vec2<f32>(1.0f), vec2<f32>(2.0f)
    %3:vec2<bool> = not %2
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(SpirvParserTest,
                         SpirvParser_FUnordLogicalTest,
                         testing::Values(SpirvLogicalParam{"Equal", "neq"},
                                         SpirvLogicalParam{"NotEqual", "eq"},
                                         SpirvLogicalParam{"GreaterThan", "lte"},
                                         SpirvLogicalParam{"GreaterThanEqual", "lt"},
                                         SpirvLogicalParam{"LessThan", "gte"},
                                         SpirvLogicalParam{"LessThanEqual", "gt"}));

using SpirvParser_IntegerTest = SpirvParserTestWithParam<SpirvLogicalParam>;
TEST_P(SpirvParser_IntegerTest, Scalar_SignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
      %v2eight = OpConstantComposite %v2uint %eight %eight
      %v2nine = OpConstantComposite %v2uint %nine %nine
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = Op)" +
                  params.spv_name + R"( %bool %one %two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
                  params.wgsl_name + R"( 1i, 2i
    ret
  }
}
)");
}

TEST_P(SpirvParser_IntegerTest, Scalar_SignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
      %v2eight = OpConstantComposite %v2uint %eight %eight
      %v2nine = OpConstantComposite %v2uint %nine %nine
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = Op)" +
                  params.spv_name + R"( %bool %one %eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
                  params.wgsl_name + R"( 1i, 8u
    ret
  }
}
)");
}

TEST_P(SpirvParser_IntegerTest, Scalar_UnsignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
      %v2eight = OpConstantComposite %v2uint %eight %eight
      %v2nine = OpConstantComposite %v2uint %nine %nine
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = Op)" +
                  params.spv_name + R"( %bool %eight %one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
                  params.wgsl_name + R"( 8u, 1i
    ret
  }
}
)");
}

TEST_P(SpirvParser_IntegerTest, Scalar_UnsignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
      %v2eight = OpConstantComposite %v2uint %eight %eight
      %v2nine = OpConstantComposite %v2uint %nine %nine
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = Op)" +
                  params.spv_name + R"( %bool %eight %nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.)" +
                  params.wgsl_name + R"( 8u, 9u
    ret
  }
}
)");
}

TEST_P(SpirvParser_IntegerTest, Vector_SignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
      %v2eight = OpConstantComposite %v2uint %eight %eight
      %v2nine = OpConstantComposite %v2uint %nine %nine
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = Op)" +
                  params.spv_name + R"( %v2bool %v2one %v2two
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
                  params.wgsl_name + R"( vec2<i32>(1i), vec2<i32>(2i)
    ret
  }
}
)");
}

TEST_P(SpirvParser_IntegerTest, Vector_SignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
      %v2eight = OpConstantComposite %v2uint %eight %eight
      %v2nine = OpConstantComposite %v2uint %nine %nine
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = Op)" +
                  params.spv_name + R"( %v2bool %v2one %v2eight
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
                  params.wgsl_name + R"( vec2<i32>(1i), vec2<u32>(8u)
    ret
  }
}
)");
}

TEST_P(SpirvParser_IntegerTest, Vector_UnsignedSigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
      %v2eight = OpConstantComposite %v2uint %eight %eight
      %v2nine = OpConstantComposite %v2uint %nine %nine
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = Op)" +
                  params.spv_name + R"( %v2bool %v2eight %v2one
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
                  params.wgsl_name + R"( vec2<u32>(8u), vec2<i32>(1i)
    ret
  }
}
)");
}

TEST_P(SpirvParser_IntegerTest, Vector_UnsignedUnsigned) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %v2bool = OpTypeVector %bool 2
      %v2int = OpTypeVector %int 2
     %v2uint = OpTypeVector %uint 2
        %one = OpConstant %int 1
        %two = OpConstant %int 2
      %eight = OpConstant %uint 8
       %nine = OpConstant %uint 9
      %v2one = OpConstantComposite %v2int %one %one
      %v2two = OpConstantComposite %v2int %two %two
      %v2eight = OpConstantComposite %v2uint %eight %eight
      %v2nine = OpConstantComposite %v2uint %nine %nine
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = Op)" +
                  params.spv_name + R"( %v2bool %v2eight %v2nine
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = spirv.)" +
                  params.wgsl_name + R"( vec2<u32>(8u), vec2<u32>(9u)
    ret
  }
}
)");
}

INSTANTIATE_TEST_SUITE_P(
    SpirvParserTest,
    SpirvParser_IntegerTest,
    testing::Values(SpirvLogicalParam{"IEqual", "equal"},
                    SpirvLogicalParam{"INotEqual", "not_equal"},
                    SpirvLogicalParam{"SGreaterThan", "s_greater_than"},
                    SpirvLogicalParam{"SGreaterThanEqual", "s_greater_than_equal"},
                    SpirvLogicalParam{"SLessThan", "s_less_than"},
                    SpirvLogicalParam{"SLessThanEqual", "s_less_than_equal"},
                    SpirvLogicalParam{"UGreaterThan", "u_greater_than"},
                    SpirvLogicalParam{"UGreaterThanEqual", "u_greater_than_equal"},
                    SpirvLogicalParam{"ULessThan", "u_less_than"},
                    SpirvLogicalParam{"ULessThanEqual", "u_less_than_equal"}));

using SpirvParser_LogicalTest = SpirvParserTestWithParam<SpirvLogicalParam>;
TEST_P(SpirvParser_LogicalTest, Scalar) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
     %v2true = OpConstantComposite %v2bool %true %true
    %v2false = OpConstantComposite %v2bool %false %false
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpLogical)" +
                  params.spv_name + R"( %bool %true %false
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = )" + params.wgsl_name +
                  R"( true, false
    ret
  }
}
)");
}

TEST_P(SpirvParser_LogicalTest, Vector) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
     %v2true = OpConstantComposite %v2bool %true %true
    %v2false = OpConstantComposite %v2bool %false %false
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpLogical)" +
                  params.spv_name + R"( %v2bool %v2true %v2false
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = )" +
                  params.wgsl_name + R"( vec2<bool>(true), vec2<bool>(false)
    ret
  }
}
)");
}
INSTANTIATE_TEST_SUITE_P(SpirvParserTest,
                         SpirvParser_LogicalTest,
                         testing::Values(SpirvLogicalParam{"And", "and"},
                                         SpirvLogicalParam{"Or", "or"},
                                         SpirvLogicalParam{"Equal", "eq"},
                                         SpirvLogicalParam{"NotEqual", "neq"}));

TEST_F(SpirvParserTest, LogicalNot_Scalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
       %true = OpConstantTrue %bool
     %v2true = OpConstantComposite %v2bool %true %true
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpLogicalNot %bool %true
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = not true
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, LogicalNot_Vector) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
       %bool = OpTypeBool
     %v2bool = OpTypeVector %bool 2
       %true = OpConstantTrue %bool
     %v2true = OpConstantComposite %v2bool %true %true
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
          %1 = OpLogicalNot %v2bool %v2true
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<bool> = not vec2<bool>(true)
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
