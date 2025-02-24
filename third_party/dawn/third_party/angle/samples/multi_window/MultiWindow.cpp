//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SampleApplication.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "util/Matrix.h"
#include "util/random_utils.h"
#include "util/shader_utils.h"

using namespace angle;

class MultiWindowSample : public SampleApplication
{
  public:
    MultiWindowSample(int argc, char **argv)
        : SampleApplication("MultiWindow", argc, argv, ClientType::ES2, 256, 256)
    {}

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

        // Set an initial rotation
        mRotation = 45.0f;

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        window rootWindow;
        rootWindow.osWindow = getWindow();
        rootWindow.surface  = getSurface();
        mWindows.push_back(rootWindow);

        const size_t numWindows = 5;
        for (size_t i = 1; i < numWindows; i++)
        {
            window window;

            window.osWindow = OSWindow::New();
            if (!window.osWindow->initialize("MultiWindow", 256, 256))
            {
                return false;
            }

            window.surface = eglCreateWindowSurface(getDisplay(), getConfig(),
                                                    window.osWindow->getNativeWindow(), nullptr);
            if (window.surface == EGL_NO_SURFACE)
            {
                return false;
            }

            window.osWindow->setVisible(true);

            mWindows.push_back(window);
        }

        int baseX = rootWindow.osWindow->getX();
        int baseY = rootWindow.osWindow->getY();
        for (auto &window : mWindows)
        {
            int x      = baseX + mRNG.randomIntBetween(0, 512);
            int y      = baseY + mRNG.randomIntBetween(0, 512);
            int width  = mRNG.randomIntBetween(128, 512);
            int height = mRNG.randomIntBetween(128, 512);
            window.osWindow->setPosition(x, y);
            window.osWindow->resize(width, height);
        }

        return true;
    }

    void destroy() override { glDeleteProgram(mProgram); }

    void step(float dt, double totalTime) override
    {
        mRotation = fmod(mRotation + (dt * 40.0f), 360.0f);

        for (auto &window : mWindows)
        {
            window.osWindow->messageLoop();
        }
    }

    void draw() override
    {
        OSWindow *rootWindow = mWindows[0].osWindow;
        int left             = rootWindow->getX();
        int right            = rootWindow->getX() + rootWindow->getWidth();
        int top              = rootWindow->getY();
        int bottom           = rootWindow->getY() + rootWindow->getHeight();

        for (auto &windowRecord : mWindows)
        {
            OSWindow *window = windowRecord.osWindow;
            left             = std::min(left, window->getX());
            right            = std::max(right, window->getX() + window->getWidth());
            top              = std::min(top, window->getY());
            bottom           = std::max(bottom, window->getY() + window->getHeight());
        }

        float midX = (left + right) * 0.5f;
        float midY = (top + bottom) * 0.5f;

        Matrix4 modelMatrix = Matrix4::translate(Vector3(midX, midY, 0.0f)) *
                              Matrix4::rotate(mRotation, Vector3(0.0f, 0.0f, 1.0f)) *
                              Matrix4::translate(Vector3(-midX, -midY, 0.0f));
        Matrix4 viewMatrix = Matrix4::identity();

        for (auto &windowRecord : mWindows)
        {
            OSWindow *window   = windowRecord.osWindow;
            EGLSurface surface = windowRecord.surface;

            eglMakeCurrent(getDisplay(), surface, surface, getContext());

            Matrix4 orthoMatrix =
                Matrix4::ortho(static_cast<float>(window->getX()),
                               static_cast<float>(window->getX() + window->getWidth()),
                               static_cast<float>(window->getY() + window->getHeight()),
                               static_cast<float>(window->getY()), 0.0f, 1.0f);
            Matrix4 mvpMatrix = orthoMatrix * viewMatrix * modelMatrix;

            Vector3 vertices[] = {
                Matrix4::transform(mvpMatrix, Vector4(midX, static_cast<float>(top), 0.0f, 1.0f)),
                Matrix4::transform(mvpMatrix, Vector4(static_cast<float>(left),
                                                      static_cast<float>(bottom), 0.0f, 1.0f)),
                Matrix4::transform(mvpMatrix, Vector4(static_cast<float>(right),
                                                      static_cast<float>(bottom), 0.0f, 1.0f)),
            };

            // Set the viewport
            glViewport(0, 0, window->getWidth(), window->getHeight());

            // Clear the color buffer
            glClear(GL_COLOR_BUFFER_BIT);

            // Use the program object
            glUseProgram(mProgram);

            // Load the vertex data
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices[0].data());
            glEnableVertexAttribArray(0);

            glDrawArrays(GL_TRIANGLES, 0, 3);

            eglSwapBuffers(getDisplay(), surface);
        }
    }

    // Override swap to do nothing as we already swapped the root
    // window in draw() and swapping another time would invalidate
    // the content of the default framebuffer.
    void swap() override {}

  private:
    // Handle to a program object
    GLuint mProgram;

    // Current rotation
    float mRotation;

    // Window and surface data
    struct window
    {
        OSWindow *osWindow;
        EGLSurface surface;
    };
    std::vector<window> mWindows;

    RNG mRNG;
};

int main(int argc, char **argv)
{
    MultiWindowSample app(argc, argv);
    return app.run();
}
