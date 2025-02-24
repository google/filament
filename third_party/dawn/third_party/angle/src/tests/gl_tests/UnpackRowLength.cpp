//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class UnpackRowLengthTest : public ANGLETest<>
{
  protected:
    UnpackRowLengthTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mProgram = 0;
    }

    void testSetUp() override
    {
        constexpr char kFS[] = R"(uniform sampler2D tex;
void main()
{
    gl_FragColor = texture2D(tex, vec2(0.0, 1.0));
})";

        mProgram = CompileProgram(essl1_shaders::vs::Simple(), kFS);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }
    }

    void testTearDown() override { glDeleteProgram(mProgram); }

    void testRowLength(int texSize, int rowLength)
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);

        if ((getClientMajorVersion() == 3) || IsGLExtensionEnabled("GL_EXT_unpack_subimage"))
        {
            // Only texSize * texSize region is filled as WHITE, other parts are BLACK.
            // If the UNPACK_ROW_LENGTH is implemented correctly, all texels inside this texture are
            // WHITE.
            std::vector<GLubyte> buf(rowLength * texSize * 4);
            for (int y = 0; y < texSize; y++)
            {
                std::vector<GLubyte>::iterator rowIter = buf.begin() + y * rowLength * 4;
                std::fill(rowIter, rowIter + texSize * 4, static_cast<GLubyte>(255u));
                std::fill(rowIter + texSize * 4, rowIter + rowLength * 4, static_cast<GLubyte>(0u));
            }

            GLuint tex;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         &buf[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);

            EXPECT_PIXEL_EQ(0, 0, 255, 255, 255, 255);
            EXPECT_PIXEL_EQ(1, 0, 255, 255, 255, 255);

            glDeleteTextures(1, &tex);
        }
        else
        {
            EXPECT_GL_ERROR(GL_INVALID_ENUM);
        }
    }

    GLuint mProgram;
};

TEST_P(UnpackRowLengthTest, RowLength128)
{
    testRowLength(128, 128);
}

TEST_P(UnpackRowLengthTest, RowLength1024)
{
    testRowLength(128, 1024);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(UnpackRowLengthTest);

}  // namespace
