//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
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

#include "common/vector_utils.h"
#include "texture_utils.h"
#include "util/shader_utils.h"

#include <cstring>
#include <iostream>
#include <vector>

using namespace angle;

class SimpleInstancingSample : public SampleApplication
{
  public:
    SimpleInstancingSample(int argc, char **argv)
        : SampleApplication("SimpleInstancing", argc, argv)
    {}

    bool initialize() override
    {
        // init instancing functions
        char *extensionString = (char *)glGetString(GL_EXTENSIONS);
        if (strstr(extensionString, "GL_ANGLE_instanced_arrays"))
        {
            mVertexAttribDivisorANGLE =
                (PFNGLVERTEXATTRIBDIVISORANGLEPROC)eglGetProcAddress("glVertexAttribDivisorANGLE");
            mDrawArraysInstancedANGLE =
                (PFNGLDRAWARRAYSINSTANCEDANGLEPROC)eglGetProcAddress("glDrawArraysInstancedANGLE");
            mDrawElementsInstancedANGLE = (PFNGLDRAWELEMENTSINSTANCEDANGLEPROC)eglGetProcAddress(
                "glDrawElementsInstancedANGLE");
        }

        if (!mVertexAttribDivisorANGLE || !mDrawArraysInstancedANGLE ||
            !mDrawElementsInstancedANGLE)
        {
            std::cerr << "Unable to load GL_ANGLE_instanced_arrays entry points.";
            return false;
        }

        constexpr char kVS[] = R"(attribute vec3 a_position;
attribute vec2 a_texCoord;
attribute vec3 a_instancePos;
varying vec2 v_texCoord;
void main()
{
    gl_Position = vec4(a_position.xyz + a_instancePos.xyz, 1.0);
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
        mPositionLoc    = glGetAttribLocation(mProgram, "a_position");
        mTexCoordLoc    = glGetAttribLocation(mProgram, "a_texCoord");
        mInstancePosLoc = glGetAttribLocation(mProgram, "a_instancePos");

        // Get the sampler location
        mSamplerLoc = glGetUniformLocation(mProgram, "s_texture");

        // Load the texture
        mTextureID = CreateSimpleTexture2D();

        // Initialize the vertex and index vectors
        const GLfloat quadRadius = 0.01f;

        mVertices.push_back(Vector3(-quadRadius, quadRadius, 0.0f));
        mVertices.push_back(Vector3(-quadRadius, -quadRadius, 0.0f));
        mVertices.push_back(Vector3(quadRadius, -quadRadius, 0.0f));
        mVertices.push_back(Vector3(quadRadius, quadRadius, 0.0f));

        mTexcoords.push_back(Vector2(0.0f, 0.0f));
        mTexcoords.push_back(Vector2(0.0f, 1.0f));
        mTexcoords.push_back(Vector2(1.0f, 1.0f));
        mTexcoords.push_back(Vector2(1.0f, 0.0f));

        mIndices.push_back(0);
        mIndices.push_back(1);
        mIndices.push_back(2);
        mIndices.push_back(0);
        mIndices.push_back(2);
        mIndices.push_back(3);

        // Tile thousands of quad instances
        for (float y = -1.0f + quadRadius; y < 1.0f - quadRadius; y += quadRadius * 3)
        {
            for (float x = -1.0f + quadRadius; x < 1.0f - quadRadius; x += quadRadius * 3)
            {
                mInstances.push_back(Vector3(x, y, 0.0f));
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    void destroy() override
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTextureID);
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
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, mVertices.data());
        glEnableVertexAttribArray(mPositionLoc);

        // Load the texture coordinate
        glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, mTexcoords.data());
        glEnableVertexAttribArray(mTexCoordLoc);

        // Load the instance position
        glVertexAttribPointer(mInstancePosLoc, 3, GL_FLOAT, GL_FALSE, 0, mInstances.data());
        glEnableVertexAttribArray(mInstancePosLoc);

        // Enable instancing
        mVertexAttribDivisorANGLE(mInstancePosLoc, 1);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        // Set the sampler texture unit to 0
        glUniform1i(mSamplerLoc, 0);

        // Do the instanced draw
        mDrawElementsInstancedANGLE(GL_TRIANGLES, static_cast<GLsizei>(mIndices.size()),
                                    GL_UNSIGNED_SHORT, mIndices.data(),
                                    static_cast<GLsizei>(mInstances.size()));
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
    GLuint mTextureID;

    // Instance VBO
    GLint mInstancePosLoc;

    // Loaded entry points
    PFNGLVERTEXATTRIBDIVISORANGLEPROC mVertexAttribDivisorANGLE;
    PFNGLDRAWARRAYSINSTANCEDANGLEPROC mDrawArraysInstancedANGLE;
    PFNGLDRAWELEMENTSINSTANCEDANGLEPROC mDrawElementsInstancedANGLE;

    // Vertex data
    std::vector<Vector3> mVertices;
    std::vector<Vector2> mTexcoords;
    std::vector<Vector3> mInstances;
    std::vector<GLushort> mIndices;
};

int main(int argc, char **argv)
{
    SimpleInstancingSample app(argc, argv);
    return app.run();
}
