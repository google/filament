//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This sample shows basic usage of the GL_OVR_multiview2 extension.

#include "SampleApplication.h"

#include "util/geometry_utils.h"
#include "util/shader_utils.h"

#include <iostream>

namespace
{

void FillTranslationMatrix(float xOffset, float yOffset, float zOffset, float *matrix)
{
    matrix[0] = 1.0f;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = xOffset;

    matrix[4] = 0.0f;
    matrix[5] = 1.0f;
    matrix[6] = 0.0f;
    matrix[7] = yOffset;

    matrix[8]  = 0.0f;
    matrix[9]  = 0.0f;
    matrix[10] = 1.0f;
    matrix[11] = zOffset;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

}  // namespace

class MultiviewSample : public SampleApplication
{
  public:
    MultiviewSample(int argc, char **argv)
        : SampleApplication("Multiview", argc, argv, ClientType::ES3_0),
          mMultiviewProgram(0),
          mMultiviewPersperiveUniformLoc(-1),
          mMultiviewLeftEyeCameraUniformLoc(-1),
          mMultiviewRightEyeCameraUniformLoc(-1),
          mMultiviewTranslationUniformLoc(-1),
          mMultiviewFBO(0),
          mColorTexture(0),
          mDepthTexture(0),
          mQuadVAO(0),
          mQuadVBO(0),
          mCubeVAO(0),
          mCubePosVBO(0),
          mCubeNormalVBO(0),
          mCubeIBO(0),
          mCombineProgram(0)
    {}

    bool initialize() override
    {
        // Check whether the GL_OVR_multiview(2) extension is supported. If not, abort
        // initialization.
        const char *allExtensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
        const std::string paddedExtensions = std::string(" ") + allExtensions + std::string(" ");
        if ((paddedExtensions.find(std::string(" GL_OVR_multiview2 ")) == std::string::npos) &&
            (paddedExtensions.find(std::string(" GL_OVR_multiview ")) == std::string::npos))
        {
            std::cout << "GL_OVR_multiview(2) is not available." << std::endl;
            return false;
        }

        // A view covers horizontally half of the screen.
        int viewWidth  = getWindow()->getWidth() / 2;
        int viewHeight = getWindow()->getHeight();

        // Create color and depth texture arrays with two layers to which we render each view.
        glGenTextures(1, &mColorTexture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, mColorTexture);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, viewWidth, viewHeight, 2, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glGenTextures(1, &mDepthTexture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, mDepthTexture);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, viewWidth, viewHeight, 2, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        // Generate multiview framebuffer for layered rendering.
        glGenFramebuffers(1, &mMultiviewFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mMultiviewFBO);
        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColorTexture, 0, 0,
                                         2);
        glFramebufferTextureMultiviewOVR(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture, 0, 0,
                                         2);
        GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
        glDrawBuffers(1, &drawBuffer);

        // Check that the framebuffer is complete. Abort initialization otherwise.
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            return false;
        }

        // Create multiview program and query the uniform locations.
        // The program has two code paths based on the gl_ViewID_OVR attribute which tells us which
        // view is currently being rendered to. Based on it we decide which eye's camera matrix to
        // use.
        constexpr char kMultiviewVS[] =
            "#version 300 es\n"
            "#extension GL_OVR_multiview2 : require\n"
            "layout(num_views = 2) in;\n"
            "layout(location=0) in vec3 posIn;\n"
            "layout(location=1) in vec3 normalIn;\n"
            "uniform mat4 uPerspective;\n"
            "uniform mat4 uCameraLeftEye;\n"
            "uniform mat4 uCameraRightEye;\n"
            "uniform mat4 uTranslation;\n"
            "out vec3 oNormal;\n"
            "void main()\n"
            "{\n"
            "   vec4 p = uTranslation * vec4(posIn,1.);\n"
            "   if (gl_ViewID_OVR == 0u) {\n"
            "       p = uCameraLeftEye * p;\n"
            "   } else {\n"
            "       p = uCameraRightEye * p;\n"
            "   }\n"
            "   oNormal = normalIn;\n"
            "   gl_Position = uPerspective * p;\n"
            "}\n";

        constexpr char kMultiviewFS[] =
            "#version 300 es\n"
            "#extension GL_OVR_multiview2 : require\n"
            "precision mediump float;\n"
            "out vec4 color;\n"
            "in vec3 oNormal;\n"
            "void main()\n"
            "{\n"
            "   vec3 col = 0.5 * oNormal + vec3(0.5);\n"
            "   color = vec4(col, 1.);\n"
            "}\n";

        mMultiviewProgram = CompileProgram(kMultiviewVS, kMultiviewFS);
        if (!mMultiviewProgram)
        {
            return false;
        }
        mMultiviewPersperiveUniformLoc = glGetUniformLocation(mMultiviewProgram, "uPerspective");
        mMultiviewLeftEyeCameraUniformLoc =
            glGetUniformLocation(mMultiviewProgram, "uCameraLeftEye");
        mMultiviewRightEyeCameraUniformLoc =
            glGetUniformLocation(mMultiviewProgram, "uCameraRightEye");
        mMultiviewTranslationUniformLoc = glGetUniformLocation(mMultiviewProgram, "uTranslation");

        // Create a normal program to combine both layers of the color array texture.
        constexpr char kCombineVS[] =
            "#version 300 es\n"
            "in vec2 vIn;\n"
            "out vec2 uv;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = vec4(vIn, 0., 1.);\n"
            "   uv = vIn * .5 + vec2(.5);\n"
            "}\n";

        constexpr char kCombineFS[] =
            "#version 300 es\n"
            "precision mediump float;\n"
            "precision mediump sampler2DArray;\n"
            "uniform sampler2DArray uMultiviewTex;\n"
            "in vec2 uv;\n"
            "out vec4 color;\n"
            "void main()\n"
            "{\n"
            "   float scaledX = 2.0 * uv.x;\n"
            "   float layer = floor(scaledX);\n"
            "   vec2 adjustedUV = vec2(fract(scaledX), uv.y);\n"
            "   vec3 texColor = texture(uMultiviewTex, vec3(adjustedUV, layer)).rgb;\n"
            "   color = vec4(texColor, 1.);\n"
            "}\n";

        mCombineProgram = CompileProgram(kCombineVS, kCombineFS);
        if (!mCombineProgram)
        {
            return false;
        }

        // Generate a quad which covers the whole screen.
        glGenVertexArrays(1, &mQuadVAO);
        glBindVertexArray(mQuadVAO);

        glGenBuffers(1, &mQuadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
        const float kQuadPositionData[] = {1.f, -1.f, 1.f, 1.f, -1.f, -1.f, -1.f, 1.f};
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, kQuadPositionData, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        // Generate a cube.
        GenerateCubeGeometry(1.0f, &mCube);
        glGenVertexArrays(1, &mCubeVAO);
        glBindVertexArray(mCubeVAO);

        glGenBuffers(1, &mCubePosVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mCubePosVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(angle::Vector3) * mCube.positions.size(),
                     mCube.positions.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &mCubeNormalVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mCubeNormalVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(angle::Vector3) * mCube.normals.size(),
                     mCube.normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &mCubeIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mCubeIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mCube.indices.size(),
                     mCube.indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    void destroy() override
    {
        glDeleteProgram(mMultiviewProgram);
        glDeleteFramebuffers(1, &mMultiviewFBO);
        glDeleteTextures(1, &mColorTexture);
        glDeleteTextures(1, &mDepthTexture);
        glDeleteVertexArrays(1, &mQuadVAO);
        glDeleteBuffers(1, &mQuadVBO);
        glDeleteVertexArrays(1, &mCubeVAO);
        glDeleteBuffers(1, &mQuadVBO);
        glDeleteBuffers(1, &mCubePosVBO);
        glDeleteBuffers(1, &mCubeNormalVBO);
        glDeleteBuffers(1, &mCubeIBO);
        glDeleteProgram(mCombineProgram);
    }

    void draw() override
    {
        // Draw to multiview fbo.
        {
            // Generate the perspective projection matrix.
            const int viewWidth          = getWindow()->getWidth() / 2;
            const int viewHeight         = getWindow()->getHeight();
            const float kFOV             = 90.f;
            const float kNear            = 1.0f;
            const float kFar             = 100.0f;
            const float kPlaneDifference = kFar - kNear;
            const float kXYScale         = 1.f / (tanf(kFOV / 2.0f));
            const float kAspectRatio     = static_cast<float>(viewWidth) / viewHeight;
            float kPerspectiveProjectionMatrix[16];
            kPerspectiveProjectionMatrix[0] = kXYScale / kAspectRatio;
            kPerspectiveProjectionMatrix[1] = .0f;
            kPerspectiveProjectionMatrix[2] = .0f;
            kPerspectiveProjectionMatrix[3] = .0f;

            kPerspectiveProjectionMatrix[4] = .0f;
            kPerspectiveProjectionMatrix[5] = kXYScale;
            kPerspectiveProjectionMatrix[6] = .0f;
            kPerspectiveProjectionMatrix[7] = .0f;

            kPerspectiveProjectionMatrix[8]  = .0f;
            kPerspectiveProjectionMatrix[9]  = .0;
            kPerspectiveProjectionMatrix[10] = -kFar / kPlaneDifference;
            kPerspectiveProjectionMatrix[11] = -1.f;

            kPerspectiveProjectionMatrix[12] = .0f;
            kPerspectiveProjectionMatrix[13] = .0;
            kPerspectiveProjectionMatrix[14] = -kFar * kNear / kPlaneDifference;
            kPerspectiveProjectionMatrix[15] = .0;

            // Generate the camera matrices for the left and right eye.
            const float kXOffset = 1.5f;
            const float kYOffset = 1.5f;
            const float kZOffset = 5.0f;
            float kLeftCameraMatrix[16];
            FillTranslationMatrix(kXOffset, -kYOffset, -kZOffset, kLeftCameraMatrix);
            float kRightCameraMatrix[16];
            FillTranslationMatrix(-kXOffset, -kYOffset, -kZOffset, kRightCameraMatrix);

            // Bind and clear the multiview framebuffer.
            glBindFramebuffer(GL_FRAMEBUFFER, mMultiviewFBO);
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set the viewport to be the size as one of the views.
            glViewport(0, 0, viewWidth, viewHeight);

            // Bind multiview program and set matrices.
            glUseProgram(mMultiviewProgram);
            glUniformMatrix4fv(mMultiviewPersperiveUniformLoc, 1, GL_TRUE,
                               kPerspectiveProjectionMatrix);
            glUniformMatrix4fv(mMultiviewLeftEyeCameraUniformLoc, 1, GL_TRUE, kLeftCameraMatrix);
            glUniformMatrix4fv(mMultiviewRightEyeCameraUniformLoc, 1, GL_TRUE, kRightCameraMatrix);

            glBindVertexArray(mCubeVAO);

            // Draw first cube.
            float kTranslationMatrix[16];
            FillTranslationMatrix(0.0f, 0.0f, 0.0f, kTranslationMatrix);
            glUniformMatrix4fv(mMultiviewTranslationUniformLoc, 1, GL_TRUE, kTranslationMatrix);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mCube.indices.size()),
                           GL_UNSIGNED_SHORT, nullptr);

            // Draw second cube.
            FillTranslationMatrix(1.0f, 1.0f, -2.0f, kTranslationMatrix);
            glUniformMatrix4fv(mMultiviewTranslationUniformLoc, 1, GL_TRUE, kTranslationMatrix);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mCube.indices.size()),
                           GL_UNSIGNED_SHORT, nullptr);

            glBindVertexArray(0);
        }

        // Combine both views.
        {
            // Bind the default framebuffer object and clear.
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Set the viewport to cover the whole screen.
            glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

            glUseProgram(mCombineProgram);

            // Bind the 2D array texture to be used as a sampler.
            glUniform1i(glGetUniformLocation(mCombineProgram, "uMultiviewTex"), 0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, mMultiviewFBO);
            glActiveTexture(GL_TEXTURE0);

            // Draw a quad which covers the whole screen. Layer and texture coordinates are
            // calculated in the vertex shader based on the UV coordinates of the quad.
            glBindVertexArray(mQuadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        }
    }

  private:
    GLuint mMultiviewProgram;
    GLint mMultiviewPersperiveUniformLoc;
    GLint mMultiviewLeftEyeCameraUniformLoc;
    GLint mMultiviewRightEyeCameraUniformLoc;
    GLint mMultiviewTranslationUniformLoc;

    GLuint mMultiviewFBO;
    GLuint mColorTexture;
    GLuint mDepthTexture;

    GLuint mQuadVAO;
    GLuint mQuadVBO;

    CubeGeometry mCube;
    GLuint mCubeVAO;
    GLuint mCubePosVBO;
    GLuint mCubeNormalVBO;
    GLuint mCubeIBO;

    GLuint mCombineProgram;
};

int main(int argc, char **argv)
{
    MultiviewSample app(argc, argv);
    return app.run();
}
