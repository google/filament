// Copyright (c) 2019 Valve Corporation
// Copyright (c) 2019 LunarG Inc.
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

#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

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

  const std::string defs_before =
      R"(OpCapability Shader
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
OpDecorate %u_info DescriptorSet 0
OpDecorate %u_info Binding 0
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
)";

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
OpCapability Int64
OpExtension "SPV_EXT_physical_storage_buffer"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
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
OpDecorate %u_info DescriptorSet 0
OpDecorate %u_info Binding 0
OpDecorate %_runtimearr_ulong ArrayStride 8
OpDecorate %_struct_39 Block
OpMemberDecorate %_struct_39 0 Offset 0
OpDecorate %41 DescriptorSet 7
OpDecorate %41 Binding 2
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_77 Block
OpMemberDecorate %_struct_77 0 Offset 0
OpMemberDecorate %_struct_77 1 Offset 4
OpDecorate %79 DescriptorSet 7
OpDecorate %79 Binding 0
OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
%void = OpTypeVoid
%8 = OpTypeFunction %void
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
%ulong = OpTypeInt 64 0
%uint_4 = OpConstant %uint 4
%bool = OpTypeBool
%28 = OpTypeFunction %bool %ulong %uint
%uint_1 = OpConstant %uint 1
%_runtimearr_ulong = OpTypeRuntimeArray %ulong
%_struct_39 = OpTypeStruct %_runtimearr_ulong
%_ptr_StorageBuffer__struct_39 = OpTypePointer StorageBuffer %_struct_39
%41 = OpVariable %_ptr_StorageBuffer__struct_39 StorageBuffer
%_ptr_StorageBuffer_ulong = OpTypePointer StorageBuffer %ulong
%uint_0 = OpConstant %uint 0
%uint_32 = OpConstant %uint 32
%70 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_77 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_77 = OpTypePointer StorageBuffer %_struct_77
%79 = OpVariable %_ptr_StorageBuffer__struct_77 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_23 = OpConstant %uint 23
%uint_5 = OpConstant %uint 5
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_9 = OpConstant %uint 9
%uint_48 = OpConstant %uint 48
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%17 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %u_info %int_0
%18 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %17
%22 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %18 %int_1
OpStore %22 %int_3239 Aligned 16
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %8
%19 = OpLabel
%20 = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_bufStruct %u_info %int_0
%21 = OpLoad %_ptr_PhysicalStorageBuffer_bufStruct %20
%22 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %21 %int_1
%24 = OpConvertPtrToU %ulong %22
%61 = OpFunctionCall %bool %26 %24 %uint_4
OpSelectionMerge %62 None
OpBranchConditional %61 %63 %64
%63 = OpLabel
OpStore %22 %int_3239 Aligned 16
OpBranch %62
%64 = OpLabel
%65 = OpUConvert %uint %24
%67 = OpShiftRightLogical %ulong %24 %uint_32
%68 = OpUConvert %uint %67
%124 = OpFunctionCall %void %69 %uint_48 %uint_2 %65 %68
OpBranch %62
%62 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%26 = OpFunction %bool None %28
%29 = OpFunctionParameter %ulong
%30 = OpFunctionParameter %uint
%31 = OpLabel
OpBranch %32
%32 = OpLabel
%34 = OpPhi %uint %uint_1 %31 %35 %33
OpLoopMerge %37 %33 None
OpBranch %33
%33 = OpLabel
%35 = OpIAdd %uint %34 %uint_1
%44 = OpAccessChain %_ptr_StorageBuffer_ulong %41 %uint_0 %35
%45 = OpLoad %ulong %44
%46 = OpUGreaterThan %bool %45 %29
OpBranchConditional %46 %37 %32
%37 = OpLabel
%47 = OpISub %uint %35 %uint_1
%48 = OpAccessChain %_ptr_StorageBuffer_ulong %41 %uint_0 %47
%49 = OpLoad %ulong %48
%50 = OpISub %ulong %29 %49
%51 = OpUConvert %ulong %30
%52 = OpIAdd %ulong %50 %51
%53 = OpAccessChain %_ptr_StorageBuffer_ulong %41 %uint_0 %uint_0
%54 = OpLoad %ulong %53
%55 = OpUConvert %uint %54
%56 = OpISub %uint %47 %uint_1
%57 = OpIAdd %uint %56 %55
%58 = OpAccessChain %_ptr_StorageBuffer_ulong %41 %uint_0 %57
%59 = OpLoad %ulong %58
%60 = OpULessThanEqual %bool %52 %59
OpReturnValue %60
OpFunctionEnd
%69 = OpFunction %void None %70
%71 = OpFunctionParameter %uint
%72 = OpFunctionParameter %uint
%73 = OpFunctionParameter %uint
%74 = OpFunctionParameter %uint
%75 = OpLabel
%81 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_0
%83 = OpAtomicIAdd %uint %81 %uint_4 %uint_0 %uint_10
%84 = OpIAdd %uint %83 %uint_10
%85 = OpArrayLength %uint %79 1
%86 = OpULessThanEqual %bool %84 %85
OpSelectionMerge %87 None
OpBranchConditional %86 %88 %87
%88 = OpLabel
%89 = OpIAdd %uint %83 %uint_0
%90 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %89
OpStore %90 %uint_10
%92 = OpIAdd %uint %83 %uint_1
%93 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %92
OpStore %93 %uint_23
%94 = OpIAdd %uint %83 %uint_2
%95 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %94
OpStore %95 %71
%98 = OpIAdd %uint %83 %uint_3
%99 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %98
OpStore %99 %uint_5
%103 = OpLoad %v3uint %gl_GlobalInvocationID
%104 = OpCompositeExtract %uint %103 0
%105 = OpCompositeExtract %uint %103 1
%106 = OpCompositeExtract %uint %103 2
%107 = OpIAdd %uint %83 %uint_4
%108 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %107
OpStore %108 %104
%109 = OpIAdd %uint %83 %uint_5
%110 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %109
OpStore %110 %105
%112 = OpIAdd %uint %83 %uint_6
%113 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %112
OpStore %113 %106
%115 = OpIAdd %uint %83 %uint_7
%116 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %115
OpStore %116 %72
%118 = OpIAdd %uint %83 %uint_8
%119 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %118
OpStore %119 %73
%121 = OpIAdd %uint %83 %uint_9
%122 = OpAccessChain %_ptr_StorageBuffer_uint %79 %uint_1 %121
OpStore %122 %74
OpBranch %87
%87 = OpLabel
OpReturn
OpFunctionEnd
)";

  // SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBuffAddrCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u);
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

  const std::string defs_before =
      R"(OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
OpExtension "SPV_EXT_physical_storage_buffer"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpSourceExtension "GL_EXT_buffer_reference"
OpName %main "main"
OpName %blockType "blockType"
OpMemberName %blockType 0 "x"
OpMemberName %blockType 1 "next"
OpName %rootBlock "rootBlock"
OpMemberName %rootBlock 0 "root"
OpName %r "r"
OpMemberDecorate %blockType 0 Offset 0
OpMemberDecorate %blockType 1 Offset 8
OpDecorate %blockType Block
OpMemberDecorate %rootBlock 0 Offset 0
OpDecorate %rootBlock Block
OpDecorate %r DescriptorSet 0
OpDecorate %r Binding 0
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

  const std::string defs_after =
      R"(OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
OpCapability Int64
OpExtension "SPV_EXT_physical_storage_buffer"
OpExtension "SPV_KHR_storage_buffer_storage_class"
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpEntryPoint GLCompute %main "main" %gl_GlobalInvocationID
OpExecutionMode %main LocalSize 1 1 1
OpSource GLSL 450
OpSourceExtension "GL_EXT_buffer_reference"
OpName %main "main"
OpName %blockType "blockType"
OpMemberName %blockType 0 "x"
OpMemberName %blockType 1 "next"
OpName %rootBlock "rootBlock"
OpMemberName %rootBlock 0 "root"
OpName %r "r"
OpMemberDecorate %blockType 0 Offset 0
OpMemberDecorate %blockType 1 Offset 8
OpDecorate %blockType Block
OpMemberDecorate %rootBlock 0 Offset 0
OpDecorate %rootBlock Block
OpDecorate %r DescriptorSet 0
OpDecorate %r Binding 0
OpDecorate %_runtimearr_ulong ArrayStride 8
OpDecorate %_struct_45 Block
OpMemberDecorate %_struct_45 0 Offset 0
OpDecorate %47 DescriptorSet 7
OpDecorate %47 Binding 2
OpDecorate %_runtimearr_uint ArrayStride 4
OpDecorate %_struct_84 Block
OpMemberDecorate %_struct_84 0 Offset 0
OpMemberDecorate %_struct_84 1 Offset 4
OpDecorate %86 DescriptorSet 7
OpDecorate %86 Binding 0
OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
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
%uint = OpTypeInt 32 0
%uint_2 = OpConstant %uint 2
%ulong = OpTypeInt 64 0
%uint_8 = OpConstant %uint 8
%bool = OpTypeBool
%34 = OpTypeFunction %bool %ulong %uint
%uint_1 = OpConstant %uint 1
%_runtimearr_ulong = OpTypeRuntimeArray %ulong
%_struct_45 = OpTypeStruct %_runtimearr_ulong
%_ptr_StorageBuffer__struct_45 = OpTypePointer StorageBuffer %_struct_45
%47 = OpVariable %_ptr_StorageBuffer__struct_45 StorageBuffer
%_ptr_StorageBuffer_ulong = OpTypePointer StorageBuffer %ulong
%uint_0 = OpConstant %uint 0
%uint_32 = OpConstant %uint 32
%77 = OpTypeFunction %void %uint %uint %uint %uint
%_runtimearr_uint = OpTypeRuntimeArray %uint
%_struct_84 = OpTypeStruct %uint %_runtimearr_uint
%_ptr_StorageBuffer__struct_84 = OpTypePointer StorageBuffer %_struct_84
%86 = OpVariable %_ptr_StorageBuffer__struct_84 StorageBuffer
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
%uint_10 = OpConstant %uint 10
%uint_4 = OpConstant %uint 4
%uint_23 = OpConstant %uint 23
%uint_5 = OpConstant %uint 5
%uint_3 = OpConstant %uint 3
%v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_9 = OpConstant %uint 9
%uint_44 = OpConstant %uint 44
%132 = OpConstantNull %ulong
%uint_46 = OpConstant %uint 46
)";

  const std::string func_before =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_blockType %r %int_0
%17 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %16
%21 = OpAccessChain %_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_blockType %17 %int_1
%22 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %21 Aligned 8
%26 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %22 %int_0
OpStore %26 %int_531 Aligned 16
OpReturn
OpFunctionEnd
)";

  const std::string func_after =
      R"(%main = OpFunction %void None %3
%5 = OpLabel
%16 = OpAccessChain %_ptr_StorageBuffer__ptr_PhysicalStorageBuffer_blockType %r %int_0
%17 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %16
%21 = OpAccessChain %_ptr_PhysicalStorageBuffer__ptr_PhysicalStorageBuffer_blockType %17 %int_1
%30 = OpConvertPtrToU %ulong %21
%67 = OpFunctionCall %bool %32 %30 %uint_8
OpSelectionMerge %68 None
OpBranchConditional %67 %69 %70
%69 = OpLabel
%71 = OpLoad %_ptr_PhysicalStorageBuffer_blockType %21 Aligned 8
OpBranch %68
%70 = OpLabel
%72 = OpUConvert %uint %30
%74 = OpShiftRightLogical %ulong %30 %uint_32
%75 = OpUConvert %uint %74
%131 = OpFunctionCall %void %76 %uint_44 %uint_2 %72 %75
%133 = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_blockType %132
OpBranch %68
%68 = OpLabel
%134 = OpPhi %_ptr_PhysicalStorageBuffer_blockType %71 %69 %133 %70
%26 = OpAccessChain %_ptr_PhysicalStorageBuffer_int %134 %int_0
%135 = OpConvertPtrToU %ulong %26
%136 = OpFunctionCall %bool %32 %135 %uint_4
OpSelectionMerge %137 None
OpBranchConditional %136 %138 %139
%138 = OpLabel
OpStore %26 %int_531 Aligned 16
OpBranch %137
%139 = OpLabel
%140 = OpUConvert %uint %135
%141 = OpShiftRightLogical %ulong %135 %uint_32
%142 = OpUConvert %uint %141
%144 = OpFunctionCall %void %76 %uint_46 %uint_2 %140 %142
OpBranch %137
%137 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string new_funcs =
      R"(%32 = OpFunction %bool None %34
%35 = OpFunctionParameter %ulong
%36 = OpFunctionParameter %uint
%37 = OpLabel
OpBranch %38
%38 = OpLabel
%40 = OpPhi %uint %uint_1 %37 %41 %39
OpLoopMerge %43 %39 None
OpBranch %39
%39 = OpLabel
%41 = OpIAdd %uint %40 %uint_1
%50 = OpAccessChain %_ptr_StorageBuffer_ulong %47 %uint_0 %41
%51 = OpLoad %ulong %50
%52 = OpUGreaterThan %bool %51 %35
OpBranchConditional %52 %43 %38
%43 = OpLabel
%53 = OpISub %uint %41 %uint_1
%54 = OpAccessChain %_ptr_StorageBuffer_ulong %47 %uint_0 %53
%55 = OpLoad %ulong %54
%56 = OpISub %ulong %35 %55
%57 = OpUConvert %ulong %36
%58 = OpIAdd %ulong %56 %57
%59 = OpAccessChain %_ptr_StorageBuffer_ulong %47 %uint_0 %uint_0
%60 = OpLoad %ulong %59
%61 = OpUConvert %uint %60
%62 = OpISub %uint %53 %uint_1
%63 = OpIAdd %uint %62 %61
%64 = OpAccessChain %_ptr_StorageBuffer_ulong %47 %uint_0 %63
%65 = OpLoad %ulong %64
%66 = OpULessThanEqual %bool %58 %65
OpReturnValue %66
OpFunctionEnd
%76 = OpFunction %void None %77
%78 = OpFunctionParameter %uint
%79 = OpFunctionParameter %uint
%80 = OpFunctionParameter %uint
%81 = OpFunctionParameter %uint
%82 = OpLabel
%88 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_0
%91 = OpAtomicIAdd %uint %88 %uint_4 %uint_0 %uint_10
%92 = OpIAdd %uint %91 %uint_10
%93 = OpArrayLength %uint %86 1
%94 = OpULessThanEqual %bool %92 %93
OpSelectionMerge %95 None
OpBranchConditional %94 %96 %95
%96 = OpLabel
%97 = OpIAdd %uint %91 %uint_0
%98 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %97
OpStore %98 %uint_10
%100 = OpIAdd %uint %91 %uint_1
%101 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %100
OpStore %101 %uint_23
%102 = OpIAdd %uint %91 %uint_2
%103 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %102
OpStore %103 %78
%106 = OpIAdd %uint %91 %uint_3
%107 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %106
OpStore %107 %uint_5
%111 = OpLoad %v3uint %gl_GlobalInvocationID
%112 = OpCompositeExtract %uint %111 0
%113 = OpCompositeExtract %uint %111 1
%114 = OpCompositeExtract %uint %111 2
%115 = OpIAdd %uint %91 %uint_4
%116 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %115
OpStore %116 %112
%117 = OpIAdd %uint %91 %uint_5
%118 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %117
OpStore %118 %113
%120 = OpIAdd %uint %91 %uint_6
%121 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %120
OpStore %121 %114
%123 = OpIAdd %uint %91 %uint_7
%124 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %123
OpStore %124 %79
%125 = OpIAdd %uint %91 %uint_8
%126 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %125
OpStore %126 %80
%128 = OpIAdd %uint %91 %uint_9
%129 = OpAccessChain %_ptr_StorageBuffer_uint %86 %uint_1 %128
OpStore %129 %81
OpBranch %95
%95 = OpLabel
OpReturn
OpFunctionEnd
)";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);
  SinglePassRunAndCheck<InstBuffAddrCheckPass>(
      defs_before + func_before, defs_after + func_after + new_funcs, true,
      true, 7u, 23u);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
