//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class WEBGLVideoTextureTest : public ANGLETest<>
{
  protected:
    WEBGLVideoTextureTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

class WEBGLVideoTextureES300Test : public ANGLETest<>
{
  protected:
    WEBGLVideoTextureES300Test()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Test to verify samplerVideoWEBGL works fine when extension is enabled.
TEST_P(WEBGLVideoTextureTest, VerifySamplerVideoWEBGL)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_WEBGL_video_texture"));

    constexpr char kVS[] = R"(
attribute vec2 position;
varying mediump vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(

#extension GL_WEBGL_video_texture : require
precision mediump float;
varying mediump vec2 texCoord;
uniform mediump samplerVideoWEBGL s;
void main()
{
    gl_FragColor = textureVideoWEBGL(s, texCoord);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, texture);
    glTexImage2D(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redColors.data());
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0);
    ASSERT_GL_NO_ERROR();
}

// Test to verify samplerVideoWEBGL works fine as parameter of user defined function
// when extension is enabled.
TEST_P(WEBGLVideoTextureTest, VerifySamplerVideoWEBGLAsParameter)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_WEBGL_video_texture"));

    constexpr char kVS[] = R"(
attribute vec2 position;
varying mediump vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(

#extension GL_WEBGL_video_texture : require
precision mediump float;
varying mediump vec2 texCoord;
uniform mediump samplerVideoWEBGL s;

vec4 wrapTextureVideoWEBGL(mediump samplerVideoWEBGL sampler, vec2 coord)
{
    return textureVideoWEBGL(sampler, coord);
}

void main()
{
    gl_FragColor = wrapTextureVideoWEBGL(s, texCoord);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, texture);
    glTexImage2D(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redColors.data());
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0);
    ASSERT_GL_NO_ERROR();
}

// Test to ensure ANGLE state manager knows the change when binding VideoImage
// and can handle it correctly based on the program.
TEST_P(WEBGLVideoTextureTest, VerifyStateManagerKnowsBindingVideoImage)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_WEBGL_video_texture"));

    constexpr char kVS[] = R"(
attribute vec2 position;
varying mediump vec2 texCoord;
void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS2D[] = R"(

precision mediump float;
varying mediump vec2 texCoord;
uniform mediump sampler2D s;

void main()
{
    gl_FragColor = texture2D(s, texCoord);
})";

    constexpr char kFSVideoImage[] = R"(

#extension GL_WEBGL_video_texture : require
precision mediump float;
varying mediump vec2 texCoord;
uniform mediump samplerVideoWEBGL s;

void main()
{
    gl_FragColor = textureVideoWEBGL(s, texCoord);
})";

    ANGLE_GL_PROGRAM(program2D, kVS, kFS2D);
    ANGLE_GL_PROGRAM(programVideoImage, kVS, kFSVideoImage);
    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    const std::vector<GLColor> greenColors(4, GLColor::green);
    GLTexture texture2D;
    GLTexture textureVideoImage;
    glBindTexture(GL_TEXTURE_2D, texture2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, redColors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // This should unbind the native TEXTURE_2D
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, textureVideoImage);
    glTexImage2D(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 greenColors.data());
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // ANGLE will check state change and apply changes through state manager. If state manager
    // is aware of the unbind, it will bind the correct texture back in native and the draw should
    // work fine.
    drawQuad(program2D, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    drawQuad(programVideoImage, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::green);
    drawQuad(program2D, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);

    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    ASSERT_GL_NO_ERROR();
}

// Test to verify samplerVideoWEBGL works fine in ES300 when extension is enabled.
TEST_P(WEBGLVideoTextureES300Test, VerifySamplerVideoWEBGLInES300)
{
    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_WEBGL_video_texture"));

    constexpr char kVS[] = R"(#version 300 es
in vec2 position;
out mediump vec2 texCoord;

void main()
{
    gl_Position = vec4(position, 0, 1);
    texCoord = position * 0.5 + vec2(0.5);
})";

    constexpr char kFS[] = R"(#version 300 es
#extension GL_WEBGL_video_texture : require
precision mediump float;
in mediump vec2 texCoord;
uniform mediump samplerVideoWEBGL s;
out vec4 my_FragColor;
void main()
{
    my_FragColor = texture(s, texCoord);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    // Initialize basic red texture.
    const std::vector<GLColor> redColors(4, GLColor::red);
    GLTexture texture;
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, texture);
    glTexImage2D(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 redColors.data());
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_VIDEO_IMAGE_WEBGL, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    drawQuad(program, "position", 0.0f);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::red);
    glBindTexture(GL_TEXTURE_VIDEO_IMAGE_WEBGL, 0);
    ASSERT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(WEBGLVideoTextureTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(WEBGLVideoTextureES300Test);
ANGLE_INSTANTIATE_TEST_ES3(WEBGLVideoTextureES300Test);
}  // namespace
