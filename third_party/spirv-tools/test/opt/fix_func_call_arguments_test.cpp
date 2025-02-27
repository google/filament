// Copyright (c) 2022 Advanced Micro Devices, Inc.
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

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using FixFuncCallArgumentsTest = PassTest<::testing::Test>;
TEST_F(FixFuncCallArgumentsTest, Simple) {
  const std::string text = R"(
;
; CHECK: [[v0:%\w+]] = OpVariable %_ptr_Function_float Function
; CHECK: [[v1:%\w+]] = OpVariable %_ptr_Function_float Function
; CHECK: [[v2:%\w+]] = OpVariable %_ptr_Function_T Function
; CHECK: [[ac0:%\w+]] = OpAccessChain %_ptr_Function_float %t %int_0
; CHECK: [[ac1:%\w+]] = OpAccessChain %_ptr_Uniform_float %r1 %int_0 %uint_0
; CHECK: [[ld0:%\w+]] = OpLoad %float [[ac0]]
; CHECK:                OpStore [[v1]] [[ld0]]
; CHECK: [[ld1:%\w+]] = OpLoad %float [[ac1]]
; CHECK:                OpStore [[v0]] [[ld1]]
; CHECK: [[func:%\w+]] = OpFunctionCall %void %fn [[v1]] [[v0]]
; CHECK: [[ld2:%\w+]] = OpLoad %float [[v0]]
; CHECK: OpStore [[ac1]] [[ld2]]
; CHECK: [[ld3:%\w+]] = OpLoad %float [[v1]]
; CHECK: OpStore [[ac0]] [[ld3]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpSource HLSL 630
OpName %type_RWStructuredBuffer_float "type.RWStructuredBuffer.float"
OpName %r1 "r1"
OpName %type_ACSBuffer_counter "type.ACSBuffer.counter"
OpMemberName %type_ACSBuffer_counter 0 "counter"
OpName %counter_var_r1 "counter.var.r1"
OpName %main "main"
OpName %bb_entry "bb.entry"
OpName %T "T"
OpMemberName %T 0 "t0"
OpName %t "t"
OpName %fn "fn"
OpName %p0 "p0"
OpName %p2 "p2"
OpName %bb_entry_0 "bb.entry"
OpDecorate %main LinkageAttributes "main" Export
OpDecorate %r1 DescriptorSet 0
OpDecorate %r1 Binding 0
OpDecorate %counter_var_r1 DescriptorSet 0
OpDecorate %counter_var_r1 Binding 1
OpDecorate %_runtimearr_float ArrayStride 4
OpMemberDecorate %type_RWStructuredBuffer_float 0 Offset 0
OpDecorate %type_RWStructuredBuffer_float BufferBlock
OpMemberDecorate %type_ACSBuffer_counter 0 Offset 0
OpDecorate %type_ACSBuffer_counter BufferBlock
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%int_1 = OpConstant %int 1
%float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
%type_RWStructuredBuffer_float = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_type_RWStructuredBuffer_float = OpTypePointer Uniform %type_RWStructuredBuffer_float
%type_ACSBuffer_counter = OpTypeStruct %int
%_ptr_Uniform_type_ACSBuffer_counter = OpTypePointer Uniform %type_ACSBuffer_counter
%15 = OpTypeFunction %int
%T = OpTypeStruct %float
%_ptr_Function_T = OpTypePointer Function %T
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%void = OpTypeVoid
%27 = OpTypeFunction %void %_ptr_Function_float %_ptr_Function_float
%r1 = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_float Uniform
%counter_var_r1 = OpVariable %_ptr_Uniform_type_ACSBuffer_counter Uniform
%main = OpFunction %int None %15
%bb_entry = OpLabel
%t = OpVariable %_ptr_Function_T Function
%21 = OpAccessChain %_ptr_Function_float %t %int_0
%23 = OpAccessChain %_ptr_Uniform_float %r1 %int_0 %uint_0
%25 = OpFunctionCall %void %fn %21 %23
OpReturnValue %int_1
OpFunctionEnd
%fn = OpFunction %void DontInline %27
%p0 = OpFunctionParameter %_ptr_Function_float
%p2 = OpFunctionParameter %_ptr_Function_float
%bb_entry_0 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<FixFuncCallArgumentsPass>(text, true);
}

TEST_F(FixFuncCallArgumentsTest, NotAccessChainInput) {
  const std::string text = R"(
;
; CHECK: [[o:%\w+]] = OpCopyObject %_ptr_Function_float %t
; CHECK: [[func:%\w+]] = OpFunctionCall %void %fn [[o]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpSource HLSL 630
OpName %main "main"
OpName %bb_entry "bb.entry"
OpName %t "t"
OpName %fn "fn"
OpName %p0 "p0"
OpName %bb_entry_0 "bb.entry"
OpDecorate %main LinkageAttributes "main" Export
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%4 = OpTypeFunction %int
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%12 = OpTypeFunction %void %_ptr_Function_float
%main = OpFunction %int None %4
%bb_entry = OpLabel
%t = OpVariable %_ptr_Function_float Function
%t1 = OpCopyObject %_ptr_Function_float %t
%10 = OpFunctionCall %void %fn %t1
OpReturnValue %int_1
OpFunctionEnd
%fn = OpFunction %void DontInline %12
%p0 = OpFunctionParameter %_ptr_Function_float
%bb_entry_0 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<FixFuncCallArgumentsPass>(text, false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools