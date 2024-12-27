// Copyright (c) 2017 Google Inc.
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

#include "source/opt/scalar_replacement_pass.h"
#include "test/opt/assembly_builder.h"
#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using ScalarReplacementPassName = ::testing::Test;

TEST_F(ScalarReplacementPassName, Default) {
  auto srp = ScalarReplacementPass();
  EXPECT_STREQ(srp.name(), "scalar-replacement=100");
}

TEST_F(ScalarReplacementPassName, Large) {
  auto srp = ScalarReplacementPass(0xffffffffu);
  EXPECT_STREQ(srp.name(), "scalar-replacement=4294967295");
}

using ScalarReplacementTest = PassTest<::testing::Test>;

TEST_F(ScalarReplacementTest, SimpleStruct) {
  const std::string text = R"(
;
; CHECK: [[struct:%\w+]] = OpTypeStruct [[elem:%\w+]]
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: [[elem_ptr:%\w+]] = OpTypePointer Function [[elem]]
; CHECK: OpConstantNull [[struct]]
; CHECK: [[null:%\w+]] = OpConstantNull [[elem]]
; CHECK-NOT: OpVariable [[struct_ptr]]
; CHECK: [[one:%\w+]] = OpVariable [[elem_ptr]] Function [[null]]
; CHECK-NEXT: [[two:%\w+]] = OpVariable [[elem_ptr]] Function [[null]]
; CHECK-NOT: OpVariable [[elem_ptr]] Function [[null]]
; CHECK-NOT: OpVariable [[struct_ptr]]
; CHECK-NOT: OpInBoundsAccessChain
; CHECK: [[l1:%\w+]] = OpLoad [[elem]] [[two]]
; CHECK-NOT: OpAccessChain
; CHECK: [[l2:%\w+]] = OpLoad [[elem]] [[one]]
; CHECK: OpIAdd [[elem]] [[l1]] [[l2]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %6 "simple_struct"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeStruct %2 %2 %2 %2
%4 = OpTypePointer Function %3
%5 = OpTypePointer Function %2
%6 = OpTypeFunction %2
%7 = OpConstantNull %3
%8 = OpConstant %2 0
%9 = OpConstant %2 1
%10 = OpConstant %2 2
%11 = OpConstant %2 3
%12 = OpFunction %2 None %6
%13 = OpLabel
%14 = OpVariable %4 Function %7
%15 = OpInBoundsAccessChain %5 %14 %8
%16 = OpLoad %2 %15
%17 = OpAccessChain %5 %14 %10
%18 = OpLoad %2 %17
%19 = OpIAdd %2 %16 %18
OpReturnValue %19
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, StructInitialization) {
  const std::string text = R"(
;
; CHECK: [[elem:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct:%\w+]] = OpTypeStruct [[elem]] [[elem]] [[elem]] [[elem]]
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: [[elem_ptr:%\w+]] = OpTypePointer Function [[elem]]
; CHECK: [[zero:%\w+]] = OpConstant [[elem]] 0
; CHECK: [[undef:%\w+]] = OpUndef [[elem]]
; CHECK: [[two:%\w+]] = OpConstant [[elem]] 2
; CHECK: [[null:%\w+]] = OpConstantNull [[elem]]
; CHECK-NOT: OpVariable [[struct_ptr]]
; CHECK: OpVariable [[elem_ptr]] Function [[null]]
; CHECK-NEXT: OpVariable [[elem_ptr]] Function [[two]]
; CHECK-NOT: OpVariable [[elem_ptr]] Function [[undef]]
; CHECK-NEXT: OpVariable [[elem_ptr]] Function
; CHECK-NEXT: OpVariable [[elem_ptr]] Function [[zero]]
; CHECK-NOT: OpVariable [[elem_ptr]] Function [[undef]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %6 "struct_init"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeStruct %2 %2 %2 %2
%4 = OpTypePointer Function %3
%20 = OpTypePointer Function %2
%6 = OpTypeFunction %1
%7 = OpConstant %2 0
%8 = OpUndef %2
%9 = OpConstant %2 2
%30 = OpConstant %2 1
%31 = OpConstant %2 3
%10 = OpConstantNull %2
%11 = OpConstantComposite %3 %7 %8 %9 %10
%12 = OpFunction %1 None %6
%13 = OpLabel
%14 = OpVariable %4 Function %11
%15 = OpAccessChain %20 %14 %7
OpStore %15 %10
%16 = OpAccessChain %20 %14 %9
OpStore %16 %10
%17 = OpAccessChain %20 %14 %30
OpStore %17 %10
%18 = OpAccessChain %20 %14 %31
OpStore %18 %10
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, SpecConstantInitialization) {
  const std::string text = R"(
;
; CHECK: [[int:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct:%\w+]] = OpTypeStruct [[int]] [[int]]
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: [[int_ptr:%\w+]] = OpTypePointer Function [[int]]
; CHECK: [[spec_comp:%\w+]] = OpSpecConstantComposite [[struct]]
; CHECK: [[ex0:%\w+]] = OpSpecConstantOp [[int]] CompositeExtract [[spec_comp]] 0
; CHECK: [[ex1:%\w+]] = OpSpecConstantOp [[int]] CompositeExtract [[spec_comp]] 1
; CHECK-NOT: OpVariable [[struct]]
; CHECK: OpVariable [[int_ptr]] Function [[ex1]]
; CHECK-NEXT: OpVariable [[int_ptr]] Function [[ex0]]
; CHECK-NOT: OpVariable [[struct]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %6 "spec_const"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeStruct %2 %2
%4 = OpTypePointer Function %3
%20 = OpTypePointer Function %2
%5 = OpTypeFunction %1
%6 = OpConstant %2 0
%30 = OpConstant %2 1
%7 = OpSpecConstant %2 0
%8 = OpSpecConstantOp %2 IAdd %7 %7
%9 = OpSpecConstantComposite %3 %7 %8
%10 = OpFunction %1 None %5
%11 = OpLabel
%12 = OpVariable %4 Function %9
%13 = OpAccessChain %20 %12 %6
%14 = OpLoad %2 %13
%15 = OpAccessChain %20 %12 %30
%16 = OpLoad %2 %15
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

// TODO(alanbaker): Re-enable when vector and matrix scalarization is supported.
// TEST_F(ScalarReplacementTest, VectorInitialization) {
//  const std::string text = R"(
// ;
// ; CHECK: [[elem:%\w+]] = OpTypeInt 32 0
// ; CHECK: [[vector:%\w+]] = OpTypeVector [[elem]] 4
// ; CHECK: [[vector_ptr:%\w+]] = OpTypePointer Function [[vector]]
// ; CHECK: [[elem_ptr:%\w+]] = OpTypePointer Function [[elem]]
// ; CHECK: [[zero:%\w+]] = OpConstant [[elem]] 0
// ; CHECK: [[undef:%\w+]] = OpUndef [[elem]]
// ; CHECK: [[two:%\w+]] = OpConstant [[elem]] 2
// ; CHECK: [[null:%\w+]] = OpConstantNull [[elem]]
// ; CHECK-NOT: OpVariable [[vector_ptr]]
// ; CHECK: OpVariable [[elem_ptr]] Function [[zero]]
// ; CHECK-NOT: OpVariable [[elem_ptr]] Function [[undef]]
// ; CHECK-NEXT: OpVariable [[elem_ptr]] Function
// ; CHECK-NEXT: OpVariable [[elem_ptr]] Function [[two]]
// ; CHECK-NEXT: OpVariable [[elem_ptr]] Function [[null]]
// ; CHECK-NOT: OpVariable [[elem_ptr]] Function [[undef]]
// ;
//  OpCapability Shader
//  OpCapability Linkage
//  OpMemoryModel Logical GLSL450
//  OpName %6 "vector_init"
// %1 = OpTypeVoid
// %2 = OpTypeInt 32 0
// %3 = OpTypeVector %2 4
// %4 = OpTypePointer Function %3
// %20 = OpTypePointer Function %2
// %6 = OpTypeFunction %1
// %7 = OpConstant %2 0
// %8 = OpUndef %2
// %9 = OpConstant %2 2
// %30 = OpConstant %2 1
// %31 = OpConstant %2 3
// %10 = OpConstantNull %2
// %11 = OpConstantComposite %3 %10 %9 %8 %7
// %12 = OpFunction %1 None %6
// %13 = OpLabel
// %14 = OpVariable %4 Function %11
// %15 = OpAccessChain %20 %14 %7
//  OpStore %15 %10
// %16 = OpAccessChain %20 %14 %9
//  OpStore %16 %10
// %17 = OpAccessChain %20 %14 %30
//  OpStore %17 %10
// %18 = OpAccessChain %20 %14 %31
//  OpStore %18 %10
//  OpReturn
//  OpFunctionEnd
//   )";
//
//   SinglePassRunAndMatch<opt::ScalarReplacementPass>(text, true);
// }
//
//  TEST_F(ScalarReplacementTest, MatrixInitialization) {
//   const std::string text = R"(
// ;
// ; CHECK: [[float:%\w+]] = OpTypeFloat 32
// ; CHECK: [[vector:%\w+]] = OpTypeVector [[float]] 2
// ; CHECK: [[matrix:%\w+]] = OpTypeMatrix [[vector]] 2
// ; CHECK: [[matrix_ptr:%\w+]] = OpTypePointer Function [[matrix]]
// ; CHECK: [[float_ptr:%\w+]] = OpTypePointer Function [[float]]
// ; CHECK: [[vec_ptr:%\w+]] = OpTypePointer Function [[vector]]
// ; CHECK: [[zerof:%\w+]] = OpConstant [[float]] 0
// ; CHECK: [[onef:%\w+]] = OpConstant [[float]] 1
// ; CHECK: [[one_zero:%\w+]] = OpConstantComposite [[vector]] [[onef]]
// [[zerof]] ; CHECK: [[zero_one:%\w+]] = OpConstantComposite [[vector]]
// [[zerof]] [[onef]] ; CHECK: [[const_mat:%\w+]] = OpConstantComposite
// [[matrix]] [[one_zero]]
// [[zero_one]] ; CHECK-NOT: OpVariable [[matrix]] ; CHECK-NOT: OpVariable
// [[vector]] Function [[one_zero]] ; CHECK: [[f1:%\w+]] = OpVariable
// [[float_ptr]] Function [[zerof]] ; CHECK-NEXT: [[f2:%\w+]] = OpVariable
// [[float_ptr]] Function [[onef]] ; CHECK-NEXT: [[vec_var:%\w+]] = OpVariable
// [[vec_ptr]] Function [[zero_one]] ; CHECK-NOT: OpVariable [[matrix]] ;
//  CHECK-NOT: OpVariable [[vector]] Function [[one_zero]]
// ;
//  OpCapability Shader
//  OpCapability Linkage
//  OpMemoryModel Logical GLSL450
//  OpName %7 "matrix_init"
// %1 = OpTypeVoid
// %2 = OpTypeFloat 32
// %3 = OpTypeVector %2 2
// %4 = OpTypeMatrix %3 2
// %5 = OpTypePointer Function %4
// %6 = OpTypePointer Function %2
// %30 = OpTypePointer Function %3
// %10 = OpTypeInt 32 0
// %7 = OpTypeFunction %1 %10
// %8 = OpConstant %2 0.0
// %9 = OpConstant %2 1.0
// %11 = OpConstant %10 0
// %12 = OpConstant %10 1
// %13 = OpConstantComposite %3 %9 %8
// %14 = OpConstantComposite %3 %8 %9
// %15 = OpConstantComposite %4 %13 %14
// %16 = OpFunction %1 None %7
// %31 = OpFunctionParameter %10
// %17 = OpLabel
// %18 = OpVariable %5 Function %15
// %19 = OpAccessChain %6 %18 %11 %12
//  OpStore %19 %8
// %20 = OpAccessChain %6 %18 %11 %11
//  OpStore %20 %8
// %21 = OpAccessChain %30 %18 %12
//  OpStore %21 %14
//  OpReturn
//  OpFunctionEnd
//   )";
//
//   SinglePassRunAndMatch<opt::ScalarReplacementPass>(text, true);
// }

TEST_F(ScalarReplacementTest, ElideAccessChain) {
  const std::string text = R"(
;
; CHECK: [[var:%\w+]] = OpVariable
; CHECK-NOT: OpAccessChain
; CHECK: OpStore [[var]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %6 "elide_access_chain"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeStruct %2 %2 %2 %2
%4 = OpTypePointer Function %3
%20 = OpTypePointer Function %2
%6 = OpTypeFunction %1
%7 = OpConstant %2 0
%8 = OpUndef %2
%9 = OpConstant %2 2
%10 = OpConstantNull %2
%11 = OpConstantComposite %3 %7 %8 %9 %10
%12 = OpFunction %1 None %6
%13 = OpLabel
%14 = OpVariable %4 Function %11
%15 = OpAccessChain %20 %14 %7
OpStore %15 %10
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ElideMultipleAccessChains) {
  const std::string text = R"(
;
; CHECK: [[var:%\w+]] = OpVariable
; CHECK-NOT: OpInBoundsAccessChain
; CHECK OpStore [[var]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %6 "elide_two_access_chains"
%1 = OpTypeVoid
%2 = OpTypeFloat 32
%3 = OpTypeStruct %2 %2
%4 = OpTypeStruct %3 %3
%5 = OpTypePointer Function %4
%6 = OpTypePointer Function %2
%7 = OpTypeFunction %1
%8 = OpConstant %2 0.0
%9 = OpConstant %2 1.0
%10 = OpTypeInt 32 0
%11 = OpConstant %10 0
%12 = OpConstant %10 1
%13 = OpConstantComposite %3 %9 %8
%14 = OpConstantComposite %3 %8 %9
%15 = OpConstantComposite %4 %13 %14
%16 = OpFunction %1 None %7
%17 = OpLabel
%18 = OpVariable %5 Function %15
%19 = OpInBoundsAccessChain %6 %18 %11 %12
OpStore %19 %8
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ReplaceAccessChain) {
  const std::string text = R"(
;
; CHECK: [[param:%\w+]] = OpFunctionParameter
; CHECK: [[var:%\w+]] = OpVariable
; CHECK: [[access:%\w+]] = OpAccessChain {{%\w+}} [[var]] [[param]]
; CHECK: OpStore [[access]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %7 "replace_access_chain"
%1 = OpTypeVoid
%2 = OpTypeFloat 32
%10 = OpTypeInt 32 0
%uint_2 = OpConstant %10 2
%3 = OpTypeArray %2 %uint_2
%4 = OpTypeStruct %3 %3
%5 = OpTypePointer Function %4
%20 = OpTypePointer Function %3
%6 = OpTypePointer Function %2
%7 = OpTypeFunction %1 %10
%8 = OpConstant %2 0.0
%9 = OpConstant %2 1.0
%11 = OpConstant %10 0
%12 = OpConstant %10 1
%13 = OpConstantComposite %3 %9 %8
%14 = OpConstantComposite %3 %8 %9
%15 = OpConstantComposite %4 %13 %14
%16 = OpFunction %1 None %7
%32 = OpFunctionParameter %10
%17 = OpLabel
%18 = OpVariable %5 Function %15
%19 = OpAccessChain %6 %18 %11 %32
OpStore %19 %8
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ArrayInitialization) {
  const std::string text = R"(
;
; CHECK: [[float:%\w+]] = OpTypeFloat 32
; CHECK: [[array:%\w+]] = OpTypeArray
; CHECK: [[array_ptr:%\w+]] = OpTypePointer Function [[array]]
; CHECK: [[float_ptr:%\w+]] = OpTypePointer Function [[float]]
; CHECK: [[float0:%\w+]] = OpConstant [[float]] 0
; CHECK: [[float1:%\w+]] = OpConstant [[float]] 1
; CHECK: [[float2:%\w+]] = OpConstant [[float]] 2
; CHECK-NOT: OpVariable [[array_ptr]]
; CHECK: [[var0:%\w+]] = OpVariable [[float_ptr]] Function [[float0]]
; CHECK-NEXT: [[var1:%\w+]] = OpVariable [[float_ptr]] Function [[float1]]
; CHECK-NEXT: [[var2:%\w+]] = OpVariable [[float_ptr]] Function [[float2]]
; CHECK-NOT: OpVariable [[array_ptr]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "array_init"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%float_array = OpTypeArray %float %uint_3
%array_ptr = OpTypePointer Function %float_array
%float_ptr = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%const_array = OpConstantComposite %float_array %float_2 %float_1 %float_0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%3 = OpVariable %array_ptr Function %const_array
%4 = OpInBoundsAccessChain %float_ptr %3 %uint_0
OpStore %4 %float_0
%5 = OpInBoundsAccessChain %float_ptr %3 %uint_1
OpStore %5 %float_0
%6 = OpInBoundsAccessChain %float_ptr %3 %uint_2
OpStore %6 %float_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, NonUniformCompositeInitialization) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[long:%\w+]] = OpTypeInt 64 1
; CHECK: [[dvector:%\w+]] = OpTypeVector
; CHECK: [[vector:%\w+]] = OpTypeVector
; CHECK: [[array:%\w+]] = OpTypeArray
; CHECK: [[matrix:%\w+]] = OpTypeMatrix
; CHECK: [[struct1:%\w+]] = OpTypeStruct [[uint]] [[vector]]
; CHECK: [[struct2:%\w+]] = OpTypeStruct [[struct1]] [[matrix]] [[array]] [[uint]]
; CHECK: [[struct1_ptr:%\w+]] = OpTypePointer Function [[struct1]]
; CHECK: [[matrix_ptr:%\w+]] = OpTypePointer Function [[matrix]]
; CHECK: [[array_ptr:%\w+]] = OpTypePointer Function [[array]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[struct2_ptr:%\w+]] = OpTypePointer Function [[struct2]]
; CHECK: [[const_array:%\w+]] = OpConstantComposite [[array]]
; CHECK: [[const_matrix:%\w+]] = OpConstantNull [[matrix]]
; CHECK: [[const_struct1:%\w+]] = OpConstantComposite [[struct1]]
; CHECK: OpUndef [[uint]]
; CHECK: OpUndef [[vector]]
; CHECK: OpUndef [[long]]
; CHECK: OpFunction
; CHECK-NOT: OpVariable [[struct2_ptr]] Function
; CHECK: OpVariable [[uint_ptr]] Function
; CHECK-NEXT: OpVariable [[matrix_ptr]] Function [[const_matrix]]
; CHECK-NOT: OpVariable [[struct1_ptr]] Function [[const_struct1]]
; CHECK-NOT: OpVariable [[struct2_ptr]] Function
;
OpCapability Shader
OpCapability Linkage
OpCapability Int64
OpCapability Float64
OpMemoryModel Logical GLSL450
OpName %func "non_uniform_composite_init"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%int64 = OpTypeInt 64 1
%float = OpTypeFloat 32
%double = OpTypeFloat 64
%double2 = OpTypeVector %double 2
%float4 = OpTypeVector %float 4
%int64_0 = OpConstant %int64 0
%int64_1 = OpConstant %int64 1
%int64_2 = OpConstant %int64 2
%int64_3 = OpConstant %int64 3
%int64_array3 = OpTypeArray %int64 %int64_3
%matrix_double2 = OpTypeMatrix %double2 2
%struct1 = OpTypeStruct %uint %float4
%struct2 = OpTypeStruct %struct1 %matrix_double2 %int64_array3 %uint
%struct1_ptr = OpTypePointer Function %struct1
%matrix_double2_ptr = OpTypePointer Function %matrix_double2
%int64_array_ptr = OpTypePointer Function %int64_array3
%uint_ptr = OpTypePointer Function %uint
%struct2_ptr = OpTypePointer Function %struct2
%const_uint = OpConstant %uint 0
%const_int64_array = OpConstantComposite %int64_array3 %int64_0 %int64_1 %int64_2
%const_double2 = OpConstantNull %double2
%const_matrix_double2 = OpConstantNull %matrix_double2
%undef_float4 = OpUndef %float4
%const_struct1 = OpConstantComposite %struct1 %const_uint %undef_float4
%const_struct2 = OpConstantComposite %struct2 %const_struct1 %const_matrix_double2 %const_int64_array %const_uint
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct2_ptr Function %const_struct2
%3 = OpAccessChain %struct1_ptr %var %int64_0
OpStore %3 %const_struct1
%4 = OpAccessChain %matrix_double2_ptr %var %int64_1
OpStore %4 %const_matrix_double2
%5 = OpAccessChain %int64_array_ptr %var %int64_2
OpStore %5 %const_int64_array
%6 = OpAccessChain %uint_ptr %var %int64_3
OpStore %6 %const_uint
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ElideUncombinedAccessChains) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[const:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[var:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK-NOT: OpAccessChain
; CHECK: OpStore [[var]] [[const]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "elide_uncombined_access_chains"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint
%struct2 = OpTypeStruct %struct1
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%struct2_ptr = OpTypePointer Function %struct2
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct2_ptr Function
%3 = OpAccessChain %struct1_ptr %var %uint_0
%4 = OpAccessChain %uint_ptr %3 %uint_0
OpStore %4 %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ElideSingleUncombinedAccessChains) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[array:%\w+]] = OpTypeArray [[uint]]
; CHECK: [[array_ptr:%\w+]] = OpTypePointer Function [[array]]
; CHECK: [[const:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[param:%\w+]] = OpFunctionParameter [[uint]]
; CHECK: [[var:%\w+]] = OpVariable [[array_ptr]] Function
; CHECK: [[access:%\w+]] = OpAccessChain {{.*}} [[var]] [[param]]
; CHECK: OpStore [[access]] [[const]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "elide_single_uncombined_access_chains"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%array = OpTypeArray %uint %uint_1
%struct2 = OpTypeStruct %array
%uint_ptr = OpTypePointer Function %uint
%array_ptr = OpTypePointer Function %array
%struct2_ptr = OpTypePointer Function %struct2
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void %uint
%1 = OpFunction %void None %func
%param = OpFunctionParameter %uint
%2 = OpLabel
%var = OpVariable %struct2_ptr Function
%3 = OpAccessChain %array_ptr %var %uint_0
%4 = OpAccessChain %uint_ptr %3 %param
OpStore %4 %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ReplaceWholeLoad) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct1:%\w+]] = OpTypeStruct [[uint]] [[uint]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[const:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[var1:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[var0:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[l1:%\w+]] = OpLoad [[uint]] [[var1]]
; CHECK: [[l0:%\w+]] = OpLoad [[uint]] [[var0]]
; CHECK: OpCompositeConstruct [[struct1]] [[l0]] [[l1]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "replace_whole_load"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%load = OpLoad %struct1 %var
%3 = OpAccessChain %uint_ptr %var %uint_0
OpStore %3 %uint_0
%4 = OpAccessChain %uint_ptr %var %uint_1
OpStore %4 %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ReplaceWholeLoadCopyMemoryAccess) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct1:%\w+]] = OpTypeStruct [[uint]] [[uint]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[undef:%\w+]] = OpUndef [[uint]]
; CHECK: [[var0:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[l0:%\w+]] = OpLoad [[uint]] [[var0]] Nontemporal
; CHECK: OpCompositeConstruct [[struct1]] [[l0]] [[undef]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "replace_whole_load_copy_memory_access"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%load = OpLoad %struct1 %var Nontemporal
%3 = OpAccessChain %uint_ptr %var %uint_0
OpStore %3 %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ReplaceWholeStore) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct1:%\w+]] = OpTypeStruct [[uint]] [[uint]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[const:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[const_struct:%\w+]] = OpConstantComposite [[struct1]] [[const]] [[const]]
; CHECK: [[var0:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[ex0:%\w+]] = OpCompositeExtract [[uint]] [[const_struct]] 0
; CHECK: OpStore [[var0]] [[ex0]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "replace_whole_store"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%const_struct = OpConstantComposite %struct1 %uint_0 %uint_0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
OpStore %var %const_struct
%3 = OpAccessChain %uint_ptr %var %uint_0
%4 = OpLoad %uint %3
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ReplaceWholeStoreCopyMemoryAccess) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct1:%\w+]] = OpTypeStruct [[uint]] [[uint]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[const:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[const_struct:%\w+]] = OpConstantComposite [[struct1]] [[const]] [[const]]
; CHECK: [[var0:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK-NOT: OpVariable
; CHECK: [[ex0:%\w+]] = OpCompositeExtract [[uint]] [[const_struct]] 0
; CHECK: OpStore [[var0]] [[ex0]] Aligned 4
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "replace_whole_store_copy_memory_access"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%const_struct = OpConstantComposite %struct1 %uint_0 %uint_0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
OpStore %var %const_struct Aligned 4
%3 = OpAccessChain %uint_ptr %var %uint_0
%4 = OpLoad %uint %3
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DontTouchVolatileLoad) {
  const std::string text = R"(
;
; CHECK: [[struct:%\w+]] = OpTypeStruct
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: OpLabel
; CHECK-NEXT: OpVariable [[struct_ptr]]
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "dont_touch_volatile_load"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%3 = OpAccessChain %uint_ptr %var %uint_0
%4 = OpLoad %uint %3 Volatile
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DontTouchVolatileStore) {
  const std::string text = R"(
;
; CHECK: [[struct:%\w+]] = OpTypeStruct
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: OpLabel
; CHECK-NEXT: OpVariable [[struct_ptr]]
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "dont_touch_volatile_store"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%3 = OpAccessChain %uint_ptr %var %uint_0
OpStore %3 %uint_0 Volatile
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DontTouchSpecNonFunctionVariable) {
  const std::string text = R"(
;
; CHECK: [[struct:%\w+]] = OpTypeStruct
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Uniform [[struct]]
; CHECK: OpConstant
; CHECK-NEXT: OpVariable [[struct_ptr]]
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "dont_touch_spec_constant_access_chain"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint
%uint_ptr = OpTypePointer Uniform %uint
%struct1_ptr = OpTypePointer Uniform %struct1
%uint_0 = OpConstant %uint 0
%var = OpVariable %struct1_ptr Uniform
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%3 = OpAccessChain %uint_ptr %var %uint_0
OpStore %3 %uint_0 Volatile
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DontTouchSpecConstantAccessChain) {
  const std::string text = R"(
;
; CHECK: [[array:%\w+]] = OpTypeArray
; CHECK: [[array_ptr:%\w+]] = OpTypePointer Function [[array]]
; CHECK: OpLabel
; CHECK-NEXT: OpVariable [[array_ptr]]
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "dont_touch_spec_constant_access_chain"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%array = OpTypeArray %uint %uint_1
%uint_ptr = OpTypePointer Function %uint
%array_ptr = OpTypePointer Function %array
%uint_0 = OpConstant %uint 0
%spec_const = OpSpecConstant %uint 0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %array_ptr Function
%3 = OpAccessChain %uint_ptr %var %spec_const
OpStore %3 %uint_0 Volatile
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, NoPartialAccesses) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: OpLabel
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "no_partial_accesses"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%const = OpConstantNull %struct1
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
OpStore %var %const
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DontTouchPtrAccessChain) {
  const std::string text = R"(
;
; CHECK: [[struct:%\w+]] = OpTypeStruct
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: OpLabel
; CHECK-NEXT: OpVariable [[struct_ptr]]
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "dont_touch_ptr_access_chain"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%3 = OpPtrAccessChain %uint_ptr %var %uint_0 %uint_0
OpStore %3 %uint_0
%4 = OpAccessChain %uint_ptr %var %uint_0
OpStore %4 %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, false);
}

TEST_F(ScalarReplacementTest, DontTouchInBoundsPtrAccessChain) {
  const std::string text = R"(
;
; CHECK: [[struct:%\w+]] = OpTypeStruct
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: OpLabel
; CHECK-NEXT: OpVariable [[struct_ptr]]
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "dont_touch_in_bounds_ptr_access_chain"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%3 = OpInBoundsPtrAccessChain %uint_ptr %var %uint_0 %uint_0
OpStore %3 %uint_0
%4 = OpInBoundsAccessChain %uint_ptr %var %uint_0
OpStore %4 %uint_0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, false);
}

TEST_F(ScalarReplacementTest, DonTouchAliasedDecoration) {
  const std::string text = R"(
;
; CHECK: [[struct:%\w+]] = OpTypeStruct
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: OpLabel
; CHECK-NEXT: OpVariable [[struct_ptr]]
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "aliased"
OpDecorate %var Aliased
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%3 = OpAccessChain %uint_ptr %var %uint_0
%4 = OpLoad %uint %3
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, CopyRestrictDecoration) {
  const std::string text = R"(
;
; CHECK: OpName
; CHECK-NEXT: OpDecorate [[var0:%\w+]] Restrict
; CHECK-NEXT: OpDecorate [[var1:%\w+]] Restrict
; CHECK: [[int:%\w+]] = OpTypeInt
; CHECK: [[struct:%\w+]] = OpTypeStruct
; CHECK: [[int_ptr:%\w+]] = OpTypePointer Function [[int]]
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: OpLabel
; CHECK-NEXT: [[var1]] = OpVariable [[int_ptr]]
; CHECK-NEXT: [[var0]] = OpVariable [[int_ptr]]
; CHECK-NOT: OpVariable [[struct_ptr]]
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "restrict"
OpDecorate %var Restrict
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%3 = OpAccessChain %uint_ptr %var %uint_0
%4 = OpLoad %uint %3
%5 = OpAccessChain %uint_ptr %var %uint_1
%6 = OpLoad %uint %5
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DontClobberDecoratesOnSubtypes) {
  const std::string text = R"(
;
; CHECK: OpDecorate [[array:%\w+]] ArrayStride 1
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[array]] = OpTypeArray [[uint]]
; CHECK: [[array_ptr:%\w+]] = OpTypePointer Function [[array]]
; CHECK: OpLabel
; CHECK-NEXT: OpVariable [[array_ptr]] Function
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "array_stride"
OpDecorate %array ArrayStride 1
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%array = OpTypeArray %uint %uint_1
%struct1 = OpTypeStruct %array
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void %uint
%1 = OpFunction %void None %func
%param = OpFunctionParameter %uint
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%3 = OpAccessChain %uint_ptr %var %uint_0 %param
%4 = OpLoad %uint %3
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DontCopyMemberDecorate) {
  const std::string text = R"(
;
; CHECK-NOT: OpDecorate
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct:%\w+]] = OpTypeStruct [[uint]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[struct_ptr:%\w+]] = OpTypePointer Function [[struct]]
; CHECK: OpLabel
; CHECK-NEXT: OpVariable [[uint_ptr]] Function
; CHECK-NOT: OpVariable
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "member_decorate"
OpMemberDecorate %struct1 0 Offset 1
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_1 = OpConstant %uint 1
%struct1 = OpTypeStruct %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%func = OpTypeFunction %void %uint
%1 = OpFunction %void None %func
%2 = OpLabel
%var = OpVariable %struct1_ptr Function
%3 = OpAccessChain %uint_ptr %var %uint_0
%4 = OpLoad %uint %3
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, NoPartialAccesses2) {
  const std::string text = R"(
;
; CHECK: [[float:%\w+]] = OpTypeFloat 32
; CHECK: [[float_ptr:%\w+]] = OpTypePointer Function [[float]]
; CHECK: OpVariable [[float_ptr]] Function
; CHECK: OpVariable [[float_ptr]] Function
; CHECK: OpVariable [[float_ptr]] Function
; CHECK: OpVariable [[float_ptr]] Function
; CHECK: OpVariable [[float_ptr]] Function
; CHECK: OpVariable [[float_ptr]] Function
; CHECK: OpVariable [[float_ptr]] Function
; CHECK-NOT: OpVariable
;
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %fo
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 430
OpName %main "main"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %ts1 "ts1"
OpName %S_0 "S"
OpMemberName %S_0 0 "x"
OpMemberName %S_0 1 "y"
OpName %U_t "U_t"
OpMemberName %U_t 0 "g_s1"
OpMemberName %U_t 1 "g_s2"
OpMemberName %U_t 2 "g_s3"
OpName %_ ""
OpName %ts2 "ts2"
OpName %_Globals_ "_Globals_"
OpMemberName %_Globals_ 0 "g_b"
OpName %__0 ""
OpName %ts3 "ts3"
OpName %ts4 "ts4"
OpName %fo "fo"
OpMemberDecorate %S_0 0 Offset 0
OpMemberDecorate %S_0 1 Offset 4
OpMemberDecorate %U_t 0 Offset 0
OpMemberDecorate %U_t 1 Offset 8
OpMemberDecorate %U_t 2 Offset 16
OpDecorate %U_t BufferBlock
OpDecorate %_ DescriptorSet 0
OpMemberDecorate %_Globals_ 0 Offset 0
OpDecorate %_Globals_ Block
OpDecorate %__0 DescriptorSet 0
OpDecorate %__0 Binding 0
OpDecorate %fo Location 0
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float = OpTypeFloat 32
%S = OpTypeStruct %float %float
%_ptr_Function_S = OpTypePointer Function %S
%S_0 = OpTypeStruct %float %float
%U_t = OpTypeStruct %S_0 %S_0 %S_0
%_ptr_Uniform_U_t = OpTypePointer Uniform %U_t
%_ = OpVariable %_ptr_Uniform_U_t Uniform
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_S_0 = OpTypePointer Uniform %S_0
%_ptr_Function_float = OpTypePointer Function %float
%int_1 = OpConstant %int 1
%uint = OpTypeInt 32 0
%_Globals_ = OpTypeStruct %uint
%_ptr_Uniform__Globals_ = OpTypePointer Uniform %_Globals_
%__0 = OpVariable %_ptr_Uniform__Globals_ Uniform
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%bool = OpTypeBool
%uint_0 = OpConstant %uint 0
%_ptr_Output_float = OpTypePointer Output %float
%fo = OpVariable %_ptr_Output_float Output
%main = OpFunction %void None %15
%30 = OpLabel
%ts1 = OpVariable %_ptr_Function_S Function
%ts2 = OpVariable %_ptr_Function_S Function
%ts3 = OpVariable %_ptr_Function_S Function
%ts4 = OpVariable %_ptr_Function_S Function
%31 = OpAccessChain %_ptr_Uniform_S_0 %_ %int_0
%32 = OpLoad %S_0 %31
%33 = OpCompositeExtract %float %32 0
%34 = OpAccessChain %_ptr_Function_float %ts1 %int_0
OpStore %34 %33
%35 = OpCompositeExtract %float %32 1
%36 = OpAccessChain %_ptr_Function_float %ts1 %int_1
OpStore %36 %35
%37 = OpAccessChain %_ptr_Uniform_S_0 %_ %int_1
%38 = OpLoad %S_0 %37
%39 = OpCompositeExtract %float %38 0
%40 = OpAccessChain %_ptr_Function_float %ts2 %int_0
OpStore %40 %39
%41 = OpCompositeExtract %float %38 1
%42 = OpAccessChain %_ptr_Function_float %ts2 %int_1
OpStore %42 %41
%43 = OpAccessChain %_ptr_Uniform_uint %__0 %int_0
%44 = OpLoad %uint %43
%45 = OpINotEqual %bool %44 %uint_0
OpSelectionMerge %46 None
OpBranchConditional %45 %47 %48
%47 = OpLabel
%49 = OpLoad %S %ts1
OpStore %ts3 %49
OpBranch %46
%48 = OpLabel
%50 = OpLoad %S %ts2
OpStore %ts3 %50
OpBranch %46
%46 = OpLabel
%51 = OpLoad %S %ts3
OpStore %ts4 %51
%52 = OpAccessChain %_ptr_Function_float %ts4 %int_1
%53 = OpLoad %float %52
OpStore %fo %53
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ReplaceWholeLoadAndStore) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct1:%\w+]] = OpTypeStruct [[uint]] [[uint]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[const:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[undef:%\w+]] = OpUndef [[uint]]
; CHECK: [[var0:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[var1:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK-NOT: OpVariable
; CHECK: [[l0:%\w+]] = OpLoad [[uint]] [[var0]]
; CHECK: [[c0:%\w+]] = OpCompositeConstruct [[struct1]] [[l0]] [[undef]]
; CHECK: [[e0:%\w+]] = OpCompositeExtract [[uint]] [[c0]] 0
; CHECK: OpStore [[var1]] [[e0]]
; CHECK: [[l1:%\w+]] = OpLoad [[uint]] [[var1]]
; CHECK: [[c1:%\w+]] = OpCompositeConstruct [[struct1]] [[l1]] [[undef]]
; CHECK: [[e1:%\w+]] = OpCompositeExtract [[uint]] [[c1]] 0
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "replace_whole_load"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var2 = OpVariable %struct1_ptr Function
%var1 = OpVariable %struct1_ptr Function
%load1 = OpLoad %struct1 %var1
OpStore %var2 %load1
%load2 = OpLoad %struct1 %var2
%3 = OpCompositeExtract %uint %load2 0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ReplaceWholeLoadAndStore2) {
  // TODO: We can improve this case by ensuring that |var2| is processed first.
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct1:%\w+]] = OpTypeStruct [[uint]] [[uint]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[const:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[undef:%\w+]] = OpUndef [[uint]]
; CHECK: [[var1:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[var0a:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[var0b:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK-NOT: OpVariable
; CHECK: [[l0a:%\w+]] = OpLoad [[uint]] [[var0a]]
; CHECK: [[l0b:%\w+]] = OpLoad [[uint]] [[var0b]]
; CHECK: [[c0:%\w+]] = OpCompositeConstruct [[struct1]] [[l0b]] [[l0a]]
; CHECK: [[e0:%\w+]] = OpCompositeExtract [[uint]] [[c0]] 0
; CHECK: OpStore [[var1]] [[e0]]
; CHECK: [[l1:%\w+]] = OpLoad [[uint]] [[var1]]
; CHECK: [[c1:%\w+]] = OpCompositeConstruct [[struct1]] [[l1]] [[undef]]
; CHECK: [[e1:%\w+]] = OpCompositeExtract [[uint]] [[c1]] 0
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "replace_whole_load"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct1 = OpTypeStruct %uint %uint
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var1 = OpVariable %struct1_ptr Function
%var2 = OpVariable %struct1_ptr Function
%load1 = OpLoad %struct1 %var1
OpStore %var2 %load1
%load2 = OpLoad %struct1 %var2
%3 = OpCompositeExtract %uint %load2 0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, CreateAmbiguousNullConstant1) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct1:%\w+]] = OpTypeStruct [[uint]] [[struct_member:%\w+]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[const:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[undef:%\w+]] = OpUndef [[struct_member]]
; CHECK: [[var0a:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[var1:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[var0b:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK-NOT: OpVariable
; CHECK: OpStore [[var1]]
; CHECK: [[l1:%\w+]] = OpLoad [[uint]] [[var1]]
; CHECK: [[c1:%\w+]] = OpCompositeConstruct [[struct1]] [[l1]] [[undef]]
; CHECK: [[e1:%\w+]] = OpCompositeExtract [[uint]] [[c1]] 0
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "replace_whole_load"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct2 = OpTypeStruct %uint
%struct3 = OpTypeStruct %uint
%struct1 = OpTypeStruct %uint %struct2
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var1 = OpVariable %struct1_ptr Function
%var2 = OpVariable %struct1_ptr Function
%load1 = OpLoad %struct1 %var1
OpStore %var2 %load1
%load2 = OpLoad %struct1 %var2
%3 = OpCompositeExtract %uint %load2 0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, SpecConstantArray) {
  const std::string text = R"(
; CHECK: [[int:%\w+]] = OpTypeInt
; CHECK: [[spec_const:%\w+]] = OpSpecConstant [[int]] 4
; CHECK: [[spec_op:%\w+]] = OpSpecConstantOp [[int]] IAdd [[spec_const]] [[spec_const]]
; CHECK: [[array1:%\w+]] = OpTypeArray [[int]] [[spec_const]]
; CHECK: [[array2:%\w+]] = OpTypeArray [[int]] [[spec_op]]
; CHECK: [[ptr_array1:%\w+]] = OpTypePointer Function [[array1]]
; CHECK: [[ptr_array2:%\w+]] = OpTypePointer Function [[array2]]
; CHECK: OpLabel
; CHECK-NEXT: OpVariable [[ptr_array1]] Function
; CHECK-NEXT: OpVariable [[ptr_array2]] Function
; CHECK-NOT: OpVariable
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%int = OpTypeInt 32 0
%spec_const = OpSpecConstant %int 4
%spec_op = OpSpecConstantOp %int IAdd %spec_const %spec_const
%array_1 = OpTypeArray %int %spec_const
%array_2 = OpTypeArray %int %spec_op
%ptr_array_1_Function = OpTypePointer Function %array_1
%ptr_array_2_Function = OpTypePointer Function %array_2
%func = OpFunction %void None %void_fn
%1 = OpLabel
%var_1 = OpVariable %ptr_array_1_Function Function
%var_2 = OpVariable %ptr_array_2_Function Function
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, CreateAmbiguousNullConstant2) {
  const std::string text = R"(
;
; CHECK: [[uint:%\w+]] = OpTypeInt 32 0
; CHECK: [[struct1:%\w+]] = OpTypeStruct [[uint]] [[struct_member:%\w+]]
; CHECK: [[uint_ptr:%\w+]] = OpTypePointer Function [[uint]]
; CHECK: [[const:%\w+]] = OpConstant [[uint]] 0
; CHECK: [[undef:%\w+]] = OpUndef [[struct_member]]
; CHECK: [[var0a:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[var1:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: [[var0b:%\w+]] = OpVariable [[uint_ptr]] Function
; CHECK: OpStore [[var1]]
; CHECK: [[l1:%\w+]] = OpLoad [[uint]] [[var1]]
; CHECK: [[c1:%\w+]] = OpCompositeConstruct [[struct1]] [[l1]] [[undef]]
; CHECK: [[e1:%\w+]] = OpCompositeExtract [[uint]] [[c1]] 0
;
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %func "replace_whole_load"
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%struct3 = OpTypeStruct %uint
%struct2 = OpTypeStruct %uint
%struct1 = OpTypeStruct %uint %struct2
%uint_ptr = OpTypePointer Function %uint
%struct1_ptr = OpTypePointer Function %struct1
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%var1 = OpVariable %struct1_ptr Function
%var2 = OpVariable %struct1_ptr Function
%load1 = OpLoad %struct1 %var1
OpStore %var2 %load1
%load2 = OpLoad %struct1 %var2
%3 = OpCompositeExtract %uint %load2 0
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

// Test that a struct of size 4 is not replaced when there is a limit of 2.
TEST_F(ScalarReplacementTest, TestLimit) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %6 "simple_struct"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeStruct %2 %2 %2 %2
%4 = OpTypePointer Function %3
%5 = OpTypePointer Function %2
%6 = OpTypeFunction %2
%7 = OpConstantNull %3
%8 = OpConstant %2 0
%9 = OpConstant %2 1
%10 = OpConstant %2 2
%11 = OpConstant %2 3
%12 = OpFunction %2 None %6
%13 = OpLabel
%14 = OpVariable %4 Function %7
%15 = OpInBoundsAccessChain %5 %14 %8
%16 = OpLoad %2 %15
%17 = OpAccessChain %5 %14 %10
%18 = OpLoad %2 %17
%19 = OpIAdd %2 %16 %18
OpReturnValue %19
OpFunctionEnd
  )";

  auto result =
      SinglePassRunAndDisassemble<ScalarReplacementPass>(text, true, false, 2);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

// Test that a struct of size 4 is replaced when there is a limit of 0 (no
// limit).  This is the same spir-v as a test above, so we do not check that it
// is correctly transformed.  We leave that to the test above.
TEST_F(ScalarReplacementTest, TestUnimited) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
OpName %6 "simple_struct"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%3 = OpTypeStruct %2 %2 %2 %2
%4 = OpTypePointer Function %3
%5 = OpTypePointer Function %2
%6 = OpTypeFunction %2
%7 = OpConstantNull %3
%8 = OpConstant %2 0
%9 = OpConstant %2 1
%10 = OpConstant %2 2
%11 = OpConstant %2 3
%12 = OpFunction %2 None %6
%13 = OpLabel
%14 = OpVariable %4 Function %7
%15 = OpInBoundsAccessChain %5 %14 %8
%16 = OpLoad %2 %15
%17 = OpAccessChain %5 %14 %10
%18 = OpLoad %2 %17
%19 = OpIAdd %2 %16 %18
OpReturnValue %19
OpFunctionEnd
  )";

  auto result =
      SinglePassRunAndDisassemble<ScalarReplacementPass>(text, true, false, 0);
  EXPECT_EQ(Pass::Status::SuccessWithChange, std::get<1>(result));
}

TEST_F(ScalarReplacementTest, AmbigousPointer) {
  const std::string text = R"(
; CHECK: [[s1:%\w+]] = OpTypeStruct %uint
; CHECK: [[s2:%\w+]] = OpTypeStruct %uint
; CHECK: [[s3:%\w+]] = OpTypeStruct [[s2]]
; CHECK: [[s3_const:%\w+]] = OpConstantComposite [[s3]]
; CHECK: [[s2_ptr:%\w+]] = OpTypePointer Function [[s2]]
; CHECK: OpCompositeExtract [[s2]] [[s3_const]]

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpSource ESSL 310
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
  %_struct_7 = OpTypeStruct %uint
  %_struct_8 = OpTypeStruct %uint
  %_struct_9 = OpTypeStruct %_struct_8
     %uint_1 = OpConstant %uint 1
         %11 = OpConstantComposite %_struct_8 %uint_1
         %12 = OpConstantComposite %_struct_9 %11
%_ptr_Function__struct_9 = OpTypePointer Function %_struct_9
%_ptr_Function__struct_7 = OpTypePointer Function %_struct_7
          %2 = OpFunction %void None %5
         %15 = OpLabel
        %var = OpVariable %_ptr_Function__struct_9 Function
               OpStore %var %12
         %ld = OpLoad %_struct_9 %var
         %ex = OpCompositeExtract %_struct_8 %ld 0
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

// Test that scalar replacement does not crash when there is an OpAccessChain
// with no index.  If we choose to handle this case in the future, then the
// result can change.
TEST_F(ScalarReplacementTest, TestAccessChainWithNoIndexes) {
  const std::string text = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main"
               OpExecutionMode %1 OriginLowerLeft
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
  %_struct_5 = OpTypeStruct %float
%_ptr_Function__struct_5 = OpTypePointer Function %_struct_5
          %1 = OpFunction %void None %3
          %7 = OpLabel
          %8 = OpVariable %_ptr_Function__struct_5 Function
          %9 = OpAccessChain %_ptr_Function__struct_5 %8
               OpReturn
               OpFunctionEnd
  )";

  auto result =
      SinglePassRunAndDisassemble<ScalarReplacementPass>(text, true, false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

// Test that id overflow is handled gracefully.
TEST_F(ScalarReplacementTest, IdBoundOverflow1) {
  const std::string text = R"(
OpCapability ImageQuery
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main"
OpExecutionMode %4 OriginUpperLeft
OpDecorate %4194302 DescriptorSet 1073495039
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeStruct %6 %6
%557056 = OpTypeStruct %7
%9 = OpTypePointer Function %7
%18 = OpTypeFunction %7 %9
%4 = OpFunction %2 Pure|Const %3
%1836763 = OpLabel
%4194302 = OpVariable %9 Function
%10 = OpVariable %9 Function
OpKill
%4194301 = OpLabel
%524296 = OpLoad %7 %4194302
OpKill
OpFunctionEnd
  )";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."},
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<ScalarReplacementPass>(text, true, false);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

// Test that id overflow is handled gracefully.
TEST_F(ScalarReplacementTest, IdBoundOverflow2) {
  const std::string text = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %17
OpExecutionMode %4 OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeVector %6 4
%8 = OpTypeStruct %7
%9 = OpTypePointer Function %8
%16 = OpTypePointer Output %7
%21 = OpTypeInt 32 1
%22 = OpConstant %21 0
%23 = OpTypePointer Function %7
%17 = OpVariable %16 Output
%4 = OpFunction %2 None %3
%5 = OpLabel
%4194300 = OpVariable %23 Function
%10 = OpVariable %9 Function
%4194301 = OpAccessChain %23 %10 %22
%4194302 = OpLoad %7 %4194301
OpStore %4194300 %4194302
%15 = OpLoad %7 %4194300
OpStore %17 %15
OpReturn
OpFunctionEnd
  )";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<ScalarReplacementPass>(text, true, false);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

// Test that id overflow is handled gracefully.
TEST_F(ScalarReplacementTest, IdBoundOverflow3) {
  const std::string text = R"(
OpCapability InterpolationFunction
OpExtension "z"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main"
OpExecutionMode %4 OriginUpperLeft
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeStruct %6 %6
%9 = OpTypePointer Function %7
%18 = OpTypeFunction %7 %9
%21 = OpTypeInt 32 0
%22 = OpConstant %21 4293000676
%4194302 = OpConstantNull %6
%4 = OpFunction %2 Inline|Pure %3
%786464 = OpLabel
%4194298 = OpVariable %9 Function
%10 = OpVariable %9 Function
%4194299 = OpUDiv %21 %22 %22
%4194300 = OpLoad %7 %10
%50959 = OpLoad %7 %4194298
OpKill
OpFunctionEnd
%1 = OpFunction %7 None %18
%19 = OpFunctionParameter %9
%147667 = OpLabel
%2044391 = OpUDiv %21 %22 %22
%25 = OpLoad %7 %19
OpReturnValue %25
OpFunctionEnd
%4194295 = OpFunction %2 None %3
%4194296 = OpLabel
OpKill
OpFunctionEnd
  )";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  std::vector<Message> messages = {
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."},
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."},
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."},
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."},
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."},
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."},
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."},
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."},
      {SPV_MSG_ERROR, "", 0, 0, "ID overflow. Try running compact-ids."}};
  SetMessageConsumer(GetTestMessageConsumer(messages));
  auto result = SinglePassRunToBinary<ScalarReplacementPass>(text, true, false);
  EXPECT_EQ(Pass::Status::Failure, std::get<1>(result));
}

// Test that replacements for OpAccessChain do not go out of bounds.
// https://github.com/KhronosGroup/SPIRV-Tools/issues/2609.
TEST_F(ScalarReplacementTest, OutOfBoundOpAccessChain) {
  const std::string text = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %_GLF_color
               OpExecutionMode %main OriginUpperLeft
               OpSource ESSL 310
               OpName %main "main"
               OpName %a "a"
               OpName %_GLF_color "_GLF_color"
               OpDecorate %_GLF_color Location 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %float = OpTypeFloat 32
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
%_arr_float_uint_1 = OpTypeArray %float %uint_1
%_ptr_Function__arr_float_uint_1 = OpTypePointer Function %_arr_float_uint_1
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
 %_GLF_color = OpVariable %_ptr_Output_float Output
       %main = OpFunction %void None %3
          %5 = OpLabel
          %a = OpVariable %_ptr_Function__arr_float_uint_1 Function
         %21 = OpAccessChain %_ptr_Function_float %a %int_1
         %22 = OpLoad %float %21
               OpStore %_GLF_color %22
               OpReturn
               OpFunctionEnd
  )";

  SetAssembleOptions(SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS);

  auto result =
      SinglePassRunAndDisassemble<ScalarReplacementPass>(text, true, false);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

TEST_F(ScalarReplacementTest, CharIndex) {
  const std::string text = R"(
; CHECK: [[int:%\w+]] = OpTypeInt 32 0
; CHECK: [[ptr:%\w+]] = OpTypePointer Function [[int]]
; CHECK: OpVariable [[ptr]] Function
OpCapability Shader
OpCapability Int8
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_1024 = OpConstant %int 1024
%char = OpTypeInt 8 0
%char_1 = OpConstant %char 1
%array = OpTypeArray %int %int_1024
%ptr_func_array = OpTypePointer Function %array
%ptr_func_int = OpTypePointer Function %int
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%var = OpVariable %ptr_func_array Function
%gep = OpAccessChain %ptr_func_int %var %char_1
OpStore %gep %int_1024
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true, 0);
}

TEST_F(ScalarReplacementTest, OutOfBoundsOpAccessChainNegative) {
  const std::string text = R"(
OpCapability Shader
OpCapability Int8
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_1024 = OpConstant %int 1024
%char = OpTypeInt 8 1
%char_n1 = OpConstant %char -1
%array = OpTypeArray %int %int_1024
%ptr_func_array = OpTypePointer Function %array
%ptr_func_int = OpTypePointer Function %int
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
%var = OpVariable %ptr_func_array Function
%gep = OpAccessChain %ptr_func_int %var %char_n1
OpStore %gep %int_1024
OpReturn
OpFunctionEnd
)";

  auto result =
      SinglePassRunAndDisassemble<ScalarReplacementPass>(text, true, true, 0);
  EXPECT_EQ(Pass::Status::SuccessWithoutChange, std::get<1>(result));
}

TEST_F(ScalarReplacementTest, RelaxedPrecisionMemberDecoration) {
  const std::string text = R"(
; CHECK: OpDecorate {{%\w+}} RelaxedPrecision
; CHECK: OpDecorate [[new_var:%\w+]] RelaxedPrecision
; CHECK: [[new_var]] = OpVariable %_ptr_Function_v3float Function
; CHECK: OpLoad %v3float [[new_var]]
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %1 "Draw2DTexCol_VS" %2 %3
               OpSource HLSL 600
               OpDecorate %2 Location 0
               OpDecorate %3 Location 1
               OpDecorate %3 RelaxedPrecision
               OpMemberDecorate %_struct_4 1 RelaxedPrecision
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
%_ptr_Input_v3float = OpTypePointer Input %v3float
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
  %_struct_4 = OpTypeStruct %v3float %v3float
%_ptr_Function__struct_4 = OpTypePointer Function %_struct_4
%_ptr_Function_v3float = OpTypePointer Function %v3float
          %2 = OpVariable %_ptr_Input_v3float Input
          %3 = OpVariable %_ptr_Input_v3float Input
          %1 = OpFunction %void None %11
         %14 = OpLabel
         %15 = OpVariable %_ptr_Function__struct_4 Function
         %16 = OpLoad %v3float %2
         %17 = OpLoad %v3float %3
         %18 = OpCompositeConstruct %_struct_4 %16 %17
               OpStore %15 %18
         %19 = OpAccessChain %_ptr_Function_v3float %15 %int_1
         %20 = OpLoad %v3float %19
               OpReturn
               OpFunctionEnd
)";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DebugDeclare) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
%test = OpString "test"
OpName %6 "simple_struct"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%uint_32 = OpConstant %2 32
%3 = OpTypeStruct %2 %2 %2 %2
%4 = OpTypePointer Function %3
%5 = OpTypePointer Function %2
%6 = OpTypeFunction %2
%7 = OpConstantNull %3
%8 = OpConstant %2 0
%9 = OpConstant %2 1
%10 = OpConstant %2 2
%11 = OpConstant %2 3
%null_expr = OpExtInst %1 %ext DebugExpression
%src = OpExtInst %1 %ext DebugSource %test
%cu = OpExtInst %1 %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %1 %ext DebugTypeBasic %test %uint_32 Float
%main_ty = OpExtInst %1 %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %1
%dbg_main = OpExtInst %1 %ext DebugFunction %test %main_ty %src 0 0 %cu %test FlagIsProtected|FlagIsPrivate 0 %12
%dbg_foo = OpExtInst %1 %ext DebugLocalVariable %test %dbg_tf %src 0 0 %dbg_main FlagIsLocal
%12 = OpFunction %2 None %6
%13 = OpLabel
%scope = OpExtInst %1 %ext DebugScope %dbg_main
%14 = OpVariable %4 Function %7

; CHECK: [[deref:%\w+]] = OpExtInst %void [[ext:%\w+]] DebugOperation Deref
; CHECK: [[dbg_local_var:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable
; CHECK: [[deref_expr:%\w+]] = OpExtInst %void [[ext]] DebugExpression [[deref]]
; CHECK: [[repl3:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl2:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl1:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl0:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl3]] [[deref_expr]] %int_3
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl2]] [[deref_expr]] %int_2
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl1]] [[deref_expr]] %int_1
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl0]] [[deref_expr]] %int_0
; CHECK-NOT: DebugDeclare
%decl = OpExtInst %1 %ext DebugDeclare %dbg_foo %14 %null_expr

%15 = OpInBoundsAccessChain %5 %14 %8
%16 = OpLoad %2 %15
%17 = OpAccessChain %5 %14 %10
%18 = OpLoad %2 %17
%19 = OpIAdd %2 %16 %18
OpReturnValue %19
OpFunctionEnd
)";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DebugValue) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
%test = OpString "test"
OpName %6 "simple_struct"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%uint_32 = OpConstant %2 32
%3 = OpTypeStruct %2 %2 %2 %2
%4 = OpTypePointer Function %3
%5 = OpTypePointer Function %2
%6 = OpTypeFunction %2
%7 = OpConstantNull %3
%8 = OpConstant %2 0
%9 = OpConstant %2 1
%10 = OpConstant %2 2
%11 = OpConstant %2 3
%deref = OpExtInst %1 %ext DebugOperation Deref
%deref_expr = OpExtInst %1 %ext DebugExpression %deref
%null_expr = OpExtInst %1 %ext DebugExpression
%src = OpExtInst %1 %ext DebugSource %test
%cu = OpExtInst %1 %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %1 %ext DebugTypeBasic %test %uint_32 Float
%main_ty = OpExtInst %1 %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %1
%dbg_main = OpExtInst %1 %ext DebugFunction %test %main_ty %src 0 0 %cu %test FlagIsProtected|FlagIsPrivate 0 %12
%dbg_foo = OpExtInst %1 %ext DebugLocalVariable %test %dbg_tf %src 0 0 %dbg_main FlagIsLocal
%12 = OpFunction %2 None %6
%13 = OpLabel
%scope = OpExtInst %1 %ext DebugScope %dbg_main
%14 = OpVariable %4 Function %7

; CHECK: [[deref:%\w+]] = OpExtInst %void [[ext:%\w+]] DebugOperation Deref
; CHECK: [[deref_expr:%\w+]] = OpExtInst %void [[ext]] DebugExpression [[deref]]
; CHECK: [[dbg_local_var:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable
; CHECK: [[repl3:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl2:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl1:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl0:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl0]] [[deref_expr]] %int_0
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl1]] [[deref_expr]] %int_1
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl2]] [[deref_expr]] %int_2
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl3]] [[deref_expr]] %int_3
%value = OpExtInst %1 %ext DebugValue %dbg_foo %14 %deref_expr

%15 = OpInBoundsAccessChain %5 %14 %8
%16 = OpLoad %2 %15
%17 = OpAccessChain %5 %14 %10
%18 = OpLoad %2 %17
%19 = OpIAdd %2 %16 %18
OpReturnValue %19
OpFunctionEnd
)";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DebugDeclareRecursive) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
%test = OpString "test"
OpName %6 "simple_struct"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%uint_32 = OpConstant %2 32
%float = OpTypeFloat 32
%float_1 = OpConstant %float 1
%member = OpTypeStruct %2 %float
%3 = OpTypeStruct %2 %member %float
%4 = OpTypePointer Function %3
%5 = OpTypePointer Function %2
%ptr_float_Function = OpTypePointer Function %float
%6 = OpTypeFunction %2
%cmember = OpConstantComposite %member %uint_32 %float_1
%7 = OpConstantComposite %3 %uint_32 %cmember %float_1
%8 = OpConstant %2 0
%9 = OpConstant %2 1
%10 = OpConstant %2 2
%null_expr = OpExtInst %1 %ext DebugExpression
%src = OpExtInst %1 %ext DebugSource %test
%cu = OpExtInst %1 %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %1 %ext DebugTypeBasic %test %uint_32 Float
%main_ty = OpExtInst %1 %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %1
%dbg_main = OpExtInst %1 %ext DebugFunction %test %main_ty %src 0 0 %cu %test FlagIsProtected|FlagIsPrivate 0 %12
%dbg_foo = OpExtInst %1 %ext DebugLocalVariable %test %dbg_tf %src 0 0 %dbg_main FlagIsLocal
%12 = OpFunction %2 None %6
%13 = OpLabel
%scope = OpExtInst %1 %ext DebugScope %dbg_main
%14 = OpVariable %4 Function %7

; CHECK: [[deref:%\w+]] = OpExtInst %void [[ext:%\w+]] DebugOperation Deref
; CHECK: [[dbg_local_var:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable
; CHECK: [[deref_expr:%\w+]] = OpExtInst %void [[ext]] DebugExpression [[deref]]
; CHECK: [[repl2:%\w+]] = OpVariable %_ptr_Function_float Function %float_1
; CHECK: [[repl1:%\w+]] = OpVariable %_ptr_Function_uint Function %uint_32
; CHECK: [[repl3:%\w+]] = OpVariable %_ptr_Function_float Function %float_1
; CHECK: [[repl0:%\w+]] = OpVariable %_ptr_Function_uint Function %uint_32
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl3]] [[deref_expr]] %int_2
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl1]] [[deref_expr]] %int_1 %int_0
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl2]] [[deref_expr]] %int_1 %int_1
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl0]] [[deref_expr]] %int_0
; CHECK-NOT: DebugDeclare
%decl = OpExtInst %1 %ext DebugDeclare %dbg_foo %14 %null_expr

%15 = OpInBoundsAccessChain %5 %14 %8
%16 = OpLoad %2 %15
%17 = OpAccessChain %ptr_float_Function %14 %10
%18 = OpLoad %float %17
%value = OpConvertFToU %2 %18
%19 = OpIAdd %2 %16 %value
OpReturnValue %19
OpFunctionEnd
)";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DebugValueWithIndex) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
%test = OpString "test"
OpName %6 "simple_struct"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%uint_32 = OpConstant %2 32
%3 = OpTypeStruct %2 %2 %2 %2
%4 = OpTypePointer Function %3
%5 = OpTypePointer Function %2
%6 = OpTypeFunction %2
%7 = OpConstantNull %3
%8 = OpConstant %2 0
%9 = OpConstant %2 1
%10 = OpConstant %2 2
%11 = OpConstant %2 3
%deref = OpExtInst %1 %ext DebugOperation Deref
%deref_expr = OpExtInst %1 %ext DebugExpression %deref
%null_expr = OpExtInst %1 %ext DebugExpression
%src = OpExtInst %1 %ext DebugSource %test
%cu = OpExtInst %1 %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %1 %ext DebugTypeBasic %test %uint_32 Float
%main_ty = OpExtInst %1 %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %1
%dbg_main = OpExtInst %1 %ext DebugFunction %test %main_ty %src 0 0 %cu %test FlagIsProtected|FlagIsPrivate 0 %12
%dbg_foo = OpExtInst %1 %ext DebugLocalVariable %test %dbg_tf %src 0 0 %dbg_main FlagIsLocal
%12 = OpFunction %2 None %6
%13 = OpLabel
%scope = OpExtInst %1 %ext DebugScope %dbg_main
%14 = OpVariable %4 Function %7

; CHECK: [[deref:%\w+]] = OpExtInst %void [[ext:%\w+]] DebugOperation Deref
; CHECK: [[deref_expr:%\w+]] = OpExtInst %void [[ext]] DebugExpression [[deref]]
; CHECK: [[dbg_local_var:%\w+]] = OpExtInst %void [[ext]] DebugLocalVariable
; CHECK: [[repl3:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl2:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl1:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl0:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl0]] [[deref_expr]] %uint_0 %uint_1 %uint_2 %int_0
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl1]] [[deref_expr]] %uint_0 %uint_1 %uint_2 %int_1
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl2]] [[deref_expr]] %uint_0 %uint_1 %uint_2 %int_2
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl3]] [[deref_expr]] %uint_0 %uint_1 %uint_2 %int_3
%value = OpExtInst %1 %ext DebugValue %dbg_foo %14 %deref_expr %8 %9 %10

%15 = OpInBoundsAccessChain %5 %14 %8
%16 = OpLoad %2 %15
%17 = OpAccessChain %5 %14 %10
%18 = OpLoad %2 %17
%19 = OpIAdd %2 %16 %18
OpReturnValue %19
OpFunctionEnd
)";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, DebugDeclareForVariableInOtherBB) {
  const std::string text = R"(
OpCapability Shader
OpCapability Linkage
%ext = OpExtInstImport "OpenCL.DebugInfo.100"
OpMemoryModel Logical GLSL450
%test = OpString "test"
OpName %6 "simple_struct"
%1 = OpTypeVoid
%2 = OpTypeInt 32 0
%uint_32 = OpConstant %2 32
%3 = OpTypeStruct %2 %2 %2 %2
%4 = OpTypePointer Function %3
%5 = OpTypePointer Function %2
%6 = OpTypeFunction %2
%7 = OpConstantNull %3
%8 = OpConstant %2 0
%9 = OpConstant %2 1
%10 = OpConstant %2 2
%11 = OpConstant %2 3
%deref = OpExtInst %1 %ext DebugOperation Deref
%deref_expr = OpExtInst %1 %ext DebugExpression %deref
%null_expr = OpExtInst %1 %ext DebugExpression
%src = OpExtInst %1 %ext DebugSource %test
%cu = OpExtInst %1 %ext DebugCompilationUnit 1 4 %src HLSL
%dbg_tf = OpExtInst %1 %ext DebugTypeBasic %test %uint_32 Float
%main_ty = OpExtInst %1 %ext DebugTypeFunction FlagIsProtected|FlagIsPrivate %1
%dbg_main = OpExtInst %1 %ext DebugFunction %test %main_ty %src 0 0 %cu %test FlagIsProtected|FlagIsPrivate 0 %12
%dbg_foo = OpExtInst %1 %ext DebugLocalVariable %test %dbg_tf %src 0 0 %dbg_main FlagIsLocal
%12 = OpFunction %2 None %6
%13 = OpLabel
%scope = OpExtInst %1 %ext DebugScope %dbg_main
%14 = OpVariable %4 Function %7

; CHECK: [[dbg_local_var:%\w+]] = OpExtInst %void [[ext:%\w+]] DebugLocalVariable
; CHECK: [[repl3:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl2:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl1:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: [[repl0:%\w+]] = OpVariable %_ptr_Function_uint Function
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl3]] [[deref_expr:%\w+]] %int_3
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl2]] [[deref_expr]] %int_2
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl1]] [[deref_expr]] %int_1
; CHECK: OpExtInst %void [[ext]] DebugValue [[dbg_local_var]] [[repl0]] [[deref_expr]] %int_0

OpBranch %20
%20 = OpLabel
%value = OpExtInst %1 %ext DebugDeclare %dbg_foo %14 %null_expr
%15 = OpInBoundsAccessChain %5 %14 %8
%16 = OpLoad %2 %15
%17 = OpAccessChain %5 %14 %10
%18 = OpLoad %2 %17
%19 = OpIAdd %2 %16 %18
OpReturnValue %19
OpFunctionEnd
)";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, ImageTexelPointer) {
  // Test whether the scalar replacement correctly checks the
  // OpImageTexelPointer user of an aggregate with an image type.
  const std::string text = R"(
;
; CHECK: [[imgTy:%\w+]] = OpTypeImage %uint Buffer 2 0 0 2 R32ui
; CHECK: [[ptrImgTy:%\w+]] = OpTypePointer Function [[imgTy]]
; CHECK: [[img:%\w+]] = OpVariable [[ptrImgTy]] Function
; CHECK: [[imgTexelPtr:%\w+]] = OpImageTexelPointer {{%\w+}} [[img]] %uint_0 %uint_0
; CHECK: OpAtomicIAdd %uint [[imgTexelPtr]] %uint_1 %uint_0 %uint_1
;
OpCapability Shader
OpCapability SampledBuffer
OpCapability ImageBuffer
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %1 "main"
OpExecutionMode %1 LocalSize 64 1 1
%void = OpTypeVoid
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%_ptr_Image_uint = OpTypePointer Image %uint
%type_buffer_image = OpTypeImage %uint Buffer 2 0 0 2 R32ui
%_ptr_Function_type_buffer_image = OpTypePointer Function %type_buffer_image
%image_struct = OpTypeStruct %type_buffer_image %type_buffer_image
%_ptr_Function_image_struct = OpTypePointer Function %image_struct
%func = OpTypeFunction %void
%1 = OpFunction %void None %func
%2 = OpLabel
%3 = OpVariable %_ptr_Function_image_struct Function
%4 = OpAccessChain %_ptr_Function_type_buffer_image %3 %uint_1
%5 = OpImageTexelPointer %_ptr_Image_uint %4 %uint_0 %uint_0
%6 = OpAtomicIAdd %uint %5 %uint_1 %uint_0 %uint_1
OpReturn
OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, false);
}

TEST_F(ScalarReplacementTest, FunctionDeclaration) {
  // Make sure the pass works with a function declaration that is called.
  const std::string text = R"(OpCapability Addresses
OpCapability Linkage
OpCapability Kernel
OpCapability Int8
%1 = OpExtInstImport "OpenCL.std"
OpMemoryModel Physical64 OpenCL
OpEntryPoint Kernel %2 "_Z23julia__1166_kernel_77094Bool"
OpExecutionMode %2 ContractionOff
OpSource Unknown 0
OpDecorate %3 LinkageAttributes "julia_error_7712" Import
%void = OpTypeVoid
%5 = OpTypeFunction %void
%3 = OpFunction %void None %5
OpFunctionEnd
%2 = OpFunction %void None %5
%6 = OpLabel
%7 = OpFunctionCall %void %3
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<ScalarReplacementPass>(text, text, false);
}

TEST_F(ScalarReplacementTest, UndefImageMember) {
  // Test that scalar replacement creates an undef for a type that cannot have
  // and OpConstantNull.
  const std::string text = R"(
; CHECK: [[image_type:%\w+]] = OpTypeSampledImage {{%\w+}}
; CHECK: [[struct_type:%\w+]] = OpTypeStruct [[image_type]]
; CHECK: [[undef:%\w+]] = OpUndef [[image_type]]
; CHECK: {{%\w+}} = OpCompositeConstruct [[struct_type]] [[undef]]
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %6 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %7 = OpTypeSampledImage %6
  %_struct_8 = OpTypeStruct %7
          %9 = OpTypeFunction %_struct_8
         %10 = OpUndef %_struct_8
%_ptr_Function__struct_8 = OpTypePointer Function %_struct_8
          %2 = OpFunction %void None %4
         %11 = OpLabel
         %16 = OpVariable %_ptr_Function__struct_8 Function
               OpStore %16 %10
         %12 = OpLoad %_struct_8 %16
               OpReturn
               OpFunctionEnd
  )";

  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

TEST_F(ScalarReplacementTest, RestrictPointer) {
  // This test makes sure that a variable with the restrict pointer decoration
  // is replaced, and that the pointer is applied to the new variable.
  const std::string text = R"(
; CHECK: OpDecorate [[new_var:%\w+]] RestrictPointer
; CHECK: [[struct_type:%\w+]] = OpTypeStruct %int
; CHECK: [[ptr_type:%\w+]] = OpTypePointer PhysicalStorageBuffer [[struct_type]]
; CHECK: [[dup_struct_type:%\w+]] = OpTypeStruct %int
; CHECK: {{%\w+}} = OpTypePointer PhysicalStorageBuffer [[dup_struct_type]]
; CHECK: [[var_type:%\w+]] = OpTypePointer Function [[ptr_type]]
; CHECK: [[new_var]] = OpVariable [[var_type]] Function
               OpCapability Shader
               OpCapability PhysicalStorageBufferAddresses
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel PhysicalStorageBuffer64 GLSL450
               OpEntryPoint Fragment %2 "main"
               OpExecutionMode %2 OriginUpperLeft
               OpMemberDecorate %3 0 Offset 0
               OpDecorate %3 Block
               OpMemberDecorate %4 0 Offset 0
               OpDecorate %4 Block
               OpDecorate %5 RestrictPointer
          %6 = OpTypeVoid
          %7 = OpTypeFunction %6
          %8 = OpTypeInt 32 1
          %9 = OpConstant %8 0
          %3 = OpTypeStruct %8
         %10 = OpTypePointer PhysicalStorageBuffer %3
         %11 = OpTypeStruct %10
          %4 = OpTypeStruct %8
         %12 = OpTypePointer PhysicalStorageBuffer %4
         %13 = OpTypePointer Function %11
         %14 = OpTypePointer Function %10
         %15 = OpTypePointer Function %12
         %16 = OpUndef %11
          %2 = OpFunction %6 None %7
         %17 = OpLabel
          %5 = OpVariable %13 Function
               OpStore %5 %16
         %18 = OpAccessChain %14 %5 %9
               OpReturn
               OpFunctionEnd
  )";

  SetTargetEnv(SPV_ENV_UNIVERSAL_1_6);
  SinglePassRunAndMatch<ScalarReplacementPass>(text, true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
