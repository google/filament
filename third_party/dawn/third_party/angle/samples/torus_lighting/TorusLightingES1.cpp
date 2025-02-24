//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Based on CubeMapActivity.java from The Android Open Source Project ApiDemos
// https://android.googlesource.com/platform/development/+/refs/heads/master/samples/ApiDemos/src/com/example/android/apis/graphics/CubeMapActivity.java

#include "SampleApplication.h"
#include "torus.h"

const float kDegreesPerSecond = 90.0f;

class GLES1TorusLightingSample : public SampleApplication
{
  public:
    GLES1TorusLightingSample(int argc, char **argv)
        : SampleApplication("GLES1 Torus Lighting", argc, argv, ClientType::ES1)
    {}

    bool initialize() override
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glShadeModel(GL_SMOOTH);

        glEnable(GL_LIGHTING);
        GLfloat light_model_ambient[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);

        glEnable(GL_LIGHT0);
        GLfloat lightDir[] = {0.0f, 0.0f, 1.0f, 0.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, lightDir);

        GenerateTorus(&mVertexBuffer, &mIndexBuffer, &mIndexCount);

        glEnableClientState(GL_VERTEX_ARRAY);

        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), nullptr);

        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 6 * sizeof(GLfloat),
                        reinterpret_cast<const void *>(3 * sizeof(GLfloat)));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);

        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        float ratio = static_cast<float>(getWindow()->getWidth()) /
                      static_cast<float>(getWindow()->getHeight());
        glMatrixMode(GL_PROJECTION);
        glFrustumf(-ratio, ratio, -1, 1, 1.0f, 20.0f);

        return true;
    }

    void destroy() override
    {
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteBuffers(1, &mIndexBuffer);
    }

    void step(float dt, double totalTime) override { mAngle += kDegreesPerSecond * dt; }

    void draw() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0, 0, -5);
        glRotatef(mAngle, 0, 1, 0);
        glRotatef(mAngle * 0.25f, 1, 0, 0);

        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, 0);
    }

  private:
    GLuint mVertexBuffer = 0;
    GLuint mIndexBuffer  = 0;
    GLsizei mIndexCount  = 0;
    float mAngle         = 0;
};

int main(int argc, char **argv)
{
    GLES1TorusLightingSample app(argc, argv);
    return app.run();
}
