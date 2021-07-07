// Copyright (c) 2019 Google LLC
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

#include <array>
#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "pass_fixture.h"
#include "pass_utils.h"
#include "source/opt/graphics_robust_access_pass.h"

namespace {

using namespace spvtools;

using opt::GraphicsRobustAccessPass;
using GraphicsRobustAccessTest = opt::PassTest<::testing::Test>;

// Test incompatible module, determined at module-level.

TEST_F(GraphicsRobustAccessTest, FailNotShader) {
  const std::string text = R"(
; CHECK: Can only process Shader modules
OpCapability Kernel
)";

  SinglePassRunAndFail<GraphicsRobustAccessPass>(text);
}

TEST_F(GraphicsRobustAccessTest, FailCantProcessVariablePointers) {
  const std::string text = R"(
; CHECK: Can't process modules with VariablePointers capability
OpCapability VariablePointers
)";

  SinglePassRunAndFail<GraphicsRobustAccessPass>(text);
}

TEST_F(GraphicsRobustAccessTest, FailCantProcessVariablePointersStorageBuffer) {
  const std::string text = R"(
; CHECK: Can't process modules with VariablePointersStorageBuffer capability
OpCapability VariablePointersStorageBuffer
)";

  SinglePassRunAndFail<GraphicsRobustAccessPass>(text);
}

TEST_F(GraphicsRobustAccessTest, FailCantProcessRuntimeDescriptorArrayEXT) {
  const std::string text = R"(
; CHECK: Can't process modules with RuntimeDescriptorArrayEXT capability
OpCapability RuntimeDescriptorArrayEXT
)";

  SinglePassRunAndFail<GraphicsRobustAccessPass>(text);
}

TEST_F(GraphicsRobustAccessTest, FailCantProcessPhysical32AddressingModel) {
  const std::string text = R"(
; CHECK: Addressing model must be Logical.  Found OpMemoryModel Physical32 OpenCL
OpCapability Shader
OpMemoryModel Physical32 OpenCL
)";

  SinglePassRunAndFail<GraphicsRobustAccessPass>(text);
}

TEST_F(GraphicsRobustAccessTest, FailCantProcessPhysical64AddressingModel) {
  const std::string text = R"(
; CHECK: Addressing model must be Logical.  Found OpMemoryModel Physical64 OpenCL
OpCapability Shader
OpMemoryModel Physical64 OpenCL
)";

  SinglePassRunAndFail<GraphicsRobustAccessPass>(text);
}

TEST_F(GraphicsRobustAccessTest,
       FailCantProcessPhysicalStorageBuffer64EXTAddressingModel) {
  const std::string text = R"(
; CHECK: Addressing model must be Logical.  Found OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpCapability Shader
OpMemoryModel PhysicalStorageBuffer64EXT GLSL450
)";

  SinglePassRunAndFail<GraphicsRobustAccessPass>(text);
}

// Test access chains

// Returns the names of access chain instructions handled by the pass.
// For the purposes of this pass, regular and in-bounds access chains are the
// same.)
std::vector<const char*> AccessChains() {
  return {"OpAccessChain", "OpInBoundsAccessChain"};
}

std::string ShaderPreamble() {
  return R"(
  OpCapability Shader
  OpMemoryModel Logical Simple
  OpEntryPoint GLCompute %main "main"
)";
}

std::string ShaderPreamble(const std::vector<std::string>& names) {
  std::ostringstream os;
  os << ShaderPreamble();
  for (auto& name : names) {
    os << "  OpName %" << name << " \"" << name << "\"\n";
  }
  return os.str();
}

std::string ShaderPreambleAC() {
  return ShaderPreamble({"ac", "ptr_ty", "var"});
}

std::string ShaderPreambleAC(const std::vector<std::string>& names) {
  auto names2 = names;
  names2.push_back("ac");
  names2.push_back("ptr_ty");
  names2.push_back("var");
  return ShaderPreamble(names2);
}

std::string DecoSSBO() {
  return R"(
  OpDecorate %ssbo_s BufferBlock
  OpMemberDecorate %ssbo_s 0 Offset 0
  OpMemberDecorate %ssbo_s 1 Offset 4
  OpMemberDecorate %ssbo_s 2 Offset 16
  OpDecorate %var DescriptorSet 0
  OpDecorate %var Binding 0
)";
}

std::string TypesVoid() {
  return R"(
  %void = OpTypeVoid
  %void_fn = OpTypeFunction %void
)";
}

std::string TypesInt() {
  return R"(
  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
)";
}

std::string TypesFloat() {
  return R"(
  %float = OpTypeFloat 32
)";
}

std::string TypesShort() {
  return R"(
  %ushort = OpTypeInt 16 0
  %short = OpTypeInt 16 1
)";
}

std::string TypesLong() {
  return R"(
  %ulong = OpTypeInt 64 0
  %long = OpTypeInt 64 1
)";
}

std::string MainPrefix() {
  return R"(
  %main = OpFunction %void None %void_fn
  %entry = OpLabel
)";
}

std::string MainSuffix() {
  return R"(
  OpReturn
  OpFunctionEnd
)";
}

std::string ACCheck(const std::string& access_chain_inst,
                    const std::string& original,
                    const std::string& transformed) {
  return "\n ; CHECK: %ac = " + access_chain_inst + " %ptr_ty %var" +
         (transformed.empty() ? "" : " ") + transformed +
         "\n ; CHECK-NOT: " + access_chain_inst +
         "\n ; CHECK-NEXT: OpReturn"
         "\n %ac = " +
         access_chain_inst + " %ptr_ty %var " + (original.empty() ? "" : " ") +
         original + "\n";
}

std::string ACCheckFail(const std::string& access_chain_inst,
                        const std::string& original,
                        const std::string& transformed) {
  return "\n ; CHECK: %ac = " + access_chain_inst + " %ptr_ty %var" +
         (transformed.empty() ? "" : " ") + transformed +
         "\n ; CHECK-NOT: " + access_chain_inst +
         "\n ; CHECK-NOT: OpReturn"
         "\n %ac = " +
         access_chain_inst + " %ptr_ty %var " + (original.empty() ? "" : " ") +
         original + "\n";
}

// Access chain into:
//   Vector
//     Vector sizes 2, 3, 4
//   Matrix
//     Matrix columns 2, 4
//     Component is vector 2, 4
//   Array
//   Struct
//   TODO(dneto): RuntimeArray

TEST_F(GraphicsRobustAccessTest, ACVectorLeastInboundConstantUntouched) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt() << R"(
       %uvec2 = OpTypeVector %uint 2
       %var_ty = OpTypePointer Function %uvec2
       %ptr_ty = OpTypePointer Function %uint
       %uint_0 = OpConstant %uint 0
       )"
            << MainPrefix() << R"(
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%uint_0", "%uint_0")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACVectorMostInboundConstantUntouched) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt() << R"(
       %v4uint = OpTypeVector %uint 4
       %var_ty = OpTypePointer Function %v4uint
       %ptr_ty = OpTypePointer Function %uint
       %uint_3 = OpConstant %uint 3
       )"
            << MainPrefix() << R"(
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%uint_3", "%uint_3")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACVectorExcessConstantClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt() << R"(
       %v4uint = OpTypeVector %uint 4
       %var_ty = OpTypePointer Function %v4uint
       %ptr_ty = OpTypePointer Function %uint
       %uint_4 = OpConstant %uint 4
       )"
            << MainPrefix() << R"(
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%uint_4", "%int_3")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACVectorNegativeConstantClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt() << R"(
       %v4uint = OpTypeVector %uint 4
       %var_ty = OpTypePointer Function %v4uint
       %ptr_ty = OpTypePointer Function %uint
       %int_n1 = OpConstant %int -1
       )"
            << MainPrefix() << R"(
       ; CHECK: %int_0 = OpConstant %int 0
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%int_n1", "%int_0")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

// Like the previous test, but ensures the pass knows how to modify an index
// which does not come first in the access chain.
TEST_F(GraphicsRobustAccessTest, ACVectorInArrayNegativeConstantClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt() << R"(
       %v4uint = OpTypeVector %uint 4
       %uint_1 = OpConstant %uint 1
       %uint_2 = OpConstant %uint 2
       %arr = OpTypeArray %v4uint %uint_2
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %uint
       %int_n1 = OpConstant %int -1
       )"
            << MainPrefix() << R"(
       ; CHECK: %int_0 = OpConstant %int 0
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%uint_1 %int_n1", "%uint_1 %int_0") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACVectorGeneralClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt() << R"(
       %v4uint = OpTypeVector %uint 4
       %var_ty = OpTypePointer Function %v4uint
       %ptr_ty = OpTypePointer Function %uint
       %i = OpUndef %int)"
            << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_3 = OpConstant %int 3
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %i %int_0 %int_3
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%i", "%[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACVectorGeneralShortClamped) {
  // Show that signed 16 bit integers are clamped as well.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int16\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesShort() <<
        R"(
       %v4short = OpTypeVector %short 4
       %var_ty = OpTypePointer Function %v4short
       %ptr_ty = OpTypePointer Function %short
       %i = OpUndef %short)"
            << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-NOT: = OpTypeInt 32
       ; CHECK-DAG: %short_0 = OpConstant %short 0
       ; CHECK-DAG: %short_3 = OpConstant %short 3
       ; CHECK-NOT: = OpTypeInt 32
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %short %[[GLSLSTD450]] SClamp %i %short_0 %short_3
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%i", "%[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACVectorGeneralUShortClamped) {
  // Show that unsigned 16 bit integers are clamped as well.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int16\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesShort() <<
        R"(
       %v4ushort = OpTypeVector %ushort 4
       %var_ty = OpTypePointer Function %v4ushort
       %ptr_ty = OpTypePointer Function %ushort
       %i = OpUndef %ushort)"
            << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-NOT: = OpTypeInt 32
       ; CHECK-DAG: %short_0 = OpConstant %short 0
       ; CHECK-DAG: %short_3 = OpConstant %short 3
       ; CHECK-NOT: = OpTypeInt 32
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %ushort %[[GLSLSTD450]] SClamp %i %short_0 %short_3
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%i", "%[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACVectorGeneralLongClamped) {
  // Show that signed 64 bit integers are clamped as well.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int64\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesLong() <<
        R"(
       %v4long = OpTypeVector %long 4
       %var_ty = OpTypePointer Function %v4long
       %ptr_ty = OpTypePointer Function %long
       %i = OpUndef %long)"
            << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-NOT: = OpTypeInt 32
       ; CHECK-DAG: %long_0 = OpConstant %long 0
       ; CHECK-DAG: %long_3 = OpConstant %long 3
       ; CHECK-NOT: = OpTypeInt 32
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %long %[[GLSLSTD450]] SClamp %i %long_0 %long_3
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%i", "%[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACVectorGeneralULongClamped) {
  // Show that unsigned 64 bit integers are clamped as well.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int64\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesLong() <<
        R"(
       %v4ulong = OpTypeVector %ulong 4
       %var_ty = OpTypePointer Function %v4ulong
       %ptr_ty = OpTypePointer Function %ulong
       %i = OpUndef %ulong)"
            << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-NOT: = OpTypeInt 32
       ; CHECK-DAG: %long_0 = OpConstant %long 0
       ; CHECK-DAG: %long_3 = OpConstant %long 3
       ; CHECK-NOT: = OpTypeInt 32
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %ulong %[[GLSLSTD450]] SClamp %i %long_0 %long_3
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%i", "%[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACMatrixLeastInboundConstantUntouched) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %v2float = OpTypeVector %float 2
       %mat4v2float = OpTypeMatrix %v2float 4
       %var_ty = OpTypePointer Function %mat4v2float
       %ptr_ty = OpTypePointer Function %float
       %uint_0 = OpConstant %uint 0
       %uint_1 = OpConstant %uint 1
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%uint_0 %uint_1", "%uint_0 %uint_1")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACMatrixMostInboundConstantUntouched) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %v2float = OpTypeVector %float 2
       %mat4v2float = OpTypeMatrix %v2float 4
       %var_ty = OpTypePointer Function %mat4v2float
       %ptr_ty = OpTypePointer Function %float
       %uint_1 = OpConstant %uint 1
       %uint_3 = OpConstant %uint 3
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%uint_3 %uint_1", "%uint_3 %uint_1")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACMatrixExcessConstantClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %v2float = OpTypeVector %float 2
       %mat4v2float = OpTypeMatrix %v2float 4
       %var_ty = OpTypePointer Function %mat4v2float
       %ptr_ty = OpTypePointer Function %float
       %uint_1 = OpConstant %uint 1
       %uint_4 = OpConstant %uint 4
       )" << MainPrefix() << R"(
       ; CHECK: %int_3 = OpConstant %int 3
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%uint_4 %uint_1", "%int_3 %uint_1") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACMatrixNegativeConstantClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %v2float = OpTypeVector %float 2
       %mat4v2float = OpTypeMatrix %v2float 4
       %var_ty = OpTypePointer Function %mat4v2float
       %ptr_ty = OpTypePointer Function %float
       %uint_1 = OpConstant %uint 1
       %int_n1 = OpConstant %int -1
       )" << MainPrefix() << R"(
       ; CHECK: %int_0 = OpConstant %int 0
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%int_n1 %uint_1", "%int_0 %uint_1") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACMatrixGeneralClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %v2float = OpTypeVector %float 2
       %mat4v2float = OpTypeMatrix %v2float 4
       %var_ty = OpTypePointer Function %mat4v2float
       %ptr_ty = OpTypePointer Function %float
       %uint_1 = OpConstant %uint 1
       %i = OpUndef %int
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_3 = OpConstant %int 3
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %i %int_0 %int_3
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i %uint_1", "%[[clamp]] %uint_1") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayLeastInboundConstantUntouched) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %uint_200 = OpConstant %uint 200
       %arr = OpTypeArray %float %uint_200
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %int_0 = OpConstant %int 0
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%int_0", "%int_0") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayMostInboundConstantUntouched) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %uint_200 = OpConstant %uint 200
       %arr = OpTypeArray %float %uint_200
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %int_199 = OpConstant %int 199
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%int_199", "%int_199") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayGeneralClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %uint_200 = OpConstant %uint 200
       %arr = OpTypeArray %float %uint_200
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %int
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_199 = OpConstant %int 199
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %i %int_0 %int_199
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayGeneralShortIndexUIntBoundsClamped) {
  // Index is signed short, array bounds overflows the index type.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int16\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesShort() << TypesFloat() << R"(
       %uint_70000 = OpConstant %uint 70000 ; overflows 16bits
       %arr = OpTypeArray %float %uint_70000
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %short
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_69999 = OpConstant %int 69999
       ; CHECK: OpLabel
       ; CHECK: %[[i_ext:\w+]] = OpSConvert %uint %i
       ; CHECK: %[[clamp:\w+]] = OpExtInst %uint %[[GLSLSTD450]] SClamp %[[i_ext]] %int_0 %int_69999
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayGeneralUShortIndexIntBoundsClamped) {
  // Index is unsigned short, array bounds overflows the index type.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int16\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesShort() << TypesFloat() << R"(
       %int_70000 = OpConstant %int 70000 ; overflows 16bits
       %arr = OpTypeArray %float %int_70000
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %ushort
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_69999 = OpConstant %int 69999
       ; CHECK: OpLabel
       ; CHECK: %[[i_ext:\w+]] = OpUConvert %uint %i
       ; CHECK: %[[clamp:\w+]] = OpExtInst %uint %[[GLSLSTD450]] SClamp %[[i_ext]] %int_0 %int_69999
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayGeneralUIntIndexShortBoundsClamped) {
  // Signed int index i is wider than the array bounds type.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int16\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesShort() << TypesFloat() << R"(
       %short_200 = OpConstant %short 200
       %arr = OpTypeArray %float %short_200
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %uint
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_199 = OpConstant %int 199
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %uint %[[GLSLSTD450]] SClamp %i %int_0 %int_199
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayGeneralIntIndexUShortBoundsClamped) {
  // Unsigned int index i is wider than the array bounds type.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int16\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesShort() << TypesFloat() << R"(
       %ushort_200 = OpConstant %ushort 200
       %arr = OpTypeArray %float %ushort_200
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %int
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_199 = OpConstant %int 199
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %i %int_0 %int_199
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayGeneralLongIndexUIntBoundsClamped) {
  // Signed long index i is wider than the array bounds type.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int64\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesLong() << TypesFloat() << R"(
       %uint_200 = OpConstant %uint 200
       %arr = OpTypeArray %float %uint_200
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %long
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %long_0 = OpConstant %long 0
       ; CHECK-DAG: %long_199 = OpConstant %long 199
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %long %[[GLSLSTD450]] SClamp %i %long_0 %long_199
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayGeneralULongIndexIntBoundsClamped) {
  // Unsigned long index i is wider than the array bounds type.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int64\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesLong() << TypesFloat() << R"(
       %int_200 = OpConstant %int 200
       %arr = OpTypeArray %float %int_200
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %ulong
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %long_0 = OpConstant %long 0
       ; CHECK-DAG: %long_199 = OpConstant %long 199
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %ulong %[[GLSLSTD450]] SClamp %i %long_0 %long_199
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest,
       ACArrayGeneralShortIndeArrayBiggerThanShortMaxClipsToShortIntMax) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int16\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesShort()
            << TypesInt() << TypesFloat() << R"(
       %uint_50000 = OpConstant %uint 50000
       %arr = OpTypeArray %float %uint_50000
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %ushort
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %short_0 = OpConstant %short 0
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %short 32767
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %ushort %[[GLSLSTD450]] SClamp %i %short_0 %[[intmax]]
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest,
       ACArrayGeneralIntIndexArrayBiggerThanIntMaxClipsToSignedIntMax) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %uint_3000000000 = OpConstant %uint 3000000000
       %arr = OpTypeArray %float %uint_3000000000
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %uint
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %int 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %uint %[[GLSLSTD450]] SClamp %i %int_0 %[[intmax]]
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%i", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest,
       ACArrayGeneralLongIndexArrayBiggerThanLongMaxClipsToSignedLongMax) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int64\n"
            << ShaderPreambleAC({"i"}) << TypesVoid() << TypesInt()
            << TypesLong()
            << TypesFloat()
            // 2^63 == 9,223,372,036,854,775,807
            << R"(
       %ulong_9223372036854775999 = OpConstant %ulong 9223372036854775999
       %arr = OpTypeArray %float %ulong_9223372036854775999
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %ulong
       )"
            << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %long_0 = OpConstant %long 0
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %long 9223372036854775807
       ; CHECK: OpLabel
       ; CHECK: %[[clamp:\w+]] = OpExtInst %ulong %[[GLSLSTD450]] SClamp %i %long_0 %[[intmax]]
       %var = OpVariable %var_ty Function)" << ACCheck(ac, "%i", "%[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArraySpecIdSizedAlwaysClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"spec200"}) << R"(
       OpDecorate %spec200 SpecId 0 )" << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %spec200 = OpSpecConstant %int 200
       %arr = OpTypeArray %float %spec200
       %var_ty = OpTypePointer Function %arr
       %ptr_ty = OpTypePointer Function %float
       %uint_5 = OpConstant %uint 5
       )" << MainPrefix() << R"(
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %uint_0 = OpConstant %uint 0
       ; CHECK-DAG: %uint_1 = OpConstant %uint 1
       ; CHECK-DAG: %[[uint_intmax:\w+]] = OpConstant %uint 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[max:\w+]] = OpISub %uint %spec200 %uint_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %uint %[[GLSLSTD450]] UMin %[[max]] %[[uint_intmax]]
       ; CHECK: %[[clamp:\w+]] = OpExtInst %uint %[[GLSLSTD450]] SClamp %uint_5 %uint_0 %[[smin]]
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%uint_5", "%[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACStructLeastUntouched) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %struct = OpTypeStruct %float %float %float
       %var_ty = OpTypePointer Function %struct
       %ptr_ty = OpTypePointer Function %float
       %int_0 = OpConstant %int 0
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%int_0", "%int_0") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACStructMostUntouched) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %struct = OpTypeStruct %float %float %float
       %var_ty = OpTypePointer Function %struct
       %ptr_ty = OpTypePointer Function %float
       %int_2 = OpConstant %int 2
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function)"
            << ACCheck(ac, "%int_2", "%int_2") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACStructSpecConstantFail) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"struct", "spec200"})
            << "OpDecorate %spec200 SpecId 0\n"
            <<

        TypesVoid() << TypesInt() << TypesFloat() << R"(
       %spec200 = OpSpecConstant %int 200
       %struct = OpTypeStruct %float %float %float
       %var_ty = OpTypePointer Function %struct
       %ptr_ty = OpTypePointer Function %float
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function
       ; CHECK: Member index into struct is not a constant integer
       ; CHECK-SAME: %spec200 = OpSpecConstant %int 200
       )"
            << ACCheckFail(ac, "%spec200", "%spec200") << MainSuffix();
    SinglePassRunAndFail<GraphicsRobustAccessPass>(shaders.str());
  }
}

TEST_F(GraphicsRobustAccessTest, ACStructFloatConstantFail) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"struct"}) <<

        TypesVoid() << TypesInt() << TypesFloat() << R"(
       %float_2 = OpConstant %float 2
       %struct = OpTypeStruct %float %float %float
       %var_ty = OpTypePointer Function %struct
       %ptr_ty = OpTypePointer Function %float
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function
       ; CHECK: Member index into struct is not a constant integer
       ; CHECK-SAME: %float_2 = OpConstant %float 2
       )"
            << ACCheckFail(ac, "%float_2", "%float_2") << MainSuffix();
    SinglePassRunAndFail<GraphicsRobustAccessPass>(shaders.str());
  }
}

TEST_F(GraphicsRobustAccessTest, ACStructNonConstantFail) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"struct", "i"}) <<

        TypesVoid() << TypesInt() << TypesFloat() << R"(
       %float_2 = OpConstant %float 2
       %struct = OpTypeStruct %float %float %float
       %var_ty = OpTypePointer Function %struct
       %ptr_ty = OpTypePointer Function %float
       %i = OpUndef %int
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function
       ; CHECK: Member index into struct is not a constant integer
       ; CHECK-SAME: %i = OpUndef %int
       )"
            << ACCheckFail(ac, "%i", "%i") << MainSuffix();
    SinglePassRunAndFail<GraphicsRobustAccessPass>(shaders.str());
  }
}

TEST_F(GraphicsRobustAccessTest, ACStructExcessFail) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"struct", "i"}) << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %struct = OpTypeStruct %float %float %float
       %var_ty = OpTypePointer Function %struct
       %ptr_ty = OpTypePointer Function %float
       %i = OpConstant %int 4
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function
       ; CHECK: Member index 4 is out of bounds for struct type:
       ; CHECK-SAME: %struct = OpTypeStruct %float %float %float
       )"
            << ACCheckFail(ac, "%i", "%i") << MainSuffix();
    SinglePassRunAndFail<GraphicsRobustAccessPass>(shaders.str());
  }
}

TEST_F(GraphicsRobustAccessTest, ACStructNegativeFail) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"struct", "i"}) << TypesVoid() << TypesInt()
            << TypesFloat() << R"(
       %struct = OpTypeStruct %float %float %float
       %var_ty = OpTypePointer Function %struct
       %ptr_ty = OpTypePointer Function %float
       %i = OpConstant %int -1
       )" << MainPrefix() << R"(
       %var = OpVariable %var_ty Function
       ; CHECK: Member index -1 is out of bounds for struct type:
       ; CHECK-SAME: %struct = OpTypeStruct %float %float %float
       )"
            << ACCheckFail(ac, "%i", "%i") << MainSuffix();
    SinglePassRunAndFail<GraphicsRobustAccessPass>(shaders.str());
  }
}

TEST_F(GraphicsRobustAccessTest, ACRTArrayLeastInboundClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC() << "OpDecorate %rtarr ArrayStride 4 "
            << DecoSSBO() << TypesVoid() << TypesInt() << TypesFloat() << R"(
       %rtarr = OpTypeRuntimeArray %float
       %ssbo_s = OpTypeStruct %uint %uint %rtarr
       %var_ty = OpTypePointer Uniform %ssbo_s
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %int_0 = OpConstant %int 0
       %int_2 = OpConstant %int 2
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_1 = OpConstant %int 1
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %int 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %var 2
       ; CHECK: %[[max:\w+]] = OpISub %int %[[arrlen]] %int_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %int %[[GLSLSTD450]] UMin %[[max]] %[[intmax]]
       ; CHECK: %[[clamp:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %int_0 %int_0 %[[smin]]
       )"
            << MainPrefix() << ACCheck(ac, "%int_2 %int_0", "%int_2 %[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACRTArrayGeneralShortIndexClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int16\n"
            << ShaderPreambleAC({"i"}) << "OpDecorate %rtarr ArrayStride 4 "
            << DecoSSBO() << TypesVoid() << TypesShort() << TypesFloat() << R"(
       %rtarr = OpTypeRuntimeArray %float
       %ssbo_s = OpTypeStruct %short %short %rtarr
       %var_ty = OpTypePointer Uniform %ssbo_s
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %short_2 = OpConstant %short 2
       %i = OpUndef %short
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK: %uint = OpTypeInt 32 0
       ; CHECK-DAG: %uint_1 = OpConstant %uint 1
       ; CHECK-DAG: %uint_0 = OpConstant %uint 0
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %uint 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %var 2
       ; CHECK-DAG: %[[max:\w+]] = OpISub %uint %[[arrlen]] %uint_1
       ; CHECK-DAG: %[[i_ext:\w+]] = OpSConvert %uint %i
       ; CHECK: %[[smin:\w+]] = OpExtInst %uint %[[GLSLSTD450]] UMin %[[max]] %[[intmax]]
       ; CHECK: %[[clamp:\w+]] = OpExtInst %uint %[[GLSLSTD450]] SClamp %[[i_ext]] %uint_0 %[[smin]]
       )"
            << MainPrefix() << ACCheck(ac, "%short_2 %i", "%short_2 %[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACRTArrayGeneralUShortIndexClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int16\n"
            << ShaderPreambleAC({"i"}) << "OpDecorate %rtarr ArrayStride 4 "
            << DecoSSBO() << TypesVoid() << TypesShort() << TypesFloat() << R"(
       %rtarr = OpTypeRuntimeArray %float
       %ssbo_s = OpTypeStruct %short %short %rtarr
       %var_ty = OpTypePointer Uniform %ssbo_s
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %short_2 = OpConstant %short 2
       %i = OpUndef %ushort
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK: %uint = OpTypeInt 32 0
       ; CHECK-DAG: %uint_1 = OpConstant %uint 1
       ; CHECK-DAG: %uint_0 = OpConstant %uint 0
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %uint 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %var 2
       ; CHECK-DAG: %[[max:\w+]] = OpISub %uint %[[arrlen]] %uint_1
       ; CHECK-DAG: %[[i_ext:\w+]] = OpSConvert %uint %i
       ; CHECK: %[[smin:\w+]] = OpExtInst %uint %[[GLSLSTD450]] UMin %[[max]] %[[intmax]]
       ; CHECK: %[[clamp:\w+]] = OpExtInst %uint %[[GLSLSTD450]] SClamp %[[i_ext]] %uint_0 %[[smin]]
       )"
            << MainPrefix() << ACCheck(ac, "%short_2 %i", "%short_2 %[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACRTArrayGeneralIntIndexClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i"}) << "OpDecorate %rtarr ArrayStride 4 "
            << DecoSSBO() << TypesVoid() << TypesInt() << TypesFloat() << R"(
       %rtarr = OpTypeRuntimeArray %float
       %ssbo_s = OpTypeStruct %int %int %rtarr
       %var_ty = OpTypePointer Uniform %ssbo_s
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %int_2 = OpConstant %int 2
       %i = OpUndef %int
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_1 = OpConstant %int 1
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %int 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %var 2
       ; CHECK: %[[max:\w+]] = OpISub %int %[[arrlen]] %int_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %int %[[GLSLSTD450]] UMin %[[max]] %[[intmax]]
       ; CHECK: %[[clamp:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %i %int_0 %[[smin]]
       )"
            << MainPrefix() << ACCheck(ac, "%int_2 %i", "%int_2 %[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACRTArrayGeneralUIntIndexClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i"}) << "OpDecorate %rtarr ArrayStride 4 "
            << DecoSSBO() << TypesVoid() << TypesInt() << TypesFloat() << R"(
       %rtarr = OpTypeRuntimeArray %float
       %ssbo_s = OpTypeStruct %int %int %rtarr
       %var_ty = OpTypePointer Uniform %ssbo_s
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %int_2 = OpConstant %int 2
       %i = OpUndef %uint
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %uint_1 = OpConstant %uint 1
       ; CHECK-DAG: %uint_0 = OpConstant %uint 0
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %uint 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %var 2
       ; CHECK: %[[max:\w+]] = OpISub %uint %[[arrlen]] %uint_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %uint %[[GLSLSTD450]] UMin %[[max]] %[[intmax]]
       ; CHECK: %[[clamp:\w+]] = OpExtInst %uint %[[GLSLSTD450]] SClamp %i %uint_0 %[[smin]]
       )"
            << MainPrefix() << ACCheck(ac, "%int_2 %i", "%int_2 %[[clamp]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACRTArrayGeneralLongIndexClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int64" << ShaderPreambleAC({"i"})
            << "OpDecorate %rtarr ArrayStride 4 " << DecoSSBO() << TypesVoid()
            << TypesInt() << TypesLong() << TypesFloat() << R"(
       %rtarr = OpTypeRuntimeArray %float
       %ssbo_s = OpTypeStruct %int %int %rtarr
       %var_ty = OpTypePointer Uniform %ssbo_s
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %int_2 = OpConstant %int 2
       %i = OpUndef %long
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %long_0 = OpConstant %long 0
       ; CHECK-DAG: %long_1 = OpConstant %long 1
       ; CHECK-DAG: %[[longmax:\w+]] = OpConstant %long 9223372036854775807
       ; CHECK: OpLabel
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %var 2
       ; CHECK: %[[arrlen_ext:\w+]] = OpUConvert %ulong %[[arrlen]]
       ; CHECK: %[[max:\w+]] = OpISub %long %[[arrlen_ext]] %long_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %long %[[GLSLSTD450]] UMin %[[max]] %[[longmax]]
       ; CHECK: %[[clamp:\w+]] = OpExtInst %long %[[GLSLSTD450]] SClamp %i %long_0 %[[smin]]
       )" << MainPrefix()
            << ACCheck(ac, "%int_2 %i", "%int_2 %[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACRTArrayGeneralULongIndexClamped) {
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << "OpCapability Int64" << ShaderPreambleAC({"i"})
            << "OpDecorate %rtarr ArrayStride 4 " << DecoSSBO() << TypesVoid()
            << TypesInt() << TypesLong() << TypesFloat() << R"(
       %rtarr = OpTypeRuntimeArray %float
       %ssbo_s = OpTypeStruct %int %int %rtarr
       %var_ty = OpTypePointer Uniform %ssbo_s
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %int_2 = OpConstant %int 2
       %i = OpUndef %ulong
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %ulong_0 = OpConstant %ulong 0
       ; CHECK-DAG: %ulong_1 = OpConstant %ulong 1
       ; CHECK-DAG: %[[longmax:\w+]] = OpConstant %ulong 9223372036854775807
       ; CHECK: OpLabel
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %var 2
       ; CHECK: %[[arrlen_ext:\w+]] = OpUConvert %ulong %[[arrlen]]
       ; CHECK: %[[max:\w+]] = OpISub %ulong %[[arrlen_ext]] %ulong_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %ulong %[[GLSLSTD450]] UMin %[[max]] %[[longmax]]
       ; CHECK: %[[clamp:\w+]] = OpExtInst %ulong %[[GLSLSTD450]] SClamp %i %ulong_0 %[[smin]]
       )" << MainPrefix()
            << ACCheck(ac, "%int_2 %i", "%int_2 %[[clamp]]") << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACRTArrayStructVectorElem) {
  // The point of this test is that the access chain can have indices past the
  // index into the runtime array.  For good measure, the index into the final
  // struct is out of bounds.  We have to clamp that index too.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i", "j"})
            << "OpDecorate %rtarr ArrayStride 32\n"
            << DecoSSBO() << "OpMemberDecorate %rtelem 0 Offset 0\n"
            << "OpMemberDecorate %rtelem 1 Offset 16\n"
            << TypesVoid() << TypesInt() << TypesFloat() << R"(
       %v4float = OpTypeVector %float 4
       %rtelem = OpTypeStruct %v4float %v4float
       %rtarr = OpTypeRuntimeArray %rtelem
       %ssbo_s = OpTypeStruct %int %int %rtarr
       %var_ty = OpTypePointer Uniform %ssbo_s
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %int_1 = OpConstant %int 1
       %int_2 = OpConstant %int 2
       %i = OpUndef %int
       %j = OpUndef %int
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_3 = OpConstant %int 3
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %int 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %var 2
       ; CHECK: %[[max:\w+]] = OpISub %int %[[arrlen]] %int_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %int %[[GLSLSTD450]] UMin %[[max]] %[[intmax]]
       ; CHECK: %[[clamp_i:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %i %int_0 %[[smin]]
       ; CHECK: %[[clamp_j:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %j %int_0 %int_3
       )" << MainPrefix()
            << ACCheck(ac, "%int_2 %i %int_1 %j",
                       "%int_2 %[[clamp_i]] %int_1 %[[clamp_j]]")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACArrayRTArrayStructVectorElem) {
  // Now add an additional level of arrays around the Block-decorated struct.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i", "ssbo_s"})
            << "OpDecorate %rtarr ArrayStride 32\n"
            << DecoSSBO() << "OpMemberDecorate %rtelem 0 Offset 0\n"
            << "OpMemberDecorate %rtelem 1 Offset 16\n"
            << TypesVoid() << TypesInt() << TypesFloat() << R"(
       %v4float = OpTypeVector %float 4
       %rtelem = OpTypeStruct %v4float %v4float
       %rtarr = OpTypeRuntimeArray %rtelem
       %ssbo_s = OpTypeStruct %int %int %rtarr
       %arr_size = OpConstant %int 10
       %arr_ssbo = OpTypeArray %ssbo_s %arr_size
       %var_ty = OpTypePointer Uniform %arr_ssbo
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %int_1 = OpConstant %int 1
       %int_2 = OpConstant %int 2
       %int_17 = OpConstant %int 17
       %i = OpUndef %int
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %[[ssbo_p:\w+]] = OpTypePointer Uniform %ssbo_s
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_9 = OpConstant %int 9
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %int 2147483647
       ; CHECK: OpLabel
       ; This access chain is manufatured only so we can compute the array length.
       ; Note that the %int_9 is already clamped
       ; CHECK: %[[ssbo_base:\w+]] = )" << ac
            << R"( %[[ssbo_p]] %var %int_9
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %[[ssbo_base]] 2
       ; CHECK: %[[max:\w+]] = OpISub %int %[[arrlen]] %int_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %int %[[GLSLSTD450]] UMin %[[max]] %[[intmax]]
       ; CHECK: %[[clamp_i:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %i %int_0 %[[smin]]
       )" << MainPrefix()
            << ACCheck(ac, "%int_17 %int_2 %i %int_1 %int_2",
                       "%int_9 %int_2 %[[clamp_i]] %int_1 %int_2")
            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, ACSplitACArrayRTArrayStructVectorElem) {
  // Split the address calculation across two access chains.  Force
  // the transform to walk up the access chains to find the base variable.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i", "j", "k", "ssbo_s", "ssbo_pty",
                                 "rtarr_pty", "ac_ssbo", "ac_rtarr"})
            << "OpDecorate %rtarr ArrayStride 32\n"
            << DecoSSBO() << "OpMemberDecorate %rtelem 0 Offset 0\n"
            << "OpMemberDecorate %rtelem 1 Offset 16\n"
            << TypesVoid() << TypesInt() << TypesFloat() << R"(
       %v4float = OpTypeVector %float 4
       %rtelem = OpTypeStruct %v4float %v4float
       %rtarr = OpTypeRuntimeArray %rtelem
       %ssbo_s = OpTypeStruct %int %int %rtarr
       %arr_size = OpConstant %int 10
       %arr_ssbo = OpTypeArray %ssbo_s %arr_size
       %var_ty = OpTypePointer Uniform %arr_ssbo
       %ssbo_pty = OpTypePointer Uniform %ssbo_s
       %rtarr_pty = OpTypePointer Uniform %rtarr
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %int_1 = OpConstant %int 1
       %int_2 = OpConstant %int 2
       %i = OpUndef %int
       %j = OpUndef %int
       %k = OpUndef %int
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_9 = OpConstant %int 9
       ; CHECK-DAG: %int_3 = OpConstant %int 3
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %int 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[clamp_i:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %i %int_0 %int_9
       ; CHECK: %ac_ssbo = )" << ac
            << R"( %ssbo_pty %var %[[clamp_i]]
       ; CHECK: %ac_rtarr = )"
            << ac << R"( %rtarr_pty %ac_ssbo %int_2

       ; This is the interesting bit.  This array length is needed for an OpAccessChain
       ; computing %ac, but the algorithm had to track back through %ac_rtarr's
       ; definition to find the base pointer %ac_ssbo.
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %ac_ssbo 2
       ; CHECK: %[[max:\w+]] = OpISub %int %[[arrlen]] %int_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %int %[[GLSLSTD450]] UMin %[[max]] %[[intmax]]
       ; CHECK: %[[clamp_j:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %j %int_0 %[[smin]]
       ; CHECK: %[[clamp_k:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %k %int_0 %int_3
       ; CHECK: %ac = )" << ac
            << R"( %ptr_ty %ac_rtarr %[[clamp_j]] %int_1 %[[clamp_k]]
       ; CHECK-NOT: AccessChain
       )" << MainPrefix()
            << "%ac_ssbo = " << ac << " %ssbo_pty %var %i\n"
            << "%ac_rtarr = " << ac << " %rtarr_pty %ac_ssbo %int_2\n"
            << "%ac = " << ac << " %ptr_ty %ac_rtarr %j %int_1 %k\n"

            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest,
       ACSplitACArrayRTArrayStructVectorElemAcrossBasicBlocks) {
  // Split the address calculation across two access chains.  Force
  // the transform to walk up the access chains to find the base variable.
  // This time, put the different access chains in different basic blocks.
  // This is an integrity check to ensure that we keep the instruction-to-block
  // mapping consistent.
  for (auto* ac : AccessChains()) {
    std::ostringstream shaders;
    shaders << ShaderPreambleAC({"i", "j", "k", "bb1", "bb2", "ssbo_s",
                                 "ssbo_pty", "rtarr_pty", "ac_ssbo",
                                 "ac_rtarr"})
            << "OpDecorate %rtarr ArrayStride 32\n"
            << DecoSSBO() << "OpMemberDecorate %rtelem 0 Offset 0\n"
            << "OpMemberDecorate %rtelem 1 Offset 16\n"
            << TypesVoid() << TypesInt() << TypesFloat() << R"(
       %v4float = OpTypeVector %float 4
       %rtelem = OpTypeStruct %v4float %v4float
       %rtarr = OpTypeRuntimeArray %rtelem
       %ssbo_s = OpTypeStruct %int %int %rtarr
       %arr_size = OpConstant %int 10
       %arr_ssbo = OpTypeArray %ssbo_s %arr_size
       %var_ty = OpTypePointer Uniform %arr_ssbo
       %ssbo_pty = OpTypePointer Uniform %ssbo_s
       %rtarr_pty = OpTypePointer Uniform %rtarr
       %ptr_ty = OpTypePointer Uniform %float
       %var = OpVariable %var_ty Uniform
       %int_1 = OpConstant %int 1
       %int_2 = OpConstant %int 2
       %i = OpUndef %int
       %j = OpUndef %int
       %k = OpUndef %int
       ; CHECK: %[[GLSLSTD450:\w+]] = OpExtInstImport "GLSL.std.450"
       ; CHECK-DAG: %int_0 = OpConstant %int 0
       ; CHECK-DAG: %int_9 = OpConstant %int 9
       ; CHECK-DAG: %int_3 = OpConstant %int 3
       ; CHECK-DAG: %[[intmax:\w+]] = OpConstant %int 2147483647
       ; CHECK: OpLabel
       ; CHECK: %[[clamp_i:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %i %int_0 %int_9
       ; CHECK: %ac_ssbo = )" << ac
            << R"( %ssbo_pty %var %[[clamp_i]]
       ; CHECK: %bb1 = OpLabel
       ; CHECK: %ac_rtarr = )"
            << ac << R"( %rtarr_pty %ac_ssbo %int_2
       ; CHECK: %bb2 = OpLabel

       ; This is the interesting bit.  This array length is needed for an OpAccessChain
       ; computing %ac, but the algorithm had to track back through %ac_rtarr's
       ; definition to find the base pointer %ac_ssbo.
       ; CHECK: %[[arrlen:\w+]] = OpArrayLength %uint %ac_ssbo 2
       ; CHECK: %[[max:\w+]] = OpISub %int %[[arrlen]] %int_1
       ; CHECK: %[[smin:\w+]] = OpExtInst %int %[[GLSLSTD450]] UMin %[[max]] %[[intmax]]
       ; CHECK: %[[clamp_j:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %j %int_0 %[[smin]]
       ; CHECK: %[[clamp_k:\w+]] = OpExtInst %int %[[GLSLSTD450]] SClamp %k %int_0 %int_3
       ; CHECK: %ac = )" << ac
            << R"( %ptr_ty %ac_rtarr %[[clamp_j]] %int_1 %[[clamp_k]]
       ; CHECK-NOT: AccessChain
       )" << MainPrefix()
            << "%ac_ssbo = " << ac << " %ssbo_pty %var %i\n"
            << "OpBranch %bb1\n%bb1 = OpLabel\n"
            << "%ac_rtarr = " << ac << " %rtarr_pty %ac_ssbo %int_2\n"
            << "OpBranch %bb2\n%bb2 = OpLabel\n"
            << "%ac = " << ac << " %ptr_ty %ac_rtarr %j %int_1 %k\n"

            << MainSuffix();
    SinglePassRunAndMatch<GraphicsRobustAccessPass>(shaders.str(), true);
  }
}

TEST_F(GraphicsRobustAccessTest, bug3813) {
  // This shader comes from Dawn's
  // TextureViewSamplingTest.TextureCubeMapOnWholeTexture, converted from GLSL
  // by glslang.
  // The pass was inserting a signed 32-bit int type, but not correctly marking
  // the shader as changed.
  std::string shader = R"(
; SPIR-V
; Version: 1.0
; Generator: Google Shaderc over Glslang; 10
; Bound: 46
; Schema: 0
       OpCapability Shader
  %1 = OpExtInstImport "GLSL.std.450"
       OpMemoryModel Logical GLSL450
       OpEntryPoint Fragment %4 "main" %12 %29
       OpExecutionMode %4 OriginUpperLeft
       OpSource GLSL 450
       OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
       OpSourceExtension "GL_GOOGLE_include_directive"
       OpName %4 "main"
       OpName %8 "sc"
       OpName %12 "texCoord"
       OpName %21 "tc"
       OpName %29 "fragColor"
       OpName %32 "texture0"
       OpName %36 "sampler0"
       OpDecorate %12 Location 0
       OpDecorate %29 Location 0
       OpDecorate %32 DescriptorSet 0
       OpDecorate %32 Binding 1
       OpDecorate %36 DescriptorSet 0
       OpDecorate %36 Binding 0
  %2 = OpTypeVoid
  %3 = OpTypeFunction %2
  %6 = OpTypeFloat 32
  %7 = OpTypePointer Function %6
  %9 = OpConstant %6 2
 %10 = OpTypeVector %6 2
 %11 = OpTypePointer Input %10
 %12 = OpVariable %11 Input
 %13 = OpTypeInt 32 0
 %14 = OpConstant %13 0
 %15 = OpTypePointer Input %6
 %19 = OpConstant %6 1
 %22 = OpConstant %13 1
 %27 = OpTypeVector %6 4
 %28 = OpTypePointer Output %27
 %29 = OpVariable %28 Output
 %30 = OpTypeImage %6 Cube 0 0 0 1 Unknown
 %31 = OpTypePointer UniformConstant %30
 %32 = OpVariable %31 UniformConstant
 %34 = OpTypeSampler
 %35 = OpTypePointer UniformConstant %34
 %36 = OpVariable %35 UniformConstant
 %38 = OpTypeSampledImage %30
 %43 = OpTypeVector %6 3
  %4 = OpFunction %2 None %3
  %5 = OpLabel
  %8 = OpVariable %7 Function
 %21 = OpVariable %7 Function
 %16 = OpAccessChain %15 %12 %14
 %17 = OpLoad %6 %16
 %18 = OpFMul %6 %9 %17
 %20 = OpFSub %6 %18 %19
       OpStore %8 %20
 %23 = OpAccessChain %15 %12 %22
 %24 = OpLoad %6 %23
 %25 = OpFMul %6 %9 %24
 %26 = OpFSub %6 %25 %19
       OpStore %21 %26
 %33 = OpLoad %30 %32
 %37 = OpLoad %34 %36
 %39 = OpSampledImage %38 %33 %37
 %40 = OpLoad %6 %21
 %41 = OpLoad %6 %8
 %42 = OpFNegate %6 %41
 %44 = OpCompositeConstruct %43 %19 %40 %42
 %45 = OpImageSampleImplicitLod %27 %39 %44
       OpStore %29 %45
       OpReturn
       OpFunctionEnd
)";

  std::string expected = R"(OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %texCoord %fragColor
OpExecutionMode %main OriginUpperLeft
OpSource GLSL 450
OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
OpSourceExtension "GL_GOOGLE_include_directive"
OpName %main "main"
OpName %sc "sc"
OpName %texCoord "texCoord"
OpName %tc "tc"
OpName %fragColor "fragColor"
OpName %texture0 "texture0"
OpName %sampler0 "sampler0"
OpDecorate %texCoord Location 0
OpDecorate %fragColor Location 0
OpDecorate %texture0 DescriptorSet 0
OpDecorate %texture0 Binding 1
OpDecorate %sampler0 DescriptorSet 0
OpDecorate %sampler0 Binding 0
%void = OpTypeVoid
%10 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%v2float = OpTypeVector %float 2
%_ptr_Input_v2float = OpTypePointer Input %v2float
%texCoord = OpVariable %_ptr_Input_v2float Input
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%float_1 = OpConstant %float 1
%uint_1 = OpConstant %uint 1
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%fragColor = OpVariable %_ptr_Output_v4float Output
%23 = OpTypeImage %float Cube 0 0 0 1 Unknown
%_ptr_UniformConstant_23 = OpTypePointer UniformConstant %23
%texture0 = OpVariable %_ptr_UniformConstant_23 UniformConstant
%25 = OpTypeSampler
%_ptr_UniformConstant_25 = OpTypePointer UniformConstant %25
%sampler0 = OpVariable %_ptr_UniformConstant_25 UniformConstant
%27 = OpTypeSampledImage %23
%v3float = OpTypeVector %float 3
%int = OpTypeInt 32 1
%main = OpFunction %void None %10
%29 = OpLabel
%sc = OpVariable %_ptr_Function_float Function
%tc = OpVariable %_ptr_Function_float Function
%30 = OpAccessChain %_ptr_Input_float %texCoord %uint_0
%31 = OpLoad %float %30
%32 = OpFMul %float %float_2 %31
%33 = OpFSub %float %32 %float_1
OpStore %sc %33
%34 = OpAccessChain %_ptr_Input_float %texCoord %uint_1
%35 = OpLoad %float %34
%36 = OpFMul %float %float_2 %35
%37 = OpFSub %float %36 %float_1
OpStore %tc %37
%38 = OpLoad %23 %texture0
%39 = OpLoad %25 %sampler0
%40 = OpSampledImage %27 %38 %39
%41 = OpLoad %float %tc
%42 = OpLoad %float %sc
%43 = OpFNegate %float %42
%44 = OpCompositeConstruct %v3float %float_1 %41 %43
%45 = OpImageSampleImplicitLod %v4float %40 %44
OpStore %fragColor %45
OpReturn
OpFunctionEnd
)";

  SinglePassRunAndCheck<GraphicsRobustAccessPass>(shader, expected, false,
                                                  true);
}

TEST_F(GraphicsRobustAccessTest, ReplaceIndexReportsChanged) {
  // A ClusterFuzz generated shader that triggered a
  // "Binary size unexpectedly changed despite the optimizer saying there was no
  // change" assertion.
  // See https://github.com/KhronosGroup/SPIRV-Tools/issues/4166.
  std::string shader = R"(
; SPIR-V
; Version: 1.0
; Generator: Google Shaderc over Glslang; 245
; Bound: 41
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "else" %gl_GlobalInvocationID
               OpExecutionMode %main LocalSize 1 1 3338665985
               OpSource GLSL 450
               OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
               OpSourceExtension "GL_GOOGLE_include_directive"
               OpName %main "main"
               OpName %index "index"
               OpName %gl_GlobalInvocationID "gl_GlobalInvocationID"
               OpName %S "S"
               OpMemberName %_struct_24 0 ""
               OpMemberName %_struct_24 1 ""
               OpName %Dst "Dst"
               OpMemberName %Dst 0 "s"
               OpName %dst "dst"
               OpName %Src "Src"
               OpMemberName %Src 0 "s"
               OpName %src "src"
               OpDecorate %gl_GlobalInvocationID BuiltIn GlobalInvocationId
               OpMemberDecorate %_struct_24 0 Offset 64
               OpMemberDecorate %_struct_24 1 Offset 8
               OpDecorate %_arr__struct_24_uint_1 ArrayStride 16
               OpMemberDecorate %Dst 0 Offset 0
               OpDecorate %Dst BufferBlock
               OpDecorate %dst DescriptorSet 0
               OpDecorate %dst Binding 1
               OpDecorate %_arr__struct_24_uint_1_0 ArrayStride 16
               OpMemberDecorate %Src 0 Offset 0
               OpDecorate %Src Block
               OpDecorate %src DescriptorSet 0
               OpDecorate %src Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %v3uint = OpTypeVector %uint 3
%_ptr_Input_v3uint = OpTypePointer Input %v3uint
%gl_GlobalInvocationID = OpVariable %_ptr_Input_v3uint Input
  %uint_4864 = OpConstant %uint 4864
%_ptr_Input_uint = OpTypePointer Input %uint
     %uint_1 = OpConstant %uint 1
       %bool = OpTypeBool
     %v2uint = OpTypeVector %uint 2
 %_struct_24 = OpTypeStruct %_ptr_Input_uint %v2uint
%_arr__struct_24_uint_1 = OpTypeArray %_struct_24 %uint_1
        %Dst = OpTypeStruct %_arr__struct_24_uint_1
%_ptr_Uniform_Dst = OpTypePointer Uniform %Dst
        %dst = OpVariable %_ptr_Uniform_Dst Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_arr__struct_24_uint_1_0 = OpTypeArray %_struct_24 %uint_1
        %Src = OpTypeStruct %_arr__struct_24_uint_1_0
%_ptr_Uniform_Src = OpTypePointer Uniform %Src
        %src = OpVariable %_ptr_Uniform_Src Uniform
%_ptr_Uniform__struct_24 = OpTypePointer Uniform %_struct_24
       %main = OpFunction %void None %3
          %5 = OpLabel
      %index = OpVariable %_ptr_Function_uint Function
         %14 = OpAccessChain %_ptr_Input_uint %gl_GlobalInvocationID %uint_4864
         %15 = OpLoad %uint %14
               OpStore %index %15
         %16 = OpLoad %uint %index
          %S = OpUGreaterThanEqual %bool %16 %uint_1
               OpSelectionMerge %21 None
               OpBranchConditional %S %20 %21
         %20 = OpLabel
               OpReturn
         %21 = OpLabel
         %31 = OpLoad %uint %index
         %36 = OpLoad %uint %index
         %38 = OpAccessChain %_ptr_Uniform__struct_24 %src %int_0 %36
         %39 = OpLoad %_struct_24 %38
         %40 = OpAccessChain %_ptr_Uniform__struct_24 %dst %int_0 %31
               OpStore %40 %39
               OpReturn
               OpFunctionEnd
)";

  std::vector<uint32_t> optimized_bin;
  auto status = spvtools::opt::Pass::Status::Failure;
  std::tie(optimized_bin, status) =
      SinglePassRunToBinary<GraphicsRobustAccessPass>(shader, false);
  // Check whether the pass returns the correct modification indication.
  EXPECT_EQ(status, spvtools::opt::Pass::Status::SuccessWithChange);
}

// TODO(dneto): Test access chain index wider than 64 bits?
// TODO(dneto): Test struct access chain index wider than 64 bits?
// TODO(dneto): OpImageTexelPointer
//   - all Dim types: 1D 2D Cube 3D Rect Buffer
//   - all Dim types that can be arrayed: 1D 2D 3D
//   - sample index: set to 0 if not multisampled
//   - Dim (2D, Cube Rect} with multisampling
//      -1 0 max excess
// TODO(dneto): Test OpImageTexelPointer with coordinate component index other
// than 32 bits.

}  // namespace
