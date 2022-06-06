// Copyright (c) 2019 Google LLC.
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

// Validation tests for non-semantic instructions

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_code_generator.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

struct TestResult {
  TestResult(spv_result_t in_validation_result = SPV_SUCCESS,
             const char* in_error_str = nullptr,
             const char* in_error_str2 = nullptr)
      : validation_result(in_validation_result),
        error_str(in_error_str),
        error_str2(in_error_str2) {}
  spv_result_t validation_result;
  const char* error_str;
  const char* error_str2;
};

using ::testing::Combine;
using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateNonSemanticGenerated = spvtest::ValidateBase<
    std::tuple<bool, bool, const char*, const char*, TestResult>>;
using ValidateNonSemanticString = spvtest::ValidateBase<bool>;

CodeGenerator GetNonSemanticCodeGenerator(const bool declare_ext,
                                          const bool declare_extinst,
                                          const char* const global_extinsts,
                                          const char* const function_extinsts) {
  CodeGenerator generator = CodeGenerator::GetDefaultShaderCodeGenerator();

  if (declare_ext) {
    generator.extensions_ += "OpExtension \"SPV_KHR_non_semantic_info\"\n";
  }
  if (declare_extinst) {
    generator.extensions_ +=
        "%extinst = OpExtInstImport \"NonSemantic.Testing.Set\"\n";
  }

  generator.after_types_ = global_extinsts;

  generator.before_types_ = "%decorate_group = OpDecorationGroup";

  EntryPoint entry_point;
  entry_point.name = "main";
  entry_point.execution_model = "Vertex";

  entry_point.body = R"(
)";
  entry_point.body += function_extinsts;
  generator.entry_points_.push_back(std::move(entry_point));

  return generator;
}

TEST_P(ValidateNonSemanticGenerated, InTest) {
  const bool declare_ext = std::get<0>(GetParam());
  const bool declare_extinst = std::get<1>(GetParam());
  const char* const global_extinsts = std::get<2>(GetParam());
  const char* const function_extinsts = std::get<3>(GetParam());
  const TestResult& test_result = std::get<4>(GetParam());

  CodeGenerator generator = GetNonSemanticCodeGenerator(
      declare_ext, declare_extinst, global_extinsts, function_extinsts);

  CompileSuccessfully(generator.Build(), SPV_ENV_VULKAN_1_0);
  ASSERT_EQ(test_result.validation_result,
            ValidateInstructions(SPV_ENV_VULKAN_1_0));
  if (test_result.error_str) {
    EXPECT_THAT(getDiagnosticString(),
                testing::ContainsRegex(test_result.error_str));
  }
  if (test_result.error_str2) {
    EXPECT_THAT(getDiagnosticString(),
                testing::ContainsRegex(test_result.error_str2));
  }
}

INSTANTIATE_TEST_SUITE_P(OnlyOpExtension, ValidateNonSemanticGenerated,
                         Combine(Values(true), Values(false), Values(""),
                                 Values(""), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    MissingOpExtensionPre1p6, ValidateNonSemanticGenerated,
    Combine(Values(false), Values(true), Values(""), Values(""),
            Values(TestResult(
                SPV_ERROR_INVALID_DATA,
                "NonSemantic extended instruction sets cannot be declared "
                "without SPV_KHR_non_semantic_info."))));

INSTANTIATE_TEST_SUITE_P(NoExtInst, ValidateNonSemanticGenerated,
                         Combine(Values(true), Values(true), Values(""),
                                 Values(""), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SimpleGlobalExtInst, ValidateNonSemanticGenerated,
    Combine(Values(true), Values(true),
            Values("%result = OpExtInst %void %extinst 123 %i32"), Values(""),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    ComplexGlobalExtInst, ValidateNonSemanticGenerated,
    Combine(Values(true), Values(true),
            Values("%result = OpExtInst %void %extinst  123 %i32 %u32_2 "
                   "%f32vec4_1234 %u32_0"),
            Values(""), Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    SimpleFunctionLevelExtInst, ValidateNonSemanticGenerated,
    Combine(Values(true), Values(true), Values(""),
            Values("%result = OpExtInst %void %extinst 123 %i32"),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    FunctionTypeReference, ValidateNonSemanticGenerated,
    Combine(Values(true), Values(true),
            Values("%result = OpExtInst %void %extinst 123 %func"), Values(""),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    EntryPointReference, ValidateNonSemanticGenerated,
    Combine(Values(true), Values(true), Values(""),
            Values("%result = OpExtInst %void %extinst 123 %main"),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    DecorationGroupReference, ValidateNonSemanticGenerated,
    Combine(Values(true), Values(true), Values(""),
            Values("%result = OpExtInst %void %extinst 123 %decorate_group"),
            Values(TestResult())));

INSTANTIATE_TEST_SUITE_P(
    UnknownIDReference, ValidateNonSemanticGenerated,
    Combine(Values(true), Values(true),
            Values("%result = OpExtInst %void %extinst 123 %undefined_id"),
            Values(""),
            Values(TestResult(SPV_ERROR_INVALID_ID,
                              "ID .* has not been defined"))));

INSTANTIATE_TEST_SUITE_P(
    NonSemanticUseInSemantic, ValidateNonSemanticGenerated,
    Combine(Values(true), Values(true),
            Values("%result = OpExtInst %f32 %extinst 123 %i32\n"
                   "%invalid = OpConstantComposite %f32vec2 %f32_0 %result"),
            Values(""),
            Values(TestResult(SPV_ERROR_INVALID_ID,
                              "in semantic instruction cannot be a "
                              "non-semantic instruction"))));

TEST_F(ValidateNonSemanticString, InvalidSectionOpExtInst) {
  const std::string spirv = R"(
OpCapability Shader
OpExtension "SPV_KHR_non_semantic_info"
%extinst = OpExtInstImport "NonSemantic.Testing.Set"
%test = OpExtInst %void %extinst 4 %void
OpMemoryModel Logical GLSL450
OpEntryPoint Vertex %main "main"
)";

  CompileSuccessfully(spirv);
  EXPECT_THAT(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_VULKAN_1_0));

  // there's no specific error for using an OpExtInst too early, it requires a
  // type so by definition any use of a type in it will be an undefined ID
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("ID 2[%2] has not been defined"));
}

TEST_F(ValidateNonSemanticString, MissingOpExtensionPost1p6) {
  const std::string spirv = R"(
OpCapability Shader
%extinst = OpExtInstImport "NonSemantic.Testing.Set"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main"
OpExecutionMode %main OriginUpperLeft
%void = OpTypeVoid
%test = OpExtInst %void %extinst 3
%void_fn = OpTypeFunction %void
%main = OpFunction %void None %void_fn
%entry = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_6);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_6));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
