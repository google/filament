//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
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

class GLES1HelloTriangleSample : public SampleApplication
{
  public:
    GLES1HelloTriangleSample(int argc, char **argv)
        : SampleApplication("GLES1HelloTriangle", argc, argv, ClientType::ES1)
    {}

    bool initialize() override
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    void draw() override
    {
        GLfloat vertices[] = {
            0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
        };

        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glClear(GL_COLOR_BUFFER_BIT);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices);

        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
};

int main(int argc, char **argv)
{
    GLES1HelloTriangleSample app(argc, argv);
    return app.run();
}
