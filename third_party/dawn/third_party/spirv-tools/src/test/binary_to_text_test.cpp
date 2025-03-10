// Copyright (c) 2015-2016 The Khronos Group Inc.
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
#include <tuple>
#include <vector>

#include "gmock/gmock.h"
#include "source/spirv_constant.h"
#include "test/test_fixture.h"
#include "test/unit_spirv.h"

namespace spvtools {
namespace {

using spvtest::AutoText;
using spvtest::ScopedContext;
using spvtest::TextToBinaryTest;
using ::testing::Combine;
using ::testing::Eq;
using ::testing::HasSubstr;

class BinaryToText : public ::testing::Test {
 public:
  BinaryToText()
      : context(spvContextCreate(SPV_ENV_UNIVERSAL_1_0)), binary(nullptr) {}
  ~BinaryToText() override {
    spvBinaryDestroy(binary);
    spvContextDestroy(context);
  }

  void SetUp() override {
    const char* textStr = R"(
      OpSource OpenCL_C 12
      OpMemoryModel Physical64 OpenCL
      OpSourceExtension "PlaceholderExtensionName"
      OpEntryPoint Kernel %1 "foo"
      OpExecutionMode %1 LocalSizeHint 1 1 1
 %2 = OpTypeVoid
 %3 = OpTypeBool
 %4 = OpTypeInt 8 0
 %5 = OpTypeInt 8 1
 %6 = OpTypeInt 16 0
 %7 = OpTypeInt 16 1
 %8 = OpTypeInt 32 0
 %9 = OpTypeInt 32 1
%10 = OpTypeInt 64 0
%11 = OpTypeInt 64 1
%12 = OpTypeFloat 16
%13 = OpTypeFloat 32
%14 = OpTypeFloat 64
%15 = OpTypeVector %4 2
)";
    spv_text_t text = {textStr, strlen(textStr)};
    spv_diagnostic diagnostic = nullptr;
    spv_result_t error =
        spvTextToBinary(context, text.str, text.length, &binary, &diagnostic);
    spvDiagnosticPrint(diagnostic);
    spvDiagnosticDestroy(diagnostic);
    ASSERT_EQ(SPV_SUCCESS, error);
  }

  void TearDown() override {
    spvBinaryDestroy(binary);
    binary = nullptr;
  }

  // Compiles the given assembly text, and saves it into 'binary'.
  void CompileSuccessfully(std::string text) {
    spvBinaryDestroy(binary);
    binary = nullptr;
    spv_diagnostic diagnostic = nullptr;
    EXPECT_EQ(SPV_SUCCESS, spvTextToBinary(context, text.c_str(), text.size(),
                                           &binary, &diagnostic));
  }

  spv_context context;
  spv_binary binary;
};

TEST_F(BinaryToText, Default) {
  spv_text text = nullptr;
  spv_diagnostic diagnostic = nullptr;
  ASSERT_EQ(
      SPV_SUCCESS,
      spvBinaryToText(context, binary->code, binary->wordCount,
                      SPV_BINARY_TO_TEXT_OPTION_NONE, &text, &diagnostic));
  printf("%s", text->str);
  spvTextDestroy(text);
}

TEST_F(BinaryToText, Print) {
  spv_text text = nullptr;
  spv_diagnostic diagnostic = nullptr;
  ASSERT_EQ(
      SPV_SUCCESS,
      spvBinaryToText(context, binary->code, binary->wordCount,
                      SPV_BINARY_TO_TEXT_OPTION_PRINT, &text, &diagnostic));
  ASSERT_EQ(text, nullptr);
  spvTextDestroy(text);
}

TEST_F(BinaryToText, MissingModule) {
  spv_text text;
  spv_diagnostic diagnostic = nullptr;
  EXPECT_EQ(
      SPV_ERROR_INVALID_BINARY,
      spvBinaryToText(context, nullptr, 42, SPV_BINARY_TO_TEXT_OPTION_NONE,
                      &text, &diagnostic));
  EXPECT_THAT(diagnostic->error, Eq(std::string("Missing module.")));
  if (diagnostic) {
    spvDiagnosticPrint(diagnostic);
    spvDiagnosticDestroy(diagnostic);
  }
}

TEST_F(BinaryToText, TruncatedModule) {
  // Make a valid module with zero instructions.
  CompileSuccessfully("");
  EXPECT_EQ(SPV_INDEX_INSTRUCTION, binary->wordCount);

  for (size_t length = 0; length < SPV_INDEX_INSTRUCTION; length++) {
    spv_text text = nullptr;
    spv_diagnostic diagnostic = nullptr;
    EXPECT_EQ(
        SPV_ERROR_INVALID_BINARY,
        spvBinaryToText(context, binary->code, length,
                        SPV_BINARY_TO_TEXT_OPTION_NONE, &text, &diagnostic));
    ASSERT_NE(nullptr, diagnostic);
    std::stringstream expected;
    expected << "Module has incomplete header: only " << length
             << " words instead of " << SPV_INDEX_INSTRUCTION;
    EXPECT_THAT(diagnostic->error, Eq(expected.str()));
    spvDiagnosticDestroy(diagnostic);
  }
}

TEST_F(BinaryToText, InvalidMagicNumber) {
  CompileSuccessfully("");
  std::vector<uint32_t> damaged_binary(binary->code,
                                       binary->code + binary->wordCount);
  damaged_binary[SPV_INDEX_MAGIC_NUMBER] ^= 123;

  spv_diagnostic diagnostic = nullptr;
  spv_text text;
  EXPECT_EQ(
      SPV_ERROR_INVALID_BINARY,
      spvBinaryToText(context, damaged_binary.data(), damaged_binary.size(),
                      SPV_BINARY_TO_TEXT_OPTION_NONE, &text, &diagnostic));
  ASSERT_NE(nullptr, diagnostic);
  std::stringstream expected;
  expected << "Invalid SPIR-V magic number '" << std::hex
           << damaged_binary[SPV_INDEX_MAGIC_NUMBER] << "'.";
  EXPECT_THAT(diagnostic->error, Eq(expected.str()));
  spvDiagnosticDestroy(diagnostic);
}

struct FailedDecodeCase {
  std::string source_text;
  std::vector<uint32_t> appended_instruction;
  std::string expected_error_message;
};

using BinaryToTextFail =
    spvtest::TextToBinaryTestBase<::testing::TestWithParam<FailedDecodeCase>>;

TEST_P(BinaryToTextFail, EncodeSuccessfullyDecodeFailed) {
  EXPECT_THAT(EncodeSuccessfullyDecodeFailed(GetParam().source_text,
                                             GetParam().appended_instruction),
              Eq(GetParam().expected_error_message));
}

INSTANTIATE_TEST_SUITE_P(
    InvalidIds, BinaryToTextFail,
    ::testing::ValuesIn(std::vector<FailedDecodeCase>{
        {"", spvtest::MakeInstruction(spv::Op::OpTypeVoid, {0}),
         "Error: Result Id is 0"},
        {"", spvtest::MakeInstruction(spv::Op::OpConstant, {0, 1, 42}),
         "Error: Type Id is 0"},
        {"%1 = OpTypeVoid", spvtest::MakeInstruction(spv::Op::OpTypeVoid, {1}),
         "Id 1 is defined more than once"},
        {"%1 = OpTypeVoid\n"
         "%2 = OpNot %1 %foo",
         spvtest::MakeInstruction(spv::Op::OpNot, {1, 2, 3}),
         "Id 2 is defined more than once"},
        {"%1 = OpTypeVoid\n"
         "%2 = OpNot %1 %foo",
         spvtest::MakeInstruction(spv::Op::OpNot, {1, 1, 3}),
         "Id 1 is defined more than once"},
        // The following are the two failure cases for
        // Parser::setNumericTypeInfoForType.
        {"", spvtest::MakeInstruction(spv::Op::OpConstant, {500, 1, 42}),
         "Type Id 500 is not a type"},
        {"%1 = OpTypeInt 32 0\n"
         "%2 = OpTypeVector %1 4",
         spvtest::MakeInstruction(spv::Op::OpConstant, {2, 3, 999}),
         "Type Id 2 is not a scalar numeric type"},
    }));

INSTANTIATE_TEST_SUITE_P(
    InvalidIdsCheckedDuringLiteralCaseParsing, BinaryToTextFail,
    ::testing::ValuesIn(std::vector<FailedDecodeCase>{
        {"", spvtest::MakeInstruction(spv::Op::OpSwitch, {1, 2, 3, 4}),
         "Invalid OpSwitch: selector id 1 has no type"},
        {"%1 = OpTypeVoid\n",
         spvtest::MakeInstruction(spv::Op::OpSwitch, {1, 2, 3, 4}),
         "Invalid OpSwitch: selector id 1 is a type, not a value"},
        {"%1 = OpConstantTrue !500",
         spvtest::MakeInstruction(spv::Op::OpSwitch, {1, 2, 3, 4}),
         "Type Id 500 is not a type"},
        {"%1 = OpTypeFloat 32\n%2 = OpConstant %1 1.5",
         spvtest::MakeInstruction(spv::Op::OpSwitch, {2, 3, 4, 5}),
         "Invalid OpSwitch: selector id 2 is not a scalar integer"},
    }));

TEST_F(TextToBinaryTest, OneInstruction) {
  const std::string input = "OpSource OpenCL_C 12\n";
  EXPECT_EQ(input, EncodeAndDecodeSuccessfully(input));
}

// Exercise the case where an operand itself has operands.
// This could detect problems in updating the expected-set-of-operands
// list.
TEST_F(TextToBinaryTest, OperandWithOperands) {
  const std::string input = R"(OpEntryPoint Kernel %1 "foo"
OpExecutionMode %1 LocalSizeHint 100 200 300
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%1 = OpFunction %1 None %3
)";
  EXPECT_EQ(input, EncodeAndDecodeSuccessfully(input));
}

using RoundTripInstructionsTest = spvtest::TextToBinaryTestBase<
    ::testing::TestWithParam<std::tuple<spv_target_env, std::string>>>;

TEST_P(RoundTripInstructionsTest, Sample) {
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  std::get<1>(GetParam()), SPV_BINARY_TO_TEXT_OPTION_NONE,
                  SPV_TEXT_TO_BINARY_OPTION_NONE, std::get<0>(GetParam())),
              Eq(std::get<1>(GetParam())));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    NumericLiterals, RoundTripInstructionsTest,
    // This test is independent of environment, so just test the one.
    Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                              SPV_ENV_UNIVERSAL_1_2, SPV_ENV_UNIVERSAL_1_3),
            ::testing::ValuesIn(std::vector<std::string>{
                "%1 = OpTypeInt 12 0\n%2 = OpConstant %1 1867\n",
                "%1 = OpTypeInt 12 1\n%2 = OpConstant %1 1867\n",
                "%1 = OpTypeInt 12 1\n%2 = OpConstant %1 -1867\n",
                "%1 = OpTypeInt 32 0\n%2 = OpConstant %1 1867\n",
                "%1 = OpTypeInt 32 1\n%2 = OpConstant %1 1867\n",
                "%1 = OpTypeInt 32 1\n%2 = OpConstant %1 -1867\n",
                "%1 = OpTypeInt 64 0\n%2 = OpConstant %1 18446744073709551615\n",
                "%1 = OpTypeInt 64 1\n%2 = OpConstant %1 9223372036854775807\n",
                "%1 = OpTypeInt 64 1\n%2 = OpConstant %1 -9223372036854775808\n",
                // 16-bit floats print as hex floats.
                "%1 = OpTypeFloat 16\n%2 = OpConstant %1 0x1.ff4p+16\n",
                "%1 = OpTypeFloat 16\n%2 = OpConstant %1 -0x1.d2cp-10\n",
                // 32-bit floats
                "%1 = OpTypeFloat 32\n%2 = OpConstant %1 -3.125\n",
                "%1 = OpTypeFloat 32\n%2 = OpConstant %1 0x1.8p+128\n", // NaN
                "%1 = OpTypeFloat 32\n%2 = OpConstant %1 -0x1.0002p+128\n", // NaN
                "%1 = OpTypeFloat 32\n%2 = OpConstant %1 0x1p+128\n", // Inf
                "%1 = OpTypeFloat 32\n%2 = OpConstant %1 -0x1p+128\n", // -Inf
                // 64-bit floats
                "%1 = OpTypeFloat 64\n%2 = OpConstant %1 -3.125\n",
                "%1 = OpTypeFloat 64\n%2 = OpConstant %1 0x1.ffffffffffffap-1023\n", // small normal
                "%1 = OpTypeFloat 64\n%2 = OpConstant %1 -0x1.ffffffffffffap-1023\n",
                "%1 = OpTypeFloat 64\n%2 = OpConstant %1 0x1.8p+1024\n", // NaN
                "%1 = OpTypeFloat 64\n%2 = OpConstant %1 -0x1.0002p+1024\n", // NaN
                "%1 = OpTypeFloat 64\n%2 = OpConstant %1 0x1p+1024\n", // Inf
                "%1 = OpTypeFloat 64\n%2 = OpConstant %1 -0x1p+1024\n", // -Inf
            })));
// clang-format on

INSTANTIATE_TEST_SUITE_P(
    MemoryAccessMasks, RoundTripInstructionsTest,
    Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                              SPV_ENV_UNIVERSAL_1_2, SPV_ENV_UNIVERSAL_1_3),
            ::testing::ValuesIn(std::vector<std::string>{
                "OpStore %1 %2\n",       // 3 words long.
                "OpStore %1 %2 None\n",  // 4 words long, explicit final 0.
                "OpStore %1 %2 Volatile\n",
                "OpStore %1 %2 Aligned 8\n",
                "OpStore %1 %2 Nontemporal\n",
                // Combinations show the names from LSB to MSB
                "OpStore %1 %2 Volatile|Aligned 16\n",
                "OpStore %1 %2 Volatile|Nontemporal\n",
                "OpStore %1 %2 Volatile|Aligned|Nontemporal 32\n",
            })));

INSTANTIATE_TEST_SUITE_P(
    FPFastMathModeMasks, RoundTripInstructionsTest,
    Combine(
        ::testing::Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                          SPV_ENV_UNIVERSAL_1_2, SPV_ENV_UNIVERSAL_1_3),
        ::testing::ValuesIn(std::vector<std::string>{
            "OpDecorate %1 FPFastMathMode None\n",
            "OpDecorate %1 FPFastMathMode NotNaN\n",
            "OpDecorate %1 FPFastMathMode NotInf\n",
            "OpDecorate %1 FPFastMathMode NSZ\n",
            "OpDecorate %1 FPFastMathMode AllowRecip\n",
            "OpDecorate %1 FPFastMathMode Fast\n",
            // Combinations show the names from LSB to MSB
            "OpDecorate %1 FPFastMathMode NotNaN|NotInf\n",
            "OpDecorate %1 FPFastMathMode NSZ|AllowRecip\n",
            "OpDecorate %1 FPFastMathMode NotNaN|NotInf|NSZ|AllowRecip|Fast\n",
        })));

INSTANTIATE_TEST_SUITE_P(
    LoopControlMasks, RoundTripInstructionsTest,
    Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                              SPV_ENV_UNIVERSAL_1_3, SPV_ENV_UNIVERSAL_1_2),
            ::testing::ValuesIn(std::vector<std::string>{
                "OpLoopMerge %1 %2 None\n",
                "OpLoopMerge %1 %2 Unroll\n",
                "OpLoopMerge %1 %2 DontUnroll\n",
                "OpLoopMerge %1 %2 Unroll|DontUnroll\n",
            })));

INSTANTIATE_TEST_SUITE_P(LoopControlMasksV11, RoundTripInstructionsTest,
                         Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_1,
                                                   SPV_ENV_UNIVERSAL_1_2,
                                                   SPV_ENV_UNIVERSAL_1_3),
                                 ::testing::ValuesIn(std::vector<std::string>{
                                     "OpLoopMerge %1 %2 DependencyInfinite\n",
                                     "OpLoopMerge %1 %2 DependencyLength 8\n",
                                 })));

INSTANTIATE_TEST_SUITE_P(
    SelectionControlMasks, RoundTripInstructionsTest,
    Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                              SPV_ENV_UNIVERSAL_1_3, SPV_ENV_UNIVERSAL_1_2),
            ::testing::ValuesIn(std::vector<std::string>{
                "OpSelectionMerge %1 None\n",
                "OpSelectionMerge %1 Flatten\n",
                "OpSelectionMerge %1 DontFlatten\n",
                "OpSelectionMerge %1 Flatten|DontFlatten\n",
            })));

INSTANTIATE_TEST_SUITE_P(
    FunctionControlMasks, RoundTripInstructionsTest,
    Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                              SPV_ENV_UNIVERSAL_1_2, SPV_ENV_UNIVERSAL_1_3),
            ::testing::ValuesIn(std::vector<std::string>{
                "%2 = OpFunction %1 None %3\n",
                "%2 = OpFunction %1 Inline %3\n",
                "%2 = OpFunction %1 DontInline %3\n",
                "%2 = OpFunction %1 Pure %3\n",
                "%2 = OpFunction %1 Const %3\n",
                "%2 = OpFunction %1 Inline|Pure|Const %3\n",
                "%2 = OpFunction %1 DontInline|Const %3\n",
            })));

INSTANTIATE_TEST_SUITE_P(
    ImageMasks, RoundTripInstructionsTest,
    Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_0, SPV_ENV_UNIVERSAL_1_1,
                              SPV_ENV_UNIVERSAL_1_2, SPV_ENV_UNIVERSAL_1_3),
            ::testing::ValuesIn(std::vector<std::string>{
                "%2 = OpImageFetch %1 %3 %4\n",
                "%2 = OpImageFetch %1 %3 %4 None\n",
                "%2 = OpImageFetch %1 %3 %4 Bias %5\n",
                "%2 = OpImageFetch %1 %3 %4 Lod %5\n",
                "%2 = OpImageFetch %1 %3 %4 Grad %5 %6\n",
                "%2 = OpImageFetch %1 %3 %4 ConstOffset %5\n",
                "%2 = OpImageFetch %1 %3 %4 Offset %5\n",
                "%2 = OpImageFetch %1 %3 %4 ConstOffsets %5\n",
                "%2 = OpImageFetch %1 %3 %4 Sample %5\n",
                "%2 = OpImageFetch %1 %3 %4 MinLod %5\n",
                "%2 = OpImageFetch %1 %3 %4 Bias|Lod|Grad %5 %6 %7 %8\n",
                "%2 = OpImageFetch %1 %3 %4 ConstOffset|Offset|ConstOffsets"
                " %5 %6 %7\n",
                "%2 = OpImageFetch %1 %3 %4 Sample|MinLod %5 %6\n",
                "%2 = OpImageFetch %1 %3 %4"
                " Bias|Lod|Grad|ConstOffset|Offset|ConstOffsets|Sample|MinLod"
                " %5 %6 %7 %8 %9 %10 %11 %12 %13\n"})));

INSTANTIATE_TEST_SUITE_P(
    NewInstructionsInSPIRV1_2, RoundTripInstructionsTest,
    Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_2, SPV_ENV_UNIVERSAL_1_3),
            ::testing::ValuesIn(std::vector<std::string>{
                "OpExecutionModeId %1 SubgroupsPerWorkgroupId %2\n",
                "OpExecutionModeId %1 LocalSizeId %2 %3 %4\n",
                "OpExecutionModeId %1 LocalSizeHintId %2 %3 %4\n",
                "OpDecorateId %1 AlignmentId %2\n",
                "OpDecorateId %1 MaxByteOffsetId %2\n",
            })));

INSTANTIATE_TEST_SUITE_P(
    CacheControlsINTEL, RoundTripInstructionsTest,
    Combine(
        ::testing::Values(SPV_ENV_UNIVERSAL_1_0),
        ::testing::ValuesIn(std::vector<std::string>{
            "OpDecorate %1 CacheControlLoadINTEL 0 UncachedINTEL\n",
            "OpDecorate %1 CacheControlLoadINTEL 1 CachedINTEL\n",
            "OpDecorate %1 CacheControlLoadINTEL 2 StreamingINTEL\n",
            "OpDecorate %1 CacheControlLoadINTEL 3 InvalidateAfterReadINTEL\n",
            "OpDecorate %1 CacheControlLoadINTEL 4 ConstCachedINTEL\n",
            "OpDecorate %1 CacheControlStoreINTEL 0 UncachedINTEL\n",
            "OpDecorate %1 CacheControlStoreINTEL 1 WriteThroughINTEL\n",
            "OpDecorate %1 CacheControlStoreINTEL 2 WriteBackINTEL\n",
            "OpDecorate %1 CacheControlStoreINTEL 3 StreamingINTEL\n",
        })));

INSTANTIATE_TEST_SUITE_P(
    HostAccessINTEL, RoundTripInstructionsTest,
    Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_0),
            ::testing::ValuesIn(std::vector<std::string>{
                "OpDecorate %1 HostAccessINTEL NoneINTEL \"none\"\n",
                "OpDecorate %1 HostAccessINTEL ReadINTEL \"read\"\n",
                "OpDecorate %1 HostAccessINTEL WriteINTEL \"write\"\n",
                "OpDecorate %1 HostAccessINTEL ReadWriteINTEL \"readwrite\"\n",
            })));

// clang-format off
INSTANTIATE_TEST_SUITE_P(
    MatrixMultiplyAccumulateOperands, RoundTripInstructionsTest,
    Combine(::testing::Values(SPV_ENV_UNIVERSAL_1_0),
            ::testing::ValuesIn(std::vector<std::string>{
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 None\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixASignedComponentsINTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixBSignedComponentsINTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixCBFloat16INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixResultBFloat16INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixAPackedInt8INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixBPackedInt8INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixAPackedInt4INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixBPackedInt4INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixATF32INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixBTF32INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixCBFloat16INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixAPackedFloat16INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixBPackedFloat16INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixAPackedBFloat16INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 MatrixBPackedBFloat16INTEL\n",
                "%2 = OpSubgroupMatrixMultiplyAccumulateINTEL %1 %3 %4 %5 %6 "
                "MatrixASignedComponentsINTEL|MatrixBSignedComponentsINTEL|MatrixAPackedInt8INTEL|MatrixBPackedInt8INTEL\n",
            })));
// clang-format on

using MaskSorting = TextToBinaryTest;

TEST_F(MaskSorting, MasksAreSortedFromLSBToMSB) {
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  "OpStore %1 %2 Nontemporal|Aligned|Volatile 32"),
              Eq("OpStore %1 %2 Volatile|Aligned|Nontemporal 32\n"));
  EXPECT_THAT(
      EncodeAndDecodeSuccessfully(
          "OpDecorate %1 FPFastMathMode NotInf|Fast|AllowRecip|NotNaN|NSZ"),
      Eq("OpDecorate %1 FPFastMathMode NotNaN|NotInf|NSZ|AllowRecip|Fast\n"));
  EXPECT_THAT(
      EncodeAndDecodeSuccessfully("OpLoopMerge %1 %2 DontUnroll|Unroll"),
      Eq("OpLoopMerge %1 %2 Unroll|DontUnroll\n"));
  EXPECT_THAT(
      EncodeAndDecodeSuccessfully("OpSelectionMerge %1 DontFlatten|Flatten"),
      Eq("OpSelectionMerge %1 Flatten|DontFlatten\n"));
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  "%2 = OpFunction %1 DontInline|Const|Pure|Inline %3"),
              Eq("%2 = OpFunction %1 Inline|DontInline|Pure|Const %3\n"));
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  "%2 = OpImageFetch %1 %3 %4"
                  " MinLod|Sample|Offset|Lod|Grad|ConstOffsets|ConstOffset|Bias"
                  " %5 %6 %7 %8 %9 %10 %11 %12 %13\n"),
              Eq("%2 = OpImageFetch %1 %3 %4"
                 " Bias|Lod|Grad|ConstOffset|Offset|ConstOffsets|Sample|MinLod"
                 " %5 %6 %7 %8 %9 %10 %11 %12 %13\n"));
}

using OperandTypeTest = TextToBinaryTest;

TEST_F(OperandTypeTest, OptionalTypedLiteralNumber) {
  const std::string input =
      "%1 = OpTypeInt 32 0\n"
      "%2 = OpConstant %1 42\n"
      "OpSwitch %2 %3 100 %4\n";
  EXPECT_EQ(input, EncodeAndDecodeSuccessfully(input));
}

using IndentTest = spvtest::TextToBinaryTest;

TEST_F(IndentTest, Sample) {
  const std::string input = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
%1 = OpTypeInt 32 0
%2 = OpTypeStruct %1 %3 %4 %5 %6 %7 %8 %9 %10 ; force IDs into double digits
%11 = OpConstant %1 42
OpStore %2 %3 Aligned|Volatile 4 ; bogus, but not indented
)";
  const std::string expected =
      R"(               OpCapability Shader
               OpMemoryModel Logical GLSL450
          %1 = OpTypeInt 32 0
          %2 = OpTypeStruct %1 %3 %4 %5 %6 %7 %8 %9 %10
         %11 = OpConstant %1 42
               OpStore %2 %3 Volatile|Aligned 4
)";
  EXPECT_THAT(
      EncodeAndDecodeSuccessfully(input, SPV_BINARY_TO_TEXT_OPTION_INDENT),
      expected);
}

TEST_F(IndentTest, NestedIf) {
  const std::string input = R"(
OpCapability Shader
OpMemoryModel Logical Simple
OpEntryPoint Fragment %100 "main"
OpExecutionMode %100 OriginUpperLeft
OpName %var "var"
%void = OpTypeVoid
%3 = OpTypeFunction %void
%bool = OpTypeBool
%5 = OpConstantNull %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%uint = OpTypeInt 32 0
%int = OpTypeInt 32 1
%uint_42 = OpConstant %uint 42
%int_42 = OpConstant %int 42
%13 = OpTypeFunction %uint
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%uint_4 = OpConstant %uint 4
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_10 = OpConstant %uint 10
%uint_20 = OpConstant %uint 20
%uint_30 = OpConstant %uint 30
%uint_40 = OpConstant %uint 40
%uint_50 = OpConstant %uint 50
%uint_90 = OpConstant %uint 90
%uint_99 = OpConstant %uint 99
%_ptr_Private_uint = OpTypePointer Private %uint
%var = OpVariable %_ptr_Private_uint Private
%uint_999 = OpConstant %uint 999
%100 = OpFunction %void None %3
%10 = OpLabel
OpStore %var %uint_0
OpSelectionMerge %99 None
OpBranchConditional %5 %30 %40
%30 = OpLabel
OpStore %var %uint_1
OpBranch %99
%40 = OpLabel
OpStore %var %uint_2
OpBranch %99
%99 = OpLabel
OpStore %var %uint_999
OpReturn
OpFunctionEnd
)";
  const std::string expected =
      R"(               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %100 "main"
               OpExecutionMode %100 OriginUpperLeft
               OpName %1 "var"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeBool
          %5 = OpConstantNull %4
          %6 = OpConstantTrue %4
          %7 = OpConstantFalse %4
          %8 = OpTypeInt 32 0
          %9 = OpTypeInt 32 1
         %11 = OpConstant %8 42
         %12 = OpConstant %9 42
         %13 = OpTypeFunction %8
         %14 = OpConstant %8 0
         %15 = OpConstant %8 1
         %16 = OpConstant %8 2
         %17 = OpConstant %8 3
         %18 = OpConstant %8 4
         %19 = OpConstant %8 5
         %20 = OpConstant %8 6
         %21 = OpConstant %8 7
         %22 = OpConstant %8 8
         %23 = OpConstant %8 10
         %24 = OpConstant %8 20
         %25 = OpConstant %8 30
         %26 = OpConstant %8 40
         %27 = OpConstant %8 50
         %28 = OpConstant %8 90
         %29 = OpConstant %8 99
         %31 = OpTypePointer Private %8
          %1 = OpVariable %31 Private
         %32 = OpConstant %8 999
        %100 = OpFunction %2 None %3

         %10 = OpLabel
                 OpStore %1 %14
                 OpSelectionMerge %99 None
                 OpBranchConditional %5 %30 %40

         %30 =     OpLabel
                     OpStore %1 %15
                     OpBranch %99

         %40 =     OpLabel
                     OpStore %1 %16
                     OpBranch %99

         %99 = OpLabel
                 OpStore %1 %32
                 OpReturn
               OpFunctionEnd
)";
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  input,
                  SPV_BINARY_TO_TEXT_OPTION_INDENT |
                      SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS),
              expected);
}

TEST_F(IndentTest, NestedWhile) {
  const std::string input = R"(
OpCapability Shader
OpMemoryModel Logical Simple
OpEntryPoint Fragment %100 "main"
OpExecutionMode %100 OriginUpperLeft
OpName %var "var"
%void = OpTypeVoid
%3 = OpTypeFunction %void
%bool = OpTypeBool
%5 = OpConstantNull %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%uint = OpTypeInt 32 0
%int = OpTypeInt 32 1
%uint_42 = OpConstant %uint 42
%int_42 = OpConstant %int 42
%13 = OpTypeFunction %uint
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%uint_4 = OpConstant %uint 4
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_10 = OpConstant %uint 10
%uint_20 = OpConstant %uint 20
%uint_30 = OpConstant %uint 30
%uint_40 = OpConstant %uint 40
%uint_50 = OpConstant %uint 50
%uint_90 = OpConstant %uint 90
%uint_99 = OpConstant %uint 99
%_ptr_Private_uint = OpTypePointer Private %uint
%var = OpVariable %_ptr_Private_uint Private
%uint_999 = OpConstant %uint 999
%100 = OpFunction %void None %3
%10 = OpLabel
OpStore %var %uint_0
OpBranch %20
%20 = OpLabel
OpStore %var %uint_1
OpLoopMerge %99 %20 None
OpBranch %80
%80 = OpLabel
OpStore %var %uint_2
OpBranchConditional %5 %99 %20
%99 = OpLabel
OpStore %var %uint_3
OpReturn
OpFunctionEnd
)";
  const std::string expected =
      R"(               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %100 "main"
               OpExecutionMode %100 OriginUpperLeft
               OpName %1 "var"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeBool
          %5 = OpConstantNull %4
          %6 = OpConstantTrue %4
          %7 = OpConstantFalse %4
          %8 = OpTypeInt 32 0
          %9 = OpTypeInt 32 1
         %11 = OpConstant %8 42
         %12 = OpConstant %9 42
         %13 = OpTypeFunction %8
         %14 = OpConstant %8 0
         %15 = OpConstant %8 1
         %16 = OpConstant %8 2
         %17 = OpConstant %8 3
         %18 = OpConstant %8 4
         %19 = OpConstant %8 5
         %21 = OpConstant %8 6
         %22 = OpConstant %8 7
         %23 = OpConstant %8 8
         %24 = OpConstant %8 10
         %25 = OpConstant %8 20
         %26 = OpConstant %8 30
         %27 = OpConstant %8 40
         %28 = OpConstant %8 50
         %29 = OpConstant %8 90
         %30 = OpConstant %8 99
         %31 = OpTypePointer Private %8
          %1 = OpVariable %31 Private
         %32 = OpConstant %8 999
        %100 = OpFunction %2 None %3

         %10 = OpLabel
                 OpStore %1 %14
                 OpBranch %20

         %20 = OpLabel
                 OpStore %1 %15
                 OpLoopMerge %99 %20 None
                 OpBranch %80

         %80 =     OpLabel
                     OpStore %1 %16
                     OpBranchConditional %5 %99 %20

         %99 = OpLabel
                 OpStore %1 %17
                 OpReturn
               OpFunctionEnd
)";
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  input,
                  SPV_BINARY_TO_TEXT_OPTION_INDENT |
                      SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS),
              expected);
}

TEST_F(IndentTest, NestedLoopInLoop) {
  const std::string input = R"(
OpCapability Shader
OpMemoryModel Logical Simple
OpEntryPoint Fragment %100 "main"
OpExecutionMode %100 OriginUpperLeft
OpName %var "var"
%void = OpTypeVoid
%3 = OpTypeFunction %void
%bool = OpTypeBool
%5 = OpConstantNull %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%uint = OpTypeInt 32 0
%int = OpTypeInt 32 1
%uint_42 = OpConstant %uint 42
%int_42 = OpConstant %int 42
%13 = OpTypeFunction %uint
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%uint_4 = OpConstant %uint 4
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_10 = OpConstant %uint 10
%uint_20 = OpConstant %uint 20
%uint_30 = OpConstant %uint 30
%uint_40 = OpConstant %uint 40
%uint_50 = OpConstant %uint 50
%uint_90 = OpConstant %uint 90
%uint_99 = OpConstant %uint 99
%_ptr_Private_uint = OpTypePointer Private %uint
%var = OpVariable %_ptr_Private_uint Private
%uint_999 = OpConstant %uint 999
%100 = OpFunction %void None %3
%10 = OpLabel
OpBranch %20
%20 = OpLabel
OpLoopMerge %99 %50 None
OpBranchConditional %5 %30 %99
%30 = OpLabel
OpLoopMerge %49 %40 None
OpBranchConditional %true %35 %49
%35 = OpLabel
OpBranch %37
%37 = OpLabel
OpBranch %40
%40 = OpLabel
OpBranch %30
%49 = OpLabel
OpBranch %50
%50 = OpLabel
OpBranch %20
%99 = OpLabel
OpReturn
OpFunctionEnd
)";
  const std::string expected =
      R"(               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %100 "main"
               OpExecutionMode %100 OriginUpperLeft
               OpName %1 "var"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeBool
          %5 = OpConstantNull %4
          %6 = OpConstantTrue %4
          %7 = OpConstantFalse %4
          %8 = OpTypeInt 32 0
          %9 = OpTypeInt 32 1
         %11 = OpConstant %8 42
         %12 = OpConstant %9 42
         %13 = OpTypeFunction %8
         %14 = OpConstant %8 0
         %15 = OpConstant %8 1
         %16 = OpConstant %8 2
         %17 = OpConstant %8 3
         %18 = OpConstant %8 4
         %19 = OpConstant %8 5
         %21 = OpConstant %8 6
         %22 = OpConstant %8 7
         %23 = OpConstant %8 8
         %24 = OpConstant %8 10
         %25 = OpConstant %8 20
         %26 = OpConstant %8 30
         %27 = OpConstant %8 40
         %28 = OpConstant %8 50
         %29 = OpConstant %8 90
         %31 = OpConstant %8 99
         %32 = OpTypePointer Private %8
          %1 = OpVariable %32 Private
         %33 = OpConstant %8 999
        %100 = OpFunction %2 None %3

         %10 = OpLabel
                 OpBranch %20

         %20 = OpLabel
                 OpLoopMerge %99 %50 None
                 OpBranchConditional %5 %30 %99

         %30 =     OpLabel
                     OpLoopMerge %49 %40 None
                     OpBranchConditional %6 %35 %49

         %35 =         OpLabel
                         OpBranch %37

         %37 =         OpLabel
                         OpBranch %40

         %40 =       OpLabel
                       OpBranch %30

         %49 =     OpLabel
                     OpBranch %50

         %50 =   OpLabel
                   OpBranch %20

         %99 = OpLabel
                 OpReturn
               OpFunctionEnd
)";
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  input,
                  SPV_BINARY_TO_TEXT_OPTION_INDENT |
                      SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS),
              expected);
}

TEST_F(IndentTest, NestedSwitch) {
  const std::string input = R"(
OpCapability Shader
OpMemoryModel Logical Simple
OpEntryPoint Fragment %100 "main"
OpExecutionMode %100 OriginUpperLeft
OpName %var "var"
%void = OpTypeVoid
%3 = OpTypeFunction %void
%bool = OpTypeBool
%5 = OpConstantNull %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%uint = OpTypeInt 32 0
%int = OpTypeInt 32 1
%uint_42 = OpConstant %uint 42
%int_42 = OpConstant %int 42
%13 = OpTypeFunction %uint
%uint_0 = OpConstant %uint 0
%uint_1 = OpConstant %uint 1
%uint_2 = OpConstant %uint 2
%uint_3 = OpConstant %uint 3
%uint_4 = OpConstant %uint 4
%uint_5 = OpConstant %uint 5
%uint_6 = OpConstant %uint 6
%uint_7 = OpConstant %uint 7
%uint_8 = OpConstant %uint 8
%uint_10 = OpConstant %uint 10
%uint_20 = OpConstant %uint 20
%uint_30 = OpConstant %uint 30
%uint_40 = OpConstant %uint 40
%uint_50 = OpConstant %uint 50
%uint_90 = OpConstant %uint 90
%uint_99 = OpConstant %uint 99
%_ptr_Private_uint = OpTypePointer Private %uint
%var = OpVariable %_ptr_Private_uint Private
%uint_999 = OpConstant %uint 999
%100 = OpFunction %void None %3
%10 = OpLabel
OpSelectionMerge %99 None
OpSwitch %uint_42 %80 20 %20 30 %30
%20 = OpLabel
OpBranch %80
%80 = OpLabel
OpBranch %30
%30 = OpLabel
OpBranch %99
%99 = OpLabel
OpReturn
OpFunctionEnd
)";
  const std::string expected =
      R"(               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %100 "main"
               OpExecutionMode %100 OriginUpperLeft
               OpName %1 "var"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeBool
          %5 = OpConstantNull %4
          %6 = OpConstantTrue %4
          %7 = OpConstantFalse %4
          %8 = OpTypeInt 32 0
          %9 = OpTypeInt 32 1
         %11 = OpConstant %8 42
         %12 = OpConstant %9 42
         %13 = OpTypeFunction %8
         %14 = OpConstant %8 0
         %15 = OpConstant %8 1
         %16 = OpConstant %8 2
         %17 = OpConstant %8 3
         %18 = OpConstant %8 4
         %19 = OpConstant %8 5
         %21 = OpConstant %8 6
         %22 = OpConstant %8 7
         %23 = OpConstant %8 8
         %24 = OpConstant %8 10
         %25 = OpConstant %8 20
         %26 = OpConstant %8 30
         %27 = OpConstant %8 40
         %28 = OpConstant %8 50
         %29 = OpConstant %8 90
         %31 = OpConstant %8 99
         %32 = OpTypePointer Private %8
          %1 = OpVariable %32 Private
         %33 = OpConstant %8 999
        %100 = OpFunction %2 None %3

         %10 = OpLabel
                 OpSelectionMerge %99 None
                 OpSwitch %11 %80 20 %20 30 %30

         %20 =     OpLabel
                     OpBranch %80

         %80 =     OpLabel
                     OpBranch %30

         %30 =     OpLabel
                     OpBranch %99

         %99 = OpLabel
                 OpReturn
               OpFunctionEnd
)";
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  input,
                  SPV_BINARY_TO_TEXT_OPTION_INDENT |
                      SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS),
              expected);
}

TEST_F(IndentTest, ReorderedIf) {
  const std::string input = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %100 "main"
               OpExecutionMode %100 OriginUpperLeft
               OpName %1 "var"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeBool
          %5 = OpConstantNull %4
          %6 = OpConstantTrue %4
          %7 = OpConstantFalse %4
          %8 = OpTypeInt 32 0
          %9 = OpTypeInt 32 1
         %11 = OpConstant %8 42
         %12 = OpConstant %9 42
         %13 = OpTypeFunction %8
         %14 = OpConstant %8 0
         %15 = OpConstant %8 1
         %16 = OpConstant %8 2
         %17 = OpConstant %8 3
         %18 = OpConstant %8 4
         %19 = OpConstant %8 5
         %21 = OpConstant %8 6
         %22 = OpConstant %8 7
         %23 = OpConstant %8 8
         %24 = OpConstant %8 10
         %25 = OpConstant %8 20
         %26 = OpConstant %8 30
         %27 = OpConstant %8 40
         %28 = OpConstant %8 50
         %29 = OpConstant %8 90
         %31 = OpConstant %8 99
         %32 = OpTypePointer Private %8
          %1 = OpVariable %32 Private
         %33 = OpConstant %8 999
        %100 = OpFunction %2 None %3
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpBranchConditional %5 %20 %50
         %99 = OpLabel
               OpReturn
         %20 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %5 %30 %40
         %49 = OpLabel
               OpBranch %99
         %40 = OpLabel
               OpBranch %49
         %30 = OpLabel
               OpBranch %49
         %50 = OpLabel
               OpSelectionMerge %79 None
               OpBranchConditional %5 %60 %70
         %79 = OpLabel
               OpBranch %99
         %60 = OpLabel
               OpBranch %79
         %70 = OpLabel
               OpBranch %79
               OpFunctionEnd
)";
  const std::string expected =
      R"(               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %100 "main"
               OpExecutionMode %100 OriginUpperLeft
               OpName %1 "var"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeBool
          %5 = OpConstantNull %4
          %6 = OpConstantTrue %4
          %7 = OpConstantFalse %4
          %8 = OpTypeInt 32 0
          %9 = OpTypeInt 32 1
         %11 = OpConstant %8 42
         %12 = OpConstant %9 42
         %13 = OpTypeFunction %8
         %14 = OpConstant %8 0
         %15 = OpConstant %8 1
         %16 = OpConstant %8 2
         %17 = OpConstant %8 3
         %18 = OpConstant %8 4
         %19 = OpConstant %8 5
         %21 = OpConstant %8 6
         %22 = OpConstant %8 7
         %23 = OpConstant %8 8
         %24 = OpConstant %8 10
         %25 = OpConstant %8 20
         %26 = OpConstant %8 30
         %27 = OpConstant %8 40
         %28 = OpConstant %8 50
         %29 = OpConstant %8 90
         %31 = OpConstant %8 99
         %32 = OpTypePointer Private %8
          %1 = OpVariable %32 Private
         %33 = OpConstant %8 999
        %100 = OpFunction %2 None %3
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpBranchConditional %5 %20 %50
         %20 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %5 %30 %40
         %30 = OpLabel
               OpBranch %49
         %40 = OpLabel
               OpBranch %49
         %49 = OpLabel
               OpBranch %99
         %50 = OpLabel
               OpSelectionMerge %79 None
               OpBranchConditional %5 %60 %70
         %60 = OpLabel
               OpBranch %79
         %70 = OpLabel
               OpBranch %79
         %79 = OpLabel
               OpBranch %99
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)";
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  input,
                  SPV_BINARY_TO_TEXT_OPTION_INDENT |
                      SPV_BINARY_TO_TEXT_OPTION_REORDER_BLOCKS,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS),
              expected);
}

TEST_F(IndentTest, ReorderedFallThroughInSwitch) {
  const std::string input = R"(
               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %100 "main"
               OpExecutionMode %100 OriginUpperLeft
               OpName %1 "var"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeBool
          %5 = OpConstantNull %4
          %6 = OpConstantTrue %4
          %7 = OpConstantFalse %4
          %8 = OpTypeInt 32 0
          %9 = OpTypeInt 32 1
         %11 = OpConstant %8 42
         %12 = OpConstant %9 42
         %13 = OpTypeFunction %8
         %14 = OpConstant %8 0
         %15 = OpConstant %8 1
         %16 = OpConstant %8 2
         %17 = OpConstant %8 3
         %18 = OpConstant %8 4
         %19 = OpConstant %8 5
         %21 = OpConstant %8 6
         %22 = OpConstant %8 7
         %23 = OpConstant %8 8
         %24 = OpConstant %8 10
         %25 = OpConstant %8 20
         %26 = OpConstant %8 30
         %27 = OpConstant %8 40
         %28 = OpConstant %8 50
         %29 = OpConstant %8 90
         %31 = OpConstant %8 99
         %32 = OpTypePointer Private %8
          %1 = OpVariable %32 Private
         %33 = OpConstant %8 999
        %100 = OpFunction %2 None %3
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %11 %50 20 %20 50 %50
         %99 = OpLabel
               OpReturn
         %20 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %5 %30 %40
         %49 = OpLabel
               OpBranchConditional %5 %99 %50
         %30 = OpLabel
               OpBranch %49
         %40 = OpLabel
               OpBranch %49
         %50 = OpLabel
               OpSelectionMerge %79 None
               OpBranchConditional %5 %60 %70
         %79 = OpLabel
               OpBranch %99
         %60 = OpLabel
               OpBranch %79
         %70 = OpLabel
               OpBranch %79
               OpFunctionEnd
)";
  const std::string expected =
      R"(               OpCapability Shader
               OpMemoryModel Logical Simple
               OpEntryPoint Fragment %100 "main"
               OpExecutionMode %100 OriginUpperLeft
               OpName %1 "var"
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %4 = OpTypeBool
          %5 = OpConstantNull %4
          %6 = OpConstantTrue %4
          %7 = OpConstantFalse %4
          %8 = OpTypeInt 32 0
          %9 = OpTypeInt 32 1
         %11 = OpConstant %8 42
         %12 = OpConstant %9 42
         %13 = OpTypeFunction %8
         %14 = OpConstant %8 0
         %15 = OpConstant %8 1
         %16 = OpConstant %8 2
         %17 = OpConstant %8 3
         %18 = OpConstant %8 4
         %19 = OpConstant %8 5
         %21 = OpConstant %8 6
         %22 = OpConstant %8 7
         %23 = OpConstant %8 8
         %24 = OpConstant %8 10
         %25 = OpConstant %8 20
         %26 = OpConstant %8 30
         %27 = OpConstant %8 40
         %28 = OpConstant %8 50
         %29 = OpConstant %8 90
         %31 = OpConstant %8 99
         %32 = OpTypePointer Private %8
          %1 = OpVariable %32 Private
         %33 = OpConstant %8 999
        %100 = OpFunction %2 None %3
         %10 = OpLabel
               OpSelectionMerge %99 None
               OpSwitch %11 %50 20 %20 50 %50
         %20 = OpLabel
               OpSelectionMerge %49 None
               OpBranchConditional %5 %30 %40
         %30 = OpLabel
               OpBranch %49
         %40 = OpLabel
               OpBranch %49
         %49 = OpLabel
               OpBranchConditional %5 %99 %50
         %50 = OpLabel
               OpSelectionMerge %79 None
               OpBranchConditional %5 %60 %70
         %60 = OpLabel
               OpBranch %79
         %70 = OpLabel
               OpBranch %79
         %79 = OpLabel
               OpBranch %99
         %99 = OpLabel
               OpReturn
               OpFunctionEnd
)";
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  input,
                  SPV_BINARY_TO_TEXT_OPTION_INDENT |
                      SPV_BINARY_TO_TEXT_OPTION_REORDER_BLOCKS,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS),
              expected);
}

TEST_F(IndentTest, ReorderedNested) {
  const std::string input = R"(
OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %4 "main" %204
OpExecutionMode %4 OriginUpperLeft
OpSource GLSL 450
OpName %4 "main"
OpName %16 "ff(vf2;f1;"
OpName %14 "g"
OpName %15 "f"
OpName %19 "vg"
OpName %20 "Block140"
OpMemberName %20 0 "a"
OpMemberName %20 1 "b"
OpName %22 "b140"
OpName %35 "sv"
OpName %39 "s"
OpName %46 "f"
OpName %51 "g"
OpName %57 "x"
OpName %69 "param"
OpName %75 "i"
OpName %80 "vc"
OpName %88 "j"
OpName %95 "size"
OpName %174 "v"
OpName %187 "i"
OpName %204 "o_color"
OpMemberDecorate %20 0 Offset 0
OpMemberDecorate %20 1 Offset 16
OpDecorate %20 Block
OpDecorate %22 DescriptorSet 1
OpDecorate %22 Binding 0
OpDecorate %39 DescriptorSet 0
OpDecorate %39 Binding 1
OpDecorate %95 SpecId 20
OpDecorate %204 Location 2
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpTypeVector %6 2
%8 = OpTypePointer Function %7
%9 = OpTypeVector %6 4
%10 = OpTypeInt 32 0
%11 = OpConstant %10 2
%12 = OpTypeArray %9 %11
%13 = OpTypeFunction %12 %8 %6
%18 = OpTypePointer Private %9
%19 = OpVariable %18 Private
%20 = OpTypeStruct %6 %9
%21 = OpTypePointer Uniform %20
%22 = OpVariable %21 Uniform
%23 = OpTypeInt 32 1
%24 = OpConstant %23 1
%25 = OpTypePointer Uniform %9
%28 = OpConstant %6 0
%29 = OpConstantComposite %9 %28 %28 %28 %28
%34 = OpTypePointer Function %9
%36 = OpTypeImage %6 2D 0 0 0 1 Unknown
%37 = OpTypeSampledImage %36
%38 = OpTypePointer UniformConstant %37
%39 = OpVariable %38 UniformConstant
%41 = OpConstantComposite %7 %28 %28
%45 = OpTypePointer Function %6
%47 = OpConstant %23 0
%48 = OpTypePointer Uniform %6
%53 = OpConstant %6 1
%55 = OpTypeBool
%56 = OpTypePointer Function %55
%58 = OpConstant %10 0
%59 = OpTypePointer Private %6
%74 = OpTypePointer Function %23
%87 = OpTypePointer Function %10
%95 = OpSpecConstant %10 2
%100 = OpConstant %10 1
%109 = OpConstantComposite %9 %53 %53 %53 %53
%127 = OpConstant %23 10
%139 = OpConstant %6 2
%143 = OpConstant %6 3
%158 = OpConstant %6 4
%177 = OpConstant %6 0.5
%195 = OpConstant %23 100
%202 = OpTypeVector %10 4
%203 = OpTypePointer Output %202
%204 = OpVariable %203 Output
%4 = OpFunction %2 None %3
%5 = OpLabel
%35 = OpVariable %34 Function
%46 = OpVariable %45 Function
%51 = OpVariable %45 Function
%57 = OpVariable %56 Function
%69 = OpVariable %8 Function
%75 = OpVariable %74 Function
%80 = OpVariable %34 Function
%88 = OpVariable %87 Function
%174 = OpVariable %45 Function
%187 = OpVariable %74 Function
%26 = OpAccessChain %25 %22 %24
%27 = OpLoad %9 %26
OpStore %19 %27
%40 = OpLoad %37 %39
%42 = OpImageSampleImplicitLod %9 %40 %41
%43 = OpLoad %9 %19
%44 = OpFAdd %9 %42 %43
OpStore %35 %44
%49 = OpAccessChain %48 %22 %47
%50 = OpLoad %6 %49
OpStore %46 %50
%52 = OpLoad %6 %46
%54 = OpFAdd %6 %52 %53
OpStore %51 %54
%60 = OpAccessChain %59 %19 %58
%61 = OpLoad %6 %60
%62 = OpFOrdGreaterThan %55 %61 %28
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%64 = OpLabel
%73 = OpPhi %55 %62 %5 %72 %63
OpStore %57 %73
OpStore %75 %47
OpBranch %76
%197 = OpLabel
OpBranch %190
%63 = OpLabel
%65 = OpLoad %6 %46
%66 = OpLoad %6 %51
%67 = OpCompositeConstruct %7 %65 %66
%68 = OpLoad %6 %51
OpStore %69 %67
%70 = OpFunctionCall %12 %16 %69 %68
%71 = OpCompositeExtract %6 %70 0 0
%72 = OpFOrdGreaterThan %55 %71 %28
OpBranch %64
%77 = OpLabel
%81 = OpLoad %9 %19
OpStore %80 %81
%82 = OpAccessChain %45 %80 %58
%83 = OpLoad %6 %82
%84 = OpFOrdGreaterThan %55 %83 %28
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %113
%85 = OpLabel
OpStore %88 %58
OpBranch %89
%89 = OpLabel
OpLoopMerge %91 %92 None
OpBranch %93
%93 = OpLabel
%94 = OpLoad %10 %88
%96 = OpULessThan %55 %94 %95
OpBranchConditional %96 %90 %91
%105 = OpLabel
OpBranch %92
%198 = OpLabel
OpBranch %191
%163 = OpLabel
OpBranch %136
%104 = OpLabel
OpBranch %91
%76 = OpLabel
OpLoopMerge %78 %79 None
OpBranch %77
%92 = OpLabel
%107 = OpLoad %10 %88
%108 = OpIAdd %10 %107 %24
OpStore %88 %108
OpBranch %89
%91 = OpLabel
%110 = OpLoad %9 %80
%111 = OpFAdd %9 %110 %109
OpStore %80 %111
OpBranch %79
%113 = OpLabel
%114 = OpLoad %9 %80
%115 = OpFSub %9 %114 %109
OpStore %80 %115
OpBranch %86
%132 = OpLabel
%137 = OpLoad %6 %51
%138 = OpFAdd %6 %137 %53
OpStore %51 %138
OpBranch %133
%86 = OpLabel
%116 = OpAccessChain %45 %80 %100
%117 = OpLoad %6 %116
%118 = OpFOrdGreaterThan %55 %117 %28
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
OpBranch %78
%120 = OpLabel
%122 = OpAccessChain %45 %80 %11
%123 = OpLoad %6 %122
%124 = OpFAdd %6 %123 %53
%125 = OpAccessChain %45 %80 %11
OpStore %125 %124
OpBranch %79
%79 = OpLabel
%126 = OpLoad %23 %75
%128 = OpSLessThan %55 %126 %127
OpBranchConditional %128 %76 %78
%78 = OpLabel
%129 = OpAccessChain %48 %22 %47
%130 = OpLoad %6 %129
%131 = OpConvertFToS %23 %130
OpSelectionMerge %136 None
OpSwitch %131 %135 0 %132 1 %132 2 %132 3 %133 4 %134
%90 = OpLabel
%97 = OpLoad %9 %19
%98 = OpLoad %9 %80
%99 = OpFAdd %9 %98 %97
OpStore %80 %99
%101 = OpAccessChain %45 %80 %100
%102 = OpLoad %6 %101
%103 = OpFOrdLessThan %55 %102 %28
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%161 = OpLabel
OpLoopMerge %163 %164 None
OpBranch %165
%165 = OpLabel
%166 = OpLoad %6 %51
%167 = OpFOrdLessThan %55 %166 %139
OpBranchConditional %167 %162 %163
%164 = OpLabel
OpBranch %161
%162 = OpLabel
%168 = OpLoad %6 %46
%169 = OpFOrdLessThan %55 %168 %53
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%135 = OpLabel
%159 = OpLoad %6 %51
%160 = OpFAdd %6 %159 %158
OpStore %51 %160
OpBranch %161
%133 = OpLabel
%140 = OpLoad %6 %51
%141 = OpFAdd %6 %140 %139
OpStore %51 %141
OpBranch %136
%134 = OpLabel
%144 = OpLoad %6 %51
%145 = OpFAdd %6 %144 %143
OpStore %51 %145
OpBranch %146
%146 = OpLabel
OpLoopMerge %148 %149 None
OpBranch %150
%150 = OpLabel
%151 = OpLoad %6 %51
%152 = OpFOrdLessThan %55 %151 %139
OpBranchConditional %152 %147 %148
%147 = OpLabel
%153 = OpLoad %6 %46
%154 = OpFOrdLessThan %55 %153 %53
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
OpBranch %148
%156 = OpLabel
OpBranch %149
%149 = OpLabel
OpBranch %146
%148 = OpLabel
OpBranch %135
%136 = OpLabel
OpStore %174 %53
%175 = OpAccessChain %45 %35 %58
%176 = OpLoad %6 %175
%178 = OpFOrdLessThanEqual %55 %176 %177
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %181
%179 = OpLabel
OpStore %174 %28
OpBranch %180
%185 = OpLabel
OpStore %174 %139
OpBranch %186
%181 = OpLabel
%182 = OpAccessChain %45 %35 %58
%183 = OpLoad %6 %182
%184 = OpFOrdGreaterThanEqual %55 %183 %177
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%170 = OpLabel
OpBranch %163
%171 = OpLabel
OpBranch %164
%186 = OpLabel
OpBranch %180
%188 = OpLabel
OpLoopMerge %190 %191 None
OpBranch %189
%189 = OpLabel
%192 = OpLoad %9 %19
%193 = OpFAdd %9 %192 %109
OpStore %19 %193
%194 = OpLoad %23 %187
%196 = OpSGreaterThan %55 %194 %195
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%180 = OpLabel
OpStore %187 %47
OpBranch %188
%191 = OpLabel
%200 = OpLoad %23 %187
%201 = OpIAdd %23 %200 %24
OpStore %187 %201
OpBranch %188
%190 = OpLabel
OpReturn
OpFunctionEnd
%16 = OpFunction %12 None %13
%14 = OpFunctionParameter %8
%15 = OpFunctionParameter %6
%17 = OpLabel
%30 = OpCompositeConstruct %9 %15 %15 %15 %15
%31 = OpCompositeConstruct %12 %29 %30
OpReturnValue %31
OpFunctionEnd
)";
  const std::string expected =
      R"(               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %204
               OpExecutionMode %4 OriginUpperLeft
               OpSource GLSL 450
               OpName %4 "main"
               OpName %16 "ff(vf2;f1;"
               OpName %14 "g"
               OpName %15 "f"
               OpName %19 "vg"
               OpName %20 "Block140"
               OpMemberName %20 0 "a"
               OpMemberName %20 1 "b"
               OpName %22 "b140"
               OpName %35 "sv"
               OpName %39 "s"
               OpName %46 "f"
               OpName %51 "g"
               OpName %57 "x"
               OpName %69 "param"
               OpName %75 "i"
               OpName %80 "vc"
               OpName %88 "j"
               OpName %95 "size"
               OpName %174 "v"
               OpName %187 "i"
               OpName %204 "o_color"
               OpMemberDecorate %20 0 Offset 0
               OpMemberDecorate %20 1 Offset 16
               OpDecorate %20 Block
               OpDecorate %22 DescriptorSet 1
               OpDecorate %22 Binding 0
               OpDecorate %39 DescriptorSet 0
               OpDecorate %39 Binding 1
               OpDecorate %95 SpecId 20
               OpDecorate %204 Location 2
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypeVector %6 2
          %8 = OpTypePointer Function %7
          %9 = OpTypeVector %6 4
         %10 = OpTypeInt 32 0
         %11 = OpConstant %10 2
         %12 = OpTypeArray %9 %11
         %13 = OpTypeFunction %12 %8 %6
         %18 = OpTypePointer Private %9
         %19 = OpVariable %18 Private
         %20 = OpTypeStruct %6 %9
         %21 = OpTypePointer Uniform %20
         %22 = OpVariable %21 Uniform
         %23 = OpTypeInt 32 1
         %24 = OpConstant %23 1
         %25 = OpTypePointer Uniform %9
         %28 = OpConstant %6 0
         %29 = OpConstantComposite %9 %28 %28 %28 %28
         %34 = OpTypePointer Function %9
         %36 = OpTypeImage %6 2D 0 0 0 1 Unknown
         %37 = OpTypeSampledImage %36
         %38 = OpTypePointer UniformConstant %37
         %39 = OpVariable %38 UniformConstant
         %41 = OpConstantComposite %7 %28 %28
         %45 = OpTypePointer Function %6
         %47 = OpConstant %23 0
         %48 = OpTypePointer Uniform %6
         %53 = OpConstant %6 1
         %55 = OpTypeBool
         %56 = OpTypePointer Function %55
         %58 = OpConstant %10 0
         %59 = OpTypePointer Private %6
         %74 = OpTypePointer Function %23
         %87 = OpTypePointer Function %10
         %95 = OpSpecConstant %10 2
        %100 = OpConstant %10 1
        %109 = OpConstantComposite %9 %53 %53 %53 %53
        %127 = OpConstant %23 10
        %139 = OpConstant %6 2
        %143 = OpConstant %6 3
        %158 = OpConstant %6 4
        %177 = OpConstant %6 0.5
        %195 = OpConstant %23 100
        %202 = OpTypeVector %10 4
        %203 = OpTypePointer Output %202
        %204 = OpVariable %203 Output
          %4 = OpFunction %2 None %3

          %5 = OpLabel
         %35 =   OpVariable %34 Function
         %46 =   OpVariable %45 Function
         %51 =   OpVariable %45 Function
         %57 =   OpVariable %56 Function
         %69 =   OpVariable %8 Function
         %75 =   OpVariable %74 Function
         %80 =   OpVariable %34 Function
         %88 =   OpVariable %87 Function
        %174 =   OpVariable %45 Function
        %187 =   OpVariable %74 Function
         %26 =   OpAccessChain %25 %22 %24
         %27 =   OpLoad %9 %26
                 OpStore %19 %27
         %40 =   OpLoad %37 %39
         %42 =   OpImageSampleImplicitLod %9 %40 %41
         %43 =   OpLoad %9 %19
         %44 =   OpFAdd %9 %42 %43
                 OpStore %35 %44
         %49 =   OpAccessChain %48 %22 %47
         %50 =   OpLoad %6 %49
                 OpStore %46 %50
         %52 =   OpLoad %6 %46
         %54 =   OpFAdd %6 %52 %53
                 OpStore %51 %54
         %60 =   OpAccessChain %59 %19 %58
         %61 =   OpLoad %6 %60
         %62 =   OpFOrdGreaterThan %55 %61 %28
                 OpSelectionMerge %64 None
                 OpBranchConditional %62 %63 %64

         %63 =     OpLabel
         %65 =       OpLoad %6 %46
         %66 =       OpLoad %6 %51
         %67 =       OpCompositeConstruct %7 %65 %66
         %68 =       OpLoad %6 %51
                     OpStore %69 %67
         %70 =       OpFunctionCall %12 %16 %69 %68
         %71 =       OpCompositeExtract %6 %70 0 0
         %72 =       OpFOrdGreaterThan %55 %71 %28
                     OpBranch %64

         %64 = OpLabel
         %73 =   OpPhi %55 %62 %5 %72 %63
                 OpStore %57 %73
                 OpStore %75 %47
                 OpBranch %76

         %76 = OpLabel
                 OpLoopMerge %78 %79 None
                 OpBranch %77

         %77 =     OpLabel
         %81 =       OpLoad %9 %19
                     OpStore %80 %81
         %82 =       OpAccessChain %45 %80 %58
         %83 =       OpLoad %6 %82
         %84 =       OpFOrdGreaterThan %55 %83 %28
                     OpSelectionMerge %86 None
                     OpBranchConditional %84 %85 %113

         %85 =         OpLabel
                         OpStore %88 %58
                         OpBranch %89

         %89 =         OpLabel
                         OpLoopMerge %91 %92 None
                         OpBranch %93

         %93 =             OpLabel
         %94 =               OpLoad %10 %88
         %96 =               OpULessThan %55 %94 %95
                             OpBranchConditional %96 %90 %91

         %90 =                 OpLabel
         %97 =                   OpLoad %9 %19
         %98 =                   OpLoad %9 %80
         %99 =                   OpFAdd %9 %98 %97
                                 OpStore %80 %99
        %101 =                   OpAccessChain %45 %80 %100
        %102 =                   OpLoad %6 %101
        %103 =                   OpFOrdLessThan %55 %102 %28
                                 OpSelectionMerge %105 None
                                 OpBranchConditional %103 %104 %105

        %104 =                     OpLabel
                                     OpBranch %91

        %105 =                 OpLabel
                                 OpBranch %92

         %92 =           OpLabel
        %107 =             OpLoad %10 %88
        %108 =             OpIAdd %10 %107 %24
                           OpStore %88 %108
                           OpBranch %89

         %91 =         OpLabel
        %110 =           OpLoad %9 %80
        %111 =           OpFAdd %9 %110 %109
                         OpStore %80 %111
                         OpBranch %79

        %113 =         OpLabel
        %114 =           OpLoad %9 %80
        %115 =           OpFSub %9 %114 %109
                         OpStore %80 %115
                         OpBranch %86

         %86 =     OpLabel
        %116 =       OpAccessChain %45 %80 %100
        %117 =       OpLoad %6 %116
        %118 =       OpFOrdGreaterThan %55 %117 %28
                     OpSelectionMerge %120 None
                     OpBranchConditional %118 %119 %120

        %119 =         OpLabel
                         OpBranch %78

        %120 =     OpLabel
        %122 =       OpAccessChain %45 %80 %11
        %123 =       OpLoad %6 %122
        %124 =       OpFAdd %6 %123 %53
        %125 =       OpAccessChain %45 %80 %11
                     OpStore %125 %124
                     OpBranch %79

         %79 =   OpLabel
        %126 =     OpLoad %23 %75
        %128 =     OpSLessThan %55 %126 %127
                   OpBranchConditional %128 %76 %78

         %78 = OpLabel
        %129 =   OpAccessChain %48 %22 %47
        %130 =   OpLoad %6 %129
        %131 =   OpConvertFToS %23 %130
                 OpSelectionMerge %136 None
                 OpSwitch %131 %135 0 %132 1 %132 2 %132 3 %133 4 %134

        %132 =     OpLabel
        %137 =       OpLoad %6 %51
        %138 =       OpFAdd %6 %137 %53
                     OpStore %51 %138
                     OpBranch %133

        %133 =     OpLabel
        %140 =       OpLoad %6 %51
        %141 =       OpFAdd %6 %140 %139
                     OpStore %51 %141
                     OpBranch %136

        %134 =     OpLabel
        %144 =       OpLoad %6 %51
        %145 =       OpFAdd %6 %144 %143
                     OpStore %51 %145
                     OpBranch %146

        %146 =     OpLabel
                     OpLoopMerge %148 %149 None
                     OpBranch %150

        %150 =         OpLabel
        %151 =           OpLoad %6 %51
        %152 =           OpFOrdLessThan %55 %151 %139
                         OpBranchConditional %152 %147 %148

        %147 =             OpLabel
        %153 =               OpLoad %6 %46
        %154 =               OpFOrdLessThan %55 %153 %53
                             OpSelectionMerge %156 None
                             OpBranchConditional %154 %155 %156

        %155 =                 OpLabel
                                 OpBranch %148

        %156 =             OpLabel
                             OpBranch %149

        %149 =       OpLabel
                       OpBranch %146

        %148 =     OpLabel
                     OpBranch %135

        %135 =     OpLabel
        %159 =       OpLoad %6 %51
        %160 =       OpFAdd %6 %159 %158
                     OpStore %51 %160
                     OpBranch %161

        %161 =     OpLabel
                     OpLoopMerge %163 %164 None
                     OpBranch %165

        %165 =         OpLabel
        %166 =           OpLoad %6 %51
        %167 =           OpFOrdLessThan %55 %166 %139
                         OpBranchConditional %167 %162 %163

        %162 =             OpLabel
        %168 =               OpLoad %6 %46
        %169 =               OpFOrdLessThan %55 %168 %53
                             OpSelectionMerge %171 None
                             OpBranchConditional %169 %170 %171

        %170 =                 OpLabel
                                 OpBranch %163

        %171 =             OpLabel
                             OpBranch %164

        %164 =       OpLabel
                       OpBranch %161

        %163 =     OpLabel
                     OpBranch %136

        %136 = OpLabel
                 OpStore %174 %53
        %175 =   OpAccessChain %45 %35 %58
        %176 =   OpLoad %6 %175
        %178 =   OpFOrdLessThanEqual %55 %176 %177
                 OpSelectionMerge %180 None
                 OpBranchConditional %178 %179 %181

        %179 =     OpLabel
                     OpStore %174 %28
                     OpBranch %180

        %181 =     OpLabel
        %182 =       OpAccessChain %45 %35 %58
        %183 =       OpLoad %6 %182
        %184 =       OpFOrdGreaterThanEqual %55 %183 %177
                     OpSelectionMerge %186 None
                     OpBranchConditional %184 %185 %186

        %185 =         OpLabel
                         OpStore %174 %139
                         OpBranch %186

        %186 =     OpLabel
                     OpBranch %180

        %180 = OpLabel
                 OpStore %187 %47
                 OpBranch %188

        %188 = OpLabel
                 OpLoopMerge %190 %191 None
                 OpBranch %189

        %189 =     OpLabel
        %192 =       OpLoad %9 %19
        %193 =       OpFAdd %9 %192 %109
                     OpStore %19 %193
        %194 =       OpLoad %23 %187
        %196 =       OpSGreaterThan %55 %194 %195
                     OpSelectionMerge %198 None
                     OpBranchConditional %196 %197 %198

        %197 =         OpLabel
                         OpBranch %190

        %198 =     OpLabel
                     OpBranch %191

        %191 =   OpLabel
        %200 =     OpLoad %23 %187
        %201 =     OpIAdd %23 %200 %24
                   OpStore %187 %201
                   OpBranch %188

        %190 = OpLabel
                 OpReturn
               OpFunctionEnd
         %16 = OpFunction %12 None %13
         %14 = OpFunctionParameter %8
         %15 = OpFunctionParameter %6

         %17 = OpLabel
         %30 =   OpCompositeConstruct %9 %15 %15 %15 %15
         %31 =   OpCompositeConstruct %12 %29 %30
                 OpReturnValue %31
               OpFunctionEnd
)";
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  input,
                  SPV_BINARY_TO_TEXT_OPTION_INDENT |
                      SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT |
                      SPV_BINARY_TO_TEXT_OPTION_REORDER_BLOCKS,
                  SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS),
              expected);
}

using FriendlyNameDisassemblyTest = spvtest::TextToBinaryTest;

TEST_F(FriendlyNameDisassemblyTest, Sample) {
  const std::string input = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
%1 = OpTypeInt 32 0
%2 = OpTypeStruct %1 %3 %4 %5 %6 %7 %8 %9 %10 ; force IDs into double digits
%11 = OpConstant %1 42
)";
  const std::string expected =
      R"(OpCapability Shader
OpMemoryModel Logical GLSL450
%uint = OpTypeInt 32 0
%_struct_2 = OpTypeStruct %uint %3 %4 %5 %6 %7 %8 %9 %10
%uint_42 = OpConstant %uint 42
)";
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  input, SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES),
              expected);
}

TEST_F(TextToBinaryTest, ShowByteOffsetsWhenRequested) {
  const std::string input = R"(
OpCapability Shader
OpMemoryModel Logical GLSL450
%1 = OpTypeInt 32 0
%2 = OpTypeVoid
)";
  const std::string expected =
      R"(OpCapability Shader                                 ; 0x00000014
OpMemoryModel Logical GLSL450                       ; 0x0000001c
%1 = OpTypeInt 32 0                                 ; 0x00000028
%2 = OpTypeVoid                                     ; 0x00000038
)";
  EXPECT_THAT(EncodeAndDecodeSuccessfully(
                  input, SPV_BINARY_TO_TEXT_OPTION_SHOW_BYTE_OFFSET),
              expected);
}

TEST_F(TextToBinaryTest, Comments) {
  const std::string input = R"(OpCapability Shader
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %63 "main" %4 %22
OpExecutionMode %63 OriginUpperLeft
OpSource GLSL 450
OpName %4 "_ue"
OpName %8 "_uf"
OpName %11 "_ug"
OpName %12 "_uA"
OpMemberName %12 0 "_ux"
OpName %14 "_uc"
OpName %15 "_uB"
OpMemberName %15 0 "_ux"
OpName %20 "_ud"
OpName %22 "_ucol"
OpName %26 "ANGLEDepthRangeParams"
OpMemberName %26 0 "near"
OpMemberName %26 1 "far"
OpMemberName %26 2 "diff"
OpMemberName %26 3 "reserved"
OpName %27 "ANGLEUniformBlock"
OpMemberName %27 0 "viewport"
OpMemberName %27 1 "clipDistancesEnabled"
OpMemberName %27 2 "xfbActiveUnpaused"
OpMemberName %27 3 "xfbVerticesPerInstance"
OpMemberName %27 4 "numSamples"
OpMemberName %27 5 "xfbBufferOffsets"
OpMemberName %27 6 "acbBufferOffsets"
OpMemberName %27 7 "depthRange"
OpName %29 "ANGLEUniforms"
OpName %33 "_uc"
OpName %32 "_uh"
OpName %49 "_ux"
OpName %50 "_uy"
OpName %48 "_ui"
OpName %63 "main"
OpName %65 "param"
OpName %68 "param"
OpName %73 "param"
OpDecorate %4 Location 0
OpDecorate %8 RelaxedPrecision
OpDecorate %8 DescriptorSet 0
OpDecorate %8 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %11 Binding 1
OpMemberDecorate %12 0 Offset 0
OpMemberDecorate %12 0 RelaxedPrecision
OpDecorate %12 Block
OpDecorate %14 DescriptorSet 0
OpDecorate %14 Binding 2
OpMemberDecorate %15 0 Offset 0
OpMemberDecorate %15 0 RelaxedPrecision
OpDecorate %15 BufferBlock
OpDecorate %20 DescriptorSet 0
OpDecorate %20 Binding 3
OpDecorate %22 RelaxedPrecision
OpDecorate %22 Location 0
OpMemberDecorate %26 0 Offset 0
OpMemberDecorate %26 1 Offset 4
OpMemberDecorate %26 2 Offset 8
OpMemberDecorate %26 3 Offset 12
OpMemberDecorate %27 0 Offset 0
OpMemberDecorate %27 1 Offset 16
OpMemberDecorate %27 2 Offset 20
OpMemberDecorate %27 3 Offset 24
OpMemberDecorate %27 4 Offset 28
OpMemberDecorate %27 5 Offset 32
OpMemberDecorate %27 6 Offset 48
OpMemberDecorate %27 7 Offset 64
OpMemberDecorate %27 2 RelaxedPrecision
OpMemberDecorate %27 4 RelaxedPrecision
OpDecorate %27 Block
OpDecorate %29 DescriptorSet 0
OpDecorate %29 Binding 4
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
%1 = OpTypeFloat 32
%2 = OpTypeVector %1 4
%5 = OpTypeImage %1 2D 0 0 0 1 Unknown
%6 = OpTypeSampledImage %5
%9 = OpTypeImage %1 2D 0 0 0 2 Rgba8
%12 = OpTypeStruct %2
%15 = OpTypeStruct %2
%16 = OpTypeInt 32 0
%17 = OpConstant %16 2
%18 = OpTypeArray %15 %17
%23 = OpTypeInt 32 1
%24 = OpTypeVector %23 4
%25 = OpTypeVector %16 4
%26 = OpTypeStruct %1 %1 %1 %1
%27 = OpTypeStruct %2 %16 %16 %23 %23 %24 %25 %26
%35 = OpTypeVector %1 2
%40 = OpTypeVector %23 2
%61 = OpTypeVoid
%69 = OpConstant %16 0
%78 = OpConstant %16 1
%3 = OpTypePointer Input %2
%7 = OpTypePointer UniformConstant %6
%10 = OpTypePointer UniformConstant %9
%13 = OpTypePointer Uniform %12
%19 = OpTypePointer Uniform %18
%21 = OpTypePointer Output %2
%28 = OpTypePointer Uniform %27
%30 = OpTypePointer Function %2
%70 = OpTypePointer Uniform %2
%31 = OpTypeFunction %2 %30
%47 = OpTypeFunction %2 %30 %30
%62 = OpTypeFunction %61
%4 = OpVariable %3 Input
%8 = OpVariable %7 UniformConstant
%11 = OpVariable %10 UniformConstant
%14 = OpVariable %13 Uniform
%20 = OpVariable %19 Uniform
%22 = OpVariable %21 Output
%29 = OpVariable %28 Uniform
%32 = OpFunction %2 None %31
%33 = OpFunctionParameter %30
%34 = OpLabel
%36 = OpLoad %6 %8
%37 = OpLoad %2 %33
%38 = OpVectorShuffle %35 %37 %37 0 1
%39 = OpImageSampleImplicitLod %2 %36 %38
%41 = OpLoad %2 %33
%42 = OpVectorShuffle %35 %41 %41 2 3
%43 = OpConvertFToS %40 %42
%44 = OpLoad %9 %11
%45 = OpImageRead %2 %44 %43
%46 = OpFAdd %2 %39 %45
OpReturnValue %46
OpFunctionEnd
%48 = OpFunction %2 None %47
%49 = OpFunctionParameter %30
%50 = OpFunctionParameter %30
%51 = OpLabel
%52 = OpLoad %2 %49
%53 = OpVectorShuffle %35 %52 %52 0 1
%54 = OpLoad %2 %50
%55 = OpVectorShuffle %35 %54 %54 2 3
%56 = OpCompositeExtract %1 %53 0
%57 = OpCompositeExtract %1 %53 1
%58 = OpCompositeExtract %1 %55 0
%59 = OpCompositeExtract %1 %55 1
%60 = OpCompositeConstruct %2 %56 %57 %58 %59
OpReturnValue %60
OpFunctionEnd
%63 = OpFunction %61 None %62
%64 = OpLabel
%65 = OpVariable %30 Function
%68 = OpVariable %30 Function
%73 = OpVariable %30 Function
%66 = OpLoad %2 %4
OpStore %65 %66
%67 = OpFunctionCall %2 %32 %65
%71 = OpAccessChain %70 %14 %69
%72 = OpLoad %2 %71
OpStore %68 %72
%74 = OpAccessChain %70 %20 %69 %69
%75 = OpLoad %2 %74
OpStore %73 %75
%76 = OpFunctionCall %2 %48 %68 %73
%77 = OpFAdd %2 %67 %76
%79 = OpAccessChain %70 %20 %78 %69
%80 = OpLoad %2 %79
%81 = OpFAdd %2 %77 %80
OpStore %22 %81
OpReturn
OpFunctionEnd
)";
  const std::string expected = R"(               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %63 "main" %4 %22
               OpExecutionMode %63 OriginUpperLeft

               ; Debug Information
               OpSource GLSL 450
               OpName %4 "_ue"                      ; id %4
               OpName %8 "_uf"                      ; id %8
               OpName %11 "_ug"                     ; id %11
               OpName %12 "_uA"                     ; id %12
               OpMemberName %12 0 "_ux"
               OpName %14 "_uc"                     ; id %14
               OpName %15 "_uB"                     ; id %15
               OpMemberName %15 0 "_ux"
               OpName %20 "_ud"                     ; id %20
               OpName %22 "_ucol"                   ; id %22
               OpName %26 "ANGLEDepthRangeParams"   ; id %26
               OpMemberName %26 0 "near"
               OpMemberName %26 1 "far"
               OpMemberName %26 2 "diff"
               OpMemberName %26 3 "reserved"
               OpName %27 "ANGLEUniformBlock"       ; id %27
               OpMemberName %27 0 "viewport"
               OpMemberName %27 1 "clipDistancesEnabled"
               OpMemberName %27 2 "xfbActiveUnpaused"
               OpMemberName %27 3 "xfbVerticesPerInstance"
               OpMemberName %27 4 "numSamples"
               OpMemberName %27 5 "xfbBufferOffsets"
               OpMemberName %27 6 "acbBufferOffsets"
               OpMemberName %27 7 "depthRange"
               OpName %29 "ANGLEUniforms"           ; id %29
               OpName %33 "_uc"                     ; id %33
               OpName %32 "_uh"                     ; id %32
               OpName %49 "_ux"                     ; id %49
               OpName %50 "_uy"                     ; id %50
               OpName %48 "_ui"                     ; id %48
               OpName %63 "main"                    ; id %63
               OpName %65 "param"                   ; id %65
               OpName %68 "param"                   ; id %68
               OpName %73 "param"                   ; id %73

               ; Annotations
               OpDecorate %4 Location 0
               OpDecorate %8 RelaxedPrecision
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %11 Binding 1
               OpMemberDecorate %12 0 Offset 0
               OpMemberDecorate %12 0 RelaxedPrecision
               OpDecorate %12 Block
               OpDecorate %14 DescriptorSet 0
               OpDecorate %14 Binding 2
               OpMemberDecorate %15 0 Offset 0
               OpMemberDecorate %15 0 RelaxedPrecision
               OpDecorate %15 BufferBlock
               OpDecorate %20 DescriptorSet 0
               OpDecorate %20 Binding 3
               OpDecorate %22 RelaxedPrecision
               OpDecorate %22 Location 0
               OpMemberDecorate %26 0 Offset 0
               OpMemberDecorate %26 1 Offset 4
               OpMemberDecorate %26 2 Offset 8
               OpMemberDecorate %26 3 Offset 12
               OpMemberDecorate %27 0 Offset 0
               OpMemberDecorate %27 1 Offset 16
               OpMemberDecorate %27 2 Offset 20
               OpMemberDecorate %27 3 Offset 24
               OpMemberDecorate %27 4 Offset 28
               OpMemberDecorate %27 5 Offset 32
               OpMemberDecorate %27 6 Offset 48
               OpMemberDecorate %27 7 Offset 64
               OpMemberDecorate %27 2 RelaxedPrecision
               OpMemberDecorate %27 4 RelaxedPrecision
               OpDecorate %27 Block
               OpDecorate %29 DescriptorSet 0
               OpDecorate %29 Binding 4
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision

               ; Types, variables and constants
          %1 = OpTypeFloat 32
          %2 = OpTypeVector %1 4
          %5 = OpTypeImage %1 2D 0 0 0 1 Unknown
          %6 = OpTypeSampledImage %5
          %9 = OpTypeImage %1 2D 0 0 0 2 Rgba8
         %12 = OpTypeStruct %2                      ; Block
         %15 = OpTypeStruct %2                      ; BufferBlock
         %16 = OpTypeInt 32 0
         %17 = OpConstant %16 2
         %18 = OpTypeArray %15 %17
         %23 = OpTypeInt 32 1
         %24 = OpTypeVector %23 4
         %25 = OpTypeVector %16 4
         %26 = OpTypeStruct %1 %1 %1 %1
         %27 = OpTypeStruct %2 %16 %16 %23 %23 %24 %25 %26  ; Block
         %35 = OpTypeVector %1 2
         %40 = OpTypeVector %23 2
         %61 = OpTypeVoid
         %69 = OpConstant %16 0
         %78 = OpConstant %16 1
          %3 = OpTypePointer Input %2
          %7 = OpTypePointer UniformConstant %6
         %10 = OpTypePointer UniformConstant %9
         %13 = OpTypePointer Uniform %12
         %19 = OpTypePointer Uniform %18
         %21 = OpTypePointer Output %2
         %28 = OpTypePointer Uniform %27
         %30 = OpTypePointer Function %2
         %70 = OpTypePointer Uniform %2
         %31 = OpTypeFunction %2 %30
         %47 = OpTypeFunction %2 %30 %30
         %62 = OpTypeFunction %61
          %4 = OpVariable %3 Input                  ; Location 0
          %8 = OpVariable %7 UniformConstant        ; RelaxedPrecision, DescriptorSet 0, Binding 0
         %11 = OpVariable %10 UniformConstant       ; DescriptorSet 0, Binding 1
         %14 = OpVariable %13 Uniform               ; DescriptorSet 0, Binding 2
         %20 = OpVariable %19 Uniform               ; DescriptorSet 0, Binding 3
         %22 = OpVariable %21 Output                ; RelaxedPrecision, Location 0
         %29 = OpVariable %28 Uniform               ; DescriptorSet 0, Binding 4

               ; Function 32
         %32 = OpFunction %2 None %31               ; RelaxedPrecision
         %33 = OpFunctionParameter %30              ; RelaxedPrecision
         %34 = OpLabel
         %36 = OpLoad %6 %8                         ; RelaxedPrecision
         %37 = OpLoad %2 %33                        ; RelaxedPrecision
         %38 = OpVectorShuffle %35 %37 %37 0 1      ; RelaxedPrecision
         %39 = OpImageSampleImplicitLod %2 %36 %38  ; RelaxedPrecision
         %41 = OpLoad %2 %33                        ; RelaxedPrecision
         %42 = OpVectorShuffle %35 %41 %41 2 3      ; RelaxedPrecision
         %43 = OpConvertFToS %40 %42                ; RelaxedPrecision
         %44 = OpLoad %9 %11
         %45 = OpImageRead %2 %44 %43
         %46 = OpFAdd %2 %39 %45
               OpReturnValue %46
               OpFunctionEnd

               ; Function 48
         %48 = OpFunction %2 None %47               ; RelaxedPrecision
         %49 = OpFunctionParameter %30              ; RelaxedPrecision
         %50 = OpFunctionParameter %30              ; RelaxedPrecision
         %51 = OpLabel
         %52 = OpLoad %2 %49                        ; RelaxedPrecision
         %53 = OpVectorShuffle %35 %52 %52 0 1      ; RelaxedPrecision
         %54 = OpLoad %2 %50                        ; RelaxedPrecision
         %55 = OpVectorShuffle %35 %54 %54 2 3      ; RelaxedPrecision
         %56 = OpCompositeExtract %1 %53 0          ; RelaxedPrecision
         %57 = OpCompositeExtract %1 %53 1          ; RelaxedPrecision
         %58 = OpCompositeExtract %1 %55 0          ; RelaxedPrecision
         %59 = OpCompositeExtract %1 %55 1          ; RelaxedPrecision
         %60 = OpCompositeConstruct %2 %56 %57 %58 %59  ; RelaxedPrecision
               OpReturnValue %60
               OpFunctionEnd

               ; Function 63
         %63 = OpFunction %61 None %62
         %64 = OpLabel
         %65 = OpVariable %30 Function
         %68 = OpVariable %30 Function              ; RelaxedPrecision
         %73 = OpVariable %30 Function              ; RelaxedPrecision
         %66 = OpLoad %2 %4
               OpStore %65 %66
         %67 = OpFunctionCall %2 %32 %65            ; RelaxedPrecision
         %71 = OpAccessChain %70 %14 %69
         %72 = OpLoad %2 %71                        ; RelaxedPrecision
               OpStore %68 %72
         %74 = OpAccessChain %70 %20 %69 %69
         %75 = OpLoad %2 %74                        ; RelaxedPrecision
               OpStore %73 %75
         %76 = OpFunctionCall %2 %48 %68 %73        ; RelaxedPrecision
         %77 = OpFAdd %2 %67 %76                    ; RelaxedPrecision
         %79 = OpAccessChain %70 %20 %78 %69
         %80 = OpLoad %2 %79                        ; RelaxedPrecision
         %81 = OpFAdd %2 %77 %80                    ; RelaxedPrecision
               OpStore %22 %81
               OpReturn
               OpFunctionEnd
)";

  EXPECT_THAT(
      EncodeAndDecodeSuccessfully(
          input,
          SPV_BINARY_TO_TEXT_OPTION_COMMENT | SPV_BINARY_TO_TEXT_OPTION_INDENT,
          SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS),
      expected);
}

TEST_F(TextToBinaryTest, NestedWithComments) {
  const std::string input = R"(OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %8 %44
               OpExecutionMode %4 OriginUpperLeft
               OpSource ESSL 310
               OpName %4 "main"
               OpName %8 "v"
               OpName %44 "color"
               OpDecorate %8 RelaxedPrecision
               OpDecorate %8 Location 0
               OpDecorate %9 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %44 Location 0
               OpDecorate %45 RelaxedPrecision
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Input %6
          %8 = OpVariable %7 Input
         %10 = OpConstant %6 0
         %11 = OpTypeBool
         %15 = OpTypeVector %6 4
         %16 = OpTypePointer Function %15
         %21 = OpConstant %6 -0.5
         %22 = OpConstant %6 -0.300000012
         %38 = OpConstant %6 0.5
         %43 = OpTypePointer Output %15
         %44 = OpVariable %43 Output
          %4 = OpFunction %2 None %3
          %5 = OpLabel
          %9 = OpLoad %6 %8
         %12 = OpFOrdLessThanEqual %11 %9 %10
               OpSelectionMerge %14 None
               OpBranchConditional %12 %13 %32
         %13 = OpLabel
         %18 = OpLoad %6 %8
         %19 = OpExtInst %6 %1 Log %18
         %20 = OpLoad %6 %8
         %23 = OpExtInst %6 %1 FClamp %20 %21 %22
         %24 = OpFMul %6 %19 %23
         %25 = OpLoad %6 %8
         %26 = OpExtInst %6 %1 Sin %25
         %27 = OpLoad %6 %8
         %28 = OpExtInst %6 %1 Cos %27
         %29 = OpLoad %6 %8
         %30 = OpExtInst %6 %1 Exp %29
         %31 = OpCompositeConstruct %15 %24 %26 %28 %30
               OpBranch %14
         %32 = OpLabel
         %33 = OpLoad %6 %8
         %34 = OpExtInst %6 %1 Sqrt %33
         %35 = OpLoad %6 %8
         %36 = OpExtInst %6 %1 FSign %35
         %37 = OpLoad %6 %8
         %39 = OpExtInst %6 %1 FMax %37 %38
         %40 = OpLoad %6 %8
         %41 = OpExtInst %6 %1 Floor %40
         %42 = OpCompositeConstruct %15 %34 %36 %39 %41
               OpBranch %14
         %14 = OpLabel
         %45 = OpPhi %15 %31 %13 %42 %32
               OpStore %44 %45
               OpReturn
               OpFunctionEnd
)";
  const std::string expected = R"(               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %4 "main" %8 %44
               OpExecutionMode %4 OriginUpperLeft

               ; Debug Information
               OpSource ESSL 310
               OpName %4 "main"                     ; id %4
               OpName %8 "v"                        ; id %8
               OpName %44 "color"                   ; id %44

               ; Annotations
               OpDecorate %8 RelaxedPrecision
               OpDecorate %8 Location 0
               OpDecorate %9 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %44 Location 0
               OpDecorate %45 RelaxedPrecision

               ; Types, variables and constants
          %2 = OpTypeVoid
          %3 = OpTypeFunction %2
          %6 = OpTypeFloat 32
          %7 = OpTypePointer Input %6
          %8 = OpVariable %7 Input                  ; RelaxedPrecision, Location 0
         %10 = OpConstant %6 0
         %11 = OpTypeBool
         %15 = OpTypeVector %6 4
         %16 = OpTypePointer Function %15
         %21 = OpConstant %6 -0.5
         %22 = OpConstant %6 -0.300000012
         %38 = OpConstant %6 0.5
         %43 = OpTypePointer Output %15
         %44 = OpVariable %43 Output                ; RelaxedPrecision, Location 0


               ; Function 4
          %4 = OpFunction %2 None %3

          %5 = OpLabel
          %9 =   OpLoad %6 %8                       ; RelaxedPrecision
         %12 =   OpFOrdLessThanEqual %11 %9 %10
                 OpSelectionMerge %14 None
                 OpBranchConditional %12 %13 %32

         %13 =     OpLabel
         %18 =       OpLoad %6 %8                   ; RelaxedPrecision
         %19 =       OpExtInst %6 %1 Log %18        ; RelaxedPrecision
         %20 =       OpLoad %6 %8                   ; RelaxedPrecision
         %23 =       OpExtInst %6 %1 FClamp %20 %21 %22     ; RelaxedPrecision
         %24 =       OpFMul %6 %19 %23                      ; RelaxedPrecision
         %25 =       OpLoad %6 %8                           ; RelaxedPrecision
         %26 =       OpExtInst %6 %1 Sin %25                ; RelaxedPrecision
         %27 =       OpLoad %6 %8                           ; RelaxedPrecision
         %28 =       OpExtInst %6 %1 Cos %27                ; RelaxedPrecision
         %29 =       OpLoad %6 %8                           ; RelaxedPrecision
         %30 =       OpExtInst %6 %1 Exp %29                ; RelaxedPrecision
         %31 =       OpCompositeConstruct %15 %24 %26 %28 %30   ; RelaxedPrecision
                     OpBranch %14

         %32 =     OpLabel
         %33 =       OpLoad %6 %8                   ; RelaxedPrecision
         %34 =       OpExtInst %6 %1 Sqrt %33       ; RelaxedPrecision
         %35 =       OpLoad %6 %8                   ; RelaxedPrecision
         %36 =       OpExtInst %6 %1 FSign %35      ; RelaxedPrecision
         %37 =       OpLoad %6 %8                   ; RelaxedPrecision
         %39 =       OpExtInst %6 %1 FMax %37 %38   ; RelaxedPrecision
         %40 =       OpLoad %6 %8                   ; RelaxedPrecision
         %41 =       OpExtInst %6 %1 Floor %40      ; RelaxedPrecision
         %42 =       OpCompositeConstruct %15 %34 %36 %39 %41   ; RelaxedPrecision
                     OpBranch %14

         %14 = OpLabel
         %45 =   OpPhi %15 %31 %13 %42 %32          ; RelaxedPrecision
                 OpStore %44 %45
                 OpReturn
               OpFunctionEnd
)";

  EXPECT_THAT(
      EncodeAndDecodeSuccessfully(
          input,
          SPV_BINARY_TO_TEXT_OPTION_COMMENT | SPV_BINARY_TO_TEXT_OPTION_INDENT |
              SPV_BINARY_TO_TEXT_OPTION_NESTED_INDENT,
          SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS),
      expected);
}

// Test version string.
TEST_F(TextToBinaryTest, VersionString) {
  auto words = CompileSuccessfully("");
  spv_text decoded_text = nullptr;
  EXPECT_THAT(spvBinaryToText(ScopedContext().context, words.data(),
                              words.size(), SPV_BINARY_TO_TEXT_OPTION_NONE,
                              &decoded_text, &diagnostic),
              Eq(SPV_SUCCESS));
  EXPECT_EQ(nullptr, diagnostic);

  EXPECT_THAT(decoded_text->str, HasSubstr("Version: 1.0\n"))
      << EncodeAndDecodeSuccessfully("");
  spvTextDestroy(decoded_text);
}

// Test generator string.

// A test case for the generator string.  This allows us to
// test both of the 16-bit components of the generator word.
struct GeneratorStringCase {
  uint16_t generator;
  uint16_t misc;
  std::string expected;
};

using GeneratorStringTest = spvtest::TextToBinaryTestBase<
    ::testing::TestWithParam<GeneratorStringCase>>;

TEST_P(GeneratorStringTest, Sample) {
  auto words = CompileSuccessfully("");
  EXPECT_EQ(2u, SPV_INDEX_GENERATOR_NUMBER);
  words[SPV_INDEX_GENERATOR_NUMBER] =
      SPV_GENERATOR_WORD(GetParam().generator, GetParam().misc);

  spv_text decoded_text = nullptr;
  EXPECT_THAT(spvBinaryToText(ScopedContext().context, words.data(),
                              words.size(), SPV_BINARY_TO_TEXT_OPTION_NONE,
                              &decoded_text, &diagnostic),
              Eq(SPV_SUCCESS));
  EXPECT_THAT(diagnostic, Eq(nullptr));
  EXPECT_THAT(std::string(decoded_text->str), HasSubstr(GetParam().expected));
  spvTextDestroy(decoded_text);
}

INSTANTIATE_TEST_SUITE_P(GeneratorStrings, GeneratorStringTest,
                         ::testing::ValuesIn(std::vector<GeneratorStringCase>{
                             {SPV_GENERATOR_KHRONOS, 12, "Khronos; 12"},
                             {SPV_GENERATOR_LUNARG, 99, "LunarG; 99"},
                             {SPV_GENERATOR_VALVE, 1, "Valve; 1"},
                             {SPV_GENERATOR_CODEPLAY, 65535, "Codeplay; 65535"},
                             {SPV_GENERATOR_NVIDIA, 19, "NVIDIA; 19"},
                             {SPV_GENERATOR_ARM, 1000, "ARM; 1000"},
                             {SPV_GENERATOR_KHRONOS_LLVM_TRANSLATOR, 38,
                              "Khronos LLVM/SPIR-V Translator; 38"},
                             {SPV_GENERATOR_KHRONOS_ASSEMBLER, 2,
                              "Khronos SPIR-V Tools Assembler; 2"},
                             {SPV_GENERATOR_KHRONOS_GLSLANG, 1,
                              "Khronos Glslang Reference Front End; 1"},
                             {1000, 18, "Unknown(1000); 18"},
                             {65535, 32767, "Unknown(65535); 32767"},
                         }));

// TODO(dneto): Test new instructions and enums in SPIR-V 1.3

}  // namespace
}  // namespace spvtools
