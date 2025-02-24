//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
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

#include "texture_utils.h"
#include "util/gles_loader_autogen.h"
#include "util/shader_utils.h"

class GLES1SimpleTexture2DSample : public SampleApplication
{
  public:
    GLES1SimpleTexture2DSample(int argc, char **argv)
        : SampleApplication("GLES1SimpleTexture2D", argc, argv, ClientType::ES1)
    {}

    bool initialize() override
    {
        // Load the texture
        mTexture = CreateSimpleTexture2D();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_TEXTURE_2D);

        return true;
    }

    void destroy() override { glDeleteTextures(1, &mTexture); }

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

        // Load the vertex position
        glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), vertices);
        // Load the texture coordinate
        glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), vertices + 3);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexture);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }

  private:
    // Texture handle
    GLuint mTexture = 0;
};

int main(int argc, char **argv)
{
    GLES1SimpleTexture2DSample app(argc, argv);
    return app.run();
}
