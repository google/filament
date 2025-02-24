//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Simple_TextureCubemap.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"

#include "texture_utils.h"
#include "util/geometry_utils.h"
#include "util/shader_utils.h"

class SimpleTextureCubemapSample : public SampleApplication
{
  public:
    SimpleTextureCubemapSample(int argc, char **argv)
        : SampleApplication("SimpleTextureCubemap", argc, argv)
    {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(attribute vec4 a_position;
attribute vec3 a_normal;
varying vec3 v_normal;
void main()
{
    gl_Position = a_position;
    v_normal = a_normal;
})";

        constexpr char kFS[] = R"(precision mediump float;
varying vec3 v_normal;
uniform samplerCube s_texture;
void main()
{
    gl_FragColor = textureCube(s_texture, v_normal);
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        // Get the attribute locations
        mPositionLoc = glGetAttribLocation(mProgram, "a_position");
        mNormalLoc   = glGetAttribLocation(mProgram, "a_normal");

        // Get the sampler locations
        mSamplerLoc = glGetUniformLocation(mProgram, "s_texture");

        // Load the texture
        mTexture = CreateSimpleTextureCubemap();

        // Generate the geometry data
        CreateSphereGeometry(128, 0.75f, &mSphere);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);

        return true;
    }

    void destroy() override
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTexture);
    }

    void draw() override
    {
        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex position
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, mSphere.positions.data());
        glEnableVertexAttribArray(mPositionLoc);

        // Load the normal
        glVertexAttribPointer(mNormalLoc, 3, GL_FLOAT, GL_FALSE, 0, mSphere.normals.data());
        glEnableVertexAttribArray(mNormalLoc);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mTexture);

        // Set the texture sampler to texture unit to 0
        glUniform1i(mSamplerLoc, 0);

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mSphere.indices.size()),
                       GL_UNSIGNED_SHORT, mSphere.indices.data());
    }

  private:
    // Handle to a program object
    GLuint mProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mNormalLoc;

    // Sampler location
    GLint mSamplerLoc;

    // Texture handle
    GLuint mTexture;

    // Geometry data
    SphereGeometry mSphere;
};

int main(int argc, char **argv)
{
    SimpleTextureCubemapSample app(argc, argv);
    return app.run();
}
