//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Simple_VertexShader.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"

#include "texture_utils.h"
#include "util/Matrix.h"
#include "util/geometry_utils.h"
#include "util/shader_utils.h"

#include <cmath>

class SimpleVertexShaderSample : public SampleApplication
{
  public:
    SimpleVertexShaderSample(int argc, char **argv)
        : SampleApplication("SimpleVertexShader", argc, argv)
    {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(uniform mat4 u_mvpMatrix;
attribute vec4 a_position;
attribute vec2 a_texcoord;
varying vec2 v_texcoord;
void main()
{
    gl_Position = u_mvpMatrix * a_position;
    v_texcoord = a_texcoord;
})";

        constexpr char kFS[] = R"(precision mediump float;
varying vec2 v_texcoord;
void main()
{
    gl_FragColor = vec4(v_texcoord.x, v_texcoord.y, 1.0, 1.0);
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        // Get the attribute locations
        mPositionLoc = glGetAttribLocation(mProgram, "a_position");
        mTexcoordLoc = glGetAttribLocation(mProgram, "a_texcoord");

        // Get the uniform locations
        mMVPMatrixLoc = glGetUniformLocation(mProgram, "u_mvpMatrix");

        // Generate the geometry data
        GenerateCubeGeometry(0.5f, &mCube);

        // Set an initial rotation
        mRotation = 45.0f;

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);

        return true;
    }

    void destroy() override { glDeleteProgram(mProgram); }

    void step(float dt, double totalTime) override
    {
        mRotation = fmod(mRotation + (dt * 40.0f), 360.0f);

        Matrix4 perspectiveMatrix = Matrix4::perspective(
            60.0f, float(getWindow()->getWidth()) / getWindow()->getHeight(), 1.0f, 20.0f);

        Matrix4 modelMatrix = Matrix4::translate(angle::Vector3(0.0f, 0.0f, -2.0f)) *
                              Matrix4::rotate(mRotation, angle::Vector3(1.0f, 0.0f, 1.0f));

        Matrix4 viewMatrix = Matrix4::identity();

        Matrix4 mvpMatrix = perspectiveMatrix * viewMatrix * modelMatrix;

        // Load the matrices
        glUniformMatrix4fv(mMVPMatrixLoc, 1, GL_FALSE, mvpMatrix.data);
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
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, mCube.positions.data());
        glEnableVertexAttribArray(mPositionLoc);

        // Load the texcoord data
        glVertexAttribPointer(mTexcoordLoc, 2, GL_FLOAT, GL_FALSE, 0, mCube.texcoords.data());
        glEnableVertexAttribArray(mTexcoordLoc);

        // Draw the cube
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mCube.indices.size()), GL_UNSIGNED_SHORT,
                       mCube.indices.data());
    }

  private:
    // Handle to a program object
    GLuint mProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mTexcoordLoc;

    // Uniform locations
    GLuint mMVPMatrixLoc;

    // Current rotation
    float mRotation;

    // Geometry data
    CubeGeometry mCube;
};

int main(int argc, char **argv)
{
    SimpleVertexShaderSample app(argc, argv);
    return app.run();
}
