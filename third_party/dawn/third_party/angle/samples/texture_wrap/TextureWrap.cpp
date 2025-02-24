//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on TextureWrap.c from
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

class TextureWrapSample : public SampleApplication
{
  public:
    TextureWrapSample(int argc, char **argv) : SampleApplication("TextureWrap", argc, argv) {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(uniform float u_offset;
attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
void main()
{
    gl_Position = a_position;
    gl_Position.x += u_offset;
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

        // Get the offset location
        mOffsetLoc = glGetUniformLocation(mProgram, "u_offset");

        // Load the texture
        mTexture = CreateMipMappedTexture2D();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    void destroy() override { glDeleteProgram(mProgram); }

    void draw() override
    {
        GLfloat vertices[] = {
            -0.3f, 0.3f,  0.0f, 1.0f,  // Position 0
            -1.0f, -1.0f,              // TexCoord 0
            -0.3f, -0.3f, 0.0f, 1.0f,  // Position 1
            -1.0f, 2.0f,               // TexCoord 1
            0.3f,  -0.3f, 0.0f, 1.0f,  // Position 2
            2.0f,  2.0f,               // TexCoord 2
            0.3f,  0.3f,  0.0f, 1.0f,  // Position 3
            2.0f,  -1.0f               // TexCoord 3
        };
        GLushort indices[] = {0, 1, 2, 0, 2, 3};

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex position
        glVertexAttribPointer(mPositionLoc, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), vertices);
        glEnableVertexAttribArray(mPositionLoc);

        // Load the texture coordinate
        glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                              vertices + 4);
        glEnableVertexAttribArray(mTexCoordLoc);

        // Set the sampler texture unit to 0
        glUniform1i(mSamplerLoc, 0);

        // Draw quad with repeat wrap mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glUniform1f(mOffsetLoc, -0.7f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        // Draw quad with clamp to edge wrap mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1f(mOffsetLoc, 0.0f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        // Draw quad with mirrored repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glUniform1f(mOffsetLoc, 0.7f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }

  private:
    // Handle to a program object
    GLuint mProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mTexCoordLoc;

    // Sampler location
    GLint mSamplerLoc;

    // Offset location
    GLint mOffsetLoc;

    // Texture handle
    GLuint mTexture;
};

int main(int argc, char **argv)
{
    TextureWrapSample app(argc, argv);
    return app.run();
}
