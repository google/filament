//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Hello_Triangle.c from
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

#include <cstring>
#include <iostream>

// This sample demonstrates the differences in rendering efficiency when
// drawing with already-created textures whose dimensions have been altered
// versus drawing with newly created textures.
//
// In order to support GL's per-level texture creation semantics over the
// D3D API in particular, which requires textures' full mip chains to be
// created at texture object creation time, ANGLE maintains copies of the
// constituent texture images in system memory until the texture is used in
// a draw call, at which time, if the texture passes GL's mip completeness
// rules, the D3D texture is created and the contents of the texture are
// uploaded. Once the texture is created, redefinition of the dimensions or
// format of the texture is costly-- a new D3D texture needs to be created,
// and ANGLE may need to read the contents back into system memory.
//
// Creating an entirely new texture also requires that a new D3D texture be
// created, but any overhead associated with tracking the already-present
// texture images is eliminated, as it's a novel texture. This sample
// demonstrates the contrast in draw call time between these two situations.
//
// The resizing & creation of a new texture is delayed until several frames
// after startup, to eliminate draw time differences caused by caching of
// rendering state subsequent to the first frame.

class TexRedefBenchSample : public SampleApplication
{
  public:
    TexRedefBenchSample(int argc, char **argv)
        : SampleApplication("Microbench", argc, argv, ClientType::ES2, 1280, 1280),
          mPixelsResize(nullptr),
          mPixelsNewTex(nullptr),
          mTimeFrame(false),
          mFrameCount(0)
    {}

    void defineSquareTexture2D(GLuint texId,
                               GLsizei baseDimension,
                               GLenum format,
                               GLenum type,
                               void *data)
    {
        glBindTexture(GL_TEXTURE_2D, texId);
        GLsizei curDim = baseDimension;
        GLuint level   = 0;

        while (curDim >= 1)
        {
            glTexImage2D(GL_TEXTURE_2D, level, format, curDim, curDim, 0, format, type, data);
            curDim /= 2;
            level++;
        }
    }

    void createPixelData()
    {
        mPixelsResize     = new GLubyte[512 * 512 * 4];
        mPixelsNewTex     = new GLubyte[512 * 512 * 4];
        GLubyte *pixPtr0  = mPixelsResize;
        GLubyte *pixPtr1  = mPixelsNewTex;
        GLubyte zeroPix[] = {0, 192, 192, 255};
        GLubyte onePix[]  = {192, 0, 0, 255};
        for (int i = 0; i < 512 * 512; ++i)
        {
            memcpy(pixPtr0, zeroPix, 4 * sizeof(GLubyte));
            memcpy(pixPtr1, onePix, 4 * sizeof(GLubyte));
            pixPtr0 += 4;
            pixPtr1 += 4;
        }
    }

    bool initialize() override
    {
        constexpr char kVS[] = R"(attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
void main()
{
    gl_Position = a_position;
    v_texCoord = a_texCoord;
})";

        constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_texture;
void main()
{
    gl_FragColor = texture2D(s_texture, v_texCoord);
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        // Get the attribute locations
        mPositionLoc = glGetAttribLocation(mProgram, "a_position");
        mTexCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

        // Get the sampler location
        mSamplerLoc = glGetUniformLocation(mProgram, "s_texture");

        // Generate texture IDs, and create texture 0
        glGenTextures(3, mTextureIds);

        createPixelData();
        defineSquareTexture2D(mTextureIds[0], 256, GL_RGBA, GL_UNSIGNED_BYTE, mPixelsResize);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    void destroy() override
    {
        glDeleteProgram(mProgram);

        delete[] mPixelsResize;
        delete[] mPixelsNewTex;
    }

    void draw() override
    {
        GLfloat vertices[] = {
            -0.5f, 0.5f,  0.0f,  // Position 0
            0.0f,  0.0f,         // TexCoord 0
            -0.5f, -0.5f, 0.0f,  // Position 1
            0.0f,  1.0f,         // TexCoord 1
            0.5f,  -0.5f, 0.0f,  // Position 2
            1.0f,  1.0f,         // TexCoord 2
            0.5f,  0.5f,  0.0f,  // Position 3
            1.0f,  0.0f          // TexCoord 3
        };
        GLushort indices[] = {0, 1, 2, 0, 2, 3};

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex position
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices);
        // Load the texture coordinate
        glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                              vertices + 3);

        glEnableVertexAttribArray(mPositionLoc);
        glEnableVertexAttribArray(mTexCoordLoc);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTextureIds[0]);

        // Set the texture sampler to texture unit to 0
        glUniform1i(mSamplerLoc, 0);

        // We delay timing of texture resize/creation until after the first frame, as
        // caching optimizations will reduce draw time for subsequent frames for reasons
        // unreleated to texture creation. mTimeFrame is set to true on the fifth frame.
        if (mTimeFrame)
        {
            mOrigTimer.start();
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        if (mTimeFrame)
        {
            mOrigTimer.stop();
            // This timer indicates draw time for an already-created texture resident on the GPU,
            // which needs no updates. It will be faster than the other draws.
            std::cout << "Original texture draw: " << mOrigTimer.getElapsedWallClockTime() * 1000
                      << "msec" << std::endl;

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Now, change the texture dimensions of the original texture
            mResizeDefineTimer.start();
            defineSquareTexture2D(mTextureIds[0], 512, GL_RGBA, GL_UNSIGNED_BYTE, mPixelsResize);
            mResizeDefineTimer.stop();

            mResizeDrawTimer.start();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
            mResizeDrawTimer.stop();
            // This timer indicates draw time for a texture which has already been used in a draw,
            // causing the underlying resource to be allocated, and then resized, requiring resource
            // reallocation and related overhead.
            std::cout << "Resized texture definition: "
                      << mResizeDefineTimer.getElapsedWallClockTime() * 1000 << "msec" << std::endl;
            std::cout << "Resized texture draw: "
                      << mResizeDrawTimer.getElapsedWallClockTime() * 1000 << "msec" << std::endl;

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Create texure at same dimensions we resized previous texture to
            mNewTexDefineTimer.start();
            defineSquareTexture2D(mTextureIds[1], 512, GL_RGBA, GL_UNSIGNED_BYTE, mPixelsNewTex);
            mNewTexDefineTimer.stop();

            mNewTexDrawTimer.start();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
            mNewTexDrawTimer.stop();
            // This timer indicates draw time for a texture newly created this frame. The underlying
            // resource will need to be created, but because it has not previously been used, there
            // is no already-resident texture object to manage. This draw is expected to be faster
            // than the resized texture draw.
            std::cout << "Newly created texture definition: "
                      << mNewTexDefineTimer.getElapsedWallClockTime() * 1000 << "msec" << std::endl;
            std::cout << "Newly created texture draw: "
                      << mNewTexDrawTimer.getElapsedWallClockTime() * 1000 << "msec" << std::endl;
        }

        if (mFrameCount == 5)
            mTimeFrame = true;
        else
            mTimeFrame = false;

        mFrameCount++;
    }

  private:
    // Handle to a program object
    GLuint mProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mTexCoordLoc;

    // Sampler location
    GLint mSamplerLoc;

    // Texture handle
    GLuint mTextureIds[2];  // 0: texture created, then resized
                            // 1: texture newly created with TexImage

    // Texture pixel data
    GLubyte *mPixelsResize;
    GLubyte *mPixelsNewTex;

    Timer mOrigTimer;
    Timer mResizeDrawTimer;
    Timer mResizeDefineTimer;
    Timer mNewTexDrawTimer;
    Timer mNewTexDefineTimer;
    bool mTimeFrame;
    unsigned int mFrameCount;
};

int main(int argc, char **argv)
{
    TexRedefBenchSample app(argc, argv);
    return app.run();
}
