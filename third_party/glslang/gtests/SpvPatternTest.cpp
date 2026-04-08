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

#include "TestFixture.h"
#include "glslang/Public/ResourceLimits.h"
#include <gtest/gtest.h>
#include <regex>
#include <sstream>
#include <string>

namespace glslangtest {

class SpvPatternTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        // Set up any common test state.
    }

    void TearDown() override
    {
        // Clean up any common test state.
    }

    // Helper function to compile shader and get SPIR-V disassembly.
    std::string compileShaderToSpirv(const std::string& shaderSource, EShLanguage stage)
    {
        glslang::TShader shader(stage);
        glslang::TProgram program;

        // Compile the shader
        const char* shaderStrings = shaderSource.c_str();
        shader.setStrings(&shaderStrings, 1);

        if (!shader.parse(GetDefaultResources(), 450, false, EShMsgDefault)) {
            return "COMPILATION_FAILED: " + std::string(shader.getInfoLog());
        }

        program.addShader(&shader);
        if (!program.link(EShMsgDefault)) {
            return "LINKING_FAILED: " + std::string(program.getInfoLog());
        }

        // Generate SPIR-V.
        std::vector<uint32_t> spirv;
        glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

        // Disassemble SPIR-V to text.
        std::ostringstream disassembly_stream;
        spv::Disassemble(disassembly_stream, spirv);

        return disassembly_stream.str();
    }

    // Helper function to check if the given SPIR-V string contains a specific pattern.
    bool containsPattern(const std::string& spirvText, const std::string& pattern)
    {
        return spirvText.find(pattern) != std::string::npos;
    }

    // Helper function to check if the given SPIR-V string contains a UConvert instruction.
    bool containsUConvert(const std::string& spirvText) { return containsPattern(spirvText, "UConvert"); }
};

// Test 1: Indexing an array with a regular int or uint should not generate a zero extension.
TEST_F(SpvPatternTest, RegularIntUintArrayIndexNoConversion)
{
    const std::string shaderSource = R"(
        #version 450 core

        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        void main() {
            uint u = 150u;
            int i = 100;
            float arr[200];
            float x = arr[u];  // Regular uint index
            float y = arr[i];  // Regular int index
        }
        )";

    std::string spirv = compileShaderToSpirv(shaderSource, EShLangCompute);

    // Check that the SPIR-V does NOT contain conversion instructions for regular int/uint indices.
    EXPECT_FALSE(containsUConvert(spirv))
        << "SPIR-V should not contain OpUConvert instruction for regular int/uint array indexing.\n"
        << "Generated SPIR-V:\n"
        << spirv;
}

// Test 2: Indexing an array with a variable index of type uint8_t should generate a zero extension.
TEST_F(SpvPatternTest, Uint8VariableIndexGeneratesUConvert)
{
    const std::string shaderSource = R"(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types : enable

        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        void main() {
            uint8_t u8 = uint8_t(150);
            float arr[200];
            float x = arr[u8];  // Variable uint8_t index
        }
        )";

    std::string spirv = compileShaderToSpirv(shaderSource, EShLangCompute);

    // Check that the SPIR-V contains OpUConvert instruction for variable uint8_t index.
    EXPECT_TRUE(containsUConvert(spirv))
        << "SPIR-V should contain OpUConvert instruction for variable uint8_t array indexing.\n"
        << "Generated SPIR-V:\n"
        << spirv;
}

// Test 2: Indexing an array with a variable index of type uint16_t should generate a zero extension.
TEST_F(SpvPatternTest, Uint16VariableIndexGeneratesUConvert)
{
    const std::string shaderSource = R"(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types : enable

        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        void main() {
            uint16_t u16 = uint16_t(150);
            float arr[200];
            float x = arr[u16];  // Variable uint16_t index
        }
        )";

    std::string spirv = compileShaderToSpirv(shaderSource, EShLangCompute);

    // Check that the SPIR-V contains OpUConvert instruction for variable uint16_t index.
    EXPECT_TRUE(containsUConvert(spirv))
        << "SPIR-V should contain OpUConvert instruction for variable uint16_t array indexing.\n"
        << "Generated SPIR-V:\n"
        << spirv;
}

// Test 3: Indexing an array with a constant index of type uint8_t should NOT generate a zero extension.
// Glslang generates small constants as regular 32-bit integers.
TEST_F(SpvPatternTest, Uint8ConstantIndexNoConversion)
{
    const std::string shaderSource = R"(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types : enable

        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        void main() {
            float arr[200];
            float x = arr[uint8_t(150)];  // Constant uint8_t index
        }
        )";

    std::string spirv = compileShaderToSpirv(shaderSource, EShLangCompute);

    // Check that the SPIR-V does NOT contain OpUConvert instruction for constant uint8_t index.
    // Glslang generates small constants as regular 32-bit integers, so no conversion is needed.
    EXPECT_FALSE(containsUConvert(spirv))
        << "SPIR-V should not contain OpUConvert instruction for constant uint8_t array indexing.\n"
        << "Generated SPIR-V:\n"
        << spirv;
}

// Test 3: Indexing an array with a constant index of type uint16_t should NOT generate a zero extension.
// (Glslang generates small constants as regular 32-bit integers.)
TEST_F(SpvPatternTest, Uint16ConstantIndexNoConversion)
{
    const std::string shaderSource = R"(
        #version 450 core
        #extension GL_EXT_shader_explicit_arithmetic_types : enable

        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

        void main() {
            float arr[200];
            float x = arr[uint16_t(150)];  // Constant uint16_t index
        }
        )";

    std::string spirv = compileShaderToSpirv(shaderSource, EShLangCompute);

    // Check that the SPIR-V does NOT contain OpUConvert instruction for constant uint16_t index.
    // Glslang generates small constants as regular 32-bit integers, so no conversion is needed.
    EXPECT_FALSE(containsUConvert(spirv))
        << "SPIR-V should not contain OpUConvert instruction for constant uint16_t array indexing.\n"
        << "Generated SPIR-V:\n"
        << spirv;
}

} // namespace glslangtest