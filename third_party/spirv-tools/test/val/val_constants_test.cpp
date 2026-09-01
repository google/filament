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

// Test validation of constants.
//
// This file contains newer tests.  Older tests may be in other files such as
// val_id_test.cpp.

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "test/unit_spirv.h"
#include "test/val/val_code_generator.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using ::testing::Combine;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Values;
using ::testing::ValuesIn;

using ValidateConstant = spvtest::ValidateBase<bool>;

#define kBasicTypes                                        \
  "%bool = OpTypeBool "                                    \
  "%uint = OpTypeInt 32 0 "                                \
  "%uint64 = OpTypeInt 64 0 "                              \
  "%uint2 = OpTypeVector %uint 2 "                         \
  "%float = OpTypeFloat 32 "                               \
  "%float64 = OpTypeFloat 64 "                             \
  "%_ptr_uint = OpTypePointer Workgroup %uint "            \
  "%uint_0 = OpConstantNull %uint "                        \
  "%uint64_0 = OpConstantNull %uint64 "                    \
  "%uint2_0 = OpConstantComposite %uint2 %uint_0 %uint_0 " \
  "%float_0 = OpConstantNull %float "                      \
  "%float64_0 = OpConstantNull %float64 "                  \
  "%false = OpConstantFalse %bool "                        \
  "%true = OpConstantTrue %bool "                          \
  "%null = OpConstantNull %_ptr_uint "

#define kShaderPreamble                         \
  "OpCapability Shader\n"                       \
  "OpCapability Linkage\n"                      \
  "OpCapability Int64\n"                        \
  "OpCapability Float64\n"                      \
  "OpCapability VariablePointers\n"             \
  "OpExtension \"SPV_KHR_variable_pointers\"\n" \
  "OpMemoryModel Logical Simple\n"

#define kKernelPreamble           \
  "OpCapability Kernel\n"         \
  "OpCapability Linkage\n"        \
  "OpCapability Int64\n"          \
  "OpCapability Float64\n"        \
  "OpCapability GenericPointer\n" \
  "OpCapability Addresses\n"      \
  "OpMemoryModel Physical32 OpenCL\n"

struct ConstantOpCase {
  spv_target_env env;
  std::string assembly;
  bool expect_success;
  std::string expect_err;
  spv_result_t expected_result = SPV_ERROR_INVALID_ID;
};

using ValidateConstantOp = spvtest::ValidateBase<ConstantOpCase>;

TEST_P(ValidateConstantOp, Samples) {
  const auto env = GetParam().env;
  CompileSuccessfully(GetParam().assembly, env);
  const auto result = ValidateInstructions(env);
  if (GetParam().expect_success) {
    EXPECT_EQ(SPV_SUCCESS, result);
    EXPECT_THAT(getDiagnosticString(), Eq(""));
  } else {
    EXPECT_EQ(GetParam().expected_result, result);
    EXPECT_THAT(getDiagnosticString(), HasSubstr(GetParam().expect_err));
  }
}

#define GOOD_SHADER_10(STR) \
  { SPV_ENV_UNIVERSAL_1_0, kShaderPreamble kBasicTypes STR, true, "" }
#define GOOD_KERNEL_10(STR) \
  { SPV_ENV_UNIVERSAL_1_0, kKernelPreamble kBasicTypes STR, true, "" }
INSTANTIATE_TEST_SUITE_P(
    UniversalInShader, ValidateConstantOp,
    ValuesIn(std::vector<ConstantOpCase>{
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint SConvert %uint64_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %float FConvert %float64_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint SNegate %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint Not %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint IAdd %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint ISub %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint IMul %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint UDiv %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint SDiv %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint UMod %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint SRem %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint SMod %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %uint ShiftRightLogical %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %uint ShiftRightArithmetic %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %uint ShiftLeftLogical %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %uint BitwiseOr %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %uint BitwiseXor %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %uint2 VectorShuffle %uint2_0 %uint2_0 1 3"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %uint CompositeExtract %uint2_0 1"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %uint2 CompositeInsert %uint_0 %uint2_0 1"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %bool LogicalOr %true %false"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %bool LogicalNot %true"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %bool LogicalAnd %true %false"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %bool LogicalEqual %true %false"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %bool LogicalNotEqual %true %false"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %uint Select %true %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %bool IEqual %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %bool INotEqual %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %bool ULessThan %uint_0 %uint_0"),
        GOOD_SHADER_10("%v = OpSpecConstantOp %bool SLessThan %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %bool ULessThanEqual %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %bool SLessThanEqual %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %bool UGreaterThan %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %bool UGreaterThanEqual %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %bool SGreaterThan %uint_0 %uint_0"),
        GOOD_SHADER_10(
            "%v = OpSpecConstantOp %bool SGreaterThanEqual %uint_0 %uint_0"),
    }));

INSTANTIATE_TEST_SUITE_P(
    UniversalInKernel, ValidateConstantOp,
    ValuesIn(std::vector<ConstantOpCase>{
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint SConvert %uint64_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float FConvert %float64_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint SNegate %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint Not %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint IAdd %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint ISub %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint IMul %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint UDiv %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint SDiv %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint UMod %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint SRem %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint SMod %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %uint ShiftRightLogical %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %uint ShiftRightArithmetic %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %uint ShiftLeftLogical %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint BitwiseOr %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %uint BitwiseXor %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %uint2 VectorShuffle %uint2_0 %uint2_0 1 3"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %uint CompositeExtract %uint2_0 1"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %uint2 CompositeInsert %uint_0 %uint2_0 1"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %bool LogicalOr %true %false"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %bool LogicalNot %true"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %bool LogicalAnd %true %false"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %bool LogicalEqual %true %false"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %bool LogicalNotEqual %true %false"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %uint Select %true %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %bool IEqual %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %bool INotEqual %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %bool ULessThan %uint_0 %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %bool SLessThan %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %bool ULessThanEqual %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %bool SLessThanEqual %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %bool UGreaterThan %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %bool UGreaterThanEqual %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %bool SGreaterThan %uint_0 %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %bool SGreaterThanEqual %uint_0 %uint_0"),
    }));

INSTANTIATE_TEST_SUITE_P(
    UConvert, ValidateConstantOp,
    ValuesIn(std::vector<ConstantOpCase>{
        {SPV_ENV_UNIVERSAL_1_0,
         kKernelPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         true, ""},
        {SPV_ENV_UNIVERSAL_1_1,
         kKernelPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         true, ""},
        {SPV_ENV_UNIVERSAL_1_3,
         kKernelPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         true, ""},
        {SPV_ENV_UNIVERSAL_1_3,
         kKernelPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         true, ""},
        {SPV_ENV_UNIVERSAL_1_4,
         kKernelPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         true, ""},
        {SPV_ENV_UNIVERSAL_1_0,
         kShaderPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         false,
         "Prior to SPIR-V 1.4, specialization constant operation "
         "UConvert requires Kernel capability"},
        {SPV_ENV_UNIVERSAL_1_1,
         kShaderPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         false,
         "Prior to SPIR-V 1.4, specialization constant operation "
         "UConvert requires Kernel capability"},
        {SPV_ENV_UNIVERSAL_1_3,
         kShaderPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         false,
         "Prior to SPIR-V 1.4, specialization constant operation "
         "UConvert requires Kernel capability"},
        {SPV_ENV_UNIVERSAL_1_3,
         kShaderPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         false,
         "Prior to SPIR-V 1.4, specialization constant operation "
         "UConvert requires Kernel capability"},
        {SPV_ENV_UNIVERSAL_1_4,
         kShaderPreamble kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         true, ""},
    }));

INSTANTIATE_TEST_SUITE_P(
    KernelInKernel, ValidateConstantOp,
    ValuesIn(std::vector<ConstantOpCase>{
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint ConvertFToS %float_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float ConvertSToF %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint ConvertFToU %float_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float ConvertUToF %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint UConvert %uint64_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %uint Bitcast %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float FNegate %float_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float FAdd %float_0 %float_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float FSub %float_0 %float_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float FMul %float_0 %float_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float FDiv %float_0 %float_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float FRem %float_0 %float_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %float FMod %float_0 %float_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %_ptr_uint AccessChain %null %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %_ptr_uint InBoundsAccessChain "
                       "%null %uint_0"),
        GOOD_KERNEL_10(
            "%v = OpSpecConstantOp %_ptr_uint PtrAccessChain %null %uint_0"),
        GOOD_KERNEL_10("%v = OpSpecConstantOp %_ptr_uint "
                       "InBoundsPtrAccessChain %null %uint_0"),
    }));

#define BAD_SHADER_10(STR, NAME)                                   \
  {                                                                \
    SPV_ENV_UNIVERSAL_1_0, kShaderPreamble kBasicTypes STR, false, \
        "Specialization constant operation " NAME                  \
        " requires Kernel capability"                              \
  }
INSTANTIATE_TEST_SUITE_P(
    KernelInShader, ValidateConstantOp,
    ValuesIn(std::vector<ConstantOpCase>{
        // Don't need to test GenericCastToPtr or PtrCastToGeneric as a valid
        // module can't have a Generic storage class with Shader capability
        BAD_SHADER_10("%v = OpSpecConstantOp %uint ConvertFToS %float_0",
                      "ConvertFToS"),
        BAD_SHADER_10("%v = OpSpecConstantOp %float ConvertSToF %uint_0",
                      "ConvertSToF"),
        BAD_SHADER_10("%v = OpSpecConstantOp %uint ConvertFToU %float_0",
                      "ConvertFToU"),
        BAD_SHADER_10("%v = OpSpecConstantOp %float ConvertUToF %uint_0",
                      "ConvertUToF"),
        BAD_SHADER_10("%v = OpSpecConstantOp %uint Bitcast %uint_0", "Bitcast"),
        BAD_SHADER_10("%v = OpSpecConstantOp %float FNegate %float_0",
                      "FNegate"),
        BAD_SHADER_10("%v = OpSpecConstantOp %float FAdd %float_0 %float_0",
                      "FAdd"),
        BAD_SHADER_10("%v = OpSpecConstantOp %float FSub %float_0 %float_0",
                      "FSub"),
        BAD_SHADER_10("%v = OpSpecConstantOp %float FMul %float_0 %float_0",
                      "FMul"),
        BAD_SHADER_10("%v = OpSpecConstantOp %float FDiv %float_0 %float_0",
                      "FDiv"),
        BAD_SHADER_10("%v = OpSpecConstantOp %float FRem %float_0 %float_0",
                      "FRem"),
        BAD_SHADER_10("%v = OpSpecConstantOp %float FMod %float_0 %float_0",
                      "FMod"),
        BAD_SHADER_10(
            "%v = OpSpecConstantOp %_ptr_uint AccessChain %null %uint_0",
            "AccessChain"),
        BAD_SHADER_10("%v = OpSpecConstantOp %_ptr_uint InBoundsAccessChain "
                      "%null %uint_0",
                      "InBoundsAccessChain"),
        BAD_SHADER_10(
            "%v = OpSpecConstantOp %_ptr_uint PtrAccessChain %null %uint_0",
            "PtrAccessChain"),
        BAD_SHADER_10("%v = OpSpecConstantOp %_ptr_uint "
                      "InBoundsPtrAccessChain %null %uint_0",
                      "InBoundsPtrAccessChain"),
    }));

INSTANTIATE_TEST_SUITE_P(
    UConvertInAMD_gpu_shader_int16, ValidateConstantOp,
    ValuesIn(std::vector<ConstantOpCase>{
        // SPV_AMD_gpu_shader_int16 should enable UConvert for OpSpecConstantOp
        // https://github.com/KhronosGroup/glslang/issues/848
        {SPV_ENV_UNIVERSAL_1_0,
         "OpCapability Shader\n"
         "OpCapability Int64\n"
         "OpCapability Float64\n"
         "OpCapability VariablePointers\n"
         "OpCapability Linkage ; So we don't need to define a function\n"
         "OpExtension \"SPV_AMD_gpu_shader_int16\"\n"
         "OpExtension \"SPV_KHR_variable_pointers\"\n"
         "OpMemoryModel Logical Simple " kBasicTypes
         "%v = OpSpecConstantOp %uint UConvert %uint64_0",
         true, ""},
    }));

TEST_F(ValidateConstant, SpecConstantUConvert1p3Binary1p4EnvBad) {
  const std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%int = OpTypeInt 32 0
%int0 = OpConstant %int 0
%const = OpSpecConstantOp %int UConvert %int0
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "Prior to SPIR-V 1.4, specialization constant operation UConvert "
          "requires Kernel capability or extension SPV_AMD_gpu_shader_int16"));
}

using SmallStorageConstants = spvtest::ValidateBase<std::string>;

CodeGenerator GetSmallStorageCodeGenerator() {
  CodeGenerator generator;
  generator.capabilities_ = R"(
OpCapability Shader
OpCapability Linkage
OpCapability UniformAndStorageBuffer16BitAccess
OpCapability StoragePushConstant16
OpCapability StorageInputOutput16
OpCapability UniformAndStorageBuffer8BitAccess
OpCapability StoragePushConstant8
)";
  generator.extensions_ = R"(
OpExtension "SPV_KHR_16bit_storage"
OpExtension "SPV_KHR_8bit_storage"
)";
  generator.memory_model_ = "OpMemoryModel Logical GLSL450\n";
  generator.types_ = R"(
%short = OpTypeInt 16 0
%short2 = OpTypeVector %short 2
%char = OpTypeInt 8 0
%char2 = OpTypeVector %char 2
%half = OpTypeFloat 16
%half2 = OpTypeVector %half 2
%int = OpTypeInt 32 0
%int_0 = OpConstant %int 0
%float = OpTypeFloat 32
%float_0 = OpConstant %float 0
)";
  return generator;
}

TEST_P(SmallStorageConstants, SmallConstant) {
  std::string constant = GetParam();
  CodeGenerator generator = GetSmallStorageCodeGenerator();
  generator.after_types_ += constant + "\n";
  CompileSuccessfully(generator.Build(), SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Cannot form constants of 8- or 16-bit types"));
}

// Constant composites would be caught through scalar constants.
INSTANTIATE_TEST_SUITE_P(
    SmallConstants, SmallStorageConstants,
    Values("%c = OpConstant %char 0", "%c = OpConstantNull %char2",
           "%c = OpConstant %short 0", "%c = OpConstantNull %short",
           "%c = OpConstant %half 0", "%c = OpConstantNull %half",
           "%c = OpSpecConstant %char 0", "%c = OpSpecConstant %short 0",
           "%c = OpSpecConstant %half 0",
           "%c = OpSpecConstantOp %char SConvert %int_0",
           "%c = OpSpecConstantOp %short SConvert %int_0",
           "%c = OpSpecConstantOp %half FConvert %float_0"));

TEST_F(ValidateConstant, NullPointerTo16BitStorageOk) {
  std::string spirv = R"(
OpCapability Shader
OpCapability VariablePointersStorageBuffer
OpCapability UniformAndStorageBuffer16BitAccess
OpCapability Linkage
OpExtension "SPV_KHR_16bit_storage"
OpMemoryModel Logical GLSL450
%half = OpTypeFloat 16
%ptr_ssbo_half = OpTypePointer StorageBuffer %half
%null_ptr = OpConstantNull %ptr_ssbo_half
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

TEST_F(ValidateConstant, NullMatrix) {
  std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%float = OpTypeFloat 32
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%null_vector = OpConstantNull %v2float
%null_matrix = OpConstantComposite %mat2x2 %null_vector %null_vector
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConstant, NullPhysicalStorageBuffer) {
  std::string spirv = R"(
OpCapability Shader
OpCapability PhysicalStorageBufferAddresses
OpCapability Linkage
OpExtension "SPV_KHR_physical_storage_buffer"
OpMemoryModel PhysicalStorageBuffer64 GLSL450
OpName %ptr "ptr"
%int = OpTypeInt 32 0
%ptr = OpTypePointer PhysicalStorageBuffer %int
%null = OpConstantNull %ptr
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpConstantNull Result Type <id> '1[%ptr]' cannot have "
                        "a null value"));
}

TEST_F(ValidateConstant, VectorMismatchedConstituents) {
  std::string spirv = kShaderPreamble kBasicTypes R"(
%int = OpTypeInt 32 1
%int_0 = OpConstantNull %int
%const_vector = OpConstantComposite %uint2 %uint_0 %int_0
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpConstantComposite Constituent <id> '17[%17]'s type "
          "does not match Result Type <id> '4[%v2uint]'s vector element type"));
}

TEST_F(ValidateConstant, ConstantCompositeReplicateVectorGood) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Logical Simple\n") +
      kBasicTypes + R"(
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%int_0 = OpConstant %int 0
%const_vector = OpConstantCompositeReplicateEXT %v4int %int_0
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConstant, SpecConstantCompositeReplicateVectorGood) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Logical Simple\n") +
      kBasicTypes + R"(
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%int_0 = OpSpecConstant %int 0
%const_vector = OpSpecConstantCompositeReplicateEXT %v4int %int_0
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConstant, ConstantCompositeReplicateMatrixGood) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Logical Simple\n") +
      kBasicTypes + R"(
%v2float = OpTypeVector %float 2
%mat2x2 = OpTypeMatrix %v2float 2
%v_0 = OpConstantNull %v2float
%const_matrix = OpConstantCompositeReplicateEXT %mat2x2 %v_0
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConstant, ConstantCompositeReplicateArrayGood) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Logical Simple\n") +
      kBasicTypes + R"(
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%arr = OpTypeArray %int %int_4
%int_0 = OpConstantNull %int
%const_arr = OpConstantCompositeReplicateEXT %arr %int_0
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConstant, ConstantCompositeReplicateStructGood) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Logical Simple\n") +
      kBasicTypes + R"(
%int = OpTypeInt 32 1
%struct = OpTypeStruct %int %int %int
%int_0 = OpConstantNull %int
%const_struct = OpConstantCompositeReplicateEXT %struct %int_0
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConstant, ConstantCompositeReplicateWrongOperandType) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Logical Simple\n") +
      kBasicTypes + R"(
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%const_vector = OpConstantCompositeReplicateEXT %v4int %float_0
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpConstantCompositeReplicateEXT Constituent <id> '11[%11]'s type "
          "does not match Result Type <id> '17[%v4int]'s element type"));
}

TEST_F(ValidateConstant, ConstantCompositeReplicateSpecOperand) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Logical Simple\n") +
      kBasicTypes + R"(
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%arr = OpTypeArray %int %int_4
%int_0 = OpSpecConstant %int 0
%const_arr = OpConstantCompositeReplicateEXT %arr %int_0
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpConstantCompositeReplicateEXT must not have spec "
                        "constant operands: <id>"));
}

TEST_F(ValidateConstant, ConstantCompositeReplicateNotConstant) {
  std::string spirv =
      std::string(
          "OpCapability Kernel\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability Addresses\nOpCapability "
          "ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Physical64 "
          "OpenCL\n") +
      kBasicTypes + R"(
%uint_4 = OpConstant %uint 4
%ptr = OpTypePointer Private %uint
%var = OpVariable %ptr Private
%arr = OpTypeArray %ptr %uint_4
%const_arr = OpConstantCompositeReplicateEXT %arr %var
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("OpConstantCompositeReplicateEXT must only have "
                        "constant, undef, or poison operands: <id>"));
}

TEST_F(ValidateConstant, ConstantCompositeSpecOperand) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Logical Simple\n") +
      kBasicTypes + R"(
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%arr = OpTypeArray %int %int_4
%int_0 = OpSpecConstant %int 0
%const_arr = OpConstantComposite %arr %int_0 %int_0 %int_0 %int_0
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr(
          "OpConstantComposite must not have spec constant operands: <id>"));
}

TEST_F(ValidateConstant, ConstantCompositeNotConstant) {
  std::string spirv =
      std::string(
          "OpCapability Kernel\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability Addresses\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpMemoryModel Physical64 OpenCL\n") +
      kBasicTypes + R"(
%uint_4 = OpConstant %uint 4
%ptr = OpTypePointer Private %uint
%var = OpVariable %ptr Private
%arr = OpTypeArray %ptr %uint_4
%const_arr = OpConstantComposite %arr %var %var %var %var
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpConstantComposite must only have constant, undef, or "
                "poison operands: <id>"));
}

TEST_F(ValidateConstant, ConstantCompositePoisonConstituentGood) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "PoisonFreezeKHR\nOpExtension \"SPV_KHR_poison_freeze\"\n"
          "OpMemoryModel Logical Simple\n") +
      R"(
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%arr = OpTypeArray %int %int_4
%poison = OpPoisonKHR %int
%const_arr = OpConstantComposite %arr %poison %poison %poison %poison
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConstant, SpecConstantCompositePoisonConstituentGood) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "PoisonFreezeKHR\nOpExtension \"SPV_KHR_poison_freeze\"\n"
          "OpMemoryModel Logical Simple\n") +
      R"(
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%arr = OpTypeArray %int %int_4
%poison = OpPoisonKHR %int
%const_arr = OpSpecConstantComposite %arr %poison %poison %poison %poison
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
}

TEST_F(ValidateConstant, ConstantCompositeReplicateNotComposite) {
  std::string spirv =
      std::string(
          "OpCapability Shader\nOpCapability Linkage\nOpCapability "
          "Int64\nOpCapability Float64\nOpCapability "
          "VariablePointers\nOpCapability ReplicatedCompositesEXT\nOpExtension "
          "\"SPV_KHR_variable_pointers\"\nOpExtension "
          "\"SPV_EXT_replicated_composites\"\nOpMemoryModel Logical Simple\n") +
      kBasicTypes + R"(
%int = OpTypeInt 32 1
%int_0 = OpConstantNull %int
%const_vector = OpConstantCompositeReplicateEXT %float %int_0
)";
  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("OpConstantCompositeReplicateEXT Result Type <id> '5[%float]' "
                "is not a composite type"));
}

TEST_F(ValidateConstant, BadShaderOperandsQuantizeToF16) {
  std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpMemoryModel Logical GLSL450
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%uint_1 = OpConstant %uint 1
%float_1 = OpConstant %float 1
%good = OpSpecConstantOp %float QuantizeToF16 %float_1
%bad1 = OpSpecConstantOp %uint QuantizeToF16 %float_1
%bad2 = OpSpecConstantOp %float QuantizeToF16 %uint_1
)";

  CompileSuccessfully(spirv);
  ASSERT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions());
  EXPECT_THAT(
      getDiagnosticString(),
      HasSubstr("Expected 32-bit float scalar or vector type as Result Type"));
}

TEST_F(ValidateConstant, BadCooperativeMatrixLength) {
  std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability CooperativeMatrixKHR
OpCapability VulkanMemoryModelKHR
OpExtension "SPV_KHR_cooperative_matrix"
OpExtension "SPV_KHR_vulkan_memory_model"
OpMemoryModel Logical VulkanKHR
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%uint_1 = OpConstant %uint 1
%uint_8 = OpConstant %uint 8
%float_1 = OpConstant %float 1
%subgroup = OpConstant %uint 3
%use_A = OpConstant %uint 0
%f16mat = OpTypeCooperativeMatrixKHR %float %subgroup %uint_8 %uint_8 %use_A

%good = OpSpecConstantOp %uint CooperativeMatrixLengthKHR %f16mat
%bad1 = OpSpecConstantOp %uint CooperativeMatrixLengthKHR %float_1
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_4);
  ASSERT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions(SPV_ENV_UNIVERSAL_1_4));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("must be OpTypeCooperativeMatrixKHR"));
}

TEST_F(ValidateConstant, UntypedAccessChainSpecConstantOpGood) {
  std::string spirv = R"(
OpCapability Addresses
OpCapability Kernel
OpCapability Linkage
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Physical32 OpenCL
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_4 = OpConstant %uint 4
%arr = OpTypeArray %uint %uint_4
%ptr = OpTypeUntypedPointerKHR CrossWorkgroup
%var = OpUntypedVariableKHR %ptr CrossWorkgroup %arr
%ac = OpSpecConstantOp %ptr UntypedAccessChainKHR %arr %var %uint_0
%iac = OpSpecConstantOp %ptr UntypedInBoundsAccessChainKHR %arr %var %uint_0
%pac = OpSpecConstantOp %ptr UntypedPtrAccessChainKHR %arr %var %uint_0
%ipac = OpSpecConstantOp %ptr UntypedInBoundsPtrAccessChainKHR %arr %var %uint_0
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(), Eq(""));
}

TEST_F(ValidateConstant, UntypedAccessChainSpecConstantOpRequiresKernel) {
  std::string spirv = R"(
OpCapability Shader
OpCapability Linkage
OpCapability UntypedPointersKHR
OpExtension "SPV_KHR_untyped_pointers"
OpMemoryModel Logical GLSL450
%uint = OpTypeInt 32 0
%uint_0 = OpConstant %uint 0
%uint_4 = OpConstant %uint 4
%arr = OpTypeArray %uint %uint_4
%ptr = OpTypeUntypedPointerKHR Workgroup
%var = OpUntypedVariableKHR %ptr Workgroup %arr
%ac = OpSpecConstantOp %ptr UntypedInBoundsPtrAccessChainKHR %arr %var %uint_0
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_ERROR_INVALID_ID, ValidateInstructions());
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Specialization constant operation "
                        "UntypedInBoundsPtrAccessChainKHR requires Kernel "
                        "capability"));
}

// Some check use SPV_ERROR_INVALID_DATA vs SPV_ERROR_INVALID_ID
#define BAD_KERNEL_OPERANDS(STR, ERR)                                   \
  {                                                                     \
    SPV_ENV_UNIVERSAL_1_0, kKernelPreamble kBasicTypes STR, false, ERR, \
        SPV_ERROR_INVALID_DATA                                          \
  }

#define BAD_KERNEL_OPERANDS_ID(STR, ERR) \
  { SPV_ENV_UNIVERSAL_1_0, kKernelPreamble kBasicTypes STR, false, ERR, }

// 2 of each, first has bad return type, second has bad operand
INSTANTIATE_TEST_SUITE_P(
    BadOperandsKernel, ValidateConstantOp,
    ValuesIn(std::vector<ConstantOpCase>{
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float SConvert %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint SConvert %uint_0",
            "Expected input to have different bit width from Result Type"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint FConvert %float_0",
            "Expected float scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %float FConvert %float_0",
                            "Expected component type of Value to be different "
                            "from component type of Result Type"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float UConvert %uint_0",
            "Expected unsigned int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint UConvert %uint_0",
            "Expected input to have different bit width from Result Type"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float ConvertFToS %float_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint ConvertFToS %uint2_0",
                            "Expected input to be float scalar or vector"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint ConvertSToF %uint_0",
            "Expected float scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %float ConvertSToF %float_0",
                            "Expected input to be int scalar or vector"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float ConvertFToU %float_0",
            "Expected unsigned int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint ConvertFToU %uint2_0",
                            "Expected input to be float scalar or vector"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint ConvertUToF %uint_0",
            "Expected float scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %float ConvertUToF %float_0",
                            "Expected input to be int scalar or vector"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint ConvertUToF %uint_0",
            "Expected float scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %float ConvertUToF %float_0",
                            "Expected input to be int scalar or vector"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float UDiv %uint_0 %uint_0",
            "Expected unsigned int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint UDiv %uint_0 %float_0",
            "Expected arithmetic operands to be of Result Type"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float UMod %uint_0 %uint_0",
            "Expected unsigned int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint UMod %uint_0 %float_0",
            "Expected arithmetic operands to be of Result Type"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float ISub %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint ISub %uint_0 %float_0",
                            "Expected int scalar or vector type as operand"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float IAdd %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint IAdd %uint_0 %float_0",
                            "Expected int scalar or vector type as operand"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float IMul %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint IMul %uint_0 %float_0",
                            "Expected int scalar or vector type as operand"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float SDiv %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint SDiv %uint_0 %float_0",
                            "Expected int scalar or vector type as operand"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float SRem %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint SRem %uint_0 %float_0",
                            "Expected int scalar or vector type as operand"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float SMod %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint SMod %uint_0 %float_0",
                            "Expected int scalar or vector type as operand"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float SNegate %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint SNegate %float_0",
                            "Expected int scalar or vector type as operand"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint FAdd %float_0 %float_0",
            "Expected floating scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float FAdd %float_0 %uint_0",
            "Expected arithmetic operands to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint FSub %float_0 %float_0",
            "Expected floating scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float FSub %float_0 %uint_0",
            "Expected arithmetic operands to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint FMul %float_0 %float_0",
            "Expected floating scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float FMul %float_0 %uint_0",
            "Expected arithmetic operands to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint FDiv %float_0 %float_0",
            "Expected floating scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float FDiv %float_0 %uint_0",
            "Expected arithmetic operands to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint FRem %float_0 %float_0",
            "Expected floating scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float FRem %float_0 %uint_0",
            "Expected arithmetic operands to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint FMod %float_0 %float_0",
            "Expected floating scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float FMod %float_0 %uint_0",
            "Expected arithmetic operands to be of Result Type"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float ShiftRightLogical %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint ShiftRightLogical %uint_0 %float_0",
            "Expected Shift to be int scalar or vector"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float ShiftRightArithmetic %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint ShiftRightArithmetic %uint_0 %float_0",
            "Expected Shift to be int scalar or vector"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float ShiftLeftLogical %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint ShiftLeftLogical %uint_0 %float_0",
            "Expected Shift to be int scalar or vector"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float BitwiseOr %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint BitwiseOr %uint_0 %float_0",
            "Expected int scalar or vector as operand"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float BitwiseXor %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint BitwiseXor %uint_0 %float_0",
            "Expected int scalar or vector as operand"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float BitwiseAnd %uint_0 %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint BitwiseAnd %uint_0 %float_0",
            "Expected int scalar or vector as operand"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float Not %uint_0",
            "Expected int scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint Not %float_0",
                            "Expected int scalar or vector as operand"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float LogicalOr %true %false",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool LogicalOr %true %uint_0",
            "Expected both operands to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float LogicalAnd %true %false",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool LogicalAnd %true %uint_0",
            "Expected both operands to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float LogicalEqual %true %false",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool LogicalEqual %true %uint_0",
            "Expected both operands to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float LogicalNotEqual %true %false",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool LogicalNotEqual %uint_0 %false",
            "Expected both operands to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float LogicalNot %true",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %bool LogicalNot %uint_0",
                            "Expected operand to be of Result Type"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float Select %true %uint_0 %uint_0",
            "Expected both objects to be of Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint Select %uint_0 %uint_0 %uint_0",
            "Expected bool scalar or vector type as condition"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float IEqual %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool IEqual %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float INotEqual %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool INotEqual %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float ULessThan %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool ULessThan %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float SLessThan %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool SLessThan %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float ULessThanEqual %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool ULessThanEqual %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float SLessThanEqual %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool SLessThanEqual %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float UGreaterThan %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool UGreaterThan %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float UGreaterThanEqual %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool UGreaterThanEqual %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float SGreaterThan %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool SGreaterThan %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float SGreaterThanEqual %uint_0 %uint_0",
            "Expected bool scalar or vector type as Result Type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %bool SGreaterThanEqual %uint_0 %float_0",
            "Expected operands to be scalar or vector int"),

        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float GenericCastToPtr %null",
            "Expected Result Type to be a pointer"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float PtrCastToGeneric %null",
            "Expected Result Type to be a pointer"),

        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %bool Bitcast %uint_0",
                            "Expected Result Type to be a pointer or int or "
                            "float vector or scalar type"),
        BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint Bitcast %true",
                            "Expected input to be a pointer or int or float "
                            "vector or scalar"),

        BAD_KERNEL_OPERANDS_ID(
            "%v = OpSpecConstantOp %float VectorShuffle %uint2_0 %uint2_0 1 3",
            "The Result Type of OpVectorShuffle must be a vector type"),
        BAD_KERNEL_OPERANDS_ID(
            "%v = OpSpecConstantOp %uint2 VectorShuffle %uint2_0 %uint_0 1 3",
            "The type of Vector 2 must be a vector type"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float CompositeExtract %uint2_0 1",
            "Result type (OpTypeFloat) does not match the type that results "
            "from indexing into the composite (OpTypeInt)"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint CompositeExtract %uint_0 1",
            "Reached non-composite type while indexes still remain to be "
            "traversed"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %float CompositeInsert %uint_0 %uint2_0 1",
            "The Result Type must be the same as Composite type in "
            "OpSpecConstantOp yielding Result Id 5"),
        BAD_KERNEL_OPERANDS(
            "%v = OpSpecConstantOp %uint2 CompositeInsert %uint_0 %uint_0 1",
            "The Result Type must be the same as Composite type in "
            "OpSpecConstantOp yielding Result Id 4"),

        // TODO - Still need to add access chains
        //
        // BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint AccessChain %null",
        //                     "AccessChain"),
        // BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %_ptr_uint AccessChain
        // %null %float_0",
        //     "AccessChain"),
        // BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint InBoundsAccessChain
        // %null",
        //     "InBoundsAccessChain"),
        // BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %_ptr_uint
        // InBoundsAccessChain %null %float_0",
        //                     "InBoundsAccessChain"),
        // BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint PtrAccessChain %null
        // %uint_0",
        //     "PtrAccessChain"),
        // BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %_ptr_uint PtrAccessChain
        // %float_0 %float_0",
        //     "PtrAccessChain"),
        // BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %uint
        // InBoundsPtrAccessChain %null %uint_0",
        //     "InBoundsPtrAccessChain"),
        // BAD_KERNEL_OPERANDS("%v = OpSpecConstantOp %_ptr_uint
        // InBoundsPtrAccessChain %float_0 %float_0",
        //                     "InBoundsPtrAccessChain"),
    }));

TEST_F(ValidateConstant, ForwardConstantFunctionPointerINTEL) {
  const std::string spirv = R"(
OpCapability Linkage
OpCapability Shader
OpCapability FunctionPointersINTEL
OpExtension "SPV_INTEL_function_pointers"
OpMemoryModel Logical Simple
%void = OpTypeVoid
%functype = OpTypeFunction %void
; UniformConstant avoids logical pointer validation conflicts in Function sc
%ptr_fun = OpTypePointer UniformConstant %functype
%const_ptr = OpConstantFunctionPointerINTEL %ptr_fun %target_func
%target_func = OpFunction %void None %functype
%lbl = OpLabel
OpReturn
OpFunctionEnd
)";

  CompileSuccessfully(spirv, SPV_ENV_UNIVERSAL_1_3);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
