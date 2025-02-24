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

#include "texture_utils.h"
#include "util/shader_utils.h"
#include "util/test_utils.h"

#include <cstring>
#include <iostream>

class MultipleDrawBuffersSample : public SampleApplication
{
  public:
    MultipleDrawBuffersSample(int argc, char **argv)
        : SampleApplication("MultipleDrawBuffers", argc, argv)
    {}

    bool initialize() override
    {
        // Check EXT_draw_buffers is supported
        char *extensionString = (char *)glGetString(GL_EXTENSIONS);
        if (strstr(extensionString, "GL_EXT_draw_buffers") != nullptr)
        {
            // Retrieve the address of glDrawBuffersEXT from EGL
            mDrawBuffers = (PFNGLDRAWBUFFERSEXTPROC)eglGetProcAddress("glDrawBuffersEXT");
        }
        else
        {
            mDrawBuffers = glDrawBuffers;
        }

        if (!mDrawBuffers)
        {
            std::cerr << "Unable to load glDrawBuffers[EXT] entry point.";
            return false;
        }

        std::stringstream vsStream;
        vsStream << angle::GetExecutableDirectory() << "/multiple_draw_buffers_vs.glsl";

        std::stringstream fsStream;
        fsStream << angle::GetExecutableDirectory() << "/multiple_draw_buffers_fs.glsl";

        std::stringstream copyFsStream;
        copyFsStream << angle::GetExecutableDirectory() << "/multiple_draw_buffers_copy_fs.glsl";

        mMRTProgram = CompileProgramFromFiles(vsStream.str(), fsStream.str());
        if (!mMRTProgram)
        {
            return false;
        }

        mCopyProgram = CompileProgramFromFiles(vsStream.str(), copyFsStream.str());
        if (!mCopyProgram)
        {
            return false;
        }

        // Get the attribute locations
        mPositionLoc = glGetAttribLocation(mCopyProgram, "a_position");
        mTexCoordLoc = glGetAttribLocation(mCopyProgram, "a_texCoord");

        // Get the sampler location
        mSamplerLoc = glGetUniformLocation(mCopyProgram, "s_texture");

        // Load the texture
        mTexture = CreateSimpleTexture2D();

        // Initialize the user framebuffer
        glGenFramebuffers(1, &mFramebuffer);
        glGenTextures(mFramebufferAttachmentCount, mFramebufferTextures);

        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        for (size_t i = 0; i < mFramebufferAttachmentCount; i++)
        {
            // Create textures for the four color attachments
            glBindTexture(GL_TEXTURE_2D, mFramebufferTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, getWindow()->getWidth(),
                         getWindow()->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   static_cast<GLenum>(GL_COLOR_ATTACHMENT0_EXT + i), GL_TEXTURE_2D,
                                   mFramebufferTextures[i], 0);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    void destroy() override
    {
        glDeleteProgram(mCopyProgram);
        glDeleteProgram(mMRTProgram);
        glDeleteTextures(1, &mTexture);
        glDeleteTextures(mFramebufferAttachmentCount, mFramebufferTextures);
        glDeleteFramebuffers(1, &mFramebuffer);
    }

    void draw() override
    {
        GLfloat vertices[] = {
            -0.8f, 0.8f,  0.0f,  // Position 0
            0.0f,  0.0f,         // TexCoord 0
            -0.8f, -0.8f, 0.0f,  // Position 1
            0.0f,  1.0f,         // TexCoord 1
            0.8f,  -0.8f, 0.0f,  // Position 2
            1.0f,  1.0f,         // TexCoord 2
            0.8f,  0.8f,  0.0f,  // Position 3
            1.0f,  0.0f          // TexCoord 3
        };
        GLushort indices[]                              = {0, 1, 2, 0, 2, 3};
        GLenum drawBuffers[mFramebufferAttachmentCount] = {
            GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT,
            GL_COLOR_ATTACHMENT3_EXT};

        // Enable drawing to the four color attachments of the user framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        mDrawBuffers(mFramebufferAttachmentCount, drawBuffers);

        // Set the viewport
        GLint width  = static_cast<GLint>(getWindow()->getWidth());
        GLint height = static_cast<GLint>(getWindow()->getHeight());
        glViewport(0, 0, width, height);

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mMRTProgram);

        // Load the vertex position
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices);
        glEnableVertexAttribArray(mPositionLoc);

        // Load the texture coordinate
        glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                              vertices + 3);
        glEnableVertexAttribArray(mTexCoordLoc);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexture);

        // Set the sampler texture unit to 0
        glUniform1i(mSamplerLoc, 0);

        // Draw the textured quad to the four render targets
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        // Enable the default framebuffer and single textured drawing
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(mCopyProgram);

        // Draw the four textured quads to a separate region in the viewport
        glBindTexture(GL_TEXTURE_2D, mFramebufferTextures[0]);
        glViewport(0, 0, width / 2, height / 2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        glBindTexture(GL_TEXTURE_2D, mFramebufferTextures[1]);
        glViewport(width / 2, 0, width / 2, height / 2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        glBindTexture(GL_TEXTURE_2D, mFramebufferTextures[2]);
        glViewport(0, height / 2, width / 2, height / 2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        glBindTexture(GL_TEXTURE_2D, mFramebufferTextures[3]);
        glViewport(width / 2, height / 2, width / 2, height / 2);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }

  private:
    // Handle to a program object
    GLuint mMRTProgram;
    GLuint mCopyProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mTexCoordLoc;

    // Sampler location
    GLint mSamplerLoc;

    // Texture handle
    GLuint mTexture;

    // Framebuffer object handle
    GLuint mFramebuffer;

    // Framebuffer color attachments
    static const size_t mFramebufferAttachmentCount = 4;
    GLuint mFramebufferTextures[mFramebufferAttachmentCount];

    // Loaded draw buffer entry points
    PFNGLDRAWBUFFERSEXTPROC mDrawBuffers;
};

int main(int argc, char **argv)
{
    MultipleDrawBuffersSample app(argc, argv);
    return app.run();
}
