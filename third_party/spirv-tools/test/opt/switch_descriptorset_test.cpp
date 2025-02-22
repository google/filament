// Copyright (c) 2023 LunarG Inc.
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

// Bindless Check Instrumentation Tests.
// Tests ending with V2 use version 2 record format.

#include <string>
#include <vector>

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using SwitchDescriptorSetTest = PassTest<::testing::Test>;

TEST_F(SwitchDescriptorSetTest, Basic) {
  // #version 450
  // #extension GL_EXT_buffer_reference : enable
  //
  // layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
  //
  // layout(set = 7, binding = 7) uniform ufoo {
  //     bufStruct data;
  //     uint offset;
  // } u_info;
  //
  // layout(buffer_reference, std140) buffer bufStruct {
  //     layout(offset = 0) int a[2];
  //     layout(offset = 32) int b;
  // };
  //
  // void main() {
  //     u_info.data.b = 0xca7;
  // }

  const std::string spirv = R"(
OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_EXT_physical_storage_buffer"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpSourceExtension "GL_EXT_buffer_reference"
OpName %main "main"
OpName %ufoo "ufoo"
OpMemberName %ufoo 0 "data"
OpMemberName %ufoo 1 "offset"
OpName %bufStruct "bufStruct"
OpMemberName %bufStruct 0 "a"
OpMemberName %bufStruct 1 "b"
OpName %u_info "u_info"
OpMemberDecorate %ufoo 0 Offset 0
OpMemberDecorate %ufoo 1 Offset 8
OpDecorate %ufoo Block
OpDecorate %_arr_int_uint_2 ArrayStride 16
OpMemberDecorate %bufStruct 0 Offset 0
OpMemberDecorate %bufStruct 1 Offset 32
OpDecorate %bufStruct Block
OpDecorate %u_info DescriptorSet 7
;CHECK: OpDecorate %u_info DescriptorSet 31
OpDecorate %u_info Binding 7
;CHECK: OpDecorate %u_info Binding 7
%void = OpTypeVoid
%3 = OpTypeFunction %void
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_bufStruct PhysicalStorageBuffer
%uint = OpTypeInt 32 0
%ufoo = OpTypeStruct %_ptr_PhysicalStorageBuffer_bufStruct %uint
%int = OpTypeInt 32 1
%uint_2 = OpConstant %uint 2
%_arr_int_uint_2 = OpTypeArray %int %uint_2
%bufStruct = OpTypeStruct %_arr_int_uint_2 %int
%_ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer PhysicalStorageBuffer %bufStruct
%_ptr_Uniform_ufoo = OpTypePointer Uniform %ufoo
%u_info = OpVariable %_ptr_Uniform_ufoo Uniform
%int_0 = OpConstant %int 0
%_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer Uniform %_ptr_PhysicalStorageBuffer_bufStruct
%int_1 = OpConstant %int 1
%int_3239 = OpConstant %int 3239
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%main = OpFunction %void None %3
%5 = OpLabel
%17 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %u_info %int_0
%18 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %17
%22 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_1
OpReturn
OpFunctionEnd
)";
  // clang-format off

  SinglePassRunAndMatch<SwitchDescriptorSetPass>(spirv, true, 7, 31);
}


// Make sure DescriptorSet decorations that don't match the requested number
// are left unchanged.
TEST_F(SwitchDescriptorSetTest, Unchanged) {
  // #version 450
  // #extension GL_EXT_buffer_reference : enable
  //
  // layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
  //
  // layout(set = 11, binding = 7) uniform ufoo {
  //     bufStruct data;
  //     uint offset;
  // } u_info;
  //
  // layout(buffer_reference, std140) buffer bufStruct {
  //     layout(offset = 0) int a[2];
  //     layout(offset = 32) int b;
  // };
  //
  // void main() {
  //     u_info.data.b = 0xca7;
  // }

  const std::string spirv = R"(
OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_EXT_physical_storage_buffer"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpSourceExtension "GL_EXT_buffer_reference"
OpName %main "main"
OpName %ufoo "ufoo"
OpMemberName %ufoo 0 "data"
OpMemberName %ufoo 1 "offset"
OpName %bufStruct "bufStruct"
OpMemberName %bufStruct 0 "a"
OpMemberName %bufStruct 1 "b"
OpName %u_info "u_info"
OpMemberDecorate %ufoo 0 Offset 0
OpMemberDecorate %ufoo 1 Offset 8
OpDecorate %ufoo Block
OpDecorate %_arr_int_uint_2 ArrayStride 16
OpMemberDecorate %bufStruct 0 Offset 0
OpMemberDecorate %bufStruct 1 Offset 32
OpDecorate %bufStruct Block
OpDecorate %u_info DescriptorSet 11
;CHECK: OpDecorate %u_info DescriptorSet 11
OpDecorate %u_info Binding 7
;CHECK: OpDecorate %u_info Binding 7
%void = OpTypeVoid
%3 = OpTypeFunction %void
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_bufStruct PhysicalStorageBuffer
%uint = OpTypeInt 32 0
%ufoo = OpTypeStruct %_ptr_PhysicalStorageBuffer_bufStruct %uint
%int = OpTypeInt 32 1
%uint_2 = OpConstant %uint 2
%_arr_int_uint_2 = OpTypeArray %int %uint_2
%bufStruct = OpTypeStruct %_arr_int_uint_2 %int
%_ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer PhysicalStorageBuffer %bufStruct
%_ptr_Uniform_ufoo = OpTypePointer Uniform %ufoo
%u_info = OpVariable %_ptr_Uniform_ufoo Uniform
%int_0 = OpConstant %int 0
%_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer Uniform %_ptr_PhysicalStorageBuffer_bufStruct
%int_1 = OpConstant %int 1
%int_3239 = OpConstant %int 3239
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
%main = OpFunction %void None %3
%5 = OpLabel
%17 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %u_info %int_0
%18 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %17
%22 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_1
OpReturn
OpFunctionEnd
)";
  // clang-format off

  SinglePassRunAndMatch<SwitchDescriptorSetPass>(spirv, true, 7, 31);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
