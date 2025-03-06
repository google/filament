// Copyright (c) 2023 The Khronos Group Inc.
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

#include "gmock/gmock.h"
#include "source/opt/licm_pass.h"
#include "test/opt/pass_fixture.h"

namespace spvtools {
namespace opt {
namespace {

using PassClassTest = PassTest<::testing::Test>;

/*
  Tests for the LICM pass to check it handles access chains correctly

  Generated from the following GLSL fragment shader
--eliminate-local-multi-store has also been run on the spv binary
#version 460
void main() {
  for (uint i = 0; i < 123u; ++i) {
    vec2 do_not_hoist_store = vec2(0.0f);
    float do_not_hoist_access_chain_load = do_not_hoist_store.x;
  }
}
*/

TEST_F(PassClassTest, HoistAccessChains) {
  const std::string before_hoist = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 460
OpName %main "main"
OpName %i "i"
OpName %do_not_hoist_store "do_not_hoist_store"
OpName %do_not_hoist_access_chain_load "do_not_hoist_access_chain_load"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_0 = OpConstant %uint 0
%uint_123 = OpConstant %uint 123
%bool = OpTypeBool
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_0 = OpConstant %float 0
%17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%main = OpFunction %void None %7
%21 = OpLabel
%i = OpVariable %_ptr_Function_uint Function
%do_not_hoist_store = OpVariable %_ptr_Function_v2float Function
%do_not_hoist_access_chain_load = OpVariable %_ptr_Function_float Function
OpStore %i %uint_0
OpBranch %22
%22 = OpLabel
OpLoopMerge %23 %24 None
OpBranch %25
%25 = OpLabel
%26 = OpLoad %uint %i
%27 = OpULessThan %bool %26 %uint_123
OpBranchConditional %27 %28 %23
%28 = OpLabel
OpStore %do_not_hoist_store %17
%29 = OpAccessChain %_ptr_Function_float %do_not_hoist_store %uint_0
%30 = OpLoad %float %29
OpStore %do_not_hoist_access_chain_load %30
OpBranch %24
%24 = OpLabel
%31 = OpLoad %uint %i
%32 = OpIAdd %uint %31 %int_1
OpStore %i %32
OpBranch %22
%23 = OpLabel
OpReturn
OpFunctionEnd
)";

  const std::string after_hoist = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 460
OpName %main "main"
OpName %i "i"
OpName %do_not_hoist_store "do_not_hoist_store"
OpName %do_not_hoist_access_chain_load "do_not_hoist_access_chain_load"
%void = OpTypeVoid
%7 = OpTypeFunction %void
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_0 = OpConstant %uint 0
%uint_123 = OpConstant %uint 123
%bool = OpTypeBool
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%float_0 = OpConstant %float 0
%17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%main = OpFunction %void None %7
%21 = OpLabel
%i = OpVariable %_ptr_Function_uint Function
%do_not_hoist_store = OpVariable %_ptr_Function_v2float Function
%do_not_hoist_access_chain_load = OpVariable %_ptr_Function_float Function
OpStore %i %uint_0
%29 = OpAccessChain %_ptr_Function_float %do_not_hoist_store %uint_0
OpBranch %22
%22 = OpLabel
OpLoopMerge %23 %24 None
OpBranch %25
%25 = OpLabel
%26 = OpLoad %uint %i
%27 = OpULessThan %bool %26 %uint_123
OpBranchConditional %27 %28 %23
%28 = OpLabel
OpStore %do_not_hoist_store %17
%30 = OpLoad %float %29
OpStore %do_not_hoist_access_chain_load %30
OpBranch %24
%24 = OpLabel
%31 = OpLoad %uint %i
%32 = OpIAdd %uint %31 %int_1
OpStore %i %32
OpBranch %22
%23 = OpLabel
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<LICMPass>(before_hoist, after_hoist, true);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
