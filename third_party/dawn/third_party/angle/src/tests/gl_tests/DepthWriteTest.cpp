//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The tests assert OpenGL behavior for all combinations of the following states
// - Depth Range
//   - Full (0, 1)
//   - Reduced
// - Depth Clamp
//   - Enabled (if supported)
//   - Disabled
// - Vertex Depth
//   - Inside clip volume
//   - Less than -1
//   - Greater than +1
// - gl_FragDepth
//   - Unused
//   - Passthrough (gl_FragCoord.z)
//   - Within the depth range
//   - Between 0 and near clipping plane
//   - Between 1 and far clipping plane
//   - Negative
//   - Greater than 1
// - Depth buffer format
//   - DEPTH_COMPONENT16
//   - DEPTH_COMPONENT32F (ES 3.x only)

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

enum class DepthRange
{
    Full,
    Reduced,
};

enum class VertexDepth
{
    InsideClipVolume,
    LessThanMinusOne,
    GreaterThanOne,
};

enum class FragmentDepth
{
    Unused,
    Passthrough,
    WithinDepthRange,
    BetweenZeroAndNearPlane,
    BetweenFarPlaneAndOne,
    Negative,
    GreaterThanOne,
};

// Variations corresponding to enums above.
using DepthWriteVariationsTestParams =
    std::tuple<angle::PlatformParameters, DepthRange, bool, VertexDepth, FragmentDepth, GLenum>;

std::ostream &operator<<(std::ostream &out, DepthRange depthRange)
{
    switch (depthRange)
    {
        case DepthRange::Full:
            out << "Full";
            break;
        case DepthRange::Reduced:
            out << "Reduced";
            break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, VertexDepth vertexDepth)
{
    switch (vertexDepth)
    {
        case VertexDepth::InsideClipVolume:
            out << "InsideClipVolume";
            break;
        case VertexDepth::LessThanMinusOne:
            out << "LessThanMinusOne";
            break;
        case VertexDepth::GreaterThanOne:
            out << "GreaterThanOne";
            break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, FragmentDepth fragmentDepth)
{
    switch (fragmentDepth)
    {
        case FragmentDepth::Unused:
            out << "Unused";
            break;
        case FragmentDepth::Passthrough:
            out << "Passthrough";
            break;
        case FragmentDepth::WithinDepthRange:
            out << "WithinDepthRange";
            break;
        case FragmentDepth::BetweenZeroAndNearPlane:
            out << "BetweenZeroAndNearPlane";
            break;
        case FragmentDepth::BetweenFarPlaneAndOne:
            out << "BetweenFarPlaneAndOne";
            break;
        case FragmentDepth::Negative:
            out << "Negative";
            break;
        case FragmentDepth::GreaterThanOne:
            out << "GreaterThanOne";
            break;
    }

    return out;
}

std::string BufferFormatToString(GLenum format)
{
    switch (format)
    {
        case GL_DEPTH_COMPONENT16:
            return "Depth16Unorm";
        case GL_DEPTH_COMPONENT32F:
            return "Depth32Float";
        default:
            return nullptr;
    }
}

void ParseDepthWriteVariationsTestParams(const DepthWriteVariationsTestParams &params,
                                         DepthRange *depthRangeOut,
                                         bool *depthClampEnabledOut,
                                         VertexDepth *vertexDepthOut,
                                         FragmentDepth *fragmentDepthOut,
                                         GLenum *depthBufferFormatOut)
{
    *depthRangeOut        = std::get<1>(params);
    *depthClampEnabledOut = std::get<2>(params);
    *vertexDepthOut       = std::get<3>(params);
    *fragmentDepthOut     = std::get<4>(params);
    *depthBufferFormatOut = std::get<5>(params);
}

std::string DepthWriteVariationsTestPrint(
    const ::testing::TestParamInfo<DepthWriteVariationsTestParams> &paramsInfo)
{
    const DepthWriteVariationsTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params);

    DepthRange depthRange;
    bool depthClampEnabled;
    VertexDepth vertexDepth;
    FragmentDepth fragmentDepth;
    GLenum depthBufferFormat;
    ParseDepthWriteVariationsTestParams(params, &depthRange, &depthClampEnabled, &vertexDepth,
                                        &fragmentDepth, &depthBufferFormat);

    out << "__"
        << "DepthRange" << depthRange << "_" << (depthClampEnabled ? "Clamped" : "Clipped") << "_"
        << "VertexDepth" << vertexDepth << "_"
        << "FragmentDepth" << fragmentDepth << "_" << BufferFormatToString(depthBufferFormat);
    return out.str();
}

class DepthWriteTest : public ANGLETest<DepthWriteVariationsTestParams>
{};

// Test correctness of depth writes
TEST_P(DepthWriteTest, Test)
{
    if (getClientMajorVersion() < 3)
    {
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_depth_texture"));
        ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_texture_half_float"));
    }
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_EXT_color_buffer_half_float") &&
                       !IsGLExtensionEnabled("GL_EXT_color_buffer_float"));

    DepthRange depthRange;
    bool depthClampEnabled;
    VertexDepth vertexDepth;
    FragmentDepth fragmentDepth;
    GLenum depthBufferFormat;
    ParseDepthWriteVariationsTestParams(GetParam(), &depthRange, &depthClampEnabled, &vertexDepth,
                                        &fragmentDepth, &depthBufferFormat);

    if (getClientMajorVersion() < 3)
    {
        ANGLE_SKIP_TEST_IF(fragmentDepth != FragmentDepth::Unused &&
                           !IsGLExtensionEnabled("GL_EXT_frag_depth"));
        ANGLE_SKIP_TEST_IF(depthBufferFormat == GL_DEPTH_COMPONENT32F);
    }
    ANGLE_SKIP_TEST_IF(depthClampEnabled && !IsGLExtensionEnabled("GL_EXT_depth_clamp"));

    const float near = depthRange == DepthRange::Full ? 0.0 : 0.25;
    const float far  = depthRange == DepthRange::Full ? 1.0 : 0.75;
    glDepthRangef(near, far);

    if (depthClampEnabled)
    {
        glEnable(GL_DEPTH_CLAMP_EXT);
    }

    float vertexDepthValue = 0.0;
    switch (vertexDepth)
    {
        case VertexDepth::InsideClipVolume:
            vertexDepthValue = 0.25;  // maps to 0.625
            break;
        case VertexDepth::LessThanMinusOne:
            vertexDepthValue = -1.5;
            break;
        case VertexDepth::GreaterThanOne:
            vertexDepthValue = 1.5;
            break;
    }

    float fragmentDepthValue = 0.0;
    switch (fragmentDepth)
    {
        case FragmentDepth::Unused:
        case FragmentDepth::Passthrough:
            break;
        case FragmentDepth::WithinDepthRange:
            fragmentDepthValue = 0.375;
            break;
        case FragmentDepth::BetweenZeroAndNearPlane:
            fragmentDepthValue = depthRange == DepthRange::Reduced ? 0.125 : 0.0;
            break;
        case FragmentDepth::BetweenFarPlaneAndOne:
            fragmentDepthValue = depthRange == DepthRange::Reduced ? 0.875 : 1.0;
            break;
        case FragmentDepth::Negative:
            fragmentDepthValue = -0.25;
            break;
        case FragmentDepth::GreaterThanOne:
            fragmentDepthValue = 1.25;
            break;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    const int w = getWindowWidth();
    const int h = getWindowHeight();

    const bool es2       = getClientMajorVersion() < 3;
    const bool fragCoord = fragmentDepth == FragmentDepth::Passthrough;
    std::stringstream fragmentSource;
    fragmentSource << (es2 ? "#extension GL_EXT_frag_depth : require\n" : "#version 300 es\n")
                   << (es2 ? "" : "out mediump vec4 fragColor;\n")
                   << (fragCoord ? "" : "uniform mediump float u_depth;\n") << "void main()\n"
                   << "{\n"
                   << (es2 ? "    gl_FragColor" : "    fragColor")
                   << " = vec4(1.0, 0.0, 0.0, 1.0);\n"
                   << "    gl_FragDepth" << (es2 ? "EXT" : "")
                   << (fragCoord ? " = gl_FragCoord.z;\n" : " = u_depth;\n") << "}";

    ANGLE_GL_PROGRAM(program, es2 ? essl1_shaders::vs::Simple() : essl3_shaders::vs::Simple(),
                     fragmentDepth == FragmentDepth::Unused
                         ? (es2 ? essl1_shaders::fs::Red() : essl3_shaders::fs::Red())
                         : fragmentSource.str().c_str());
    glUseProgram(program);
    if (fragmentDepth != FragmentDepth::Unused && fragmentDepth != FragmentDepth::Passthrough)
    {
        glUniform1f(glGetUniformLocation(program, "u_depth"), fragmentDepthValue);
    }
    ASSERT_GL_NO_ERROR();

    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    GLRenderbuffer rb;
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);

    GLTexture texDepth;
    glBindTexture(GL_TEXTURE_2D, texDepth);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D, 0,
        depthBufferFormat == GL_DEPTH_COMPONENT32F ? GL_DEPTH_COMPONENT32F : GL_DEPTH_COMPONENT, w,
        h, 0, GL_DEPTH_COMPONENT,
        depthBufferFormat == GL_DEPTH_COMPONENT32F ? GL_FLOAT : GL_UNSIGNED_SHORT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texDepth, 0);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glClearDepthf(0.33333333);
    glClear(GL_DEPTH_BUFFER_BIT);

    drawQuad(program, essl1_shaders::PositionAttrib(), vertexDepthValue);
    ASSERT_GL_NO_ERROR();

    auto getExpectedValue = [&]() {
        auto clamp = [](float x, float min, float max) {
            return x < min ? min : (x > max ? max : x);
        };

        if (depthClampEnabled)
        {
            if (fragmentDepth != FragmentDepth::Unused &&
                fragmentDepth != FragmentDepth::Passthrough)
            {
                // Fragment value clamped to the depth range
                return clamp(fragmentDepthValue, near, far);
            }
            else
            {
                // Vertex value transformed to window coordinates and clamped to the depth range
                return clamp(0.5f * ((far - near) * vertexDepthValue + (far + near)), near, far);
            }
        }
        else if (vertexDepthValue >= -1.0f && vertexDepthValue <= 1.0f)
        {
            if (fragmentDepth != FragmentDepth::Unused &&
                fragmentDepth != FragmentDepth::Passthrough)
            {
                // Fragment value clamped to [0, 1]
                return clamp(fragmentDepthValue, 0.0f, 1.0f);
            }
            else
            {
                // Vertex value transformed to window coordinates
                return 0.5f * ((far - near) * vertexDepthValue + (far + near));
            }
        }
        return 0.33333333f;
    };

    // Second pass to read written depth value
    ANGLE_GL_PROGRAM(readProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(readProgram);
    glUniform1i(glGetUniformLocation(readProgram, essl1_shaders::Texture2DUniform()), 0);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer readFb;
    glBindFramebuffer(GL_FRAMEBUFFER, readFb);

    GLRenderbuffer readRb;
    glBindRenderbuffer(GL_RENDERBUFFER, readRb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, readRb);

    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(readProgram, essl1_shaders::PositionAttrib(), 0.0f);

    float writtenValue[4] = {std::numeric_limits<float>::quiet_NaN()};
    glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, writtenValue);
    ASSERT_GL_NO_ERROR();

    EXPECT_NEAR(getExpectedValue(), writtenValue[0], 0.001);
}

constexpr DepthRange kDepthRanges[] = {
    DepthRange::Full,
    DepthRange::Reduced,
};
constexpr VertexDepth kVertexDepths[] = {
    VertexDepth::InsideClipVolume,
    VertexDepth::LessThanMinusOne,
    VertexDepth::GreaterThanOne,
};
constexpr FragmentDepth kFragmentDepths[] = {
    FragmentDepth::Unused,
    FragmentDepth::Passthrough,
    FragmentDepth::WithinDepthRange,
    FragmentDepth::BetweenZeroAndNearPlane,
    FragmentDepth::BetweenFarPlaneAndOne,
    FragmentDepth::Negative,
    FragmentDepth::GreaterThanOne,
};
constexpr GLenum kDepthBufferFormats[] = {
    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT32F,
};

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(DepthWriteTest);
ANGLE_INSTANTIATE_TEST_COMBINE_5(DepthWriteTest,
                                 DepthWriteVariationsTestPrint,
                                 testing::ValuesIn(kDepthRanges),
                                 testing::Bool(),
                                 testing::ValuesIn(kVertexDepths),
                                 testing::ValuesIn(kFragmentDepths),
                                 testing::ValuesIn(kDepthBufferFormats),
                                 ANGLE_ALL_TEST_PLATFORMS_ES2,
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

}  // anonymous namespace
