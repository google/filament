// Copyright (C) 2025 NVIDIA Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <gtest/gtest.h>

#include "TestFixture.h"

namespace glslangtest {
namespace {

using StructNameTest = GlslangTest<::testing::Test>;

// Test the original bug report case from issue #3931.
TEST_F(StructNameTest, StructNameAsStructMember)
{
    const std::string inputFname = GlobalTestSettings.testRoot + "/struct_name_as_struct_member.frag";
    std::string input;
    tryLoadFile(inputFname, "input", &input);

    EShMessages controls = DeriveOptions(Source::GLSL, Semantics::OpenGL, Target::AST);
    GlslangResult result = compileAndLink("struct_name_as_struct_member.frag", input, "", controls,
                                          glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_0);

    // Should NOT have the original syntax error that was reported in the GitHub issue.
    EXPECT_EQ(result.linkingError.find("syntax error, unexpected TYPE_NAME, expecting IDENTIFIER"), std::string::npos);

    // Should compile successfully.
    EXPECT_EQ(result.shaderResults[0].output.find("compilation errors"), std::string::npos);
}

// Test struct parameter after non-struct parameter (minimal regression test).
TEST_F(StructNameTest, StructParameterAfterNonStruct)
{
    const std::string inputFname = GlobalTestSettings.testRoot + "/struct_parameter_after_non_struct.frag";
    std::string input;
    tryLoadFile(inputFname, "input", &input);

    EShMessages controls = DeriveOptions(Source::GLSL, Semantics::OpenGL, Target::AST);
    GlslangResult result = compileAndLink("struct_parameter_after_non_struct.frag", input, "", controls,
                                          glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_0);

    // Should NOT have syntax errors related to struct types in function parameters.
    EXPECT_EQ(result.linkingError.find("syntax error, unexpected IDENTIFIER"), std::string::npos);
    EXPECT_EQ(result.shaderResults[0].output.find("syntax error, unexpected IDENTIFIER"), std::string::npos);

    // Should compile successfully.
    EXPECT_EQ(result.shaderResults[0].output.find("compilation errors"), std::string::npos);
}

// Test struct names as member names in declarator lists (original issue #3931) - More comprehensive patterns.
TEST_F(StructNameTest, StructMemberDeclaratorLists)
{
    const std::string inputFname = GlobalTestSettings.testRoot + "/struct_member_declarator_lists.frag";
    std::string input;
    tryLoadFile(inputFname, "input", &input);

    EShMessages controls = DeriveOptions(Source::GLSL, Semantics::OpenGL, Target::AST);
    GlslangResult result = compileAndLink("struct_member_declarator_lists.frag", input, "", controls,
                                          glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_0);

    // Should NOT have the original syntax error from issue #3931.
    EXPECT_EQ(result.linkingError.find("syntax error, unexpected TYPE_NAME, expecting IDENTIFIER"), std::string::npos);
    EXPECT_EQ(result.shaderResults[0].output.find("syntax error, unexpected TYPE_NAME, expecting IDENTIFIER"), std::string::npos);

    // Should compile successfully.
    EXPECT_EQ(result.shaderResults[0].output.find("compilation errors"), std::string::npos);
}

// Test function parameters with struct types (regression test for issue #4001).
TEST_F(StructNameTest, StructFunctionParameters)
{
    const std::string inputFname = GlobalTestSettings.testRoot + "/struct_function_parameters.frag";
    std::string input;
    tryLoadFile(inputFname, "input", &input);

    EShMessages controls = DeriveOptions(Source::GLSL, Semantics::OpenGL, Target::AST);
    GlslangResult result = compileAndLink("struct_function_parameters.frag", input, "", controls,
                                          glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_0);

    // Should NOT have syntax errors related to struct types in function parameters.
    EXPECT_EQ(result.linkingError.find("syntax error, unexpected IDENTIFIER"), std::string::npos);
    EXPECT_EQ(result.shaderResults[0].output.find("syntax error, unexpected IDENTIFIER"), std::string::npos);

    // Should compile successfully.
    EXPECT_EQ(result.shaderResults[0].output.find("compilation errors"), std::string::npos);
}

// Test struct construction patterns - comprehensive patterns including struct constructors.
TEST_F(StructNameTest, StructConstructionPatterns)
{
    const std::string inputFname = GlobalTestSettings.testRoot + "/struct_construction_patterns.frag";
    std::string input;
    tryLoadFile(inputFname, "input", &input);

    EShMessages controls = DeriveOptions(Source::GLSL, Semantics::OpenGL, Target::AST);
    GlslangResult result = compileAndLink("struct_construction_patterns.frag", input, "", controls,
                                          glslang::EShTargetVulkan_1_0, glslang::EShTargetSpv_1_0);

    // Should NOT have syntax errors in struct construction patterns.
    EXPECT_EQ(result.linkingError.find("syntax error, unexpected IDENTIFIER"), std::string::npos);
    EXPECT_EQ(result.shaderResults[0].output.find("syntax error, unexpected IDENTIFIER"), std::string::npos);

    // Should NOT have the original syntax error from issue #3931.
    EXPECT_EQ(result.linkingError.find("syntax error, unexpected TYPE_NAME, expecting IDENTIFIER"), std::string::npos);
    EXPECT_EQ(result.shaderResults[0].output.find("syntax error, unexpected TYPE_NAME, expecting IDENTIFIER"), std::string::npos);

    // Should compile successfully.
    EXPECT_EQ(result.shaderResults[0].output.find("compilation errors"), std::string::npos);
}

} // anonymous namespace
} // namespace glslangtest