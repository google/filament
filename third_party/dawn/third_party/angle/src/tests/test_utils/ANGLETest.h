//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ANGLETest:
//   Implementation of common ANGLE testing fixture.
//

#ifndef ANGLE_TESTS_ANGLE_TEST_H_
#define ANGLE_TESTS_ANGLE_TEST_H_

#include <gtest/gtest.h>
#include <algorithm>
#include <array>

#include "RenderDoc.h"
#include "angle_test_configs.h"
#include "angle_test_platform.h"
#include "common/angleutils.h"
#include "common/system_utils.h"
#include "common/vector_utils.h"
#include "platform/PlatformMethods.h"
#include "util/EGLWindow.h"
#include "util/shader_utils.h"
#include "util/util_gl.h"

namespace angle
{
struct SystemInfo;
class RNG;
}  // namespace angle

#define ASSERT_GL_TRUE(a) ASSERT_EQ(static_cast<GLboolean>(GL_TRUE), (a))
#define ASSERT_GL_FALSE(a) ASSERT_EQ(static_cast<GLboolean>(GL_FALSE), (a))
#define EXPECT_GL_TRUE(a) EXPECT_EQ(static_cast<GLboolean>(GL_TRUE), (a))
#define EXPECT_GL_FALSE(a) EXPECT_EQ(static_cast<GLboolean>(GL_FALSE), (a))

#define EXPECT_GL_ERROR(err) EXPECT_EQ(static_cast<GLenum>(err), glGetError())
#define EXPECT_GL_NO_ERROR() EXPECT_EQ(static_cast<GLenum>(GL_NO_ERROR), glGetError())

#define ASSERT_GL_ERROR(err) ASSERT_EQ(static_cast<GLenum>(err), glGetError())
#define ASSERT_GL_NO_ERROR() ASSERT_EQ(static_cast<GLenum>(GL_NO_ERROR), glGetError())

#define EXPECT_EGL_ERROR(err) EXPECT_EQ((err), eglGetError())
#define EXPECT_EGL_SUCCESS() EXPECT_EGL_ERROR(EGL_SUCCESS)

// EGLBoolean is |unsigned int| but EGL_TRUE is 0, not 0u.
#define ASSERT_EGL_TRUE(a) ASSERT_EQ(static_cast<EGLBoolean>(EGL_TRUE), static_cast<EGLBoolean>(a))
#define ASSERT_EGL_FALSE(a) \
    ASSERT_EQ(static_cast<EGLBoolean>(EGL_FALSE), static_cast<EGLBoolean>(a))
#define EXPECT_EGL_TRUE(a) EXPECT_EQ(static_cast<EGLBoolean>(EGL_TRUE), static_cast<EGLBoolean>(a))
#define EXPECT_EGL_FALSE(a) \
    EXPECT_EQ(static_cast<EGLBoolean>(EGL_FALSE), static_cast<EGLBoolean>(a))

#define ASSERT_EGL_ERROR(err) ASSERT_EQ((err), eglGetError())
#define ASSERT_EGL_SUCCESS() ASSERT_EGL_ERROR(EGL_SUCCESS)

#define ASSERT_GLENUM_EQ(expected, actual) \
    ASSERT_EQ(static_cast<GLenum>(expected), static_cast<GLenum>(actual))
#define EXPECT_GLENUM_EQ(expected, actual) \
    EXPECT_EQ(static_cast<GLenum>(expected), static_cast<GLenum>(actual))
#define ASSERT_GLENUM_NE(expected, actual) \
    ASSERT_NE(static_cast<GLenum>(expected), static_cast<GLenum>(actual))
#define EXPECT_GLENUM_NE(expected, actual) \
    EXPECT_NE(static_cast<GLenum>(expected), static_cast<GLenum>(actual))

testing::AssertionResult AssertEGLEnumsEqual(const char *lhsExpr,
                                             const char *rhsExpr,
                                             EGLenum lhs,
                                             EGLenum rhs);

#define ASSERT_EGLENUM_EQ(expected, actual)                                  \
    ASSERT_PRED_FORMAT2(AssertEGLEnumsEqual, static_cast<EGLenum>(expected), \
                        static_cast<EGLenum>(actual))
#define EXPECT_EGLENUM_EQ(expected, actual)                                  \
    EXPECT_PRED_FORMAT2(AssertEGLEnumsEqual, static_cast<EGLenum>(expected), \
                        static_cast<EGLenum>(actual))

#define ASSERT_GL_FRAMEBUFFER_COMPLETE(framebuffer) \
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(framebuffer))
#define EXPECT_GL_FRAMEBUFFER_COMPLETE(framebuffer) \
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(framebuffer))

namespace angle
{
struct GLColorRGB
{
    constexpr GLColorRGB() : R(0), G(0), B(0) {}
    constexpr GLColorRGB(GLubyte r, GLubyte g, GLubyte b) : R(r), G(g), B(b) {}
    GLColorRGB(const angle::Vector3 &floatColor);

    const GLubyte *data() const { return &R; }
    GLubyte *data() { return &R; }

    GLubyte R, G, B;

    static const GLColorRGB black;
    static const GLColorRGB blue;
    static const GLColorRGB green;
    static const GLColorRGB red;
    static const GLColorRGB yellow;
};

struct GLColorRG
{
    constexpr GLColorRG() : R(0), G(0) {}
    constexpr GLColorRG(GLubyte r, GLubyte g) : R(r), G(g) {}
    GLColorRG(const angle::Vector2 &floatColor);

    const GLubyte *data() const { return &R; }
    GLubyte *data() { return &R; }

    GLubyte R, G;
};

struct GLColorR
{
    constexpr GLColorR() : R(0) {}
    constexpr GLColorR(GLubyte r) : R(r) {}
    GLColorR(const float floatColor);

    const GLubyte *data() const { return &R; }
    GLubyte *data() { return &R; }

    GLubyte R;
};

struct GLColor
{
    constexpr GLColor() : R(0), G(0), B(0), A(0) {}
    constexpr GLColor(GLubyte r, GLubyte g, GLubyte b, GLubyte a) : R(r), G(g), B(b), A(a) {}
    GLColor(const angle::Vector3 &floatColor);
    GLColor(const angle::Vector4 &floatColor);
    GLColor(GLuint colorValue);

    angle::Vector4 toNormalizedVector() const;

    GLubyte &operator[](size_t index) { return (&R)[index]; }

    const GLubyte &operator[](size_t index) const { return (&R)[index]; }

    const GLubyte *data() const { return &R; }
    GLubyte *data() { return &R; }

    GLuint asUint() const;

    testing::AssertionResult ExpectNear(const GLColor &expected, const GLColor &err) const;

    GLubyte R, G, B, A;

    static const GLColor black;
    static const GLColor blue;
    static const GLColor cyan;
    static const GLColor green;
    static const GLColor red;
    static const GLColor transparentBlack;
    static const GLColor white;
    static const GLColor yellow;
    static const GLColor magenta;
};

template <typename T>
struct GLColorT
{
    constexpr GLColorT() : GLColorT(0, 0, 0, 0) {}
    constexpr GLColorT(T r, T g, T b, T a) : R(r), G(g), B(b), A(a) {}

    T R, G, B, A;
};

using GLColor16   = GLColorT<uint16_t>;
using GLColor32F  = GLColorT<float>;
using GLColor32I  = GLColorT<int32_t>;
using GLColor32UI = GLColorT<uint32_t>;

static constexpr GLColor32F kFloatBlack = {0.0f, 0.0f, 0.0f, 1.0f};
static constexpr GLColor32F kFloatRed   = {1.0f, 0.0f, 0.0f, 1.0f};
static constexpr GLColor32F kFloatGreen = {0.0f, 1.0f, 0.0f, 1.0f};
static constexpr GLColor32F kFloatBlue  = {0.0f, 0.0f, 1.0f, 1.0f};

// The input here for pixelPoints are the expected integer window coordinates, we add .5 to every
// one of them and re-scale the numbers to be between [-1,1]. Using this technique, we can make
// sure the rasterization stage will end up drawing pixels at the expected locations.
void CreatePixelCenterWindowCoords(const std::vector<Vector2> &pixelPoints,
                                   int windowWidth,
                                   int windowHeight,
                                   std::vector<Vector3> *outVertices);

// Useful to cast any type to GLubyte.
template <typename TR, typename TG, typename TB, typename TA>
GLColor MakeGLColor(TR r, TG g, TB b, TA a)
{
    return GLColor(static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b),
                   static_cast<GLubyte>(a));
}

GLColor RandomColor(angle::RNG *rng);

bool operator==(const GLColor &a, const GLColor &b);
bool operator!=(const GLColor &a, const GLColor &b);
std::ostream &operator<<(std::ostream &ostream, const GLColor &color);
GLColor ReadColor(GLint x, GLint y);

bool operator==(const GLColorRGB &a, const GLColorRGB &b);
bool operator!=(const GLColorRGB &a, const GLColorRGB &b);
std::ostream &operator<<(std::ostream &ostream, const GLColorRGB &color);

// Useful to cast any type to GLfloat.
template <typename TR, typename TG, typename TB, typename TA>
GLColor32F MakeGLColor32F(TR r, TG g, TB b, TA a)
{
    return GLColor32F(static_cast<GLfloat>(r), static_cast<GLfloat>(g), static_cast<GLfloat>(b),
                      static_cast<GLfloat>(a));
}

template <typename T>
bool operator==(const GLColorT<T> &a, const GLColorT<T> &b)
{
    return a.R == b.R && a.G == b.G && a.B == b.B && a.A == b.A;
}

std::ostream &operator<<(std::ostream &ostream, const GLColor32F &color);
GLColor32F ReadColor32F(GLint x, GLint y);

constexpr std::array<GLenum, 6> kCubeFaces = {
    {GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
     GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
     GL_TEXTURE_CUBE_MAP_NEGATIVE_Z}};

void LoadEntryPointsWithUtilLoader(angle::GLESDriverType driver);

bool IsFormatEmulated(GLenum target);
}  // namespace angle

#define EXPECT_PIXEL_EQ(x, y, r, g, b, a) \
    EXPECT_EQ(angle::MakeGLColor(r, g, b, a), angle::ReadColor(x, y))

#define EXPECT_PIXEL_NE(x, y, r, g, b, a) \
    EXPECT_NE(angle::MakeGLColor(r, g, b, a), angle::ReadColor(x, y))

#define EXPECT_PIXEL_32F_EQ(x, y, r, g, b, a) \
    EXPECT_EQ(angle::MakeGLColor32F(r, g, b, a), angle::ReadColor32F(x, y))

#define EXPECT_PIXEL_ALPHA_EQ(x, y, a) EXPECT_EQ(a, angle::ReadColor(x, y).A)

#define EXPECT_PIXEL_ALPHA_NEAR(x, y, a, abs_error) \
    EXPECT_NEAR(a, angle::ReadColor(x, y).A, abs_error);

#define EXPECT_PIXEL_ALPHA32F_EQ(x, y, a) EXPECT_EQ(a, angle::ReadColor32F(x, y).A)

#define EXPECT_PIXEL_COLOR_EQ(x, y, angleColor) EXPECT_EQ(angleColor, angle::ReadColor(x, y))
#define EXPECT_PIXEL_COLOR_EQ_VEC2(vec2, angleColor) \
    EXPECT_EQ(angleColor,                            \
              angle::ReadColor(static_cast<GLint>(vec2.x()), static_cast<GLint>(vec2.y())))

#define EXPECT_PIXEL_COLOR32F_EQ(x, y, angleColor) EXPECT_EQ(angleColor, angle::ReadColor32F(x, y))

#define EXPECT_PIXEL_RECT_T_EQ(T, x, y, width, height, format, type, color)           \
    do                                                                                \
    {                                                                                 \
        std::vector<T> actualColors((width) * (height));                              \
        glReadPixels((x), (y), (width), (height), format, type, actualColors.data()); \
        std::vector<T> expectedColors((width) * (height), color);                     \
        EXPECT_EQ(expectedColors, actualColors);                                      \
    } while (0)

#define EXPECT_PIXEL_RECT_EQ(x, y, width, height, color) \
    EXPECT_PIXEL_RECT_T_EQ(GLColor, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, color)

#define EXPECT_PIXEL_RECT32F_EQ(x, y, width, height, color) \
    EXPECT_PIXEL_RECT_T_EQ(GLColor32F, x, y, width, height, GL_RGBA, GL_FLOAT, color)

#define EXPECT_PIXEL_RECT32I_EQ(x, y, width, height, color) \
    EXPECT_PIXEL_RECT_T_EQ(GLColor32I, x, y, width, height, GL_RGBA_INTEGER, GL_INT, color)

#define EXPECT_PIXEL_RECT32UI_EQ(x, y, width, height, color)                                   \
    EXPECT_PIXEL_RECT_T_EQ(GLColor32UI, x, y, width, height, GL_RGBA_INTEGER, GL_UNSIGNED_INT, \
                           color)

#define EXPECT_PIXEL_NEAR_HELPER(x, y, r, g, b, a, abs_error, ctype, format, type) \
    do                                                                             \
    {                                                                              \
        ctype pixel[4];                                                            \
        glReadPixels((x), (y), 1, 1, format, type, pixel);                         \
        EXPECT_GL_NO_ERROR();                                                      \
        EXPECT_NEAR((r), pixel[0], abs_error);                                     \
        EXPECT_NEAR((g), pixel[1], abs_error);                                     \
        EXPECT_NEAR((b), pixel[2], abs_error);                                     \
        EXPECT_NEAR((a), pixel[3], abs_error);                                     \
    } while (0)

#define EXPECT_PIXEL_EQ_HELPER(x, y, r, g, b, a, ctype, format, type) \
    do                                                                \
    {                                                                 \
        ctype pixel[4];                                               \
        glReadPixels((x), (y), 1, 1, format, type, pixel);            \
        EXPECT_GL_NO_ERROR();                                         \
        EXPECT_EQ((r), pixel[0]);                                     \
        EXPECT_EQ((g), pixel[1]);                                     \
        EXPECT_EQ((b), pixel[2]);                                     \
        EXPECT_EQ((a), pixel[3]);                                     \
    } while (0)

#define EXPECT_PIXEL_NEAR(x, y, r, g, b, a, abs_error) \
    EXPECT_PIXEL_NEAR_HELPER(x, y, r, g, b, a, abs_error, GLubyte, GL_RGBA, GL_UNSIGNED_BYTE)

#define EXPECT_PIXEL_16_NEAR(x, y, r, g, b, a, abs_error) \
    EXPECT_PIXEL_NEAR_HELPER(x, y, r, g, b, a, abs_error, GLushort, GL_RGBA, GL_UNSIGNED_SHORT)

#define EXPECT_PIXEL_8S_NEAR(x, y, r, g, b, a, abs_error) \
    EXPECT_PIXEL_NEAR_HELPER(x, y, r, g, b, a, abs_error, GLbyte, GL_RGBA, GL_BYTE)

#define EXPECT_PIXEL_16S_NEAR(x, y, r, g, b, a, abs_error) \
    EXPECT_PIXEL_NEAR_HELPER(x, y, r, g, b, a, abs_error, GLshort, GL_RGBA, GL_SHORT)

#define EXPECT_PIXEL_32F_NEAR(x, y, r, g, b, a, abs_error) \
    EXPECT_PIXEL_NEAR_HELPER(x, y, r, g, b, a, abs_error, GLfloat, GL_RGBA, GL_FLOAT)

#define EXPECT_PIXEL_8I(x, y, r, g, b, a) \
    EXPECT_PIXEL_EQ_HELPER(x, y, r, g, b, a, GLbyte, GL_RGBA_INTEGER, GL_BYTE)

#define EXPECT_PIXEL_8UI(x, y, r, g, b, a) \
    EXPECT_PIXEL_EQ_HELPER(x, y, r, g, b, a, GLubyte, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE)

#define EXPECT_PIXEL_32UI(x, y, r, g, b, a) \
    EXPECT_PIXEL_EQ_HELPER(x, y, r, g, b, a, GLuint, GL_RGBA_INTEGER, GL_UNSIGNED_INT)

#define EXPECT_PIXEL_32I(x, y, r, g, b, a) \
    EXPECT_PIXEL_EQ_HELPER(x, y, r, g, b, a, GLint, GL_RGBA_INTEGER, GL_INT)

#define EXPECT_PIXEL_32UI_COLOR(x, y, color) \
    EXPECT_PIXEL_32UI(x, y, color.R, color.G, color.B, color.A)

#define EXPECT_PIXEL_32I_COLOR(x, y, color) \
    EXPECT_PIXEL_32I(x, y, color.R, color.G, color.B, color.A)

// TODO(jmadill): Figure out how we can use GLColor's nice printing with EXPECT_NEAR.
#define EXPECT_PIXEL_COLOR_NEAR(x, y, angleColor, abs_error) \
    EXPECT_PIXEL_NEAR(x, y, angleColor.R, angleColor.G, angleColor.B, angleColor.A, abs_error)

#define EXPECT_PIXEL_COLOR16_NEAR(x, y, angleColor, abs_error) \
    EXPECT_PIXEL_16_NEAR(x, y, angleColor.R, angleColor.G, angleColor.B, angleColor.A, abs_error)

#define EXPECT_PIXEL_COLOR32F_NEAR(x, y, angleColor, abs_error) \
    EXPECT_PIXEL32F_NEAR(x, y, angleColor.R, angleColor.G, angleColor.B, angleColor.A, abs_error)

#define EXPECT_COLOR_NEAR(expected, actual, abs_error) \
    do                                                 \
    {                                                  \
        EXPECT_NEAR(expected.R, actual.R, abs_error);  \
        EXPECT_NEAR(expected.G, actual.G, abs_error);  \
        EXPECT_NEAR(expected.B, actual.B, abs_error);  \
        EXPECT_NEAR(expected.A, actual.A, abs_error);  \
    } while (0)
#define EXPECT_PIXEL32F_NEAR(x, y, r, g, b, a, abs_error)       \
    do                                                          \
    {                                                           \
        GLfloat pixel[4];                                       \
        glReadPixels((x), (y), 1, 1, GL_RGBA, GL_FLOAT, pixel); \
        EXPECT_GL_NO_ERROR();                                   \
        EXPECT_NEAR((r), pixel[0], abs_error);                  \
        EXPECT_NEAR((g), pixel[1], abs_error);                  \
        EXPECT_NEAR((b), pixel[2], abs_error);                  \
        EXPECT_NEAR((a), pixel[3], abs_error);                  \
    } while (0)

#define EXPECT_PIXEL_COLOR32F_NEAR(x, y, angleColor, abs_error) \
    EXPECT_PIXEL32F_NEAR(x, y, angleColor.R, angleColor.G, angleColor.B, angleColor.A, abs_error)

#define EXPECT_PIXEL_STENCIL_EQ(x, y, expected)                                    \
    do                                                                             \
    {                                                                              \
        GLubyte actual;                                                            \
        glReadPixels((x), (y), 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &actual); \
        EXPECT_GL_NO_ERROR();                                                      \
        EXPECT_EQ((expected), actual);                                             \
    } while (0)

class ANGLETestBase;
class EGLWindow;
class GLWindowBase;
class OSWindow;
class WGLWindow;

struct TestPlatformContext final : private angle::NonCopyable
{
    bool ignoreMessages        = false;
    bool warningsAsErrors      = false;
    ANGLETestBase *currentTest = nullptr;
};

class ANGLETestBase
{
  protected:
    ANGLETestBase(const angle::PlatformParameters &params);
    virtual ~ANGLETestBase();

  public:
    void setWindowVisible(OSWindow *osWindow, bool isVisible);

    static void ReleaseFixtures();

    bool isSwiftshader() const
    {
        // Renderer might be swiftshader even if local swiftshader not used.
        return mCurrentParams->isSwiftshader() || angle::IsSwiftshaderDevice();
    }

    bool enableDebugLayers() const
    {
        return mCurrentParams->eglParameters.debugLayersEnabled != EGL_FALSE;
    }

    void *operator new(size_t size);
    void operator delete(void *ptr);

  protected:
    void ANGLETestSetUp();
    void ANGLETestPreTearDown();
    void ANGLETestTearDown();

    virtual void swapBuffers();

    void setupQuadVertexBuffer(GLfloat positionAttribZ, GLfloat positionAttribXYScale);
    void setupIndexedQuadVertexBuffer(GLfloat positionAttribZ, GLfloat positionAttribXYScale);
    void setupIndexedQuadIndexBuffer();

    void drawQuad(GLuint program, const std::string &positionAttribName, GLfloat positionAttribZ);
    void drawQuad(GLuint program,
                  const std::string &positionAttribName,
                  GLfloat positionAttribZ,
                  GLfloat positionAttribXYScale);
    void drawQuad(GLuint program,
                  const std::string &positionAttribName,
                  GLfloat positionAttribZ,
                  GLfloat positionAttribXYScale,
                  bool useVertexBuffer);
    void drawQuadInstanced(GLuint program,
                           const std::string &positionAttribName,
                           GLfloat positionAttribZ,
                           GLfloat positionAttribXYScale,
                           bool useVertexBuffer,
                           GLuint numInstances);
    void drawPatches(GLuint program,
                     const std::string &positionAttribName,
                     GLfloat positionAttribZ,
                     GLfloat positionAttribXYScale,
                     bool useVertexBuffer);

    void drawQuadPPO(GLuint vertProgram,
                     const std::string &positionAttribName,
                     const GLfloat positionAttribZ,
                     const GLfloat positionAttribXYScale);

    static std::array<angle::Vector3, 6> GetQuadVertices();
    static std::array<GLushort, 6> GetQuadIndices();
    static std::array<angle::Vector3, 4> GetIndexedQuadVertices();

    void drawIndexedQuad(GLuint program,
                         const std::string &positionAttribName,
                         GLfloat positionAttribZ);
    void drawIndexedQuad(GLuint program,
                         const std::string &positionAttribName,
                         GLfloat positionAttribZ,
                         GLfloat positionAttribXYScale);
    void drawIndexedQuad(GLuint program,
                         const std::string &positionAttribName,
                         GLfloat positionAttribZ,
                         GLfloat positionAttribXYScale,
                         bool useBufferObject);

    void drawIndexedQuad(GLuint program,
                         const std::string &positionAttribName,
                         GLfloat positionAttribZ,
                         GLfloat positionAttribXYScale,
                         bool useBufferObject,
                         bool restrictedRange);

    void draw2DTexturedQuad(GLfloat positionAttribZ,
                            GLfloat positionAttribXYScale,
                            bool useVertexBuffer);

    // The layer parameter chooses the 3D texture layer to sample from.
    void draw3DTexturedQuad(GLfloat positionAttribZ,
                            GLfloat positionAttribXYScale,
                            bool useVertexBuffer,
                            float layer);

    // The layer parameter chooses the 2DArray texture layer to sample from.
    void draw2DArrayTexturedQuad(GLfloat positionAttribZ,
                                 GLfloat positionAttribXYScale,
                                 bool useVertexBuffer,
                                 float layer);

    void setWindowWidth(int width);
    void setWindowHeight(int height);
    void setConfigRedBits(int bits);
    void setConfigGreenBits(int bits);
    void setConfigBlueBits(int bits);
    void setConfigAlphaBits(int bits);
    void setConfigDepthBits(int bits);
    void setConfigStencilBits(int bits);
    void setConfigComponentType(EGLenum componentType);
    void setMultisampleEnabled(bool enabled);
    void setSamples(EGLint samples);
    void setDebugEnabled(bool enabled);
    void setNoErrorEnabled(bool enabled);
    void setWebGLCompatibilityEnabled(bool webglCompatibility);
    void setExtensionsEnabled(bool extensionsEnabled);
    void setRobustAccess(bool enabled);
    void setBindGeneratesResource(bool bindGeneratesResource);
    void setClientArraysEnabled(bool enabled);
    void setRobustResourceInit(bool enabled);
    void setMutableRenderBuffer(bool enabled);
    void setContextProgramCacheEnabled(bool enabled);
    void setContextResetStrategy(EGLenum resetStrategy);
    void forceNewDisplay();

    // Some EGL extension tests would like to defer the Context init until the test body.
    void setDeferContextInit(bool enabled);

    int getClientMajorVersion() const;
    int getClientMinorVersion() const;

    GLWindowBase *getGLWindow() const;
    EGLWindow *getEGLWindow() const;
    int getWindowWidth() const;
    int getWindowHeight() const;

    EGLint getPlatformRenderer() const;

    void ignoreD3D11SDKLayersWarnings();

    OSWindow *getOSWindow() { return mFixture->osWindow; }

    GLuint get2DTexturedQuadProgram();

    // Has a float uniform "u_layer" to choose the 3D texture layer.
    GLuint get3DTexturedQuadProgram();

    // Has a float uniform "u_layer" to choose the 2DArray texture layer.
    GLuint get2DArrayTexturedQuadProgram();

    class [[nodiscard]] ScopedIgnorePlatformMessages : angle::NonCopyable
    {
      public:
        ScopedIgnorePlatformMessages();
        ~ScopedIgnorePlatformMessages();
    };

    // Can be used before we get a GL context.
    bool isGLRenderer() const
    {
        return mCurrentParams->getRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE;
    }

    bool isGLESRenderer() const
    {
        return mCurrentParams->getRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE;
    }

    bool isD3D11Renderer() const
    {
        return mCurrentParams->getRenderer() == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
    }

    bool isVulkanRenderer() const
    {
        return mCurrentParams->getRenderer() == EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE;
    }

    bool isVulkanSwiftshaderRenderer() const
    {
        return mCurrentParams->getRenderer() == EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE &&
               mCurrentParams->isSwiftshader();
    }

    bool platformSupportsMultithreading() const;

    bool mIsSetUp = false;

  private:
    void checkD3D11SDKLayersMessages();

    void drawQuad(GLuint program,
                  const std::string &positionAttribName,
                  GLfloat positionAttribZ,
                  GLfloat positionAttribXYScale,
                  bool useVertexBuffer,
                  bool useInstancedDrawCalls,
                  bool useTessellationPatches,
                  GLuint numInstances);

    void initOSWindow();

    struct TestFixture
    {
        TestFixture();
        ~TestFixture();

        EGLWindow *eglWindow = nullptr;
        WGLWindow *wglWindow = nullptr;
        OSWindow *osWindow   = nullptr;
        ConfigParameters configParams;
        uint32_t reuseCounter = 0;
    };

    int mWidth;
    int mHeight;

    bool mIgnoreD3D11SDKLayersWarnings;

    // Used for indexed quad rendering
    GLuint mQuadVertexBuffer;
    GLuint mQuadIndexBuffer;

    // Used for texture rendering.
    GLuint m2DTexturedQuadProgram;
    GLuint m3DTexturedQuadProgram;
    GLuint m2DArrayTexturedQuadProgram;

    bool mDeferContextInit;
    bool mAlwaysForceNewDisplay;
    bool mForceNewDisplay;

    bool mSetUpCalled;
    bool mTearDownCalled;

    // Usually, we use an OS Window per "fixture" (a frontend and backend combination).
    // This allows:
    // 1. Reusing EGL Display on Windows.
    //    Other platforms have issues with display reuse even if a window per fixture is used.
    // 2. Hiding only SwiftShader OS Window on Linux.
    //    OS Windows for other backends must be visible, to allow driver to communicate with X11.
    // However, we must use a single OS Window for all backends on Android,
    // since test Application can have only one window.
    static OSWindow *mOSWindowSingleton;

    static std::map<angle::PlatformParameters, TestFixture> gFixtures;
    const angle::PlatformParameters *mCurrentParams;
    TestFixture *mFixture;

    RenderDoc mRenderDoc;

    // Workaround for NVIDIA not being able to share a window with OpenGL and Vulkan.
    static Optional<EGLint> mLastRendererType;
    static Optional<angle::GLESDriverType> mLastLoadedDriver;
};

template <typename Params = angle::PlatformParameters>
class ANGLETest : public ANGLETestBase, public ::testing::TestWithParam<Params>
{
  protected:
    ANGLETest();

    virtual void testSetUp() {}
    virtual void testTearDown() {}

    void recreateTestFixture()
    {
        TearDown();
        SetUp();
    }

  private:
    void SetUp() final
    {
        ANGLETestBase::ANGLETestSetUp();
        if (mIsSetUp)
        {
            testSetUp();
        }
    }

    void TearDown() final
    {
        ANGLETestBase::ANGLETestPreTearDown();
        if (mIsSetUp)
        {
            testTearDown();
        }
        ANGLETestBase::ANGLETestTearDown();
    }
};

enum class APIExtensionVersion
{
    Core,
    OES,
    EXT,
    KHR,
};

template <typename Params>
ANGLETest<Params>::ANGLETest()
    : ANGLETestBase(std::get<angle::PlatformParameters>(this->GetParam()))
{}

template <>
inline ANGLETest<angle::PlatformParameters>::ANGLETest() : ANGLETestBase(this->GetParam())
{}

class ANGLETestEnvironment : public testing::Environment
{
  public:
    void SetUp() override;
    void TearDown() override;

    static angle::Library *GetDriverLibrary(angle::GLESDriverType driver);

  private:
    static angle::Library *GetAngleEGLLibrary();
    static angle::Library *GetAngleVulkanSecondariesEGLLibrary();
    static angle::Library *GetMesaEGLLibrary();
    static angle::Library *GetSystemEGLLibrary();
    static angle::Library *GetSystemWGLLibrary();

    // For loading entry points.
    static std::unique_ptr<angle::Library> gAngleEGLLibrary;
    static std::unique_ptr<angle::Library> gAngleVulkanSecondariesEGLLibrary;
    static std::unique_ptr<angle::Library> gMesaEGLLibrary;
    static std::unique_ptr<angle::Library> gSystemEGLLibrary;
    static std::unique_ptr<angle::Library> gSystemWGLLibrary;
};

extern angle::PlatformMethods gDefaultPlatformMethods;

#endif  // ANGLE_TESTS_ANGLE_TEST_H_
