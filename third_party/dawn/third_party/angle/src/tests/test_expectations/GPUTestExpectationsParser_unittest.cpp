//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GPUTestExpectationsParser_unittest.cpp: Unit tests for GPUTestExpectationsParser*
//

#include "tests/test_expectations/GPUTestExpectationsParser.h"
#include <gtest/gtest.h>
#include "tests/test_expectations/GPUTestConfig.h"

using namespace angle;

namespace
{
enum class ConditionTestType
{
    OnLoad,
    OnGet,
};

class GPUTestConfigTester : public GPUTestConfig
{
  public:
    GPUTestConfigTester()
    {
        mConditions.reset();
        mConditions[GPUTestConfig::kConditionWin]    = true;
        mConditions[GPUTestConfig::kConditionNVIDIA] = true;
        mConditions[GPUTestConfig::kConditionD3D11]  = true;
    }
};

class GPUTestExpectationsParserTest : public testing::TestWithParam<ConditionTestType>
{
  public:
    bool load(const std::string &line)
    {
        if (GetParam() == ConditionTestType::OnLoad)
        {
            return parser.loadTestExpectations(config, line);
        }
        else
        {
            return parser.loadAllTestExpectations(line);
        }
    }

    int32_t get(const std::string &testName)
    {
        if (GetParam() == ConditionTestType::OnLoad)
        {
            return parser.getTestExpectation(testName);
        }
        else
        {
            return parser.getTestExpectationWithConfig(config, testName);
        }
    }

    GPUTestConfigTester config;
    GPUTestExpectationsParser parser;
};

// A correct entry with a test that's skipped on all platforms should not lead
// to any errors, and should properly return the expectation SKIP.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserSkip)
{
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestSkip);
}

// A correct entry with a test that's failed on all platforms should not lead
// to any errors, and should properly return the expectation FAIL.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserFail)
{
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = FAIL)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestFail);
}

// A correct entry with a test that's passed on all platforms should not lead
// to any errors, and should properly return the expectation PASS.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserPass)
{
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = PASS)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A correct entry with a test that's timed out on all platforms should not lead
// to any errors, and should properly return the expectation TIMEOUT.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserTimeout)
{
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = TIMEOUT)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestTimeout);
}

// A correct entry with a test that's flaky on all platforms should not lead
// to any errors, and should properly return the expectation FLAKY.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserFlaky)
{
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = FLAKY)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestFlaky);
}

// A correct entry with a test that's skipped on windows should not lead
// to any errors, and should properly return the expectation SKIP on this
// tester.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserSingleLineWin)
{
    std::string line =
        R"(100 WIN : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestSkip);
}

// A correct entry with a test that's skipped on windows/NVIDIA should not lead
// to any errors, and should properly return the expectation SKIP on this
// tester.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserSingleLineWinNVIDIA)
{
    std::string line =
        R"(100 WIN NVIDIA : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestSkip);
}

// A correct entry with a test that's skipped on windows/NVIDIA/D3D11 should not
// lead to any errors, and should properly return the expectation SKIP on this
// tester.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserSingleLineWinNVIDIAD3D11)
{
    std::string line =
        R"(100 WIN NVIDIA D3D11 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestSkip);
}

// Same as GPUTestExpectationsParserSingleLineWinNVIDIAD3D11, but verifying that the order
// of these conditions doesn't matter
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserSingleLineWinNVIDIAD3D11OtherOrder)
{
    std::string line =
        R"(100 D3D11 NVIDIA WIN : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestSkip);
}

// A correct entry with a test that's skipped on mac should not lead
// to any errors, and should default to PASS on this tester (windows).
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserSingleLineMac)
{
    std::string line =
        R"(100 MAC : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A correct entry with a test that has conflicting entries should not lead
// to any errors, and should default to PASS.
// (https:anglebug.com/42262036) In the future, this condition should cause an
// error
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserSingleLineConflict)
{
    std::string line =
        R"(100 WIN MAC : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A line without a bug ID should return an error and not add the expectation.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserMissingBugId)
{
    std::string line = R"( : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_FALSE(load(line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
    {
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    }
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A line without a bug ID should return an error and not add the expectation, (even if
// the line contains conditions that might be mistaken for a bug id)
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserMissingBugIdWithConditions)
{
    std::string line =
        R"(WIN D3D11 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_FALSE(load(line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
    {
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    }
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A line without a colon should return an error and not add the expectation.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserMissingColon)
{
    std::string line = R"(100 dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_FALSE(load(line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
    {
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    }
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A wild character (*) at the end of a line should match any expectations that are a subset of that
// line. It should not greedily match to omany expectations that aren't in that subset.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserWildChar)
{
    std::string line = R"(100 : dEQP-GLES31.functional.layout_binding.ubo.* = SKIP)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestSkip);
    // Also ensure the wild char is not too wild, only covers tests that are more specific
    EXPECT_EQ(get("dEQP-GLES31.functional.program_interface_query.transform_feedback_varying."
                  "resource_list.vertex_fragment.builtin_gl_position"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A line without an equals should return an error and not add the expectation.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserMissingEquals)
{
    std::string line = R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max SKIP)";
    EXPECT_FALSE(load(line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
    {
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    }
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A line without an expectation should return an error and not add the expectation.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserMissingExpectation)
{
    std::string line = R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max =)";
    EXPECT_FALSE(load(line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
    {
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    }
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A line with an expectation that doesn't exist should return an error and not add the expectation.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserInvalidExpectation)
{
    std::string line =
        R"(100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = WRONG)";
    EXPECT_FALSE(load(line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
    {
        EXPECT_EQ(parser.getErrorMessages()[0], "Line 1 : entry with wrong format");
    }
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// ChromeOS is reserved as a token, but doesn't actually check any conditions. Any tokens that
// do not check conditions should return an error and not add the expectation
// (https://anglebug.com/42262032) Remove/update this test when ChromeOS is supported
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserUnimplementedCondition)
{
    // Does not apply when loading all expectations and not checking the config.
    if (GetParam() == ConditionTestType::OnGet)
    {
        GTEST_SKIP() << "Test does not apply when loading all expectations.";
    }

    std::string line =
        R"(100 CHROMEOS : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_FALSE(load(line));
    EXPECT_EQ(parser.getErrorMessages().size(), 1u);
    if (parser.getErrorMessages().size() >= 1)
    {
        EXPECT_EQ(parser.getErrorMessages()[0],
                  "Line 1 : entry invalid, likely unimplemented modifiers");
    }
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// If a line starts with a comment, it's ignored and should not be added to the list.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserComment)
{
    std::string line =
        R"(//100 : dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max = SKIP)";
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
}

// A misspelled expectation should not be matched from getTestExpectation, and should lead to an
// unused expectation when later queried.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserMisspelledExpectation)
{
    std::string line =
        R"(100 : dEQP-GLES31.functionaal.layout_binding.ubo.* = SKIP)";  // "functionaal"
    EXPECT_TRUE(load(line));
    EXPECT_TRUE(parser.getErrorMessages().empty());
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestPass);
    EXPECT_EQ(parser.getUnusedExpectationsMessages().size(), 1u);
    if (parser.getUnusedExpectationsMessages().size() >= 1)
    {
        EXPECT_EQ(parser.getUnusedExpectationsMessages()[0], "Line 1: expectation was unused.");
    }
}

// The parse should still compute correctly which lines were used and which were unused.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserOverrideExpectation)
{
    // Fail all layout_binding tests, but skip the layout_binding.ubo subset.
    std::string line = R"(100 : dEQP-GLES31.functional.layout_binding.* = FAIL
100 : dEQP-GLES31.functional.layout_binding.ubo.* = SKIP)";
    EXPECT_TRUE(load(line));
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestFail);
    // The FAIL expectation was unused because it was overridden.
    ASSERT_EQ(parser.getUnusedExpectationsMessages().size(), 1u);
    EXPECT_EQ(parser.getUnusedExpectationsMessages()[0], "Line 2: expectation was unused.");
    // Now try a test that doesn't match the override criteria
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.image.test"),
              GPUTestExpectationsParser::kGpuTestFail);
    ASSERT_EQ(parser.getUnusedExpectationsMessages().size(), 1u);
    EXPECT_EQ(parser.getUnusedExpectationsMessages()[0], "Line 2: expectation was unused.");
}

// This test is the same as GPUTestExpectationsParserOverrideExpectation, but verifying the order
// doesn't matter when overriding.
TEST_P(GPUTestExpectationsParserTest, GPUTestExpectationsParserOverrideExpectationOtherOrder)
{
    // Fail all layout_binding tests, but skip the layout_binding.ubo subset.
    std::string line = R"(100 : dEQP-GLES31.functional.layout_binding.ubo.* = SKIP
100 : dEQP-GLES31.functional.layout_binding.* = FAIL)";
    EXPECT_TRUE(load(line));
    // Default behavior is to let missing tests pass
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestSkip);
    // The FAIL expectation was unused because it was overridden.
    EXPECT_EQ(parser.getUnusedExpectationsMessages().size(), 1u);
    if (parser.getUnusedExpectationsMessages().size() >= 1)
    {
        EXPECT_EQ(parser.getUnusedExpectationsMessages()[0], "Line 2: expectation was unused.");
    }
    // Now try a test that doesn't match the override criteria
    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.image.test"),
              GPUTestExpectationsParser::kGpuTestFail);
    EXPECT_TRUE(parser.getUnusedExpectationsMessages().empty());
}

// Tests that overlap checking doesn't generate false positives.
TEST_P(GPUTestExpectationsParserTest, OverlapConditions)
{
    std::string lines = R"(
100 NVIDIA VULKAN : dEQP-GLES31.functional.layout_binding.ubo.* = SKIP
100 NVIDIA D3D11 : dEQP-GLES31.functional.layout_binding.ubo.* = SKIP)";

    ASSERT_TRUE(load(lines));
    ASSERT_TRUE(parser.getErrorMessages().empty());

    EXPECT_EQ(get("dEQP-GLES31.functional.layout_binding.ubo.vertex_binding_max"),
              GPUTestExpectationsParser::kGpuTestSkip);
}

std::string ConditionTestTypeName(testing::TestParamInfo<ConditionTestType> testParamInfo)
{
    if (testParamInfo.param == ConditionTestType::OnLoad)
    {
        return "OnLoad";
    }
    else
    {
        return "OnGet";
    }
}

INSTANTIATE_TEST_SUITE_P(,
                         GPUTestExpectationsParserTest,
                         testing::Values(ConditionTestType::OnGet, ConditionTestType::OnLoad),
                         ConditionTestTypeName);
}  // anonymous namespace
