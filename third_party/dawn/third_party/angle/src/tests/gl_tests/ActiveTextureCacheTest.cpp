//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ActiveTextureCacheTest.cpp: Regression tests of ANGLE's ActiveTextureCache inside gl::State.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

namespace angle
{

class ActiveTextureCacheTest : public ANGLETest<>
{
  protected:
    ActiveTextureCacheTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void testSetUp() override
    {
        constexpr char kVS[] =
            "precision highp float;\n"
            "\n"
            "void main()\n"
            "{\n"
            "   gl_Position = vec4(0.0, 0.0, 0.0, 0.0);\n"
            "}\n";

        constexpr char k2DFS[] =
            "precision highp float;\n"
            "uniform sampler2D tex2D;\n"
            "uniform samplerCube texCube;\n"
            "\n"
            "void main()\n"
            "{\n"
            "   gl_FragColor = texture2D(tex2D, vec2(0.0, 0.0)) + textureCube(texCube, vec3(0.0, "
            "0.0, 0.0));\n"
            "}\n";

        mProgram = CompileProgram(kVS, k2DFS);
        ASSERT_NE(0u, mProgram);

        m2DTextureLocation = glGetUniformLocation(mProgram, "tex2D");
        ASSERT_NE(-1, m2DTextureLocation);

        mCubeTextureLocation = glGetUniformLocation(mProgram, "texCube");
        ASSERT_NE(-1, mCubeTextureLocation);
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    GLuint mProgram            = 0;
    GLint m2DTextureLocation   = -1;
    GLint mCubeTextureLocation = -1;
};

// Regression test for a bug that causes the ActiveTexturesCache to get out of sync with the
// currently bound textures when changing program uniforms in such a way that the program becomes
// invalid.
TEST_P(ActiveTextureCacheTest, UniformChangeUpdatesActiveTextureCache)
{
    glUseProgram(mProgram);

    // Generate two textures and reset the texture binding
    GLuint tex0 = 0;
    glGenTextures(1, &tex0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLuint tex1 = 0;
    glGenTextures(1, &tex1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Set the active texture to 1 and bind tex0.
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex0);

    // Point the program's 2D sampler at texture binding 1. The texture will be added to the
    // ActiveTexturesCache because it matches the program's sampler type for this texture binding.
    glUniform1i(m2DTextureLocation, 1);

    // Point the program's cube sampler to texture binding 1 as well. This causes the program's
    // samplers become invalid and the ActiveTexturesCache is NOT updated.
    glUniform1i(mCubeTextureLocation, 1);

    // Bind tex1. ActiveTexturesCache is NOT updated (still contains tex0). The current texture
    // bindings do not match ActiveTexturesCache's state.
    glBindTexture(GL_TEXTURE_2D, tex1);

    // Delete tex0. The ActiveTexturesCache entry that points to tex0 is not cleared because tex0 is
    // not currently bound.
    glDeleteTextures(1, &tex0);

    // Use-after-free occurs during context destruction when the ActiveTexturesCache is cleared.
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(ActiveTextureCacheTest);

}  // namespace angle
