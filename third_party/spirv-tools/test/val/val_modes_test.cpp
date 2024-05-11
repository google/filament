// Copyright (c) 2018 Google LLC.
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

#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "source/spirv_target_env.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Combine;
using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateMode = spvtest::ValidateBase<bool>;

const std::string kVoidFunction = R"(%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

TEST_F(ValidateMode, GLComputeNoMode) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, GLComputeNoModeVulkan) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-LocalSize-06426"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "In the Vulkan environment, GLCompute execution model entry "
          "points require either the LocalSize or LocalSizeId execution mode "
          "or an object decorated with WorkgroupSize must be specified."));
}

TEST_F(ValidateMode, GLComputeNoModeVulkanWorkgroupSize) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpDecorate %int3_1 BuiltIn WorkgroupSize
%int = OpTypeInt 32 0
%int3 = OpTypeVector %int 3
%int_1 = OpConstant %int 1
%int3_1 = OpConstantComposite %int3 %int_1 %int_1 %int_1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateMode, GLComputeVulkanLocalSize) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionMode %main LocalSize 1 1 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateMode, GLComputeVulkanLocalSizeIdBad) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_1;  // need SPIR-V 1.2
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("LocalSizeId mode is not allowed by the current environment."));
}

TEST_F(ValidateMode, GLComputeVulkanLocalSizeIdGood) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_1;  // need SPIR-V 1.2
  CompileSuccessfully(spirv, env);
  spvValidatorOptionsSetAllowLocalSizeId(getValidatorOptions(), true);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateMode, FragmentOriginLowerLeftVulkan) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginLowerLeft
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-OriginLowerLeft-04653"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("In the Vulkan environment, the OriginLowerLeft "
                        "execution mode must not be used."));
}

TEST_F(ValidateMode, FragmentPixelCenterIntegerVulkan) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main PixelCenterInteger
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_VULKAN_1_0;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              AnyVUID("VUID-StandaloneSpirv-PixelCenterInteger-04654"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("In the Vulkan environment, the PixelCenterInteger "
                        "execution mode must not be used."));
}

TEST_F(ValidateMode, GeometryNoOutputMode) {
  const std::string spirv = R"(
OpCapability Geometry
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main"
OpExecutionMode %main InputPoints
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Geometry execution model entry points must specify "
                        "exactly one of OutputPoints, OutputLineStrip or "
                        "OutputTriangleStrip execution modes."));
}

TEST_F(ValidateMode, GeometryNoInputMode) {
  const std::string spirv = R"(
OpCapability Geometry
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main"
OpExecutionMode %main OutputPoints
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Geometry execution model entry points must specify exactly "
                "one of InputPoints, InputLines, InputLinesAdjacency, "
                "Triangles or InputTrianglesAdjacency execution modes."));
}

TEST_F(ValidateMode, FragmentNoOrigin) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points require either an "
                "OriginUpperLeft or OriginLowerLeft execution mode."));
}

TEST_F(ValidateMode, FragmentBothOrigins) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main OriginLowerLeft
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points can only specify one of "
                "OriginUpperLeft or OriginLowerLeft execution modes."));
}

TEST_F(ValidateMode, FragmentDepthGreaterAndLess) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main DepthGreater
OpExecutionMode %main DepthLess
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Fragment execution model entry points can specify at "
                        "most one of DepthGreater, DepthLess or DepthUnchanged "
                        "execution modes."));
}

TEST_F(ValidateMode, FragmentDepthGreaterAndUnchanged) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main DepthGreater
OpExecutionMode %main DepthUnchanged
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Fragment execution model entry points can specify at "
                        "most one of DepthGreater, DepthLess or DepthUnchanged "
                        "execution modes."));
}

TEST_F(ValidateMode, FragmentDepthLessAndUnchanged) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main DepthLess
OpExecutionMode %main DepthUnchanged
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Fragment execution model entry points can specify at "
                        "most one of DepthGreater, DepthLess or DepthUnchanged "
                        "execution modes."));
}

TEST_F(ValidateMode, FragmentAllDepths) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main DepthGreater
OpExecutionMode %main DepthLess
OpExecutionMode %main DepthUnchanged
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Fragment execution model entry points can specify at "
                        "most one of DepthGreater, DepthLess or DepthUnchanged "
                        "execution modes."));
}

TEST_F(ValidateMode, TessellationControlSpacingEqualAndFractionalOdd) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalOdd
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode, TessellationControlSpacingEqualAndSpacingFractionalEven) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode,
       TessellationControlSpacingFractionalOddAndSpacingFractionalEven) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main SpacingFractionalOdd
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode, TessellationControlAllSpacing) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalOdd
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode,
       TessellationEvaluationSpacingEqualAndSpacingFractionalOdd) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalOdd
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode,
       TessellationEvaluationSpacingEqualAndSpacingFractionalEven) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode,
       TessellationEvaluationSpacingFractionalOddAndSpacingFractionalEven) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main SpacingFractionalOdd
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode, TessellationEvaluationAllSpacing) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main SpacingEqual
OpExecutionMode %main SpacingFractionalOdd
OpExecutionMode %main SpacingFractionalEven
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Tessellation execution model entry points can specify "
                        "at most one of SpacingEqual, SpacingFractionalOdd or "
                        "SpacingFractionalEven execution modes."));
}

TEST_F(ValidateMode, TessellationControlBothVertex) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationControl %main "main"
OpExecutionMode %main VertexOrderCw
OpExecutionMode %main VertexOrderCcw
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Tessellation execution model entry points can specify at most "
                "one of VertexOrderCw or VertexOrderCcw execution modes."));
}

TEST_F(ValidateMode, TessellationEvaluationBothVertex) {
  const std::string spirv = R"(
OpCapability Tessellation
OpMemoryModel Logical GLSL450
OpEntryPoint TessellationEvaluation %main "main"
OpExecutionMode %main VertexOrderCw
OpExecutionMode %main VertexOrderCcw
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Tessellation execution model entry points can specify at most "
                "one of VertexOrderCw or VertexOrderCcw execution modes."));
}

using ValidateModeGeometry = spvtest::ValidateBase<std::tuple<
    std::tuple<std::string, std::string, std::string, std::string, std::string>,
    std::tuple<std::string, std::string, std::string>>>;

TEST_P(ValidateModeGeometry, ExecutionMode) {
  std::vector<std::string> input_modes;
  std::vector<std::string> output_modes;
  input_modes.push_back(std::get<0>(std::get<0>(GetParam())));
  input_modes.push_back(std::get<1>(std::get<0>(GetParam())));
  input_modes.push_back(std::get<2>(std::get<0>(GetParam())));
  input_modes.push_back(std::get<3>(std::get<0>(GetParam())));
  input_modes.push_back(std::get<4>(std::get<0>(GetParam())));
  output_modes.push_back(std::get<0>(std::get<1>(GetParam())));
  output_modes.push_back(std::get<1>(std::get<1>(GetParam())));
  output_modes.push_back(std::get<2>(std::get<1>(GetParam())));

  std::ostringstream sstr;
  sstr << "OpCapability Geometry\n";
  sstr << "OpMemoryModel Logical GLSL450\n";
  sstr << "OpEntryPoint Geometry %main \"main\"\n";
  size_t num_input_modes = 0;
  for (auto input : input_modes) {
    if (!input.empty()) {
      num_input_modes++;
      sstr << "OpExecutionMode %main " << input << "\n";
    }
  }
  size_t num_output_modes = 0;
  for (auto output : output_modes) {
    if (!output.empty()) {
      num_output_modes++;
      sstr << "OpExecutionMode %main " << output << "\n";
    }
  }
  sstr << "%void = OpTypeVoid\n";
  sstr << "%void_fn = OpTypeFunction %void\n";
  sstr << "%int = OpTypeInt 32 0\n";
  sstr << "%int1 = OpConstant %int 1\n";
  sstr << "%main = OpFunction %void None %void_fn\n";
  sstr << "%entry = OpLabel\n";
  sstr << "OpReturn\n";
  sstr << "OpFunctionEnd\n";

  CompileSuccessfully(sstr.str());
  if (num_input_modes == 1 && num_output_modes == 1) {
    EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
  } else {
    EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
    if (num_input_modes != 1) {
      EXPECT_THAT(getDiagnosticString(),
                  HasSubstr("Geometry execution model entry points must "
                            "specify exactly one of InputPoints, InputLines, "
                            "InputLinesAdjacency, Triangles or "
                            "InputTrianglesAdjacency execution modes."));
    } else {
      EXPECT_THAT(
          getDiagnosticString(),
          HasSubstr("Geometry execution model entry points must specify "
                    "exactly one of OutputPoints, OutputLineStrip or "
                    "OutputTriangleStrip execution modes."));
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
    GeometryRequiredModes, ValidateModeGeometry,
    Combine(Combine(Values("InputPoints", ""), Values("InputLines", ""),
                    Values("InputLinesAdjacency", ""), Values("Triangles", ""),
                    Values("InputTrianglesAdjacency", "")),
            Combine(Values("OutputPoints", ""), Values("OutputLineStrip", ""),
                    Values("OutputTriangleStrip", ""))));

using ValidateModeExecution =
    spvtest::ValidateBase<std::tuple<spv_result_t, std::string, std::string,
                                     std::string, spv_target_env>>;

TEST_P(ValidateModeExecution, ExecutionMode) {
  const spv_result_t expectation = std::get<0>(GetParam());
  const std::string error = std::get<1>(GetParam());
  const std::string model = std::get<2>(GetParam());
  const std::string mode = std::get<3>(GetParam());
  const spv_target_env env = std::get<4>(GetParam());

  std::ostringstream sstr;
  sstr << "OpCapability Shader\n";
  sstr << "OpCapability Geometry\n";
  sstr << "OpCapability Tessellation\n";
  sstr << "OpCapability TransformFeedback\n";
  if (!spvIsVulkanEnv(env)) {
    sstr << "OpCapability Kernel\n";
    if (env == SPV_ENV_UNIVERSAL_1_3) {
      sstr << "OpCapability SubgroupDispatch\n";
    }
  }
  sstr << "OpMemoryModel Logical GLSL450\n";
  sstr << "OpEntryPoint " << model << " %main \"main\"\n";
  if (mode.find("LocalSizeId") == 0 || mode.find("LocalSizeHintId") == 0 ||
      mode.find("SubgroupsPerWorkgroupId") == 0) {
    sstr << "OpExecutionModeId %main " << mode << "\n";
  } else {
    sstr << "OpExecutionMode %main " << mode << "\n";
  }
  if (model == "Geometry") {
    if (!(mode.find("InputPoints") == 0 || mode.find("InputLines") == 0 ||
          mode.find("InputLinesAdjacency") == 0 ||
          mode.find("Triangles") == 0 ||
          mode.find("InputTrianglesAdjacency") == 0)) {
      // Exactly one of the above modes is required for Geometry shaders.
      sstr << "OpExecutionMode %main InputPoints\n";
    }
    if (!(mode.find("OutputPoints") == 0 || mode.find("OutputLineStrip") == 0 ||
          mode.find("OutputTriangleStrip") == 0)) {
      // Exactly one of the above modes is required for Geometry shaders.
      sstr << "OpExecutionMode %main OutputPoints\n";
    }
  } else if (model == "Fragment") {
    if (!(mode.find("OriginUpperLeft") == 0 ||
          mode.find("OriginLowerLeft") == 0)) {
      // Exactly one of the above modes is required for Fragment shaders.
      sstr << "OpExecutionMode %main OriginUpperLeft\n";
    }
  }
  sstr << "%void = OpTypeVoid\n";
  sstr << "%void_fn = OpTypeFunction %void\n";
  sstr << "%int = OpTypeInt 32 0\n";
  sstr << "%int1 = OpConstant %int 1\n";
  sstr << "%main = OpFunction %void None %void_fn\n";
  sstr << "%entry = OpLabel\n";
  sstr << "OpReturn\n";
  sstr << "OpFunctionEnd\n";

  CompileSuccessfully(sstr.str(), env);
  EXPECT_THAT(expectation, ValidateInstructions(env));
  if (expectation != SPV_SUCCESS) {
    EXPECT_THAT(getDiagnosticString(), HasSubstr(error));
  }
}

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGeometryOnlyGoodSpv10, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""), Values("Geometry"),
            Values("Invocations 3", "InputPoints", "InputLines",
                   "InputLinesAdjacency", "InputTrianglesAdjacency",
                   "OutputPoints", "OutputLineStrip", "OutputTriangleStrip"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGeometryOnlyBadSpv10, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with the Geometry "
                   "execution model."),
            Values("Fragment", "TessellationEvaluation", "TessellationControl",
                   "GLCompute", "Vertex", "Kernel"),
            Values("Invocations 3", "InputPoints", "InputLines",
                   "InputLinesAdjacency", "InputTrianglesAdjacency",
                   "OutputPoints", "OutputLineStrip", "OutputTriangleStrip"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeTessellationOnlyGoodSpv10, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""),
            Values("TessellationControl", "TessellationEvaluation"),
            Values("SpacingEqual", "SpacingFractionalEven",
                   "SpacingFractionalOdd", "VertexOrderCw", "VertexOrderCcw",
                   "PointMode", "Quads", "Isolines"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeTessellationOnlyBadSpv10, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with a tessellation "
                   "execution model."),
            Values("Fragment", "Geometry", "GLCompute", "Vertex", "Kernel"),
            Values("SpacingEqual", "SpacingFractionalEven",
                   "SpacingFractionalOdd", "VertexOrderCw", "VertexOrderCcw",
                   "PointMode", "Quads", "Isolines"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(ValidateModeGeometryAndTessellationGoodSpv10,
                         ValidateModeExecution,
                         Combine(Values(SPV_SUCCESS), Values(""),
                                 Values("TessellationControl",
                                        "TessellationEvaluation", "Geometry"),
                                 Values("Triangles", "OutputVertices 3"),
                                 Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGeometryAndTessellationBadSpv10, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with a Geometry or "
                   "tessellation execution model."),
            Values("Fragment", "GLCompute", "Vertex", "Kernel"),
            Values("Triangles", "OutputVertices 3"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeFragmentOnlyGoodSpv10, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""), Values("Fragment"),
            Values("PixelCenterInteger", "OriginUpperLeft", "OriginLowerLeft",
                   "EarlyFragmentTests", "DepthReplacing", "DepthLess",
                   "DepthUnchanged"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeFragmentOnlyBadSpv10, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with the Fragment "
                   "execution model."),
            Values("Geometry", "TessellationControl", "TessellationEvaluation",
                   "GLCompute", "Vertex", "Kernel"),
            Values("PixelCenterInteger", "OriginUpperLeft", "OriginLowerLeft",
                   "EarlyFragmentTests", "DepthReplacing", "DepthGreater",
                   "DepthLess", "DepthUnchanged"),
            Values(SPV_ENV_UNIVERSAL_1_0)));

INSTANTIATE_TEST_SUITE_P(ValidateModeKernelOnlyGoodSpv13, ValidateModeExecution,
                         Combine(Values(SPV_SUCCESS), Values(""),
                                 Values("Kernel"),
                                 Values("LocalSizeHint 1 1 1", "VecTypeHint 4",
                                        "ContractionOff",
                                        "LocalSizeHintId %int1 %int1 %int1"),
                                 Values(SPV_ENV_UNIVERSAL_1_3)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeKernelOnlyBadSpv13, ValidateModeExecution,
    Combine(
        Values(SPV_ERROR_INVALID_DATA),
        Values(
            "Execution mode can only be used with the Kernel execution model."),
        Values("Geometry", "TessellationControl", "TessellationEvaluation",
               "GLCompute", "Vertex", "Fragment"),
        Values("LocalSizeHint 1 1 1", "VecTypeHint 4", "ContractionOff",
               "LocalSizeHintId %int1 %int1 %int1"),
        Values(SPV_ENV_UNIVERSAL_1_3)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGLComputeAndKernelGoodSpv13, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""), Values("Kernel", "GLCompute"),
            Values("LocalSize 1 1 1", "LocalSizeId %int1 %int1 %int1"),
            Values(SPV_ENV_UNIVERSAL_1_3)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeGLComputeAndKernelBadSpv13, ValidateModeExecution,
    Combine(Values(SPV_ERROR_INVALID_DATA),
            Values("Execution mode can only be used with a Kernel or GLCompute "
                   "execution model."),
            Values("Geometry", "TessellationControl", "TessellationEvaluation",
                   "Fragment", "Vertex"),
            Values("LocalSize 1 1 1", "LocalSizeId %int1 %int1 %int1"),
            Values(SPV_ENV_UNIVERSAL_1_3)));

INSTANTIATE_TEST_SUITE_P(
    ValidateModeAllGoodSpv13, ValidateModeExecution,
    Combine(Values(SPV_SUCCESS), Values(""),
            Values("Kernel", "GLCompute", "Geometry", "TessellationControl",
                   "TessellationEvaluation", "Fragment", "Vertex"),
            Values("Xfb", "Initializer", "Finalizer", "SubgroupSize 1",
                   "SubgroupsPerWorkgroup 1", "SubgroupsPerWorkgroupId %int1"),
            Values(SPV_ENV_UNIVERSAL_1_3)));

TEST_F(ValidateModeExecution, MeshNVLocalSize) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshNV %main "main"
OpExecutionMode %main LocalSize 1 1 1
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateModeExecution, TaskNVLocalSize) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint TaskNV %main "main"
OpExecutionMode %main LocalSize 1 1 1
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateModeExecution, MeshNVOutputPoints) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshNV %main "main"
OpExecutionMode %main OutputPoints
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateModeExecution, MeshNVOutputVertices) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshNV %main "main"
OpExecutionMode %main OutputVertices 42
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateModeExecution, MeshNVLocalSizeId) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint MeshNV %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, TaskNVLocalSizeId) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability MeshShadingNV
OpExtension "SPV_NV_mesh_shader"
OpMemoryModel Logical GLSL450
OpEntryPoint TaskNV %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, ExecModeSubgroupsPerWorkgroupIdBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability SubgroupDispatch
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionMode %main SubgroupsPerWorkgroupId %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpExecutionMode is only valid when the Mode operand "
                        "is an execution mode that takes no Extra Operands"));
}

TEST_F(ValidateModeExecution, ExecModeIdSubgroupsPerWorkgroupIdGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability SubgroupDispatch
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionModeId %main SubgroupsPerWorkgroupId %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, ExecModeIdSubgroupsPerWorkgroupIdNonConstantBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability SubgroupDispatch
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionModeId %main SubgroupsPerWorkgroupId %int_1
%int = OpTypeInt 32 0
%int_ptr = OpTypePointer Private %int
%int_1 = OpVariable %int_ptr Private
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For OpExecutionModeId all Extra Operand ids must be "
                        "constant instructions."));
}

TEST_F(ValidateModeExecution, ExecModeLocalSizeHintIdBad) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Kernel %main "main"
OpExecutionMode %main LocalSizeHintId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpExecutionMode is only valid when the Mode operand "
                        "is an execution mode that takes no Extra Operands"));
}

TEST_F(ValidateModeExecution, ExecModeIdLocalSizeHintIdGood) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Kernel %main "main"
OpExecutionModeId %main LocalSizeHintId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, ExecModeIdLocalSizeHintIdNonConstantBad) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionModeId %main LocalSizeHintId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_ptr = OpTypePointer Private %int
%int_1 = OpVariable %int_ptr Private
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For OpExecutionModeId all Extra Operand ids must be "
                        "constant instructions."));
}

TEST_F(ValidateModeExecution, ExecModeLocalSizeIdBad) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Kernel %main "main"
OpExecutionMode %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpExecutionMode is only valid when the Mode operand "
                        "is an execution mode that takes no Extra Operands"));
}

TEST_F(ValidateModeExecution, ExecModeIdLocalSizeIdGood) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Kernel %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions(env));
}

TEST_F(ValidateModeExecution, ExecModeIdLocalSizeIdNonConstantBad) {
  const std::string spirv = R"(
OpCapability Kernel
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%int = OpTypeInt 32 0
%int_ptr = OpTypePointer Private %int
%int_1 = OpVariable %int_ptr Private
)" + kVoidFunction;

  spv_target_env env = SPV_ENV_UNIVERSAL_1_3;
  CompileSuccessfully(spirv, env);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions(env));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For OpExecutionModeId all Extra Operand ids must be "
                        "constant instructions."));
}

TEST_F(ValidateMode, FragmentShaderInterlockVertexBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FragmentShaderPixelInterlockEXT
OpExtension "SPV_EXT_fragment_shader_interlock"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
OpExecutionMode %main PixelInterlockOrderedEXT
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Execution mode can only be used with the Fragment execution model"));
}

TEST_F(ValidateMode, FragmentShaderInterlockTooManyModesBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FragmentShaderPixelInterlockEXT
OpCapability FragmentShaderSampleInterlockEXT
OpExtension "SPV_EXT_fragment_shader_interlock"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main PixelInterlockOrderedEXT
OpExecutionMode %main SampleInterlockOrderedEXT
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points can specify at most "
                "one fragment shader interlock execution mode"));
}

TEST_F(ValidateMode, FragmentShaderInterlockNoModeBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FragmentShaderPixelInterlockEXT
OpExtension "SPV_EXT_fragment_shader_interlock"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%func = OpFunction %void None %void_fn
%entryf = OpLabel
OpBeginInvocationInterlockEXT
OpEndInvocationInterlockEXT
OpReturn
OpFunctionEnd
%main = OpFunction %void None %void_fn
%entry = OpLabel
%1 = OpFunctionCall %void %func
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpBeginInvocationInterlockEXT/OpEndInvocationInterlockEXT require a "
          "fragment shader interlock execution mode"));
}

TEST_F(ValidateMode, FragmentShaderInterlockGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability FragmentShaderPixelInterlockEXT
OpExtension "SPV_EXT_fragment_shader_interlock"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main PixelInterlockOrderedEXT
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%func = OpFunction %void None %void_fn
%entryf = OpLabel
OpBeginInvocationInterlockEXT
OpEndInvocationInterlockEXT
OpReturn
OpFunctionEnd
%main = OpFunction %void None %void_fn
%entry = OpLabel
%1 = OpFunctionCall %void %func
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}


TEST_F(ValidateMode, FragmentShaderStencilRefFrontTooManyModesBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability StencilExportEXT
OpExtension "SPV_AMD_shader_early_and_late_fragment_tests"
OpExtension "SPV_EXT_shader_stencil_export"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main EarlyAndLateFragmentTestsAMD
OpExecutionMode %main StencilRefLessFrontAMD
OpExecutionMode %main StencilRefGreaterFrontAMD
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points can specify at most "
                "one of StencilRefUnchangedFrontAMD, "
                "StencilRefLessFrontAMD or StencilRefGreaterFrontAMD "
                "execution modes."));
}

TEST_F(ValidateMode, FragmentShaderStencilRefBackTooManyModesBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability StencilExportEXT
OpExtension "SPV_AMD_shader_early_and_late_fragment_tests"
OpExtension "SPV_EXT_shader_stencil_export"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main EarlyAndLateFragmentTestsAMD
OpExecutionMode %main StencilRefLessBackAMD
OpExecutionMode %main StencilRefGreaterBackAMD
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Fragment execution model entry points can specify at most "
                "one of StencilRefUnchangedBackAMD, "
                "StencilRefLessBackAMD or StencilRefGreaterBackAMD "
                "execution modes."));
}

TEST_F(ValidateMode, FragmentShaderStencilRefFrontGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability StencilExportEXT
OpExtension "SPV_AMD_shader_early_and_late_fragment_tests"
OpExtension "SPV_EXT_shader_stencil_export"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main EarlyAndLateFragmentTestsAMD
OpExecutionMode %main StencilRefLessFrontAMD
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, FragmentShaderStencilRefBackGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability StencilExportEXT
OpExtension "SPV_AMD_shader_early_and_late_fragment_tests"
OpExtension "SPV_EXT_shader_stencil_export"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
OpExecutionMode %main EarlyAndLateFragmentTestsAMD
OpExecutionMode %main StencilRefLessBackAMD
)" + kVoidFunction;

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, FragmentShaderDemoteVertexBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability DemoteToHelperInvocationEXT
OpExtension "SPV_EXT_demote_to_helper_invocation"
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
%bool = OpTypeBool
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpDemoteToHelperInvocationEXT
%1 = OpIsHelperInvocationEXT %bool
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpDemoteToHelperInvocationEXT requires Fragment execution model"));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpIsHelperInvocationEXT requires Fragment execution model"));
}

TEST_F(ValidateMode, FragmentShaderDemoteGood) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability DemoteToHelperInvocationEXT
OpExtension "SPV_EXT_demote_to_helper_invocation"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%bool = OpTypeBool
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpDemoteToHelperInvocationEXT
%1 = OpIsHelperInvocationEXT %bool
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateMode, FragmentShaderDemoteBadType) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability DemoteToHelperInvocationEXT
OpExtension "SPV_EXT_demote_to_helper_invocation"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%u32 = OpTypeInt 32 0
%void = OpTypeVoid
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpDemoteToHelperInvocationEXT
%1 = OpIsHelperInvocationEXT %u32
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Expected bool scalar type as Result Type"));
}

TEST_F(ValidateMode, LocalSizeIdVulkan1p3DoesNotRequireOption) {
  const std::string spirv = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint GLCompute %main "main"
OpExecutionModeId %main LocalSizeId %int_1 %int_1 %int_1
%void = OpTypeVoid
%int = OpTypeInt 32 0
%int_1 = OpConstant %int 1
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_VULKAN_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_VULKAN_1_3));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
