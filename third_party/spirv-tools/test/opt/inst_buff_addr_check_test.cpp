// Copyright (c) 2019-2022 Valve Corporation
// Copyright (c) 2019-2022 LunarG Inc.
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

static const std::string kFuncName = "inst_buff_addr_search_and_test";
static const std::string kImportDeco = R"(
;CHECK: OpDecorate %)" + kFuncName + R"( LinkageAttributes ")" +
                                       kFuncName + R"(" Import
)";
static const std::string kImportStub = R"(
;CHECK: %)" + kFuncName + R"( = OpFunction %bool None {{%\w+}}
;CHECK: OpFunctionEnd
)";
// clang-format on

using InstBuffAddrTest = PassTest<::testing::Test>;

TEST_F(InstBuffAddrTest, InstPhysicalStorageBufferStore) {
  // #version 450
  // #extension GL_EXT_buffer_reference : enable
  //
  // layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
  //
  // layout(set = 0, binding = 0) uniform ufoo {
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

  const std::string defs = R"(
OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
;CHECK: OpCapability Int64
OpExtension "SPV_EXT_physical_storage_buffer"
;CHECK: OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main"
;CHECK: OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
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
)";

  // clang-format off
  const std::string decorates = R"(
OpMemberDecorate %ufoo 0 Offset 0
OpMemberDecorate %ufoo 1 Offset 8
OpDecorate %ufoo Block
OpDecorate %_arr_int_uint_2 ArrayStride 16
OpMemberDecorate %bufStruct 0 Offset 0
OpMemberDecorate %bufStruct 1 Offset 32
OpDecorate %bufStruct Block
OpDecorate %u_info DescriptorSet 0
OpDecorate %u_info Binding 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
)";

  const std::string globals = R"(
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
;CHECK: %ulong = OpTypeInt 64 0
;CHECK: %bool = OpTypeBool
;CHECK: %v3uint = OpTypeVector %uint 3
;CHECK: %_ptr_Input_v3uint = OpTypePointer Input %v3uint
;CHECK: %gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
)";
// clang-format off

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%17 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %u_info %int_0
%18 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %17
%22 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_1
;CHECK-NOT: %17 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %u_info %int_0
;CHECK-NOT: %18 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %17
;CHECK-NOT: %22 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_1
;CHECK: %20 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %u_info %int_0
;CHECK: %21 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %20
;CHECK: %22 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %21 %int_1
;CHECK: {{%\w+}} = OpConvertPtrToU %ulong %22
;CHECK: {{%\w+}} = OpLoad %v3uint %gl_GlobalInvocationID
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_49 {{%\w+}} {{%\w+}} %uint_4
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpStore %22 %int_3239 Aligned 16
;CHECK: OpStore %22 %int_3239 Aligned 16
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBuffAddrCheckPass>(
      defs + decorates + globals + kImportStub + main_func, true, 23u);
}

TEST_F(InstBuffAddrTest, InstPhysicalStorageBufferLoadAndStore) {
  // #version 450
  // #extension GL_EXT_buffer_reference : enable

  // // forward reference
  // layout(buffer_reference) buffer blockType;

  // layout(buffer_reference, std430, buffer_reference_align = 16) buffer
  // blockType {
  //   int x;
  //   blockType next;
  // };

  // layout(std430) buffer rootBlock {
  //   blockType root;
  // } r;

  // void main()
  // {
  //   blockType b = r.root;
  //   b = b.next;
  //   b.x = 531;
  // }

  const std::string defs = R"(
OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
;CHECK: OpCapability Int64
OpExtension "SPV_EXT_physical_storage_buffer"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpSourceExtension "GL_EXT_buffer_reference"
OpName %main "main"
;CHECK: OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
OpName %blockType "blockType"
OpMemberName %blockType 0 "x"
OpMemberName %blockType 1 "next"
OpName %rootBlock "rootBlock"
OpMemberName %rootBlock 0 "root"
OpName %r "r"
)";

// clang-format off
  const std::string decorates = R"(
OpMemberDecorate %blockType 0 Offset 0
OpMemberDecorate %blockType 1 Offset 8
OpDecorate %blockType Block
OpMemberDecorate %rootBlock 0 Offset 0
OpDecorate %rootBlock Block
OpDecorate %r DescriptorSet 0
OpDecorate %r Binding 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
)";

  const std::string globals = R"(
%void = OpTypeVoid
%3 = OpTypeFunction %void
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_blockType PhysicalStorageBuffer
%int = OpTypeInt 32 1
%blockType = OpTypeStruct %int %_ptr_PhysicalStorageBuffer_blockType
%_ptr_PhysicalStorageBuffer_blockType = OpTypePointer PhysicalStorageBuffer %blockType
%rootBlock = OpTypeStruct %_ptr_PhysicalStorageBuffer_blockType
%_ptr_StorageBuffer_rootBlock = OpTypePointer StorageBuffer %rootBlock
%r = OpVariable %_ptr_StorageBuffer_rootBlock StorageBuffer
%int_0 = OpConstant %int 0
%_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_blockType = OpTypePointer StorageBuffer %_ptr_PhysicalStorageBuffer_blockType
%int_1 = OpConstant %int 1
%_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_blockType = OpTypePointer PhysicalStorageBuffer %_ptr_PhysicalStorageBuffer_blockType
%int_531 = OpConstant %int 531
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_blockType %r %int_0
%17 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %16
%21 = OpAccessChain %_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_blockType %17 %int_1
%22 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %21 Aligned 8
%26 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %22 %int_0
OpStore %26 %int_531 Aligned 16
;CHECK-NOT: %22 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %21 Aligned 8
;CHECK-NOT: %26 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %22 %int_0
;CHECK: {{%\w+}} = OpConvertPtrToU %ulong %21
;CHECK: {{%\w+}} = OpLoad %v3uint %gl_GlobalInvocationID
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_45 {{%\w+}} {{%\w+}} %uint_8
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %_ptr_PhysicalStorageBuffer_blockType %21 Aligned 8
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_blockType %52
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpPhi %_ptr_PhysicalStorageBuffer_blockType {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: %26 = OpAccessChain %_ptr_PhysicalStorageBuffer_int {{%\w+}} %int_0
;CHECK: {{%\w+}} = OpConvertPtrToU %ulong %26
;CHECK: {{%\w+}} = OpLoad %v3uint %gl_GlobalInvocationID
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 2
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_5 {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_47 {{%\w+}} {{%\w+}} %uint_4
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %26 %int_531 Aligned 16
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBuffAddrCheckPass>(
      defs + decorates + globals + kImportStub + main_func, true, 23u);
}

TEST_F(InstBuffAddrTest, StructLoad) {
  // #version 450
  //   #extension GL_EXT_buffer_reference : enable
  //   #extension GL_ARB_gpu_shader_int64 : enable
  //   struct Test {
  //   float a;
  // };
  //
  // layout(buffer_reference, std430, buffer_reference_align = 16) buffer
  // TestBuffer { Test test; };
  //
  // Test GetTest(uint64_t ptr) {
  //   return TestBuffer(ptr).test;
  // }
  //
  // void main() {
  //   GetTest(0xe0000000);
  // }

  const std::string defs =
      R"(
OpCapability Shader
OpCapability Int64
OpCapability PhysicalStorageBufferAddresses
;CHECK: OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Fragment %main "main"
;CHECK: OpEntryPoint Fragment %main "main" %gl_FragCoord
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_ARB_gpu_shader_int64"
OpSourceExtension "GL_EXT_buffer_reference"
OpName %main "main"
OpName %Test "Test"
OpMemberName %Test 0 "a"
OpName %Test_0 "Test"
OpMemberName %Test_0 0 "a"
OpName %TestBuffer "TestBuffer"
OpMemberName %TestBuffer 0 "test"
)";

  // clang-format off
  const std::string decorates = R"(
OpMemberDecorate %Test_0 0 Offset 0
OpMemberDecorate %TestBuffer 0 Offset 0
OpDecorate %TestBuffer Block
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_FragCoord BuiltIn FragCoord
)";

  const std::string globals = R"(
%void = OpTypeVoid
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%float = OpTypeFloat 32
%Test = OpTypeStruct %float
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_TestBuffer PhysicalStorageBuffer
%Test_0 = OpTypeStruct %float
%TestBuffer = OpTypeStruct %Test_0
%_ptr_PhysicalStorageBuffer_TestBuffer = OpTypePointer PhysicalStorageBuffer %TestBuffer
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_PhysicalStorageBuffer_Test_0 = OpTypePointer PhysicalStorageBuffer %Test_0
%ulong_18446744073172680704 = OpConstant %ulong 18446744073172680704
;CHECK: {{%\w+}} = OpConstantNull %Test_0
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%37 = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_TestBuffer %ulong_18446744073172680704
%38 = OpAccessChain %_ptr_PhysicalStorageBuffer_Test_0 %37 %int_0
%39 = OpLoad %Test_0 %38 Aligned 16
;CHECK-NOT: %39 = OpLoad %Test_0 %38 Aligned 16
;CHECK: {{%\w+}} = OpConvertPtrToU %ulong %38
;CHECK: {{%\w+}} = OpLoad %v4float %gl_FragCoord
;CHECK: {{%\w+}} = OpBitcast %v4uint {{%\w+}}
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 0
;CHECK: {{%\w+}} = OpCompositeExtract %uint {{%\w+}} 1
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_4 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_38 {{%\w+}} {{%\w+}} %uint_4
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %Test_0 %38 Aligned 16
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %Test_0 {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
%40 = OpCopyLogical %Test %39
;CHECK-NOT: %40 = OpCopyLogical %Test %39
;CHECK: %40 = OpCopyLogical %Test [[phi_result]]
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBuffAddrCheckPass>(
      defs + decorates + globals + kImportStub + main_func, true);
}

TEST_F(InstBuffAddrTest, PaddedStructLoad) {
  // #version 450
  // #extension GL_EXT_buffer_reference : enable
  // #extension GL_ARB_gpu_shader_int64 : enable
  // struct Test {
  //   uvec3 pad_1;  // Offset 0 Size 12
  //   double pad_2; // Offset 16 Size 8 (alignment requirement)
  //   float a;      // Offset 24 Size 4
  // }; // Total Size 28
  //
  // layout(buffer_reference, std430, buffer_reference_align = 16) buffer
  // TestBuffer { Test test; };
  //
  // Test GetTest(uint64_t ptr) {
  //   return TestBuffer(ptr).test;
  // }
  //
  // void main() {
  //   GetTest(0xe0000000);
  // }

  const std::string defs =
      R"(
OpCapability Shader
OpCapability Float64
OpCapability Int64
OpCapability PhysicalStorageBufferAddresses
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Vertex %main "main"
OpSource GLSL 450
OpSourceExtension "GL_ARB_gpu_shader_int64"
OpSourceExtension "GL_EXT_buffer_reference"
OpName %main "main"
OpName %Test "Test"
OpMemberName %Test 0 "pad_1"
OpMemberName %Test 1 "pad_2"
OpMemberName %Test 2 "a"
OpName %GetTest_u641_ "GetTest(u641;"
OpName %ptr "ptr"
OpName %Test_0 "Test"
OpMemberName %Test_0 0 "pad_1"
OpMemberName %Test_0 1 "pad_2"
OpMemberName %Test_0 2 "a"
OpName %TestBuffer "TestBuffer"
OpMemberName %TestBuffer 0 "test"
OpName %param "param"
)";

  // clang-format off
  const std::string decorates = R"(
OpDecorate %TestBuffer Block
OpMemberDecorate %Test_0 0 Offset 0
OpMemberDecorate %Test_0 1 Offset 16
OpMemberDecorate %Test_0 2 Offset 24
OpMemberDecorate %TestBuffer 0 Offset 0
)" + kImportDeco + R"(
;CHECK: OpDecorate %gl_VertexIndex BuiltIn VertexIndex
;CHECK: OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
)";

  const std::string globals = R"(
%void = OpTypeVoid
%3 = OpTypeFunction %void
%ulong = OpTypeInt 64 0
%_ptr_Function_ulong = OpTypePointer Function %ulong
%uint = OpTypeInt 32 0
%v3uint = OpTypeVector %uint 3
%double = OpTypeFloat 64
%float = OpTypeFloat 32
%Test = OpTypeStruct %v3uint %double %float
%13 = OpTypeFunction %Test %_ptr_Function_ulong
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_TestBuffer PhysicalStorageBuffer
%Test_0 = OpTypeStruct %v3uint %double %float
%TestBuffer = OpTypeStruct %Test_0
%_ptr_PhysicalStorageBuffer_TestBuffer = OpTypePointer PhysicalStorageBuffer %TestBuffer
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_PhysicalStorageBuffer_Test_0 = OpTypePointer PhysicalStorageBuffer %Test_0
%_ptr_Function_Test = OpTypePointer Function %Test
%ulong_18446744073172680704 = OpConstant %ulong 18446744073172680704
;CHECK: {{%\w+}} = OpConstantNull %Test_0
)";

  const std::string main_func = R"(
%main = OpFunction %void None %3
%5 = OpLabel
%param = OpVariable %_ptr_Function_ulong Function
OpStore %param %ulong_18446744073172680704
%35 = OpFunctionCall %Test %GetTest_u641_ %param
OpReturn
OpFunctionEnd
%GetTest_u641_ = OpFunction %Test None %13
%ptr = OpFunctionParameter %_ptr_Function_ulong
%16 = OpLabel
%28 = OpVariable %_ptr_Function_Test Function
%17 = OpLoad %ulong %ptr
%21 = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_TestBuffer %17
%25 = OpAccessChain %_ptr_PhysicalStorageBuffer_Test_0 %21 %int_0
%26 = OpLoad %Test_0 %25 Aligned 16
%29 = OpCopyLogical %Test %26
;CHECK-NOT: %30 = OpLoad %Test %28
;CHECK-NOT: %26 = OpLoad %Test_0 %25 Aligned 16
;CHECK-NOT: %29 = OpCopyLogical %Test %26
;CHECK: {{%\w+}} = OpConvertPtrToU %ulong %25
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_63 {{%\w+}} {{%\w+}} %uint_28
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %Test_0 %25 Aligned 16
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %Test_0 {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: %29 = OpCopyLogical %Test [[phi_result]]
OpStore %28 %29
%30 = OpLoad %Test %28
OpReturnValue %30
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBuffAddrCheckPass>(
      defs + decorates + globals + kImportStub + main_func, true);
}

TEST_F(InstBuffAddrTest, DeviceBufferAddressOOB) {
  // #version 450
  // #extension GL_EXT_buffer_reference : enable
  //  layout(buffer_reference, buffer_reference_align = 16) buffer bufStruct;
  // layout(set = 0, binding = 0) uniform ufoo {
  //     bufStruct data;
  //     int nWrites;
  // } u_info;
  // layout(buffer_reference, std140) buffer bufStruct {
  //     int a[4];
  // };
  // void main() {
  //     for (int i=0; i < u_info.nWrites; ++i) {
  //         u_info.data.a[i] = 0xdeadca71;
  //     }
  // }

  // clang-format off
  const std::string text = R"(
OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Vertex %main "main" %u_info
;CHECK: OpEntryPoint Vertex %main "main" %u_info %gl_VertexIndex %gl_InstanceIndex
OpSource GLSL 450
OpSourceExtension "GL_EXT_buffer_reference"
OpName %main "main"
OpName %i "i"
OpName %ufoo "ufoo"
OpMemberName %ufoo 0 "data"
OpMemberName %ufoo 1 "nWrites"
OpName %bufStruct "bufStruct"
OpMemberName %bufStruct 0 "a"
OpName %u_info "u_info"
OpMemberDecorate %ufoo 0 Offset 0
OpMemberDecorate %ufoo 1 Offset 8
OpDecorate %ufoo Block
OpDecorate %_arr_int_uint_4 ArrayStride 16
OpMemberDecorate %bufStruct 0 Offset 0
OpDecorate %bufStruct Block
OpDecorate %u_info DescriptorSet 0
OpDecorate %u_info Binding 0
)" + kImportDeco + R"(
%void = OpTypeVoid
%3 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_bufStruct PhysicalStorageBuffer
%ufoo = OpTypeStruct %_ptr_PhysicalStorageBuffer_bufStruct %int
%uint = OpTypeInt 32 0
%uint_4 = OpConstant %uint 4
%_arr_int_uint_4 = OpTypeArray %int %uint_4
%bufStruct = OpTypeStruct %_arr_int_uint_4
%_ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer PhysicalStorageBuffer %bufStruct
%_ptr_Uniform_ufoo = OpTypePointer Uniform %ufoo
%u_info = OpVariable %_ptr_Uniform_ufoo Uniform
%int_1 = OpConstant %int 1
%_ptr_Uniform_int = OpTypePointer Uniform %int
%bool = OpTypeBool
%_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct = OpTypePointer Uniform %_ptr_PhysicalStorageBuffer_bufStruct
%int_n559035791 = OpConstant %int -559035791
%_ptr_PhysicalStorageBuffer_int = OpTypePointer PhysicalStorageBuffer %int
)" + kImportStub + R"(
%main = OpFunction %void None %3
%5 = OpLabel
%i = OpVariable %_ptr_Function_int Function
OpStore %i %int_0
OpBranch %10
%10 = OpLabel
OpLoopMerge %12 %13 None
OpBranch %14
%14 = OpLabel
%15 = OpLoad %int %i
%26 = OpAccessChain %_ptr_Uniform_int %u_info %int_1
%27 = OpLoad %int %26
%29 = OpSLessThan %bool %15 %27
OpBranchConditional %29 %11 %12
%11 = OpLabel
%31 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %u_info %int_0
%32 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %31
%33 = OpLoad %int %i
%36 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %32 %int_0 %33
OpStore %36 %int_n559035791 Aligned 16
;CHECK: {{%\w+}} = OpConvertPtrToU %ulong %36
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: {{%\w+}} = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_63 {{%\w+}} {{%\w+}} %uint_4
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpStore %36 %int_n559035791 Aligned 16
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
OpBranch %13
%13 = OpLabel
%37 = OpLoad %int %i
%38 = OpIAdd %int %37 %int_1
OpStore %i %38
OpBranch %10
%12 = OpLabel
OpReturn
OpFunctionEnd)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndMatch<InstBuffAddrCheckPass>(text, true, 23);
}

TEST_F(InstBuffAddrTest, UVec3ScalarAddressOOB) {
  // clang-format off
  // #version 450
  //    #extension GL_EXT_buffer_reference : enable
  //    #extension GL_EXT_scalar_block_layout : enable
  //    layout(buffer_reference, std430, scalar) readonly buffer IndexBuffer
  //    {
  //        uvec3 indices[];
  //    };
  //    layout(set = 0, binding = 0) uniform ufoo {
  //        IndexBuffer data;
  //        int nReads;
  //    } u_info;
  //    void main() {
  //        uvec3 readvec;
  //        for (int i=0; i < u_info.nReads; ++i) {
  //            readvec = u_info.data.indices[i];
  //        }
  //    }
  const std::string text = R"(
OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint Vertex %main "main" %u_info
OpSource GLSL 450
OpSourceExtension "GL_EXT_buffer_reference"
OpSourceExtension "GL_EXT_scalar_block_layout"
OpName %main "main"
OpName %i "i"
OpName %ufoo "ufoo"
OpMemberName %ufoo 0 "data"
OpMemberName %ufoo 1 "nReads"
OpName %IndexBuffer "IndexBuffer"
OpMemberName %IndexBuffer 0 "indices"
OpName %u_info "u_info"
OpName %readvec "readvec"
OpMemberDecorate %ufoo 0 Offset 0
OpMemberDecorate %ufoo 1 Offset 8
OpDecorate %ufoo Block
OpDecorate %_runtimearr_v3uint ArrayStride 12
OpMemberDecorate %IndexBuffer 0 NonWritable
OpMemberDecorate %IndexBuffer 0 Offset 0
OpDecorate %IndexBuffer Block
OpDecorate %u_info DescriptorSet 0
OpDecorate %u_info Binding 0
)" + kImportDeco + R"(
%void = OpTypeVoid
%3 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
OpTypeForwardPointer %_ptr_PhysicalStorageBuffer_IndexBuffer PhysicalStorageBuffer
%ufoo = OpTypeStruct %_ptr_PhysicalStorageBuffer_IndexBuffer %int
%uint = OpTypeInt 32 0
%v3uint = OpTypeVector %uint 3
%_runtimearr_v3uint = OpTypeRuntimeArray %v3uint
%IndexBuffer = OpTypeStruct %_runtimearr_v3uint
%_ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer PhysicalStorageBuffer %IndexBuffer
%_ptr_Uniform_ufoo = OpTypePointer Uniform %ufoo
%u_info = OpVariable %_ptr_Uniform_ufoo Uniform
%int_1 = OpConstant %int 1
%_ptr_Uniform_int = OpTypePointer Uniform %int
%bool = OpTypeBool
)" + kImportStub + R"(
%_ptr_Function_v3uint = OpTypePointer Function %v3uint
%_ptr_Uniform__ptr_PhysicalStorageBuffer_IndexBuffer = OpTypePointer Uniform %_ptr_PhysicalStorageBuffer_IndexBuffer
%_ptr_PhysicalStorageBuffer_v3uint = OpTypePointer PhysicalStorageBuffer %v3uint
%main = OpFunction %void None %3
%5 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%readvec = OpVariable %_ptr_Function_v3uint Function
OpStore %i %int_0
OpBranch %10
%10 = OpLabel
OpLoopMerge %12 %13 None
OpBranch %14
%14 = OpLabel
%15 = OpLoad %int %i
%26 = OpAccessChain %_ptr_Uniform_int %u_info %int_1
%27 = OpLoad %int %26
%29 = OpSLessThan %bool %15 %27
OpBranchConditional %29 %11 %12
%11 = OpLabel
%33 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_IndexBuffer %u_info %int_0
%34 = OpLoad %_ptr_PhysicalStorageBuffer_IndexBuffer %33
%35 = OpLoad %int %i
%37 = OpAccessChain %_ptr_PhysicalStorageBuffer_v3uint %34 %int_0 %35
%38 = OpLoad %v3uint %37 Aligned 4
OpStore %readvec %38
;CHECK-NOT: %38 = OpLoad %v3uint %37 Aligned 4
;CHECK-NOT: OpStore %readvec %38
;CHECK: {{%\w+}} = OpConvertPtrToU %ulong %37
;CHECK: {{%\w+}} = OpLoad %uint %gl_VertexIndex
;CHECK: {{%\w+}} = OpLoad %uint %gl_InstanceIndex
;CHECK: {{%\w+}} = OpCompositeConstruct %v4uint %uint_0 {{%\w+}} {{%\w+}} %uint_0
;CHECK: [[test_result:%\w+]] = OpFunctionCall %bool %)" + kFuncName + R"( %uint_23 %uint_67 {{%\w+}} {{%\w+}} %uint_12
;CHECK: OpSelectionMerge {{%\w+}} None
;CHECK: OpBranchConditional [[test_result]] {{%\w+}} {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: {{%\w+}} = OpLoad %v3uint %37 Aligned 4
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: OpBranch {{%\w+}}
;CHECK: {{%\w+}} = OpLabel
;CHECK: [[phi_result:%\w+]] = OpPhi %v3uint {{%\w+}} {{%\w+}} {{%\w+}} {{%\w+}}
;CHECK: OpStore %readvec [[phi_result]]
OpBranch %13
%13 = OpLabel
%39 = OpLoad %int %i
%40 = OpIAdd %int %39 %int_1
OpStore %i %40
OpBranch %10
%12 = OpLabel
OpReturn
OpFunctionEnd
)";
  // clang-format on

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_4);
  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  ValidatorOptions()->scalar_block_layout = true;
  SinglePassRunAndMatch<InstBuffAddrCheckPass>(text, true, 23);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
