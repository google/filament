//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on ParticleSystem.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"

#include "common/vector_utils.h"
#include "tga_utils.h"
#include "util/random_utils.h"
#include "util/shader_utils.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <string>

using namespace angle;

class ParticleSystemSample : public SampleApplication
{
  public:
    ParticleSystemSample(int argc, char **argv) : SampleApplication("ParticleSystem", argc, argv) {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(uniform float u_time;
uniform vec3 u_centerPosition;
attribute float a_lifetime;
attribute vec3 a_startPosition;
attribute vec3 a_endPosition;
varying float v_lifetime;
void main()
{
    if (u_time <= a_lifetime)
    {
        gl_Position.xyz = a_startPosition + (u_time * a_endPosition);
        gl_Position.xyz += u_centerPosition;
        gl_Position.w = 1.0;
    }
    else
    {
        gl_Position = vec4(-1000, -1000, 0, 0);
    }
    v_lifetime = 1.0 - (u_time / a_lifetime);
    v_lifetime = clamp(v_lifetime, 0.0, 1.0);
    gl_PointSize = (v_lifetime * v_lifetime) * 40.0;
})";

        constexpr char kFS[] = R"(precision mediump float;
uniform vec4 u_color;
varying float v_lifetime;
uniform sampler2D s_texture;
void main()
{
    vec4 texColor;
    texColor = texture2D(s_texture, gl_PointCoord);
    gl_FragColor = vec4(u_color) * texColor;
    gl_FragColor.a *= v_lifetime;
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        // Get the attribute locations
        mLifetimeLoc      = glGetAttribLocation(mProgram, "a_lifetime");
        mStartPositionLoc = glGetAttribLocation(mProgram, "a_startPosition");
        mEndPositionLoc   = glGetAttribLocation(mProgram, "a_endPosition");

        // Get the uniform locations
        mTimeLoc           = glGetUniformLocation(mProgram, "u_time");
        mCenterPositionLoc = glGetUniformLocation(mProgram, "u_centerPosition");
        mColorLoc          = glGetUniformLocation(mProgram, "u_color");
        mSamplerLoc        = glGetUniformLocation(mProgram, "s_texture");

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        // Fill in particle data array
        for (size_t i = 0; i < mParticleCount; i++)
        {
            mParticles[i].lifetime = mRNG.randomFloatBetween(0.0f, 1.0f);

            float endAngle                = mRNG.randomFloatBetween(0, 2.0f * float(M_PI));
            float endRadius               = mRNG.randomFloatBetween(0.0f, 2.0f);
            mParticles[i].endPosition.x() = sinf(endAngle) * endRadius;
            mParticles[i].endPosition.y() = cosf(endAngle) * endRadius;
            mParticles[i].endPosition.z() = 0.0f;

            float startAngle                = mRNG.randomFloatBetween(0, 2.0f * float(M_PI));
            float startRadius               = mRNG.randomFloatBetween(0.0f, 0.25f);
            mParticles[i].startPosition.x() = sinf(startAngle) * startRadius;
            mParticles[i].startPosition.y() = cosf(startAngle) * startRadius;
            mParticles[i].startPosition.z() = 0.0f;
        }

        mParticleTime = 1.0f;

        std::stringstream smokeStr;
        smokeStr << angle::GetExecutableDirectory() << "/smoke.tga";

        TGAImage img;
        if (!LoadTGAImageFromFile(smokeStr.str(), &img))
        {
            return false;
        }
        mTextureID = LoadTextureFromTGAImage(img);
        if (!mTextureID)
        {
            return false;
        }

        return true;
    }

    void destroy() override { glDeleteProgram(mProgram); }

    void step(float dt, double totalTime) override
    {
        // Use the program object
        glUseProgram(mProgram);

        mParticleTime += dt;
        if (mParticleTime >= 1.0f)
        {
            mParticleTime = 0.0f;

            // Pick a new start location and color
            Vector3 centerPos(mRNG.randomFloatBetween(-0.5f, 0.5f),
                              mRNG.randomFloatBetween(-0.5f, 0.5f),
                              mRNG.randomFloatBetween(-0.5f, 0.5f));
            glUniform3fv(mCenterPositionLoc, 1, centerPos.data());

            // Random color
            Vector4 color(mRNG.randomFloatBetween(0.0f, 1.0f), mRNG.randomFloatBetween(0.0f, 1.0f),
                          mRNG.randomFloatBetween(0.0f, 1.0f), 0.5f);
            glUniform4fv(mColorLoc, 1, color.data());
        }

        // Load uniform time variable
        glUniform1f(mTimeLoc, mParticleTime);
    }

    void draw() override
    {
        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex attributes
        glVertexAttribPointer(mLifetimeLoc, 1, GL_FLOAT, GL_FALSE, sizeof(Particle),
                              &mParticles[0].lifetime);
        glVertexAttribPointer(mEndPositionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                              &mParticles[0].endPosition);
        glVertexAttribPointer(mStartPositionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Particle),
                              &mParticles[0].startPosition);

        glEnableVertexAttribArray(mLifetimeLoc);
        glEnableVertexAttribArray(mEndPositionLoc);
        glEnableVertexAttribArray(mStartPositionLoc);

        // Blend particles
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        // Set the sampler texture unit to 0
        glUniform1i(mSamplerLoc, 0);

        glDrawArrays(GL_POINTS, 0, mParticleCount);
    }

  private:
    // Handle to a program object
    GLuint mProgram;

    // Attribute locations
    GLint mLifetimeLoc;
    GLint mStartPositionLoc;
    GLint mEndPositionLoc;

    // Uniform location
    GLint mTimeLoc;
    GLint mColorLoc;
    GLint mCenterPositionLoc;
    GLint mSamplerLoc;

    // Texture handle
    GLuint mTextureID;

    // Particle vertex data
    struct Particle
    {
        float lifetime;
        Vector3 startPosition;
        Vector3 endPosition;
    };
    static const size_t mParticleCount = 1024;
    std::array<Particle, mParticleCount> mParticles;
    float mParticleTime;
    RNG mRNG;
};

int main(int argc, char **argv)
{
    ParticleSystemSample app(argc, argv);
    return app.run();
}
