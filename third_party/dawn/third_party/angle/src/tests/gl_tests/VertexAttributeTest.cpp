//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "anglebase/numerics/safe_conversions.h"
#include "common/mathutil.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/random_utils.h"

using namespace angle;

namespace
{

GLsizei TypeStride(GLenum attribType)
{
    switch (attribType)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            return 1;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_HALF_FLOAT:
        case GL_HALF_FLOAT_OES:
            return 2;
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
        case GL_UNSIGNED_INT_10_10_10_2_OES:
        case GL_INT_10_10_10_2_OES:
            return 4;
        default:
            EXPECT_TRUE(false);
            return 0;
    }
}

template <typename T>
GLfloat Normalize(T value)
{
    static_assert(std::is_integral<T>::value, "Integer required.");
    if (std::is_signed<T>::value)
    {
        typedef typename std::make_unsigned<T>::type unsigned_type;
        return (2.0f * static_cast<GLfloat>(value) + 1.0f) /
               static_cast<GLfloat>(std::numeric_limits<unsigned_type>::max());
    }
    else
    {
        return static_cast<GLfloat>(value) / static_cast<GLfloat>(std::numeric_limits<T>::max());
    }
}

// Normalization for each channel of signed/unsigned 10_10_10_2 types
template <typename T>
GLfloat Normalize10(T value)
{
    static_assert(std::is_integral<T>::value, "Integer required.");
    GLfloat floatOutput;
    if (std::is_signed<T>::value)
    {
        const uint32_t signMask     = 0x200;       // 1 set at the 9th bit
        const uint32_t negativeMask = 0xFFFFFC00;  // All bits from 10 to 31 set to 1

        if (value & signMask)
        {
            int negativeNumber = value | negativeMask;
            floatOutput        = static_cast<GLfloat>(negativeNumber);
        }
        else
        {
            floatOutput = static_cast<GLfloat>(value);
        }

        const int32_t maxValue = 0x1FF;       // 1 set in bits 0 through 8
        const int32_t minValue = 0xFFFFFE01;  // Inverse of maxValue

        // A 10-bit two's complement number has the possibility of being minValue - 1 but
        // OpenGL's normalization rules dictate that it should be clamped to minValue in
        // this case.
        if (floatOutput < minValue)
            floatOutput = minValue;

        const int32_t halfRange = (maxValue - minValue) >> 1;
        floatOutput             = ((floatOutput - minValue) / halfRange) - 1.0f;
    }
    else
    {
        const GLfloat maxValue = 1023.0f;  // 1 set in bits 0 through 9
        floatOutput            = static_cast<GLfloat>(value) / maxValue;
    }
    return floatOutput;
}

template <typename T>
GLfloat Normalize2(T value)
{
    static_assert(std::is_integral<T>::value, "Integer required.");
    if (std::is_signed<T>::value)
    {
        GLfloat outputValue = static_cast<float>(value) / 1.0f;
        outputValue         = (outputValue >= -1.0f) ? (outputValue) : (-1.0f);
        return outputValue;
    }
    else
    {
        return static_cast<float>(value) / 3.0f;
    }
}

template <typename DestT, typename SrcT>
DestT Pack1010102(std::array<SrcT, 4> input)
{
    static_assert(std::is_integral<SrcT>::value, "Integer required.");
    static_assert(std::is_integral<DestT>::value, "Integer required.");
    static_assert(std::is_unsigned<SrcT>::value == std::is_unsigned<DestT>::value,
                  "Signedness should be equal.");
    DestT rOut, gOut, bOut, aOut;
    rOut = static_cast<DestT>(input[0]);
    gOut = static_cast<DestT>(input[1]);
    bOut = static_cast<DestT>(input[2]);
    aOut = static_cast<DestT>(input[3]);

    if (std::is_unsigned<SrcT>::value)
    {
        return rOut << 22 | gOut << 12 | bOut << 2 | aOut;
    }
    else
    {
        // Need to apply bit mask to account for sign extension
        return (0xFFC00000u & rOut << 22) | (0x003FF000u & gOut << 12) | (0x00000FFCu & bOut << 2) |
               (0x00000003u & aOut);
    }
}

class VertexAttributeTest : public ANGLETest<>
{
  protected:
    VertexAttributeTest() : mProgram(0), mTestAttrib(-1), mExpectedAttrib(-1), mBuffer(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    enum class Source
    {
        BUFFER,
        IMMEDIATE,
    };

    struct TestData final : private angle::NonCopyable
    {
        TestData(GLenum typeIn,
                 GLboolean normalizedIn,
                 Source sourceIn,
                 const void *inputDataIn,
                 const GLfloat *expectedDataIn)
            : type(typeIn),
              normalized(normalizedIn),
              bufferOffset(0),
              source(sourceIn),
              inputData(inputDataIn),
              expectedData(expectedDataIn),
              clearBeforeDraw(false)
        {}

        GLenum type;
        GLboolean normalized;
        size_t bufferOffset;
        Source source;

        const void *inputData;
        const GLfloat *expectedData;

        bool clearBeforeDraw;
    };

    void setupTest(const TestData &test, GLint typeSize)
    {
        if (mProgram == 0)
        {
            initBasicProgram();
        }

        if (test.source == Source::BUFFER)
        {
            GLsizei dataSize = kVertexCount * TypeStride(test.type);
            glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
            glBufferData(GL_ARRAY_BUFFER, dataSize, test.inputData, GL_STATIC_DRAW);
            glVertexAttribPointer(mTestAttrib, typeSize, test.type, test.normalized, 0,
                                  reinterpret_cast<void *>(test.bufferOffset));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            ASSERT_EQ(Source::IMMEDIATE, test.source);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribPointer(mTestAttrib, typeSize, test.type, test.normalized, 0,
                                  test.inputData);
        }

        glVertexAttribPointer(mExpectedAttrib, typeSize, GL_FLOAT, GL_FALSE, 0, test.expectedData);

        glEnableVertexAttribArray(mTestAttrib);
        glEnableVertexAttribArray(mExpectedAttrib);
    }

    void checkPixels()
    {
        GLint viewportSize[4];
        glGetIntegerv(GL_VIEWPORT, viewportSize);

        GLint midPixelX = (viewportSize[0] + viewportSize[2]) / 2;
        GLint midPixelY = (viewportSize[1] + viewportSize[3]) / 2;

        // We need to offset our checks from triangle edges to ensure we don't fall on a single tri
        // Avoid making assumptions of drawQuad with four checks to check the four possible tri
        // regions
        EXPECT_PIXEL_EQ((midPixelX + viewportSize[0]) / 2, midPixelY, 255, 255, 255, 255);
        EXPECT_PIXEL_EQ((midPixelX + viewportSize[2]) / 2, midPixelY, 255, 255, 255, 255);
        EXPECT_PIXEL_EQ(midPixelX, (midPixelY + viewportSize[1]) / 2, 255, 255, 255, 255);
        EXPECT_PIXEL_EQ(midPixelX, (midPixelY + viewportSize[3]) / 2, 255, 255, 255, 255);
    }

    void checkPixelsUnEqual()
    {
        GLint viewportSize[4];
        glGetIntegerv(GL_VIEWPORT, viewportSize);

        GLint midPixelX = (viewportSize[0] + viewportSize[2]) / 2;
        GLint midPixelY = (viewportSize[1] + viewportSize[3]) / 2;

        // We need to offset our checks from triangle edges to ensure we don't fall on a single tri
        // Avoid making assumptions of drawQuad with four checks to check the four possible tri
        // regions
        EXPECT_PIXEL_NE((midPixelX + viewportSize[0]) / 2, midPixelY, 255, 255, 255, 255);
        EXPECT_PIXEL_NE((midPixelX + viewportSize[2]) / 2, midPixelY, 255, 255, 255, 255);
        EXPECT_PIXEL_NE(midPixelX, (midPixelY + viewportSize[1]) / 2, 255, 255, 255, 255);
        EXPECT_PIXEL_NE(midPixelX, (midPixelY + viewportSize[3]) / 2, 255, 255, 255, 255);
    }

    void runTest(const TestData &test) { runTest(test, true); }

    void runTest(const TestData &test, bool checkPixelEqual)
    {
        // TODO(geofflang): Figure out why this is broken on AMD OpenGL
        ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

        for (GLint i = 0; i < 4; i++)
        {
            GLint typeSize = i + 1;
            setupTest(test, typeSize);

            if (test.clearBeforeDraw)
            {
                glClear(GL_COLOR_BUFFER_BIT);
            }

            drawQuad(mProgram, "position", 0.5f);

            glDisableVertexAttribArray(mTestAttrib);
            glDisableVertexAttribArray(mExpectedAttrib);

            if (checkPixelEqual)
            {
                checkPixels();
            }
            else
            {
                checkPixelsUnEqual();
            }
        }
    }

    void testSetUp() override
    {
        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        glGenBuffers(1, &mBuffer);
    }

    void testTearDown() override
    {
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mBuffer);
    }

    GLuint compileMultiAttribProgram(GLint attribCount)
    {
        std::stringstream shaderStream;

        shaderStream << "attribute mediump vec4 position;" << std::endl;
        for (GLint attribIndex = 0; attribIndex < attribCount; ++attribIndex)
        {
            shaderStream << "attribute float a" << attribIndex << ";" << std::endl;
        }
        shaderStream << "varying mediump float color;" << std::endl
                     << "void main() {" << std::endl
                     << "  gl_Position = position;" << std::endl
                     << "  color = 0.0;" << std::endl;
        for (GLint attribIndex = 0; attribIndex < attribCount; ++attribIndex)
        {
            shaderStream << "  color += a" << attribIndex << ";" << std::endl;
        }
        shaderStream << "}" << std::endl;

        constexpr char kFS[] =
            "varying mediump float color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(color, 0.0, 0.0, 1.0);\n"
            "}\n";

        return CompileProgram(shaderStream.str().c_str(), kFS);
    }

    void setupMultiAttribs(GLuint program, GLint attribCount, GLfloat value)
    {
        glUseProgram(program);
        for (GLint attribIndex = 0; attribIndex < attribCount; ++attribIndex)
        {
            std::stringstream attribStream;
            attribStream << "a" << attribIndex;
            GLint location = glGetAttribLocation(program, attribStream.str().c_str());
            ASSERT_NE(-1, location);
            glVertexAttrib1f(location, value);
            glDisableVertexAttribArray(location);
        }
    }

    void initBasicProgram()
    {
        constexpr char kVS[] =
            "attribute mediump vec4 position;\n"
            "attribute highp vec4 test;\n"
            "attribute highp vec4 expected;\n"
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = position;\n"
            "    vec4 threshold = max(abs(expected) * 0.01, 1.0 / 64.0);\n"
            "    color = vec4(lessThanEqual(abs(test - expected), threshold));\n"
            "}\n";

        constexpr char kFS[] =
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = color;\n"
            "}\n";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        mTestAttrib = glGetAttribLocation(mProgram, "test");
        ASSERT_NE(-1, mTestAttrib);
        mExpectedAttrib = glGetAttribLocation(mProgram, "expected");
        ASSERT_NE(-1, mExpectedAttrib);

        glUseProgram(mProgram);
    }

    static constexpr size_t kVertexCount = 24;

    static void InitTestData(std::array<GLfloat, kVertexCount> &inputData,
                             std::array<GLfloat, kVertexCount> &expectedData)
    {
        for (size_t count = 0; count < kVertexCount; ++count)
        {
            inputData[count]    = static_cast<GLfloat>(count);
            expectedData[count] = inputData[count];
        }
    }

    static void InitQuadVertexBuffer(GLBuffer *buffer)
    {
        auto quadVertices = GetQuadVertices();
        GLsizei quadVerticesSize =
            static_cast<GLsizei>(quadVertices.size() * sizeof(quadVertices[0]));

        glBindBuffer(GL_ARRAY_BUFFER, *buffer);
        glBufferData(GL_ARRAY_BUFFER, quadVerticesSize, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, quadVerticesSize, quadVertices.data());
    }

    static void InitQuadPlusOneVertexBuffer(GLBuffer *buffer)
    {
        auto quadVertices = GetQuadVertices();
        GLsizei quadVerticesSize =
            static_cast<GLsizei>(quadVertices.size() * sizeof(quadVertices[0]));

        glBindBuffer(GL_ARRAY_BUFFER, *buffer);
        glBufferData(GL_ARRAY_BUFFER, quadVerticesSize + sizeof(Vector3), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, quadVerticesSize, quadVertices.data());
        glBufferSubData(GL_ARRAY_BUFFER, quadVerticesSize, sizeof(Vector3), &quadVertices[0]);
    }

    GLuint mProgram;
    GLint mTestAttrib;
    GLint mExpectedAttrib;
    GLuint mBuffer;
};

TEST_P(VertexAttributeTest, UnsignedByteUnnormalized)
{
    std::array<GLubyte, kVertexCount> inputData = {
        {0, 1, 2, 3, 4, 5, 6, 7, 125, 126, 127, 128, 129, 250, 251, 252, 253, 254, 255}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = inputData[i];
    }

    TestData data(GL_UNSIGNED_BYTE, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                  expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, UnsignedByteNormalized)
{
    std::array<GLubyte, kVertexCount> inputData = {
        {0, 1, 2, 3, 4, 5, 6, 7, 125, 126, 127, 128, 129, 250, 251, 252, 253, 254, 255}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_UNSIGNED_BYTE, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                  expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, ByteUnnormalized)
{
    std::array<GLbyte, kVertexCount> inputData = {
        {0, 1, 2, 3, 4, -1, -2, -3, -4, 125, 126, 127, -128, -127, -126}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = inputData[i];
    }

    TestData data(GL_BYTE, GL_FALSE, Source::IMMEDIATE, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, ByteNormalized)
{
    std::array<GLbyte, kVertexCount> inputData = {
        {0, 1, 2, 3, 4, -1, -2, -3, -4, 125, 126, 127, -128, -127, -126}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_BYTE, GL_TRUE, Source::IMMEDIATE, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, UnsignedShortUnnormalized)
{
    std::array<GLushort, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, 32766, 32767, 32768, 65533, 65534, 65535}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = inputData[i];
    }

    TestData data(GL_UNSIGNED_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                  expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, UnsignedShortNormalized)
{
    std::array<GLushort, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, 32766, 32767, 32768, 65533, 65534, 65535}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_UNSIGNED_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                  expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, ShortUnnormalized)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = inputData[i];
    }

    TestData data(GL_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, ShortNormalized)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(), expectedData.data());
    runTest(data);
}

// Verify that vertex data is updated correctly when using a float/half-float client memory pointer.
TEST_P(VertexAttributeTest, HalfFloatClientMemoryPointer)
{
    std::array<GLhalf, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData = {
        {0.f, 1.5f, 2.3f, 3.2f, -1.8f, -2.2f, -3.9f, -4.f, 34.5f, 32.2f, -78.8f, -77.4f, -76.1f}};

    for (size_t i = 0; i < kVertexCount; i++)
    {
        inputData[i] = gl::float32ToFloat16(expectedData[i]);
    }

    // If the extension is enabled run the test on all contexts
    if (IsGLExtensionEnabled("GL_OES_vertex_half_float"))
    {
        TestData imediateData(GL_HALF_FLOAT_OES, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                              expectedData.data());
        runTest(imediateData);
    }
    // Otherwise run the test only if it is an ES3 context
    else if (getClientMajorVersion() >= 3)
    {
        TestData imediateData(GL_HALF_FLOAT, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                              expectedData.data());
        runTest(imediateData);
    }
}

// Verify that vertex data is updated correctly when using a float/half-float buffer.
TEST_P(VertexAttributeTest, HalfFloatBuffer)
{
    std::array<GLhalf, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData = {
        {0.f, 1.5f, 2.3f, 3.2f, -1.8f, -2.2f, -3.9f, -4.f, 34.5f, 32.2f, -78.8f, -77.4f, -76.1f}};

    for (size_t i = 0; i < kVertexCount; i++)
    {
        inputData[i] = gl::float32ToFloat16(expectedData[i]);
    }

    // If the extension is enabled run the test on all contexts
    if (IsGLExtensionEnabled("GL_OES_vertex_half_float"))
    {
        TestData bufferData(GL_HALF_FLOAT_OES, GL_FALSE, Source::BUFFER, inputData.data(),
                            expectedData.data());
        runTest(bufferData);
    }
    // Otherwise run the test only if it is an ES3 context
    else if (getClientMajorVersion() >= 3)
    {
        TestData bufferData(GL_HALF_FLOAT, GL_FALSE, Source::BUFFER, inputData.data(),
                            expectedData.data());
        runTest(bufferData);
    }
}

// Verify that using the same client memory pointer in different format won't mess up the draw.
TEST_P(VertexAttributeTest, UsingDifferentFormatAndSameClientMemoryPointer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
    }

    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    std::array<GLfloat, kVertexCount> normalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        inputData[i]              = -inputData[i];
        normalizedExpectedData[i] = Normalize(inputData[i]);
    }

    TestData normalizedData(GL_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                            normalizedExpectedData.data());
    runTest(normalizedData);
}

// Verify that vertex format is updated correctly when the client memory pointer is same.
TEST_P(VertexAttributeTest, NegativeUsingDifferentFormatAndSameClientMemoryPointer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
    }

    // Use unnormalized short as the format of the data in client memory pointer in the first draw.
    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    // Use normalized short as the format of the data in client memory pointer in the second draw,
    // but mExpectedAttrib is the same as the first draw.
    TestData normalizedData(GL_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                            unnormalizedExpectedData.data());
    runTest(normalizedData, false);
}

// Verify that using different vertex format and same buffer won't mess up the draw.
TEST_P(VertexAttributeTest, UsingDifferentFormatAndSameBuffer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    std::array<GLfloat, kVertexCount> normalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
        normalizedExpectedData[i]   = Normalize(inputData[i]);
    }

    // Use unnormalized short as the format of the data in mBuffer in the first draw.
    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::BUFFER, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    // Use normalized short as the format of the data in mBuffer in the second draw.
    TestData normalizedData(GL_SHORT, GL_TRUE, Source::BUFFER, inputData.data(),
                            normalizedExpectedData.data());
    runTest(normalizedData);
}

// Verify that vertex format is updated correctly when the buffer is same.
TEST_P(VertexAttributeTest, NegativeUsingDifferentFormatAndSameBuffer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
    }

    // Use unnormalized short as the format of the data in mBuffer in the first draw.
    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::BUFFER, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    // Use normalized short as the format of the data in mBuffer in the second draw, but
    // mExpectedAttrib is the same as the first draw.
    TestData normalizedData(GL_SHORT, GL_TRUE, Source::BUFFER, inputData.data(),
                            unnormalizedExpectedData.data());

    // The check should fail because the test data is changed while the expected data is the same.
    runTest(normalizedData, false);
}

// Verify that mixed using buffer and client memory pointer won't mess up the draw.
TEST_P(VertexAttributeTest, MixedUsingBufferAndClientMemoryPointer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    std::array<GLfloat, kVertexCount> normalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
        normalizedExpectedData[i]   = Normalize(inputData[i]);
    }

    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    TestData unnormalizedBufferData(GL_SHORT, GL_FALSE, Source::BUFFER, inputData.data(),
                                    unnormalizedExpectedData.data());
    runTest(unnormalizedBufferData);

    TestData normalizedData(GL_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                            normalizedExpectedData.data());
    runTest(normalizedData);
}

// Verify signed unnormalized INT_10_10_10_2 vertex type
TEST_P(VertexAttributeTest, SignedPacked1010102ExtensionUnnormalized)
{
    std::string extensionList(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
    ANGLE_SKIP_TEST_IF((extensionList.find("OES_vertex_type_10_10_10_2") == std::string::npos));

    // RGB channels are 10-bits, alpha is 2-bits
    std::array<std::array<GLshort, 4>, kVertexCount / 4> unpackedInput = {{{0, 1, 2, 0},
                                                                           {254, 255, 256, 1},
                                                                           {256, 255, 254, -2},
                                                                           {511, 510, 509, -1},
                                                                           {-512, -511, -500, -2},
                                                                           {-1, -2, -3, 1}}};

    std::array<GLint, kVertexCount> packedInput;
    std::array<GLfloat, kVertexCount> expectedTypeSize4;
    std::array<GLfloat, kVertexCount> expectedTypeSize3;

    for (size_t i = 0; i < kVertexCount / 4; i++)
    {
        packedInput[i] = Pack1010102<GLint, GLshort>(unpackedInput[i]);

        expectedTypeSize3[i * 3 + 0] = expectedTypeSize4[i * 4 + 0] = unpackedInput[i][0];
        expectedTypeSize3[i * 3 + 1] = expectedTypeSize4[i * 4 + 1] = unpackedInput[i][1];
        expectedTypeSize3[i * 3 + 2] = expectedTypeSize4[i * 4 + 2] = unpackedInput[i][2];

        // when the type size is 3, alpha will be 1.0f by GLES driver
        expectedTypeSize4[i * 4 + 3] = unpackedInput[i][3];
    }

    TestData data4(GL_INT_10_10_10_2_OES, GL_FALSE, Source::IMMEDIATE, packedInput.data(),
                   expectedTypeSize4.data());
    TestData bufferedData4(GL_INT_10_10_10_2_OES, GL_FALSE, Source::BUFFER, packedInput.data(),
                           expectedTypeSize4.data());
    TestData data3(GL_INT_10_10_10_2_OES, GL_FALSE, Source::IMMEDIATE, packedInput.data(),
                   expectedTypeSize3.data());
    TestData bufferedData3(GL_INT_10_10_10_2_OES, GL_FALSE, Source::BUFFER, packedInput.data(),
                           expectedTypeSize3.data());

    std::array<std::pair<const TestData &, GLint>, 4> dataSet = {
        {{data4, 4}, {bufferedData4, 4}, {data3, 3}, {bufferedData3, 3}}};

    for (auto data : dataSet)
    {
        setupTest(data.first, data.second);
        drawQuad(mProgram, "position", 0.5f);
        glDisableVertexAttribArray(mTestAttrib);
        glDisableVertexAttribArray(mExpectedAttrib);
        checkPixels();
    }
}

// Verify signed normalized INT_10_10_10_2 vertex type
TEST_P(VertexAttributeTest, SignedPacked1010102ExtensionNormalized)
{
    std::string extensionList(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
    ANGLE_SKIP_TEST_IF((extensionList.find("OES_vertex_type_10_10_10_2") == std::string::npos));

    // RGB channels are 10-bits, alpha is 2-bits
    std::array<std::array<GLshort, 4>, kVertexCount / 4> unpackedInput = {{{0, 1, 2, 0},
                                                                           {254, 255, 256, 1},
                                                                           {256, 255, 254, -2},
                                                                           {511, 510, 509, -1},
                                                                           {-512, -511, -500, -2},
                                                                           {-1, -2, -3, 1}}};
    std::array<GLint, kVertexCount> packedInput;
    std::array<GLfloat, kVertexCount> expectedNormalizedTypeSize4;
    std::array<GLfloat, kVertexCount> expectedNormalizedTypeSize3;

    for (size_t i = 0; i < kVertexCount / 4; i++)
    {
        packedInput[i] = Pack1010102<GLint, GLshort>(unpackedInput[i]);

        expectedNormalizedTypeSize3[i * 3 + 0] = expectedNormalizedTypeSize4[i * 4 + 0] =
            Normalize10<GLshort>(unpackedInput[i][0]);
        expectedNormalizedTypeSize3[i * 3 + 1] = expectedNormalizedTypeSize4[i * 4 + 1] =
            Normalize10<GLshort>(unpackedInput[i][1]);
        expectedNormalizedTypeSize3[i * 3 + 2] = expectedNormalizedTypeSize4[i * 4 + 2] =
            Normalize10<GLshort>(unpackedInput[i][2]);

        // when the type size is 3, alpha will be 1.0f by GLES driver
        expectedNormalizedTypeSize4[i * 4 + 3] = Normalize2<GLshort>(unpackedInput[i][3]);
    }

    TestData data4(GL_INT_10_10_10_2_OES, GL_TRUE, Source::IMMEDIATE, packedInput.data(),
                   expectedNormalizedTypeSize4.data());
    TestData bufferedData4(GL_INT_10_10_10_2_OES, GL_TRUE, Source::BUFFER, packedInput.data(),
                           expectedNormalizedTypeSize4.data());
    TestData data3(GL_INT_10_10_10_2_OES, GL_TRUE, Source::IMMEDIATE, packedInput.data(),
                   expectedNormalizedTypeSize3.data());
    TestData bufferedData3(GL_INT_10_10_10_2_OES, GL_TRUE, Source::BUFFER, packedInput.data(),
                           expectedNormalizedTypeSize3.data());

    std::array<std::pair<const TestData &, GLint>, 4> dataSet = {
        {{data4, 4}, {bufferedData4, 4}, {data3, 3}, {bufferedData3, 3}}};

    for (auto data : dataSet)
    {
        setupTest(data.first, data.second);
        drawQuad(mProgram, "position", 0.5f);
        glDisableVertexAttribArray(mTestAttrib);
        glDisableVertexAttribArray(mExpectedAttrib);
        checkPixels();
    }
}

// Verify unsigned unnormalized INT_10_10_10_2 vertex type
TEST_P(VertexAttributeTest, UnsignedPacked1010102ExtensionUnnormalized)
{
    std::string extensionList(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
    ANGLE_SKIP_TEST_IF((extensionList.find("OES_vertex_type_10_10_10_2") == std::string::npos));

    // RGB channels are 10-bits, alpha is 2-bits
    std::array<std::array<GLushort, 4>, kVertexCount / 4> unpackedInput = {{{0, 1, 2, 0},
                                                                            {511, 512, 513, 1},
                                                                            {1023, 1022, 1021, 3},
                                                                            {513, 512, 511, 2},
                                                                            {2, 1, 0, 3},
                                                                            {1023, 1022, 1022, 0}}};

    std::array<GLuint, kVertexCount> packedInput;
    std::array<GLfloat, kVertexCount> expectedTypeSize3;
    std::array<GLfloat, kVertexCount> expectedTypeSize4;

    for (size_t i = 0; i < kVertexCount / 4; i++)
    {
        packedInput[i] = Pack1010102<GLuint, GLushort>(unpackedInput[i]);

        expectedTypeSize3[i * 3 + 0] = expectedTypeSize4[i * 4 + 0] = unpackedInput[i][0];
        expectedTypeSize3[i * 3 + 1] = expectedTypeSize4[i * 4 + 1] = unpackedInput[i][1];
        expectedTypeSize3[i * 3 + 2] = expectedTypeSize4[i * 4 + 2] = unpackedInput[i][2];

        // when the type size is 3, alpha will be 1.0f by GLES driver
        expectedTypeSize4[i * 4 + 3] = unpackedInput[i][3];
    }

    TestData data4(GL_UNSIGNED_INT_10_10_10_2_OES, GL_FALSE, Source::IMMEDIATE, packedInput.data(),
                   expectedTypeSize4.data());
    TestData bufferedData4(GL_UNSIGNED_INT_10_10_10_2_OES, GL_FALSE, Source::BUFFER,
                           packedInput.data(), expectedTypeSize4.data());
    TestData data3(GL_UNSIGNED_INT_10_10_10_2_OES, GL_FALSE, Source::BUFFER, packedInput.data(),
                   expectedTypeSize3.data());
    TestData bufferedData3(GL_UNSIGNED_INT_10_10_10_2_OES, GL_FALSE, Source::BUFFER,
                           packedInput.data(), expectedTypeSize3.data());

    std::array<std::pair<const TestData &, GLint>, 4> dataSet = {
        {{data4, 4}, {bufferedData4, 4}, {data3, 3}, {bufferedData3, 3}}};

    for (auto data : dataSet)
    {
        setupTest(data.first, data.second);
        drawQuad(mProgram, "position", 0.5f);
        glDisableVertexAttribArray(mTestAttrib);
        glDisableVertexAttribArray(mExpectedAttrib);
        checkPixels();
    }
}

// Verify unsigned normalized INT_10_10_10_2 vertex type
TEST_P(VertexAttributeTest, UnsignedPacked1010102ExtensionNormalized)
{
    std::string extensionList(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
    ANGLE_SKIP_TEST_IF((extensionList.find("OES_vertex_type_10_10_10_2") == std::string::npos));

    // RGB channels are 10-bits, alpha is 2-bits
    std::array<std::array<GLushort, 4>, kVertexCount / 4> unpackedInput = {{{0, 1, 2, 0},
                                                                            {511, 512, 513, 1},
                                                                            {1023, 1022, 1021, 3},
                                                                            {513, 512, 511, 2},
                                                                            {2, 1, 0, 3},
                                                                            {1023, 1022, 1022, 0}}};

    std::array<GLuint, kVertexCount> packedInput;
    std::array<GLfloat, kVertexCount> expectedTypeSize4;
    std::array<GLfloat, kVertexCount> expectedTypeSize3;

    for (size_t i = 0; i < kVertexCount / 4; i++)
    {
        packedInput[i] = Pack1010102<GLuint, GLushort>(unpackedInput[i]);

        expectedTypeSize3[i * 3 + 0] = expectedTypeSize4[i * 4 + 0] =
            Normalize10<GLushort>(unpackedInput[i][0]);
        expectedTypeSize3[i * 3 + 1] = expectedTypeSize4[i * 4 + 1] =
            Normalize10<GLushort>(unpackedInput[i][1]);
        expectedTypeSize3[i * 3 + 2] = expectedTypeSize4[i * 4 + 2] =
            Normalize10<GLushort>(unpackedInput[i][2]);

        // when the type size is 3, alpha will be 1.0f by GLES driver
        expectedTypeSize4[i * 4 + 3] = Normalize2<GLushort>(unpackedInput[i][3]);
    }

    TestData data4(GL_UNSIGNED_INT_10_10_10_2_OES, GL_TRUE, Source::IMMEDIATE, packedInput.data(),
                   expectedTypeSize4.data());
    TestData bufferedData4(GL_UNSIGNED_INT_10_10_10_2_OES, GL_TRUE, Source::BUFFER,
                           packedInput.data(), expectedTypeSize4.data());
    TestData data3(GL_UNSIGNED_INT_10_10_10_2_OES, GL_TRUE, Source::IMMEDIATE, packedInput.data(),
                   expectedTypeSize3.data());
    TestData bufferedData3(GL_UNSIGNED_INT_10_10_10_2_OES, GL_TRUE, Source::BUFFER,
                           packedInput.data(), expectedTypeSize3.data());

    std::array<std::pair<const TestData &, GLint>, 4> dataSet = {
        {{data4, 4}, {bufferedData4, 4}, {data3, 3}, {bufferedData3, 3}}};

    for (auto data : dataSet)
    {
        setupTest(data.first, data.second);
        drawQuad(mProgram, "position", 0.5f);
        glDisableVertexAttribArray(mTestAttrib);
        glDisableVertexAttribArray(mExpectedAttrib);
        checkPixels();
    };
}

// Test that mixing array and current vertex attribute values works with the same matrix input
TEST_P(VertexAttributeTest, MixedMatrixSources)
{
    constexpr char kVS[] = R"(
attribute vec4 a_position;
attribute mat4 a_matrix;
varying vec4 v_color;
void main() {
    v_color = vec4(0.5, 0.25, 0.125, 0.0625) * a_matrix;
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(
precision mediump float;
varying vec4 v_color;
void main() {
    gl_FragColor = v_color;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "a_position");
    glBindAttribLocation(program, 1, "a_matrix");
    glLinkProgram(program);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    GLBuffer buffer;
    for (size_t i = 0; i < 4; ++i)
    {
        // Setup current attributes for all columns except one
        for (size_t col = 0; col < 4; ++col)
        {
            GLfloat v[4] = {0.0, 0.0, 0.0, 0.0};
            v[col]       = col == i ? 0.0 : 1.0;
            glVertexAttrib4fv(1 + col, v);
            glDisableVertexAttribArray(1 + col);
        }

        // Setup vertex array data for the i-th column
        GLfloat data[16]{};
        data[0 * 4 + i] = 1.0;
        data[1 * 4 + i] = 1.0;
        data[2 * 4 + i] = 1.0;
        data[3 * 4 + i] = 1.0;
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
        glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void *>(0));
        glEnableVertexAttribArray(1 + i);
        ASSERT_GL_NO_ERROR();

        glClear(GL_COLOR_BUFFER_BIT);
        drawIndexedQuad(program, "a_position", 0.0f, 1.0f, true);
        EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(128, 64, 32, 16), 1);
    }
}

// Test that interleaved layout works for drawing one vertex
TEST_P(VertexAttributeTest, InterleavedOneVertex)
{
    float pointSizeRange[2] = {};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);
    ANGLE_SKIP_TEST_IF(pointSizeRange[1] < 8);

    constexpr char kVS[] = R"(
attribute vec4 a_pos;
attribute vec4 a_col;
varying mediump vec4 v_col;
void main() {
    gl_PointSize = 8.0;
    gl_Position = a_pos;
    v_col = a_col;
})";
    constexpr char kFS[] = R"(
varying mediump vec4 v_col;
void main() {
    gl_FragColor = v_col;
})";
    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "a_pos");
    glBindAttribLocation(program, 1, "a_col");
    glUseProgram(program);

    GLBuffer buf;
    glBindBuffer(GL_ARRAY_BUFFER, buf);

    // One vertex, magenta
    const GLfloat data1[8] = {
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };

    // Two vertices, red and blue
    const GLfloat data2[16] = {
        -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        +0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    };

    // One vertex, green
    const GLfloat data3[8] = {
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    };

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void *>(0));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void *>(16));

    // The second attribute stride is reaching beyond the buffer's length.
    // It must not cause any errors as there only one vertex to draw.
    glBufferData(GL_ARRAY_BUFFER, 32, data1, GL_STATIC_DRAW);
    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(64, 64, GLColor::magenta);

    // Replace data and draw two vertices to ensure that stride has been applied correctly.
    glBufferData(GL_ARRAY_BUFFER, 64, data2, GL_STATIC_DRAW);
    glDrawArrays(GL_POINTS, 0, 2);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(32, 64, GLColor::red);
    EXPECT_PIXEL_COLOR_EQ(96, 64, GLColor::blue);

    // Replace data reducing the buffer size back to one vertex
    glBufferData(GL_ARRAY_BUFFER, 32, data3, GL_STATIC_DRAW);
    glDrawArrays(GL_POINTS, 0, 1);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(64, 64, GLColor::green);
}

class VertexAttributeTestES3 : public VertexAttributeTest
{
  protected:
    VertexAttributeTestES3() {}
};

TEST_P(VertexAttributeTestES3, IntUnnormalized)
{
    GLint lo                                  = std::numeric_limits<GLint>::min();
    GLint hi                                  = std::numeric_limits<GLint>::max();
    std::array<GLint, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, -1, hi, hi - 1, lo, lo + 1}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = static_cast<GLfloat>(inputData[i]);
    }

    TestData data(GL_INT, GL_FALSE, Source::BUFFER, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTestES3, IntNormalized)
{
    GLint lo                                  = std::numeric_limits<GLint>::min();
    GLint hi                                  = std::numeric_limits<GLint>::max();
    std::array<GLint, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, -1, hi, hi - 1, lo, lo + 1}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_INT, GL_TRUE, Source::BUFFER, inputData.data(), expectedData.data());
    runTest(data);
}

// Same as IntUnnormalized but with glClear() before running the test to force
// starting a render pass. This to verify that buffer format conversion within
// an active render pass works as expected in Metal back-end.
TEST_P(VertexAttributeTestES3, IntUnnormalizedWithClear)
{
    GLint lo                                  = std::numeric_limits<GLint>::min();
    GLint hi                                  = std::numeric_limits<GLint>::max();
    std::array<GLint, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, -1, hi, hi - 1, lo, lo + 1}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = static_cast<GLfloat>(inputData[i]);
    }

    TestData data(GL_INT, GL_FALSE, Source::BUFFER, inputData.data(), expectedData.data());
    data.clearBeforeDraw = true;

    runTest(data);
}

// Same as IntNormalized but with glClear() before running the test to force
// starting a render pass. This to verify that buffer format conversion within
// an active render pass works as expected in Metal back-end.
TEST_P(VertexAttributeTestES3, IntNormalizedWithClear)
{
    GLint lo                                  = std::numeric_limits<GLint>::min();
    GLint hi                                  = std::numeric_limits<GLint>::max();
    std::array<GLint, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, -1, hi, hi - 1, lo, lo + 1}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_INT, GL_TRUE, Source::BUFFER, inputData.data(), expectedData.data());
    data.clearBeforeDraw = true;

    runTest(data);
}

TEST_P(VertexAttributeTestES3, UnsignedIntUnnormalized)
{
    GLuint mid                                 = std::numeric_limits<GLuint>::max() >> 1;
    GLuint hi                                  = std::numeric_limits<GLuint>::max();
    std::array<GLuint, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, mid - 1, mid, mid + 1, hi - 2, hi - 1, hi}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = static_cast<GLfloat>(inputData[i]);
    }

    TestData data(GL_UNSIGNED_INT, GL_FALSE, Source::BUFFER, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTestES3, UnsignedIntNormalized)
{
    GLuint mid                                 = std::numeric_limits<GLuint>::max() >> 1;
    GLuint hi                                  = std::numeric_limits<GLuint>::max();
    std::array<GLuint, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, mid - 1, mid, mid + 1, hi - 2, hi - 1, hi}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_UNSIGNED_INT, GL_TRUE, Source::BUFFER, inputData.data(), expectedData.data());
    runTest(data);
}

// Same as UnsignedIntNormalized but with glClear() before running the test to force
// starting a render pass. This to verify that buffer format conversion within
// an active render pass works as expected in Metal back-end.
TEST_P(VertexAttributeTestES3, UnsignedIntNormalizedWithClear)
{
    GLuint mid                                 = std::numeric_limits<GLuint>::max() >> 1;
    GLuint hi                                  = std::numeric_limits<GLuint>::max();
    std::array<GLuint, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, mid - 1, mid, mid + 1, hi - 2, hi - 1, hi}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_UNSIGNED_INT, GL_TRUE, Source::BUFFER, inputData.data(), expectedData.data());
    data.clearBeforeDraw = true;
    runTest(data);
}

void SetupColorsForUnitQuad(GLint location, const GLColor32F &color, GLenum usage, GLBuffer *vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    std::vector<GLColor32F> vertices(6, color);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLColor32F), vertices.data(), usage);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

// Tests that rendering works as expected with VAOs.
TEST_P(VertexAttributeTestES3, VertexArrayObjectRendering)
{
    constexpr char kVertexShader[] =
        "attribute vec4 a_position;\n"
        "attribute vec4 a_color;\n"
        "varying vec4 v_color;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = a_position;\n"
        "   v_color = a_color;\n"
        "}";

    constexpr char kFragmentShader[] =
        "precision mediump float;\n"
        "varying vec4 v_color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = v_color;\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);

    GLint positionLoc = glGetAttribLocation(program, "a_position");
    ASSERT_NE(-1, positionLoc);
    GLint colorLoc = glGetAttribLocation(program, "a_color");
    ASSERT_NE(-1, colorLoc);

    GLVertexArray vaos[2];
    GLBuffer positionBuffer;
    GLBuffer colorBuffers[2];

    const auto &quadVertices = GetQuadVertices();

    glBindVertexArray(vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vector3), quadVertices.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    SetupColorsForUnitQuad(colorLoc, kFloatRed, GL_STREAM_DRAW, &colorBuffers[0]);

    glBindVertexArray(vaos[1]);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    SetupColorsForUnitQuad(colorLoc, kFloatGreen, GL_STATIC_DRAW, &colorBuffers[1]);

    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    for (int ii = 0; ii < 2; ++ii)
    {
        glBindVertexArray(vaos[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        glBindVertexArray(vaos[1]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }

    ASSERT_GL_NO_ERROR();
}

// Validate that we can support GL_MAX_ATTRIBS attribs
TEST_P(VertexAttributeTest, MaxAttribs)
{
    // TODO(jmadill): Figure out why we get this error on AMD/OpenGL.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    GLint maxAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
    ASSERT_GL_NO_ERROR();

    // Reserve one attrib for position
    GLint drawAttribs = maxAttribs - 1;

    GLuint program = compileMultiAttribProgram(drawAttribs);
    ASSERT_NE(0u, program);

    setupMultiAttribs(program, drawAttribs, 0.5f / static_cast<float>(drawAttribs));
    drawQuad(program, "position", 0.5f);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 0, 0, 255, 1);
}

// Validate that we cannot support GL_MAX_ATTRIBS+1 attribs
TEST_P(VertexAttributeTest, MaxAttribsPlusOne)
{
    // TODO(jmadill): Figure out why we get this error on AMD/ES2/OpenGL
    ANGLE_SKIP_TEST_IF(IsAMD() && GetParam() == ES2_OPENGL());

    GLint maxAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
    ASSERT_GL_NO_ERROR();

    // Exceed attrib count by one (counting position)
    GLint drawAttribs = maxAttribs;

    GLuint program = compileMultiAttribProgram(drawAttribs);
    ASSERT_EQ(0u, program);
}

// Simple test for when we use glBindAttribLocation
TEST_P(VertexAttributeTest, SimpleBindAttribLocation)
{
    // Re-use the multi-attrib program, binding attribute 0
    GLuint program = compileMultiAttribProgram(1);
    glBindAttribLocation(program, 2, "position");
    glBindAttribLocation(program, 3, "a0");
    glLinkProgram(program);

    // Setup and draw the quad
    setupMultiAttribs(program, 1, 0.5f);
    drawQuad(program, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 0, 0, 255, 1);
}

class VertexAttributeOORTest : public VertexAttributeTest
{
  public:
    VertexAttributeOORTest()
    {
        setWebGLCompatibilityEnabled(true);
        setRobustAccess(false);
    }
};

class RobustVertexAttributeTest : public VertexAttributeTest
{
  public:
    RobustVertexAttributeTest()
    {
        // mac GL and metal do not support robustness.
        if (!IsMac() && !IsIOS())
        {
            setRobustAccess(true);
        }
    }
};

// Verify that drawing with a large out-of-range offset generates INVALID_OPERATION.
// Requires WebGL compatibility with robust access behaviour disabled.
TEST_P(VertexAttributeOORTest, ANGLEDrawArraysBufferTooSmall)
{
    // Test skipped due to supporting GL_KHR_robust_buffer_access_behavior
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_KHR_robust_buffer_access_behavior"));

    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    TestData data(GL_FLOAT, GL_FALSE, Source::BUFFER, inputData.data(), expectedData.data());
    data.bufferOffset = kVertexCount * TypeStride(GL_FLOAT);

    setupTest(data, 1);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Verify that index draw with an out-of-range offset generates INVALID_OPERATION.
// Requires WebGL compatibility with robust access behaviour disabled.
TEST_P(VertexAttributeOORTest, ANGLEDrawElementsBufferTooSmall)
{
    // Test skipped due to supporting GL_KHR_robust_buffer_access_behavior
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_KHR_robust_buffer_access_behavior"));

    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    TestData data(GL_FLOAT, GL_FALSE, Source::BUFFER, inputData.data(), expectedData.data());
    data.bufferOffset = (kVertexCount - 3) * TypeStride(GL_FLOAT);

    setupTest(data, 1);
    drawIndexedQuad(mProgram, "position", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Verify that DrawArarys with an out-of-range offset generates INVALID_OPERATION.
// Requires WebGL compatibility with robust access behaviour disabled.
TEST_P(VertexAttributeOORTest, ANGLEDrawArraysOutOfBoundsCases)
{
    // Test skipped due to supporting GL_KHR_robust_buffer_access_behavior
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_KHR_robust_buffer_access_behavior"));

    initBasicProgram();

    GLfloat singleFloat = 1.0f;
    GLsizei dataSize    = TypeStride(GL_FLOAT);

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, &singleFloat, GL_STATIC_DRAW);
    glVertexAttribPointer(mTestAttrib, 2, GL_FLOAT, GL_FALSE, 8, 0);
    glEnableVertexAttribArray(mTestAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawIndexedQuad(mProgram, "position", 0.5f, 1.0f, true);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test that enabling a buffer in an unused attribute doesn't crash.  There should be an active
// attribute after that.
TEST_P(RobustVertexAttributeTest, BoundButUnusedBuffer)
{
    constexpr char kVS[] = R"(attribute vec2 offset;
void main()
{
    gl_Position = vec4(offset.xy, 0, 1);
    gl_PointSize = 1.0;
})";

    constexpr char kFS[] = R"(precision mediump float;
void main()
{
    gl_FragColor = vec4(1.0, 0, 0, 1.0);
})";

    const GLuint vs = CompileShader(GL_VERTEX_SHADER, kVS);
    const GLuint fs = CompileShader(GL_FRAGMENT_SHADER, kFS);

    GLuint program = glCreateProgram();
    glBindAttribLocation(program, 1, "offset");
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 100, nullptr, GL_STATIC_DRAW);

    // Enable an unused attribute that is within the range of active attributes (not beyond it)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, 0);

    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Destroy the buffer.  Regression test for a tracking bug where the buffer was used by
    // SwiftShader (even though location 1 is inactive), but not marked as used by ANGLE.
    buffer.reset();
}

// Verify that using a different start vertex doesn't mess up the draw.
TEST_P(VertexAttributeTest, DrawArraysWithBufferOffset)
{
    initBasicProgram();
    glUseProgram(mProgram);

    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    GLBuffer quadBuffer;
    InitQuadPlusOneVertexBuffer(&quadBuffer);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    GLsizei dataSize = kVertexCount * TypeStride(GL_FLOAT);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize + TypeStride(GL_FLOAT), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, inputData.data());
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Vertex draw with no start vertex offset (second argument is zero).
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Draw offset by one vertex.
    glDrawArrays(GL_TRIANGLES, 1, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Verify that using an unaligned offset doesn't mess up the draw.
TEST_P(VertexAttributeTest, DrawArraysWithUnalignedBufferOffset)
{
    initBasicProgram();
    glUseProgram(mProgram);

    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    GLBuffer quadBuffer;
    InitQuadPlusOneVertexBuffer(&quadBuffer);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Unaligned buffer offset (3)
    GLsizei dataSize = kVertexCount * TypeStride(GL_FLOAT) + 3;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 3, dataSize - 3, inputData.data());
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void *>(3));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Vertex draw with no start vertex offset (second argument is zero).
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Draw offset by one vertex.
    glDrawArrays(GL_TRIANGLES, 1, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Verify that using an unaligned offset & GL_SHORT vertex attribute doesn't mess up the draw.
// In Metal backend, GL_SHORTx3 is coverted to GL_SHORTx4 if offset is unaligned.
TEST_P(VertexAttributeTest, DrawArraysWithUnalignedShortBufferOffset)
{
    initBasicProgram();
    glUseProgram(mProgram);

    // input data is GL_SHORTx3 (6 bytes) but stride=8
    std::array<GLshort, 4 * kVertexCount> inputData;
    std::array<GLfloat, 3 * kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; ++i)
    {
        inputData[4 * i]     = 3 * i;
        inputData[4 * i + 1] = 3 * i + 1;
        inputData[4 * i + 2] = 3 * i + 2;

        expectedData[3 * i]     = 3 * i;
        expectedData[3 * i + 1] = 3 * i + 1;
        expectedData[3 * i + 2] = 3 * i + 2;
    }

    GLBuffer quadBuffer;
    InitQuadPlusOneVertexBuffer(&quadBuffer);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Unaligned buffer offset (8)
    GLsizei dataSize = 3 * kVertexCount * TypeStride(GL_SHORT) + 8;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 8, dataSize - 8, inputData.data());
    glVertexAttribPointer(mTestAttrib, 3, GL_SHORT, GL_FALSE, /* stride */ 8,
                          reinterpret_cast<void *>(8));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Vertex draw with no start vertex offset (second argument is zero).
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Draw offset by one vertex.
    glDrawArrays(GL_TRIANGLES, 1, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Verify that using a GL_FLOATx2 attribute with offset not divisible by 8 works.
TEST_P(VertexAttributeTest, DrawArraysWith2FloatAtOffsetNotDivisbleBy8)
{
    initBasicProgram();
    glUseProgram(mProgram);

    // input data is GL_FLOATx2 (8 bytes) and stride=36
    std::array<GLubyte, 36 * kVertexCount> inputData;
    std::array<GLfloat, 2 * kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; ++i)
    {
        expectedData[2 * i]     = 2 * i;
        expectedData[2 * i + 1] = 2 * i + 1;

        GLubyte *input = inputData.data() + 36 * i;
        memcpy(input, &expectedData[2 * i], sizeof(float));
        memcpy(input + sizeof(float), &expectedData[2 * i + 1], sizeof(float));
    }

    GLBuffer quadBuffer;
    InitQuadPlusOneVertexBuffer(&quadBuffer);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // offset is not float2 aligned (28)
    GLsizei dataSize = 36 * kVertexCount + 28;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 28, dataSize - 28, inputData.data());
    glVertexAttribPointer(mTestAttrib, 2, GL_FLOAT, GL_FALSE, /* stride */ 36,
                          reinterpret_cast<void *>(28));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 2, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Vertex draw with no start vertex offset (second argument is zero).
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Draw offset by one vertex.
    glDrawArrays(GL_TRIANGLES, 1, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Verify that using offset=1 for GL_BYTE vertex attribute doesn't mess up the draw.
// Depending on backend, offset=1 for GL_BYTE could be natively supported or not.
// In the latter case, a vertex data conversion will have to be performed.
TEST_P(VertexAttributeTest, DrawArraysWithByteAtOffset1)
{
    initBasicProgram();
    glUseProgram(mProgram);

    // input data is GL_BYTEx3 (3 bytes) but stride=4
    std::array<GLbyte, 4 * kVertexCount> inputData;
    std::array<GLfloat, 3 * kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; ++i)
    {
        inputData[4 * i]     = 3 * i;
        inputData[4 * i + 1] = 3 * i + 1;
        inputData[4 * i + 2] = 3 * i + 2;

        expectedData[3 * i]     = 3 * i;
        expectedData[3 * i + 1] = 3 * i + 1;
        expectedData[3 * i + 2] = 3 * i + 2;
    }

    GLBuffer quadBuffer;
    InitQuadPlusOneVertexBuffer(&quadBuffer);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Buffer offset (1)
    GLsizei dataSize = 4 * kVertexCount + 1;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 1, dataSize - 1, inputData.data());
    glVertexAttribPointer(mTestAttrib, 3, GL_BYTE, GL_FALSE, /* stride */ 4,
                          reinterpret_cast<void *>(1));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Vertex draw with no start vertex offset (second argument is zero).
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Draw offset by one vertex.
    glDrawArrays(GL_TRIANGLES, 1, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Verify that using an aligned but non-multiples of 4 offset vertex attribute doesn't mess up the
// draw.
TEST_P(VertexAttributeTest, DrawArraysWithShortBufferOffsetNotMultipleOf4)
{
    // http://anglebug.com/42263937
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsVulkan());

    initBasicProgram();
    glUseProgram(mProgram);

    // input data is GL_SHORTx3 (6 bytes) but stride=8
    std::array<GLshort, 4 * kVertexCount> inputData;
    std::array<GLfloat, 3 * kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; ++i)
    {
        inputData[4 * i]     = 3 * i;
        inputData[4 * i + 1] = 3 * i + 1;
        inputData[4 * i + 2] = 3 * i + 2;

        expectedData[3 * i]     = 3 * i;
        expectedData[3 * i + 1] = 3 * i + 1;
        expectedData[3 * i + 2] = 3 * i + 2;
    }

    GLBuffer quadBuffer;
    InitQuadPlusOneVertexBuffer(&quadBuffer);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // Aligned but not multiples of 4 buffer offset (18)
    GLsizei dataSize = 4 * kVertexCount * TypeStride(GL_SHORT) + 8;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 18, dataSize - 18, inputData.data());
    glVertexAttribPointer(mTestAttrib, 3, GL_SHORT, GL_FALSE, /* stride */ 8,
                          reinterpret_cast<void *>(18));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Vertex draw with no start vertex offset (second argument is zero).
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Draw offset by one vertex.
    glDrawArrays(GL_TRIANGLES, 1, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Verify that using both aligned and unaligned offsets doesn't mess up the draw.
TEST_P(VertexAttributeTest, DrawArraysWithAlignedAndUnalignedBufferOffset)
{
    initBasicProgram();
    glUseProgram(mProgram);

    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    GLBuffer quadBuffer;
    InitQuadPlusOneVertexBuffer(&quadBuffer);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    // ----------- Aligned buffer offset (4) -------------
    GLsizei dataSize = kVertexCount * TypeStride(GL_FLOAT) + 4;
    GLBuffer alignedBufer;
    glBindBuffer(GL_ARRAY_BUFFER, alignedBufer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 4, dataSize - 4, inputData.data());
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void *>(4));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Vertex draw with no start vertex offset (second argument is zero).
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Draw offset by one vertex.
    glDrawArrays(GL_TRIANGLES, 1, 6);
    checkPixels();

    // ----------- Unaligned buffer offset (3) -------------
    glClear(GL_COLOR_BUFFER_BIT);

    dataSize = kVertexCount * TypeStride(GL_FLOAT) + 3;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 3, dataSize - 3, inputData.data());
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void *>(3));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Vertex draw with no start vertex offset (second argument is zero).
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Draw offset by one vertex.
    glDrawArrays(GL_TRIANGLES, 1, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Verify that when we pass a client memory pointer to a disabled attribute the draw is still
// correct.
TEST_P(VertexAttributeTest, DrawArraysWithDisabledAttribute)
{
    initBasicProgram();

    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    GLBuffer buffer;
    InitQuadVertexBuffer(&buffer);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(inputData), inputData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // mProgram2 adds an attribute 'disabled' on the basis of mProgram.
    constexpr char testVertexShaderSource2[] =
        "attribute mediump vec4 position;\n"
        "attribute mediump vec4 test;\n"
        "attribute mediump vec4 expected;\n"
        "attribute mediump vec4 disabled;\n"
        "varying mediump vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = position;\n"
        "    vec4 threshold = max(abs(expected + disabled) * 0.005, 1.0 / 64.0);\n"
        "    color = vec4(lessThanEqual(abs(test - expected), threshold));\n"
        "}\n";

    constexpr char testFragmentShaderSource[] =
        "varying mediump vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = color;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, testVertexShaderSource2, testFragmentShaderSource);
    GLuint mProgram2 = program;

    ASSERT_EQ(positionLocation, glGetAttribLocation(mProgram2, "position"));
    ASSERT_EQ(mTestAttrib, glGetAttribLocation(mProgram2, "test"));
    ASSERT_EQ(mExpectedAttrib, glGetAttribLocation(mProgram2, "expected"));

    // Pass a client memory pointer to disabledAttribute and disable it.
    GLint disabledAttribute = glGetAttribLocation(mProgram2, "disabled");
    ASSERT_EQ(-1, glGetAttribLocation(mProgram, "disabled"));
    glVertexAttribPointer(disabledAttribute, 1, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glDisableVertexAttribArray(disabledAttribute);

    glUseProgram(mProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Now enable disabledAttribute which should be used in mProgram2.
    glEnableVertexAttribArray(disabledAttribute);
    glUseProgram(mProgram2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Test based on WebGL Test attribs/gl-disabled-vertex-attrib.html
TEST_P(VertexAttributeTest, DisabledAttribArrays)
{
    // Known failure on Retina MBP: http://crbug.com/635081
    ANGLE_SKIP_TEST_IF(IsMac() && IsNVIDIA());

    constexpr char kVS[] =
        "attribute vec4 a_position;\n"
        "attribute vec4 a_color;\n"
        "varying vec4 v_color;\n"
        "bool isCorrectColor(vec4 v) {\n"
        "    return v.x == 0.0 && v.y == 0.0 && v.z == 0.0 && v.w == 1.0;\n"
        "}"
        "void main() {\n"
        "    gl_Position = a_position;\n"
        "    v_color = isCorrectColor(a_color) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);\n"
        "}";

    constexpr char kFS[] =
        "varying mediump vec4 v_color;\n"
        "void main() {\n"
        "    gl_FragColor = v_color;\n"
        "}";

    GLint maxVertexAttribs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);

    for (GLint colorIndex = 0; colorIndex < maxVertexAttribs; ++colorIndex)
    {
        GLuint program = CompileProgram(kVS, kFS, [&](GLuint program) {
            glBindAttribLocation(program, colorIndex, "a_color");
        });
        ASSERT_NE(0u, program);

        drawQuad(program, "a_position", 0.5f);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green) << "color index " << colorIndex;

        glDeleteProgram(program);
    }
}

// Test that draw with offset larger than vertex attribute's stride can work
TEST_P(VertexAttributeTest, DrawWithLargeBufferOffset)
{
    constexpr size_t kBufferOffset    = 10000;
    constexpr size_t kQuadVertexCount = 4;

    std::array<GLbyte, kQuadVertexCount> validInputData = {{0, 1, 2, 3}};

    // 4 components
    std::array<GLbyte, 4 * kQuadVertexCount + kBufferOffset> inputData = {};

    std::array<GLfloat, 4 * kQuadVertexCount> expectedData;
    for (size_t i = 0; i < kQuadVertexCount; i++)
    {
        for (int j = 0; j < 4; ++j)
        {
            inputData[kBufferOffset + 4 * i + j] = validInputData[i];
            expectedData[4 * i + j]              = validInputData[i];
        }
    }

    initBasicProgram();

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, inputData.size(), inputData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(mTestAttrib, 4, GL_BYTE, GL_FALSE, 0,
                          reinterpret_cast<const void *>(kBufferOffset));
    glEnableVertexAttribArray(mTestAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glVertexAttribPointer(mExpectedAttrib, 4, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    drawIndexedQuad(mProgram, "position", 0.5f);

    checkPixels();
}

// Test that drawing with large vertex attribute pointer offset and less components than
// shader expects is OK
TEST_P(VertexAttributeTest, DrawWithLargeBufferOffsetAndLessComponents)
{
    // Shader expects vec4 but glVertexAttribPointer only provides 2 components
    constexpr char kVS[] = R"(attribute vec4 a_position;
attribute vec4 a_attrib;
varying vec4 v_attrib;
void main()
{
    v_attrib = a_attrib;
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(precision mediump float;
varying vec4 v_attrib;
void main()
{
    gl_FragColor = v_attrib;
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "a_position");
    glBindAttribLocation(program, 1, "a_attrib");
    glLinkProgram(program);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    constexpr size_t kBufferOffset = 4998;

    // Set up color data so yellow is drawn (only R, G components are provided)
    std::vector<GLushort> data(kBufferOffset + 12);
    for (int i = 0; i < 12; ++i)
    {
        data[kBufferOffset + i] = 0xffff;
    }

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLushort) * data.size(), data.data(), GL_STATIC_DRAW);
    // Provide only 2 components for the vec4 in the shader
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, 0,
                          reinterpret_cast<const void *>(sizeof(GLushort) * kBufferOffset));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(1);

    drawQuad(program, "a_position", 0.5f);
    // Verify yellow was drawn
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::yellow);
}

// Test that drawing with vertex attribute pointer with two non-overlapping BufferSubData calls
// works correctly, especially when vertex conversion is involved.
TEST_P(VertexAttributeTest, DrawArraysWithNonOverlapBufferSubData)
{
    constexpr size_t vertexCount = 6;
    initBasicProgram();
    glUseProgram(mProgram);

    // input data is GL_BYTEx3 (3 bytes) but stride=4
    std::array<GLbyte, 4 * vertexCount> inputData;
    std::array<GLfloat, 3 * vertexCount> expectedData;
    for (size_t i = 0; i < vertexCount; ++i)
    {
        inputData[4 * i]     = 3 * i;
        inputData[4 * i + 1] = 3 * i + 1;
        inputData[4 * i + 2] = 3 * i + 2;

        expectedData[3 * i]     = 3 * i;
        expectedData[3 * i + 1] = 3 * i + 1;
        expectedData[3 * i + 2] = 3 * i + 2;
    }

    GLBuffer quadBuffer;
    InitQuadVertexBuffer(&quadBuffer);
    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    GLsizei fullDataSize = 4 * vertexCount;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, fullDataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, fullDataSize, inputData.data());
    glVertexAttribPointer(mTestAttrib, 3, GL_BYTE, GL_FALSE, /* stride */ 4, nullptr);
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Draw quad and verify data
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    checkPixels();

    // Update data with two non-overlapping BufferSubData calls with different set of data
    for (size_t i = 0; i < vertexCount; ++i)
    {
        inputData[4 * i]     = 3 * (i + vertexCount);
        inputData[4 * i + 1] = 3 * (i + vertexCount) + 1;
        inputData[4 * i + 2] = 3 * (i + vertexCount) + 2;

        expectedData[3 * i]     = 3 * (i + vertexCount);
        expectedData[3 * i + 1] = 3 * (i + vertexCount) + 1;
        expectedData[3 * i + 2] = 3 * (i + vertexCount) + 2;
    }
    size_t halfDataSize = fullDataSize / 2;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, halfDataSize, inputData.data());
    glBufferSubData(GL_ARRAY_BUFFER, halfDataSize, fullDataSize - halfDataSize,
                    inputData.data() + halfDataSize);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0, expectedData.data());

    // Draw quad and verify data
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Test that drawing with vertex attribute pointer with two overlapping BufferSubData calls works
// correctly, especially when vertex conversion is involved.
TEST_P(VertexAttributeTest, DrawArraysWithOverlapBufferSubData)
{
    constexpr size_t vertexCount = 6;
    initBasicProgram();
    glUseProgram(mProgram);

    // input data is GL_BYTEx3 (3 bytes) but stride=4
    std::array<GLbyte, 4 * vertexCount> inputData;
    std::array<GLfloat, 3 * vertexCount> expectedData;
    for (size_t i = 0; i < vertexCount; ++i)
    {
        inputData[4 * i]     = 3 * i;
        inputData[4 * i + 1] = 3 * i + 1;
        inputData[4 * i + 2] = 3 * i + 2;

        expectedData[3 * i]     = 3 * i;
        expectedData[3 * i + 1] = 3 * i + 1;
        expectedData[3 * i + 2] = 3 * i + 2;
    }

    GLBuffer quadBuffer;
    InitQuadVertexBuffer(&quadBuffer);
    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    GLsizei fullDataSize = 4 * vertexCount;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, fullDataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, fullDataSize, inputData.data());
    glVertexAttribPointer(mTestAttrib, 3, GL_BYTE, GL_FALSE, /* stride */ 4, nullptr);
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Draw quad and verify data
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    checkPixels();

    // Update data with two overlapping BufferSubData calls with different set of data.
    size_t halfVertexCount = vertexCount / 2;
    size_t quadDataSize    = fullDataSize / 4;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    // First subData is for the first 3 quarter of buffer, but only first half contains valid data.
    for (size_t i = 0; i < vertexCount; ++i)
    {
        if (i < halfVertexCount)
        {
            inputData[4 * i]     = 3 * (i + vertexCount);
            inputData[4 * i + 1] = 3 * (i + vertexCount) + 1;
            inputData[4 * i + 2] = 3 * (i + vertexCount) + 2;
        }
        else
        {
            inputData[4 * i]     = 0;
            inputData[4 * i + 1] = 0;
            inputData[4 * i + 2] = 0;
        }

        expectedData[3 * i]     = 3 * (i + vertexCount);
        expectedData[3 * i + 1] = 3 * (i + vertexCount) + 1;
        expectedData[3 * i + 2] = 3 * (i + vertexCount) + 2;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * quadDataSize, inputData.data());
    // Second subData call is for the last half buffer, which overlaps with previous subData range.
    size_t halfDataSize = fullDataSize / 2;
    for (size_t i = halfVertexCount; i < vertexCount; ++i)
    {
        inputData[4 * i]     = 3 * (i + vertexCount);
        inputData[4 * i + 1] = 3 * (i + vertexCount) + 1;
        inputData[4 * i + 2] = 3 * (i + vertexCount) + 2;
    }
    glBufferSubData(GL_ARRAY_BUFFER, halfDataSize, fullDataSize - halfDataSize,
                    inputData.data() + halfDataSize);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0, expectedData.data());

    // Draw quad and verify data
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Test that drawing with vertex attribute pointer with different offset. The second offset is
// multiple stride after first offset.
TEST_P(VertexAttributeTest, DrawArraysWithLargerBindingOffset)
{
    constexpr size_t vertexCount = 6;
    initBasicProgram();
    glUseProgram(mProgram);

    // input data is GL_BYTEx3 (3 bytes) but stride=4
    std::array<GLbyte, 4 * vertexCount * 2> inputData;
    std::array<GLfloat, 3 * vertexCount * 2> expectedData;
    for (size_t i = 0; i < vertexCount * 2; ++i)
    {
        inputData[4 * i]     = 3 * i;
        inputData[4 * i + 1] = 3 * i + 1;
        inputData[4 * i + 2] = 3 * i + 2;

        expectedData[3 * i]     = 3 * i;
        expectedData[3 * i + 1] = 3 * i + 1;
        expectedData[3 * i + 2] = 3 * i + 2;
    }

    GLBuffer quadBuffer;
    InitQuadVertexBuffer(&quadBuffer);
    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    GLsizei fullDataSize = 4 * vertexCount * 2;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, fullDataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, fullDataSize, inputData.data());
    glVertexAttribPointer(mTestAttrib, 3, GL_BYTE, GL_FALSE, /* stride */ 4, /*offset*/ nullptr);
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0, /*data*/ expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Draw quad and verify data
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    checkPixels();

    // Now bind to a larger offset and then draw and verify
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glVertexAttribPointer(mTestAttrib, 3, GL_BYTE, GL_FALSE, /* stride */ 4,
                          reinterpret_cast<const void *>(4 * vertexCount));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0,
                          expectedData.data() + 3 * vertexCount);
    // Draw quad and verify data
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Test that drawing with vertex attribute pointer with different offset. The second offset is
// multiple stride before first offset.
TEST_P(VertexAttributeTest, DrawArraysWithSmallerBindingOffset)
{
    constexpr size_t vertexCount = 6;
    initBasicProgram();
    glUseProgram(mProgram);

    // input data is GL_BYTEx3 (3 bytes) but stride=4
    std::array<GLbyte, 4 * vertexCount * 2> inputData;
    std::array<GLfloat, 3 * vertexCount * 2> expectedData;
    for (size_t i = 0; i < vertexCount * 2; ++i)
    {
        inputData[4 * i]     = 3 * i;
        inputData[4 * i + 1] = 3 * i + 1;
        inputData[4 * i + 2] = 3 * i + 2;

        expectedData[3 * i]     = 3 * i;
        expectedData[3 * i + 1] = 3 * i + 1;
        expectedData[3 * i + 2] = 3 * i + 2;
    }

    GLBuffer quadBuffer;
    InitQuadVertexBuffer(&quadBuffer);
    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    GLsizei fullDataSize = 4 * vertexCount * 2;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, fullDataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, fullDataSize, inputData.data());
    glVertexAttribPointer(mTestAttrib, 3, GL_BYTE, GL_FALSE, /* stride */ 4,
                          reinterpret_cast<const void *>(4 * vertexCount));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0,
                          expectedData.data() + 3 * vertexCount);
    glEnableVertexAttribArray(mExpectedAttrib);

    // Draw quad and verify data
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    checkPixels();

    // Now bind to a smaller offset and draw and verify.
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glVertexAttribPointer(mTestAttrib, 3, GL_BYTE, GL_FALSE, /* stride */ 4, /*offset*/ nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 3, GL_FLOAT, GL_FALSE, 0, /*data*/ expectedData.data());
    // Draw quad and verify data
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Tests that we do not generate a SIGBUS error on arm when translating unaligned data.
// GL_RG32_SNORM_ANGLEX is used when using glVertexAttribPointer with certain parameters.
TEST_P(VertexAttributeTestES3, DrawWithUnalignedData)
{
    constexpr char kVS[] = R"(#version 300 es
precision highp float;
in highp vec4 a_position;
in highp vec2 a_ColorTest;
out highp vec2 v_colorTest;

void main() {
    v_colorTest = a_ColorTest;
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
in highp vec2 v_colorTest;
out vec4 fragColor;

void main() {
    // The input value is 0x01000000 / 0x7FFFFFFF
    if(abs(v_colorTest.x - 0.0078125) < 0.001) {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "a_position");
    glBindAttribLocation(program, 1, "a_ColorTest");
    glLinkProgram(program);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    constexpr size_t kDataSize = 12;

    // Initialize vertex attribute data with 1u32s, but shifted right by a variable number of bytes
    GLubyte colorTestData[(kDataSize + 1) * sizeof(GLuint)];

    for (size_t offset = 0; offset < sizeof(GLuint); offset++)
    {
        for (size_t dataIndex = 0; dataIndex < kDataSize * sizeof(GLuint); dataIndex++)
        {
            if (dataIndex % sizeof(GLuint) == sizeof(GLuint) - 1)
            {
                colorTestData[dataIndex + offset] = 1;
            }
            else
            {

                colorTestData[dataIndex + offset] = 0;
            }
        }

        GLubyte *offsetPtr = &colorTestData[offset];
        glVertexAttribPointer(1, 2, GL_INT, GL_TRUE, sizeof(GLuint), offsetPtr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glEnableVertexAttribArray(1);

        drawIndexedQuad(program, "a_position", 0.5f, 1.0f, false, true);

        // Verify green was drawn.
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green);
        ASSERT_GL_NO_ERROR();
    }
}

// Tests that rendering is fine if GL_ANGLE_relaxed_vertex_attribute_type is enabled
// and mismatched integer signedness between the program's attribute type and the
// attribute type specified by VertexAttribIPointer are used.
TEST_P(VertexAttributeTestES3, DrawWithRelaxedVertexAttributeType)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_relaxed_vertex_attribute_type"));

    constexpr char kVS[] = R"(#version 300 es
precision highp float;
in highp vec4 a_position;
in highp ivec4 a_ColorTest;
out highp vec4 v_colorTest;

void main() {
    v_colorTest = vec4(a_ColorTest);
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
in highp vec4 v_colorTest;
out vec4 fragColor;

void main() {
    if(v_colorTest.x > 0.5) {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "a_position");
    glBindAttribLocation(program, 1, "a_ColorTest");
    glLinkProgram(program);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    constexpr size_t kDataSize = 48;

    // Interleave test data with 0's.
    // This guards against a future code change that adjusts stride to 0

    // clang-format off
    constexpr GLuint kColorTestData[kDataSize] = {
        // Vertex attribute data      Unused data
        0u, 0u, 0u, 0u, /*red*/       0u, 0u, 0u, 0u,
        1u, 1u, 1u, 1u,               0u, 0u, 0u, 0u,
        1u, 1u, 1u, 1u,               0u, 0u, 0u, 0u,
        1u, 1u, 1u, 1u,               0u, 0u, 0u, 0u,
        1u, 1u, 1u, 1u,               0u, 0u, 0u, 0u,
        1u, 1u, 1u, 1u,               0u, 0u, 0u, 0u
    };
    // clang-format on

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * kDataSize, kColorTestData, GL_STATIC_DRAW);

    glVertexAttribIPointer(1, 4, GL_UNSIGNED_INT, 8 * sizeof(GLuint),
                           reinterpret_cast<const void *>(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(1);

    drawQuad(program, "a_position", 0.5f);

    // Verify green was drawn. If the stride isn't adjusted to 0 this corner will be green. If it is
    // adjusted to 0, the whole image will be red
    EXPECT_PIXEL_COLOR_EQ(getWindowWidth() - 1, getWindowHeight() - 1, GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test that ensures we do not send data for components not specified by glVertexAttribPointer when
// component types and sizes are mismatched
TEST_P(VertexAttributeTestES3, DrawWithMismatchedComponentCount)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_relaxed_vertex_attribute_type"));

    // To ensure the test results are valid when we don't send data for every component, the
    // shader's values must be defined by the backend.
    // Vulkan Spec 22.3. Vertex Attribute Divisor in Instanced Rendering
    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#_vertex_attribute_divisor_in_instanced_rendering
    // If the format does not include G, B, or A components, then those are filled with (0,0,1) as
    // needed (using either 1.0f or integer 1 based on the format) for attributes that are not
    // 64-bit data types.
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kVS[] = R"(#version 300 es
precision highp float;
in highp vec4 a_position;
in highp ivec2 a_ColorTest;
out highp vec2 v_colorTest;

void main() {
    v_colorTest = vec2(a_ColorTest);
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
in highp vec2 v_colorTest;
out vec4 fragColor;

void main() {
    if(v_colorTest.y < 0.5) {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "a_position");
    glBindAttribLocation(program, 1, "a_ColorTest");
    glLinkProgram(program);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    constexpr size_t kDataSize = 24;

    // Initialize vertex attribute data with 1s.
    GLuint kColorTestData[kDataSize];
    for (size_t dataIndex = 0; dataIndex < kDataSize; dataIndex++)
    {
        kColorTestData[dataIndex] = 1u;
    }

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * kDataSize, kColorTestData, GL_STATIC_DRAW);

    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 4 * sizeof(GLuint),
                           reinterpret_cast<const void *>(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(1);

    drawQuad(program, "a_position", 0.5f);

    // Verify green was drawn.
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::green);
    ASSERT_GL_NO_ERROR();
}

// Test that ensures we do not send data for components not specified by glVertexAttribPointer when
// component types and sizes are mismatched. Also guard against out of bound errors when atttribute
// locations are specified.
TEST_P(VertexAttributeTestES3, DrawWithMismatchedComponentCountLocationSpecified)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_relaxed_vertex_attribute_type"));

    // To ensure the test results are valid when we don't send data for every component, the
    // shader's values must be defined by the backend.
    // Vulkan Spec 22.3. Vertex Attribute Divisor in Instanced Rendering
    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#_vertex_attribute_divisor_in_instanced_rendering
    // If the format does not include G, B, or A components, then those are filled with (0,0,1) as
    // needed (using either 1.0f or integer 1 based on the format) for attributes that are not
    // 64-bit data types.
    ANGLE_SKIP_TEST_IF(!IsVulkan());

    constexpr char kVS[] = R"(#version 300 es
precision highp float;
layout(location = 2) in highp vec4 a_position;
layout(location = 0) in highp ivec2 a_ColorTest;
out highp vec2 v_colorTest;

void main() {
    v_colorTest = vec2(a_ColorTest);
    gl_Position = a_position;
})";

    constexpr char kFS[] = R"(#version 300 es
precision highp float;
in highp vec2 v_colorTest;
out vec4 fragColor;

void main() {
    if(v_colorTest.y < 0.5) {
        fragColor = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glLinkProgram(program);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    constexpr size_t kDataSize = 24;

    // Initialize vertex attribute data with 1s.
    GLuint kColorTestData[kDataSize];
    for (size_t dataIndex = 0; dataIndex < kDataSize; dataIndex++)
    {
        kColorTestData[dataIndex] = 1u;
    }

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * kDataSize, kColorTestData, GL_STATIC_DRAW);

    GLint colorLocation = glGetAttribLocation(program, "a_ColorTest");
    ASSERT_NE(colorLocation, -1);
    glVertexAttribIPointer(colorLocation, 1, GL_UNSIGNED_INT, 4 * sizeof(GLuint),
                           reinterpret_cast<const void *>(0));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(1);

    drawQuad(program, "a_position", 0.5f);

    // Verify green was drawn.
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::green);
    ASSERT_GL_NO_ERROR();
}

class VertexAttributeTestES31 : public VertexAttributeTestES3
{
  protected:
    VertexAttributeTestES31() {}

    void initTest()
    {
        initBasicProgram();
        glUseProgram(mProgram);

        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        auto quadVertices = GetQuadVertices();
        GLsizeiptr quadVerticesSize =
            static_cast<GLsizeiptr>(quadVertices.size() * sizeof(quadVertices[0]));
        glGenBuffers(1, &mQuadBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mQuadBuffer);
        glBufferData(GL_ARRAY_BUFFER, quadVerticesSize, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(mProgram, "position");
        ASSERT_NE(-1, positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(positionLocation);

        std::array<GLfloat, kVertexCount> expectedData;
        for (size_t count = 0; count < kVertexCount; ++count)
        {
            expectedData[count] = static_cast<GLfloat>(count);
        }

        const GLsizei kExpectedDataSize = kVertexCount * kFloatStride;
        glGenBuffers(1, &mExpectedBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mExpectedBuffer);
        glBufferData(GL_ARRAY_BUFFER, kExpectedDataSize, expectedData.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(mExpectedAttrib);
    }

    void testTearDown() override
    {
        VertexAttributeTestES3::testTearDown();

        glDeleteBuffers(1, &mQuadBuffer);
        glDeleteBuffers(1, &mExpectedBuffer);
        glDeleteVertexArrays(1, &mVAO);
    }

    void drawArraysWithStrideAndRelativeOffset(GLint stride, GLuint relativeOffset)
    {
        initTest();

        GLint floatStride          = std::max(stride / kFloatStride, 1);
        GLuint floatRelativeOffset = relativeOffset / kFloatStride;
        size_t floatCount = static_cast<size_t>(floatRelativeOffset) + kVertexCount * floatStride;
        GLsizeiptr inputSize = static_cast<GLsizeiptr>(floatCount) * kFloatStride;

        std::vector<GLfloat> inputData(floatCount);
        for (size_t count = 0; count < kVertexCount; ++count)
        {
            inputData[floatRelativeOffset + count * floatStride] = static_cast<GLfloat>(count);
        }

        // Ensure inputSize, inputStride and inputOffset are multiples of TypeStride(GL_FLOAT).
        GLsizei inputStride            = floatStride * kFloatStride;
        GLsizeiptr inputRelativeOffset = floatRelativeOffset * kFloatStride;
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, inputSize, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, inputSize, inputData.data());
        glVertexAttribFormat(mTestAttrib, 1, GL_FLOAT, GL_FALSE,
                             base::checked_cast<GLuint>(inputRelativeOffset));
        glBindVertexBuffer(mTestAttrib, mBuffer, 0, inputStride);
        glEnableVertexAttribArray(mTestAttrib);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        checkPixels();

        EXPECT_GL_NO_ERROR();
    }

    void initOnlyUpdateBindingTest(GLint bindingToUpdate)
    {
        initTest();

        constexpr GLuint kTestFloatOffset1                               = kVertexCount;
        std::array<GLfloat, kTestFloatOffset1 + kVertexCount> inputData1 = {};
        for (size_t count = 0; count < kVertexCount; ++count)
        {
            GLfloat value                         = static_cast<GLfloat>(count);
            inputData1[kTestFloatOffset1 + count] = value;
        }

        GLBuffer testBuffer1;
        glBindBuffer(GL_ARRAY_BUFFER, testBuffer1);
        glBufferData(GL_ARRAY_BUFFER, inputData1.size() * kFloatStride, inputData1.data(),
                     GL_STATIC_DRAW);

        ASSERT_NE(bindingToUpdate, mTestAttrib);
        ASSERT_NE(bindingToUpdate, mExpectedAttrib);

        // Set mTestAttrib using the binding bindingToUpdate.
        glVertexAttribFormat(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0);
        glBindVertexBuffer(bindingToUpdate, testBuffer1, kTestFloatOffset1 * kFloatStride,
                           kFloatStride);
        glVertexAttribBinding(mTestAttrib, bindingToUpdate);
        glEnableVertexAttribArray(mTestAttrib);

        // In the first draw the current VAO states are set to driver.
        glDrawArrays(GL_TRIANGLES, 0, 6);
        checkPixels();
        EXPECT_GL_NO_ERROR();

        // We need the second draw to ensure all VAO dirty bits are reset.
        // e.g. On D3D11 back-ends, Buffer11::resize is called in the first draw, where the related
        // binding is set to dirty again.
        glDrawArrays(GL_TRIANGLES, 0, 6);
        checkPixels();
        EXPECT_GL_NO_ERROR();
    }

    std::string makeMismatchingSignsTestVS(uint32_t attribCount, uint16_t signedMask);
    std::string makeMismatchingSignsTestFS(uint32_t attribCount);
    uint16_t setupVertexAttribPointersForMismatchSignsTest(uint16_t currentSignedMask,
                                                           uint16_t toggleMask);

    GLuint mVAO            = 0;
    GLuint mExpectedBuffer = 0;
    GLuint mQuadBuffer     = 0;

    const GLsizei kFloatStride = TypeStride(GL_FLOAT);

    // Set the maximum value for stride and relativeOffset in case they are too large.
    const GLint MAX_STRIDE_FOR_TEST          = 4095;
    const GLint MAX_RELATIVE_OFFSET_FOR_TEST = 4095;
};

// Verify that MAX_VERTEX_ATTRIB_STRIDE is no less than the minimum required value (2048) in ES3.1.
TEST_P(VertexAttributeTestES31, MaxVertexAttribStride)
{
    GLint maxStride;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &maxStride);
    ASSERT_GL_NO_ERROR();

    EXPECT_GE(maxStride, 2048);
}

// Verify that GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET is no less than the minimum required value
// (2047) in ES3.1.
TEST_P(VertexAttributeTestES31, MaxVertexAttribRelativeOffset)
{
    GLint maxRelativeOffset;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, &maxRelativeOffset);
    ASSERT_GL_NO_ERROR();

    EXPECT_GE(maxRelativeOffset, 2047);
}

// Verify using MAX_VERTEX_ATTRIB_STRIDE as stride doesn't mess up the draw.
// Use default value if the value of MAX_VERTEX_ATTRIB_STRIDE is too large for this test.
TEST_P(VertexAttributeTestES31, DrawArraysWithLargeStride)
{
    GLint maxStride;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &maxStride);
    ASSERT_GL_NO_ERROR();

    GLint largeStride = std::min(maxStride, MAX_STRIDE_FOR_TEST);
    drawArraysWithStrideAndRelativeOffset(largeStride, 0);
}

// Verify using MAX_VERTEX_ATTRIB_RELATIVE_OFFSET as relativeOffset doesn't mess up the draw.
// Use default value if the value of MAX_VERTEX_ATTRIB_RELATIVE_OFFSSET is too large for this test.
TEST_P(VertexAttributeTestES31, DrawArraysWithLargeRelativeOffset)
{
    GLint maxRelativeOffset;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, &maxRelativeOffset);
    ASSERT_GL_NO_ERROR();

    GLint largeRelativeOffset = std::min(maxRelativeOffset, MAX_RELATIVE_OFFSET_FOR_TEST);
    drawArraysWithStrideAndRelativeOffset(0, largeRelativeOffset);
}

// Test that vertex array object works correctly when render pipeline and compute pipeline are
// crossly executed.
TEST_P(VertexAttributeTestES31, MixedComputeAndRenderPipelines)
{
    constexpr char kComputeShader[] =
        R"(#version 310 es
layout(local_size_x=1) in;
void main()
{
})";
    ANGLE_GL_COMPUTE_PROGRAM(computeProgram, kComputeShader);

    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    glClearColor(0, 0, 0, 0);

    constexpr char kVertexShader[] =
        R"(#version 310 es
precision mediump float;
layout(location = 0) in vec4 position;
layout(location = 2) in vec2 aOffset;
layout(location = 3) in vec4 aColor;
out vec4 vColor;
void main() {
    vColor = aColor;
    gl_Position = position + vec4(aOffset, 0.0, 0.0);
})";

    constexpr char kFragmentShader[] =
        R"(#version 310 es
precision mediump float;
in vec4 vColor;
out vec4  color;
void main() {
    color = vColor;
})";

    ANGLE_GL_PROGRAM(renderProgram, kVertexShader, kFragmentShader);

    constexpr char kVertexShader1[] =
        R"(#version 310 es
precision mediump float;
layout(location = 1) in vec4 position;
layout(location = 2) in vec2 aOffset;
layout(location = 3) in vec4 aColor;
out vec4 vColor;
void main() {
    vColor = aColor;
    gl_Position = position + vec4(aOffset, 0.0, 0.0);
})";

    ANGLE_GL_PROGRAM(renderProgram1, kVertexShader1, kFragmentShader);

    std::array<GLfloat, 8> offsets = {
        -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0,
    };
    GLBuffer offsetBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, offsetBuffer);
    glBufferData(GL_ARRAY_BUFFER, offsets.size() * sizeof(GLfloat), offsets.data(), GL_STATIC_DRAW);

    std::array<GLfloat, 16> colors0 = {
        1.0, 0.0, 0.0, 1.0,  // Red
        0.0, 1.0, 0.0, 1.0,  // Green
        0.0, 0.0, 1.0, 1.0,  // Blue
        1.0, 1.0, 0.0, 1.0,  // Yellow
    };
    std::array<GLfloat, 16> colors1 = {
        1.0, 1.0, 0.0, 1.0,  // Yellow
        0.0, 0.0, 1.0, 1.0,  // Blue
        0.0, 1.0, 0.0, 1.0,  // Green
        1.0, 0.0, 0.0, 1.0,  // Red
    };
    GLBuffer colorBuffers[2];
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, colors0.size() * sizeof(GLfloat), colors0.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, colors1.size() * sizeof(GLfloat), colors1.data(), GL_STATIC_DRAW);

    std::array<GLfloat, 16> positions = {1.0, 1.0, -1.0, 1.0,  -1.0, -1.0,
                                         1.0, 1.0, -1.0, -1.0, 1.0,  -1.0};
    GLBuffer positionBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(),
                 GL_STATIC_DRAW);

    const int kInstanceCount = 4;
    GLVertexArray vao[2];
    for (size_t i = 0u; i < 2u; ++i)
    {
        glBindVertexArray(vao[i]);

        glBindBuffer(GL_ARRAY_BUFFER, offsetBuffer);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
        glVertexAttribDivisor(2, 1);

        glBindBuffer(GL_ARRAY_BUFFER, colorBuffers[i]);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, false, 0, 0);
        glVertexAttribDivisor(3, 1);

        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 2, GL_FLOAT, false, 0, 0);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < 3; i++)
    {
        glUseProgram(renderProgram);
        glBindVertexArray(vao[0]);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, kInstanceCount);

        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() / 2, GLColor::red) << i;
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::green) << i;
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::blue) << i;
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, 0, GLColor::yellow) << i;

        glBindVertexArray(vao[1]);
        glUseProgram(computeProgram);
        glDispatchCompute(1, 1, 1);

        glUseProgram(renderProgram1);
        glBindVertexArray(vao[1]);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, kInstanceCount);

        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, getWindowHeight() / 2, GLColor::yellow) << i;
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::blue) << i;
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green) << i;
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, 0, GLColor::red) << i;
    }
    ASSERT_GL_NO_ERROR();
}

TEST_P(VertexAttributeTestES31, UseComputeShaderToUpdateVertexBuffer)
{
    initTest();
    constexpr char kComputeShader[] =
        R"(#version 310 es
layout(local_size_x=24) in;
layout(std430, binding = 0) buffer buf {
    uint outData[24];
};
void main()
{
    outData[gl_LocalInvocationIndex] = gl_LocalInvocationIndex;
})";

    ANGLE_GL_COMPUTE_PROGRAM(computeProgram, kComputeShader);
    glUseProgram(mProgram);

    GLuint mid                                 = std::numeric_limits<GLuint>::max() >> 1;
    GLuint hi                                  = std::numeric_limits<GLuint>::max();
    std::array<GLuint, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, mid - 1, mid, mid + 1, hi - 2, hi - 1, hi}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    // Normalized unsigned int attribute will be classified as translated static attribute.
    TestData data(GL_UNSIGNED_INT, GL_TRUE, Source::BUFFER, inputData.data(), expectedData.data());
    GLint typeSize   = 4;
    GLsizei dataSize = kVertexCount * TypeStride(data.type);
    GLBuffer testBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, testBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data.inputData, GL_STATIC_DRAW);
    glVertexAttribPointer(mTestAttrib, typeSize, data.type, data.normalized, 0,
                          reinterpret_cast<void *>(data.bufferOffset));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, mExpectedBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data.expectedData, GL_STATIC_DRAW);
    glVertexAttribPointer(mExpectedAttrib, typeSize, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Draw twice to make sure that all static attributes dirty bits are synced.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Modify the testBuffer using a raw buffer
    glUseProgram(computeProgram);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, testBuffer);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    // Draw again to verify that testBuffer has been changed.
    glUseProgram(mProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();
    checkPixelsUnEqual();
}

TEST_P(VertexAttributeTestES31, UsePpoComputeShaderToUpdateVertexBuffer)
{
    // PPOs are only supported in the Vulkan backend
    ANGLE_SKIP_TEST_IF(!isVulkanRenderer());

    initTest();
    constexpr char kComputeShader[] =
        R"(#version 310 es
layout(local_size_x=24) in;
layout(std430, binding = 0) buffer buf {
    uint outData[24];
};
void main()
{
    outData[gl_LocalInvocationIndex] = gl_LocalInvocationIndex;
})";

    glUseProgram(mProgram);

    GLuint mid                                 = std::numeric_limits<GLuint>::max() >> 1;
    GLuint hi                                  = std::numeric_limits<GLuint>::max();
    std::array<GLuint, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, mid - 1, mid, mid + 1, hi - 2, hi - 1, hi}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    // Normalized unsigned int attribute will be classified as translated static attribute.
    TestData data(GL_UNSIGNED_INT, GL_TRUE, Source::BUFFER, inputData.data(), expectedData.data());
    GLint typeSize   = 4;
    GLsizei dataSize = kVertexCount * TypeStride(data.type);
    GLBuffer testBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, testBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data.inputData, GL_STATIC_DRAW);
    glVertexAttribPointer(mTestAttrib, typeSize, data.type, data.normalized, 0,
                          reinterpret_cast<void *>(data.bufferOffset));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, mExpectedBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data.expectedData, GL_STATIC_DRAW);
    glVertexAttribPointer(mExpectedAttrib, typeSize, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Draw twice to make sure that all static attributes dirty bits are synced.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Modify the testBuffer using a raw buffer
    GLProgramPipeline pipeline;
    ANGLE_GL_COMPUTE_PROGRAM(computeProgram, kComputeShader);
    glProgramParameteri(computeProgram, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glUseProgramStages(pipeline, GL_COMPUTE_SHADER_BIT, computeProgram);
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(pipeline);
    EXPECT_GL_NO_ERROR();
    glUseProgram(0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, testBuffer);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    // Draw again to verify that testBuffer has been changed.
    glUseProgram(mProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();
    checkPixelsUnEqual();
}

TEST_P(VertexAttributeTestES31, UseComputeShaderToUpdateVertexBufferSamePpo)
{
    // PPOs are only supported in the Vulkan backend
    ANGLE_SKIP_TEST_IF(!isVulkanRenderer());

    initTest();
    constexpr char kComputeShader[] =
        R"(#version 310 es
layout(local_size_x=24) in;
layout(std430, binding = 0) buffer buf {
    uint outData[24];
};
void main()
{
    outData[gl_LocalInvocationIndex] = gl_LocalInvocationIndex;
})";

    // Mark the program separable and re-link it so it can be bound to the PPO.
    glProgramParameteri(mProgram, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glLinkProgram(mProgram);
    mProgram = CheckLinkStatusAndReturnProgram(mProgram, true);

    GLProgramPipeline pipeline;
    EXPECT_GL_NO_ERROR();
    glBindProgramPipeline(pipeline);
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT, mProgram);
    EXPECT_GL_NO_ERROR();
    glUseProgram(0);

    GLuint mid                                 = std::numeric_limits<GLuint>::max() >> 1;
    GLuint hi                                  = std::numeric_limits<GLuint>::max();
    std::array<GLuint, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, mid - 1, mid, mid + 1, hi - 2, hi - 1, hi}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    // Normalized unsigned int attribute will be classified as translated static attribute.
    TestData data(GL_UNSIGNED_INT, GL_TRUE, Source::BUFFER, inputData.data(), expectedData.data());
    GLint typeSize   = 4;
    GLsizei dataSize = kVertexCount * TypeStride(data.type);
    GLBuffer testBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, testBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data.inputData, GL_STATIC_DRAW);
    glVertexAttribPointer(mTestAttrib, typeSize, data.type, data.normalized, 0,
                          reinterpret_cast<void *>(data.bufferOffset));
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, mExpectedBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data.expectedData, GL_STATIC_DRAW);
    glVertexAttribPointer(mExpectedAttrib, typeSize, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Draw twice to make sure that all static attributes dirty bits are synced.
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Modify the testBuffer using a raw buffer
    ANGLE_GL_COMPUTE_PROGRAM(computeProgram, kComputeShader);
    glProgramParameteri(computeProgram, GL_PROGRAM_SEPARABLE, GL_TRUE);
    glUseProgramStages(pipeline, GL_COMPUTE_SHADER_BIT, computeProgram);
    EXPECT_GL_NO_ERROR();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, testBuffer);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    // Draw again to verify that testBuffer has been changed.
    glUseProgram(mProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_GL_NO_ERROR();
    checkPixelsUnEqual();
}

// Verify that using VertexAttribBinding after VertexAttribPointer won't mess up the draw.
TEST_P(VertexAttributeTestES31, ChangeAttribBindingAfterVertexAttribPointer)
{
    initTest();

    constexpr GLint kInputStride = 2;
    constexpr GLint kFloatOffset = 10;
    std::array<GLfloat, kVertexCount + kFloatOffset> inputData1;
    std::array<GLfloat, kVertexCount * kInputStride> inputData2;
    for (size_t count = 0; count < kVertexCount; ++count)
    {
        inputData1[kFloatOffset + count] = static_cast<GLfloat>(count);
        inputData2[count * kInputStride] = static_cast<GLfloat>(count);
    }

    GLBuffer mBuffer1;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer1);
    glBufferData(GL_ARRAY_BUFFER, inputData1.size() * kFloatStride, inputData1.data(),
                 GL_STATIC_DRAW);
    // Update the format indexed mTestAttrib and the binding indexed mTestAttrib by
    // VertexAttribPointer.
    const GLintptr kOffset = static_cast<GLintptr>(kFloatStride * kFloatOffset);
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0,
                          reinterpret_cast<const GLvoid *>(kOffset));
    glEnableVertexAttribArray(mTestAttrib);

    constexpr GLint kTestBinding = 10;
    ASSERT_NE(mTestAttrib, kTestBinding);

    GLBuffer mBuffer2;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer2);
    glBufferData(GL_ARRAY_BUFFER, inputData2.size() * kFloatStride, inputData2.data(),
                 GL_STATIC_DRAW);
    glBindVertexBuffer(kTestBinding, mBuffer2, 0, kFloatStride * kInputStride);

    // The attribute indexed mTestAttrib is using the binding indexed kTestBinding in the first
    // draw.
    glVertexAttribBinding(mTestAttrib, kTestBinding);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();
    EXPECT_GL_NO_ERROR();

    // The attribute indexed mTestAttrib is using the binding indexed mTestAttrib which should be
    // set after the call VertexAttribPointer before the first draw.
    glVertexAttribBinding(mTestAttrib, mTestAttrib);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();
    EXPECT_GL_NO_ERROR();
}

// Verify that using VertexAttribFormat after VertexAttribPointer won't mess up the draw.
TEST_P(VertexAttributeTestES31, ChangeAttribFormatAfterVertexAttribPointer)
{
    initTest();

    constexpr GLuint kFloatOffset = 10;
    std::array<GLfloat, kVertexCount + kFloatOffset> inputData;
    for (size_t count = 0; count < kVertexCount; ++count)
    {
        inputData[kFloatOffset + count] = static_cast<GLfloat>(count);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, inputData.size() * kFloatStride, inputData.data(),
                 GL_STATIC_DRAW);

    // Call VertexAttribPointer on mTestAttrib. Now the relativeOffset of mTestAttrib should be 0.
    const GLuint kOffset = static_cast<GLuint>(kFloatStride * kFloatOffset);
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mTestAttrib);

    // Call VertexAttribFormat on mTestAttrib to modify the relativeOffset to kOffset.
    glVertexAttribFormat(mTestAttrib, 1, GL_FLOAT, GL_FALSE, kOffset);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();
    EXPECT_GL_NO_ERROR();
}

// Verify that only updating a binding without updating the bound format won't mess up this draw.
TEST_P(VertexAttributeTestES31, OnlyUpdateBindingByBindVertexBuffer)
{
    // Default binding index for test
    constexpr GLint kTestBinding = 10;
    initOnlyUpdateBindingTest(kTestBinding);

    constexpr GLuint kTestFloatOffset2                               = kVertexCount * 2;
    std::array<GLfloat, kVertexCount> expectedData2                  = {};
    std::array<GLfloat, kTestFloatOffset2 + kVertexCount> inputData2 = {};
    for (size_t count = 0; count < kVertexCount; ++count)
    {
        GLfloat value2                        = static_cast<GLfloat>(count) * 2;
        expectedData2[count]                  = value2;
        inputData2[count + kTestFloatOffset2] = value2;
    }

    // Set another set of data for mExpectedAttrib.
    GLBuffer expectedBuffer2;
    glBindBuffer(GL_ARRAY_BUFFER, expectedBuffer2);
    glBufferData(GL_ARRAY_BUFFER, expectedData2.size() * kFloatStride, expectedData2.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLBuffer testBuffer2;
    glBindBuffer(GL_ARRAY_BUFFER, testBuffer2);
    glBufferData(GL_ARRAY_BUFFER, inputData2.size() * kFloatStride, inputData2.data(),
                 GL_STATIC_DRAW);

    // Only update the binding kTestBinding in the second draw by BindVertexBuffer.
    glBindVertexBuffer(kTestBinding, testBuffer2, kTestFloatOffset2 * kFloatStride, kFloatStride);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();
    EXPECT_GL_NO_ERROR();
}

// Verify that only updating a binding without updating the bound format won't mess up this draw.
TEST_P(VertexAttributeTestES31, OnlyUpdateBindingByVertexAttribPointer)
{
    // Default binding index for test
    constexpr GLint kTestBinding = 10;
    initOnlyUpdateBindingTest(kTestBinding);

    constexpr GLuint kTestFloatOffset2                               = kVertexCount * 3;
    std::array<GLfloat, kVertexCount> expectedData2                  = {};
    std::array<GLfloat, kTestFloatOffset2 + kVertexCount> inputData2 = {};
    for (size_t count = 0; count < kVertexCount; ++count)
    {
        GLfloat value2                        = static_cast<GLfloat>(count) * 3;
        expectedData2[count]                  = value2;
        inputData2[count + kTestFloatOffset2] = value2;
    }

    // Set another set of data for mExpectedAttrib.
    GLBuffer expectedBuffer2;
    glBindBuffer(GL_ARRAY_BUFFER, expectedBuffer2);
    glBufferData(GL_ARRAY_BUFFER, expectedData2.size() * kFloatStride, expectedData2.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLBuffer testBuffer2;
    glBindBuffer(GL_ARRAY_BUFFER, testBuffer2);
    glBufferData(GL_ARRAY_BUFFER, inputData2.size() * kFloatStride, inputData2.data(),
                 GL_STATIC_DRAW);

    // Only update the binding kTestBinding in the second draw by VertexAttribPointer.
    glVertexAttribPointer(
        kTestBinding, 1, GL_FLOAT, GL_FALSE, 0,
        reinterpret_cast<const void *>(static_cast<uintptr_t>(kTestFloatOffset2 * kFloatStride)));

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();
    EXPECT_GL_NO_ERROR();
}

class VertexAttributeCachingTest : public VertexAttributeTest
{
  protected:
    VertexAttributeCachingTest() {}

    void testSetUp() override;

    template <typename DestT>
    static std::vector<GLfloat> GetExpectedData(const std::vector<GLubyte> &srcData,
                                                GLenum attribType,
                                                GLboolean normalized);

    void initDoubleAttribProgram()
    {
        constexpr char kVS[] =
            "attribute mediump vec4 position;\n"
            "attribute mediump vec4 test;\n"
            "attribute mediump vec4 expected;\n"
            "attribute mediump vec4 test2;\n"
            "attribute mediump vec4 expected2;\n"
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = position;\n"
            "    vec4 threshold = max(abs(expected) * 0.01, 1.0 / 64.0);\n"
            "    color = vec4(lessThanEqual(abs(test - expected), threshold));\n"
            "    vec4 threshold2 = max(abs(expected2) * 0.01, 1.0 / 64.0);\n"
            "    color += vec4(lessThanEqual(abs(test2 - expected2), threshold2));\n"
            "}\n";

        constexpr char kFS[] =
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = color;\n"
            "}\n";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);

        mTestAttrib = glGetAttribLocation(mProgram, "test");
        ASSERT_NE(-1, mTestAttrib);
        mExpectedAttrib = glGetAttribLocation(mProgram, "expected");
        ASSERT_NE(-1, mExpectedAttrib);

        glUseProgram(mProgram);
    }

    struct AttribData
    {
        AttribData(GLenum typeIn, GLint sizeIn, GLboolean normalizedIn, GLsizei strideIn);

        GLenum type;
        GLint size;
        GLboolean normalized;
        GLsizei stride;
    };

    std::vector<AttribData> mTestData;
    std::map<GLenum, std::vector<GLfloat>> mExpectedData;
    std::map<GLenum, std::vector<GLfloat>> mNormExpectedData;
};

VertexAttributeCachingTest::AttribData::AttribData(GLenum typeIn,
                                                   GLint sizeIn,
                                                   GLboolean normalizedIn,
                                                   GLsizei strideIn)
    : type(typeIn), size(sizeIn), normalized(normalizedIn), stride(strideIn)
{}

// static
template <typename DestT>
std::vector<GLfloat> VertexAttributeCachingTest::GetExpectedData(
    const std::vector<GLubyte> &srcData,
    GLenum attribType,
    GLboolean normalized)
{
    std::vector<GLfloat> expectedData;

    const DestT *typedSrcPtr = reinterpret_cast<const DestT *>(srcData.data());
    size_t iterations        = srcData.size() / TypeStride(attribType);

    if (normalized)
    {
        for (size_t index = 0; index < iterations; ++index)
        {
            expectedData.push_back(Normalize(typedSrcPtr[index]));
        }
    }
    else
    {
        for (size_t index = 0; index < iterations; ++index)
        {
            expectedData.push_back(static_cast<GLfloat>(typedSrcPtr[index]));
        }
    }

    return expectedData;
}

void VertexAttributeCachingTest::testSetUp()
{
    VertexAttributeTest::testSetUp();

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);

    std::vector<GLubyte> srcData;
    for (size_t count = 0; count < 4; ++count)
    {
        for (GLubyte i = 0; i < std::numeric_limits<GLubyte>::max(); ++i)
        {
            srcData.push_back(i);
        }
    }

    glBufferData(GL_ARRAY_BUFFER, srcData.size(), srcData.data(), GL_STATIC_DRAW);

    GLint viewportSize[4];
    glGetIntegerv(GL_VIEWPORT, viewportSize);

    std::vector<GLenum> attribTypes;
    attribTypes.push_back(GL_BYTE);
    attribTypes.push_back(GL_UNSIGNED_BYTE);
    attribTypes.push_back(GL_SHORT);
    attribTypes.push_back(GL_UNSIGNED_SHORT);

    if (getClientMajorVersion() >= 3)
    {
        attribTypes.push_back(GL_INT);
        attribTypes.push_back(GL_UNSIGNED_INT);
    }

    constexpr GLint kMaxSize     = 4;
    constexpr GLsizei kMaxStride = 4;

    for (GLenum attribType : attribTypes)
    {
        for (GLint attribSize = 1; attribSize <= kMaxSize; ++attribSize)
        {
            for (GLsizei stride = 1; stride <= kMaxStride; ++stride)
            {
                mTestData.push_back(AttribData(attribType, attribSize, GL_FALSE, stride));
                if (attribType != GL_FLOAT)
                {
                    mTestData.push_back(AttribData(attribType, attribSize, GL_TRUE, stride));
                }
            }
        }
    }

    mExpectedData[GL_BYTE]          = GetExpectedData<GLbyte>(srcData, GL_BYTE, GL_FALSE);
    mExpectedData[GL_UNSIGNED_BYTE] = GetExpectedData<GLubyte>(srcData, GL_UNSIGNED_BYTE, GL_FALSE);
    mExpectedData[GL_SHORT]         = GetExpectedData<GLshort>(srcData, GL_SHORT, GL_FALSE);
    mExpectedData[GL_UNSIGNED_SHORT] =
        GetExpectedData<GLushort>(srcData, GL_UNSIGNED_SHORT, GL_FALSE);
    mExpectedData[GL_INT]          = GetExpectedData<GLint>(srcData, GL_INT, GL_FALSE);
    mExpectedData[GL_UNSIGNED_INT] = GetExpectedData<GLuint>(srcData, GL_UNSIGNED_INT, GL_FALSE);

    mNormExpectedData[GL_BYTE] = GetExpectedData<GLbyte>(srcData, GL_BYTE, GL_TRUE);
    mNormExpectedData[GL_UNSIGNED_BYTE] =
        GetExpectedData<GLubyte>(srcData, GL_UNSIGNED_BYTE, GL_TRUE);
    mNormExpectedData[GL_SHORT] = GetExpectedData<GLshort>(srcData, GL_SHORT, GL_TRUE);
    mNormExpectedData[GL_UNSIGNED_SHORT] =
        GetExpectedData<GLushort>(srcData, GL_UNSIGNED_SHORT, GL_TRUE);
    mNormExpectedData[GL_INT]          = GetExpectedData<GLint>(srcData, GL_INT, GL_TRUE);
    mNormExpectedData[GL_UNSIGNED_INT] = GetExpectedData<GLuint>(srcData, GL_UNSIGNED_INT, GL_TRUE);
}

// In D3D11, we must sometimes translate buffer data into static attribute caches. We also use a
// cache management scheme which garbage collects old attributes after we start using too much
// cache data. This test tries to make as many attribute caches from a single buffer as possible
// to stress-test the caching code.
TEST_P(VertexAttributeCachingTest, BufferMulticaching)
{
    ANGLE_SKIP_TEST_IF(IsAMD() && IsDesktopOpenGL());

    initBasicProgram();

    glEnableVertexAttribArray(mTestAttrib);
    glEnableVertexAttribArray(mExpectedAttrib);

    ASSERT_GL_NO_ERROR();

    for (const AttribData &data : mTestData)
    {
        const auto &expected =
            (data.normalized) ? mNormExpectedData[data.type] : mExpectedData[data.type];

        GLsizei baseStride = static_cast<GLsizei>(data.size) * data.stride;
        GLsizei stride     = TypeStride(data.type) * baseStride;

        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glVertexAttribPointer(mTestAttrib, data.size, data.type, data.normalized, stride, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(mExpectedAttrib, data.size, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * baseStride, expected.data());
        drawQuad(mProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::white);
    }
}

// With D3D11 dirty bits for VertxArray11, we can leave vertex state unchanged if there aren't any
// GL calls that affect it. This test targets leaving one vertex attribute unchanged between draw
// calls while changing another vertex attribute enough that it clears the static buffer cache
// after enough iterations. It validates the unchanged attributes don't get deleted incidentally.
TEST_P(VertexAttributeCachingTest, BufferMulticachingWithOneUnchangedAttrib)
{
    ANGLE_SKIP_TEST_IF(IsAMD() && IsDesktopOpenGL());

    initDoubleAttribProgram();

    GLint testAttrib2Location = glGetAttribLocation(mProgram, "test2");
    ASSERT_NE(-1, testAttrib2Location);
    GLint expectedAttrib2Location = glGetAttribLocation(mProgram, "expected2");
    ASSERT_NE(-1, expectedAttrib2Location);

    glEnableVertexAttribArray(mTestAttrib);
    glEnableVertexAttribArray(mExpectedAttrib);
    glEnableVertexAttribArray(testAttrib2Location);
    glEnableVertexAttribArray(expectedAttrib2Location);

    ASSERT_GL_NO_ERROR();

    // Use an attribute that we know must be converted. This is a bit sensitive.
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glVertexAttribPointer(testAttrib2Location, 3, GL_UNSIGNED_SHORT, GL_FALSE, 6, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(expectedAttrib2Location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3,
                          mExpectedData[GL_UNSIGNED_SHORT].data());

    for (const auto &data : mTestData)
    {
        const auto &expected =
            (data.normalized) ? mNormExpectedData[data.type] : mExpectedData[data.type];

        GLsizei baseStride = static_cast<GLsizei>(data.size) * data.stride;
        GLsizei stride     = TypeStride(data.type) * baseStride;

        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glVertexAttribPointer(mTestAttrib, data.size, data.type, data.normalized, stride, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(mExpectedAttrib, data.size, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * baseStride, expected.data());
        drawQuad(mProgram, "position", 0.5f);

        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 255, 255, 255);
    }
}

// Test that if there are gaps in the attribute indices, the attributes have their correct values.
TEST_P(VertexAttributeTest, UnusedVertexAttribWorks)
{
    constexpr char kVertexShader[] = R"(attribute vec2 position;
attribute float actualValue;
uniform float expectedValue;
varying float result;
void main()
{
    result = (actualValue == expectedValue) ? 1.0 : 0.0;
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kFragmentShader[] = R"(varying mediump float result;
void main()
{
    gl_FragColor = result > 0.0 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);

    // Force a gap in attributes by using location 0 and 3
    GLint positionLocation = 0;
    glBindAttribLocation(program, positionLocation, "position");

    GLint attribLoc = 3;
    glBindAttribLocation(program, attribLoc, "actualValue");

    // Re-link the program to update the attribute locations
    glLinkProgram(program);
    ASSERT_TRUE(CheckLinkStatusAndReturnProgram(program, true));

    glUseProgram(program);

    GLint uniLoc = glGetUniformLocation(program, "expectedValue");
    ASSERT_NE(-1, uniLoc);

    glVertexAttribPointer(attribLoc, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    ASSERT_NE(-1, positionLocation);
    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    std::array<GLfloat, 4> testValues = {{1, 2, 3, 4}};
    for (GLfloat testValue : testValues)
    {
        glUniform1f(uniLoc, testValue);
        glVertexAttrib1f(attribLoc, testValue);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
}

// Tests that repeatedly updating a disabled vertex attribute works as expected.
// This covers an ANGLE bug where dirty bits for current values were ignoring repeated updates.
TEST_P(VertexAttributeTest, DisabledAttribUpdates)
{
    constexpr char kVertexShader[] = R"(attribute vec2 position;
attribute float actualValue;
uniform float expectedValue;
varying float result;
void main()
{
    result = (actualValue == expectedValue) ? 1.0 : 0.0;
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kFragmentShader[] = R"(varying mediump float result;
void main()
{
    gl_FragColor = result > 0.0 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);

    glUseProgram(program);
    GLint attribLoc = glGetAttribLocation(program, "actualValue");
    ASSERT_NE(-1, attribLoc);

    GLint uniLoc = glGetUniformLocation(program, "expectedValue");
    ASSERT_NE(-1, uniLoc);

    glVertexAttribPointer(attribLoc, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);
    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    std::array<GLfloat, 4> testValues = {{1, 2, 3, 4}};
    for (GLfloat testValue : testValues)
    {
        glUniform1f(uniLoc, testValue);
        glVertexAttrib1f(attribLoc, testValue);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
}

// Test that even inactive attributes are taken into account when checking for aliasing in case the
// shader version is >= 3.00. GLSL ES 3.00.6 section 12.46.
TEST_P(VertexAttributeTestES3, InactiveAttributeAliasing)
{
    constexpr char vertexShader[] =
        R"(#version 300 es
        precision mediump float;
        in vec4 input_active;
        in vec4 input_unused;
        void main()
        {
            gl_Position = input_active;
        })";

    constexpr char fragmentShader[] =
        R"(#version 300 es
        precision mediump float;
        out vec4 color;
        void main()
        {
            color = vec4(0.0);
        })";

    ANGLE_GL_PROGRAM(program, vertexShader, fragmentShader);
    glBindAttribLocation(program, 0, "input_active");
    glBindAttribLocation(program, 0, "input_unused");
    glLinkProgram(program);
    EXPECT_GL_NO_ERROR();
    GLint linkStatus = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);
}

// Test that enabling inactive attributes doesn't cause a crash
// shader version is >= 3.00
TEST_P(VertexAttributeTestES3, EnabledButInactiveAttributes)
{
    // This is similar to runtest(), and the test is disabled there
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    constexpr char testVertexShaderSource[] =
        R"(#version 300 es
precision mediump float;
in vec4 position;
layout(location = 1) in vec4 test;
layout(location = 2) in vec4 unused1;
layout(location = 3) in vec4 unused2;
layout(location = 4) in vec4 unused3;
layout(location = 5) in vec4 expected;
out vec4 color;
void main(void)
{
    gl_Position = position;
    vec4 threshold = max(abs(expected) * 0.01, 1.0 / 64.0);
    color = vec4(lessThanEqual(abs(test - expected), threshold));
})";

    // Same as previous one, except it uses unused1/2 instead of test/expected, leaving unused3
    // unused
    constexpr char testVertexShader2Source[] =
        R"(#version 300 es
precision mediump float;
in vec4 position;
layout(location = 1) in vec4 test;
layout(location = 2) in vec4 unused1;
layout(location = 3) in vec4 unused2;
layout(location = 4) in vec4 unused3;
layout(location = 5) in vec4 expected;
out vec4 color;
void main(void)
{
    gl_Position = position;
    vec4 threshold = max(abs(unused2) * 0.01, 1.0 / 64.0);
    color = vec4(lessThanEqual(abs(unused1 - unused2), threshold));
})";

    constexpr char testFragmentShaderSource[] =
        R"(#version 300 es
precision mediump float;
in vec4 color;
out vec4 out_color;
void main()
{
    out_color = color;
})";

    std::array<GLubyte, kVertexCount> inputData = {
        {0, 1, 2, 3, 4, 5, 6, 7, 125, 126, 127, 128, 129, 250, 251, 252, 253, 254, 255}};
    std::array<GLubyte, kVertexCount> inputData2;
    std::array<GLfloat, kVertexCount> expectedData;
    std::array<GLfloat, kVertexCount> expectedData2;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i]  = inputData[i];
        inputData2[i]    = inputData[i] > 128 ? inputData[i] - 1 : inputData[i] + 1;
        expectedData2[i] = inputData2[i];
    }

    // Setup the program
    mProgram = CompileProgram(testVertexShaderSource, testFragmentShaderSource);
    ASSERT_NE(0u, mProgram);

    mTestAttrib = glGetAttribLocation(mProgram, "test");
    ASSERT_EQ(1, mTestAttrib);
    mExpectedAttrib = glGetAttribLocation(mProgram, "expected");
    ASSERT_EQ(5, mExpectedAttrib);

    GLint unused1Attrib = 2;
    GLint unused2Attrib = 3;
    GLint unused3Attrib = 4;

    // Test enabling an unused attribute before glUseProgram
    glEnableVertexAttribArray(unused3Attrib);

    glUseProgram(mProgram);

    // Setup the test data
    TestData data(GL_UNSIGNED_BYTE, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                  expectedData.data());
    setupTest(data, 1);

    // Test enabling an unused attribute after glUseProgram
    glVertexAttribPointer(unused1Attrib, 1, data.type, data.normalized, 0, inputData2.data());
    glEnableVertexAttribArray(unused1Attrib);

    glVertexAttribPointer(unused2Attrib, 1, GL_FLOAT, GL_FALSE, 0, expectedData2.data());
    glEnableVertexAttribArray(unused2Attrib);

    // Run the test.  This shouldn't use the unused attributes.  Note that one of them is nullptr
    // which can cause a crash on certain platform-driver combination.
    drawQuad(mProgram, "position", 0.5f);
    checkPixels();

    // Now test with the same attributes enabled, but with a program with different attributes
    // active
    mProgram = CompileProgram(testVertexShader2Source, testFragmentShaderSource);
    ASSERT_NE(0u, mProgram);

    // Make sure all the attributes are in the same location
    ASSERT_EQ(glGetAttribLocation(mProgram, "unused1"), unused1Attrib);
    ASSERT_EQ(glGetAttribLocation(mProgram, "unused2"), unused2Attrib);

    glUseProgram(mProgram);

    // Run the test again.  unused1/2 were disabled in the previous run (as they were inactive in
    // the shader), but should be re-enabled now.
    drawQuad(mProgram, "position", 0.5f);
    checkPixels();
}

// Test that default integer attribute works correctly even if there is a gap in
// attribute locations.
TEST_P(VertexAttributeTestES3, DefaultIntAttribWithGap)
{
    constexpr char kVertexShader[] = R"(#version 300 es
layout(location = 0) in vec2 position;
layout(location = 3) in int actualValue;
uniform int expectedValue;
out float result;
void main()
{
    result = (actualValue == expectedValue) ? 1.0 : 0.0;
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kFragmentShader[] = R"(#version 300 es
in mediump float result;
layout(location = 0) out lowp vec4 out_color;
void main()
{
    out_color = result > 0.0 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);

    // Re-link the program to update the attribute locations
    glLinkProgram(program);
    ASSERT_TRUE(CheckLinkStatusAndReturnProgram(program, true));

    glUseProgram(program);

    GLint uniLoc = glGetUniformLocation(program, "expectedValue");
    ASSERT_NE(-1, uniLoc);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    std::array<GLint, 4> testValues = {{1, 2, 3, 4}};
    for (GLfloat testValue : testValues)
    {
        glUniform1i(uniLoc, testValue);
        glVertexAttribI4i(3, testValue, 0, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
}

// Test that when three vertex attribute indices are enabled, but only two attributes among them
// include data via glVertexAttribPointer(), there is no crash.
TEST_P(VertexAttributeTest, VertexAttribPointerCopyBufferFromInvalidAddress)
{
    const GLfloat vertices[] = {
        // position   // color                // texCoord
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // Lower left corner
        1.0f,  -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // Bottom right corner
        0.0f,  1.0f,  0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 1.0f   // Top
    };

    constexpr char kVS[] = R"(
        attribute highp vec2 position;
        attribute mediump vec4 color;
        attribute highp vec2 texCoord;
        varying mediump vec4 fragColor;
        varying highp vec2 fragTexCoord;
        void main() {
            gl_Position = vec4(position, 0.0, 1.0);
            fragColor = color;
            fragTexCoord = texCoord;
        }
    )";

    constexpr char kFS[] = R"(
        precision mediump float;
        varying mediump vec4 fragColor;
        varying highp vec2 fragTexCoord;
        void main() {
           if (fragTexCoord.x > 0.5) {
                gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
           } else {
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
           }
        }
    )";

    mProgram = CompileProgram(kVS, kFS);
    ASSERT_NE(0u, mProgram);
    glBindAttribLocation(mProgram, 0, "position");
    glBindAttribLocation(mProgram, 1, "color");
    glBindAttribLocation(mProgram, 2, "texCoord");
    glUseProgram(mProgram);
    EXPECT_GL_NO_ERROR();

    glGenBuffers(1, &mBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    EXPECT_GL_NO_ERROR();

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)0);
    // Missing VertexAttribPointer at index 1
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid *)(6 * sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    EXPECT_GL_NO_ERROR();
}

// Test that default unsigned integer attribute works correctly even if there is a gap in
// attribute locations.
TEST_P(VertexAttributeTestES3, DefaultUIntAttribWithGap)
{
    constexpr char kVertexShader[] = R"(#version 300 es
layout(location = 0) in vec2 position;
layout(location = 3) in uint actualValue;
uniform uint expectedValue;
out float result;
void main()
{
    result = (actualValue == expectedValue) ? 1.0 : 0.0;
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kFragmentShader[] = R"(#version 300 es
in mediump float result;
layout(location = 0) out lowp vec4 out_color;
void main()
{
    out_color = result > 0.0 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);

    // Re-link the program to update the attribute locations
    glLinkProgram(program);
    ASSERT_TRUE(CheckLinkStatusAndReturnProgram(program, true));

    glUseProgram(program);

    GLint uniLoc = glGetUniformLocation(program, "expectedValue");
    ASSERT_NE(-1, uniLoc);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    std::array<GLuint, 4> testValues = {{1, 2, 3, 4}};
    for (GLfloat testValue : testValues)
    {
        glUniform1ui(uniLoc, testValue);
        glVertexAttribI4ui(3, testValue, 0, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
}

// Tests that large strides that read past the end of the buffer work correctly.
// Requires ES 3.1 to query MAX_VERTEX_ATTRIB_STRIDE.
TEST_P(VertexAttributeTestES31, LargeStride)
{
    struct Vertex
    {
        Vector4 position;
        Vector2 color;
    };

    constexpr uint32_t kColorOffset = offsetof(Vertex, color);

    // Get MAX_VERTEX_ATTRIB_STRIDE.
    GLint maxStride;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &maxStride);

    uint32_t bufferSize  = static_cast<uint32_t>(maxStride);
    uint32_t stride      = sizeof(Vertex);
    uint32_t numVertices = bufferSize / stride;

    // The last vertex fits in the buffer size. The last vertex stride extends past it.
    ASSERT_LT(numVertices * stride, bufferSize);
    ASSERT_GT(numVertices * stride + kColorOffset, bufferSize);

    RNG rng(0);

    std::vector<Vertex> vertexData(bufferSize, {Vector4(), Vector2()});
    std::vector<GLColor> expectedColors;
    for (uint32_t vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
    {
        int x = vertexIndex % getWindowWidth();
        int y = vertexIndex / getWindowWidth();

        // Generate and clamp a 2 component vector.
        Vector4 randomVec4 = RandomVec4(rng.randomInt(), 0.0f, 1.0f);
        GLColor randomColor(randomVec4);
        randomColor[2]     = 0;
        randomColor[3]     = 255;
        Vector4 clampedVec = randomColor.toNormalizedVector();

        vertexData[vertexIndex] = {Vector4(x, y, 0.0f, 1.0f),
                                   Vector2(clampedVec[0], clampedVec[1])};
        expectedColors.push_back(randomColor);
    }

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, vertexData.data(), GL_STATIC_DRAW);

    vertexData.resize(numVertices);

    constexpr char kVS[] = R"(#version 310 es
in vec4 pos;
in vec2 color;
out vec2 vcolor;
void main()
{
    vcolor = color;
    gl_Position = vec4(((pos.x + 0.5) / 64.0) - 1.0, ((pos.y + 0.5) / 64.0) - 1.0, 0, 1);
    gl_PointSize = 1.0;
})";

    constexpr char kFS[] = R"(#version 310 es
precision mediump float;
in vec2 vcolor;
out vec4 fcolor;
void main()
{
    fcolor = vec4(vcolor, 0.0, 1.0);
})";

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, "pos");
    ASSERT_NE(-1, posLoc);
    GLint colorLoc = glGetAttribLocation(program, "color");
    ASSERT_NE(-1, colorLoc);

    glVertexAttribPointer(posLoc, 4, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(colorLoc, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(kColorOffset)));
    glEnableVertexAttribArray(colorLoc);

    glDrawArrays(GL_POINTS, 0, numVertices);

    // Validate pixels.
    std::vector<GLColor> actualColors(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actualColors.data());

    actualColors.resize(numVertices);

    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(expectedColors, actualColors);
}

std::string VertexAttributeTestES31::makeMismatchingSignsTestVS(uint32_t attribCount,
                                                                uint16_t signedMask)
{
    std::ostringstream shader;

    shader << R"(#version 310 es
precision highp float;

// The signedness is determined by |signedMask|.
)";

    for (uint32_t i = 0; i < attribCount; ++i)
    {
        shader << "in highp " << ((signedMask >> i & 1) == 0 ? "u" : "i") << "vec4 attrib" << i
               << ";\n";
    }

    shader << "flat out highp uvec4 v[" << attribCount << "];\n";

    shader << R"(
void main() {
)";

    for (uint32_t i = 0; i < attribCount; ++i)
    {
        shader << "v[" << i << "] = uvec4(attrib" << i << ");\n";
    }

    shader << R"(
    // gl_VertexID    x    y
    //      0        -1   -1
    //      1         1   -1
    //      2        -1    1
    //      3         1    1
    int bit0 = gl_VertexID & 1;
    int bit1 = gl_VertexID >> 1;
    gl_Position = vec4(bit0 * 2 - 1, bit1 * 2 - 1, gl_VertexID % 2 == 0 ? -1 : 1, 1);
})";

    return shader.str();
}

std::string VertexAttributeTestES31::makeMismatchingSignsTestFS(uint32_t attribCount)
{
    std::ostringstream shader;

    shader << R"(#version 310 es
precision highp float;
)";

    shader << "flat in highp uvec4 v[" << attribCount << "];\n";

    shader << R"(out vec4 fragColor;
uniform vec4 colorScale;

bool isOk(uvec4 inputVarying, uint index)
{
    return inputVarying.x == index &&
        inputVarying.y == index * 2u &&
        inputVarying.z == index + 1u &&
        inputVarying.w == index + 0x12345u;
}

void main()
{
    bool result = true;
)";
    shader << "    for (uint index = 0u; index < " << attribCount << "u; ++index)\n";
    shader << R"({
        result = result && isOk(v[index], index);
    }

    fragColor = vec4(result) * colorScale;
})";

    return shader.str();
}

uint16_t VertexAttributeTestES31::setupVertexAttribPointersForMismatchSignsTest(
    uint16_t currentSignedMask,
    uint16_t toggleMask)
{
    uint16_t newSignedMask = currentSignedMask ^ toggleMask;

    for (uint32_t i = 0; i < 16; ++i)
    {
        if ((toggleMask >> i & 1) == 0)
        {
            continue;
        }

        const GLenum type = (newSignedMask >> i & 1) == 0 ? GL_UNSIGNED_INT : GL_INT;
        glVertexAttribIPointer(i, 4, type, sizeof(GLuint[4]),
                               reinterpret_cast<const void *>(sizeof(GLuint[4][4]) * i));
    }

    return newSignedMask;
}

// Test changing between matching and mismatching signedness of vertex attributes, when the
// attribute changes type.
TEST_P(VertexAttributeTestES31, MismatchingSignsChangingAttributeType)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_relaxed_vertex_attribute_type"));

    // GL supports a minimum of 16 vertex attributes, and gl_VertexID is counted as one.
    // The signedness pattern used here is:
    //
    //   0                14
    //   iiui uuii iuui uui
    //
    // Which is chosen such that there's no clear repeating / mirror pattern.
    const std::string vs = makeMismatchingSignsTestVS(15, 0x49CB);
    const std::string fs = makeMismatchingSignsTestFS(15);

    ANGLE_GL_PROGRAM(program, vs.c_str(), fs.c_str());
    for (uint32_t i = 0; i < 15; ++i)
    {
        char attribName[20];
        snprintf(attribName, sizeof(attribName), "attrib%u\n", i);
        glBindAttribLocation(program, i, attribName);
    }
    glLinkProgram(program);
    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    GLint colorScaleLoc = glGetUniformLocation(program, "colorScale");
    ASSERT_NE(-1, colorScaleLoc);

    GLuint data[15][4][4];
    for (GLuint i = 0; i < 15; ++i)
    {
        for (GLuint j = 0; j < 4; ++j)
        {
            // Match the expectation in the shader
            data[i][j][0] = i;
            data[i][j][1] = i * 2;
            data[i][j][2] = i + 1;
            data[i][j][3] = i + 0x12345;
        }
    }

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    // Randomly match and mismatch the component type
    uint16_t signedMask = setupVertexAttribPointersForMismatchSignsTest(0x0FA5, 0x7FFF);

    for (uint32_t i = 0; i < 15; ++i)
    {
        glEnableVertexAttribArray(i);
    }

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glUniform4f(colorScaleLoc, 1, 0, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Modify the attributes randomly and make sure tests still pass

    signedMask = setupVertexAttribPointersForMismatchSignsTest(signedMask, 0x3572);
    glUniform4f(colorScaleLoc, 0, 1, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    signedMask = setupVertexAttribPointersForMismatchSignsTest(signedMask, 0x4B1C);
    glUniform4f(colorScaleLoc, 0, 0, 1, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    signedMask = setupVertexAttribPointersForMismatchSignsTest(signedMask, 0x19D6);
    glUniform4f(colorScaleLoc, 0, 0, 0, 1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // All channels must be 1
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::white);
    ASSERT_GL_NO_ERROR();
}

// Test changing between matching and mismatching signedness of vertex attributes, when the
// program itself changes the type.
TEST_P(VertexAttributeTestES31, MismatchingSignsChangingProgramType)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_ANGLE_relaxed_vertex_attribute_type"));

    // GL supports a minimum of 16 vertex attributes, and gl_VertexID is counted as one.
    constexpr uint32_t kAttribCount[4]      = {12, 9, 15, 7};
    constexpr uint32_t kAttribSignedMask[4] = {0x94f, 0x6A, 0x765B, 0x29};

    GLProgram programs[4];

    for (uint32_t progIndex = 0; progIndex < 4; ++progIndex)
    {
        const std::string vs =
            makeMismatchingSignsTestVS(kAttribCount[progIndex], kAttribSignedMask[progIndex]);
        const std::string fs = makeMismatchingSignsTestFS(kAttribCount[progIndex]);

        programs[progIndex].makeRaster(vs.c_str(), fs.c_str());
        for (uint32_t i = 0; i < kAttribCount[progIndex]; ++i)
        {
            char attribName[20];
            snprintf(attribName, sizeof(attribName), "attrib%u\n", i);
            glBindAttribLocation(programs[progIndex], i, attribName);
        }
        glLinkProgram(programs[progIndex]);
        glUseProgram(programs[progIndex]);
        ASSERT_GL_NO_ERROR();
    }

    GLuint data[15][4][4];
    for (GLuint i = 0; i < 15; ++i)
    {
        for (GLuint j = 0; j < 4; ++j)
        {
            // Match the expectation in the shader
            data[i][j][0] = i;
            data[i][j][1] = i * 2;
            data[i][j][2] = i + 1;
            data[i][j][3] = i + 0x12345;
        }
    }

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    // Randomly match and mismatch the component type
    setupVertexAttribPointersForMismatchSignsTest(0x55F8, 0x7FFF);

    for (uint32_t i = 0; i < 15; ++i)
    {
        glEnableVertexAttribArray(i);
    }

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glUseProgram(programs[0]);
    GLint colorScaleLoc = glGetUniformLocation(programs[0], "colorScale");
    ASSERT_NE(-1, colorScaleLoc);
    glUniform4f(colorScaleLoc, 1, 0, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Change the program, which have randomly different attribute component types and make sure
    // tests still pass

    glUseProgram(programs[1]);
    colorScaleLoc = glGetUniformLocation(programs[1], "colorScale");
    ASSERT_NE(-1, colorScaleLoc);
    glUniform4f(colorScaleLoc, 0, 1, 0, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(programs[2]);
    colorScaleLoc = glGetUniformLocation(programs[2], "colorScale");
    ASSERT_NE(-1, colorScaleLoc);
    glUniform4f(colorScaleLoc, 0, 0, 1, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(programs[3]);
    colorScaleLoc = glGetUniformLocation(programs[3], "colorScale");
    ASSERT_NE(-1, colorScaleLoc);
    glUniform4f(colorScaleLoc, 0, 0, 0, 1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // All channels must be 1
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::white);
    ASSERT_GL_NO_ERROR();
}

// Test that aliasing attribute locations work with es 100 shaders.  Note that es 300 and above
// don't allow vertex attribute aliasing.  This test excludes matrix types.
TEST_P(VertexAttributeTest, AliasingVectorAttribLocations)
{
    // http://anglebug.com/42263740
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // http://anglebug.com/42262130
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    // http://anglebug.com/42262131
    ANGLE_SKIP_TEST_IF(IsD3D());

    // This test needs 10 total attributes. All backends support this except some old Android
    // devices.
    GLint maxVertexAttribs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    ANGLE_SKIP_TEST_IF(maxVertexAttribs < 10);

    constexpr char kVS[] = R"(attribute vec4 position;
// 4 aliasing attributes
attribute float attr0f;
attribute vec2 attr0v2;
attribute vec3 attr0v3;
attribute vec4 attr0v4;
const vec4 attr0Expected = vec4(0.1, 0.2, 0.3, 0.4);

// 2 aliasing attributes
attribute vec2 attr1v2;
attribute vec3 attr1v3;
const vec3 attr1Expected = vec3(0.5, 0.6, 0.7);

// 2 aliasing attributes
attribute vec4 attr2v4;
attribute float attr2f;
const vec4 attr2Expected = vec4(0.8, 0.85, 0.9, 0.95);

// 2 aliasing attributes
attribute float attr3f1;
attribute float attr3f2;
const float attr3Expected = 1.0;

uniform float attr0Select;
uniform float attr1Select;
uniform float attr2Select;
uniform float attr3Select;

// Each channel controlled by success from each set of aliasing attributes.  If a channel is 0, the
// attribute test has failed.  Otherwise it will be 0.25, 0.5, 0.75 or 1.0, depending on how many
// channels there are in the compared attribute (except attr3).
varying mediump vec4 color;
void main()
{
    gl_Position = position;

    vec4 result = vec4(0);

    if (attr0Select < 0.5)
        result.r = abs(attr0f - attr0Expected.x) < 0.01 ? 0.25 : 0.0;
    else if (attr0Select < 1.5)
        result.r = all(lessThan(abs(attr0v2 - attr0Expected.xy), vec2(0.01))) ? 0.5 : 0.0;
    else if (attr0Select < 2.5)
        result.r = all(lessThan(abs(attr0v3 - attr0Expected.xyz), vec3(0.01))) ? 0.75 : 0.0;
    else
        result.r = all(lessThan(abs(attr0v4 - attr0Expected), vec4(0.01 )))? 1.0 : 0.0;

    if (attr1Select < 0.5)
        result.g = all(lessThan(abs(attr1v2 - attr1Expected.xy), vec2(0.01 )))? 0.5 : 0.0;
    else
        result.g = all(lessThan(abs(attr1v3 - attr1Expected), vec3(0.01 )))? 0.75 : 0.0;

    if (attr2Select < 0.5)
        result.b = abs(attr2f - attr2Expected.x) < 0.01 ? 0.25 : 0.0;
    else
        result.b = all(lessThan(abs(attr2v4 - attr2Expected), vec4(0.01))) ? 1.0 : 0.0;

    if (attr3Select < 0.5)
        result.a = abs(attr3f1 - attr3Expected) < 0.01 ? 0.25 : 0.0;
    else
        result.a = abs(attr3f2 - attr3Expected) < 0.01 ? 0.5 : 0.0;

    color = result;
})";

    constexpr char kFS[] = R"(varying mediump vec4 color;
    void main(void)
    {
        gl_FragColor = color;
    })";

    // Compile shaders.
    GLuint program = CompileProgram(kVS, kFS);
    ASSERT_NE(program, 0u);

    // Setup bindings.
    glBindAttribLocation(program, 0, "attr0f");
    glBindAttribLocation(program, 0, "attr0v2");
    glBindAttribLocation(program, 0, "attr0v3");
    glBindAttribLocation(program, 0, "attr0v4");
    glBindAttribLocation(program, 1, "attr1v2");
    glBindAttribLocation(program, 1, "attr1v3");
    glBindAttribLocation(program, 2, "attr2v4");
    glBindAttribLocation(program, 2, "attr2f");
    glBindAttribLocation(program, 3, "attr3f1");
    glBindAttribLocation(program, 3, "attr3f2");
    EXPECT_GL_NO_ERROR();

    // Link program and get uniform locations.
    glLinkProgram(program);
    glUseProgram(program);
    GLint attr0SelectLoc = glGetUniformLocation(program, "attr0Select");
    GLint attr1SelectLoc = glGetUniformLocation(program, "attr1Select");
    GLint attr2SelectLoc = glGetUniformLocation(program, "attr2Select");
    GLint attr3SelectLoc = glGetUniformLocation(program, "attr3Select");
    ASSERT_NE(-1, attr0SelectLoc);
    ASSERT_NE(-1, attr1SelectLoc);
    ASSERT_NE(-1, attr2SelectLoc);
    ASSERT_NE(-1, attr3SelectLoc);
    EXPECT_GL_NO_ERROR();

    // Set values for attributes.
    glVertexAttrib4f(0, 0.1f, 0.2f, 0.3f, 0.4f);
    glVertexAttrib3f(1, 0.5f, 0.6f, 0.7f);
    glVertexAttrib4f(2, 0.8f, 0.85f, 0.9f, 0.95f);
    glVertexAttrib1f(3, 1.0f);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    EXPECT_GL_NO_ERROR();

    // Go through different combination of attributes and make sure reading through every alias is
    // correctly handled.
    GLColor expected;
    for (uint32_t attr0Select = 0; attr0Select < 4; ++attr0Select)
    {
        glUniform1f(attr0SelectLoc, attr0Select);
        expected.R = attr0Select * 64 + 63;

        for (uint32_t attr1Select = 0; attr1Select < 2; ++attr1Select)
        {
            glUniform1f(attr1SelectLoc, attr1Select);
            expected.G = attr1Select * 64 + 127;

            for (uint32_t attr2Select = 0; attr2Select < 2; ++attr2Select)
            {
                glUniform1f(attr2SelectLoc, attr2Select);
                expected.B = attr2Select * 192 + 63;

                for (uint32_t attr3Select = 0; attr3Select < 2; ++attr3Select)
                {
                    glUniform1f(attr3SelectLoc, attr3Select);
                    expected.A = attr3Select * 64 + 63;

                    drawQuad(program, "position", 0.5f);
                    EXPECT_GL_NO_ERROR();
                    EXPECT_PIXEL_COLOR_NEAR(0, 0, expected, 1);
                }
            }
        }
    }
}

// Test that aliasing attribute locations work with es 100 shaders.  Note that es 300 and above
// don't allow vertex attribute aliasing.  This test includes matrix types.
TEST_P(VertexAttributeTest, AliasingMatrixAttribLocations)
{
    // http://anglebug.com/42263740
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // http://anglebug.com/42262130
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    // http://anglebug.com/42262131
    ANGLE_SKIP_TEST_IF(IsD3D());

    // This test needs 16 total attributes. All backends support this except some old Android
    // devices.
    GLint maxVertexAttribs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    ANGLE_SKIP_TEST_IF(maxVertexAttribs < 16);

    constexpr char kVS[] = R"(attribute vec4 position;
// attributes aliasing location 0 and above
attribute float attr0f;
attribute mat3 attr0m3;
attribute mat2 attr0m2;

// attributes aliasing location 1 and above
attribute vec4 attr1v4;

// attributes aliasing location 2 and above
attribute mat4 attr2m4;

// attributes aliasing location 3 and above
attribute mat2 attr3m2;

// attributes aliasing location 5 and above
attribute vec2 attr5v2;

// In summary (attr prefix shortened to a):
//
// location 0: a0f a0m3[0] a0m2[0]
// location 1:     a0m3[1] a0m2[1] a1v4
// location 2:     a0m3[2]              a2m4[0]
// location 3:                          a2m4[1] a3m2[0]
// location 4:                          a2m4[2] a3m2[1]
// location 5:                          a2m4[3]         a5v2

const vec3 loc0Expected = vec3(0.05, 0.1, 0.15);
const vec4 loc1Expected = vec4(0.2, 0.25, 0.3, 0.35);
const vec4 loc2Expected = vec4(0.4, 0.45, 0.5, 0.55);
const vec4 loc3Expected = vec4(0.6, 0.65, 0.7, 0.75);
const vec4 loc4Expected = vec4(0.8, 0.85, 0.9, 0.95);
const vec4 loc5Expected = vec4(0.25, 0.5, 0.75, 1.0);

uniform float loc0Select;
uniform float loc1Select;
uniform float loc2Select;
uniform float loc3Select;
uniform float loc4Select;
uniform float loc5Select;

// Each channel controlled by success from each set of aliasing locations.  Locations 2 and 3
// contribute to B together, while locations 4 and 5 contribute to A together.  If a channel is 0,
// the attribute test has failed.  Otherwise it will be 1/N, 2/N, ..., 1, depending on how many
// possible values there are for the controlling uniforms.
varying mediump vec4 color;
void main()
{
    gl_Position = position;

    vec4 result = vec4(0);

    if (loc0Select < 0.5)
        result.r = abs(attr0f - loc0Expected.x) < 0.01 ? 0.333333 : 0.0;
    else if (loc0Select < 1.5)
        result.r = all(lessThan(abs(attr0m2[0] - loc0Expected.xy), vec2(0.01))) ? 0.666667 : 0.0;
    else
        result.r = all(lessThan(abs(attr0m3[0] - loc0Expected), vec3(0.01))) ? 1.0 : 0.0;

    if (loc1Select < 0.5)
        result.g = all(lessThan(abs(attr0m3[1] - loc1Expected.xyz), vec3(0.01))) ? 0.333333 : 0.0;
    else if (loc1Select < 1.5)
        result.g = all(lessThan(abs(attr0m2[1] - loc1Expected.xy), vec2(0.01))) ? 0.666667 : 0.0;
    else
        result.g = all(lessThan(abs(attr1v4 - loc1Expected), vec4(0.01))) ? 1.0 : 0.0;

    bool loc2Ok = false;
    bool loc3Ok = false;

    if (loc2Select < 0.5)
        loc2Ok = all(lessThan(abs(attr0m3[2] - loc2Expected.xyz), vec3(0.01)));
    else
        loc2Ok = all(lessThan(abs(attr2m4[0] - loc2Expected), vec4(0.01)));

    if (loc3Select < 0.5)
        loc3Ok = all(lessThan(abs(attr2m4[1] - loc3Expected), vec4(0.01)));
    else
        loc3Ok = all(lessThan(abs(attr3m2[0] - loc3Expected.xy), vec2(0.01)));

    if (loc2Ok && loc3Ok)
    {
        if (loc2Select < 0.5)
            if (loc3Select < 0.5)
                result.b = 0.25;
            else
                result.b = 0.5;
        else
            if (loc3Select < 0.5)
                result.b = 0.75;
            else
                result.b = 1.0;
    }

    bool loc4Ok = false;
    bool loc5Ok = false;

    if (loc4Select < 0.5)
        loc4Ok = all(lessThan(abs(attr2m4[2] - loc4Expected), vec4(0.01)));
    else
        loc4Ok = all(lessThan(abs(attr3m2[1] - loc4Expected.xy), vec2(0.01)));

    if (loc5Select < 0.5)
        loc5Ok = all(lessThan(abs(attr2m4[3] - loc5Expected), vec4(0.01)));
    else
        loc5Ok = all(lessThan(abs(attr5v2 - loc5Expected.xy), vec2(0.01)));

    if (loc4Ok && loc5Ok)
    {
        if (loc4Select < 0.5)
            if (loc5Select < 0.5)
                result.a = 0.25;
            else
                result.a = 0.5;
        else
            if (loc5Select < 0.5)
                result.a = 0.75;
            else
                result.a = 1.0;
    }

    color = result;
})";

    constexpr char kFS[] = R"(varying mediump vec4 color;
    void main(void)
    {
        gl_FragColor = color;
    })";

    // Compile shaders.
    GLuint program = CompileProgram(kVS, kFS);
    ASSERT_NE(program, 0u);

    // Setup bindings.
    glBindAttribLocation(program, 0, "attr0f");
    glBindAttribLocation(program, 0, "attr0m3");
    glBindAttribLocation(program, 0, "attr0m2");
    glBindAttribLocation(program, 1, "attr1v4");
    glBindAttribLocation(program, 2, "attr2m4");
    glBindAttribLocation(program, 3, "attr3m2");
    glBindAttribLocation(program, 5, "attr5v2");
    EXPECT_GL_NO_ERROR();

    // Link program and get uniform locations.
    glLinkProgram(program);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLint loc0SelectLoc = glGetUniformLocation(program, "loc0Select");
    GLint loc1SelectLoc = glGetUniformLocation(program, "loc1Select");
    GLint loc2SelectLoc = glGetUniformLocation(program, "loc2Select");
    GLint loc3SelectLoc = glGetUniformLocation(program, "loc3Select");
    GLint loc4SelectLoc = glGetUniformLocation(program, "loc4Select");
    GLint loc5SelectLoc = glGetUniformLocation(program, "loc5Select");
    ASSERT_NE(-1, loc0SelectLoc);
    ASSERT_NE(-1, loc1SelectLoc);
    ASSERT_NE(-1, loc2SelectLoc);
    ASSERT_NE(-1, loc3SelectLoc);
    ASSERT_NE(-1, loc4SelectLoc);
    ASSERT_NE(-1, loc5SelectLoc);
    EXPECT_GL_NO_ERROR();

    // Set values for attributes.
    glVertexAttrib3f(0, 0.05, 0.1, 0.15);
    glVertexAttrib4f(1, 0.2, 0.25, 0.3, 0.35);
    glVertexAttrib4f(2, 0.4, 0.45, 0.5, 0.55);
    glVertexAttrib4f(3, 0.6, 0.65, 0.7, 0.75);
    glVertexAttrib4f(4, 0.8, 0.85, 0.9, 0.95);
    glVertexAttrib4f(5, 0.25, 0.5, 0.75, 1.0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    EXPECT_GL_NO_ERROR();

    // Go through different combination of attributes and make sure reading through every alias is
    // correctly handled.
    GLColor expected;
    for (uint32_t loc0Select = 0; loc0Select < 3; ++loc0Select)
    {
        glUniform1f(loc0SelectLoc, loc0Select);
        expected.R = loc0Select * 85 + 85;

        for (uint32_t loc1Select = 0; loc1Select < 3; ++loc1Select)
        {
            glUniform1f(loc1SelectLoc, loc1Select);
            expected.G = loc1Select * 85 + 85;

            for (uint32_t loc2Select = 0; loc2Select < 2; ++loc2Select)
            {
                glUniform1f(loc2SelectLoc, loc2Select);

                for (uint32_t loc3Select = 0; loc3Select < 2; ++loc3Select)
                {
                    glUniform1f(loc3SelectLoc, loc3Select);
                    expected.B = (loc2Select << 1 | loc3Select) * 64 + 63;

                    for (uint32_t loc4Select = 0; loc4Select < 2; ++loc4Select)
                    {
                        glUniform1f(loc4SelectLoc, loc4Select);

                        for (uint32_t loc5Select = 0; loc5Select < 2; ++loc5Select)
                        {
                            glUniform1f(loc5SelectLoc, loc5Select);
                            expected.A = (loc4Select << 1 | loc5Select) * 64 + 63;

                            drawQuad(program, "position", 0.5f);
                            EXPECT_GL_NO_ERROR();
                            EXPECT_PIXEL_COLOR_NEAR(0, 0, expected, 1);
                        }
                    }
                }
            }
        }
    }
}

// Test that aliasing attribute locations work with differing precisions.
TEST_P(VertexAttributeTest, AliasingVectorAttribLocationsDifferingPrecisions)
{
    // http://anglebug.com/42263740
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // http://anglebug.com/42262130
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    // http://anglebug.com/42262131
    ANGLE_SKIP_TEST_IF(IsD3D());

    constexpr char kVS[] = R"(attribute vec4 position;
// aliasing attributes.
attribute mediump vec2 attr0v2;
attribute highp vec3 attr0v3;
const vec3 attr0Expected = vec3(0.125, 0.25, 0.375);

// aliasing attributes.
attribute highp vec2 attr1v2;
attribute mediump vec3 attr1v3;
const vec3 attr1Expected = vec3(0.5, 0.625, 0.75);

uniform float attr0Select;
uniform float attr1Select;

// Each channel controlled by success from each set of aliasing attributes (R and G used only).  If
// a channel is 0, the attribute test has failed.  Otherwise it will be 0.5 or 1.0.
varying mediump vec4 color;
void main()
{
    gl_Position = position;

    vec4 result = vec4(0, 0, 0, 1);

    if (attr0Select < 0.5)
        result.r = all(lessThan(abs(attr0v2 - attr0Expected.xy), vec2(0.01))) ? 0.5 : 0.0;
    else
        result.r = all(lessThan(abs(attr0v3 - attr0Expected), vec3(0.01))) ? 1.0 : 0.0;

    if (attr1Select < 0.5)
        result.g = all(lessThan(abs(attr1v2 - attr1Expected.xy), vec2(0.01))) ? 0.5 : 0.0;
    else
        result.g = all(lessThan(abs(attr1v3 - attr1Expected), vec3(0.01))) ? 1.0 : 0.0;

    color = result;
})";

    constexpr char kFS[] = R"(varying mediump vec4 color;
    void main(void)
    {
        gl_FragColor = color;
    })";

    // Compile shaders.
    GLuint program = CompileProgram(kVS, kFS);
    ASSERT_NE(program, 0u);

    // Setup bindings.
    glBindAttribLocation(program, 0, "attr0v2");
    glBindAttribLocation(program, 0, "attr0v3");
    glBindAttribLocation(program, 1, "attr1v2");
    glBindAttribLocation(program, 1, "attr1v3");
    EXPECT_GL_NO_ERROR();

    // Link program and get uniform locations.
    glLinkProgram(program);
    glUseProgram(program);
    GLint attr0SelectLoc = glGetUniformLocation(program, "attr0Select");
    GLint attr1SelectLoc = glGetUniformLocation(program, "attr1Select");
    ASSERT_NE(-1, attr0SelectLoc);
    ASSERT_NE(-1, attr1SelectLoc);
    EXPECT_GL_NO_ERROR();

    // Set values for attributes.
    glVertexAttrib3f(0, 0.125f, 0.25f, 0.375f);
    glVertexAttrib3f(1, 0.5f, 0.625f, 0.75f);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    EXPECT_GL_NO_ERROR();

    // Go through different combination of attributes and make sure reading through every alias is
    // correctly handled.
    GLColor expected;
    expected.B = 0;
    expected.A = 255;
    for (uint32_t attr0Select = 0; attr0Select < 2; ++attr0Select)
    {
        glUniform1f(attr0SelectLoc, attr0Select);
        expected.R = attr0Select * 128 + 127;

        for (uint32_t attr1Select = 0; attr1Select < 2; ++attr1Select)
        {
            glUniform1f(attr1SelectLoc, attr1Select);
            expected.G = attr1Select * 128 + 127;

            drawQuad(program, "position", 0.5f);
            EXPECT_GL_NO_ERROR();
            EXPECT_PIXEL_COLOR_NEAR(0, 0, expected, 1);
        }
    }
}

// Test that unsupported vertex format specified on non-existing attribute doesn't crash.
TEST_P(VertexAttributeTest, VertexFormatConversionOfNonExistingAttribute)
{
    constexpr char kVS[] = R"(precision highp float;
attribute vec3 attr1;
void main(void) {
   gl_Position = vec4(attr1, 1.0);
})";

    constexpr char kFS[] = R"(precision highp float;
void main(void) {
   gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
})";

    GLBuffer emptyBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, emptyBuffer);

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glBindAttribLocation(program, 0, "attr1");
    glLinkProgram(program);
    ASSERT_TRUE(CheckLinkStatusAndReturnProgram(program, true));
    glUseProgram(program);

    // Use the RGB8 format for non-existing attribute 1.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_BYTE, false, 1, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    EXPECT_GL_NO_ERROR();
}

// Covers a bug with integer formats and an element size larger than the vertex stride.
TEST_P(VertexAttributeTestES3, StrideSmallerThanIntegerElementSize)
{
    constexpr char kVS[] = R"(#version 300 es
in vec4 position;
in ivec2 intAttrib;
in vec2 floatAttrib;
out vec4 colorVarying;
void main()
{
    gl_Position = position;
    if (vec2(intAttrib) == floatAttrib)
    {
        colorVarying = vec4(0, 1, 0, 1);
    }
    else
    {
        colorVarying = vec4(1, 0, 0, 1);
    }
})";

    constexpr char kFS[] = R"(#version 300 es
precision mediump float;
in vec4 colorVarying;
out vec4 fragColor;
void main()
{
    fragColor = colorVarying;
})";

    ANGLE_GL_PROGRAM(testProgram, kVS, kFS);
    glUseProgram(testProgram);

    GLBuffer positionBuffer;
    {
        const std::array<Vector3, 6> &quadVerts = GetQuadVertices();

        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
        glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                     GL_STATIC_DRAW);

        GLint posLoc = glGetAttribLocation(testProgram, "position");
        ASSERT_NE(posLoc, -1);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(posLoc);
    }

    GLBuffer intBuffer;
    {
        std::array<GLbyte, 12> intData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

        glBindBuffer(GL_ARRAY_BUFFER, intBuffer);
        glBufferData(GL_ARRAY_BUFFER, intData.size() * sizeof(intData[0]), intData.data(),
                     GL_STATIC_DRAW);

        GLint intLoc = glGetAttribLocation(testProgram, "intAttrib");
        ASSERT_NE(intLoc, -1);
        glVertexAttribIPointer(intLoc, 2, GL_BYTE, 1, nullptr);
        glEnableVertexAttribArray(intLoc);
    }

    GLBuffer floatBuffer;
    {
        std::array<GLfloat, 12> floatData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

        glBindBuffer(GL_ARRAY_BUFFER, floatBuffer);
        glBufferData(GL_ARRAY_BUFFER, floatData.size() * sizeof(floatData[0]), floatData.data(),
                     GL_STATIC_DRAW);

        GLint floatLoc = glGetAttribLocation(testProgram, "floatAttrib");
        ASSERT_NE(floatLoc, -1);
        glVertexAttribPointer(floatLoc, 2, GL_FLOAT, GL_FALSE, 4, nullptr);
        glEnableVertexAttribArray(floatLoc);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::green);
}

// Test that pipeline is recreated properly when switching from ARRAY buffer to client buffer,
// while removing client buffer. Bug observed in Dragonmania game.
TEST_P(VertexAttributeTestES31, ArrayToClientBufferStride)
{
    constexpr char kVS[] = R"(#version 310 es
precision highp float;
in vec4 in_pos;
in vec4 in_color;
out vec4 color;
void main(void) {
   gl_Position = in_pos;
   color = in_color;
})";

    constexpr char kFS[] = R"(#version 310 es
precision highp float;
in vec4 color;
out vec4 frag_color;
void main(void) {
   frag_color = color;
})";
    swapBuffers();

    ANGLE_GL_PROGRAM(program, kVS, kFS);
    glUseProgram(program);
    GLint posLoc   = glGetAttribLocation(program, "in_pos");
    GLint colorLoc = glGetAttribLocation(program, "in_color");
    ASSERT_NE(posLoc, -1);
    ASSERT_NE(colorLoc, -1);

    const std::array<Vector3, 6> &quadVerts = GetQuadVertices();
    // Data for packed attributes.
    std::array<float, ((3 + 4) * 6)> data;

    float kYellow[4] = {1.0f, 1.0f, 0.0f, 1.0f};
    float kGreen[4]  = {0.0f, 1.0f, 0.0f, 1.0f};

    for (int i = 0; i < 6; i++)
    {
        memcpy(&data[i * (3 + 4)], &quadVerts[i], sizeof(Vector3));
        memcpy(&data[i * (3 + 4) + 3], &kYellow, 4 * sizeof(float));
    }

    {
        GLBuffer buffer;
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data[0]), data.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(posLoc);
        glEnableVertexAttribArray(colorLoc);

        glVertexAttribPointer(posLoc, 3, GL_FLOAT, false, 28, reinterpret_cast<void *>(0));
        glVertexAttribPointer(colorLoc, 4, GL_FLOAT, false, 28, reinterpret_cast<void *>(12));

        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::yellow);
        EXPECT_GL_NO_ERROR();
        // Unbind before destroy.
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Modify color to green.
    for (int i = 0; i < 6; i++)
    {
        memcpy(&data[i * (3 + 4) + 3], &kGreen, 4 * sizeof(float));
    }

    // Provide client pointer.
    glVertexAttribPointer(posLoc, 3, GL_FLOAT, false, 28, data.data());
    glVertexAttribPointer(colorLoc, 4, GL_FLOAT, false, 28, &data[3]);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    EXPECT_PIXEL_RECT_EQ(0, 0, getWindowWidth(), getWindowHeight(), GLColor::green);
    EXPECT_GL_NO_ERROR();
}

// Create a vertex array with an empty array buffer and attribute offsets.
// This succeded in the end2end and capture/replay tests, but resulted in a trace
// producing a GL error when using MEC.
// Validation complained about the following:
// "Client data cannot be used with a non-default vertex array object."

// To capture this test with MEC run:
// mkdir src/tests/capture_replay_tests/empty_array_buffer_test
// ANGLE_CAPTURE_ENABLED=1 ANGLE_CAPTURE_FRAME_START=2 \
// ANGLE_CAPTURE_FRAME_END=2 ANGLE_CAPTURE_LABEL=empty_array_buffer_test \
// ANGLE_CAPTURE_OUT_DIR=src/tests/capture_replay_tests/empty_array_buffer_test \
// ./out/Debug/angle_end2end_tests \
// --gtest_filter="VertexAttributeTestES3.EmptyArrayBuffer/ES3_Vulkan"
TEST_P(VertexAttributeTestES3, EmptyArrayBuffer)
{
    GLVertexArray vertexArray;
    glBindVertexArray(vertexArray);

    GLBuffer emptyArrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, emptyArrayBuffer);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_TRUE, 20, reinterpret_cast<const void *>(16));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, reinterpret_cast<const void *>(8));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 20, nullptr);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    EXPECT_GL_NO_ERROR();

    // Swap a frame for MEC
    swapBuffers();
}

// Set an attrib pointer and delete it's buffer after usage, while keeping the vertex array.
// This will cause MEC to capture an invalid attribute pointer and also trigger
// "Client data cannot be used with a non-default vertex array object."
TEST_P(VertexAttributeTestES3, InvalidAttribPointer)
{
    GLVertexArray vertexArray;
    glBindVertexArray(vertexArray);

    std::array<GLbyte, 12> vertexData = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    {
        GLBuffer toBeDeletedArrayBuffer;
        glBindBuffer(GL_ARRAY_BUFFER, toBeDeletedArrayBuffer);

        glBufferData(GL_ARRAY_BUFFER, vertexData.size(), vertexData.data(), GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6, nullptr);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6, reinterpret_cast<const void *>(6));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        EXPECT_GL_NO_ERROR();
    }

    // Set an attrib pointer that will be actually picked up by MEC, since the buffer will be kept.
    glEnableVertexAttribArray(0);

    GLBuffer arrayBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuffer);

    glBufferData(GL_ARRAY_BUFFER, vertexData.size(), vertexData.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3, reinterpret_cast<const void *>(3));

    EXPECT_GL_NO_ERROR();

    // Swap a frame for MEC
    swapBuffers();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnableVertexAttribArray(0);
}

// Test maxinum attribs full of Client buffers and then switch to mixed.
TEST_P(VertexAttributeTestES3, fullClientBuffersSwitchToMixed)
{
    GLint maxAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
    ASSERT_GL_NO_ERROR();

    // Reserve one attrib for position
    GLint drawAttribs = maxAttribs - 1;

    GLuint program = compileMultiAttribProgram(drawAttribs);
    ASSERT_NE(0u, program);

    const std::array<Vector2, 4> kIndexedQuadVertices = {{
        Vector2(-1.0f, 1.0f),
        Vector2(-1.0f, -1.0f),
        Vector2(1.0f, -1.0f),
        Vector2(1.0f, 1.0f),
    }};

    GLsizei stride = (maxAttribs + 1) * sizeof(GLfloat);

    constexpr std::array<GLushort, 6> kIndexedQuadIndices = {{0, 1, 2, 0, 2, 3}};
    GLuint indexBuffer                                    = 0;
    glGenBuffers(1, &indexBuffer);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndexedQuadIndices), kIndexedQuadIndices.data(),
                 GL_STATIC_DRAW);

    // Vertex drawAttribs color attributes plus position (x, y).
    GLint totalComponents = drawAttribs + 2;
    std::vector<GLfloat> vertexData(totalComponents * 4, 0.0f);
    for (GLint index = 0; index < 4; ++index)
    {
        vertexData[index * totalComponents + drawAttribs]     = kIndexedQuadVertices[index].x();
        vertexData[index * totalComponents + drawAttribs + 1] = kIndexedQuadVertices[index].y();
    }

    GLfloat attributeValue = 0.0f;
    GLfloat delta          = 1.0f / 256.0f;
    for (GLint attribIndex = 0; attribIndex < drawAttribs; ++attribIndex)
    {
        vertexData[attribIndex]                       = attributeValue;
        vertexData[attribIndex + totalComponents]     = attributeValue;
        vertexData[attribIndex + totalComponents * 2] = attributeValue;
        vertexData[attribIndex + totalComponents * 3] = attributeValue;
        attributeValue += delta;
    }

    glUseProgram(program);
    for (GLint attribIndex = 0; attribIndex < drawAttribs; ++attribIndex)
    {
        std::stringstream attribStream;
        attribStream << "a" << attribIndex;
        GLint location = glGetAttribLocation(program, attribStream.str().c_str());
        ASSERT_NE(-1, location);
        glVertexAttribPointer(location, 1, GL_FLOAT, GL_FALSE, stride,
                              vertexData.data() + attribIndex);
        glEnableVertexAttribArray(location);
    }
    GLint posLoc = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, posLoc);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, stride, vertexData.data() + drawAttribs);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    // the result color should be (0 + 1 + 2 + ... + 14)/256 * 255;
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 105, 0, 0, 255, 1);

    // disable a few attribute use default attribute color
    GLint l0  = glGetAttribLocation(program, "a0");
    GLint l5  = glGetAttribLocation(program, "a5");
    GLint l13 = glGetAttribLocation(program, "a13");
    glDisableVertexAttribArray(l0);
    glVertexAttrib1f(l0, 1.0f / 16.0f);
    glDisableVertexAttribArray(l5);
    glVertexAttrib1f(l5, 1.0f / 16.0f);
    glDisableVertexAttribArray(l13);
    glVertexAttrib1f(l13, 1.0f / 16.0f);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 134, 0, 0, 255, 1);

    // disable all the client buffers.
    for (GLint attribIndex = 0; attribIndex < drawAttribs; ++attribIndex)
    {
        std::stringstream attribStream;
        attribStream << "a" << attribIndex;
        GLint location = glGetAttribLocation(program, attribStream.str().c_str());
        ASSERT_NE(-1, location);
        glDisableVertexAttribArray(location);
        glVertexAttrib1f(location, 1.0f / 16.0f);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 239, 0, 0, 255, 1);

    // enable all the client buffers.
    for (GLint attribIndex = 0; attribIndex < drawAttribs; ++attribIndex)
    {
        std::stringstream attribStream;
        attribStream << "a" << attribIndex;
        GLint location = glGetAttribLocation(program, attribStream.str().c_str());
        ASSERT_NE(-1, location);
        glVertexAttribPointer(location, 1, GL_FLOAT, GL_FALSE, stride,
                              vertexData.data() + attribIndex);
        glEnableVertexAttribArray(location);
    }
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 105, 0, 0, 255, 1);
}

// Test bind an empty buffer for vertex attribute does not crash
TEST_P(VertexAttributeTestES3, emptyBuffer)
{
    constexpr char vs2[] =
        R"(#version 300 es
            in uvec4 attr0;
            void main()
            {
                gl_Position = vec4(attr0.x, 0.0, 0.0, 0.0);
            })";
    constexpr char fs[] =
        R"(#version 300 es
            precision highp float;
            out vec4 color;
            void main()
            {
                color = vec4(1.0, 0.0, 0.0, 1.0);
            })";
    GLuint program2 = CompileProgram(vs2, fs);
    GLBuffer buf;
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(0, 4, GL_UNSIGNED_BYTE, 0, 0);
    glVertexAttribDivisor(0, 2);
    glUseProgram(program2);
    glDrawArrays(GL_POINTS, 0, 1);

    swapBuffers();
}

// This is a test for use with ANGLE's Capture/Replay.
// It emulates a situation we see in some apps, where attribs are passed in but may not be used.
// In particular, that test asks for all active attributes and iterates through each one. Before any
// changes to FrameCapture, this will create calls that look up attributes that are considered
// unused on some platforms, making the trace non-portable. Whether they are used depends on how
// well the stack optimizes the shader pipeline. In this instance, we are just passing them across
// the pipeline boundary where they are dead in the fragment shader, but other cases have included
// attributes passed to empty functions, or some eliminated with math. The more optimizations
// applied by the driver, the higher chance of getting an unused attribute.
TEST_P(VertexAttributeTestES3, UnusedAttribsMEC)
{
    constexpr char vertexShader[] =
        R"(#version 300 es
        precision mediump float;
        in vec4 position;
        in vec4 input_unused;
        out vec4 passthrough;
        void main()
        {
            passthrough = input_unused;
            gl_Position = position;
        })";

    constexpr char fragmentShader[] =
        R"(#version 300 es
        precision mediump float;
        in vec4 passthrough;
        out vec4 color;
        void main()
        {
            // ignore passthrough - this makes it unused with cross stage optimizations
            color = vec4(1.0);
        })";

    GLuint program = CompileProgram(vertexShader, fragmentShader);
    glUseProgram(program);

    // Set up vertex data
    GLBuffer positionBuffer;
    const std::array<Vector3, 6> &quadVerts = GetQuadVertices();
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    // Loop through a sequence multiple times, so MEC can capture it.
    // Ask about vertex attribs and set them up, regardless of whether they are used.
    // This matches behavior seen in some apps.
    for (int i = 0; i < 10; i++)
    {
        // Look up the number of attribs
        GLint activeAttribCount = 0;
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &activeAttribCount);

        // Look up how big they might get
        GLint maxActiveAttribLength = 0;
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxActiveAttribLength);

        GLsizei attribLength  = 0;
        GLint attribSize      = 0;
        GLenum attribType     = 0;
        GLchar attribName[16] = {0};
        ASSERT(maxActiveAttribLength < 16);

        // Look up each attribute and set them up
        for (int j = 0; j < activeAttribCount; j++)
        {
            glGetActiveAttrib(program, j, maxActiveAttribLength, &attribLength, &attribSize,
                              &attribType, attribName);
            GLint posLoc = glGetAttribLocation(program, attribName);
            ASSERT_NE(posLoc, -1);
            glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, j, nullptr);
            glEnableVertexAttribArray(posLoc);
        }

        // Draw and swap on each loop to trigger MEC
        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_PIXEL_COLOR_EQ(getWindowWidth() / 2, getWindowHeight() / 2, GLColor::white);
        swapBuffers();
    }
}

// Test that a particular vertex attribute naming does not affect the functionality.
// Tests the vertex attrib aliasing part. Note that aliasing works with es 100 shaders.
TEST_P(VertexAttributeTest, AliasingAttribNaming)
{
    // http://anglebug.com/42263740
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // http://anglebug.com/42262130
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    // http://anglebug.com/42262131
    ANGLE_SKIP_TEST_IF(IsD3D());

    // This test needs 16 total attributes. All backends support this except some old Android
    // devices.
    GLint maxVertexAttribs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    ANGLE_SKIP_TEST_IF(maxVertexAttribs < 16);

    constexpr char kVS[] = R"(attribute vec4 position;
// attributes aliasing location 0 and above
attribute mat3 a;
attribute mat2 a_;

// attributes aliasing location 1 and above
attribute vec4 a_1;

// attributes aliasing location 2 and above
attribute mat4 a_0;

// In summary:
//
// location 0: a[0] a_[0]
// location 1: a[1] a_[1] a_1
// location 2: a[2]          a_0[0]
// location 3:               a_0[1] (untested)
// location 4:               a_0[2] (untested)
// location 5:               a_0[3] (untested)

const vec3 loc0Expected = vec3(0.05, 0.1, 0.15);
const vec4 loc1Expected = vec4(0.2, 0.25, 0.3, 0.35);
const vec4 loc2Expected = vec4(0.4, 0.45, 0.5, 0.55);

uniform float loc0Select;
uniform float loc1Select;
uniform float loc2Select;

// Each channel controlled by success from each set of aliasing locations.  If a channel is 0,
// the attribute test has failed.  Otherwise it will be 1/N, 2/N, ..., 1, depending on how many
// possible values there are for the controlling uniforms.
varying mediump vec4 color;
void main()
{
    gl_Position = position;

    vec4 result = vec4(0);

    if (loc0Select < 0.5)
        result.r = all(lessThan(abs(a[0] - loc0Expected.xyz), vec3(0.01))) ? 0.5 : 0.0;
    else
        result.r = all(lessThan(abs(a_[0] - loc0Expected.xy), vec2(0.01))) ? 1.0 : 0.0;

    if (loc1Select < 0.5)
        result.g = all(lessThan(abs(a[1] - loc1Expected.xyz), vec3(0.01))) ? 0.333333 : 0.0;
    else if (loc1Select < 1.5)
        result.g = all(lessThan(abs(a_[1] - loc1Expected.xy), vec2(0.01))) ? 0.666667 : 0.0;
    else
        result.g = all(lessThan(abs(a_1 - loc1Expected), vec4(0.01))) ? 1.0 : 0.0;

    if (loc2Select < 0.5)
        result.b = all(lessThan(abs(a[2] - loc2Expected.xyz), vec3(0.01))) ? 0.5 : 0.0;
    else
        result.b = all(lessThan(abs(a_0[0] - loc2Expected), vec4(0.01))) ? 1.0 : 0.0;
    result.a = 1.0;
    color = result;
})";

    constexpr char kFS[] = R"(varying mediump vec4 color;
    void main(void)
    {
        gl_FragColor = color;
    })";

    // Compile shaders.
    GLuint program = CompileProgram(kVS, kFS);
    ASSERT_NE(program, 0u);

    // Setup bindings.
    glBindAttribLocation(program, 0, "a");
    glBindAttribLocation(program, 0, "a_");
    glBindAttribLocation(program, 1, "a_1");
    glBindAttribLocation(program, 2, "a_0");
    EXPECT_GL_NO_ERROR();

    // Link program and get uniform locations.
    glLinkProgram(program);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    GLint loc0SelectLoc = glGetUniformLocation(program, "loc0Select");
    GLint loc1SelectLoc = glGetUniformLocation(program, "loc1Select");
    GLint loc2SelectLoc = glGetUniformLocation(program, "loc2Select");
    ASSERT_NE(-1, loc0SelectLoc);
    ASSERT_NE(-1, loc1SelectLoc);
    ASSERT_NE(-1, loc2SelectLoc);
    EXPECT_GL_NO_ERROR();

    // Set values for attributes.
    glVertexAttrib3f(0, 0.05, 0.1, 0.15);
    glVertexAttrib4f(1, 0.2, 0.25, 0.3, 0.35);
    glVertexAttrib4f(2, 0.4, 0.45, 0.5, 0.55);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    EXPECT_GL_NO_ERROR();

    // Go through different combination of attributes and make sure reading through every alias is
    // correctly handled.
    GLColor expected;
    expected.A = 255;
    for (uint32_t loc0Select = 0; loc0Select < 2; ++loc0Select)
    {
        glUniform1f(loc0SelectLoc, loc0Select);
        expected.R = loc0Select * 127 + 127;

        for (uint32_t loc1Select = 0; loc1Select < 3; ++loc1Select)
        {
            glUniform1f(loc1SelectLoc, loc1Select);
            expected.G = loc1Select * 85 + 85;

            for (uint32_t loc2Select = 0; loc2Select < 2; ++loc2Select)
            {
                glUniform1f(loc2SelectLoc, loc2Select);
                expected.B = loc2Select * 127 + 127;
                drawQuad(program, "position", 0.5f);
                EXPECT_GL_NO_ERROR();
                EXPECT_PIXEL_COLOR_NEAR(0, 0, expected, 1);
            }
        }
    }
}

// Test that a particular vertex attribute naming does not affect the functionality.
TEST_P(VertexAttributeTestES3, AttribNaming)
{
    // http://anglebug.com/42263740
    ANGLE_SKIP_TEST_IF(IsAndroid() && IsOpenGL());

    // http://anglebug.com/42262130
    ANGLE_SKIP_TEST_IF(IsMac() && IsOpenGL());

    // http://anglebug.com/42262131
    ANGLE_SKIP_TEST_IF(IsD3D());

    // This test needs roughly 16 total attributes. All backends support this except some old
    // Android devices.
    GLint maxVertexAttribs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
    ANGLE_SKIP_TEST_IF(maxVertexAttribs < 16);

    constexpr char kVS[] = R"(#version 300 es
precision mediump float;
in vec4 position;
in mat3 a;
in mat2 a_;
in vec4 a_1;
in vec4 a_0;
const mat3 aExpected = mat3(0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8);
const mat2 a_Expected = mat2(1.0, 1.1, 1.2, 1.3);
const vec4 a_1Expected = vec4(2.0, 2.1, 2.2, 2.3);
const vec4 a_0Expected = vec4(3.0, 3.1, 3.2, 3.3);
out mediump vec4 color;
void main()
{
    gl_Position = position;

    vec4 result = vec4(0);
    mat3 diff3 = a - aExpected;
    result.r = all(lessThan(abs(diff3[0]) + abs(diff3[1]) + abs(diff3[2]), vec3(0.01))) ? 1.0 : 0.0;
    mat2 diff2 = a_ - a_Expected;
    result.g = all(lessThan(abs(diff2[0]) + abs(diff2[1]), vec2(0.01))) ? 1.0 : 0.0;
    result.b = all(lessThan(abs(a_1 - a_1Expected), vec4(0.01))) ? 1.0 : 0.0;
    result.a = all(lessThan(abs(a_0 - a_0Expected), vec4(0.01))) ? 1.0 : 0.0;
    color = result;
})";

    constexpr char kFS[] = R"(#version 300 es
in mediump vec4 color;
out mediump vec4 fragColor;
void main(void)
{
    fragColor = color;
})";

    GLuint program = CompileProgram(kVS, kFS);
    ASSERT_NE(program, 0u);

    glBindAttribLocation(program, 0, "a");
    glBindAttribLocation(program, 4, "a_");
    glBindAttribLocation(program, 6, "a_1");
    glBindAttribLocation(program, 7, "a_0");
    EXPECT_GL_NO_ERROR();

    glLinkProgram(program);
    glUseProgram(program);
    EXPECT_GL_NO_ERROR();

    // Set values for attributes.
    glVertexAttrib3f(0, 0.0, 0.1, 0.2);
    glVertexAttrib3f(1, 0.3, 0.4, 0.5);
    glVertexAttrib3f(2, 0.6, 0.7, 0.8);
    glVertexAttrib2f(4, 1.0, 1.1);
    glVertexAttrib2f(5, 1.2, 1.3);
    glVertexAttrib4f(6, 2.0, 2.1, 2.2, 2.3);
    glVertexAttrib4f(7, 3.0, 3.1, 3.2, 3.3);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(6);
    glDisableVertexAttribArray(7);
    glDisableVertexAttribArray(8);
    glDisableVertexAttribArray(9);
    EXPECT_GL_NO_ERROR();

    // Go through different combination of attributes and make sure reading through every alias is
    // correctly handled.
    GLColor expected{255, 255, 255, 255};
    drawQuad(program, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_NEAR(0, 0, expected, 1);
}

// VAO emulation fails on Mac but is not used on Mac in the wild. http://anglebug.com/40096758
#if !defined(__APPLE__)
#    define EMULATED_VAO_CONFIGS                                          \
        ES2_OPENGL().enable(Feature::SyncAllVertexArraysToDefault),       \
            ES2_OPENGLES().enable(Feature::SyncAllVertexArraysToDefault), \
            ES3_OPENGL().enable(Feature::SyncAllVertexArraysToDefault),   \
            ES3_OPENGLES().enable(Feature::SyncAllVertexArraysToDefault),
#else
#    define EMULATED_VAO_CONFIGS
#endif

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(
    VertexAttributeTest,
    ES2_VULKAN().enable(Feature::ForceFallbackFormat),
    ES2_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
    ES3_METAL().disable(Feature::HasExplicitMemBarrier).disable(Feature::HasCheapRenderPass),
    ES3_METAL().disable(Feature::HasExplicitMemBarrier).enable(Feature::HasCheapRenderPass),
    ES2_OPENGL().enable(Feature::ForceMinimumMaxVertexAttributes),
    ES2_OPENGLES().enable(Feature::ForceMinimumMaxVertexAttributes),
    EMULATED_VAO_CONFIGS);

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(
    VertexAttributeOORTest,
    ES2_VULKAN().enable(Feature::ForceFallbackFormat),
    ES2_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
    ES3_METAL().disable(Feature::HasExplicitMemBarrier).disable(Feature::HasCheapRenderPass),
    ES3_METAL().disable(Feature::HasExplicitMemBarrier).enable(Feature::HasCheapRenderPass));

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(RobustVertexAttributeTest);

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VertexAttributeTestES3);
ANGLE_INSTANTIATE_TEST_ES3_AND(
    VertexAttributeTestES3,
    ES3_VULKAN().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
    ES3_METAL().disable(Feature::HasExplicitMemBarrier).disable(Feature::HasCheapRenderPass),
    ES3_METAL().disable(Feature::HasExplicitMemBarrier).enable(Feature::HasCheapRenderPass),
    ES3_VULKAN()
        .disable(Feature::UseVertexInputBindingStrideDynamicState)
        .disable(Feature::SupportsGraphicsPipelineLibrary));

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(VertexAttributeTestES31);
ANGLE_INSTANTIATE_TEST_ES31_AND(VertexAttributeTestES31,
                                ES31_VULKAN().enable(Feature::ForceFallbackFormat),
                                ES31_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
                                ES31_VULKAN()
                                    .disable(Feature::UseVertexInputBindingStrideDynamicState)
                                    .disable(Feature::SupportsGraphicsPipelineLibrary));

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(
    VertexAttributeCachingTest,
    ES2_VULKAN().enable(Feature::ForceFallbackFormat),
    ES2_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN().enable(Feature::ForceFallbackFormat),
    ES3_VULKAN_SWIFTSHADER().enable(Feature::ForceFallbackFormat),
    ES3_METAL().disable(Feature::HasExplicitMemBarrier).disable(Feature::HasCheapRenderPass),
    ES3_METAL().disable(Feature::HasExplicitMemBarrier).enable(Feature::HasCheapRenderPass));

}  // anonymous namespace
