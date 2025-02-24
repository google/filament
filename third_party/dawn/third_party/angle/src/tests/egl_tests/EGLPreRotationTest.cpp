//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLPreRotationTest:
//   Tests pertaining to Android pre-rotation.
//

#include <gtest/gtest.h>

#include <vector>

#include "common/Color.h"
#include "common/platform.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"
#include "util/Timer.h"
#include "util/test_utils.h"

using namespace angle;

namespace
{

using EGLPreRotationSurfaceTestParams = std::tuple<angle::PlatformParameters, bool>;

std::string PrintToStringParamName(
    const ::testing::TestParamInfo<EGLPreRotationSurfaceTestParams> &info)
{
    std::stringstream ss;
    ss << std::get<0>(info.param);
    if (std::get<1>(info.param))
    {
        ss << "__PreRotationEnabled";
    }
    else
    {
        ss << "__PreRotationDisabled";
    }
    return ss.str();
}

// A class to test various Android pre-rotation cases.  In order to make it easier to debug test
// failures, the initial window size is 256x256, and each pixel will have a unique and predictable
// value.  The red channel will increment with the x axis, and the green channel will increment
// with the y axis.  The four corners will have the following values:
//
// Where                 GLES Render &  ReadPixels coords       Color    (in Hex)
// Lower-left,  which is (-1.0,-1.0) & (  0,   0) in GLES will be black  (0x00, 0x00, 0x00, 0xFF)
// Lower-right, which is ( 1.0,-1.0) & (256,   0) in GLES will be red    (0xFF, 0x00, 0x00, 0xFF)
// Upper-left,  which is (-1.0, 1.0) & (  0, 256) in GLES will be green  (0x00, 0xFF, 0x00, 0xFF)
// Upper-right, which is ( 1.0, 1.0) & (256, 256) in GLES will be yellow (0xFF, 0xFF, 0x00, 0xFF)
class EGLPreRotationSurfaceTest : public ANGLETest<EGLPreRotationSurfaceTestParams>
{
  protected:
    EGLPreRotationSurfaceTest()
        : mDisplay(EGL_NO_DISPLAY),
          mWindowSurface(EGL_NO_SURFACE),
          mContext(EGL_NO_CONTEXT),
          mOSWindow(nullptr),
          mSize(256)
    {}

    // Release any resources created in the test body
    void testTearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (mWindowSurface != EGL_NO_SURFACE)
            {
                eglDestroySurface(mDisplay, mWindowSurface);
                mWindowSurface = EGL_NO_SURFACE;
            }

            if (mContext != EGL_NO_CONTEXT)
            {
                eglDestroyContext(mDisplay, mContext);
                mContext = EGL_NO_CONTEXT;
            }

            eglTerminate(mDisplay);
            mDisplay = EGL_NO_DISPLAY;
        }

        mOSWindow->destroy();
        OSWindow::Delete(&mOSWindow);

        ASSERT_TRUE(mWindowSurface == EGL_NO_SURFACE && mContext == EGL_NO_CONTEXT);
    }

    void testSetUp() override
    {
        mOSWindow = OSWindow::New();
        mOSWindow->initialize("EGLSurfaceTest", mSize, mSize);
    }

    void initializeDisplay()
    {
        const angle::PlatformParameters platform = ::testing::get<0>(GetParam());
        GLenum platformType                      = platform.getRenderer();
        GLenum deviceType                        = platform.getDeviceType();

        std::vector<const char *> enabledFeatures;
        std::vector<const char *> disabledFeatures;
        if (::testing::get<1>(GetParam()))
        {
            enabledFeatures.push_back("enablePreRotateSurfaces");
        }
        else
        {
            disabledFeatures.push_back("enablePreRotateSurfaces");
        }
        enabledFeatures.push_back(nullptr);
        disabledFeatures.push_back(nullptr);

        std::vector<EGLAttrib> displayAttributes;
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
        displayAttributes.push_back(platformType);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
        displayAttributes.push_back(EGL_DONT_CARE);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
        displayAttributes.push_back(EGL_DONT_CARE);
        displayAttributes.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
        displayAttributes.push_back(deviceType);
        displayAttributes.push_back(EGL_FEATURE_OVERRIDES_ENABLED_ANGLE);
        displayAttributes.push_back(reinterpret_cast<EGLAttrib>(enabledFeatures.data()));
        displayAttributes.push_back(EGL_FEATURE_OVERRIDES_DISABLED_ANGLE);
        displayAttributes.push_back(reinterpret_cast<EGLAttrib>(disabledFeatures.data()));
        displayAttributes.push_back(EGL_NONE);

        mDisplay = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE,
                                         reinterpret_cast<void *>(mOSWindow->getNativeDisplay()),
                                         displayAttributes.data());
        ASSERT_TRUE(mDisplay != EGL_NO_DISPLAY);

        EGLint majorVersion, minorVersion;
        ASSERT_TRUE(eglInitialize(mDisplay, &majorVersion, &minorVersion) == EGL_TRUE);

        eglBindAPI(EGL_OPENGL_ES_API);
        ASSERT_EGL_SUCCESS();
    }

    void initializeContext()
    {
        EGLint contextAttibutes[] = {EGL_CONTEXT_CLIENT_VERSION,
                                     ::testing::get<0>(GetParam()).majorVersion, EGL_NONE};

        mContext = eglCreateContext(mDisplay, mConfig, nullptr, contextAttibutes);
        ASSERT_EGL_SUCCESS();
    }

    void initializeSurfaceWithRGBA8888Config()
    {
        const EGLint configAttributes[] = {
            EGL_RED_SIZE,   8, EGL_GREEN_SIZE,   8, EGL_BLUE_SIZE,      8, EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 0, EGL_STENCIL_SIZE, 0, EGL_SAMPLE_BUFFERS, 0, EGL_NONE};

        EGLint configCount;
        EGLConfig config;
        ASSERT_TRUE(eglChooseConfig(mDisplay, configAttributes, &config, 1, &configCount) ||
                    (configCount != 1) == EGL_TRUE);

        mConfig = config;

        EGLint surfaceType = EGL_NONE;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_SURFACE_TYPE, &surfaceType);

        std::vector<EGLint> windowAttributes;
        windowAttributes.push_back(EGL_NONE);

        if (surfaceType & EGL_WINDOW_BIT)
        {
            // Create first window surface
            mWindowSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(),
                                                    windowAttributes.data());
            ASSERT_EGL_SUCCESS();
        }

        initializeContext();
    }

    void initializeSurfaceWithRGBA8888d24s8Config()
    {
        const EGLint configAttributes[] = {
            EGL_RED_SIZE,   8,  EGL_GREEN_SIZE,   8, EGL_BLUE_SIZE,      8, EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8, EGL_SAMPLE_BUFFERS, 0, EGL_NONE};

        EGLint configCount;
        EGLConfig config;
        ASSERT_TRUE(eglChooseConfig(mDisplay, configAttributes, &config, 1, &configCount) ||
                    (configCount != 1) == EGL_TRUE);

        mConfig = config;

        EGLint surfaceType = EGL_NONE;
        eglGetConfigAttrib(mDisplay, mConfig, EGL_SURFACE_TYPE, &surfaceType);

        std::vector<EGLint> windowAttributes;
        windowAttributes.push_back(EGL_NONE);

        if (surfaceType & EGL_WINDOW_BIT)
        {
            // Create first window surface
            mWindowSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(),
                                                    windowAttributes.data());
            ASSERT_EGL_SUCCESS();
        }

        initializeContext();
    }

    void testDrawingAndReadPixels()
    {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(0, mSize - 1, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(mSize - 1, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(mSize - 1, mSize - 1, GLColor::yellow);
        ASSERT_GL_NO_ERROR();

        eglSwapBuffers(mDisplay, mWindowSurface);
        ASSERT_EGL_SUCCESS();

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(0, mSize - 1, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(mSize - 1, 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(mSize - 1, mSize - 1, GLColor::yellow);
        ASSERT_GL_NO_ERROR();

        {
            // Now, test a 4x4 area in the center of the window, which should tell us if a non-1x1
            // ReadPixels is oriented correctly for the device's orientation:
            GLint xOffset  = 126;
            GLint yOffset  = 126;
            GLsizei width  = 4;
            GLsizei height = 4;
            std::vector<GLColor> pixels(width * height);
            glReadPixels(xOffset, yOffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
            EXPECT_GL_NO_ERROR();
            // Expect that all red values equate to x and green values equate to y
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int index = (y * width) + x;
                    GLColor expectedPixel(xOffset + x, yOffset + y, 0, 255);
                    GLColor actualPixel = pixels[index];
                    EXPECT_EQ(expectedPixel, actualPixel);
                }
            }
        }

        {
            // Now, test a 8x4 area off-the-center of the window, just to make sure that works too:
            GLint xOffset  = 13;
            GLint yOffset  = 26;
            GLsizei width  = 8;
            GLsizei height = 4;
            std::vector<GLColor> pixels2(width * height);
            glReadPixels(xOffset, yOffset, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels2[0]);
            EXPECT_GL_NO_ERROR();
            // Expect that all red values equate to x and green values equate to y
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int index = (y * width) + x;
                    GLColor expectedPixel(xOffset + x, yOffset + y, 0, 255);
                    GLColor actualPixel = pixels2[index];
                    EXPECT_EQ(expectedPixel, actualPixel);
                }
            }
        }

        eglSwapBuffers(mDisplay, mWindowSurface);
        ASSERT_EGL_SUCCESS();
    }

    EGLDisplay mDisplay;
    EGLSurface mWindowSurface;
    EGLContext mContext;
    EGLConfig mConfig;
    OSWindow *mOSWindow;
    int mSize;
};

// Provide a predictable pattern for testing pre-rotation
TEST_P(EGLPreRotationSurfaceTest, OrientedWindowWithDraw)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "attribute vec2 redGreen;\n"
        "varying vec2 v_data;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "  v_data = redGreen;\n"
        "}";

    constexpr char kFS[] =
        "varying highp vec2 v_data;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(v_data, 0, 1);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);

    GLint redGreenLocation = glGetAttribLocation(program, "redGreen");
    ASSERT_NE(-1, redGreenLocation);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];

    glBindVertexArray(vertexArray);

    std::vector<GLushort> indices = {0, 1, 2, 2, 3, 0};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), &indices[0],
                 GL_STATIC_DRAW);

    std::vector<GLfloat> positionData = {// quad vertices
                                         -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(positionLocation);

    std::vector<GLfloat> redGreenData = {// green(0,1), black(0,0), red(1,0), yellow(1,1)
                                         0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * redGreenData.size(), &redGreenData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(redGreenLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(redGreenLocation);

    ASSERT_GL_NO_ERROR();

    testDrawingAndReadPixels();
}

// Use dFdx() and dFdy() and still provide a predictable pattern for testing pre-rotation
// In this case, the color values will be the following: (dFdx(v_data.x), dFdy(v_data.y), 0, 1).
// To help make this meaningful for pre-rotation, the derivatives will vary in the four corners of
// the window:
//
//  +------------+------------+      +--------+--------+
//  | (  0, 219) | (239, 249) |      | Green  | Yellow |
//  +------------+------------+  OR  +--------+--------+
//  | (  0,   0) | (229,   0) |      | Black  |  Red   |
//  +------------+------------+      +--------+--------+
TEST_P(EGLPreRotationSurfaceTest, OrientedWindowWithDerivativeDraw)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    constexpr char kVS[] =
        "#version 300 es\n"
        "in highp vec2 position;\n"
        "in highp vec2 redGreen;\n"
        "out highp vec2 v_data;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "  v_data = redGreen;\n"
        "}";

    constexpr char kFS[] =
        "#version 300 es\n"
        "in highp vec2 v_data;\n"
        "out highp vec4 FragColor;\n"
        "void main() {\n"
        "  FragColor = vec4(dFdx(v_data.x), dFdy(v_data.y), 0, 1);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);

    GLint redGreenLocation = glGetAttribLocation(program, "redGreen");
    ASSERT_NE(-1, redGreenLocation);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];

    glBindVertexArray(vertexArray);

    std::vector<GLushort> indices = {// 4 squares each made up of 6 vertices:
                                     // 1st square, in the upper-left part of window
                                     0, 1, 2, 2, 3, 0,
                                     // 2nd square, in the upper-right part of window
                                     4, 5, 6, 6, 7, 4,
                                     // 3rd square, in the lower-left part of window
                                     8, 9, 10, 10, 11, 8,
                                     // 4th square, in the lower-right part of window
                                     12, 13, 14, 14, 15, 12};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), &indices[0],
                 GL_STATIC_DRAW);

    std::vector<GLfloat> positionData = {// 4 squares each made up of quad vertices
                                         // 1st square, in the upper-left part of window
                                         -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                         // 2nd square, in the upper-right part of window
                                         0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                                         // 3rd square, in the lower-left part of window
                                         -1.0f, 0.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                                         // 4th square, in the lower-right part of window
                                         0.0f, 0.0f, 0.0f, -1.0f, 1.0f, -1.0f, 1.0f, 0.0f};

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(positionLocation);

    std::vector<GLfloat> redGreenData = {// green(0,110), black(0,0), red(115,0), yellow(120,125)
                                         // 4 squares each made up of 4 pairs of half-color values:
                                         // 1st square, in the upper-left part of window
                                         0.0f, 110.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 110.0f,
                                         // 2nd square, in the upper-right part of window
                                         0.0f, 125.0f, 0.0f, 0.0f, 120.0f, 0.0f, 120.0f, 125.0f,
                                         // 3rd square, in the lower-left part of window
                                         0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                         // 4th square, in the lower-right part of window
                                         0.0f, 0.0f, 0.0f, 0.0f, 115.0f, 0.0f, 115.0f, 0.0f};

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * redGreenData.size(), &redGreenData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(redGreenLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(redGreenLocation);

    ASSERT_GL_NO_ERROR();

    // Draw and check the 4 corner pixels, to ensure we're getting the expected "colors"
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr);
    GLColor expectedPixelLowerLeft(0, 0, 0, 255);
    GLColor expectedPixelLowerRight(229, 0, 0, 255);
    GLColor expectedPixelUpperLeft(0, 219, 0, 255);
    GLColor expectedPixelUpperRight(239, 249, 0, 255);
    EXPECT_PIXEL_COLOR_EQ(0, 0, expectedPixelLowerLeft);
    EXPECT_PIXEL_COLOR_EQ(mSize - 1, 0, expectedPixelLowerRight);
    EXPECT_PIXEL_COLOR_EQ(0, mSize - 1, expectedPixelUpperLeft);
    EXPECT_PIXEL_COLOR_EQ(mSize - 1, mSize - 1, expectedPixelUpperRight);
    ASSERT_GL_NO_ERROR();

    // Make the image visible
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    // Draw again and check the 4 center pixels, to ensure we're getting the expected "colors"
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr);
    EXPECT_PIXEL_COLOR_EQ((mSize / 2) - 1, (mSize / 2) - 1, expectedPixelLowerLeft);
    EXPECT_PIXEL_COLOR_EQ((mSize / 2) - 1, (mSize / 2), expectedPixelUpperLeft);
    EXPECT_PIXEL_COLOR_EQ((mSize / 2), (mSize / 2) - 1, expectedPixelLowerRight);
    EXPECT_PIXEL_COLOR_EQ((mSize / 2), (mSize / 2), expectedPixelUpperRight);
    ASSERT_GL_NO_ERROR();
}

// Android-specific test that changes a window's rotation, which requires ContextVk::syncState() to
// handle the new rotation
TEST_P(EGLPreRotationSurfaceTest, ChangeRotationWithDraw)
{
    // This test uses functionality that is only available on Android
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && !IsAndroid());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "attribute vec2 redGreen;\n"
        "varying vec2 v_data;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "  v_data = redGreen;\n"
        "}";

    constexpr char kFS[] =
        "varying highp vec2 v_data;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(v_data, 0, 1);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);

    GLint redGreenLocation = glGetAttribLocation(program, "redGreen");
    ASSERT_NE(-1, redGreenLocation);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];

    glBindVertexArray(vertexArray);

    std::vector<GLushort> indices = {0, 1, 2, 2, 3, 0};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), &indices[0],
                 GL_STATIC_DRAW);

    std::vector<GLfloat> positionData = {// quad vertices
                                         -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(positionLocation);

    std::vector<GLfloat> redGreenData = {// green(0,1), black(0,0), red(1,0), yellow(1,1)
                                         0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * redGreenData.size(), &redGreenData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(redGreenLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(redGreenLocation);

    ASSERT_GL_NO_ERROR();

    // Change the rotation back and forth between landscape and portrait, and make sure that the
    // drawing and reading happen consistently with the desired rotation.
    // Last rotation needs to be portrait, since other tests expect it to be the default.
    for (int i = 0; i < 4; i++)
    {
        bool landscape;
        EGLint actualWidth   = 0;
        EGLint actualHeight  = 0;
        EGLint desiredWidth  = 0;
        EGLint desiredHeight = 0;
        if ((i % 2) == 0)
        {
            landscape     = true;
            desiredWidth  = 300;
            desiredHeight = 200;
        }
        else
        {
            landscape     = false;
            desiredWidth  = 200;
            desiredHeight = 300;
        }
        mOSWindow->resize(desiredWidth, desiredHeight);
        // setOrientation() uses a reverse-JNI call, which sends data to other parts of Android.
        // Sometime later (i.e. asynchronously), the window is updated.  Sleep a little here, and
        // then allow for multiple eglSwapBuffers calls to eventually see the new rotation.
        mOSWindow->setOrientation(desiredWidth, desiredHeight);
        angle::Sleep(1000);
        eglSwapBuffers(mDisplay, mWindowSurface);
        ASSERT_EGL_SUCCESS();

        while ((actualWidth != desiredWidth) && (actualHeight != desiredHeight))
        {
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
            EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
            if (landscape)
            {
                EXPECT_PIXEL_COLOR_EQ(mSize - 1, 0, GLColor::red);
            }
            else
            {
                EXPECT_PIXEL_COLOR_EQ(0, mSize - 1, GLColor::green);
            }
            ASSERT_GL_NO_ERROR();

            eglSwapBuffers(mDisplay, mWindowSurface);
            ASSERT_EGL_SUCCESS();

            eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &actualHeight);
            eglQuerySurface(mDisplay, mWindowSurface, EGL_WIDTH, &actualWidth);
        }
    }
}

// Android-specific test that changes a window's rotation and size. This is to check the actual size
// and the surface capabilities returned by vkGetPhysicalDeviceSurfaceCapabilitiesKHR.
TEST_P(EGLPreRotationSurfaceTest, CheckSurfaceCapabilities)
{
    // This test is confined to Android.
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && !IsAndroid() && IsLinux() && isSwiftshader());

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    EGLint preWindowSurfaceWidth  = 0;
    EGLint preWindowSurfaceHeight = 0;
    EGLint curWindowSurfaceWidth  = 300;
    EGLint curWindowSurfaceHeight = 200;
    EGLint actualWidth            = 0;
    EGLint actualHeight           = 0;

    // Set the initial window surface size.
    mOSWindow->resize(curWindowSurfaceWidth, curWindowSurfaceHeight);
    mOSWindow->setOrientation(curWindowSurfaceWidth, curWindowSurfaceHeight);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    eglQuerySurface(mDisplay, mWindowSurface, EGL_WIDTH, &actualWidth);
    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &actualHeight);
    ASSERT_EGL_SUCCESS();

    // eglSwapBuffers(vkQueuePresentKHR) is called before eglQuerySurface
    // so actualWidth and actualHeight need to be curWindowSurfaceHeight and curWindowSurfaceHeight
    // (300, 200).
    EXPECT_EQ(curWindowSurfaceWidth, actualWidth);
    EXPECT_EQ(curWindowSurfaceHeight, actualHeight);

    // Store the old values
    preWindowSurfaceWidth  = curWindowSurfaceWidth;
    preWindowSurfaceHeight = curWindowSurfaceHeight;

    // Set the new values
    curWindowSurfaceWidth  = 200;
    curWindowSurfaceHeight = 300;

    mOSWindow->resize(curWindowSurfaceWidth, curWindowSurfaceHeight);
    mOSWindow->setOrientation(curWindowSurfaceWidth, curWindowSurfaceHeight);

    eglQuerySurface(mDisplay, mWindowSurface, EGL_WIDTH, &actualWidth);
    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &actualHeight);
    ASSERT_EGL_SUCCESS();

    // eglSwapBuffers(vkQueuePresentKHR) is not called before eglQuerySurface
    // so actualWidth and actualHeight need to be preWindowSurfaceWidth and preWindowSurfaceHeight
    // (300, 200).
    EXPECT_EQ(preWindowSurfaceWidth, actualWidth);
    EXPECT_EQ(preWindowSurfaceHeight, actualHeight);

    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    eglQuerySurface(mDisplay, mWindowSurface, EGL_WIDTH, &actualWidth);
    eglQuerySurface(mDisplay, mWindowSurface, EGL_HEIGHT, &actualHeight);
    ASSERT_EGL_SUCCESS();

    // Now eglSwapBuffers(vkQueuePresentKHR) is called
    // so actualWidth and actualHeight will be curWindowSurfaceHeight and curWindowSurfaceHeight
    // (200, 300).
    EXPECT_EQ(curWindowSurfaceWidth, actualWidth);
    EXPECT_EQ(curWindowSurfaceHeight, actualHeight);
}

// A slight variation of EGLPreRotationSurfaceTest, where the initial window size is 400x300, yet
// the drawing is still 256x256.  In addition, gl_FragCoord is used in a "clever" way, as the color
// of the 256x256 drawing area, which reproduces an interesting pre-rotation case from the
// following dEQP tests:
//
// - dEQP.GLES31/functional_texture_multisample_samples_*_sample_position
//
// This will test the rotation of gl_FragCoord, as well as the viewport, scissor, and rendering
// area calculations, especially when the Android device is rotated.
class EGLPreRotationLargeSurfaceTest : public EGLPreRotationSurfaceTest
{
  protected:
    EGLPreRotationLargeSurfaceTest() : mSize(256) {}

    void testSetUp() override
    {
        mOSWindow = OSWindow::New();
        mOSWindow->initialize("EGLSurfaceTest", 400, 300);
    }

    int mSize;
};

// Provide a predictable pattern for testing pre-rotation
TEST_P(EGLPreRotationLargeSurfaceTest, OrientedWindowWithFragCoordDraw)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    constexpr char kVS[] =
        "attribute vec2 position;\n"
        "void main() {\n"
        "  gl_Position = vec4(position, 0, 1);\n"
        "}";

    constexpr char kFS[] =
        "void main() {\n"
        "  gl_FragColor = vec4(gl_FragCoord.x / 256.0, gl_FragCoord.y / 256.0, 0.0, 1.0);\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffer;

    glBindVertexArray(vertexArray);

    std::vector<GLushort> indices = {0, 1, 2, 2, 3, 0};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), &indices[0],
                 GL_STATIC_DRAW);

    std::vector<GLfloat> positionData = {// quad vertices
                                         -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                 GL_STATIC_DRAW);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(positionLocation);

    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);

    testDrawingAndReadPixels();
}

// Pre-rotation tests for glBlitFramebuffer.  A slight variation of EGLPreRotationLargeSurfaceTest,
// where the initial window size is still 400x300, and the drawing is still 256x256.  In addition,
// glBlitFramebuffer is tested in a variety of ways.  Separate tests are used to make debugging
// simpler, but they all share common setup.  These tests reproduce interesting pre-rotation cases
// from dEQP tests such as the following:
//
// - dEQP.GLES3/functional_fbo_blit_default_framebuffer_*
// - dEQP.GLES3/functional_fbo_invalidate_*
constexpr GLuint kCoordMidWayShort       = 127;
constexpr GLuint kCoordMidWayLong        = 128;
constexpr GLColor kColorMidWayShortShort = GLColor(127, 127, 0, 255);
constexpr GLColor kColorMidWayShortLong  = GLColor(127, 128, 0, 255);
constexpr GLColor kColorMidWayLongShort  = GLColor(128, 127, 0, 255);
constexpr GLColor kColorMidWayLongLong   = GLColor(128, 128, 0, 255);
// When scaling horizontally, the "black" and "green" colors have a 1 in the red component
constexpr GLColor kColorScaleHorizBlack = GLColor(1, 0, 0, 255);
constexpr GLColor kColorScaleHorizGreen = GLColor(1, 255, 0, 255);
// When scaling vertically, the "black" and "red" colors have a 1 in the green component
constexpr GLColor kColorScaleVertBlack = GLColor(0, 1, 0, 255);
constexpr GLColor kColorScaleVertRed   = GLColor(255, 1, 0, 255);

class EGLPreRotationBlitFramebufferTest : public EGLPreRotationLargeSurfaceTest
{
  protected:
    EGLPreRotationBlitFramebufferTest() {}

    GLuint createProgram()
    {
        constexpr char kVS[] =
            "attribute vec2 position;\n"
            "attribute vec2 redGreen;\n"
            "varying vec2 v_data;\n"
            "void main() {\n"
            "  gl_Position = vec4(position, 0, 1);\n"
            "  v_data = redGreen;\n"
            "}";

        constexpr char kFS[] =
            "varying highp vec2 v_data;\n"
            "void main() {\n"
            "  gl_FragColor = vec4(v_data, 0, 1);\n"
            "}";

        return CompileProgram(kVS, kFS);
    }

    void initializeGeometry(GLuint program,
                            GLBuffer *indexBuffer,
                            GLVertexArray *vertexArray,
                            GLBuffer *vertexBuffers)
    {
        GLint positionLocation = glGetAttribLocation(program, "position");
        ASSERT_NE(-1, positionLocation);

        GLint redGreenLocation = glGetAttribLocation(program, "redGreen");
        ASSERT_NE(-1, redGreenLocation);

        glBindVertexArray(*vertexArray);

        std::vector<GLushort> indices = {0, 1, 2, 2, 3, 0};
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), &indices[0],
                     GL_STATIC_DRAW);

        std::vector<GLfloat> positionData = {// quad vertices
                                             -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                     GL_STATIC_DRAW);
        glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2,
                              nullptr);
        glEnableVertexAttribArray(positionLocation);

        std::vector<GLfloat> redGreenData = {// green(0,1), black(0,0), red(1,0), yellow(1,1)
                                             0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f};

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * redGreenData.size(), &redGreenData[0],
                     GL_STATIC_DRAW);
        glVertexAttribPointer(redGreenLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2,
                              nullptr);
        glEnableVertexAttribArray(redGreenLocation);
    }

    void initializeFBO(GLFramebuffer *framebuffer, GLTexture *texture)
    {
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mSize, mSize, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     nullptr);

        glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
    }

    // Ensures that the correct colors are where they should be when the entire 256x256 pattern has
    // been rendered or blitted to a location relative to an x and y offset.
    void test256x256PredictablePattern(GLint xOffset, GLint yOffset)
    {
        EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + 0, GLColor::black);
        EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + mSize - 1, GLColor::green);
        EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + 0, GLColor::red);
        EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + mSize - 1, GLColor::yellow);
        EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort, yOffset + kCoordMidWayShort,
                              kColorMidWayShortShort);
        EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort, yOffset + kCoordMidWayLong,
                              kColorMidWayShortLong);
        EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayLong, yOffset + kCoordMidWayShort,
                              kColorMidWayLongShort);
        EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayLong, yOffset + kCoordMidWayLong,
                              kColorMidWayLongLong);
    }
};

// Draw a predictable pattern (for testing pre-rotation) into an FBO, and then use glBlitFramebuffer
// to blit that pattern into various places within the 400x300 window
TEST_P(EGLPreRotationBlitFramebufferTest, BasicBlitFramebuffer)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];
    initializeGeometry(program, &indexBuffer, &vertexArray, vertexBuffers);
    ASSERT_GL_NO_ERROR();

    // Create a texture-backed FBO and render the predictable pattern to it
    GLFramebuffer fbo;
    GLTexture texture;
    initializeFBO(&fbo, &texture);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    // Ensure the predictable pattern seems correct in the FBO
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    //
    // Test blitting the entire FBO image to a 256x256 part of the default framebuffer (no scaling)
    //

    // Blit from the FBO to the default framebuffer (i.e. the swapchain)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, 0, 0, mSize, mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();

    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, 0, 0, mSize, mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    // Clear to black and blit to a different part of the window
    glClear(GL_COLOR_BUFFER_BIT);
    GLint xOffset = 40;
    GLint yOffset = 30;
    glViewport(xOffset, yOffset, mSize, mSize);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();

    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    test256x256PredictablePattern(xOffset, yOffset);
    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_SUCCESS();
}

// Blit the ms0 stencil buffer to the default framebuffer with rotation on android.
TEST_P(EGLPreRotationBlitFramebufferTest, BlitStencilWithRotation)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888d24s8Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    mOSWindow->setOrientation(300, 400);
    angle::Sleep(1000);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    GLRenderbuffer colorbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuf.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_RGBA8, 64, 128);

    GLRenderbuffer depthstencilbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, depthstencilbuf.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_DEPTH24_STENCIL8, 64, 128);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              depthstencilbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthstencilbuf);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Replace stencil to 1.
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 255);
    drawQuad(drawRed.get(), essl3_shaders::PositionAttrib(), 0.8f);

    // Blit stencil buffer to default frambuffer.
    GLenum attachments1[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 64, 128, 0, 0, 64, 128, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    glDisable(GL_STENCIL_TEST);
    drawQuad(drawGreen.get(), essl3_shaders::PositionAttrib(), 0.5f);

    // Draw blue color if the stencil is equal to 1.
    // If the blit finished successfully, the stencil test should all pass.
    ANGLE_GL_PROGRAM(drawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 255);
    drawQuad(drawBlue.get(), essl3_shaders::PositionAttrib(), 0.3f);

    // Check the result, especially the boundaries.
    EXPECT_PIXEL_COLOR_EQ(0, 127, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(32, 127, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(32, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(63, 0, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(63, 64, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(32, 64, GLColor::blue);

    // Some pixels around x=0/63 (related to the pre-rotation degree) still fail on android.
    // From the image in the window, the failures near one of the image's edge look like "aliasing".
    // We need to fix blit with pre-rotation. http://anglebug.com/42263612
    // EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    // EXPECT_PIXEL_COLOR_EQ(0, 64, GLColor::blue);
    // EXPECT_PIXEL_COLOR_EQ(63, 1, GLColor::blue);
    // EXPECT_PIXEL_COLOR_EQ(63, 127, GLColor::blue);

    eglSwapBuffers(mDisplay, mWindowSurface);

    ASSERT_GL_NO_ERROR();
}

// Blit the multisample stencil buffer to the default framebuffer with rotation on android.
TEST_P(EGLPreRotationBlitFramebufferTest, BlitMultisampleStencilWithRotation)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888d24s8Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    mOSWindow->setOrientation(300, 400);
    angle::Sleep(1000);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    GLRenderbuffer colorbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuf.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, 128, 64);

    GLRenderbuffer depthstencilbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, depthstencilbuf.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, 128, 64);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              depthstencilbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthstencilbuf);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Replace stencil to 1.
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 255);
    drawQuad(drawRed.get(), essl3_shaders::PositionAttrib(), 0.8f);

    // Blit multisample stencil buffer to default frambuffer.
    GLenum attachments1[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, 128, 64, 0, 0, 128, 64, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
                      GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    glDisable(GL_STENCIL_TEST);
    drawQuad(drawGreen.get(), essl3_shaders::PositionAttrib(), 0.5f);

    // Draw blue color if the stencil is equal to 1.
    // If the blit finished successfully, the stencil test should all pass.
    ANGLE_GL_PROGRAM(drawBlue, essl3_shaders::vs::Simple(), essl3_shaders::fs::Blue());
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 255);
    drawQuad(drawBlue.get(), essl3_shaders::PositionAttrib(), 0.3f);

    // Check the result, especially the boundaries.
    EXPECT_PIXEL_COLOR_EQ(127, 32, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(64, 32, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(0, 63, GLColor::blue);
    EXPECT_PIXEL_COLOR_EQ(64, 63, GLColor::blue);

    // Some pixels around x=0/127 or y=0 (related to the pre-rotation degree)still fail on android.
    // We need to fix blit with pre-rotation. http://anglebug.com/42263612
    // Failures of Rotated90Degrees.
    // EXPECT_PIXEL_COLOR_EQ(127, 1, GLColor::blue);
    // EXPECT_PIXEL_COLOR_EQ(127, 63, GLColor::blue);
    // Failures of Rotated180Degrees.
    // EXPECT_PIXEL_COLOR_EQ(64, 0, GLColor::blue);
    // EXPECT_PIXEL_COLOR_EQ(127, 0, GLColor::blue);
    // Failures of Rotated270Degrees.
    // EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    // EXPECT_PIXEL_COLOR_EQ(0, 32, GLColor::blue);

    eglSwapBuffers(mDisplay, mWindowSurface);

    ASSERT_GL_NO_ERROR();
}

// Blit stencil to default framebuffer with flip and prerotation.
TEST_P(EGLPreRotationBlitFramebufferTest, BlitStencilWithFlip)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // We need to fix blit with pre-rotation. http://anglebug.com/42263612
    ANGLE_SKIP_TEST_IF(IsPixel4() || IsPixel4XL() || IsWindows());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888d24s8Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    mOSWindow->setOrientation(300, 400);
    angle::Sleep(1000);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    constexpr int kSize = 128;
    glViewport(0, 0, kSize, kSize);

    GLRenderbuffer colorbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuf.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_RGBA8, kSize, kSize);

    GLRenderbuffer depthstencilbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, depthstencilbuf.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_DEPTH24_STENCIL8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                              depthstencilbuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              depthstencilbuf);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Replace stencil to 1.
    ANGLE_GL_PROGRAM(drawRed, essl3_shaders::vs::Simple(), essl3_shaders::fs::Red());
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 255);
    drawQuad(drawRed.get(), essl3_shaders::PositionAttrib(), 0.8f);

    // Blit stencil buffer to default frambuffer with X-flip.
    GLenum attachments1[] = {GL_COLOR_ATTACHMENT0};
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, attachments1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, kSize, kSize, kSize, 0, 0, kSize,
                      GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());
    glDisable(GL_STENCIL_TEST);
    drawQuad(drawGreen.get(), essl3_shaders::PositionAttrib(), 0.5f);

    // Draw blue color if the stencil is equal to 1.
    // If the blit finished successfully, the stencil test should all pass.
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 255);
    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    // Check the result, especially the boundaries.
    EXPECT_PIXEL_NEAR(0, 0, 0, 0, 0, 255, 1.0);                      // Black
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 253, 0, 0, 255, 1.0);            // Red
    EXPECT_PIXEL_NEAR(0, kSize - 1, 0, 253, 0, 255, 1.0);            // Green
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 253, 253, 0, 255, 1.0);  // Yellow

    eglSwapBuffers(mDisplay, mWindowSurface);

    ASSERT_GL_NO_ERROR();
}

// Blit color buffer to default framebuffer with Y-flip/X-flip.
TEST_P(EGLPreRotationBlitFramebufferTest, BlitColorToDefault)
{
    // This test uses functionality that is only available on Android
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && !IsAndroid());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    constexpr int kSize = 128;
    glViewport(0, 0, kSize, kSize);

    GLRenderbuffer colorbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuf.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_RGBA8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuf);

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());

    EGLint desiredWidth  = 300;
    EGLint desiredHeight = 400;
    mOSWindow->resize(desiredWidth, desiredHeight);
    mOSWindow->setOrientation(desiredWidth, desiredHeight);
    angle::Sleep(1000);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.get());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    // Blit color buffer to default frambuffer without flip.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // Check the result, especially the boundaries.
    EXPECT_PIXEL_NEAR(0, 0, 0, 0, 0, 255, 1.0);                      // Balck
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 253, 0, 0, 255, 1.0);            // Red
    EXPECT_PIXEL_NEAR(0, kSize - 1, 0, 253, 0, 255, 1.0);            // Green
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 253, 253, 0, 255, 1.0);  // Yellow

    // Blit color buffer to default frambuffer with Y-flip.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.get());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, kSize, kSize, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    EXPECT_PIXEL_NEAR(0, 0, 0, 253, 0, 255, 1.0);                  // Green
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 253, 253, 0, 255, 1.0);        // Yellow
    EXPECT_PIXEL_NEAR(0, kSize - 1, 0, 0, 0, 255, 1.0);            // Balck
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 253, 0, 0, 255, 1.0);  // Red

    // Blit color buffer to default frambuffer with X-flip.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.get());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, kSize, kSize, kSize, 0, 0, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    EXPECT_PIXEL_NEAR(0, 0, 253, 0, 0, 255, 1.0);                  // Red
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 0, 0, 0, 255, 1.0);            // Balck
    EXPECT_PIXEL_NEAR(0, kSize - 1, 253, 253, 0, 255, 1.0);        // Yellow
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 0, 253, 0, 255, 1.0);  // Green

    ASSERT_GL_NO_ERROR();
}

// Blit color buffer from default framebuffer with Y-flip/X-flip.
TEST_P(EGLPreRotationBlitFramebufferTest, BlitColorFromDefault)
{
    // This test uses functionality that is only available on Android
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && !IsAndroid());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    constexpr int kSize = 128;
    glViewport(0, 0, kSize, kSize);

    GLRenderbuffer colorbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuf.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_RGBA8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuf);
    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());

    EGLint desiredWidth  = 300;
    EGLint desiredHeight = 400;
    mOSWindow->resize(desiredWidth, desiredHeight);
    mOSWindow->setOrientation(desiredWidth, desiredHeight);
    angle::Sleep(1000);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    // Blit color buffer from default frambuffer without flip.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.get());
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.get());

    // Check the result, especially the boundaries.
    EXPECT_PIXEL_NEAR(0, 0, 0, 0, 0, 255, 1.0);                      // Balck
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 253, 0, 0, 255, 1.0);            // Red
    EXPECT_PIXEL_NEAR(0, kSize - 1, 0, 253, 0, 255, 1.0);            // Green
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 253, 253, 0, 255, 1.0);  // Yellow

    // Blit color buffer from default frambuffer with Y-flip.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.get());
    glBlitFramebuffer(0, 0, kSize, kSize, 0, kSize, kSize, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.get());

    EXPECT_PIXEL_NEAR(0, 0, 0, 253, 0, 255, 1.0);                  // Green
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 253, 253, 0, 255, 1.0);        // Yellow
    EXPECT_PIXEL_NEAR(0, kSize - 1, 0, 0, 0, 255, 1.0);            // Balck
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 253, 0, 0, 255, 1.0);  // Red

    // Blit color buffer from default frambuffer with X-flip.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer.get());
    glBlitFramebuffer(0, 0, kSize, kSize, kSize, 0, 0, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.get());

    EXPECT_PIXEL_NEAR(0, 0, 253, 0, 0, 255, 1.0);                  // Red
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 0, 0, 0, 255, 1.0);            // Balck
    EXPECT_PIXEL_NEAR(0, kSize - 1, 253, 253, 0, 255, 1.0);        // Yellow
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 0, 253, 0, 255, 1.0);  // Green

    ASSERT_GL_NO_ERROR();
}

// Blit multisample color buffer to resolved framebuffer.
TEST_P(EGLPreRotationBlitFramebufferTest, BlitMultisampleColorToResolved)
{
    // This test uses functionality that is only available on Android
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && !IsAndroid());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    constexpr int kSize = 128;
    glViewport(0, 0, kSize, kSize);

    GLRenderbuffer colorMS;
    glBindRenderbuffer(GL_RENDERBUFFER, colorMS.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, kSize, kSize);

    GLRenderbuffer colorResolved;
    glBindRenderbuffer(GL_RENDERBUFFER, colorResolved.get());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kSize, kSize);

    GLFramebuffer framebufferMS;
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferMS.get());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorMS);

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    GLFramebuffer framebufferResolved;
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferResolved.get());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              colorResolved.get());

    EGLint desiredWidth  = 300;
    EGLint desiredHeight = 400;
    mOSWindow->resize(desiredWidth, desiredHeight);
    mOSWindow->setOrientation(desiredWidth, desiredHeight);
    angle::Sleep(1000);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferMS.get());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferMS.get());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferResolved.get());
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferResolved.get());

    // Check the result, especially the boundaries.
    EXPECT_PIXEL_NEAR(0, 0, 0, 0, 0, 255, 1.0);                      // Balck
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 253, 0, 0, 255, 1.0);            // Red
    EXPECT_PIXEL_NEAR(0, kSize - 1, 0, 253, 0, 255, 1.0);            // Green
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 253, 253, 0, 255, 1.0);  // Yellow

    ASSERT_GL_NO_ERROR();
}

// Blit color buffer to default framebuffer with linear filter.
TEST_P(EGLPreRotationBlitFramebufferTest, BlitColorWithLinearFilter)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    mOSWindow->setOrientation(300, 400);
    angle::Sleep(1000);
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();

    constexpr int kSize = 128;
    glViewport(0, 0, kSize, kSize);

    GLRenderbuffer colorbuf;
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuf.get());
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 0, GL_RGBA8, kSize, kSize);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.get());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuf);

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // Check the result, especially the boundaries.
    EXPECT_PIXEL_NEAR(0, 0, 0, 0, 0, 255, 1.0);                      // Black
    EXPECT_PIXEL_NEAR(kSize - 1, 0, 253, 0, 0, 255, 1.0);            // Red
    EXPECT_PIXEL_NEAR(0, kSize - 1, 0, 253, 0, 255, 1.0);            // Green
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 253, 253, 0, 255, 1.0);  // Yellow

    eglSwapBuffers(mDisplay, mWindowSurface);

    ASSERT_GL_NO_ERROR();
}

// Draw a predictable pattern (for testing pre-rotation) into an FBO, and then use glBlitFramebuffer
// to blit the left and right halves of that pattern into various places within the 400x300 window
TEST_P(EGLPreRotationBlitFramebufferTest, LeftAndRightBlitFramebuffer)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];
    initializeGeometry(program, &indexBuffer, &vertexArray, vertexBuffers);
    ASSERT_GL_NO_ERROR();

    // Create a texture-backed FBO and render the predictable pattern to it
    GLFramebuffer fbo;
    GLTexture texture;
    initializeFBO(&fbo, &texture);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    // Ensure the predictable pattern seems correct in the FBO
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    // Prepare to blit to the default framebuffer and read from the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Blit to an offset part of the 400x300 window
    GLint xOffset = 40;
    GLint yOffset = 30;

    //
    // Test blitting half of the FBO image to a 128x256 or 256x128 part of the default framebuffer
    // (no scaling)
    //

    // 1st) Clear to black and blit the left and right halves of the texture to the left and right
    // halves of that different part of the window
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(xOffset, yOffset, mSize, mSize);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, mSize / 2, mSize, xOffset, yOffset, xOffset + (mSize / 2),
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(mSize / 2, 0, mSize, mSize, xOffset + (mSize / 2), yOffset, xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();

    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize / 2, mSize, xOffset, yOffset, xOffset + (mSize / 2),
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(mSize / 2, 0, mSize, mSize, xOffset + (mSize / 2), yOffset, xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    test256x256PredictablePattern(xOffset, yOffset);
    ASSERT_GL_NO_ERROR();

    // 2nd) Clear to black and this time blit the left half of the source texture to the right half
    // of the destination window, and then blit the right half of the source texture to the left
    // half of the destination window
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(xOffset, yOffset, mSize, mSize);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBlitFramebuffer(mSize / 2, 0, mSize, mSize, xOffset, yOffset, xOffset + (mSize / 2),
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, mSize / 2, mSize, xOffset + (mSize / 2), yOffset, xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();

    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(mSize / 2, 0, mSize, mSize, xOffset, yOffset, xOffset + (mSize / 2),
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, mSize / 2, mSize, xOffset + (mSize / 2), yOffset, xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort + 1, yOffset + 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort + 1, yOffset + mSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort, yOffset + 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort, yOffset + mSize - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + kCoordMidWayShort, kColorMidWayShortShort);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + kCoordMidWayLong, kColorMidWayShortLong);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + kCoordMidWayShort, kColorMidWayLongShort);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + kCoordMidWayLong, kColorMidWayLongLong);
    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_SUCCESS();
}

// Draw a predictable pattern (for testing pre-rotation) into an FBO, and then use glBlitFramebuffer
// to blit the top and bottom halves of that pattern into various places within the 400x300 window
TEST_P(EGLPreRotationBlitFramebufferTest, TopAndBottomBlitFramebuffer)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];
    initializeGeometry(program, &indexBuffer, &vertexArray, vertexBuffers);
    ASSERT_GL_NO_ERROR();

    // Create a texture-backed FBO and render the predictable pattern to it
    GLFramebuffer fbo;
    GLTexture texture;
    initializeFBO(&fbo, &texture);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    // Ensure the predictable pattern seems correct in the FBO
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    // Prepare to blit to the default framebuffer and read from the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Blit to an offset part of the 400x300 window
    GLint xOffset = 40;
    GLint yOffset = 30;

    //
    // Test blitting half of the FBO image to a 128x256 or 256x128 part of the default framebuffer
    // (no scaling)
    //

    // 1st) Clear to black and blit the top and bottom halves of the texture to the top and bottom
    // halves of that different part of the window
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(xOffset, yOffset, mSize, mSize);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, mSize, mSize / 2, xOffset, yOffset, xOffset + mSize,
                      yOffset + (mSize / 2), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, mSize / 2, mSize, mSize, xOffset, yOffset + (mSize / 2), xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();

    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize / 2, xOffset, yOffset, xOffset + mSize,
                      yOffset + (mSize / 2), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, mSize / 2, mSize, mSize, xOffset, yOffset + (mSize / 2), xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    test256x256PredictablePattern(xOffset, yOffset);
    ASSERT_GL_NO_ERROR();

    // 2nd) Clear to black and this time blit the top half of the source texture to the bottom half
    // of the destination window, and then blit the bottom half of the source texture to the top
    // half of the destination window
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(xOffset, yOffset, mSize, mSize);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, mSize, mSize / 2, xOffset, yOffset + (mSize / 2), xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, mSize / 2, mSize, mSize, xOffset, yOffset, xOffset + mSize,
                      yOffset + (mSize / 2), GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();

    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize / 2, xOffset, yOffset + (mSize / 2), xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, mSize / 2, mSize, mSize, xOffset, yOffset, xOffset + mSize,
                      yOffset + (mSize / 2), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + kCoordMidWayShort + 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + kCoordMidWayShort, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + kCoordMidWayShort + 1, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + kCoordMidWayShort, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort, yOffset + mSize - 1, kColorMidWayShortShort);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort, yOffset + 0, kColorMidWayShortLong);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayLong, yOffset + mSize - 1, kColorMidWayLongShort);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayLong, yOffset + 0, kColorMidWayLongLong);
    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_SUCCESS();
}

// Draw a predictable pattern (for testing pre-rotation) into an FBO, and then use glBlitFramebuffer
// to blit that pattern into various places within the 400x300 window, but being scaled to one-half
// size
TEST_P(EGLPreRotationBlitFramebufferTest, ScaledBlitFramebuffer)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];
    initializeGeometry(program, &indexBuffer, &vertexArray, vertexBuffers);
    ASSERT_GL_NO_ERROR();

    // Create a texture-backed FBO and render the predictable pattern to it
    GLFramebuffer fbo;
    GLTexture texture;
    initializeFBO(&fbo, &texture);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    // Ensure the predictable pattern seems correct in the FBO
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    // Prepare to blit to the default framebuffer and read from the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Blit to an offset part of the 400x300 window
    GLint xOffset = 40;
    GLint yOffset = 30;

    //
    // Test blitting the entire FBO image to a 128x256 or 256x128 part of the default framebuffer
    // (requires scaling)
    //

    // 1st) Clear to black and blit the FBO to the left and right halves of that different part of
    // the window
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(xOffset, yOffset, mSize, mSize);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + (mSize / 2), yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset + (mSize / 2), yOffset, xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();

    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + (mSize / 2), yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset + (mSize / 2), yOffset, xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + 0, kColorScaleHorizBlack);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + mSize - 1, kColorScaleHorizGreen);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort, yOffset + 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayShort, yOffset + mSize - 1, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayLong, yOffset + 0, kColorScaleHorizBlack);
    EXPECT_PIXEL_COLOR_EQ(xOffset + kCoordMidWayLong, yOffset + mSize - 1, kColorScaleHorizGreen);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + 0, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + mSize - 1, GLColor::yellow);

    // 2nd) Clear to black and blit the FBO to the top and bottom halves of that different part of
    // the window
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(xOffset, yOffset, mSize, mSize);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + (mSize / 2),
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset + (mSize / 2), xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();

    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + (mSize / 2),
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset + (mSize / 2), xOffset + mSize,
                      yOffset + mSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + 0, kColorScaleVertBlack);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + 0, kColorScaleVertRed);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + kCoordMidWayShort, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + kCoordMidWayShort, GLColor::yellow);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + kCoordMidWayLong, kColorScaleVertBlack);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + kCoordMidWayLong, kColorScaleVertRed);
    EXPECT_PIXEL_COLOR_EQ(xOffset + 0, yOffset + mSize - 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(xOffset + mSize - 1, yOffset + mSize - 1, GLColor::yellow);
    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_SUCCESS();
}

// Draw a predictable pattern (for testing pre-rotation) into a 256x256 portion of the 400x300
// window, and then use glBlitFramebuffer to blit that pattern into an FBO
TEST_P(EGLPreRotationBlitFramebufferTest, FboDestBlitFramebuffer)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];
    initializeGeometry(program, &indexBuffer, &vertexArray, vertexBuffers);
    ASSERT_GL_NO_ERROR();

    // Create a texture-backed FBO and render the predictable pattern to it
    GLFramebuffer fbo;
    GLTexture texture;
    initializeFBO(&fbo, &texture);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    // Ensure the predictable pattern seems correct in the FBO
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    // Prepare to blit to the default framebuffer and read from the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Blit to an offset part of the 400x300 window
    GLint xOffset = 40;
    GLint yOffset = 30;

    //
    // Test blitting a 256x256 part of the default framebuffer to the entire FBO (no scaling)
    //

    // To get the entire predictable pattern into the default framebuffer at the desired offset,
    // blit it from the FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(xOffset, yOffset, mSize, mSize);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();
    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Clear the FBO to black and blit from the window to the FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glViewport(0, 0, mSize, mSize);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(xOffset, yOffset, xOffset + mSize, yOffset + mSize, 0, 0, mSize, mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Ensure the predictable pattern seems correct in the FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_SUCCESS();
}

// Draw a predictable pattern (for testing pre-rotation) into a 256x256 portion of the 400x300
// window, and then use glBlitFramebuffer to blit that pattern into an FBO, but with coordinates
// that are partially out-of-bounds of the source
TEST_P(EGLPreRotationBlitFramebufferTest, FboDestOutOfBoundsSourceBlitFramebuffer)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];
    initializeGeometry(program, &indexBuffer, &vertexArray, vertexBuffers);
    ASSERT_GL_NO_ERROR();

    // Create a texture-backed FBO and render the predictable pattern to it
    GLFramebuffer fbo;
    GLTexture texture;
    initializeFBO(&fbo, &texture);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    // Ensure the predictable pattern seems correct in the FBO
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    // Prepare to blit to the default framebuffer and read from the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Blit to the origin of the 400x300 window
    GLint xOffset = 0;
    GLint yOffset = 0;

    //
    // Test blitting a 256x256 part of the default framebuffer to the entire FBO (no scaling)
    //

    // To get the entire predictable pattern into the default framebuffer at the desired offset,
    // blit it from the FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(xOffset, yOffset, mSize, mSize);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();
    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Clear the FBO to black and blit from the window to the FBO, but give source coordinates that
    // are partially outside of the window
    xOffset = -10;
    yOffset = -15;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glViewport(0, 0, mSize, mSize);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(xOffset, yOffset, xOffset + mSize, yOffset + mSize, 0, 0, mSize, mSize,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Ensure the predictable pattern seems correct in the FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    // NOTE: There is a strip of black on the left and bottom edges of the PBO, corresponding to
    // the source coordinates that were outside of the source.  The strip of black is xOffset
    // pixels wide on the left side, and yOffset pixels tall on the bottom side.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(0, 255, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(-xOffset - 1, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(-xOffset - 1, 255, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(0, -yOffset - 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(255, -yOffset - 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(255 + xOffset, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(255 + xOffset, -yOffset - 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(0, 255 + yOffset, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(-xOffset - 1, 255 + yOffset, GLColor::black);

    // FBO coordinate (-xOffset, -yOffset) (or (10, 15)) has the values from the bottom-left corner
    // of the source (which happens to be black).  Thus, the following two tests are equivalent:
    EXPECT_PIXEL_COLOR_EQ(-xOffset, -yOffset, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(10, 15, GLColor::black);

    // Note: the following is equivalent to (0, 0):
    EXPECT_PIXEL_COLOR_EQ(10 + xOffset, 15 + yOffset, GLColor::black);

    EXPECT_PIXEL_COLOR_EQ(-xOffset + 1, -yOffset + 1, GLColor(1, 1, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(-xOffset + 10, -yOffset + 10, GLColor(10, 10, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(-xOffset + 20, -yOffset + 20, GLColor(20, 20, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(-xOffset + 100, -yOffset + 100, GLColor(100, 100, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(-xOffset + 200, -yOffset + 200, GLColor(200, 200, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(-xOffset + 230, -yOffset + 230, GLColor(230, 230, 0, 255));
    // Note how the offset works differently when added to the same coordinate value as above.  The
    // black strip causes the value to be 2X less the offset in each direction.  Thus, coordinate
    // (230+xOffset, 230+yOffset) yields actual coordinate (220, 215) and red-green values
    // (230+(2*xOffset), 230+(2*yOffset)) or (210, 200).  The following two tests are equivalent:
    EXPECT_PIXEL_COLOR_EQ(230 + xOffset, 230 + yOffset,
                          GLColor(230 + (2 * xOffset), 230 + (2 * yOffset), 0, 255));
    EXPECT_PIXEL_COLOR_EQ(220, 215, GLColor(210, 200, 0, 255));
    // FBO coordinate (245, 240) has the highest pixel values from the source.  The value of the
    // FBO pixel at (245, 240) is smaller than the same coordinate in the source because of the
    // blit's offsets.  That is, the value is (245-xOffset, 240-yOffset) or (235, 225).  Thus, the
    // following two tests are the same:
    EXPECT_PIXEL_COLOR_EQ(255 + xOffset, 255 + yOffset,
                          GLColor(255 + (2 * xOffset), 255 + (2 * yOffset), 0, 255));
    EXPECT_PIXEL_COLOR_EQ(245, 240, GLColor(235, 225, 0, 255));

    // Again, the "mid-way" coordinates will get values that aren't truly mid-way:
    EXPECT_PIXEL_COLOR_EQ(
        xOffset + kCoordMidWayShort, yOffset + kCoordMidWayShort,
        GLColor(kCoordMidWayShort + (2 * xOffset), kCoordMidWayShort + (2 * yOffset), 0, 255));
    EXPECT_PIXEL_COLOR_EQ(
        xOffset + kCoordMidWayShort, yOffset + kCoordMidWayLong,
        GLColor(kCoordMidWayShort + (2 * xOffset), kCoordMidWayLong + (2 * yOffset), 0, 255));
    EXPECT_PIXEL_COLOR_EQ(
        xOffset + kCoordMidWayLong, yOffset + kCoordMidWayShort,
        GLColor(kCoordMidWayLong + (2 * xOffset), kCoordMidWayShort + (2 * yOffset), 0, 255));
    EXPECT_PIXEL_COLOR_EQ(
        xOffset + kCoordMidWayLong, yOffset + kCoordMidWayLong,
        GLColor(kCoordMidWayLong + (2 * xOffset), kCoordMidWayLong + (2 * yOffset), 0, 255));

    // Almost Red
    EXPECT_PIXEL_COLOR_EQ(255, -yOffset, GLColor(255 + xOffset, 0, 0, 255));
    // Almost Green
    EXPECT_PIXEL_COLOR_EQ(-xOffset, 255, GLColor(0, 255 + yOffset, 0, 255));
    // Almost Yellow
    EXPECT_PIXEL_COLOR_EQ(255, 255, GLColor(255 + xOffset, 255 + yOffset, 0, 255));

    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_SUCCESS();
}

// Draw a predictable pattern (for testing pre-rotation) into a 256x256 portion of the 400x300
// window, and then use glBlitFramebuffer to blit that pattern into an FBO, but with coordinates
// that are partially out-of-bounds of the source, and cause a "stretch" to occur
TEST_P(EGLPreRotationBlitFramebufferTest, FboDestOutOfBoundsSourceWithStretchBlitFramebuffer)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];
    initializeGeometry(program, &indexBuffer, &vertexArray, vertexBuffers);
    ASSERT_GL_NO_ERROR();

    // Create a texture-backed FBO and render the predictable pattern to it
    GLFramebuffer fbo;
    GLTexture texture;
    initializeFBO(&fbo, &texture);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    // Ensure the predictable pattern seems correct in the FBO
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    // Prepare to blit to the default framebuffer and read from the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Blit to the origin of the 400x300 window
    GLint xOffset = 0;
    GLint yOffset = 0;

    //
    // Test blitting a 256x256 part of the default framebuffer to the entire FBO (no scaling)
    //

    // To get the entire predictable pattern into the default framebuffer at the desired offset,
    // blit it from the FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(xOffset, yOffset, mSize, mSize);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();
    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Clear the FBO to black and blit from the window to the FBO, but give source coordinates that
    // are partially outside of the window, but "stretch" the result by 0.5 (i.e. 2X shrink in x)
    xOffset = -10;
    yOffset = -15;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glViewport(0, 0, mSize, mSize);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(xOffset, yOffset, xOffset + mSize, yOffset + mSize, 0, 0, mSize / 2, mSize,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Ensure the predictable pattern seems correct in the FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    // NOTE: There is a strip of black on the left and bottom edges of the PBO, corresponding to
    // the source coordinates that were outside of the source.  The strip of black is xOffset/2
    // pixels wide on the left side, and yOffset pixels tall on the bottom side.
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, 255, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR((-xOffset / 2) - 1, 0, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR((-xOffset / 2) - 1, 255, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, -yOffset - 1, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR(255 / 2, -yOffset - 1, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR((255 + xOffset) / 2, 0, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR((255 + xOffset) / 2, -yOffset - 1, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR(0, 255 + yOffset, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR((-xOffset / 2) - 1, 255 + yOffset, GLColor::black, 1);

    // FBO coordinate (-xOffset, -yOffset) (or (10, 15)) has the values from the bottom-left corner
    // of the source (which happens to be black).  Thus, the following two tests are equivalent:
    EXPECT_PIXEL_COLOR_NEAR(-xOffset / 2, -yOffset, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR(10 + xOffset, 15 + yOffset, GLColor::black, 1);
    EXPECT_PIXEL_COLOR_NEAR(220 / 2, 215, GLColor(210, 200, 0, 255), 1);

    EXPECT_PIXEL_COLOR_NEAR((254 + xOffset) / 2, 255 + yOffset,
                            GLColor(254 + (2 * xOffset), 255 + (2 * yOffset), 0, 255), 1);
    EXPECT_PIXEL_COLOR_NEAR(254 / 2, 240, GLColor(244, 225, 0, 255), 1);

    // Almost Red
    EXPECT_PIXEL_COLOR_NEAR(254 / 2, -yOffset, GLColor(254 + xOffset, 0, 0, 255), 1);
    // Almost Green
    EXPECT_PIXEL_COLOR_NEAR(-xOffset / 2, 255, GLColor(0, 255 + yOffset, 0, 255), 1);
    // Almost Yellow
    EXPECT_PIXEL_COLOR_NEAR(254 / 2, 255, GLColor(254 + xOffset, 255 + yOffset, 0, 255), 1);

    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_SUCCESS();
}

// Draw a predictable pattern (for testing pre-rotation) into a 256x256 portion of the 400x300
// window, and then use glBlitFramebuffer to blit that pattern into an FBO, but with source and FBO
// coordinates that are partially out-of-bounds of the source
TEST_P(EGLPreRotationBlitFramebufferTest, FboDestOutOfBoundsSourceAndDestBlitFramebuffer)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers[2];
    initializeGeometry(program, &indexBuffer, &vertexArray, vertexBuffers);
    ASSERT_GL_NO_ERROR();

    // Create a texture-backed FBO and render the predictable pattern to it
    GLFramebuffer fbo;
    GLTexture texture;
    initializeFBO(&fbo, &texture);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    ASSERT_GL_NO_ERROR();

    // Ensure the predictable pattern seems correct in the FBO
    test256x256PredictablePattern(0, 0);
    ASSERT_GL_NO_ERROR();

    // Prepare to blit to the default framebuffer and read from the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Blit to the origin of the 400x300 window
    GLint xOffset = 0;
    GLint yOffset = 0;

    //
    // Test blitting a 256x256 part of the default framebuffer to the entire FBO (no scaling)
    //

    // To get the entire predictable pattern into the default framebuffer at the desired offset,
    // blit it from the FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(xOffset, yOffset, mSize, mSize);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // Swap buffers to put the image in the window (so the test can be visually checked)
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_GL_NO_ERROR();
    // Blit again to check the colors in the back buffer
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(0, 0, mSize, mSize, xOffset, yOffset, xOffset + mSize, yOffset + mSize,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Clear the FBO to black and blit from the window to the FBO, but give source coordinates that
    // are partially outside of the window, and give destination coordinates that are partially
    // outside of the FBO
    xOffset = -10;
    yOffset = -15;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glViewport(0, 0, mSize, mSize);
    glClear(GL_COLOR_BUFFER_BIT);
    glBlitFramebuffer(xOffset, yOffset, (2 * xOffset) + mSize, (2 * yOffset) + mSize, -xOffset,
                      -yOffset, mSize, mSize, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Ensure the predictable pattern seems correct in the FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    // NOTE: There is a strip of black on the left and bottom edges of the PBO, corresponding to
    // the source coordinates that were outside of the source.  The strip of black is xOffset*2
    // pixels wide on the left side, and yOffset*2 pixels tall on the bottom side.
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(0, 255, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2) - 1, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2) - 1, 255, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(0, (-yOffset * 2) - 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(255, (-yOffset * 2) - 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(255 + xOffset, 0, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(255 + xOffset, (-yOffset * 2) - 1, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(0, 255 + yOffset, GLColor::black);
    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2) - 1, 255 + yOffset, GLColor::black);

    // FBO coordinate (-xOffset*2, -yOffset*2) (or (20, 30)) has the values from the bottom-left
    // corner of the source (which happens to be black).  Thus, the following two tests are
    // equivalent:
    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2), (-yOffset * 2), GLColor::black);
    EXPECT_PIXEL_COLOR_EQ(20, 30, GLColor::black);

    // Note: the following is equivalent to (0, 0):
    EXPECT_PIXEL_COLOR_EQ(20 + (xOffset * 2), 30 + (yOffset * 2), GLColor::black);

    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2) + 1, (-yOffset * 2) + 1, GLColor(1, 1, 0, 255));
    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2) + 10, (-yOffset * 2) + 10, GLColor(10, 10, 0, 255));
    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2) + 20, (-yOffset * 2) + 20, GLColor(20, 20, 0, 255));
    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2) + 100, (-yOffset * 2) + 100, GLColor(100, 100, 0, 255));
    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2) + 200, (-yOffset * 2) + 200, GLColor(200, 200, 0, 255));
    EXPECT_PIXEL_COLOR_EQ((-xOffset * 2) + 230, (-yOffset * 2) + 225, GLColor(230, 225, 0, 255));

    // Almost Red
    EXPECT_PIXEL_COLOR_EQ(255, -yOffset * 2, GLColor(255 + (xOffset * 2), 0, 0, 255));
    // Almost Green
    EXPECT_PIXEL_COLOR_EQ(-xOffset * 2, 255, GLColor(0, 255 + (yOffset * 2), 0, 255));
    // Almost Yellow
    EXPECT_PIXEL_COLOR_EQ(255, 255, GLColor(255 + (xOffset * 2), 255 + (yOffset * 2), 0, 255));

    ASSERT_GL_NO_ERROR();

    ASSERT_EGL_SUCCESS();
}

class EGLPreRotationInterpolateAtOffsetTest : public EGLPreRotationSurfaceTest
{
  protected:
    EGLPreRotationInterpolateAtOffsetTest() {}

    GLuint createProgram()
    {
        // Init program
        constexpr char kVS[] =
            "#version 310 es\n"
            "#extension GL_OES_shader_multisample_interpolation : require\n"
            "in highp vec2 position;\n"
            "uniform float screen_width;\n"
            "uniform float screen_height;\n"
            "out highp vec2 v_screenPosition;\n"
            "out highp vec2 v_offset;\n"
            "void main (void)\n"
            "{\n"
            "   gl_Position = vec4(position, 0, 1);\n"
            "   v_screenPosition = (position.xy + vec2(1.0, 1.0)) / 2.0 * vec2(screen_width, "
            "screen_height);\n"
            "   v_offset = position.xy * 0.5f;\n"
            "}";

        constexpr char kFS[] =
            "#version 310 es\n"
            "#extension GL_OES_shader_multisample_interpolation : require\n"
            "in highp vec2 v_screenPosition;\n"
            "in highp vec2 v_offset;\n"
            "layout(location = 0) out mediump vec4 FragColor;\n"
            "void main() {\n"
            "   const highp float threshold = 0.15625; // 4 subpixel bits. Assume 3 accurate bits "
            "+ 0.03125 for other errors\n"
            "\n"
            "   highp vec2 pixelCenter = floor(v_screenPosition) + vec2(0.5, 0.5);\n"
            "   highp vec2 offsetValue = interpolateAtOffset(v_screenPosition, v_offset);\n"
            "   highp vec2 refValue = pixelCenter + v_offset;\n"
            "\n"
            "   bool valuesEqual = all(lessThan(abs(offsetValue - refValue), vec2(threshold)));\n"
            "   if (valuesEqual)\n"
            "       FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
            "   else\n"
            "       FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "}";

        return CompileProgram(kVS, kFS);
    }
    void initializeGeometry(GLuint program,
                            GLBuffer *indexBuffer,
                            GLVertexArray *vertexArray,
                            GLBuffer *vertexBuffers)
    {
        GLint positionLocation = glGetAttribLocation(program, "position");
        ASSERT_NE(-1, positionLocation);

        GLuint screenWidthId  = glGetUniformLocation(program, "screen_width");
        GLuint screenHeightId = glGetUniformLocation(program, "screen_height");

        glUniform1f(screenWidthId, (GLfloat)mSize);
        glUniform1f(screenHeightId, (GLfloat)mSize);

        glBindVertexArray(*vertexArray);

        std::vector<GLushort> indices = {0, 1, 2, 2, 3, 0};
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), &indices[0],
                     GL_STATIC_DRAW);

        std::vector<GLfloat> positionData = {// quad vertices
                                             -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f};

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positionData.size(), &positionData[0],
                     GL_STATIC_DRAW);
        glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2,
                              nullptr);
        glEnableVertexAttribArray(positionLocation);
    }
};

// Draw with interpolateAtOffset() builtin function to pre-rotated default FBO
TEST_P(EGLPreRotationInterpolateAtOffsetTest, InterpolateAtOffsetWithDefaultFBO)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers;
    initializeGeometry(program, &indexBuffer, &vertexArray, &vertexBuffers);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(0, 255, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(mSize - 1, 0, GLColor(0, 255, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(0, mSize - 1, GLColor(0, 255, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(mSize - 1, mSize - 1, GLColor(0, 255, 0, 255));
    ASSERT_GL_NO_ERROR();

    // Make the image visible
    eglSwapBuffers(mDisplay, mWindowSurface);
    ASSERT_EGL_SUCCESS();
}

// Draw with interpolateAtOffset() builtin function to pre-rotated custom FBO
TEST_P(EGLPreRotationInterpolateAtOffsetTest, InterpolateAtOffsetWithCustomFBO)
{
    // http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(isVulkanRenderer() && IsLinux() && IsIntel());

    // Flaky on Linux SwANGLE http://anglebug.com/42263074
    ANGLE_SKIP_TEST_IF(IsLinux() && isSwiftshader());

    // To aid in debugging, we want this window visible
    setWindowVisible(mOSWindow, true);

    initializeDisplay();
    initializeSurfaceWithRGBA8888Config();

    eglMakeCurrent(mDisplay, mWindowSurface, mWindowSurface, mContext);
    ASSERT_EGL_SUCCESS();

    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_shader_multisample_interpolation"));

    // Init program
    GLuint program = createProgram();
    ASSERT_NE(0u, program);
    glUseProgram(program);

    GLBuffer indexBuffer;
    GLVertexArray vertexArray;
    GLBuffer vertexBuffers;
    initializeGeometry(program, &indexBuffer, &vertexArray, &vertexBuffers);
    ASSERT_GL_NO_ERROR();

    // Create a texture-backed FBO
    GLFramebuffer fbo;
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mSize, mSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    ASSERT_GL_NO_ERROR();

    glViewport(0, 0, mSize, mSize);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor(0, 255, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(mSize - 1, 0, GLColor(0, 255, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(0, mSize - 1, GLColor(0, 255, 0, 255));
    EXPECT_PIXEL_COLOR_EQ(mSize - 1, mSize - 1, GLColor(0, 255, 0, 255));
    ASSERT_GL_NO_ERROR();
}

}  // anonymous namespace

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLPreRotationInterpolateAtOffsetTest);
ANGLE_INSTANTIATE_TEST_COMBINE_1(EGLPreRotationInterpolateAtOffsetTest,
                                 PrintToStringParamName,
                                 testing::Bool(),
                                 WithNoFixture(ES31_VULKAN()));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLPreRotationSurfaceTest);
ANGLE_INSTANTIATE_TEST_COMBINE_1(EGLPreRotationSurfaceTest,
                                 PrintToStringParamName,
                                 testing::Bool(),
                                 WithNoFixture(ES2_VULKAN()),
                                 WithNoFixture(ES3_VULKAN()),
                                 WithNoFixture(ES3_VULKAN_SWIFTSHADER()));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLPreRotationLargeSurfaceTest);
ANGLE_INSTANTIATE_TEST_COMBINE_1(EGLPreRotationLargeSurfaceTest,
                                 PrintToStringParamName,
                                 testing::Bool(),
                                 WithNoFixture(ES2_VULKAN()),
                                 WithNoFixture(ES3_VULKAN()),
                                 WithNoFixture(ES3_VULKAN_SWIFTSHADER()));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(EGLPreRotationBlitFramebufferTest);
ANGLE_INSTANTIATE_TEST_COMBINE_1(EGLPreRotationBlitFramebufferTest,
                                 PrintToStringParamName,
                                 testing::Bool(),
                                 WithNoFixture(ES2_VULKAN()),
                                 WithNoFixture(ES3_VULKAN()),
                                 WithNoFixture(ES3_VULKAN_SWIFTSHADER()));
