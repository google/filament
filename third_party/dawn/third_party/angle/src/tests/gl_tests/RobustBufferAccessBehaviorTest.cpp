//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RobustBufferAccessBehaviorTest:
//   Various tests related for GL_KHR_robust_buffer_access_behavior.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"

#include <array>

using namespace angle;

namespace
{

class RobustBufferAccessBehaviorTest : public ANGLETest<>
{
  protected:
    RobustBufferAccessBehaviorTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
    }

    void testTearDown() override
    {
        glDeleteProgram(mProgram);
        EGLWindow::Delete(&mEGLWindow);
        OSWindow::Delete(&mOSWindow);
    }

    bool initExtension()
    {
        mOSWindow = OSWindow::New();
        if (!mOSWindow->initialize("RobustBufferAccessBehaviorTest", getWindowWidth(),
                                   getWindowHeight()))
        {
            return false;
        }
        setWindowVisible(mOSWindow, true);

        Library *driverLib = ANGLETestEnvironment::GetDriverLibrary(GLESDriverType::AngleEGL);

        const PlatformParameters &params = GetParam();
        mEGLWindow                       = EGLWindow::New(params.majorVersion, params.minorVersion);
        if (!mEGLWindow->initializeDisplay(mOSWindow, driverLib, GLESDriverType::AngleEGL,
                                           GetParam().eglParameters))
        {
            return false;
        }

        EGLDisplay display = mEGLWindow->getDisplay();
        if (!IsEGLDisplayExtensionEnabled(display, "EGL_EXT_create_context_robustness"))
        {
            return false;
        }

        ConfigParameters configParams;
        configParams.redBits      = 8;
        configParams.greenBits    = 8;
        configParams.blueBits     = 8;
        configParams.alphaBits    = 8;
        configParams.robustAccess = true;

        if (mEGLWindow->initializeSurface(mOSWindow, driverLib, configParams) !=
            GLWindowResult::NoError)
        {
            return false;
        }

        if (!mEGLWindow->initializeContext())
        {
            return false;
        }

        if (!mEGLWindow->makeCurrent())
        {
            return false;
        }

        if (!IsGLExtensionEnabled("GL_KHR_robust_buffer_access_behavior"))
        {
            return false;
        }
        return true;
    }

    void initBasicProgram()
    {
        constexpr char kVS[] =
            "precision mediump float;\n"
            "attribute vec4 position;\n"
            "attribute vec4 vecRandom;\n"
            "varying vec4 v_color;\n"
            "bool testFloatComponent(float component) {\n"
            "    return (component == 0.2 || component == 0.0);\n"
            "}\n"
            "bool testLastFloatComponent(float component) {\n"
            "    return testFloatComponent(component) || component == 1.0;\n"
            "}\n"
            "void main() {\n"
            "    if (testFloatComponent(vecRandom.x) &&\n"
            "        testFloatComponent(vecRandom.y) &&\n"
            "        testFloatComponent(vecRandom.z) &&\n"
            "        testLastFloatComponent(vecRandom.w)) {\n"
            "        v_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
            "    } else {\n"
            "        v_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "    }\n"
            "    gl_Position = position;\n"
            "}\n";

        constexpr char kFS[] =
            "precision mediump float;\n"
            "varying vec4 v_color;\n"
            "void main() {\n"
            "    gl_FragColor = v_color;\n"
            "}\n";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        mTestAttrib = glGetAttribLocation(mProgram, "vecRandom");
        ASSERT_NE(-1, mTestAttrib);

        glUseProgram(mProgram);
    }

    void runIndexOutOfRangeTests(GLenum drawType)
    {
        if (mProgram == 0)
        {
            initBasicProgram();
        }

        GLBuffer bufferIncomplete;
        glBindBuffer(GL_ARRAY_BUFFER, bufferIncomplete);
        std::array<GLfloat, 12> randomData = {
            {0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f, 0.2f}};
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * randomData.size(), randomData.data(),
                     drawType);

        glEnableVertexAttribArray(mTestAttrib);
        glVertexAttribPointer(mTestAttrib, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

        glClearColor(0.0, 0.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawIndexedQuad(mProgram, "position", 0.5f);

        int width     = getWindowWidth();
        int height    = getWindowHeight();
        GLenum result = glGetError();
        // For D3D dynamic draw, we still return invalid operation. Once we force the index buffer
        // to clamp any out of range indices instead of invalid operation, this part can be removed.
        // We can always get GL_NO_ERROR.
        if (result == GL_INVALID_OPERATION)
        {
            EXPECT_PIXEL_COLOR_EQ(width * 1 / 4, height * 1 / 4, GLColor::blue);
            EXPECT_PIXEL_COLOR_EQ(width * 1 / 4, height * 3 / 4, GLColor::blue);
            EXPECT_PIXEL_COLOR_EQ(width * 3 / 4, height * 1 / 4, GLColor::blue);
            EXPECT_PIXEL_COLOR_EQ(width * 3 / 4, height * 3 / 4, GLColor::blue);
        }
        else
        {
            EXPECT_GLENUM_EQ(GL_NO_ERROR, result);
            EXPECT_PIXEL_COLOR_EQ(width * 1 / 4, height * 1 / 4, GLColor::green);
            EXPECT_PIXEL_COLOR_EQ(width * 1 / 4, height * 3 / 4, GLColor::green);
            EXPECT_PIXEL_COLOR_EQ(width * 3 / 4, height * 1 / 4, GLColor::green);
            EXPECT_PIXEL_COLOR_EQ(width * 3 / 4, height * 3 / 4, GLColor::green);
        }
    }

    OSWindow *mOSWindow   = nullptr;
    EGLWindow *mEGLWindow = nullptr;
    GLuint mProgram       = 0;
    GLint mTestAttrib     = 0;
};

// Test that static draw with out-of-bounds reads will not read outside of the data store of the
// buffer object and will not result in GL interruption or termination when
// GL_KHR_robust_buffer_access_behavior is supported.
TEST_P(RobustBufferAccessBehaviorTest, DrawElementsIndexOutOfRangeWithStaticDraw)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    runIndexOutOfRangeTests(GL_STATIC_DRAW);
}

// Test that dynamic draw with out-of-bounds reads will not read outside of the data store of the
// buffer object and will not result in GL interruption or termination when
// GL_KHR_robust_buffer_access_behavior is supported.
TEST_P(RobustBufferAccessBehaviorTest, DrawElementsIndexOutOfRangeWithDynamicDraw)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    runIndexOutOfRangeTests(GL_DYNAMIC_DRAW);
}

// Test that vertex buffers are rebound with the correct offsets in subsequent calls in the D3D11
// backend.  http://crbug.com/837002
TEST_P(RobustBufferAccessBehaviorTest, D3D11StateSynchronizationOrderBug)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    glDisable(GL_DEPTH_TEST);

    // 2 quads, the first one red, the second one green
    const std::array<angle::Vector4, 16> vertices{
        angle::Vector4(-1.0f, 1.0f, 0.5f, 1.0f),   // v0
        angle::Vector4(1.0f, 0.0f, 0.0f, 1.0f),    // c0
        angle::Vector4(-1.0f, -1.0f, 0.5f, 1.0f),  // v1
        angle::Vector4(1.0f, 0.0f, 0.0f, 1.0f),    // c1
        angle::Vector4(1.0f, -1.0f, 0.5f, 1.0f),   // v2
        angle::Vector4(1.0f, 0.0f, 0.0f, 1.0f),    // c2
        angle::Vector4(1.0f, 1.0f, 0.5f, 1.0f),    // v3
        angle::Vector4(1.0f, 0.0f, 0.0f, 1.0f),    // c3

        angle::Vector4(-1.0f, 1.0f, 0.5f, 1.0f),   // v4
        angle::Vector4(0.0f, 1.0f, 0.0f, 1.0f),    // c4
        angle::Vector4(-1.0f, -1.0f, 0.5f, 1.0f),  // v5
        angle::Vector4(0.0f, 1.0f, 0.0f, 1.0f),    // c5
        angle::Vector4(1.0f, -1.0f, 0.5f, 1.0f),   // v6
        angle::Vector4(0.0f, 1.0f, 0.0f, 1.0f),    // c6
        angle::Vector4(1.0f, 1.0f, 0.5f, 1.0f),    // v7
        angle::Vector4(0.0f, 1.0f, 0.0f, 1.0f),    // c7
    };

    GLBuffer vb;
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    const std::array<GLushort, 12> indicies{
        0, 1, 2, 0, 2, 3,  // quad0
        4, 5, 6, 4, 6, 7,  // quad1
    };

    GLBuffer ib;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies.data(), GL_STATIC_DRAW);

    constexpr char kVS[] = R"(
precision highp float;
attribute vec4 a_position;
attribute vec4 a_color;

varying vec4 v_color;

void main()
{
    gl_Position = a_position;
    v_color = a_color;
})";

    constexpr char kFS[] = R"(
precision highp float;
varying vec4 v_color;

void main()
{
    gl_FragColor = v_color;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint positionLocation = glGetAttribLocation(program, "a_position");
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, sizeof(angle::Vector4) * 2, 0);

    GLint colorLocation = glGetAttribLocation(program, "a_color");
    glEnableVertexAttribArray(colorLocation);
    glVertexAttribPointer(colorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(angle::Vector4) * 2,
                          reinterpret_cast<const void *>(sizeof(angle::Vector4)));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,
                   reinterpret_cast<const void *>(sizeof(GLshort) * 6));
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,
                   reinterpret_cast<const void *>(sizeof(GLshort) * 6));
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Covers drawing with a very large vertex range which overflows GLsizei. http://crbug.com/842028
TEST_P(RobustBufferAccessBehaviorTest, VeryLargeVertexCountWithDynamicVertexData)
{
    ANGLE_SKIP_TEST_IF(!initExtension());
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_OES_element_index_uint"));

    constexpr GLsizei kIndexCount           = 32;
    std::array<GLuint, kIndexCount> indices = {{}};
    for (GLsizei index = 0; index < kIndexCount; ++index)
    {
        indices[index] = ((std::numeric_limits<GLuint>::max() - 2) / kIndexCount) * index;
    }

    GLBuffer indexBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(),
                 GL_STATIC_DRAW);

    std::array<GLfloat, 256> vertexData = {{}};

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(),
                 GL_DYNAMIC_DRAW);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(program);

    GLint attribLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, attribLoc);

    glVertexAttribPointer(attribLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(attribLoc);
    ASSERT_GL_NO_ERROR();

    glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_INT, nullptr);

    // This may or may not generate an error, but it should not crash.
}

// Test that robust access works even if there's no data uploaded to the vertex buffer at all.
TEST_P(RobustBufferAccessBehaviorTest, NoBufferData)
{
    ANGLE_SKIP_TEST_IF(!initExtension());
    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    glUseProgram(program);

    glEnableVertexAttribArray(0);
    GLBuffer buf;
    glBindBuffer(GL_ARRAY_BUFFER, buf);

    glVertexAttribPointer(0, 1, GL_FLOAT, false, 0, nullptr);
    ASSERT_GL_NO_ERROR();

    std::array<GLubyte, 1u> indices = {0};
    glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_BYTE, indices.data());
    ASSERT_GL_NO_ERROR();
}

constexpr char kWebGLVS[] = R"(attribute vec2 position;
attribute vec4 aOne;
attribute vec4 aTwo;
varying vec4 v;
uniform vec2 comparison;

bool isRobust(vec4 value) {
    // The valid buffer range is filled with this value.
    if (value.xy == comparison)
        return true;
    // Checking the w value is a bit complex.
    return (value.xyz == vec3(0, 0, 0));
}

void main() {
    gl_Position = vec4(position, 0, 1);
    if (isRobust(aOne) && isRobust(aTwo)) {
        v = vec4(0, 1, 0, 1);
    } else {
        v = vec4(1, 0, 0, 1);
    }
})";

constexpr char kWebGLFS[] = R"(precision mediump float;
varying vec4 v;
void main() {
    gl_FragColor = v;
})";

// Test buffer with interleaved (3+2) float vectors. Adapted from WebGL test
// conformance/rendering/draw-arrays-out-of-bounds.html
TEST_P(RobustBufferAccessBehaviorTest, InterleavedAttributes)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    ANGLE_GL_PROGRAM(program, kWebGLVS, kWebGLFS);
    glUseProgram(program);

    constexpr GLint kPosLoc = 0;
    constexpr GLint kOneLoc = 1;
    constexpr GLint kTwoLoc = 2;

    ASSERT_EQ(kPosLoc, glGetAttribLocation(program, "position"));
    ASSERT_EQ(kOneLoc, glGetAttribLocation(program, "aOne"));
    ASSERT_EQ(kTwoLoc, glGetAttribLocation(program, "aTwo"));

    // Create a buffer of 200 valid sets of quad lists.
    constexpr size_t kNumQuads = 200;
    using QuadVerts            = std::array<Vector3, 6>;
    std::vector<QuadVerts> quadVerts(kNumQuads, GetQuadVertices());

    GLBuffer positionBuf;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuf);
    glBufferData(GL_ARRAY_BUFFER, kNumQuads * sizeof(QuadVerts), quadVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(kPosLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(kPosLoc);

    constexpr GLfloat kDefaultFloat = 0.2f;
    std::vector<Vector4> defaultFloats(kNumQuads * 2, Vector4(kDefaultFloat));

    GLBuffer vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // enough for 9 vertices, so 3 triangles
    glBufferData(GL_ARRAY_BUFFER, 9 * 5 * sizeof(GLfloat), defaultFloats.data(), GL_STATIC_DRAW);

    // bind first 3 elements, with a stride of 5 float elements
    glVertexAttribPointer(kOneLoc, 3, GL_FLOAT, GL_FALSE, 5 * 4, 0);
    // bind 2 elements, starting after the first 3; same stride of 5 float elements
    glVertexAttribPointer(kTwoLoc, 2, GL_FLOAT, GL_FALSE, 5 * 4,
                          reinterpret_cast<const GLvoid *>(3 * 4));

    glEnableVertexAttribArray(kOneLoc);
    glEnableVertexAttribArray(kTwoLoc);

    // set test uniform
    GLint uniLoc = glGetUniformLocation(program, "comparison");
    ASSERT_NE(-1, uniLoc);
    glUniform2f(uniLoc, kDefaultFloat, kDefaultFloat);

    // Draw out of bounds.
    glDrawArrays(GL_TRIANGLES, 0, 10000);
    GLenum err = glGetError();
    if (err == GL_NO_ERROR)
    {
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
    else
    {
        EXPECT_GLENUM_EQ(GL_INVALID_OPERATION, err);
    }

    glDrawArrays(GL_TRIANGLES, (kNumQuads - 1) * 6, 6);
    err = glGetError();
    if (err == GL_NO_ERROR)
    {
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
    else
    {
        EXPECT_GLENUM_EQ(GL_INVALID_OPERATION, err);
    }
}

// Tests redefining an empty buffer. Adapted from WebGL test
// conformance/rendering/draw-arrays-out-of-bounds.html
TEST_P(RobustBufferAccessBehaviorTest, EmptyBuffer)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    ANGLE_GL_PROGRAM(program, kWebGLVS, kWebGLFS);
    glUseProgram(program);

    constexpr GLint kPosLoc = 0;
    constexpr GLint kOneLoc = 1;
    constexpr GLint kTwoLoc = 2;

    ASSERT_EQ(kPosLoc, glGetAttribLocation(program, "position"));
    ASSERT_EQ(kOneLoc, glGetAttribLocation(program, "aOne"));
    ASSERT_EQ(kTwoLoc, glGetAttribLocation(program, "aTwo"));

    // Create a buffer of 200 valid sets of quad lists.
    constexpr size_t kNumQuads = 200;
    using QuadVerts            = std::array<Vector3, 6>;
    std::vector<QuadVerts> quadVerts(kNumQuads, GetQuadVertices());

    GLBuffer positionBuf;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuf);
    glBufferData(GL_ARRAY_BUFFER, kNumQuads * sizeof(QuadVerts), quadVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(kPosLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(kPosLoc);

    // set test uniform
    GLint uniLoc = glGetUniformLocation(program, "comparison");
    ASSERT_NE(-1, uniLoc);
    glUniform2f(uniLoc, 0, 0);

    // Define empty buffer.
    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(kOneLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(kOneLoc);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    GLenum err = glGetError();
    if (err == GL_NO_ERROR)
    {
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
    else
    {
        EXPECT_GLENUM_EQ(GL_INVALID_OPERATION, err);
    }

    // Redefine buffer with 3 float vectors.
    constexpr GLfloat kFloats[] = {0, 0.5, 0, -0.5, -0.5, 0, 0.5, -0.5, 0};
    glBufferData(GL_ARRAY_BUFFER, sizeof(kFloats), kFloats, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    ASSERT_GL_NO_ERROR();
}

// Tests robust buffer access with dynamic buffer usage.
TEST_P(RobustBufferAccessBehaviorTest, DynamicBuffer)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    ANGLE_GL_PROGRAM(program, kWebGLVS, kWebGLFS);
    glUseProgram(program);

    constexpr GLint kPosLoc = 0;
    constexpr GLint kOneLoc = 1;
    constexpr GLint kTwoLoc = 2;

    ASSERT_EQ(kPosLoc, glGetAttribLocation(program, "position"));
    ASSERT_EQ(kOneLoc, glGetAttribLocation(program, "aOne"));
    ASSERT_EQ(kTwoLoc, glGetAttribLocation(program, "aTwo"));

    // Create a buffer of 200 valid sets of quad lists.
    constexpr size_t kNumQuads = 200;
    using QuadVerts            = std::array<Vector3, 6>;
    std::vector<QuadVerts> quadVerts(kNumQuads, GetQuadVertices());

    GLBuffer positionBuf;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuf);
    glBufferData(GL_ARRAY_BUFFER, kNumQuads * sizeof(QuadVerts), quadVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(kPosLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(kPosLoc);

    constexpr GLfloat kDefaultFloat = 0.2f;
    std::vector<Vector4> defaultFloats(kNumQuads * 2, Vector4(kDefaultFloat));

    GLBuffer vbo;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // enough for 9 vertices, so 3 triangles
    glBufferData(GL_ARRAY_BUFFER, 9 * 5 * sizeof(GLfloat), defaultFloats.data(), GL_DYNAMIC_DRAW);

    // bind first 3 elements, with a stride of 5 float elements
    glVertexAttribPointer(kOneLoc, 3, GL_FLOAT, GL_FALSE, 5 * 4, 0);
    // bind 2 elements, starting after the first 3; same stride of 5 float elements
    glVertexAttribPointer(kTwoLoc, 2, GL_FLOAT, GL_FALSE, 5 * 4,
                          reinterpret_cast<const GLvoid *>(3 * 4));

    glEnableVertexAttribArray(kOneLoc);
    glEnableVertexAttribArray(kTwoLoc);

    // set test uniform
    GLint uniLoc = glGetUniformLocation(program, "comparison");
    ASSERT_NE(-1, uniLoc);
    glUniform2f(uniLoc, kDefaultFloat, kDefaultFloat);

    // Draw out of bounds.
    glDrawArrays(GL_TRIANGLES, 0, 10000);
    GLenum err = glGetError();
    if (err == GL_NO_ERROR)
    {
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
    else
    {
        EXPECT_GLENUM_EQ(GL_INVALID_OPERATION, err);
    }

    glDrawArrays(GL_TRIANGLES, (kNumQuads - 1) * 6, 6);
    err = glGetError();
    if (err == GL_NO_ERROR)
    {
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
    else
    {
        EXPECT_GLENUM_EQ(GL_INVALID_OPERATION, err);
    }
}

// Tests out of bounds read by divisor emulation due to a user-provided offset.
// Adapted from https://crbug.com/1285885.
TEST_P(RobustBufferAccessBehaviorTest, IndexOutOfBounds)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    constexpr char kVS[] = R"(precision highp float;
attribute vec4 a_position;
void main(void) {
   gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision highp float;
uniform sampler2D oTexture;
uniform float oColor[3];
void main(void) {
   gl_FragData[0] = texture2DProj(oTexture, vec3(0.1,0.1,0.1));
})";

    GLfloat singleFloat = 1.0f;

    GLBuffer buf;
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, 4, &singleFloat, GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "a_position");
    glLinkProgram(program);
    ASSERT_TRUE(CheckLinkStatusAndReturnProgram(program, true));

    glEnableVertexAttribArray(0);

    // Trying to exceed renderer->getMaxVertexAttribDivisor()
    GLuint constexpr kDivisor = 4096;
    glVertexAttribDivisor(0, kDivisor);

    size_t outOfBoundsOffset = 0x50000000;
    glVertexAttribPointer(0, 1, GL_FLOAT, false, 8, reinterpret_cast<void *>(outOfBoundsOffset));

    glUseProgram(program);

    glDrawArrays(GL_TRIANGLES, 0, 32);

    // No assertions, just checking for crashes.
}

// Similar to the test above but index is first within bounds then goes out of bounds.
TEST_P(RobustBufferAccessBehaviorTest, IndexGoingOutOfBounds)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    constexpr char kVS[] = R"(precision highp float;
attribute vec4 a_position;
void main(void) {
   gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision highp float;
uniform sampler2D oTexture;
uniform float oColor[3];
void main(void) {
   gl_FragData[0] = texture2DProj(oTexture, vec3(0.1,0.1,0.1));
})";

    GLBuffer buf;
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    std::array<GLfloat, 2> buffer = {{0.2f, 0.2f}};
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * buffer.size(), buffer.data(), GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "a_position");
    glLinkProgram(program);
    ASSERT_TRUE(CheckLinkStatusAndReturnProgram(program, true));

    glEnableVertexAttribArray(0);

    // Trying to exceed renderer->getMaxVertexAttribDivisor()
    GLuint constexpr kDivisor = 4096;
    glVertexAttribDivisor(0, kDivisor);

    // 6 bytes remaining in the buffer from offset so only a single vertex can be read
    glVertexAttribPointer(0, 1, GL_FLOAT, false, 8, reinterpret_cast<void *>(2));

    glUseProgram(program);

    // Each vertex is read `kDivisor` times so the last read goes out of bounds
    GLsizei instanceCount = kDivisor + 1;
    glDrawArraysInstanced(GL_TRIANGLES, 0, 32, instanceCount);

    // No assertions, just checking for crashes.
}

// Draw out-of-bounds beginning with the start offset passed in.
// Ensure that drawArrays flags either no error or INVALID_OPERATION. In the case of
// INVALID_OPERATION, no canvas pixels can be touched.  In the case of NO_ERROR, all written values
// must either be the zero vertex or a value in the vertex buffer.  See vsCheckOutOfBounds shader.
void DrawAndVerifyOutOfBoundsArrays(int first, int count)
{
    glClearColor(0.0, 0.0, 1.0, 1.0);  // Start with blue to indicate no pixels touched.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, first, count);
    GLenum error = glGetError();
    if (error == GL_INVALID_OPERATION)
    {
        // testPassed. drawArrays flagged INVALID_OPERATION, which is valid so long as all canvas
        // pixels were not touched.
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    }
    else
    {
        ASSERT_GL_NO_ERROR();
        // testPassed. drawArrays flagged NO_ERROR, which is valid so long as all canvas pixels are
        // green.
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
}

// Adapted from WebGL test
// conformance/rendering/out-of-bounds-array-buffers.html
// This test verifies that out-of-bounds array buffers behave according to spec.
TEST_P(RobustBufferAccessBehaviorTest, OutOfBoundsArrayBuffers)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    constexpr char vsCheckOutOfBounds[] =
        "precision mediump float;\n"
        "attribute vec2 position;\n"
        "attribute vec4 vecRandom;\n"
        "varying vec4 v_color;\n"
        "\n"
        "// Per the spec, each component can either contain existing contents\n"
        "// of the buffer or 0.\n"
        "bool testFloatComponent(float component) {\n"
        "   return (component == 0.2 || component == 0.0);\n"
        "}\n"
        ""  // The last component is additionally allowed to be 1.0.\n"
        "bool testLastFloatComponent(float component) {\n"
        "   return testFloatComponent(component) || component == 1.0;\n"
        "}\n"
        "\n"
        "void main() {\n"
        "   if (testFloatComponent(vecRandom.x) &&\n"
        "       testFloatComponent(vecRandom.y) &&\n"
        "       testFloatComponent(vecRandom.z) &&\n"
        "       testLastFloatComponent(vecRandom.w)) {\n"
        "           v_color = vec4(0.0, 1.0, 0.0, 1.0); // green -- We're good\n"
        "       } else {\n"
        "           v_color = vec4(1.0, 0.0, 0.0, 1.0); // red -- Unexpected value\n"
        "       }\n"
        "   gl_Position = vec4(position, 0.0, 1.0);\n"
        "}\n";

    constexpr char simpleVertexColorFragmentShader[] =
        "precision mediump float;\n"
        "varying vec4 v_color;\n"
        "void main() {\n"
        "    gl_FragColor = v_color;\n"
        "}";

    // Setup the verification program.
    ANGLE_GL_PROGRAM(program, vsCheckOutOfBounds, simpleVertexColorFragmentShader);
    glUseProgram(program);

    GLint kPosLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(kPosLoc, -1);
    GLint kRandomLoc = glGetAttribLocation(program, "vecRandom");
    ASSERT_NE(kRandomLoc, -1);

    // Create a buffer of 200 valid sets of quad lists.
    constexpr size_t numberOfQuads = 200;
    // Create a vertex buffer with 200 properly formed triangle quads. These quads will cover the
    // canvas texture such that every single pixel is touched by the fragment shader.
    GLBuffer glQuadBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, glQuadBuffer);
    std::array<float, numberOfQuads * 2 * 6> quadPositions;
    for (unsigned int i = 0; i < quadPositions.size(); i += 2 * 6)
    {
        quadPositions[i + 0]  = -1.0;  // upper left
        quadPositions[i + 1]  = 1.0;
        quadPositions[i + 2]  = 1.0;  // upper right
        quadPositions[i + 3]  = 1.0;
        quadPositions[i + 4]  = -1.0;  // lower left
        quadPositions[i + 5]  = -1.0;
        quadPositions[i + 6]  = 1.0;  // upper right
        quadPositions[i + 7]  = 1.0;
        quadPositions[i + 8]  = 1.0;  // lower right
        quadPositions[i + 9]  = -1.0;
        quadPositions[i + 10] = -1.0;  // lower left
        quadPositions[i + 11] = -1.0;
    }
    glBufferData(GL_ARRAY_BUFFER, quadPositions.size() * sizeof(float), quadPositions.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(kPosLoc);
    glVertexAttribPointer(kPosLoc, 2, GL_FLOAT, false, 0, 0);

    // Create a small vertex buffer with determined-ahead-of-time "random" values (0.2). This buffer
    // will be the one read past the end.
    GLBuffer glVertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, glVertexBuffer);
    std::array<float, 6> vertexData = {0.2, 0.2, 0.2, 0.2, 0.2, 0.2};
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(kRandomLoc);
    glVertexAttribPointer(kRandomLoc, 4, GL_FLOAT, false, 0, 0);

    // Test -- Draw off the end of the vertex buffer near the beginning of the out of bounds area.
    DrawAndVerifyOutOfBoundsArrays(/*first*/ 6, /*count*/ 6);

    // Test -- Draw off the end of the vertex buffer near the end of the out of bounds area.
    DrawAndVerifyOutOfBoundsArrays(/*first*/ (numberOfQuads - 1) * 6, /*count*/ 6);
}

// Regression test for glBufferData with slightly increased size. Implementation may decided to
// reuse the buffer storage if underline storage is big enough (due to alignment, implementation may
// allocate more storage than data size.) This tests ensure it works correctly when this reuse
// happens.
TEST_P(RobustBufferAccessBehaviorTest, BufferDataWithIncreasedSize)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());

    // Clear to red and draw one triangle on the bottom left with green. The right top half should
    // be red.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    std::array<float, 2 * 3> quadVertices = {-1, 1, -1, -1, 1, -1};
    constexpr size_t kBufferSize          = sizeof(quadVertices[0]) * quadVertices.size();
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, kBufferSize, quadVertices.data(), GL_STATIC_DRAW);
    glUseProgram(drawGreen);
    const GLint positionLocation = glGetAttribLocation(drawGreen, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);
    EXPECT_GL_NO_ERROR();

    // Clear to blue and call glBufferData with two triangles and draw the entire window with green.
    // Both bottom left and top right should be green.
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    std::array<float, 2 * 3 * 2> twoQuadVertices = {-1, 1, -1, -1, 1, -1, -1, 1, 1, -1, 1, 1};
    glBufferData(GL_ARRAY_BUFFER, kBufferSize * 2, twoQuadVertices.data(), GL_STATIC_DRAW);
    glUseProgram(drawGreen);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(1, 1, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green);
    EXPECT_GL_NO_ERROR();
}

// Similar to BufferDataWithIncreasedSize. But this time the buffer is bound to two VAOs. The change
// in the buffer should be picked up by both VAOs.
TEST_P(RobustBufferAccessBehaviorTest, BufferDataWithIncreasedSizeAndUseWithVAOs)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    ANGLE_GL_PROGRAM(drawGreen, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::Green());

    // Clear to red and draw one triangle with VAO1 on the bottom left with green. The right top
    // half should be red.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    std::array<float, 2 * 3> quadVertices = {-1, 1, -1, -1, 1, -1};
    constexpr size_t kBufferSize          = sizeof(quadVertices[0]) * quadVertices.size();
    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, kBufferSize, quadVertices.data(), GL_STATIC_DRAW);
    glUseProgram(drawGreen);
    const GLint positionLocation = glGetAttribLocation(drawGreen, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, positionLocation);
    GLVertexArray vao1;
    glBindVertexArray(vao1);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_PIXEL_COLOR_EQ(2, 2, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::red);
    EXPECT_GL_NO_ERROR();

    // Now use the same buffer on VAO2
    GLVertexArray vao2;
    glBindVertexArray(vao2);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_PIXEL_COLOR_EQ(2, 2, GLColor::green);
    // Clear to blue and call glBufferData with two triangles and draw the entire window with green.
    // Both bottom left and top right should be green.
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    std::array<float, 2 * 3 * 2> twoQuadVertices = {-1, 1, -1, -1, 1, -1, -1, 1, 1, -1, 1, 1};
    glBufferData(GL_ARRAY_BUFFER, kBufferSize * 2, twoQuadVertices.data(), GL_STATIC_DRAW);
    glUseProgram(drawGreen);
    glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_COLOR_EQ(2, 2, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green);
    EXPECT_GL_NO_ERROR();

    // Buffer's change should be piked by VAO1 as well. If not, then we should get validation error.
    glBindVertexArray(vao1);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_PIXEL_COLOR_EQ(2, 2, GLColor::green);
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green);
    EXPECT_GL_NO_ERROR();
}

// Prepare an element array buffer that indexes out-of-bounds beginning with the start index passed
// in. Ensure that drawElements flags either no error or INVALID_OPERATION. In the case of
// INVALID_OPERATION, no canvas pixels can be touched.  In the case of NO_ERROR, all written values
// must either be the zero vertex or a value in the vertex buffer.  See vsCheckOutOfBounds shader.
void DrawAndVerifyOutOfBoundsIndex(int startIndex)
{
    glClearColor(0.0, 0.0, 1.0, 1.0);  // Start with blue to indicate no pixels touched.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Create an element array buffer with a tri-strip that starts at startIndex and make
    // it the active element array buffer.
    GLBuffer glElementArrayBuffer;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glElementArrayBuffer);
    std::array<uint16_t, 4> quadIndices;
    for (unsigned int i = 0; i < quadIndices.size(); i++)
    {
        quadIndices[i] = startIndex + i;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, quadIndices.size() * sizeof(uint16_t), quadIndices.data(),
                 GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, /*offset*/ 0);
    GLenum error = glGetError();
    if (error == GL_INVALID_OPERATION)
    {
        // testPassed. drawElements flagged INVALID_OPERATION, which is valid so long as all canvas
        // pixels were not touched.
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue);
    }
    else
    {
        ASSERT_GL_NO_ERROR();
        // testPassed. drawElements flagged NO_ERROR, which is valid so long as all canvas pixels
        // are green.
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
}

// Adapted from WebGL test
// conformance/rendering/out-of-bounds-index-buffers.html
// This test verifies that out-of-bounds index buffers behave according to spec.
TEST_P(RobustBufferAccessBehaviorTest, OutOfBoundsIndexBuffers)
{
    ANGLE_SKIP_TEST_IF(!initExtension());

    constexpr char vsCheckOutOfBounds[] =
        "precision mediump float;\n"
        "attribute vec2 position;\n"
        "attribute vec4 vecRandom;\n"
        "varying vec4 v_color;\n"
        "\n"
        "// Per the spec, each component can either contain existing contents\n"
        "// of the buffer or 0.\n"
        "bool testFloatComponent(float component) {\n"
        "   return (component == 0.2 || component == 0.0);\n"
        "}\n"
        ""  // The last component is additionally allowed to be 1.0.\n"
        "bool testLastFloatComponent(float component) {\n"
        "   return testFloatComponent(component) || component == 1.0;\n"
        "}\n"
        "\n"
        "void main() {\n"
        "   if (testFloatComponent(vecRandom.x) &&\n"
        "       testFloatComponent(vecRandom.y) &&\n"
        "       testFloatComponent(vecRandom.z) &&\n"
        "       testLastFloatComponent(vecRandom.w)) {\n"
        "           v_color = vec4(0.0, 1.0, 0.0, 1.0); // green -- We're good\n"
        "       } else {\n"
        "           v_color = vec4(1.0, 0.0, 0.0, 1.0); // red -- Unexpected value\n"
        "       }\n"
        "   gl_Position = vec4(position, 0.0, 1.0);\n"
        "}\n";

    constexpr char simpleVertexColorFragmentShader[] =
        "precision mediump float;\n"
        "varying vec4 v_color;\n"
        "void main() {\n"
        "    gl_FragColor = v_color;\n"
        "}";

    // Setup the verification program.
    ANGLE_GL_PROGRAM(program, vsCheckOutOfBounds, simpleVertexColorFragmentShader);
    glUseProgram(program);

    GLint kPosLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(kPosLoc, -1);
    GLint kRandomLoc = glGetAttribLocation(program, "vecRandom");
    ASSERT_NE(kRandomLoc, -1);

    // Create a buffer of 200 valid sets of quad lists.
    constexpr size_t numberOfQuads = 200;
    // Create a vertex buffer with 200 properly formed tri-strip quads. These quads will cover the
    // canvas texture such that every single pixel is touched by the fragment shader.
    GLBuffer glQuadBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, glQuadBuffer);
    std::array<float, numberOfQuads * 2 * 4> quadPositions;
    for (unsigned int i = 0; i < quadPositions.size(); i += 2 * 4)
    {
        quadPositions[i + 0] = -1.0;  // upper left
        quadPositions[i + 1] = 1.0;
        quadPositions[i + 2] = 1.0;  // upper right
        quadPositions[i + 3] = 1.0;
        quadPositions[i + 4] = -1.0;  // lower left
        quadPositions[i + 5] = -1.0;
        quadPositions[i + 6] = 1.0;  // lower right
        quadPositions[i + 7] = -1.0;
    }
    glBufferData(GL_ARRAY_BUFFER, quadPositions.size() * sizeof(float), quadPositions.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(kPosLoc);
    glVertexAttribPointer(kPosLoc, 2, GL_FLOAT, false, 0, 0);

    // Create a small vertex buffer with determined-ahead-of-time "random" values (0.2). This buffer
    // will be the one indexed off the end.
    GLBuffer glVertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, glVertexBuffer);
    std::array<float, 4> vertexData = {0.2, 0.2, 0.2, 0.2};
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(kRandomLoc);
    glVertexAttribPointer(kRandomLoc, 4, GL_FLOAT, false, 0, 0);

    // Test -- Index off the end of the vertex buffer near the beginning of the out of bounds area.
    DrawAndVerifyOutOfBoundsIndex(/*StartIndex*/ 4);

    // Test -- Index off the end of the vertex buffer near the end of the out of bounds area.
    DrawAndVerifyOutOfBoundsIndex(/*StartIndex*/ numberOfQuads - 4);
}

ANGLE_INSTANTIATE_TEST(RobustBufferAccessBehaviorTest,
                       WithNoFixture(ES3_VULKAN()),
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES3_OPENGLES()),
                       WithNoFixture(ES3_D3D11()),
                       WithNoFixture(ES3_METAL()));

}  // namespace
