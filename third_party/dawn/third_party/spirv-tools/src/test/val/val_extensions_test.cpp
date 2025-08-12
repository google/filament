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

// Tests for OpExtension validator rules.

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "source/extensions.h"
#include "source/spirv_target_env.h"
#include "test/unit_spirv.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateKnownExtensions = spvtest::ValidateBase<std::string>;
using ValidateUnknownExtensions = spvtest::ValidateBase<std::string>;
using ValidateExtensionCapabilities = spvtest::ValidateBase<bool>;

// Returns expected error string if |extension| is not recognized.
std::string GetErrorString(const std::string& extension) {
  return "Found unrecognized extension " + extension;
}

INSTANTIATE_TEST_SUITE_P(
    ExpectSuccess, ValidateKnownExtensions,
    Values(
        // Match the order as published on the SPIR-V Registry.
        "SPV_AMD_shader_explicit_vertex_parameter",
        "SPV_AMD_shader_trinary_minmax", "SPV_AMD_gcn_shader",
        "SPV_KHR_shader_ballot", "SPV_AMD_shader_ballot",
        "SPV_AMD_gpu_shader_half_float", "SPV_KHR_shader_draw_parameters",
        "SPV_KHR_subgroup_vote", "SPV_KHR_16bit_storage",
        "SPV_KHR_device_group", "SPV_KHR_multiview",
        "SPV_NVX_multiview_per_view_attributes", "SPV_NV_viewport_array2",
        "SPV_NV_stereo_view_rendering", "SPV_NV_sample_mask_override_coverage",
        "SPV_NV_geometry_shader_passthrough", "SPV_AMD_texture_gather_bias_lod",
        "SPV_KHR_storage_buffer_storage_class", "SPV_KHR_variable_pointers",
        "SPV_AMD_gpu_shader_int16", "SPV_KHR_post_depth_coverage",
        "SPV_KHR_shader_atomic_counter_ops", "SPV_EXT_shader_stencil_export",
        "SPV_EXT_shader_viewport_index_layer",
        "SPV_AMD_shader_image_load_store_lod", "SPV_AMD_shader_fragment_mask",
        "SPV_GOOGLE_decorate_string", "SPV_GOOGLE_hlsl_functionality1",
        "SPV_NV_shader_subgroup_partitioned", "SPV_EXT_descriptor_indexing",
        "SPV_KHR_terminate_invocation", "SPV_KHR_relaxed_extended_instruction",
        "SPV_EXT_float8"));

INSTANTIATE_TEST_SUITE_P(FailSilently, ValidateUnknownExtensions,
                         Values("ERROR_unknown_extension", "SPV_KHR_",
                                "SPV_KHR_shader_ballot_ERROR"));

TEST_P(ValidateKnownExtensions, ExpectSuccess) {
  const std::string extension = GetParam();
  const std::string str =
      "OpCapability Shader\nOpCapability Linkage\nOpExtension \"" + extension +
      "\"\nOpMemoryModel Logical GLSL450";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), Not(HasSubstr(GetErrorString(extension))));
}

TEST_P(ValidateUnknownExtensions, FailSilently) {
  const std::string extension = GetParam();
  const std::string str =
      "OpCapability Shader\nOpCapability Linkage\nOpExtension \"" + extension +
      "\"\nOpMemoryModel Logical GLSL450";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr(GetErrorString(extension)));
}

TEST_F(ValidateUnknownExtensions, HitMaxNumOfWarnings) {
  const std::string str =
      std::string("OpCapability Shader\n") + "OpCapability Linkage\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpExtension \"bad_ext\"\n" + "OpExtension \"bad_ext\"\n" +
      "OpMemoryModel Logical GLSL450";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Other warnings have been suppressed."));
}

TEST_F(ValidateExtensionCapabilities, DeclCapabilitySuccess) {
  const std::string str =
      "OpCapability Shader\nOpCapability Linkage\nOpCapability DeviceGroup\n"
      "OpExtension \"SPV_KHR_device_group\""
      "\nOpMemoryModel Logical GLSL450";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateExtensionCapabilities, DeclCapabilityFailure) {
  const std::string str =
      "OpCapability Shader\nOpCapability Linkage\nOpCapability DeviceGroup\n"
      "\nOpMemoryModel Logical GLSL450";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_MISSING_EXTENSION, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("1st operand of Capability"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("requires one of these extensions"));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("SPV_KHR_device_group"));
}

TEST_F(ValidateExtensionCapabilities,
       DeclCapabilityFailureBlockMatchWIndowSAD) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability TextureBlockMatch2QCOM
               OpExtension "SPV_QCOM_image_processing2"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %v_texcoord %fragColor %target_samp %ref_samp
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_QCOM_image_processing"
               OpSourceExtension "GL_QCOM_image_processing2"
               OpName %main "main"
               OpName %tgt_coords "tgt_coords"
               OpName %v_texcoord "v_texcoord"
               OpName %ref_coords "ref_coords"
               OpName %blockSize "blockSize"
               OpName %fragColor "fragColor"
               OpName %target_samp "target_samp"
               OpName %ref_samp "ref_samp"
               OpDecorate %v_texcoord Location 0
               OpDecorate %fragColor Location 0
               OpDecorate %target_samp DescriptorSet 0
               OpDecorate %target_samp Binding 4
               OpDecorate %ref_samp DescriptorSet 0
               OpDecorate %ref_samp Binding 5
               OpDecorate %target_samp BlockMatchTextureQCOM
               OpDecorate %target_samp BlockMatchSamplerQCOM
               OpDecorate %ref_samp BlockMatchTextureQCOM
               OpDecorate %ref_samp BlockMatchSamplerQCOM
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
 %v_texcoord = OpVariable %_ptr_Input_v4float Input
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %uint_4 = OpConstant %uint 4
         %39 = OpConstantComposite %v2uint %uint_4 %uint_4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %fragColor = OpVariable %_ptr_Output_v4float Output
         %42 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %43 = OpTypeSampledImage %42
%_ptr_UniformConstant_43 = OpTypePointer UniformConstant %43
%target_samp = OpVariable %_ptr_UniformConstant_43 UniformConstant
   %ref_samp = OpVariable %_ptr_UniformConstant_43 UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
 %tgt_coords = OpVariable %_ptr_Function_v2uint Function
 %ref_coords = OpVariable %_ptr_Function_v2uint Function
  %blockSize = OpVariable %_ptr_Function_v2uint Function
         %16 = OpAccessChain %_ptr_Input_float %v_texcoord %uint_0
         %17 = OpLoad %float %16
         %18 = OpConvertFToU %uint %17
         %20 = OpAccessChain %_ptr_Function_uint %tgt_coords %uint_0
               OpStore %20 %18
         %22 = OpAccessChain %_ptr_Input_float %v_texcoord %uint_1
         %23 = OpLoad %float %22
         %24 = OpConvertFToU %uint %23
         %25 = OpAccessChain %_ptr_Function_uint %tgt_coords %uint_0
               OpStore %25 %24
         %28 = OpAccessChain %_ptr_Input_float %v_texcoord %uint_2
         %29 = OpLoad %float %28
         %30 = OpConvertFToU %uint %29
         %31 = OpAccessChain %_ptr_Function_uint %ref_coords %uint_0
               OpStore %31 %30
         %33 = OpAccessChain %_ptr_Input_float %v_texcoord %uint_3
         %34 = OpLoad %float %33
         %35 = OpConvertFToU %uint %34
         %36 = OpAccessChain %_ptr_Function_uint %ref_coords %uint_1
               OpStore %36 %35
               OpStore %blockSize %39
         %46 = OpLoad %43 %target_samp
         %47 = OpLoad %v2uint %tgt_coords
         %49 = OpLoad %43 %ref_samp
         %50 = OpLoad %v2uint %ref_coords
         %51 = OpLoad %v2uint %blockSize
         %52 = OpImageBlockMatchWindowSADQCOM %v4float %46 %47 %49 %50 %51
               OpStore %fragColor %52
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_MISSING_EXTENSION, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("2nd operand of Decorate"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("requires one of these extensions"));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("SPV_QCOM_image_processing"));
}

TEST_F(ValidateExtensionCapabilities,
       DeclCapabilityFailureBlockMatchWIndowSSD) {
  const std::string str = R"(
               OpCapability Shader
               OpCapability TextureBlockMatch2QCOM
               OpExtension "SPV_QCOM_image_processing2"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %v_texcoord %fragColor %tex2D_src1 %samp %tex2D_src2
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpSourceExtension "GL_QCOM_image_processing"
               OpSourceExtension "GL_QCOM_image_processing2"
               OpName %main "main"
               OpName %tgt_coords "tgt_coords"
               OpName %v_texcoord "v_texcoord"
               OpName %ref_coords "ref_coords"
               OpName %blockSize "blockSize"
               OpName %fragColor "fragColor"
               OpName %tex2D_src1 "tex2D_src1"
               OpName %samp "samp"
               OpName %tex2D_src2 "tex2D_src2"
               OpDecorate %v_texcoord Location 0
               OpDecorate %fragColor Location 0
               OpDecorate %tex2D_src1 DescriptorSet 0
               OpDecorate %tex2D_src1 Binding 1
               OpDecorate %samp DescriptorSet 0
               OpDecorate %samp Binding 3
               OpDecorate %tex2D_src2 DescriptorSet 0
               OpDecorate %tex2D_src2 Binding 2
               OpDecorate %tex2D_src1 BlockMatchTextureQCOM
               OpDecorate %samp BlockMatchSamplerQCOM
               OpDecorate %tex2D_src2 BlockMatchTextureQCOM
               OpDecorate %samp BlockMatchSamplerQCOM
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
 %v_texcoord = OpVariable %_ptr_Input_v4float Input
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_1 = OpConstant %uint 1
     %uint_2 = OpConstant %uint 2
     %uint_3 = OpConstant %uint 3
     %uint_4 = OpConstant %uint 4
         %39 = OpConstantComposite %v2uint %uint_4 %uint_4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %fragColor = OpVariable %_ptr_Output_v4float Output
         %42 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_42 = OpTypePointer UniformConstant %42
 %tex2D_src1 = OpVariable %_ptr_UniformConstant_42 UniformConstant
         %46 = OpTypeSampler
%_ptr_UniformConstant_46 = OpTypePointer UniformConstant %46
       %samp = OpVariable %_ptr_UniformConstant_46 UniformConstant
         %50 = OpTypeSampledImage %42
 %tex2D_src2 = OpVariable %_ptr_UniformConstant_42 UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
 %tgt_coords = OpVariable %_ptr_Function_v2uint Function
 %ref_coords = OpVariable %_ptr_Function_v2uint Function
  %blockSize = OpVariable %_ptr_Function_v2uint Function
         %16 = OpAccessChain %_ptr_Input_float %v_texcoord %uint_0
         %17 = OpLoad %float %16
         %18 = OpConvertFToU %uint %17
         %20 = OpAccessChain %_ptr_Function_uint %tgt_coords %uint_0
               OpStore %20 %18
         %22 = OpAccessChain %_ptr_Input_float %v_texcoord %uint_1
         %23 = OpLoad %float %22
         %24 = OpConvertFToU %uint %23
         %25 = OpAccessChain %_ptr_Function_uint %tgt_coords %uint_0
               OpStore %25 %24
         %28 = OpAccessChain %_ptr_Input_float %v_texcoord %uint_2
         %29 = OpLoad %float %28
         %30 = OpConvertFToU %uint %29
         %31 = OpAccessChain %_ptr_Function_uint %ref_coords %uint_0
               OpStore %31 %30
         %33 = OpAccessChain %_ptr_Input_float %v_texcoord %uint_3
         %34 = OpLoad %float %33
         %35 = OpConvertFToU %uint %34
         %36 = OpAccessChain %_ptr_Function_uint %ref_coords %uint_1
               OpStore %36 %35
               OpStore %blockSize %39
         %45 = OpLoad %42 %tex2D_src1
         %49 = OpLoad %46 %samp
         %51 = OpSampledImage %50 %45 %49
         %52 = OpLoad %v2uint %tgt_coords
         %54 = OpLoad %42 %tex2D_src2
         %55 = OpLoad %46 %samp
         %56 = OpSampledImage %50 %54 %55
         %57 = OpLoad %v2uint %ref_coords
         %58 = OpLoad %v2uint %blockSize
         %59 = OpImageBlockMatchWindowSSDQCOM %v4float %51 %52 %56 %57 %58
               OpStore %fragColor %59
               OpReturn
               OpFunctionEnd
)";
  CompileSuccessfully(str.c_str());
  ASSERT_EQ(SPV_ERROR_MISSING_EXTENSION, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), HasSubstr("2nd operand of Decorate"));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("requires one of these extensions"));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("SPV_QCOM_image_processing"));
}

using ValidateAMDShaderBallotCapabilities = spvtest::ValidateBase<std::string>;

// Returns a vector of strings for the prefix of a SPIR-V assembly shader
// that can use the group instructions introduced by SPV_AMD_shader_ballot.
std::vector<std::string> ShaderPartsForAMDShaderBallot() {
  return std::vector<std::string>{R"(
  OpCapability Shader
  OpCapability Linkage
  )",
                                  R"(
  OpMemoryModel Logical GLSL450
  %float = OpTypeFloat 32
  %uint = OpTypeInt 32 0
  %int = OpTypeInt 32 1
  %scope = OpConstant %uint 3
  %uint_const = OpConstant %uint 42
  %int_const = OpConstant %uint 45
  %float_const = OpConstant %float 3.5

  %void = OpTypeVoid
  %fn_ty = OpTypeFunction %void
  %fn = OpFunction %void None %fn_ty
  %entry = OpLabel
  )"};
}

// Returns a list of SPIR-V assembly strings, where each uses only types
// and IDs that can fit with a shader made from parts from the result
// of ShaderPartsForAMDShaderBallot.
std::vector<std::string> AMDShaderBallotGroupInstructions() {
  return std::vector<std::string>{
      "%iadd_reduce = OpGroupIAddNonUniformAMD %uint %scope Reduce %uint_const",
      "%iadd_iscan = OpGroupIAddNonUniformAMD %uint %scope InclusiveScan "
      "%uint_const",
      "%iadd_escan = OpGroupIAddNonUniformAMD %uint %scope ExclusiveScan "
      "%uint_const",

      "%fadd_reduce = OpGroupFAddNonUniformAMD %float %scope Reduce "
      "%float_const",
      "%fadd_iscan = OpGroupFAddNonUniformAMD %float %scope InclusiveScan "
      "%float_const",
      "%fadd_escan = OpGroupFAddNonUniformAMD %float %scope ExclusiveScan "
      "%float_const",

      "%fmin_reduce = OpGroupFMinNonUniformAMD %float %scope Reduce "
      "%float_const",
      "%fmin_iscan = OpGroupFMinNonUniformAMD %float %scope InclusiveScan "
      "%float_const",
      "%fmin_escan = OpGroupFMinNonUniformAMD %float %scope ExclusiveScan "
      "%float_const",

      "%umin_reduce = OpGroupUMinNonUniformAMD %uint %scope Reduce %uint_const",
      "%umin_iscan = OpGroupUMinNonUniformAMD %uint %scope InclusiveScan "
      "%uint_const",
      "%umin_escan = OpGroupUMinNonUniformAMD %uint %scope ExclusiveScan "
      "%uint_const",

      "%smin_reduce = OpGroupUMinNonUniformAMD %int %scope Reduce %int_const",
      "%smin_iscan = OpGroupUMinNonUniformAMD %int %scope InclusiveScan "
      "%int_const",
      "%smin_escan = OpGroupUMinNonUniformAMD %int %scope ExclusiveScan "
      "%int_const",

      "%fmax_reduce = OpGroupFMaxNonUniformAMD %float %scope Reduce "
      "%float_const",
      "%fmax_iscan = OpGroupFMaxNonUniformAMD %float %scope InclusiveScan "
      "%float_const",
      "%fmax_escan = OpGroupFMaxNonUniformAMD %float %scope ExclusiveScan "
      "%float_const",

      "%umax_reduce = OpGroupUMaxNonUniformAMD %uint %scope Reduce %uint_const",
      "%umax_iscan = OpGroupUMaxNonUniformAMD %uint %scope InclusiveScan "
      "%uint_const",
      "%umax_escan = OpGroupUMaxNonUniformAMD %uint %scope ExclusiveScan "
      "%uint_const",

      "%smax_reduce = OpGroupUMaxNonUniformAMD %int %scope Reduce %int_const",
      "%smax_iscan = OpGroupUMaxNonUniformAMD %int %scope InclusiveScan "
      "%int_const",
      "%smax_escan = OpGroupUMaxNonUniformAMD %int %scope ExclusiveScan "
      "%int_const"};
}

TEST_P(ValidateAMDShaderBallotCapabilities, ExpectSuccess) {
  // Succeed because the module specifies the SPV_AMD_shader_ballot extension.
  auto parts = ShaderPartsForAMDShaderBallot();

  const std::string assembly =
      parts[0] + "OpExtension \"SPV_AMD_shader_ballot\"\n" + parts[1] +
      GetParam() + "\nOpReturn OpFunctionEnd";

  CompileSuccessfully(assembly.c_str());
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions()) << getDiagnosticString();
}

INSTANTIATE_TEST_SUITE_P(ExpectSuccess, ValidateAMDShaderBallotCapabilities,
                         ValuesIn(AMDShaderBallotGroupInstructions()));

TEST_P(ValidateAMDShaderBallotCapabilities, ExpectFailure) {
  // Fail because the module does not specify the SPV_AMD_shader_ballot
  // extension.
  auto parts = ShaderPartsForAMDShaderBallot();

  const std::string assembly =
      parts[0] + parts[1] + GetParam() + "\nOpReturn OpFunctionEnd";

  CompileSuccessfully(assembly.c_str());
  EXPECT_EQ(SPV_ERROR_INVALID_CAPABILITY, ValidateInstructions());

  // Make sure we get an appropriate error message.
  // Find just the opcode name, skipping over the "Op" part.
  auto prefix_with_opcode = GetParam().substr(GetParam().find("Group"));
  auto opcode = prefix_with_opcode.substr(0, prefix_with_opcode.find(' '));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(std::string("Opcode " + opcode +
                            " requires one of these capabilities: Groups")));
}

INSTANTIATE_TEST_SUITE_P(ExpectFailure, ValidateAMDShaderBallotCapabilities,
                         ValuesIn(AMDShaderBallotGroupInstructions()));

struct ExtIntoCoreCase {
  const char* ext;
  const char* cap;
  const char* builtin;
  spv_target_env env;
  bool success;
};

using ValidateExtIntoCore = spvtest::ValidateBase<ExtIntoCoreCase>;

// Make sure that we don't panic about missing extensions for using
// functionalities that introduced in extensions but became core SPIR-V later.

TEST_P(ValidateExtIntoCore, DoNotAskForExtensionInLaterVersion) {
  const std::string code = std::string(R"(
               OpCapability Shader
               OpCapability )") +
                           GetParam().cap + R"(
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %builtin
               OpDecorate %builtin BuiltIn )" + GetParam().builtin + R"(
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
    %builtin = OpVariable %_ptr_Input_int Input
       %main = OpFunction %void None %3
          %5 = OpLabel
         %18 = OpLoad %int %builtin
               OpReturn
               OpFunctionEnd)";

  CompileSuccessfully(code.c_str(), GetParam().env);
  if (GetParam().success) {
    ASSERT_EQ(SPV_SUCCESS, ValidateInstructions(GetParam().env))
        << getDiagnosticString();
  } else {
    ASSERT_NE(SPV_SUCCESS, ValidateInstructions(GetParam().env))
        << " in " << spvTargetEnvDescription(GetParam().env) << ":\n"
        << code;
    const std::string message = getDiagnosticString();
    if (spvIsVulkanEnv(GetParam().env)) {
      EXPECT_THAT(message, HasSubstr(std::string(GetParam().cap) +
                                     " is not allowed by Vulkan"));
      EXPECT_THAT(message, HasSubstr(std::string("or requires extension")));
    } else {
      EXPECT_THAT(message,
                  HasSubstr(std::string("requires one of these extensions: ") +
                            GetParam().ext));
    }
  }
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    KHR_extensions, ValidateExtIntoCore,
    ValuesIn(std::vector<ExtIntoCoreCase>{
        // SPV_KHR_shader_draw_parameters became core SPIR-V 1.3
        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "BaseVertex", SPV_ENV_UNIVERSAL_1_3, true},
        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "BaseVertex", SPV_ENV_UNIVERSAL_1_2, false},
        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "BaseVertex", SPV_ENV_UNIVERSAL_1_1, false},
        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "BaseVertex", SPV_ENV_UNIVERSAL_1_0, false},
        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "BaseVertex", SPV_ENV_VULKAN_1_1, true},
        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "BaseVertex", SPV_ENV_VULKAN_1_0, false},

        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "BaseInstance", SPV_ENV_UNIVERSAL_1_3, true},
        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "BaseInstance", SPV_ENV_VULKAN_1_0, false},

        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "DrawIndex", SPV_ENV_UNIVERSAL_1_3, true},
        {"SPV_KHR_shader_draw_parameters", "DrawParameters", "DrawIndex", SPV_ENV_UNIVERSAL_1_1, false},

        // SPV_KHR_multiview became core SPIR-V 1.3
        {"SPV_KHR_multiview", "MultiView", "ViewIndex", SPV_ENV_UNIVERSAL_1_3, true},
        {"SPV_KHR_multiview", "MultiView", "ViewIndex", SPV_ENV_UNIVERSAL_1_2, false},
        {"SPV_KHR_multiview", "MultiView", "ViewIndex", SPV_ENV_UNIVERSAL_1_1, false},
        {"SPV_KHR_multiview", "MultiView", "ViewIndex", SPV_ENV_UNIVERSAL_1_0, false},
        {"SPV_KHR_multiview", "MultiView", "ViewIndex", SPV_ENV_VULKAN_1_1, true},
        {"SPV_KHR_multiview", "MultiView", "ViewIndex", SPV_ENV_VULKAN_1_0, false},

        // SPV_KHR_device_group became core SPIR-V 1.3
        {"SPV_KHR_device_group", "DeviceGroup", "DeviceIndex", SPV_ENV_UNIVERSAL_1_3, true},
        {"SPV_KHR_device_group", "DeviceGroup", "DeviceIndex", SPV_ENV_UNIVERSAL_1_2, false},
        {"SPV_KHR_device_group", "DeviceGroup", "DeviceIndex", SPV_ENV_UNIVERSAL_1_1, false},
        {"SPV_KHR_device_group", "DeviceGroup", "DeviceIndex", SPV_ENV_UNIVERSAL_1_0, false},
        {"SPV_KHR_device_group", "DeviceGroup", "DeviceIndex", SPV_ENV_VULKAN_1_1, true},
        {"SPV_KHR_device_group", "DeviceGroup", "DeviceIndex", SPV_ENV_VULKAN_1_0, false},
    }));
// clang-format on

using ValidateRelaxedExtendedInstructionExt = spvtest::ValidateBase<bool>;

TEST_F(ValidateRelaxedExtendedInstructionExt, RequiresExtension) {
  const std::string str = R"(
             OpCapability Shader
             OpExtension "SPV_KHR_non_semantic_info"
        %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
             OpMemoryModel Logical GLSL450
             OpEntryPoint GLCompute %2 "main"
             OpExecutionMode %2 LocalSize 1 1 1
        %3 = OpString "sample"
     %void = OpTypeVoid
     %uint = OpTypeInt 32 0
   %uint_0 = OpConstant %uint 0
        %7 = OpTypeFunction %void
        %8 = OpExtInst %void %1 DebugSource %3 %3
        %9 = OpExtInst %void %1 DebugCompilationUnit %uint_0 %uint_0 %8 %uint_0
       %10 = OpExtInstWithForwardRefsKHR %void %1 DebugTypeFunction %uint_0 %11
       %12 = OpExtInstWithForwardRefsKHR %void %1 DebugFunction %3 %10 %8 %uint_0 %uint_0 %11 %3 %uint_0 %uint_0
       %11 = OpExtInst %void %1 DebugTypeComposite %3 %uint_0 %8 %uint_0 %uint_0 %9 %3 %uint_0 %uint_0 %12
        %2 = OpFunction %void None %7
       %13 = OpLabel
             OpReturn
             OpFunctionEnd
)";

  CompileSuccessfully(str.c_str());
  EXPECT_NE(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "ExtInstWithForwardRefsKHR requires one of the following extensions:"
          " SPV_KHR_relaxed_extended_instruction \n"
          "  %10 = OpExtInstWithForwardRefsKHR %void %1 DebugTypeFunction "
          "%uint_0 "
          "%11\n"));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
