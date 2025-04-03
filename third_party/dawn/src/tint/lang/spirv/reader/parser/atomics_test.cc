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

using SpirvParser_AtomicsTest = SpirvParserTest;

TEST_F(SpirvParserDeathTest, AtomicLoad_float) {
    auto src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
        %i32 = OpTypeInt 32 1
        %f32 = OpTypeFloat 32
      %int_0 = OpConstant %i32 0
      %int_1 = OpConstant %i32 1
      %int_2 = OpConstant %i32 2
      %int_4 = OpConstant %i32 4
        %arr = OpTypeArray %f32 %int_4
    %ptr_arr = OpTypePointer Workgroup %arr
    %ptr_f32 = OpTypePointer Workgroup %f32
       %void = OpTypeVoid
         %wg = OpVariable %ptr_arr Workgroup
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %42 = OpAccessChain %ptr_f32 %wg %int_1
         %50 = OpAtomicLoad %f32 %42 %int_2 %int_0
               OpReturn
               OpFunctionEnd
)";
    EXPECT_DEATH_IF_SUPPORTED({ auto _ = Run(src); }, "internal compiler error");
}

TEST_F(SpirvParserDeathTest, AtomicStore_float) {
    auto src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
        %i32 = OpTypeInt 32 1
        %f32 = OpTypeFloat 32
      %int_0 = OpConstant %i32 0
      %int_1 = OpConstant %i32 1
      %int_2 = OpConstant %i32 2
      %int_4 = OpConstant %i32 4
      %f32_1 = OpConstant %f32 1
        %arr = OpTypeArray %f32 %int_4
    %ptr_arr = OpTypePointer Workgroup %arr
    %ptr_f32 = OpTypePointer Workgroup %f32
       %void = OpTypeVoid
         %wg = OpVariable %ptr_arr Workgroup
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %42 = OpAccessChain %ptr_f32 %wg %int_1
               OpAtomicStore %42 %int_2 %int_0 %f32_1
               OpReturn
               OpFunctionEnd
)";
    EXPECT_DEATH_IF_SUPPORTED({ auto _ = Run(src); }, "internal compiler error");
}

TEST_F(SpirvParserDeathTest, AtomicExchange_float) {
    auto src = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
        %i32 = OpTypeInt 32 1
        %f32 = OpTypeFloat 32
      %int_0 = OpConstant %i32 0
      %int_1 = OpConstant %i32 1
      %int_4 = OpConstant %i32 4
      %f32_1 = OpConstant %f32 1
        %arr = OpTypeArray %f32 %int_4
    %ptr_arr = OpTypePointer Workgroup %arr
    %ptr_f32 = OpTypePointer Workgroup %f32
       %void = OpTypeVoid
         %wg = OpVariable %ptr_arr Workgroup
    %ep_type = OpTypeFunction %void
       %main = OpFunction %void None %ep_type
         %10 = OpLabel
         %42 = OpAccessChain %ptr_f32 %wg %int_1
         %16 = OpAtomicExchange %f32 %42 %int_1 %int_0 %f32_1
               OpReturn
               OpFunctionEnd
)";
    EXPECT_DEATH_IF_SUPPORTED({ auto _ = Run(src); }, "internal compiler error");
}

TEST_F(SpirvParser_AtomicsTest, ArrayStore) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_4 = OpConstant %uint 4
      %int_1 = OpConstant %int 1
        %arr = OpTypeArray %uint %uint_4
    %ptr_arr = OpTypePointer Workgroup %arr
   %ptr_uint = OpTypePointer Workgroup %uint
       %void = OpTypeVoid
         %wg = OpVariable %ptr_arr Workgroup
         %43 = OpTypeFunction %void
       %main = OpFunction %void None %43
         %45 = OpLabel
         %42 = OpAccessChain %ptr_uint %wg %int_1
               OpAtomicStore %42 %uint_2 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %wg:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 1i
    %4:void = spirv.atomic_store %3, 2u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ArrayStore_CopiedObject) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_4 = OpConstant %uint 4
      %int_1 = OpConstant %int 1
        %arr = OpTypeArray %uint %uint_4
    %ptr_arr = OpTypePointer Workgroup %arr
   %ptr_uint = OpTypePointer Workgroup %uint
       %void = OpTypeVoid
         %wg = OpVariable %ptr_arr Workgroup
         %43 = OpTypeFunction %void
       %main = OpFunction %void None %43
         %45 = OpLabel
         %41 = OpCopyObject %ptr_arr %wg
         %42 = OpAccessChain %ptr_uint %41 %int_1
               OpAtomicStore %42 %uint_2 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %wg:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, array<u32, 4>, read_write> = let %wg
    %4:ptr<workgroup, u32, read_write> = access %3, 1i
    %5:void = spirv.atomic_store %4, 2u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ArrayNested) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
        %arr = OpTypeArray %uint %uint_1
    %arr_arr = OpTypeArray %arr %uint_2
%arr_arr_arr = OpTypeArray %arr_arr %uint_3
%ptr_arr_arr_arr = OpTypePointer Workgroup %arr_arr_arr
         %wg = OpVariable %ptr_arr_arr_arr Workgroup
   %ptr_uint = OpTypePointer Workgroup %uint
         %51 = OpConstantNull %int
         %53 = OpTypeFunction %void
       %main = OpFunction %void None %53
         %55 = OpLabel
         %52 = OpAccessChain %ptr_uint %wg %int_2 %int_1 %51
               OpAtomicStore %52 %uint_2 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %wg:ptr<workgroup, array<array<array<u32, 1>, 2>, 3>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 2i, 1i, 0i
    %4:void = spirv.atomic_store %3, 2u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, FlatSingleAtomic) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "a"
               OpMemberName %S 2 "y"
               OpName %wg "wg"
               OpName %main "main"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %int %uint %uint
      %ptr_s = OpTypePointer Workgroup %S
         %wg = OpVariable %ptr_s Workgroup
    %ptr_int = OpTypePointer Workgroup %int
   %ptr_uint = OpTypePointer Workgroup %uint
         %16 = OpConstantNull %int
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
         %23 = OpConstantNull %uint
         %31 = OpTypeFunction %void
       %main = OpFunction %void None %31
         %33 = OpLabel
         %15 = OpAccessChain %ptr_int %wg %uint_0
               OpStore %15 %16
         %22 = OpAccessChain %ptr_uint %wg %uint_1
               OpAtomicStore %22 %uint_2 %uint_0 %23
         %25 = OpAccessChain %ptr_uint %wg %uint_2
               OpStore %25 %23
               OpReturn
               OpFunctionEnd

)",
              R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
  y:u32 @offset(8)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = access %wg, 0u
    store %3, 0i
    %4:ptr<workgroup, u32, read_write> = access %wg, 1u
    %5:void = spirv.atomic_store %4, 2u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 2u
    store %6, 0u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, FlatMultipleAtomics) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "a"
               OpMemberName %S 2 "b"
               OpName %wg "wg"
               OpName %main "main"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
          %S = OpTypeStruct %int %uint %uint
      %ptr_s = OpTypePointer Workgroup %S
         %wg = OpVariable %ptr_s Workgroup
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
    %ptr_int = OpTypePointer Workgroup %int
   %ptr_uint = OpTypePointer Workgroup %uint
         %16 = OpConstantNull %int
         %23 = OpConstantNull %uint
         %35 = OpTypeFunction %void
       %main = OpFunction %void None %35
         %37 = OpLabel
         %15 = OpAccessChain %ptr_int %wg %uint_0
               OpStore %15 %16
         %22 = OpAccessChain %ptr_uint %wg %uint_1
               OpAtomicStore %22 %uint_2 %uint_0 %23
         %24 = OpAccessChain %ptr_uint %wg %uint_2
               OpAtomicStore %24 %uint_2 %uint_0 %23
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
  b:u32 @offset(8)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = access %wg, 0u
    store %3, 0i
    %4:ptr<workgroup, u32, read_write> = access %wg, 1u
    %5:void = spirv.atomic_store %4, 2u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 2u
    %7:void = spirv.atomic_store %6, 2u, 0u, 0u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, Nested) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S2 "S2"
               OpMemberName %S2 0 "x"
               OpMemberName %S2 1 "y"
               OpMemberName %S2 2 "z"
               OpMemberName %S2 3 "a"
               OpName %S1 "S1"
               OpMemberName %S1 0 "x"
               OpMemberName %S1 1 "a"
               OpName %S0 "S0"
               OpMemberName %S0 0 "x"
               OpMemberName %S0 1 "a"
               OpMemberName %S0 2 "y"
               OpMemberName %S0 3 "z"
               OpMemberName %S1 2 "y"
               OpMemberName %S1 3 "z"
               OpName %wg "wg"
               OpName %main "main"
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
         %S0 = OpTypeStruct %int %uint %int %int
         %S1 = OpTypeStruct %int %S0 %int %int
         %S2 = OpTypeStruct %int %int %int %S1
%_ptr_Workgroup_S2 = OpTypePointer Workgroup %S2
         %wg = OpVariable %_ptr_Workgroup_S2 Workgroup
       %void = OpTypeVoid
         %10 = OpTypeFunction %void %uint
     %uint_0 = OpConstant %uint 0
%_ptr_Workgroup_int = OpTypePointer Workgroup %int
         %18 = OpConstantNull %int
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
%_ptr_Workgroup_uint = OpTypePointer Workgroup %uint
         %30 = OpConstantNull %uint
   %uint_264 = OpConstant %uint 264
         %40 = OpTypeFunction %void
       %main = OpFunction %void None %40
         %42 = OpLabel
         %25 = OpAccessChain %_ptr_Workgroup_int %wg %uint_3 %uint_1 %uint_0
               OpStore %25 %18
         %29 = OpAccessChain %_ptr_Workgroup_uint %wg %uint_3 %uint_1 %uint_1
               OpAtomicStore %29 %uint_2 %uint_0 %30
         %31 = OpAccessChain %_ptr_Workgroup_int %wg %uint_3 %uint_1 %uint_2
               OpStore %31 %18
               OpReturn
               OpFunctionEnd
)",
              R"(
S0 = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
  y:i32 @offset(8)
  z:i32 @offset(12)
}

S1 = struct @align(4) {
  x:i32 @offset(0)
  a:S0 @offset(4)
  y:i32 @offset(20)
  z:i32 @offset(24)
}

S2 = struct @align(4) {
  x:i32 @offset(0)
  y:i32 @offset(4)
  z:i32 @offset(8)
  a:S1 @offset(12)
}

$B1: {  # root
  %wg:ptr<workgroup, S2, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, i32, read_write> = access %wg, 3u, 1u, 0u
    store %3, 0i
    %4:ptr<workgroup, u32, read_write> = access %wg, 3u, 1u, 1u
    %5:void = spirv.atomic_store %4, 2u, 0u, 0u
    %6:ptr<workgroup, i32, read_write> = access %wg, 3u, 1u, 2u
    store %6, 0i
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ArrayOfStruct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "a"
               OpName %wg "wg"
               OpName %main "main"
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_4 = OpConstant %int 4
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
    %uint_10 = OpConstant %uint 10
          %S = OpTypeStruct %int %uint
        %arr = OpTypeArray %S %uint_10
    %ptr_arr = OpTypePointer Workgroup %arr
   %ptr_uint = OpTypePointer Workgroup %uint
         %wg = OpVariable %ptr_arr Workgroup
         %49 = OpTypeFunction %void
       %main = OpFunction %void None %49
         %51 = OpLabel
         %48 = OpAccessChain %ptr_uint %wg %int_4 %uint_1
               OpAtomicStore %48 %uint_2 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd

)",
              R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:u32 @offset(4)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S, 10>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 4i, 1u
    %4:void = spirv.atomic_store %3, 2u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, StructOfArray) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "a"
               OpMemberName %S 2 "y"
               OpName %wg "wg"
               OpName %main "main"
       %void = OpTypeVoid
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
      %int_4 = OpConstant %int 4
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
    %uint_10 = OpConstant %uint 10
        %arr = OpTypeArray %uint %uint_10
          %S = OpTypeStruct %int %arr %uint
      %ptr_s = OpTypePointer Workgroup %S
   %ptr_uint = OpTypePointer Workgroup %uint
         %wg = OpVariable %ptr_s Workgroup
         %49 = OpTypeFunction %void
       %main = OpFunction %void None %49
         %51 = OpLabel
         %48 = OpAccessChain %ptr_uint %wg %uint_1 %int_4
               OpAtomicStore %48 %uint_2 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  x:i32 @offset(0)
  a:array<u32, 10> @offset(4)
  y:u32 @offset(44)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:ptr<workgroup, u32, read_write> = access %wg, 1u, 4i
    %4:void = spirv.atomic_store %3, 2u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicAdd) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicIAdd %int %15 %uint_1 %uint_0 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicIAdd %uint %17 %uint_1 %uint_0 %uint_1
         %19 = OpAtomicIAdd %int %wg_int %uint_1 %uint_0 %int_1
         %20 = OpAtomicIAdd %uint %wg_uint %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_i_add %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_i_add %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_i_add %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_i_add %wg_u32, 1u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicSub) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicISub %int %15 %uint_1 %uint_0 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicISub %uint %17 %uint_1 %uint_0 %uint_1
         %19 = OpAtomicISub %int %wg_int %uint_1 %uint_0 %int_1
         %20 = OpAtomicISub %uint %wg_uint %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_i_sub %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_i_sub %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_i_sub %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_i_sub %wg_u32, 1u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicAnd) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicAnd %int %15 %uint_1 %uint_0 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicAnd %uint %17 %uint_1 %uint_0 %uint_1
         %19 = OpAtomicAnd %int %wg_int %uint_1 %uint_0 %int_1
         %20 = OpAtomicAnd %uint %wg_uint %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_and %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_and %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_and %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_and %wg_u32, 1u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicOr) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicOr %int %15 %uint_1 %uint_0 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicOr %uint %17 %uint_1 %uint_0 %uint_1
         %19 = OpAtomicOr %int %wg_int %uint_1 %uint_0 %int_1
         %20 = OpAtomicOr %uint %wg_uint %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_or %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_or %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_or %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_or %wg_u32, 1u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicXor) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicXor %int %15 %uint_1 %uint_0 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicXor %uint %17 %uint_1 %uint_0 %uint_1
         %19 = OpAtomicXor %int %wg_int %uint_1 %uint_0 %int_1
         %20 = OpAtomicXor %uint %wg_uint %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_xor %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_xor %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_xor %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_xor %wg_u32, 1u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicMax) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicSMax %int %15 %uint_1 %uint_0 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicUMax %uint %17 %uint_1 %uint_0 %uint_1
         %19 = OpAtomicSMax %int %wg_int %uint_1 %uint_0 %int_1
         %20 = OpAtomicUMax %uint %wg_uint %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_s_max %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_u_max %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_s_max %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_u_max %wg_u32, 1u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicMin) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicSMin %int %15 %uint_1 %uint_0 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicUMin %uint %17 %uint_1 %uint_0 %uint_1
         %19 = OpAtomicSMin %int %wg_int %uint_1 %uint_0 %int_1
         %20 = OpAtomicUMin %uint %wg_uint %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_s_min %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_u_min %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_s_min %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_u_min %wg_u32, 1u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicExchange) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicExchange %int %15 %uint_1 %uint_0 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicExchange %uint %17 %uint_1 %uint_0 %uint_1
         %19 = OpAtomicExchange %int %wg_int %uint_1 %uint_0 %int_1
         %20 = OpAtomicExchange %uint %wg_uint %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_exchange %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_exchange %7, 1u, 0u, 1u
    %9:i32 = spirv.atomic_exchange %wg_i32, 1u, 0u, 1i
    %10:u32 = spirv.atomic_exchange %wg_u32, 1u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicCompareExchange) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicCompareExchange %int %15 %uint_1 %uint_0 %uint_0 %int_1 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicCompareExchange %uint %17 %uint_1 %uint_0 %uint_0 %uint_1 %uint_1
         %19 = OpAtomicCompareExchange %int %wg_int %uint_1 %uint_0 %uint_0 %int_1 %int_1
         %20 = OpAtomicCompareExchange %uint %wg_uint %uint_1 %uint_0 %uint_0 %uint_1 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_compare_exchange %5, 1u, 0u, 0u, 1i, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_compare_exchange %7, 1u, 0u, 0u, 1u, 1u
    %9:i32 = spirv.atomic_compare_exchange %wg_i32, 1u, 0u, 0u, 1i, 1i
    %10:u32 = spirv.atomic_compare_exchange %wg_u32, 1u, 0u, 0u, 1u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicLoad) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicLoad %int %15 %uint_1 %uint_0
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicLoad %uint %17 %uint_1 %uint_0
         %19 = OpAtomicLoad %int %wg_int %uint_1 %uint_0
         %20 = OpAtomicLoad %uint %wg_uint %uint_1 %uint_0
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_load %5, 1u, 0u
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_load %7, 1u, 0u
    %9:i32 = spirv.atomic_load %wg_i32, 1u, 0u
    %10:u32 = spirv.atomic_load %wg_u32, 1u, 0u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicStore) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
               OpAtomicStore %15 %uint_1 %uint_0 %int_1
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
               OpAtomicStore %17 %uint_1 %uint_0 %uint_1
               OpAtomicStore %wg_int %uint_1 %uint_0 %int_1
               OpAtomicStore %wg_uint %uint_1 %uint_0 %uint_1
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:void = spirv.atomic_store %5, 1u, 0u, 1i
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:void = spirv.atomic_store %7, 1u, 0u, 1u
    %9:void = spirv.atomic_store %wg_i32, 1u, 0u, 1i
    %10:void = spirv.atomic_store %wg_u32, 1u, 0u, 1u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicDecrement) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicIDecrement %int %15 %uint_1 %uint_0
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicIDecrement %uint %17 %uint_1 %uint_0
         %19 = OpAtomicIDecrement %int %wg_int %uint_1 %uint_0
         %20 = OpAtomicIDecrement %uint %wg_uint %uint_1 %uint_0
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_i_decrement %5, 1u, 0u
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_i_decrement %7, 1u, 0u
    %9:i32 = spirv.atomic_i_decrement %wg_i32, 1u, 0u
    %10:u32 = spirv.atomic_i_decrement %wg_u32, 1u, 0u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, AtomicIncrement) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %S "S"
               OpName %wg_int "wg_i32"
               OpName %wg_uint "wg_u32"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpName %sb "sb"
               OpName %main "main"
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %sb DescriptorSet 0
               OpDecorate %sb Binding 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %int %uint
      %ptr_s = OpTypePointer StorageBuffer %S
          %5 = OpTypeFunction %void
%ptr_int_storage = OpTypePointer StorageBuffer %int
%ptr_uint_storage = OpTypePointer StorageBuffer %uint
%ptr_int_workgroup = OpTypePointer Workgroup %int
%ptr_uint_workgroup = OpTypePointer Workgroup %uint
         %sb = OpVariable %ptr_s StorageBuffer
     %wg_int = OpVariable %ptr_int_workgroup Workgroup
    %wg_uint = OpVariable %ptr_uint_workgroup Workgroup
    %ptr_int = OpTypePointer Function %int
       %main = OpFunction %void None %5
          %8 = OpLabel
         %15 = OpAccessChain %ptr_int_storage %sb %uint_0
         %16 = OpAtomicIIncrement %int %15 %uint_1 %uint_0
         %17 = OpAccessChain %ptr_uint_storage %sb %uint_1
         %18 = OpAtomicIIncrement %uint %17 %uint_1 %uint_0
         %19 = OpAtomicIIncrement %int %wg_int %uint_1 %uint_0
         %20 = OpAtomicIIncrement %uint %wg_uint %uint_1 %uint_0
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

$B1: {  # root
  %sb:ptr<storage, S, read_write> = var undef @binding_point(0, 0)
  %wg_i32:ptr<workgroup, i32, read_write> = var undef
  %wg_u32:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %5:ptr<storage, i32, read_write> = access %sb, 0u
    %6:i32 = spirv.atomic_i_increment %5, 1u, 0u
    %7:ptr<storage, u32, read_write> = access %sb, 1u
    %8:u32 = spirv.atomic_i_increment %7, 1u, 0u
    %9:i32 = spirv.atomic_i_increment %wg_i32, 1u, 0u
    %10:u32 = spirv.atomic_i_increment %wg_u32, 1u, 0u
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceAssignsAndDecls_Scalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
               OpName %b "b"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %5 = OpTypeFunction %void
   %ptr_uint = OpTypePointer Workgroup %uint
         %wg = OpVariable %ptr_uint Workgroup
%ptr_uint_fn = OpTypePointer Function %uint
       %main = OpFunction %void None %5
          %8 = OpLabel
          %b = OpVariable %ptr_uint_fn Function
         %11 = OpAtomicIAdd %uint %wg %uint_1 %uint_0 %uint_0
               OpStore %wg %uint_0
         %15 = OpLoad %uint %wg
         %16 = OpCopyObject %uint %15
         %18 = OpLoad %uint %wg
               OpStore %b %18
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %wg:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:u32 = spirv.atomic_i_add %wg, 1u, 0u, 0u
    store %wg, 0u
    %5:u32 = load %wg
    %6:u32 = let %5
    %7:u32 = load %wg
    store %b, %7
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceAssignsAndDecls_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
               OpName %S "S"
               OpName %b "b"
               OpMemberName %S 0 "a"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %uint
          %5 = OpTypeFunction %void
      %ptr_s = OpTypePointer Workgroup %S
         %wg = OpVariable %ptr_s Workgroup
   %ptr_uint = OpTypePointer Workgroup %uint
%ptr_uint_fn = OpTypePointer Function %uint
       %main = OpFunction %void None %5
          %8 = OpLabel
          %b = OpVariable %ptr_uint_fn Function
          %9 = OpAccessChain %ptr_uint %wg %uint_0
         %11 = OpAtomicIAdd %uint %9 %uint_1 %uint_0 %uint_0
         %12 = OpAccessChain %ptr_uint %wg %uint_0
               OpStore %12 %uint_0
         %14 = OpAccessChain %ptr_uint %wg %uint_0
         %15 = OpLoad %uint %14
         %16 = OpCopyObject %uint %15
         %17 = OpAccessChain %ptr_uint %wg %uint_0
         %18 = OpLoad %uint %17
               OpStore %b %18
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 0u
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 0u
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 0u
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceAssignsAndDecls_NestedStruct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
               OpName %S0 "S0"
               OpName %S1 "S1"
               OpName %b "b"
               OpMemberName %S0 0 "a"
               OpMemberName %S1 0 "s0"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %uint
          %5 = OpTypeFunction %void
         %S0 = OpTypeStruct %uint
         %S1 = OpTypeStruct %S0
      %ptr_s = OpTypePointer Workgroup %S1
         %wg = OpVariable %ptr_s Workgroup
   %ptr_uint = OpTypePointer Workgroup %uint
%ptr_uint_fn = OpTypePointer Function %uint
       %main = OpFunction %void None %5
          %8 = OpLabel
          %b = OpVariable %ptr_uint_fn Function
          %9 = OpAccessChain %ptr_uint %wg %uint_0 %uint_0
         %11 = OpAtomicIAdd %uint %9 %uint_1 %uint_0 %uint_0
         %12 = OpAccessChain %ptr_uint %wg %uint_0 %uint_0
               OpStore %12 %uint_0
         %14 = OpAccessChain %ptr_uint %wg %uint_0 %uint_0
         %15 = OpLoad %uint %14
         %16 = OpCopyObject %uint %15
         %17 = OpAccessChain %ptr_uint %wg %uint_0 %uint_0
         %18 = OpLoad %uint %17
               OpStore %b %18
               OpReturn
               OpFunctionEnd
)",
              R"(
S0 = struct @align(4) {
  a:u32 @offset(0)
}

S1 = struct @align(4) {
  s0:S0 @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S1, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 0u, 0u
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 0u, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 0u, 0u
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 0u, 0u
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceAssignsAndDecls_StructMultipleAtomics) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
               OpName %S "S"
               OpName %d "d"
               OpName %e "e"
               OpName %f "f"
               OpMemberName %S 0 "a"
               OpMemberName %S 1 "b"
               OpMemberName %S 2 "c"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
          %S = OpTypeStruct %uint %uint %uint
          %5 = OpTypeFunction %void
      %ptr_s = OpTypePointer Workgroup %S
         %wg = OpVariable %ptr_s Workgroup
   %ptr_uint = OpTypePointer Workgroup %uint
%ptr_uint_fn = OpTypePointer Function %uint
       %main = OpFunction %void None %5
          %8 = OpLabel
          %d = OpVariable %ptr_uint_fn Function
          %e = OpVariable %ptr_uint_fn Function
          %f = OpVariable %ptr_uint_fn Function
          %9 = OpAccessChain %ptr_uint %wg %uint_0
         %11 = OpAtomicIAdd %uint %9 %uint_1 %uint_0 %uint_0
         %12 = OpAccessChain %ptr_uint %wg %uint_1
         %13 = OpAtomicIAdd %uint %12 %uint_1 %uint_0 %uint_0
         %14 = OpAccessChain %ptr_uint %wg %uint_0
               OpStore %14 %uint_0
         %15 = OpAccessChain %ptr_uint %wg %uint_0
         %16 = OpLoad %uint %15
         %17 = OpCopyObject %uint %16
         %18 = OpAccessChain %ptr_uint %wg %uint_0
         %19 = OpLoad %uint %18
               OpStore %d %19
         %20 = OpAccessChain %ptr_uint %wg %uint_0
               OpStore %20 %uint_0
         %21 = OpAccessChain %ptr_uint %wg %uint_1
         %22 = OpLoad %uint %21
         %99 = OpCopyObject %uint %22
         %23 = OpAccessChain %ptr_uint %wg %uint_1
         %24 = OpLoad %uint %23
               OpStore %e %24
         %25 = OpAccessChain %ptr_uint %wg %uint_2
               OpStore %25 %uint_0
         %26 = OpAccessChain %ptr_uint %wg %uint_2
         %27 = OpLoad %uint %26
         %98 = OpCopyObject %uint %27
         %28 = OpAccessChain %ptr_uint %wg %uint_2
         %29 = OpLoad %uint %28
               OpStore %f %29
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
  c:u32 @offset(8)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %d:ptr<function, u32, read_write> = var undef
    %e:ptr<function, u32, read_write> = var undef
    %f:ptr<function, u32, read_write> = var undef
    %6:ptr<workgroup, u32, read_write> = access %wg, 0u
    %7:u32 = spirv.atomic_i_add %6, 1u, 0u, 0u
    %8:ptr<workgroup, u32, read_write> = access %wg, 1u
    %9:u32 = spirv.atomic_i_add %8, 1u, 0u, 0u
    %10:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %10, 0u
    %11:ptr<workgroup, u32, read_write> = access %wg, 0u
    %12:u32 = load %11
    %13:u32 = let %12
    %14:ptr<workgroup, u32, read_write> = access %wg, 0u
    %15:u32 = load %14
    store %d, %15
    %16:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %16, 0u
    %17:ptr<workgroup, u32, read_write> = access %wg, 1u
    %18:u32 = load %17
    %19:u32 = let %18
    %20:ptr<workgroup, u32, read_write> = access %wg, 1u
    %21:u32 = load %20
    store %e, %21
    %22:ptr<workgroup, u32, read_write> = access %wg, 2u
    store %22, 0u
    %23:ptr<workgroup, u32, read_write> = access %wg, 2u
    %24:u32 = load %23
    %25:u32 = let %24
    %26:ptr<workgroup, u32, read_write> = access %wg, 2u
    %27:u32 = load %26
    store %f, %27
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceAssignsAndDecls_ArrayOfScalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
               OpName %S "S"
               OpName %b "b"
               OpMemberName %S 0 "a"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
          %S = OpTypeStruct %uint
          %5 = OpTypeFunction %void
        %arr = OpTypeArray %uint %uint_4
      %ptr_s = OpTypePointer Workgroup %arr
         %wg = OpVariable %ptr_s Workgroup
   %ptr_uint = OpTypePointer Workgroup %uint
%ptr_uint_fn = OpTypePointer Function %uint
       %main = OpFunction %void None %5
          %8 = OpLabel
          %b = OpVariable %ptr_uint_fn Function
          %9 = OpAccessChain %ptr_uint %wg %int_1
         %11 = OpAtomicIAdd %uint %9 %uint_1 %uint_0 %uint_0
         %12 = OpAccessChain %ptr_uint %wg %int_1
               OpStore %12 %uint_0
         %14 = OpAccessChain %ptr_uint %wg %int_1
         %15 = OpLoad %uint %14
         %16 = OpCopyObject %uint %15
         %17 = OpAccessChain %ptr_uint %wg %int_1
         %18 = OpLoad %uint %17
               OpStore %b %18
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %wg:ptr<workgroup, array<u32, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 1i
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 1i
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 1i
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 1i
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceAssignsAndDecls_ArrayOfStruct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
               OpName %S "S"
               OpName %b "b"
               OpMemberName %S 0 "a"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
          %S = OpTypeStruct %uint
        %arr = OpTypeArray %S %uint_4
          %5 = OpTypeFunction %void
      %ptr_s = OpTypePointer Workgroup %arr
         %wg = OpVariable %ptr_s Workgroup
   %ptr_uint = OpTypePointer Workgroup %uint
%ptr_uint_fn = OpTypePointer Function %uint
       %main = OpFunction %void None %5
          %8 = OpLabel
          %b = OpVariable %ptr_uint_fn Function
          %9 = OpAccessChain %ptr_uint %wg %int_1 %uint_0
         %11 = OpAtomicIAdd %uint %9 %uint_1 %uint_0 %uint_0
         %12 = OpAccessChain %ptr_uint %wg %int_1 %uint_0
               OpStore %12 %uint_0
         %14 = OpAccessChain %ptr_uint %wg %int_1 %uint_0
         %15 = OpLoad %uint %14
         %16 = OpCopyObject %uint %15
         %17 = OpAccessChain %ptr_uint %wg %int_1 %uint_0
         %18 = OpLoad %uint %17
               OpStore %b %18
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, array<S, 4>, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 1i, 0u
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 1i, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 1i, 0u
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<workgroup, u32, read_write> = access %wg, 1i, 0u
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceAssignsAndDecls_StructOfArray) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %sg "sg"
               OpName %main "main"
               OpName %S "S"
               OpName %b "b"
               OpMemberName %S 0 "a"
               OpDecorate %sg DescriptorSet 0
               OpDecorate %sg Binding 1
               OpDecorate %S Block
               OpDecorate %arr ArrayStride 4
               OpMemberDecorate %S 0 Offset 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_4 = OpConstant %int 4
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
        %arr = OpTypeRuntimeArray %uint
          %S = OpTypeStruct %arr
          %5 = OpTypeFunction %void
      %ptr_s = OpTypePointer StorageBuffer %S
         %sg = OpVariable %ptr_s StorageBuffer
   %ptr_uint = OpTypePointer StorageBuffer %uint
%ptr_uint_fn = OpTypePointer Function %uint
       %main = OpFunction %void None %5
          %8 = OpLabel
          %b = OpVariable %ptr_uint_fn Function
          %9 = OpAccessChain %ptr_uint %sg %uint_0 %int_4
         %11 = OpAtomicIAdd %uint %9 %uint_1 %uint_0 %uint_0
         %12 = OpAccessChain %ptr_uint %sg %uint_0 %int_4
               OpStore %12 %uint_0
         %14 = OpAccessChain %ptr_uint %sg %uint_0 %int_4
         %15 = OpLoad %uint %14
         %16 = OpCopyObject %uint %15
         %17 = OpAccessChain %ptr_uint %sg %uint_0 %int_4
         %18 = OpLoad %uint %17
               OpStore %b %18
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:array<u32> @offset(0)
}

$B1: {  # root
  %sg:ptr<storage, S, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<storage, u32, read_write> = access %sg, 0u, 4i
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<storage, u32, read_write> = access %sg, 0u, 4i
    store %6, 0u
    %7:ptr<storage, u32, read_write> = access %sg, 0u, 4i
    %8:u32 = load %7
    %9:u32 = let %8
    %10:ptr<storage, u32, read_write> = access %sg, 0u, 4i
    %11:u32 = load %10
    store %b, %11
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceAssignsAndDecls_Let) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpExtension "SPV_KHR_storage_buffer_storage_class"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %sg "s"
               OpName %main "main"
               OpName %S "S"
               OpName %b "b"
               OpMemberName %S 0 "a"
               OpDecorate %sg DescriptorSet 0
               OpDecorate %sg Binding 1
               OpDecorate %S Block
               OpMemberDecorate %S 0 Offset 0
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %uint
          %5 = OpTypeFunction %void
      %ptr_s = OpTypePointer StorageBuffer %S
         %sg = OpVariable %ptr_s StorageBuffer
   %ptr_uint = OpTypePointer StorageBuffer %uint
%ptr_uint_fn = OpTypePointer Function %uint
       %main = OpFunction %void None %5
          %8 = OpLabel
          %b = OpVariable %ptr_uint_fn Function
          %9 = OpCopyObject %ptr_s %sg
         %10 = OpAccessChain %ptr_uint %9 %uint_0
         %11 = OpCopyObject %ptr_uint %10
         %12 = OpAtomicIAdd %uint %11 %uint_1 %uint_0 %uint_0
               OpStore %11 %uint_0
         %13 = OpLoad %uint %11
         %14 = OpCopyObject %uint %13
         %15 = OpLoad %uint %11
               OpStore %b %15
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %s:ptr<storage, S, read_write> = var undef @binding_point(0, 1)
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, u32, read_write> = var undef
    %4:ptr<storage, S, read_write> = let %s
    %5:ptr<storage, u32, read_write> = access %4, 0u
    %6:ptr<storage, u32, read_write> = let %5
    %7:u32 = spirv.atomic_i_add %6, 1u, 0u, 0u
    store %6, 0u
    %8:u32 = load %6
    %9:u32 = let %8
    %10:u32 = load %6
    store %b, %10
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceBitcastArgument_Scalar) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
               OpName %b "b"
       %void = OpTypeVoid
        %f32 = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %5 = OpTypeFunction %void
    %ptr_f32 = OpTypePointer Function %f32
   %ptr_uint = OpTypePointer Workgroup %uint
         %wg = OpVariable %ptr_uint Workgroup
       %main = OpFunction %void None %5
          %8 = OpLabel
          %b = OpVariable %ptr_f32 Function
         %11 = OpAtomicIAdd %uint %wg %uint_1 %uint_0 %uint_0
               OpStore %wg %uint_0
         %12 = OpLoad %uint %wg
         %13 = OpBitcast %f32 %12
               OpStore %b %13
               OpReturn
               OpFunctionEnd
)",
              R"(
$B1: {  # root
  %wg:ptr<workgroup, u32, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, f32, read_write> = var undef
    %4:u32 = spirv.atomic_i_add %wg, 1u, 0u, 0u
    store %wg, 0u
    %5:u32 = load %wg
    %6:f32 = bitcast %5
    store %b, %6
    ret
  }
}
)");
}

TEST_F(SpirvParser_AtomicsTest, ReplaceBitcastArgument_Struct) {
    EXPECT_IR(R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
               OpName %wg "wg"
               OpName %main "main"
               OpName %S "S"
               OpName %b "b"
               OpMemberName %S 0 "a"
       %void = OpTypeVoid
       %uint = OpTypeInt 32 0
        %f32 = OpTypeFloat 32
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
          %S = OpTypeStruct %uint
          %5 = OpTypeFunction %void
      %ptr_s = OpTypePointer Workgroup %S
         %wg = OpVariable %ptr_s Workgroup
   %ptr_uint = OpTypePointer Workgroup %uint
 %ptr_f32_fn = OpTypePointer Function %f32
       %main = OpFunction %void None %5
          %8 = OpLabel
          %b = OpVariable %ptr_f32_fn Function
          %9 = OpAccessChain %ptr_uint %wg %uint_0
         %11 = OpAtomicIAdd %uint %9 %uint_1 %uint_0 %uint_0
         %12 = OpAccessChain %ptr_uint %wg %uint_0
               OpStore %12 %uint_0
         %14 = OpAccessChain %ptr_uint %wg %uint_0
         %15 = OpLoad %uint %14
         %16 = OpBitcast %f32 %15
               OpStore %b %16
               OpReturn
               OpFunctionEnd
)",
              R"(
S = struct @align(4) {
  a:u32 @offset(0)
}

$B1: {  # root
  %wg:ptr<workgroup, S, read_write> = var undef
}

%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %b:ptr<function, f32, read_write> = var undef
    %4:ptr<workgroup, u32, read_write> = access %wg, 0u
    %5:u32 = spirv.atomic_i_add %4, 1u, 0u, 0u
    %6:ptr<workgroup, u32, read_write> = access %wg, 0u
    store %6, 0u
    %7:ptr<workgroup, u32, read_write> = access %wg, 0u
    %8:u32 = load %7
    %9:f32 = bitcast %8
    store %b, %9
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::spirv::reader
