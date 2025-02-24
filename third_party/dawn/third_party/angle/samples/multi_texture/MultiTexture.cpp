//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on MultiTexture.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"

#include "tga_utils.h"
#include "util/shader_utils.h"
#include "util/test_utils.h"

class MultiTextureSample : public SampleApplication
{
  public:
    MultiTextureSample(int argc, char **argv) : SampleApplication("MultiTexture", argc, argv) {}

    GLuint loadTexture(const std::string &path)
    {
        TGAImage img;
        if (!LoadTGAImageFromFile(path, &img))
        {
            return 0;
        }

        return LoadTextureFromTGAImage(img);
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
uniform sampler2D s_baseMap;
uniform sampler2D s_lightMap;
void main()
{
    vec4 baseColor;
    vec4 lightColor;

    baseColor = texture2D(s_baseMap, v_texCoord);
    lightColor = texture2D(s_lightMap, v_texCoord);
    gl_FragColor = baseColor * (lightColor + 0.25);
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
        mBaseMapLoc  = glGetUniformLocation(mProgram, "s_baseMap");
        mLightMapLoc = glGetUniformLocation(mProgram, "s_lightMap");

        // Load the textures
        std::stringstream baseStr;
        baseStr << angle::GetExecutableDirectory() << "/basemap.tga";

        std::stringstream lightStr;
        lightStr << angle::GetExecutableDirectory() << "/lightmap.tga";

        mBaseMapTexID  = loadTexture(baseStr.str());
        mLightMapTexID = loadTexture(lightStr.str());
        if (mBaseMapTexID == 0 || mLightMapTexID == 0)
        {
            return false;
        }

        return true;
    }

    void destroy() override
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mBaseMapTexID);
        glDeleteTextures(1, &mLightMapTexID);
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
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex position
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices);
        // Load the texture coordinate
        glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                              vertices + 3);

        glEnableVertexAttribArray(mPositionLoc);
        glEnableVertexAttribArray(mTexCoordLoc);

        // Bind the base map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mBaseMapTexID);

        // Set the base map sampler to texture unit to 0
        glUniform1i(mBaseMapLoc, 0);

        // Bind the light map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mLightMapTexID);

        // Set the light map sampler to texture unit 1
        glUniform1i(mLightMapLoc, 1);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }

  private:
    // Handle to a program object
    GLuint mProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mTexCoordLoc;

    // Sampler locations
    GLint mBaseMapLoc;
    GLint mLightMapLoc;

    // Texture handle
    GLuint mBaseMapTexID;
    GLuint mLightMapTexID;
};

int main(int argc, char **argv)
{
    MultiTextureSample app(argc, argv);
    return app.run();
}
