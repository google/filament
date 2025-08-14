// Copyright 2023 The Dawn & Tint Authors
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

TEST_F(SpirvParserTest, FunctionVar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %u32_ptr Function
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionVar_RelaxedPrecision) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var RelaxedPrecision
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %u32_ptr Function
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, FunctionVar_Initializer) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
     %u32_42 = OpConstant %u32 42
    %u32_ptr = OpTypePointer Function %u32
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
        %var = OpVariable %u32_ptr Function %u32_42
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var 42u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, PrivateVar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %u32_ptr = OpTypePointer Private %u32
    %ep_type = OpTypeFunction %void
        %var = OpVariable %u32_ptr Private
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, PrivateVar_Initializer) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
     %u32_42 = OpConstant %u32 42
    %u32_ptr = OpTypePointer Private %u32
    %ep_type = OpTypeFunction %void
        %var = OpVariable %u32_ptr Private %u32_42
       %main = OpFunction %void None %ep_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<private, u32, read_write> = var 42u
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StorageVar_ReadOnly) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 NonWritable
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_StorageBuffer_str = OpTypePointer StorageBuffer %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_StorageBuffer_str StorageBuffer
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StorageVar_ReadWrite) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_StorageBuffer_str = OpTypePointer StorageBuffer %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_StorageBuffer_str StorageBuffer
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StorageVar_Coherent) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
               OpDecorate %6 Coherent
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
        %str = OpTypeStruct %uint
%_ptr_StorageBuffer_str = OpTypePointer StorageBuffer %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_StorageBuffer_str StorageBuffer
          %1 = OpFunction %void None %5
          %7 = OpLabel
          %8 = OpAccessChain %_ptr_StorageBuffer_uint %6 %uint_0
          %9 = OpLoad %uint %8
               OpStore %8 %9
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, u32, read_write> = access %1, 0u
    %4:u32 = load %3
    store %3, %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, StorageVar_ReadOnly_And_ReadWrite) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 NonWritable
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
               OpDecorate %7 DescriptorSet 1
               OpDecorate %7 Binding 3
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_StorageBuffer_str = OpTypePointer StorageBuffer %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_StorageBuffer_str StorageBuffer
          %7 = OpVariable %_ptr_StorageBuffer_str StorageBuffer
          %1 = OpFunction %void None %5
          %8 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read> = var undef @binding_point(1, 2)
  %2:ptr<storage, tint_symbol_1, read_write> = var undef @binding_point(1, 3)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, UniformVar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 NonWritable
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_Uniform_str = OpTypePointer Uniform %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_Uniform_str Uniform
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<uniform, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, UniformVar_BufferBlock) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
               OpDecorate %str BufferBlock
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_Uniform_str = OpTypePointer Uniform %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_Uniform_str Uniform
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, UniformVar_BufferBlock_NonReadable) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 NonReadable
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
               OpDecorate %str BufferBlock
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_Uniform_str = OpTypePointer Uniform %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_Uniform_str Uniform
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, write> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, UniformVar_BufferBlock_Propagate) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 NonReadable
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
               OpDecorate %str BufferBlock
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
        %str = OpTypeStruct %uint
%_ptr_Uniform_str = OpTypePointer Uniform %str
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_Uniform_str Uniform
          %1 = OpFunction %void None %5
          %7 = OpLabel
          %8 = OpCopyObject %_ptr_Uniform_str %6
          %9 = OpAccessChain %_ptr_Uniform_uint %8 %uint_0
         %10 = OpCopyObject %_ptr_Uniform_uint %9
               OpStore %10 %uint_0
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, write> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<storage, tint_symbol_1, write> = let %1
    %4:ptr<storage, u32, write> = access %3, 0u
    %5:ptr<storage, u32, write> = let %4
    store %5, 0u
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, UniformVar_BufferBlock_NonWritable) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpMemberDecorate %str 0 Offset 0
               OpDecorate %6 NonWritable
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
               OpDecorate %str BufferBlock
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %str = OpTypeStruct %uint
%_ptr_Uniform_str = OpTypePointer Uniform %str
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_Uniform_str Uniform
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_1 = struct @align(4) {
  tint_symbol:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, UniformConstantVar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %6 DescriptorSet 1
               OpDecorate %6 Binding 2
       %void = OpTypeVoid
       %samp = OpTypeSampler
%_ptr_Uniform_samp = OpTypePointer UniformConstant %samp
          %5 = OpTypeFunction %void
          %6 = OpVariable %_ptr_Uniform_samp UniformConstant
          %1 = OpFunction %void None %5
          %7 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<handle, sampler, read> = var undef @binding_point(1, 2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

struct BuiltinCase {
    std::string spirv_type;
    std::string spirv_builtin;
    std::string ir;
};
std::string PrintBuiltinCase(testing::TestParamInfo<BuiltinCase> bc) {
    return bc.param.spirv_builtin;
}

using BuiltinInputTest = SpirvParserTestWithParam<BuiltinCase>;

TEST_P(BuiltinInputTest, Enum) {
    auto params = GetParam();
    EXPECT_IR_SPV(R"(
               OpCapability GroupNonUniform
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var BuiltIn )" +
                      params.spirv_builtin + R"(
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
      %vec3u = OpTypeVector %u32 3
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
      %u32_1 = OpConstant %u32 1
  %arr_u32_1 = OpTypeArray %u32 %u32_1
    %fn_type = OpTypeFunction %void

 %_ptr_Input = OpTypePointer Input %)" +
                      params.spirv_type + R"(
        %var = OpVariable %_ptr_Input Input

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
                  params.ir, SPV_ENV_VULKAN_1_1);
}

INSTANTIATE_TEST_SUITE_P(
    SpirvParser,
    BuiltinInputTest,
    testing::Values(
        BuiltinCase{
            "vec4f",
            "FragCoord",
            "%1:ptr<__in, vec4<f32>, read> = var undef @builtin(position)",
        },
        BuiltinCase{
            "bool",
            "FrontFacing",
            "%1:ptr<__in, bool, read> = var undef @builtin(front_facing)",
        },
        BuiltinCase{
            "vec3u",
            "GlobalInvocationId",
            "%1:ptr<__in, vec3<u32>, read> = var undef @builtin(global_invocation_id)",
        },
        BuiltinCase{
            "u32",
            "InstanceIndex",
            "%1:ptr<__in, u32, read> = var undef @builtin(instance_index)",
        },
        BuiltinCase{
            "vec3u",
            "LocalInvocationId",
            "%1:ptr<__in, vec3<u32>, read> = var undef @builtin(local_invocation_id)",
        },
        BuiltinCase{
            "u32",
            "LocalInvocationIndex",
            "%1:ptr<__in, u32, read> = var undef @builtin(local_invocation_index)",
        },
        BuiltinCase{
            "vec3u",
            "NumWorkgroups",
            "%1:ptr<__in, vec3<u32>, read> = var undef @builtin(num_workgroups)",
        },
        BuiltinCase{
            "u32",
            "SampleId",
            "%1:ptr<__in, u32, read> = var undef @builtin(sample_index)",
        },
        BuiltinCase{
            "arr_u32_1",
            "SampleMask",
            "%1:ptr<__in, array<u32, 1>, read> = var undef @builtin(sample_mask)",
        },
        BuiltinCase{
            "u32",
            "SubgroupId",
            "%1:ptr<__in, u32, read> = var undef @builtin(subgroup_id)",
        },
        BuiltinCase{
            "u32",
            "SubgroupSize",
            "%1:ptr<__in, u32, read> = var undef @builtin(subgroup_size)",
        },
        BuiltinCase{
            "u32",
            "SubgroupLocalInvocationId",
            "%1:ptr<__in, u32, read> = var undef @builtin(subgroup_invocation_id)",
        },
        BuiltinCase{
            "u32",
            "VertexIndex",
            "%1:ptr<__in, u32, read> = var undef @builtin(vertex_index)",
        },
        BuiltinCase{
            "vec3u",
            "WorkgroupId",
            "%1:ptr<__in, vec3<u32>, read> = var undef @builtin(workgroup_id)",
        }),
    PrintBuiltinCase);

using BuiltinOutputTest = SpirvParserTestWithParam<BuiltinCase>;

TEST_P(BuiltinOutputTest, Enum) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var BuiltIn )" +
                  params.spirv_builtin + R"(
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
      %u32_1 = OpConstant %u32 1
  %arr_u32_1 = OpTypeArray %u32 %u32_1
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %)" +
                  params.spirv_type + R"(
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              params.ir);
}

INSTANTIATE_TEST_SUITE_P(
    SpirvParser,
    BuiltinOutputTest,
    testing::Values(
        BuiltinCase{
            "f32",
            "PointSize",
            "%1:ptr<__out, f32, read_write> = var undef @builtin(__point_size)",
        },
        BuiltinCase{
            "vec4f",
            "Position",
            "%1:ptr<__out, vec4<f32>, read_write> = var undef @builtin(position)",
        },
        BuiltinCase{
            "arr_u32_1",
            "SampleMask",
            "%1:ptr<__out, array<u32, 1>, read_write> = var undef @builtin(sample_mask)",
        }),
    PrintBuiltinCase);

TEST_F(SpirvParserTest, Invariant_OnVariable) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var BuiltIn Position
               OpDecorate %var Invariant
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %vec4f
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              "%1:ptr<__out, vec4<f32>, read_write> = var undef @invariant @builtin(position)");
}

struct LocationCase {
    std::string spirv_decorations;
    std::string ir;
};

using LocationVarTest = SpirvParserTestWithParam<LocationCase>;

TEST_P(LocationVarTest, Input) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          )" + params.spirv_decorations +
                  R"(
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void

%_ptr_Input = OpTypePointer Input %vec4f
        %var = OpVariable %_ptr_Input Input

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              "%1:ptr<__in, vec4<f32>, read> = " + params.ir);
}

TEST_P(LocationVarTest, Output) {
    auto params = GetParam();
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          )" + params.spirv_decorations +
                  R"(
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %vec4f
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              "%1:ptr<__out, vec4<f32>, read_write> = " + params.ir);
}

INSTANTIATE_TEST_SUITE_P(SpirvParser,
                         LocationVarTest,
                         testing::Values(
                             LocationCase{
                                 "OpDecorate %var Location 1 ",
                                 "var undef @location(1)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 2 "
                                 "OpDecorate %var NoPerspective ",
                                 "var undef @location(2) @interpolate(linear)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 3 "
                                 "OpDecorate %var Flat ",
                                 "var undef @location(3) @interpolate(flat)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 4 "
                                 "OpDecorate %var Centroid ",
                                 "var undef @location(4) @interpolate(perspective, centroid)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 5 "
                                 "OpDecorate %var Sample ",
                                 "var undef @location(5) @interpolate(perspective, sample)",
                             },
                             LocationCase{
                                 "OpDecorate %var Location 6 "
                                 "OpDecorate %var NoPerspective "
                                 "OpDecorate %var Centroid ",
                                 "var undef @location(6) @interpolate(linear, centroid)",
                             }));

TEST_F(SpirvParserTest, Vertex_Output_I32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var Location 0
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %i32
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<__out, i32, read_write> = var undef @location(0) @interpolate(flat)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Vertex_Output_U32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var Location 0
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %u32
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<__out, u32, read_write> = var undef @location(0) @interpolate(flat)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Vertex_Output_Vec3_I32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var Location 0
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
      %v3i32 = OpTypeVector %i32 3
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %v3i32
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<__out, vec3<i32>, read_write> = var undef @location(0) @interpolate(flat)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Vertex_Output_F32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpDecorate %var Location 0
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
    %fn_type = OpTypeFunction %void

%_ptr_Output = OpTypePointer Output %f32
        %var = OpVariable %_ptr_Output Output

       %main = OpFunction %void None %fn_type
 %main_start = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<__out, f32, read_write> = var undef @location(0)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantTrue) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
          %c = OpSpecConstantTrue %bool
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:bool = override true @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = and %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstant_f16) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
        %f16 = OpTypeFloat 16
          %c = OpSpecConstant %f16 2
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpFAdd %f16 %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:f16 = override 2.0h @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:f16 = add %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstant_f32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
          %c = OpSpecConstant %f32 2
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpFAdd %f32 %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:f32 = override 2.0f @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:f32 = add %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstant_u32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
          %c = OpSpecConstant %u32 2
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpIAdd %u32 %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:u32 = override 2u @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:u32 = spirv.add<u32> %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstant_i32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
          %c = OpSpecConstant %i32 2
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpIAdd %i32 %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:i32 = override 2i @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:i32 = spirv.add<i32> %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantTrue_NoSpecId) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
          %c = OpSpecConstantTrue %bool
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = and true, true
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantFalse) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
               OpDecorate %c SpecId 12
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
          %c = OpSpecConstantFalse %bool
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %myconst:bool = override false @id(12)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:bool = and %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantFalse_NoSpecId) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %c "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %f32 = OpTypeFloat 32
      %vec4f = OpTypeVector %f32 4
          %c = OpSpecConstantFalse %bool
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %c %c
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = and false, false
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalAnd) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalAnd %false %true
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalAnd %bool %cond %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = and false, true
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = and %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalOr) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalOr %false %true
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalOr %bool %cond %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = or false, true
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = or %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalNot) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalNot %false
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %b = OpLogicalNot %bool %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = not false
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = not %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalEqual %true %false
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpLogicalEqual %bool %cond %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = eq true, false
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = eq %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_LogicalNotEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
       %cond = OpSpecConstantOp %bool LogicalNotEqual %true %false
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpLogicalNotEqual %bool %cond %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = neq true, false
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = neq %myconst, %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_Not_ResultI32_i32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %cond "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %i32 = OpTypeInt 32 1
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
        %one = OpConstant %i32 1
       %cond = OpSpecConstantOp %i32 Not %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpNot %i32 %cond
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.not<i32> 1i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = spirv.not<i32> %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_Not_ResultI32_u32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
        %one = OpConstant %u32 1
    %myconst = OpSpecConstantOp %i32 Not %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.not<i32> 1u
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_Not_ResultU32_i32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
        %one = OpConstant %i32 1
    %myconst = OpSpecConstantOp %u32 Not %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.not<u32> 1i
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_Not_ResultU32_u32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
      %false = OpSpecConstantFalse %bool
       %true = OpSpecConstantTrue %bool
        %one = OpConstant %u32 1
    %myconst = OpSpecConstantOp %u32 Not %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.not<u32> 1u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_FConvert) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability Float16
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %f16 = OpTypeFloat 16
        %one = OpConstant %f16 1.0
    %myconst = OpSpecConstantOp %f32 FConvert %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %f32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:f32 = convert 1.0h
  %myconst:f32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:f32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SNegate_ResultI32_i32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
    %myconst = OpSpecConstantOp %i32 SNegate %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.s_negate<i32> 1i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SNegate_ResultI32_u32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %u32 1
    %myconst = OpSpecConstantOp %i32 SNegate %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.s_negate<i32> 1u
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SNegate_ResultU32_i32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %i32 1
    %myconst = OpSpecConstantOp %u32 SNegate %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.s_negate<u32> 1i
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SNegate_ResultU32_u32) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %u32 1
    %myconst = OpSpecConstantOp %u32 SNegate %one
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.s_negate<u32> 1u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_IAdd_ResultI32_i32_i32) {
    // All signed
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %i32 IAdd %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.add<i32> 1i, 2i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_IAdd_ResultI32_i32_u32) {
    // Last arg different
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %i32 IAdd %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.add<i32> 1i, 2u
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_IAdd_ResultI32_u32_i32) {
    // Second arg different
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %u32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %i32 IAdd %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.add<i32> 1u, 2i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_IAdd_ResultU32_i32_i32) {
    // Result different
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %u32 IAdd %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.add<u32> 1i, 2i
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_ISub_ResultI32_i32_i32) {
    // All signed
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %i32 ISub %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.sub<i32> 1i, 2i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_ISub_ResultI32_i32_u32) {
    // Last arg different
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %i32 ISub %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.sub<i32> 1i, 2u
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_ISub_ResultI32_u32_i32) {
    // Second arg different
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %u32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %i32 ISub %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.sub<i32> 1u, 2i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_ISub_ResultU32_i32_i32) {
    // Result different
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %u32 ISub %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.sub<u32> 1i, 2i
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_IMul_ResultI32_i32_i32) {
    // All signed
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %i32 IMul %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.mul<i32> 1i, 2i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_IMul_ResultI32_i32_u32) {
    // Last arg different
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %i32 IMul %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.mul<i32> 1i, 2u
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_IMul_ResultI32_u32_i32) {
    // Second arg different
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %u32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %i32 IMul %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.mul<i32> 1u, 2i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_IMul_ResultU32_i32_i32) {
    // Result different
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %u32 IMul %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.mul<u32> 1i, 2i
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SDiv) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 SDiv %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.s_div<u32> 1i, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_UDiv) {
    // OpUDiv requires signedness 0 for both args and the result type.
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %u32 = OpTypeInt 32 0
        %one = OpConstant %u32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 UDiv %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = div 1u, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_UMod) {
    // SPIR-V UMod requires arguments and result type to be unsigned.
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %one = OpConstant %u32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 UMod %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = mod 1u, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SMod) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %u32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %u32 SMod %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.s_mod<u32> 1u, 2i
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SRem) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 SRem %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.s_mod<u32> 1i, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_BitwiseAnd) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 BitwiseAnd %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.bitwise_and<u32> 1i, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_BitwiseOr) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 BitwiseOr %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.bitwise_or<u32> 1i, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_BitwiseXor) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 BitwiseXor %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.bitwise_xor<u32> 1i, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_IEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool IEqual %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.equal 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_INotEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool INotEqual %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.not_equal 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SGreaterThan) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool SGreaterThan %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.s_greater_than 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SGreaterThanEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool SGreaterThanEqual %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.s_greater_than_equal 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SLessThan) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool SLessThan %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.s_less_than 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_SLessThanEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool SLessThanEqual %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.s_less_than_equal 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_UGreaterThan) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool UGreaterThan %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.u_greater_than 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_UGreaterThanEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool UGreaterThanEqual %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.u_greater_than_equal 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_ULessThan) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool ULessThan %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.u_less_than 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_ULessThanEqual) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %bool ULessThanEqual %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %bool %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:bool = spirv.u_less_than_equal 1i, 2u
  %myconst:bool = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:bool = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_ShiftLeftLogical) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 ShiftLeftLogical %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.shift_left_logical<u32> 1i, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_ShiftRightLogical) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 ShiftRightLogical %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.shift_right_logical<u32> 1i, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_ShiftRightArithmetic) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
        %one = OpConstant %i32 1
        %two = OpConstant %u32 2
    %myconst = OpSpecConstantOp %u32 ShiftRightArithmetic %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %u32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:u32 = spirv.shift_right_arithmetic<u32> 1i, 2u
  %myconst:u32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:u32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_CompositeExtract) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
      %vec2i = OpTypeVector %i32 2
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
    %one_two = OpConstantComposite %vec2i %one %two
    %myconst = OpSpecConstantOp %i32 CompositeExtract %one_two 1
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = access vec2<i32>(1i, 2i), 1u
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantOp_Select_CondScalar_ArgsScalar) {
    // In SPIR-V the arg types must be the same as the result type.
    // WGSL only supports overrides with scalar type, so only test the
    // case where the result type is scalar.
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %myconst "myconst"
       %void = OpTypeVoid
       %bool = OpTypeBool
        %u32 = OpTypeInt 32 0
        %i32 = OpTypeInt 32 1
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
    %myconst = OpSpecConstantOp %i32 Select %true %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %1 = OpCopyObject %i32 %myconst
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = spirv.select true, 1i, 2i
  %myconst:i32 = override %1
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = let %myconst
    ret
  }
}
)");
}

// In the case of all literals, SPIR-V opt treats the `OpSpecConstantComposite` as an
// `OpConstantComposite` so it appears in the constant manager already. This then needs no handling
// on our side.
TEST_F(SpirvParserTest, Var_OpSpecConstantComposite_vec2_literals) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %1 "myconst"
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %v2i = OpTypeVector %i32 2
        %one = OpConstant %i32 1
        %two = OpConstant %i32 2
          %1 = OpSpecConstantComposite %v2i %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %2 = OpIAdd %v2i %1 %1
               OpReturn
               OpFunctionEnd
)",
              R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.add<i32> vec2<i32>(1i, 2i), vec2<i32>(1i, 2i)
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantComposite_vec2_SpecConstants) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %1 "myconst"
               OpDecorate %one SpecId 1
               OpDecorate %two SpecId 2
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %v2i = OpTypeVector %i32 2
        %one = OpSpecConstant %i32 1
        %two = OpSpecConstant %i32 2
          %1 = OpSpecConstantComposite %v2i %one %two
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %2 = OpIAdd %v2i %1 %1
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = override 1i @id(1)
  %2:i32 = override 2i @id(2)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:vec2<i32> = construct %1, %2
    %5:vec2<i32> = spirv.add<i32> %4, %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantComposite_vec4_Mixed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %1 "myconst"
               OpDecorate %one SpecId 1
               OpDecorate %three SpecId 3
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %v4i = OpTypeVector %i32 4
        %one = OpSpecConstant %i32 1
        %two = OpConstant %i32 2
      %three = OpSpecConstant %i32 3
       %four = OpConstant %i32 4
          %1 = OpSpecConstantComposite %v4i %one %two %three %four
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %2 = OpIAdd %v4i %1 %1
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = override 1i @id(1)
  %2:i32 = override 3i @id(3)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:vec4<i32> = construct %1, 2i, %2, 4i
    %5:vec4<i32> = spirv.add<i32> %4, %4
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantComposite_mat3x4_Mixed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %1 "myconst"
               OpDecorate %one SpecId 1
               OpDecorate %three SpecId 3
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
        %v3f = OpTypeVector %f32 3
     %mat4x3 = OpTypeMatrix %v3f 4
        %one = OpSpecConstant %f32 1
        %two = OpConstant %f32 2
      %three = OpSpecConstant %f32 3
       %four = OpConstant %f32 4
          %1 = OpSpecConstantComposite %v3f %one %two %three
          %2 = OpSpecConstantComposite %mat4x3 %1 %1 %1 %1
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
          %3 = OpMatrixTimesScalar %mat4x3 %2 %four
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:f32 = override 1.0f @id(1)
  %2:f32 = override 3.0f @id(3)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:vec3<f32> = construct %1, 2.0f, %2
    %5:mat4x3<f32> = construct %4, %4, %4, %4
    %6:mat4x3<f32> = mul %5, 4.0f
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantComposite_array_Mixed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %1 "myconst"
               OpDecorate %one SpecId 1
               OpDecorate %three SpecId 3
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
        %f32 = OpTypeFloat 32
        %ary = OpTypeArray %f32 %uint_3
        %ptr = OpTypePointer Function %f32
     %fn_ptr = OpTypePointer Function %ary
        %one = OpSpecConstant %f32 1
        %two = OpConstant %f32 2
      %three = OpSpecConstant %f32 3
          %1 = OpSpecConstantComposite %ary %one %two %three
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
  %indexable = OpVariable %fn_ptr Function
               OpStore %indexable %1
         %20 = OpAccessChain %ptr %indexable %uint_2
         %21 = OpLoad %f32 %20
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:f32 = override 1.0f @id(1)
  %2:f32 = override 3.0f @id(3)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:ptr<function, array<f32, 3>, read_write> = var undef
    %5:array<f32, 3> = construct %1, 2.0f, %2
    store %4, %5
    %6:ptr<function, f32, read_write> = access %4, 2u
    %7:f32 = load %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantComposite_struct_Mixed) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %1 "main"
               OpExecutionMode %1 LocalSize 1 1 1
               OpDecorate %str Block
               OpMemberDecorate %str 0 Offset 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
        %f32 = OpTypeFloat 32
        %str = OpTypeStruct %uint %f32
    %ptr_str = OpTypePointer Function %str
    %ptr_f32 = OpTypePointer Function %f32
      %int_1 = OpConstant %int 1
        %one = OpSpecConstant %uint 1
        %two = OpConstant %f32 2
          %5 = OpTypeFunction %void
          %2 = OpSpecConstantComposite %str %one %two
          %1 = OpFunction %void None %5
         %11 = OpLabel
          %b = OpVariable %ptr_str Function
               OpStore %b %2
         %24 = OpAccessChain %ptr_f32 %b %int_1
         %17 = OpLoad %f32 %24
         %25 = OpFAdd %f32 %17 %17
               OpReturn
               OpFunctionEnd
)",
              R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:u32 @offset(0)
  tint_symbol_1:f32 @offset(4)
}

$B1: {  # root
  %1:u32 = override 1u
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<function, tint_symbol_2, read_write> = var undef
    %4:tint_symbol_2 = construct %1, 2.0f
    store %3, %4
    %5:ptr<function, f32, read_write> = access %3, 1i
    %6:f32 = load %5
    %7:f32 = add %6, %6
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, Var_OpSpecConstantComposite_Extracted) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %1 "myconst"
               OpDecorate %one SpecId 1
               OpDecorate %two SpecId 2
       %void = OpTypeVoid
        %i32 = OpTypeInt 32 1
        %v2i = OpTypeVector %i32 2
        %one = OpSpecConstant %i32 1
        %two = OpSpecConstant %i32 2
          %1 = OpSpecConstantComposite %v2i %one %two
        %foo = OpSpecConstantOp %i32 CompositeExtract %1 1
     %voidfn = OpTypeFunction %void
       %main = OpFunction %void None %voidfn
 %main_entry = OpLabel
           %3 = OpIAdd %i32 %foo %foo
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:i32 = override 1i @id(1)
  %2:i32 = override 2i @id(2)
  %3:vec2<i32> = construct %1, %2
  %4:i32 = access %3, 1u
  %5:i32 = override %4
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %7:i32 = spirv.add<i32> %5, %5
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, InterpolationFlatNoLocation) {
    EXPECT_IR(R"(
             OpCapability Shader
             OpCapability SampleRateShading
             OpMemoryModel Logical Simple
             OpEntryPoint Fragment %3 "main" %gl_SampleID
             OpExecutionMode %3 OriginUpperLeft
             OpDecorate %gl_SampleID BuiltIn SampleId
             OpDecorate %gl_SampleID Flat
     %void = OpTypeVoid
        %5 = OpTypeFunction %void
    %float = OpTypeFloat 32
     %uint = OpTypeInt 32 0
      %int = OpTypeInt 32 1
%_ptr_Input_uint = OpTypePointer Input %uint
%gl_SampleID = OpVariable %_ptr_Input_uint Input
        %3 = OpFunction %void None %5
       %10 = OpLabel
        %2 = OpLoad %uint %gl_SampleID
             OpReturn
             OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<__in, u32, read> = var undef @builtin(sample_index)
}

%main = @fragment func():void {
  $B2: {
    undef = phony %1
    %3:u32 = load %1
    ret
  }
}
)");
}

TEST_F(SpirvParserTest, VarMarkedFragDepth) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpCapability SampleRateShading
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %2 "main" %gl_FragDepth
               OpExecutionMode %2 OriginUpperLeft
               OpDecorate %gl_FragDepth BuiltIn FragDepth
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
%gl_FragDepth = OpVariable %_ptr_Output_float Output %float_0
          %2 = OpFunction %void None %4
         %29 = OpLabel
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %1:ptr<__out, f32, read_write> = var 0.0f @builtin(frag_depth)
}

%main = @fragment func():void {
  $B2: {
    undef = phony %1
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
