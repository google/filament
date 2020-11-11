// Copyright (c) 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using ReduceLoadSizeTest = PassTest<::testing::Test>;

TEST_F(ReduceLoadSizeTest, cbuffer_load_extract) {
  // Originally from the following HLSL:
  //   struct S {
  //     uint f;
  //   };
  //
  //
  //   cbuffer gBuffer { uint a[32]; };
  //
  //   RWStructuredBuffer<S> gRWSBuffer;
  //
  //   uint foo(uint p[32]) {
  //     return p[1];
  //   }
  //
  //   [numthreads(1,1,1)]
  //   void main() {
  //      gRWSBuffer[0].f = foo(a);
  //   }
  const std::string test =
      R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource HLSL 600
               OpName %type_gBuffer "type.gBuffer"
               OpMemberName %type_gBuffer 0 "a"
               OpName %gBuffer "gBuffer"
               OpName %S "S"
               OpMemberName %S 0 "f"
               OpName %type_RWStructuredBuffer_S "type.RWStructuredBuffer.S"
               OpName %gRWSBuffer "gRWSBuffer"
               OpName %main "main"
               OpDecorate %_arr_uint_uint_32 ArrayStride 16
               OpMemberDecorate %type_gBuffer 0 Offset 0
               OpDecorate %type_gBuffer Block
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 4
               OpMemberDecorate %type_RWStructuredBuffer_S 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_S BufferBlock
               OpDecorate %gBuffer DescriptorSet 0
               OpDecorate %gBuffer Binding 0
               OpDecorate %gRWSBuffer DescriptorSet 0
               OpDecorate %gRWSBuffer Binding 1
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
%_arr_uint_uint_32 = OpTypeArray %uint %uint_32
%type_gBuffer = OpTypeStruct %_arr_uint_uint_32
%_ptr_Uniform_type_gBuffer = OpTypePointer Uniform %type_gBuffer
          %S = OpTypeStruct %uint
%_runtimearr_S = OpTypeRuntimeArray %S
%type_RWStructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_type_RWStructuredBuffer_S = OpTypePointer Uniform %type_RWStructuredBuffer_S
        %int = OpTypeInt 32 1
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform__arr_uint_uint_32 = OpTypePointer Uniform %_arr_uint_uint_32
     %uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
    %gBuffer = OpVariable %_ptr_Uniform_type_gBuffer Uniform
 %gRWSBuffer = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
       %main = OpFunction %void None %15
         %20 = OpLabel
; CHECK: [[ac1:%\w+]] = OpAccessChain {{%\w+}} %gBuffer %int_0
; CHECK: [[ac2:%\w+]] = OpAccessChain {{%\w+}} [[ac1]] %uint_1
; CHECK: [[ld:%\w+]] = OpLoad {{%\w+}} [[ac2]]
; CHECK: OpStore {{%\w+}} [[ld]]
         %21 = OpAccessChain %_ptr_Uniform__arr_uint_uint_32 %gBuffer %int_0
         %22 = OpLoad %_arr_uint_uint_32 %21    ; Load of 32-element array.
         %23 = OpCompositeExtract %uint %22 1
         %24 = OpAccessChain %_ptr_Uniform_uint %gRWSBuffer %int_0 %uint_0 %int_0
               OpStore %24 %23
               OpReturn
               OpFunctionEnd
  )";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<ReduceLoadSize>(test, false);
}

TEST_F(ReduceLoadSizeTest, cbuffer_load_extract_not_affected_by_debug_instr) {
  // Originally from the following HLSL:
  //   struct S {
  //     uint f;
  //   };
  //
  //
  //   cbuffer gBuffer { uint a[32]; };
  //
  //   RWStructuredBuffer<S> gRWSBuffer;
  //
  //   uint foo(uint p[32]) {
  //     return p[1];
  //   }
  //
  //   [numthreads(1,1,1)]
  //   void main() {
  //      gRWSBuffer[0].f = foo(a);
  //   }
  const std::string test =
      R"(
               OpCapability Shader
        %ext = OpExtInstImport "OpenCL.DebugInfo.100"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpSource HLSL 600
  %file_name = OpString "test"
 %float_name = OpString "float"
  %main_name = OpString "main"
     %f_name = OpString "f"
               OpName %type_gBuffer "type.gBuffer"
               OpMemberName %type_gBuffer 0 "a"
               OpName %gBuffer "gBuffer"
               OpName %S "S"
               OpMemberName %S 0 "f"
               OpName %type_RWStructuredBuffer_S "type.RWStructuredBuffer.S"
               OpName %gRWSBuffer "gRWSBuffer"
               OpName %main "main"
               OpDecorate %_arr_uint_uint_32 ArrayStride 16
               OpMemberDecorate %type_gBuffer 0 Offset 0
               OpDecorate %type_gBuffer Block
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %_runtimearr_S ArrayStride 4
               OpMemberDecorate %type_RWStructuredBuffer_S 0 Offset 0
               OpDecorate %type_RWStructuredBuffer_S BufferBlock
               OpDecorate %gBuffer DescriptorSet 0
               OpDecorate %gBuffer Binding 0
               OpDecorate %gRWSBuffer DescriptorSet 0
               OpDecorate %gRWSBuffer Binding 1
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
%_arr_uint_uint_32 = OpTypeArray %uint %uint_32
%type_gBuffer = OpTypeStruct %_arr_uint_uint_32
%_ptr_Uniform_type_gBuffer = OpTypePointer Uniform %type_gBuffer
          %S = OpTypeStruct %uint
%_runtimearr_S = OpTypeRuntimeArray %S
%type_RWStructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_type_RWStructuredBuffer_S = OpTypePointer Uniform %type_RWStructuredBuffer_S
        %int = OpTypeInt 32 1
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform__arr_uint_uint_32 = OpTypePointer Uniform %_arr_uint_uint_32
     %uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
    %gBuffer = OpVariable %_ptr_Uniform_type_gBuffer Uniform
 %gRWSBuffer = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
  %null_expr = OpExtInst %void %ext DebugExpression
        %src = OpExtInst %void %ext DebugSource %file_name
         %cu = OpExtInst %void %ext DebugCompilationUnit 1 4 %src HLSL
     %dbg_tf = OpExtInst %void %ext DebugTypeBasic %float_name %uint_32 Float
    %main_ty = OpExtInst %void %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %dbg_tf
   %dbg_main = OpExtInst %void %ext DebugFunction %main_name %main_ty %src 0 0 %cu %main_name FlagIsProtected|FlagIsPrivate 10 %main
      %dbg_f = OpExtInst %void %ext DebugLocalVariable %f_name %dbg_tf %src 0 0 %dbg_main FlagIsLocal
       %main = OpFunction %void None %15
         %20 = OpLabel
          %s = OpExtInst %void %ext DebugScope %dbg_main
; CHECK: [[ac1:%\w+]] = OpAccessChain {{%\w+}} %gBuffer %int_0
; CHECK: [[ac2:%\w+]] = OpAccessChain {{%\w+}} [[ac1]] %uint_1
; CHECK: [[ld:%\w+]] = OpLoad {{%\w+}} [[ac2]]
; CHECK: OpStore {{%\w+}} [[ld]]
         %21 = OpAccessChain %_ptr_Uniform__arr_uint_uint_32 %gBuffer %int_0
         %22 = OpLoad %_arr_uint_uint_32 %21    ; Load of 32-element array.
      %value = OpExtInst %void %ext DebugValue %dbg_f %22 %null_expr
         %23 = OpCompositeExtract %uint %22 1
         %24 = OpAccessChain %_ptr_Uniform_uint %gRWSBuffer %int_0 %uint_0 %int_0
               OpStore %24 %23
               OpReturn
               OpFunctionEnd
  )";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndMatch<ReduceLoadSize>(test, false);
}

TEST_F(ReduceLoadSizeTest, cbuffer_load_extract_vector) {
  // Originally from the following HLSL:
  //   struct S {
  //     uint f;
  //   };
  //
  //
  //   cbuffer gBuffer { uint4 a; };
  //
  //   RWStructuredBuffer<S> gRWSBuffer;
  //
  //   uint foo(uint p[32]) {
  //     return p[1];
  //   }
  //
  //   [numthreads(1,1,1)]
  //   void main() {
  //      gRWSBuffer[0].f = foo(a);
  //   }
  const std::string test =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource HLSL 600
OpName %type_gBuffer "type.gBuffer"
OpMemberName %type_gBuffer 0 "a"
OpName %gBuffer "gBuffer"
OpName %S "S"
OpMemberName %S 0 "f"
OpName %type_RWStructuredBuffer_S "type.RWStructuredBuffer.S"
OpName %gRWSBuffer "gRWSBuffer"
OpName %main "main"
OpMemberDecorate %type_gBuffer 0 Offset 0
OpDecorate %type_gBuffer Block
OpMemberDecorate %S 0 Offset 0
OpDecorate %_runtimearr_S ArrayStride 4
OpMemberDecorate %type_RWStructuredBuffer_S 0 Offset 0
OpDecorate %type_RWStructuredBuffer_S BufferBlock
OpDecorate %gBuffer DescriptorSet 0
OpDecorate %gBuffer Binding 0
OpDecorate %gRWSBuffer DescriptorSet 0
OpDecorate %gRWSBuffer Binding 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%v4uint = OpTypeVector %uint 4
%type_gBuffer = OpTypeStruct %v4uint
%_ptr_Uniform_type_gBuffer = OpTypePointer Uniform %type_gBuffer
%S = OpTypeStruct %uint
%_runtimearr_S = OpTypeRuntimeArray %S
%type_RWStructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_type_RWStructuredBuffer_S = OpTypePointer Uniform %type_RWStructuredBuffer_S
%int = OpTypeInt 32 1
%void = OpTypeVoid
%15 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Uniform_v4uint = OpTypePointer Uniform %v4uint
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%gBuffer = OpVariable %_ptr_Uniform_type_gBuffer Uniform
%gRWSBuffer = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
%main = OpFunction %void None %15
%20 = OpLabel
%21 = OpAccessChain %_ptr_Uniform_v4uint %gBuffer %int_0
%22 = OpLoad %v4uint %21
%23 = OpCompositeExtract %uint %22 1
%24 = OpAccessChain %_ptr_Uniform_uint %gRWSBuffer %int_0 %uint_0 %int_0
OpStore %24 %23
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndCheck<ReduceLoadSize>(test, test, true, false);
}

TEST_F(ReduceLoadSizeTest, cbuffer_load_5_extract) {
  // All of the elements of the value loaded are used, so we should not
  // change the load.
  const std::string test =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource HLSL 600
OpName %type_gBuffer "type.gBuffer"
OpMemberName %type_gBuffer 0 "a"
OpName %gBuffer "gBuffer"
OpName %S "S"
OpMemberName %S 0 "f"
OpName %type_RWStructuredBuffer_S "type.RWStructuredBuffer.S"
OpName %gRWSBuffer "gRWSBuffer"
OpName %main "main"
OpDecorate %_arr_uint_uint_5 ArrayStride 16
OpMemberDecorate %type_gBuffer 0 Offset 0
OpDecorate %type_gBuffer Block
OpMemberDecorate %S 0 Offset 0
OpDecorate %_runtimearr_S ArrayStride 4
OpMemberDecorate %type_RWStructuredBuffer_S 0 Offset 0
OpDecorate %type_RWStructuredBuffer_S BufferBlock
OpDecorate %gBuffer DescriptorSet 0
OpDecorate %gBuffer Binding 0
OpDecorate %gRWSBuffer DescriptorSet 0
OpDecorate %gRWSBuffer Binding 1
%uint = OpTypeInt 32 0
%uint_5 = OpConstant %uint 5
%_arr_uint_uint_5 = OpTypeArray %uint %uint_5
%type_gBuffer = OpTypeStruct %_arr_uint_uint_5
%_ptr_Uniform_type_gBuffer = OpTypePointer Uniform %type_gBuffer
%S = OpTypeStruct %uint
%_runtimearr_S = OpTypeRuntimeArray %S
%type_RWStructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_type_RWStructuredBuffer_S = OpTypePointer Uniform %type_RWStructuredBuffer_S
%int = OpTypeInt 32 1
%void = OpTypeVoid
%15 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_uint_uint_5 = OpTypePointer Uniform %_arr_uint_uint_5
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%gBuffer = OpVariable %_ptr_Uniform_type_gBuffer Uniform
%gRWSBuffer = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
%main = OpFunction %void None %15
%20 = OpLabel
%21 = OpAccessChain %_ptr_Uniform__arr_uint_uint_5 %gBuffer %int_0
%22 = OpLoad %_arr_uint_uint_5 %21
%23 = OpCompositeExtract %uint %22 0
%24 = OpCompositeExtract %uint %22 1
%25 = OpCompositeExtract %uint %22 2
%26 = OpCompositeExtract %uint %22 3
%27 = OpCompositeExtract %uint %22 4
%28 = OpIAdd %uint %23 %24
%29 = OpIAdd %uint %28 %25
%30 = OpIAdd %uint %29 %26
%31 = OpIAdd %uint %20 %27
%32 = OpAccessChain %_ptr_Uniform_uint %gRWSBuffer %int_0 %uint_0 %int_0
OpStore %32 %31
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndCheck<ReduceLoadSize>(test, test, true, false);
}

TEST_F(ReduceLoadSizeTest, cbuffer_load_fully_used) {
  // The result of the load (%22) is used in an instruction that uses the whole
  // load and has only 1 in operand.  This trigger issue #1559.
  const std::string test =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource HLSL 600
OpName %type_gBuffer "type.gBuffer"
OpMemberName %type_gBuffer 0 "a"
OpName %gBuffer "gBuffer"
OpName %S "S"
OpMemberName %S 0 "f"
OpName %type_RWStructuredBuffer_S "type.RWStructuredBuffer.S"
OpName %gRWSBuffer "gRWSBuffer"
OpName %main "main"
OpMemberDecorate %type_gBuffer 0 Offset 0
OpDecorate %type_gBuffer Block
OpMemberDecorate %S 0 Offset 0
OpDecorate %_runtimearr_S ArrayStride 4
OpMemberDecorate %type_RWStructuredBuffer_S 0 Offset 0
OpDecorate %type_RWStructuredBuffer_S BufferBlock
OpDecorate %gBuffer DescriptorSet 0
OpDecorate %gBuffer Binding 0
OpDecorate %gRWSBuffer DescriptorSet 0
OpDecorate %gRWSBuffer Binding 1
%uint = OpTypeInt 32 0
%uint_32 = OpConstant %uint 32
%v4uint = OpTypeVector %uint 4
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%type_gBuffer = OpTypeStruct %v4uint
%_ptr_Uniform_type_gBuffer = OpTypePointer Uniform %type_gBuffer
%S = OpTypeStruct %uint
%_runtimearr_S = OpTypeRuntimeArray %S
%type_RWStructuredBuffer_S = OpTypeStruct %_runtimearr_S
%_ptr_Uniform_type_RWStructuredBuffer_S = OpTypePointer Uniform %type_RWStructuredBuffer_S
%int = OpTypeInt 32 1
%void = OpTypeVoid
%15 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Uniform_v4uint = OpTypePointer Uniform %v4uint
%uint_0 = OpConstant %uint 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%gBuffer = OpVariable %_ptr_Uniform_type_gBuffer Uniform
%gRWSBuffer = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
%main = OpFunction %void None %15
%20 = OpLabel
%21 = OpAccessChain %_ptr_Uniform_v4uint %gBuffer %int_0
%22 = OpLoad %v4uint %21
%23 = OpCompositeExtract %uint %22 1
%24 = OpConvertUToF %v4float %22
%25 = OpAccessChain %_ptr_Uniform_uint %gRWSBuffer %int_0 %uint_0 %int_0
OpStore %25 %23
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SetDisassembleOptions(SPV_BINARY_TO_TEXT_OPTION_NO_HEADER |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
  SinglePassRunAndCheck<ReduceLoadSize>(test, test, true, false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
