//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Based on CubeMapActivity.java from The Android Open Source Project ApiDemos
// https://android.googlesource.com/platform/development/+/refs/heads/master/samples/ApiDemos/src/com/example/android/apis/graphics/CubeMapActivity.java

// Hue to RGB conversion in GLSL based on
// https://github.com/tobspr/GLSL-Color-Spaces

#include "SampleApplication.h"

#include "common/debug.h"
#include "torus.h"
#include "util/Matrix.h"
#include "util/shader_utils.h"

#include <iostream>

const float kDegreesPerSecond = 90.0f;
const GLushort kHuesSize      = (kSize + 1) * (kSize + 1);

class BufferStorageSample : public SampleApplication
{
  public:
    BufferStorageSample(int argc, char **argv)
        : SampleApplication("GLES3.1 Buffer Storage", argc, argv, ClientType::ES3_1)
    {}

    bool initialize() override
    {
        if (!IsGLExtensionEnabled("GL_EXT_buffer_storage"))
        {
            std::cout << "GL_EXT_buffer_storage not available." << std::endl;
            return false;
        }

        constexpr char kVS[] = R"(#version 300 es
uniform mat4 mv;
uniform mat4 mvp;

in vec4 position;
in vec3 normal;
in float hue;

out vec3 normal_view;
out vec4 color;

vec4 hue_to_rgba(float hue)
{
    hue = mod(hue, 1.0);
    float r = abs(hue * 6.0 - 3.0) - 1.0;
    float g = 2.0 - abs(hue * 6.0 - 2.0);
    float b = 2.0 - abs(hue * 6.0 - 4.0);
    return vec4(r, g, b, 1.0);
}

void main()
{
    normal_view = vec3(mv * vec4(normal, 0.0));
    color = hue_to_rgba(hue);
    gl_Position = mvp * position;
})";

        constexpr char kFS[] = R"(#version 300 es
precision mediump float;

in vec3 normal_view;
in vec4 color;

out vec4 frag_color;

void main()
{
    frag_color = color * dot(vec3(0.0, 0.0, 1.0), normalize(normal_view));
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        mPositionLoc = glGetAttribLocation(mProgram, "position");
        mNormalLoc   = glGetAttribLocation(mProgram, "normal");
        mHueLoc      = glGetAttribLocation(mProgram, "hue");

        mMVPMatrixLoc = glGetUniformLocation(mProgram, "mvp");
        mMVMatrixLoc  = glGetUniformLocation(mProgram, "mv");

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);

        generateTorus();

        float ratio = static_cast<float>(getWindow()->getWidth()) /
                      static_cast<float>(getWindow()->getHeight());
        mPerspectiveMatrix = Matrix4::frustum(-ratio, ratio, -1, 1, 1.0f, 20.0f);
        mTranslationMatrix = Matrix4::translate(angle::Vector3(0, 0, -5));

        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glUseProgram(mProgram);

        glEnableVertexAttribArray(mPositionLoc);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, false, 6 * sizeof(GLfloat), nullptr);

        glVertexAttribPointer(mNormalLoc, 3, GL_FLOAT, false, 6 * sizeof(GLfloat),
                              reinterpret_cast<const void *>(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(mNormalLoc);

        glBindBuffer(GL_ARRAY_BUFFER, mHueBuffer);
        glVertexAttribPointer(mHueLoc, 1, GL_FLOAT, false, sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(mHueLoc);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

        return true;
    }

    void destroy() override
    {
        glBindBuffer(GL_ARRAY_BUFFER, mHueBuffer);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(mPositionLoc);
        glDisableVertexAttribArray(mNormalLoc);
        glDisableVertexAttribArray(mHueLoc);
        glDeleteBuffers(1, &mHueBuffer);
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteBuffers(1, &mIndexBuffer);
        glDeleteProgram(mProgram);
    }

    void step(float dt, double totalTime) override
    {
        mAngle += kDegreesPerSecond * dt;
        if (mLastFullSecond != static_cast<uint32_t>(totalTime))
        {
            mLastFullSecond = static_cast<uint32_t>(totalTime);
            regenerateTorus();
        }
        updateHues(totalTime);
    }

    void draw() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Matrix4 modelMatrix = mTranslationMatrix * Matrix4::rotate(mAngle, mYUnitVec) *
                              Matrix4::rotate(mAngle * 0.25f, mXUnitVec);

        Matrix4 mvpMatrix = mPerspectiveMatrix * modelMatrix;

        glUniformMatrix4fv(mMVMatrixLoc, 1, GL_FALSE, modelMatrix.data);
        glUniformMatrix4fv(mMVPMatrixLoc, 1, GL_FALSE, mvpMatrix.data);

        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, false, 6 * sizeof(GLfloat), nullptr);
        glVertexAttribPointer(mNormalLoc, 3, GL_FLOAT, false, 6 * sizeof(GLfloat),
                              reinterpret_cast<const void *>(3 * sizeof(GLfloat)));
        glBindBuffer(GL_ARRAY_BUFFER, mHueBuffer);
        glVertexAttribPointer(mHueLoc, 1, GL_FLOAT, false, sizeof(GLfloat), nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glVertexAttribPointer(mHueLoc, 1, GL_FLOAT, false, sizeof(GLfloat), nullptr);

        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, 0);

        ASSERT(static_cast<GLenum>(GL_NO_ERROR) == glGetError());
    }

    void updateHues(double time)
    {
        for (uint32_t i = 0; i < kHuesSize; i++)
        {
            mHueMapPtr[i] = static_cast<GLfloat>(i) / static_cast<GLfloat>(kHuesSize) +
                            static_cast<GLfloat>(time);
        }

        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glClientWaitSync(sync, 0, 0);
        glDeleteSync(sync);
    }

    void generateTorus()
    {
        GenerateTorus(&mVertexBuffer, &mIndexBuffer, &mIndexCount);

        std::vector<GLfloat> hues(kHuesSize, 0.0f);

        glGenBuffers(1, &mHueBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mHueBuffer);
        glBufferStorageEXT(GL_ARRAY_BUFFER, kHuesSize * sizeof(GLfloat), hues.data(),
                           GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

        mHueMapPtr = static_cast<float *>(
            glMapBufferRange(GL_ARRAY_BUFFER, 0, kHuesSize * sizeof(GLfloat),
                             GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
                                 GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT));

        ASSERT(mHueMapPtr != nullptr);
        ASSERT(static_cast<GLenum>(GL_NO_ERROR) == glGetError());
    }

    void regenerateTorus()
    {
        glBindBuffer(GL_ARRAY_BUFFER, mHueBuffer);
        glDisableVertexAttribArray(mHueLoc);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glDeleteBuffers(1, &mHueBuffer);

        std::vector<GLfloat> hues(kHuesSize, 0.0f);

        glGenBuffers(1, &mHueBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mHueBuffer);
        glBufferStorageEXT(GL_ARRAY_BUFFER, kHuesSize * sizeof(GLfloat), hues.data(),
                           GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

        mHueMapPtr = static_cast<float *>(
            glMapBufferRange(GL_ARRAY_BUFFER, 0, kHuesSize * sizeof(GLfloat),
                             GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
                                 GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT));

        ASSERT(mHueMapPtr != nullptr);
        ASSERT(static_cast<GLenum>(GL_NO_ERROR) == glGetError());

        glEnableVertexAttribArray(mHueLoc);

        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glClientWaitSync(sync, 0, 0);
        glDeleteSync(sync);
    }

  private:
    GLuint mProgram = 0;

    GLint mPositionLoc = 0;
    GLint mNormalLoc   = 0;
    GLint mHueLoc      = 0;

    GLuint mMVPMatrixLoc = 0;
    GLuint mMVMatrixLoc  = 0;

    GLuint mVertexBuffer = 0;
    GLuint mHueBuffer    = 0;
    GLuint mIndexBuffer  = 0;
    GLsizei mIndexCount  = 0;

    Matrix4 mPerspectiveMatrix;
    Matrix4 mTranslationMatrix;

    const angle::Vector3 mYUnitVec{0.0f, 1.0f, 0.0f};
    const angle::Vector3 mXUnitVec{1.0f, 0.0f, 0.0f};

    float *mHueMapPtr = nullptr;

    float mAngle = 0;

    uint32_t mLastFullSecond = 0;
};

int main(int argc, char **argv)
{
    BufferStorageSample app(argc, argv);
    return app.run();
}
