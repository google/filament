//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Based on CubeMapActivity.java from The Android Open Source Project ApiDemos
// https://android.googlesource.com/platform/development/+/refs/heads/master/samples/ApiDemos/src/com/example/android/apis/graphics/CubeMapActivity.java

#include "SampleApplication.h"

#include "torus.h"
#include "util/Matrix.h"
#include "util/shader_utils.h"

const float kDegreesPerSecond = 90.0f;

class GLES2TorusLightingSample : public SampleApplication
{
  public:
    GLES2TorusLightingSample(int argc, char **argv)
        : SampleApplication("GLES2 Torus Lighting", argc, argv)
    {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(uniform mat4 mv;
uniform mat4 mvp;

attribute vec4 position;
attribute vec3 normal;

varying vec3 normal_view;

void main()
{
    normal_view = vec3(mv * vec4(normal, 0.0));
    gl_Position = mvp * position;
})";

        constexpr char kFS[] = R"(precision mediump float;

varying vec3 normal_view;

void main() {
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0) * dot(vec3(0.0, 0, 1.0), normalize(normal_view));
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        mPositionLoc = glGetAttribLocation(mProgram, "position");
        mNormalLoc   = glGetAttribLocation(mProgram, "normal");

        mMVPMatrixLoc = glGetUniformLocation(mProgram, "mvp");
        mMVMatrixLoc  = glGetUniformLocation(mProgram, "mv");

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);

        GenerateTorus(&mVertexBuffer, &mIndexBuffer, &mIndexCount);

        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glUseProgram(mProgram);
        glEnableVertexAttribArray(mPositionLoc);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, false, 6 * sizeof(GLfloat), nullptr);

        glVertexAttribPointer(mNormalLoc, 3, GL_FLOAT, false, 6 * sizeof(GLfloat),
                              reinterpret_cast<const void *>(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(mNormalLoc);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

        float ratio = static_cast<float>(getWindow()->getWidth()) /
                      static_cast<float>(getWindow()->getHeight());
        mPerspectiveMatrix = Matrix4::frustum(-ratio, ratio, -1, 1, 1.0f, 20.0f);
        mTranslationMatrix = Matrix4::translate(angle::Vector3(0, 0, -5));

        return true;
    }

    void destroy() override
    {
        glDisableVertexAttribArray(mPositionLoc);
        glDisableVertexAttribArray(mNormalLoc);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteBuffers(1, &mIndexBuffer);
    }

    void step(float dt, double totalTime) override { mAngle += kDegreesPerSecond * dt; }

    void draw() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Matrix4 modelMatrix = mTranslationMatrix * Matrix4::rotate(mAngle, mYUnitVec) *
                              Matrix4::rotate(mAngle * 0.25f, mXUnitVec);

        Matrix4 mvpMatrix = mPerspectiveMatrix * modelMatrix;

        glUniformMatrix4fv(mMVMatrixLoc, 1, GL_FALSE, modelMatrix.data);
        glUniformMatrix4fv(mMVPMatrixLoc, 1, GL_FALSE, mvpMatrix.data);

        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, 0);
    }

  private:
    GLuint mProgram = 0;

    GLint mPositionLoc = 0;
    GLint mNormalLoc   = 0;

    GLuint mMVPMatrixLoc = 0;
    GLuint mMVMatrixLoc  = 0;

    GLuint mVertexBuffer = 0;
    GLuint mIndexBuffer  = 0;
    GLsizei mIndexCount  = 0;

    float mAngle = 0;

    Matrix4 mPerspectiveMatrix;
    Matrix4 mTranslationMatrix;

    const angle::Vector3 mYUnitVec{0.0f, 1.0f, 0.0f};
    const angle::Vector3 mXUnitVec{1.0f, 0.0f, 0.0f};
};

int main(int argc, char **argv)
{
    GLES2TorusLightingSample app(argc, argv);
    return app.run();
}
