//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Simple_Texture2D.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"
#include "texture_utils.h"
#include "util/shader_utils.h"
#include "util/test_utils.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

class GLES1DrawTextureSample : public SampleApplication
{
  public:
    GLES1DrawTextureSample(int argc, char **argv)
        : SampleApplication("GLES1DrawTexture", argc, argv, ClientType::ES1, 1280, 800)
    {}

    bool initialize() override
    {
        // Load the texture
        mTexture = CreateSimpleTexture2D();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mTexture);

        GLint crop[4] = {0, 0, 2, 2};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        return true;
    }

    void destroy() override { glDeleteTextures(1, &mTexture); }

    void draw() override
    {
        glClear(GL_COLOR_BUFFER_BIT);

        GLint windowWidth  = getWindow()->getWidth();
        GLint windowHeight = getWindow()->getHeight();

        glDrawTexiOES(mX, mY, 0, mWidth, mHeight);
        glDrawTexiOES(windowWidth - mX, mY, 0, mWidth, mHeight);
        glDrawTexiOES(mX, windowHeight - mY, 0, mWidth, mHeight);
        glDrawTexiOES(windowWidth - mX, windowHeight - mY, 0, mWidth, mHeight);

        mX += mReverseX ? -1 : 1;
        mY += mReverseY ? -1 : 1;

        if (mX + mWidth >= windowWidth)
            mReverseX = true;
        if (mX < 0)
            mReverseX = false;

        if (mY + mHeight >= windowHeight)
            mReverseY = true;
        if (mY < 0)
            mReverseY = false;

        ++mWidth;
        ++mHeight;
        if (mWidth >= windowWidth)
            mWidth = 0;
        if (mHeight >= windowHeight)
            mHeight = 0;

        angle::Sleep(16);
    }

  private:
    // Texture handle
    GLuint mTexture = 0;

    // Draw texture coordinates and dimensions to loop through
    GLint mX      = 0;
    GLint mY      = 0;
    GLint mWidth  = 0;
    GLint mHeight = 0;

    bool mReverseX = false;
    bool mReverseY = false;
};

int main(int argc, char **argv)
{
    GLES1DrawTextureSample app(argc, argv);
    return app.run();
}
