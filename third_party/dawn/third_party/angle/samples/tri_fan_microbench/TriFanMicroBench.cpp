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

#include "util/shader_utils.h"

#include <cstring>
#include <iostream>

// This small sample compares the per-frame render time for a series of
// squares drawn with TRIANGLE_FANS versus squares drawn with TRIANGLES.
// To exacerbate differences between the two, we use a large collection
// of short buffers with pre-translated vertex data.

class TriangleFanBenchSample : public SampleApplication
{
  public:
    TriangleFanBenchSample(int argc, char **argv)
        : SampleApplication("Microbench", argc, argv, ClientType::ES2, 1280, 1280), mFrameCount(0)
    {}

    void createVertexBuffers()
    {
        const unsigned int slices         = 8;
        const unsigned int numFanVertices = slices + 2;
        const unsigned int fanFloats      = numFanVertices * 3;

        mNumFanVerts = numFanVertices;

        const GLfloat halfDim = 0.0625;
        GLfloat fanVertices[] = {
            0.0f,     0.0f,     0.0f,  // center
            -halfDim, -halfDim, 0.0f,  // LL
            -halfDim, 0.0f,     0.0f,  // CL
            -halfDim, halfDim,  0.0f,  // UL
            0.0f,     halfDim,  0.0f,  // UC
            halfDim,  halfDim,  0.0f,  // UR
            halfDim,  0.0f,     0.0f,  // CR
            halfDim,  -halfDim, 0.0f,  // LR
            0.0f,     -halfDim, 0.0f,  // LC
            -halfDim, -halfDim, 0.0f   // LL (closes the fan)
        };

        const GLfloat xMin = -1.0f;  // We leave viewport/worldview untransformed in this sample
        const GLfloat xMax = 1.0f;
        const GLfloat yMin = -1.0f;
        // const GLfloat yMax = 1.0f;

        glGenBuffers(mNumSquares, mFanBufId);

        GLfloat xOffset = xMin;
        GLfloat yOffset = yMin;
        for (unsigned int i = 0; i < mNumSquares; ++i)
        {
            GLfloat tempVerts[fanFloats] = {0};
            for (unsigned int j = 0; j < numFanVertices; ++j)
            {
                tempVerts[j * 3]     = fanVertices[j * 3] + xOffset;
                tempVerts[j * 3 + 1] = fanVertices[j * 3 + 1] + yOffset;
                tempVerts[j * 3 + 2] = 0.0f;
            }

            glBindBuffer(GL_ARRAY_BUFFER, mFanBufId[i]);
            glBufferData(GL_ARRAY_BUFFER, fanFloats * sizeof(GLfloat), tempVerts, GL_STATIC_DRAW);

            xOffset += 2 * halfDim;
            if (xOffset > xMax)
            {
                xOffset = xMin;
                yOffset += 2 * halfDim;
            }
        }

        const unsigned int numTriVertices = slices * 3;
        const unsigned int triFloats      = numTriVertices * 3;
        GLfloat triVertices[triFloats];
        GLfloat *triPointer = triVertices;

        mNumTriVerts = numTriVertices;

        for (unsigned int i = 0; i < slices; ++i)
        {
            memcpy(triPointer, fanVertices,
                   3 * sizeof(GLfloat));  // copy center point as first vertex for this slice
            triPointer += 3;
            for (unsigned int j = 1; j < 3; ++j)
            {
                GLfloat *vertex =
                    &(fanVertices[(i + j) * 3]);  // copy two outer vertices for this point
                memcpy(triPointer, vertex, 3 * sizeof(GLfloat));
                triPointer += 3;
            }
        }

        // GLfloat triVertices2[triFloats];
        glGenBuffers(mNumSquares, mTriBufId);
        xOffset = xMin;
        yOffset = yMin;

        for (unsigned int i = 0; i < mNumSquares; ++i)
        {
            triPointer = triVertices;
            GLfloat tempVerts[triFloats];
            for (unsigned int j = 0; j < numTriVertices; ++j)
            {
                tempVerts[j * 3]     = triPointer[0] + xOffset;
                tempVerts[j * 3 + 1] = triPointer[1] + yOffset;
                tempVerts[j * 3 + 2] = 0.0f;
                triPointer += 3;
            }

            glBindBuffer(GL_ARRAY_BUFFER, mTriBufId[i]);
            glBufferData(GL_ARRAY_BUFFER, triFloats * sizeof(GLfloat), tempVerts, GL_STATIC_DRAW);
            xOffset += 2 * halfDim;
            if (xOffset > xMax)
            {
                yOffset += 2 * halfDim;
                xOffset = xMin;
            }
        }
    }

    bool initialize() override
    {
        constexpr char kVS[] = R"(attribute vec4 vPosition;
void main()
{
    gl_Position = vPosition;
})";

        constexpr char kFS[] = R"(precision mediump float;
void main()
{
    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        createVertexBuffers();

        mFanTotalTime = 0;
        mTriTotalTime = 0;

        return true;
    }

    void destroy() override
    {
        std::cout << "Total draw time using TRIANGLE_FAN: " << mFanTotalTime << "ms ("
                  << (float)mFanTotalTime / (float)mFrameCount << " average per frame)"
                  << std::endl;
        std::cout << "Total draw time using TRIANGLES: " << mTriTotalTime << "ms ("
                  << (float)mTriTotalTime / (float)mFrameCount << " average per frame)"
                  << std::endl;
        glDeleteProgram(mProgram);
    }

    void draw() override
    {
        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Bind the vertex data
        glEnableVertexAttribArray(0);

        // Draw using triangle fans, stored in VBO
        mFanTimer.start();
        for (unsigned i = 0; i < mNumSquares; ++i)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mFanBufId[i]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glDrawArrays(GL_TRIANGLE_FAN, 0, mNumFanVerts);
        }
        mFanTimer.stop();

        mFanTotalTime +=
            static_cast<unsigned int>(mFanTimer.getElapsedWallClockTime() *
                                      1000);  // convert from usec to msec when accumulating

        // Clear to eliminate driver-side gains from occlusion
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw using triangles, stored in VBO
        mTriTimer.start();
        for (unsigned i = 1; i < mNumSquares; ++i)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mTriBufId[i]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
            glDrawArrays(GL_TRIANGLES, 0, mNumTriVerts);
        }
        mTriTimer.stop();

        mTriTotalTime +=
            static_cast<unsigned int>(mTriTimer.getElapsedWallClockTime() *
                                      1000);  // convert from usec to msec when accumulating

        mFrameCount++;
    }

  private:
    static const unsigned int mNumSquares = 289;
    unsigned int mNumFanVerts;
    unsigned int mNumTriVerts;
    GLuint mProgram;
    GLuint mFanBufId[mNumSquares];
    GLuint mTriBufId[mNumSquares];

    Timer mFanTimer;
    Timer mTriTimer;
    unsigned int mFrameCount;
    unsigned int mTriTotalTime;
    unsigned int mFanTotalTime;
};

int main(int argc, char **argv)
{
    TriangleFanBenchSample app(argc, argv);
    return app.run();
}
