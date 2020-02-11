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
#include "gmock/gmock.h"

#include "spirv-tools/optimizer.hpp"

#include "test/opt/pass_fixture.h"
#include "test/opt/pass_utils.h"

namespace spvtools {
namespace opt {
namespace {

using StripLineReflectInfoTest = PassTest<::testing::Test>;

// This test acts as an end-to-end code example on how to strip
// reflection info from a SPIR-V module.  Use this code pattern
// when you have compiled HLSL code with Glslang or DXC using
// option -fhlsl_functionality1 to insert reflection information,
// but then want to filter out the extra instructions before sending
// it to a driver that does not implement VK_GOOGLE_hlsl_functionality1.
TEST_F(StripLineReflectInfoTest, StripReflectEnd2EndExample) {
  // This is a non-sensical example, but exercises the instructions.
  std::string before = R"(OpCapability Shader
OpCapability Linkage
OpExtension "SPV_GOOGLE_decorate_string"
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpMemoryModel Logical Simple
OpDecorateStringGOOGLE %float HlslSemanticGOOGLE "foobar"
OpDecorateStringGOOGLE %void HlslSemanticGOOGLE "my goodness"
%void = OpTypeVoid
%float = OpTypeFloat 32
)";
  SpirvTools tools(SPV_ENV_UNIVERSAL_1_1);
  std::vector<uint32_t> binary_in;
  tools.Assemble(before, &binary_in);

  // Instantiate the optimizer, and run the strip-reflection-info
  // pass over the |binary_in| module, and place the modified module
  // into |binary_out|.
  spvtools::Optimizer optimizer(SPV_ENV_UNIVERSAL_1_1);
  optimizer.RegisterPass(spvtools::CreateStripReflectInfoPass());
  std::vector<uint32_t> binary_out;
  optimizer.Run(binary_in.data(), binary_in.size(), &binary_out);

  // Check results
  std::string disassembly;
  tools.Disassemble(binary_out.data(), binary_out.size(), &disassembly);
  std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical Simple
%void = OpTypeVoid
%float = OpTypeFloat 32
)";
  EXPECT_THAT(disassembly, testing::Eq(after));
}

// This test is functionally the same as the end-to-end test above,
// but uses the test SinglePassRunAndCheck test fixture instead.
TEST_F(StripLineReflectInfoTest, StripHlslSemantic) {
  // This is a non-sensical example, but exercises the instructions.
  std::string before = R"(OpCapability Shader
OpCapability Linkage
OpExtension "SPV_GOOGLE_decorate_string"
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpMemoryModel Logical Simple
OpDecorateStringGOOGLE %float HlslSemanticGOOGLE "foobar"
OpDecorateStringGOOGLE %void HlslSemanticGOOGLE "my goodness"
%void = OpTypeVoid
%float = OpTypeFloat 32
)";
  std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical Simple
%void = OpTypeVoid
%float = OpTypeFloat 32
)";

  SinglePassRunAndCheck<StripReflectInfoPass>(before, after, false);
}

TEST_F(StripLineReflectInfoTest, StripHlslCounterBuffer) {
  std::string before = R"(OpCapability Shader
OpCapability Linkage
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpMemoryModel Logical Simple
OpDecorateId %void HlslCounterBufferGOOGLE %float
%void = OpTypeVoid
%float = OpTypeFloat 32
)";
  std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical Simple
%void = OpTypeVoid
%float = OpTypeFloat 32
)";

  SinglePassRunAndCheck<StripReflectInfoPass>(before, after, false);
}

TEST_F(StripLineReflectInfoTest, StripHlslSemanticOnMember) {
  // This is a non-sensical example, but exercises the instructions.
  std::string before = R"(OpCapability Shader
OpCapability Linkage
OpExtension "SPV_GOOGLE_decorate_string"
OpExtension "SPV_GOOGLE_hlsl_functionality1"
OpMemoryModel Logical Simple
OpMemberDecorateStringGOOGLE %struct 0 HlslSemanticGOOGLE "foobar"
%float = OpTypeFloat 32
%_struct_3 = OpTypeStruct %float
)";
  std::string after = R"(OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical Simple
%float = OpTypeFloat 32
%_struct_3 = OpTypeStruct %float
)";

  SinglePassRunAndCheck<StripReflectInfoPass>(before, after, false);
}

}  // namespace
}  // namespace opt
}  // namespace spvtools
