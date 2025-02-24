//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawCallPerf:
//   Performance tests for ANGLE draw call overhead.
//

#include "ANGLEPerfTest.h"
#include "DrawCallPerfParams.h"
#include "common/PackedEnums.h"
#include "test_utils/draw_call_perf_utils.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{
enum class StateChange
{
    NoChange,
    VertexAttrib,
    VertexBuffer,
    ManyVertexBuffers,
    Texture,
    Program,
    VertexBufferCycle,
    Scissor,
    ManyTextureDraw,
    Uniform,
    InvalidEnum,
    EnumCount = InvalidEnum,
};

constexpr size_t kCycleVBOPoolSize  = 200;
constexpr size_t kManyTexturesCount = 8;

struct DrawArraysPerfParams : public DrawCallPerfParams
{
    DrawArraysPerfParams() = default;
    DrawArraysPerfParams(const DrawCallPerfParams &base) : DrawCallPerfParams(base) {}

    std::string story() const override;

    StateChange stateChange = StateChange::NoChange;
};

std::string DrawArraysPerfParams::story() const
{
    std::stringstream strstr;

    strstr << DrawCallPerfParams::story();

    switch (stateChange)
    {
        case StateChange::VertexAttrib:
            strstr << "_attrib_change";
            break;
        case StateChange::VertexBuffer:
            strstr << "_vbo_change";
            break;
        case StateChange::ManyVertexBuffers:
            strstr << "_manyvbos_change";
            break;
        case StateChange::Texture:
            strstr << "_tex_change";
            break;
        case StateChange::Program:
            strstr << "_prog_change";
            break;
        case StateChange::VertexBufferCycle:
            strstr << "_vbo_cycle";
            break;
        case StateChange::Scissor:
            strstr << "_scissor_change";
            break;
        case StateChange::ManyTextureDraw:
            strstr << "_many_tex_draw";
            break;
        case StateChange::Uniform:
            strstr << "_uniform";
            break;
        default:
            break;
    }

    return strstr.str();
}

std::ostream &operator<<(std::ostream &os, const DrawArraysPerfParams &params)
{
    os << params.backendAndStory().substr(1);
    return os;
}

GLuint CreateSimpleTexture2D()
{
    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    GLuint texture;
    glGenTextures(1, &texture);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Load the texture: 2x2 Image, 3 bytes per pixel (R, G, B)
    constexpr size_t width             = 2;
    constexpr size_t height            = 2;
    GLubyte pixels[width * height * 4] = {
        255, 0,   0,   0,  // Red
        0,   255, 0,   0,  // Green
        0,   0,   255, 0,  // Blue
        255, 255, 0,   0   // Yellow
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texture;
}

class DrawCallPerfBenchmark : public ANGLERenderTest,
                              public ::testing::WithParamInterface<DrawArraysPerfParams>
{
  public:
    DrawCallPerfBenchmark();

    void initializeBenchmark() override;
    void destroyBenchmark() override;
    void drawBenchmark() override;

  private:
    GLuint mProgram1   = 0;
    GLuint mProgram2   = 0;
    GLuint mProgram3   = 0;
    GLuint mBuffer1    = 0;
    GLuint mBuffer2    = 0;
    GLuint mFBO        = 0;
    GLuint mFBOTexture = 0;
    std::vector<GLuint> mTextures;
    int mNumTris = GetParam().numTris;
    std::vector<GLuint> mVBOPool;
    size_t mCurrentVBO = 0;
};

DrawCallPerfBenchmark::DrawCallPerfBenchmark() : ANGLERenderTest("DrawCallPerf", GetParam())
{
    const auto &params = GetParam();
    if (IsPixel6() && params.eglParameters.renderer == EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE &&
        params.surfaceType == SurfaceType::Offscreen &&
        (params.stateChange == StateChange::VertexAttrib ||
         params.stateChange == StateChange::Program))
    {
        skipTest("https://issuetracker.google.com/issues/298407224 Fails on Pixel 6 GLES");
    }
}

void DrawCallPerfBenchmark::initializeBenchmark()
{
    const auto &params = GetParam();

    if (params.stateChange == StateChange::Texture)
    {
        mProgram1 = SetupSimpleTextureProgram();
        ASSERT_NE(0u, mProgram1);
    }
    else if (params.stateChange == StateChange::ManyTextureDraw)
    {
        mProgram3 = SetupEightTextureProgram();
        ASSERT_NE(0u, mProgram3);
    }
    else if (params.stateChange == StateChange::Program)
    {
        mProgram1 = SetupSimpleTextureProgram();
        mProgram2 = SetupDoubleTextureProgram();
        ASSERT_NE(0u, mProgram1);
        ASSERT_NE(0u, mProgram2);
    }
    else if (params.stateChange == StateChange::ManyVertexBuffers)
    {
        constexpr char kVS[] = R"(attribute vec2 vPosition;
attribute vec2 v0;
attribute vec2 v1;
attribute vec2 v2;
attribute vec2 v3;
const float scale = 0.5;
const float offset = -0.5;

varying vec2 v;

void main()
{
    gl_Position = vec4(vPosition * vec2(scale) + vec2(offset), 0, 1);
    v = (v0 + v1 + v2 + v3) * 0.25;
})";

        constexpr char kFS[] = R"(precision mediump float;
varying vec2 v;
void main()
{
    gl_FragColor = vec4(v, 0, 1);
})";

        mProgram1 = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram1);
        glBindAttribLocation(mProgram1, 1, "v0");
        glBindAttribLocation(mProgram1, 2, "v1");
        glBindAttribLocation(mProgram1, 3, "v2");
        glBindAttribLocation(mProgram1, 4, "v3");
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
    }
    else if (params.stateChange == StateChange::VertexBufferCycle)
    {
        mProgram1 = SetupSimpleDrawProgram();
        ASSERT_NE(0u, mProgram1);

        for (size_t bufferIndex = 0; bufferIndex < kCycleVBOPoolSize; ++bufferIndex)
        {
            GLuint buffer = Create2DTriangleBuffer(mNumTris, GL_STATIC_DRAW);
            mVBOPool.push_back(buffer);
        }
    }
    else if (params.stateChange == StateChange::Uniform)
    {
        constexpr char kVS[] = R"(attribute vec2 vPosition;
void main()
{
    gl_Position = vec4(vPosition, 0, 1);
})";

        constexpr char kFS[] = R"(precision mediump float;
uniform vec4 uni;
void main()
{
    gl_FragColor = uni;
})";

        mProgram1 = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram1);
    }
    else
    {
        mProgram1 = SetupSimpleDrawProgram();
        ASSERT_NE(0u, mProgram1);
    }

    // Re-link program to ensure the attrib bindings are used.
    if (mProgram1)
    {
        glBindAttribLocation(mProgram1, 0, "vPosition");
        glLinkProgram(mProgram1);
        glUseProgram(mProgram1);
    }

    if (mProgram2)
    {
        glBindAttribLocation(mProgram2, 0, "vPosition");
        glLinkProgram(mProgram2);
    }

    if (mProgram3)
    {
        glBindAttribLocation(mProgram3, 0, "vPosition");
        glLinkProgram(mProgram3);
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    mBuffer1 = Create2DTriangleBuffer(mNumTris, GL_STATIC_DRAW);
    mBuffer2 = Create2DTriangleBuffer(mNumTris, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    if (params.surfaceType == SurfaceType::Offscreen)
    {
        CreateColorFBO(getWindow()->getWidth(), getWindow()->getHeight(), &mFBOTexture, &mFBO);
    }

    for (size_t i = 0; i < kManyTexturesCount; ++i)
    {
        mTextures.emplace_back(CreateSimpleTexture2D());
    }

    if (params.stateChange == StateChange::Program)
    {
        // Bind the textures as appropriate, they are not modified during the test.
        GLint program1Tex1Loc = glGetUniformLocation(mProgram1, "tex");
        GLint program2Tex1Loc = glGetUniformLocation(mProgram2, "tex1");
        GLint program2Tex2Loc = glGetUniformLocation(mProgram2, "tex2");

        glUseProgram(mProgram1);
        glUniform1i(program1Tex1Loc, 0);

        glUseProgram(mProgram2);
        glUniform1i(program2Tex1Loc, 0);
        glUniform1i(program2Tex2Loc, 1);
    }

    if (params.stateChange == StateChange::ManyTextureDraw)
    {
        GLint program3TexLocs[kManyTexturesCount];

        for (size_t i = 0; i < mTextures.size(); ++i)
        {
            char stringBuffer[8];
            snprintf(stringBuffer, sizeof(stringBuffer), "tex%zu", i);
            program3TexLocs[i] = glGetUniformLocation(mProgram3, stringBuffer);
        }

        glUseProgram(mProgram3);
        for (size_t i = 0; i < mTextures.size(); ++i)
        {
            glUniform1i(program3TexLocs[i], i);
        }

        for (size_t i = 0; i < mTextures.size(); ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        }
    }

    ASSERT_GL_NO_ERROR();
}

void DrawCallPerfBenchmark::destroyBenchmark()
{
    glDeleteProgram(mProgram1);
    glDeleteProgram(mProgram2);
    glDeleteProgram(mProgram3);
    glDeleteBuffers(1, &mBuffer1);
    glDeleteBuffers(1, &mBuffer2);
    glDeleteTextures(1, &mFBOTexture);
    glDeleteTextures(mTextures.size(), mTextures.data());
    glDeleteFramebuffers(1, &mFBO);

    if (!mVBOPool.empty())
    {
        glDeleteBuffers(mVBOPool.size(), mVBOPool.data());
    }
}

void ClearThenDraw(unsigned int iterations, GLsizei numElements)
{
    glClear(GL_COLOR_BUFFER_BIT);

    for (unsigned int it = 0; it < iterations; it++)
    {
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

void JustDraw(unsigned int iterations, GLsizei numElements)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

template <int kArrayBufferCount>
void ChangeVertexAttribThenDraw(unsigned int iterations, GLsizei numElements, GLuint buffer)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    for (unsigned int it = 0; it < iterations; it++)
    {
        for (int arrayIndex = 0; arrayIndex < kArrayBufferCount; ++arrayIndex)
        {
            glVertexAttribPointer(arrayIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, numElements);

        for (int arrayIndex = 0; arrayIndex < kArrayBufferCount; ++arrayIndex)
        {
            glVertexAttribPointer(arrayIndex, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}
template <int kArrayBufferCount>
void ChangeArrayBuffersThenDraw(unsigned int iterations,
                                GLsizei numElements,
                                GLuint buffer1,
                                GLuint buffer2)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer1);
        for (int arrayIndex = 0; arrayIndex < kArrayBufferCount; ++arrayIndex)
        {
            glVertexAttribPointer(arrayIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, numElements);

        glBindBuffer(GL_ARRAY_BUFFER, buffer2);
        for (int arrayIndex = 0; arrayIndex < kArrayBufferCount; ++arrayIndex)
        {
            glVertexAttribPointer(arrayIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

void ChangeTextureThenDraw(unsigned int iterations,
                           GLsizei numElements,
                           GLuint texture1,
                           GLuint texture2)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        glBindTexture(GL_TEXTURE_2D, texture1);
        glDrawArrays(GL_TRIANGLES, 0, numElements);

        glBindTexture(GL_TEXTURE_2D, texture2);
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

void ChangeProgramThenDraw(unsigned int iterations,
                           GLsizei numElements,
                           GLuint program1,
                           GLuint program2)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        glUseProgram(program1);
        glDrawArrays(GL_TRIANGLES, 0, numElements);

        glUseProgram(program2);
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

void CycleVertexBufferThenDraw(unsigned int iterations,
                               GLsizei numElements,
                               const std::vector<GLuint> &vbos,
                               size_t *currentVBO)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        GLuint vbo = vbos[*currentVBO];
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, numElements);
        *currentVBO = (*currentVBO + 1) % vbos.size();
    }
}

void ChangeScissorThenDraw(unsigned int iterations,
                           GLsizei numElements,
                           unsigned int windowWidth,
                           unsigned int windowHeight)
{
    // Change scissor as such:
    //
    // - Start with a narrow vertical bar:
    //
    //           Scissor
    //              |
    //              V
    //       +-----+-+-----+
    //       |     | |     | <-- Window
    //       |     | |     |
    //       |     | |     |
    //       |     | |     |
    //       |     | |     |
    //       |     | |     |
    //       +-----+-+-----+
    //
    // - Gradually reduce height and increase width, to end up with a narrow horizontal bar:
    //
    //       +-------------+
    //       |             |
    //       |             |
    //       +-------------+ <-- Scissor
    //       +-------------+
    //       |             |
    //       |             |
    //       +-------------+
    //
    // - If more iterations left, restart, but shift the initial bar left to cover more area:
    //
    //       +---+-+-------+          +-------------+
    //       |   | |       |          |             |
    //       |   | |       |          +-------------+
    //       |   | |       |   --->   |             |
    //       |   | |       |          |             |
    //       |   | |       |          +-------------+
    //       |   | |       |          |             |
    //       +---+-+-------+          +-------------+
    //
    //       +-+-+---------+          +-------------+
    //       | | |         |          +-------------+
    //       | | |         |          |             |
    //       | | |         |   --->   |             |
    //       | | |         |          |             |
    //       | | |         |          |             |
    //       | | |         |          +-------------+
    //       +-+-+---------+          +-------------+

    glEnable(GL_SCISSOR_TEST);

    constexpr unsigned int kScissorStep  = 2;
    unsigned int scissorX                = windowWidth / 2 - 1;
    unsigned int scissorY                = 0;
    unsigned int scissorWidth            = 2;
    unsigned int scissorHeight           = windowHeight;
    unsigned int scissorPatternIteration = 0;

    for (unsigned int it = 0; it < iterations; it++)
    {
        glScissor(scissorX, scissorY, scissorWidth, scissorHeight);
        glDrawArrays(GL_TRIANGLES, 0, numElements);

        if (scissorX < kScissorStep || scissorHeight < kScissorStep * 2)
        {
            ++scissorPatternIteration;
            scissorX      = windowWidth / 2 - 1 - scissorPatternIteration * 2;
            scissorY      = 0;
            scissorWidth  = 2;
            scissorHeight = windowHeight;
        }
        else
        {
            scissorX -= kScissorStep;
            scissorY += kScissorStep;
            scissorWidth += kScissorStep * 2;
            scissorHeight -= kScissorStep * 2;
        }
    }
}

void DrawWithEightTextures(unsigned int iterations,
                           GLsizei numElements,
                           std::vector<GLuint> textures)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        for (size_t i = 0; i < textures.size(); ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            size_t index = (it + i) % textures.size();
            glBindTexture(GL_TEXTURE_2D, textures[index]);
        }

        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

void UpdateUniformThenDraw(unsigned int iterations, GLsizei numElements)
{
    for (unsigned int it = 0; it < iterations; it++)
    {
        float f = static_cast<float>(it) / static_cast<float>(iterations);
        glUniform4f(0, f, f + 0.1f, f + 0.2f, f + 0.3f);
        glDrawArrays(GL_TRIANGLES, 0, numElements);
    }
}

void DrawCallPerfBenchmark::drawBenchmark()
{
    // This workaround fixes a huge queue of graphics commands accumulating on the GL
    // back-end. The GL back-end doesn't have a proper NULL device at the moment.
    // TODO(jmadill): Remove this when/if we ever get a proper OpenGL NULL device.
    const auto &eglParams = GetParam().eglParameters;
    const auto &params    = GetParam();
    GLsizei numElements   = static_cast<GLsizei>(3 * mNumTris);

    switch (params.stateChange)
    {
        case StateChange::VertexAttrib:
            ChangeVertexAttribThenDraw<1>(params.iterationsPerStep, numElements, mBuffer1);
            break;
        case StateChange::VertexBuffer:
            ChangeArrayBuffersThenDraw<1>(params.iterationsPerStep, numElements, mBuffer1,
                                          mBuffer2);
            break;
        case StateChange::ManyVertexBuffers:
            ChangeArrayBuffersThenDraw<5>(params.iterationsPerStep, numElements, mBuffer1,
                                          mBuffer2);
            break;
        case StateChange::Texture:
            ChangeTextureThenDraw(params.iterationsPerStep, numElements, mTextures[0],
                                  mTextures[1]);
            break;
        case StateChange::Program:
            ChangeProgramThenDraw(params.iterationsPerStep, numElements, mProgram1, mProgram2);
            break;
        case StateChange::NoChange:
            if (eglParams.deviceType != EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE ||
                (eglParams.renderer != EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE &&
                 eglParams.renderer != EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE))
            {
                ClearThenDraw(params.iterationsPerStep, numElements);
            }
            else
            {
                JustDraw(params.iterationsPerStep, numElements);
            }
            break;
        case StateChange::VertexBufferCycle:
            CycleVertexBufferThenDraw(params.iterationsPerStep, numElements, mVBOPool,
                                      &mCurrentVBO);
            break;
        case StateChange::Scissor:
            ChangeScissorThenDraw(params.iterationsPerStep, numElements, getWindow()->getWidth(),
                                  getWindow()->getHeight());
            break;
        case StateChange::ManyTextureDraw:
            glUseProgram(mProgram3);
            DrawWithEightTextures(params.iterationsPerStep, numElements, mTextures);
            break;
        case StateChange::Uniform:
            UpdateUniformThenDraw(params.iterationsPerStep, numElements);
            break;
        case StateChange::InvalidEnum:
            ADD_FAILURE() << "Invalid state change.";
            break;
    }

    ASSERT_GL_NO_ERROR();
}

TEST_P(DrawCallPerfBenchmark, Run)
{
    run();
}

using namespace params;

DrawArraysPerfParams CombineStateChange(const DrawArraysPerfParams &in, StateChange stateChange)
{
    DrawArraysPerfParams out = in;
    out.stateChange          = stateChange;

    // Crank up iteration count to ensure we cycle through all VBs before a swap.
    if (stateChange == StateChange::VertexBufferCycle)
    {
        out.iterationsPerStep = kCycleVBOPoolSize * 2;
    }

    return out;
}

using P = DrawArraysPerfParams;

std::vector<P> gTestsWithStateChange =
    CombineWithValues({P()}, angle::AllEnums<StateChange>(), CombineStateChange);
std::vector<P> gTestsWithRenderer =
    CombineWithFuncs(gTestsWithStateChange, {D3D11<P>, GL<P>, Metal<P>, Vulkan<P>, WGL<P>});
std::vector<P> gTestsWithDevice =
    CombineWithFuncs(gTestsWithRenderer, {Passthrough<P>, Offscreen<P>, NullDevice<P>});

ANGLE_INSTANTIATE_TEST_ARRAY(DrawCallPerfBenchmark, gTestsWithDevice);

}  // anonymous namespace
