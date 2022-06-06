// Copyright (c) 2022 Google LLC.
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

#include "source/diff/diff.h"

#include "diff_test_utils.h"

#include "source/opt/build_module.h"
#include "source/opt/ir_context.h"
#include "source/spirv_constant.h"
#include "spirv-tools/libspirv.hpp"
#include "tools/io.h"
#include "tools/util/cli_consumer.h"

#include <fstream>
#include <string>

#include "gtest/gtest.h"

namespace spvtools {
namespace diff {
namespace {

constexpr auto kDefaultEnvironment = SPV_ENV_UNIVERSAL_1_6;

std::unique_ptr<spvtools::opt::IRContext> Assemble(const std::string& spirv) {
  spvtools::SpirvTools t(kDefaultEnvironment);
  t.SetMessageConsumer(spvtools::utils::CLIMessageConsumer);
  std::vector<uint32_t> binary;
  if (!t.Assemble(spirv, &binary,
                  spvtools::SpirvTools::kDefaultAssembleOption |
                      SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS))
    return nullptr;
  return spvtools::BuildModule(kDefaultEnvironment,
                               spvtools::utils::CLIMessageConsumer,
                               binary.data(), binary.size());
}

TEST(DiffIndentTest, Diff) {
  const std::string src = R"(OpCapability Shader
    %ext_inst = OpExtInstImport "GLSL.std.450"
    OpMemoryModel Logical GLSL450
    OpEntryPoint Fragment %main "main"
    OpExecutionMode %main OriginUpperLeft
    %void = OpTypeVoid
    %func = OpTypeFunction %void

    %main = OpFunction %void None %func
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd;)";

  const std::string dst = R"(OpCapability Shader
    OpMemoryModel Logical GLSL450
    OpEntryPoint Fragment %main "main"
    OpExecutionMode %main OriginUpperLeft
    %void = OpTypeVoid
    %func = OpTypeFunction %void

    %main = OpFunction %void None %func
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd;)";

  const std::string diff = R"( ; SPIR-V
 ; Version: 1.6
 ; Generator: Khronos SPIR-V Tools Assembler; 0
 ; Bound: 6
 ; Schema: 0
                OpCapability Shader
-          %1 = OpExtInstImport "GLSL.std.450"
                OpMemoryModel Logical GLSL450
                OpEntryPoint Fragment %2 "main"
                OpExecutionMode %2 OriginUpperLeft
           %3 = OpTypeVoid
           %4 = OpTypeFunction %3
           %2 = OpFunction %3 None %4
           %5 = OpLabel
                OpReturn
                OpFunctionEnd
)";

  Options options;
  options.indent = true;
  DoStringDiffTest(src, dst, diff, options);
}

TEST(DiffNoHeaderTest, Diff) {
  const std::string src = R"(OpCapability Shader
    %ext_inst = OpExtInstImport "GLSL.std.450"
    OpMemoryModel Logical GLSL450
    OpEntryPoint Fragment %main "main"
    OpExecutionMode %main OriginUpperLeft
    %void = OpTypeVoid
    %func = OpTypeFunction %void

    %main = OpFunction %void None %func
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd;)";

  const std::string dst = R"(OpCapability Shader
    OpMemoryModel Logical GLSL450
    OpEntryPoint Fragment %main "main"
    OpExecutionMode %main OriginUpperLeft
    %void = OpTypeVoid
    %func = OpTypeFunction %void

    %main = OpFunction %void None %func
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd;)";

  const std::string diff = R"( OpCapability Shader
-%1 = OpExtInstImport "GLSL.std.450"
 OpMemoryModel Logical GLSL450
 OpEntryPoint Fragment %2 "main"
 OpExecutionMode %2 OriginUpperLeft
 %3 = OpTypeVoid
 %4 = OpTypeFunction %3
 %2 = OpFunction %3 None %4
 %5 = OpLabel
 OpReturn
 OpFunctionEnd
)";

  Options options;
  options.no_header = true;
  DoStringDiffTest(src, dst, diff, options);
}

TEST(DiffHeaderTest, Diff) {
  const std::string src_spirv = R"(OpCapability Shader
    %ext_inst = OpExtInstImport "GLSL.std.450"
    OpMemoryModel Logical GLSL450
    OpEntryPoint Fragment %main "main"
    OpExecutionMode %main OriginUpperLeft
    %void = OpTypeVoid
    %func = OpTypeFunction %void

    %main = OpFunction %void None %func
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd;)";

  const std::string dst_spirv = R"(OpCapability Shader
    OpMemoryModel Logical GLSL450
    OpEntryPoint Fragment %main "main"
    OpExecutionMode %main OriginUpperLeft
    %void = OpTypeVoid
    %func = OpTypeFunction %void

    %main = OpFunction %void None %func
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd;)";

  const std::string diff = R"( ; SPIR-V
-; Version: 1.3
+; Version: 1.2
-; Generator: Khronos SPIR-V Tools Assembler; 3
+; Generator: Khronos Glslang Reference Front End; 10
 ; Bound: 6
 ; Schema: 0
 OpCapability Shader
-%1 = OpExtInstImport "GLSL.std.450"
 OpMemoryModel Logical GLSL450
 OpEntryPoint Fragment %2 "main"
 OpExecutionMode %2 OriginUpperLeft
 %3 = OpTypeVoid
 %4 = OpTypeFunction %3
 %2 = OpFunction %3 None %4
 %5 = OpLabel
 OpReturn
 OpFunctionEnd
)";

  // Load the src and dst modules
  std::unique_ptr<spvtools::opt::IRContext> src = Assemble(src_spirv);
  ASSERT_TRUE(src);

  std::unique_ptr<spvtools::opt::IRContext> dst = Assemble(dst_spirv);
  ASSERT_TRUE(dst);

  // Differentiate them in the header.
  const spvtools::opt::ModuleHeader src_header = {
      SpvMagicNumber,
      SPV_SPIRV_VERSION_WORD(1, 3),
      SPV_GENERATOR_WORD(SPV_GENERATOR_KHRONOS_ASSEMBLER, 3),
      src->module()->IdBound(),
      src->module()->schema(),
  };
  const spvtools::opt::ModuleHeader dst_header = {
      SpvMagicNumber,
      SPV_SPIRV_VERSION_WORD(1, 2),
      SPV_GENERATOR_WORD(SPV_GENERATOR_KHRONOS_GLSLANG, 10),
      dst->module()->IdBound(),
      dst->module()->schema(),
  };

  src->module()->SetHeader(src_header);
  dst->module()->SetHeader(dst_header);

  // Take the diff
  Options options;
  std::ostringstream diff_result;
  spv_result_t result =
      spvtools::diff::Diff(src.get(), dst.get(), diff_result, options);
  ASSERT_EQ(result, SPV_SUCCESS);

  // Expect they match
  EXPECT_EQ(diff_result.str(), diff);
}

}  // namespace
}  // namespace diff
}  // namespace spvtools
