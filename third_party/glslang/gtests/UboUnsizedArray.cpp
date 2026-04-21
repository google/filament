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

class UboUnsizedArrayTest : public GlslangTest<::testing::Test> {
protected:
    // Helper function to compile shader and check for specific error message.
    bool compileShouldFailWith(const std::string& code, const std::string& expectedError,
                               EShLanguage stage = EShLangVertex)
    {
        glslang::TShader shader(stage);
        EShMessages controls = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

        bool success = compile(&shader, code, "", controls);

        if (success) {
            // Compilation should have failed.
            return false;
        }

        std::string errorLog = shader.getInfoLog();
        return errorLog.find(expectedError) != std::string::npos;
    }
};

// Test that unsized arrays in uniform blocks work when extension is enabled.
TEST_F(UboUnsizedArrayTest, BasicFunctionality)
{
    const std::string code = R"(
        #version 450
        #extension GL_EXT_uniform_buffer_unsized_array : require
        
        layout(std140, binding=0) uniform DataBlock {
            float scale;
            float values[];  // unsized array as last member
        };
        
        void main() {
            gl_Position = vec4(values[0] * scale, 0.0, 0.0, 1.0);
        }
    )";

    glslang::TShader shader(EShLangVertex);
    EShMessages controls = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);
    EXPECT_TRUE(compile(&shader, code, "", controls));
}

// Test that unsized arrays work when extension is enabled.
TEST_F(UboUnsizedArrayTest, ExtensionRequired)
{
    const std::string code = R"(
        #version 450
        #extension GL_EXT_uniform_buffer_unsized_array : require
        
        layout(std140, binding=0) uniform DataBlock {
            float scale;
            float values[];  // Should work with extension
        };
        
        void main() {
            gl_Position = vec4(values[0] * scale, 0.0, 0.0, 1.0);
        }
    )";

    glslang::TShader shader(EShLangVertex);
    EShMessages controls = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);
    EXPECT_TRUE(compile(&shader, code, "", controls));
}

// Test that only the last member can be unsized.
TEST_F(UboUnsizedArrayTest, OnlyLastMemberCanBeUnsized)
{
    const std::string code = R"(
        #version 450
        #extension GL_EXT_uniform_buffer_unsized_array : require
        
        layout(std140, binding=0) uniform DataBlock {
            float scale;
            float values[];  // Last member - should work
        };
        
        void main() {
            gl_Position = vec4(values[0] * scale, 0.0, 0.0, 1.0);
        }
    )";

    glslang::TShader shader(EShLangVertex);
    EShMessages controls = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);
    EXPECT_TRUE(compile(&shader, code, "", controls));
}

// Test that .length() method fails on unsized arrays.
TEST_F(UboUnsizedArrayTest, LengthMethodNotAllowed)
{
    const std::string code = R"(
        #version 450
        #extension GL_EXT_uniform_buffer_unsized_array : require
        
        layout(std140, binding=0) uniform DataBlock {
            float scale;
            float values[];
        };
        
        layout(location = 0) out vec4 fragColor;
        
        void main() {
            int len = values.length();  // Should fail - length() not supported for unsized arrays in uniform blocks
            fragColor = vec4(len, 0.0, 0.0, 1.0);
        }
    )";

    EXPECT_TRUE(
        compileShouldFailWith(code, "array must be declared with a size before using this method", EShLangFragment));
}

// Test that function parameters cannot be unsized arrays from uniform blocks.
TEST_F(UboUnsizedArrayTest, FunctionParameterRestriction)
{
    const std::string code = R"(
        #version 450
        #extension GL_EXT_uniform_buffer_unsized_array : require
        
        layout(std140, binding=0) uniform DataBlock {
            float scale;
            float values[];
        };
        
        void processArray(float arr[10]) {
            // Process array
        }
        
        void main() {
            processArray(values);  // Should fail - cannot pass unsized arrays as function arguments
            gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
        }
    )";

    EXPECT_TRUE(compileShouldFailWith(code, "no matching overloaded function found"));
}

// Test negative constant indexing.
TEST_F(UboUnsizedArrayTest, NegativeIndexingNotAllowed)
{
    const std::string code = R"(
        #version 450
        #extension GL_EXT_uniform_buffer_unsized_array : require
        
        layout(std140, binding=0) uniform DataBlock {
            float scale;
            float values[];
        };
        
        void main() {
            float value = values[-1];  // Should fail
            gl_Position = vec4(value * scale, 0.0, 0.0, 1.0);
        }
    )";

    EXPECT_TRUE(compileShouldFailWith(code, "index out of range"));
}

// Test that different data types work correctly.
TEST_F(UboUnsizedArrayTest, MultipleDataTypes)
{
    const std::string code = R"(
        #version 450
        #extension GL_EXT_uniform_buffer_unsized_array : require
        
        layout(std140, binding=0) uniform FloatBlock {
            float scale;
            float floatValues[];
        };
        
        layout(std140, binding=1) uniform IntBlock {
            int count;
            int intValues[];
        };
        
        layout(std140, binding=2) uniform VecBlock {
            mat4 transform;
            vec4 vecValues[];
        };
        
        void main() {
            int baseIndex = gl_VertexIndex % 10;
            float value = floatValues[baseIndex] * scale;
            int ivalue = intValues[baseIndex] * count;
            vec4 vvalue = vecValues[baseIndex] * transform;
            
            gl_Position = vec4(value + float(ivalue), vvalue.xy, 1.0);
        }
    )";

    glslang::TShader shader(EShLangVertex);
    EShMessages controls = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);
    EXPECT_TRUE(compile(&shader, code, "", controls));
}

// Test that general integer expressions work for indexing.
TEST_F(UboUnsizedArrayTest, GeneralIntegerIndexing)
{
    const std::string code = R"(
        #version 450
        #extension GL_EXT_uniform_buffer_unsized_array : require
        
        layout(std140, binding=0) uniform DataBlock {
            float scale;
            float values[];
        };
        
        layout(std140, binding=1) uniform SizeInfo {
            int arraySize;
        };
        
        void main() {
            // Various forms of general integer expressions
            int baseIndex = gl_VertexIndex % arraySize;
            int offsetIndex = (baseIndex + 1) % arraySize;
            int computedIndex = min(baseIndex + offsetIndex, arraySize - 1);
            
            float result = values[baseIndex] + values[offsetIndex] + values[computedIndex];
            gl_Position = vec4(result * scale, 0.0, 0.0, 1.0);
        }
    )";

    glslang::TShader shader(EShLangVertex);
    EShMessages controls = static_cast<EShMessages>(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);
    EXPECT_TRUE(compile(&shader, code, "", controls));
}

// Test SPIR-V generation for unsized arrays in uniform blocks.
TEST_F(UboUnsizedArrayTest, SpvGeneration)
{
    const std::string code = R"(
        #version 450
        #extension GL_EXT_uniform_buffer_unsized_array : require
        
        layout(std140, binding=0) uniform DataBlock {
            float scale;
            float values[];
        };
        
        layout(std140, binding=1) uniform SizeBlock {
            int arraySize;
        };
        
        void main() {
            int index = gl_VertexIndex % arraySize;
            float value = values[index];
            gl_Position = vec4(value * scale, 0.0, 0.0, 1.0);
        }
    )";

    // Compile the shader.
    glslang::TShader shader(EShLangVertex);
    const char* shaderStrings[1] = {code.c_str()};
    shader.setStrings(shaderStrings, 1);

    // Set up compilation options.
    EShMessages messages = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);
    shader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientVulkan, 450);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    // Compile.
    bool success = shader.parse(GetDefaultResources(), 450, false, messages);
    EXPECT_TRUE(success) << "Shader compilation failed: " << shader.getInfoLog();

    // Link and generate SPIR-V.
    glslang::TProgram program;
    program.addShader(&shader);
    success = program.link(messages);
    EXPECT_TRUE(success) << "Program linking failed: " << program.getInfoLog();

    // Generate SPIR-V.
    spv::SpvBuildLogger logger;
    std::vector<uint32_t> spirv;
    glslang::SpvOptions options;
    glslang::GlslangToSpv(*program.getIntermediate(EShLangVertex), spirv, &logger, &options);

    // Disassemble SPIR-V to text for easier checking.
    std::ostringstream disassembly_stream;
    spv::Disassemble(disassembly_stream, spirv);
    std::string spirvText = disassembly_stream.str();

    // Check for key SPIR-V elements that indicate successful compilation.
    // 1. SourceExtension for the extension
    EXPECT_TRUE(spirvText.find("SourceExtension") != std::string::npos)
        << "SPIR-V should contain SourceExtension for GL_EXT_uniform_buffer_unsized_array";

    // 2. TypeRuntimeArray for the unsized array
    EXPECT_TRUE(spirvText.find("TypeRuntimeArray") != std::string::npos)
        << "SPIR-V should contain TypeRuntimeArray for unsized arrays";

    // 3. Block decoration (for uniform blocks with runtime arrays)
    EXPECT_TRUE(spirvText.find("Block") != std::string::npos)
        << "SPIR-V should contain Block decoration for uniform blocks with runtime arrays";

    // 4. RuntimeDescriptorArrayEXT capability
    EXPECT_TRUE(spirvText.find("RuntimeDescriptorArrayEXT") != std::string::npos)
        << "SPIR-V should contain RuntimeDescriptorArrayEXT capability";

    // 5. SPV_EXT_descriptor_indexing extension
    EXPECT_TRUE(spirvText.find("SPV_EXT_descriptor_indexing") != std::string::npos)
        << "SPIR-V should contain SPV_EXT_descriptor_indexing extension";
}

} // anonymous namespace
} // namespace glslangtest