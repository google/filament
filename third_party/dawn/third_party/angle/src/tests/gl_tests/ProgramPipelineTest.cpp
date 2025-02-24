//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineTest:
//   Various tests related to Program Pipeline.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class ProgramPipelineTest : public ANGLETest<>
{
  protected:
    ProgramPipelineTest()
    {
        setWindowWidth(64);
        setWindowHeight(64);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Verify that program pipeline is not supported in version lower than ES31.
TEST_P(ProgramPipelineTest, GenerateProgramPipelineObject)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    if (getClientMajorVersion() < 3 || getClientMinorVersion() < 1)
    {
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
    else
    {
        EXPECT_GL_NO_ERROR();

        glDeleteProgramPipelines(1, &pipeline);
        EXPECT_GL_NO_ERROR();
    }
}

// Verify that program pipeline errors out without GL_EXT_separate_shader_objects extension.
TEST_P(ProgramPipelineTest, GenerateProgramPipelineObjectEXT)
{
    GLuint pipeline;
    glGenProgramPipelinesEXT(1, &pipeline);
    if (!IsGLExtensionEnabled("GL_EXT_separate_shader_objects"))
    {
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
    else
    {
        EXPECT_GL_NO_ERROR();

        glDeleteProgramPipelinesEXT(1, &pipeline);
        EXPECT_GL_NO_ERROR();
    }
}

class ProgramPipelineTest31 : public ProgramPipelineTest
{
  protected:
    void testTearDown() override
    {
        glDeleteProgram(mVertProg);
        glDeleteProgram(mFragProg);
        glDeleteProgramPipelines(1, &mPipeline);
    }

    void bindProgramPipeline(const GLchar *vertString, const GLchar *fragString);
    void drawQuadWithPPO(const std::string &positionAttribName,
                         const GLfloat positionAttribZ,
                         const GLfloat positionAttribXYScale);
    GLint getAvailableProgramBinaryFormatCount() const;

    GLuint mVertProg = 0;
    GLuint mFragProg = 0;
    GLuint mPipeline = 0;
};

class ProgramPipelineXFBTest31 : public ProgramPipelineTest31
{
  protected:
    void testSetUp() override
    {
        glGenBuffers(1, &mTransformFeedbackBuffer);
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBuffer);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, mTransformFeedbackBufferSize, nullptr,
                     GL_STATIC_DRAW);

        glGenTransformFeedbacks(1, &mTransformFeedback);

        ASSERT_GL_NO_ERROR();
    }
    void testTearDown() override
    {
        if (mTransformFeedbackBuffer != 0)
        {
            glDeleteBuffers(1, &mTransformFeedbackBuffer);
            mTransformFeedbackBuffer = 0;
        }

        if (mTransformFeedback != 0)
        {
            glDeleteTransformFeedbacks(1, &mTransformFeedback);
            mTransformFeedback = 0;
        }
    }

    void bindProgramPipelineWithXFBVaryings(const GLchar *vertString,
                                            const GLchar *fragStringconst,
                                            const std::vector<std::string> &tfVaryings,
                                            GLenum bufferMode);

    static const size_t mTransformFeedbackBufferSize = 1 << 24;
    GLuint mTransformFeedbackBuffer;
    GLuint mTransformFeedback;
};

void ProgramPipelineTest31::bindProgramPipeline(const GLchar *vertString, const GLchar *fragString)
{
    mVertProg = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertString);
    ASSERT_NE(mVertProg, 0u);
    mFragProg = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString);
    ASSERT_NE(mFragProg, 0u);

    // Generate a program pipeline and attach the programs to their respective stages
    glGenProgramPipelines(1, &mPipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(mPipeline);
    EXPECT_GL_NO_ERROR();
}

void ProgramPipelineXFBTest31::bindProgramPipelineWithXFBVaryings(
    const GLchar *vertString,
    const GLchar *fragString,
    const std::vector<std::string> &tfVaryings,
    GLenum bufferMode)
{
    GLShader vertShader(GL_VERTEX_SHADER);
    mVertProg = glCreateProgram();

    glShaderSource(vertShader, 1, &vertString, nullptr);
    glCompileShader(vertShader);
    glProgramParameteri(mVertProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mVertProg, vertShader);
    glLinkProgram(mVertProg);
    EXPECT_GL_NO_ERROR();

    mFragProg = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString);
    ASSERT_NE(mFragProg, 0u);

    if (tfVaryings.size() > 0)
    {
        std::vector<const char *> constCharTFVaryings;

        for (const std::string &transformFeedbackVarying : tfVaryings)
        {
            constCharTFVaryings.push_back(transformFeedbackVarying.c_str());
        }

        glTransformFeedbackVaryings(mVertProg, static_cast<GLsizei>(tfVaryings.size()),
                                    &constCharTFVaryings[0], bufferMode);
        glLinkProgram(mVertProg);
    }
    // Generate a program pipeline and attach the programs to their respective stages
    glGenProgramPipelines(1, &mPipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(mPipeline);
    EXPECT_GL_NO_ERROR();
}

// Test generate or delete program pipeline.
TEST_P(ProgramPipelineTest31, GenOrDeleteProgramPipelineTest)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    GLuint pipeline;
    glGenProgramPipelines(-1, &pipeline);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glGenProgramPipelines(0, &pipeline);
    EXPECT_GL_NO_ERROR();

    glDeleteProgramPipelines(-1, &pipeline);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glDeleteProgramPipelines(0, &pipeline);
    EXPECT_GL_NO_ERROR();
}

// Test BindProgramPipeline.
TEST_P(ProgramPipelineTest31, BindProgramPipelineTest)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    glBindProgramPipeline(0);
    EXPECT_GL_NO_ERROR();

    glBindProgramPipeline(2);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();

    glDeleteProgramPipelines(1, &pipeline);
    glBindProgramPipeline(pipeline);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test IsProgramPipeline
TEST_P(ProgramPipelineTest31, IsProgramPipelineTest)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    EXPECT_GL_FALSE(glIsProgramPipeline(0));
    EXPECT_GL_NO_ERROR();

    EXPECT_GL_FALSE(glIsProgramPipeline(2));
    EXPECT_GL_NO_ERROR();

    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    glBindProgramPipeline(pipeline);
    EXPECT_GL_TRUE(glIsProgramPipeline(pipeline));
    EXPECT_GL_NO_ERROR();

    glBindProgramPipeline(0);
    glDeleteProgramPipelines(1, &pipeline);
    EXPECT_GL_FALSE(glIsProgramPipeline(pipeline));
    EXPECT_GL_NO_ERROR();
}

// Simulates a call to glCreateShaderProgramv()
GLuint createShaderProgram(GLenum type,
                           const GLchar *shaderString,
                           unsigned int varyingsCount,
                           const char *const *varyings)
{
    GLShader shader(type);
    if (!shader)
    {
        return 0;
    }

    glShaderSource(shader, 1, &shaderString, nullptr);
    EXPECT_GL_NO_ERROR();

    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog.data());
        INFO() << "Compilation failed:\n"
               << (infoLogLength > 0 ? infoLog.data() : "") << "\n for shader:\n"
               << shaderString << "\n";
        return 0;
    }

    GLuint program = glCreateProgram();

    if (program)
    {
        glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
        if (compiled)
        {
            glAttachShader(program, shader);
            EXPECT_GL_NO_ERROR();

            if (varyingsCount > 0)
            {
                glTransformFeedbackVaryings(program, varyingsCount, varyings, GL_SEPARATE_ATTRIBS);
                EXPECT_GL_NO_ERROR();
            }

            glLinkProgram(program);

            GLint linked = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &linked);

            if (linked == 0)
            {
                glDeleteProgram(program);
                return 0;
            }
            glDetachShader(program, shader);
        }
    }

    EXPECT_GL_NO_ERROR();

    return program;
}

GLuint createShaderProgram(GLenum type, const GLchar *shaderString)
{
    return createShaderProgram(type, shaderString, 0, nullptr);
}

void ProgramPipelineTest31::drawQuadWithPPO(const std::string &positionAttribName,
                                            const GLfloat positionAttribZ,
                                            const GLfloat positionAttribXYScale)
{
    return drawQuadPPO(mVertProg, positionAttribName, positionAttribZ, positionAttribXYScale);
}

GLint ProgramPipelineTest31::getAvailableProgramBinaryFormatCount() const
{
    GLint formatCount = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS_OES, &formatCount);
    return formatCount;
}

// Test glUseProgramStages
TEST_P(ProgramPipelineTest31, UseProgramStages)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = essl31_shaders::fs::Red();

    mVertProg = createShaderProgram(GL_VERTEX_SHADER, vertString);
    ASSERT_NE(mVertProg, 0u);
    mFragProg = createShaderProgram(GL_FRAGMENT_SHADER, fragString);
    ASSERT_NE(mFragProg, 0u);

    // Generate a program pipeline and attach the programs to their respective stages
    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test glUseProgramStages with different programs
TEST_P(ProgramPipelineTest31, UseProgramStagesWithDifferentPrograms)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString  = essl31_shaders::vs::Simple();
    const GLchar *fragString1 = R"(#version 310 es
precision highp float;
uniform float redColorIn;
uniform float greenColorIn;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, 0.0, 1.0);
})";
    const GLchar *fragString2 = R"(#version 310 es
precision highp float;
uniform float greenColorIn;
uniform float blueColorIn;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(0.0, greenColorIn, blueColorIn, 1.0);
})";

    bindProgramPipeline(vertString, fragString1);

    // Set the output color to red
    GLint location = glGetUniformLocation(mFragProg, "redColorIn");
    glActiveShaderProgram(mPipeline, mFragProg);
    glUniform1f(location, 1.0);
    location = glGetUniformLocation(mFragProg, "greenColorIn");
    glActiveShaderProgram(mPipeline, mFragProg);
    glUniform1f(location, 0.0);

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    GLuint fragProg;
    fragProg = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString2);
    ASSERT_NE(fragProg, 0u);
    EXPECT_GL_NO_ERROR();

    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, fragProg);
    EXPECT_GL_NO_ERROR();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the output color to blue
    location = glGetUniformLocation(fragProg, "greenColorIn");
    glActiveShaderProgram(mPipeline, fragProg);
    glUniform1f(location, 0.0);
    location = glGetUniformLocation(fragProg, "blueColorIn");
    glActiveShaderProgram(mPipeline, fragProg);
    glUniform1f(location, 1.0);

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    EXPECT_GL_NO_ERROR();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glDeleteProgram(mVertProg);
    glDeleteProgram(mFragProg);
    glDeleteProgram(fragProg);
}

// Test glUseProgramStages
TEST_P(ProgramPipelineTest31, UseCreateShaderProgramv)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = essl31_shaders::fs::Red();

    bindProgramPipeline(vertString, fragString);

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test pipeline without vertex shader
TEST_P(ProgramPipelineTest31, PipelineWithoutVertexShader)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create a separable program object with a fragment shader
    const GLchar *fragString = essl31_shaders::fs::Red();
    mFragProg                = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString);
    ASSERT_NE(mFragProg, 0u);

    // Generate a program pipeline and attach the program to it's respective stage
    glGenProgramPipelines(1, &mPipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(mPipeline);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 3);
    ASSERT_GL_NO_ERROR();
}

// Test pipeline without any shaders
TEST_P(ProgramPipelineTest31, PipelineWithoutShaders)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Generate a program pipeline
    glGenProgramPipelines(1, &mPipeline);
    EXPECT_GL_NO_ERROR();

    glBindProgramPipeline(mPipeline);
    EXPECT_GL_NO_ERROR();

    glDrawArrays(GL_POINTS, 0, 3);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Ensure validation fails
    GLint value;
    glValidateProgramPipeline(mPipeline);
    glGetProgramPipelineiv(mPipeline, GL_VALIDATE_STATUS, &value);
    EXPECT_FALSE(value);
}

// Test glUniform
TEST_P(ProgramPipelineTest31, FragmentStageUniformTest)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
uniform float redColorIn;
uniform float greenColorIn;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, 0.0, 1.0);
})";

    bindProgramPipeline(vertString, fragString);

    // Set the output color to yellow
    GLint location = glGetUniformLocation(mFragProg, "redColorIn");
    glActiveShaderProgram(mPipeline, mFragProg);
    glUniform1f(location, 1.0);
    location = glGetUniformLocation(mFragProg, "greenColorIn");
    glActiveShaderProgram(mPipeline, mFragProg);
    glUniform1f(location, 1.0);

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the output color to red
    location = glGetUniformLocation(mFragProg, "redColorIn");
    glActiveShaderProgram(mPipeline, mFragProg);
    glUniform1f(location, 1.0);
    location = glGetUniformLocation(mFragProg, "greenColorIn");
    glActiveShaderProgram(mPipeline, mFragProg);
    glUniform1f(location, 0.0);

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glDeleteProgram(mVertProg);
    glDeleteProgram(mFragProg);
}

// Test glUniformBlockBinding and then glBufferData
TEST_P(ProgramPipelineTest31, FragmentStageUniformBlockBufferDataTest)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
layout (std140) uniform color_ubo
{
    float redColorIn;
    float greenColorIn;
};

out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, 0.0, 1.0);
})";

    bindProgramPipeline(vertString, fragString);

    // Set the output color to yellow
    glActiveShaderProgram(mPipeline, mFragProg);
    GLint uboIndex = glGetUniformBlockIndex(mFragProg, "color_ubo");
    GLBuffer uboBuf;
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBuf);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatRed, GL_STATIC_DRAW);
    glUniformBlockBinding(mFragProg, uboIndex, 0);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Clear and test again
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    // Set the output color to red
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatGreen, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBuf);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glDeleteProgram(mVertProg);
    glDeleteProgram(mFragProg);
}

// Test glUniformBlockBinding followed by glBindBufferRange
TEST_P(ProgramPipelineTest31, FragmentStageUniformBlockBindBufferRangeTest)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
layout (std140) uniform color_ubo
{
    float redColorIn;
    float greenColorIn;
    float blueColorIn;
};

out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, blueColorIn, 1.0);
})";

    // Setup three uniform buffers, one with red, one with green, and one with blue
    GLBuffer uboBufRed;
    glBindBuffer(GL_UNIFORM_BUFFER, uboBufRed);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatRed, GL_STATIC_DRAW);
    GLBuffer uboBufGreen;
    glBindBuffer(GL_UNIFORM_BUFFER, uboBufGreen);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatGreen, GL_STATIC_DRAW);
    GLBuffer uboBufBlue;
    glBindBuffer(GL_UNIFORM_BUFFER, uboBufBlue);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatBlue, GL_STATIC_DRAW);

    // Setup pipeline program using red uniform buffer
    bindProgramPipeline(vertString, fragString);
    glActiveShaderProgram(mPipeline, mFragProg);
    GLint uboIndex = glGetUniformBlockIndex(mFragProg, "color_ubo");
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBufRed);
    glUniformBlockBinding(mFragProg, uboIndex, 0);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Clear and test again
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    // bind to green uniform buffer
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBufGreen);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    // bind to blue uniform buffer
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBufBlue);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);

    glDeleteProgram(mVertProg);
    glDeleteProgram(mFragProg);
}

// Test that glUniformBlockBinding can successfully change the binding for PPOs
TEST_P(ProgramPipelineTest31, FragmentStageUniformBlockBinding)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
layout (std140) uniform color_ubo
{
    float redColorIn;
    float greenColorIn;
    float blueColorIn;
};

out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, blueColorIn, 1.0);
})";

    // Setup three uniform buffers, one with red, one with green, and one with blue
    GLBuffer uboBufRed;
    glBindBuffer(GL_UNIFORM_BUFFER, uboBufRed);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatRed, GL_STATIC_DRAW);
    GLBuffer uboBufGreen;
    glBindBuffer(GL_UNIFORM_BUFFER, uboBufGreen);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatGreen, GL_STATIC_DRAW);
    GLBuffer uboBufBlue;
    glBindBuffer(GL_UNIFORM_BUFFER, uboBufBlue);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatBlue, GL_STATIC_DRAW);

    // Setup pipeline program using red uniform buffer
    bindProgramPipeline(vertString, fragString);
    glActiveShaderProgram(mPipeline, mFragProg);
    GLint uboIndex = glGetUniformBlockIndex(mFragProg, "color_ubo");

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBufRed);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboBufGreen);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboBufBlue);

    // Bind the UBO to binding 0 and draw, should be red
    const int w = getWindowWidth();
    const int h = getWindowHeight();
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, w / 2, h / 2);
    glUniformBlockBinding(mFragProg, uboIndex, 0);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);

    // Bind it to binding 1 and draw, should be green
    glScissor(w / 2, 0, w - w / 2, h / 2);
    glUniformBlockBinding(mFragProg, uboIndex, 1);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);

    // Bind it to binding 2 and draw, should be blue
    glScissor(0, h / 2, w, h);
    glUniformBlockBinding(mFragProg, uboIndex, 2);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, w / 2, h / 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(w / 2, 0, w - w / 2, h / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(0, h / 2, w, h - h / 2, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

// Test that dirty bits related to UBOs propagates to the PPO.
TEST_P(ProgramPipelineTest31, UniformBufferUpdatesBeforeBindToPPO)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
layout (std140) uniform color_ubo
{
    float redColorIn;
    float greenColorIn;
    float blueColorIn;
};

out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, blueColorIn, 1.0);
})";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertString);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragString);
    mVertProg = glCreateProgram();
    mFragProg = glCreateProgram();

    // Compile and link a separable vertex shader
    glProgramParameteri(mVertProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mVertProg, vs);
    glLinkProgram(mVertProg);
    EXPECT_GL_NO_ERROR();

    // Compile and link a separable fragment shader
    glProgramParameteri(mFragProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mFragProg, fs);
    glLinkProgram(mFragProg);
    EXPECT_GL_NO_ERROR();

    // Generate a program pipeline and attach the programs
    glGenProgramPipelines(1, &mPipeline);
    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glBindProgramPipeline(mPipeline);
    EXPECT_GL_NO_ERROR();

    GLBuffer uboBufRed;
    glBindBuffer(GL_UNIFORM_BUFFER, uboBufRed);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatRed, GL_STATIC_DRAW);
    GLBuffer uboBufGreen;
    glBindBuffer(GL_UNIFORM_BUFFER, uboBufGreen);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatGreen, GL_STATIC_DRAW);
    GLBuffer uboBufBlue;
    glBindBuffer(GL_UNIFORM_BUFFER, uboBufBlue);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatBlue, GL_STATIC_DRAW);

    glActiveShaderProgram(mPipeline, mFragProg);
    GLint uboIndex = glGetUniformBlockIndex(mFragProg, "color_ubo");
    glUniformBlockBinding(mFragProg, uboIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBufRed);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboBufGreen);

    // Draw once so all dirty bits are handled.  Should be red
    const int w = getWindowWidth();
    const int h = getWindowHeight();
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, w / 2, h / 2);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);

    // Unbind the fragment program from the PPO
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, 0);
    EXPECT_GL_NO_ERROR();

    // Modify the UBO bindings, reattach the program and draw again.  Should be green
    glUniformBlockBinding(mFragProg, uboIndex, 1);

    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glScissor(w / 2, 0, w - w / 2, h / 2);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);

    // Unbind the fragment program from the PPO again, and modify the UBO bindings differently.
    // Draw again, which should be blue.
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboBufBlue);

    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glScissor(0, h / 2, w, h);
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, w / 2, h / 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(w / 2, 0, w - w / 2, h / 2, GLColor::green);
    EXPECT_PIXEL_RECT_EQ(0, h / 2, w, h - h / 2, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

// Test glBindBufferRange between draw calls in the presence of multiple UBOs between VS and FS
TEST_P(ProgramPipelineTest31, BindBufferRangeForMultipleUBOsInMultipleStages)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = R"(#version 310 es
precision highp float;
layout (std140) uniform vsUBO1
{
    float redIn;
};

layout (std140) uniform vsUBO2
{
    float greenIn;
};

out float red;
out float green;

void main()
{
    vec2 pos = vec2(0.0);
    switch (gl_VertexID) {
        case 0: pos = vec2(-1.0, -1.0); break;
        case 1: pos = vec2(3.0, -1.0); break;
        case 2: pos = vec2(-1.0, 3.0); break;
    };
    gl_Position = vec4(pos, 0.0, 1.0);
    red = redIn;
    green = greenIn;
})";
    const GLchar *fragString = R"(#version 310 es
precision highp float;
layout (std140) uniform fsUBO1
{
    float blueIn;
};

layout (std140) uniform fsUBO2
{
    float alphaIn;
};

in float red;
in float green;

out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(red, green, blueIn, alphaIn);
})";

    // Setup two uniform buffers, one with 1, one with 0
    GLBuffer ubo0;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo0);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatBlack, GL_STATIC_DRAW);
    GLBuffer ubo1;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo1);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLColor32F), &kFloatRed, GL_STATIC_DRAW);

    bindProgramPipeline(vertString, fragString);

    // Setup the initial bindings to draw red
    const GLint vsUBO1 = glGetUniformBlockIndex(mVertProg, "vsUBO1");
    const GLint vsUBO2 = glGetUniformBlockIndex(mVertProg, "vsUBO2");
    const GLint fsUBO1 = glGetUniformBlockIndex(mFragProg, "fsUBO1");
    const GLint fsUBO2 = glGetUniformBlockIndex(mFragProg, "fsUBO2");
    glUniformBlockBinding(mVertProg, vsUBO1, 0);
    glUniformBlockBinding(mVertProg, vsUBO2, 1);
    glUniformBlockBinding(mFragProg, fsUBO1, 2);
    glUniformBlockBinding(mFragProg, fsUBO2, 3);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, ubo1);

    const int w = getWindowWidth();
    const int h = getWindowHeight();
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, w / 2, h / 2);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Change the bindings in the vertex shader UBO binding and draw again, should be yellow.
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo1);
    glScissor(w / 2, 0, w - w / 2, h / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Change the bindings in the fragment shader UBO binding and draw again, should be white.
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo1);
    glScissor(0, h / 2, w / 2, h - h / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Change the bindings in both shader UBO bindings and draw again, should be green.
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, ubo0);
    glScissor(w / 2, h / 2, w - w / 2, h - h / 2);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, w / 2, h / 2, GLColor::red);
    EXPECT_PIXEL_RECT_EQ(w / 2, 0, w - w / 2, h / 2, GLColor::yellow);
    EXPECT_PIXEL_RECT_EQ(0, h / 2, w / 2, h - h / 2, GLColor::white);
    EXPECT_PIXEL_RECT_EQ(w / 2, h / 2, w - w / 2, h - h / 2, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test varyings
TEST_P(ProgramPipelineTest31, ProgramPipelineVaryings)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Passthrough();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
in vec4 v_position;
out vec4 my_FragColor;
void main()
{
    my_FragColor = round(v_position);
})";

    bindProgramPipeline(vertString, fragString);

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();

    int w = getWindowWidth() - 2;
    int h = getWindowHeight() - 2;

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(w, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, h, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(w, h, GLColor::yellow);
}

// Creates a program pipeline with a 2D texture and renders with it.
TEST_P(ProgramPipelineTest31, DrawWith2DTexture)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    const GLchar *vertString = R"(#version 310 es
precision highp float;
in vec4 a_position;
out vec2 texCoord;
void main()
{
    gl_Position = a_position;
    texCoord = vec2(a_position.x, a_position.y) * 0.5 + vec2(0.5);
})";

    const GLchar *fragString = R"(#version 310 es
precision highp float;
in vec2 texCoord;
uniform sampler2D tex;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(tex, texCoord);
})";

    std::array<GLColor, 4> colors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    bindProgramPipeline(vertString, fragString);

    drawQuadWithPPO("a_position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();

    int w = getWindowWidth() - 2;
    int h = getWindowHeight() - 2;

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(w, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, h, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(w, h, GLColor::yellow);
}

// Test modifying a shader after it has been detached from a pipeline
TEST_P(ProgramPipelineTest31, DetachAndModifyShader)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = essl31_shaders::fs::Green();

    GLShader vertShader(GL_VERTEX_SHADER);
    GLShader fragShader(GL_FRAGMENT_SHADER);
    mVertProg = glCreateProgram();
    mFragProg = glCreateProgram();

    // Compile and link a separable vertex shader
    glShaderSource(vertShader, 1, &vertString, nullptr);
    glCompileShader(vertShader);
    glProgramParameteri(mVertProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mVertProg, vertShader);
    glLinkProgram(mVertProg);
    EXPECT_GL_NO_ERROR();

    // Compile and link a separable fragment shader
    glShaderSource(fragShader, 1, &fragString, nullptr);
    glCompileShader(fragShader);
    glProgramParameteri(mFragProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mFragProg, fragShader);
    glLinkProgram(mFragProg);
    EXPECT_GL_NO_ERROR();

    // Generate a program pipeline and attach the programs
    glGenProgramPipelines(1, &mPipeline);
    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glBindProgramPipeline(mPipeline);
    EXPECT_GL_NO_ERROR();

    // Draw once to ensure this worked fine
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Detach the fragment shader and modify it such that it no longer fits with this pipeline
    glDetachShader(mFragProg, fragShader);

    // Add an input to the fragment shader, which will make it incompatible
    const GLchar *fragString2 = R"(#version 310 es
precision highp float;
in vec4 color;
out vec4 my_FragColor;
void main()
{
    my_FragColor = color;
})";
    glShaderSource(fragShader, 1, &fragString2, nullptr);
    glCompileShader(fragShader);

    // Link and draw with the program again, which should be fine since the shader was detached
    glLinkProgram(mFragProg);

    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
}

// Test binding two programs that use a texture as different types
TEST_P(ProgramPipelineTest31, DifferentTextureTypes)
{
    // Only the Vulkan backend supports PPO
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Per the OpenGL ES 3.1 spec:
    //
    // It is not allowed to have variables of different sampler types pointing to the same texture
    // image unit within a program object. This situation can only be detected at the next rendering
    // command issued which triggers shader invocations, and an INVALID_OPERATION error will then
    // be generated
    //

    // Create a vertex shader that uses the texture as 2D
    const GLchar *vertString = R"(#version 310 es
precision highp float;
in vec4 a_position;
uniform sampler2D tex2D;
layout(location = 0) out vec4 texColorOut;
layout(location = 1) out vec2 texCoordOut;
void main()
{
    gl_Position = a_position;
    vec2 texCoord = vec2(a_position.x, a_position.y) * 0.5 + vec2(0.5);
    texColorOut = textureLod(tex2D, texCoord, 0.0);
    texCoordOut = texCoord;
})";

    // Create a fragment shader that uses the texture as Cube
    const GLchar *fragString = R"(#version 310 es
precision highp float;
layout(location = 0) in vec4 texColor;
layout(location = 1) in vec2 texCoord;
uniform samplerCube texCube;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(texCube, vec3(texCoord.x, texCoord.y, 0.0));
})";

    // Create and populate the 2D texture
    std::array<GLColor, 4> colors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create a pipeline that uses the bad combination.  This should fail to link the pipeline.
    bindProgramPipeline(vertString, fragString);
    drawQuadWithPPO("a_position", 0.5f, 1.0f);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);

    // Update the fragment shader to correctly use 2D texture
    const GLchar *fragString2 = R"(#version 310 es
precision highp float;
layout(location = 0) in vec4 texColor;
layout(location = 1) in vec2 texCoord;
uniform sampler2D tex2D;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(tex2D, texCoord);
})";

    // Bind the pipeline again, which should succeed.
    bindProgramPipeline(vertString, fragString2);
    drawQuadWithPPO("a_position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
}

// Tests that we receive a PPO link validation error when attempting to draw with the bad PPO
TEST_P(ProgramPipelineTest31, VerifyPpoLinkErrorSignalledCorrectly)
{
    // Create pipeline that should fail link
    // Bind program
    // Draw
    // Unbind program
    // Draw  <<--- expect a link validation error here

    // Only the Vulkan backend supports PPOs
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = essl31_shaders::fs::Red();
    // Create a fragment shader that takes a color input
    // This should cause the PPO link to fail, since the varyings don't match (no output from VS).
    const GLchar *fragStringBad = R"(#version 310 es
precision highp float;
layout(location = 0) in vec4 colorIn;
out vec4 my_FragColor;
void main()
{
    my_FragColor = colorIn;
})";
    bindProgramPipeline(vertString, fragStringBad);

    ANGLE_GL_PROGRAM(program, vertString, fragString);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.0f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Draw with the PPO, which should generate an error due to the link failure.
    glUseProgram(0);
    ASSERT_GL_NO_ERROR();
    drawQuadWithPPO(essl1_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests creating two program pipelines with a common shader and a varying location mismatch.
TEST_P(ProgramPipelineTest31, VaryingLocationMismatch)
{
    // Only the Vulkan backend supports PPOs
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create a fragment shader using the varying location "5".
    const char *kFS = R"(#version 310 es
precision mediump float;
layout(location = 5) in vec4 color;
out vec4 colorOut;
void main()
{
    colorOut = color;
})";

    // Create a pipeline with a vertex shader using varying location "5". Should succeed.
    const char *kVSGood = R"(#version 310 es
precision mediump float;
layout(location = 5) out vec4 color;
in vec4 position;
uniform float uniOne;
void main()
{
    gl_Position = position;
    color = vec4(0, uniOne, 0, 1);
})";

    mVertProg = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &kVSGood);
    ASSERT_NE(mVertProg, 0u);
    mFragProg = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &kFS);
    ASSERT_NE(mFragProg, 0u);

    // Generate a program pipeline and attach the programs to their respective stages
    glGenProgramPipelines(1, &mPipeline);
    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glBindProgramPipeline(mPipeline);
    ASSERT_GL_NO_ERROR();

    GLint location = glGetUniformLocation(mVertProg, "uniOne");
    ASSERT_NE(-1, location);
    glActiveShaderProgram(mPipeline, mVertProg);
    glUniform1f(location, 1.0);
    ASSERT_GL_NO_ERROR();

    drawQuadWithPPO("position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Create a pipeline with a vertex shader using varying location "3". Should fail.
    const char *kVSBad = R"(#version 310 es
precision mediump float;
layout(location = 3) out vec4 color;
in vec4 position;
uniform float uniOne;
void main()
{
    gl_Position = position;
    color = vec4(0, uniOne, 0, 1);
})";

    glDeleteProgram(mVertProg);
    mVertProg = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &kVSBad);
    ASSERT_NE(mVertProg, 0u);

    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    ASSERT_GL_NO_ERROR();

    drawQuadWithPPO("position", 0.5f, 1.0f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that uniform updates are propagated with minimal state changes.
TEST_P(ProgramPipelineTest31, UniformUpdate)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
uniform float redColorIn;
uniform float greenColorIn;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, 0.0, 1.0);
})";

    bindProgramPipeline(vertString, fragString);

    GLint redLoc = glGetUniformLocation(mFragProg, "redColorIn");
    ASSERT_NE(-1, redLoc);
    GLint greenLoc = glGetUniformLocation(mFragProg, "greenColorIn");
    ASSERT_NE(-1, greenLoc);

    glActiveShaderProgram(mPipeline, mFragProg);

    std::array<Vector3, 6> verts = GetQuadVertices();

    GLBuffer vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(verts[0]), verts.data(), GL_STATIC_DRAW);

    GLint posLoc = glGetAttribLocation(mVertProg, essl31_shaders::PositionAttrib());
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the output color to red, draw to left half of window.
    glUniform1f(redLoc, 1.0);
    glUniform1f(greenLoc, 0.0);
    glViewport(0, 0, getWindowWidth() / 2, getWindowHeight());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Set the output color to green, draw to right half of window.
    glUniform1f(redLoc, 0.0);
    glUniform1f(greenLoc, 1.0);

    glViewport(getWindowWidth() / 2, 0, getWindowWidth() / 2, getWindowHeight());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2 + 1, 0, GLColor::green);
}

// Test that uniform updates propagate to two pipelines.
TEST_P(ProgramPipelineTest31, UniformUpdateTwoPipelines)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two separable program objects from a
    // single source string respectively (vertSrc and fragSrc)
    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
uniform float redColorIn;
uniform float greenColorIn;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(redColorIn, greenColorIn, 0.0, 1.0);
})";

    mVertProg = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertString);
    ASSERT_NE(mVertProg, 0u);
    mFragProg = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString);
    ASSERT_NE(mFragProg, 0u);

    GLint redLoc = glGetUniformLocation(mFragProg, "redColorIn");
    ASSERT_NE(-1, redLoc);
    GLint greenLoc = glGetUniformLocation(mFragProg, "greenColorIn");
    ASSERT_NE(-1, greenLoc);

    GLProgramPipeline ppo1;
    glUseProgramStages(ppo1, GL_VERTEX_SHADER_BIT, mVertProg);
    glUseProgramStages(ppo1, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glBindProgramPipeline(ppo1);
    glActiveShaderProgram(ppo1, mFragProg);
    ASSERT_GL_NO_ERROR();

    GLProgramPipeline ppo2;
    glUseProgramStages(ppo2, GL_VERTEX_SHADER_BIT, mVertProg);
    glUseProgramStages(ppo2, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glBindProgramPipeline(ppo2);
    glActiveShaderProgram(ppo2, mFragProg);
    ASSERT_GL_NO_ERROR();

    std::array<Vector3, 6> verts = GetQuadVertices();

    GLBuffer vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(verts[0]), verts.data(), GL_STATIC_DRAW);

    GLint posLoc = glGetAttribLocation(mVertProg, essl31_shaders::PositionAttrib());
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    const GLsizei w = getWindowWidth() / 2;
    const GLsizei h = getWindowHeight() / 2;

    // Set the output color to red, draw to UL quad of window with first PPO.
    glUniform1f(redLoc, 1.0);
    glUniform1f(greenLoc, 0.0);
    glBindProgramPipeline(ppo1);
    glViewport(0, 0, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Draw red to UR half of window with second PPO.
    glBindProgramPipeline(ppo2);
    glViewport(w, 0, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Draw green to LL corner of window with first PPO.
    glUniform1f(redLoc, 0.0);
    glUniform1f(greenLoc, 1.0);
    glBindProgramPipeline(ppo1);
    glViewport(0, h, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Draw green to LR half of window with second PPO.
    glBindProgramPipeline(ppo2);
    glViewport(w, h, w, h);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(w + 1, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(0, h + 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(w + 1, h + 1, GLColor::green);
}

// Tests that setting sampler bindings on a program before the pipeline works as expected.
TEST_P(ProgramPipelineTest31, BindSamplerBeforeCreatingPipeline)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Create two textures - red and green.
    GLTexture redTex;
    glBindTexture(GL_TEXTURE_2D, redTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::red);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLTexture greenTex;
    glBindTexture(GL_TEXTURE_2D, greenTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &GLColor::green);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Bind red to texture unit 0 and green to unit 1.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, redTex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, greenTex);

    // Create the separable programs.
    const char *vsSource = essl1_shaders::vs::Texture2D();
    mVertProg            = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vsSource);
    ASSERT_NE(0u, mVertProg);

    const char *fsSource = essl1_shaders::fs::Texture2D();
    mFragProg            = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fsSource);
    ASSERT_NE(0u, mFragProg);

    // Set the program to sample from the green texture.
    GLint texLoc = glGetUniformLocation(mFragProg, essl1_shaders::Texture2DUniform());
    ASSERT_NE(-1, texLoc);

    glUseProgram(mFragProg);
    glUniform1i(texLoc, 1);

    ASSERT_GL_NO_ERROR();

    // Create and draw with the pipeline.
    GLProgramPipeline ppo;
    glUseProgramStages(ppo, GL_VERTEX_SHADER_BIT, mVertProg);
    glUseProgramStages(ppo, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glBindProgramPipeline(ppo);

    ASSERT_GL_NO_ERROR();

    drawQuadWithPPO(essl1_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();

    // We should have sampled from the second texture bound to unit 1.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a shader IO block varying with separable program links
// successfully.
TEST_P(ProgramPipelineTest31, VaryingIOBlockSeparableProgram)
{
    // Only the Vulkan backend supports PPOs
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] =
        R"(#version 310 es
        #extension GL_EXT_shader_io_blocks : require

        precision highp float;
        in vec4 inputAttribute;
        out Block_inout { vec4 value; } user_out;

        void main()
        {
            gl_Position    = inputAttribute;
            user_out.value = vec4(4.0, 5.0, 6.0, 7.0);
        })";

    constexpr char kFS[] =
        R"(#version 310 es
        #extension GL_EXT_shader_io_blocks : require

        precision highp float;
        layout(location = 0) out mediump vec4 color;
        in Block_inout { vec4 value; } user_in;

        void main()
        {
            color = vec4(1, 0, 0, 1);
        })";

    bindProgramPipeline(kVS, kFS);
    drawQuadWithPPO("inputAttribute", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that a shader IO block varying with separable program links
// successfully.
TEST_P(ProgramPipelineXFBTest31, VaryingIOBlockSeparableProgramWithXFB)
{
    // Only the Vulkan backend supports PPOs
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] =
        R"(#version 310 es
        #extension GL_EXT_shader_io_blocks : require

        precision highp float;
        in vec4 inputAttribute;
        out Block_inout { vec4 value; vec4 value2; } user_out;

        void main()
        {
            gl_Position    = inputAttribute;
            user_out.value = vec4(4.0, 5.0, 6.0, 7.0);
            user_out.value2 = vec4(8.0, 9.0, 10.0, 11.0);
        })";

    constexpr char kFS[] =
        R"(#version 310 es
        #extension GL_EXT_shader_io_blocks : require

        precision highp float;
        layout(location = 0) out mediump vec4 color;
        in Block_inout { vec4 value; vec4 value2; } user_in;

        void main()
        {
            color = vec4(1, 0, 0, 1);
        })";
    std::vector<std::string> tfVaryings;
    tfVaryings.push_back("Block_inout.value");
    tfVaryings.push_back("Block_inout.value2");
    bindProgramPipelineWithXFBVaryings(kVS, kFS, tfVaryings, GL_INTERLEAVED_ATTRIBS);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mTransformFeedbackBuffer);

    // Make sure reconfiguring the vertex shader's transform feedback varyings without a link does
    // not affect the pipeline.  Same with changing buffer modes
    std::vector<const char *> tfVaryingsBogus = {"some", "invalid[0]", "names"};
    glTransformFeedbackVaryings(mVertProg, static_cast<GLsizei>(tfVaryingsBogus.size()),
                                tfVaryingsBogus.data(), GL_SEPARATE_ATTRIBS);

    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuadWithPPO("inputAttribute", 0.5f, 1.0f);
    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    void *mappedBuffer =
        glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(float) * 8, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mappedBuffer);

    float *mappedFloats = static_cast<float *>(mappedBuffer);
    for (unsigned int cnt = 0; cnt < 8; ++cnt)
    {
        EXPECT_EQ(4 + cnt, mappedFloats[cnt]);
    }
    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    EXPECT_GL_NO_ERROR();
}

// Test that XFB GL_SEPARATE_ATTRIBS behaves correctly with PPO.
TEST_P(ProgramPipelineXFBTest31, SeparableProgramWithXFBSeparateMode)
{
    // Only the Vulkan backend supports PPOs
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    const GLchar *vertString = R"(#version 310 es
precision highp float;
in vec4 inputAttribute;
uniform vec4 u_color;
out vec4 texCoord;
out vec4 unused1;
void main()
{
    gl_Position = inputAttribute;
    texCoord = u_color;
})";

    GLShader vertShader(GL_VERTEX_SHADER);
    mVertProg = glCreateProgram();

    // Compile and attach a separable vertex shader
    glShaderSource(vertShader, 1, &vertString, nullptr);
    glCompileShader(vertShader);
    glProgramParameteri(mVertProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mVertProg, vertShader);
    EXPECT_GL_NO_ERROR();

    // Select the varyings for XFB with GL_SEPARATE_ATTRIBS mode
    std::vector<const char *> tfVaryings = {"unused1", "texCoord"};
    glTransformFeedbackVaryings(mVertProg, static_cast<GLsizei>(tfVaryings.size()),
                                tfVaryings.data(), GL_SEPARATE_ATTRIBS);
    glLinkProgram(mVertProg);
    ASSERT_GL_NO_ERROR();

    GLint uniformLoc = glGetUniformLocation(mVertProg, "u_color");
    ASSERT_NE(uniformLoc, -1);
    glProgramUniform4f(mVertProg, uniformLoc, 1, 2, 3, 4);
    ASSERT_GL_NO_ERROR();

    // Generate a program pipeline and attach the program to respective stage
    glGenProgramPipelines(1, &mPipeline);
    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    glBindProgramPipeline(mPipeline);
    ASSERT_GL_NO_ERROR();

    GLuint transformFeedbackBuffer[2] = {0};
    // Allocate space for vec4
    static const size_t transformFeedbackBufferSize = 16;
    glGenBuffers(2, &transformFeedbackBuffer[0]);

    // Bind buffers to the transform feedback binding points
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, transformFeedbackBuffer[0]);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, transformFeedbackBufferSize, NULL, GL_STATIC_DRAW);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, transformFeedbackBuffer[1]);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, transformFeedbackBufferSize, NULL, GL_STATIC_DRAW);

    // Process one point with transform feedback
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, 1);
    glEndTransformFeedback();
    ASSERT_GL_NO_ERROR();

    // Second XFB buffer
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, transformFeedbackBuffer[1]);
    void *mappedBuffer = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
                                          transformFeedbackBufferSize, GL_MAP_READ_BIT);
    ASSERT_NE(nullptr, mappedBuffer);

    float *mappedFloats = static_cast<float *>(mappedBuffer);
    // Expect vec4(1, 2, 3, 4)
    EXPECT_EQ(1, mappedFloats[0]);
    EXPECT_EQ(2, mappedFloats[1]);
    EXPECT_EQ(3, mappedFloats[2]);
    EXPECT_EQ(4, mappedFloats[3]);

    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
    glDeleteBuffers(2, transformFeedbackBuffer);

    EXPECT_GL_NO_ERROR();
}

// Test modifying a shader and re-linking it updates the PPO too
TEST_P(ProgramPipelineTest31, ModifyAndRelinkShader)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    const GLchar *vertString      = essl31_shaders::vs::Simple();
    const GLchar *fragStringGreen = essl31_shaders::fs::Green();
    const GLchar *fragStringRed   = essl31_shaders::fs::Red();

    GLShader vertShader(GL_VERTEX_SHADER);
    GLShader fragShader(GL_FRAGMENT_SHADER);
    mVertProg = glCreateProgram();
    mFragProg = glCreateProgram();

    // Compile and link a separable vertex shader
    glShaderSource(vertShader, 1, &vertString, nullptr);
    glCompileShader(vertShader);
    glProgramParameteri(mVertProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mVertProg, vertShader);
    glLinkProgram(mVertProg);
    EXPECT_GL_NO_ERROR();

    // Compile and link a separable fragment shader
    glShaderSource(fragShader, 1, &fragStringGreen, nullptr);
    glCompileShader(fragShader);
    glProgramParameteri(mFragProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mFragProg, fragShader);
    glLinkProgram(mFragProg);
    EXPECT_GL_NO_ERROR();

    // Generate a program pipeline and attach the programs
    glGenProgramPipelines(1, &mPipeline);
    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glBindProgramPipeline(mPipeline);
    EXPECT_GL_NO_ERROR();

    // Draw once to ensure this worked fine
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Detach the fragment shader and modify it such that it no longer fits with this pipeline
    glDetachShader(mFragProg, fragShader);

    // Modify the FS and re-link it
    glShaderSource(fragShader, 1, &fragStringRed, nullptr);
    glCompileShader(fragShader);
    glProgramParameteri(mFragProg, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glAttachShader(mFragProg, fragShader);
    glLinkProgram(mFragProg);
    EXPECT_GL_NO_ERROR();

    // Draw with the PPO again and verify it's now red
    drawQuadWithPPO(essl31_shaders::PositionAttrib(), 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that a PPO can be used when the attached shader programs are created with glProgramBinary().
// This validates the necessary programs' information is serialized/deserialized so they can be
// linked by the PPO during glDrawArrays.
TEST_P(ProgramPipelineTest31, ProgramBinary)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    ANGLE_SKIP_TEST_IF(getAvailableProgramBinaryFormatCount() == 0);

    const GLchar *vertString = R"(#version 310 es
precision highp float;
in vec4 a_position;
out vec2 texCoord;
void main()
{
    gl_Position = a_position;
    texCoord = vec2(a_position.x, a_position.y) * 0.5 + vec2(0.5);
})";

    const GLchar *fragString = R"(#version 310 es
precision highp float;
in vec2 texCoord;
uniform sampler2D tex;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(tex, texCoord);
})";

    std::array<GLColor, 4> colors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    mVertProg = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertString);
    ASSERT_NE(mVertProg, 0u);
    mFragProg = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragString);
    ASSERT_NE(mFragProg, 0u);

    // Save the VS program binary out
    std::vector<uint8_t> vsBinary(0);
    GLint vsProgramLength = 0;
    GLint vsWrittenLength = 0;
    GLenum vsBinaryFormat = 0;
    glGetProgramiv(mVertProg, GL_PROGRAM_BINARY_LENGTH, &vsProgramLength);
    ASSERT_GL_NO_ERROR();
    vsBinary.resize(vsProgramLength);
    glGetProgramBinary(mVertProg, vsProgramLength, &vsWrittenLength, &vsBinaryFormat,
                       vsBinary.data());
    ASSERT_GL_NO_ERROR();

    // Save the FS program binary out
    std::vector<uint8_t> fsBinary(0);
    GLint fsProgramLength = 0;
    GLint fsWrittenLength = 0;
    GLenum fsBinaryFormat = 0;
    glGetProgramiv(mFragProg, GL_PROGRAM_BINARY_LENGTH, &fsProgramLength);
    ASSERT_GL_NO_ERROR();
    fsBinary.resize(fsProgramLength);
    glGetProgramBinary(mFragProg, fsProgramLength, &fsWrittenLength, &fsBinaryFormat,
                       fsBinary.data());
    ASSERT_GL_NO_ERROR();

    mVertProg = glCreateProgram();
    glProgramBinary(mVertProg, vsBinaryFormat, vsBinary.data(), vsWrittenLength);
    mFragProg = glCreateProgram();
    glProgramBinary(mFragProg, fsBinaryFormat, fsBinary.data(), fsWrittenLength);

    // Generate a program pipeline and attach the programs to their respective stages
    glGenProgramPipelines(1, &mPipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(mPipeline);
    EXPECT_GL_NO_ERROR();

    drawQuadWithPPO("a_position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();

    int w = getWindowWidth() - 2;
    int h = getWindowHeight() - 2;

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(w, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, h, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(w, h, GLColor::yellow);
}

// Test that updating a sampler uniform in a separable program behaves correctly with PPOs.
TEST_P(ProgramPipelineTest31, SampleTextureAThenTextureB)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr int kWidth  = 2;
    constexpr int kHeight = 2;

    const GLchar *vertString = R"(#version 310 es
precision highp float;
in vec2 a_position;
out vec2 texCoord;
void main()
{
    gl_Position = vec4(a_position, 0, 1);
    texCoord = a_position * 0.5 + vec2(0.5);
})";

    const GLchar *fragString = R"(#version 310 es
precision highp float;
in vec2 texCoord;
uniform sampler2D tex;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(tex, texCoord);
})";

    std::array<GLColor, kWidth * kHeight> redColor = {
        {GLColor::red, GLColor::red, GLColor::red, GLColor::red}};
    std::array<GLColor, kWidth * kHeight> greenColor = {
        {GLColor::green, GLColor::green, GLColor::green, GLColor::green}};

    // Create a red texture and bind to texture unit 0
    GLTexture redTex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, redTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redColor.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();
    // Create a green texture and bind to texture unit 1
    GLTexture greenTex;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, greenTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 greenColor.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0);
    ASSERT_GL_NO_ERROR();

    bindProgramPipeline(vertString, fragString);

    GLint location1 = glGetUniformLocation(mFragProg, "tex");
    ASSERT_NE(location1, -1);
    glActiveShaderProgram(mPipeline, mFragProg);
    ASSERT_GL_NO_ERROR();

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // Draw red
    glUniform1i(location1, 0);
    ASSERT_GL_NO_ERROR();
    drawQuadWithPPO("a_position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();

    // Draw green
    glUniform1i(location1, 1);
    ASSERT_GL_NO_ERROR();
    drawQuadWithPPO("a_position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, kWidth, kHeight, GLColor::yellow);
}

// Verify that image uniforms can be used with separable programs
TEST_P(ProgramPipelineTest31, ImageUniforms)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    GLint maxVertexImageUniforms;
    glGetIntegerv(GL_MAX_VERTEX_IMAGE_UNIFORMS, &maxVertexImageUniforms);
    ANGLE_SKIP_TEST_IF(maxVertexImageUniforms == 0);

    const GLchar *vertString = R"(#version 310 es
precision highp float;
precision highp image2D;
layout(binding = 0, r32f) uniform image2D img;

void main()
{
    gl_Position = imageLoad(img, ivec2(0, 0));
})";

    const GLchar *fragString = essl31_shaders::fs::Red();

    bindProgramPipeline(vertString, fragString);

    GLTexture texture;
    GLfloat value = 1.0;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED, GL_FLOAT, &value);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

    glDrawArrays(GL_POINTS, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// Verify that image uniforms can link in separable programs
TEST_P(ProgramPipelineTest31, LinkedImageUniforms)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    GLint maxVertexImageUniforms;
    glGetIntegerv(GL_MAX_VERTEX_IMAGE_UNIFORMS, &maxVertexImageUniforms);
    ANGLE_SKIP_TEST_IF(maxVertexImageUniforms == 0);

    const GLchar *vertString = R"(#version 310 es
precision highp float;
precision highp image2D;
layout(binding = 0, r32f) uniform image2D img;

void main()
{
    vec2 position = -imageLoad(img, ivec2(0, 0)).rr;
    if (gl_VertexID == 1)
        position = vec2(3, -1);
    else if (gl_VertexID == 2)
        position = vec2(-1, 3);

    gl_Position = vec4(position, 0, 1);
})";

    const GLchar *fragString = R"(#version 310 es
precision highp float;
precision highp image2D;
layout(binding = 0, r32f) uniform image2D img;
layout(location = 0) out vec4 color;

void main()
{
    color = imageLoad(img, ivec2(0, 0));
})";

    bindProgramPipeline(vertString, fragString);

    GLTexture texture;
    GLfloat value = 1.0;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED, GL_FLOAT, &value);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Verify that we can have the max amount of uniform buffer objects as part of a program
// pipeline.
TEST_P(ProgramPipelineTest31, MaxFragmentUniformBufferObjects)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    GLint maxUniformBlocks;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &maxUniformBlocks);

    const GLchar *vertString = essl31_shaders::vs::Simple();
    std::stringstream fragStringStream;
    fragStringStream << R"(#version 310 es
precision highp float;
out vec4 my_FragColor;
layout(binding = 0) uniform block {
    float data;
} ubo[)";
    fragStringStream << maxUniformBlocks;
    fragStringStream << R"(];
void main()
{
    my_FragColor = vec4(1.0);
)";
    for (GLint index = 0; index < maxUniformBlocks; index++)
    {
        fragStringStream << "my_FragColor.x + ubo[" << index << "].data;" << std::endl;
    }
    fragStringStream << "}" << std::endl;

    bindProgramPipeline(vertString, fragStringStream.str().c_str());

    std::vector<GLBuffer> buffers(maxUniformBlocks);
    for (GLint index = 0; index < maxUniformBlocks; ++index)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, buffers[index]);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat), NULL, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, index, buffers[index]);
    }

    glDrawArrays(GL_POINTS, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// Verify that we can have the max amount of shader storage buffer objects as part of a program
// pipeline.
TEST_P(ProgramPipelineTest31, MaxFragmentShaderStorageBufferObjects)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    GLint maxShaderStorageBuffers;
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxShaderStorageBuffers);
    const GLchar *vertString = essl31_shaders::vs::Simple();
    std::stringstream fragStringStream;
    fragStringStream << R"(#version 310 es
precision highp float;
out vec4 my_FragColor;
layout(binding = 0) buffer buf {
    float data;
} ssbo[)";
    fragStringStream << maxShaderStorageBuffers;
    fragStringStream << R"(];
void main()
{
    my_FragColor = vec4(1.0);
)";
    for (GLint index = 0; index < maxShaderStorageBuffers; index++)
    {
        fragStringStream << "my_FragColor.x + ssbo[" << index << "].data;" << std::endl;
    }
    fragStringStream << "}" << std::endl;

    bindProgramPipeline(vertString, fragStringStream.str().c_str());

    std::vector<GLBuffer> buffers(maxShaderStorageBuffers);
    for (GLint index = 0; index < maxShaderStorageBuffers; ++index)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[index]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat), NULL, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffers[index]);
    }

    glDrawArrays(GL_POINTS, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// Test validation of redefinition of gl_Position and gl_PointSize in the vertex shader when
// GL_EXT_separate_shader_objects is enabled.
TEST_P(ProgramPipelineTest31, ValidatePositionPointSizeRedefinition)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_separate_shader_objects"));

    {
        constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_separate_shader_objects: require

out float gl_PointSize;

void main()
{
    gl_Position = vec4(0);
    gl_PointSize = 1.;
})";

        // Should fail because gl_Position is not declared.
        GLuint shader = createShaderProgram(GL_VERTEX_SHADER, kVS);
        EXPECT_EQ(shader, 0u);
    }

    {
        constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_separate_shader_objects: require

out float gl_PointSize;

void f()
{
    gl_PointSize = 1.;
}

out vec4 gl_Position;

void main()
{
    gl_Position = vec4(0);
})";

        // Should fail because gl_PointSize is used before gl_Position is declared.
        GLuint shader = createShaderProgram(GL_VERTEX_SHADER, kVS);
        EXPECT_EQ(shader, 0u);
    }

    {
        constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_separate_shader_objects: require

out float gl_PointSize;
out vec4 gl_Position;

void main()
{
    gl_Position = vec4(0);
    gl_PointSize = 1.;
})";

        // Should compile.
        GLuint shader = createShaderProgram(GL_VERTEX_SHADER, kVS);
        EXPECT_NE(shader, 0u);
    }
}

// Basic draw test with GL_EXT_separate_shader_objects enabled in the vertex shader
TEST_P(ProgramPipelineTest31, BasicDrawWithPositionPointSizeRedefinition)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_separate_shader_objects"));

    const char kVS[] = R"(#version 310 es
#extension GL_EXT_separate_shader_objects: require

out float gl_PointSize;
out vec4 gl_Position;

in vec4 a_position;

void main()
{
    gl_Position = a_position;
})";

    mVertProg = createShaderProgram(GL_VERTEX_SHADER, kVS);
    ASSERT_NE(mVertProg, 0u);
    mFragProg = createShaderProgram(GL_FRAGMENT_SHADER, essl31_shaders::fs::Red());
    ASSERT_NE(mFragProg, 0u);

    // Generate a program pipeline and attach the programs to their respective stages
    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();

    drawQuadWithPPO("a_position", 0.5f, 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test a PPO scenario from a game, calling glBindBufferRange between two draws with
// multiple binding points, some unused.  This would result in a crash without the fix.
// https://issuetracker.google.com/issues/299532942
TEST_P(ProgramPipelineTest31, ProgramPipelineBindBufferRange)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    const GLchar *vertString = R"(#version 310 es
in vec4 position;
layout(std140, binding = 0) uniform ubo1 {
    vec4 color;
};
layout(location=0) out vec4 vsColor;
void main()
{
    vsColor = color;
    gl_Position = position;
})";

    const GLchar *fragString = R"(#version 310 es
precision mediump float;
layout(std140, binding = 1) uniform globals {
    vec4 fsColor;
};
layout(std140, binding = 2) uniform params {
    vec4 foo;
};
layout(std140, binding = 3) uniform layer {
    vec4 bar;
};
layout(location=0) highp in vec4 vsColor;
layout(location=0) out vec4 diffuse;
void main()
{
    diffuse = vsColor + fsColor;
})";

    // Create the pipeline
    GLProgramPipeline programPipeline;
    glBindProgramPipeline(programPipeline);

    // Create the vertex shader
    GLShader vertShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertString, nullptr);
    glCompileShader(vertShader);
    mVertProg = glCreateProgram();
    glProgramParameteri(mVertProg, GL_PROGRAM_SEPARABLE, 1);
    glAttachShader(mVertProg, vertShader);
    glLinkProgram(mVertProg);
    glUseProgramStages(programPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    EXPECT_GL_NO_ERROR();

    // Create the fragment shader
    GLShader fragShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragString, nullptr);
    glCompileShader(fragShader);
    mFragProg = glCreateProgram();
    glProgramParameteri(mFragProg, GL_PROGRAM_SEPARABLE, 1);
    glAttachShader(mFragProg, fragShader);
    glLinkProgram(mFragProg);
    glUseProgramStages(programPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    EXPECT_GL_NO_ERROR();

    // Set up a uniform buffer with room for five offsets, four active at a time
    GLBuffer ubo;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, 2048, 0, GL_DYNAMIC_DRAW);
    uint8_t *mappedBuffer =
        static_cast<uint8_t *>(glMapBufferRange(GL_UNIFORM_BUFFER, 0, 2048, GL_MAP_WRITE_BIT));
    ASSERT_NE(nullptr, mappedBuffer);

    // Only set up three of the five offsets. The other two must be present, but unused.
    GLColor32F *binding0 = reinterpret_cast<GLColor32F *>(mappedBuffer);
    GLColor32F *binding1 = reinterpret_cast<GLColor32F *>(mappedBuffer + 256);
    GLColor32F *binding4 = reinterpret_cast<GLColor32F *>(mappedBuffer + 1024);
    *binding0            = kFloatRed;
    *binding1            = kFloatGreen;
    *binding4            = kFloatBlue;
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    // Start with binding0=red and binding1=green
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, 256);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, ubo, 256, 512);
    glBindBufferRange(GL_UNIFORM_BUFFER, 2, ubo, 512, 768);
    glBindBufferRange(GL_UNIFORM_BUFFER, 3, ubo, 768, 1024);
    EXPECT_GL_NO_ERROR();

    // Set up data for draw
    std::array<Vector3, 6> verts = GetQuadVertices();
    GLBuffer vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(verts[0]), verts.data(), GL_STATIC_DRAW);
    GLint posLoc = glGetAttribLocation(mVertProg, "position");
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    EXPECT_GL_NO_ERROR();

    // Perform the first draw
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // At this point we have red+green=yellow, but read-back changes dirty bits and breaks the test
    // EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
    EXPECT_GL_NO_ERROR();

    // This is the key here - call glBindBufferRange between glDraw* calls, changing binding0=blue
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 1024, 1280);
    EXPECT_GL_NO_ERROR();

    // The next draw would crash in handleDirtyGraphicsUniformBuffers without the accompanying fix
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // We should now have green+blue=cyan
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::cyan);
    EXPECT_GL_NO_ERROR();
}

class ProgramPipelineTest32 : public ProgramPipelineTest
{
  protected:
    void testTearDown() override
    {
        glDeleteProgram(mVertProg);
        glDeleteProgram(mFragProg);
        glDeleteProgramPipelines(1, &mPipeline);
    }

    void bindProgramPipeline(const GLchar *vertString,
                             const GLchar *fragString,
                             const GLchar *geomString);
    void drawQuadWithPPO(const std::string &positionAttribName,
                         const GLfloat positionAttribZ,
                         const GLfloat positionAttribXYScale);

    GLuint mVertProg = 0;
    GLuint mFragProg = 0;
    GLuint mGeomProg = 0;
    GLuint mPipeline = 0;
};

void ProgramPipelineTest32::bindProgramPipeline(const GLchar *vertString,
                                                const GLchar *fragString,
                                                const GLchar *geomString)
{
    mVertProg = createShaderProgram(GL_VERTEX_SHADER, vertString);
    ASSERT_NE(mVertProg, 0u);
    mFragProg = createShaderProgram(GL_FRAGMENT_SHADER, fragString);
    ASSERT_NE(mFragProg, 0u);
    mGeomProg = createShaderProgram(GL_GEOMETRY_SHADER, geomString);
    ASSERT_NE(mGeomProg, 0u);

    // Generate a program pipeline and attach the programs to their respective stages
    glGenProgramPipelines(1, &mPipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_VERTEX_SHADER_BIT, mVertProg);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_FRAGMENT_SHADER_BIT, mFragProg);
    EXPECT_GL_NO_ERROR();
    glUseProgramStages(mPipeline, GL_GEOMETRY_SHADER_BIT, mGeomProg);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(mPipeline);
    EXPECT_GL_NO_ERROR();
}

// Verify that we can have the max amount of uniforms with a geometry shader as part of a program
// pipeline.
TEST_P(ProgramPipelineTest32, MaxGeometryImageUniforms)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan() || !IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    GLint maxGeometryImageUnits;
    glGetIntegerv(GL_MAX_GEOMETRY_IMAGE_UNIFORMS_EXT, &maxGeometryImageUnits);

    const GLchar *vertString = essl31_shaders::vs::Simple();
    const GLchar *fragString = R"(#version 310 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    my_FragColor = vec4(1.0);
})";

    std::stringstream geomStringStream;

    geomStringStream << R"(#version 310 es
#extension GL_OES_geometry_shader : require
layout (points)                   in;
layout (points, max_vertices = 1) out;

precision highp iimage2D;

ivec4 counter = ivec4(0);
)";

    for (GLint index = 0; index < maxGeometryImageUnits; ++index)
    {
        geomStringStream << "layout(binding = " << index << ", r32i) uniform iimage2D img" << index
                         << ";" << std::endl;
    }

    geomStringStream << R"(
void main()
{
)";

    for (GLint index = 0; index < maxGeometryImageUnits; ++index)
    {
        geomStringStream << "counter += imageLoad(img" << index << ", ivec2(0, 0));" << std::endl;
    }

    geomStringStream << R"(
    gl_Position = vec4(float(counter.x), 0.0, 0.0, 1.0);
    EmitVertex();
}
)";

    bindProgramPipeline(vertString, fragString, geomStringStream.str().c_str());

    std::vector<GLTexture> textures(maxGeometryImageUnits);
    for (GLint index = 0; index < maxGeometryImageUnits; ++index)
    {
        GLint value = index + 1;

        glBindTexture(GL_TEXTURE_2D, textures[index]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, 1, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_INT, &value);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glBindImageTexture(index, textures[index], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32I);
    }

    glDrawArrays(GL_POINTS, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// Verify creation of seperable tessellation control shader program with transform feeback varying
TEST_P(ProgramPipelineTest32, CreateProgramWithTransformFeedbackVarying)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));

    const char *kVS =
        "#version 320 es\n"
        "\n"
        "#extension GL_EXT_shader_io_blocks : require\n"
        "\n"
        "precision highp float;\n"

        "out BLOCK_INOUT { vec4 value; } user_out;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position    = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "    user_out.value = vec4(4.0, 5.0, 6.0, 7.0);\n"
        "}\n";

    // Fragment shader body
    const char *kFS =
        "#version 320 es\n"
        "\n"
        "#extension GL_EXT_shader_io_blocks : require\n"
        "\n"
        "precision highp float;\n"
        "in BLOCK_INOUT { vec4 value; } user_in;\n"
        "\n"
        "void main()\n"
        "{\n"
        "}\n";

    // Geometry shader body
    const char *kGS =
        "#version 320 es\n"
        "\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "\n"
        "layout(points)                   in;\n"
        "layout(points, max_vertices = 1) out;\n"
        "\n"
        "precision highp float;\n"
        "//${IN_PER_VERTEX_DECL_ARRAY}\n"
        "//${OUT_PER_VERTEX_DECL}\n"
        "in  BLOCK_INOUT { vec4 value; } user_in[];\n"
        "out BLOCK_INOUT { vec4 value; } user_out;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    user_out.value = vec4(1.0, 2.0, 3.0, 4.0);\n"
        "    gl_Position    = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "\n"
        "    EmitVertex();\n"
        "}\n";

    // tessellation control shader body
    const char *kTCS =
        "#version 320 es\n"
        "\n"
        "#extension GL_EXT_tessellation_shader : require\n"
        "#extension GL_EXT_shader_io_blocks : require\n"
        "\n"
        "layout (vertices=4) out;\n"
        "\n"
        "precision highp float;\n"
        "in  BLOCK_INOUT { vec4 value; } user_in[];\n"
        "out BLOCK_INOUT { vec4 value; } user_out[];\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_out   [gl_InvocationID].gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "    user_out [gl_InvocationID].value       = vec4(2.0, 3.0, 4.0, 5.0);\n"
        "\n"
        "    gl_TessLevelOuter[0] = 1.0;\n"
        "    gl_TessLevelOuter[1] = 1.0;\n"
        "}\n";

    // Tessellation evaluation shader
    const char *kTES =
        "#version 320 es\n"
        "\n"
        "#extension GL_EXT_tessellation_shader : require\n"
        "#extension GL_EXT_shader_io_blocks : require\n"
        "\n"
        "layout (isolines, point_mode) in;\n"
        "\n"
        "precision highp float;\n"
        "in  BLOCK_INOUT { vec4 value; } user_in[];\n"
        "out BLOCK_INOUT { vec4 value; } user_out;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position     = gl_in[0].gl_Position;\n"
        "    user_out.value = vec4(3.0, 4.0, 5.0, 6.0);\n"
        "}\n";
    const GLchar *kVaryingName = "BLOCK_INOUT.value";

    GLuint fsProgram = createShaderProgram(GL_FRAGMENT_SHADER, kFS);
    ASSERT_NE(0u, fsProgram);

    GLuint vsProgram = createShaderProgram(GL_VERTEX_SHADER, kVS, 1, &kVaryingName);
    ASSERT_NE(0u, vsProgram);

    GLuint gsProgram = 0u;
    if (IsGLExtensionEnabled("GL_EXT_geometry_shader"))
    {
        gsProgram = createShaderProgram(GL_GEOMETRY_SHADER, kGS, 1, &kVaryingName);
        ASSERT_NE(0u, gsProgram);
    }

    GLuint tcsProgram = createShaderProgram(GL_TESS_CONTROL_SHADER, kTCS, 1, &kVaryingName);
    // Should fail here.
    ASSERT_EQ(0u, tcsProgram);

    // try compiling without transform feedback varying it should pass
    tcsProgram = createShaderProgram(GL_TESS_CONTROL_SHADER, kTCS);
    ASSERT_NE(0u, tcsProgram);

    GLuint tesProgram = createShaderProgram(GL_TESS_EVALUATION_SHADER, kTES, 1, &kVaryingName);
    ASSERT_NE(0u, tesProgram);

    glDeleteProgram(fsProgram);
    glDeleteProgram(vsProgram);
    if (gsProgram != 0u)
    {
        glDeleteProgram(gsProgram);
    }
    glDeleteProgram(tcsProgram);
    glDeleteProgram(tesProgram);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramPipelineTest);
ANGLE_INSTANTIATE_TEST_ES3_AND_ES31(ProgramPipelineTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramPipelineTest31);
ANGLE_INSTANTIATE_TEST_ES31(ProgramPipelineTest31);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramPipelineXFBTest31);
ANGLE_INSTANTIATE_TEST_ES31(ProgramPipelineXFBTest31);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ProgramPipelineTest32);
ANGLE_INSTANTIATE_TEST_ES32(ProgramPipelineTest32);

}  // namespace
