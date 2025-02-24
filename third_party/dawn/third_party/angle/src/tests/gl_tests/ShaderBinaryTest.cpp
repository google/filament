//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <vector>

#include "GLSLANG/ShaderLang.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class ShaderBinaryTest : public ANGLETest<>
{
  protected:
    ShaderBinaryTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        // Test flakiness was noticed when reusing displays.
        forceNewDisplay();
    }

    void testSetUp() override
    {
        ASSERT_EQ(sh::Initialize(), true);

        if (!supported())
        {
            // Must return early because the initialization below will crash otherwise.
            // Individal tests will skip themselves as well.
            return;
        }

        mCompileOptions.objectCode                    = true;
        mCompileOptions.emulateGLDrawID               = true;
        mCompileOptions.initializeUninitializedLocals = true;

        sh::InitBuiltInResources(&mResources);

        // Generate a shader binary:
        ShShaderSpec spec     = SH_GLES2_SPEC;
        ShShaderOutput output = SH_SPIRV_VULKAN_OUTPUT;

        // Vertex shader:
        const char *source = essl1_shaders::vs::Simple();
        ShHandle vertexCompiler =
            sh::ConstructCompiler(GL_VERTEX_SHADER, spec, output, &mResources);
        bool compileResult =
            sh::GetShaderBinary(vertexCompiler, &source, 1, mCompileOptions, &mVertexShaderBinary);
        ASSERT_TRUE(compileResult);

        if (mVertexShaderBinary.size() == 0)
        {
            FAIL() << "Creating vertex shader binary failed.";
        }

        // Fragment shader:
        source = essl1_shaders::fs::Red();
        ShHandle fragmentCompiler =
            sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &mResources);
        compileResult = sh::GetShaderBinary(fragmentCompiler, &source, 1, mCompileOptions,
                                            &mFragmentShaderBinary);
        ASSERT_TRUE(compileResult);

        if (mFragmentShaderBinary.size() == 0)
        {
            FAIL() << "Creating fragment shader binary failed.";
        }
    }

    void testTearDown() override
    {
        sh::Finalize();

        if (!supported())
        {
            // Return early because the initialization didn't complete.
            return;
        }

        glDeleteBuffers(1, &mBuffer);
    }

    bool supported() const
    {
        GLint formatCount;
        glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &formatCount);
        if (formatCount == 0)
        {
            std::cout << "Test skipped because no program binary formats are available."
                      << std::endl;
            return false;
        }
        std::vector<GLint> formats(formatCount);
        glGetIntegerv(GL_SHADER_BINARY_FORMATS, formats.data());

        ASSERT(formats[0] == GL_SHADER_BINARY_ANGLE);

        return true;
    }

    ShCompileOptions mCompileOptions = {};
    ShBuiltInResources mResources;
    GLuint mBuffer;
    sh::ShaderBinaryBlob mVertexShaderBinary;
    sh::ShaderBinaryBlob mFragmentShaderBinary;
};

// This tests the ability to successfully create and load a shader binary.
TEST_P(ShaderBinaryTest, CreateAndLoadBinary)
{
    ANGLE_SKIP_TEST_IF(!supported());

    GLint compileResult;
    // Create vertex shader and load binary
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    // Create fragment shader and load binary
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &fragShader, GL_SHADER_BINARY_ANGLE, mFragmentShaderBinary.data(),
                   mFragmentShaderBinary.size());
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    // Create program from the shaders
    GLuint newProgram = glCreateProgram();
    glAttachShader(newProgram, vertShader);
    glAttachShader(newProgram, fragShader);
    glLinkProgram(newProgram);
    newProgram = CheckLinkStatusAndReturnProgram(newProgram, true);

    // Test with a basic draw
    drawQuad(newProgram, "a_position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Check invalid gl call parameters, such as providing a GL type when a shader handle is expected.
TEST_P(ShaderBinaryTest, InvalidCallParams)
{
    ANGLE_SKIP_TEST_IF(!supported());

    GLuint vertShader[2];
    vertShader[0]     = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Invalid shader
    vertShader[1] = -1;
    glShaderBinary(1, &vertShader[1], GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // GL_INVALID_ENUM is generated if binaryFormat is not an accepted value.
    glShaderBinary(1, &vertShader[0], GL_INVALID_ENUM, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // GL_INVALID_VALUE is generated if n or length is negative
    glShaderBinary(-1, &vertShader[0], GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glShaderBinary(1, &vertShader[0], GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(), -1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // GL_INVALID_OPERATION is generated if any value in shaders is not a shader object.
    GLuint program = glCreateProgram();
    glShaderBinary(1, &program, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // GL_INVALID_OPERATION is generated if more than one of the handles in shaders refers to the
    // same shader object.
    vertShader[1] = vertShader[0];
    glShaderBinary(2, &vertShader[0], GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // GL_INVALID_VALUE is generated if the data pointed to by binary does not match the format
    // specified by binaryFormat.
    std::string invalid("Invalid Shader Blob.");
    glShaderBinary(1, &vertShader[0], GL_SHADER_BINARY_ANGLE, invalid.data(), invalid.size());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Try loading vertex shader binary into fragment shader
    glShaderBinary(1, &fragShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Check attempting to get source code from a shader that was loaded with glShaderBinary.
TEST_P(ShaderBinaryTest, GetSourceFromBinaryShader)
{
    ANGLE_SKIP_TEST_IF(!supported());

    GLint compileResult;
    // Create vertex shader and load binary
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    GLsizei length = 0;
    glGetShaderSource(vertShader, 0, &length, nullptr);

    EXPECT_EQ(length, 0);
}

// Create a program from both shader source code and a binary blob.
TEST_P(ShaderBinaryTest, CombineSourceAndBinaryShaders)
{
    ANGLE_SKIP_TEST_IF(!supported());

    GLint compileResult;
    // Create vertex shader and load binary
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    // Create fragment shader
    GLuint fragShader = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::Red());

    GLuint newProgram = glCreateProgram();
    glAttachShader(newProgram, vertShader);
    glAttachShader(newProgram, fragShader);
    glLinkProgram(newProgram);
    newProgram = CheckLinkStatusAndReturnProgram(newProgram, true);

    // Test with a basic draw
    drawQuad(newProgram, "a_position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shaders loaded with glShaderBinary do not cause false hits in the program cache.
TEST_P(ShaderBinaryTest, ProgramCacheWithShaderBinary)
{
    ANGLE_SKIP_TEST_IF(!supported());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_get_program_binary"));

    GLint compileResult;
    // Create vertex shader that will be shared between the programs
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    // Create a program with a red vertex shader
    GLuint fragShaderRed = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &fragShaderRed, GL_SHADER_BINARY_ANGLE, mFragmentShaderBinary.data(),
                   mFragmentShaderBinary.size());
    glGetShaderiv(fragShaderRed, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    GLuint programRed = glCreateProgram();
    glAttachShader(programRed, vertShader);
    glAttachShader(programRed, fragShaderRed);
    glLinkProgram(programRed);
    programRed = CheckLinkStatusAndReturnProgram(programRed, true);

    // Test with a basic draw
    drawQuad(programRed, "a_position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Create a program with a blue fragment shader, also loaded from a binary
    ShShaderSpec spec     = SH_GLES2_SPEC;
    ShShaderOutput output = SH_SPIRV_VULKAN_OUTPUT;

    const char *source = essl1_shaders::fs::Blue();
    sh::ShaderBinaryBlob fragShaderBlueData;
    ShHandle fragmentCompiler =
        sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &mResources);
    bool binaryCompileResult =
        sh::GetShaderBinary(fragmentCompiler, &source, 1, mCompileOptions, &fragShaderBlueData);
    ASSERT_TRUE(binaryCompileResult);
    if (fragShaderBlueData.size() == 0)
    {
        FAIL() << "Creating fragment shader binary failed.";
    }

    GLuint fragShaderBlue = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &fragShaderBlue, GL_SHADER_BINARY_ANGLE, fragShaderBlueData.data(),
                   fragShaderBlueData.size());
    glGetShaderiv(fragShaderBlue, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    GLuint programBlue = glCreateProgram();
    glAttachShader(programBlue, vertShader);
    glAttachShader(programBlue, fragShaderBlue);
    glLinkProgram(programBlue);
    programBlue = CheckLinkStatusAndReturnProgram(programBlue, true);

    // The program cache should miss and create a new program
    drawQuad(programBlue, "a_position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

class ShaderBinaryTestES31 : public ShaderBinaryTest
{
  protected:
    void testSetUp() override
    {
        ASSERT_EQ(sh::Initialize(), true);

        mCompileOptions.objectCode                    = true;
        mCompileOptions.emulateGLDrawID               = true;
        mCompileOptions.initializeUninitializedLocals = true;

        sh::InitBuiltInResources(&mResources);
        mResources.EXT_geometry_shader     = 1;
        mResources.EXT_tessellation_shader = 1;

        // Generate a shader binary:
        ShShaderSpec spec     = SH_GLES3_1_SPEC;
        ShShaderOutput output = SH_SPIRV_VULKAN_OUTPUT;

        // Vertex shader:
        const char *source = essl31_shaders::vs::Simple();
        ShHandle vertexCompiler =
            sh::ConstructCompiler(GL_VERTEX_SHADER, spec, output, &mResources);
        bool compileResult =
            sh::GetShaderBinary(vertexCompiler, &source, 1, mCompileOptions, &mVertexShaderBinary);
        ASSERT_TRUE(compileResult);

        if (mVertexShaderBinary.size() == 0)
        {
            FAIL() << "Creating vertex shader binary failed.";
        }

        // Fragment shader:
        source = essl31_shaders::fs::Red();
        ShHandle fragmentCompiler =
            sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &mResources);
        compileResult = sh::GetShaderBinary(fragmentCompiler, &source, 1, mCompileOptions,
                                            &mFragmentShaderBinary);
        ASSERT_TRUE(compileResult);

        if (mFragmentShaderBinary.size() == 0)
        {
            FAIL() << "Creating fragment shader binary failed.";
        }
    }
};

// Test all shader stages
TEST_P(ShaderBinaryTestES31, AllShaderStages)
{
    ANGLE_SKIP_TEST_IF(!supported());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));

    const char *kGS = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
precision mediump float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

void main() {
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}
)";

    const char *kTCS = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (vertices = 1) out;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = 1.0;
    gl_TessLevelOuter[0] = 1.0;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = 1.0;
    gl_TessLevelOuter[3] = 1.0;
}

)";

    const char *kTES = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (quads, cw, fractional_odd_spacing) in;

void main()
{
    gl_Position = vec4(gl_TessCoord.xy * 2. - 1., 0, 1);
}
)";

    // Generate a shader binary for geo, tcs, tes:
    ShShaderSpec spec                  = SH_GLES3_1_SPEC;
    ShShaderOutput output              = SH_SPIRV_VULKAN_OUTPUT;
    mResources.EXT_geometry_shader     = 1;
    mResources.EXT_tessellation_shader = 1;

    // Geometry shader:
    sh::ShaderBinaryBlob geometryShaderBinary;
    ShHandle geometryCompiler =
        sh::ConstructCompiler(GL_GEOMETRY_SHADER, spec, output, &mResources);
    bool compileResult =
        sh::GetShaderBinary(geometryCompiler, &kGS, 1, mCompileOptions, &geometryShaderBinary);
    ASSERT_TRUE(compileResult);
    if (geometryShaderBinary.size() == 0)
    {
        FAIL() << "Creating geometry shader binary failed.";
    }

    // tesselation control shader:
    sh::ShaderBinaryBlob tessControlShaderBinary;
    ShHandle tessControlCompiler =
        sh::ConstructCompiler(GL_TESS_CONTROL_SHADER, spec, output, &mResources);
    compileResult = sh::GetShaderBinary(tessControlCompiler, &kTCS, 1, mCompileOptions,
                                        &tessControlShaderBinary);
    ASSERT_TRUE(compileResult);
    if (tessControlShaderBinary.size() == 0)
    {
        FAIL() << "Creating tesselation control shader binary failed.";
    }

    // tesselation evaluation shader:
    sh::ShaderBinaryBlob tessEvaluationShaderBinary;
    ShHandle tessEvaluationCompiler =
        sh::ConstructCompiler(GL_TESS_EVALUATION_SHADER, spec, output, &mResources);
    compileResult = sh::GetShaderBinary(tessEvaluationCompiler, &kTES, 1, mCompileOptions,
                                        &tessEvaluationShaderBinary);
    ASSERT_TRUE(compileResult);
    if (tessEvaluationShaderBinary.size() == 0)
    {
        FAIL() << "Creating tesselation evaluation shader binary failed.";
    }

    GLint loadResult;
    // Create vertex shader and load binary
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &loadResult);
    ASSERT_GL_TRUE(loadResult);

    // Create geometry shader and load binary
    GLuint geoShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderBinary(1, &geoShader, GL_SHADER_BINARY_ANGLE, geometryShaderBinary.data(),
                   geometryShaderBinary.size());
    glGetShaderiv(geoShader, GL_COMPILE_STATUS, &loadResult);
    ASSERT_GL_TRUE(loadResult);

    // Create tesselation control shader and load binary
    GLuint tcShader = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderBinary(1, &tcShader, GL_SHADER_BINARY_ANGLE, tessControlShaderBinary.data(),
                   tessControlShaderBinary.size());
    glGetShaderiv(tcShader, GL_COMPILE_STATUS, &loadResult);
    ASSERT_GL_TRUE(loadResult);

    // Create tesselation evaluation and load binary
    GLuint teShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
    glShaderBinary(1, &teShader, GL_SHADER_BINARY_ANGLE, tessEvaluationShaderBinary.data(),
                   tessEvaluationShaderBinary.size());
    glGetShaderiv(teShader, GL_COMPILE_STATUS, &loadResult);
    ASSERT_GL_TRUE(loadResult);

    // Create fragment shader and load binary
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &fragShader, GL_SHADER_BINARY_ANGLE, mFragmentShaderBinary.data(),
                   mFragmentShaderBinary.size());
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &loadResult);
    ASSERT_GL_TRUE(loadResult);

    // Create program from the shaders
    GLuint newProgram = glCreateProgram();
    glAttachShader(newProgram, vertShader);
    glAttachShader(newProgram, geoShader);
    glAttachShader(newProgram, tcShader);
    glAttachShader(newProgram, teShader);
    glAttachShader(newProgram, fragShader);
    glLinkProgram(newProgram);
    newProgram = CheckLinkStatusAndReturnProgram(newProgram, true);

    // Test with a basic draw
    drawPatches(newProgram, "a_position", 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test glShaderBinary with complex shaders
TEST_P(ShaderBinaryTestES31, ComplexShader)
{
    ANGLE_SKIP_TEST_IF(!supported());

    const char *kVertexShader = R"(#version 310 es
uniform vec2 table[4];

in vec2 position;
in vec4 aTest;

out vec2 texCoord;
out vec4 vTest;

void main()
{
    gl_Position = vec4(position + table[gl_InstanceID], 0, 1);
    vTest       = aTest;
    texCoord    = gl_Position.xy * 0.5 + vec2(0.5);
})";

    const char *kFragmentShader = R"(#version 310 es
precision mediump float;

struct S { sampler2D sampler; };
uniform S uStruct;

layout (binding = 0, std430) buffer Input {
    float sampledInput;
};

in vec2 texCoord;
in vec4 vTest;
out vec4 my_FragColor;

void main()
{
    if (sampledInput == 1.0)
    {
        my_FragColor = texture(uStruct.sampler, texCoord);
    }
    else
    {
        my_FragColor = vTest;
    }
})";

    // Generate shader binaries:
    ShShaderSpec spec     = SH_GLES3_1_SPEC;
    ShShaderOutput output = SH_SPIRV_VULKAN_OUTPUT;

    // Vertex shader:
    sh::ShaderBinaryBlob vertexShaderBinary;
    ShHandle vertexCompiler = sh::ConstructCompiler(GL_VERTEX_SHADER, spec, output, &mResources);
    bool compileResult = sh::GetShaderBinary(vertexCompiler, &kVertexShader, 1, mCompileOptions,
                                             &vertexShaderBinary);
    ASSERT_TRUE(compileResult);

    if (vertexShaderBinary.size() == 0)
    {
        FAIL() << "Creating vertex shader binary failed.";
    }

    // Fragment shader:
    sh::ShaderBinaryBlob fragmentShaderBinary;
    ShHandle fragmentCompiler =
        sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &mResources);
    compileResult = sh::GetShaderBinary(fragmentCompiler, &kFragmentShader, 1, mCompileOptions,
                                        &fragmentShaderBinary);
    ASSERT_TRUE(compileResult);

    if (fragmentShaderBinary.size() == 0)
    {
        FAIL() << "Creating fragment shader binary failed.";
    }

    GLint loadResult;
    // Create vertex shader and load binary
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertShader, GL_SHADER_BINARY_ANGLE, vertexShaderBinary.data(),
                   vertexShaderBinary.size());
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &loadResult);
    ASSERT_GL_TRUE(loadResult);

    // Create fragment shader and load binary
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &fragShader, GL_SHADER_BINARY_ANGLE, fragmentShaderBinary.data(),
                   fragmentShaderBinary.size());
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &loadResult);
    ASSERT_GL_TRUE(loadResult);

    // Create program from the shaders
    GLuint newProgram = glCreateProgram();
    glAttachShader(newProgram, vertShader);
    glAttachShader(newProgram, fragShader);
    glLinkProgram(newProgram);
    newProgram = CheckLinkStatusAndReturnProgram(newProgram, true);
    glUseProgram(newProgram);
    ASSERT_GL_NO_ERROR();

    // Setup instance offset table
    constexpr GLfloat table[] = {-1, -1, -1, 1, 1, -1, 1, 1};
    GLint tableMemberLoc      = glGetUniformLocation(newProgram, "table");
    ASSERT_NE(-1, tableMemberLoc);
    glUniform2fv(tableMemberLoc, 4, table);
    ASSERT_GL_NO_ERROR();

    // Setup red testure and sampler uniform
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {255u, 0u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);

    GLint samplerMemberLoc = glGetUniformLocation(newProgram, "uStruct.sampler");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    ASSERT_GL_NO_ERROR();

    // Setup the `aTest` attribute to blue
    std::vector<Vector4> kInputAttribute(6, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
    GLint positionLocation = glGetAttribLocation(newProgram, "aTest");
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, kInputAttribute.data());
    glEnableVertexAttribArray(positionLocation);

    // Setup 'sampledInput' storage buffer to 1
    constexpr GLfloat kInputDataOne = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat), &kInputDataOne, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    // Test sampling texture with an instanced draw
    drawQuadInstanced(newProgram, "position", 0.5f, 0.5f, GL_FALSE, 4);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Setup 'sampledInput' storage buffer to 0
    constexpr GLfloat kInputDataZero = 0.0f;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat), &kInputDataZero, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    // Test color attribute with an instanced draw
    drawQuadInstanced(newProgram, "position", 0.5f, 0.5f, GL_FALSE, 4);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND_ES31(ShaderBinaryTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShaderBinaryTestES31);
ANGLE_INSTANTIATE_TEST_ES31(ShaderBinaryTestES31);
