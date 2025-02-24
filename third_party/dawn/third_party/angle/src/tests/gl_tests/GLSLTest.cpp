//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
class GLSLTest : public ANGLETest<>
{
  protected:
    GLSLTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    std::string GenerateVaryingType(GLint vectorSize)
    {
        char varyingType[10];

        if (vectorSize == 1)
        {
            snprintf(varyingType, sizeof(varyingType), "float");
        }
        else
        {
            snprintf(varyingType, sizeof(varyingType), "vec%d", vectorSize);
        }

        return std::string(varyingType);
    }

    std::string GenerateVectorVaryingDeclaration(GLint vectorSize, GLint arraySize, GLint id)
    {
        char buff[100];

        if (arraySize == 1)
        {
            snprintf(buff, sizeof(buff), "varying %s v%d;\n",
                     GenerateVaryingType(vectorSize).c_str(), id);
        }
        else
        {
            snprintf(buff, sizeof(buff), "varying %s v%d[%d];\n",
                     GenerateVaryingType(vectorSize).c_str(), id, arraySize);
        }

        return std::string(buff);
    }

    std::string GenerateVectorVaryingSettingCode(GLint vectorSize, GLint arraySize, GLint id)
    {
        std::string returnString;
        char buff[100];

        if (arraySize == 1)
        {
            snprintf(buff, sizeof(buff), "\t v%d = %s(1.0);\n", id,
                     GenerateVaryingType(vectorSize).c_str());
            returnString += buff;
        }
        else
        {
            for (int i = 0; i < arraySize; i++)
            {
                snprintf(buff, sizeof(buff), "\t v%d[%d] = %s(1.0);\n", id, i,
                         GenerateVaryingType(vectorSize).c_str());
                returnString += buff;
            }
        }

        return returnString;
    }

    std::string GenerateVectorVaryingUseCode(GLint arraySize, GLint id)
    {
        if (arraySize == 1)
        {
            char buff[100];
            snprintf(buff, sizeof(buff), "v%d + ", id);
            return std::string(buff);
        }
        else
        {
            std::string returnString;
            for (int i = 0; i < arraySize; i++)
            {
                char buff[100];
                snprintf(buff, sizeof(buff), "v%d[%d] + ", id, i);
                returnString += buff;
            }
            return returnString;
        }
    }

    void GenerateGLSLWithVaryings(GLint floatCount,
                                  GLint floatArrayCount,
                                  GLint vec2Count,
                                  GLint vec2ArrayCount,
                                  GLint vec3Count,
                                  GLint vec3ArrayCount,
                                  GLint vec4Count,
                                  GLint vec4ArrayCount,
                                  bool useFragCoord,
                                  bool usePointCoord,
                                  bool usePointSize,
                                  std::string *fragmentShader,
                                  std::string *vertexShader)
    {
        // Generate a string declaring the varyings, to share between the fragment shader and the
        // vertex shader.
        std::string varyingDeclaration;

        unsigned int varyingCount = 0;

        for (GLint i = 0; i < floatCount; i++)
        {
            varyingDeclaration += GenerateVectorVaryingDeclaration(1, 1, varyingCount);
            varyingCount += 1;
        }

        for (GLint i = 0; i < floatArrayCount; i++)
        {
            varyingDeclaration += GenerateVectorVaryingDeclaration(1, 2, varyingCount);
            varyingCount += 1;
        }

        for (GLint i = 0; i < vec2Count; i++)
        {
            varyingDeclaration += GenerateVectorVaryingDeclaration(2, 1, varyingCount);
            varyingCount += 1;
        }

        for (GLint i = 0; i < vec2ArrayCount; i++)
        {
            varyingDeclaration += GenerateVectorVaryingDeclaration(2, 2, varyingCount);
            varyingCount += 1;
        }

        for (GLint i = 0; i < vec3Count; i++)
        {
            varyingDeclaration += GenerateVectorVaryingDeclaration(3, 1, varyingCount);
            varyingCount += 1;
        }

        for (GLint i = 0; i < vec3ArrayCount; i++)
        {
            varyingDeclaration += GenerateVectorVaryingDeclaration(3, 2, varyingCount);
            varyingCount += 1;
        }

        for (GLint i = 0; i < vec4Count; i++)
        {
            varyingDeclaration += GenerateVectorVaryingDeclaration(4, 1, varyingCount);
            varyingCount += 1;
        }

        for (GLint i = 0; i < vec4ArrayCount; i++)
        {
            varyingDeclaration += GenerateVectorVaryingDeclaration(4, 2, varyingCount);
            varyingCount += 1;
        }

        // Generate the vertex shader
        vertexShader->clear();
        vertexShader->append(varyingDeclaration);
        vertexShader->append("\nvoid main()\n{\n");

        unsigned int currentVSVarying = 0;

        for (GLint i = 0; i < floatCount; i++)
        {
            vertexShader->append(GenerateVectorVaryingSettingCode(1, 1, currentVSVarying));
            currentVSVarying += 1;
        }

        for (GLint i = 0; i < floatArrayCount; i++)
        {
            vertexShader->append(GenerateVectorVaryingSettingCode(1, 2, currentVSVarying));
            currentVSVarying += 1;
        }

        for (GLint i = 0; i < vec2Count; i++)
        {
            vertexShader->append(GenerateVectorVaryingSettingCode(2, 1, currentVSVarying));
            currentVSVarying += 1;
        }

        for (GLint i = 0; i < vec2ArrayCount; i++)
        {
            vertexShader->append(GenerateVectorVaryingSettingCode(2, 2, currentVSVarying));
            currentVSVarying += 1;
        }

        for (GLint i = 0; i < vec3Count; i++)
        {
            vertexShader->append(GenerateVectorVaryingSettingCode(3, 1, currentVSVarying));
            currentVSVarying += 1;
        }

        for (GLint i = 0; i < vec3ArrayCount; i++)
        {
            vertexShader->append(GenerateVectorVaryingSettingCode(3, 2, currentVSVarying));
            currentVSVarying += 1;
        }

        for (GLint i = 0; i < vec4Count; i++)
        {
            vertexShader->append(GenerateVectorVaryingSettingCode(4, 1, currentVSVarying));
            currentVSVarying += 1;
        }

        for (GLint i = 0; i < vec4ArrayCount; i++)
        {
            vertexShader->append(GenerateVectorVaryingSettingCode(4, 2, currentVSVarying));
            currentVSVarying += 1;
        }

        if (usePointSize)
        {
            vertexShader->append("gl_PointSize = 1.0;\n");
        }

        vertexShader->append("}\n");

        // Generate the fragment shader
        fragmentShader->clear();
        fragmentShader->append("precision highp float;\n");
        fragmentShader->append(varyingDeclaration);
        fragmentShader->append("\nvoid main() \n{ \n\tvec4 retColor = vec4(0,0,0,0);\n");

        unsigned int currentFSVarying = 0;

        // Make use of the float varyings
        fragmentShader->append("\tretColor += vec4(");

        for (GLint i = 0; i < floatCount; i++)
        {
            fragmentShader->append(GenerateVectorVaryingUseCode(1, currentFSVarying));
            currentFSVarying += 1;
        }

        for (GLint i = 0; i < floatArrayCount; i++)
        {
            fragmentShader->append(GenerateVectorVaryingUseCode(2, currentFSVarying));
            currentFSVarying += 1;
        }

        fragmentShader->append("0.0, 0.0, 0.0, 0.0);\n");

        // Make use of the vec2 varyings
        fragmentShader->append("\tretColor += vec4(");

        for (GLint i = 0; i < vec2Count; i++)
        {
            fragmentShader->append(GenerateVectorVaryingUseCode(1, currentFSVarying));
            currentFSVarying += 1;
        }

        for (GLint i = 0; i < vec2ArrayCount; i++)
        {
            fragmentShader->append(GenerateVectorVaryingUseCode(2, currentFSVarying));
            currentFSVarying += 1;
        }

        fragmentShader->append("vec2(0.0, 0.0), 0.0, 0.0);\n");

        // Make use of the vec3 varyings
        fragmentShader->append("\tretColor += vec4(");

        for (GLint i = 0; i < vec3Count; i++)
        {
            fragmentShader->append(GenerateVectorVaryingUseCode(1, currentFSVarying));
            currentFSVarying += 1;
        }

        for (GLint i = 0; i < vec3ArrayCount; i++)
        {
            fragmentShader->append(GenerateVectorVaryingUseCode(2, currentFSVarying));
            currentFSVarying += 1;
        }

        fragmentShader->append("vec3(0.0, 0.0, 0.0), 0.0);\n");

        // Make use of the vec4 varyings
        fragmentShader->append("\tretColor += ");

        for (GLint i = 0; i < vec4Count; i++)
        {
            fragmentShader->append(GenerateVectorVaryingUseCode(1, currentFSVarying));
            currentFSVarying += 1;
        }

        for (GLint i = 0; i < vec4ArrayCount; i++)
        {
            fragmentShader->append(GenerateVectorVaryingUseCode(2, currentFSVarying));
            currentFSVarying += 1;
        }

        fragmentShader->append("vec4(0.0, 0.0, 0.0, 0.0);\n");

        // Set gl_FragColor, and use special variables if requested
        fragmentShader->append("\tgl_FragColor = retColor");

        if (useFragCoord)
        {
            fragmentShader->append(" + gl_FragCoord");
        }

        if (usePointCoord)
        {
            fragmentShader->append(" + vec4(gl_PointCoord, 0.0, 0.0)");
        }

        fragmentShader->append(";\n}");
    }

    void VaryingTestBase(GLint floatCount,
                         GLint floatArrayCount,
                         GLint vec2Count,
                         GLint vec2ArrayCount,
                         GLint vec3Count,
                         GLint vec3ArrayCount,
                         GLint vec4Count,
                         GLint vec4ArrayCount,
                         bool useFragCoord,
                         bool usePointCoord,
                         bool usePointSize,
                         bool expectSuccess)
    {
        std::string fragmentShaderSource;
        std::string vertexShaderSource;

        GenerateGLSLWithVaryings(floatCount, floatArrayCount, vec2Count, vec2ArrayCount, vec3Count,
                                 vec3ArrayCount, vec4Count, vec4ArrayCount, useFragCoord,
                                 usePointCoord, usePointSize, &fragmentShaderSource,
                                 &vertexShaderSource);

        GLuint program = CompileProgram(vertexShaderSource.c_str(), fragmentShaderSource.c_str());

        if (expectSuccess)
        {
            EXPECT_NE(0u, program);
        }
        else
        {
            EXPECT_EQ(0u, program);
        }
    }

    void CompileGLSLWithUniformsAndSamplers(GLint vertexUniformCount,
                                            GLint fragmentUniformCount,
                                            GLint vertexSamplersCount,
                                            GLint fragmentSamplersCount,
                                            bool expectSuccess)
    {
        std::stringstream vertexShader;
        std::stringstream fragmentShader;

        // Generate the vertex shader
        vertexShader << "precision mediump float;\n";

        for (int i = 0; i < vertexUniformCount; i++)
        {
            vertexShader << "uniform vec4 v" << i << ";\n";
        }

        for (int i = 0; i < vertexSamplersCount; i++)
        {
            vertexShader << "uniform sampler2D s" << i << ";\n";
        }

        vertexShader << "void main()\n{\n";

        for (int i = 0; i < vertexUniformCount; i++)
        {
            vertexShader << "    gl_Position +=  v" << i << ";\n";
        }

        for (int i = 0; i < vertexSamplersCount; i++)
        {
            vertexShader << "    gl_Position +=  texture2D(s" << i << ", vec2(0.0, 0.0));\n";
        }

        if (vertexUniformCount == 0 && vertexSamplersCount == 0)
        {
            vertexShader << "   gl_Position = vec4(0.0);\n";
        }

        vertexShader << "}\n";

        // Generate the fragment shader
        fragmentShader << "precision mediump float;\n";

        for (int i = 0; i < fragmentUniformCount; i++)
        {
            fragmentShader << "uniform vec4 v" << i << ";\n";
        }

        for (int i = 0; i < fragmentSamplersCount; i++)
        {
            fragmentShader << "uniform sampler2D s" << i << ";\n";
        }

        fragmentShader << "void main()\n{\n";

        for (int i = 0; i < fragmentUniformCount; i++)
        {
            fragmentShader << "    gl_FragColor +=  v" << i << ";\n";
        }

        for (int i = 0; i < fragmentSamplersCount; i++)
        {
            fragmentShader << "    gl_FragColor +=  texture2D(s" << i << ", vec2(0.0, 0.0));\n";
        }

        if (fragmentUniformCount == 0 && fragmentSamplersCount == 0)
        {
            fragmentShader << "    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n";
        }

        fragmentShader << "}\n";

        GLuint program = CompileProgram(vertexShader.str().c_str(), fragmentShader.str().c_str());

        if (expectSuccess)
        {
            EXPECT_NE(0u, program);
        }
        else
        {
            EXPECT_EQ(0u, program);
        }
    }

    std::string QueryErrorMessage(GLuint program)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        EXPECT_GL_NO_ERROR();

        if (infoLogLength >= 1)
        {
            std::vector<GLchar> infoLog(infoLogLength);
            glGetProgramInfoLog(program, static_cast<GLsizei>(infoLog.size()), nullptr,
                                infoLog.data());
            EXPECT_GL_NO_ERROR();
            return infoLog.data();
        }

        return "";
    }

    void validateComponentsInErrorMessage(const char *vertexShader,
                                          const char *fragmentShader,
                                          const char *expectedErrorType,
                                          const char *expectedVariableFullName)
    {
        GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

        GLuint program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        glDetachShader(program, vs);
        glDetachShader(program, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);

        const std::string errorMessage = QueryErrorMessage(program);
        printf("%s\n", errorMessage.c_str());

        EXPECT_NE(std::string::npos, errorMessage.find(expectedErrorType));
        EXPECT_NE(std::string::npos, errorMessage.find(expectedVariableFullName));

        glDeleteProgram(program);
        ASSERT_GL_NO_ERROR();
    }

    void verifyAttachment2DColor(unsigned int index,
                                 GLuint textureName,
                                 GLenum target,
                                 GLint level,
                                 GLColor color)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, color)
            << "index " << index;
    }

    std::string ExpectedExtensionMacros(std::vector<std::string> expected)
    {
        std::string shader;
        for (const auto &ext : expected)
        {
            if (IsGLExtensionEnabled(ext))
            {
                shader += "\n#ifndef " + ext + "\n#error !defined(" + ext + ")\n#endif\n";
            }
        }
        return shader;
    }

    std::string UnexpectedExtensionMacros(std::vector<std::string> unexpected)
    {
        std::string shader;
        for (const auto &ext : unexpected)
        {
            shader += "\n#ifdef " + ext + "\n#error defined(" + ext + ")\n#endif\n";
        }
        return shader;
    }
};

class GLSLTestNoValidation : public GLSLTest
{
  public:
    GLSLTestNoValidation() { setNoErrorEnabled(true); }
};

class GLSLTest_ES3 : public GLSLTest
{};

class GLSLTest_ES31 : public GLSLTest
{
  protected:
    void testArrayOfArrayOfSamplerDynamicIndex(const APIExtensionVersion usedExtension);
    void testTessellationTextureBufferAccess(const APIExtensionVersion usedExtension);
};

// Tests the "init output variables" ANGLE shader translator option.
class GLSLTest_ES3_InitShaderVariables : public GLSLTest
{};
class GLSLTest_ES31_InitShaderVariables : public GLSLTest
{};

std::string BuildBigInitialStackShader(int length)
{
    std::string result;
    result += "void main() { \n";
    for (int i = 0; i < length; i++)
    {
        result += "  if (true) { \n";
    }
    result += "  int temp; \n";
    for (int i = 0; i <= length; i++)
    {
        result += "} \n";
    }
    return result;
}

// Tests a shader from conformance.olges/GL/build/build_017_to_024
// This shader uses chained assign-equals ops with swizzle, often reusing the same variable
// as part of a swizzle.
TEST_P(GLSLTest, SwizzledChainedAssignIncrement)
{
    constexpr char kFS[] =
        R"(
        precision mediump float;
        void main() {
            vec2 v = vec2(1,5);
            // at the end of next statement, values in
            // v.x = 12, v.y = 12
            v.xy += v.yx += v.xy;
            // v1 and v2, both are initialized with (12,12)
            vec2 v1 = v, v2 = v;
            v1.xy += v2.yx += ++(v.xy);  // v1 = 37, v2 = 25 each
            v1.xy += v2.yx += (v.xy)++;  // v1 = 75, v2 = 38 each
            gl_FragColor = vec4(v1,v2)/255.;  // 75, 75, 38, 38
        })";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(75, 75, 38, 38));
}

TEST_P(GLSLTest, NamelessScopedStructs)
{
    constexpr char kFS[] = R"(precision mediump float;
void main()
{
    struct
    {
        float q;
    } b;

    gl_FragColor = vec4(1, 0, 0, 1);
    gl_FragColor.a += b.q;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
}

TEST_P(GLSLTest_ES3, CompareEqualityOfArrayOfVectors)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 fragColor;
uniform vec3 a[3];
uniform vec3 b[3];
void main() {
  bool same = a == b;
  fragColor = vec4(0);
  if (same) {
    fragColor = vec4(1);
  }
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint aLocation = glGetUniformLocation(program, "a");
    GLint bLocation = glGetUniformLocation(program, "b");
    EXPECT_NE(aLocation, -1);
    EXPECT_NE(bLocation, -1);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    static float almostZeros[] = {0, 0, 0, 0, 0, 0, 0, 1, 0};
    glUniform3fv(bLocation, 9, almostZeros);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    glUniform3fv(aLocation, 9, almostZeros);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    ASSERT_GL_NO_ERROR();
}

TEST_P(GLSLTest_ES3, CompareEqualityOfArrayOfMatrices)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 fragColor;
uniform mat3 a[3];
uniform mat3 b[3];
void main() {
  bool same = a == b;
  fragColor = vec4(0);
  if (same) {
    fragColor = vec4(1);
  }
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint aLocation = glGetUniformLocation(program, "a");
    GLint bLocation = glGetUniformLocation(program, "b");
    EXPECT_NE(aLocation, -1);
    EXPECT_NE(bLocation, -1);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    static float almostZeros[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    };
    glUniformMatrix3fv(bLocation, 27, false, almostZeros);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    glUniformMatrix3fv(aLocation, 27, false, almostZeros);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    ASSERT_GL_NO_ERROR();
}

TEST_P(GLSLTest_ES3, CompareEqualityOfArrayOfFloats)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 fragColor;
uniform float a[3];
uniform float b[3];
void main() {
  bool same = a == b;
  fragColor = vec4(0);
  if (same) {
    fragColor = vec4(1);
  }
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint aLocation = glGetUniformLocation(program, "a");
    GLint bLocation = glGetUniformLocation(program, "b");
    EXPECT_NE(aLocation, -1);
    EXPECT_NE(bLocation, -1);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    static float almostZeros[] = {
        0,
        0,
        1,
    };
    glUniform1fv(bLocation, 3, almostZeros);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    glUniform1fv(aLocation, 3, almostZeros);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    ASSERT_GL_NO_ERROR();
}

TEST_P(GLSLTest_ES3, CompareInequalityOfArrayOfVectors)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 fragColor;
uniform vec3 a[3];
uniform vec3 b[3];
void main() {
  bool notSame = a != b;
  fragColor = vec4(0);
  if (notSame) {
    fragColor = vec4(1);
  }
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint aLocation = glGetUniformLocation(program, "a");
    GLint bLocation = glGetUniformLocation(program, "b");
    EXPECT_NE(aLocation, -1);
    EXPECT_NE(bLocation, -1);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    static float almostZeros[] = {0, 0, 0, 0, 0, 0, 0, 1, 0};
    glUniform3fv(bLocation, 9, almostZeros);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    glUniform3fv(aLocation, 9, almostZeros);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    ASSERT_GL_NO_ERROR();
}

TEST_P(GLSLTest_ES3, CompareInequalityOfArrayOfMatrices)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 fragColor;
uniform mat3 a[3];
uniform mat3 b[3];
void main() {
  bool notSame = a != b;
  fragColor = vec4(0);
  if (notSame) {
    fragColor = vec4(1);
  }
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint aLocation = glGetUniformLocation(program, "a");
    GLint bLocation = glGetUniformLocation(program, "b");
    EXPECT_NE(aLocation, -1);
    EXPECT_NE(bLocation, -1);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    static float almostZeros[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    };
    glUniformMatrix3fv(bLocation, 27, false, almostZeros);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    glUniformMatrix3fv(aLocation, 27, false, almostZeros);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    ASSERT_GL_NO_ERROR();
}

TEST_P(GLSLTest_ES3, CompareInequalityOfArrayOfFloats)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 fragColor;
uniform float a[3];
uniform float b[3];
void main() {
  bool notSame = a != b;
  fragColor = vec4(0);
  if (notSame) {
    fragColor = vec4(1);
  }
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint aLocation = glGetUniformLocation(program, "a");
    GLint bLocation = glGetUniformLocation(program, "b");
    EXPECT_NE(aLocation, -1);
    EXPECT_NE(bLocation, -1);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    static float almostZeros[] = {
        0,
        0,
        1,
    };
    glUniform1fv(bLocation, 3, almostZeros);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);

    glUniform1fv(aLocation, 3, almostZeros);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);

    ASSERT_GL_NO_ERROR();
}

// Test that array of fragment shader outputs is processed properly and draws
// E.g. was issue with "out vec4 frag_color[4];"
TEST_P(GLSLTest_ES3, FragmentShaderOutputArray)
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    GLuint textures[4];
    glGenTextures(4, textures);

    for (size_t texIndex = 0; texIndex < ArraySize(textures); texIndex++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }

    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    ASSERT_GE(maxDrawBuffers, 4);

    GLuint readFramebuffer;
    glGenFramebuffers(1, &readFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);

    constexpr char kFS[] = R"(#version 300 es
precision highp float;

out vec4 frag_color[4];

void main()
{
    frag_color[0] = vec4(1.0, 0.0, 0.0, 1.0);
    frag_color[1] = vec4(0.0, 1.0, 0.0, 1.0);
    frag_color[2] = vec4(0.0, 0.0, 1.0, 1.0);
    frag_color[3] = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    GLenum allBufs[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                         GL_COLOR_ATTACHMENT3};

    constexpr GLuint kMaxBuffers = 4;

    // Enable all draw buffers.
    for (GLuint texIndex = 0; texIndex < kMaxBuffers; texIndex++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + texIndex, GL_TEXTURE_2D,
                               textures[texIndex], 0);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + texIndex, GL_TEXTURE_2D,
                               textures[texIndex], 0);
    }
    glDrawBuffers(kMaxBuffers, allBufs);

    // Draw with simple program.
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    verifyAttachment2DColor(0, textures[0], GL_TEXTURE_2D, 0, GLColor::red);
    verifyAttachment2DColor(1, textures[1], GL_TEXTURE_2D, 0, GLColor::green);
    verifyAttachment2DColor(2, textures[2], GL_TEXTURE_2D, 0, GLColor::blue);
    verifyAttachment2DColor(3, textures[3], GL_TEXTURE_2D, 0, GLColor::white);
}

// Test that inactive fragment shader outputs don't cause a crash.
TEST_P(GLSLTest_ES3, InactiveFragmentShaderOutput)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;

// Make color0 inactive but specify color1 first.  The Vulkan backend assigns bogus locations when
// compiling and fixes it up in SPIR-V.  If color0's location is not fixed, it will return location
// 1 (aliasing color1).  This will lead to a Vulkan validation warning about attachment 0 not being
// written to, which shouldn't be fatal.
layout(location = 1) out vec4 color1;
layout(location = 0) out vec4 color0;

void main()
{
    color1 = vec4(0.0, 1.0, 0.0, 1.0);
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    constexpr GLint kDrawBufferCount = 2;

    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    ASSERT_GE(maxDrawBuffers, kDrawBufferCount);

    GLTexture textures[kDrawBufferCount];

    for (GLint texIndex = 0; texIndex < kDrawBufferCount; ++texIndex)
    {
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }

    GLenum allBufs[kDrawBufferCount] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

    GLFramebuffer fbo;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    // Enable all draw buffers.
    for (GLint texIndex = 0; texIndex < kDrawBufferCount; ++texIndex)
    {
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + texIndex, GL_TEXTURE_2D,
                               textures[texIndex], 0);
    }
    glDrawBuffers(kDrawBufferCount, allBufs);

    // Draw with simple program.
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
}

TEST_P(GLSLTest, ScopedStructsOrderBug)
{
    constexpr char kFS[] = R"(precision mediump float;

struct T
{
    float f;
};

void main()
{
    T a;

    struct T
    {
        float q;
    };

    T b;

    gl_FragColor = vec4(1, 0, 0, 1);
    gl_FragColor.a += a.f;
    gl_FragColor.a += b.q;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
}

// Test that defining a struct together with an inactive uniform, then using it in a scope that has
// another struct with the same name declared works.
TEST_P(GLSLTest, ScopedStructsOrderBug2)
{
    constexpr char kFS[] = R"(precision mediump float;

uniform struct T
{
    float f;
} x;

void main()
{
    T a;

    struct T
    {
        float q;
    };

    T b;

    gl_FragColor = vec4(1, 0, 0, 1);
    gl_FragColor.a += a.f;
    gl_FragColor.a += b.q;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
}

// Test that inactive uniforms of struct type don't cause any errors.
TEST_P(GLSLTest, InactiveStructUniform)
{
    constexpr char kVS[] = R"(
uniform struct
{
    vec4 c;
} s;
void main()
{
})";

    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
}

// Test that struct with same name can be declared in inner scope.
TEST_P(GLSLTest, SameNameStructInInnerScope)
{
    constexpr char kVS[] = R"(
void main() {
    gl_Position = vec4(0);
})";

    constexpr char kFS[] = R"(
struct S
{
    mediump float f;
};
void main()
{
    struct S
    {
        S n;
    };
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Regression test based on WebGL's conformance/glsl/misc/empty-declaration.html
TEST_P(GLSLTest, StructEmptyDeclaratorBug)
{
    constexpr char kVS[] = R"(
struct S {
    float member;
}, a;
void main() {
    a.member = 0.0;
    gl_Position = vec4(a.member);
})";

    constexpr char kFS[] = R"(precision mediump float;
void main()
{
    gl_FragColor = vec4(1.0,0.0,0.0,1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Regression test based on WebGL's conformance/ogles/GL/build/build_001_to_008.html
TEST_P(GLSLTest, StructConstantFoldingBug)
{
    constexpr char kVS[] = R"(
void main()
{

   const struct s2 {
       int i;
       vec3 v3;
       bvec4 bv4;
   } s22  = s2(8, vec3(9, 10, 11), bvec4(true, false, true, false));

   struct s4 {
       int ii;
       vec4 v4;
      };

   const struct s1 {
      s2 ss;
      int i;
      float f;
      mat4 m;
      s4 s44;
   } s11 = s1(s22, 2, 4.0, mat4(5), s4(6, vec4(7, 8, 9, 10))) ;

  const int field3 = s11.i * s11.ss.i;  // constant folding (int * int)
  const vec4 field4 = s11.s44.v4 * s11.s44.v4; // constant folding (vec4 * vec4)
 // 49, 64, 81, 100
  const vec4 v4 = vec4(s11.ss.v3.y, s11.m[3][3], field3, field4[2]);  // 10.0, 5.0, 16.0, 81.0
  gl_Position = v4;
})";

    constexpr char kFS[] = R"(precision mediump float;
precision mediump float;
void main()
{
    gl_FragColor = vec4(1.0,0.0,0.0,1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that constant folding doesn't remove struct declaration.
TEST_P(GLSLTest, StructConstantFoldingBug2)
{
    constexpr char kVS[] = R"(
uniform vec4 u;

void main()
{

   const struct s2 {
       int i;
       vec3 v3;
       bvec4 bv4;
   } s22  = s2(8, vec3(9, 10, 11), bvec4(true, false, true, false));

   s2 x;
   x.v3 = u.xyz;

   gl_Position = vec4(x.v3, float(s22.i));
})";

    constexpr char kFS[] = R"(precision mediump float;
precision mediump float;
void main()
{
    gl_FragColor = vec4(1.0,0.0,0.0,1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

TEST_P(GLSLTest, ScopedStructsBug)
{
    constexpr char kFS[] = R"(precision mediump float;

struct T_0
{
    float f;
};

void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);

    struct T
    {
        vec2 v;
    };

    T_0 a;
    T b;

    gl_FragColor.a += a.f;
    gl_FragColor.a += b.v.x;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
}

TEST_P(GLSLTest, DxPositionBug)
{
    constexpr char kVS[] = R"(attribute vec4 inputAttribute;
varying float dx_Position;
void main()
{
    gl_Position = vec4(inputAttribute);
    dx_Position = 0.0;
})";

    constexpr char kFS[] = R"(precision mediump float;

varying float dx_Position;

void main()
{
    gl_FragColor = vec4(dx_Position, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Draw an array of points with the first vertex offset at 0 using gl_VertexID
TEST_P(GLSLTest_ES3, GLVertexIDOffsetZeroDrawArray)
{
    constexpr int kStartIndex  = 0;
    constexpr int kArrayLength = 5;
    constexpr char kVS[]       = R"(#version 300 es
precision highp float;
void main() {
    gl_Position = vec4(float(gl_VertexID)/10.0, 0, 0, 1);
    gl_PointSize = 3.0;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;
void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glUseProgram(program);
    glDrawArrays(GL_POINTS, kStartIndex, kArrayLength);

    double pointCenterX = static_cast<double>(getWindowWidth()) / 2.0;
    double pointCenterY = static_cast<double>(getWindowHeight()) / 2.0;
    for (int i = kStartIndex; i < kStartIndex + kArrayLength; i++)
    {
        double pointOffsetX = static_cast<double>(i * getWindowWidth()) / 20.0;
        EXPECT_PIXEL_COLOR_EQ(static_cast<int>(pointCenterX + pointOffsetX),
                              static_cast<int>(pointCenterY), GLColor::red);
    }
}

GLint GetFirstIntPixelRedValue()
{
    GLint pixel[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, pixel);
    return pixel[0];
}

TEST_P(GLSLTest_ES3, GLVertexIDIntegerTextureDrawElements)
{
    constexpr char kVS[] = R"(#version 300 es
    flat out highp int vVertexID;

    void main() {
        vVertexID = gl_VertexID;
        gl_PointSize = 1.0;
        gl_Position = vec4(0,0,0,1);
    })";

    constexpr char kFS[] = R"(#version 300 es
    flat in highp int vVertexID;
    out highp int oVertexID;
    void main() {
        oVertexID = vVertexID;
    })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glViewport(0, 0, 1, 1);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, 1, 1);
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    EXPECT_GL_NO_ERROR();

    GLint clearData[4] = {42};
    glClearBufferiv(GL_COLOR, 0, clearData);
    EXPECT_EQ(42, GetFirstIntPixelRedValue());

    const int kIndexDataSize = 5;
    GLushort indexData[]     = {1, 2, 5, 3, 10000};
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

    for (size_t first = 0; first < kIndexDataSize; ++first)
    {
        for (size_t count = 1; first + count <= kIndexDataSize; ++count)
        {
            glDrawElements(GL_POINTS, count, GL_UNSIGNED_SHORT,
                           reinterpret_cast<const void *>(first * 2u));
            GLint expected = indexData[first + count - 1];
            GLint actual   = GetFirstIntPixelRedValue();
            EXPECT_EQ(expected, actual);
        }
    }
    EXPECT_GL_NO_ERROR();
}

TEST_P(GLSLTest_ES3, GLVertexIDIntegerTextureDrawElementsU8)
{
    constexpr char kVS[] = R"(#version 300 es
    flat out highp int vVertexID;

    void main() {
        vVertexID = gl_VertexID;
        gl_PointSize = 1.0;
        gl_Position = vec4(0,0,0,1);
    })";

    constexpr char kFS[] = R"(#version 300 es
    flat in highp int vVertexID;
    out highp int oVertexID;
    void main() {
        oVertexID = vVertexID;
    })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glViewport(0, 0, 1, 1);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, 1, 1);
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    EXPECT_GL_NO_ERROR();

    GLint clearData[4] = {42};
    glClearBufferiv(GL_COLOR, 0, clearData);
    EXPECT_EQ(42, GetFirstIntPixelRedValue());

    const int kIndexDataSize = 5;
    GLubyte indexData[]      = {1, 2, 5, 3, 100};
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

    for (size_t first = 0; first < kIndexDataSize; ++first)
    {
        for (size_t count = 1; first + count <= kIndexDataSize; ++count)
        {
            glDrawElements(GL_POINTS, count, GL_UNSIGNED_BYTE,
                           reinterpret_cast<const void *>(first));
            GLint expected = indexData[first + count - 1];
            GLint actual   = GetFirstIntPixelRedValue();
            EXPECT_EQ(expected, actual);
        }
    }
    EXPECT_GL_NO_ERROR();
}

void GLVertexIDIntegerTextureDrawElementsU8Line_Helper(size_t first, const GLubyte *indices)
{
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(first));

    GLint pixels[8];
    glReadPixels(0, 0, 2, 1, GL_RGBA_INTEGER, GL_INT, pixels);

    GLint expected = indices[first + 1];
    EXPECT_EQ(expected, pixels[0]);
    EXPECT_EQ(expected, pixels[4]);
}

TEST_P(GLSLTest_ES3, GLVertexIDIntegerTextureDrawElementsU8Line)
{
    constexpr char kVS[] = R"(#version 300 es
    flat out highp int vVertexID;
    layout(location = 0) in vec4 position;

    void main() {
        vVertexID = gl_VertexID;
        gl_Position = position;
    })";

    constexpr char kFS[] = R"(#version 300 es
    flat in highp int vVertexID;
    out highp int oVertexID;
    void main() {
        oVertexID = vVertexID;
    })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glViewport(0, 0, 2, 1);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, 2, 1);
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    EXPECT_GL_NO_ERROR();

    struct LR
    {
        LR() : X0(-1.0f), X1(1.0f) {}
        float X0;
        float X1;
    };
    constexpr int kNumVertices = 100;
    LR vertData[kNumVertices];
    GLBuffer buf;
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertData), vertData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLint clearData[4] = {42};
    glClearBufferiv(GL_COLOR, 0, clearData);
    EXPECT_EQ(42, GetFirstIntPixelRedValue());

    GLubyte indexData[] = {1, 4, 5, 2, 50, 61};
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

    GLVertexIDIntegerTextureDrawElementsU8Line_Helper(0, indexData);
    GLVertexIDIntegerTextureDrawElementsU8Line_Helper(1, indexData);
    GLVertexIDIntegerTextureDrawElementsU8Line_Helper(2, indexData);
    GLVertexIDIntegerTextureDrawElementsU8Line_Helper(4, indexData);

    EXPECT_GL_NO_ERROR();
}

// Test gl_VertexID works with lines
TEST_P(GLSLTest_ES3, GLVertexIDIntegerTextureDrawElementsU8LineIds)
{
    // Draws lines via indices (glDrawElements). Each pair of indices
    // draws the next consecutive pixel. For 2 points, because we're
    // using int attributes, they must be "flat" and so the spec
    // says for a given line the value should come from the second
    // of the 2 points. (see: OpenGL ES 3.0.2 spec Table 2.12)
    // Each line is only 1 pixel long so every other pixel should
    // be the default value.
    constexpr char kVS[] = R"(#version 300 es
    flat out highp int vVertexID;
    layout(location = 0) in float position;
    uniform float width;

    void main() {
        vVertexID = gl_VertexID;
        gl_Position = vec4(position / width * 2.0 - 1.0, 0, 0, 1);
    })";

    constexpr char kFS[] = R"(#version 300 es
    flat in highp int vVertexID;
    out highp int oVertexID;
    void main() {
        oVertexID = vVertexID;
    })";

    GLubyte indexData[]          = {1, 4, 5, 2, 50, 61, 32, 33};
    constexpr size_t kNumIndices = sizeof(indexData) / sizeof(indexData[0]);
    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "width"), kNumIndices);
    glViewport(0, 0, kNumIndices, 1);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, kNumIndices, 1);
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    constexpr int kNumVertices = 100;
    std::vector<float> vertData(kNumVertices, -1.0f);
    {
        int i = 0;
        for (GLubyte ndx : indexData)
        {
            vertData[ndx] = i++;
        }
    }
    GLBuffer buf;
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, vertData.size() * sizeof(float), vertData.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLint kDefaultValue = 42;
    GLint clearData[4]  = {kDefaultValue};
    glClearBufferiv(GL_COLOR, 0, clearData);
    EXPECT_EQ(kDefaultValue, GetFirstIntPixelRedValue());

    EXPECT_GL_NO_ERROR();

    glDrawElements(GL_LINES, kNumIndices, GL_UNSIGNED_BYTE, 0);

    GLint pixels[kNumIndices * 4];
    glReadPixels(0, 0, kNumIndices, 1, GL_RGBA_INTEGER, GL_INT, pixels);

    for (size_t i = 0; i < kNumIndices; ++i)
    {
        const int expected = i % 2 ? kDefaultValue : indexData[i + 1];
        const int actual   = pixels[i * 4];
        EXPECT_EQ(expected, actual);
    }

    EXPECT_GL_NO_ERROR();
}

// Helper function for the GLVertexIDIntegerTextureDrawArrays test
void GLVertexIDIntegerTextureDrawArrays_helper(int first, int count, GLenum err)
{
    glDrawArrays(GL_POINTS, first, count);

    int pixel[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, pixel);
    // If we call this function with err as GL_NO_ERROR, then we expect no error and check the
    // pixels.
    if (err == static_cast<GLenum>(GL_NO_ERROR))
    {
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(pixel[0], first + count - 1);
    }
    else
    {
        // If we call this function with err set, we will allow the error, but check the pixels if
        // the error hasn't occurred.
        GLenum glError = glGetError();
        if (glError == err || glError == static_cast<GLenum>(GL_NO_ERROR))
        {
            EXPECT_EQ(pixel[0], first + count - 1);
        }
    }
}

// Ensure gl_VertexID gets passed to an integer texture properly when drawArrays is called. This
// is based off the WebGL test:
// https://github.com/KhronosGroup/WebGL/blob/master/sdk/tests/conformance2/rendering/vertex-id.html
TEST_P(GLSLTest_ES3, GLVertexIDIntegerTextureDrawArrays)
{
    constexpr char kVS[] = R"(#version 300 es
flat out highp int vVertexID;
void main() {
    vVertexID = gl_VertexID;
    gl_Position = vec4(0,0,0,1);
    gl_PointSize = 1.0;
})";

    constexpr char kFS[] = R"(#version 300 es
flat in highp int vVertexID;
out highp int oVertexID;
void main() {
    oVertexID = vVertexID;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glViewport(0, 0, 1, 1);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32I, 1, 1);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    EXPECT_GL_NO_ERROR();

    // Clear the texture to 42 to ensure the first test case doesn't accidentally pass
    GLint val[4] = {42};
    glClearBufferiv(GL_COLOR, 0, val);
    int pixel[4];
    glReadPixels(0, 0, 1, 1, GL_RGBA_INTEGER, GL_INT, pixel);
    EXPECT_EQ(pixel[0], val[0]);

    GLVertexIDIntegerTextureDrawArrays_helper(0, 1, GL_NO_ERROR);
    GLVertexIDIntegerTextureDrawArrays_helper(1, 1, GL_NO_ERROR);
    GLVertexIDIntegerTextureDrawArrays_helper(10000, 1, GL_NO_ERROR);
    GLVertexIDIntegerTextureDrawArrays_helper(100000, 1, GL_NO_ERROR);
    GLVertexIDIntegerTextureDrawArrays_helper(1000000, 1, GL_NO_ERROR);
    GLVertexIDIntegerTextureDrawArrays_helper(0, 2, GL_NO_ERROR);
    GLVertexIDIntegerTextureDrawArrays_helper(1, 2, GL_NO_ERROR);
    GLVertexIDIntegerTextureDrawArrays_helper(10000, 2, GL_NO_ERROR);
    GLVertexIDIntegerTextureDrawArrays_helper(100000, 2, GL_NO_ERROR);
    GLVertexIDIntegerTextureDrawArrays_helper(1000000, 2, GL_NO_ERROR);

    int32_t int32Max = 0x7FFFFFFF;
    GLVertexIDIntegerTextureDrawArrays_helper(int32Max - 2, 1, GL_OUT_OF_MEMORY);
    GLVertexIDIntegerTextureDrawArrays_helper(int32Max - 1, 1, GL_OUT_OF_MEMORY);
}

// Draw an array of points with the first vertex offset at 5 using gl_VertexID
TEST_P(GLSLTest_ES3, GLVertexIDOffsetFiveDrawArray)
{
    // Bug in Nexus drivers, offset does not work. (anglebug.com/42261941)
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsOpenGLES());

    constexpr int kStartIndex  = 5;
    constexpr int kArrayLength = 5;
    constexpr char kVS[]       = R"(#version 300 es
precision highp float;
void main() {
    gl_Position = vec4(float(gl_VertexID)/10.0, 0, 0, 1);
    gl_PointSize = 3.0;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;
void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glUseProgram(program);
    glDrawArrays(GL_POINTS, kStartIndex, kArrayLength);

    double pointCenterX = static_cast<double>(getWindowWidth()) / 2.0;
    double pointCenterY = static_cast<double>(getWindowHeight()) / 2.0;
    for (int i = kStartIndex; i < kStartIndex + kArrayLength; i++)
    {
        double pointOffsetX = static_cast<double>(i * getWindowWidth()) / 20.0;
        EXPECT_PIXEL_COLOR_EQ(static_cast<int>(pointCenterX + pointOffsetX),
                              static_cast<int>(pointCenterY), GLColor::red);
    }
}

TEST_P(GLSLTest, ElseIfRewriting)
{
    constexpr char kVS[] =
        "attribute vec4 a_position;\n"
        "varying float v;\n"
        "void main() {\n"
        "  gl_Position = a_position;\n"
        "  v = 1.0;\n"
        "  if (a_position.x <= 0.5) {\n"
        "    v = 0.0;\n"
        "  } else if (a_position.x >= 0.5) {\n"
        "    v = 2.0;\n"
        "  }\n"
        "}\n";

    constexpr char kFS[] =
        "precision highp float;\n"
        "varying float v;\n"
        "void main() {\n"
        "  vec4 color = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "  if (v >= 1.0) color = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "  if (v >= 2.0) color = vec4(0.0, 0.0, 1.0, 1.0);\n"
        "  gl_FragColor = color;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    drawQuad(program, "a_position", 0.5f);

    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);
    EXPECT_PIXEL_EQ(getWindowWidth() - 1, 0, 0, 255, 0, 255);
}

TEST_P(GLSLTest, TwoElseIfRewriting)
{
    constexpr char kVS[] =
        "attribute vec4 a_position;\n"
        "varying float v;\n"
        "void main() {\n"
        "  gl_Position = a_position;\n"
        "  if (a_position.x == 0.0) {\n"
        "    v = 1.0;\n"
        "  } else if (a_position.x > 0.5) {\n"
        "    v = 0.0;\n"
        "  } else if (a_position.x > 0.75) {\n"
        "    v = 0.5;\n"
        "  }\n"
        "}\n";

    constexpr char kFS[] =
        "precision highp float;\n"
        "varying float v;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(v, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

TEST_P(GLSLTest, FrontFacingAndVarying)
{
    EGLPlatformParameters platform = GetParam().eglParameters;

    constexpr char kVS[] = R"(attribute vec4 a_position;
varying float v_varying;
void main()
{
    v_varying = a_position.x;
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying float v_varying;
void main()
{
    vec4 c;

    if (gl_FrontFacing)
    {
        c = vec4(v_varying, 0, 0, 1.0);
    }
    else
    {
        c = vec4(0, v_varying, 0, 1.0);
    }
    gl_FragColor = c;
})";

    GLuint program = CompileProgram(kVS, kFS);

    // Compilation should fail on D3D11 feature level 9_3, since gl_FrontFacing isn't supported.
    if (platform.renderer == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        if (platform.majorVersion == 9 && platform.minorVersion == 3)
        {
            EXPECT_EQ(0u, program);
            return;
        }
    }

    // Otherwise, compilation should succeed
    EXPECT_NE(0u, program);
}

// Test that we can release the shader compiler and still compile things properly.
TEST_P(GLSLTest, ReleaseCompilerThenCompile)
{
    // Draw with the first program.
    ANGLE_GL_PROGRAM(program1, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program1, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Clear and release shader compiler.
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    glReleaseShaderCompiler();
    ASSERT_GL_NO_ERROR();

    // Draw with a second program.
    ANGLE_GL_PROGRAM(program2, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    drawQuad(program2, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Verify that linking shaders declaring different shading language versions fails.
TEST_P(GLSLTest_ES3, VersionMismatch)
{
    GLuint program = CompileProgram(essl3_shaders::vs::Simple(), essl1_shaders::fs::Red());
    EXPECT_EQ(0u, program);

    program = CompileProgram(essl1_shaders::vs::Simple(), essl3_shaders::fs::Red());
    EXPECT_EQ(0u, program);
}

// Verify that declaring varying as invariant only in vertex shader fails in ESSL 1.00.
TEST_P(GLSLTest, InvariantVaryingOut)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "varying float v_varying;\n"
        "void main() { gl_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "attribute vec4 a_position;\n"
        "invariant varying float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Verify that declaring varying as invariant only in vertex shader succeeds in ESSL 3.00.
TEST_P(GLSLTest_ES3, InvariantVaryingOut)
{
    // TODO: ESSL 3.00 -> GLSL 1.20 translation should add "invariant" in fragment shader
    // for varyings which are invariant in vertex shader (http://anglebug.com/40096344)
    ANGLE_SKIP_TEST_IF(IsDesktopOpenGL());

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in float v_varying;\n"
        "out vec4 my_FragColor;\n"
        "void main() { my_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 a_position;\n"
        "invariant out float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

// Verify that declaring varying as invariant only in fragment shader fails in ESSL 1.00.
TEST_P(GLSLTest, InvariantVaryingIn)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "invariant varying float v_varying;\n"
        "void main() { gl_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "attribute vec4 a_position;\n"
        "varying float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Verify that declaring varying as invariant only in fragment shader fails in ESSL 3.00.
TEST_P(GLSLTest_ES3, InvariantVaryingIn)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "invariant in float v_varying;\n"
        "out vec4 my_FragColor;\n"
        "void main() { my_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 a_position;\n"
        "out float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Verify that declaring varying as invariant in both shaders succeeds in ESSL 1.00.
TEST_P(GLSLTest, InvariantVaryingBoth)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "invariant varying float v_varying;\n"
        "void main() { gl_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "attribute vec4 a_position;\n"
        "invariant varying float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

// Verify that declaring varying as invariant in both shaders fails in ESSL 3.00.
TEST_P(GLSLTest_ES3, InvariantVaryingBoth)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "invariant in float v_varying;\n"
        "out vec4 my_FragColor;\n"
        "void main() { my_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 a_position;\n"
        "invariant out float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Verify that declaring gl_Position as invariant succeeds in ESSL 1.00.
TEST_P(GLSLTest, InvariantGLPosition)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "varying float v_varying;\n"
        "void main() { gl_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "attribute vec4 a_position;\n"
        "invariant gl_Position;\n"
        "varying float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

// Verify that declaring gl_Position as invariant succeeds in ESSL 3.00.
TEST_P(GLSLTest_ES3, InvariantGLPosition)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in float v_varying;\n"
        "out vec4 my_FragColor;\n"
        "void main() { my_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 a_position;\n"
        "invariant gl_Position;\n"
        "out float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

// Verify that using invariant(all) in both shaders fails in ESSL 1.00.
TEST_P(GLSLTest, InvariantAllBoth)
{
    constexpr char kFS[] =
        "#pragma STDGL invariant(all)\n"
        "precision mediump float;\n"
        "varying float v_varying;\n"
        "void main() { gl_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "#pragma STDGL invariant(all)\n"
        "attribute vec4 a_position;\n"
        "varying float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Verify that using a struct as both invariant and non-invariant output works.
TEST_P(GLSLTest_ES31, StructBothInvariantAndNot)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require

struct S
{
    vec4 s;
};

out Output
{
    vec4 x;
    invariant S s;
};

out S s2;

void main(){
    x = vec4(0);
    s.s = vec4(1);
    s2.s = vec4(2);
    S s3 = s;
    s.s = s3.s;
})";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Verify that using maximum size as atomic counter offset results in compilation failure.
TEST_P(GLSLTest_ES31, CompileWithMaxAtomicCounterOffsetFails)
{
    GLint maxSize;
    glGetIntegerv(GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE, &maxSize);

    std::ostringstream srcStream;
    srcStream << "#version 310 es\n"
              << "layout(location = 0) out uvec4 color;\n"
              << "layout(binding = 0, offset = " << maxSize << ") uniform atomic_uint a_counter;\n"
              << "void main() {\n"
              << "color = uvec4(atomicCounterIncrement(a_counter)); \n"
              << "}";
    std::string fsStream = srcStream.str();
    const char *strFS    = fsStream.c_str();

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, strFS);
    EXPECT_EQ(0u, shader);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest, MissingReturnFloat)
{
    constexpr char kVS[] =
        "varying float v_varying;\n"
        "float f() { if (v_varying > 0.0) return 1.0; }\n"
        "void main() { gl_Position = vec4(f(), 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl1_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest, MissingReturnVec2)
{
    constexpr char kVS[] =
        "varying float v_varying;\n"
        "vec2 f() { if (v_varying > 0.0) return vec2(1.0, 1.0); }\n"
        "void main() { gl_Position = vec4(f().x, 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl1_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest, MissingReturnVec3)
{
    constexpr char kVS[] =
        "varying float v_varying;\n"
        "vec3 f() { if (v_varying > 0.0) return vec3(1.0, 1.0, 1.0); }\n"
        "void main() { gl_Position = vec4(f().x, 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl1_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest, MissingReturnVec4)
{
    constexpr char kVS[] =
        "varying float v_varying;\n"
        "vec4 f() { if (v_varying > 0.0) return vec4(1.0, 1.0, 1.0, 1.0); }\n"
        "void main() { gl_Position = vec4(f().x, 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl1_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest, MissingReturnIVec4)
{
    constexpr char kVS[] =
        "varying float v_varying;\n"
        "ivec4 f() { if (v_varying > 0.0) return ivec4(1, 1, 1, 1); }\n"
        "void main() { gl_Position = vec4(f().x, 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl1_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest, MissingReturnMat4)
{
    constexpr char kVS[] =
        "varying float v_varying;\n"
        "mat4 f() { if (v_varying > 0.0) return mat4(1.0); }\n"
        "void main() { gl_Position = vec4(f()[0][0], 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl1_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest, MissingReturnStruct)
{
    constexpr char kVS[] =
        "varying float v_varying;\n"
        "struct s { float a; int b; vec2 c; };\n"
        "s f() { if (v_varying > 0.0) return s(1.0, 1, vec2(1.0, 1.0)); }\n"
        "void main() { gl_Position = vec4(f().a, 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl1_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest_ES3, MissingReturnArray)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in float v_varying;\n"
        "vec2[2] f() { if (v_varying > 0.0) { return vec2[2](vec2(1.0, 1.0), vec2(1.0, 1.0)); } }\n"
        "void main() { gl_Position = vec4(f()[0].x, 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl3_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest_ES3, MissingReturnArrayOfStructs)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in float v_varying;\n"
        "struct s { float a; int b; vec2 c; };\n"
        "s[2] f() { if (v_varying > 0.0) { return s[2](s(1.0, 1, vec2(1.0, 1.0)), s(1.0, 1, "
        "vec2(1.0, 1.0))); } }\n"
        "void main() { gl_Position = vec4(f()[0].a, 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl3_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that functions without return statements still compile
TEST_P(GLSLTest_ES3, MissingReturnStructOfArrays)
{
    // TODO(crbug.com/998505): Test failing on Android FYI Release (NVIDIA Shield TV)
    ANGLE_SKIP_TEST_IF(IsNVIDIAShield());

    constexpr char kVS[] =
        "#version 300 es\n"
        "in float v_varying;\n"
        "struct s { float a[2]; int b[2]; vec2 c[2]; };\n"
        "s f() { if (v_varying > 0.0) { return s(float[2](1.0, 1.0), int[2](1, 1),"
        "vec2[2](vec2(1.0, 1.0), vec2(1.0, 1.0))); } }\n"
        "void main() { gl_Position = vec4(f().a[0], 0, 0, 1); }\n";

    GLuint program = CompileProgram(kVS, essl3_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that non-const index used on an array returned by a function compiles
TEST_P(GLSLTest_ES3, ReturnArrayOfStructsThenNonConstIndex)
{
    constexpr char kVS[] = R"(#version 300 es
in float v_varying;
struct s { float a; int b; vec2 c; };
s[2] f()
{
    return s[2](s(v_varying, 1, vec2(1.0, 1.0)), s(v_varying / 2.0, 1, vec2(1.0, 1.0)));
}
void main()
{
    gl_Position = vec4(f()[uint(v_varying)].a, 0, 0, 1);
})";

    GLuint program = CompileProgram(kVS, essl3_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Verify that using invariant(all) in both shaders fails in ESSL 3.00.
TEST_P(GLSLTest_ES3, InvariantAllBoth)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "#pragma STDGL invariant(all)\n"
        "precision mediump float;\n"
        "in float v_varying;\n"
        "out vec4 my_FragColor;\n"
        "void main() { my_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "#version 300 es\n"
        "#pragma STDGL invariant(all)\n"
        "in vec4 a_position;\n"
        "out float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Verify that using invariant(all) only in fragment shader succeeds in ESSL 1.00.
TEST_P(GLSLTest, InvariantAllIn)
{
    constexpr char kFS[] =
        "#pragma STDGL invariant(all)\n"
        "precision mediump float;\n"
        "varying float v_varying;\n"
        "void main() { gl_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "attribute vec4 a_position;\n"
        "varying float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

// Verify that using invariant(all) only in fragment shader fails in ESSL 3.00.
TEST_P(GLSLTest_ES3, InvariantAllIn)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "#pragma STDGL invariant(all)\n"
        "precision mediump float;\n"
        "in float v_varying;\n"
        "out vec4 my_FragColor;\n"
        "void main() { my_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 a_position;\n"
        "out float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Verify that using invariant(all) only in vertex shader fails in ESSL 1.00.
TEST_P(GLSLTest, InvariantAllOut)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "varying float v_varying;\n"
        "void main() { gl_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "#pragma STDGL invariant(all)\n"
        "attribute vec4 a_position;\n"
        "varying float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Verify that using invariant(all) only in vertex shader succeeds in ESSL 3.00.
TEST_P(GLSLTest_ES3, InvariantAllOut)
{
    // TODO: ESSL 3.00 -> GLSL 1.20 translation should add "invariant" in fragment shader
    // for varyings which are invariant in vertex shader,
    // because of invariant(all) being used in vertex shader (http://anglebug.com/40096344)
    ANGLE_SKIP_TEST_IF(IsDesktopOpenGL());

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in float v_varying;\n"
        "out vec4 my_FragColor;\n"
        "void main() { my_FragColor = vec4(v_varying, 0, 0, 1.0); }\n";

    constexpr char kVS[] =
        "#version 300 es\n"
        "#pragma STDGL invariant(all)\n"
        "in vec4 a_position;\n"
        "out float v_varying;\n"
        "void main() { v_varying = a_position.x; gl_Position = a_position; }\n";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

TEST_P(GLSLTest, MaxVaryingVec4)
{
    // TODO(geofflang): Find out why this doesn't compile on Apple AMD OpenGL drivers
    // (http://anglebug.com/42260302)
    ANGLE_SKIP_TEST_IF(IsMac() && IsAMD() && IsOpenGL());

    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 0, 0, 0, 0, maxVaryings, 0, false, false, false, true);
}

// Verify we can pack registers with one builtin varying.
TEST_P(GLSLTest, MaxVaryingVec4_OneBuiltin)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    // Generate shader code that uses gl_FragCoord.
    VaryingTestBase(0, 0, 0, 0, 0, 0, maxVaryings - 1, 0, true, false, false, true);
}

// Verify we can pack registers with two builtin varyings.
TEST_P(GLSLTest, MaxVaryingVec4_TwoBuiltins)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    // Generate shader code that uses gl_FragCoord and gl_PointCoord.
    VaryingTestBase(0, 0, 0, 0, 0, 0, maxVaryings - 2, 0, true, true, false, true);
}

// Verify we can pack registers with three builtin varyings.
TEST_P(GLSLTest, MaxVaryingVec4_ThreeBuiltins)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    // Generate shader code that uses gl_FragCoord, gl_PointCoord and gl_PointSize.
    VaryingTestBase(0, 0, 0, 0, 0, 0, maxVaryings - 3, 0, true, true, true, true);
}

// This covers a problematic case in D3D9 - we are limited by the number of available semantics,
// rather than total register use.
TEST_P(GLSLTest, MaxVaryingsSpecialCases)
{
    ANGLE_SKIP_TEST_IF(!IsD3D9());

    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(maxVaryings, 0, 0, 0, 0, 0, 0, 0, true, false, false, false);
    VaryingTestBase(maxVaryings - 1, 0, 0, 0, 0, 0, 0, 0, true, true, false, false);
    VaryingTestBase(maxVaryings - 2, 0, 0, 0, 0, 0, 0, 0, true, true, false, true);

    // Special case for gl_PointSize: we get it for free on D3D9.
    VaryingTestBase(maxVaryings - 2, 0, 0, 0, 0, 0, 0, 0, true, true, true, true);
}

// This covers a problematic case in D3D9 - we are limited by the number of available semantics,
// rather than total register use.
TEST_P(GLSLTest, MaxMinusTwoVaryingVec2PlusOneSpecialVariable)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    // Generate shader code that uses gl_FragCoord.
    VaryingTestBase(0, 0, maxVaryings, 0, 0, 0, 0, 0, true, false, false, !IsD3D9());
}

TEST_P(GLSLTest, MaxVaryingVec3)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 0, 0, maxVaryings, 0, 0, 0, false, false, false, true);
}

TEST_P(GLSLTest, MaxVaryingVec3Array)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 0, 0, 0, maxVaryings / 2, 0, 0, false, false, false, true);
}

// Only fails on D3D9 because of packing limitations.
TEST_P(GLSLTest, MaxVaryingVec3AndOneFloat)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(1, 0, 0, 0, maxVaryings, 0, 0, 0, false, false, false, !IsD3D9());
}

// Only fails on D3D9 because of packing limitations.
TEST_P(GLSLTest, MaxVaryingVec3ArrayAndOneFloatArray)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 1, 0, 0, 0, maxVaryings / 2, 0, 0, false, false, false, !IsD3D9());
}

// Only fails on D3D9 because of packing limitations.
TEST_P(GLSLTest, TwiceMaxVaryingVec2)
{
    // TODO(geofflang): Figure out why this fails on NVIDIA's GLES driver
    // (http://anglebug.com/42262492)
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGLES());

    // TODO(geofflang): Find out why this doesn't compile on Apple AMD OpenGL drivers
    // (http://anglebug.com/42260302)
    ANGLE_SKIP_TEST_IF(IsMac() && IsAMD() && IsOpenGL());

    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 2 * maxVaryings, 0, 0, 0, 0, 0, false, false, false, !IsD3D9());
}

// Disabled because of a failure in D3D9
TEST_P(GLSLTest, MaxVaryingVec2Arrays)
{
    ANGLE_SKIP_TEST_IF(IsD3D9());

    // TODO(geofflang): Figure out why this fails on NVIDIA's GLES driver
    ANGLE_SKIP_TEST_IF(IsOpenGLES());

    // TODO(geofflang): Find out why this doesn't compile on Apple AMD OpenGL drivers
    // (http://anglebug.com/42260302)
    ANGLE_SKIP_TEST_IF(IsMac() && IsAMD() && IsOpenGL());

    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    // Special case: because arrays of mat2 are packed as small grids of two rows by two columns,
    // we should be aware that when we're packing into an odd number of varying registers the
    // last row will be empty and can not fit the final vec2 arrary.
    GLint maxVec2Arrays = (maxVaryings >> 1) << 1;

    VaryingTestBase(0, 0, 0, maxVec2Arrays, 0, 0, 0, 0, false, false, false, true);
}

// Verify max varying with feedback and gl_line enabled
TEST_P(GLSLTest_ES3, MaxVaryingWithFeedbackAndGLline)
{
    // (http://anglebug.com/42263058)
    ANGLE_SKIP_TEST_IF(IsAMD() && IsWindows() && IsVulkan());

    // http://anglebug.com/42263066
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    std::stringstream vertexShaderSource;
    std::stringstream fragmentShaderSource;

    // substract 1 here for gl_PointSize
    const GLint vec4Count     = maxVaryings - 1;
    unsigned int varyingCount = 0;
    std::string varyingDeclaration;
    for (GLint i = 0; i < vec4Count; i++)
    {
        varyingDeclaration += GenerateVectorVaryingDeclaration(4, 1, varyingCount);
        varyingCount += 1;
    }
    // Generate the vertex shader
    vertexShaderSource.clear();
    vertexShaderSource << varyingDeclaration;
    vertexShaderSource << "\nattribute vec4 a_position;\n";
    vertexShaderSource << "\nvoid main()\n{\n";
    unsigned int currentVSVarying = 0;
    for (GLint i = 0; i < vec4Count; i++)
    {
        vertexShaderSource << GenerateVectorVaryingSettingCode(4, 1, currentVSVarying);
        currentVSVarying += 1;
    }
    vertexShaderSource << "\tgl_Position = vec4(a_position.rgb, 1);\n";
    vertexShaderSource << "\tgl_PointSize = 1.0;\n";
    vertexShaderSource << "}\n";

    // Generate the fragment shader
    fragmentShaderSource.clear();
    fragmentShaderSource << "precision highp float;\n";
    fragmentShaderSource << varyingDeclaration;
    fragmentShaderSource << "\nvoid main() \n{ \n\tvec4 retColor = vec4(0,0,0,0);\n";
    unsigned int currentFSVarying = 0;
    // Make use of the vec4 varyings
    fragmentShaderSource << "\tretColor += ";
    for (GLint i = 0; i < vec4Count; i++)
    {
        fragmentShaderSource << GenerateVectorVaryingUseCode(1, currentFSVarying);
        currentFSVarying += 1;
    }
    fragmentShaderSource << "vec4(0.0, 0.0, 0.0, 0.0);\n";
    constexpr GLuint testValue = 234;
    fragmentShaderSource << "\tgl_FragColor = (retColor/vec4(" << std::to_string(currentFSVarying)
                         << ")) /255.0*" << std::to_string(testValue) << ".0;\n";
    fragmentShaderSource << "}\n";

    std::vector<std::string> tfVaryings = {"gl_Position", "gl_PointSize"};
    ANGLE_GL_PROGRAM_TRANSFORM_FEEDBACK(program1, vertexShaderSource.str().c_str(),
                                        fragmentShaderSource.str().c_str(), tfVaryings,
                                        GL_INTERLEAVED_ATTRIBS);

    GLBuffer xfbBuffer;
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfbBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 6 * (sizeof(float[4]) + sizeof(float)), nullptr,
                 GL_STATIC_DRAW);

    GLTransformFeedback xfb;
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfbBuffer);

    glUseProgram(program1);

    const GLint positionLocation = glGetAttribLocation(program1, essl1_shaders::PositionAttrib());
    GLBuffer vertexBuffer;
    // need to shift half pixel to make sure the line covers the center of the pixel
    const Vector3 vertices[2] = {
        {-1.0f, -1.0f + 0.5f / static_cast<float>(getWindowHeight()), 0.0f},
        {1.0f, -1.0f + 0.5f / static_cast<float>(getWindowHeight()), 0.0f}};
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(*vertices) * 2, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBeginTransformFeedback(GL_LINES);
    glDrawArrays(GL_LINES, 0, 2);
    glEndTransformFeedback();

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(testValue, testValue, testValue, testValue));
}

// Verify shader source with a fixed length that is less than the null-terminated length will
// compile.
TEST_P(GLSLTest, FixedShaderLength)
{
    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const std::string appendGarbage = "abcdefghijklmnopqrstuvwxyz";
    const std::string source   = "void main() { gl_FragColor = vec4(0, 0, 0, 0); }" + appendGarbage;
    const char *sourceArray[1] = {source.c_str()};
    GLint lengths[1]           = {static_cast<GLint>(source.length() - appendGarbage.length())};
    glShaderSource(shader, static_cast<GLsizei>(ArraySize(sourceArray)), sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Verify that a negative shader source length is treated as a null-terminated length.
TEST_P(GLSLTest, NegativeShaderLength)
{
    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {essl1_shaders::fs::Red()};
    GLint lengths[1]           = {-10};
    glShaderSource(shader, static_cast<GLsizei>(ArraySize(sourceArray)), sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Check that having an invalid char after the "." doesn't cause an assert.
TEST_P(GLSLTest, InvalidFieldFirstChar)
{
    GLuint shader      = glCreateShader(GL_VERTEX_SHADER);
    const char *source = "void main() {vec4 x; x.}";
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_EQ(0, compileResult);
}

// Verify that a length array with mixed positive and negative values compiles.
TEST_P(GLSLTest, MixedShaderLengths)
{
    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[] = {
        "void main()",
        "{",
        "    gl_FragColor = vec4(0, 0, 0, 0);",
        "}",
    };
    GLint lengths[] = {
        -10,
        1,
        static_cast<GLint>(strlen(sourceArray[2])),
        -1,
    };
    ASSERT_EQ(ArraySize(sourceArray), ArraySize(lengths));

    glShaderSource(shader, static_cast<GLsizei>(ArraySize(sourceArray)), sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Verify that zero-length shader source does not affect shader compilation.
TEST_P(GLSLTest, ZeroShaderLength)
{
    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[] = {
        "abcdefg", "34534", "void main() { gl_FragColor = vec4(0, 0, 0, 0); }", "", "abcdefghijklm",
    };
    GLint lengths[] = {
        0, 0, -1, 0, 0,
    };
    ASSERT_EQ(ArraySize(sourceArray), ArraySize(lengths));

    glShaderSource(shader, static_cast<GLsizei>(ArraySize(sourceArray)), sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Tests that bad index expressions don't crash ANGLE's translator.
// https://code.google.com/p/angleproject/issues/detail?id=857
TEST_P(GLSLTest, BadIndexBug)
{
    constexpr char kFSSourceVec[] =
        "precision mediump float;\n"
        "uniform vec4 uniformVec;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(uniformVec[int()]);\n"
        "}";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFSSourceVec);
    EXPECT_EQ(0u, shader);

    if (shader != 0)
    {
        glDeleteShader(shader);
    }

    constexpr char kFSSourceMat[] =
        "precision mediump float;\n"
        "uniform mat4 uniformMat;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(uniformMat[int()]);\n"
        "}";

    shader = CompileShader(GL_FRAGMENT_SHADER, kFSSourceMat);
    EXPECT_EQ(0u, shader);

    if (shader != 0)
    {
        glDeleteShader(shader);
    }

    constexpr char kFSSourceArray[] =
        "precision mediump float;\n"
        "uniform vec4 uniformArray;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(uniformArray[int()]);\n"
        "}";

    shader = CompileShader(GL_FRAGMENT_SHADER, kFSSourceArray);
    EXPECT_EQ(0u, shader);

    if (shader != 0)
    {
        glDeleteShader(shader);
    }
}

// Test that structs defined in uniforms are translated correctly.
TEST_P(GLSLTest, StructSpecifiersUniforms)
{
    constexpr char kFS[] = R"(precision mediump float;

uniform struct S { float field; } s;

void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    gl_FragColor.a += s.field;
})";

    GLuint program = CompileProgram(essl1_shaders::vs::Simple(), kFS);
    EXPECT_NE(0u, program);
}

// Test that structs declaration followed directly by an initialization is translated correctly.
TEST_P(GLSLTest, StructWithInitializer)
{
    constexpr char kFS[] = R"(precision mediump float;

struct S { float a; } s = S(1.0);

void main()
{
    gl_FragColor = vec4(0, 0, 0, 1);
    gl_FragColor.r += s.a;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    // Test drawing, should be red.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_GL_NO_ERROR();
}

// Test that structs without initializer, followed by a uniform usage works as expected.
TEST_P(GLSLTest, UniformStructWithoutInitializer)
{
    constexpr char kFS[] = R"(precision mediump float;

struct S { float a; };
uniform S u_s;

void main()
{
    gl_FragColor = vec4(u_s.a);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    // Test drawing, should be red.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    EXPECT_GL_NO_ERROR();
}

// Test that structs declaration followed directly by an initialization in a uniform.
TEST_P(GLSLTest, StructWithUniformInitializer)
{
    constexpr char kFS[] = R"(precision mediump float;

struct S { float a; } s = S(1.0);
uniform S us;

void main()
{
    gl_FragColor = vec4(0, 0, 0, 1);
    gl_FragColor.r += s.a;
    gl_FragColor.g += us.a;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    // Test drawing, should be red.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    EXPECT_GL_NO_ERROR();
}

// Test that gl_DepthRange is not stored as a uniform location. Since uniforms
// beginning with "gl_" are filtered out by our validation logic, we must
// bypass the validation to test the behaviour of the implementation.
// (note this test is still Impl-independent)
TEST_P(GLSLTestNoValidation, DepthRangeUniforms)
{
    constexpr char kFS[] = R"(precision mediump float;

void main()
{
    gl_FragColor = vec4(gl_DepthRange.near, gl_DepthRange.far, gl_DepthRange.diff, 1);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);

    // We need to bypass validation for this call.
    GLint nearIndex = glGetUniformLocation(program, "gl_DepthRange.near");
    EXPECT_EQ(-1, nearIndex);

    // Test drawing does not throw an exception.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    EXPECT_GL_NO_ERROR();
}

std::string GenerateSmallPowShader(double base, double exponent)
{
    std::stringstream stream;

    stream.precision(8);

    double result = pow(base, exponent);

    stream << "precision highp float;\n"
           << "float fun(float arg)\n"
           << "{\n"
           << "    return pow(arg, " << std::fixed << exponent << ");\n"
           << "}\n"
           << "\n"
           << "void main()\n"
           << "{\n"
           << "    const float a = " << std::scientific << base << ";\n"
           << "    float b = fun(a);\n"
           << "    if (abs(" << result << " - b) < " << std::abs(result * 0.001) << ")\n"
           << "    {\n"
           << "        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
           << "    }\n"
           << "    else\n"
           << "    {\n"
           << "        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
           << "    }\n"
           << "}\n";

    return stream.str();
}

// Covers the WebGL test 'glsl/bugs/pow-of-small-constant-in-user-defined-function'
// See http://anglebug.com/40096900
TEST_P(GLSLTest, PowOfSmallConstant)
{
    // Test with problematic exponents that are close to an integer.
    std::vector<double> testExponents;
    std::array<double, 5> epsilonMultipliers = {-100.0, -1.0, 0.0, 1.0, 100.0};
    for (double epsilonMultiplier : epsilonMultipliers)
    {
        for (int i = -4; i <= 5; ++i)
        {
            if (i >= -1 && i <= 1)
                continue;
            const double epsilon = 1.0e-8;
            double bad           = static_cast<double>(i) + epsilonMultiplier * epsilon;
            testExponents.push_back(bad);
        }
    }

    // Also test with a few exponents that are not close to an integer.
    testExponents.push_back(3.6);
    testExponents.push_back(3.4);

    for (double testExponent : testExponents)
    {
        const std::string &fragmentShaderSource = GenerateSmallPowShader(1.0e-6, testExponent);

        ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), fragmentShaderSource.c_str());

        drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
        EXPECT_GL_NO_ERROR();
    }
}

// Test fragment shaders which contain non-constant loop indexers
TEST_P(GLSLTest, LoopIndexingValidation)
{
    constexpr char kFS[] = R"(precision mediump float;

uniform float loopMax;

void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    for (float l = 0.0; l < loopMax; l++)
    {
        if (loopMax > 3.0)
        {
            gl_FragColor.a += 0.1;
        }
    }
})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    glShaderSource(shader, 1, sourceArray, nullptr);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    // If the test is configured to run limited to Feature Level 9_3, then it is
    // assumed that shader compilation will fail with an expected error message containing
    // "Loop index cannot be compared with non-constant expression"
    if (GetParam() == ES2_D3D9())
    {
        if (compileResult != 0)
        {
            FAIL() << "Shader compilation succeeded, expected failure";
        }
        else
        {
            GLint infoLogLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            std::string infoLog;
            infoLog.resize(infoLogLength);
            glGetShaderInfoLog(shader, static_cast<GLsizei>(infoLog.size()), nullptr, &infoLog[0]);

            if (infoLog.find("Loop index cannot be compared with non-constant expression") ==
                std::string::npos)
            {
                FAIL() << "Shader compilation failed with unexpected error message";
            }
        }
    }
    else
    {
        EXPECT_NE(0, compileResult);
    }

    if (shader != 0)
    {
        glDeleteShader(shader);
    }
}

// Tests that the maximum uniforms count returned from querying GL_MAX_VERTEX_UNIFORM_VECTORS
// can actually be used.
TEST_P(GLSLTest, VerifyMaxVertexUniformVectors)
{
    // crbug.com/680631
    ANGLE_SKIP_TEST_IF(IsOzone() && IsIntel());

    int maxUniforms = 10000;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxUniforms);
    EXPECT_GL_NO_ERROR();
    std::cout << "Validating GL_MAX_VERTEX_UNIFORM_VECTORS = " << maxUniforms << std::endl;

    CompileGLSLWithUniformsAndSamplers(maxUniforms, 0, 0, 0, true);
}

// Tests that the maximum uniforms count returned from querying GL_MAX_VERTEX_UNIFORM_VECTORS
// can actually be used along with the maximum number of texture samplers.
TEST_P(GLSLTest, VerifyMaxVertexUniformVectorsWithSamplers)
{
    ANGLE_SKIP_TEST_IF(IsOpenGL() || IsOpenGLES());

    // Times out on D3D11 on test infra. http://anglebug.com/42263645
    ANGLE_SKIP_TEST_IF(IsD3D11() && IsIntel());

    int maxUniforms = 10000;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxUniforms);
    EXPECT_GL_NO_ERROR();
    std::cout << "Validating GL_MAX_VERTEX_UNIFORM_VECTORS = " << maxUniforms << std::endl;

    int maxTextureImageUnits = 0;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);

    CompileGLSLWithUniformsAndSamplers(maxUniforms, 0, maxTextureImageUnits, 0, true);
}

// Tests that the maximum uniforms count + 1 from querying GL_MAX_VERTEX_UNIFORM_VECTORS
// fails shader compilation.
TEST_P(GLSLTest, VerifyMaxVertexUniformVectorsExceeded)
{
    int maxUniforms = 10000;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxUniforms);
    EXPECT_GL_NO_ERROR();
    std::cout << "Validating GL_MAX_VERTEX_UNIFORM_VECTORS + 1 = " << maxUniforms + 1 << std::endl;

    CompileGLSLWithUniformsAndSamplers(maxUniforms + 1, 0, 0, 0, false);
}

// Tests that the maximum uniforms count returned from querying GL_MAX_FRAGMENT_UNIFORM_VECTORS
// can actually be used.
TEST_P(GLSLTest, VerifyMaxFragmentUniformVectors)
{
    // crbug.com/680631
    ANGLE_SKIP_TEST_IF(IsOzone() && IsIntel());

    int maxUniforms = 10000;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &maxUniforms);
    EXPECT_GL_NO_ERROR();
    std::cout << "Validating GL_MAX_FRAGMENT_UNIFORM_VECTORS = " << maxUniforms << std::endl;

    CompileGLSLWithUniformsAndSamplers(0, maxUniforms, 0, 0, true);
}

// Tests that the maximum uniforms count returned from querying GL_MAX_FRAGMENT_UNIFORM_VECTORS
// can actually be used along with the maximum number of texture samplers.
TEST_P(GLSLTest, VerifyMaxFragmentUniformVectorsWithSamplers)
{
    ANGLE_SKIP_TEST_IF(IsOpenGL() || IsOpenGLES());

    int maxUniforms = 10000;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &maxUniforms);
    EXPECT_GL_NO_ERROR();

    int maxTextureImageUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);

    CompileGLSLWithUniformsAndSamplers(0, maxUniforms, 0, maxTextureImageUnits, true);
}

// Tests that the maximum uniforms count + 1 from querying GL_MAX_FRAGMENT_UNIFORM_VECTORS
// fails shader compilation.
TEST_P(GLSLTest, VerifyMaxFragmentUniformVectorsExceeded)
{
    int maxUniforms = 10000;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &maxUniforms);
    EXPECT_GL_NO_ERROR();
    std::cout << "Validating GL_MAX_FRAGMENT_UNIFORM_VECTORS + 1 = " << maxUniforms + 1
              << std::endl;

    CompileGLSLWithUniformsAndSamplers(0, maxUniforms + 1, 0, 0, false);
}

// Test compiling shaders using the GL_EXT_shader_texture_lod extension
TEST_P(GLSLTest, TextureLOD)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_texture_lod"));

    constexpr char kFS[] =
        "#extension GL_EXT_shader_texture_lod : require\n"
        "uniform sampler2D u_texture;\n"
        "void main() {\n"
        "    gl_FragColor = texture2DGradEXT(u_texture, vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, "
        "0.0));\n"
        "}\n";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    ASSERT_NE(0u, shader);
    glDeleteShader(shader);
}

// HLSL generates extra lod0 variants of functions. There was a bug that incorrectly reworte
// function calls to use them in vertex shaders.  http://anglebug.com/42262136
TEST_P(GLSLTest, TextureLODRewriteInVertexShader)
{
    constexpr char kVS[] = R"(
  precision highp float;
  uniform int uni;
  uniform sampler2D texture;

  vec4 A();

  vec4 B() {
    vec4 a;
    for(int r=0; r<14; r++){
      if (r < uni) return vec4(0.0);
      a = A();
    }
    return a;
  }

  vec4 A() {
    return texture2D(texture, vec2(0.0, 0.0));
  }

  void main() {
    gl_Position = B();
  })";

    constexpr char kFS[] = R"(
void main() { gl_FragColor = vec4(gl_FragCoord.x / 640.0, gl_FragCoord.y / 480.0, 0, 1); }
)";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test to verify the a shader can have a sampler unused in a vertex shader
// but used in the fragment shader.
TEST_P(GLSLTest, VerifySamplerInBothVertexAndFragmentShaders)
{
    constexpr char kVS[] = R"(
attribute vec2 position;
varying mediump vec2 texCoord;
uniform sampler2D tex;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(
varying mediump vec2 texCoord;
uniform sampler2D tex;
void main()
{
    gl_FragColor = texture2D(tex, texCoord);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, redColors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
}

// Test that array of structs containing array of samplers work as expected.
TEST_P(GLSLTest, ArrayOfStructContainingArrayOfSamplers)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "struct Data { mediump sampler2D data[2]; };\n"
        "uniform Data test[2];\n"
        "void main() {\n"
        "    gl_FragColor = vec4(texture2D(test[1].data[1], vec2(0.0, 0.0)).r,\n"
        "                        texture2D(test[1].data[0], vec2(0.0, 0.0)).r,\n"
        "                        texture2D(test[0].data[1], vec2(0.0, 0.0)).r,\n"
        "                        texture2D(test[0].data[0], vec2(0.0, 0.0)).r);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[4];
    GLColor expected = MakeGLColor(32, 64, 96, 255);
    GLubyte data[8]  = {};  // 4 bytes of padding, so that texture can be initialized with 4 bytes
    memcpy(data, expected.data(), sizeof(expected));
    for (int i = 0; i < 4; i++)
    {
        int outerIdx = i % 2;
        int innerIdx = i / 2;
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        // Each element provides two components.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data + i);
        std::stringstream uniformName;
        uniformName << "test[" << innerIdx << "].data[" << outerIdx << "]";
        // Then send it as a uniform
        GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
        // The uniform should be active.
        EXPECT_NE(uniformLocation, -1);

        glUniform1i(uniformLocation, 3 - i);
    }
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, expected);
}

// Test that if a non-preprocessor token is seen in a disabled if-block then it does not disallow
// extension pragmas later
TEST_P(GLSLTest, NonPreprocessorTokensInIfBlocks)
{
    constexpr const char *kFS = R"(
#if __VERSION__ >= 300
    inout mediump vec4 fragData;
#else
    #extension GL_EXT_shader_texture_lod :enable
#endif

void main()
{
}
    )";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_NE(0u, shader);
}

// Test that two constructors which have vec4 and mat2 parameters get disambiguated (issue in
// HLSL).
TEST_P(GLSLTest_ES3, AmbiguousConstructorCall2x2)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 a_vec;\n"
        "in mat2 a_mat;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(a_vec) + vec4(a_mat);\n"
        "}";

    GLuint program = CompileProgram(kVS, essl3_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Test that two constructors which have mat2x3 and mat3x2 parameters get disambiguated.
// This was suspected to be an issue in HLSL, but HLSL seems to be able to natively choose between
// the function signatures in this case.
TEST_P(GLSLTest_ES3, AmbiguousConstructorCall2x3)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "in mat3x2 a_matA;\n"
        "in mat2x3 a_matB;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(a_matA) + vec4(a_matB);\n"
        "}";

    GLuint program = CompileProgram(kVS, essl3_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Test that two functions which have vec4 and mat2 parameters get disambiguated (issue in HLSL).
TEST_P(GLSLTest_ES3, AmbiguousFunctionCall2x2)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec4 a_vec;\n"
        "in mat2 a_mat;\n"
        "vec4 foo(vec4 a)\n"
        "{\n"
        "    return a;\n"
        "}\n"
        "vec4 foo(mat2 a)\n"
        "{\n"
        "    return vec4(a[0][0]);\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    gl_Position = foo(a_vec) + foo(a_mat);\n"
        "}";

    GLuint program = CompileProgram(kVS, essl3_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Test that constructing matrices from non-float types works.
TEST_P(GLSLTest_ES3, ConstructMatrixFromNonFloat)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 color;

uniform int i;
uniform uint u;
uniform bool b;

void main()
{
    mat3x2 mi = mat3x2(i);
    mat4 mu = mat4(u);
    mat2x4 mb = mat2x4(b);

    mat3x2 m = mat3x2(ivec2(i), uvec2(u), bvec2(b));

    color = vec4(mi[0][0] == -123.0 ? 1 : 0,
                 mu[2][2] == 456.0 ? 1 : 0,
                 mb[1][1] == 1.0 ? 1 : 0,
                 m[0][1] == -123.0 && m[1][0] == 456.0 && m[2][0] == 1.0 ? 1 : 0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint iloc = glGetUniformLocation(program, "i");
    GLint uloc = glGetUniformLocation(program, "u");
    GLint bloc = glGetUniformLocation(program, "b");
    ASSERT_NE(iloc, -1);
    ASSERT_NE(uloc, -1);
    ASSERT_NE(bloc, -1);
    glUniform1i(iloc, -123);
    glUniform1ui(uloc, 456);
    glUniform1ui(bloc, 1);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that constructing vectors from non-float types works.
TEST_P(GLSLTest_ES3, ConstructVectorFromNonFloat)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 color;

uniform ivec2 i;
uniform uvec2 u;
uniform bvec2 b;

void main()
{
    vec2 v2 = vec2(i.x, b);
    vec3 v3 = vec3(b, u);
    vec4 v4 = vec4(i, u);

    color = vec4(v2.x == -123.0 && v2.y == 1.0 ? 1 : 0,
                 v3.x == 1.0 && v3.y == 0.0 && v3.z == 456.0 ? 1 : 0,
                 v4.x == -123.0 && v4.y == -23.0 && v4.z == 456.0 && v4.w == 76.0 ? 1 : 0,
                 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint iloc = glGetUniformLocation(program, "i");
    GLint uloc = glGetUniformLocation(program, "u");
    GLint bloc = glGetUniformLocation(program, "b");
    ASSERT_NE(iloc, -1);
    ASSERT_NE(uloc, -1);
    ASSERT_NE(bloc, -1);
    glUniform2i(iloc, -123, -23);
    glUniform2ui(uloc, 456, 76);
    glUniform2ui(bloc, 1, 0);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that constructing non-float vectors from matrix types works.
TEST_P(GLSLTest_ES3, ConstructNonFloatVectorFromMatrix)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 color;

uniform float f;

void main()
{
    mat4 m = mat4(f);
    ivec3 vi = ivec3(m);
    uvec2 vu = uvec2(m);
    bvec4 vb = bvec4(m);
    bvec2 vb2 = bvec2(vi.x, m);

    color = vec4(vi.x == int(f) ? 1 : 0,
                 vu.x == uint(f) ? 1 : 0,
                 vb.x == bool(f) ? 1 : 0,
                 vb2.x == bool(f) && vb2.y == bool(f) ? 1 : 0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint floc = glGetUniformLocation(program, "f");
    ASSERT_NE(floc, -1);
    glUniform1f(floc, 123);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that == and != for vector and matrix types work.
TEST_P(GLSLTest_ES3, NonScalarEqualOperator)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 color;

uniform float f;
uniform int i;
uniform uint u;

void main()
{
    mat3x2 m32_1 = mat3x2(vec2(f), vec2(i), vec2(u));
    mat3x2 m32_2 = mat3x2(m32_1);
    mat3x2 m32_3 = mat3x2(vec2(i), vec2(u), vec2(f));
    mat2x3 m23_1 = mat2x3(vec3(f), vec3(i));
    mat2x3 m23_2 = mat2x3(m23_1);
    mat2x3 m23_3 = mat2x3(vec3(i), vec3(u));
    vec2 v2_1 = m32_1[0];
    vec2 v2_2 = m32_2[0];
    ivec3 v3_1 = ivec3(transpose(m32_1)[0]);
    ivec3 v3_2 = ivec3(transpose(m32_2)[0]);
    uvec4 v4_1 = uvec4(m32_1[1], m32_1[2]);
    uvec4 v4_2 = uvec4(m32_2[1], m32_2[2]);

    color = vec4((m32_1 == m32_2 ? 0.5 : 0.0) + (m23_1 == m23_2 ? 0.5 : 0.0),
                 v2_1 == v2_2 ? 1 : 0,
                 (v3_1 == v3_2 ? 0.5 : 0.0) +
                    (v4_1 == v4_2 ? 0.5 : 0.0),
                 (m32_1 != m32_3 ? 0.125 : 0.0) +
                    (m23_1 != m23_3 ? 0.125 : 0.0) +
                    (v2_1 != vec2(v3_2) ? 0.25 : 0.0) +
                    (v3_1 != ivec3(v4_2) ? 0.25 : 0.0) +
                    (v4_1 != uvec4(v2_1, v2_2) ? 0.25 : 0.0));
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint floc = glGetUniformLocation(program, "f");
    GLint iloc = glGetUniformLocation(program, "i");
    GLint uloc = glGetUniformLocation(program, "u");
    ASSERT_NE(floc, -1);
    ASSERT_NE(iloc, -1);
    ASSERT_NE(uloc, -1);
    glUniform1f(floc, 1.5);
    glUniform1i(iloc, -123);
    glUniform1ui(uloc, 456);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that == and != for structs and array types work.
TEST_P(GLSLTest_ES31, StructAndArrayEqualOperator)
{
    constexpr char kFS[] = R"(#version 310 es
precision highp float;
out vec4 color;

uniform float f;
uniform int i;
uniform uint u;

struct S
{
    float f;
    int i;
    uint u;
    vec4 v;
    ivec3 iv;
    uvec2 uv;
    mat3x2 m32;
    mat2x3 m23;
    float fa[3][4][5];
    int ia[4];
    uint ua[6][2];
};

struct T
{
    S s1;
    S s2[3][2];
};

void main()
{
    float fa[5] = float[5](f, f, f, f, f);
    int ia[4] = int[4](i, i, i, i);
    uint ua[2] = uint[2](u, u);

    S s1 = S(f, i, u, vec4(f), ivec3(i), uvec2(u),
             mat3x2(vec2(f), vec2(i), vec2(u)),
             mat2x3(vec3(f), vec3(i)),
             float[3][4][5](
                            float[4][5](fa, fa, fa, fa),
                            float[4][5](fa, fa, fa, fa),
                            float[4][5](fa, fa, fa, fa)),
             ia,
             uint[6][2](ua, ua, ua, ua, ua, ua));

    S s2[2] = S[2](s1, s1);
    s2[1].fa[0][1][2] = float(i);

    T t1 = T(s1, S[3][2](s2, s2, s2));
    T t2 = T(s2[1], S[3][2](s2, s2, s2));

    T ta1[2] = T[2](t1, t2);
    T ta2[2] = T[2](t1, t2);
    T ta3[2] = T[2](t2, t1);

    color = vec4((s1 == s2[0] ? 0.5 : 0.0) + (s1 != s2[1] ? 0.5 : 0.0),
                 (s1.fa[0] == s2[0].fa[0] ? 0.5 : 0.0) + (s1.fa[0] != s2[1].fa[0] ? 0.5 : 0.0),
                 (ta1[0] == t1 ? 0.5 : 0.0) + (ta1[1] != t1 ? 0.5 : 0.0),
                 (ta1 == ta2 ? 0.5 : 0.0) + (ta1 != ta3 ? 0.5 : 0.0));
})";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint floc = glGetUniformLocation(program, "f");
    GLint iloc = glGetUniformLocation(program, "i");
    GLint uloc = glGetUniformLocation(program, "u");
    ASSERT_NE(floc, -1);
    ASSERT_NE(iloc, -1);
    ASSERT_NE(uloc, -1);
    glUniform1f(floc, 1.5);
    glUniform1i(iloc, -123);
    glUniform1ui(uloc, 456);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that an user-defined function with a large number of float4 parameters doesn't fail due to
// the function name being too long.
TEST_P(GLSLTest_ES3, LargeNumberOfFloat4Parameters)
{
    std::stringstream vertexShaderStream;
    // Note: SPIR-V doesn't allow more than 255 parameters to a function.
    const unsigned int paramCount = (IsVulkan() || IsMetal()) ? 255u : 1024u;

    vertexShaderStream << "#version 300 es\n"
                          "precision highp float;\n"
                          "in vec4 a_vec;\n"
                          "vec4 lotsOfVec4Parameters(";
    for (unsigned int i = 0; i < paramCount - 1; ++i)
    {
        vertexShaderStream << "vec4 a" << i << ", ";
    }
    vertexShaderStream << "vec4 aLast)\n"
                          "{\n"
                          "    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);\n";
    for (unsigned int i = 0; i < paramCount - 1; ++i)
    {
        vertexShaderStream << "    sum += a" << i << ";\n";
    }
    vertexShaderStream << "    sum += aLast;\n"
                          "    return sum;\n "
                          "}\n"
                          "void main()\n"
                          "{\n"
                          "    gl_Position = lotsOfVec4Parameters(";
    for (unsigned int i = 0; i < paramCount - 1; ++i)
    {
        vertexShaderStream << "a_vec, ";
    }
    vertexShaderStream << "a_vec);\n"
                          "}";

    GLuint program = CompileProgram(vertexShaderStream.str().c_str(), essl3_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// This test was written specifically to stress DeferGlobalInitializers AST transformation.
// Test a shader where a global constant array is initialized with an expression containing array
// indexing. This initializer is tricky to constant fold, so if it's not constant folded it needs to
// be handled in a way that doesn't generate statements in the global scope in HLSL output.
// Also includes multiple array initializers in one declaration, where only the second one has
// array indexing. This makes sure that the qualifier for the declaration is set correctly if
// transformations are applied to the declaration also in the case of ESSL output.
TEST_P(GLSLTest_ES3, InitGlobalArrayWithArrayIndexing)
{
    // TODO(ynovikov): re-enable once root cause of http://anglebug.com/42260423 is fixed
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsAdreno() && IsOpenGLES());

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "const highp float f[2] = float[2](0.1, 0.2);\n"
        "const highp float[2] g = float[2](0.3, 0.4), h = float[2](0.5, f[1]);\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = vec4(h[1]);\n"
        "}";

    GLuint program = CompileProgram(essl3_shaders::vs::Simple(), kFS);
    EXPECT_NE(0u, program);
}

// Test that constant global matrix array with an initializer compiles.
TEST_P(GLSLTest_ES3, InitConstantMatrixArray)
{
    constexpr char kFS[] = R"(#version 300 es
        precision highp float;
        uniform int index;

        const mat4 matrix = mat4(1.0);
        const mat4 array[1] = mat4[1](matrix);
        out vec4 my_FragColor;
        void main() {
            my_FragColor = vec4(array[index][1].rgb, 1.0);
        })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that index-constant sampler array indexing is supported.
TEST_P(GLSLTest, IndexConstantSamplerArrayIndexing)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "uniform sampler2D uni[2];\n"
        "\n"
        "float zero(int x)\n"
        "{\n"
        "    return float(x) - float(x);\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "    vec4 c = vec4(0,0,0,0);\n"
        "    for (int ii = 1; ii < 3; ++ii) {\n"
        "        if (c.x > 255.0) {\n"
        "            c.x = 255.0 + zero(ii);\n"
        "            break;\n"
        "        }\n"
        // Index the sampler array with a predictable loop index (index-constant) as opposed to
        // a true constant. This is valid in OpenGL ES but isn't in many Desktop OpenGL versions,
        // without an extension.
        "        c += texture2D(uni[ii - 1], vec2(0.5, 0.5));\n"
        "    }\n"
        "    gl_FragColor = c;\n"
        "}";

    GLuint program = CompileProgram(essl1_shaders::vs::Simple(), kFS);
    EXPECT_NE(0u, program);
}

// Test that the #pragma directive is supported and doesn't trigger a compilation failure on the
// native driver. The only pragma that gets passed to the OpenGL driver is "invariant" but we don't
// want to test its behavior, so don't use any varyings.
TEST_P(GLSLTest, PragmaDirective)
{
    constexpr char kVS[] =
        "#pragma STDGL invariant(all)\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    GLuint program = CompileProgram(kVS, essl1_shaders::fs::Red());
    EXPECT_NE(0u, program);
}

// Sequence operator evaluates operands from left to right (ESSL 3.00 section 5.9).
// The function call that returns the array needs to be evaluated after ++j for the expression to
// return the correct value (true).
TEST_P(GLSLTest_ES3, SequenceOperatorEvaluationOrderArray)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor; \n"
        "int[2] func(int param) {\n"
        "    return int[2](param, param);\n"
        "}\n"
        "void main() {\n"
        "    int a[2]; \n"
        "    for (int i = 0; i < 2; ++i) {\n"
        "        a[i] = 1;\n"
        "    }\n"
        "    int j = 0; \n"
        "    bool result = ((++j), (a == func(j)));\n"
        "    my_FragColor = vec4(0.0, (result ? 1.0 : 0.0), 0.0, 1.0);\n"
        "}\n";

    GLuint program = CompileProgram(essl3_shaders::vs::Simple(), kFS);
    ASSERT_NE(0u, program);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Sequence operator evaluates operands from left to right (ESSL 3.00 section 5.9).
// The short-circuiting expression needs to be evaluated after ++j for the expression to return the
// correct value (true).
TEST_P(GLSLTest_ES3, SequenceOperatorEvaluationOrderShortCircuit)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor; \n"
        "void main() {\n"
        "    int j = 0; \n"
        "    bool result = ((++j), (j == 1 ? true : (++j == 3)));\n"
        "    my_FragColor = vec4(0.0, ((result && j == 1) ? 1.0 : 0.0), 0.0, 1.0);\n"
        "}\n";

    GLuint program = CompileProgram(essl3_shaders::vs::Simple(), kFS);
    ASSERT_NE(0u, program);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Sequence operator evaluates operands from left to right (ESSL 3.00 section 5.9).
// Indexing the vector needs to be evaluated after func() for the right result.
TEST_P(GLSLTest_ES3, SequenceOperatorEvaluationOrderDynamicVectorIndexingInLValue)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform int u_zero;\n"
        "int sideEffectCount = 0;\n"
        "float func() {\n"
        "    ++sideEffectCount;\n"
        "    return -1.0;\n"
        "}\n"
        "void main() {\n"
        "    vec4 v = vec4(0.0, 2.0, 4.0, 6.0); \n"
        "    float f = (func(), (++v[u_zero + sideEffectCount]));\n"
        "    bool green = abs(f - 3.0) < 0.01 && abs(v[1] - 3.0) < 0.01 && sideEffectCount == 1;\n"
        "    my_FragColor = vec4(0.0, (green ? 1.0 : 0.0), 0.0, 1.0);\n"
        "}\n";

    GLuint program = CompileProgram(essl3_shaders::vs::Simple(), kFS);
    ASSERT_NE(0u, program);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that using gl_PointCoord with GL_TRIANGLES doesn't produce a link error.
// From WebGL test conformance/rendering/point-specific-shader-variables.html
// See http://anglebug.com/42260376
TEST_P(GLSLTest, RenderTrisWithPointCoord)
{
    constexpr char kVS[] =
        "attribute vec2 aPosition;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(aPosition, 0, 1);\n"
        "    gl_PointSize = 1.0;\n"
        "}";
    constexpr char kFS[] =
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(gl_PointCoord.xy, 0, 1);\n"
        "    gl_FragColor = vec4(0, 1, 0, 1);\n"
        "}";

    ANGLE_GL_PROGRAM(prog, kVS, kFS);
    drawQuad(prog, "aPosition", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Convers a bug with the integer pow statement workaround.
TEST_P(GLSLTest, NestedPowStatements)
{
    // https://crbug.com/1127866 - possible NVIDIA driver issue
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsVulkan() && IsWindows());

    constexpr char kFS[] =
        "precision mediump float;\n"
        "float func(float v)\n"
        "{\n"
        "   float f1 = pow(v, 2.0);\n"
        "   return pow(f1 + v, 2.0);\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    float v = func(2.0);\n"
        "    gl_FragColor = abs(v - 36.0) < 0.001 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);\n"
        "}";

    ANGLE_GL_PROGRAM(prog, essl1_shaders::vs::Simple(), kFS);
    drawQuad(prog, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// This test covers a crash seen in an application during SPIR-V compilation
TEST_P(GLSLTest_ES3, NestedPowFromUniform)
{
    constexpr char kVS[] = R"(#version 300 es
in vec2 position;
void main()
{
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
precision mediump int;

uniform highp vec4 scale;
out mediump vec4 out_FragColor;
void main()
{
    highp vec4 v0;
    v0 = scale;
    highp vec3 v1;
    v1.xyz = v0.xyz;
    if ((v0.y!=1.0))
    {
        vec3 v3;
        v3.xyz = pow(v0.xyz,v0.xxx);
        float h0;
        if ((v3.x < 3.13))
        {
            h0 = (v3.x * 1.29);
        }
        else
        {
            h0 = ((pow(v3.x,4.16)*1.055)+-5.5);
        }
        float h1;
        if ((v3.y<3.13))
        {
            h1 = (v3.y*1.29);
        }
        else
        {
            h1 = ((pow(v3.y,4.16)*1.055)+-5.5);
        }
        float h2;
        if ((v3.z<3.13))
        {
            h2 = (v3.z*1.29);
        }
        else
        {
            h2 = ((pow(v3.z,4.16)*1.055)+-5.5);
        }
        v1.xyz = vec3(h0, h1, h2);
    }
    out_FragColor = vec4(v1, v0.w);
}
)";

    ANGLE_GL_PROGRAM(prog, kVS, kFS);

    GLint scaleIndex = glGetUniformLocation(prog, "scale");
    ASSERT_NE(-1, scaleIndex);

    glUseProgram(prog);
    glUniform4f(scaleIndex, 0.5, 0.5, 0.5, 0.5);

    // Don't crash
    drawQuad(prog, "position", 0.5f);
}

// Test that -float calculation is correct.
TEST_P(GLSLTest_ES3, UnaryMinusOperatorFloat)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "out highp vec4 o_color;\n"
        "void main() {\n"
        "    highp float f = -1.0;\n"
        "    // atan(tan(0.5), -f) should be 0.5.\n"
        "    highp float v = atan(tan(0.5), -f);\n"
        "    o_color = abs(v - 0.5) < 0.001 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);\n"
        "}\n";

    ANGLE_GL_PROGRAM(prog, essl3_shaders::vs::Simple(), kFS);
    drawQuad(prog, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that atan(vec2, vec2) calculation is correct.
TEST_P(GLSLTest_ES3, AtanVec2)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "out highp vec4 o_color;\n"
        "void main() {\n"
        "    highp float f = 1.0;\n"
        "    // atan(tan(0.5), f) should be 0.5.\n"
        "    highp vec2 v = atan(vec2(tan(0.5)), vec2(f));\n"
        "    o_color = (abs(v[0] - 0.5) < 0.001 && abs(v[1] - 0.5) < 0.001) ? vec4(0, 1, 0, 1) : "
        "vec4(1, 0, 0, 1);\n"
        "}\n";

    ANGLE_GL_PROGRAM(prog, essl3_shaders::vs::Simple(), kFS);
    drawQuad(prog, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Convers a bug with the unary minus operator on signed integer workaround.
TEST_P(GLSLTest_ES3, UnaryMinusOperatorSignedInt)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in highp vec4 position;\n"
        "out mediump vec4 v_color;\n"
        "uniform int ui_one;\n"
        "uniform int ui_two;\n"
        "uniform int ui_three;\n"
        "void main() {\n"
        "    int s[3];\n"
        "    s[0] = ui_one;\n"
        "    s[1] = -(-(-ui_two + 1) + 1);\n"  // s[1] = -ui_two
        "    s[2] = ui_three;\n"
        "    int result = 0;\n"
        "    for (int i = 0; i < ui_three; i++) {\n"
        "        result += s[i];\n"
        "    }\n"
        "    v_color = (result == 2) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);\n"
        "    gl_Position = position;\n"
        "}\n";
    constexpr char kFS[] =
        "#version 300 es\n"
        "in mediump vec4 v_color;\n"
        "layout(location=0) out mediump vec4 o_color;\n"
        "void main() {\n"
        "    o_color = v_color;\n"
        "}\n";

    ANGLE_GL_PROGRAM(prog, kVS, kFS);

    GLint oneIndex = glGetUniformLocation(prog, "ui_one");
    ASSERT_NE(-1, oneIndex);
    GLint twoIndex = glGetUniformLocation(prog, "ui_two");
    ASSERT_NE(-1, twoIndex);
    GLint threeIndex = glGetUniformLocation(prog, "ui_three");
    ASSERT_NE(-1, threeIndex);
    glUseProgram(prog);
    glUniform1i(oneIndex, 1);
    glUniform1i(twoIndex, 2);
    glUniform1i(threeIndex, 3);

    drawQuad(prog, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Convers a bug with the unary minus operator on unsigned integer workaround.
TEST_P(GLSLTest_ES3, UnaryMinusOperatorUnsignedInt)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in highp vec4 position;\n"
        "out mediump vec4 v_color;\n"
        "uniform uint ui_one;\n"
        "uniform uint ui_two;\n"
        "uniform uint ui_three;\n"
        "void main() {\n"
        "    uint s[3];\n"
        "    s[0] = ui_one;\n"
        "    s[1] = -(-(-ui_two + 1u) + 1u);\n"  // s[1] = -ui_two
        "    s[2] = ui_three;\n"
        "    uint result = 0u;\n"
        "    for (uint i = 0u; i < ui_three; i++) {\n"
        "        result += s[i];\n"
        "    }\n"
        "    v_color = (result == 2u) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);\n"
        "    gl_Position = position;\n"
        "}\n";
    constexpr char kFS[] =
        "#version 300 es\n"
        "in mediump vec4 v_color;\n"
        "layout(location=0) out mediump vec4 o_color;\n"
        "void main() {\n"
        "    o_color = v_color;\n"
        "}\n";

    ANGLE_GL_PROGRAM(prog, kVS, kFS);

    GLint oneIndex = glGetUniformLocation(prog, "ui_one");
    ASSERT_NE(-1, oneIndex);
    GLint twoIndex = glGetUniformLocation(prog, "ui_two");
    ASSERT_NE(-1, twoIndex);
    GLint threeIndex = glGetUniformLocation(prog, "ui_three");
    ASSERT_NE(-1, threeIndex);
    glUseProgram(prog);
    glUniform1ui(oneIndex, 1u);
    glUniform1ui(twoIndex, 2u);
    glUniform1ui(threeIndex, 3u);

    drawQuad(prog, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test a nested sequence operator with a ternary operator inside. The ternary operator is
// intended to be such that it gets converted to an if statement on the HLSL backend.
TEST_P(GLSLTest, NestedSequenceOperatorWithTernaryInside)
{
    // Note that the uniform keep_flop_positive doesn't need to be set - the test expects it to have
    // its default value false.
    constexpr char kFS[] =
        "precision mediump float;\n"
        "uniform bool keep_flop_positive;\n"
        "float flop;\n"
        "void main() {\n"
        "    flop = -1.0,\n"
        "    (flop *= -1.0,\n"
        "    keep_flop_positive ? 0.0 : flop *= -1.0),\n"
        "    gl_FragColor = vec4(0, -flop, 0, 1);\n"
        "}";

    ANGLE_GL_PROGRAM(prog, essl1_shaders::vs::Simple(), kFS);
    drawQuad(prog, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that nesting ternary and short-circuitting operators work.
TEST_P(GLSLTest, NestedTernaryAndShortCircuit)
{
    // Note that the uniform doesn't need to be set, and will contain the default value of false.
    constexpr char kFS[] = R"(
precision mediump float;
uniform bool u;
void main()
{
    int a = u ? 12345 : 2;      // will be 2
    int b = u ? 12345 : 4;      // will be 4
    int c = u ? 12345 : 0;      // will be 0

    if (a == 2                  // true path is taken
        ? (b == 3               // false path is taken
            ? (a=0) != 0
            : b != 0            // true
          ) && (                // short-circuit evaluates RHS
            (a=7) == 7          // true, modifies a
            ||                  // short-circuit doesn't evaluate RHS
            (b=8) == 8
          )
        : (a == 0 && b == 0
            ? (c += int((a=0) == 0 && (b=0) == 0)) != 0
            : (c += int((a=0) != 0 && (b=0) != 0)) != 0))
    {
        c += 15;                // will execute
    }

    // Verify that a is 7, b is 4 and c is 15.
    gl_FragColor = vec4(a == 7, b == 4, c == 15, 1);
})";

    ANGLE_GL_PROGRAM(prog, essl1_shaders::vs::Simple(), kFS);
    drawQuad(prog, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that uniform bvecN passed to functions work.
TEST_P(GLSLTest_ES3, UniformBoolVectorPassedToFunctions)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform bvec4 u;
out vec4 color;

bool f(bvec4 bv)
{
    return all(bv.xz) && !any(bv.yw);
}

void main() {
    color = f(u) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(prog, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(prog);

    GLint uloc = glGetUniformLocation(prog, "u");
    ASSERT_NE(uloc, -1);
    glUniform4ui(uloc, true, false, true, false);

    drawQuad(prog, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that bvecN in storage buffer passed to functions work.
TEST_P(GLSLTest_ES31, StorageBufferBoolVectorPassedToFunctions)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 0, std430) buffer Output {
    bvec4 b;
    bool valid;
} outbuf;

bool f_in(bvec4 bv)
{
    return all(bv.xz) && !any(bv.yw);
}

bool f_inout(inout bvec4 bv)
{
    bool ok = all(bv.xz) && !any(bv.yw);
    bv.xw = bvec2(false, true);
    return ok;
}

void f_out(out bvec4 bv)
{
    bv = bvec4(false, true, false, true);
}

void main() {
    bool valid = f_in(outbuf.b);
    valid = f_inout(outbuf.b) && valid;
    f_out(outbuf.b);
    outbuf.valid = valid;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    constexpr std::array<GLuint, 5> kOutputInitData = {true, false, true, false, false};
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kOutputInitData), kOutputInitData.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, outputBuffer);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kOutputInitData), GL_MAP_READ_BIT));
    fprintf(stderr, "%d %d %d %d %d\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]);
    EXPECT_FALSE(ptr[0]);
    EXPECT_TRUE(ptr[1]);
    EXPECT_FALSE(ptr[2]);
    EXPECT_TRUE(ptr[3]);
    EXPECT_TRUE(ptr[4]);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that using a sampler2D and samplerExternalOES in the same shader works
// (anglebug.com/42260512)
TEST_P(GLSLTest, ExternalAnd2DSampler)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_EGL_image_external"));

    constexpr char kFS[] = R"(#extension GL_OES_EGL_image_external : enable
precision mediump float;
uniform samplerExternalOES tex0;
uniform sampler2D tex1;
void main(void)
{
    vec2 uv = vec2(0.0, 0.0);
    gl_FragColor = texture2D(tex0, uv) + texture2D(tex1, uv);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
}

// Tests that monomorphizing functions does not crash if there is a main prototype.
TEST_P(GLSLTest, MonomorphizeMainPrototypeNoCrash)
{
    constexpr char kFS[] = R"(precision mediump float;
struct S { sampler2D source; };
vec4 f(S s)
{
    return texture2D(s.source, vec2(5));
}
uniform S green;
void main();
void main() {
    f(green);
})";
    CompileShader(GL_FRAGMENT_SHADER, kFS);
    ASSERT_GL_NO_ERROR();
}

// Test that using a varying matrix array is supported.
TEST_P(GLSLTest, VaryingMatrixArray)
{
    constexpr char kVS[] =
        "uniform vec2 u_a1;\n"
        "uniform vec2 u_a2;\n"
        "attribute vec4 a_position;\n"
        "varying mat2 v_mat[2];\n"
        "void main() {\n"
        "    v_mat[0] = mat2(u_a1, u_a2);\n"
        "    v_mat[1] = mat2(1.0 - u_a2, 1.0 - u_a1);\n"
        "    gl_Position = a_position;\n"
        "}";

    constexpr char kFS[] =
        "precision mediump float;\n"
        "varying mat2 v_mat[2];\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = vec4(v_mat[0][0].x, v_mat[0][0].y, v_mat[1][0].x, 1.0);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint oneIndex = glGetUniformLocation(program, "u_a1");
    ASSERT_NE(-1, oneIndex);
    GLint twoIndex = glGetUniformLocation(program, "u_a2");
    ASSERT_NE(-1, twoIndex);
    glUseProgram(program);
    glUniform2f(oneIndex, 1, 0.5f);
    glUniform2f(twoIndex, 0.25f, 0.125f);

    drawQuad(program, "a_position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 127, 255 - 63, 255), 1.0);
}

// Test that using a centroid varying matrix array is supported.
TEST_P(GLSLTest_ES3, CentroidVaryingMatrixArray)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "uniform vec2 u_a1;\n"
        "uniform vec2 u_a2;\n"
        "in vec4 a_position;\n"
        "centroid out mat3x2 v_mat[2];\n"
        "void main() {\n"
        "    v_mat[0] = mat3x2(u_a1, u_a2, vec2(0.0));\n"
        "    v_mat[1] = mat3x2(vec2(0.0), 1.0 - u_a2, 1.0 - u_a1);\n"
        "    gl_Position = a_position;\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "centroid in mat3x2 v_mat[2];\n"
        "layout(location = 0) out vec4 out_color;\n"
        "void main(void)\n"
        "{\n"
        "    out_color = vec4(v_mat[0][0].x, v_mat[0][0].y, v_mat[1][1].x, 1.0);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint oneIndex = glGetUniformLocation(program, "u_a1");
    ASSERT_NE(-1, oneIndex);
    GLint twoIndex = glGetUniformLocation(program, "u_a2");
    ASSERT_NE(-1, twoIndex);
    glUseProgram(program);
    glUniform2f(oneIndex, 1, 0.5f);
    glUniform2f(twoIndex, 0.25f, 0.125f);

    drawQuad(program, "a_position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 127, 255 - 63, 255), 1.0);
}

// Test that using a flat varying matrix array is supported.
TEST_P(GLSLTest_ES3, FlatVaryingMatrixArray)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "uniform vec2 u_a1;\n"
        "uniform vec2 u_a2;\n"
        "in vec4 a_position;\n"
        "flat out mat2 v_mat[2];\n"
        "void main() {\n"
        "    v_mat[0] = mat2(u_a1, u_a2);\n"
        "    v_mat[1] = mat2(u_a2, u_a1);\n"
        "    gl_Position = a_position;\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "flat in mat2 v_mat[2];\n"
        "layout(location = 0) out vec4 out_color;\n"
        "void main(void)\n"
        "{\n"
        "    out_color = vec4(v_mat[0][0].x, v_mat[0][0].y, v_mat[1][0].x, 1.0);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint oneIndex = glGetUniformLocation(program, "u_a1");
    ASSERT_NE(-1, oneIndex);
    GLint twoIndex = glGetUniformLocation(program, "u_a2");
    ASSERT_NE(-1, twoIndex);
    glUseProgram(program);
    glUniform2f(oneIndex, 1, 0.5f);
    glUniform2f(twoIndex, 0.25f, 0.125f);

    drawQuad(program, "a_position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 127, 63, 255), 1.0);
}

// Test that using a varying matrix array works with a particular input names scheme.
// Test that having "mat3 a[*]" and "mat2 a_0" does not cause internal shader compile failures.
// For a buggy naming scheme both would expand into a_0_0 and cause problems.
TEST_P(GLSLTest_ES3, VaryingMatrixArrayNaming2)
{
    constexpr char kVS[] = R"(#version 300 es
precision mediump float;
uniform mat3 r0;
uniform mat2 r1;
in vec4 a_position;
out mat2 a_0;
out mat3 a[2];
void main() {
    a[0] = r0;
    a[1] = r0 + mat3(1, 1, 1, 1, 1, 1, 1, 1, 1);
    a_0 = r1;
    gl_Position = a_position;
})";
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
in mat2 a_0;
in mat3 a[2];
layout(location = 0) out vec4 o;
void main(void) {
    mat3 diff0 = a[0] - mat3(0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8);
    mat3 diff1 = a[1] - mat3(1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8);
    mat2 diff2 = a_0 - mat2(3.0, 3.1, 3.2, 3.3);
    o.r = all(lessThan(abs(diff0[0]) + abs(diff0[1]) + abs(diff0[2]), vec3(0.01))) ? 1.0 : 0.0;
    o.g = all(lessThan(abs(diff1[0]) + abs(diff1[1]) + abs(diff1[2]), vec3(0.01))) ? 1.0 : 0.0;
    o.b = all(lessThan(abs(diff2[0]) + abs(diff2[1]), vec2(0.01))) ? 1.0 : 0.0;
    o.a = 1.0;
})";
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    GLint r0 = glGetUniformLocation(program, "r0");
    ASSERT_NE(-1, r0);
    GLint r1 = glGetUniformLocation(program, "r1");
    ASSERT_NE(-1, r1);
    glUseProgram(program);
    float r0v[] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    glUniformMatrix3fv(r0, 1, false, r0v);
    float r1v[] = {3.0f, 3.1f, 3.2f, 3.3f};
    glUniformMatrix2fv(r1, 1, false, r1v);
    drawQuad(program, "a_position", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 255, 255, 255), 1.0);
}

// Test that literal infinity can be written out from the shader translator.
// A similar test can't be made for NaNs, since ESSL 3.00.6 requirements for NaNs are very loose.
TEST_P(GLSLTest_ES3, LiteralInfinityOutput)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 out_color;\n"
        "uniform float u;\n"
        "void main()\n"
        "{\n"
        "   float infVar = 1.0e40 - u;\n"
        "   bool correct = isinf(infVar) && infVar > 0.0;\n"
        "   out_color = correct ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that literal negative infinity can be written out from the shader translator.
// A similar test can't be made for NaNs, since ESSL 3.00.6 requirements for NaNs are very loose.
TEST_P(GLSLTest_ES3, LiteralNegativeInfinityOutput)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 out_color;\n"
        "uniform float u;\n"
        "void main()\n"
        "{\n"
        "   float infVar = -1.0e40 + u;\n"
        "   bool correct = isinf(infVar) && infVar < 0.0;\n"
        "   out_color = correct ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// The following MultipleDeclaration* tests are testing TranslatorHLSL specific simplification
// passes. Because the interaction of multiple passes must be tested, it is difficult to write
// a unittest for them. Instead we add the tests as end2end so will in particular test
// TranslatorHLSL when run on Windows.

// Test that passes splitting multiple declarations and comma operators are correctly ordered.
TEST_P(GLSLTest_ES3, MultipleDeclarationWithCommaOperator)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

uniform float u;
float c = 0.0;
float sideEffect()
{
    c = u;
    return c;
}

void main(void)
{
    float a = 0.0, b = ((gl_FragCoord.x < 0.5 ? a : sideEffect()), a);
    color = vec4(b + c);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
}

// Test that passes splitting multiple declarations and comma operators and for loops are
// correctly ordered.
TEST_P(GLSLTest_ES3, MultipleDeclarationWithCommaOperatorInForLoop)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

uniform float u;
float c = 0.0;
float sideEffect()
{
    c = u;
    return c;
}

void main(void)
{
    for(float a = 0.0, b = ((gl_FragCoord.x < 0.5 ? a : sideEffect()), a); a < 10.0; a++)
    {
        b += 1.0;
        color = vec4(b);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
}

// Test that splitting multiple declaration in for loops works with no loop condition
TEST_P(GLSLTest_ES3, MultipleDeclarationInForLoopEmptyCondition)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "void main(void)\n"
        "{\n"
        " for(float a = 0.0, b = 1.0;; a++)\n"
        " {\n"
        "  b += 1.0;\n"
        "  if (a > 10.0) {break;}\n"
        "  color = vec4(b);\n"
        " }\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
}

// Test that splitting multiple declaration in for loops works with no loop expression
TEST_P(GLSLTest_ES3, MultipleDeclarationInForLoopEmptyExpression)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 color;\n"
        "void main(void)\n"
        "{\n"
        " for(float a = 0.0, b = 1.0; a < 10.0;)\n"
        " {\n"
        "  b += 1.0;\n"
        "  a += 1.0;\n"
        "  color = vec4(b);\n"
        " }\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
}

// Test that dynamic indexing of a matrix inside a dynamic indexing of a vector in an l-value works
// correctly.
TEST_P(GLSLTest_ES3, NestedDynamicIndexingInLValue)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform int u_zero;\n"
        "void main() {\n"
        "    mat2 m = mat2(0.0, 0.0, 0.0, 0.0);\n"
        "    m[u_zero + 1][u_zero + 1] = float(u_zero + 1);\n"
        "    float f = m[1][1];\n"
        "    my_FragColor = vec4(1.0 - f, f, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

class WebGLGLSLTest : public GLSLTest
{
  protected:
    WebGLGLSLTest() { setWebGLCompatibilityEnabled(true); }
};

class WebGL2GLSLTest : public GLSLTest
{
  protected:
    WebGL2GLSLTest() { setWebGLCompatibilityEnabled(true); }
};

TEST_P(WebGLGLSLTest, MaxVaryingVec4PlusFragCoord)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    // Generate shader code that uses gl_FragCoord, a special fragment shader variables.
    // This test should fail, since we are really using (maxVaryings + 1) varyings.
    VaryingTestBase(0, 0, 0, 0, 0, 0, maxVaryings, 0, true, false, false, false);
}

TEST_P(WebGLGLSLTest, MaxVaryingVec4PlusPointCoord)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    // Generate shader code that uses gl_FragCoord, a special fragment shader variables.
    // This test should fail, since we are really using (maxVaryings + 1) varyings.
    VaryingTestBase(0, 0, 0, 0, 0, 0, maxVaryings, 0, false, true, false, false);
}

TEST_P(WebGLGLSLTest, MaxPlusOneVaryingVec3)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 0, 0, maxVaryings + 1, 0, 0, 0, false, false, false, false);
}

TEST_P(WebGLGLSLTest, MaxPlusOneVaryingVec3Array)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 0, 0, 0, maxVaryings / 2 + 1, 0, 0, false, false, false, false);
}

TEST_P(WebGLGLSLTest, MaxVaryingVec3AndOneVec2)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 1, 0, maxVaryings, 0, 0, 0, false, false, false, false);
}

TEST_P(WebGLGLSLTest, MaxPlusOneVaryingVec2)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 2 * maxVaryings + 1, 0, 0, 0, 0, 0, false, false, false, false);
}

TEST_P(WebGLGLSLTest, MaxVaryingVec3ArrayAndMaxPlusOneFloatArray)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, maxVaryings / 2 + 1, 0, 0, 0, 0, 0, maxVaryings / 2, false, false, false,
                    false);
}

// Test that FindLSB and FindMSB return correct values in their corner cases.
TEST_P(GLSLTest_ES31, FindMSBAndFindLSBCornerCases)
{
    // Suspecting AMD driver bug - failure seen on bots running on AMD R5 230.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL() && IsLinux());

    // Failing on N5X Oreo http://anglebug.com/42261013
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsAdreno() && IsOpenGLES());

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform int u_zero;\n"
        "void main() {\n"
        "    if (findLSB(u_zero) == -1 && findMSB(u_zero) == -1 && findMSB(u_zero - 1) == -1)\n"
        "    {\n"
        "        my_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        my_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "    }\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that reading from a swizzled vector that is dynamically indexed succeeds.
TEST_P(GLSLTest_ES3, ReadFromDynamicIndexingOfSwizzledVector)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;

uniform int index;
uniform vec4 data;

out vec4 color;
void main() {
    color = vec4(vec4(data.x, data.y, data.z, data.w).zyxw[index], 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint dataLoc = glGetUniformLocation(program, "data");
    glUniform4f(dataLoc, 0.2, 0.4, 0.6, 0.8);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);

    EXPECT_PIXEL_NEAR(0, 0, 153, 0, 0, 255, 1);
}

// Test that writing into a swizzled vector that is dynamically indexed succeeds.
TEST_P(GLSLTest_ES3, WriteIntoDynamicIndexingOfSwizzledVector)
{
    // http://anglebug.com/40644616
    ANGLE_SKIP_TEST_IF(IsOpenGL());

    // The shader first assigns v.x to v.z (1.0)
    // Then v.y to v.y (2.0)
    // Then v.z to v.x (1.0)
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    vec3 v = vec3(1.0, 2.0, 3.0);\n"
        "    for (int i = 0; i < 3; i++) {\n"
        "        v.zyx[i] = v[i];\n"
        "    }\n"
        "    my_FragColor = distance(v, vec3(1.0, 2.0, 1.0)) < 0.01 ? vec4(0, 1, 0, 1) : vec4(1, "
        "0, 0, 1);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test including Ternary using a uniform block is correctly
// expanded.
TEST_P(GLSLTest_ES3, NamelessUniformBlockTernary)
{
    const char kVS[] = R"(#version 300 es
    precision highp float;
    out vec4 color_interp;
    void  main()
    {
        color_interp = vec4(0.0);
    }
)";
    const char kFS[] = R"(#version 300 es
    precision highp float;
    out vec4 fragColor;
    in vec4 color_interp;
layout(std140) uniform TestData {
    int a;
    int b;
};
void main()
{
    int c, a1;
    a1 += c > 0 ? a : b;
    fragColor = vec4(a1,a1,a1,1.0);
}
)";
    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
}

// Test that uniform block variables work as comma expression results.
TEST_P(GLSLTest_ES3, UniformBlockCommaExpressionResult)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
layout (std140) uniform C {
    float u;
    float v;
};
out vec4 o;
void main() {
    vec2 z = vec2(1.0 - u, v);
    vec2 b = vec2((z=z, u)); // Being tested.
    o = vec4(b.x, z.x, b.x, 1.0);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    constexpr GLfloat kInput1Data[2] = {1.f, 0.f};
    GLBuffer input1;
    glBindBuffer(GL_UNIFORM_BUFFER, input1);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat) * 2, &kInput1Data, GL_STATIC_COPY);
    const GLuint kInput1Index = glGetUniformBlockIndex(program, "C");
    glUniformBlockBinding(program, kInput1Index, 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, input1);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::magenta);
    ASSERT_GL_NO_ERROR();
}

// Test that the length() method is correctly translated in Vulkan atomic counter buffer emulation.
TEST_P(GLSLTest_ES31, AtomicCounterArrayLength)
{
    // Crashes on an assertion.  The driver reports no atomic counter buffers when queried from the
    // program, but ANGLE believes there to be one.
    //
    // This is likely due to the fact that ANGLE generates the following code, as a side effect of
    // the code on which .length() is being called:
    //
    //     _uac1[(_uvalue = _utestSideEffectValue)];
    //
    // The driver is optimizing the subscription out, and calling the atomic counter inactive.  This
    // was observed on nvidia, mesa and amd/windows.
    //
    // The fix would be for ANGLE to skip uniforms it believes should exist, but when queried, the
    // driver says don't.
    //
    // http://anglebug.com/42262426
    ANGLE_SKIP_TEST_IF(IsOpenGL());

    constexpr char kCS[] = R"(#version 310 es
precision mediump float;
layout(local_size_x=1) in;

layout(binding = 0) uniform atomic_uint ac1[2][3][4];
uniform uint testSideEffectValue;

layout(binding = 1, std140) buffer Result
{
    uint value;
} result;

void main() {
    bool passed = true;
    if (ac1.length() != 2)
    {
        passed = false;
    }
    uint value = 0u;
    if (ac1[value = testSideEffectValue].length() != 3)
    {
        passed = false;
    }
    if (value != testSideEffectValue)
    {
        passed = false;
    }
    if (ac1[1][value = testSideEffectValue + 1u].length() != 4)
    {
        passed = false;
    }
    if (value != testSideEffectValue + 1u)
    {
        passed = false;
    }
    result.value = passed ? 255u : 127u;
})";

    constexpr unsigned int kUniformTestValue     = 17;
    constexpr unsigned int kExpectedSuccessValue = 255;
    constexpr unsigned int kAtomicCounterRows    = 2;
    constexpr unsigned int kAtomicCounterCols    = 3;

    GLint maxAtomicCounters = 0;
    glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTERS, &maxAtomicCounters);
    EXPECT_GL_NO_ERROR();

    // Required minimum is 8 by the spec
    EXPECT_GE(maxAtomicCounters, 8);
    ANGLE_SKIP_TEST_IF(static_cast<uint32_t>(maxAtomicCounters) <
                       kAtomicCounterRows * kAtomicCounterCols);

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    glUseProgram(program);

    constexpr unsigned int kBufferData[kAtomicCounterRows * kAtomicCounterCols] = {};
    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(kBufferData), kBufferData, GL_STATIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);

    constexpr unsigned int kOutputInitValue = 0;
    GLBuffer shaderStorageBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kOutputInitValue), &kOutputInitValue,
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shaderStorageBuffer);

    GLint uniformLocation = glGetUniformLocation(program, "testSideEffectValue");
    EXPECT_NE(uniformLocation, -1);
    glUniform1ui(uniformLocation, kUniformTestValue);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), GL_MAP_READ_BIT));
    EXPECT_EQ(*ptr, kExpectedSuccessValue);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that inactive images don't cause any errors.
TEST_P(GLSLTest_ES31, InactiveImages)
{
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(rgba32ui) uniform highp readonly uimage2D image1;
layout(rgba32ui) uniform highp readonly uimage2D image2[4];
void main()
{
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Verify that the images are indeed inactive.
    GLuint index = glGetProgramResourceIndex(program, GL_UNIFORM, "image1");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_INVALID_INDEX, index);

    index = glGetProgramResourceIndex(program, GL_UNIFORM, "image2");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_INVALID_INDEX, index);
}

// Test that inactive atomic counters don't cause any errors.
TEST_P(GLSLTest_ES31, InactiveAtomicCounters)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
layout(binding = 0, offset = 0) uniform atomic_uint ac1;
layout(binding = 0, offset = 4) uniform atomic_uint ac2[5];
void main()
{
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    // Verify that the atomic counters are indeed inactive.
    GLuint index = glGetProgramResourceIndex(program, GL_UNIFORM, "ac1");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_INVALID_INDEX, index);

    index = glGetProgramResourceIndex(program, GL_UNIFORM, "ac2");
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_INVALID_INDEX, index);
}

// Test that inactive samplers in structs don't cause any errors.
TEST_P(GLSLTest_ES31, InactiveSamplersInStructCS)
{
    constexpr char kCS[] = R"(#version 310 es
layout(local_size_x=1, local_size_y=1, local_size_z=1) in;
struct S
{
    vec4 v;
    sampler2D t[10];
};
uniform S s;
void main()
{
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();
}

// Test that array indices for arrays of arrays of basic types work as expected.
TEST_P(GLSLTest_ES31, ArraysOfArraysBasicType)
{
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform ivec2 test[2][2];\n"
        "void main() {\n"
        "    bool passed = true;\n"
        "    for (int i = 0; i < 2; i++) {\n"
        "        for (int j = 0; j < 2; j++) {\n"
        "            if (test[i][j] != ivec2(i + 1, j + 1)) {\n"
        "                passed = false;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            std::stringstream uniformName;
            uniformName << "test[" << i << "][" << j << "]";
            GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
            // All array indices should be used.
            EXPECT_NE(uniformLocation, -1);
            glUniform2i(uniformLocation, i + 1, j + 1);
        }
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that array indices for arrays of arrays of basic types work as expected
// inside blocks.
TEST_P(GLSLTest_ES31, ArraysOfArraysBlockBasicType)
{
    // anglebug.com/42262465 - fails on AMD Windows
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsOpenGL());
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "layout(packed) uniform UBO { ivec2 test[2][2]; } ubo_data;\n"
        "void main() {\n"
        "    bool passed = true;\n"
        "    for (int i = 0; i < 2; i++) {\n"
        "        for (int j = 0; j < 2; j++) {\n"
        "            if (ubo_data.test[i][j] != ivec2(i + 1, j + 1)) {\n"
        "                passed = false;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    // Use interface queries to determine buffer size and offset
    GLuint uboBlockIndex   = glGetProgramResourceIndex(program, GL_UNIFORM_BLOCK, "UBO");
    GLenum uboDataSizeProp = GL_BUFFER_DATA_SIZE;
    GLint uboDataSize;
    glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, uboBlockIndex, 1, &uboDataSizeProp, 1,
                           nullptr, &uboDataSize);
    std::unique_ptr<char[]> uboData(new char[uboDataSize]);
    for (int i = 0; i < 2; i++)
    {
        std::stringstream resourceName;
        resourceName << "UBO.test[" << i << "][0]";
        GLenum resourceProps[] = {GL_ARRAY_STRIDE, GL_OFFSET};
        struct
        {
            GLint stride;
            GLint offset;
        } values;
        GLuint resourceIndex =
            glGetProgramResourceIndex(program, GL_UNIFORM, resourceName.str().c_str());
        ASSERT_NE(resourceIndex, GL_INVALID_INDEX);
        glGetProgramResourceiv(program, GL_UNIFORM, resourceIndex, 2, &resourceProps[0], 2, nullptr,
                               &values.stride);
        for (int j = 0; j < 2; j++)
        {
            GLint(&dataPtr)[2] =
                *reinterpret_cast<GLint(*)[2]>(&uboData[values.offset + j * values.stride]);
            dataPtr[0] = i + 1;
            dataPtr[1] = j + 1;
        }
    }
    GLBuffer ubo;
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, uboDataSize, &uboData[0], GL_STATIC_DRAW);
    GLuint ubo_index = glGetUniformBlockIndex(program, "UBO");
    ASSERT_NE(ubo_index, GL_INVALID_INDEX);
    glUniformBlockBinding(program, ubo_index, 5);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, ubo);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that arrays of arrays of samplers work as expected.
TEST_P(GLSLTest_ES31, ArraysOfArraysSampler)
{
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform mediump isampler2D test[2][2];\n"
        "void main() {\n"
        "    bool passed = true;\n"
        "#define DO_CHECK(i,j) \\\n"
        "    if (texture(test[i][j], vec2(0.0, 0.0)) != ivec4(i + 1, j + 1, 0, 1)) { \\\n"
        "        passed = false; \\\n"
        "    }\n"
        "    DO_CHECK(0, 0)\n"
        "    DO_CHECK(0, 1)\n"
        "    DO_CHECK(1, 0)\n"
        "    DO_CHECK(1, 1)\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[2][2];
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            // First generate the texture
            int textureUnit = i * 2 + j;
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, textures[i][j]);
            GLint texData[2] = {i + 1, j + 1};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &texData[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // Then send it as a uniform
            std::stringstream uniformName;
            uniformName << "test[" << i << "][" << j << "]";
            GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
            // All array indices should be used.
            EXPECT_NE(uniformLocation, -1);
            glUniform1i(uniformLocation, textureUnit);
        }
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that arrays of arrays of images work as expected.
TEST_P(GLSLTest_ES31, ArraysOfArraysImage)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    // Fails on D3D due to mistranslation.
    ANGLE_SKIP_TEST_IF(IsD3D());

    // Fails on Android on GLES.
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    GLint maxTextures, maxComputeImageUniforms;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
    glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &maxComputeImageUniforms);
    ANGLE_SKIP_TEST_IF(maxTextures < 1 * 2 * 3);
    ANGLE_SKIP_TEST_IF(maxComputeImageUniforms < 1 * 2 * 3);

    constexpr char kComputeShader[] = R"(#version 310 es
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(binding = 0, r32ui) uniform highp readonly uimage2D image[1][2][3];
        layout(binding = 1, std430) buffer Output {
            uint image_value;
        } outbuf;

        void main(void)
        {
            outbuf.image_value = uint(0.0);
            outbuf.image_value += imageLoad(image[0][0][0], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image[0][0][1], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image[0][0][2], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image[0][1][0], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image[0][1][1], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image[0][1][2], ivec2(0, 0)).x;
        })";
    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    GLuint outputInitData[1] = {10};
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outputInitData), outputInitData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
    EXPECT_GL_NO_ERROR();

    GLuint imageData = 200u;
    GLTexture images[1][2][3];
    for (int i = 0; i < 1; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                glBindTexture(GL_TEXTURE_2D, images[i][j][k]);
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
                                &imageData);
                glBindImageTexture(i * 6 + j * 3 + k, images[i][j][k], 0, GL_FALSE, 0, GL_READ_ONLY,
                                   GL_R32UI);
                EXPECT_GL_NO_ERROR();
            }
        }
    }

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // read back
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(outputInitData), GL_MAP_READ_BIT));
    memcpy(outputInitData, ptr, sizeof(outputInitData));
    EXPECT_EQ(outputInitData[0], imageData * 1 * 2 * 3);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that multiple arrays of arrays of images work as expected.
TEST_P(GLSLTest_ES31, ConsecutiveArraysOfArraysImage)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    // Fails on D3D due to mistranslation.
    ANGLE_SKIP_TEST_IF(IsD3D());

    constexpr GLsizei kImage1Layers = 3;
    constexpr GLsizei kImage1Rows   = 2;
    constexpr GLsizei kImage1Cols   = 1;
    constexpr GLsizei kImage2Rows   = 2;
    constexpr GLsizei kImage2Cols   = 4;

    constexpr GLsizei kImage1Units = kImage1Layers * kImage1Rows * kImage1Cols;
    constexpr GLsizei kImage2Units = kImage2Rows * kImage2Cols;
    constexpr GLsizei kImage3Units = 1;

    constexpr GLsizei kTotalImageCount = kImage1Units + kImage2Units + kImage3Units;

    GLint maxTextures, maxComputeImageUniforms;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
    glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &maxComputeImageUniforms);
    ANGLE_SKIP_TEST_IF(maxTextures < kTotalImageCount);
    ANGLE_SKIP_TEST_IF(maxComputeImageUniforms < kTotalImageCount);

    constexpr char kComputeShader[] = R"(#version 310 es
        layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
        layout(binding = 0, r32ui) uniform highp readonly uimage2D image1[3][2][1];
        layout(binding = 6, r32ui) uniform highp readonly uimage2D image2[2][4];
        layout(binding = 14, r32ui) uniform highp readonly uimage2D image3;
        layout(binding = 0, std430) buffer Output {
            uint image_value;
        } outbuf;

        void main(void)
        {
            outbuf.image_value = uint(0.0);

            outbuf.image_value += imageLoad(image1[0][0][0], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image1[0][1][0], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image1[1][0][0], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image1[1][1][0], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image1[2][0][0], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image1[2][1][0], ivec2(0, 0)).x;

            outbuf.image_value += imageLoad(image2[0][0], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image2[0][1], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image2[0][2], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image2[0][3], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image2[1][0], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image2[1][1], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image2[1][2], ivec2(0, 0)).x;
            outbuf.image_value += imageLoad(image2[1][3], ivec2(0, 0)).x;

            outbuf.image_value += imageLoad(image3, ivec2(0, 0)).x;
        })";
    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    constexpr GLuint kOutputInitData = 10;
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kOutputInitData), &kOutputInitData,
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, outputBuffer);
    EXPECT_GL_NO_ERROR();

    constexpr GLsizei kImage1Binding = 0;
    constexpr GLsizei kImage2Binding = kImage1Binding + kImage1Units;
    constexpr GLsizei kImage3Binding = kImage2Binding + kImage2Units;

    constexpr GLuint kImage1Data = 13;
    GLTexture images1[kImage1Layers][kImage1Rows][kImage1Cols];
    for (int layer = 0; layer < kImage1Layers; layer++)
    {
        for (int row = 0; row < kImage1Rows; row++)
        {
            for (int col = 0; col < kImage1Cols; col++)
            {
                glBindTexture(GL_TEXTURE_2D, images1[layer][row][col]);
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
                                &kImage1Data);
                glBindImageTexture(kImage1Binding + (layer * kImage1Rows + row) * kImage1Cols + col,
                                   images1[layer][row][col], 0, GL_FALSE, 0, GL_READ_ONLY,
                                   GL_R32UI);
                EXPECT_GL_NO_ERROR();
            }
        }
    }

    constexpr GLuint kImage2Data = 17;
    GLTexture images2[kImage2Rows][kImage2Cols];
    for (int row = 0; row < kImage2Rows; row++)
    {
        for (int col = 0; col < kImage2Cols; col++)
        {
            glBindTexture(GL_TEXTURE_2D, images2[row][col]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT,
                            &kImage2Data);
            glBindImageTexture(kImage2Binding + row * kImage2Cols + col, images2[row][col], 0,
                               GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
            EXPECT_GL_NO_ERROR();
        }
    }

    constexpr GLuint kImage3Data = 19;
    GLTexture image3;
    glBindTexture(GL_TEXTURE_2D, image3);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &kImage3Data);
    glBindImageTexture(kImage3Binding, image3, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // read back
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kOutputInitData), GL_MAP_READ_BIT));
    EXPECT_EQ(*ptr,
              kImage1Data * kImage1Units + kImage2Data * kImage2Units + kImage3Data * kImage3Units);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that arrays of arrays of images of r32f format work when passed to functions.
TEST_P(GLSLTest_ES31, ArraysOfArraysOfR32fImages)
{
    // Skip if GL_OES_shader_image_atomic is not enabled.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_shader_image_atomic"));

    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    // Fails on D3D due to mistranslation.
    ANGLE_SKIP_TEST_IF(IsD3D());

    // Fails on Android on GLES.
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // http://anglebug.com/42263895
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGL());

    GLint maxComputeImageUniforms;
    glGetIntegerv(GL_MAX_COMPUTE_IMAGE_UNIFORMS, &maxComputeImageUniforms);
    ANGLE_SKIP_TEST_IF(maxComputeImageUniforms < 7);

    constexpr char kComputeShader[] = R"(#version 310 es
#extension GL_OES_shader_image_atomic : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 0, r32f) uniform highp image2D image1[2][3];
layout(binding = 6, r32f) uniform highp image2D image2;

void testFunction(highp image2D imageOut[2][3])
{
    // image1 is an array of 1x1 images.
    // image2 is a 1x4 image with the following data:
    //
    // (0, 0): 234.5
    // (0, 1): 4.0
    // (0, 2): 456.0
    // (0, 3): 987.0


    // Write to [0][0]
    imageStore(imageOut[0][0], ivec2(0, 0), vec4(1234.5));

    // Write to [0][1]
    imageStore(imageOut[0][1], ivec2(0, 0), imageLoad(image2, ivec2(0, 0)));

    // Write to [0][2]
    imageStore(imageOut[0][2], ivec2(0, 0), vec4(imageSize(image2).y));

    // Write to [1][0]
    imageStore(imageOut[1][0], ivec2(0,
                 imageSize(image2).y - int(imageLoad(image2, ivec2(0, 1)).x)
                ), vec4(678.0));

    // Write to [1][1]
    imageStore(imageOut[1][1], ivec2(0, 0),
                vec4(imageAtomicExchange(image2, ivec2(0, 2), 135.0)));

    // Write to [1][2]
    imageStore(imageOut[1][2], ivec2(0, 0),
                    imageLoad(image2, ivec2(imageSize(image2).x - 1, 3)));
}

void main(void)
{
    testFunction(image1);
})";
    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    constexpr GLsizei kImageRows = 2;
    constexpr GLsizei kImageCols = 3;
    constexpr GLfloat kImageData = 0;
    GLTexture images[kImageRows][kImageCols];
    for (size_t row = 0; row < kImageRows; row++)
    {
        for (size_t col = 0; col < kImageCols; col++)
        {
            glBindTexture(GL_TEXTURE_2D, images[row][col]);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, 1, 1);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED, GL_FLOAT, &kImageData);
            glBindImageTexture(row * kImageCols + col, images[row][col], 0, GL_FALSE, 0,
                               GL_READ_WRITE, GL_R32F);
            EXPECT_GL_NO_ERROR();
        }
    }

    constexpr GLsizei kImage2Size                          = 4;
    constexpr std::array<GLfloat, kImage2Size> kImage2Data = {
        234.5f,
        4.0f,
        456.0f,
        987.0f,
    };
    GLTexture image2;
    glBindTexture(GL_TEXTURE_2D, image2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, 1, kImage2Size);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, kImage2Size, GL_RED, GL_FLOAT, kImage2Data.data());
    glBindImageTexture(6, image2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Verify the previous dispatch with another dispatch
    constexpr char kVerifyShader[] = R"(#version 310 es
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 0, r32f) uniform highp readonly image2D image1[2][3];
layout(binding = 6, r32f) uniform highp readonly image2D image2;
layout(binding = 0, std430) buffer Output {
    float image2Data[4];
    float image1Data[6];
} outbuf;

void main(void)
{
    for (int i = 0; i < 4; ++i)
    {
        outbuf.image2Data[i] = imageLoad(image2, ivec2(0, i)).x;
    }
    outbuf.image1Data[0] = imageLoad(image1[0][0], ivec2(0, 0)).x;
    outbuf.image1Data[1] = imageLoad(image1[0][1], ivec2(0, 0)).x;
    outbuf.image1Data[2] = imageLoad(image1[0][2], ivec2(0, 0)).x;
    outbuf.image1Data[3] = imageLoad(image1[1][0], ivec2(0, 0)).x;
    outbuf.image1Data[4] = imageLoad(image1[1][1], ivec2(0, 0)).x;
    outbuf.image1Data[5] = imageLoad(image1[1][2], ivec2(0, 0)).x;
})";
    ANGLE_GL_COMPUTE_PROGRAM(verifyProgram, kVerifyShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(verifyProgram);

    constexpr std::array<GLfloat, kImage2Size + kImageRows * kImageCols> kOutputInitData = {};
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(kOutputInitData), kOutputInitData.data(),
                 GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, outputBuffer);
    EXPECT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // Verify
    const GLfloat *ptr = reinterpret_cast<const GLfloat *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(kOutputInitData), GL_MAP_READ_BIT));

    EXPECT_EQ(ptr[0], kImage2Data[0]);
    EXPECT_EQ(ptr[1], kImage2Data[1]);
    EXPECT_NEAR(ptr[2], 135.0f, 0.0001f);
    EXPECT_EQ(ptr[3], kImage2Data[3]);

    EXPECT_NEAR(ptr[4], 1234.5f, 0.0001f);
    EXPECT_NEAR(ptr[5], kImage2Data[0], 0.0001f);
    EXPECT_NEAR(ptr[6], kImage2Size, 0.0001f);
    EXPECT_NEAR(ptr[7], 678.0f, 0.0001f);
    EXPECT_NEAR(ptr[8], kImage2Data[2], 0.0001f);
    EXPECT_NEAR(ptr[9], kImage2Data[3], 0.0001f);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Check that imageLoad gives the correct color after clearing the texture -- anglebug.com/42265826
TEST_P(GLSLTest_ES31, ImageLoadAfterClear)
{
    ANGLE_GL_PROGRAM(program,
                     R"(#version 310 es
precision highp float;
void main()
{
    gl_Position.x = ((gl_VertexID & 1) == 0 ? -1.0 : 1.0);
    gl_Position.y = ((gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    gl_Position.zw = vec2(0, 1);
})",

                     R"(#version 310 es
precision highp float;
layout(binding=0, rgba8) readonly highp uniform image2D img;
out vec4 fragColor;
void main()
{
    ivec2 imgcoord = ivec2(floor(gl_FragCoord.xy));
    fragColor = vec4(1, 0, 0, 0) + imageLoad(img, imgcoord);
})");
    ASSERT_TRUE(program.valid());
    glUseProgram(program);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

    // Clear the texture to green.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);

    // Draw the texture via imageLoad, plus red, into the main framebuffer. Make sure the texture
    // was still green. (green + red == yellow.)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::yellow);
    ASSERT_GL_NO_ERROR();
}

// Check that writeonly image2D handles can be passed as function args.
TEST_P(GLSLTest_ES31, WriteOnlyImage2DAsFunctionArg)
{
    // Create an image.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    // Clear the image to red.
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    const char kVS[] = R"(#version 310 es
precision highp float;
void main()
{
    gl_Position.x = ((gl_VertexID & 1) == 0 ? -1.0 : 1.0);
    gl_Position.y = ((gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    gl_Position.zw = vec2(0, 1);
})";

    const char kFS[] = R"(#version 310 es
precision highp float;
layout(binding=0, rgba8) writeonly highp uniform image2D uniformImage;
void store(writeonly highp image2D img, vec4 color)
{
    ivec2 imgcoord = ivec2(floor(gl_FragCoord.xy));
    imageStore(img, imgcoord, color);
}
void main()
{
    store(uniformImage, vec4(1, 1, 0, 1));
})";

    // Store yellow to the image.
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    // Check that the image is yellow.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::yellow);
    ASSERT_GL_NO_ERROR();
}

// Check that readonly image2D handles can be passed as function args.
TEST_P(GLSLTest_ES31, ReadOnlyImage2DAsFunctionArg)
{
    const int w = getWindowWidth();
    const int h = getWindowHeight();

    // Create an image.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);

    const std::vector<GLColor> kInitData(w * h, GLColor::red);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, kInitData.data());

    // Create a framebuffer.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    EXPECT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Initialize the framebuffer with the contents of the texture.
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    const char kVS[] = R"(#version 310 es
precision highp float;
void main()
{
    gl_Position.x = ((gl_VertexID & 1) == 0 ? -1.0 : 1.0);
    gl_Position.y = ((gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    gl_Position.zw = vec2(0, 1);
})";

    const char kFS[] = R"(#version 310 es
precision highp float;
layout(binding=0, rgba8) readonly highp uniform image2D uniformImage;
out vec4 color;
vec4 load(readonly highp image2D img)
{
    ivec2 imgcoord = ivec2(floor(gl_FragCoord.xy));
    return imageLoad(img, imgcoord);
}
void main()
{
    color = load(uniformImage);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Check that the framebuffer is red.
    EXPECT_PIXEL_RECT_EQ(0, 0, w, h, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Check that the volatile keyword combined with memoryBarrierImage() allow load/store from
// different aliases of the same image -- anglebug.com/42265813
//
// ES 3.1 requires most image formats to be either readonly or writeonly. (It appears that this
// limitation exists due to atomics, since we still have the volatile keyword and the built-in
// memoryBarrierImage(), which ought to allow us to load and store different aliases of the same
// image.) To test this, we create two aliases of the same image -- one for reading and one for
// writing.
TEST_P(GLSLTest_ES31, AliasedLoadStore)
{

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    ANGLE_GL_PROGRAM(program,

                     R"(#version 310 es
precision highp float;
void main()
{
    gl_Position.x = ((gl_VertexID & 1) == 0 ? -1.0 : 1.0);
    gl_Position.y = ((gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    gl_Position.zw = vec2(0, 1);
})",

                     R"(#version 310 es
precision highp float;
layout(binding=0, rgba8) volatile readonly highp uniform image2D img_r;
layout(binding=0, rgba8) volatile writeonly highp uniform image2D img_w;
uniform vec4 drawColor;
void main()
{
    ivec2 coord = ivec2(floor(gl_FragCoord.xy));
    vec4 oldval = imageLoad(img_r, coord);
    memoryBarrierImage();
    imageStore(img_w, coord, oldval + drawColor);
})");

    ASSERT_TRUE(program.valid());
    glUseProgram(program);
    GLint drawColorLocation = glGetUniformLocation(program, "drawColor");

    // Tell the driver the binding is GL_READ_WRITE, since it will be referenced by two image2Ds:
    // one readeonly and one writeonly.
    glBindImageTexture(0, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUniform4f(drawColorLocation, 0, 1, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Ensure the cleared color was loaded before we stored.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::yellow);

    ASSERT_GL_NO_ERROR();

    // Now make two draws to ensure the imageStore is coherent.
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUniform4f(drawColorLocation, 0, 1, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glUniform4f(drawColorLocation, 0, 0, 1, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Ensure the first imageStore was loaded by the second imageLoad.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::white);

    ASSERT_GL_NO_ERROR();
}

// Test that structs containing arrays of samplers work as expected.
TEST_P(GLSLTest_ES31, StructArraySampler)
{
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct Data { mediump sampler2D data[2]; };\n"
        "uniform Data test;\n"
        "void main() {\n"
        "    my_FragColor = vec4(texture(test.data[0], vec2(0.0, 0.0)).rg,\n"
        "                        texture(test.data[1], vec2(0.0, 0.0)).rg);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[2];
    GLColor expected = MakeGLColor(32, 64, 96, 255);
    GLubyte data[6]  = {};  // Two bytes of padding, so that texture can be initialized with 4 bytes
    memcpy(data, expected.data(), sizeof(expected));
    for (int i = 0; i < 2; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        // Each element provides two components.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data + 2 * i);
        std::stringstream uniformName;
        uniformName << "test.data[" << i << "]";
        // Then send it as a uniform
        GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
        // The uniform should be active.
        EXPECT_NE(uniformLocation, -1);
        glUniform1i(uniformLocation, i);
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, expected);
}

// Test that arrays of arrays of samplers inside structs work as expected.
TEST_P(GLSLTest_ES31, StructArrayArraySampler)
{
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct Data { mediump isampler2D data[2][2]; };\n"
        "uniform Data test;\n"
        "void main() {\n"
        "    bool passed = true;\n"
        "#define DO_CHECK(i,j) \\\n"
        "    if (texture(test.data[i][j], vec2(0.0, 0.0)) != ivec4(i + 1, j + 1, 0, 1)) { \\\n"
        "        passed = false; \\\n"
        "    }\n"
        "    DO_CHECK(0, 0)\n"
        "    DO_CHECK(0, 1)\n"
        "    DO_CHECK(1, 0)\n"
        "    DO_CHECK(1, 1)\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[2][2];
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            // First generate the texture
            int textureUnit = i * 2 + j;
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, textures[i][j]);
            GLint texData[2] = {i + 1, j + 1};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &texData[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // Then send it as a uniform
            std::stringstream uniformName;
            uniformName << "test.data[" << i << "][" << j << "]";
            GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
            // All array indices should be used.
            EXPECT_NE(uniformLocation, -1);
            glUniform1i(uniformLocation, textureUnit);
        }
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that an array of structs with arrays of arrays of samplers works.
TEST_P(GLSLTest_ES31, ArrayStructArrayArraySampler)
{
    GLint numTextures;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTextures);
    ANGLE_SKIP_TEST_IF(numTextures < 2 * (2 * 2 + 2 * 2));
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct Data { mediump isampler2D data0[2][2]; mediump isampler2D data1[2][2]; };\n"
        "uniform Data test[2];\n"
        "void main() {\n"
        "    bool passed = true;\n"
        "#define DO_CHECK_ikl(i,k,l) \\\n"
        "    if (texture(test[i].data0[k][l], vec2(0.0, 0.0)) != ivec4(i, 0, k, l)+1) { \\\n"
        "        passed = false; \\\n"
        "    } \\\n"
        "    if (texture(test[i].data1[k][l], vec2(0.0, 0.0)) != ivec4(i, 1, k, l)+1) { \\\n"
        "        passed = false; \\\n"
        "    }\n"
        "#define DO_CHECK_ik(i,k) \\\n"
        "    DO_CHECK_ikl(i, k, 0) \\\n"
        "    DO_CHECK_ikl(i, k, 1)\n"
        "#define DO_CHECK_i(i) \\\n"
        "    DO_CHECK_ik(i, 0) \\\n"
        "    DO_CHECK_ik(i, 1)\n"
        "    DO_CHECK_i(0)\n"
        "    DO_CHECK_i(1)\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[2][2][2][2];
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                for (int l = 0; l < 2; l++)
                {
                    // First generate the texture
                    int textureUnit = l + 2 * (k + 2 * (j + 2 * i));
                    glActiveTexture(GL_TEXTURE0 + textureUnit);
                    glBindTexture(GL_TEXTURE_2D, textures[i][j][k][l]);
                    GLint texData[4] = {i + 1, j + 1, k + 1, l + 1};
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, 1, 1, 0, GL_RGBA_INTEGER, GL_INT,
                                 &texData[0]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    // Then send it as a uniform
                    std::stringstream uniformName;
                    uniformName << "test[" << i << "].data" << j << "[" << k << "][" << l << "]";
                    GLint uniformLocation =
                        glGetUniformLocation(program, uniformName.str().c_str());
                    // All array indices should be used.
                    EXPECT_NE(uniformLocation, -1);
                    glUniform1i(uniformLocation, textureUnit);
                }
            }
        }
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a complex chain of structs and arrays of samplers works as expected.
TEST_P(GLSLTest_ES31, ComplexStructArraySampler)
{
    GLint numTextures;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTextures);
    ANGLE_SKIP_TEST_IF(numTextures < 2 * 3 * (2 + 3));
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct Data { mediump isampler2D data0[2]; mediump isampler2D data1[3]; };\n"
        "uniform Data test[2][3];\n"
        "const vec2 ZERO = vec2(0.0, 0.0);\n"
        "void main() {\n"
        "    bool passed = true;\n"
        "#define DO_CHECK_INNER0(i,j,l) \\\n"
        "    if (texture(test[i][j].data0[l], ZERO) != ivec4(i, j, 0, l) + 1) { \\\n"
        "        passed = false; \\\n"
        "    }\n"
        "#define DO_CHECK_INNER1(i,j,l) \\\n"
        "    if (texture(test[i][j].data1[l], ZERO) != ivec4(i, j, 1, l) + 1) { \\\n"
        "        passed = false; \\\n"
        "    }\n"
        "#define DO_CHECK(i,j) \\\n"
        "    DO_CHECK_INNER0(i, j, 0) \\\n"
        "    DO_CHECK_INNER0(i, j, 1) \\\n"
        "    DO_CHECK_INNER1(i, j, 0) \\\n"
        "    DO_CHECK_INNER1(i, j, 1) \\\n"
        "    DO_CHECK_INNER1(i, j, 2)\n"
        "    DO_CHECK(0, 0)\n"
        "    DO_CHECK(0, 1)\n"
        "    DO_CHECK(0, 2)\n"
        "    DO_CHECK(1, 0)\n"
        "    DO_CHECK(1, 1)\n"
        "    DO_CHECK(1, 2)\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    struct Data
    {
        GLTexture data1[2];
        GLTexture data2[3];
    };
    Data textures[2][3];
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            GLTexture *arrays[]     = {&textures[i][j].data1[0], &textures[i][j].data2[0]};
            size_t arrayLengths[]   = {2, 3};
            size_t arrayOffsets[]   = {0, 2};
            size_t totalArrayLength = 5;
            for (int k = 0; k < 2; k++)
            {
                GLTexture *array   = arrays[k];
                size_t arrayLength = arrayLengths[k];
                size_t arrayOffset = arrayOffsets[k];
                for (int l = 0; l < static_cast<int>(arrayLength); l++)
                {
                    // First generate the texture
                    int textureUnit = arrayOffset + l + totalArrayLength * (j + 3 * i);
                    glActiveTexture(GL_TEXTURE0 + textureUnit);
                    glBindTexture(GL_TEXTURE_2D, array[l]);
                    GLint texData[4] = {i + 1, j + 1, k + 1, l + 1};
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, 1, 1, 0, GL_RGBA_INTEGER, GL_INT,
                                 &texData[0]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    // Then send it as a uniform
                    std::stringstream uniformName;
                    uniformName << "test[" << i << "][" << j << "].data" << k << "[" << l << "]";
                    GLint uniformLocation =
                        glGetUniformLocation(program, uniformName.str().c_str());
                    // All array indices should be used.
                    EXPECT_NE(uniformLocation, -1);
                    glUniform1i(uniformLocation, textureUnit);
                }
            }
        }
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

TEST_P(GLSLTest_ES31, ArraysOfArraysStructDifferentTypesSampler)
{
    GLint numTextures;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTextures);
    ANGLE_SKIP_TEST_IF(numTextures < 3 * (2 + 2));
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct Data { mediump isampler2D data0[2]; mediump sampler2D data1[2]; };\n"
        "uniform Data test[3];\n"
        "ivec4 f2i(vec4 x) { return ivec4(x * 4.0 + 0.5); }"
        "void main() {\n"
        "    bool passed = true;\n"
        "#define DO_CHECK_ik(i,k) \\\n"
        "    if (texture(test[i].data0[k], vec2(0.0, 0.0)) != ivec4(i, 0, k, 0)+1) { \\\n"
        "        passed = false; \\\n"
        "    } \\\n"
        "    if (f2i(texture(test[i].data1[k], vec2(0.0, 0.0))) != ivec4(i, 1, k, 0)+1) { \\\n"
        "        passed = false; \\\n"
        "    }\n"
        "#define DO_CHECK_i(i) \\\n"
        "    DO_CHECK_ik(i, 0) \\\n"
        "    DO_CHECK_ik(i, 1)\n"
        "    DO_CHECK_i(0)\n"
        "    DO_CHECK_i(1)\n"
        "    DO_CHECK_i(2)\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[3][2][2];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                // First generate the texture
                int textureUnit = k + 2 * (j + 2 * i);
                glActiveTexture(GL_TEXTURE0 + textureUnit);
                glBindTexture(GL_TEXTURE_2D, textures[i][j][k]);
                GLint texData[4]        = {i + 1, j + 1, k + 1, 1};
                GLubyte texDataFloat[4] = {static_cast<GLubyte>((i + 1) * 64 - 1),
                                           static_cast<GLubyte>((j + 1) * 64 - 1),
                                           static_cast<GLubyte>((k + 1) * 64 - 1), 64};
                if (j == 0)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, 1, 1, 0, GL_RGBA_INTEGER, GL_INT,
                                 &texData[0]);
                }
                else
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                                 &texDataFloat[0]);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                // Then send it as a uniform
                std::stringstream uniformName;
                uniformName << "test[" << i << "].data" << j << "[" << k << "]";
                GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
                // All array indices should be used.
                EXPECT_NE(uniformLocation, -1);
                glUniform1i(uniformLocation, textureUnit);
            }
        }
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that arrays of arrays of samplers as parameters works as expected.
TEST_P(GLSLTest_ES31, ParameterArraysOfArraysSampler)
{
    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform mediump isampler2D test[2][3];\n"
        "const vec2 ZERO = vec2(0.0, 0.0);\n"
        "\n"
        "bool check(mediump isampler2D data[2][3]);\n"
        "bool check(mediump isampler2D data[2][3]) {\n"
        "#define DO_CHECK(i,j) \\\n"
        "    if (texture(data[i][j], ZERO) != ivec4(i+1, j+1, 0, 1)) { \\\n"
        "        return false; \\\n"
        "    }\n"
        "    DO_CHECK(0, 0)\n"
        "    DO_CHECK(0, 1)\n"
        "    DO_CHECK(0, 2)\n"
        "    DO_CHECK(1, 0)\n"
        "    DO_CHECK(1, 1)\n"
        "    DO_CHECK(1, 2)\n"
        "    return true;\n"
        "}\n"
        "void main() {\n"
        "    bool passed = check(test);\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[2][3];
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            // First generate the texture
            int textureUnit = i * 3 + j;
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, textures[i][j]);
            GLint texData[2] = {i + 1, j + 1};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &texData[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // Then send it as a uniform
            std::stringstream uniformName;
            uniformName << "test[" << i << "][" << j << "]";
            GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
            // All array indices should be used.
            EXPECT_NE(uniformLocation, -1);
            glUniform1i(uniformLocation, textureUnit);
        }
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that structs with arrays of arrays of samplers as parameters works as expected.
TEST_P(GLSLTest_ES31, ParameterStructArrayArraySampler)
{
    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct Data { mediump isampler2D data[2][3]; };\n"
        "uniform Data test;\n"
        "const vec2 ZERO = vec2(0.0, 0.0);\n"
        "\n"
        "bool check(Data data) {\n"
        "#define DO_CHECK(i,j) \\\n"
        "    if (texture(data.data[i][j], ZERO) != ivec4(i+1, j+1, 0, 1)) { \\\n"
        "        return false; \\\n"
        "    }\n"
        "    DO_CHECK(0, 0)\n"
        "    DO_CHECK(0, 1)\n"
        "    DO_CHECK(0, 2)\n"
        "    DO_CHECK(1, 0)\n"
        "    DO_CHECK(1, 1)\n"
        "    DO_CHECK(1, 2)\n"
        "    return true;\n"
        "}\n"
        "void main() {\n"
        "    bool passed = check(test);\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[2][3];
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            // First generate the texture
            int textureUnit = i * 3 + j;
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, textures[i][j]);
            GLint texData[2] = {i + 1, j + 1};
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32I, 1, 1, 0, GL_RG_INTEGER, GL_INT, &texData[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // Then send it as a uniform
            std::stringstream uniformName;
            uniformName << "test.data[" << i << "][" << j << "]";
            GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
            // All array indices should be used.
            EXPECT_NE(uniformLocation, -1);
            glUniform1i(uniformLocation, textureUnit);
        }
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that arrays of arrays of structs with arrays of arrays of samplers
// as parameters works as expected.
TEST_P(GLSLTest_ES31, ParameterArrayArrayStructArrayArraySampler)
{
    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    GLint numTextures;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTextures);
    ANGLE_SKIP_TEST_IF(numTextures < 3 * 2 * 2 * 2);
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "struct Data { mediump isampler2D data[2][2]; };\n"
        "uniform Data test[3][2];\n"
        "const vec2 ZERO = vec2(0.0, 0.0);\n"
        "\n"
        "bool check(Data data[3][2]) {\n"
        "#define DO_CHECK_ijkl(i,j,k,l) \\\n"
        "    if (texture(data[i][j].data[k][l], ZERO) != ivec4(i, j, k, l) + 1) { \\\n"
        "        return false; \\\n"
        "    }\n"
        "#define DO_CHECK_ij(i,j) \\\n"
        "    DO_CHECK_ijkl(i, j, 0, 0) \\\n"
        "    DO_CHECK_ijkl(i, j, 0, 1) \\\n"
        "    DO_CHECK_ijkl(i, j, 1, 0) \\\n"
        "    DO_CHECK_ijkl(i, j, 1, 1)\n"
        "    DO_CHECK_ij(0, 0)\n"
        "    DO_CHECK_ij(1, 0)\n"
        "    DO_CHECK_ij(2, 0)\n"
        "    DO_CHECK_ij(0, 1)\n"
        "    DO_CHECK_ij(1, 1)\n"
        "    DO_CHECK_ij(2, 1)\n"
        "    return true;\n"
        "}\n"
        "void main() {\n"
        "    bool passed = check(test);\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures[3][2][2][2];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                for (int l = 0; l < 2; l++)
                {
                    // First generate the texture
                    int textureUnit = l + 2 * (k + 2 * (j + 2 * i));
                    glActiveTexture(GL_TEXTURE0 + textureUnit);
                    glBindTexture(GL_TEXTURE_2D, textures[i][j][k][l]);
                    GLint texData[4] = {i + 1, j + 1, k + 1, l + 1};
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, 1, 1, 0, GL_RGBA_INTEGER, GL_INT,
                                 &texData[0]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    // Then send it as a uniform
                    std::stringstream uniformName;
                    uniformName << "test[" << i << "][" << j << "].data[" << k << "][" << l << "]";
                    GLint uniformLocation =
                        glGetUniformLocation(program, uniformName.str().c_str());
                    // All array indices should be used.
                    EXPECT_NE(uniformLocation, -1);
                    glUniform1i(uniformLocation, textureUnit);
                }
            }
        }
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that 3D arrays with sub-arrays passed as parameters works as expected.
TEST_P(GLSLTest_ES31, ParameterArrayArrayArraySampler)
{
    GLint numTextures;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTextures);
    ANGLE_SKIP_TEST_IF(numTextures < 2 * 3 * 4 + 4);

    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // http://anglebug.com/42264082
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsOpenGL());

    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "uniform mediump isampler2D test[2][3][4];\n"
        "uniform mediump isampler2D test2[4];\n"
        "const vec2 ZERO = vec2(0.0, 0.0);\n"
        "\n"
        "bool check1D(mediump isampler2D arr[4], int x, int y) {\n"
        "    if (texture(arr[0], ZERO) != ivec4(x, y, 0, 0)+1) return false;\n"
        "    if (texture(arr[1], ZERO) != ivec4(x, y, 1, 0)+1) return false;\n"
        "    if (texture(arr[2], ZERO) != ivec4(x, y, 2, 0)+1) return false;\n"
        "    if (texture(arr[3], ZERO) != ivec4(x, y, 3, 0)+1) return false;\n"
        "    return true;\n"
        "}\n"
        "bool check2D(mediump isampler2D arr[3][4], int x) {\n"
        "    if (!check1D(arr[0], x, 0)) return false;\n"
        "    if (!check1D(arr[1], x, 1)) return false;\n"
        "    if (!check1D(arr[2], x, 2)) return false;\n"
        "    return true;\n"
        "}\n"
        "bool check3D(mediump isampler2D arr[2][3][4]) {\n"
        "    if (!check2D(arr[0], 0)) return false;\n"
        "    if (!check2D(arr[1], 1)) return false;\n"
        "    return true;\n"
        "}\n"
        "void main() {\n"
        "    bool passed = check3D(test) && check1D(test2, 7, 8);\n"
        "    my_FragColor = passed ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture textures1[2][3][4];
    GLTexture textures2[4];
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                // First generate the texture
                int textureUnit = k + 4 * (j + 3 * i);
                glActiveTexture(GL_TEXTURE0 + textureUnit);
                glBindTexture(GL_TEXTURE_2D, textures1[i][j][k]);
                GLint texData[3] = {i + 1, j + 1, k + 1};
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32I, 1, 1, 0, GL_RGB_INTEGER, GL_INT,
                             &texData[0]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                // Then send it as a uniform
                std::stringstream uniformName;
                uniformName << "test[" << i << "][" << j << "][" << k << "]";
                GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
                // All array indices should be used.
                EXPECT_NE(uniformLocation, -1);
                glUniform1i(uniformLocation, textureUnit);
            }
        }
    }
    for (int k = 0; k < 4; k++)
    {
        // First generate the texture
        int textureUnit = 2 * 3 * 4 + k;
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, textures2[k]);
        GLint texData[3] = {7 + 1, 8 + 1, k + 1};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32I, 1, 1, 0, GL_RGB_INTEGER, GL_INT, &texData[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Then send it as a uniform
        std::stringstream uniformName;
        uniformName << "test2[" << k << "]";
        GLint uniformLocation = glGetUniformLocation(program, uniformName.str().c_str());
        // All array indices should be used.
        EXPECT_NE(uniformLocation, -1);
        glUniform1i(uniformLocation, textureUnit);
    }
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that names do not collide when translating arrays of arrays of samplers.
TEST_P(GLSLTest_ES31, ArraysOfArraysNameCollisionSampler)
{
    ANGLE_SKIP_TEST_IF(IsVulkan());  // anglebug.com/42262269 - rewriter can create name collisions
    GLint numTextures;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTextures);
    ANGLE_SKIP_TEST_IF(numTextures < 2 * 2 + 3 * 3 + 4 * 4);
    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump sampler2D;\n"
        "precision mediump float;\n"
        "uniform sampler2D test_field1_field2[2][2];\n"
        "struct S1 { sampler2D field2[3][3]; }; uniform S1 test_field1;\n"
        "struct S2 { sampler2D field1_field2[4][4]; }; uniform S2 test;\n"
        "vec4 func1(sampler2D param_field1_field2[2][2],\n"
        "           int param_field1_field2_offset,\n"
        "           S1 param_field1,\n"
        "           S2 param) {\n"
        "    return vec4(0.0, 1.0, 0.0, 0.0);\n"
        "}\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "    my_FragColor += func1(test_field1_field2, 0, test_field1, test);\n"
        "    vec2 uv = vec2(0.0);\n"
        "    my_FragColor += texture(test_field1_field2[0][0], uv) +\n"
        "                    texture(test_field1.field2[0][0], uv) +\n"
        "                    texture(test.field1_field2[0][0], uv);\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glActiveTexture(GL_TEXTURE0);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    GLint zero = 0;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &zero);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that regular arrays are unmodified.
TEST_P(GLSLTest_ES31, BasicTypeArrayAndArrayOfSampler)
{
    constexpr char kFS[] =
        "#version 310 es\n"
        "precision mediump sampler2D;\n"
        "precision mediump float;\n"
        "uniform sampler2D sampler_array[2][2];\n"
        "uniform int array[3][2];\n"
        "vec4 func1(int param[2],\n"
        "           int param2[3]) {\n"
        "    return vec4(0.0, 1.0, 0.0, 0.0);\n"
        "}\n"
        "out vec4 my_FragColor;\n"
        "void main() {\n"
        "    my_FragColor = texture(sampler_array[0][0], vec2(0.0));\n"
        "    my_FragColor += func1(array[1], int[](1, 2, 3));\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glActiveTexture(GL_TEXTURE0);
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    GLint zero = 0;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &zero);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// This test covers a bug (and associated workaround) with nested sampling operations in the HLSL
// compiler DLL.
TEST_P(GLSLTest_ES3, NestedSamplingOperation)
{
    // This seems to be bugged on some version of Android. Might not affect the newest versions.
    // TODO(jmadill): Lift suppression when Chromium bots are upgraded.
    // Test skipped on Android because of bug with Nexus 5X.
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    constexpr char kVS[] =
        "#version 300 es\n"
        "out vec2 texCoord;\n"
        "in vec2 position;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    texCoord = position * 0.5 + vec2(0.5);\n"
        "}\n";

    constexpr char kSimpleFS[] =
        "#version 300 es\n"
        "in mediump vec2 texCoord;\n"
        "out mediump vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = vec4(texCoord, 0, 1);\n"
        "}\n";

    constexpr char kNestedFS[] =
        "#version 300 es\n"
        "uniform mediump sampler2D samplerA;\n"
        "uniform mediump sampler2D samplerB;\n"
        "in mediump vec2 texCoord;\n"
        "out mediump vec4 fragColor;\n"
        "void main ()\n"
        "{\n"
        "    fragColor = texture(samplerB, texture(samplerA, texCoord).xy);\n"
        "}\n";

    ANGLE_GL_PROGRAM(initProg, kVS, kSimpleFS);
    ANGLE_GL_PROGRAM(nestedProg, kVS, kNestedFS);

    // Initialize a first texture with default texCoord data.
    GLTexture texA;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texA, 0);

    drawQuad(initProg, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // Initialize a second texture with a simple color pattern.
    GLTexture texB;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texB);

    std::array<GLColor, 4> simpleColors = {
        {GLColor::red, GLColor::green, GLColor::blue, GLColor::yellow}};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 simpleColors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Draw with the nested program, using the first texture to index the second.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(nestedProg);
    GLint samplerALoc = glGetUniformLocation(nestedProg, "samplerA");
    ASSERT_NE(-1, samplerALoc);
    glUniform1i(samplerALoc, 0);
    GLint samplerBLoc = glGetUniformLocation(nestedProg, "samplerB");
    ASSERT_NE(-1, samplerBLoc);
    glUniform1i(samplerBLoc, 1);

    drawQuad(nestedProg, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    // Compute four texel centers.
    Vector2 windowSize(getWindowWidth(), getWindowHeight());
    Vector2 quarterWindowSize = windowSize / 4;
    Vector2 ul                = quarterWindowSize;
    Vector2 ur(windowSize.x() - quarterWindowSize.x(), quarterWindowSize.y());
    Vector2 ll(quarterWindowSize.x(), windowSize.y() - quarterWindowSize.y());
    Vector2 lr = windowSize - quarterWindowSize;

    EXPECT_PIXEL_COLOR_EQ_VEC2(ul, simpleColors[0]);
    EXPECT_PIXEL_COLOR_EQ_VEC2(ur, simpleColors[1]);
    EXPECT_PIXEL_COLOR_EQ_VEC2(ll, simpleColors[2]);
    EXPECT_PIXEL_COLOR_EQ_VEC2(lr, simpleColors[3]);
}

// Tests that using a constant declaration as the only statement in a for loop without curly braces
// doesn't crash.
TEST_P(GLSLTest, ConstantStatementInForLoop)
{
    constexpr char kVS[] =
        "void main()\n"
        "{\n"
        "    for (int i = 0; i < 10; ++i)\n"
        "        const int b = 0;\n"
        "}\n";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Tests that using a constant declaration as a loop init expression doesn't crash. Note that this
// test doesn't work on D3D9 due to looping limitations, so it is only run on ES3.
TEST_P(GLSLTest_ES3, ConstantStatementAsLoopInit)
{
    constexpr char kVS[] =
        "void main()\n"
        "{\n"
        "    for (const int i = 0; i < 0;) {}\n"
        "}\n";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Tests that using a constant condition guarding a discard works
// Covers a failing case in the Vulkan backend: http://anglebug.com/42265506
TEST_P(GLSLTest_ES3, ConstantConditionGuardingDiscard)
{
    constexpr char kFS[] = R"(#version 300 es
void main()
{
    if (true)
    {
        discard;
    }
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Tests that nesting a discard in unconditional blocks works
// Covers a failing case in the Vulkan backend: http://anglebug.com/42265506
TEST_P(GLSLTest_ES3, NestedUnconditionalDiscards)
{
    constexpr char kFS[] = R"(#version 300 es
out mediump vec4 c;
void main()
{
    {
        c = vec4(0);
        {
            discard;
        }
    }
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Test that uninitialized local variables are initialized to 0.
TEST_P(WebGL2GLSLTest, InitUninitializedLocals)
{
    // Test skipped on Android GLES because local variable initialization is disabled.
    // http://anglebug.com/40096454
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 my_FragColor;\n"
        "int result = 0;\n"
        "void main()\n"
        "{\n"
        "    int u;\n"
        "    result += u;\n"
        "    int k = 0;\n"
        "    for (int i[2], j = i[0] + 1; k < 2; ++k)\n"
        "    {\n"
        "        result += j;\n"
        "    }\n"
        "    if (result == 2)\n"
        "    {\n"
        "        my_FragColor = vec4(0, 1, 0, 1);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        my_FragColor = vec4(1, 0, 0, 1);\n"
        "    }\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    // [WebGL 1.0]
    // DrawArrays or drawElements will generate an INVALID_OPERATION error
    // if a vertex attribute is enabled as an array via enableVertexAttribArray
    // but no buffer is bound to that attribute.
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that uninitialized structs containing arrays of structs are initialized to 0. This
// specifically tests with two different struct variables declared in the same block.
TEST_P(WebGL2GLSLTest, InitUninitializedStructContainingArrays)
{
    // Test skipped on Android GLES because local variable initialization is disabled.
    // http://anglebug.com/40096454
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    constexpr char kFS[] =
        "precision mediump float;\n"
        "struct T\n"
        "{\n"
        "    int a[2];\n"
        "};\n"
        "struct S\n"
        "{\n"
        "    T t[2];\n"
        "};\n"
        "void main()\n"
        "{\n"
        "    S s;\n"
        "    S s2;\n"
        "    if (s.t[1].a[1] == 0 && s2.t[1].a[1] == 0)\n"
        "    {\n"
        "        gl_FragColor = vec4(0, 1, 0, 1);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_FragColor = vec4(1, 0, 0, 1);\n"
        "    }\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Verify that two shaders with the same uniform name and members but different structure names will
// not link.
TEST_P(GLSLTest, StructureNameMatchingTest)
{
    const char *vsSource =
        "// Structures must have the same name, sequence of type names, and\n"
        "// type definitions, and field names to be considered the same type.\n"
        "// GLSL 1.017 4.2.4\n"
        "precision mediump float;\n"
        "struct info {\n"
        "  vec4 pos;\n"
        "  vec4 color;\n"
        "};\n"
        "\n"
        "uniform info uni;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = uni.pos;\n"
        "}\n";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    ASSERT_NE(0u, vs);
    glDeleteShader(vs);

    const char *fsSource =
        "// Structures must have the same name, sequence of type names, and\n"
        "// type definitions, and field names to be considered the same type.\n"
        "// GLSL 1.017 4.2.4\n"
        "precision mediump float;\n"
        "struct info1 {\n"
        "  vec4 pos;\n"
        "  vec4 color;\n"
        "};\n"
        "\n"
        "uniform info1 uni;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = uni.color;\n"
        "}\n";

    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);
    ASSERT_NE(0u, fs);
    glDeleteShader(fs);

    GLuint program = CompileProgram(vsSource, fsSource);
    EXPECT_EQ(0u, program);
}

// Test that an uninitialized nameless struct inside a for loop init statement works.
TEST_P(WebGL2GLSLTest, UninitializedNamelessStructInForInitStatement)
{
    // Test skipped on Android GLES because local variable initialization is disabled.
    // http://anglebug.com/40096454
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    my_FragColor = vec4(1, 0, 0, 1);\n"
        "    for (struct { float q; } b; b.q < 2.0; b.q++) {\n"
        "        my_FragColor = vec4(0, 1, 0, 1);\n"
        "    }\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that uninitialized global variables are initialized to 0.
TEST_P(WebGLGLSLTest, InitUninitializedGlobals)
{
    // http://anglebug.com/42261561
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsAdreno() && IsOpenGLES());

    constexpr char kFS[] =
        "precision mediump float;\n"
        "int result;\n"
        "int i[2], j = i[0] + 1;\n"
        "void main()\n"
        "{\n"
        "    result += j;\n"
        "    if (result == 1)\n"
        "    {\n"
        "        gl_FragColor = vec4(0, 1, 0, 1);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_FragColor = vec4(1, 0, 0, 1);\n"
        "    }\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that an uninitialized nameless struct in the global scope works.
TEST_P(WebGLGLSLTest, UninitializedNamelessStructInGlobalScope)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "struct { float q; } b;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(1, 0, 0, 1);\n"
        "    if (b.q == 0.0)\n"
        "    {\n"
        "        gl_FragColor = vec4(0, 1, 0, 1);\n"
        "    }\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that uninitialized output arguments are initialized to 0.
TEST_P(WebGL2GLSLTest, InitOutputParams)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

struct S { float a; };

out vec4 color;

float f(out float, out vec2 o1, out S o2[2], out float o3[3])
{
    float uninitialized_local;

    // leave o1 uninitialized
    // leave o2 partially uninitialized
    o2[0].a = 1.0;

    // leave o3 partially uninitialized
    o3[1] = 0.5;

    return uninitialized_local;
}

void main()
{
    float v0 = 345.;
    vec2 v1 = vec2(123., 234.);
    S v2[2] = S[2](S(-1111.), S(55.));
    float v3[3] = float[3](20., 30., 40.);
    float v4 = f(v0, v1, v2, v3);

    // Everything should be 0 now except for v2[0].a and v3[1] which should be 1.0 and 0.5
    // respectively.
    color = vec4(v0 + v1.x + v2[0].a + v3[0],  // 1.0
                 v1.y + v2[1].a + v3[1],  // 0.5
                 v3[2] + v4,              // 0
                 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_NEAR(0, 0, 255, 127, 0, 255, 1);
}

// Tests nameless struct uniforms.
TEST_P(GLSLTest, EmbeddedStructUniform)
{
    const char kFragmentShader[] = R"(precision mediump float;
uniform struct { float q; } b;
void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    if (b.q == 0.5)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);
    GLint uniLoc = glGetUniformLocation(program, "b.q");
    ASSERT_NE(-1, uniLoc);
    glUniform1f(uniLoc, 0.5f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests nameless struct uniform arrays.
TEST_P(GLSLTest, EmbeddedStructUniformArray)
{
    const char kFragmentShader[] = R"(precision mediump float;
uniform struct { float q; } b[2];
void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    if (b[0].q == 0.5)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);
    GLint uniLoc = glGetUniformLocation(program, "b[0].q");
    ASSERT_NE(-1, uniLoc);
    glUniform1f(uniLoc, 0.5f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that samplers in structs can be extracted if the first reference to the struct does not
// select an attribute.
TEST_P(GLSLTest, SamplerInStructNoMemberIndexing)
{
    constexpr char kVS[] = R"(
uniform struct {
    sampler2D n;
    vec2 c;
} s;
void main()
{
    s;
})";

    constexpr char kFS[] = R"(void main()
{
    gl_FragColor = vec4(1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Similar test to SamplerInStructNoMemberIndexing, but the struct variable is an array.
TEST_P(GLSLTest, SamplerInStructArrayNoMemberIndexing)
{
    constexpr char kVS[] = R"(
uniform struct
{
    sampler2D K;
    vec4 c;
} s[6];
void main()
{
    s[0];
})";

    constexpr char kFS[] = R"(void main()
{
    gl_FragColor = vec4(1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Tests that rewriting samplers in structs doesn't mess up indexing.
TEST_P(GLSLTest, SamplerInStructMemberIndexing)
{
    const char kVertexShader[] = R"(attribute vec2 position;
varying vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    const char kFragmentShader[] = R"(precision mediump float;
struct S { sampler2D samp; bool b; };
uniform S uni;
varying vec2 texCoord;
void main()
{
    uni;
    if (uni.b)
    {
        gl_FragColor = texture2D(uni.samp, texCoord);
    }
    else
    {
        gl_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);
    glUseProgram(program);

    GLint bLoc = glGetUniformLocation(program, "uni.b");
    ASSERT_NE(-1, bLoc);
    GLint sampLoc = glGetUniformLocation(program, "uni.samp");
    ASSERT_NE(-1, sampLoc);

    glUniform1i(bLoc, 1);

    std::array<GLColor, 4> kGreenPixels = {
        {GLColor::green, GLColor::green, GLColor::green, GLColor::green}};

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 kGreenPixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that rewriting samplers in structs works when passed as function argument.  In this test,
// the function references another struct, which is not being modified.  Regression test for AST
// validation applied to a multipass transformation, where references to declarations were attempted
// to be validated without having the entire shader.  In this case, the reference to S2 was flagged
// as invalid because S2's declaration was not visible.
TEST_P(GLSLTest, SamplerInStructAsFunctionArg)
{
    const char kFS[] = R"(precision mediump float;
struct S { sampler2D samp; bool b; };
struct S2 { float f; };

uniform S us;

float f(S s)
{
    S2 s2;
    s2.f = float(s.b);
    return s2.f;
}

void main()
{
    gl_FragColor = vec4(f(us), 0, 0, 1);
})";

    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_NE(fs, 0u);
    ASSERT_GL_NO_ERROR();
}

// Test that structs with samplers are not allowed in interface blocks.  This is forbidden per
// GLES3:
//
// > Types and declarators are the same as for other uniform variable declarations outside blocks,
// > with these exceptions:
// > * opaque types are not allowed
TEST_P(GLSLTest_ES3, StructWithSamplersDisallowedInInterfaceBlock)
{
    const char kFS[] = R"(#version 300 es
precision mediump float;
struct S { sampler2D samp; bool b; };

layout(std140) uniform Buffer { S s; } buffer;

out vec4 color;

void main()
{
    color = texture(buffer.s.samp, vec2(0));
})";

    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_EQ(fs, 0u);
    ASSERT_GL_NO_ERROR();
}

// Tests two nameless struct uniforms.
TEST_P(GLSLTest, TwoEmbeddedStructUniforms)
{
    const char kFragmentShader[] = R"(precision mediump float;
uniform struct { float q; } b, c;
void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    if (b.q == 0.5 && c.q == 1.0)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);

    GLint uniLocB = glGetUniformLocation(program, "b.q");
    ASSERT_NE(-1, uniLocB);
    glUniform1f(uniLocB, 0.5f);

    GLint uniLocC = glGetUniformLocation(program, "c.q");
    ASSERT_NE(-1, uniLocC);
    glUniform1f(uniLocC, 1.0f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a loop condition that has an initializer declares a variable.
TEST_P(GLSLTest_ES3, ConditionInitializerDeclaresVariable)
{
    constexpr char kFS[] =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "void main()\n"
        "{\n"
        "    float i = 0.0;\n"
        "    while (bool foo = (i < 1.5))\n"
        "    {\n"
        "        if (!foo)\n"
        "        {\n"
        "            ++i;\n"
        "        }\n"
        "        if (i > 3.5)\n"
        "        {\n"
        "            break;\n"
        "        }\n"
        "        ++i;\n"
        "    }\n"
        "    my_FragColor = vec4(i * 0.5 - 1.0, i * 0.5, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a variable hides a user-defined function with the same name after its initializer.
// GLSL ES 1.00.17 section 4.2.2: "A variable declaration is visible immediately following the
// initializer if present, otherwise immediately following the identifier"
TEST_P(GLSLTest, VariableHidesUserDefinedFunctionAfterInitializer)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "vec4 foo()\n"
        "{\n"
        "    return u;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    vec4 foo = foo();\n"
        "    gl_FragColor = foo + vec4(0, 1, 0, 1);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that structs with identical members are not ambiguous as function arguments.
TEST_P(GLSLTest, StructsWithSameMembersDisambiguatedByName)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "uniform float u_zero;\n"
        "struct S { float foo; };\n"
        "struct S2 { float foo; };\n"
        "float get(S s) { return s.foo + u_zero; }\n"
        "float get(S2 s2) { return 0.25 + s2.foo + u_zero; }\n"
        "void main()\n"
        "{\n"
        "    S s;\n"
        "    s.foo = 0.5;\n"
        "    S2 s2;\n"
        "    s2.foo = 0.25;\n"
        "    gl_FragColor = vec4(0.0, get(s) + get(s2), 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that an inactive varying in vertex shader but used in fragment shader can be linked
// successfully.
TEST_P(GLSLTest, InactiveVaryingInVertexActiveInFragment)
{
    // http://anglebug.com/42263408
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    constexpr char kVS[] =
        "attribute vec4 inputAttribute;\n"
        "varying vec4 varColor;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = inputAttribute;\n"
        "}\n";

    constexpr char kFS[] =
        "precision mediump float;\n"
        "varying vec4 varColor;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = varColor;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    ASSERT_GL_NO_ERROR();
}

// Test that a varying struct that's not statically used in the fragment shader works.
// GLSL ES 3.00.6 section 4.3.10.
TEST_P(GLSLTest_ES3, VaryingStructNotStaticallyUsedInFragmentShader)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "out S varStruct;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(1.0);\n"
        "    varStruct.field = vec4(0.0, 0.5, 0.0, 0.0);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "in S varStruct;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that inactive shader IO block varying are ok.
TEST_P(GLSLTest_ES31, InactiveVaryingIOBlock)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] =
        R"(#version 310 es
        #extension GL_EXT_shader_io_blocks : require

        precision highp float;
        in vec4 inputAttribute;
        out Block { vec4 v; };
        out Inactive1 { vec4 value; };
        out Inactive2 { vec4 value; } named;

        void main()
        {
            gl_Position    = inputAttribute;
            v = vec4(0);
        })";

    constexpr char kFS[] =
        R"(#version 310 es
        #extension GL_EXT_shader_io_blocks : require

        precision highp float;

        in Block { vec4 v; };
        in Inactive3 { vec4 value; };
        in Inactive4 { vec4 value; } named2;

        layout(location = 0) out mediump vec4 color;
        void main()
        {
            color = vec4(1, v.xy, 1);
        })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that a shader IO block varying that's not declared in the fragment shader links
// successfully.
TEST_P(GLSLTest_ES31, VaryingIOBlockNotDeclaredInFragmentShader)
{
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
        void main()
        {
            color = vec4(1, 0, 0, 1);
        })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that a shader IO block varying that's not declared in the vertex shader links
// successfully.
TEST_P(GLSLTest_ES31, VaryingIOBlockNotDeclaredInVertexShader)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] =
        R"(#version 310 es
        #extension GL_EXT_shader_io_blocks : require

        precision highp float;
        in vec4 inputAttribute;

        void main()
        {
            gl_Position = inputAttribute;
        })";

    constexpr char kFS[] =
        R"(#version 310 es
        #extension GL_EXT_shader_io_blocks : require

        precision highp float;
        in Block_inout { vec4 value; } user_in;
        layout(location = 0) out mediump vec4 color;

        void main()
        {
            color = vec4(1, 0, 0, 1);
        })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that a shader with sample in / sample out can be linked successfully.
TEST_P(GLSLTest_ES31, VaryingTessellationSampleInAndOut)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));

    constexpr char kVS[] =
        R"(#version 310 es
        #extension GL_OES_shader_multisample_interpolation : require

        precision highp float;
        in vec4 inputAttribute;

        sample out mediump float tc_in;
        void main()
        {
            tc_in = inputAttribute[0];
            gl_Position = inputAttribute;
        })";

    constexpr char kTCS[] =
        R"(#version 310 es
        #extension GL_EXT_tessellation_shader : require
        #extension GL_OES_shader_multisample_interpolation : require
        layout (vertices=3) out;

        sample in mediump float tc_in[];
        sample out mediump float tc_out[];
        void main()
        {
            tc_out[gl_InvocationID] = tc_in[gl_InvocationID];
            gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
            gl_TessLevelInner[0] = 2.0;
            gl_TessLevelInner[1] = 2.0;
            gl_TessLevelOuter[0] = 2.0;
            gl_TessLevelOuter[1] = 2.0;
            gl_TessLevelOuter[2] = 2.0;
            gl_TessLevelOuter[3] = 2.0;
        })";

    constexpr char kTES[] =
        R"(#version 310 es
        #extension GL_EXT_tessellation_shader : require
        #extension GL_OES_shader_multisample_interpolation : require
        layout (triangles) in;

        sample in mediump float tc_out[];
        sample out mediump float te_out;
        void main()
        {
            te_out = tc_out[2];
            gl_Position = gl_TessCoord[0] * gl_in[0].gl_Position;
        })";

    constexpr char kFS[] =
        R"(#version 310 es
        #extension GL_OES_shader_multisample_interpolation : require

        precision highp float;
        sample in mediump float te_out;
        layout(location = 0) out mediump vec4 color;

        void main()
        {
            float out0 = te_out;
            color = vec4(1, 0, 0, 1);
        })";

    ANGLE_GL_PROGRAM_WITH_TESS(program, kVS, kTCS, kTES, kFS);
    drawPatches(program, "inputAttribute", 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();
}

// Test that `smooth sample` and `flat sample` pass the validation.
TEST_P(GLSLTest_ES3, AliasedSampleQualifiers)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));

    constexpr char kVS[] =
        R"(#version 300 es
        #extension GL_OES_shader_multisample_interpolation : require

        smooth sample out mediump float f;
        flat sample out mediump int i;
        void main()
        {
            f = 1.0;
            i = 1;
            gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
        })";

    constexpr char kFS[] =
        R"(#version 300 es
        #extension GL_OES_shader_multisample_interpolation : require

        smooth sample in mediump float f;
        flat sample in mediump int i;
        out mediump vec4 color;
        void main()
        {
            color = vec4(f, float(i), 0, 1);
        })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that `noperspective centroid` passes the validation and compiles.
TEST_P(GLSLTest_ES3, NoPerspectiveCentroid)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_NV_shader_noperspective_interpolation"));

    constexpr char kVS[] =
        R"(#version 300 es
        #extension GL_NV_shader_noperspective_interpolation : require

        noperspective centroid out mediump float f;
        void main()
        {
            f = 1.0;
            gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
        })";

    constexpr char kFS[] =
        R"(#version 300 es
        #extension GL_NV_shader_noperspective_interpolation : require

        noperspective centroid in mediump float f;
        out mediump vec4 color;
        void main()
        {
            color = vec4(f, 0.0, 0.0, 1.0);
        })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that `noperspective sample` passes the validation and compiles.
TEST_P(GLSLTest_ES3, NoPerspectiveSample)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_NV_shader_noperspective_interpolation"));

    constexpr char kVS[] =
        R"(#version 300 es
        #extension GL_OES_shader_multisample_interpolation : require
        #extension GL_NV_shader_noperspective_interpolation : require

        noperspective sample out mediump float f;
        void main()
        {
            f = 1.0;
            gl_Position = vec4(f, 0.0, 0.0, 1.0);
        })";

    constexpr char kFS[] =
        R"(#version 300 es
        #extension GL_OES_shader_multisample_interpolation : require
        #extension GL_NV_shader_noperspective_interpolation : require

        noperspective sample in mediump float f;
        out mediump vec4 color;
        void main()
        {
            color = vec4(f, 0.0, 0.0, 1.0);
        })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that a shader with sample in / sample out can be used successfully when the varying
// precision is different between VS and FS.
TEST_P(GLSLTest_ES31, VaryingSampleInAndOutDifferentPrecision)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));

    constexpr char kVS[] =
        R"(#version 310 es
        #extension GL_OES_shader_multisample_interpolation : require

        precision highp float;
        in vec4 inputAttribute;

        sample out highp float v;
        void main()
        {
            v = inputAttribute[0];
            gl_Position = inputAttribute;
        })";

    constexpr char kFS[] =
        R"(#version 310 es
        #extension GL_OES_shader_multisample_interpolation : require

        precision highp float;
        sample in mediump float v;
        layout(location = 0) out mediump vec4 color;

        void main()
        {
            color = vec4(round((v + 1.) / 2. * 5.) / 5., 0, 0, 1);
        })";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, 0, GLColor::red);
}

// Test that a shader IO block varying whose block name is declared multiple(in/out) time links
// successfully.
TEST_P(GLSLTest_ES31, VaryingIOBlockDeclaredAsInAndOut)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
    #extension GL_EXT_shader_io_blocks : require
    precision highp float;
    in vec4 inputAttribute;
    out Vertex
    {
        vec4 fv;
    } outVertex;
    void main()
    {
        gl_Position = inputAttribute;
        outVertex.fv = gl_Position;
    })";

    constexpr char kTCS[] = R"(#version 310 es
    #extension GL_EXT_tessellation_shader : require
    #extension GL_EXT_shader_io_blocks : require
    precision mediump float;
    in Vertex
    {
        vec4 fv;
    } inVertex[];
    layout(vertices = 2) out;
    out Vertex
    {
        vec4 fv;
    } outVertex[];

    void main()
    {
        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
        outVertex[gl_InvocationID].fv = inVertex[gl_InvocationID].fv;
        gl_TessLevelInner[0] = 1.0;
            gl_TessLevelInner[1] = 1.0;
            gl_TessLevelOuter[0] = 1.0;
            gl_TessLevelOuter[1] = 1.0;
            gl_TessLevelOuter[2] = 1.0;
            gl_TessLevelOuter[3] = 1.0;
    })";

    constexpr char kTES[] = R"(#version 310 es
    #extension GL_EXT_tessellation_shader : require
    #extension GL_EXT_shader_io_blocks : require
    precision mediump float;
    layout (isolines, point_mode) in;
    in Vertex
    {
        vec4 fv;
    } inVertex[];
    out vec4 result_fv;

    void main()
    {
        gl_Position = gl_in[0].gl_Position;
        result_fv = inVertex[0].fv;
    })";

    constexpr char kFS[] = R"(#version 310 es
    precision mediump float;

    layout(location = 0) out mediump vec4 color;

    void main()
    {
        // Output solid green
        color = vec4(0, 1.0, 0, 1.0);
    })";

    ANGLE_GL_PROGRAM_WITH_TESS(program, kVS, kTCS, kTES, kFS);
    drawPatches(program, "inputAttribute", 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();
}

void GLSLTest_ES31::testTessellationTextureBufferAccess(const APIExtensionVersion usedExtension)
{
    ASSERT(usedExtension == APIExtensionVersion::EXT || usedExtension == APIExtensionVersion::OES);

    // Vertex shader
    constexpr char kVS[] = R"(#version 310 es
precision highp float;
in vec4 inputAttribute;

void main()
{
gl_Position = inputAttribute;
})";

    // Tessellation shaders
    constexpr char kGLSLVersion[] = R"(#version 310 es
)";
    constexpr char kTessEXT[]     = R"(#extension GL_EXT_tessellation_shader : require
)";
    constexpr char kTessOES[]     = R"(#extension GL_OES_tessellation_shader : require
)";
    constexpr char kTexBufEXT[]   = R"(#extension GL_EXT_texture_buffer : require
)";
    constexpr char kTexBufOES[]   = R"(#extension GL_OES_texture_buffer : require
)";

    std::string tcs;
    std::string tes;

    tcs.append(kGLSLVersion);
    tes.append(kGLSLVersion);

    if (usedExtension == APIExtensionVersion::EXT)
    {
        tcs.append(kTessEXT);
        tes.append(kTessEXT);
        tes.append(kTexBufEXT);
    }
    else
    {
        tcs.append(kTessOES);
        tes.append(kTessOES);
        tes.append(kTexBufOES);
    }

    constexpr char kTCSBody[] = R"(precision mediump float;
layout(vertices = 2) out;

void main()
{
gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
gl_TessLevelInner[0] = 1.0;
gl_TessLevelInner[1] = 1.0;
gl_TessLevelOuter[0] = 1.0;
gl_TessLevelOuter[1] = 1.0;
gl_TessLevelOuter[2] = 1.0;
gl_TessLevelOuter[3] = 1.0;
})";
    tcs.append(kTCSBody);

    constexpr char kTESBody[] = R"(precision mediump float;
layout (isolines, point_mode) in;

uniform highp samplerBuffer tex;

out vec4 tex_color;

void main()
{
tex_color = texelFetch(tex, 0);
gl_Position = gl_in[0].gl_Position;
})";
    tes.append(kTESBody);

    // Fragment shader
    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
layout(location = 0) out mediump vec4 color;

in vec4 tex_color;

void main()
{
color = tex_color;
})";

    constexpr GLint kBufferSize = 4;
    GLubyte texData[]           = {0u, 255u, 0u, 255u};

    GLTexture texture;
    glBindTexture(GL_TEXTURE_BUFFER, texture);

    GLBuffer buffer;
    glBindBuffer(GL_TEXTURE_BUFFER, buffer);
    glBufferData(GL_TEXTURE_BUFFER, kBufferSize, texData, GL_STATIC_DRAW);
    glTexBufferEXT(GL_TEXTURE_BUFFER, GL_RGBA8, buffer);
    ASSERT_GL_NO_ERROR();

    glClearColor(1.0, 0, 0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    ANGLE_GL_PROGRAM_WITH_TESS(program, kVS, tcs.c_str(), tes.c_str(), kFS);
    drawPatches(program, "inputAttribute", 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();
}

// Test that texture buffers can be accessed in a tessellation stage (using EXT)
TEST_P(GLSLTest_ES31, TessellationTextureBufferAccessEXT)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_texture_buffer"));
    testTessellationTextureBufferAccess(APIExtensionVersion::EXT);
}

// Test that texture buffers can be accessed in a tessellation stage (using OES)
TEST_P(GLSLTest_ES31, TessellationTextureBufferAccessOES)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_buffer"));
    testTessellationTextureBufferAccess(APIExtensionVersion::OES);
}

// Test that a varying struct that's not declared in the fragment shader links successfully.
// GLSL ES 3.00.6 section 4.3.10.
TEST_P(GLSLTest_ES3, VaryingStructNotDeclaredInFragmentShader)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "out S varStruct;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(1.0);\n"
        "    varStruct.field = vec4(0.0, 0.5, 0.0, 0.0);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that a varying struct that's not declared in the vertex shader, and is unused in the
// fragment shader links successfully.
TEST_P(GLSLTest_ES3, VaryingStructNotDeclaredInVertexShader)
{
    // GLSL ES allows the vertex shader to not declare a varying if the fragment shader is not
    // going to use it.  See section 9.1 in
    // https://www.khronos.org/registry/OpenGL/specs/es/3.2/GLSL_ES_Specification_3.20.pdf or
    // section 4.3.5 in https://www.khronos.org/files/opengles_shading_language.pdf
    //
    // However, nvidia OpenGL ES drivers fail to link this program.
    //
    // http://anglebug.com/42262078
    ANGLE_SKIP_TEST_IF(IsOpenGLES() && IsNVIDIA());

    constexpr char kVS[] =
        "#version 300 es\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(1.0);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "in S varStruct;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that a varying struct that's not initialized in the vertex shader links successfully.
TEST_P(WebGL2GLSLTest, VaryingStructNotInitializedInVertexShader)
{
    // GLSL ES allows the vertex shader to declare but not initialize a varying (with a
    // specification that the varying values are undefined in the fragment stage).  See section 9.1
    // in https://www.khronos.org/registry/OpenGL/specs/es/3.2/GLSL_ES_Specification_3.20.pdf
    // or section 4.3.5 in https://www.khronos.org/files/opengles_shading_language.pdf
    //
    // However, windows and mac OpenGL drivers fail to link this program.  With a message like:
    //
    // > Input of fragment shader 'varStruct' not written by vertex shader
    //
    // http://anglebug.com/42262078
    ANGLE_SKIP_TEST_IF(IsDesktopOpenGL() && (IsMac() || (IsWindows() && !IsNVIDIA())));

    constexpr char kVS[] =
        "#version 300 es\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "out S varStruct;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(1.0);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "in S varStruct;\n"
        "void main()\n"
        "{\n"
        "    col = varStruct.field;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that a varying struct that gets used in the fragment shader works.
TEST_P(GLSLTest_ES3, VaryingStructUsedInFragmentShader)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 inputAttribute;\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "out S varStruct;\n"
        "out S varStruct2;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = inputAttribute;\n"
        "    varStruct.field = vec4(0.0, 0.5, 0.0, 1.0);\n"
        "    varStruct2.field = vec4(0.0, 0.5, 0.0, 1.0);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "in S varStruct;\n"
        "in S varStruct2;\n"
        "void main()\n"
        "{\n"
        "    col = varStruct.field + varStruct2.field;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// This is a regression test to make sure a red quad is rendered without issues
// when a passthrough function with a vec3 input parameter is used in the fragment shader.
TEST_P(GLSLTest_ES31, SamplerPassthroughFailedLink)
{
    constexpr char kVS[] =
        "precision mediump float;\n"
        "attribute vec4 inputAttribute;\n"
        "varying mediump vec2 texCoord;\n"
        "void main() {\n"
        "    texCoord = inputAttribute.xy;\n"
        "    gl_Position = vec4(inputAttribute.x, inputAttribute.y, 0.0, 1.0);\n"
        "}\n";

    constexpr char kFS[] =
        "precision mediump float;\n"
        "varying mediump vec2 texCoord;\n"
        "uniform sampler2D testSampler;\n"
        "vec3 passthrough(vec3 c) {\n"
        "    return c;\n"
        "}\n"
        "void main() {\n"
        "    gl_FragColor = vec4(passthrough(texture2D(testSampler, texCoord).rgb), 1.0);\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    // Initialize basic red texture.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::red.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "inputAttribute", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// This is a regression test to make sure a red quad is rendered without issues
// when a passthrough function with a vec4 input parameter is used in the fragment shader.
TEST_P(GLSLTest_ES31, SamplerPassthroughIncorrectColor)
{
    constexpr char kVS[] =
        "precision mediump float;\n"
        "attribute vec4 inputAttribute;\n"
        "varying mediump vec2 texCoord;\n"
        "void main() {\n"
        "    texCoord = inputAttribute.xy;\n"
        "    gl_Position = vec4(inputAttribute.x, inputAttribute.y, 0.0, 1.0);\n"
        "}\n";

    constexpr char kFS[] =
        "precision mediump float;\n"
        "varying mediump vec2 texCoord;\n"
        "uniform sampler2D testSampler;\n"
        "vec4 passthrough(vec4 c) {\n"
        "    return c;\n"
        "}\n"
        "void main() {\n"
        "    gl_FragColor = vec4(passthrough(texture2D(testSampler, texCoord)));\n"
        "}\n";
    ANGLE_GL_PROGRAM(program, kVS, kFS);

    // Initialize basic red texture.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 GLColor::red.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "inputAttribute", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that multiple multi-field varying structs that get used in the fragment shader work.
TEST_P(GLSLTest_ES3, ComplexVaryingStructsUsedInFragmentShader)
{
    // TODO(syoussefi): fails on android with:
    //
    // > Internal Vulkan error: A return array was too small for the result
    //
    // http://anglebug.com/42261898
    ANGLE_SKIP_TEST_IF(IsVulkan() && IsAndroid());

    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 inputAttribute;\n"
        "struct S {\n"
        "    vec4 field1;\n"
        "    vec4 field2;\n"
        "};\n"
        "out S varStruct;\n"
        "out S varStruct2;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = inputAttribute;\n"
        "    varStruct.field1 = vec4(0.0, 0.5, 0.0, 1.0);\n"
        "    varStruct.field2 = vec4(0.0, 0.5, 0.0, 1.0);\n"
        "    varStruct2.field1 = vec4(0.0, 0.5, 0.0, 1.0);\n"
        "    varStruct2.field2 = vec4(0.0, 0.5, 0.0, 1.0);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "struct S {\n"
        "    vec4 field1;\n"
        "    vec4 field2;\n"
        "};\n"
        "in S varStruct;\n"
        "in S varStruct2;\n"
        "void main()\n"
        "{\n"
        "    col = varStruct.field1 + varStruct2.field2;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that an inactive varying array that doesn't get used in the fragment shader works.
TEST_P(GLSLTest_ES3, InactiveVaryingArrayUnusedInFragmentShader)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 inputAttribute;\n"
        "out vec4 varArray[4];\n"
        "void main()\n"
        "{\n"
        "    gl_Position = inputAttribute;\n"
        "    varArray[0] = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "    varArray[1] = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "    varArray[2] = vec4(0.0, 0.0, 1.0, 1.0);\n"
        "    varArray[3] = vec4(1.0, 1.0, 0.0, 1.0);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
}

// Test that an inactive varying struct that doesn't get used in the fragment shader works.
TEST_P(GLSLTest_ES3, InactiveVaryingStructUnusedInFragmentShader)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 inputAttribute;\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "out S varStruct;\n"
        "out S varStruct2;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = inputAttribute;\n"
        "    varStruct.field = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "    varStruct2.field = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "struct S {\n"
        "    vec4 field;\n"
        "};\n"
        "in S varStruct;\n"
        "in S varStruct2;\n"
        "void main()\n"
        "{\n"
        "    col = varStruct.field;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that multiple varying matrices that get used in the fragment shader work.
TEST_P(GLSLTest_ES3, VaryingMatrices)
{
    constexpr char kVS[] =
        "#version 300 es\n"
        "in vec4 inputAttribute;\n"
        "out mat2x2 varMat;\n"
        "out mat2x2 varMat2;\n"
        "out mat4x3 varMat3;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = inputAttribute;\n"
        "    varMat[0] = vec2(1, 1);\n"
        "    varMat[1] = vec2(1, 1);\n"
        "    varMat2[0] = vec2(0.5, 0.5);\n"
        "    varMat2[1] = vec2(0.5, 0.5);\n"
        "    varMat3[0] = vec3(0.75, 0.75, 0.75);\n"
        "    varMat3[1] = vec3(0.75, 0.75, 0.75);\n"
        "    varMat3[2] = vec3(0.75, 0.75, 0.75);\n"
        "    varMat3[3] = vec3(0.75, 0.75, 0.75);\n"
        "}\n";

    constexpr char kFS[] =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 col;\n"
        "in mat2x2 varMat;\n"
        "in mat2x2 varMat2;\n"
        "in mat4x3 varMat3;\n"
        "void main()\n"
        "{\n"
        "    col = vec4(varMat[0].x, varMat2[1].y, varMat3[2].z, 1);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 127, 191, 255), 1);
}

// This test covers passing a struct containing a sampler as a function argument.
TEST_P(GLSLTest, StructsWithSamplersAsFunctionArg)
{
    // Shader failed to compile on Nexus devices. http://anglebug.com/42260860
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsAdreno() && IsOpenGLES());

    const char kFragmentShader[] = R"(precision mediump float;
struct S { sampler2D samplerMember; };
uniform S uStruct;
uniform vec2 uTexCoord;
vec4 foo(S structVar)
{
    return texture2D(structVar.samplerMember, uTexCoord);
}
void main()
{
    gl_FragColor = foo(uStruct);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);

    // Initialize the texture with green.
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {0u, 255u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw
    glUseProgram(program);
    GLint samplerMemberLoc = glGetUniformLocation(program, "uStruct.samplerMember");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    GLint texCoordLoc = glGetUniformLocation(program, "uTexCoord");
    ASSERT_NE(-1, texCoordLoc);
    glUniform2f(texCoordLoc, 0.5f, 0.5f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
}

// This test covers passing a struct containing a sampler as a function argument.
TEST_P(GLSLTest, StructsWithSamplersAsFunctionArgWithPrototype)
{
    // Shader failed to compile on Android. http://anglebug.com/42260860
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsAdreno() && IsOpenGLES());

    const char kFragmentShader[] = R"(precision mediump float;
struct S { sampler2D samplerMember; };
uniform S uStruct;
uniform vec2 uTexCoord;
vec4 foo(S structVar);
vec4 foo(S structVar)
{
    return texture2D(structVar.samplerMember, uTexCoord);
}
void main()
{
    gl_FragColor = foo(uStruct);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);

    // Initialize the texture with green.
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {0u, 255u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw
    glUseProgram(program);
    GLint samplerMemberLoc = glGetUniformLocation(program, "uStruct.samplerMember");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    GLint texCoordLoc = glGetUniformLocation(program, "uTexCoord");
    ASSERT_NE(-1, texCoordLoc);
    glUniform2f(texCoordLoc, 0.5f, 0.5f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
}

// This test covers passing a struct containing a sampler as a function argument, where the function
// has non-return branch statements.
TEST_P(GLSLTest_ES3, StructsWithSamplersAsFunctionArgWithBranch)
{
    // Shader failed to compile on Nexus devices. http://anglebug.com/42260860
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsAdreno() && IsOpenGLES());

    const char kFragmentShader[] = R"(precision mediump float;
struct S { sampler2D samplerMember; };
uniform S uStruct;
uniform vec2 uTexCoord;
vec4 foo(S structVar)
{
    vec4 result;
    while (true)
    {
        result = texture2D(structVar.samplerMember, uTexCoord);
        if (result.x == 12345.)
        {
            continue;
        }
        break;
    }
    return result;
}
void main()
{
    gl_FragColor = foo(uStruct);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);

    // Initialize the texture with green.
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {0u, 255u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw
    glUseProgram(program);
    GLint samplerMemberLoc = glGetUniformLocation(program, "uStruct.samplerMember");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    GLint texCoordLoc = glGetUniformLocation(program, "uTexCoord");
    ASSERT_NE(-1, texCoordLoc);
    glUniform2f(texCoordLoc, 0.5f, 0.5f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
}

// This test covers passing an array of structs containing samplers as a function argument.
TEST_P(GLSLTest, ArrayOfStructsWithSamplersAsFunctionArg)
{
    // Shader failed to compile on Nexus devices. http://anglebug.com/42260860
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsAdreno() && IsOpenGLES());

    constexpr char kFS[] =
        "precision mediump float;\n"
        "struct S\n"
        "{\n"
        "    sampler2D samplerMember; \n"
        "};\n"
        "uniform S uStructs[2];\n"
        "uniform vec2 uTexCoord;\n"
        "\n"
        "vec4 foo(S[2] structs)\n"
        "{\n"
        "    return texture2D(structs[0].samplerMember, uTexCoord);\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = foo(uStructs);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);

    // Initialize the texture with green.
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {0u, 255u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw
    glUseProgram(program);
    GLint samplerMemberLoc = glGetUniformLocation(program, "uStructs[0].samplerMember");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    GLint texCoordLoc = glGetUniformLocation(program, "uTexCoord");
    ASSERT_NE(-1, texCoordLoc);
    glUniform2f(texCoordLoc, 0.5f, 0.5f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
}

// This test covers passing a struct containing an array of samplers as a function argument.
TEST_P(GLSLTest, StructWithSamplerArrayAsFunctionArg)
{
    // Shader failed to compile on Nexus devices. http://anglebug.com/42260860
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsAdreno() && IsOpenGLES());

    constexpr char kFS[] =
        "precision mediump float;\n"
        "struct S\n"
        "{\n"
        "    sampler2D samplerMembers[2];\n"
        "};\n"
        "uniform S uStruct;\n"
        "uniform vec2 uTexCoord;\n"
        "\n"
        "vec4 foo(S str)\n"
        "{\n"
        "    return texture2D(str.samplerMembers[0], uTexCoord);\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = foo(uStruct);\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);

    // Initialize the texture with green.
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {0u, 255u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw
    glUseProgram(program);
    GLint samplerMemberLoc = glGetUniformLocation(program, "uStruct.samplerMembers[0]");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    GLint texCoordLoc = glGetUniformLocation(program, "uTexCoord");
    ASSERT_NE(-1, texCoordLoc);
    glUniform2f(texCoordLoc, 0.5f, 0.5f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
}

// This test covers passing nested structs containing a sampler as a function argument.
TEST_P(GLSLTest, NestedStructsWithSamplersAsFunctionArg)
{
    // Shader failed to compile on Nexus devices. http://anglebug.com/42260860
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsAdreno() && IsOpenGLES());

    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    const char kFragmentShader[] = R"(precision mediump float;
struct S { sampler2D samplerMember; };
struct T { S nest; };
uniform T uStruct;
uniform vec2 uTexCoord;
vec4 foo2(S structVar)
{
    return texture2D(structVar.samplerMember, uTexCoord);
}
vec4 foo(T structVar)
{
    return foo2(structVar.nest);
}
void main()
{
    gl_FragColor = foo(uStruct);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);

    // Initialize the texture with green.
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {0u, 255u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw
    glUseProgram(program);
    GLint samplerMemberLoc = glGetUniformLocation(program, "uStruct.nest.samplerMember");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    GLint texCoordLoc = glGetUniformLocation(program, "uTexCoord");
    ASSERT_NE(-1, texCoordLoc);
    glUniform2f(texCoordLoc, 0.5f, 0.5f);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
}

// This test covers passing a compound structs containing a sampler as a function argument.
TEST_P(GLSLTest, CompoundStructsWithSamplersAsFunctionArg)
{
    // Shader failed to compile on Nexus devices. http://anglebug.com/42260860
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsAdreno() && IsOpenGLES());

    const char kFragmentShader[] = R"(precision mediump float;
struct S { sampler2D samplerMember; bool b; };
uniform S uStruct;
uniform vec2 uTexCoord;
vec4 foo(S structVar)
{
    if (structVar.b)
        return texture2D(structVar.samplerMember, uTexCoord);
    else
        return vec4(1, 0, 0, 1);
}
void main()
{
    gl_FragColor = foo(uStruct);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);

    // Initialize the texture with green.
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {0u, 255u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw
    glUseProgram(program);
    GLint samplerMemberLoc = glGetUniformLocation(program, "uStruct.samplerMember");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    GLint texCoordLoc = glGetUniformLocation(program, "uTexCoord");
    ASSERT_NE(-1, texCoordLoc);
    glUniform2f(texCoordLoc, 0.5f, 0.5f);
    GLint bLoc = glGetUniformLocation(program, "uStruct.b");
    ASSERT_NE(-1, bLoc);
    glUniform1i(bLoc, 1);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
}

// This test covers passing nested compound structs containing a sampler as a function argument.
TEST_P(GLSLTest, NestedCompoundStructsWithSamplersAsFunctionArg)
{
    // Shader failed to compile on Nexus devices. http://anglebug.com/42260860
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsAdreno() && IsOpenGLES());

    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    const char kFragmentShader[] = R"(precision mediump float;
struct S { sampler2D samplerMember; bool b; };
struct T { S nest; bool b; };
uniform T uStruct;
uniform vec2 uTexCoord;
vec4 foo2(S structVar)
{
    if (structVar.b)
        return texture2D(structVar.samplerMember, uTexCoord);
    else
        return vec4(1, 0, 0, 1);
}
vec4 foo(T structVar)
{
    if (structVar.b)
        return foo2(structVar.nest);
    else
        return vec4(1, 0, 0, 1);
}
void main()
{
    gl_FragColor = foo(uStruct);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);

    // Initialize the texture with green.
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {0u, 255u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw
    glUseProgram(program);
    GLint samplerMemberLoc = glGetUniformLocation(program, "uStruct.nest.samplerMember");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    GLint texCoordLoc = glGetUniformLocation(program, "uTexCoord");
    ASSERT_NE(-1, texCoordLoc);
    glUniform2f(texCoordLoc, 0.5f, 0.5f);

    GLint bLoc = glGetUniformLocation(program, "uStruct.b");
    ASSERT_NE(-1, bLoc);
    glUniform1i(bLoc, 1);

    GLint nestbLoc = glGetUniformLocation(program, "uStruct.nest.b");
    ASSERT_NE(-1, nestbLoc);
    glUniform1i(nestbLoc, 1);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
}

// Same as the prior test but with reordered struct members.
TEST_P(GLSLTest, MoreNestedCompoundStructsWithSamplersAsFunctionArg)
{
    // Shader failed to compile on Nexus devices. http://anglebug.com/42260860
    ANGLE_SKIP_TEST_IF(IsNexus5X() && IsAdreno() && IsOpenGLES());

    // TODO(anglebug.com/40096747): Failing on ARM-based Apple DTKs.
    ANGLE_SKIP_TEST_IF(IsMac() && IsARM64() && IsDesktopOpenGL());

    const char kFragmentShader[] = R"(precision mediump float;
struct S { bool b; sampler2D samplerMember; };
struct T { bool b; S nest; };
uniform T uStruct;
uniform vec2 uTexCoord;
vec4 foo2(S structVar)
{
    if (structVar.b)
        return texture2D(structVar.samplerMember, uTexCoord);
    else
        return vec4(1, 0, 0, 1);
}
vec4 foo(T structVar)
{
    if (structVar.b)
        return foo2(structVar.nest);
    else
        return vec4(1, 0, 0, 1);
}
void main()
{
    gl_FragColor = foo(uStruct);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);

    // Initialize the texture with green.
    GLTexture tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLubyte texData[] = {0u, 255u, 0u, 255u};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Draw
    glUseProgram(program);
    GLint samplerMemberLoc = glGetUniformLocation(program, "uStruct.nest.samplerMember");
    ASSERT_NE(-1, samplerMemberLoc);
    glUniform1i(samplerMemberLoc, 0);
    GLint texCoordLoc = glGetUniformLocation(program, "uTexCoord");
    ASSERT_NE(-1, texCoordLoc);
    glUniform2f(texCoordLoc, 0.5f, 0.5f);

    GLint bLoc = glGetUniformLocation(program, "uStruct.b");
    ASSERT_NE(-1, bLoc);
    glUniform1i(bLoc, 1);

    GLint nestbLoc = glGetUniformLocation(program, "uStruct.nest.b");
    ASSERT_NE(-1, nestbLoc);
    glUniform1i(nestbLoc, 1);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
}
// Test that a global variable declared after main() works. This is a regression test for an issue
// in global variable initialization.
TEST_P(WebGLGLSLTest, GlobalVariableDeclaredAfterMain)
{
    constexpr char kFS[] =
        "precision mediump float;\n"
        "int getFoo();\n"
        "uniform int u_zero;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(1, 0, 0, 1);\n"
        "    if (getFoo() == 0)\n"
        "    {\n"
        "        gl_FragColor = vec4(0, 1, 0, 1);\n"
        "    }\n"
        "}\n"
        "int foo;\n"
        "int getFoo()\n"
        "{\n"
        "    foo = u_zero;\n"
        "    return foo;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test calling array length() with a "this" expression having side effects inside a loop condition.
// The spec says that sequence operator operands need to run in sequence.
TEST_P(GLSLTest_ES3, ArrayLengthOnExpressionWithSideEffectsInLoopCondition)
{
    // "a" gets doubled three times in the below program.
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
uniform int u_zero;
int a;
int[2] doubleA()
{
    a *= 2;
    return int[2](a, a);
}
void main()
{
    a = u_zero + 1;
    for (int i = 0; i < doubleA().length(); ++i)
    {}
    if (a == 8)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        my_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test calling array length() with a "this" expression having side effects that interact with side
// effects of another operand of the same sequence operator. The spec says that sequence operator
// operands need to run in order from left to right (ESSL 3.00.6 section 5.9).
TEST_P(GLSLTest_ES3, ArrayLengthOnExpressionWithSideEffectsInSequence)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
uniform int u_zero;
int a;
int[3] doubleA()
{
    a *= 2;
    return int[3](a, a, a);
}
void main()
{
    a = u_zero;
    int b = (a++, doubleA().length());
    if (b == 3 && a == 2)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        my_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test calling array length() with a "this" expression that also contains a call of array length().
// Both "this" expressions also have side effects.
TEST_P(GLSLTest_ES3, NestedArrayLengthMethodsWithSideEffects)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
uniform int u_zero;
int a;
int[3] multiplyA(int multiplier)
{
    a *= multiplier;
    return int[3](a, a, a);
}
void main()
{
    a = u_zero + 1;
    int b = multiplyA(multiplyA(2).length()).length();
    if (b == 3 && a == 6)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        my_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test calling array length() with a "this" expression having side effects inside an if condition.
// This is an issue if the the side effect can be short circuited.
TEST_P(GLSLTest_ES3, ArrayLengthOnShortCircuitedExpressionWithSideEffectsInIfCondition)
{
    // Bug in the shader translator.  http://anglebug.com/42262472
    ANGLE_SKIP_TEST_IF(true);

    // "a" shouldn't get modified by this shader.
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
uniform int u_zero;
int a;
int[2] doubleA()
{
    a *= 2;
    return int[2](a, a);
}
void main()
{
    a = u_zero + 1;
    if (u_zero != 0 && doubleA().length() == 2)
    {
        ++a;
    }
    if (a == 1)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        my_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test calling array length() with a "this" expression having side effects in a statement where the
// side effect can be short circuited.
TEST_P(GLSLTest_ES3, ArrayLengthOnShortCircuitedExpressionWithSideEffectsInStatement)
{
    // Bug in the shader translator.  http://anglebug.com/42262472
    ANGLE_SKIP_TEST_IF(true);

    // "a" shouldn't get modified by this shader.
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
uniform int u_zero;
int a;
int[2] doubleA()
{
    a *= 2;
    return int[2](a, a);
}
void main()
{
    a = u_zero + 1;
    bool test = u_zero != 0 && doubleA().length() == 2;
    if (a == 1)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        my_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that array length inside vector constructor works.
TEST_P(GLSLTest_ES3, ArrayLengthInVectorConstructor)
{
    const char kVS[] = R"(#version 300 es
precision highp float;
flat out uvec4 v;

int[1] f0()
{
    return int[1](1);
}
void main()
{
    v = uvec4(vec4(f0().length()));

    gl_Position.x = ((gl_VertexID & 1) == 0 ? -1.0 : 1.0);
    gl_Position.y = ((gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    gl_Position.zw = vec2(0, 1);
})";

    const char kFS[] = R"(#version 300 es
precision highp float;
flat in uvec4 v;
out vec4 color;

bool isEq(uint a, float b) { return abs(float(a) - b) < 0.01; }

void main()
{
    if (isEq(v[0], 1.) &&
        isEq(v[1], 1.) &&
        isEq(v[2], 1.) &&
        isEq(v[3], 1.))
    {
        color = vec4(0, 1, 0, 1);
    }
    else
    {
        color = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that array length inside vector constructor works in complex expression.
TEST_P(GLSLTest_ES3, ArrayLengthInVectorConstructorComplex)
{
    const char kVS[] = R"(#version 300 es
precision highp float;
out vec4 v;

int[1] f0()
{
    return int[1](1);
}
void main()
{
    v = vec4(float(uint(f0().length()) + 1u) / 4.);

    gl_Position.x = ((gl_VertexID & 1) == 0 ? -1.0 : 1.0);
    gl_Position.y = ((gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    gl_Position.zw = vec2(0, 1);
})";

    const char kFS[] = R"(#version 300 es
precision highp float;
in vec4 v;
out vec4 color;

bool isEq(float a, float b) { return abs(float(a) - b) < 0.01; }

void main()
{
    if (isEq(v[0], 0.5) &&
        isEq(v[1], 0.5) &&
        isEq(v[2], 0.5) &&
        isEq(v[3], 0.5))
    {
        color = vec4(0, 1, 0, 1);
    }
    else
    {
        color = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that array length inside matrix constructor works.
TEST_P(GLSLTest_ES3, ArrayLengthInMatrixConstructor)
{
    const char kVS[] = R"(#version 300 es
precision highp float;
out mat2x2 v;

int[1] f0()
{
    return int[1](1);
}
void main()
{
    v = mat2x2(f0().length());

    gl_Position.x = ((gl_VertexID & 1) == 0 ? -1.0 : 1.0);
    gl_Position.y = ((gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    gl_Position.zw = vec2(0, 1);
})";

    const char kFS[] = R"(#version 300 es
precision highp float;
in mat2x2 v;
out vec4 color;

bool isEq(float a, float b) { return abs(a - b) < 0.01; }

void main()
{
    if (isEq(v[0][0], 1.) &&
        isEq(v[0][1], 0.) &&
        isEq(v[1][0], 0.) &&
        isEq(v[1][1], 1.))
    {
        color = vec4(0, 1, 0, 1);
    }
    else
    {
        color = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that array length inside vector constructor inside matrix constructor works.
TEST_P(GLSLTest_ES3, ArrayLengthInVectorInMatrixConstructor)
{
    const char kVS[] = R"(#version 300 es
precision highp float;
out mat2x2 v;

int[1] f0()
{
    return int[1](1);
}
void main()
{
    v = mat2x2(vec2(f0().length()), f0().length(), 0);

    gl_Position.x = ((gl_VertexID & 1) == 0 ? -1.0 : 1.0);
    gl_Position.y = ((gl_VertexID & 2) == 0 ? -1.0 : 1.0);
    gl_Position.zw = vec2(0, 1);
})";

    const char kFS[] = R"(#version 300 es
precision highp float;
in mat2x2 v;
out vec4 color;

bool isEq(float a, float b) { return abs(a - b) < 0.01; }

void main()
{
    if (isEq(v[0][0], 1.) &&
        isEq(v[0][1], 1.) &&
        isEq(v[1][0], 1.) &&
        isEq(v[1][1], 0.))
    {
        color = vec4(0, 1, 0, 1);
    }
    else
    {
        color = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that statements inside switch() get translated to correct HLSL.
TEST_P(GLSLTest_ES3, DifferentStatementsInsideSwitch)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform int u;
void main()
{
    switch (u)
    {
        case 0:
            ivec2 i;
            i.yx;
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
}

// Test that switch fall-through works correctly.
// This is a regression test for http://anglebug.com/40644631
TEST_P(GLSLTest_ES3, SwitchFallThroughCodeDuplication)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
uniform int u_zero;

void main()
{
    int i = 0;
    // switch should fall through both cases.
    switch(u_zero)
    {
        case 0:
            i += 1;
        case 1:
            i += 2;
    }
    if (i == 3)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
    else
    {
        my_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test switch/case where default is last.
TEST_P(GLSLTest_ES3, SwitchWithDefaultAtTheEnd)
{
    constexpr char kFS[] = R"(#version 300 es

precision highp float;
out vec4 my_FragColor;

uniform int u_zero;

void main()
{
    switch (u_zero)
    {
        case 1:
            my_FragColor = vec4(1, 0, 0, 1);
            break;
        default:
            my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a switch statement with an empty block inside as a final statement compiles.
TEST_P(GLSLTest_ES3, SwitchFinalCaseHasEmptyBlock)
{
    constexpr char kFS[] = R"(#version 300 es

precision mediump float;
uniform int i;
void main()
{
    switch (i)
    {
        case 0:
            break;
        default:
            {}
    }
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
}

// Test that a switch statement with an empty declaration inside as a final statement compiles.
TEST_P(GLSLTest_ES3, SwitchFinalCaseHasEmptyDeclaration)
{
    constexpr char kFS[] = R"(#version 300 es

precision mediump float;
uniform int i;
void main()
{
    switch (i)
    {
        case 0:
            break;
        default:
            float;
    }
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
}

// Test switch/case where break/return statements are within blocks.
TEST_P(GLSLTest_ES3, SwitchBreakOrReturnInsideBlocks)
{
    constexpr char kFS[] = R"(#version 300 es

precision highp float;

uniform int u_zero;
out vec4 my_FragColor;

bool test(int n)
{
    switch(n) {
        case 0:
        {
            {
                break;
            }
        }
        case 1:
        {
            return true;
        }
        case 2:
        {
            n++;
        }
    }
    return false;
}

void main()
{
    my_FragColor = test(u_zero + 1) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test switch/case where a variable is declared inside one of the cases and is accessed by a
// subsequent case.
TEST_P(GLSLTest_ES3, SwitchWithVariableDeclarationInside)
{
    constexpr char kFS[] = R"(#version 300 es

precision highp float;
out vec4 my_FragColor;

uniform int u_zero;

void main()
{
    my_FragColor = vec4(1, 0, 0, 1);
    switch (u_zero)
    {
        case 0:
            ivec2 i;
            i = ivec2(1, 0);
        default:
            my_FragColor = vec4(0, i[0], 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test nested switch/case where a variable is declared inside one of the cases and is accessed by a
// subsequent case.
TEST_P(GLSLTest_ES3, NestedSwitchWithVariableDeclarationInside)
{
    constexpr char kFS[] = R"(#version 300 es

precision highp float;
out vec4 my_FragColor;

uniform int u_zero;
uniform int u_zero2;

void main()
{
    my_FragColor = vec4(1, 0, 0, 1);
    switch (u_zero)
    {
        case 0:
            ivec2 i;
            i = ivec2(1, 0);
            switch (u_zero2)
            {
                case 0:
                    int j;
                default:
                    j = 1;
                    i *= j;
            }
        default:
            my_FragColor = vec4(0, i[0], 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that an empty switch/case statement is translated in a way that compiles and executes the
// init-statement.
TEST_P(GLSLTest_ES3, EmptySwitch)
{
    constexpr char kFS[] = R"(#version 300 es

precision highp float;

uniform int u_zero;
out vec4 my_FragColor;

void main()
{
    int i = u_zero;
    switch(++i) {}
    my_FragColor = (i == 1) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that an switch over a constant with mismatching cases works.
TEST_P(GLSLTest_ES3, SwitchWithConstantExpr)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 color;

void main()
{
    float r = 0.;
    float g = 1.;
    float b = 0.;

    switch(10)
    {
        case 44:
            r = 0.5;
        case 50:
            break;
    }

    switch(20)
    {
        case 198:
            g = 0.5;
        default:
            g -= 1.;
            break;
    }

    switch(30)
    {
        default:
            b = 0.5;
        case 4:
            b += 0.5;
            break;
    }

    color = vec4(r, g, b, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Test that basic infinite loops are either rejected or are pruned in WebGL
TEST_P(WebGL2GLSLTest, BasicInfiniteLoop)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform uint zero;
out vec4 color;

bool globalConstantVariable = true;

float f()
{
    // Should not be pruned
    while (true)
    {
        // Should not be pruned
        for (int i = 0; true; ++i)
        {
            if (zero < 10u)
            {
                switch (zero)
                {
                    case 0u:
                        // Loops should be pruned because of this `return`.
                        return 0.7;
                    default:
                        break;
                }
            }
        }
    }
}

void main()
{
    float r = 0.;
    float g = 1.;
    float b = 0.;

    bool localConstantVariable = true;
    bool localVariable = true;

    // Should be pruned
    while (true)
    {
        r += 0.1;
        if (r > 0.)
        {
            continue;
        }
    }

    if (zero != 0u)
    {
        localVariable = false;
    }

    // Should be pruned
    while (localConstantVariable)
    {
        g -= 0.1;
    }

    // Should not be pruned
    while (localConstantVariable)
    {
        b += 0.3;

        if (g > 0.4) { break; }
    }

    // Should be pruned
    for (; globalConstantVariable; )
    {
        g -= 0.1;

        switch (zero)
        {
            case 0u:
                r = 0.4;
                break;
            default:
                r = 0.2;
                break;
        }
    }

    // Should not be pruned
    while (localVariable)
    {
        b += 0.2;
        localVariable = !localVariable;
    }

    r = f();

    color = vec4(r, g, b, 1);
})";

    if (getEGLWindow()->isFeatureEnabled(Feature::RejectWebglShadersWithUndefinedBehavior))
    {
        GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
        EXPECT_EQ(0u, shader);
    }
    else
    {
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
        EXPECT_PIXEL_NEAR(0, 0, 178, 255, 127, 255, 1);
    }
}

// Test that while(true) loops with break/return are not rejected
TEST_P(WebGL2GLSLTest, NotInfiniteLoop)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform uint zero;
out vec4 color;

void main()
{
    float r = 0.;
    float g = 1.;
    float b = 0.;

    while (true)
    {
        r += 0.1;
        if (r > 0.4)
        {
            break;
        }
    }

    for (;;)
    {
        g -= 0.1;

        switch (zero)
        {
            case 0u:
                g -= 0.6;
                color = vec4(r, g, b, 1);
                return;
            default:
                r = 0.2;
                break;
        }
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_NEAR(0, 0, 127, 76, 0, 255, 1);
}

// Test that a constant struct inside an expression is handled correctly.
TEST_P(GLSLTest_ES3, ConstStructInsideExpression)
{
    // Incorrect output color was seen on Android. http://anglebug.com/42260946
    ANGLE_SKIP_TEST_IF(IsAndroid() && !IsNVIDIA() && IsOpenGLES());

    constexpr char kFS[] = R"(#version 300 es

precision highp float;
out vec4 my_FragColor;

uniform float u_zero;

struct S
{
    float field;
};

void main()
{
    const S constS = S(1.0);
    S nonConstS = constS;
    nonConstS.field = u_zero;
    bool fail = (constS == nonConstS);
    my_FragColor = vec4(0, 1, 0, 1);
    if (fail)
    {
        my_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a varying struct that's defined as a part of the declaration is handled correctly.
TEST_P(GLSLTest_ES3, VaryingStructWithInlineDefinition)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 inputAttribute;

flat out struct S
{
    int field;
} v_s;

void main()
{
    v_s.field = 1;
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(#version 300 es

precision highp float;
out vec4 my_FragColor;

flat in struct S
{
    int field;
} v_s;

void main()
{
    bool success = (v_s.field == 1);
    my_FragColor = vec4(1, 0, 0, 1);
    if (success)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that multi variables struct should not crash in separated struct expressions.
TEST_P(GLSLTest_ES3, VaryingStructWithInlineDefinition2)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 inputAttribute;
flat out struct A
{
    int a;
} z1, z2;
void main()
{
    z1.a = 1;
    z2.a = 2;
    gl_Position = inputAttribute;
})";
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
flat in struct A
{
    int a;
} z1, z2;
void main()
{
    bool success = (z1.a == 1 && z2.a == 2);
    my_FragColor = vec4(1, 0, 0, 1);
    if (success)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program.get(), "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a varying anonymous struct that is defined as a part of the declaration is handled
// correctly.
TEST_P(GLSLTest_ES3, VaryingAnonymousStructWithInlineDefinition)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 inputAttribute;
flat out struct
{
    int field;
} v_s;

void main()
{
    v_s.field = 1;
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
flat in struct
{
    int field;
} v_s;
void main()
{
    bool success = (v_s.field == 1);
    my_FragColor = vec4(1, 0, 0, 1);
    if (success)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program.get(), "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a varying anonymous structs that are defined as a part of the declaration is handled
// correctly.
TEST_P(GLSLTest_ES3, VaryingAnonymousStructWithInlineDefinition2)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 inputAttribute;
flat out struct
{
    int field;
} v_s0, v_s1;
void main()
{
    v_s0.field = 1;
    v_s1.field = 2;
    gl_Position = inputAttribute;
})";
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
flat in struct
{
    int field;
} v_s0, v_s1;
void main()
{
    bool success = (v_s0.field == 1 && v_s1.field == 2);
    my_FragColor = vec4(1, 0, 0, 1);
    if (success)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program.get(), "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a varying anonymous structs that are defined as a part of the declaration is handled
// in a specific way. Highlights ambiguity of ES "Chapter 9. Shader Interface Matching":
//  "When linking shaders, the type of declared vertex outputs and fragment inputs with the same
//  name must match"
TEST_P(GLSLTest_ES3, VaryingAnonymousStructWithInlineDefinition3)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 inputAttribute;
flat out struct
{
    int field;
} v_s0;
flat out struct
{
    int field;
} v_s1;
flat out struct
{
    int field;
} v_s2, v_s3;
void main()
{
    v_s0.field = 1;
    v_s1.field = 2;
    v_s2.field = 3;
    v_s3.field = 4;
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
flat in struct
{
    int field;
} v_s0, v_s1, v_s2, v_s3;
void main()
{
    bool success = v_s0.field == 1 && v_s1.field == 2 && v_s2.field == 3 && v_s3.field == 4;
    my_FragColor = vec4(1, 0, 0, 1);
    if (success)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program.get(), "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that a varying anonymous structs can be compared for equality.
TEST_P(GLSLTest_ES3, VaryingAnonymousStructEquality)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 inputAttribute;
flat out struct
{
    int field;
} v_s0;
flat out struct
{
    int field;
} v_s1;
flat out struct
{
    int field;
} v_s2, v_s3;

void main()
{
    v_s0.field = 1;
    v_s1.field = 2;
    v_s2.field = 3;
    v_s3.field = 4;
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
flat in struct
{
    int field;
} v_s0, v_s1, v_s2, v_s3;
void main()
{
    bool success = v_s0 != v_s1 && v_s0 != v_s2 && v_s0 != v_s3 && v_s1 != v_s2 && v_s1 != v_s3 && v_s2 != v_s3;
    success = success && v_s0.field == 1 && v_s1.field == 2 && v_s2.field == 3 && v_s3.field == 4;
    my_FragColor = vec4(1, 0, 0, 1);
    if (success)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program.get(), "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test mismatched precision in varying is handled correctly.
TEST_P(GLSLTest_ES3, MismatchPrecisionFloat)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 position;
uniform highp float inVal;
out highp float myVarying;

void main()
{
    myVarying = inVal;
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
in mediump float myVarying;

void main()
{
    my_FragColor = vec4(1, 0, 0, 1);
    if (myVarying > 1.0)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glUseProgram(program);
    GLint positionLocation              = glGetAttribLocation(program, "position");
    std::array<Vector3, 6> quadVertices = GetQuadVertices();
    for (Vector3 &vertex : quadVertices)
    {
        vertex.z() = 0.5f;
    }
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, quadVertices.data());
    glEnableVertexAttribArray(positionLocation);

    GLint inValLoc = glGetUniformLocation(program, "inVal");
    ASSERT_NE(-1, inValLoc);
    glUniform1f(inValLoc, static_cast<GLfloat>(1.003));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test mismatched precision in varying is handled correctly.
TEST_P(GLSLTest_ES3, MismatchPrecisionlowpFloat)
{
    // Note: SPIRV only has relaxed precision so both lowp and mediump turn into "relaxed
    // precision", thus this is the same test as MismatchPrecisionFloat but including it for
    // completeness in case something changes.
    constexpr char kVS[] = R"(#version 300 es
in vec4 position;
uniform highp float inVal;
out highp float myVarying;

void main()
{
    myVarying = inVal;
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
in lowp float myVarying;

void main()
{
    my_FragColor = vec4(1, 0, 0, 1);
    if (myVarying > 1.0)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glUseProgram(program);
    GLint positionLocation              = glGetAttribLocation(program, "position");
    std::array<Vector3, 6> quadVertices = GetQuadVertices();
    for (Vector3 &vertex : quadVertices)
    {
        vertex.z() = 0.5f;
    }
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, quadVertices.data());
    glEnableVertexAttribArray(positionLocation);

    GLint inValLoc = glGetUniformLocation(program, "inVal");
    ASSERT_NE(-1, inValLoc);
    glUniform1f(inValLoc, static_cast<GLfloat>(1.003));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test mismatched precision in varying is handled correctly.
TEST_P(GLSLTest_ES3, MismatchPrecisionVec2UnusedVarying)
{
    constexpr char kVS[] = R"(#version 300 es
in vec2 position;
uniform highp float inVal;
out highp float myVarying;
out highp vec2 texCoord;

void main()
{
    myVarying = inVal;
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
in mediump float myVarying;
in mediump vec2 texCoord;

void main()
{
    my_FragColor = vec4(1, 0, 0, 1);
    if (myVarying > 1.0)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glUseProgram(program);
    GLint positionLocation              = glGetAttribLocation(program, "position");
    std::array<Vector3, 6> quadVertices = GetQuadVertices();
    for (Vector3 &vertex : quadVertices)
    {
        vertex.z() = 0.5f;
    }
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, quadVertices.data());
    glEnableVertexAttribArray(positionLocation);

    GLint inValLoc = glGetUniformLocation(program, "inVal");
    ASSERT_NE(-1, inValLoc);
    glUniform1f(inValLoc, static_cast<GLfloat>(1.003));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test mismatched precision in varying is handled correctly.
TEST_P(GLSLTest_ES3, MismatchPrecisionMedToHigh)
{
    constexpr char kVS[] = R"(#version 300 es
in vec2 position;
uniform highp float inVal;
out mediump float myVarying;

void main()
{
    myVarying = inVal;
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
in highp float myVarying;

void main()
{
    my_FragColor = vec4(1, 0, 0, 1);
    if (myVarying > 1.0)
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glUseProgram(program);
    GLint positionLocation              = glGetAttribLocation(program, "position");
    std::array<Vector3, 6> quadVertices = GetQuadVertices();
    for (Vector3 &vertex : quadVertices)
    {
        vertex.z() = 0.5f;
    }
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, quadVertices.data());
    glEnableVertexAttribArray(positionLocation);

    GLint inValLoc = glGetUniformLocation(program, "inVal");
    ASSERT_NE(-1, inValLoc);
    glUniform1f(inValLoc, static_cast<GLfloat>(1.003));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that *= on boolean vectors fails compilation
TEST_P(GLSLTest, BVecMultiplyAssign)
{
    constexpr char kFS[] = R"(bvec4 c,s;void main(){s*=c;})";

    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_EQ(fs, 0u);
}

// Test vector/scalar arithmetic (in this case multiplication and addition).
TEST_P(GLSLTest, VectorScalarMultiplyAndAddInLoop)
{
    constexpr char kFS[] = R"(precision mediump float;

void main() {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < 2; i++)
    {
        gl_FragColor += (2.0 * gl_FragCoord.x);
    }
    if (gl_FragColor.g == gl_FragColor.r &&
        gl_FragColor.b == gl_FragColor.r &&
        gl_FragColor.a == gl_FragColor.r)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test vector/scalar arithmetic (in this case compound division and addition).
TEST_P(GLSLTest, VectorScalarDivideAndAddInLoop)
{
    constexpr char kFS[] = R"(precision mediump float;

void main() {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < 2; i++)
    {
        float x = gl_FragCoord.x;
        gl_FragColor = gl_FragColor + (x /= 2.0);
    }
    if (gl_FragColor.g == gl_FragColor.r &&
        gl_FragColor.b == gl_FragColor.r &&
        gl_FragColor.a == gl_FragColor.r)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test a fuzzer-discovered bug with the VectorizeVectorScalarArithmetic transformation.
TEST_P(GLSLTest, VectorScalarArithmeticWithSideEffectInLoop)
{
    // The VectorizeVectorScalarArithmetic transformation was generating invalid code in the past
    // (notice how sbcd references i outside the for loop.  The loop condition doesn't look right
    // either):
    //
    //     #version 450
    //     void main(){
    //     (gl_Position = vec4(0.0, 0.0, 0.0, 0.0));
    //     mat3 _utmp = mat3(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    //     vec3 _ures = vec3(0.0, 0.0, 0.0);
    //     vec3 sbcd = vec3(_ures[_ui]);
    //     for (int _ui = 0; (_ures[((_utmp[_ui] += (((sbcd *= _ures[_ui]), (_ures[_ui] = sbcd.x)),
    //     sbcd)), _ui)], (_ui < 7)); )
    //     {
    //     }
    //     }

    constexpr char kVS[] = R"(
void main()
{
    mat3 tmp;
    vec3 res;
    for(int i; res[tmp[i]+=res[i]*=res[i],i],i<7;);
})";

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);

    const char *sourceArray[1] = {kVS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kVS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that packing of excessive 3-column variables does not overflow the count of 3-column
// variables in VariablePacker
TEST_P(WebGL2GLSLTest, ExcessiveMat3UniformPacking)
{
    std::ostringstream srcStream;

    srcStream << "#version 300 es\n";
    srcStream << "precision mediump float;\n";
    srcStream << "out vec4 finalColor;\n";
    srcStream << "in vec4 color;\n";
    srcStream << "uniform mat4 r[254];\n";

    srcStream << "uniform mat3 ";
    constexpr size_t kNumUniforms = 10000;
    for (size_t i = 0; i < kNumUniforms; ++i)
    {
        if (i > 0)
        {
            srcStream << ", ";
        }
        srcStream << "m3a_" << i << "[256]";
    }
    srcStream << ";\n";

    srcStream << "void main(void) { finalColor = color; }\n";
    std::string src = std::move(srcStream).str();

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);

    const char *sourceArray[1] = {src.c_str()};
    GLint lengths[1]           = {static_cast<GLint>(src.length())};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_EQ(compileResult, 0);
}

// Test that a varying with a flat qualifier that is used as an operand of a folded ternary operator
// is handled correctly.
TEST_P(GLSLTest_ES3, FlatVaryingUsedInFoldedTernary)
{
    constexpr char kVS[] = R"(#version 300 es

in vec4 inputAttribute;

flat out int v;

void main()
{
    v = 1;
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(#version 300 es

precision highp float;
out vec4 my_FragColor;

flat in int v;

void main()
{
    my_FragColor = vec4(0, (true ? v : 0), 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "inputAttribute", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Verify that the link error message from last link failure is cleared when the new link is
// finished.
TEST_P(GLSLTest, ClearLinkErrorLog)
{
    constexpr char kVS[] = R"(attribute vec4 vert_in;
varying vec4 vert_out;
void main()
{
    gl_Position = vert_in;
    vert_out = vert_in;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying vec4 frag_in;
void main()
{
    gl_FragColor = frag_in;
})";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);

    GLuint program = glCreateProgram();

    // The first time the program link fails because of lack of fragment shader.
    glAttachShader(program, vs);
    glLinkProgram(program);
    GLint linkStatus = GL_TRUE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    ASSERT_FALSE(linkStatus);

    const std::string lackOfFragmentShader = QueryErrorMessage(program);
    EXPECT_TRUE(lackOfFragmentShader != "");

    // The second time the program link fails because of the mismatch of the varying types.
    glAttachShader(program, fs);
    glLinkProgram(program);
    linkStatus = GL_TRUE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    ASSERT_FALSE(linkStatus);

    const std::string varyingTypeMismatch = QueryErrorMessage(program);
    EXPECT_TRUE(varyingTypeMismatch != "");

    EXPECT_EQ(std::string::npos, varyingTypeMismatch.find(lackOfFragmentShader));

    glDetachShader(program, vs);
    glDetachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteProgram(program);

    ASSERT_GL_NO_ERROR();
}

// Verify that a valid program still draws correctly after a shader link error
TEST_P(GLSLTest, DrawAfterShaderLinkError)
{
    constexpr char kVS[]    = R"(attribute vec4 position;
        varying vec4 vColor;
        void main()
        {
            vColor = vec4(0.0, 1.0, 0.0, 1.0);
            gl_Position = position;
        })";
    constexpr char kFS[]    = R"(precision mediump float;
        varying vec4 vColor;
        void main()
        {
            gl_FragColor = vColor;
        })";
    constexpr char kBadFS[] = R"(WILL NOT COMPILE;)";

    GLuint fsBad = glCreateShader(GL_FRAGMENT_SHADER);

    // Create bad fragment shader
    {
        const char *sourceArray[1] = {kBadFS};
        glShaderSource(fsBad, 1, sourceArray, nullptr);
        glCompileShader(fsBad);

        GLint compileResult;
        glGetShaderiv(fsBad, GL_COMPILE_STATUS, &compileResult);
        ASSERT_FALSE(compileResult);
    }

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    GLuint fs = GetProgramShader(program, GL_FRAGMENT_SHADER);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glUseProgram(program);
    GLint positionLocation              = glGetAttribLocation(program, "position");
    std::array<Vector3, 6> quadVertices = GetQuadVertices();
    for (Vector3 &vertex : quadVertices)
    {
        vertex.z() = 0.5f;
    }
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, quadVertices.data());
    glEnableVertexAttribArray(positionLocation);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glDetachShader(program, fs);
    glAttachShader(program, fsBad);
    glLinkProgram(program);
    GLint linkStatus = GL_TRUE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    ASSERT_FALSE(linkStatus);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Validate error messages when the link mismatch occurs on the type of a non-struct varying.
TEST_P(GLSLTest, ErrorMessageOfVaryingMismatch)
{
    constexpr char kVS[] = R"(attribute vec4 inputAttribute;
varying vec4 vertex_out;
void main()
{
    vertex_out = inputAttribute;
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying float vertex_out;
void main()
{
    gl_FragColor = vec4(vertex_out, 0.0, 0.0, 1.0);
})";

    validateComponentsInErrorMessage(kVS, kFS, "Types", "varying 'vertex_out'");
}

// Validate error messages when the link mismatch occurs on the name of a varying field.
TEST_P(GLSLTest_ES3, ErrorMessageOfVaryingStructFieldNameMismatch)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 inputAttribute;
struct S {
    float val1;
    vec4 val2;
};
out S vertex_out;
void main()
{
    vertex_out.val2 = inputAttribute;
    vertex_out.val1 = inputAttribute[0];
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
struct S {
    float val1;
    vec4 val3;
};
in S vertex_out;
layout (location = 0) out vec4 frag_out;
void main()
{
    frag_out = vec4(vertex_out.val1, 0.0, 0.0, 1.0);
})";

    validateComponentsInErrorMessage(kVS, kFS, "Field names", "varying 'vertex_out'");
}

// Validate error messages when the link mismatch occurs on the type of a varying field.
TEST_P(GLSLTest_ES3, ErrorMessageOfVaryingStructFieldMismatch)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 inputAttribute;
struct S {
    float val1;
    vec4 val2;
};
out S vertex_out;
void main()
{
    vertex_out.val2 = inputAttribute;
    vertex_out.val1 = inputAttribute[0];
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
struct S {
    float val1;
    vec2 val2;
};
in S vertex_out;
layout (location = 0) out vec4 frag_out;
void main()
{
    frag_out = vec4(vertex_out.val1, 0.0, 0.0, 1.0);
})";

    validateComponentsInErrorMessage(kVS, kFS, "Types",
                                     "varying 'vertex_out' member 'vertex_out.val2'");
}

// Validate error messages when the link mismatch occurs on the name of a struct member of a uniform
// field.
TEST_P(GLSLTest, ErrorMessageOfLinkUniformStructFieldNameMismatch)
{
    constexpr char kVS[] = R"(
struct T
{
    vec2 t1;
    vec3 t2;
};
struct S {
    T val1;
    vec4 val2;
};
uniform S uni;

attribute vec4 inputAttribute;
varying vec4 vertex_out;
void main()
{
    vertex_out = uni.val2;
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(precision highp float;
struct T
{
    vec2 t1;
    vec3 t3;
};
struct S {
    T val1;
    vec4 val2;
};
uniform S uni;

varying vec4 vertex_out;
void main()
{
    gl_FragColor = vec4(uni.val1.t1[0], 0.0, 0.0, 1.0);
})";

    validateComponentsInErrorMessage(kVS, kFS, "Field names", "uniform 'uni' member 'uni.val1'");
}

// Validate error messages  when the link mismatch occurs on the type of a non-struct uniform block
// field.
TEST_P(GLSLTest_ES3, ErrorMessageOfLinkInterfaceBlockFieldMismatch)
{
    constexpr char kVS[] = R"(#version 300 es
uniform S {
    vec2 val1;
    vec4 val2;
} uni;

in vec4 inputAttribute;
out vec4 vertex_out;
void main()
{
    vertex_out = uni.val2;
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform S {
    vec2 val1;
    vec3 val2;
} uni;

in vec4 vertex_out;
layout (location = 0) out vec4 frag_out;
void main()
{
    frag_out = vec4(uni.val1[0], 0.0, 0.0, 1.0);
})";

    validateComponentsInErrorMessage(kVS, kFS, "Types", "uniform block 'S' member 'S.val2'");
}

// Validate error messages  when the link mismatch occurs on the type of a member of a uniform block
// struct field.
TEST_P(GLSLTest_ES3, ErrorMessageOfLinkInterfaceBlockStructFieldMismatch)
{
    constexpr char kVS[] = R"(#version 300 es
struct T
{
    vec2 t1;
    vec3 t2;
};
uniform S {
    T val1;
    vec4 val2;
} uni;

in vec4 inputAttribute;
out vec4 vertex_out;
void main()
{
    vertex_out = uni.val2;
    gl_Position = inputAttribute;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
struct T
{
    vec2 t1;
    vec4 t2;
};
uniform S {
    T val1;
    vec4 val2;
} uni;

in vec4 vertex_out;
layout (location = 0) out vec4 frag_out;
void main()
{
    frag_out = vec4(uni.val1.t1[0], 0.0, 0.0, 1.0);
})";

    validateComponentsInErrorMessage(kVS, kFS, "Types", "uniform block 'S' member 'S.val1.t2'");
}

// Test a vertex shader that doesn't declare any varyings with a fragment shader that statically
// uses a varying, but in a statement that gets trivially optimized out by the compiler.
TEST_P(GLSLTest_ES3, FragmentShaderStaticallyUsesVaryingMissingFromVertex)
{
    constexpr char kVS[] = R"(#version 300 es
precision mediump float;

void main()
{
    gl_Position = vec4(0, 1, 0, 1);
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
in float foo;
out vec4 my_FragColor;

void main()
{
    if (false)
    {
        float unreferenced = foo;
    }
    my_FragColor = vec4(0, 1, 0, 1);
})";

    validateComponentsInErrorMessage(kVS, kFS, "does not match any", "foo");
}

// Test a varying that is statically used but not active in the fragment shader.
TEST_P(GLSLTest_ES3, VaryingStaticallyUsedButNotActiveInFragmentShader)
{
    constexpr char kVS[] = R"(#version 300 es
precision mediump float;
in vec4 iv;
out vec4 v;
void main()
{
    gl_Position = iv;
    v = iv;
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
in vec4 v;
out vec4 color;
void main()
{
    color = true ? vec4(0.0) : v;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that linking varyings by location works.
TEST_P(GLSLTest_ES31, LinkVaryingsByLocation)
{
    constexpr char kVS[] = R"(#version 310 es
precision highp float;
in vec4 position;
layout(location = 1) out vec4 shaderOutput;
void main() {
    gl_Position = position;
    shaderOutput = vec4(0.0, 1.0, 0.0, 1.0);
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;
layout(location = 1) in vec4 shaderInput;
out vec4 outColor;
void main() {
    outColor = shaderInput;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test nesting floor() calls with a large multiplier inside.
TEST_P(GLSLTest_ES3, NestedFloorWithLargeMultiplierInside)
{
    // D3D11 seems to ignore the floor() calls in this particular case, so one of the corners ends
    // up red. http://crbug.com/838885
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
void main()
{
    vec2 coord = gl_FragCoord.xy / 500.0;
    my_FragColor = vec4(1, 0, 0, 1);
    if (coord.y + 0.1 > floor(1e-6 * floor(coord.x*4e5)))
    {
        my_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    // Verify that all the corners of the rendered result are green.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, 0, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() - 1, GLColor::green);
}

// Verify that a link error is generated when the sum of the number of active image uniforms and
// active shader storage blocks in a rendering pipeline exceeds
// GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES.
TEST_P(GLSLTest_ES31, ExceedCombinedShaderOutputResourcesInVSAndFS)
{
    // TODO(jiawei.shao@intel.com): enable this test when shader storage buffer is supported on
    // D3D11 back-ends.
    ANGLE_SKIP_TEST_IF(IsD3D11());

    GLint maxVertexShaderStorageBlocks;
    GLint maxVertexImageUniforms;
    GLint maxFragmentShaderStorageBlocks;
    GLint maxFragmentImageUniforms;
    GLint maxCombinedShaderStorageBlocks;
    GLint maxCombinedImageUniforms;
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &maxVertexShaderStorageBlocks);
    glGetIntegerv(GL_MAX_VERTEX_IMAGE_UNIFORMS, &maxVertexImageUniforms);
    glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &maxFragmentShaderStorageBlocks);
    glGetIntegerv(GL_MAX_FRAGMENT_IMAGE_UNIFORMS, &maxFragmentImageUniforms);
    glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &maxCombinedShaderStorageBlocks);
    glGetIntegerv(GL_MAX_COMBINED_IMAGE_UNIFORMS, &maxCombinedImageUniforms);

    ASSERT_GE(maxCombinedShaderStorageBlocks, maxVertexShaderStorageBlocks);
    ASSERT_GE(maxCombinedShaderStorageBlocks, maxFragmentShaderStorageBlocks);
    ASSERT_GE(maxCombinedImageUniforms, maxVertexImageUniforms);
    ASSERT_GE(maxCombinedImageUniforms, maxFragmentImageUniforms);

    GLint vertexSSBOs   = maxVertexShaderStorageBlocks;
    GLint fragmentSSBOs = maxFragmentShaderStorageBlocks;
    // Limit the sum of ssbos in vertex and fragment shaders to maxCombinedShaderStorageBlocks.
    if (vertexSSBOs + fragmentSSBOs > maxCombinedShaderStorageBlocks)
    {
        fragmentSSBOs = maxCombinedShaderStorageBlocks - vertexSSBOs;
    }

    GLint vertexImages   = maxVertexImageUniforms;
    GLint fragmentImages = maxFragmentImageUniforms;
    // Limit the sum of images in vertex and fragment shaders to maxCombinedImageUniforms.
    if (vertexImages + fragmentImages > maxCombinedImageUniforms)
    {
        vertexImages = maxCombinedImageUniforms - fragmentImages;
    }

    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);

    GLint maxCombinedShaderOutputResources;
    glGetIntegerv(GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES, &maxCombinedShaderOutputResources);
    ASSERT_GL_NO_ERROR();

    ANGLE_SKIP_TEST_IF(vertexSSBOs + fragmentSSBOs + vertexImages + fragmentImages +
                           maxDrawBuffers <=
                       maxCombinedShaderOutputResources);

    std::ostringstream vertexStream;
    vertexStream << "#version 310 es\n";
    for (int i = 0; i < vertexSSBOs; ++i)
    {
        vertexStream << "layout(shared, binding = " << i << ") buffer blockName" << i
                     << "{\n"
                        "    float data;\n"
                        "} ssbo"
                     << i << ";\n";
    }
    vertexStream << "layout(r32f, binding = 0) uniform highp image2D imageArray[" << vertexImages
                 << "];\n";
    vertexStream << "void main()\n"
                    "{\n"
                    "    float val = 0.1;\n"
                    "    vec4 val2 = vec4(0.0);\n";
    for (int i = 0; i < vertexSSBOs; ++i)
    {
        vertexStream << "    val += ssbo" << i << ".data; \n";
    }
    for (int i = 0; i < vertexImages; ++i)
    {
        vertexStream << "    val2 += imageLoad(imageArray[" << i << "], ivec2(0, 0)); \n";
    }
    vertexStream << "    gl_Position = vec4(val, val2);\n"
                    "}\n";

    std::ostringstream fragmentStream;
    fragmentStream << "#version 310 es\n" << "precision highp float;\n";
    for (int i = 0; i < fragmentSSBOs; ++i)
    {
        fragmentStream << "layout(shared, binding = " << i << ") buffer blockName" << i
                       << "{\n"
                          "    float data;\n"
                          "} ssbo"
                       << i << ";\n";
    }
    fragmentStream << "layout(r32f, binding = 0) uniform highp image2D imageArray["
                   << fragmentImages << "];\n";
    fragmentStream << "layout (location = 0) out vec4 foutput[" << maxDrawBuffers << "];\n";

    fragmentStream << "void main()\n"
                      "{\n"
                      "    float val = 0.1;\n"
                      "    vec4 val2 = vec4(0.0);\n";
    for (int i = 0; i < fragmentSSBOs; ++i)
    {
        fragmentStream << "    val += ssbo" << i << ".data; \n";
    }
    for (int i = 0; i < fragmentImages; ++i)
    {
        fragmentStream << "    val2 += imageLoad(imageArray[" << i << "], ivec2(0, 0)); \n";
    }
    for (int i = 0; i < maxDrawBuffers; ++i)
    {
        fragmentStream << "    foutput[" << i << "] = vec4(val, val2);\n";
    }
    fragmentStream << "}\n";

    GLuint program = CompileProgram(vertexStream.str().c_str(), fragmentStream.str().c_str());
    EXPECT_EQ(0u, program);

    ASSERT_GL_NO_ERROR();
}

// Test that assigning an assignment expression to a swizzled vector field in a user-defined
// function works correctly.
TEST_P(GLSLTest_ES3, AssignToSwizzled)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;

uniform float uzero;

vec3 fun(float s, float v)
{
    vec3 r = vec3(0);
    if (s < 1.0) {
        r.x = r.y = r.z = v;
        return r;
    }
    return r;
}

void main()
{
    my_FragColor.a = 1.0;
    my_FragColor.rgb = fun(uzero, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Similar to AssignToSwizzled, but uses other assignment operators than `=`.
TEST_P(GLSLTest_ES3, AssignToSwizzled2)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;

uniform float uzero;

vec3 fun(float s, float v)
{
    vec3 r = vec3(0.125, 0.5, 0.);
    if (s < 1.0) {
        r.x /= r.y *= r.z += v;
        return r;
    }
    return r;
}

void main()
{
    my_FragColor.a = 1.0;
    my_FragColor.rgb = fun(uzero, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_NEAR(0, 0, 63, 127, 255, 255, 1);
}

// Test that swizzled vector to bool cast works correctly.
TEST_P(GLSLTest_ES3, SwizzledToBoolCoercion)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 o;
uniform vec2 u;
void main()
{
    bvec2 b = bvec2(u.yx);
    if (b.x&&!b.y)
        o = vec4(1.0);
})";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLint uloc = glGetUniformLocation(program, "u");
    ASSERT_NE(uloc, -1);
    glUniform2f(uloc, 0, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test a fragment shader that returns inside if (that being the only branch that actually gets
// executed). Regression test for http://anglebug.com/42261034
TEST_P(GLSLTest, IfElseIfAndReturn)
{
    constexpr char kVS[] = R"(attribute vec4 a_position;
varying vec2 vPos;
void main()
{
    gl_Position = a_position;
    vPos = a_position.xy;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 vPos;
void main()
{
    if (vPos.x < 1.0) // This colors the whole canvas green
    {
        gl_FragColor = vec4(0, 1, 0, 1);
        return;
    }
    else if (vPos.x < 1.1) // This should have no effect
    {
        gl_FragColor = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "a_position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that if-else blocks whose contents get pruned due to compile-time constant conditions work.
TEST_P(GLSLTest, IfElsePrunedBlocks)
{
    constexpr char kFS[] = R"(precision mediump float;
uniform float u;
void main()
{
    // if with only a pruned true block
    if (u > 0.0)
        if (false) discard;

    // if with a pruned true block and a false block
    if (u > 0.0)
    {
        if (false) discard;
    }
    else
        ;

    // if with a true block and a pruned false block
    if (u > 0.0)
        ;
    else
        if (false) discard;

    // if with a pruned true block and a pruned false block
    if (u > 0.0)
    {
        if (false) discard;
    }
    else
        if (false) discard;

    gl_FragColor = vec4(0, 1, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests that PointCoord behaves the same betweeen a user FBO and the back buffer.
TEST_P(GLSLTest, PointCoordConsistency)
{
    constexpr char kPointCoordVS[] = R"(attribute vec2 position;
uniform vec2 viewportSize;
void main()
{
   gl_Position = vec4(position, 0, 1);
   gl_PointSize = viewportSize.x;
})";

    constexpr char kPointCoordFS[] = R"(void main()
{
    gl_FragColor = vec4(gl_PointCoord.xy, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kPointCoordVS, kPointCoordFS);
    glUseProgram(program);

    GLint uniLoc = glGetUniformLocation(program, "viewportSize");
    ASSERT_NE(-1, uniLoc);
    glUniform2f(uniLoc, static_cast<GLfloat>(getWindowWidth()),
                static_cast<GLfloat>(getWindowHeight()));

    // Draw to backbuffer.
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> backbufferData(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 backbufferData.data());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Draw to user FBO.
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> userFBOData(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 userFBOData.data());

    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(userFBOData.size(), backbufferData.size());
    EXPECT_EQ(userFBOData, backbufferData);
}

bool SubrectEquals(const std::vector<GLColor> &bigArray,
                   const std::vector<GLColor> &smallArray,
                   int bigSize,
                   int offset,
                   int smallSize)
{
    int badPixels = 0;
    for (int y = 0; y < smallSize; y++)
    {
        for (int x = 0; x < smallSize; x++)
        {
            int bigOffset   = (y + offset) * bigSize + x + offset;
            int smallOffset = y * smallSize + x;
            if (bigArray[bigOffset] != smallArray[smallOffset])
                badPixels++;
        }
    }
    return badPixels == 0;
}

// Tests that FragCoord behaves the same betweeen a user FBO and the back buffer.
TEST_P(GLSLTest, FragCoordConsistency)
{
    constexpr char kFragCoordShader[] = R"(uniform mediump vec2 viewportSize;
void main()
{
    gl_FragColor = vec4(gl_FragCoord.xy / viewportSize, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragCoordShader);
    glUseProgram(program);

    GLint uniLoc = glGetUniformLocation(program, "viewportSize");
    ASSERT_NE(-1, uniLoc);
    glUniform2f(uniLoc, static_cast<GLfloat>(getWindowWidth()),
                static_cast<GLfloat>(getWindowHeight()));

    // Draw to backbuffer.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> backbufferData(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 backbufferData.data());

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    ASSERT_GL_NO_ERROR();
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Draw to user FBO.
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);
    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> userFBOData(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 userFBOData.data());

    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(userFBOData.size(), backbufferData.size());
    EXPECT_EQ(userFBOData, backbufferData)
        << "FragCoord should be the same to default and user FBO";

    // Repeat the same test but with a smaller viewport.
    ASSERT_EQ(getWindowHeight(), getWindowWidth());
    const int kQuarterSize = getWindowWidth() >> 2;
    glViewport(kQuarterSize, kQuarterSize, kQuarterSize * 2, kQuarterSize * 2);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);

    std::vector<GLColor> userFBOViewportData(kQuarterSize * kQuarterSize * 4);
    glReadPixels(kQuarterSize, kQuarterSize, kQuarterSize * 2, kQuarterSize * 2, GL_RGBA,
                 GL_UNSIGNED_BYTE, userFBOViewportData.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5);

    std::vector<GLColor> defaultFBOViewportData(kQuarterSize * kQuarterSize * 4);
    glReadPixels(kQuarterSize, kQuarterSize, kQuarterSize * 2, kQuarterSize * 2, GL_RGBA,
                 GL_UNSIGNED_BYTE, defaultFBOViewportData.data());
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(userFBOViewportData, defaultFBOViewportData)
        << "FragCoord should be the same to default and user FBO even with a custom viewport";

    // Check that the subrectangles are the same between the viewport and non-viewport modes.
    EXPECT_TRUE(SubrectEquals(userFBOData, userFBOViewportData, getWindowWidth(), kQuarterSize,
                              kQuarterSize * 2));
    EXPECT_TRUE(SubrectEquals(backbufferData, defaultFBOViewportData, getWindowWidth(),
                              kQuarterSize, kQuarterSize * 2));
}

// Ensure that using defined in a macro works in this simple case. This mirrors a dEQP test.
TEST_P(GLSLTest, DefinedInMacroSucceeds)
{
    constexpr char kVS[] = R"(precision mediump float;
attribute highp vec4 position;
varying vec2 out0;

void main()
{
#define AAA defined(BBB)

#if !AAA
    out0 = vec2(0.0, 1.0);
#else
    out0 = vec2(1.0, 0.0);
#endif
    gl_Position = position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 out0;
void main()
{
    gl_FragColor = vec4(out0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Validate the defined operator is evaluated when the macro is called, not when defined.
TEST_P(GLSLTest, DefinedInMacroWithUndef)
{
    constexpr char kVS[] = R"(precision mediump float;
attribute highp vec4 position;
varying vec2 out0;

void main()
{
#define BBB 1
#define AAA defined(BBB)
#undef BBB

#if AAA
    out0 = vec2(1.0, 0.0);
#else
    out0 = vec2(0.0, 1.0);
#endif
    gl_Position = position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 out0;
void main()
{
    gl_FragColor = vec4(out0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Validate the defined operator is evaluated when the macro is called, not when defined.
TEST_P(GLSLTest, DefinedAfterMacroUsage)
{
    constexpr char kVS[] = R"(precision mediump float;
attribute highp vec4 position;
varying vec2 out0;

void main()
{
#define AAA defined(BBB)
#define BBB 1

#if AAA
    out0 = vec2(0.0, 1.0);
#else
    out0 = vec2(1.0, 0.0);
#endif
    gl_Position = position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 out0;
void main()
{
    gl_FragColor = vec4(out0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "position", 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test generating "defined" by concatenation when a macro is called. This is not allowed.
TEST_P(GLSLTest, DefinedInMacroConcatenationNotAllowed)
{
    constexpr char kVS[] = R"(precision mediump float;
attribute highp vec4 position;
varying vec2 out0;

void main()
{
#define BBB 1
#define AAA(defi, ned) defi ## ned(BBB)

#if AAA(defi, ned)
    out0 = vec2(0.0, 1.0);
#else
    out0 = vec2(1.0, 0.0);
#endif
    gl_Position = position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 out0;
void main()
{
    gl_FragColor = vec4(out0, 0, 1);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
    glDeleteProgram(program);
}

// Test using defined in a macro parameter name. This is not allowed.
TEST_P(GLSLTest, DefinedAsParameterNameNotAllowed)
{
    constexpr char kVS[] = R"(precision mediump float;
attribute highp vec4 position;
varying vec2 out0;

void main()
{
#define BBB 1
#define AAA(defined) defined(BBB)

#if AAA(defined)
    out0 = vec2(0.0, 1.0);
#else
    out0 = vec2(1.0, 0.0);
#endif
    gl_Position = position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying vec2 out0;
void main()
{
    gl_FragColor = vec4(out0, 0, 1);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
    glDeleteProgram(program);
}

// Ensure that defined in a macro is no accepted in WebGL.
TEST_P(WebGLGLSLTest, DefinedInMacroFails)
{
    constexpr char kVS[] = R"(precision mediump float;
attribute highp vec4 position;
varying float out0;

void main()
{
#define AAA defined(BBB)

#if !AAA
    out0 = 1.0;
#else
    out0 = 0.0;
#endif
    gl_Position = dEQP_Position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying float out0;
void main()
{
    gl_FragColor = vec4(out0, 0, 0, 1);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
    glDeleteProgram(program);
}

// Simple test using a define macro in WebGL.
TEST_P(WebGLGLSLTest, DefinedGLESSymbol)
{
    constexpr char kVS[] = R"(void main()
{
    gl_Position = vec4(1, 0, 0, 1);
})";

    constexpr char kFS[] = R"(#if defined(GL_ES)
precision mediump float;
void main()
{
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
#else
foo
#endif
)";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that inactive output variables compile ok in combination with initOutputVariables
// (which is enabled on WebGL).
TEST_P(WebGL2GLSLTest, InactiveOutput)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 _cassgl_2_;
void main()
{
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_NE(shader, 0u);
}

// Test that clamp applied on non-literal indices is correct on es 100 shaders.
TEST_P(GLSLTest, ValidIndexClampES100)
{
    // http://anglebug.com/42264558
    ANGLE_SKIP_TEST_IF(IsD3D9());

    constexpr char kFS[] = R"(
precision mediump float;
uniform int u;
uniform mat4 m[2];
void main()
{
    gl_FragColor = vec4(m[u][1].xyz, 1);
}
)";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint uniformLocation = glGetUniformLocation(program, "u");
    ASSERT_NE(-1, uniformLocation);

    GLint matrixLocation = glGetUniformLocation(program, "m");
    ASSERT_NE(matrixLocation, -1);
    const std::array<GLfloat, 32> mValue = {{0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f,
                                             0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f,
                                             1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                             0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}};
    glUniformMatrix4fv(matrixLocation, 2, false, mValue.data());

    glUniform1i(uniformLocation, 1);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that clamp applied on non-literal indices is correct on es 300 shaders.
TEST_P(GLSLTest_ES3, ValidIndexClampES300)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;
uniform int u;
mat4 m[4] = mat4[4](mat4(0.25), mat4(0.5), mat4(1), mat4(0.75));
void main()
{
    color = vec4(m[u][2].xyz, 1);
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint uniformLocation = glGetUniformLocation(program, "u");
    ASSERT_NE(-1, uniformLocation);

    glUniform1i(uniformLocation, 2);
    EXPECT_GL_NO_ERROR();
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
}

// Tests constant folding of non-square 'matrixCompMult'.
TEST_P(GLSLTest_ES3, NonSquareMatrixCompMult)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

const mat4x2 matA = mat4x2(2.0, 4.0, 8.0, 16.0, 32.0, 64.0, 128.0, 256.0);
const mat4x2 matB = mat4x2(1.0/2.0, 1.0/4.0, 1.0/8.0, 1.0/16.0, 1.0/32.0, 1.0/64.0, 1.0/128.0, 1.0/256.0);

out vec4 color;

void main()
{
    mat4x2 result = matrixCompMult(matA, matB);
    vec2 vresult = result * vec4(1.0, 1.0, 1.0, 1.0);
    if (vresult == vec2(4.0, 4.0))
    {
        color = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else
    {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test initializing an array with the same name of previously declared array
TEST_P(GLSLTest_ES3, InitSameNameArray)
{
    constexpr char kFS[] = R"(#version 300 es
      precision highp float;
      out vec4 my_FragColor;

      void main()
      {
          float arr[2] = float[2](1.0, 1.0);
          {
              float arr[2] = arr;
              my_FragColor = vec4(0.0, arr[0], 0.0, arr[1]);
          }
      })";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests using gl_FragData[0] instead of gl_FragColor.
TEST_P(GLSLTest, FragData)
{
    constexpr char kFS[] = R"(void main() { gl_FragData[0] = vec4(1, 0, 0, 1); })";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Tests using gl_FragData[0] instead of gl_FragColor with GL_SAMPLE_ALPHA_TO_COVERAGE
// Regression test for https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/5520
TEST_P(GLSLTest, FragData_AlphaToCoverage)
{
    constexpr char kFS[] = R"(void main() { gl_FragData[0] = vec4(1, 0, 0, 1); })";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test angle can handle big initial stack size with dynamic stack allocation.
TEST_P(GLSLTest, MemoryExhaustedTest)
{
    GLuint program =
        CompileProgram(essl1_shaders::vs::Simple(), BuildBigInitialStackShader(36).c_str());
    EXPECT_NE(0u, program);
}

// Test that inactive samplers in structs don't cause any errors.
TEST_P(GLSLTest, InactiveSamplersInStruct)
{
    constexpr char kVS[] = R"(attribute vec4 a_position;
void main() {
  gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision highp float;
struct S
{
    vec4 v;
    sampler2D t[10];
};
uniform S s;
void main() {
  gl_FragColor = s.v;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);

    drawQuad(program, "a_position", 0.5f);
}

// Helper functions for MixedRowAndColumnMajorMatrices* tests

// Round up to alignment, assuming it's a power of 2
uint32_t RoundUpPow2(uint32_t value, uint32_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

void CreateOutputBuffer(GLBuffer *buffer, uint32_t binding)
{
    unsigned int outputInitData = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outputInitData), &outputInitData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, *buffer);
    EXPECT_GL_NO_ERROR();
}

// Fill provided buffer with matrices based on the given dimensions.  The buffer should be large
// enough to accomodate the data.
uint32_t FillBuffer(const std::pair<uint32_t, uint32_t> matrixDims[],
                    const bool matrixIsColMajor[],
                    size_t matrixCount,
                    float data[],
                    bool isStd430,
                    bool isTransposed)
{
    size_t offset = 0;
    for (size_t m = 0; m < matrixCount; ++m)
    {
        uint32_t cols   = matrixDims[m].first;
        uint32_t rows   = matrixDims[m].second;
        bool isColMajor = matrixIsColMajor[m] != isTransposed;

        uint32_t arraySize              = isColMajor ? cols : rows;
        uint32_t arrayElementComponents = isColMajor ? rows : cols;
        // Note: stride is generally 4 with std140, except for scalar and gvec2 types (which
        // MixedRowAndColumnMajorMatrices* tests don't use).  With std430, small matrices can have
        // a stride of 2 between rows/columns.
        uint32_t stride = isStd430 ? RoundUpPow2(arrayElementComponents, 2) : 4;

        offset = RoundUpPow2(offset, stride);

        for (uint32_t i = 0; i < arraySize; ++i)
        {
            for (uint32_t c = 0; c < arrayElementComponents; ++c)
            {
                uint32_t row = isColMajor ? c : i;
                uint32_t col = isColMajor ? i : c;

                data[offset + i * stride + c] = col * 4 + row;
            }
        }

        offset += arraySize * stride;
    }
    return offset;
}

// Initialize and bind the buffer.
template <typename T>
void InitBuffer(GLuint program,
                const char *name,
                GLuint buffer,
                uint32_t bindingIndex,
                const T data[],
                uint32_t dataSize,
                bool isUniform)
{
    GLenum bindPoint = isUniform ? GL_UNIFORM_BUFFER : GL_SHADER_STORAGE_BUFFER;

    glBindBufferBase(bindPoint, bindingIndex, buffer);
    glBufferData(bindPoint, dataSize * sizeof(*data), data, GL_STATIC_DRAW);

    if (isUniform)
    {
        GLint blockIndex = glGetUniformBlockIndex(program, name);
        glUniformBlockBinding(program, blockIndex, bindingIndex);
    }
}

// Verify that buffer data is written by the shader as expected.
template <typename T>
bool VerifyBuffer(GLuint buffer, const T data[], uint32_t dataSize)
{
    uint32_t sizeInBytes = dataSize * sizeof(*data);

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);

    const T *ptr = reinterpret_cast<const T *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeInBytes, GL_MAP_READ_BIT));

    bool isCorrect = memcmp(ptr, data, sizeInBytes) == 0;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return isCorrect;
}

// Verify that the success output of the shader is as expected.
bool VerifySuccess(GLuint buffer)
{
    uint32_t success = 1;
    return VerifyBuffer(buffer, reinterpret_cast<const float *>(&success), 1);
}

// Test reading from UBOs and SSBOs and writing to SSBOs with mixed row- and colum-major layouts in
// both std140 and std430 layouts.  Tests many combinations of std140 vs std430, struct being used
// as row- or column-major in different UBOs, reading from UBOs and SSBOs and writing to SSBOs,
// nested structs, matrix arrays, inout parameters etc.
//
// Some very specific corner cases that are not covered here are tested in the subsequent tests.
TEST_P(GLSLTest_ES31, MixedRowAndColumnMajorMatrices)
{
    GLint maxComputeShaderStorageBlocks;
    glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &maxComputeShaderStorageBlocks);

    // The test uses 9 SSBOs.  Skip if not that many are supported.
    ANGLE_SKIP_TEST_IF(maxComputeShaderStorageBlocks < 9);

    // Fails on Nvidia because having |Matrices| qualified as row-major in one UBO makes the other
    // UBO also see it as row-major despite explicit column-major qualifier.
    // http://anglebug.com/42262474
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGL());

    // Fails on mesa because in the first UBO which is qualified as column-major, |Matrices| is
    // read column-major despite explicit row-major qualifier.  http://anglebug.com/42262481
    ANGLE_SKIP_TEST_IF(IsLinux() && IsIntel() && IsOpenGL());

    // Fails on windows AMD on GL: http://anglebug.com/42262482
    ANGLE_SKIP_TEST_IF(IsWindows() && IsOpenGL() && IsAMD());

    // Fails to compile the shader on Android.  http://anglebug.com/42262483
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // Fails on assertion in translation to D3D.  http://anglebug.com/42262486
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // Fails on SSBO validation on Android/Vulkan.  http://anglebug.com/42262485
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsVulkan());

    // Fails input verification as well as std140 SSBO validation.  http://anglebug.com/42262489
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsVulkan());

    // Fails on ARM on Vulkan.  http://anglebug.com/42263107
    ANGLE_SKIP_TEST_IF(IsARM() && IsVulkan());

    constexpr char kCS[] = R"(#version 310 es
precision highp float;
layout(local_size_x=1) in;

struct Inner
{
    mat3x4 m3c4r;
    mat4x3 m4c3r;
};

struct Matrices
{
    mat2 m2c2r;
    mat2x3 m2c3r[2];
    mat3x2 m3c2r;
    Inner inner;
};

// For simplicity, the layouts are either of:
// - col-major mat4, row-major rest
// - row-major mat4, col-major rest
//
// The former is tagged with c, the latter with r.
layout(std140, column_major) uniform Ubo140c
{
    mat4 m4c4r;
    layout(row_major) Matrices m;
} ubo140cIn;

layout(std140, row_major) uniform Ubo140r
{
    mat4 m4c4r;
    layout(column_major) Matrices m;
} ubo140rIn;

layout(std140, row_major, binding = 0) buffer Ssbo140c
{
    layout(column_major) mat4 m4c4r;
    Matrices m;
} ssbo140cIn;

layout(std140, column_major, binding = 1) buffer Ssbo140r
{
    layout(row_major) mat4 m4c4r;
    Matrices m;
} ssbo140rIn;

layout(std430, column_major, binding = 2) buffer Ssbo430c
{
    mat4 m4c4r;
    layout(row_major) Matrices m;
} ssbo430cIn;

layout(std430, row_major, binding = 3) buffer Ssbo430r
{
    mat4 m4c4r;
    layout(column_major) Matrices m;
} ssbo430rIn;

layout(std140, row_major, binding = 4) buffer Ssbo140cOut
{
    layout(column_major) mat4 m4c4r;
    Matrices m;
} ssbo140cOut;

layout(std140, column_major, binding = 5) buffer Ssbo140rOut
{
    layout(row_major) mat4 m4c4r;
    Matrices m;
} ssbo140rOut;

layout(std430, column_major, binding = 6) buffer Ssbo430cOut
{
    mat4 m4c4r;
    layout(row_major) Matrices m;
} ssbo430cOut;

layout(std430, row_major, binding = 7) buffer Ssbo430rOut
{
    mat4 m4c4r;
    layout(column_major) Matrices m;
} ssbo430rOut;

layout(std140, binding = 8) buffer Result
{
    uint success;
} resultOut;

#define EXPECT(result, expression, value) if ((expression) != value) { result = false; }
#define EXPECTV(result, expression, value) if (any(notEqual(expression, value))) { result = false; }

#define VERIFY_IN(result, mat, cols, rows)                  \
    EXPECT(result, mat[0].x, 0.0);                          \
    EXPECT(result, mat[0][1], 1.0);                         \
    EXPECTV(result, mat[0].xy, vec2(0, 1));                 \
    EXPECTV(result, mat[1].xy, vec2(4, 5));                 \
    for (int c = 0; c < cols; ++c)                          \
    {                                                       \
        for (int r = 0; r < rows; ++r)                      \
        {                                                   \
            EXPECT(result, mat[c][r], float(c * 4 + r));    \
        }                                                   \
    }

#define COPY(matIn, matOut, cols, rows)     \
    matOut = matOut + matIn;                \
    /* random operations for testing */     \
    matOut[0].x += matIn[0].x + matIn[1].x; \
    matOut[0].x -= matIn[1].x;              \
    matOut[0][1] += matIn[0][1];            \
    matOut[1] += matIn[1];                  \
    matOut[1].xy -= matIn[1].xy;            \
    /* undo the above to get back matIn */  \
    matOut[0].x -= matIn[0].x;              \
    matOut[0][1] -= matIn[0][1];            \
    matOut[1] -= matIn[1];                  \
    matOut[1].xy += matIn[1].xy;

bool verifyMatrices(in Matrices m)
{
    bool result = true;
    VERIFY_IN(result, m.m2c2r, 2, 2);
    VERIFY_IN(result, m.m2c3r[0], 2, 3);
    VERIFY_IN(result, m.m2c3r[1], 2, 3);
    VERIFY_IN(result, m.m3c2r, 3, 2);
    VERIFY_IN(result, m.inner.m3c4r, 3, 4);
    VERIFY_IN(result, m.inner.m4c3r, 4, 3);
    return result;
}

mat4 copyMat4(in mat4 m)
{
    return m;
}

void copyMatrices(in Matrices mIn, inout Matrices mOut)
{
    COPY(mIn.m2c2r, mOut.m2c2r, 2, 2);
    COPY(mIn.m2c3r[0], mOut.m2c3r[0], 2, 3);
    COPY(mIn.m2c3r[1], mOut.m2c3r[1], 2, 3);
    COPY(mIn.m3c2r, mOut.m3c2r, 3, 2);
    COPY(mIn.inner.m3c4r, mOut.inner.m3c4r, 3, 4);
    COPY(mIn.inner.m4c3r, mOut.inner.m4c3r, 4, 3);
}

void main()
{
    bool result = true;

    VERIFY_IN(result, ubo140cIn.m4c4r, 4, 4);
    VERIFY_IN(result, ubo140cIn.m.m2c3r[0], 2, 3);
    EXPECT(result, verifyMatrices(ubo140cIn.m), true);

    VERIFY_IN(result, ubo140rIn.m4c4r, 4, 4);
    VERIFY_IN(result, ubo140rIn.m.m2c2r, 2, 2);
    EXPECT(result, verifyMatrices(ubo140rIn.m), true);

    VERIFY_IN(result, ssbo140cIn.m4c4r, 4, 4);
    VERIFY_IN(result, ssbo140cIn.m.m3c2r, 3, 2);
    EXPECT(result, verifyMatrices(ssbo140cIn.m), true);

    VERIFY_IN(result, ssbo140rIn.m4c4r, 4, 4);
    VERIFY_IN(result, ssbo140rIn.m.inner.m4c3r, 4, 3);
    EXPECT(result, verifyMatrices(ssbo140rIn.m), true);

    VERIFY_IN(result, ssbo430cIn.m4c4r, 4, 4);
    VERIFY_IN(result, ssbo430cIn.m.m2c3r[1], 2, 3);
    EXPECT(result, verifyMatrices(ssbo430cIn.m), true);

    VERIFY_IN(result, ssbo430rIn.m4c4r, 4, 4);
    VERIFY_IN(result, ssbo430rIn.m.inner.m3c4r, 3, 4);
    EXPECT(result, verifyMatrices(ssbo430rIn.m), true);

    // Only assign to SSBO from a single invocation.
    if (gl_GlobalInvocationID.x == 0u)
    {
        ssbo140cOut.m4c4r = copyMat4(ssbo140cIn.m4c4r);
        copyMatrices(ssbo430cIn.m, ssbo140cOut.m);
        ssbo140cOut.m.m2c3r[1] = mat2x3(0);
        COPY(ssbo430cIn.m.m2c3r[1], ssbo140cOut.m.m2c3r[1], 2, 3);

        ssbo140rOut.m4c4r = copyMat4(ssbo140rIn.m4c4r);
        copyMatrices(ssbo430rIn.m, ssbo140rOut.m);
        ssbo140rOut.m.inner.m3c4r = mat3x4(0);
        COPY(ssbo430rIn.m.inner.m3c4r, ssbo140rOut.m.inner.m3c4r, 3, 4);

        ssbo430cOut.m4c4r = copyMat4(ssbo430cIn.m4c4r);
        copyMatrices(ssbo140cIn.m, ssbo430cOut.m);
        ssbo430cOut.m.m3c2r = mat3x2(0);
        COPY(ssbo430cIn.m.m3c2r, ssbo430cOut.m.m3c2r, 3, 2);

        ssbo430rOut.m4c4r = copyMat4(ssbo430rIn.m4c4r);
        copyMatrices(ssbo140rIn.m, ssbo430rOut.m);
        ssbo430rOut.m.inner.m4c3r = mat4x3(0);
        COPY(ssbo430rIn.m.inner.m4c3r, ssbo430rOut.m.inner.m4c3r, 4, 3);

        resultOut.success = uint(result);
    }
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 7;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4}, {2, 2}, {2, 3}, {2, 3}, {3, 2}, {3, 4}, {4, 3},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {
        true, false, false, false, false, false, false,
    };

    float dataStd140ColMajor[kMatrixCount * 4 * 4] = {};
    float dataStd140RowMajor[kMatrixCount * 4 * 4] = {};
    float dataStd430ColMajor[kMatrixCount * 4 * 4] = {};
    float dataStd430RowMajor[kMatrixCount * 4 * 4] = {};
    float dataZeros[kMatrixCount * 4 * 4]          = {};

    const uint32_t sizeStd140ColMajor =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, dataStd140ColMajor, false, false);
    const uint32_t sizeStd140RowMajor =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, dataStd140RowMajor, false, true);
    const uint32_t sizeStd430ColMajor =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, dataStd430ColMajor, true, false);
    const uint32_t sizeStd430RowMajor =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, dataStd430RowMajor, true, true);

    GLBuffer uboStd140ColMajor, uboStd140RowMajor;
    GLBuffer ssboStd140ColMajor, ssboStd140RowMajor;
    GLBuffer ssboStd430ColMajor, ssboStd430RowMajor;
    GLBuffer ssboStd140ColMajorOut, ssboStd140RowMajorOut;
    GLBuffer ssboStd430ColMajorOut, ssboStd430RowMajorOut;

    InitBuffer(program, "Ubo140c", uboStd140ColMajor, 0, dataStd140ColMajor, sizeStd140ColMajor,
               true);
    InitBuffer(program, "Ubo140r", uboStd140RowMajor, 1, dataStd140RowMajor, sizeStd140RowMajor,
               true);
    InitBuffer(program, "Ssbo140c", ssboStd140ColMajor, 0, dataStd140ColMajor, sizeStd140ColMajor,
               false);
    InitBuffer(program, "Ssbo140r", ssboStd140RowMajor, 1, dataStd140RowMajor, sizeStd140RowMajor,
               false);
    InitBuffer(program, "Ssbo430c", ssboStd430ColMajor, 2, dataStd430ColMajor, sizeStd430ColMajor,
               false);
    InitBuffer(program, "Ssbo430r", ssboStd430RowMajor, 3, dataStd430RowMajor, sizeStd430RowMajor,
               false);
    InitBuffer(program, "Ssbo140cOut", ssboStd140ColMajorOut, 4, dataZeros, sizeStd140ColMajor,
               false);
    InitBuffer(program, "Ssbo140rOut", ssboStd140RowMajorOut, 5, dataZeros, sizeStd140RowMajor,
               false);
    InitBuffer(program, "Ssbo430cOut", ssboStd430ColMajorOut, 6, dataZeros, sizeStd430ColMajor,
               false);
    InitBuffer(program, "Ssbo430rOut", ssboStd430RowMajorOut, 7, dataZeros, sizeStd430RowMajor,
               false);
    EXPECT_GL_NO_ERROR();

    GLBuffer outputBuffer;
    CreateOutputBuffer(&outputBuffer, 8);

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();
    EXPECT_TRUE(VerifySuccess(outputBuffer));

    EXPECT_TRUE(VerifyBuffer(ssboStd140ColMajorOut, dataStd140ColMajor, sizeStd140ColMajor));
    EXPECT_TRUE(VerifyBuffer(ssboStd140RowMajorOut, dataStd140RowMajor, sizeStd140RowMajor));
    EXPECT_TRUE(VerifyBuffer(ssboStd430ColMajorOut, dataStd430ColMajor, sizeStd430ColMajor));
    EXPECT_TRUE(VerifyBuffer(ssboStd430RowMajorOut, dataStd430RowMajor, sizeStd430RowMajor));
}

// Test that array UBOs are transformed correctly.
TEST_P(GLSLTest_ES3, RowMajorMatrix_ReadMat4Test)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;

layout(std140, row_major) uniform Ubo
{
    mat4 m1;
};

void main()
{
    outColor = m1[3] / 255.0;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 1;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {
        false,
    };

    float data[kMatrixCount * 4 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubos;

    InitBuffer(program, "Ubo", ubos, 0, data, size, true);

    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_NEAR(0, 0, 12, 13, 14, 15, 0);
}

// Test that array UBOs are transformed correctly.
TEST_P(GLSLTest_ES3, RowMajorMatrix_ReadMat2x3Test)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;

layout(std140, row_major) uniform Ubo
{
    mat2x3 m1;
};

void main()
{
    outColor = vec4(m1[1], 0) / 255.0;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 1;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {2, 3},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {
        false,
    };

    float data[kMatrixCount * 3 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubos;

    InitBuffer(program, "Ubo", ubos, 0, data, size, true);

    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_NEAR(0, 0, 4, 5, 6, 0, 0);
}

TEST_P(GLSLTest_ES3, RowMajorMatrix_ReadMat3x2Test)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;

layout(std140, row_major) uniform Ubo
{
    mat3x2 m1;
};

void main()
{
    outColor = vec4(m1[2], 0, 0) / 255.0;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 1;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {3, 2},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {
        false,
    };

    float data[kMatrixCount * 2 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubos;

    InitBuffer(program, "Ubo", ubos, 0, data, size, true);

    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_NEAR(0, 0, 8, 9, 0, 0, 0);
}

TEST_P(GLSLTest_ES3, RowMajorMatrix_NestedExpression)
{
    // Many OpenGL drivers seem to fail this
    ANGLE_SKIP_TEST_IF((IsLinux() || IsMac()) && IsOpenGL());

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

uniform Ubo {
  layout(row_major) mat4 u_mat[3];
  layout(row_major) mat4 u_ndx[3];
} stuff;

precision highp float;
out vec4 outColor;

void main() {
  outColor = stuff.u_mat[int(stuff.u_ndx[1][1][3])][2] / 255.0;
}
)";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    typedef float vec4[4];
    typedef vec4 mat4[4];

    constexpr size_t kMatrixCount = 6;
    mat4 data[]                   = {
        {
            {0, 1, 2, 3},      //
            {4, 5, 6, 7},      //
            {8, 9, 10, 11},    //
            {12, 13, 14, 15},  //
        },
        {
            //     +-- we should be looking up this column
            //     V
            {0, 4, 8, 12},   //
            {1, 5, 9, 13},   //
            {2, 6, 10, 14},  //
            {3, 7, 11, 15},  //
        },
        {
            {0, 2, 4, 6},      //
            {8, 10, 12, 14},   //
            {16, 18, 20, 22},  //
            {24, 26, 28, 30},  //
        },
        {
            {0, 0, 0, 0},  //
            {0, 0, 0, 0},  //
            {0, 0, 0, 0},  //
            {0, 0, 0, 0},  //
        },
        {
            {0, 0, 0, 0},  //
            {0, 0, 0, 2},  //
            {0, 0, 0, 0},  //
            {0, 1, 0, 0},
            //  ^
            //  +-- we should be using this element
        },
        {
            {0, 0, 0, 0},  //
            {0, 0, 0, 0},  //
            {0, 0, 0, 0},  //
            {0, 0, 0, 0},  //
        },
    };

    GLBuffer ubos;
    InitBuffer(program, "Ubo", ubos, 0, data, kMatrixCount, true);
    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_NEAR(0, 0, 8, 9, 10, 11, 0);
}

// Test that array UBOs are transformed correctly.
TEST_P(GLSLTest_ES3, MixedRowAndColumnMajorMatrices_ArrayBufferDeclaration)
{
    // Fails to compile the shader on Android: http://anglebug.com/42262483
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // http://anglebug.com/42262481
    ANGLE_SKIP_TEST_IF(IsLinux() && IsIntel() && IsOpenGL());

    // Fails on Mac on Intel and AMD: http://anglebug.com/42262487
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL() && (IsIntel() || IsAMD()));

    // Fails on windows AMD on GL: http://anglebug.com/42262482
    ANGLE_SKIP_TEST_IF(IsWindows() && IsOpenGL() && IsAMD());

    // Fails on D3D due to mistranslation: http://anglebug.com/42262486
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;

layout(std140, column_major) uniform Ubo
{
    mat4 m1;
    layout(row_major) mat4 m2;
} ubo[3];

#define EXPECT(result, expression, value) if ((expression) != value) { result = false; }

#define VERIFY_IN(result, mat, cols, rows)                  \
    for (int c = 0; c < cols; ++c)                          \
    {                                                       \
        for (int r = 0; r < rows; ++r)                      \
        {                                                   \
            EXPECT(result, mat[c][r], float(c * 4 + r));    \
        }                                                   \
    }

void main()
{
    bool result = true;

    VERIFY_IN(result, ubo[0].m1, 4, 4);
    VERIFY_IN(result, ubo[0].m2, 4, 4);

    VERIFY_IN(result, ubo[1].m1, 4, 4);
    VERIFY_IN(result, ubo[1].m2, 4, 4);

    VERIFY_IN(result, ubo[2].m1, 4, 4);
    VERIFY_IN(result, ubo[2].m2, 4, 4);

    outColor = result ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 2;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4},
        {4, 4},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {
        true,
        false,
    };

    float data[kMatrixCount * 4 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubos[3];

    InitBuffer(program, "Ubo[0]", ubos[0], 0, data, size, true);
    InitBuffer(program, "Ubo[1]", ubos[1], 0, data, size, true);
    InitBuffer(program, "Ubo[2]", ubos[2], 0, data, size, true);

    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that side effects when transforming read operations are preserved.
TEST_P(GLSLTest_ES3, MixedRowAndColumnMajorMatrices_ReadSideEffect)
{
    // Fails on Mac on Intel and AMD: http://anglebug.com/42262487
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL() && (IsIntel() || IsAMD()));

    // Fails on D3D due to mistranslation: http://anglebug.com/42262486
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;

struct S
{
    mat2x3 m2[3];
};

layout(std140, column_major) uniform Ubo
{
    mat4 m1;
    layout(row_major) S s[2];
} ubo;

#define EXPECT(result, expression, value) if ((expression) != value) { result = false; }

#define VERIFY_IN(result, mat, cols, rows)                  \
    for (int c = 0; c < cols; ++c)                          \
    {                                                       \
        for (int r = 0; r < rows; ++r)                      \
        {                                                   \
            EXPECT(result, mat[c][r], float(c * 4 + r));    \
        }                                                   \
    }

bool verify2x3(mat2x3 mat)
{
    bool result = true;

    for (int c = 0; c < 2; ++c)
    {
        for (int r = 0; r < 3; ++r)
        {
            EXPECT(result, mat[c][r], float(c * 4 + r));
        }
    }

    return result;
}

void main()
{
    bool result = true;

    int sideEffect = 0;
    VERIFY_IN(result, ubo.m1, 4, 4);
    EXPECT(result, verify2x3(ubo.s[0].m2[0]), true);
    EXPECT(result, verify2x3(ubo.s[0].m2[sideEffect += 1]), true);
    EXPECT(result, verify2x3(ubo.s[0].m2[sideEffect += 1]), true);

    EXPECT(result, sideEffect, 2);

    EXPECT(result, verify2x3(ubo.s[sideEffect = 1].m2[0]), true);
    EXPECT(result, verify2x3(ubo.s[1].m2[(sideEffect = 4) - 3]), true);
    EXPECT(result, verify2x3(ubo.s[1].m2[sideEffect - 2]), true);

    EXPECT(result, sideEffect, 4);

    outColor = result ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 7;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {
        true, false, false, false, false, false, false,
    };

    float data[kMatrixCount * 4 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubo;
    InitBuffer(program, "Ubo", ubo, 0, data, size, true);

    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that side effects respect the order of logical expression operands.
TEST_P(GLSLTest_ES3, MixedRowAndColumnMajorMatrices_ReadSideEffectOrder)
{
    // http://anglebug.com/42262481
    ANGLE_SKIP_TEST_IF(IsLinux() && IsIntel() && IsOpenGL());

    // IntermTraverser::insertStatementsInParentBlock that's used to move side effects does not
    // respect the order of evaluation of logical expressions.  http://anglebug.com/42262472.
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;

layout(std140, column_major) uniform Ubo
{
    mat4 m1;
    layout(row_major) mat4 m2[2];
} ubo;

void main()
{
    bool result = true;

    int x = 0;
    if (x == 0 && ubo.m2[x = 1][1][1] == 5.0)
    {
        result = true;
    }
    else
    {
        result = false;
    }

    outColor = result ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 3;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4},
        {4, 4},
        {4, 4},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {true, false, false};

    float data[kMatrixCount * 4 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubo;
    InitBuffer(program, "Ubo", ubo, 0, data, size, true);

    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

TEST_P(GLSLTest_ES3, MixedRowAndColumnMajorMatrices_ReadSideEffectOrderSurroundedByLoop)
{
    // http://anglebug.com/42262481
    ANGLE_SKIP_TEST_IF(IsLinux() && IsIntel() && IsOpenGL());

    // IntermTraverser::insertStatementsInParentBlock that's used to move side effects does not
    // respect the order of evaluation of logical expressions.  http://anglebug.com/42262472.
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;

layout(std140, column_major) uniform Ubo
{
    mat4 m1;
    layout(row_major) mat4 m2[2];
} ubo;

void main()
{
    bool result = false;

    for(int x = 0; x < 1; ++x)
    {
        if (x == 0 && ubo.m2[x = 1][1][1] == 5.0) {
          result = true;
        }
    }
    outColor = result ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 3;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4},
        {4, 4},
        {4, 4},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {true, false, false};

    float data[kMatrixCount * 4 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubo;
    InitBuffer(program, "Ubo", ubo, 0, data, size, true);

    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

TEST_P(GLSLTest_ES3, MixedRowAndColumnMajorMatrices_ReadSideEffectOrderInALoop)
{
    // http://anglebug.com/42262481
    ANGLE_SKIP_TEST_IF(IsLinux() && IsIntel() && IsOpenGL());

    // IntermTraverser::insertStatementsInParentBlock that's used to move side effects does not
    // respect the order of evaluation of logical expressions.  http://anglebug.com/42262472.
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;

layout(std140, column_major) uniform Ubo
{
    mat4 m1;
    layout(row_major) mat4 m2[2];
} ubo;

void main()
{
    bool result = false;

    for(int x = 0; x == 0 && ubo.m2[x = 1][1][1] == 5.0;)
    {
        result = true;
    }
    outColor = result ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 3;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4},
        {4, 4},
        {4, 4},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {true, false, false};

    float data[kMatrixCount * 4 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubo;
    InitBuffer(program, "Ubo", ubo, 0, data, size, true);

    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that side effects respect short-circuit.
TEST_P(GLSLTest_ES3, MixedRowAndColumnMajorMatrices_ReadSideEffectShortCircuit)
{
    // Fails on Android: http://anglebug.com/42262483
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // IntermTraverser::insertStatementsInParentBlock that's used to move side effects does not
    // respect the order of evaluation of logical expressions.  http://anglebug.com/42262472.
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 outColor;

layout(std140, column_major) uniform Ubo
{
    mat4 m1;
    layout(row_major) mat4 m2[2];
} ubo;

void main()
{
    bool result = true;

    int x = 0;
    if (x == 1 && ubo.m2[x = 1][1][1] == 5.0)
    {
        // First x == 1 should prevent the side effect of the second expression (x = 1) from
        // being executed.  If x = 1 is run before the if, the condition of the if would be true,
        // which is a failure.
        result = false;
    }
    if (x == 1)
    {
        result = false;
    }

    outColor = result ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 3;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4},
        {4, 4},
        {4, 4},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {true, false, false};

    float data[kMatrixCount * 4 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubo;
    InitBuffer(program, "Ubo", ubo, 0, data, size, true);

    EXPECT_GL_NO_ERROR();

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that indexing swizzles out of bounds fails
TEST_P(GLSLTest_ES3, OutOfBoundsIndexingOfSwizzle)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 colorOut;
uniform vec3 colorIn;

void main()
{
    colorOut = vec4(colorIn.yx[2], 0, 0, 1);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_EQ(0u, shader);
}

// Test that indexing l-value swizzles work
TEST_P(GLSLTest_ES3, IndexingOfSwizzledLValuesShouldWork)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 oColor;

bool do_test() {
    highp vec3 expected = vec3(3.0, 2.0, 1.0);
    highp vec3 vec;

    vec.yzx[2] = 3.0;
    vec.yzx[1] = 1.0;
    vec.yzx[0] = 2.0;

    return vec == expected;
}

void main()
{
    oColor = vec4(do_test(), 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Test that indexing r-value swizzles work
TEST_P(GLSLTest_ES3, IndexingOfSwizzledRValuesShouldWork)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 oColor;

bool do_test() {
    highp vec3 expected = vec3(3.0, 2.0, 1.0);
    highp vec3 vecA = vec3(1.0, 3.0, 2.0);
    highp vec3 vecB;

    vecB.x = vecA.zxy[2];
    vecB.y = vecA.zxy[0];
    vecB.z = vecA.zxy[1];

    return vecB == expected;
}

void main()
{
    oColor = vec4(do_test(), 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
    ASSERT_GL_NO_ERROR();
}

// Test that dynamic indexing of swizzled l-values should work.
// A simple porting of sdk/tests/conformance2/glsl3/vector-dynamic-indexing-swizzled-lvalue.html
TEST_P(GLSLTest_ES3, DynamicIndexingOfSwizzledLValuesShouldWork)
{
    // The shader first assigns v.x to v.z (1.0)
    // Then v.y to v.y (2.0)
    // Then v.z to v.x (1.0)
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 my_FragColor;
void main() {
    vec3 v = vec3(1.0, 2.0, 3.0);
    for (int i = 0; i < 3; i++) {
        v.zyx[i] = v[i];
    }
    my_FragColor = distance(v, vec3(1.0, 2.0, 1.0)) < 0.01 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Another test for dynamic indexing of swizzled l-values.
TEST_P(GLSLTest_ES3, DynamicIndexingOfSwizzledLValuesShouldWork2)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 oColor;

bool do_test() {
    highp vec3 expected = vec3(3.0, 2.0, 1.0);
    highp vec3 vec;

    for (int i = 0; i < 3; ++i)
    {
        vec.zyx[i] = float(1 + i);
    }

    return vec == expected;
}

void main()
{
    oColor = vec4(do_test(), 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that dead code after discard, return, continue and branch are pruned.
TEST_P(GLSLTest_ES3, DeadCodeIsPruned)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

vec4 f(vec4 c)
{
    return c;
    // dead code
    c = vec4(0, 0, 1, 1);
    return c;
}

void main()
{
    vec4 result = vec4(0, 0.5, 0, 1);
    int var = int(result.y * 2.2);

    {
        if (result.x > 1.0)
        {
            discard;
            // dead code
            result = vec4(1, 0, 0, 1);
        }
        for (int i = 0; i < 3; ++i)
        {
            if (i < 2)
            {
                result = f(result);
                continue;
                // dead code
                result = vec4(1, 0, 1, 1);
            }
            result = f(result);
            break;
            // dead code
            result = vec4(1, 0, 1, 0);
        }
        while (true)
        {
            if (result.x > -1.0)
            {
                {
                    result = f(result);
                    {
                        break;
                        // dead code
                        result = vec4(1, 0, 0, 0);
                    }
                    // dead code
                    for (int j = 0; j < 3; ++j)
                    {
                        if (j > 1) continue;
                        result = vec4(0, 0, 1, 0);
                        color = vec4(0.5, 0, 0.5, 0.5);
                        return;
                    }
                }
                // dead code
                result = vec4(0.5, 0, 0, 0);
            }
        }
        switch (var)
        {
        case 2:
            return;
            // dead code
            color = vec4(0.25, 0, 0.25, 0.25);
        case 1:
            {
                // Make sure this path is not pruned due to the return in the previous case.
                result.y += 0.5;
                break;
                // dead code
                color = vec4(0.25, 0, 0, 0);
            }
            // dead code
            color = vec4(0, 0, 0.25, 0);
            break;
        default:
            break;
        }

        color = result;
        return;
        // dead code
        color = vec4(0, 0, 0.5, 0);
    }
    // dead code
    color = vec4(0, 0, 0, 0.5);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Regression test based on fuzzer issue.  If a case has statements that are pruned, and those
// pruned statements in turn have branches, and another case follows, a prior implementation of
// dead-code elimination doubly pruned some statements.
TEST_P(GLSLTest_ES3, DeadCodeBranchInPrunedStatementsInCaseBeforeAnotherCase)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;
void main()
{
    color = vec4(0, 1, 0, 1);
    switch(0)
    {
    case 0:
        break;
        break;
        color = vec4(1, 0, 0, 1);   // The bug was pruning this statement twice
    default:
        color = vec4(0, 0, 1, 1);
        break;
    }
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test shader with all resources (default uniform, UBO, SSBO, image, sampler and atomic counter) to
// make sure they are all linked ok.  The front-end sorts these resources and traverses the list of
// "uniforms" to find the range for each resource.  A bug there was causing some resource ranges to
// be empty in the presence of other resources.
TEST_P(GLSLTest_ES31, MixOfAllResources)
{
    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    constexpr char kComputeShader[] = R"(#version 310 es
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 1, std430) buffer Output {
  uint ubo_value;
  uint default_value;
  uint sampler_value;
  uint ac_value;
  uint image_value;
} outbuf;
uniform Input {
  uint input_value;
} inbuf;
uniform uint default_uniform;
uniform sampler2D smplr;
layout(binding=0) uniform atomic_uint ac;
layout(r32ui) uniform highp readonly uimage2D image;

void main(void)
{
  outbuf.ubo_value = inbuf.input_value;
  outbuf.default_value = default_uniform;
  outbuf.sampler_value = uint(texture(smplr, vec2(0.5, 0.5)).x * 255.0);
  outbuf.ac_value = atomicCounterIncrement(ac);
  outbuf.image_value = imageLoad(image, ivec2(0, 0)).x;
}
)";
    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    unsigned int inputData = 89u;
    GLBuffer inputBuffer;
    glBindBuffer(GL_UNIFORM_BUFFER, inputBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(inputData), &inputData, GL_STATIC_DRAW);
    GLuint inputBufferIndex = glGetUniformBlockIndex(program, "Input");
    ASSERT_NE(inputBufferIndex, GL_INVALID_INDEX);
    glUniformBlockBinding(program, inputBufferIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, inputBuffer);

    unsigned int outputInitData[5] = {0x12345678u, 0x09ABCDEFu, 0x56789ABCu, 0x0DEF1234u,
                                      0x13579BDFu};
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outputInitData), outputInitData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
    EXPECT_GL_NO_ERROR();

    unsigned int uniformData = 456u;
    GLint uniformLocation    = glGetUniformLocation(program, "default_uniform");
    ASSERT_NE(uniformLocation, -1);
    glUniform1ui(uniformLocation, uniformData);

    unsigned int acData = 2u;
    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(acData), &acData, GL_STATIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
    EXPECT_GL_NO_ERROR();

    unsigned int imageData = 33u;
    GLTexture image;
    glBindTexture(GL_TEXTURE_2D, image);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &imageData);
    glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
    EXPECT_GL_NO_ERROR();

    GLColor textureData(127, 18, 189, 211);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // read back
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(outputInitData), GL_MAP_READ_BIT));
    EXPECT_EQ(ptr[0], inputData);
    EXPECT_EQ(ptr[1], uniformData);
    EXPECT_NEAR(ptr[2], textureData.R, 1.0);
    EXPECT_EQ(ptr[3], acData);
    EXPECT_EQ(ptr[4], imageData);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that sending mixture of resources to functions works.
TEST_P(GLSLTest_ES31, MixOfResourcesAsFunctionArgs)
{
    // http://anglebug.com/42264082
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsOpenGL());

    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    constexpr char kComputeShader[] = R"(#version 310 es
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 1, std430) buffer Output {
  uint success;
} outbuf;

uniform uint initialAcValue;
uniform sampler2D smplr[2][3];
layout(binding=0) uniform atomic_uint ac;

bool sampler1DAndAtomicCounter(uvec3 sExpect, in sampler2D s[3], in atomic_uint a, uint aExpect)
{
    uvec3 sResult = uvec3(uint(texture(s[0], vec2(0.5, 0.5)).x * 255.0),
                          uint(texture(s[1], vec2(0.5, 0.5)).x * 255.0),
                          uint(texture(s[2], vec2(0.5, 0.5)).x * 255.0));
    uint aResult = atomicCounterIncrement(a);

    return sExpect == sResult && aExpect == aResult;
}

bool sampler2DAndAtomicCounter(in sampler2D s[2][3], uint aInitial, in atomic_uint a)
{
    bool success = true;
    success = sampler1DAndAtomicCounter(uvec3(0, 127, 255), s[0], a, aInitial) && success;
    success = sampler1DAndAtomicCounter(uvec3(31, 63, 191), s[1], a, aInitial + 1u) && success;
    return success;
}

void main(void)
{
    outbuf.success = uint(sampler2DAndAtomicCounter(smplr, initialAcValue, ac));
}
)";
    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    unsigned int outputInitData = 0x12345678u;
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outputInitData), &outputInitData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
    EXPECT_GL_NO_ERROR();

    unsigned int acData   = 2u;
    GLint uniformLocation = glGetUniformLocation(program, "initialAcValue");
    ASSERT_NE(uniformLocation, -1);
    glUniform1ui(uniformLocation, acData);

    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(acData), &acData, GL_STATIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
    EXPECT_GL_NO_ERROR();

    const std::array<GLColor, 6> kTextureData = {
        GLColor(0, 0, 0, 0),  GLColor(127, 0, 0, 0), GLColor(255, 0, 0, 0),
        GLColor(31, 0, 0, 0), GLColor(63, 0, 0, 0),  GLColor(191, 0, 0, 0),
    };
    GLTexture textures[2][3];

    for (int dim1 = 0; dim1 < 2; ++dim1)
    {
        for (int dim2 = 0; dim2 < 3; ++dim2)
        {
            int textureUnit = dim1 * 3 + dim2;
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, textures[dim1][dim2]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         &kTextureData[textureUnit]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            std::stringstream uniformName;
            uniformName << "smplr[" << dim1 << "][" << dim2 << "]";
            GLint samplerLocation = glGetUniformLocation(program, uniformName.str().c_str());
            EXPECT_NE(samplerLocation, -1);
            glUniform1i(samplerLocation, textureUnit);
        }
    }
    ASSERT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // read back
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(outputInitData), GL_MAP_READ_BIT));
    EXPECT_EQ(ptr[0], 1u);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that array of array of samplers used as function parameter with an index that has a
// side-effect works.
TEST_P(GLSLTest_ES31, ArrayOfArrayOfSamplerAsFunctionParameterIndexedWithSideEffect)
{
    // http://anglebug.com/42264082
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsOpenGL());

    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // Skip if EXT_gpu_shader5 is not enabled.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_gpu_shader5"));

    constexpr char kComputeShader[] = R"(#version 310 es
#extension GL_EXT_gpu_shader5 : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 1, std430) buffer Output {
  uint success;
} outbuf;

uniform sampler2D smplr[2][3];
layout(binding=0) uniform atomic_uint ac;

bool sampler1DAndAtomicCounter(uvec3 sExpect, in sampler2D s[3], in atomic_uint a, uint aExpect)
{
    uvec3 sResult = uvec3(uint(texture(s[0], vec2(0.5, 0.5)).x * 255.0),
                          uint(texture(s[1], vec2(0.5, 0.5)).x * 255.0),
                          uint(texture(s[2], vec2(0.5, 0.5)).x * 255.0));
    uint aResult = atomicCounter(a);

    return sExpect == sResult && aExpect == aResult;
}

bool sampler2DAndAtomicCounter(in sampler2D s[2][3], uint aInitial, in atomic_uint a)
{
    bool success = true;
    success = sampler1DAndAtomicCounter(uvec3(0, 127, 255),
                    s[atomicCounterIncrement(ac)], a, aInitial + 1u) && success;
    success = sampler1DAndAtomicCounter(uvec3(31, 63, 191),
                    s[atomicCounterIncrement(ac)], a, aInitial + 2u) && success;
    return success;
}

void main(void)
{
    outbuf.success = uint(sampler2DAndAtomicCounter(smplr, 0u, ac));
}
)";
    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    unsigned int outputInitData = 0x12345678u;
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outputInitData), &outputInitData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
    EXPECT_GL_NO_ERROR();

    unsigned int acData = 0u;
    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(acData), &acData, GL_STATIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
    EXPECT_GL_NO_ERROR();

    const std::array<GLColor, 6> kTextureData = {
        GLColor(0, 0, 0, 0),  GLColor(127, 0, 0, 0), GLColor(255, 0, 0, 0),
        GLColor(31, 0, 0, 0), GLColor(63, 0, 0, 0),  GLColor(191, 0, 0, 0),
    };
    GLTexture textures[2][3];

    for (int dim1 = 0; dim1 < 2; ++dim1)
    {
        for (int dim2 = 0; dim2 < 3; ++dim2)
        {
            int textureUnit = dim1 * 3 + dim2;
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, textures[dim1][dim2]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         &kTextureData[textureUnit]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            std::stringstream uniformName;
            uniformName << "smplr[" << dim1 << "][" << dim2 << "]";
            GLint samplerLocation = glGetUniformLocation(program, uniformName.str().c_str());
            EXPECT_NE(samplerLocation, -1);
            glUniform1i(samplerLocation, textureUnit);
        }
    }
    ASSERT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // read back
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(outputInitData), GL_MAP_READ_BIT));
    EXPECT_EQ(ptr[0], 1u);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void GLSLTest_ES31::testArrayOfArrayOfSamplerDynamicIndex(const APIExtensionVersion usedExtension)
{
    ASSERT(usedExtension == APIExtensionVersion::EXT || usedExtension == APIExtensionVersion::OES);

    int maxTextureImageUnits = 0;
    glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
    ANGLE_SKIP_TEST_IF(maxTextureImageUnits < 24);

    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // http://anglebug.com/42264082
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsOpenGL());

    std::string computeShader;
    constexpr char kGLSLVersion[]  = R"(#version 310 es
)";
    constexpr char kGPUShaderEXT[] = R"(#extension GL_EXT_gpu_shader5 : require
)";
    constexpr char kGPUShaderOES[] = R"(#extension GL_OES_gpu_shader5 : require
)";

    computeShader.append(kGLSLVersion);
    if (usedExtension == APIExtensionVersion::EXT)
    {
        computeShader.append(kGPUShaderEXT);
    }
    else
    {
        computeShader.append(kGPUShaderOES);
    }

    constexpr char kComputeShaderBody[] = R"(
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 1, std430) buffer Output {
uint success;
} outbuf;

uniform sampler2D smplr[2][3][4];
layout(binding=0) uniform atomic_uint ac;

bool sampler1DAndAtomicCounter(uvec4 sExpect, in sampler2D s[4], in atomic_uint a, uint aExpect)
{
uvec4 sResult = uvec4(uint(texture(s[0], vec2(0.5, 0.5)).x * 255.0),
                      uint(texture(s[1], vec2(0.5, 0.5)).x * 255.0),
                      uint(texture(s[2], vec2(0.5, 0.5)).x * 255.0),
                      uint(texture(s[3], vec2(0.5, 0.5)).x * 255.0));
uint aResult = atomicCounter(a);

return sExpect == sResult && aExpect == aResult;
}

bool sampler3DAndAtomicCounter(in sampler2D s[2][3][4], uint aInitial, in atomic_uint a)
{
bool success = true;
// [0][0]
success = sampler1DAndAtomicCounter(uvec4(0, 8, 16, 24),
                s[atomicCounterIncrement(ac)][0], a, aInitial + 1u) && success;
// [1][0]
success = sampler1DAndAtomicCounter(uvec4(96, 104, 112, 120),
                s[atomicCounterIncrement(ac)][0], a, aInitial + 2u) && success;
// [0][1]
success = sampler1DAndAtomicCounter(uvec4(32, 40, 48, 56),
                s[0][atomicCounterIncrement(ac) - 1u], a, aInitial + 3u) && success;
// [0][2]
success = sampler1DAndAtomicCounter(uvec4(64, 72, 80, 88),
                s[0][atomicCounterIncrement(ac) - 1u], a, aInitial + 4u) && success;
// [1][1]
success = sampler1DAndAtomicCounter(uvec4(128, 136, 144, 152),
                s[1][atomicCounterIncrement(ac) - 3u], a, aInitial + 5u) && success;
// [1][2]
uint acValue = atomicCounterIncrement(ac);  // Returns 5
success = sampler1DAndAtomicCounter(uvec4(160, 168, 176, 184),
                s[acValue - 4u][atomicCounterIncrement(ac) - 4u], a, aInitial + 7u) && success;

return success;
}

void main(void)
{
outbuf.success = uint(sampler3DAndAtomicCounter(smplr, 0u, ac));
}
)";
    computeShader.append(kComputeShaderBody);

    ANGLE_GL_COMPUTE_PROGRAM(program, computeShader.c_str());
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    unsigned int outputInitData = 0x12345678u;
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outputInitData), &outputInitData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
    EXPECT_GL_NO_ERROR();

    unsigned int acData = 0u;
    GLBuffer atomicCounterBuffer;
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomicCounterBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(acData), &acData, GL_STATIC_DRAW);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomicCounterBuffer);
    EXPECT_GL_NO_ERROR();

    const std::array<GLColor, 24> kTextureData = {
        GLColor(0, 0, 0, 0),   GLColor(8, 0, 0, 0),   GLColor(16, 0, 0, 0),  GLColor(24, 0, 0, 0),
        GLColor(32, 0, 0, 0),  GLColor(40, 0, 0, 0),  GLColor(48, 0, 0, 0),  GLColor(56, 0, 0, 0),
        GLColor(64, 0, 0, 0),  GLColor(72, 0, 0, 0),  GLColor(80, 0, 0, 0),  GLColor(88, 0, 0, 0),
        GLColor(96, 0, 0, 0),  GLColor(104, 0, 0, 0), GLColor(112, 0, 0, 0), GLColor(120, 0, 0, 0),
        GLColor(128, 0, 0, 0), GLColor(136, 0, 0, 0), GLColor(144, 0, 0, 0), GLColor(152, 0, 0, 0),
        GLColor(160, 0, 0, 0), GLColor(168, 0, 0, 0), GLColor(176, 0, 0, 0), GLColor(184, 0, 0, 0),
    };
    GLTexture textures[2][3][4];

    for (int dim1 = 0; dim1 < 2; ++dim1)
    {
        for (int dim2 = 0; dim2 < 3; ++dim2)
        {
            for (int dim3 = 0; dim3 < 4; ++dim3)
            {
                int textureUnit = (dim1 * 3 + dim2) * 4 + dim3;
                glActiveTexture(GL_TEXTURE0 + textureUnit);
                glBindTexture(GL_TEXTURE_2D, textures[dim1][dim2][dim3]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             &kTextureData[textureUnit]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                std::stringstream uniformName;
                uniformName << "smplr[" << dim1 << "][" << dim2 << "][" << dim3 << "]";
                GLint samplerLocation = glGetUniformLocation(program, uniformName.str().c_str());
                EXPECT_NE(samplerLocation, -1);
                glUniform1i(samplerLocation, textureUnit);
            }
        }
    }
    ASSERT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // read back
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(outputInitData), GL_MAP_READ_BIT));
    EXPECT_EQ(ptr[0], 1u);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that array of array of samplers can be indexed correctly with dynamic indices.
TEST_P(GLSLTest_ES31, ArrayOfArrayOfSamplerDynamicIndexEXT)
{
    // Skip if EXT_gpu_shader5 is not enabled.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_gpu_shader5"));
    testArrayOfArrayOfSamplerDynamicIndex(APIExtensionVersion::EXT);
}

// Test that array of array of samplers can be indexed correctly with dynamic indices.
TEST_P(GLSLTest_ES31, ArrayOfArrayOfSamplerDynamicIndexOES)
{
    // Skip if OES_gpu_shader5 is not enabled.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_gpu_shader5"));
    testArrayOfArrayOfSamplerDynamicIndex(APIExtensionVersion::OES);
}

// Test that array of array of samplers is handled correctly with the comma operator.
TEST_P(GLSLTest, ArrayOfArrayOfSamplerVsComma)
{
    int maxTextureImageUnits = 0;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);

    ANGLE_SKIP_TEST_IF(maxTextureImageUnits < 6);

    constexpr char kVS[] = R"(uniform struct {
  sampler2D s1, s2[3];
} s[2];

void main()
{
    ++gl_Position, s[1].s1;
})";
    ANGLE_GL_PROGRAM(program, kVS, essl1_shaders::fs::Red());
    EXPECT_GL_NO_ERROR();
}

// Test that array of array of samplers can be indexed correctly with dynamic indices.  Uses
// samplers in structs.
TEST_P(GLSLTest_ES31, ArrayOfArrayOfSamplerInStructDynamicIndex)
{
    // Skip if EXT_gpu_shader5 is not enabled.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_gpu_shader5"));

    int maxTextureImageUnits = 0;
    glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
    ANGLE_SKIP_TEST_IF(maxTextureImageUnits < 24);

    // http://anglebug.com/42263641
    ANGLE_SKIP_TEST_IF(IsIntel() && IsLinux() && IsOpenGL());

    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    // http://anglebug.com/42264082
    ANGLE_SKIP_TEST_IF(IsWindows() && IsIntel() && IsOpenGL());

    constexpr char kComputeShader[] = R"(#version 310 es
#extension GL_EXT_gpu_shader5 : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 1, std430) buffer Output {
  uint success;
} outbuf;

struct I
{
    uint index;
};

struct S
{
    sampler2D smplr[4];
    I nested;
};

struct T
{
    S nested[3];
    uint tIndex;
};

uniform T u[2];

uint getValue(in sampler2D s)
{
    return uint(texture(s, vec2(0.5, 0.5)).x * 255.0);
}

bool sampler1DTest(uvec4 sExpect, in sampler2D s[4])
{
    uvec4 sResult = uvec4(getValue(s[0]), getValue(s[1]),
                          getValue(s[2]), getValue(s[3]));

    return sExpect == sResult;
}

bool samplerTest(T t, uint N)
{
    // u[N].tIndex == 0 + N*4
    // u[N].nested[0].nested.index == 1 + N*4
    // u[N].nested[1].nested.index == 2 + N*4
    // u[N].nested[2].nested.index == 3 + N*4

    uvec4 colorOffset = N * 3u * 4u * uvec4(8);

    bool success = true;
    // [N][0]
    success = sampler1DTest(uvec4(0, 8, 16, 24) + colorOffset,
                    t.nested[t.nested[0].nested.index - t.tIndex - 1u].smplr) && success;
    // [N][1]
    success = sampler1DTest(uvec4(32, 40, 48, 56) + colorOffset,
                    t.nested[t.nested[1].nested.index - t.tIndex - 1u].smplr) && success;
    // [N][2]
    success = sampler1DTest(uvec4(64, 72, 80, 88) + colorOffset,
                    t.nested[t.nested[2].nested.index - t.tIndex - 1u].smplr) && success;

    return success;
}

bool uniformTest(T t, uint N)
{
    // Also verify that expressions that involve structs-with-samplers are correct when not
    // referecing the sampler.

    bool success = true;
    success = (t.nested[0].nested.index - t.tIndex == 1u) && success;
    success = (t.nested[1].nested.index - t.tIndex == 2u) && success;
    success = (t.nested[2].nested.index - t.tIndex == 3u) && success;

    success = (t.nested[t.nested[0].nested.index - t.tIndex - 1u].nested.index - t.tIndex == 1u)
                && success;
    success = (t.nested[t.nested[0].nested.index - t.tIndex     ].nested.index - t.tIndex == 2u)
                && success;
    success = (t.nested[t.nested[0].nested.index - t.tIndex + 1u].nested.index - t.tIndex == 3u)
                && success;

    success = (t.nested[
                          t.nested[
                                     t.nested[2].nested.index - t.tIndex - 1u  // 2
                                  ].nested.index - t.tIndex - 2u               // 1
                       ].nested.index - t.tIndex                               // 2
                == 2u) && success;

    return success;
}

void main(void)
{
    bool success = samplerTest(u[0], 0u) && samplerTest(u[1], 1u)
                    && uniformTest(u[0], 0u) && uniformTest(u[1], 1u);
    outbuf.success = uint(success);
}
)";
    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    unsigned int outputInitData = 0x12345678u;
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outputInitData), &outputInitData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
    EXPECT_GL_NO_ERROR();

    const std::array<GLColor, 24> kTextureData = {
        GLColor(0, 0, 0, 0),   GLColor(8, 0, 0, 0),   GLColor(16, 0, 0, 0),  GLColor(24, 0, 0, 0),
        GLColor(32, 0, 0, 0),  GLColor(40, 0, 0, 0),  GLColor(48, 0, 0, 0),  GLColor(56, 0, 0, 0),
        GLColor(64, 0, 0, 0),  GLColor(72, 0, 0, 0),  GLColor(80, 0, 0, 0),  GLColor(88, 0, 0, 0),
        GLColor(96, 0, 0, 0),  GLColor(104, 0, 0, 0), GLColor(112, 0, 0, 0), GLColor(120, 0, 0, 0),
        GLColor(128, 0, 0, 0), GLColor(136, 0, 0, 0), GLColor(144, 0, 0, 0), GLColor(152, 0, 0, 0),
        GLColor(160, 0, 0, 0), GLColor(168, 0, 0, 0), GLColor(176, 0, 0, 0), GLColor(184, 0, 0, 0),
    };
    GLTexture textures[2][3][4];

    for (int dim1 = 0; dim1 < 2; ++dim1)
    {
        for (int dim2 = 0; dim2 < 3; ++dim2)
        {
            for (int dim3 = 0; dim3 < 4; ++dim3)
            {
                int textureUnit = (dim1 * 3 + dim2) * 4 + dim3;
                glActiveTexture(GL_TEXTURE0 + textureUnit);
                glBindTexture(GL_TEXTURE_2D, textures[dim1][dim2][dim3]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                             &kTextureData[textureUnit]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                std::stringstream uniformName;
                uniformName << "u[" << dim1 << "].nested[" << dim2 << "].smplr[" << dim3 << "]";
                GLint samplerLocation = glGetUniformLocation(program, uniformName.str().c_str());
                EXPECT_NE(samplerLocation, -1);
                glUniform1i(samplerLocation, textureUnit);
            }

            std::stringstream uniformName;
            uniformName << "u[" << dim1 << "].nested[" << dim2 << "].nested.index";
            GLint nestedIndexLocation = glGetUniformLocation(program, uniformName.str().c_str());
            EXPECT_NE(nestedIndexLocation, -1);
            glUniform1ui(nestedIndexLocation, dim1 * 4 + dim2 + 1);
        }

        std::stringstream uniformName;
        uniformName << "u[" << dim1 << "].tIndex";
        GLint indexLocation = glGetUniformLocation(program, uniformName.str().c_str());
        EXPECT_NE(indexLocation, -1);
        glUniform1ui(indexLocation, dim1 * 4);
    }
    ASSERT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // read back
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(outputInitData), GL_MAP_READ_BIT));
    EXPECT_EQ(ptr[0], 1u);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that array of array of samplers work when indexed with an expression that's derived from an
// array of array of samplers.
TEST_P(GLSLTest_ES31, ArrayOfArrayOfSamplerIndexedWithArrayOfArrayOfSamplers)
{
    // Skip if EXT_gpu_shader5 is not enabled.
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_gpu_shader5"));

    // anglebug.com/42262476 - no sampler array params on Android
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGLES());

    constexpr char kComputeShader[] = R"(#version 310 es
#extension GL_EXT_gpu_shader5 : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(binding = 1, std430) buffer Output {
  uint success;
} outbuf;

uniform sampler2D smplr[2][3];

uint getValue(in sampler2D s)
{
    return uint(texture(s, vec2(0.5, 0.5)).x * 255.0);
}

bool runTest(in sampler2D s[2][3])
{
    // s[0][0] should contain 2
    // s[0][1] should contain 0
    // s[0][2] should contain 1
    // s[1][0] should contain 1
    // s[1][1] should contain 2
    // s[1][2] should contain 0

    uint result = getValue(
                       s[
                           getValue(
                                s[
                                    getValue(s[0][1])   // 0
                                ][
                                    getValue(s[0][0])   // 2
                                ]
                           )                      // s[0][2] -> 1
                       ][
                           getValue(
                                s[
                                    getValue(s[1][0])   // 1
                                ][
                                    getValue(s[1][1])   // 2
                                ]
                           )                      // s[1][2] -> 0
                       ]
                  );                      // s[1][0] -> 1

    return result == 1u;
}

void main(void)
{
    outbuf.success = uint(runTest(smplr));
}
)";
    ANGLE_GL_COMPUTE_PROGRAM(program, kComputeShader);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);

    unsigned int outputInitData = 0x12345678u;
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(outputInitData), &outputInitData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
    EXPECT_GL_NO_ERROR();

    const std::array<GLColor, 6> kTextureData = {
        GLColor(2, 0, 0, 0), GLColor(0, 0, 0, 0), GLColor(1, 0, 0, 0),
        GLColor(1, 0, 0, 0), GLColor(2, 0, 0, 0), GLColor(0, 0, 0, 0),
    };
    GLTexture textures[2][3];

    for (int dim1 = 0; dim1 < 2; ++dim1)
    {
        for (int dim2 = 0; dim2 < 3; ++dim2)
        {
            int textureUnit = dim1 * 3 + dim2;
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(GL_TEXTURE_2D, textures[dim1][dim2]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         &kTextureData[textureUnit]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            std::stringstream uniformName;
            uniformName << "smplr[" << dim1 << "][" << dim2 << "]";
            GLint samplerLocation = glGetUniformLocation(program, uniformName.str().c_str());
            EXPECT_NE(samplerLocation, -1);
            glUniform1i(samplerLocation, textureUnit);
        }
    }
    ASSERT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    // read back
    const GLuint *ptr = reinterpret_cast<const GLuint *>(
        glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(outputInitData), GL_MAP_READ_BIT));
    EXPECT_EQ(ptr[0], 1u);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

// Test that multiple nested assignments are handled correctly.
TEST_P(GLSLTest_ES31, MixedRowAndColumnMajorMatrices_WriteSideEffect)
{
    // http://anglebug.com/42262475
    ANGLE_SKIP_TEST_IF(IsNVIDIA() && IsOpenGL());

    // Fails on windows AMD on GL: http://anglebug.com/42262482
    ANGLE_SKIP_TEST_IF(IsWindows() && IsOpenGL() && IsAMD());
    // http://anglebug.com/42263924
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    // Fails on D3D due to mistranslation: http://anglebug.com/42262486
    ANGLE_SKIP_TEST_IF(IsD3D11());

    constexpr char kCS[] = R"(#version 310 es
precision highp float;
layout(local_size_x=1) in;

layout(std140, column_major) uniform Ubo
{
    mat4 m1;
    layout(row_major) mat4 m2;
} ubo;

layout(std140, row_major, binding = 0) buffer Ssbo
{
    layout(column_major) mat4 m1;
    mat4 m2;
} ssbo;

layout(std140, binding = 1) buffer Result
{
    uint success;
} resultOut;

void main()
{
    bool result = true;

    // Only assign to SSBO from a single invocation.
    if (gl_GlobalInvocationID.x == 0u)
    {
        if ((ssbo.m2 = ssbo.m1 = ubo.m1) != ubo.m2)
        {
            result = false;
        }

        resultOut.success = uint(result);
    }
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 2;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4},
        {4, 4},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {
        true,
        false,
    };

    float data[kMatrixCount * 4 * 4]  = {};
    float zeros[kMatrixCount * 4 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubo, ssbo;

    InitBuffer(program, "Ubo", ubo, 0, data, size, true);
    InitBuffer(program, "Ssbo", ssbo, 0, zeros, size, false);
    EXPECT_GL_NO_ERROR();

    GLBuffer outputBuffer;
    CreateOutputBuffer(&outputBuffer, 1);

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();
    EXPECT_TRUE(VerifySuccess(outputBuffer));

    EXPECT_TRUE(VerifyBuffer(ssbo, data, size));
}

// Test that assignments to array of array of matrices are handled correctly.
TEST_P(GLSLTest_ES31, MixedRowAndColumnMajorMatrices_WriteArrayOfArray)
{
    // Fails on windows AMD on GL: http://anglebug.com/42262482
    ANGLE_SKIP_TEST_IF(IsWindows() && IsOpenGL() && IsAMD());
    // http://anglebug.com/42263924
    ANGLE_SKIP_TEST_IF(IsLinux() && IsAMD() && IsDesktopOpenGL());

    // Fails on D3D due to mistranslation: http://anglebug.com/42262486
    ANGLE_SKIP_TEST_IF(IsD3D11());

    // Fails compiling shader on Android/Vulkan.  http://anglebug.com/42262919
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsVulkan());

    // Fails on ARM on Vulkan.  http://anglebug.com/42263107
    ANGLE_SKIP_TEST_IF(IsARM() && IsVulkan());

    constexpr char kCS[] = R"(#version 310 es
precision highp float;
layout(local_size_x=1) in;

layout(std140, column_major) uniform Ubo
{
    mat4 m1;
    layout(row_major) mat4 m2[2][3];
} ubo;

layout(std140, row_major, binding = 0) buffer Ssbo
{
    layout(column_major) mat4 m1;
    mat4 m2[2][3];
} ssbo;

layout(std140, binding = 1) buffer Result
{
    uint success;
} resultOut;

void main()
{
    bool result = true;

    // Only assign to SSBO from a single invocation.
    if (gl_GlobalInvocationID.x == 0u)
    {
        ssbo.m1 = ubo.m1;
        ssbo.m2 = ubo.m2;

        resultOut.success = uint(result);
    }
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    constexpr size_t kMatrixCount                                     = 7;
    constexpr std::pair<uint32_t, uint32_t> kMatrixDims[kMatrixCount] = {
        {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},
    };
    constexpr bool kMatrixIsColMajor[kMatrixCount] = {
        true, false, false, false, false, false, false,
    };

    float data[kMatrixCount * 4 * 4]  = {};
    float zeros[kMatrixCount * 4 * 4] = {};

    const uint32_t size =
        FillBuffer(kMatrixDims, kMatrixIsColMajor, kMatrixCount, data, false, false);

    GLBuffer ubo, ssbo;

    InitBuffer(program, "Ubo", ubo, 0, data, size, true);
    InitBuffer(program, "Ssbo", ssbo, 0, zeros, size, false);
    EXPECT_GL_NO_ERROR();

    GLBuffer outputBuffer;
    CreateOutputBuffer(&outputBuffer, 1);

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();
    EXPECT_TRUE(VerifySuccess(outputBuffer));

    EXPECT_TRUE(VerifyBuffer(ssbo, data, size));
}

// Verify that types used differently (in different block storages, differently qualified etc) work
// when copied around.
TEST_P(GLSLTest_ES31, TypesUsedInDifferentBlockStorages)
{
    constexpr char kCS[] = R"(#version 310 es
precision highp float;
layout(local_size_x=1) in;

struct Inner
{
    mat3x2 m;
    float f[3];
    uvec2 u[2][4];
    ivec3 i;
    mat2x3 m2[3][2];
};

struct Outer
{
    Inner i[2];
};

layout(std140, column_major) uniform Ubo140c
{
    mat2 m;
    layout(row_major) Outer o;
} ubo140cIn;

layout(std430, row_major, binding = 0) buffer Ubo430r
{
    mat2 m;
    layout(column_major) Outer o;
} ubo430rIn;

layout(std140, column_major, binding = 1) buffer Ssbo140c
{
    layout(row_major) mat2 m[2];
    Outer o;
    layout(row_major) Inner i;
} ssbo140cOut;

layout(std430, row_major, binding = 2) buffer Ssbo430r
{
    layout(column_major) mat2 m[2];
    Outer o;
    layout(column_major) Inner i;
} ssbo430rOut;

void writeArgToStd140(uvec2 u[2][4], int innerIndex)
{
    ssbo140cOut.o.i[innerIndex].u = u;
}

void writeBlockArgToStd140(Inner i, int innerIndex)
{
    ssbo140cOut.o.i[innerIndex] = i;
}

mat2x3[3][2] readFromStd140(int innerIndex)
{
    return ubo140cIn.o.i[0].m2;
}

Inner readBlockFromStd430(int innerIndex)
{
    return ubo430rIn.o.i[innerIndex];
}

void copyFromStd140(out Inner i)
{
    i = ubo140cIn.o.i[1];
}

void main(){
    // Directly copy from one layout to another.
    ssbo140cOut.m[0] = ubo140cIn.m;
    ssbo140cOut.m[1] = ubo430rIn.m;
    ssbo140cOut.o.i[0].m = ubo140cIn.o.i[0].m;
    ssbo140cOut.o.i[0].f = ubo140cIn.o.i[0].f;
    ssbo140cOut.o.i[0].i = ubo140cIn.o.i[0].i;

    // Read from block and pass to function.
    writeArgToStd140(ubo140cIn.o.i[0].u, 0);
    writeBlockArgToStd140(ubo430rIn.o.i[0], 1);

    // Have function return value read from block.
    ssbo140cOut.o.i[0].m2 = readFromStd140(0);

    // Have function fill in value as out parameter.
    copyFromStd140(ssbo140cOut.i);

    // Initialize local variable.
    mat2 mStd140 = ubo140cIn.m;

    // Copy to variable, through multiple assignments.
    mat2 mStd430, temp;
    mStd430 = temp = ubo430rIn.m;

    // Copy from local variable
    ssbo430rOut.m[0] = mStd140;
    ssbo430rOut.m[1] = mStd430;

    // Construct from struct.
    Inner iStd140 = ubo140cIn.o.i[1];
    Outer oStd140 = Outer(Inner[2](iStd140, ubo430rIn.o.i[1]));

    // Copy struct from local variable.
    ssbo430rOut.o = oStd140;

    // Construct from arrays
    Inner iStd430 = Inner(ubo430rIn.o.i[1].m,
                          ubo430rIn.o.i[1].f,
                          ubo430rIn.o.i[1].u,
                          ubo430rIn.o.i[1].i,
                          ubo430rIn.o.i[1].m2);
    ssbo430rOut.i = iStd430;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    // Test data, laid out with padding (0) based on std140/std430 rules.
    // clang-format off
    const std::vector<float> ubo140cData = {
        // m (mat2, column-major)
        1, 2, 0, 0,     3, 4, 0, 0,

        // o.i[0].m (mat3x2, row-major)
        5, 7, 9, 0,     6, 8, 10, 0,
        // o.i[0].f (float[3])
        12, 0, 0, 0,    13, 0, 0, 0,    14, 0, 0, 0,
        // o.i[0].u (uvec2[2][4])
        15, 16, 0, 0,   17, 18, 0, 0,   19, 20, 0, 0,   21, 22, 0, 0,
        23, 24, 0, 0,   25, 26, 0, 0,   27, 28, 0, 0,   29, 30, 0, 0,
        // o.i[0].i (ivec3)
        31, 32, 33, 0,
        // o.i[0].m2 (mat2x3[3][2], row-major)
        34, 37, 0, 0,   35, 38, 0, 0,   36, 39, 0, 0,
        40, 43, 0, 0,   41, 44, 0, 0,   42, 45, 0, 0,
        46, 49, 0, 0,   47, 50, 0, 0,   48, 51, 0, 0,
        52, 55, 0, 0,   53, 56, 0, 0,   54, 57, 0, 0,
        58, 61, 0, 0,   59, 62, 0, 0,   60, 63, 0, 0,
        64, 67, 0, 0,   65, 68, 0, 0,   66, 69, 0, 0,

        // o.i[1].m (mat3x2, row-major)
        70, 72, 74, 0,     71, 73, 75, 0,
        // o.i[1].f (float[3])
        77, 0, 0, 0,    78, 0, 0, 0,    79, 0, 0, 0,
        // o.i[1].u (uvec2[2][4])
        80, 81, 0, 0,   82, 83, 0, 0,   84, 85, 0, 0,   86, 87, 0, 0,
        88, 89, 0, 0,   90, 91, 0, 0,   92, 93, 0, 0,   94, 95, 0, 0,
        // o.i[1].i (ivec3)
        96, 97, 98, 0,
        // o.i[1].m2 (mat2x3[3][2], row-major)
         99, 102, 0, 0,  100, 103, 0, 0,   101, 104, 0, 0,
        105, 108, 0, 0,  106, 109, 0, 0,   107, 110, 0, 0,
        111, 114, 0, 0,  112, 115, 0, 0,   113, 116, 0, 0,
        117, 120, 0, 0,  118, 121, 0, 0,   119, 122, 0, 0,
        123, 126, 0, 0,  124, 127, 0, 0,   125, 128, 0, 0,
        129, 132, 0, 0,  130, 133, 0, 0,   131, 134, 0, 0,
    };
    const std::vector<float> ubo430rData = {
        // m (mat2, row-major)
        135, 137,         136, 138,

        // o.i[0].m (mat3x2, column-major)
        139, 140,         141, 142,         143, 144,
        // o.i[0].f (float[3])
        146, 147, 148, 0,
        // o.i[0].u (uvec2[2][4])
        149, 150,         151, 152,         153, 154,         155, 156,
        157, 158,         159, 160,         161, 162,         163, 164, 0, 0,
        // o.i[0].i (ivec3)
        165, 166, 167, 0,
        // o.i[0].m2 (mat2x3[3][2], column-major)
        168, 169, 170, 0,   171, 172, 173, 0,
        174, 175, 176, 0,   177, 178, 179, 0,
        180, 181, 182, 0,   183, 184, 185, 0,
        186, 187, 188, 0,   189, 190, 191, 0,
        192, 193, 194, 0,   195, 196, 197, 0,
        198, 199, 200, 0,   201, 202, 203, 0,

        // o.i[1].m (mat3x2, column-major)
        204, 205,         206, 207,         208, 209,
        // o.i[1].f (float[3])
        211, 212, 213, 0,
        // o.i[1].u (uvec2[2][4])
        214, 215,         216, 217,         218, 219,         220, 221,
        222, 223,         224, 225,         226, 227,         228, 229, 0, 0,
        // o.i[1].i (ivec3)
        230, 231, 232, 0,
        // o.i[1].m2 (mat2x3[3][2], column-major)
        233, 234, 235, 0,   236, 237, 238, 0,
        239, 240, 241, 0,   242, 243, 244, 0,
        245, 246, 247, 0,   248, 249, 250, 0,
        251, 252, 253, 0,   254, 255, 256, 0,
        257, 258, 259, 0,   260, 261, 262, 0,
        263, 264, 265, 0,   266, 267, 268, 0,
    };
    const std::vector<float> ssbo140cExpect = {
        // m (mat2[2], row-major), m[0] copied from ubo140cIn.m, m[1] from ubo430rIn.m
        1, 3, 0, 0,     2, 4, 0, 0,
        135, 137, 0, 0, 136, 138, 0, 0,

        // o.i[0].m (mat3x2, column-major), copied from ubo140cIn.o.i[0].m
        5, 6, 0, 0,     7, 8, 0, 0,     9, 10, 0, 0,
        // o.i[0].f (float[3]), copied from ubo140cIn.o.i[0].f
        12, 0, 0, 0,    13, 0, 0, 0,    14, 0, 0, 0,
        // o.i[0].u (uvec2[2][4]), copied from ubo140cIn.o.i[0].u
        15, 16, 0, 0,   17, 18, 0, 0,   19, 20, 0, 0,   21, 22, 0, 0,
        23, 24, 0, 0,   25, 26, 0, 0,   27, 28, 0, 0,   29, 30, 0, 0,
        // o.i[0].i (ivec3), copied from ubo140cIn.o.i[0].i
        31, 32, 33, 0,
        // o.i[0].m2 (mat2x3[3][2], column-major), copied from ubo140cIn.o.i[0].m2
        34, 35, 36, 0,  37, 38, 39, 0,
        40, 41, 42, 0,  43, 44, 45, 0,
        46, 47, 48, 0,  49, 50, 51, 0,
        52, 53, 54, 0,  55, 56, 57, 0,
        58, 59, 60, 0,  61, 62, 63, 0,
        64, 65, 66, 0,  67, 68, 69, 0,

        // o.i[1].m (mat3x2, column-major), copied from ubo430rIn.o.i[0].m
        139, 140, 0, 0,   141, 142, 0, 0,   143, 144, 0, 0,
        // o.i[1].f (float[3]), copied from ubo430rIn.o.i[0].f
        146, 0, 0, 0,     147, 0, 0, 0,     148, 0, 0, 0,
        // o.i[1].u (uvec2[2][4]), copied from ubo430rIn.o.i[0].u
        149, 150, 0, 0,   151, 152, 0, 0,   153, 154, 0, 0,   155, 156, 0, 0,
        157, 158, 0, 0,   159, 160, 0, 0,   161, 162, 0, 0,   163, 164, 0, 0,
        // o.i[1].i (ivec3), copied from ubo430rIn.o.i[0].i
        165, 166, 167, 0,
        // o.i[1].m2 (mat2x3[3][2], column-major), copied from ubo430rIn.o.i[0].m2
        168, 169, 170, 0,   171, 172, 173, 0,
        174, 175, 176, 0,   177, 178, 179, 0,
        180, 181, 182, 0,   183, 184, 185, 0,
        186, 187, 188, 0,   189, 190, 191, 0,
        192, 193, 194, 0,   195, 196, 197, 0,
        198, 199, 200, 0,   201, 202, 203, 0,

        // i.m (mat3x2, row-major), copied from ubo140cIn.o.i[1].m
        70, 72, 74, 0,     71, 73, 75, 0,
        // i.f (float[3]), copied from ubo140cIn.o.i[1].f
        77, 0, 0, 0,    78, 0, 0, 0,    79, 0, 0, 0,
        // i.u (uvec2[2][4]), copied from ubo430rIn.o.i[1].u
        80, 81, 0, 0,   82, 83, 0, 0,   84, 85, 0, 0,   86, 87, 0, 0,
        88, 89, 0, 0,   90, 91, 0, 0,   92, 93, 0, 0,   94, 95, 0, 0,
        // i.i (ivec3), copied from ubo140cIn.o.i[1].i
        96, 97, 98, 0,
        // i.m2 (mat2x3[3][2], row-major), copied from ubo140cIn.o.i[1].m2
         99, 102, 0, 0,  100, 103, 0, 0,   101, 104, 0, 0,
        105, 108, 0, 0,  106, 109, 0, 0,   107, 110, 0, 0,
        111, 114, 0, 0,  112, 115, 0, 0,   113, 116, 0, 0,
        117, 120, 0, 0,  118, 121, 0, 0,   119, 122, 0, 0,
        123, 126, 0, 0,  124, 127, 0, 0,   125, 128, 0, 0,
        129, 132, 0, 0,  130, 133, 0, 0,   131, 134, 0, 0,
    };
    const std::vector<float> ssbo430rExpect = {
        // m (mat2[2], column-major), m[0] copied from ubo140cIn.m, m[1] from ubo430rIn.m
        1, 2,           3, 4,
        135, 136,       137, 138,

        // o.i[0].m (mat3x2, row-major), copied from ubo140cIn.o.i[1].m
        70, 72, 74, 0,  71, 73, 75, 0,
        // o.i[0].f (float[3]), copied from ubo140cIn.o.i[1].f
        77, 78, 79, 0,
        // o.i[0].u (uvec2[2][4]), copied from ubo140cIn.o.i[1].u
        80, 81,         82, 83,         84, 85,         86, 87,
        88, 89,         90, 91,         92, 93,         94, 95,
        // o.i[0].i (ivec3), copied from ubo140cIn.o.i[1].i
        96, 97, 98, 0,
        // o.i[0].m2 (mat2x3[3][2], row-major), copied from ubo140cIn.o.i[1].m2
         99, 102,        100, 103,         101, 104,
        105, 108,        106, 109,         107, 110,
        111, 114,        112, 115,         113, 116,
        117, 120,        118, 121,         119, 122,
        123, 126,        124, 127,         125, 128,
        129, 132,        130, 133,         131, 134,

        // o.i[1].m (mat3x2, row-major), copied from ubo430rIn.o.i[1].m
        204, 206, 208, 0,  205, 207, 209, 0,
        // o.i[1].f (float[3]), copied from ubo430rIn.o.i[1].f
        211, 212, 213, 0,
        // o.i[1].u (uvec2[2][4]), copied from ubo430rIn.o.i[1].u
        214, 215,         216, 217,         218, 219,         220, 221,
        222, 223,         224, 225,         226, 227,         228, 229,
        // o.i[1].i (ivec3), copied from ubo430rIn.o.i[1].i
        230, 231, 232, 0,
        // o.i[1].m2 (mat2x3[3][2], row-major), copied from ubo430rIn.o.i[1].m2
        233, 236,         234, 237,         235, 238,
        239, 242,         240, 243,         241, 244,
        245, 248,         246, 249,         247, 250,
        251, 254,         252, 255,         253, 256,
        257, 260,         258, 261,         259, 262,
        263, 266,         264, 267,         265, 268,

        // i.m (mat3x2, column-major), copied from ubo430rIn.o.i[1].m
        204, 205,          206, 207,         208, 209,
        // i.f (float[3]), copied from ubo430rIn.o.i[1].f
        211, 212, 213, 0,
        // i.u (uvec2[2][4]), copied from ubo430rIn.o.i[1].u
        214, 215,         216, 217,         218, 219,         220, 221,
        222, 223,         224, 225,         226, 227,         228, 229, 0, 0,
        // i.i (ivec3), copied from ubo430rIn.o.i[1].i
        230, 231, 232, 0,
        // i.m2 (mat2x3[3][2], column-major), copied from ubo430rIn.o.i[1].m2
        233, 234, 235, 0,   236, 237, 238, 0,
        239, 240, 241, 0,   242, 243, 244, 0,
        245, 246, 247, 0,   248, 249, 250, 0,
        251, 252, 253, 0,   254, 255, 256, 0,
        257, 258, 259, 0,   260, 261, 262, 0,
        263, 264, 265, 0,   266, 267, 268, 0,
    };
    const std::vector<float> zeros(std::max(ssbo140cExpect.size(), ssbo430rExpect.size()), 0);
    // clang-format on

    GLBuffer uboStd140ColMajor, uboStd430RowMajor;
    GLBuffer ssboStd140ColMajor, ssboStd430RowMajor;

    InitBuffer(program, "Ubo140c", uboStd140ColMajor, 0, ubo140cData.data(),
               static_cast<uint32_t>(ubo140cData.size()), true);
    InitBuffer(program, "Ubo430r", uboStd430RowMajor, 0, ubo430rData.data(),
               static_cast<uint32_t>(ubo430rData.size()), false);
    InitBuffer(program, "Ssbo140c", ssboStd140ColMajor, 1, zeros.data(),
               static_cast<uint32_t>(ssbo140cExpect.size()), false);
    InitBuffer(program, "Ssbo430r", ssboStd430RowMajor, 2, zeros.data(),
               static_cast<uint32_t>(ssbo430rExpect.size()), false);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(VerifyBuffer(ssboStd140ColMajor, ssbo140cExpect.data(),
                             static_cast<uint32_t>(ssbo140cExpect.size())));
    EXPECT_TRUE(VerifyBuffer(ssboStd430RowMajor, ssbo430rExpect.data(),
                             static_cast<uint32_t>(ssbo430rExpect.size())));
}

// Verify that bool in interface blocks work.
TEST_P(GLSLTest_ES31, BoolInInterfaceBlocks)
{
    constexpr char kCS[] = R"(#version 310 es
precision highp float;
layout(local_size_x=1) in;

struct Inner
{
    bool b;
    bvec2 b2;
    bvec3 b3;
    bvec4 b4;
    bool ba[5];
    bvec2 b2a[2][3];
};

struct Outer
{
    Inner i[2];
};

layout(std140) uniform Ubo140
{
    Outer o;
};

layout(std430, binding = 0) buffer Ubo430
{
    Outer o;
} ubo430In;

layout(std140, binding = 1) buffer Ssbo140
{
    bool valid;
    Inner i;
} ssbo140Out;

layout(std430, binding = 2) buffer Ssbo430
{
    bool valid;
    Inner i;
};

void writeArgToStd430(bool ba[5])
{
    i.ba = ba;
}

bool[5] readFromStd430(uint innerIndex)
{
    return ubo430In.o.i[innerIndex].ba;
}

void copyFromStd430(out bvec2 b2a[2][3])
{
    b2a = ubo430In.o.i[0].b2a;
}

bool destroyContent(inout Inner iOut)
{
    iOut.b = true;
    iOut.b2 = bvec2(true);
    iOut.b3 = bvec3(true);
    iOut.b4 = bvec4(true);
    iOut.ba = bool[5](true, true, true, true, true);
    bvec2 true3[3] = bvec2[3](iOut.b2, iOut.b2, iOut.b2);
    iOut.b2a = bvec2[2][3](true3, true3);
    return true;
}

void main(){
    // Directly copy from one layout to another.
    i.b = o.i[0].b;
    i.b2 = o.i[0].b2;
    i.b2a = o.i[0].b2a;

    // Copy to temp with swizzle.
    bvec4 t1 = o.i[0].b3.yxzy;
    bvec4 t2 = o.i[0].b4.xxyy;
    bvec4 t3 = o.i[0].b4.zzww;

    // Copy from temp with swizzle.
    i.b3 = t1.ywz;
    i.b4.yz = bvec2(t2.z, t3.y);
    i.b4.wx = bvec2(t3.w, t2.x);

    // Copy by passing argument to function.
    writeArgToStd430(o.i[0].ba);

    // Copy by return value.
    ssbo140Out.i.ba = readFromStd430(0u);

    // Copy by out parameter.
    copyFromStd430(ssbo140Out.i.b2a);

    // Logical operations
    uvec4 t4 = ubo430In.o.i[0].b ? uvec4(0) : uvec4(1);
    ssbo140Out.i.b = all(equal(t4, uvec4(1))) && (ubo430In.o.i[0].b ? false : true);
    ssbo140Out.i.b2 = not(ubo430In.o.i[0].b2);
    ssbo140Out.i.b3 = bvec3(all(ubo430In.o.i[0].b3), any(ubo430In.o.i[0].b3), any(ubo430In.o.i[0].b3.yx));
    ssbo140Out.i.b4 = equal(ubo430In.o.i[0].b4, bvec4(true, false, true, false));

    ssbo140Out.valid = true;
    ssbo140Out.valid = ssbo140Out.valid && all(equal(bvec3(o.i[1].b, o.i[1].b2), o.i[1].b3));
    ssbo140Out.valid = ssbo140Out.valid &&
            all(notEqual(o.i[1].b4, bvec4(o.i[1].ba[0], o.i[1].ba[1], o.i[1].ba[2], o.i[1].ba[3])));
    ssbo140Out.valid = ssbo140Out.valid && uint(o.i[1].ba[4]) == 1u;
    for (int x = 0; x < o.i[1].b2a.length(); ++x)
    {
        for (int y = 0; y < o.i[1].b2a[x].length(); ++y)
        {
            ssbo140Out.valid = ssbo140Out.valid && all(equal(uvec2(o.i[1].b2a[x][y]), uvec2(x % 2, y % 2)));
        }
    }

    valid = o.i[1] == ubo430In.o.i[1];

    // Make sure short-circuiting behavior is correct.
    bool falseVar = !valid && destroyContent(i);
    if (falseVar && destroyContent(ssbo140Out.i))
    {
        valid = false;
    }

    if (valid || o.i[uint((i.ba = bool[5](true, true, true, true, true))[1])].b)
    {
    }
    else
    {
        ssbo140Out.valid = false;
    }
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    // Test data, laid out with padding (0) based on std140/std430 rules.
    // clang-format off
    const std::vector<uint32_t> ubo140Data = {
        // o.i[0].b (bool)
        true, 0,
        // o.i[0].b2 (bvec2)
        true, false,
        // o.i[0].b3 (bvec3)
        true, true, false, 0,
        // o.i[0].b4 (bvec4)
        false, true, false, true,
        // o.i[0].ba (bool[5])
        true, 0, 0, 0,
        false, 0, 0, 0,
        false, 0, 0, 0,
        true, 0, 0, 0,
        true, 0, 0, 0,
        // o.i[0].b2a (bool[2][3])
        false, true, 0, 0,  true, true, 0, 0,    true, false, 0, 0,
        true, false, 0, 0,  false, false, 0, 0,  true, true, 0, 0,

        // o.i[1].b (bool)
        false, 0,
        // o.i[1].b2 (bvec2)
        true, true,
        // o.i[1].b3 (bvec3), expected to be equal to (b, b2)
        false, true, true, 0,
        // o.i[1].b4 (bvec4)
        true, false, true, true,
        // o.i[1].ba (bool[5]), expected to be equal to (not(b4), 1)
        false, 0, 0, 0,
        true, 0, 0, 0,
        false, 0, 0, 0,
        false, 0, 0, 0,
        true, 0, 0, 0,
        // o.i[1].b2a (bvec2[2][3]), [x][y] expected to equal (x%2,y%2)
        false, false, 0, 0,  false, true, 0, 0,  false, false, 0, 0,
        true, false, 0, 0,   true, true, 0, 0,   true, false, 0, 0,
    };
    const std::vector<uint32_t> ubo430Data = {
        // o.i[0].b (bool)
        false, 0,
        // o.i[0].b2 (bvec2)
        true, true,
        // o.i[0].b3 (bvec3)
        false, false, true, 0,
        // o.i[0].b4 (bvec4)
        true, false, true, true,
        // o.i[0].ba (bool[5])
        false, false, false, true, false, 0,
        // o.i[0].b2a (bool[2][3])
        true, false,  true, false,  true, true,
        false, true,  true, true,   false, false, 0, 0,

        // o.i[1] expected to be equal to ubo140In.o.i[1]
        // o.i[1].b (bool)
        false, 0,
        // o.i[1].b2 (bvec2)
        true, true,
        // o.i[1].b3 (bvec3)
        false, true, true, 0,
        // o.i[1].b4 (bvec4)
        true, false, true, true,
        // o.i[1].ba (bool[5])
        false, true, false, false, true, 0,
        // o.i[1].b2a (bvec2[2][3])
        false, false,  false, true,  false, false,
        true, false,   true, true,   true, false,
    };
    const std::vector<uint32_t> ssbo140Expect = {
        // valid, expected to be true
        true, 0, 0, 0,

        // i.b (bool), ubo430In.o.i[0].b ? false : true
        true, 0,
        // i.b2 (bvec2), not(ubo430In.o.i[0].b2)
        false, false,
        // i.b3 (bvec3), all(ubo430In.o.i[0].b3), any(...b3), any(...b3.yx)
        false, true, false, 0,
        // i.b4 (bvec4), ubo430In.o.i[0].b4 == (true, false, true, false)
        true, true, true, false,
        // i.ba (bool[5]), copied from ubo430In.o.i[0].ba
        false, 0, 0, 0,
        false, 0, 0, 0,
        false, 0, 0, 0,
        true, 0, 0, 0,
        false, 0, 0, 0,
        // i.b2a (bool[2][3]), copied from ubo430In.o.i[0].b2a
        true, false, 0, 0,  true, false, 0, 0,   true, true, 0, 0,
        false, true, 0, 0,  true, true, 0, 0,    false, false, 0, 0,
    };
    const std::vector<uint32_t> ssbo430Expect = {
        // valid, expected to be true
        true, 0, 0, 0,

        // o.i[0].b (bool), copied from (Ubo140::)o.i[0].b
        true, 0,
        // o.i[0].b2 (bvec2), copied from (Ubo140::)o.i[0].b2
        true, false,
        // o.i[0].b3 (bvec3), copied from (Ubo140::)o.i[0].b3
        true, true, false, 0,
        // o.i[0].b4 (bvec4), copied from (Ubo140::)o.i[0].b4
        false, true, false, true,
        // o.i[0].ba (bool[5]), copied from (Ubo140::)o.i[0].ba
        true, false, false, true, true, 0,
        // o.i[0].b2a (bool[2][3]), copied from (Ubo140::)o.i[0].b2a
        false, true,  true, true,    true, false,
        true, false,  false, false,  true, true, 0, 0,
    };
    const std::vector<uint32_t> zeros(std::max(ssbo140Expect.size(), ssbo430Expect.size()), 0);
    // clang-format on

    GLBuffer uboStd140, uboStd430;
    GLBuffer ssboStd140, ssboStd430;

    InitBuffer(program, "Ubo140", uboStd140, 0, ubo140Data.data(),
               static_cast<uint32_t>(ubo140Data.size()), true);
    InitBuffer(program, "Ubo430", uboStd430, 0, ubo430Data.data(),
               static_cast<uint32_t>(ubo430Data.size()), false);
    InitBuffer(program, "Ssbo140", ssboStd140, 1, zeros.data(),
               static_cast<uint32_t>(ssbo140Expect.size()), false);
    InitBuffer(program, "Ssbo430", ssboStd430, 2, zeros.data(),
               static_cast<uint32_t>(ssbo430Expect.size()), false);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(VerifyBuffer(ssboStd140, ssbo140Expect.data(),
                             static_cast<uint32_t>(ssbo140Expect.size())));
    EXPECT_TRUE(VerifyBuffer(ssboStd430, ssbo430Expect.data(),
                             static_cast<uint32_t>(ssbo430Expect.size())));
}

// Verify that ternary operator works when the operands are matrices used in different block
// storage.
TEST_P(GLSLTest_ES31, TernaryOnMatricesInDifferentBlockStorages)
{
    constexpr char kCS[] = R"(#version 310 es
precision highp float;
layout(local_size_x=1) in;

layout(std140, column_major) uniform Ubo140c
{
    uint u;
    layout(row_major) mat3x2 m;
} ubo140cIn;

layout(std430, row_major, binding = 0) buffer Ubo430r
{
    uint u;
    layout(column_major) mat3x2 m;
} ubo430rIn;

layout(std140, column_major, binding = 1) buffer Ssbo140c
{
    uint u;
    mat3x2 m;
} ssbo140cIn;

layout(std430, row_major, binding = 2) buffer Ssbo430r
{
    mat3x2 m1;
    mat3x2 m2;
} ssbo430rOut;

void main(){
    ssbo430rOut.m1 = ubo140cIn.u > ubo430rIn.u ? ubo140cIn.m : ubo430rIn.m;
    ssbo430rOut.m2 = ssbo140cIn.u > ubo140cIn.u ? ssbo140cIn.m : ubo140cIn.m;

    mat3x2 m = mat3x2(0);

    ssbo430rOut.m1 = ubo140cIn.u == 0u ? m : ssbo430rOut.m1;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    // Test data, laid out with padding (0) based on std140/std430 rules.
    // clang-format off
    const std::vector<float> ubo140cData = {
        // u (uint)
        1, 0, 0, 0,

        // m (mat3x2, row-major)
        5, 7, 9, 0,     6, 8, 10, 0,
    };
    const std::vector<float> ubo430rData = {
        // u (uint)
        135, 0,

        // m (mat3x2, column-major)
        139, 140,         141, 142,         143, 144,
    };
    const std::vector<float> ssbo140cData = {
        // u (uint)
        204, 0, 0, 0,

        // m (mat3x2, column-major)
        205, 206, 0, 0,  207, 208, 0, 0,  209, 210, 0, 0,
    };
    const std::vector<float> ssbo430rExpect = {
        // m1 (mat3x2, row-major), copied from ubo430rIn.m
        139, 141, 143, 0,  140, 142, 144, 0,

        // m2 (mat3x2, row-major), copied from ssbo140cIn.m
        205, 207, 209, 0,  206, 208, 210, 0,
    };
    const std::vector<float> zeros(ssbo430rExpect.size(), 0);
    // clang-format on

    GLBuffer uboStd140ColMajor, uboStd430RowMajor;
    GLBuffer ssboStd140ColMajor, ssboStd430RowMajor;

    InitBuffer(program, "Ubo140c", uboStd140ColMajor, 0, ubo140cData.data(),
               static_cast<uint32_t>(ubo140cData.size()), true);
    InitBuffer(program, "Ubo430r", uboStd430RowMajor, 0, ubo430rData.data(),
               static_cast<uint32_t>(ubo430rData.size()), false);
    InitBuffer(program, "Ssbo140c", ssboStd140ColMajor, 1, ssbo140cData.data(),
               static_cast<uint32_t>(ssbo140cData.size()), false);
    InitBuffer(program, "Ssbo430r", ssboStd430RowMajor, 2, zeros.data(),
               static_cast<uint32_t>(ssbo430rExpect.size()), false);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(VerifyBuffer(ssboStd430RowMajor, ssbo430rExpect.data(),
                             static_cast<uint32_t>(ssbo430rExpect.size())));
}

// Verify that ternary operator works when the operands are structs used in different block
// storage.
TEST_P(GLSLTest_ES31, TernaryOnStructsInDifferentBlockStorages)
{
    constexpr char kCS[] = R"(#version 310 es
precision highp float;
layout(local_size_x=1) in;

struct S
{
    mat3x2 m[2];
};

layout(std140, column_major) uniform Ubo140c
{
    uint u;
    layout(row_major) S s;
} ubo140cIn;

layout(std430, row_major, binding = 0) buffer Ubo430r
{
    uint u;
    layout(column_major) S s;
} ubo430rIn;

layout(std140, column_major, binding = 1) buffer Ssbo140c
{
    uint u;
    S s;
} ssbo140cIn;

layout(std430, row_major, binding = 2) buffer Ssbo430r
{
    S s1;
    S s2;
} ssbo430rOut;

void main(){
    ssbo430rOut.s1 = ubo140cIn.u > ubo430rIn.u ? ubo140cIn.s : ubo430rIn.s;
    ssbo430rOut.s2 = ssbo140cIn.u > ubo140cIn.u ? ssbo140cIn.s : ubo140cIn.s;

    S s;
    s.m[0] = mat3x2(0);
    s.m[1] = mat3x2(0);

    ssbo430rOut.s1 = ubo140cIn.u == 0u ? s : ssbo430rOut.s1;
})";

    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    EXPECT_GL_NO_ERROR();

    // Test data, laid out with padding (0) based on std140/std430 rules.
    // clang-format off
    const std::vector<float> ubo140cData = {
        // u (uint)
        1, 0, 0, 0,

        // s.m[0] (mat3x2, row-major)
        5, 7, 9, 0,     6, 8, 10, 0,
        // s.m[1] (mat3x2, row-major)
        25, 27, 29, 0,  26, 28, 30, 0,
    };
    const std::vector<float> ubo430rData = {
        // u (uint)
        135, 0,

        // s.m[0] (mat3x2, column-major)
        139, 140,         141, 142,         143, 144,
        // s.m[1] (mat3x2, column-major)
        189, 190,         191, 192,         193, 194,
    };
    const std::vector<float> ssbo140cData = {
        // u (uint)
        204, 0, 0, 0,

        // s.m[0] (mat3x2, column-major)
        205, 206, 0, 0,  207, 208, 0, 0,  209, 210, 0, 0,
        // s.m[1] (mat3x2, column-major)
        245, 246, 0, 0,  247, 248, 0, 0,  249, 250, 0, 0,
    };
    const std::vector<float> ssbo430rExpect = {
        // s1.m[0] (mat3x2, row-major), copied from ubo430rIn.s.m[0]
        139, 141, 143, 0,  140, 142, 144, 0,
        // s1.m[1] (mat3x2, row-major), copied from ubo430rIn.s.m[0]
        189, 191, 193, 0,  190, 192, 194, 0,

        // s2.m[0] (mat3x2, row-major), copied from ssbo140cIn.m
        205, 207, 209, 0,  206, 208, 210, 0,
        // s2.m[1] (mat3x2, row-major), copied from ssbo140cIn.m
        245, 247, 249, 0,  246, 248, 250, 0,
    };
    const std::vector<float> zeros(ssbo430rExpect.size(), 0);
    // clang-format on

    GLBuffer uboStd140ColMajor, uboStd430RowMajor;
    GLBuffer ssboStd140ColMajor, ssboStd430RowMajor;

    InitBuffer(program, "Ubo140c", uboStd140ColMajor, 0, ubo140cData.data(),
               static_cast<uint32_t>(ubo140cData.size()), true);
    InitBuffer(program, "Ubo430r", uboStd430RowMajor, 0, ubo430rData.data(),
               static_cast<uint32_t>(ubo430rData.size()), false);
    InitBuffer(program, "Ssbo140c", ssboStd140ColMajor, 1, ssbo140cData.data(),
               static_cast<uint32_t>(ssbo140cData.size()), false);
    InitBuffer(program, "Ssbo430r", ssboStd430RowMajor, 2, zeros.data(),
               static_cast<uint32_t>(ssbo430rExpect.size()), false);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    glDispatchCompute(1, 1, 1);
    EXPECT_GL_NO_ERROR();

    EXPECT_TRUE(VerifyBuffer(ssboStd430RowMajor, ssbo430rExpect.data(),
                             static_cast<uint32_t>(ssbo430rExpect.size())));
}

// Verify that uint in interface block cast to bool works.
TEST_P(GLSLTest_ES3, UintCastToBoolFromInterfaceBlocks)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;

uniform uvec4 uv4;
uniform uvec2 uv2;
uniform uint u1;
uniform uint u2;

out vec4 colorOut;

void main()
{
    bvec4 bv4 = bvec4(uv4);
    bvec2 bv2 = bvec2(uv2);
    bool b1 = bool(u1);
    bool b2 = bool(u2);

    vec4 vv4 = mix(vec4(0), vec4(0.4), bv4);
    vec2 vv2 = mix(vec2(0), vec2(0.7), bv2);
    float v1 = b1 ? 1.0 : 0.0;
    float v2 = b2 ? 0.0 : 1.0;

    colorOut = vec4(vv4.x - vv4.y + vv4.z + vv4.w,
                        (vv2.y - vv2.x) * 1.5,
                        v1,
                        v2);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    EXPECT_GL_NO_ERROR();

    glUseProgram(program);
    GLint uv4 = glGetUniformLocation(program, "uv4");
    GLint uv2 = glGetUniformLocation(program, "uv2");
    GLint u1  = glGetUniformLocation(program, "u1");
    GLint u2  = glGetUniformLocation(program, "u2");
    ASSERT_NE(uv4, -1);
    ASSERT_NE(uv2, -1);
    ASSERT_NE(u1, -1);
    ASSERT_NE(u2, -1);

    glUniform4ui(uv4, 123, 0, 9, 8297312);
    glUniform2ui(uv2, 0, 90812);
    glUniform1ui(u1, 8979421);
    glUniform1ui(u2, 0);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that the precise keyword is not reserved before ES3.1.
TEST_P(GLSLTest_ES3, PreciseNotReserved)
{
    // Skip in ES3.1+ as the precise keyword is reserved/core.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() > 3 ||
                       (getClientMajorVersion() == 3 && getClientMinorVersion() >= 1));

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
in float precise;
out vec4 my_FragColor;
void main() { my_FragColor = vec4(precise, 0, 0, 1.0); })";

    constexpr char kVS[] = R"(#version 300 es
in vec4 a_position;
out float precise;
void main() { precise = a_position.x; gl_Position = a_position; })";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

// Test that the precise keyword is reserved on ES3.0 without GL_EXT_gpu_shader5.
TEST_P(GLSLTest_ES31, PreciseReservedWithoutExtension)
{
    // Skip if EXT_gpu_shader5 is enabled.
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_EXT_gpu_shader5"));
    // Skip in ES3.2+ as the precise keyword is core.
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() > 3 ||
                       (getClientMajorVersion() == 3 && getClientMinorVersion() >= 2));

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
in float v_varying;
out vec4 my_FragColor;
void main() { my_FragColor = vec4(v_varying, 0, 0, 1.0); })";

    constexpr char kVS[] = R"(#version 310 es
in vec4 a_position;
precise out float v_varying;
void main() { v_varying = a_position.x; gl_Position = a_position; })";

    // Should fail, as precise is a reserved keyword when the extension is not enabled.
    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Regression test for a bug with precise in combination with constructor, swizzle and dynamic
// index.
TEST_P(GLSLTest_ES31, PreciseVsVectorConstructorSwizzleAndIndex)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_gpu_shader5"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_gpu_shader5 : require

uniform highp float u;

void main()
{
    precise float p = vec4(u, u, u, u).xyz[int(u)];
    gl_Position = vec4(p);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
out vec4 oColor;
void main()
{
    oColor = vec4(1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Regression test for a bug with precise in combination with matrix constructor and column index.
TEST_P(GLSLTest_ES31, PreciseVsMatrixConstructorAndIndex)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_gpu_shader5"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_gpu_shader5 : require

uniform highp vec4 u;

void main()
{
    precise vec4 p = mat4(u,vec4(0),vec4(0),vec4(0))[0];
    gl_Position = p;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
out vec4 oColor;
void main()
{
    oColor = vec4(1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Regression test for a bug with precise in combination with struct constructor and field
// selection.
TEST_P(GLSLTest_ES31, PreciseVsStructConstructorAndFieldSelection)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_gpu_shader5"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_gpu_shader5 : require

struct S
{
    float a;
    float b;
};

uniform highp float u;

void main()
{
    precise float p = S(u, u).b;
    gl_Position = vec4(p);
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
out vec4 oColor;
void main()
{
    oColor = vec4(1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
}

// Test that reusing the same variable name for different uses across stages links fine.  The SPIR-V
// transformation should ignore all names for non-shader-interface variables and not get confused by
// them.
TEST_P(GLSLTest_ES31, VariableNameReuseAcrossStages)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;
uniform highp vec4 a;
in highp vec4 b;
in highp vec4 c;
in highp vec4 d;
out highp vec4 e;

vec4 f(vec4 a)
{
    return a;
}

vec4 g(vec4 f)
{
    return f + f;
}

void main() {
    e = f(b) + a;
    gl_Position = g(c) + f(d);
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
in highp vec4 e;
uniform sampler2D f;
layout(rgba8) uniform highp readonly image2D g;
uniform A
{
    vec4 x;
} c;
layout(std140, binding=0) buffer B
{
    vec4 x;
} d[2];
out vec4 col;

vec4 h(vec4 c)
{
    return texture(f, c.xy) + imageLoad(g, ivec2(c.zw));
}

vec4 i(vec4 x, vec4 y)
{
    return vec4(x.xy, y.zw);
}

void main() {
    col = h(e) + i(c.x, d[0].x) + d[1].x;
}
)";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

// Test that reusing the same uniform variable name for different uses across stages links fine.
TEST_P(GLSLTest_ES31, UniformVariableNameReuseAcrossStages)
{
    constexpr char kVS[] = R"(#version 310 es
precision mediump float;
in highp vec4 variableWithSameName;

void main() {
    gl_Position = variableWithSameName;
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
uniform vec4 variableWithSameName;
out vec4 col;

void main() {
    col = vec4(variableWithSameName);
}
)";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_NE(0u, program);
}

// Verify that precision match validation of uniforms is performed only if they are statically used
TEST_P(GLSLTest_ES31, UniformPrecisionMatchValidation)
{
    // Nvidia driver bug: http://anglebug.com/42263793
    ANGLE_SKIP_TEST_IF(IsOpenGL() && IsWindows() && IsNVIDIA());

    constexpr char kVSUnused[] = R"(#version 300 es
precision highp float;
uniform highp vec4 positionIn;

void main()
{
    gl_Position = vec4(1, 0, 0, 1);
})";

    constexpr char kVSStaticUse[] = R"(#version 300 es
precision highp float;
uniform highp vec4 positionIn;

void main()
{
    gl_Position = positionIn;
})";

    constexpr char kFSUnused[] = R"(#version 300 es
precision highp float;
uniform highp vec4 positionIn;
out vec4 my_FragColor;

void main()
{
    my_FragColor = vec4(1, 0, 0, 1);
})";

    constexpr char kFSStaticUse[] = R"(#version 300 es
precision highp float;
uniform mediump vec4 positionIn;
out vec4 my_FragColor;

void main()
{
    my_FragColor = vec4(1, 0, 0, positionIn.z);
})";

    GLuint program = 0;

    program = CompileProgram(kVSUnused, kFSUnused);
    EXPECT_NE(0u, program);

    program = CompileProgram(kVSUnused, kFSStaticUse);
    EXPECT_NE(0u, program);

    program = CompileProgram(kVSStaticUse, kFSUnused);
    EXPECT_NE(0u, program);

    program = CompileProgram(kVSStaticUse, kFSStaticUse);
    EXPECT_EQ(0u, program);
}

// Validate that link fails when two instanceless interface blocks with different block names but
// same field names are present.
TEST_P(GLSLTest_ES31, AmbiguousInstancelessInterfaceBlockFields)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
in highp vec4 position;
layout(binding = 0) buffer BlockA { mediump float a; };
void main()
{
    a = 0.0;
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
layout(location = 0) out mediump vec4 color;
uniform BlockB { float a; };
void main()
{
    color = vec4(a, a, a, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Verify I/O block array locations
TEST_P(GLSLTest_ES31, IOBlockLocations)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require

in highp vec4 position;

layout(location = 0) out vec4 aOut;

layout(location = 6) out VSBlock
{
    vec4 b;     // location 6
    vec4 c;     // location 7
    layout(location = 1) vec4 d;
    vec4 e;     // location 2
    vec4 f[2];  // locations 3 and 4
} blockOut;

layout(location = 5) out vec4 gOut;

void main()
{
    aOut = vec4(0.03, 0.06, 0.09, 0.12);
    blockOut.b = vec4(0.15, 0.18, 0.21, 0.24);
    blockOut.c = vec4(0.27, 0.30, 0.33, 0.36);
    blockOut.d = vec4(0.39, 0.42, 0.45, 0.48);
    blockOut.e = vec4(0.51, 0.54, 0.57, 0.6);
    blockOut.f[0] = vec4(0.63, 0.66, 0.66, 0.69);
    blockOut.f[1] = vec4(0.72, 0.75, 0.78, 0.81);
    gOut = vec4(0.84, 0.87, 0.9, 0.93);
    gl_Position = position;
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// Input varyings
layout(location = 0) in vec4 aIn[];

layout(location = 6) in VSBlock
{
    vec4 b;
    vec4 c;
    layout(location = 1) vec4 d;
    vec4 e;
    vec4 f[2];
} blockIn[];

layout(location = 5) in vec4 gIn[];

// Output varyings
layout(location = 1) out vec4 aOut;

layout(location = 0) out GSBlock
{
    vec4 b;     // location 0
    layout(location = 3) vec4 c;
    layout(location = 7) vec4 d;
    layout(location = 5) vec4 e[2];
    layout(location = 4) vec4 f;
} blockOut;

layout(location = 2) out vec4 gOut;

void main()
{
    int n;
    for (n = 0; n < gl_in.length(); n++)
    {
        gl_Position = gl_in[n].gl_Position;

        aOut = aIn[n];
        blockOut.b = blockIn[n].b;
        blockOut.c = blockIn[n].c;
        blockOut.d = blockIn[n].d;
        blockOut.e[0] = blockIn[n].e;
        blockOut.e[1] = blockIn[n].f[0];
        blockOut.f = blockIn[n].f[1];
        gOut = gIn[n];

        EmitVertex();
    }
    EndPrimitive();
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;

layout(location = 0) out mediump vec4 color;

layout(location = 1) in vec4 aIn;

layout(location = 0) in GSBlock
{
    vec4 b;
    layout(location = 3) vec4 c;
    layout(location = 7) vec4 d;
    layout(location = 5) vec4 e[2];
    layout(location = 4) vec4 f;
} blockIn;

layout(location = 2) in vec4 gIn;

bool isEq(vec4 a, vec4 b) { return all(lessThan(abs(a-b), vec4(0.001))); }

void main()
{
    bool passR = isEq(aIn, vec4(0.03, 0.06, 0.09, 0.12));
    bool passG = isEq(blockIn.b, vec4(0.15, 0.18, 0.21, 0.24)) &&
                 isEq(blockIn.c, vec4(0.27, 0.30, 0.33, 0.36)) &&
                 isEq(blockIn.d, vec4(0.39, 0.42, 0.45, 0.48)) &&
                 isEq(blockIn.e[0], vec4(0.51, 0.54, 0.57, 0.6)) &&
                 isEq(blockIn.e[1], vec4(0.63, 0.66, 0.66, 0.69)) &&
                 isEq(blockIn.f, vec4(0.72, 0.75, 0.78, 0.81));
    bool passB = isEq(gIn, vec4(0.84, 0.87, 0.9, 0.93));

    color = vec4(passR, passG, passB, 1.0);
})";

    ANGLE_GL_PROGRAM_WITH_GS(program, kVS, kGS, kFS);
    EXPECT_GL_NO_ERROR();

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

    drawQuad(program, "position", 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test using builtins that can only be redefined with gl_PerVertex
TEST_P(GLSLTest_ES31, PerVertexRedefinition)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clip_cull_distance"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    gl_Position = vec4(1.0, 0.0, 0.0, 1.0);
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
#extension GL_EXT_clip_cull_distance : require

layout(lines_adjacency, invocations = 3) in;
layout(points, max_vertices = 16) out;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_ClipDistance[4];
    float gl_CullDistance[4];
};

void main()
{
    for (int n = 0; n < 16; ++n)
    {
        gl_Position = vec4(n, 0.0, 0.0, 1.0);
        EmitVertex();
    }

    EndPrimitive();
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;

out vec4 result;

void main()
{
    result = vec4(1.0);
})";

    ANGLE_GL_PROGRAM_WITH_GS(program, kVS, kGS, kFS);
    EXPECT_GL_NO_ERROR();
}

// Negative test using builtins that can only be used when redefining gl_PerVertex
TEST_P(GLSLTest_ES31, PerVertexNegativeTest)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clip_cull_distance"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    gl_Position = vec4(1.0, 0.0, 0.0, 1.0);
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
#extension GL_EXT_clip_cull_distance : require

layout(lines_adjacency, invocations = 3) in;
layout(points, max_vertices = 16) out;

vec4 gl_Position;
float gl_ClipDistance[4];
float gl_CullDistance[4];

void main()
{
    for (int n = 0; n < 16; ++n)
    {
        gl_Position = vec4(n, 0.0, 0.0, 1.0);
        EmitVertex();
    }

    EndPrimitive();
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;

out vec4 result;

void main()
{
    result = vec4(1.0);
})";

    GLuint program = CompileProgramWithGS(kVS, kGS, kFS);
    EXPECT_EQ(0u, program);
    glDeleteProgram(program);
}

// Negative test using builtins that can only be used when redefining gl_PerVertex
// but have the builtins in a differently named struct
TEST_P(GLSLTest_ES31, PerVertexRenamedNegativeTest)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_clip_cull_distance"));

    constexpr char kVS[] = R"(#version 310 es
void main()
{
    gl_Position = vec4(1.0, 0.0, 0.0, 1.0);
})";

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
#extension GL_EXT_clip_cull_distance : require

layout(lines_adjacency, invocations = 3) in;
layout(points, max_vertices = 16) out;

out Block {
    vec4 gl_Position;
    float gl_ClipDistance[4];
    float gl_CullDistance[4];
};

void main()
{
    for (int n = 0; n < 16; ++n)
    {
        gl_Position = vec4(n, 0.0, 0.0, 1.0);
        EmitVertex();
    }

    EndPrimitive();
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;

out vec4 result;

void main()
{
    result = vec4(1.0);
})";

    GLuint program = CompileProgramWithGS(kVS, kGS, kFS);
    EXPECT_EQ(0u, program);
    glDeleteProgram(program);
}

// Test varying packing in presence of multiple I/O blocks
TEST_P(GLSLTest_ES31, MultipleIOBlocks)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require

in highp vec4 position;

out VSBlock1
{
    vec4 a;
    vec4 b[2];
} blockOut1;

out VSBlock2
{
    vec4 c[2];
    vec4 d;
} blockOut2;

void main()
{
    blockOut1.a = vec4(0.15, 0.18, 0.21, 0.24);
    blockOut1.b[0] = vec4(0.27, 0.30, 0.33, 0.36);
    blockOut1.b[1] = vec4(0.39, 0.42, 0.45, 0.48);
    blockOut2.c[0] = vec4(0.51, 0.54, 0.57, 0.6);
    blockOut2.c[1] = vec4(0.63, 0.66, 0.66, 0.69);
    blockOut2.d = vec4(0.72, 0.75, 0.78, 0.81);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;

layout(location = 0) out mediump vec4 color;

in VSBlock1
{
    vec4 a;
    vec4 b[2];
} blockIn1;

in VSBlock2
{
    vec4 c[2];
    vec4 d;
} blockIn2;

bool isEq(vec4 a, vec4 b) { return all(lessThan(abs(a-b), vec4(0.001))); }

void main()
{
    bool passR = isEq(blockIn1.a, vec4(0.15, 0.18, 0.21, 0.24));
    bool passG = isEq(blockIn1.b[0], vec4(0.27, 0.30, 0.33, 0.36)) &&
                 isEq(blockIn1.b[1], vec4(0.39, 0.42, 0.45, 0.48));
    bool passB = isEq(blockIn2.c[0], vec4(0.51, 0.54, 0.57, 0.6)) &&
                 isEq(blockIn2.c[1], vec4(0.63, 0.66, 0.66, 0.69));
    bool passA = isEq(blockIn2.d, vec4(0.72, 0.75, 0.78, 0.81));

    color = vec4(passR, passG, passB, passA);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    EXPECT_GL_NO_ERROR();

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

    drawQuad(program, "position", 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test varying packing in presence of I/O block arrays
TEST_P(GLSLTest_ES31, IOBlockArray)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require

in highp vec4 position;

out VSBlock1
{
    vec4 b[2];
} blockOut1[2];

out VSBlock2
{
    vec4 d;
} blockOut2[3];

void main()
{
    blockOut1[0].b[0] = vec4(0.15, 0.18, 0.21, 0.24);
    blockOut1[0].b[1] = vec4(0.27, 0.30, 0.33, 0.36);
    blockOut1[1].b[0] = vec4(0.39, 0.42, 0.45, 0.48);
    blockOut1[1].b[1] = vec4(0.51, 0.54, 0.57, 0.6);
    blockOut2[0].d = vec4(0.63, 0.66, 0.66, 0.69);
    blockOut2[1].d = vec4(0.72, 0.75, 0.78, 0.81);
    blockOut2[2].d = vec4(0.84, 0.87, 0.9, 0.93);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;

layout(location = 0) out mediump vec4 color;

in VSBlock1
{
    vec4 b[2];
} blockIn1[2];

in VSBlock2
{
    vec4 d;
} blockIn2[3];

bool isEq(vec4 a, vec4 b) { return all(lessThan(abs(a-b), vec4(0.001))); }

void main()
{
    bool passR = isEq(blockIn1[0].b[0], vec4(0.15, 0.18, 0.21, 0.24)) &&
                 isEq(blockIn1[0].b[1], vec4(0.27, 0.30, 0.33, 0.36));
    bool passG = isEq(blockIn1[1].b[0], vec4(0.39, 0.42, 0.45, 0.48)) &&
                 isEq(blockIn1[1].b[1], vec4(0.51, 0.54, 0.57, 0.6));
    bool passB = isEq(blockIn2[0].d, vec4(0.63, 0.66, 0.66, 0.69));
    bool passA = isEq(blockIn2[1].d, vec4(0.72, 0.75, 0.78, 0.81)) &&
                 isEq(blockIn2[2].d, vec4(0.84, 0.87, 0.9, 0.93));

    color = vec4(passR, passG, passB, passA);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    EXPECT_GL_NO_ERROR();

    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 1, 1);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

    drawQuad(program, "position", 0);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Validate that link fails with I/O block member name mismatches.
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberNameMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
out VSBlock { vec4 a; vec4 b[2]; } blockOut1;
void main()
{
    blockOut1.a = vec4(0);
    blockOut1.b[0] = vec4(0);
    blockOut1.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
in VSBlock { vec4 c; vec4 b[2]; } blockIn1;
void main()
{
    color = vec4(blockIn1.c.x, blockIn1.b[0].y, blockIn1.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block member array size mismatches.
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberArraySizeMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
out VSBlock { vec4 a; vec4 b[2]; } blockOut1;
void main()
{
    blockOut1.a = vec4(0);
    blockOut1.b[0] = vec4(0);
    blockOut1.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
in VSBlock { vec4 a; vec4 b[3]; } blockIn1;
void main()
{
    color = vec4(blockIn1.a.x, blockIn1.b[0].y, blockIn1.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block member type mismatches.
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberTypeMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
out VSBlock { vec4 a; vec4 b[2]; } blockOut1;
void main()
{
    blockOut1.a = vec4(0);
    blockOut1.b[0] = vec4(0);
    blockOut1.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
in VSBlock { vec3 a; vec4 b[2]; } blockIn1;
void main()
{
    color = vec4(blockIn1.a.x, blockIn1.b[0].y, blockIn1.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block location mismatches
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkLocationMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
layout(location = 2) out VSBlock { vec4 a; vec4 b[2]; } blockOut1;
void main()
{
    blockOut1.a = vec4(0);
    blockOut1.b[0] = vec4(0);
    blockOut1.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
layout(location = 1) in VSBlock { vec4 a; vec4 b[2]; } blockIn1;
void main()
{
    color = vec4(blockIn1.a.x, blockIn1.b[0].y, blockIn1.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block member location mismatches
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberLocationMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
out VSBlock { vec4 a; layout(location = 2) vec4 b[2]; } blockOut1;
void main()
{
    blockOut1.a = vec4(0);
    blockOut1.b[0] = vec4(0);
    blockOut1.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
in VSBlock { vec4 a; layout(location = 3) vec4 b[2]; } blockIn1;
void main()
{
    color = vec4(blockIn1.a.x, blockIn1.b[0].y, blockIn1.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block member struct name mismatches.
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberStructNameMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
struct S1 { vec4 a; vec4 b[2]; };
out VSBlock { S1 s; } blockOut1;
void main()
{
    blockOut1.s.a = vec4(0);
    blockOut1.s.b[0] = vec4(0);
    blockOut1.s.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
struct S2 { vec4 a; vec4 b[2]; };
in VSBlock { S2 s; } blockIn1;
void main()
{
    color = vec4(blockIn1.s.a.x, blockIn1.s.b[0].y, blockIn1.s.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block member struct member name mismatches.
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberStructMemberNameMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
struct S { vec4 c; vec4 b[2]; };
out VSBlock { S s; } blockOut1;
void main()
{
    blockOut1.s.c = vec4(0);
    blockOut1.s.b[0] = vec4(0);
    blockOut1.s.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
struct S { vec4 a; vec4 b[2]; };
in VSBlock { S s; } blockIn1;
void main()
{
    color = vec4(blockIn1.s.a.x, blockIn1.s.b[0].y, blockIn1.s.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block member struct member type mismatches.
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberStructMemberTypeMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
struct S { vec4 a; vec4 b[2]; };
out VSBlock { S s; } blockOut1;
void main()
{
    blockOut1.s.a = vec4(0);
    blockOut1.s.b[0] = vec4(0);
    blockOut1.s.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
struct S { vec3 a; vec4 b[2]; };
in VSBlock { S s; } blockIn1;
void main()
{
    color = vec4(blockIn1.s.a.x, blockIn1.s.b[0].y, blockIn1.s.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block member struct member array size mismatches.
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberStructMemberArraySizeMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
struct S { vec4 a; vec4 b[3]; };
out VSBlock { S s; } blockOut1;
void main()
{
    blockOut1.s.a = vec4(0);
    blockOut1.s.b[0] = vec4(0);
    blockOut1.s.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
struct S { vec4 a; vec4 b[2]; };
in VSBlock { S s; } blockIn1;
void main()
{
    color = vec4(blockIn1.s.a.x, blockIn1.s.b[0].y, blockIn1.s.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block member struct member count mismatches.
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberStructMemberCountMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
struct S { vec4 a; vec4 b[2]; vec4 c; };
out VSBlock { S s; } blockOut1;
void main()
{
    blockOut1.s.c = vec4(0);
    blockOut1.s.b[0] = vec4(0);
    blockOut1.s.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
struct S { vec4 a; vec4 b[2]; };
in VSBlock { S s; } blockIn1;
void main()
{
    color = vec4(blockIn1.s.a.x, blockIn1.s.b[0].y, blockIn1.s.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Validate that link fails with I/O block member nested struct mismatches.
TEST_P(GLSLTest_ES31, NegativeIOBlocksLinkMemberNestedStructMismatch)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    constexpr char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in highp vec4 position;
struct S1 { vec4 c; vec4 b[2]; };
struct S2 { S1 s; };
struct S3 { S2 s; };
out VSBlock { S3 s; } blockOut1;
void main()
{
    blockOut1.s.s.s.c = vec4(0);
    blockOut1.s.s.s.b[0] = vec4(0);
    blockOut1.s.s.s.b[1] = vec4(0);
    gl_Position = position;
})";

    constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
layout(location = 0) out mediump vec4 color;
struct S1 { vec4 a; vec4 b[2]; };
struct S2 { S1 s; };
struct S3 { S2 s; };
in VSBlock { S3 s; } blockIn1;
void main()
{
    color = vec4(blockIn1.s.s.s.a.x, blockIn1.s.s.s.b[0].y, blockIn1.s.s.s.b[1].z, 1.0);
})";

    GLuint program = CompileProgram(kVS, kFS);
    EXPECT_EQ(0u, program);
}

// Test that separating declarators works with structs that have been separately defined.
TEST_P(GLSLTest_ES31, SeparateDeclaratorsOfStructType)
{
    constexpr char kVS[] = R"(#version 310 es
precision highp float;

struct S
{
    mat4 a;
    mat4 b;
};

S s1 = S(mat4(1), mat4(2)), s2[2][3], s3[2] = S[2](S(mat4(0), mat4(3)), S(mat4(4), mat4(5)));

void main() {
    S s4[2][3] = s2, s5 = s3[0], s6[2] = S[2](s1, s5), s7 = s5;

    gl_Position = vec4(s3[1].a[0].x, s2[0][2].b[1].y, s4[1][0].a[2].z, s6[0].b[3].w);
})";

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);

    const char *sourceArray[1] = {kVS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kVS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that separating declarators works with structs that are simultaneously defined.
TEST_P(GLSLTest_ES31, SeparateDeclaratorsOfStructTypeBeingSpecified)
{
    constexpr char kVS[] = R"(#version 310 es
precision highp float;

struct S
{
    mat4 a;
    mat4 b;
} s1 = S(mat4(1), mat4(2)), s2[2][3], s3[2] = S[2](S(mat4(0), mat4(3)), S(mat4(4), mat4(5)));

void main() {
    struct T
    {
        mat4 a;
        mat4 b;
    } s4[2][3], s5 = T(s3[0].a, s3[0].b), s6[2] = T[2](T(s1.a, s1.b), s5), s7 = s5;

    float f1 = s3[1].a[0].x, f2 = s2[0][2].b[1].y;

    gl_Position = vec4(f1, f2, s4[1][0].a[2].z, s6[0].b[3].w);
})";

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);

    const char *sourceArray[1] = {kVS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kVS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that separating declarators works with structs that are simultaneously defined and that are
// nameless.
TEST_P(GLSLTest_ES31, SeparateDeclaratorsOfNamelessStructType)
{
    constexpr char kVS[] = R"(#version 310 es
precision highp float;

struct
{
    mat4 a;
    mat4 b;
} s1, s2[2][3], s3[2];

void main() {
    struct
    {
        mat4 a;
        mat4 b;
    } s4[2][3], s5, s6[2], s7 = s5;

    float f1 = s1.a[0].x + s3[1].a[0].x, f2 = s2[0][2].b[1].y + s7.b[1].z;

    gl_Position = vec4(f1, f2, s4[1][0].a[2].z, s6[0].b[3].w);
})";

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);

    const char *sourceArray[1] = {kVS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kVS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test separation of struct declarations, case where separated struct is used as a member of
// another struct.
TEST_P(GLSLTest, SeparateStructDeclaratorStructInStruct)
{
    const char kFragmentShader[] = R"(precision mediump float;
uniform vec4 u;
struct S1 { vec4 v; } a;
void main()
{
    struct S2 { S1 s1; } b;
    a.v = u;
    b.s1 = a;
    gl_FragColor = b.s1.v + vec4(0, 0, 0, 1);
}
)";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);
    GLint u = glGetUniformLocation(program, "u");
    glUniform4f(u, 0, 1, 0, 0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Regression test for transformation bug which separates struct declarations from uniform
// declarations.  The bug was that the uniform variable usage in the initializer of a new
// declaration (y below) was not being processed.
TEST_P(GLSLTest, UniformStructBug)
{
    constexpr char kVS[] = R"(precision highp float;

uniform struct Global
{
    float x;
} u_global;

void main() {
  float y = u_global.x;

  gl_Position = vec4(y);
})";

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);

    const char *sourceArray[1] = {kVS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kVS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Regression test for transformation bug which separates struct declarations from uniform
// declarations.  The bug was that the arrayness of the declaration was not being applied to the
// replaced uniform variable.
TEST_P(GLSLTest_ES31, UniformStructBug2)
{
    constexpr char kVS[] = R"(#version 310 es
precision highp float;

uniform struct Global
{
    float x;
} u_global[2][3];

void main() {
  float y = u_global[0][0].x;

  gl_Position = vec4(y);
})";

    GLuint shader = glCreateShader(GL_VERTEX_SHADER);

    const char *sourceArray[1] = {kVS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kVS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Regression test based on fuzzer issue resulting in an AST validation failure.  Struct definition
// was not found in the tree.  Tests that struct declaration in function return value is visible to
// instantiations later on.
TEST_P(GLSLTest, MissingStructDeclarationBug)
{
    constexpr char kVS[] = R"(
struct S
{
    vec4 i;
} p();
void main()
{
    S s;
})";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Regression test based on fuzzer issue resulting in an AST validation failure.  Struct definition
// was not found in the tree.  Tests that struct declaration in function return value is visible to
// other struct declarations.
TEST_P(GLSLTest, MissingStructDeclarationBug2)
{
    constexpr char kVS[] = R"(
struct T
{
    vec4 I;
} p();
struct
{
    T c;
};
void main()
{
})";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Regression test for bug in HLSL code generation where the for loop init expression was expected
// to always have an initializer.
TEST_P(GLSLTest, HandleExcessiveLoopBug)
{
    constexpr char kVS[] = R"(void main(){for(int i;i>6;);})";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Regression test for a validation bug in the translator where func(void, int) was accepted even
// though it's illegal, and the function was callable as if the void parameter isn't there.
TEST_P(GLSLTest, NoParameterAfterVoid)
{
    constexpr char kVS[] = R"(void f(void, int a){}
void main(){f(1);})";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_EQ(0u, shader);
    glDeleteShader(shader);
}

// Similar to NoParameterAfterVoid, but tests func(void, void).
TEST_P(GLSLTest, NoParameterAfterVoid2)
{
    constexpr char kVS[] = R"(void f(void, void){}
void main(){f();})";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_EQ(0u, shader);
    glDeleteShader(shader);
}

// Test that providing more components to a matrix constructor than necessary works.  Based on a
// clusterfuzz test that caught an OOB array write in glslang.
TEST_P(GLSLTest, MatrixConstructor)
{
    constexpr char kVS[] = R"(attribute vec4 aPosition;
varying vec4 vColor;
void main()
{
    gl_Position = aPosition;
    vec4 color = vec4(aPosition.xy, 0, 1);
    mat4 m4 = mat4(color, color.yzwx, color.zwx, color.zwxy, color.wxyz);
    vColor = m4[0];
})";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Test constructors without precision
TEST_P(GLSLTest, ConstructFromBoolVector)
{
    constexpr char kFS[] = R"(precision mediump float;
uniform float u;
void main()
{
    mat4 m = mat4(u);
    mat2(0, bvec3(m));
    gl_FragColor = vec4(m);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Test constructing vector from matrix
TEST_P(GLSLTest, VectorConstructorFromMatrix)
{
    constexpr char kFS[] = R"(precision mediump float;
uniform mat2 umat2;
void main()
{
    gl_FragColor = vec4(umat2);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Test constructing matrix from vectors
TEST_P(GLSLTest, MatrixConstructorFromVectors)
{
    constexpr char kFS[] = R"(precision mediump float;
uniform vec2 uvec2;
void main()
{
    mat2 m = mat2(uvec2, uvec2.yx);
    gl_FragColor = vec4(m * uvec2, uvec2);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint uloc = glGetUniformLocation(program, "uvec2");
    ASSERT_NE(uloc, -1);
    glUniform2f(uloc, 0.5, 0.8);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(227, 204, 127, 204), 1);
}

// Test that constructing vector and matrix inside multiple declarations preserves the correct order
// of operations.
TEST_P(GLSLTest, ConstructorinSequenceOperator)
{
    constexpr char kFS[] = R"(precision mediump float;
uniform vec2 u;
void main()
{
    vec2 v = u;
    mat2 m = (v[0] += 1.0, mat2(v, v[1], -v[0]));
    gl_FragColor = vec4(m[0], m[1]);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint uloc = glGetUniformLocation(program, "u");
    ASSERT_NE(uloc, -1);
    glUniform2f(uloc, -0.5, 1.0);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(127, 255, 255, 0), 1);
}

// Test that constructing vectors inside multiple declarations preserves the correct order
// of operations.
TEST_P(GLSLTest, VectorConstructorsInMultiDeclaration)
{
    constexpr char kFS[] = R"(precision mediump float;
uniform vec2 u;
void main()
{
    vec2 v = vec2(u[0]),
         w = mat2(v, v) * u;
    gl_FragColor = vec4(v, w);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint uloc = glGetUniformLocation(program, "u");
    ASSERT_NE(uloc, -1);
    glUniform2f(uloc, 0.5, 0.8);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(127, 127, 166, 166), 1);
}

// Test complex constructor usage.
TEST_P(GLSLTest_ES3, ComplexConstructor)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform vec2 u; // = vec2(0.5, 0.8)
uniform vec2 v; // = vec2(-0.2, 1.0)

out vec4 color;

bool f(mat2 m)
{
    return m[0][0] > 0.;
}

bool isEqual(float a, float b)
{
    return abs(a - b) < 0.01;
}

void main()
{
    int shouldRemainZero = 0;

    // Test side effects inside constructor args after short-circuit
    if (u.x < 0. && f(mat2(shouldRemainZero += 1, u, v)))
    {
        shouldRemainZero += 2;
    }

    int shouldBecomeFive = 0;

    // Test directly nested constructors
    mat4x3 m = mat4x3(mat2(shouldBecomeFive += 5, v, u));

    // Test indirectly nested constructors
    mat2 m2 = mat2(f(mat2(u, v)), f(mat2(v, u)), f(mat2(f(mat2(1.)))), -1.);

    // Verify
    bool sideEffectsOk = shouldRemainZero == 0 && shouldBecomeFive == 5;

    bool mOk = isEqual(m[0][0], 5.) && isEqual(m[0][1], -0.2) && isEqual(m[0][2], 0.) &&
               isEqual(m[1][0], 1.) && isEqual(m[1][1], 0.5) && isEqual(m[1][2], 0.) &&
               isEqual(m[2][0], 0.) && isEqual(m[2][1], 0.) && isEqual(m[2][2], 1.) &&
               isEqual(m[3][0], 0.) && isEqual(m[3][1], 0.) && isEqual(m[3][2], 0.);

    bool m2Ok = isEqual(m2[0][0], 1.) && isEqual(m2[0][1], 0.) &&
               isEqual(m2[1][0], 1.) && isEqual(m2[1][1], -1.);

    color = vec4(sideEffectsOk ? 1 : 0, mOk ? 1 : 0, m2Ok ? 1 : 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint uloc = glGetUniformLocation(program, "u");
    GLint vloc = glGetUniformLocation(program, "v");
    ASSERT_NE(uloc, -1);
    ASSERT_NE(vloc, -1);
    glUniform2f(uloc, 0.5, 0.8);
    glUniform2f(vloc, -0.2, 1.0);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

// Test that scalar(nonScalar) constructors work.
TEST_P(GLSLTest_ES3, ScalarConstructor)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
uniform vec4 u;
out vec4 color;
void main()
{
    float f1 = float(u);
    mat3 m = mat3(u, u, u);
    int i = int(m);
    color = vec4(f1, float(i), 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint uloc = glGetUniformLocation(program, "u");
    ASSERT_NE(uloc, -1);
    glUniform4f(uloc, 1.0, 0.4, 0.2, 0.7);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
}

// Test that initializing global variables with non-constant values work
TEST_P(GLSLTest_ES3, InitGlobalNonConstant)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_non_constant_global_initializers"));

    constexpr char kVS[] = R"(#version 300 es
#extension GL_EXT_shader_non_constant_global_initializers : require
uniform vec4 u;
out vec4 color;

vec4 global1 = u;
vec4 global2 = u + vec4(1);
vec4 global3 = global1 * global2;
void main()
{
    color = global3;
})";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Test that initializing global variables with complex constants work
TEST_P(GLSLTest_ES3, InitGlobalComplexConstant)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 color;

struct T
{
    float f;
};

struct S
{
    vec4 v;
    mat3x4 m[2];
    T t;
};

S s = S(
        vec4(0, 1, 2, 3),
        mat3x4[2](
                  mat3x4(
                         vec4(4, 5, 6, 7),
                         vec4(8, 9, 10, 11),
                         vec4(12, 13, 14, 15)
                  ),
                  mat3x4(
                         vec4(16, 17, 18, 19),
                         vec4(20, 21, 22, 23),
                         vec4(24, 25, 26, 27)
                  )
        ),
        T(28.0)
       );

void main()
{
    vec4 result = vec4(0, 1, 0, 1);

    if (s.v != vec4(0, 1, 2, 3))
        result = vec4(1, 0, 0, 0);

    for (int index = 0; index < 2; ++index)
    {
        for (int column = 0; column < 3; ++column)
        {
            int expect = index * 12 + column * 4 + 4;
            if (s.m[index][column] != vec4(expect, expect + 1, expect + 2, expect + 3))
                result = vec4(float(index + 1) / 2.0, 0, float(column + 1) / 3.0, 1);
        }
    }

    if (s.t.f != 28.0)
        result = vec4(0, 0, 1, 0);

    color = result;
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that built-ins with out parameters work
TEST_P(GLSLTest_ES31, BuiltInsWithOutParameters)
{
    constexpr char kFS[] = R"(#version 310 es
precision highp float;
precision highp int;

out vec4 color;

uniform float f;    // = 3.41
uniform uvec4 u1;   // = 0xFEDCBA98, 0x13579BDF, 0xFEDCBA98, 0x13579BDF
uniform uvec4 u2;   // = 0xECA86420, 0x12345678, 0x12345678, 0xECA86420

struct S
{
    float fvalue;
    int ivalues[2];
    uvec4 uvalues[3];
};

struct T
{
    S s[2];
};

void main()
{
    float integer;
    float fraction = modf(f, integer);

    T t;

    t.s[0].fvalue     = frexp(f, t.s[0].ivalues[0]);
    float significand = t.s[0].fvalue;
    int exponent      = t.s[0].ivalues[0];

    t.s[0].uvalues[0] = uaddCarry(u1, u2, t.s[0].uvalues[1].yxwz);
    uvec4 addResult   = t.s[0].uvalues[0];
    uvec4 addCarry    = t.s[0].uvalues[1].yxwz;

    t.s[0].uvalues[2].wx = usubBorrow(u1.wx, u2.wx, t.s[1].uvalues[0].wx);
    uvec2 subResult      = t.s[0].uvalues[2].wx;
    uvec2 subBorrow      = t.s[1].uvalues[0].wx;

    umulExtended(u1, u2, t.s[1].uvalues[1], t.s[1].uvalues[2]);
    uvec4 mulMsb = t.s[1].uvalues[1];
    uvec4 mulLsb = t.s[1].uvalues[2];

    ivec2 imulMsb, imulLsb;
    imulExtended(ivec2(u1.wz), ivec2(u2.wz), imulMsb.yx, imulLsb.yx);

    bool modfPassed = abs(fraction - 0.41) < 0.0001 && integer == 3.0;
    bool frexpPassed = abs(significand - 0.8525) < 0.0001 && exponent == 2;
    bool addPassed =
        addResult == uvec4(0xEB851EB8, 0x258BF257, 0x11111110, 0xFFFFFFFF) &&
        addCarry == uvec4(1, 0, 1, 0);
    bool subPassed = subResult == uvec2(0x26AF37BF, 0x12345678) && subBorrow == uvec2(1, 0);
    bool mulPassed =
        mulMsb == uvec4(0xEB9B208C, 0x01601D49, 0x121FA00A, 0x11E17CC0) &&
        mulLsb == uvec4(0xA83AB300, 0xD6B9FA88, 0x35068740, 0x822E97E0);
    bool imulPassed =
        imulMsb == ivec2(0xFFEB4992, 0xFE89E0E1) &&
        imulLsb == ivec2(0x35068740, 0x822E97E0);

    color = vec4(modfPassed ? 1 : 0,
                 frexpPassed ? 1 : 0,
                 (addPassed ? 0.4 : 0.0) + (subPassed ? 0.6 : 0.0),
                 (mulPassed ? 0.4 : 0.0) + (imulPassed ? 0.6 : 0.0));
})";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint floc  = glGetUniformLocation(program, "f");
    GLint u1loc = glGetUniformLocation(program, "u1");
    GLint u2loc = glGetUniformLocation(program, "u2");
    ASSERT_NE(floc, -1);
    ASSERT_NE(u1loc, -1);
    ASSERT_NE(u2loc, -1);
    glUniform1f(floc, 3.41);
    glUniform4ui(u1loc, 0xFEDCBA98u, 0x13579BDFu, 0xFEDCBA98u, 0x13579BDFu);
    glUniform4ui(u2loc, 0xECA86420u, 0x12345678u, 0x12345678u, 0xECA86420u);

    drawQuad(program, essl31_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
}

class GLSLTestLoops : public GLSLTest
{
  protected:
    void runTest(const char *fs)
    {
        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), fs);

        drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
};

// Test basic for loops
TEST_P(GLSLTestLoops, BasicFor)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 8; ++j)
        {
            for (int k = 0; k < 2; ++k, ++j) ++result;
            for (int k = 0; k < 3; ++k)      ++result;
            for (int k = 0; k < 0; ++k)      ++result;
        }

    color = result == 150 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test for loop without condition
TEST_P(GLSLTestLoops, ForNoCondition)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    for (int i = 0; i < 10; ++i)
        for (int j = 0; ; ++j)
        {
            for (int k = 0; k < 2; ++k, ++j) ++result;
            for (int k = 0; k < 3; ++k)      ++result;
            for (int k = 0; k < 0; ++k)      ++result;

            if (j >= 8)
                break;
        }

    color = result == 150 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test for loop without init and expression
TEST_P(GLSLTestLoops, ForNoInitConditionOrExpression)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    for (int i = 0; i < 10; ++i)
    {
        int j = 0;
        for (;;)
        {
            for (int k = 0; k < 2; ++k, ++j) ++result;
            for (int k = 0; k < 3; ++k)      ++result;
            for (int k = 0; k < 0; ++k)      ++result;

            if (j >= 8)
                break;
            ++j;
        }
    }

    color = result == 150 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test for loop with continue
TEST_P(GLSLTestLoops, ForContinue)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 8; ++j)
        {
            for (int k = 0; k < 2; ++k, ++j) ++result;
            for (int k = 0; k < 3; ++k)      ++result;
            if (i > 3)
                continue;
            for (int k = 0; k < 0; ++k)      ++result;
        }

    color = result == 150 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test for loop with continue at the end of block
TEST_P(GLSLTestLoops, ForUnconditionalContinue)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 8; ++j)
        {
            for (int k = 0; k < 2; ++k, ++j) ++result;
            for (int k = 0; k < 3; ++k)      ++result;
            for (int k = 0; k < 0; ++k)      ++result;
            continue;
        }

    color = result == 150 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test for loop with break at the end of block
TEST_P(GLSLTestLoops, ForUnconditionalBreak)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 8; ++j)
        {
            for (int k = 0; k < 2; ++k, ++j) ++result;
            for (int k = 0; k < 3; ++k)      ++result;
            for (int k = 0; k < 0; ++k)      ++result;
            break;
        }

    color = result == 50 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test for loop with break and continue
TEST_P(GLSLTestLoops, ForBreakContinue)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 8; ++j)
        {
            if (j < 2) continue;
            if (j > 6) break;
            if (i < 3) continue;
            if (i > 8) break;
            ++result;
        }

    color = result == 30 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test basic while loops
TEST_P(GLSLTestLoops, BasicWhile)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    while (i < 10)
    {
        int j = 0;
        while (j < 8)
        {
            int k = 0;
            while (k < 2) { ++result; ++k; ++j; }
            while (k < 5) { ++result; ++k; }
            while (k < 4) { ++result; }
            ++j;
        }
        ++i;
    }

    color = result == 150 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test while loops with continue
TEST_P(GLSLTestLoops, WhileContinue)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    while (i < 10)
    {
        int j = 0;
        while (j < 8)
        {
            int k = 0;
            while (k < 2) { ++result; ++k; ++j; }
            while (k < 5) { ++result; ++k; }
            if (i > 3)
            {
                ++j;
                continue;
            }
            while (k < 4) { ++result; }
            ++j;
        }
        ++i;
    }

    color = result == 150 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test while loops with continue at the end of block
TEST_P(GLSLTestLoops, WhileUnconditionalContinue)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    while (i < 10)
    {
        int j = 0;
        while (j < 8)
        {
            int k = 0;
            while (k < 2) { ++result; ++k; ++j; }
            while (k < 5) { ++result; ++k; }
            while (k < 4) { ++result; }
            ++j;
            continue;
        }
        ++i;
    }

    color = result == 150 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test while loops with break
TEST_P(GLSLTestLoops, WhileBreak)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    while (i < 10)
    {
        int j = 0;
        while (true)
        {
            int k = 0;
            while (k < 2) { ++result; ++k; ++j; }
            while (k < 5) { ++result; ++k; }
            while (k < 4) { ++result; }
            ++j;
            if (j >= 8)
                break;
        }
        ++i;
    }

    color = result == 150 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test while loops with continue at the end of block
TEST_P(GLSLTestLoops, WhileUnconditionalBreak)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    while (i < 10)
    {
        int j = 0;
        while (j < 8)
        {
            int k = 0;
            while (k < 2) { ++result; ++k; ++j; }
            while (k < 5) { ++result; ++k; }
            while (k < 4) { ++result; }
            ++j;
            break;
        }
        ++i;
    }

    color = result == 50 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test basic do-while loops
TEST_P(GLSLTestLoops, BasicDoWhile)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    do
    {
        int j = 0;
        do
        {
            int k = 0;
            do { ++result; ++k; ++j; } while (k < 2);
            do { ++result; ++k;      } while (k < 5);
            do { ++result;           } while (k < 3);
            ++j;
        } while (j < 8);
        ++i;
    } while (i < 10);

    color = result == 180 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test do-while loops with continue
TEST_P(GLSLTestLoops, DoWhileContinue)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    do
    {
        int j = 0;
        do
        {
            int k = 0;
            do { ++result; ++k; ++j; } while (k < 2);
            if (i > 3)
            {
                ++j;
                continue;
            }
            do { ++result; ++k;      } while (k < 5);
            do { ++result;           } while (k < 3);
            ++j;
        } while (j < 8);
        ++i;
    } while (i < 10);

    color = result == 108 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test do-while loops with continue at the end of block
TEST_P(GLSLTestLoops, DoWhileUnconditionalContinue)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    do
    {
        int j = 0;
        do
        {
            int k = 0;
            do { ++result; ++k; ++j; continue; } while (k < 2);
            do { ++result; ++k;      continue; } while (k < 5);
            do { ++result;           continue; } while (k < 3);
            ++j;
        } while (j < 8);
        ++i;
    } while (i < 10);

    color = result == 180 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test do-while loops with break
TEST_P(GLSLTestLoops, DoWhileBreak)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    do
    {
        int j = 0;
        do
        {
            int k = 0;
            do { ++result; ++k; ++j; } while (k < 2);
            do { ++result; ++k;      } while (k < 5);
            do { ++result;           } while (k < 3);
            ++j;
            if (j >= 8)
                break;
        } while (true);
        ++i;
    } while (i < 10);

    color = result == 180 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test do-while loops with break at the end of block
TEST_P(GLSLTestLoops, DoWhileUnconditionalBreak)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    do
    {
        int j = 0;
        do
        {
            int k = 0;
            do { ++result; ++k; ++j; break; } while (k < 2);
            do { ++result; ++k;      break; } while (k < 5);
            do { ++result;           break; } while (k < 3);
            ++j;
        } while (j < 8);
        ++i;
    } while (i < 10);

    color = result == 120 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test for loop with continue inside switch.
TEST_P(GLSLTestLoops, ForContinueInSwitch)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    for (int i = 0; i < 10; ++i)
        for (int j = 0; j < 8; ++j)
        {
            switch (j)
            {
                case 2:
                case 3:
                case 4:
                    ++result;
                    // fallthrough
                case 5:
                case 6:
                    ++result;
                    break;
                default:
                    continue;
            }
        }

    color = result == 80 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test while loop with continue inside switch
TEST_P(GLSLTestLoops, WhileContinueInSwitch)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    while (i < 10)
    {
        int j = 0;
        while (j < 8)
        {
            switch (j)
            {
                case 2:
                default:
                case 3:
                case 4:
                    ++j;
                    ++result;
                    continue;
                case 0:
                case 1:
                case 7:
                    break;
            }
            ++j;
        }
        ++i;
    }

    color = result == 50 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test do-while loops with continue in switch
TEST_P(GLSLTestLoops, DoWhileContinueInSwitch)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int result = 0;
    int i = 0;
    do
    {
        int j = 0;
        do
        {
            switch (j)
            {
                case 0:
                    ++j;
                    continue;
                default:
                case 2:
                case 3:
                case 4:
                    ++j;
                    ++result;
                    if (j >= 2 && j <= 6)
                        break;
                    else
                        continue;
            }
            ++result;
        } while (j < 8);
        ++i;
    } while (i < 10);

    color = result == 120 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test prune-able loop with side effect in statements.
TEST_P(GLSLTestLoops, SideEffectsInPrunableFor)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 color;

void main()
{
    int a = 4;
    float b = 0.;
    for (int c = a++; (b += float(c) / 8.) < 0.; b += 0.3)
    {
        if (2 == 0);
    }
    int c = a - 4;

    // Expect c to be 1 and b to be 0.5
    color = c == 1 && abs(b - 0.5) < 0.001 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    runTest(kFS);
}

// Test that precision is retained for constants (which are constant folded).  Adapted from a WebGL
// test.
TEST_P(GLSLTest, ConstantFoldedConstantsRetainPrecision)
{
    constexpr char kFS[] = R"(
// It is assumed that uTest is set to 0. It's here to make the expression not constant.
uniform mediump float uTest;
void main() {
    // exact representation of 4096.5 requires 13 bits of relative precision.
    const highp float c = 4096.5;
    mediump float a = 0.0;
    // Below, addition should be evaluated at highp, since one of the operands has the highp qualifier.
    // Thus fract should also be evaluated at highp.
    // See OpenGL ES Shading Language spec section 4.5.2.
    // This should make the result 0.5, since highp provides at least 16 bits of relative precision.
    // (exceptions for operation precision are allowed for a small number of computationally
    // intensive built-in functions, but it is reasonable to think that fract is not one of those).
    // However, if fract() is incorrectly evaluated at minimum precision fulfilling mediump criteria,
    // or at IEEE half float precision, the result is 0.0.
    a = fract(c + uTest);
    // Multiply by 2.0 to make the color green.
    gl_FragColor = vec4(0.0, 2.0 * a, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that vector and matrix scalarization does not affect rendering.
TEST_P(GLSLTest, VectorAndMatrixScalarizationDoesNotAffectRendering)
{
    constexpr char kFS[] = R"(
precision mediump float;

varying vec2 v_texCoord;

float a = 0.;
#define A 0.

#define r(a)    mat2( cos( a + vec4(0,-1.5708,1.5708,0) ) )
vec2 c;
#define f(U,a)  ( c = (U) * r(a) , sin(10.*c.x) )

void main() {
    vec2 U = v_texCoord;

    gl_FragColor = U.y > .5
        ? vec4( f(U,a) , f(U*4.,a) , 0,1.0)   // top
        : vec4( f(U,A) , f(U*4.,A) , 0,1.0);  // bottom
}

)";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    // Make sure we rendered something.
    EXPECT_PIXEL_NE(0, 0, 0, 0, 0, 0);

    // Comapare one line in top half to one line in bottom half.
    int compareWidth  = getWindowWidth();
    int compareHeight = getWindowHeight() / 4;

    ASSERT_GE(compareWidth, 2);
    ASSERT_GE(compareHeight, 2);

    GLubyte pixelValue[4];
    constexpr int tolerance = 12;

    for (int x = 0; x < compareWidth; ++x)
    {
        glReadPixels(x, compareHeight, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixelValue);
        EXPECT_PIXEL_NEAR(x, getWindowHeight() - compareHeight, pixelValue[0], pixelValue[1],
                          pixelValue[2], pixelValue[3], tolerance);
    }
    EXPECT_GL_NO_ERROR();
}

// Tests initializing a shader IO block using the shader translator option.
TEST_P(GLSLTest_ES31_InitShaderVariables, InitIOBlock)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    const char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in vec4 position;
out BlockType {
    vec4 blockMember;
} BlockTypeOut;

void main()
{
    gl_Position = position;
})";

    const char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
out vec4 colorOut;
in BlockType {
    vec4 blockMember;
} BlockTypeOut;

void main()
{
    if (BlockTypeOut.blockMember == vec4(0)) {
        colorOut = vec4(0, 1, 0, 1);
    } else {
        colorOut = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
    drawQuad(testProgram, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests initializing a nameless shader IO block using the shader translator option.
TEST_P(GLSLTest_ES31_InitShaderVariables, InitIOBlockNameless)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    const char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in vec4 position;
out BlockType {
    vec4 blockMember;
};

void main()
{
    gl_Position = position;
})";

    const char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
out vec4 colorOut;
in BlockType {
    vec4 blockMember;
};

void main()
{
    if (blockMember == vec4(0)) {
        colorOut = vec4(0, 1, 0, 1);
    } else {
        colorOut = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
    drawQuad(testProgram, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests initializing a shader IO block with an array using the shader translator option.
TEST_P(GLSLTest_ES31_InitShaderVariables, InitIOBlockWithArray)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    const char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in vec4 position;
out BlockType {
    vec4 blockMember[2];
} BlockTypeOut;

void main()
{
    gl_Position = position;
})";

    const char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
out vec4 colorOut;
in BlockType {
    vec4 blockMember[2];
} BlockTypeOut;

void main()
{
    if (BlockTypeOut.blockMember[0] == vec4(0) &&
        BlockTypeOut.blockMember[1] == vec4(0)) {
        colorOut = vec4(0, 1, 0, 1);
    } else {
        colorOut = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
    drawQuad(testProgram, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests initializing a shader IO block array using the shader translator option.
TEST_P(GLSLTest_ES31_InitShaderVariables, InitIOBlockArray)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    const char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in vec4 position;
out BlockType {
    vec4 blockMember;
} BlockTypeOut[2];

void main()
{
    gl_Position = position;
})";

    const char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
out vec4 colorOut;
in BlockType {
    vec4 blockMember;
} BlockTypeOut[2];

void main()
{
    if (BlockTypeOut[0].blockMember == vec4(0) &&
        BlockTypeOut[1].blockMember == vec4(0)) {
        colorOut = vec4(0, 1, 0, 1);
    } else {
        colorOut = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
    drawQuad(testProgram, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests initializing a shader IO block with a struct using the shader translator option.
TEST_P(GLSLTest_ES31_InitShaderVariables, InitIOBlockWithStruct)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    const char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in vec4 position;
struct s {
    float f;
    vec2 v;
};
out BlockType {
    s blockMember;
} BlockTypeOut;

void main()
{
    gl_Position = position;
})";

    const char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
out vec4 colorOut;
struct s {
    float f;
    vec2 v;
};
in BlockType {
    s blockMember;
} BlockTypeOut;

void main()
{
    if (BlockTypeOut.blockMember.f == 0.0 &&
        BlockTypeOut.blockMember.v == vec2(0)) {
        colorOut = vec4(0, 1, 0, 1);
    } else {
        colorOut = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
    drawQuad(testProgram, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests initializing an IO block with a complicated set of types, using the shader translator.
TEST_P(GLSLTest_ES31_InitShaderVariables, InitIOBlockWithComplexTypes)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_io_blocks"));

    const char kVS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
in vec4 position;
struct s {
    float f;
    vec2 v;
};
out BlockType {
    vec4 v;
    s s1;
    s s2[2];
} BlockTypeOut;

void main()
{
    gl_Position = position;
})";

    const char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_io_blocks : require
precision mediump float;
out vec4 colorOut;
struct s {
    float f;
    vec2 v;
};
in BlockType {
    vec4 v;
    s s1;
    s s2[2];
} BlockTypeOut;

void main()
{
    s sz = s(0.0, vec2(0));
    if (BlockTypeOut.v == vec4(0) &&
        BlockTypeOut.s1 == sz &&
        BlockTypeOut.s2[0] == sz &&
        BlockTypeOut.s2[1] == sz) {
        colorOut = vec4(0, 1, 0, 1);
    } else {
        colorOut = vec4(1, 0, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
    drawQuad(testProgram, "position", 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests an unsuccessful re-link using glBindAttribLocation.
TEST_P(GLSLTest_ES3, UnsuccessfulRelinkWithBindAttribLocation)
{
    // Make a simple program.
    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    // Install the executable.
    glUseProgram(testProgram);

    // Re-link with a bad XFB varying and a bound attrib location.
    const char *tfVaryings = "gl_FragColor";
    glTransformFeedbackVaryings(testProgram, 1, &tfVaryings, GL_SEPARATE_ATTRIBS);
    glBindAttribLocation(testProgram, 8, essl1_shaders::PositionAttrib());
    glLinkProgram(testProgram);
    GLint linkStatus = 999;
    glGetProgramiv(testProgram, GL_LINK_STATUS, &linkStatus);
    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(linkStatus, GL_FALSE);

    // Under normal GL this is not an error.
    glDrawArrays(GL_TRIANGLES, 79, 16);
    EXPECT_GL_NO_ERROR();
}

// Tests an unsuccessful re-link using glBindAttribLocation under WebGL.
TEST_P(WebGL2GLSLTest, UnsuccessfulRelinkWithBindAttribLocation)
{
    // Make a simple program.
    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Green());

    // Install the executable.
    glUseProgram(testProgram);

    // Re-link with a bad XFB varying and a bound attrib location.
    const char *tfVaryings = "gl_FragColor";
    glTransformFeedbackVaryings(testProgram, 1, &tfVaryings, GL_SEPARATE_ATTRIBS);
    glBindAttribLocation(testProgram, 8, essl1_shaders::PositionAttrib());
    glLinkProgram(testProgram);
    GLint linkStatus = 999;
    glGetProgramiv(testProgram, GL_LINK_STATUS, &linkStatus);
    ASSERT_GL_NO_ERROR();
    ASSERT_EQ(linkStatus, GL_FALSE);

    // Under WebGL this is an error.
    glDrawArrays(GL_TRIANGLES, 79, 16);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Covers a HLSL compiler bug.
TEST_P(GLSLTest_ES3, ComplexCrossExpression)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
vec3 a = vec3(0.0);
out vec4 color;
void main()
{
    cross(max(vec3(0.0), reflect(dot(a, vec3(0.0)), 0.0)), vec3(0.0));
})";

    ANGLE_GL_PROGRAM(testProgram, essl3_shaders::vs::Simple(), kFS);
    drawQuad(testProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
}

// Regression test for a crash in SPIR-V output when faced with an array of struct constant.
TEST_P(GLSLTest_ES3, ArrayOfStructConstantBug)
{
    constexpr char kFS[] = R"(#version 300 es
struct S {
    int foo;
};
void main() {
    S a[3];
    a = S[3](S(0), S(1), S(2));
})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Regression test for a bug in SPIR-V output where float+matrix was mishandled.
TEST_P(GLSLTest_ES3, FloatPlusMatrix)
{
    constexpr char kFS[] = R"(#version 300 es

precision mediump float;

layout(location=0) out vec4 color;

uniform float f;

void main()
{
    mat3x2 m = f + mat3x2(0);
    color = vec4(m[0][0]);
})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Regression test for a bug in SPIR-V output where a transformation creates float(constant) without
// folding it into a TIntermConstantUnion.  This transformation is clamping non-constant indices in
// WebGL.  The |false ? i : 5| as index caused the transformation to consider this a non-constant
// index.
TEST_P(WebGL2GLSLTest, IndexClampConstantIndexBug)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;

layout(location=0) out float f;

uniform int i;

void main()
{
    float data[10];
    f = data[false ? i : 5];
})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test robustness of out-of-bounds lod in texelFetch
TEST_P(WebGL2GLSLTest, TexelFetchLodOutOfBounds)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 vertexPosition;
void main() {
    gl_Position = vertexPosition;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform highp sampler2DArray textureArray;
uniform int textureLod;
out vec4 fragColor;
void main() {
    fragColor = texelFetch(textureArray, ivec3(gl_FragCoord.xy, 0), textureLod);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    const GLint lodLoc = glGetUniformLocation(program, "textureLod");
    EXPECT_NE(lodLoc, -1);
    const GLint textureLoc = glGetUniformLocation(program, "textureArray");
    EXPECT_NE(textureLoc, -1);

    const GLint attribLocation = glGetAttribLocation(program, "vertexPosition");
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    constexpr float vertices[12] = {
        -1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(attribLocation);
    glVertexAttribPointer(attribLocation, 2, GL_FLOAT, false, 0, 0);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 5, GL_RGBA8, 16, 16, 3);
    glUniform1i(textureLoc, 0);

    // Test LOD too large
    glUniform1i(lodLoc, 0x7FFF);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Test LOD negative
    glUniform1i(lodLoc, -1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Test that framebuffer fetch transforms gl_LastFragData in the presence of gl_FragCoord without
// failing validation (adapted from a Chromium test, see anglebug.com/42265427)
TEST_P(GLSLTest, FramebufferFetchWithLastFragData)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    constexpr char kFS[] = R"(#version 100

#extension GL_EXT_shader_framebuffer_fetch : require
varying mediump vec4 color;
void main() {
    gl_FragColor = length(gl_FragCoord.xy) * gl_LastFragData[0];
})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that loop body ending in a branch doesn't fail compilation
TEST_P(GLSLTest, LoopBodyEndingInBranch1)
{
    constexpr char kFS[] = R"(void main(){for(int a,i;;gl_FragCoord)continue;})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that loop body ending in a branch doesn't fail compilation
TEST_P(GLSLTest, LoopBodyEndingInBranch2)
{
    constexpr char kFS[] =
        R"(void main(){for(int a,i;bool(gl_FragCoord.x);gl_FragCoord){continue;}})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that loop body ending in a branch doesn't fail compilation
TEST_P(GLSLTest, LoopBodyEndingInBranch3)
{
    constexpr char kFS[] = R"(void main(){for(int a,i;;gl_FragCoord){{continue;}}})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that loop body ending in a branch doesn't fail compilation
TEST_P(GLSLTest, LoopBodyEndingInBranch4)
{
    constexpr char kFS[] = R"(void main(){for(int a,i;;gl_FragCoord){{continue;}{}{}{{}{}}}})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that loop body ending in a branch doesn't fail compilation
TEST_P(GLSLTest, LoopBodyEndingInBranch5)
{
    constexpr char kFS[] = R"(void main(){while(bool(gl_FragCoord.x)){{continue;{}}{}}})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that loop body ending in a branch doesn't fail compilation
TEST_P(GLSLTest, LoopBodyEndingInBranch6)
{
    constexpr char kFS[] = R"(void main(){do{{continue;{}}{}}while(bool(gl_FragCoord.x));})";

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *sourceArray[1] = {kFS};
    GLint lengths[1]           = {static_cast<GLint>(sizeof(kFS) - 1)};
    glShaderSource(shader, 1, sourceArray, lengths);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    EXPECT_NE(compileResult, 0);
}

// Test that aliasing function out parameters work.  The GLSL spec says:
//
// > Because the function works with local copies of parameters, there are no issues regarding
// > aliasing of variables within a function.
//
// In the test below, while the value of x is unknown after the function call, the result of the
// function must deterministically be true.
TEST_P(GLSLTest, AliasingFunctionOutParams)
{
    constexpr char kFS[] = R"(precision highp float;

const vec4 colorGreen = vec4(0.,1.,0.,1.);
const vec4 colorRed   = vec4(1.,0.,0.,1.);

bool outParametersAreDistinct(out float x, out float y) {
    x = 1.0;
    y = 2.0;
    return x == 1.0 && y == 2.0;
}
void main() {
    float x = 0.0;
    gl_FragColor = outParametersAreDistinct(x, x) ? colorGreen : colorRed;
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), kFS);
    drawQuad(testProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test that aliasing function out parameters work even when multiple params are aliased.
TEST_P(GLSLTest, AliasingFunctionOutParamsMultiple)
{
    constexpr char kFS[] = R"(precision highp float;

const vec4 colorGreen = vec4(0.,1.,0.,1.);
const vec4 colorRed   = vec4(1.,0.,0.,1.);

bool outParametersAreDistinct(out float x, out float y, out float z, out float a) {
    x = 1.0;
    y = 2.0;
    z = 3.0;
    a = 4.0;
    return x == 1.0 && y == 2.0 && z == 3.0 && a == 4.0;
}
void main() {
    float x = 0.0;
    float y = 0.0;
    gl_FragColor = outParametersAreDistinct(x, x, y, y) ? colorGreen : colorRed;
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), kFS);
    drawQuad(testProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test that aliasing function inout parameters work.
TEST_P(GLSLTest, AliasingFunctionInOutParams)
{
    constexpr char kFS[] = R"(precision highp float;

const vec4 colorGreen = vec4(0.,1.,0.,1.);
const vec4 colorRed   = vec4(1.,0.,0.,1.);

bool inoutParametersAreDistinct(inout float x, inout float y) {
    x = 1.0;
    y = 2.0;
    return x == 1.0 && y == 2.0;
}
void main() {
    float x = 0.0;
    gl_FragColor = inoutParametersAreDistinct(x, x) ? colorGreen : colorRed;
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), kFS);
    drawQuad(testProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test coverage of some matrix/scalar ops which Metal translation was missing.
TEST_P(GLSLTest, MatrixScalarOps)
{
    constexpr char kFS[] = R"(precision highp float;
void main() {
    float scalar = 0.5;
    mat3 matrix = mat3(vec3(0.1), vec3(0.1), vec3(0.1));

    mat3 m0 = scalar / matrix;
    mat3 m1 = scalar * matrix;
    mat3 m2 = scalar + matrix;
    mat3 m3 = scalar - matrix;

    gl_FragColor = vec4(m0[0][0], m1[0][0], m2[0][0], m3[0][0]);
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();

    drawQuad(testProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(255, 13, 153, 102), 1.0);
    ASSERT_GL_NO_ERROR();
}

// Test coverage of some matrix ops which Metal translation was missing.
TEST_P(GLSLTest, MatrixNegate)
{
    constexpr char kFS[] = R"(precision highp float;
void main() {
    mat3 matrix = mat3(vec3(-0.1), vec3(-0.1), vec3(-0.1));

    mat3 m0 = -matrix;

    gl_FragColor = vec4(m0[0][0], 0, 0, 1);
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();

    drawQuad(testProgram, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(26, 0, 0, 255), 1.0);
    ASSERT_GL_NO_ERROR();
}

// Test coverage of the mix(float, float, bool) overload which was missing in Metal translation
TEST_P(GLSLTest_ES3, MixFloatFloatBool)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 fragColor;
void main() {
    vec4 testData = vec4(0.0, 1.0, 0.5, 0.25);
    float scalar = mix(testData.x, testData.y, testData.x < 0.5);
    vec2 vector = mix(testData.xy, testData.xw, bvec2(testData.x < 0.5, testData.y < 0.5));
    fragColor = vec4(scalar, vector.x, vector.y, 1);
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl3_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();

    drawQuad(testProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(255, 0, 255, 255));
    ASSERT_GL_NO_ERROR();
}

// Test coverage of the mix(uint, uint, bool) overload which was missing in D3D11 translation
TEST_P(GLSLTest_ES31, MixUintUintBool)
{
    constexpr char kFS[] = R"(#version 310 es
precision highp float;
out vec4 fragColor;
void main() {
    uvec4 testData1 = uvec4(0, 1, 2, 3);
    uvec4 testData2 = uvec4(4, 5, 6, 7);
    uint scalar = mix(testData1.x, testData2.x, true);
    uvec4 vector = mix(testData1, testData2, bvec4(false, true, true, false));
    fragColor = vec4(scalar == 4u ? 1.0 : 0.0, vector == uvec4(0, 5, 6, 3) ? 1.0 : 0.0, 0.0, 1.0);
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();

    drawQuad(testProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(255, 255, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test coverage of the mix(int, int, bool) overload which was missing in D3D11 translation
TEST_P(GLSLTest_ES31, MixIntIntBool)
{
    constexpr char kFS[] = R"(#version 310 es
precision highp float;
out vec4 fragColor;
void main() {
    ivec4 testData1 = ivec4(-4, -3, -2, -1);
    ivec4 testData2 = ivec4(4, 5, 6, 7);
    int scalar = mix(testData1.x, testData2.x, true);
    ivec4 vector = mix(testData1, testData2, bvec4(false, true, true, false));
    fragColor = vec4(scalar == 4 ? 1.0 : 0.0, vector == ivec4(-4, 5, 6, -1) ? 1.0 : 0.0, 0.0, 1.0);
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();

    drawQuad(testProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(255, 255, 0, 255));
    ASSERT_GL_NO_ERROR();
}

// Test that aliasing function inout parameters work when more than one param is aliased.
TEST_P(GLSLTest, AliasingFunctionInOutParamsMultiple)
{
    constexpr char kFS[] = R"(precision highp float;

const vec4 colorGreen = vec4(0.,1.,0.,1.);
const vec4 colorRed   = vec4(1.,0.,0.,1.);

bool inoutParametersAreDistinct(inout float x, inout float y, inout float z, inout float a) {
    x = 1.0;
    y = 2.0;
    z = 3.0;
    a = 4.0;
    return x == 1.0 && y == 2.0 && z == 3.0 && a == 4.0;
}
void main() {
    float x = 0.0;
    float y = 0.0;
    gl_FragColor = inoutParametersAreDistinct(x, x, y, y) ? colorGreen : colorRed;
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), kFS);
    drawQuad(testProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test that aliasing function out parameter with a global works.
TEST_P(GLSLTest, AliasingFunctionOutParamAndGlobal)
{
    constexpr char kFS[] = R"(precision highp float;

const vec4 colorGreen = vec4(0.,1.,0.,1.);
const vec4 colorRed   = vec4(1.,0.,0.,1.);

float x = 1.0;
bool outParametersAreDistinctFromGlobal(out float y) {
    y = 2.0;
    return x == 1.0 && y == 2.0;
}
void main() {
    gl_FragColor = outParametersAreDistinctFromGlobal(x) ? colorGreen : colorRed;
}
)";

    ANGLE_GL_PROGRAM(testProgram, essl1_shaders::vs::Simple(), kFS);
    drawQuad(testProgram, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Make sure const sampler parameters work.
TEST_P(GLSLTest, ConstSamplerParameter)
{
    constexpr char kFS[] = R"(precision mediump float;
uniform sampler2D samp;

vec4 sampleConstSampler(const sampler2D s) {
    return texture2D(s, vec2(0));
}

void main() {
    gl_FragColor = sampleConstSampler(samp);
}
)";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture texture;
    GLColor expected = MakeGLColor(32, 64, 96, 255);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, expected.data());
    GLint u = glGetUniformLocation(program, "samp");
    EXPECT_NE(u, -1);
    glUniform1i(u, 0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, expected);
    ASSERT_GL_NO_ERROR();
}

// Make sure const sampler parameters work.
TEST_P(GLSLTest, ConstInSamplerParameter)
{
    constexpr char kFS[] = R"(precision mediump float;
uniform sampler2D u;
vec4 sampleConstSampler(const in sampler2D s) {
    return texture2D(s, vec2(0));
}
void main() {
    gl_FragColor = sampleConstSampler(u);
}
)";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture texture;
    GLColor expected = MakeGLColor(32, 64, 96, 255);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, expected.data());
    GLint u = glGetUniformLocation(program, "u");
    EXPECT_NE(u, -1);
    glUniform1i(u, 0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, expected);
    ASSERT_GL_NO_ERROR();
}

// Make sure passing const sampler parameters to another function work.
TEST_P(GLSLTest, ConstSamplerParameterAsArgument)
{
    constexpr char kFS[] = R"(precision mediump float;

uniform sampler2D samp;

vec4 sampleSampler(sampler2D s) {
    return texture2D(s, vec2(0));
}

vec4 sampleConstSampler(const sampler2D s) {
    return sampleSampler(s);
}

void main() {
    gl_FragColor = sampleConstSampler(samp);
}
)";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    GLTexture texture;
    GLColor expected = MakeGLColor(32, 64, 96, 255);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, expected.data());
    GLint u = glGetUniformLocation(program, "samp");
    EXPECT_NE(u, -1);
    glUniform1i(u, 0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, expected);
    ASSERT_GL_NO_ERROR();
}

// Test for a driver bug with matrix multiplication in the tessellation control shader.
TEST_P(GLSLTest_ES31, TessellationControlShaderMatrixMultiplicationBug)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));

    constexpr char kTCS[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : enable
layout(vertices = 1) out;
precision highp float;

patch out mat4 x;

void main()
{
    x = mat4(
        0.53455, 0.47307, 0.34935, 0.28717,
        0.67195, 0.59992, 0.48213, 0.43678,
        0.76376, 0.6772, 0.55361, 0.5165,
        0.77996, 0.68862, 0.56187, 0.52611
    );

    const mat4 m = mat4(
        vec4( -1.0, 3.0,-3.0, 1.0),
        vec4(  3.0,-6.0, 3.0, 0.0),
        vec4( -3.0, 3.0, 0.0, 0.0),
        vec4(  1.0, 0.0, 0.0, 0.0)
    );

    x = m * x;

    gl_TessLevelInner[0u] = 1.;
    gl_TessLevelInner[1u] = 1.;
    gl_TessLevelOuter[0u] = 1.;
    gl_TessLevelOuter[1u] = 1.;
    gl_TessLevelOuter[2u] = 1.;
    gl_TessLevelOuter[3u] = 1.;
})";

    constexpr char kTES[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : enable
layout(quads, cw, fractional_odd_spacing) in;
precision highp float;

patch in mat4 x;

out mat4 x_fs;

void main()
{
    x_fs = x;
    gl_Position = vec4(gl_TessCoord.xy * 2. - 1., 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;

in mat4 x_fs;
out vec4 color;

void main()
{
    // Note: on the failing driver, .w of every column has the same value as .x of the same column.

    const mat4 expect = mat4(
        0.12378, -0.18672, -0.18444, 0.53455,
        0.1182, -0.13728, -0.21609, 0.67195,
        0.12351, -0.11109, -0.25968, 0.76376,
        0.1264, -0.10623, -0.27402, 0.77996
    );

    color = vec4(all(lessThan(abs(x_fs[0] - expect[0]), vec4(0.01))),
                 all(lessThan(abs(x_fs[1] - expect[1]), vec4(0.01))),
                 all(lessThan(abs(x_fs[2] - expect[2]), vec4(0.01))),
                 all(lessThan(abs(x_fs[3] - expect[3]), vec4(0.01))));
})";

    ANGLE_GL_PROGRAM_WITH_TESS(program, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    drawPatches(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, GL_FALSE);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
    ASSERT_GL_NO_ERROR();
}

// Test for a driver bug with matrix copy in the tessellation control shader.
TEST_P(GLSLTest_ES31, TessellationControlShaderMatrixCopyBug)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));

    constexpr char kTCS[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : enable
layout(vertices = 1) out;
precision highp float;

patch out mat4 x;
patch out vec4 col0;

void main()
{
    // Note: if |x| is not an |out| varying, the test passes.
    x = mat4(
        0.53455, 0.47307, 0.34935, 0.28717,
        0.67195, 0.59992, 0.48213, 0.43678,
        0.76376, 0.6772, 0.55361, 0.5165,
        0.77996, 0.68862, 0.56187, 0.52611
    );

    const mat4 m = mat4(
        vec4( -1.0, 3.0,-3.0, 1.0),
        vec4(  3.0,-6.0, 3.0, 0.0),
        vec4( -3.0, 3.0, 0.0, 0.0),
        vec4(  1.0, 0.0, 0.0, 0.0)
    );

    mat4 temp = x;

    // Note: On the failing driver, commenting this line makes the test pass.
    // However, the output being tested is |temp|, assigned above, not |x|.
    x = m * x;

    col0 = temp[0];

    gl_TessLevelInner[0u] = 1.;
    gl_TessLevelInner[1u] = 1.;
    gl_TessLevelOuter[0u] = 1.;
    gl_TessLevelOuter[1u] = 1.;
    gl_TessLevelOuter[2u] = 1.;
    gl_TessLevelOuter[3u] = 1.;
})";

    constexpr char kTES[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : enable
layout(quads, cw, fractional_odd_spacing) in;
precision highp float;

patch in vec4 col0;

out vec4 col0_fs;

void main()
{
    col0_fs = col0;
    gl_Position = vec4(gl_TessCoord.xy * 2. - 1., 0, 1);
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;

in vec4 col0_fs;
out vec4 color;

void main()
{
    // Note: on the failing driver, |col0| has the value of |m * x|, not |temp|.
    color = vec4(abs(col0_fs.x - 0.53455) < 0.01,
                abs(col0_fs.y - 0.47307) < 0.01,
                abs(col0_fs.z - 0.34935) < 0.01,
                abs(col0_fs.w - 0.28717) < 0.01);
})";

    ANGLE_GL_PROGRAM_WITH_TESS(program, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    drawPatches(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, GL_FALSE);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::white);
    ASSERT_GL_NO_ERROR();
}

// Tests the generation of HLSL functions with uint/int parameters that may be ambiguous.
TEST_P(GLSLTest_ES3, AmbiguousHLSLIntegerFunctionParameters)
{
    const char kVS[] = R"(#version 300 es
precision highp float;
void main()
{
    gl_Position = vec4(0, 0, 0, 0);
})";

    const char kFS[] = R"(#version 300 es
precision highp float;
out vec4 color;
void main()
{
    // Ensure that both uint and int to float constructors are generated before the ambiguous usage.
    int i = int(gl_FragCoord.x);
    float f1 = float(i);
    color.r = f1;

    uint ui = uint(gl_FragCoord.x);
    float f2 = float(i);
    color.g = f2;

    // Ambiguous call
    float f3 = float(1u << (2u * ui));
    color.b = f3;
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
}

// Tests adding a struct definition inline in a shader.
// Metal backend contains a pass that separates struct definition and declaration.
TEST_P(GLSLTest_ES3, StructInShader)
{
    const char kVS[] = R"(#version 300 es
precision highp float;
void main(void)
{
    struct structMain {
        float i;
    } testStruct;

    testStruct.i = 5.0 ;
    gl_Position = vec4(testStruct.i - 4.0, 0, 0, 1);
})";

    const char kFS[] = R"(#version 300 es
precision highp float;
out vec4 color;
void main()
{
    color = vec4(0,1,0,0);
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
}

// Issue: A while loop's expression, and a branch
// condition with EOpContinue were being deep
// copied as part of monomorphize functions,
// causing a crash, as they were not null-checked.
// Tests transforming a function that will be monomorphized.
TEST_P(GLSLTest_ES3, MonomorphizeForAndContinue)
{

    constexpr char kFS[] =
        R"(#version 300 es
        
        precision mediump float;
        out vec4 fragOut;
        struct aParam
        {
            sampler2D sampler;
        };
        uniform aParam theParam;

        float monomorphizedFunction(aParam a)
        {
            int i = 0;
            vec4 j = vec4(0);
            for(;;)
            {
                if(i++ < 10)
                {
                    j += texture(a.sampler, vec2(0.0f,0.0f));
                    continue;
                }
                break;
            }
            return j.a;
        }
        void main()
        {
            fragOut.a = monomorphizedFunction(theParam);
        }        
)";
    CompileShader(GL_FRAGMENT_SHADER, kFS);
    ASSERT_GL_NO_ERROR();
}

// Tests inout parameters with array references.
TEST_P(GLSLTest_ES3, InoutWithArrayRefs)
{
    const char kVS[] = R"(#version 300 es
precision highp float;
void swap(inout float a, inout float b)
{
    float tmp = a;
    a = b;
    b = tmp;
}

void main(void)
{
    vec3 testVec = vec3(0.0, 1.0, 1.0);
    swap(testVec[0], testVec[1]);
    gl_Position = vec4(testVec[0], testVec[1], testVec[2], 1.0);
})";

    const char kFS[] = R"(#version 300 es
precision highp float;
out vec4 color;
void main()
{
    color = vec4(0,1,0,0);
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
}

// Test that sample variables compile.
TEST_P(GLSLTest_ES3, SampleVariables)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_sample_variables"));

    const char kFS[] = R"(#version 300 es
#extension GL_OES_sample_variables : require
precision highp float;
out vec4 color;
void main()
{
    gl_SampleMask[0] = gl_SampleMaskIn[0] & 0x55555555;
    color = vec4(gl_SamplePosition.yx, float(gl_SampleID), float(gl_MaxSamples + gl_NumSamples));
})";

    ANGLE_GL_PROGRAM(testProgram, essl3_shaders::vs::Simple(), kFS);
}

// Test that shader caching maintains uniforms across compute shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheComputeWithUniform)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    constexpr char kCS[] = R"(#version 310 es
layout (local_size_x = 2, local_size_y = 3, local_size_z = 1) in;

uniform uint inputs[6];

layout (binding = 0, std430) buffer OutputBuffer {
    uint outputs[6];
};

void main() {
    outputs[gl_LocalInvocationIndex] = inputs[gl_LocalInvocationIndex];
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(unusedProgram, kCS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr std::array<GLuint, 6> kInputUniform = {1, 2, 3, 4, 5, 6};
    for (int i = 0; i < static_cast<int>(kInputUniform.size()); ++i)
    {
        const std::string uniformName =
            std::string("inputs[") + std::to_string(i) + std::string("]");
        int inputLocation =
            glGetUniformLocation(program, static_cast<const GLchar *>(uniformName.c_str()));
        glUniform1ui(inputLocation, kInputUniform[i]);
        ASSERT_GL_NO_ERROR();
    }

    constexpr std::array<GLuint, 6> kOutputInitData = {0, 0, 0, 0, 0, 0};
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * kOutputInitData.size(),
                 kOutputInitData.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, outputBuffer);

    glDispatchCompute(1, 1, 1);
    glDeleteProgram(program);
    ASSERT_GL_NO_ERROR();
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    auto outputData = static_cast<const GLuint *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * kOutputInitData.size(), GL_MAP_READ_BIT));
    for (int i = 0; i < static_cast<int>(kInputUniform.size()); ++i)
    {
        EXPECT_EQ(kInputUniform[i], outputData[i]);
    }
}

// Test that shader caching maintains uniform blocks across shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheComputeWithUniformBlocks)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    constexpr char kCS[] = R"(#version 310 es
layout (local_size_x = 2, local_size_y = 3, local_size_z = 1) in;

layout (std140) uniform Input1 {
    uint input1;
};

layout (std140) uniform Input2 {
    uint input2;
};

layout (binding = 0, std430) buffer OutputBuffer {
    uint outputs[6];
};

void main() {
    if (gl_LocalInvocationIndex < uint(3))
    {
        outputs[gl_LocalInvocationIndex] = input1;
    }
    else
    {
        outputs[gl_LocalInvocationIndex] = input2;
    }
}
)";

    ANGLE_GL_COMPUTE_PROGRAM(unusedProgram, kCS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_COMPUTE_PROGRAM(program, kCS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLuint kInput1Data = 1;
    GLBuffer input1;
    glBindBuffer(GL_UNIFORM_BUFFER, input1);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLuint), &kInput1Data, GL_STATIC_COPY);
    const GLuint kInput1Index = glGetUniformBlockIndex(program, "Input1");
    glUniformBlockBinding(program, kInput1Index, 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, input1);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    constexpr GLuint kInput2Data = 2;
    GLBuffer input2;
    glBindBuffer(GL_UNIFORM_BUFFER, input2);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLuint), &kInput2Data, GL_STATIC_COPY);
    const GLuint kInput2Index = glGetUniformBlockIndex(program, "Input2");
    glUniformBlockBinding(program, kInput2Index, 2);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, input2);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    constexpr std::array<GLuint, 6> kOutputInitData = {0, 0, 0, 0, 0, 0};
    GLBuffer outputBuffer;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * kOutputInitData.size(),
                 kOutputInitData.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, outputBuffer);
    ASSERT_GL_NO_ERROR();

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    glDeleteProgram(program);
    ASSERT_GL_NO_ERROR();

    auto outputData                       = static_cast<const GLuint *>(glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * kOutputInitData.size(), GL_MAP_READ_BIT));
    constexpr std::array<GLuint, 6> kWant = {kInput1Data, kInput1Data, kInput1Data,
                                             kInput2Data, kInput2Data, kInput2Data};
    for (int i = 0; i < static_cast<int>(kWant.size()); ++i)
    {
        EXPECT_EQ(kWant[i], outputData[i]);
    }
}

// Test that shader caching maintains uniforms across vertex shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheVertexWithUniform)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    constexpr char kVS[] = R"(#version 310 es

precision mediump float;

layout (location = 0) in vec4 a_position;

uniform float redInput;

out float redValue;

void main() {
    gl_Position = a_position;
    redValue = redInput;
})";

    constexpr char kFS[] = R"(#version 310 es

precision mediump float;

in float redValue;

out vec4 fragColor;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM(unusedProgram, kVS, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kRedValue = 1.0f;
    int redInputLocation        = glGetUniformLocation(program, "redInput");
    glUniform1f(redInputLocation, kRedValue);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains uniform blocks across vertex shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheVertexWithUniformBlock)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    constexpr char kVS[] = R"(#version 310 es

precision mediump float;

layout (location = 0) in vec4 a_position;

layout (std140) uniform Input {
    float redInput;
};

out float redValue;

void main() {
    gl_Position = a_position;
    redValue = redInput;
})";

    constexpr char kFS[] = R"(#version 310 es

precision mediump float;

in float redValue;

out vec4 fragColor;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM(unusedProgram, kVS, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_UNIFORM_BUFFER, input);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    const GLuint kInputIndex = glGetUniformBlockIndex(program, "Input");
    glUniformBlockBinding(program, kInputIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, input);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains SSBOs across vertex shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheVertexWithSSBO)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    // Check that GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS is at least 1.
    GLint maxVertexShaderStorageBlocks;
    glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &maxVertexShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxVertexShaderStorageBlocks == 0);
    constexpr char kVS[] = R"(#version 310 es

precision mediump float;

layout (location = 0) in vec4 a_position;

layout (binding = 0, std430) buffer Input {
    float redInput;
};

out float redValue;

void main() {
    gl_Position = a_position;
    redValue = redInput;
})";

    constexpr char kFS[] = R"(#version 310 es

precision mediump float;

in float redValue;

out vec4 fragColor;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM(unusedProgram, kVS, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains uniforms across vertex shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheFragmentWithUniform)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    constexpr char kFS[] = R"(#version 310 es

precision mediump float;

uniform float redValue;

out vec4 fragColor;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM(unusedProgram, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    GLfloat redValue     = 1.0f;
    int redInputLocation = glGetUniformLocation(program, "redValue");
    glUniform1f(redInputLocation, redValue);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains uniform blocks across vertex shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheFragmentWithUniformBlock)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    constexpr char kFS[] = R"(#version 310 es

precision mediump float;

layout (std140) uniform Input {
    float redValue;
};

out vec4 fragColor;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM(unusedProgram, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_UNIFORM_BUFFER, input);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    const GLuint kInputIndex = glGetUniformBlockIndex(program, "Input");
    glUniformBlockBinding(program, kInputIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, input);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains SSBOs across vertex shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheFragmentWithSSBO)
{
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    constexpr char kFS[] = R"(#version 310 es

precision mediump float;

layout (binding = 0, std430) buffer Input {
    float redValue;
};

out vec4 fragColor;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM(unusedProgram, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains whether GL_ARB_sample_shading is enabled across shader
// compilations.
TEST_P(GLSLTest_ES31, ShaderCacheFragmentWithARBSampleShading)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ARB_sample_shading"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kFS[] = R"(#version 310 es
#extension GL_ARB_sample_shading : enable

precision mediump float;

out vec4 fragColor;

void main()
{
#ifdef GL_ARB_sample_shading
    fragColor = vec4(1., 0., 0., 1.);
#else
    fragColor = vec4(0.);
#endif
})";

    ANGLE_GL_PROGRAM(unusedProgram, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains which advanced blending equations (provided by
// GL_KHR_blend_equation_advanced) are used across shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheFragmentWithKHRAdvancedBlendEquations)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_KHR_blend_equation_advanced"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kFS[] = R"(#version 310 es
#extension GL_KHR_blend_equation_advanced : require

layout (blend_support_multiply) out;

precision mediump float;

out vec4 fragColor;

void main()
{
    fragColor = vec4(1., 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM(unusedProgram, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains uniforms in geometry shaders across shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheGeometryWithUniform)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
precision mediump float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform float redInput;

out float redValue;

void main() {
    gl_Position = gl_in[0].gl_Position;
    redValue = redInput;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    redValue = redInput;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    redValue = redInput;
    EmitVertex();

    EndPrimitive();
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_GS(unusedProgram, essl31_shaders::vs::Simple(), kGS, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_GS(program, essl31_shaders::vs::Simple(), kGS, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kRedValue = 1.0f;
    int redInputLocation        = glGetUniformLocation(program, "redInput");
    glUniform1f(redInputLocation, kRedValue);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains uniform blocks in geometry shaders across shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheGeometryWithUniformBlock)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
precision mediump float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (std140) uniform Input {
    float redInput;
};

out float redValue;

void main() {
    gl_Position = gl_in[0].gl_Position;
    redValue = redInput;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    redValue = redInput;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    redValue = redInput;
    EmitVertex();

    EndPrimitive();
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_GS(unusedProgram, essl31_shaders::vs::Simple(), kGS, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_GS(program, essl31_shaders::vs::Simple(), kGS, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_UNIFORM_BUFFER, input);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    const GLuint kInputIndex = glGetUniformBlockIndex(program, "Input");
    glUniformBlockBinding(program, kInputIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, input);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains SSBO in geometry shaders across shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheGeometryWithSSBO)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    GLint maxGeometryShaderStorageBlocks = 0;
    glGetIntegerv(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_EXT, &maxGeometryShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxGeometryShaderStorageBlocks == 0);

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
precision mediump float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout (binding = 0, std430) buffer Input {
    float redInput;
};

out float redValue;

void main() {
    gl_Position = gl_in[0].gl_Position;
    redValue = redInput;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    redValue = redInput;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    redValue = redInput;
    EmitVertex();

    EndPrimitive();
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_GS(unusedProgram, essl31_shaders::vs::Simple(), kGS, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_GS(program, essl31_shaders::vs::Simple(), kGS, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains the number of invocations in geometry shaders across shader
// compilations.
TEST_P(GLSLTest_ES31, ShaderCacheGeometryWithInvocations)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_geometry_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kGS[] = R"(#version 310 es
#extension GL_EXT_geometry_shader : require
precision mediump float;

layout (triangles, invocations = 2) in;
layout (triangle_strip, max_vertices = 3) out;

out float redValue;

void main() {
    float redOut = 0.;
    if (gl_InvocationID == 1) {
        redOut = 1.;
    }

    gl_Position = gl_in[0].gl_Position;
    redValue = redOut;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    redValue = redOut;
    EmitVertex();

    gl_Position = gl_in[2].gl_Position;
    redValue = redOut;
    EmitVertex();

    EndPrimitive();
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_GS(unusedProgram, essl31_shaders::vs::Simple(), kGS, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_GS(program, essl31_shaders::vs::Simple(), kGS, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains uniforms in tessellation control shaders across shader
// compilations.
TEST_P(GLSLTest_ES31, ShaderCacheTessellationControlWithUniform)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kTCS[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (vertices = 1) out;

uniform float redInput;

patch out float redValueCS;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = 1.0;
    gl_TessLevelOuter[0] = 1.0;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = 1.0;
    gl_TessLevelOuter[3] = 1.0;

    redValueCS = redInput;
}

)";

    constexpr char kTES[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (quads, cw, fractional_odd_spacing) in;

patch in float redValueCS;

out float redValue;

void main()
{
    gl_Position = vec4(gl_TessCoord.xy * 2. - 1., 0, 1);

    redValue = redValueCS;
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_TESS(unusedProgram, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_TESS(program, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kRedValue = 1.0f;
    int redInputLocation        = glGetUniformLocation(program, "redInput");
    glUniform1f(redInputLocation, kRedValue);
    ASSERT_GL_NO_ERROR();

    drawPatches(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains uniform blocks in tessellation control shaders across shader
// compilations.
TEST_P(GLSLTest_ES31, ShaderCacheTessellationControlWithUniformBlock)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kTCS[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (vertices = 1) out;

layout (std140) uniform Input {
    float redInput;
};

patch out float redValueCS;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = 1.0;
    gl_TessLevelOuter[0] = 1.0;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = 1.0;
    gl_TessLevelOuter[3] = 1.0;

    redValueCS = redInput;
}

)";

    constexpr char kTES[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (quads, cw, fractional_odd_spacing) in;

patch in float redValueCS;

out float redValue;

void main()
{
    gl_Position = vec4(gl_TessCoord.xy * 2. - 1., 0, 1);

    redValue = redValueCS;
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_TESS(unusedProgram, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_TESS(program, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_UNIFORM_BUFFER, input);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    const GLuint kInputIndex = glGetUniformBlockIndex(program, "Input");
    glUniformBlockBinding(program, kInputIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, input);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawPatches(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains SSBOs in tessellation control shaders across shader
// compilations.
TEST_P(GLSLTest_ES31, ShaderCacheTessellationControlWithSSBO)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    GLint maxTessControlShaderStorageBlocks;
    glGetIntegerv(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS_EXT,
                  &maxTessControlShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxTessControlShaderStorageBlocks == 0);

    constexpr char kTCS[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (vertices = 1) out;

layout (binding = 0, std430) buffer Input {
    float redInput;
};

patch out float redValueCS;

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    gl_TessLevelInner[0] = 1.0;
    gl_TessLevelInner[1] = 1.0;
    gl_TessLevelOuter[0] = 1.0;
    gl_TessLevelOuter[1] = 1.0;
    gl_TessLevelOuter[2] = 1.0;
    gl_TessLevelOuter[3] = 1.0;

    redValueCS = redInput;
}

)";

    constexpr char kTES[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (quads, cw, fractional_odd_spacing) in;

patch in float redValueCS;

out float redValue;

void main()
{
    gl_Position = vec4(gl_TessCoord.xy * 2. - 1., 0, 1);

    redValue = redValueCS;
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_TESS(unusedProgram, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_TESS(program, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawPatches(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains uniforms in tessellation evaluation shaders across shader
// compilations.
TEST_P(GLSLTest_ES31, ShaderCacheTessellationEvalWithUniform)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kTCS[] = R"(#version 310 es
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

    constexpr char kTES[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (quads, cw, fractional_odd_spacing) in;

uniform float redInput;

out float redValue;

void main()
{
    gl_Position = vec4(gl_TessCoord.xy * 2. - 1., 0, 1);

    redValue = redInput;
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_TESS(unusedProgram, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_TESS(program, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kRedValue = 1.0f;
    int redInputLocation        = glGetUniformLocation(program, "redInput");
    glUniform1f(redInputLocation, kRedValue);
    ASSERT_GL_NO_ERROR();

    drawPatches(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains uniform blocks in tessellation evaluation shaders across
// shader compilations.
TEST_P(GLSLTest_ES31, ShaderCacheTessellationEvalWithUniformBlock)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kTCS[] = R"(#version 310 es
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

    constexpr char kTES[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (quads, cw, fractional_odd_spacing) in;

layout (std140) uniform Input {
    float redInput;
};

out float redValue;

void main()
{
    gl_Position = vec4(gl_TessCoord.xy * 2. - 1., 0, 1);

    redValue = redInput;
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_TESS(unusedProgram, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_TESS(program, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_UNIFORM_BUFFER, input);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    const GLuint kInputIndex = glGetUniformBlockIndex(program, "Input");
    glUniformBlockBinding(program, kInputIndex, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, input);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawPatches(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that shader caching maintains SSBOs in tessellation evaluation shaders across shader
// compilations.
TEST_P(GLSLTest_ES31, ShaderCacheTessellationEvalWithSSBO)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_tessellation_shader"));
    ANGLE_SKIP_TEST_IF(!IsVulkan());
    GLint maxTessEvalShaderStorageBlocks;
    glGetIntegerv(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS_EXT,
                  &maxTessEvalShaderStorageBlocks);
    ANGLE_SKIP_TEST_IF(maxTessEvalShaderStorageBlocks == 0);

    constexpr char kTCS[] = R"(#version 310 es
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

    constexpr char kTES[] = R"(#version 310 es
#extension GL_EXT_tessellation_shader : require
precision mediump float;

layout (quads, cw, fractional_odd_spacing) in;

layout (binding = 0, std430) buffer Input {
    float redInput;
};

out float redValue;

void main()
{
    gl_Position = vec4(gl_TessCoord.xy * 2. - 1., 0, 1);

    redValue = redInput;
}
)";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;

out vec4 fragColor;

in float redValue;

void main()
{
    fragColor = vec4(redValue, 0., 0., 1.);
})";

    ANGLE_GL_PROGRAM_WITH_TESS(unusedProgram, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    // Delete the shader and recompile to fetch from cache.
    glDeleteProgram(unusedProgram);
    ANGLE_GL_PROGRAM_WITH_TESS(program, essl31_shaders::vs::Simple(), kTCS, kTES, kFS);
    ASSERT_GL_NO_ERROR();
    glUseProgram(program);

    constexpr GLfloat kInputData = 1.0f;
    GLBuffer input;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, input);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat), &kInputData, GL_STATIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, input);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ASSERT_GL_NO_ERROR();

    drawPatches(program, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, GL_FALSE);
    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that only macros for ESSL 1.0 compatible extensions are defined
TEST_P(GLSLTest, ESSL1ExtensionMacros)
{
    std::string fs = essl1_shaders::fs::Red();
    fs += ExpectedExtensionMacros({
        "GL_ANGLE_multi_draw",
        "GL_APPLE_clip_distance",
        "GL_ARB_texture_rectangle",
        "GL_ARM_shader_framebuffer_fetch",
        "GL_EXT_blend_func_extended",
        "GL_EXT_draw_buffers",
        "GL_EXT_frag_depth",
        "GL_EXT_separate_shader_objects",
        "GL_EXT_shader_framebuffer_fetch_non_coherent",
        "GL_EXT_shader_framebuffer_fetch",
        "GL_EXT_shader_non_constant_global_initializers",
        "GL_EXT_shader_texture_lod",
        "GL_EXT_shadow_samplers",
        "GL_KHR_blend_equation_advanced",
        "GL_NV_EGL_stream_consumer_external",
        "GL_NV_shader_framebuffer_fetch",
        "GL_OES_EGL_image_external",
        "GL_OES_standard_derivatives",
        "GL_OES_texture_3D",
        "GL_WEBGL_video_texture",
    });
    fs += UnexpectedExtensionMacros({
        "GL_ANDROID_extension_pack_es31a",
        "GL_ANGLE_base_vertex_base_instance_shader_builtin",
        "GL_ANGLE_clip_cull_distance",
        "GL_ANGLE_shader_pixel_local_storage",
        "GL_ANGLE_texture_multisample",
        "GL_EXT_clip_cull_distance",
        "GL_EXT_geometry_shader",
        "GL_EXT_gpu_shader5",
        "GL_EXT_primitive_bounding_box",
        "GL_EXT_shader_io_blocks",
        "GL_EXT_tessellation_shader",
        "GL_EXT_texture_buffer",
        "GL_EXT_texture_cube_map_array",
        "GL_EXT_YUV_target",
        "GL_NV_shader_noperspective_interpolation",
        "GL_OES_EGL_image_external_essl3",
        "GL_OES_geometry_shader",
        "GL_OES_primitive_bounding_box",
        "GL_OES_sample_variables",
        "GL_OES_shader_image_atomic",
        "GL_OES_shader_io_blocks",
        "GL_OES_shader_multisample_interpolation",
        "GL_OES_texture_buffer",
        "GL_OES_texture_cube_map_array",
        "GL_OES_texture_storage_multisample_2d_array",
        "GL_OVR_multiview",
        "GL_OVR_multiview2",
    });
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), fs.c_str());
    ASSERT_GL_NO_ERROR();
}

// Test that only macros for ESSL 3.0 compatible extensions are defined
TEST_P(GLSLTest_ES3, ESSL3ExtensionMacros)
{
    std::string fs = essl3_shaders::fs::Red();
    fs += ExpectedExtensionMacros({
        "GL_ANGLE_base_vertex_base_instance_shader_builtin",
        "GL_ANGLE_clip_cull_distance",
        "GL_ANGLE_multi_draw",
        "GL_ANGLE_shader_pixel_local_storage",
        "GL_ANGLE_texture_multisample",
        "GL_APPLE_clip_distance",
        "GL_ARB_texture_rectangle",
        "GL_ARM_shader_framebuffer_fetch",
        "GL_EXT_blend_func_extended",
        "GL_EXT_clip_cull_distance",
        "GL_EXT_separate_shader_objects",
        "GL_EXT_shader_framebuffer_fetch_non_coherent",
        "GL_EXT_shader_framebuffer_fetch",
        "GL_EXT_shader_non_constant_global_initializers",
        "GL_EXT_YUV_target",
        "GL_KHR_blend_equation_advanced",
        "GL_NV_EGL_stream_consumer_external",
        "GL_NV_shader_noperspective_interpolation",
        // Enabled on ESSL 3+ to workaround app bug. http://issuetracker.google.com/285871779
        "GL_OES_EGL_image_external",
        "GL_OES_EGL_image_external_essl3",
        // Enabled on ESSL 3+ to workaround app bug. http://issuetracker.google.com/285871779
        "GL_OES_texture_3D",
        "GL_OES_sample_variables",
        "GL_OES_shader_multisample_interpolation",
        // Enabled on ESSL 3+ because ANGLE can support multisample textures with ES 3.0 contexts.
        "GL_OES_texture_storage_multisample_2d_array",
        "GL_OVR_multiview",
        "GL_OVR_multiview2",
        "GL_WEBGL_video_texture",
    });
    fs += UnexpectedExtensionMacros({
        "GL_ANDROID_extension_pack_es31a",
        "GL_EXT_draw_buffers",
        "GL_EXT_frag_depth",
        "GL_EXT_geometry_shader",
        "GL_EXT_gpu_shader5",
        "GL_EXT_primitive_bounding_box",
        "GL_EXT_shader_io_blocks",
        "GL_EXT_shader_texture_lod",
        "GL_EXT_shadow_samplers",
        "GL_EXT_tessellation_shader",
        "GL_EXT_texture_buffer",
        "GL_EXT_texture_cube_map_array",
        "GL_NV_shader_framebuffer_fetch",
        "GL_OES_geometry_shader",
        "GL_OES_primitive_bounding_box",
        "GL_OES_shader_image_atomic",
        "GL_OES_shader_io_blocks",
        "GL_OES_standard_derivatives",
        "GL_OES_texture_buffer",
        "GL_OES_texture_cube_map_array",
    });
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), fs.c_str());
    ASSERT_GL_NO_ERROR();
}

// Test that only macros for ESSL 3.1 compatible extensions are defined
TEST_P(GLSLTest_ES31, ESSL31ExtensionMacros)
{
    std::string fs = essl31_shaders::fs::Red();
    fs += ExpectedExtensionMacros({
        "GL_ANDROID_extension_pack_es31a",
        "GL_ANGLE_base_vertex_base_instance_shader_builtin",
        "GL_ANGLE_clip_cull_distance",
        "GL_ANGLE_multi_draw",
        "GL_ANGLE_shader_pixel_local_storage",
        "GL_ANGLE_texture_multisample",
        "GL_APPLE_clip_distance",
        "GL_ARB_texture_rectangle",
        "GL_ARM_shader_framebuffer_fetch",
        "GL_EXT_blend_func_extended",
        "GL_EXT_clip_cull_distance",
        "GL_EXT_geometry_shader",
        "GL_EXT_gpu_shader5",
        "GL_EXT_primitive_bounding_box",
        "GL_EXT_separate_shader_objects",
        "GL_EXT_shader_framebuffer_fetch_non_coherent",
        "GL_EXT_shader_framebuffer_fetch",
        "GL_EXT_shader_io_blocks",
        "GL_EXT_shader_non_constant_global_initializers",
        "GL_EXT_tessellation_shader",
        "GL_EXT_texture_buffer",
        "GL_EXT_texture_cube_map_array",
        "GL_EXT_YUV_target",
        "GL_KHR_blend_equation_advanced",
        "GL_NV_EGL_stream_consumer_external",
        "GL_NV_shader_noperspective_interpolation",
        // Enabled on ESSL 3+ to workaround app bug. http://issuetracker.google.com/285871779
        "GL_OES_EGL_image_external",
        "GL_OES_EGL_image_external_essl3",
        // Enabled on ESSL 3+ to workaround app bug. http://issuetracker.google.com/285871779
        "GL_OES_texture_3D",
        "GL_OES_geometry_shader",
        "GL_OES_primitive_bounding_box",
        "GL_OES_sample_variables",
        "GL_OES_shader_image_atomic",
        "GL_OES_shader_io_blocks",
        "GL_OES_shader_multisample_interpolation",
        "GL_OES_texture_buffer",
        "GL_OES_texture_cube_map_array",
        "GL_OES_texture_storage_multisample_2d_array",
        "GL_OVR_multiview",
        "GL_OVR_multiview2",
        "GL_WEBGL_video_texture",
    });
    fs += UnexpectedExtensionMacros({
        "GL_EXT_draw_buffers",
        "GL_EXT_frag_depth",
        "GL_EXT_shader_texture_lod",
        "GL_EXT_shadow_samplers",
        "GL_NV_shader_framebuffer_fetch",
        "GL_OES_standard_derivatives",
    });
    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), fs.c_str());
    ASSERT_GL_NO_ERROR();
}

// Make sure names starting with webgl_ work.
TEST_P(GLSLTest_ES3, NameWithWebgl)
{
    constexpr char kFS[] = R"(#version 300 es
out highp vec4 webgl_color;
void main()
{
  webgl_color = vec4(0, 1, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Make sure webgl_FragColor works.
TEST_P(GLSLTest_ES3, NameWithWebglFragColor)
{
    constexpr char kFS[] = R"(#version 300 es
out highp vec4 webgl_FragColor;
void main()
{
  webgl_FragColor = vec4(0, 1, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that the ScalarizeVecAndMatConstructorArgs workaround works correctly with constructors that
// have no precision.  Regression test for a bug where the generated helper has no precision
// specified on the parameters and return value.
TEST_P(GLSLTest, ScalarizeVectorWorkaroundVsPrecisionlessConstructor)
{
    constexpr char kFS[] = R"(precision highp float;
void main() {
    bool b1 = true;
    float f1 = dot(vec4(b1 ? 1.0 : 0.0, 0.0, 0.0, 0.0), vec4(1.0));
    gl_FragColor = vec4(f1,0.0,0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Test that Metal compiler doesn't inline non-const globals
TEST_P(WebGLGLSLTest, InvalidGlobalsNotInlined)
{
    constexpr char kFS[] = R"(#version 100
  precision highp float;
  float v1 = 0.5;
  float v2 = v1;

  float f1() {
    return v2;
  }

  void main() {
    gl_FragColor = vec4(v1 + f1(),0.0,0.0, 1.0);
  })";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    ASSERT_GL_NO_ERROR();
}

// Test that a struct can have lots of fields.  Regression test for an inefficient O(n^2) check for
// fields having unique names.
TEST_P(GLSLTest_ES3, LotsOfFieldsInStruct)
{
    std::ostringstream fs;
    fs << R"(#version 300 es
precision highp float;
struct LotsOfFields
{
)";
    // Note: 16383 is the SPIR-V limit for struct member count.
    for (uint32_t i = 0; i < 16383; ++i)
    {
        fs << "    float field" << i << ";\n";
    }
    fs << R"(};
uniform B { LotsOfFields s; };
out vec4 color;
void main() {
    color = vec4(s.field0, 0.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), fs.str().c_str());
}

// Test that structs with too many fields are rejected.  In SPIR-V, the instruction that defines the
// struct lists the fields which means the length of the instruction is a function of the field
// count.  Since SPIR-V instruction sizes are limited to 16 bits, structs with more fields cannot be
// represented.
TEST_P(GLSLTest_ES3, TooManyFieldsInStruct)
{
    std::ostringstream fs;
    fs << R"(#version 300 es
precision highp float;
struct TooManyFields
{
)";
    for (uint32_t i = 0; i < (1 << 16); ++i)
    {
        fs << "    float field" << i << ";\n";
    }
    fs << R"(};
uniform B { TooManyFields s; };
out vec4 color;
void main() {
    color = vec4(s.field0, 0.0, 0.0, 1.0);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, fs.str().c_str());
    EXPECT_EQ(0u, shader);
}

// Same as TooManyFieldsInStruct, but with samplers in the struct.
TEST_P(GLSLTest_ES3, TooManySamplerFieldsInStruct)
{
    std::ostringstream fs;
    fs << R"(#version 300 es
precision highp float;
struct TooManyFields
{
)";
    for (uint32_t i = 0; i < (1 << 16); ++i)
    {
        fs << "    sampler2D field" << i << ";\n";
    }
    fs << R"(};
uniform TooManyFields s;
out vec4 color;
void main() {
    color = texture(s.field0, vec2(0));
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, fs.str().c_str());
    EXPECT_EQ(0u, shader);
}

// More complex variation of ManySamplerFieldsInStruct.  This one compiles fine.
TEST_P(GLSLTest_ES3, ManySamplerFieldsInStructComplex)
{
    // D3D and OpenGL may be more restrictive about this many samplers.
    ANGLE_SKIP_TEST_IF(IsD3D() || IsOpenGL());

    std::ostringstream fs;
    fs << R"(#version 300 es
precision highp float;

struct X {
    mediump sampler2D a[0xf00];
    mediump sampler2D b[0xf00];
    mediump sampler2D c[0xf000];
    mediump sampler2D d[0xf00];
};

struct Y {
  X s1;
  mediump sampler2D a[0xf00];
  mediump sampler2D b[0xf000];
  mediump sampler2D c[0x14000];
};

struct S {
    Y s1;
};

struct structBuffer { S s; };

uniform structBuffer b;

out vec4 color;
void main()
{
    color = texture(b.s.s1.s1.c[0], vec2(0));
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, fs.str().c_str());
    EXPECT_NE(0u, shader);
}

// Make sure a large array of samplers works.
TEST_P(GLSLTest, ManySamplers)
{
    // D3D and OpenGL may be more restrictive about this many samplers.
    ANGLE_SKIP_TEST_IF(IsD3D() || IsOpenGL());

    std::ostringstream fs;
    fs << R"(precision highp float;

uniform mediump sampler2D c[0x12000];

void main()
{
    gl_FragColor = texture2D(c[0], vec2(0));
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, fs.str().c_str());
    EXPECT_NE(0u, shader);
}

// Make sure a large array of samplers works when declared in a struct.
TEST_P(GLSLTest, ManySamplersInStruct)
{
    // D3D and OpenGL may be more restrictive about this many samplers.
    ANGLE_SKIP_TEST_IF(IsD3D() || IsOpenGL());

    std::ostringstream fs;
    fs << R"(precision highp float;

struct X {
    mediump sampler2D c[0x12000];
};

uniform X x;

void main()
{
    gl_FragColor = texture2D(x.c[0], vec2(0));
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, fs.str().c_str());
    EXPECT_NE(0u, shader);
}

// Test that passing large arrays to functions are compiled correctly.  Regression test for the
// SPIR-V generator that made a copy of the array to pass to the function, by decomposing and
// reconstructing it (in the absence of OpCopyLogical), but the reconstruction instruction has a
// length higher than can fit in SPIR-V.
TEST_P(GLSLTest_ES3, LargeInterfaceBlockArrayPassedToFunction)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform Large { float a[65536]; };
float f(float b[65536])
{
    b[0] = 1.0;
    return b[0] + b[1];
}
out vec4 color;
void main() {
    color = vec4(f(a), 0.0, 0.0, 1.0);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_EQ(0u, shader);
}

// Make sure the shader in LargeInterfaceBlockArrayPassedToFunction works if the large local is
// avoided.
TEST_P(GLSLTest_ES3, LargeInterfaceBlockArray)
{
    int maxUniformBlockSize = 0;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    ANGLE_SKIP_TEST_IF(maxUniformBlockSize < 16384 * 4);

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform Large { float a[16384]; };
out vec4 color;
void main() {
    color = vec4(a[0], 0.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
}

// Similar to LargeInterfaceBlockArrayPassedToFunction, but the array is nested in a struct.
TEST_P(GLSLTest_ES3, LargeInterfaceBlockNestedArrayPassedToFunction)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
struct S { float a[65536]; };
uniform Large { S s; };
float f(float b[65536])
{
    b[0] = 1.0;
    return b[0] + b[1];
}
out vec4 color;
void main() {
    color = vec4(f(s.a), 0.0, 0.0, 1.0);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_EQ(0u, shader);
}

// Make sure the shader in LargeInterfaceBlockNestedArrayPassedToFunction works if the large local
// is avoided.
TEST_P(GLSLTest_ES3, LargeInterfaceBlockNestedArray)
{
    int maxUniformBlockSize = 0;
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
    ANGLE_SKIP_TEST_IF(maxUniformBlockSize < 16384 * 4);

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
struct S { float a[16384]; };
uniform Large { S s; };
out vec4 color;
void main() {
    color = vec4(s.a[0], 0.0, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
}

// Similar to LargeInterfaceBlockArrayPassedToFunction, but the large array is copied to a local
// variable instead.
TEST_P(GLSLTest_ES3, LargeInterfaceBlockArrayCopiedToLocal)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
uniform Large { float a[65536]; };
out vec4 color;
void main() {
    float b[65536] = a;
    color = vec4(b[0], 0.0, 0.0, 1.0);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_EQ(0u, shader);
}

// Similar to LargeInterfaceBlockArrayCopiedToLocal, but the array is nested in a struct
TEST_P(GLSLTest_ES3, LargeInterfaceBlockNestedArrayCopiedToLocal)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
struct S { float a[65536]; };
uniform Large { S s; };
out vec4 color;
void main() {
    S s2 = s;
    color = vec4(s2.a[0], 0.0, 0.0, 1.0);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_EQ(0u, shader);
}

// Test that too large varyings are rejected.
TEST_P(GLSLTest_ES3, LargeArrayVarying)
{
    constexpr char kFS[] = R"(#version 300 es
precision highp float;
in float a[65536];
out vec4 color;
void main() {
    color = vec4(a[0], 0.0, 0.0, 1.0);
})";

    GLuint shader = CompileShader(GL_FRAGMENT_SHADER, kFS);
    EXPECT_EQ(0u, shader);
}

// Regression test for const globals losing const qualifiers during MSL
// translation and exceeding available temporary registers on Apple GPUs.
TEST_P(GLSLTest_ES3, LargeConstGlobalArraysOfStructs)
{
    const int n = 128;
    std::stringstream fragmentShader;
    fragmentShader << "#version 300 es\n"
                   << "precision mediump float;\n"
                   << "uniform mediump int zero;\n"
                   << "out vec4 color;\n"
                   << "struct S { vec3 A; vec3 B; float C; };\n";
    for (int i = 0; i < 3; ++i)
    {
        fragmentShader << "const S s" << i << "[" << n << "] = S[" << n << "](\n";
        for (int j = 0; j < n; ++j)
        {
            fragmentShader << "  S(vec3(0., 1., 0.), vec3(" << j << "), 0.)"
                           << (j != n - 1 ? ",\n" : "\n");
        }
        fragmentShader << ");\n";
    }
    // To ensure that the array is not rescoped, it must be accessed from two functions.
    // To ensure that the array is not optimized out, it must be accessed with a dynamic index.
    fragmentShader << "vec4 foo() {\n"
                   << "  return vec4(s0[zero].A * s1[zero].A * s2[zero].A, 1.0);\n"
                   << "}\n"
                   << "void main() {\n"
                   << "  color = foo() * vec4(s0[zero].A * s1[zero].A * s2[zero].A, 1.0);\n"
                   << "}\n";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), fragmentShader.str().c_str());

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test that framebuffer fetch emulation does not add a user-visible uniform.
TEST_P(GLSLTest, FramebufferFetchDoesNotAddUniforms)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_draw_buffers"));

    static constexpr char kFS[] = R"(#version 100
#extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_EXT_draw_buffers : require
uniform highp vec4 u_color;

void main (void)
{
    gl_FragData[0] = gl_LastFragData[0] + u_color;
    gl_FragData[1] = gl_LastFragData[1] + u_color;
    gl_FragData[2] = gl_LastFragData[2] + u_color;
    gl_FragData[3] = gl_LastFragData[3] + u_color;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint activeUniforms = 0, uniformsMaxLength = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms);
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformsMaxLength);

    // There should be only one active uniform
    EXPECT_EQ(activeUniforms, 1);

    // And that is u_color
    GLsizei nameLen = uniformsMaxLength;
    std::vector<char> name(uniformsMaxLength);

    GLint size;
    GLenum type;

    glGetActiveUniform(program, 0, uniformsMaxLength, &nameLen, &size, &type, name.data());
    EXPECT_EQ(std::string(name.data()), "u_color");
    EXPECT_EQ(size, 1);
    EXPECT_EQ(type, static_cast<GLenum>(GL_FLOAT_VEC4));
}

// Test that framebuffer fetch emulation does not add a user-visible uniform.
TEST_P(GLSLTest_ES31, FramebufferFetchDoesNotAddUniforms)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_shader_framebuffer_fetch"));

    static constexpr char kFS[] = R"(#version 310 es
#extension GL_EXT_shader_framebuffer_fetch : require
layout(location = 0) inout highp vec4 o_color;

layout(std140, binding = 0) buffer outBlock {
    highp vec4 data[256];
};

uniform highp vec4 u_color;
void main (void)
{
    uint index = uint(gl_FragCoord.y) * 16u + uint(gl_FragCoord.x);
    data[index] = o_color;
    o_color += u_color;
})";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint activeUniforms = 0, uniformsMaxLength = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms);
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformsMaxLength);

    // There should be only one active uniform
    EXPECT_EQ(activeUniforms, 1);

    // And that is u_color
    GLsizei nameLen = uniformsMaxLength;
    std::vector<char> name(uniformsMaxLength);

    GLint size;
    GLenum type;

    glGetActiveUniform(program, 0, uniformsMaxLength, &nameLen, &size, &type, name.data());
    EXPECT_EQ(std::string(name.data()), "u_color");
    EXPECT_EQ(size, 1);
    EXPECT_EQ(type, static_cast<GLenum>(GL_FLOAT_VEC4));
}

// Test that advanced blend emulation does not add a user-visible uniform.
TEST_P(GLSLTest_ES31, AdvancedBlendEquationsDoesNotAddUniforms)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_KHR_blend_equation_advanced"));

    static constexpr char kFS[] = R"(#version 310 es
#extension GL_KHR_blend_equation_advanced : require

layout (blend_support_multiply) out;

out highp vec4 o_color;

layout(std140, binding = 0) buffer outBlock {
    highp vec4 data[256];
};

uniform highp vec4 u_color;
void main (void)
{
    o_color = u_color;
})";

    ANGLE_GL_PROGRAM(program, essl31_shaders::vs::Simple(), kFS);
    glUseProgram(program);

    GLint activeUniforms = 0, uniformsMaxLength = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &activeUniforms);
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformsMaxLength);

    // There should be only one active uniform
    EXPECT_EQ(activeUniforms, 1);

    // And that is u_color
    GLsizei nameLen = uniformsMaxLength;
    std::vector<char> name(uniformsMaxLength);

    GLint size;
    GLenum type;

    glGetActiveUniform(program, 0, uniformsMaxLength, &nameLen, &size, &type, name.data());
    EXPECT_EQ(std::string(name.data()), "u_color");
    EXPECT_EQ(size, 1);
    EXPECT_EQ(type, static_cast<GLenum>(GL_FLOAT_VEC4));
}

// Tests struct in function return type.
TEST_P(GLSLTest, StructInFunctionDefinition)
{
    const char kFragmentShader[] = R"(precision mediump float;
struct Foo
{
    float v;
};

Foo foo()
{
    Foo f;
    f.v = 0.5;
    return f;
}

void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    Foo f = foo();
    if (f.v == 0.5)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Tests struct definition in function return type.
TEST_P(GLSLTest, StructDefinitionInFunctionDefinition)
{
    const char kFragmentShader[] = R"(precision mediump float;
struct Foo { float v; } foo()
{
    Foo f;
    f.v = 0.5;
    return f;
}

void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    Foo f = foo();
    if (f.v == 0.5)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test struct definition in forward declaration of function return type.
TEST_P(GLSLTest, StructDefinitionInFunctionPrototype)
{
    const char kFragmentShader[] = R"(precision mediump float;
struct Foo { float v; } foo();

void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    Foo f = foo();
    if (f.v == 0.5)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
}

Foo foo()
{
    Foo f;
    f.v = 0.5;
    return f;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that struct declarations are introduced into the correct scope.
TEST_P(GLSLTest, StructDefinitionInFunctionPrototypeScope)
{
    const char kFragmentShader[] = R"(precision mediump float;

struct Foo { float v; } foo()
{
    Foo f;
    f.v = 0.5;
    return f;
}

struct Bar { Foo f; } bar()
{
    Bar b;
    b.f = foo();
    return b;
}

void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    Bar b = bar();
    if (b.f.v == 0.5)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that struct declarations are introduced into the correct scope.
TEST_P(GLSLTest, NestedReturnedStructs)
{
    const char kFragmentShader[] = R"(precision mediump float;
struct Foo { float v; } foo(float bar);

void main()
{
    gl_FragColor = vec4(1, 0, 0, 1);
    float v = foo(foo(0.5).v).v;
    if (v == 0.5)
    {
        gl_FragColor = vec4(0, 1, 0, 1);
    }
}

Foo foo(float bar)
{
    Foo f;
    f.v = bar;
    return f;
})";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that vec equality works.
TEST_P(GLSLTest, VecEquality)
{
    const char kFragmentShader[] = R"(precision mediump float;
uniform vec4 u;
void main()
{
    gl_FragColor = vec4(0, 0, 0, 1);
    vec4 a = vec4(1.0, 2.0, 3.0, 4.0);
    if (a == u)
        gl_FragColor.g = 1.0;

    vec4 b = vec4(1.0) + u;
    if (b == u)
        gl_FragColor.r = 1.0;
}
)";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);
    GLint u = glGetUniformLocation(program, "u");
    glUniform4f(u, 1, 2, 3, 4);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that mat equality works.
TEST_P(GLSLTest, MatEquality)
{
    const char kFragmentShader[] = R"(precision mediump float;
uniform vec4 u;
void main()
{
    gl_FragColor = vec4(0, 0, 0, 1);
    mat4 a = mat4(1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4);
    if (a == mat4(u, u, u, u))
        gl_FragColor.g = 1.0;
    mat4 b = mat4(1.0);
    if (b == mat4(u, u, u, u))
        gl_FragColor.r = 1.0;
}
)";

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);
    GLint u = glGetUniformLocation(program, "u");
    glUniform4f(u, 1, 2, 3, 4);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that struct equality works.
TEST_P(GLSLTest, StructEquality)
{
    const char kFragmentShader[] = R"(precision mediump float;
uniform vec4 u;
struct A {
    vec4 i;
};
void main()
{
    gl_FragColor = vec4(0, 0, 0, 1);
    A a, b;
    a.i = vec4(1,2,3,4);
    b.i = u;
    if (a == b)
        gl_FragColor.g = 1.0;
    b.i = vec4(1.0);
    if (a == b)
        gl_FragColor.r = 1.0;
}
)";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);
    GLint u = glGetUniformLocation(program, "u");
    glUniform4f(u, 1, 2, 3, 4);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that nested struct equality works.
TEST_P(GLSLTest, NestedStructEquality)
{
    const char kFragmentShader[] = R"(precision mediump float;
uniform vec4 u;
struct A {
    vec4 i;
};
struct B {
    A a;
};
void main()
{
    gl_FragColor = vec4(0, 0, 0, 1);
    B a, b;
    a.a.i = vec4(1,2,3,4);
    b.a.i = u;
    if (a == b)
        gl_FragColor.g = 1.0;
    b.a.i = vec4(1.0);
    if (a == b)
        gl_FragColor.r = 1.0;
}
)";
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), kFragmentShader);
    glUseProgram(program);
    GLint u = glGetUniformLocation(program, "u");
    glUniform4f(u, 1, 2, 3, 4);

    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that double underscores user defined name is allowed
TEST_P(GLSLTest_ES3, DoubleUnderscoresName)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 oColor;
uniform struct __Data {float red;} data;
void main() {oColor=vec4(data.red,0,1,1);})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    // populate uniform
    GLint uniformLocation = glGetUniformLocation(program, "data.red");
    EXPECT_NE(uniformLocation, -1);
    glUniform1f(uniformLocation, 0);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

// Test that user defined name starts with "ANGLE" or "ANGLE_"
TEST_P(GLSLTest_ES3, VariableNameStartsWithANGLE)
{
    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
out vec4 oColor;
uniform struct ANGLEData{float red;} data;
uniform struct ANGLE_Data{float green;} _data;
void main() {oColor=vec4(data.red,_data.green,1,1);})";

    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    glUseProgram(program);
    // populate uniform
    GLint uniformRedLocation   = glGetUniformLocation(program, "data.red");
    GLint uniformGreenLocation = glGetUniformLocation(program, "_data.green");
    EXPECT_NE(uniformRedLocation, -1);
    EXPECT_NE(uniformGreenLocation, -1);
    glUniform1f(uniformRedLocation, 0);
    glUniform1f(uniformGreenLocation, 0);

    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    ASSERT_GL_NO_ERROR();
}

// Test that underscores in array names work with out arrays.
TEST_P(GLSLTest_ES3, UnderscoresWorkWithOutArrays)
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    GLuint textures[4];
    glGenTextures(4, textures);

    for (size_t texIndex = 0; texIndex < ArraySize(textures); texIndex++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindowWidth(), getWindowHeight(), 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }

    GLint maxDrawBuffers;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
    ASSERT_GE(maxDrawBuffers, 4);

    GLuint readFramebuffer;
    glGenFramebuffers(1, &readFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
out vec4 _e[4];
void main()
{
    _e[0] = vec4(1.0, 0.0, 0.0, 1.0);
    _e[1] = vec4(0.0, 1.0, 0.0, 1.0);
    _e[2] = vec4(0.0, 0.0, 1.0, 1.0);
    _e[3] = vec4(1.0, 1.0, 1.0, 1.0);
}
)";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    GLenum allBufs[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                         GL_COLOR_ATTACHMENT3};
    constexpr GLuint kMaxBuffers = 4;
    // Enable all draw buffers.
    for (GLuint texIndex = 0; texIndex < kMaxBuffers; texIndex++)
    {
        glBindTexture(GL_TEXTURE_2D, textures[texIndex]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + texIndex, GL_TEXTURE_2D,
                               textures[texIndex], 0);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + texIndex, GL_TEXTURE_2D,
                               textures[texIndex], 0);
    }
    glDrawBuffers(kMaxBuffers, allBufs);

    // Draw with simple program.
    drawQuad(program, essl3_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();
    verifyAttachment2DColor(0, textures[0], GL_TEXTURE_2D, 0, GLColor::red);
    verifyAttachment2DColor(1, textures[1], GL_TEXTURE_2D, 0, GLColor::green);
    verifyAttachment2DColor(2, textures[2], GL_TEXTURE_2D, 0, GLColor::blue);
    verifyAttachment2DColor(3, textures[3], GL_TEXTURE_2D, 0, GLColor::white);
}

// Fuzzer test involving struct samplers and comma operator
TEST_P(GLSLTest, StructSamplerVsComma)
{
    constexpr char kVS[] = R"(uniform struct S1
{
    samplerCube ar;
    vec2 c;
} a;

struct S2
{
    vec3 c;
} b[2];

void main (void)
{
    ++b[0].c,a;
})";

    GLuint shader = CompileShader(GL_VERTEX_SHADER, kVS);
    EXPECT_NE(0u, shader);
    glDeleteShader(shader);
}

// Make sure there is no name look up clash when initializing output variables
TEST_P(GLSLTest_ES3_InitShaderVariables, NameLookup)
{
    constexpr char kFS[] = R"(#version 300 es
out highp vec4 color;
void main()
{
    highp vec4 color;
    color.x = 1.0;
}
)";
    ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Simple(), kFS);
    drawQuad(program, essl3_shaders::PositionAttrib(), 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
    ASSERT_GL_NO_ERROR();
}

// Test that lowp and mediump varyings can be correctly matched between VS and FS.
TEST_P(GLSLTest, LowpMediumpVarying)
{
    const char kVS[] = R"(varying lowp float lowpVarying;
attribute vec4 position;
void main ()
{
  lowpVarying = 1.;
  gl_Position = position;
})";

    const char kFS[] = R"(varying mediump float lowpVarying;
void main ()
{
  gl_FragColor = vec4(lowpVarying, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    drawQuad(program, "position", 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}
}  // anonymous namespace

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(
    GLSLTest,
    ES3_OPENGL().enable(Feature::ForceInitShaderVariables),
    ES3_OPENGL().enable(Feature::ScalarizeVecAndMatConstructorArgs),
    ES3_OPENGLES().enable(Feature::ScalarizeVecAndMatConstructorArgs),
    ES3_VULKAN().enable(Feature::AvoidOpSelectWithMismatchingRelaxedPrecision),
    ES3_VULKAN().enable(Feature::ForceInitShaderVariables),
    ES3_VULKAN().disable(Feature::SupportsSPIRV14),
    ES2_VULKAN().enable(Feature::VaryingsRequireMatchingPrecisionInSpirv));

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(GLSLTestNoValidation);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GLSLTest_ES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(
    GLSLTest_ES3,
    ES3_OPENGL().enable(Feature::ForceInitShaderVariables),
    ES3_OPENGL().enable(Feature::ScalarizeVecAndMatConstructorArgs),
    ES3_OPENGLES().enable(Feature::ScalarizeVecAndMatConstructorArgs),
    ES3_VULKAN().enable(Feature::AvoidOpSelectWithMismatchingRelaxedPrecision),
    ES3_VULKAN().enable(Feature::ForceInitShaderVariables),
    ES3_VULKAN().disable(Feature::SupportsSPIRV14));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GLSLTestLoops);
ANGLE_INSTANTIATE_TEST_ES3(GLSLTestLoops);

ANGLE_INSTANTIATE_TEST_ES2(WebGLGLSLTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(WebGL2GLSLTest);
ANGLE_INSTANTIATE_TEST_ES3(WebGL2GLSLTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GLSLTest_ES31);
ANGLE_INSTANTIATE_TEST_ES31_AND(GLSLTest_ES31,
                                ES31_VULKAN().enable(Feature::ForceInitShaderVariables),
                                ES31_VULKAN().disable(Feature::SupportsSPIRV14));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GLSLTest_ES3_InitShaderVariables);
ANGLE_INSTANTIATE_TEST(
    GLSLTest_ES3_InitShaderVariables,
    ES3_VULKAN().enable(Feature::ForceInitShaderVariables),
    ES3_VULKAN().disable(Feature::SupportsSPIRV14).enable(Feature::ForceInitShaderVariables));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(GLSLTest_ES31_InitShaderVariables);
ANGLE_INSTANTIATE_TEST(
    GLSLTest_ES31_InitShaderVariables,
    ES31_VULKAN().enable(Feature::ForceInitShaderVariables),
    ES31_VULKAN().disable(Feature::SupportsSPIRV14).enable(Feature::ForceInitShaderVariables));
