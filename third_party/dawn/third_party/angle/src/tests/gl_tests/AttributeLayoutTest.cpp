//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AttributeLayoutTest:
//   Test various layouts of vertex attribute data:
//   - in memory, in buffer object, or combination of both
//   - sequential or interleaved
//   - various combinations of data types

#include <vector>

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

// Test will draw these four triangles.
// clang-format off
constexpr double kTriangleData[] = {
    // xy       rgb
    0,0,        1,1,0,
    -1,+1,      1,1,0,
    +1,+1,      1,1,0,

    0,0,        0,1,0,
    +1,+1,      0,1,0,
    +1,-1,      0,1,0,

    0,0,        0,1,1,
    +1,-1,      0,1,1,
    -1,-1,      0,1,1,

    0,0,        1,0,1,
    -1,-1,      1,0,1,
    -1,+1,      1,0,1,
};
// clang-format on

constexpr size_t kNumVertices = ArraySize(kTriangleData) / 5;

// Vertex data source description.
class VertexData
{
  public:
    VertexData(int dimension,
               const double *data,
               unsigned offset,
               unsigned stride,
               unsigned numVertices)
        : mNumVertices(numVertices),
          mDimension(dimension),
          mData(data),
          mOffset(offset),
          mStride(stride)
    {}
    int getDimension() const { return mDimension; }
    unsigned getNumVertices() const { return mNumVertices; }
    double getValue(unsigned vertexNumber, int component) const
    {
        return mData[mOffset + mStride * vertexNumber + component];
    }

  private:
    unsigned mNumVertices;
    int mDimension;
    const double *mData;
    // offset and stride in doubles
    unsigned mOffset;
    unsigned mStride;
};

// A container for one or more vertex attributes.
class Container
{
  public:
    static constexpr size_t kSize = 1024;

    void open(void) { memset(mMemory, 0xff, kSize); }
    void *getDestination(size_t offset) { return mMemory + offset; }
    virtual void close(void) {}
    virtual ~Container() {}
    virtual const char *getAddress() = 0;
    virtual GLuint getBuffer()       = 0;

  protected:
    char mMemory[kSize];
};

// Vertex attribute data in client memory.
class Memory : public Container
{
  public:
    const char *getAddress() override { return mMemory; }
    GLuint getBuffer() override { return 0; }
};

// Vertex attribute data in buffer object.
class Buffer : public Container
{
  public:
    void close(void) override
    {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(mMemory), mMemory, GL_STATIC_DRAW);
    }

    const char *getAddress() override { return nullptr; }
    GLuint getBuffer() override { return mBuffer; }

  protected:
    GLBuffer mBuffer;
};

// Encapsulate the storage, layout, format and data of a vertex attribute.
struct Attrib
{
    void openContainer(void) const { mContainer->open(); }

    void fillContainer(void) const
    {
        for (unsigned i = 0; i < mData.getNumVertices(); ++i)
        {
            for (int j = 0; j < mData.getDimension(); ++j)
            {
                size_t destOffset = mOffset + mStride * i + mCTypeSize * j;
                if (destOffset + mCTypeSize > Container::kSize)
                    FAIL() << "test case does not fit container";

                double value = mData.getValue(i, j);
                if (mGLType == GL_FIXED)
                    value *= 1 << 16;
                else if (mNormalized)
                {
                    if (value < mMinIn || value > mMaxIn)
                        FAIL() << "test data does not fit format";
                    value = (value - mMinIn) * mScale + mMinOut;
                }

                mStore(value, mContainer->getDestination(destOffset));
            }
        }
    }

    void closeContainer(void) const { mContainer->close(); }

    void enable(unsigned index) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, mContainer->getBuffer());
        if (mPureInteger)
        {
            glVertexAttribIPointer(index, mData.getDimension(), mGLType, mStride,
                                   getContainerOffset());
        }
        else
        {
            glVertexAttribPointer(index, mData.getDimension(), mGLType, mNormalized, mStride,
                                  getContainerOffset());
        }
        EXPECT_GL_NO_ERROR();
        glEnableVertexAttribArray(index);
    }

    bool inClientMemory() const { return mContainer->getAddress() != nullptr; }
    const char *getContainerOffset() const
    {
        return inClientMemory() ? mContainer->getAddress() + mOffset
                                : reinterpret_cast<const char *>(mOffset);
    }

    std::shared_ptr<Container> mContainer;
    unsigned mOffset;
    unsigned mStride;
    const VertexData &mData;
    void (*mStore)(double value, void *dest);
    GLenum mGLType;
    GLboolean mNormalized;
    GLboolean mPureInteger = GL_FALSE;
    size_t mCTypeSize;
    double mMinIn;
    double mMaxIn;
    double mMinOut;
    double mScale;
};

// Change type and store.
template <class T>
void Store(double value, void *dest)
{
    T v = static_cast<T>(value);
    memcpy(dest, &v, sizeof(v));
}

// Function object that makes Attrib structs according to a vertex format.
template <class CType, GLenum GLType, bool Normalized, bool PureInteger = false>
class Format
{
    static_assert(!(Normalized && GLType == GL_FLOAT), "Normalized float does not make sense.");

  public:
    Format(bool es3) : mES3(es3) {}

    Attrib operator()(std::shared_ptr<Container> container,
                      unsigned offset,
                      unsigned stride,
                      const VertexData &data) const
    {
        double minIn    = 0;
        double maxIn    = 1;
        double minOut   = std::numeric_limits<CType>::min();
        double rangeOut = std::numeric_limits<CType>::max() - minOut;

        if (std::is_signed<CType>::value)
        {
            minIn = -1;
            maxIn = +1;
            if (mES3)
            {
                minOut += 1;
                rangeOut -= 1;
            }
        }

        return {
            container,
            offset,
            stride,
            data,
            Store<CType>,
            GLType,
            Normalized,
            PureInteger,
            sizeof(CType),
            minIn,
            maxIn,
            minOut,
            rangeOut / (maxIn - minIn),
        };
    }

  protected:
    const bool mES3;
};

typedef std::vector<Attrib> TestCase;

void PrepareTestCase(const TestCase &tc)
{
    for (const Attrib &a : tc)
        a.openContainer();
    for (const Attrib &a : tc)
        a.fillContainer();
    for (const Attrib &a : tc)
        a.closeContainer();
    unsigned i = 0;
    for (const Attrib &a : tc)
        a.enable(i++);
}

class AttributeLayoutTest : public ANGLETest<>
{
  protected:
    AttributeLayoutTest()
        : mProgram(0),
          mCoord(2, kTriangleData, 0, 5, kNumVertices),
          mColor(3, kTriangleData, 2, 5, kNumVertices)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void GetTestCases(void);

    void testSetUp() override
    {
        glClearColor(.2f, .2f, .2f, .0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        constexpr char kVS[] =
            "attribute mediump vec2 coord;\n"
            "attribute mediump vec3 color;\n"
            "varying mediump vec3 vcolor;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(coord, 0, 1);\n"
            "    vcolor = color;\n"
            "}\n";

        constexpr char kFS[] =
            "varying mediump vec3 vcolor;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(vcolor, 0);\n"
            "}\n";

        mProgram = CompileProgram(kVS, kFS);
        ASSERT_NE(0u, mProgram);
        glUseProgram(mProgram);

        glGenBuffers(1, &mIndexBuffer);

        GetTestCases();
    }

    void testTearDown() override
    {
        mTestCases.clear();
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mIndexBuffer);
    }

    virtual bool Skip(const TestCase &) { return false; }
    virtual void Draw(int firstVertex, unsigned vertexCount, const GLushort *indices) = 0;

    void Run(bool drawFirstTriangle)
    {
        glViewport(0, 0, getWindowWidth(), getWindowHeight());
        glUseProgram(mProgram);

        for (unsigned i = 0; i < mTestCases.size(); ++i)
        {
            if (mTestCases[i].size() == 0 || Skip(mTestCases[i]))
                continue;

            PrepareTestCase(mTestCases[i]);

            glClear(GL_COLOR_BUFFER_BIT);

            std::string testCase;
            if (drawFirstTriangle)
            {
                Draw(0, kNumVertices, mIndices);
                testCase = "draw";
            }
            else
            {
                Draw(3, kNumVertices - 3, mIndices + 3);
                testCase = "skip";
            }

            testCase += " first triangle case ";
            int w = getWindowWidth() / 4;
            int h = getWindowHeight() / 4;
            if (drawFirstTriangle)
            {
                EXPECT_PIXEL_EQ(w * 2, h * 3, 255, 255, 0, 0) << testCase << i;
            }
            else
            {
                EXPECT_PIXEL_EQ(w * 2, h * 3, 51, 51, 51, 0) << testCase << i;
            }
            EXPECT_PIXEL_EQ(w * 3, h * 2, 0, 255, 0, 0) << testCase << i;
            EXPECT_PIXEL_EQ(w * 2, h * 1, 0, 255, 255, 0) << testCase << i;
            EXPECT_PIXEL_EQ(w * 1, h * 2, 255, 0, 255, 0) << testCase << i;
        }
    }

    static const GLushort mIndices[kNumVertices];

    GLuint mProgram;
    GLuint mIndexBuffer;

    std::vector<TestCase> mTestCases;

    VertexData mCoord;
    VertexData mColor;
};
const GLushort AttributeLayoutTest::mIndices[kNumVertices] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

void AttributeLayoutTest::GetTestCases(void)
{
    const bool es3 = getClientMajorVersion() >= 3;

    Format<GLfloat, GL_FLOAT, false> Float(es3);
    Format<GLint, GL_FIXED, false> Fixed(es3);

    Format<GLbyte, GL_BYTE, false> SByte(es3);
    Format<GLubyte, GL_UNSIGNED_BYTE, false> UByte(es3);
    Format<GLshort, GL_SHORT, false> SShort(es3);
    Format<GLushort, GL_UNSIGNED_SHORT, false> UShort(es3);
    Format<GLint, GL_INT, false> SInt(es3);
    Format<GLuint, GL_UNSIGNED_INT, false> UInt(es3);

    Format<GLbyte, GL_BYTE, true> NormSByte(es3);
    Format<GLubyte, GL_UNSIGNED_BYTE, true> NormUByte(es3);
    Format<GLshort, GL_SHORT, true> NormSShort(es3);
    Format<GLushort, GL_UNSIGNED_SHORT, true> NormUShort(es3);
    Format<GLint, GL_INT, true> NormSInt(es3);
    Format<GLuint, GL_UNSIGNED_INT, true> NormUInt(es3);

    std::shared_ptr<Container> M0 = std::make_shared<Memory>();
    std::shared_ptr<Container> M1 = std::make_shared<Memory>();
    std::shared_ptr<Container> B0 = std::make_shared<Buffer>();
    std::shared_ptr<Container> B1 = std::make_shared<Buffer>();

    // 0. two buffers
    mTestCases.push_back({Float(B0, 0, 8, mCoord), Float(B1, 0, 12, mColor)});

    // 1. two memory
    mTestCases.push_back({Float(M0, 0, 8, mCoord), Float(M1, 0, 12, mColor)});

    // 2. one memory, sequential
    mTestCases.push_back({Float(M0, 0, 8, mCoord), Float(M0, 96, 12, mColor)});

    // 3. one memory, interleaved
    mTestCases.push_back({Float(M0, 0, 20, mCoord), Float(M0, 8, 20, mColor)});

    // 4. buffer and memory
    mTestCases.push_back({Float(B0, 0, 8, mCoord), Float(M0, 0, 12, mColor)});

    // 5. stride != size
    mTestCases.push_back({Float(B0, 0, 16, mCoord), Float(B1, 0, 12, mColor)});

    // 6-7. same stride and format, switching data between memory and buffer
    mTestCases.push_back({Float(M0, 0, 16, mCoord), Float(M1, 0, 12, mColor)});
    mTestCases.push_back({Float(B0, 0, 16, mCoord), Float(B1, 0, 12, mColor)});

    // 8-9. same stride and format, offset change
    mTestCases.push_back({Float(B0, 0, 8, mCoord), Float(B1, 0, 12, mColor)});
    mTestCases.push_back({Float(B0, 3, 8, mCoord), Float(B1, 4, 12, mColor)});

    // 10-11. unaligned buffer data
    mTestCases.push_back({Float(M0, 0, 8, mCoord), Float(B0, 1, 13, mColor)});
    mTestCases.push_back({Float(M0, 0, 8, mCoord), Float(B1, 1, 13, mColor)});

    // 12-15. byte/short
    mTestCases.push_back({SByte(M0, 0, 20, mCoord), UByte(M0, 10, 20, mColor)});
    mTestCases.push_back({SShort(M0, 0, 20, mCoord), UShort(M0, 8, 20, mColor)});
    mTestCases.push_back({NormSByte(M0, 0, 8, mCoord), NormUByte(M0, 4, 8, mColor)});
    mTestCases.push_back({NormSShort(M0, 0, 20, mCoord), NormUShort(M0, 8, 20, mColor)});

    // 16. one buffer, sequential
    mTestCases.push_back({Fixed(B0, 0, 8, mCoord), Float(B0, 96, 12, mColor)});

    // 17. one buffer, interleaved
    mTestCases.push_back({Fixed(B0, 0, 20, mCoord), Float(B0, 8, 20, mColor)});

    // 18. memory and buffer, float and integer
    mTestCases.push_back({Float(M0, 0, 8, mCoord), SByte(B0, 0, 12, mColor)});

    // 19. buffer and memory, unusual offset and stride
    mTestCases.push_back({Float(B0, 11, 13, mCoord), Float(M0, 23, 17, mColor)});

    // 20-21. remaining ES3 formats
    if (es3)
    {
        mTestCases.push_back({SInt(M0, 0, 40, mCoord), UInt(M0, 16, 40, mColor)});
        // Fails on Nexus devices (anglebug.com/42261348)
        if (!IsNexus5X())
            mTestCases.push_back({NormSInt(M0, 0, 40, mCoord), NormUInt(M0, 16, 40, mColor)});
    }
}

class AttributeLayoutNonIndexed : public AttributeLayoutTest
{
    void Draw(int firstVertex, unsigned vertexCount, const GLushort *indices) override
    {
        glDrawArrays(GL_TRIANGLES, firstVertex, vertexCount);
    }
};

class AttributeLayoutMemoryIndexed : public AttributeLayoutTest
{
    void Draw(int firstVertex, unsigned vertexCount, const GLushort *indices) override
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, indices);
    }
};

class AttributeLayoutBufferIndexed : public AttributeLayoutTest
{
    void Draw(int firstVertex, unsigned vertexCount, const GLushort *indices) override
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*mIndices) * vertexCount, indices,
                     GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, nullptr);
    }
};

TEST_P(AttributeLayoutNonIndexed, Test)
{
    Run(true);
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsOpenGL());
    Run(false);
}

TEST_P(AttributeLayoutMemoryIndexed, Test)
{
    Run(true);
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsOpenGL());
    Run(false);
}

TEST_P(AttributeLayoutBufferIndexed, Test)
{
    Run(true);
    ANGLE_SKIP_TEST_IF(IsWindows() && IsAMD() && IsOpenGL());
    Run(false);
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(AttributeLayoutNonIndexed,
                                       ES3_VULKAN()
                                           .disable(Feature::SupportsExtendedDynamicState)
                                           .disable(Feature::SupportsExtendedDynamicState2));
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(AttributeLayoutMemoryIndexed,
                                       ES3_VULKAN()
                                           .disable(Feature::SupportsExtendedDynamicState)
                                           .disable(Feature::SupportsExtendedDynamicState2));
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND(AttributeLayoutBufferIndexed,
                                       ES3_VULKAN()
                                           .disable(Feature::SupportsExtendedDynamicState)
                                           .disable(Feature::SupportsExtendedDynamicState2));

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

// clang-format off
#define VS_SHADER(ColorDataType) \
"#version 300 es\n"\
"in highp vec2 coord;\n"\
"in highp " STRINGIFY(ColorDataType) " color;\n"\
"flat out highp " STRINGIFY(ColorDataType) " vcolor;\n"\
"void main(void)\n"\
"{\n"\
"    gl_Position = vec4(coord, 0, 1);\n"\
"    vcolor = color;\n"\
"}\n"

#define PS_SHADER(ColorDataType) \
"#version 300 es\n"\
"flat in highp " STRINGIFY(ColorDataType) " vcolor;\n"\
"out highp " STRINGIFY(ColorDataType) " outColor;\n"\
"void main(void)\n"\
"{\n"\
"    outColor = vcolor;\n"\
"}\n"

// clang-format on

// clang-format off
constexpr double kVertexData[] = {
   //x   y       rgba
    -1, -1,      128, 128, 93, 255,
    +1, -1,      128, 128, 93, 255,
    -1, +1,      128, 128, 93, 255,
    +1, +1,      128, 128, 93, 255,
};
// clang-format on

template <class ResType>
ResType GetRefValue(const void *data, GLenum glType)
{
    switch (glType)
    {
        case GL_BYTE:
        {
            const int8_t *p = reinterpret_cast<const int8_t *>(data);
            return ResType(p[0], p[1], p[2], p[3]);
        }
        case GL_SHORT:
        case GL_HALF_FLOAT:
        {
            const int16_t *p = reinterpret_cast<const int16_t *>(data);
            return ResType(p[0], p[1], p[2], p[3]);
        }
        case GL_INT:
        case GL_FIXED:
        {
            const int32_t *p = reinterpret_cast<const int32_t *>(data);
            return ResType(p[0], p[1], p[2], p[3]);
        }
        case GL_UNSIGNED_BYTE:
        {
            const uint8_t *p = reinterpret_cast<const uint8_t *>(data);
            return ResType(p[0], p[1], p[2], p[3]);
        }
        case GL_UNSIGNED_SHORT:
        {
            const uint16_t *p = reinterpret_cast<const uint16_t *>(data);
            return ResType(p[0], p[1], p[2], p[3]);
        }
        case GL_FLOAT:
        case GL_UNSIGNED_INT:
        {
            const uint32_t *p = reinterpret_cast<const uint32_t *>(data);
            return ResType(p[0], p[1], p[2], p[3]);
        }
        default:
        {
            ASSERT(0);
            const uint32_t *p = reinterpret_cast<const uint32_t *>(data);
            return ResType(p[0], p[1], p[2], p[3]);
        }
    }
}

constexpr size_t kIndexCount = 6;
constexpr int kRboSize       = 8;

GLColor ConvertFloatToUnorm8(const GLColor32F &color32f)
{
    float r = std::clamp(color32f.R, 0.0f, 1.0f);
    float g = std::clamp(color32f.G, 0.0f, 1.0f);
    float b = std::clamp(color32f.B, 0.0f, 1.0f);
    float a = std::clamp(color32f.A, 0.0f, 1.0f);
    return GLColor(std::round(r * 255), std::round(g * 255), std::round(b * 255),
                   std::round(a * 255));
}

void BindAttribLocation(GLuint program)
{
    glBindAttribLocation(program, 0, "coord");
    glBindAttribLocation(program, 1, "color");
}

class AttributeDataTypeMismatchTest : public ANGLETest<>
{
  public:
    enum VsInputDataType
    {
        FLOAT    = 0,
        INT      = 1,
        UNSIGNED = 2,
        COUNT    = 3,
    };

  protected:
    AttributeDataTypeMismatchTest()
        : mCoord(2, kVertexData, 0, 6, 4), mColor(4, kVertexData, 2, 6, 4)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    GLuint createFbo(GLuint rbo)
    {

        GLuint fbo = 0;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return fbo;
    }

    GLuint createRbo(VsInputDataType inputDataType)
    {
        GLuint rbo = 0;
        glGenRenderbuffers(1, &rbo);
        GLenum format = GL_RGBA8;
        if (inputDataType == VsInputDataType::INT)
        {
            format = GL_RGBA32I;
        }
        else if (inputDataType == VsInputDataType::UNSIGNED)
        {
            format = GL_RGBA32UI;
        }
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, format, kRboSize, kRboSize);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        return rbo;
    }

    void testSetUp() override
    {
        glClearColor(.2f, .2f, .2f, .0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        constexpr const char *kVS[VsInputDataType::COUNT] = {
            VS_SHADER(vec4),
            VS_SHADER(ivec4),
            VS_SHADER(uvec4),
        };

        constexpr const char *kFS[VsInputDataType::COUNT] = {
            PS_SHADER(vec4),
            PS_SHADER(ivec4),
            PS_SHADER(uvec4),
        };

        for (int i = VsInputDataType::FLOAT; i < VsInputDataType::COUNT; ++i)
        {
            mProgram[i] = CompileProgram(kVS[i], kFS[i], BindAttribLocation);
            ASSERT_NE(0u, mProgram[i]);
            mRbo[i] = createRbo(static_cast<VsInputDataType>(i));
            ASSERT_NE(0u, mRbo[i]);
            mFbo[i] = createFbo(mRbo[i]);
            ASSERT_NE(0u, mFbo[i]);
        }

        glGenBuffers(1, &mIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mIndices), mIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void GetTestCases(VsInputDataType dataType)
    {
        const bool es3 = getClientMajorVersion() >= 3;

        std::shared_ptr<Container> B0 = std::make_shared<Buffer>();
        if (dataType != VsInputDataType::FLOAT)
        {
            // float and fixed.
            Format<GLfloat, GL_FLOAT, false> Float(es3);
            Format<GLint, GL_FIXED, false> Fixed(es3);
            Format<GLshort, GL_HALF_FLOAT, false> halfFloat(es3);
            // for UScale, SScale.
            Format<GLbyte, GL_BYTE, false> SByte(es3);
            Format<GLubyte, GL_UNSIGNED_BYTE, false> UByte(es3);
            Format<GLshort, GL_SHORT, false> SShort(es3);
            Format<GLushort, GL_UNSIGNED_SHORT, false> UShort(es3);
            // UScale32, Scale32 may emulated. testing unsigned<-->int
            Format<GLint, GL_INT, false, true> SInt(es3);
            Format<GLuint, GL_UNSIGNED_INT, false, true> UInt(es3);
            mTestCases.push_back({Float(B0, 0, 12, mCoord), SByte(B0, 8, 12, mColor)});
            mTestCases.push_back({Float(B0, 0, 12, mCoord), UByte(B0, 8, 12, mColor)});
            mTestCases.push_back({Float(B0, 0, 16, mCoord), SShort(B0, 8, 16, mColor)});
            mTestCases.push_back({Float(B0, 0, 16, mCoord), UShort(B0, 8, 16, mColor)});
            mTestCases.push_back({Float(B0, 0, 16, mCoord), halfFloat(B0, 8, 16, mColor)});
            mTestCases.push_back({Float(B0, 0, 24, mCoord), SInt(B0, 8, 24, mColor)});
            mTestCases.push_back({Float(B0, 0, 24, mCoord), UInt(B0, 8, 24, mColor)});
            mTestCases.push_back({Float(B0, 0, 24, mCoord), Float(B0, 8, 24, mColor)});
            // for GL_FIXED, angle may use GLfloat emulated.
            // mTestCases.push_back({Float(B0, 0, 24, mCoord), Fixed(B0, 8, 24, mColor)});
        }
        else
        {
            Format<GLfloat, GL_FLOAT, false> Float(es3);
            Format<GLbyte, GL_BYTE, false, true> SByte(es3);
            Format<GLubyte, GL_UNSIGNED_BYTE, false, true> UByte(es3);
            Format<GLshort, GL_SHORT, false, true> SShort(es3);
            Format<GLushort, GL_UNSIGNED_SHORT, false, true> UShort(es3);
            Format<GLint, GL_INT, false, true> SInt(es3);
            Format<GLuint, GL_UNSIGNED_INT, false, true> UInt(es3);
            mTestCases.push_back({Float(B0, 0, 12, mCoord), SByte(B0, 8, 12, mColor)});
            mTestCases.push_back({Float(B0, 0, 12, mCoord), UByte(B0, 8, 12, mColor)});
            mTestCases.push_back({Float(B0, 0, 16, mCoord), SShort(B0, 8, 16, mColor)});
            mTestCases.push_back({Float(B0, 0, 16, mCoord), UShort(B0, 8, 16, mColor)});
            // UScale32, Scale32 may emulated.
            // mTestCases.push_back({Float(B0, 0, 24, mCoord), SInt(B0, 8, 24, mColor)});
            // mTestCases.push_back({Float(B0, 0, 24, mCoord), UInt(B0, 8, 24, mColor)});
        }
    }

    void testTearDown() override
    {
        mTestCases.clear();
        for (int i = 0; i < VsInputDataType::COUNT; ++i)
        {
            glDeleteProgram(mProgram[i]);
            glDeleteFramebuffers(1, &mFbo[i]);
            glDeleteRenderbuffers(1, &mRbo[i]);
        }
        glDeleteBuffers(1, &mIndexBuffer);
    }

    GLenum GetMappedGLType(GLenum glType, VsInputDataType vsInputDataType)
    {
        switch (glType)
        {
            case GL_BYTE:
                return vsInputDataType != VsInputDataType::UNSIGNED ? GL_BYTE : GL_UNSIGNED_BYTE;
            case GL_SHORT:
            case GL_HALF_FLOAT:
                return vsInputDataType != VsInputDataType::UNSIGNED ? GL_SHORT : GL_UNSIGNED_SHORT;
            case GL_INT:
            case GL_FIXED:
                return vsInputDataType != VsInputDataType::UNSIGNED ? GL_INT : GL_UNSIGNED_INT;
            case GL_UNSIGNED_BYTE:
                return vsInputDataType != VsInputDataType::INT ? GL_UNSIGNED_BYTE : GL_BYTE;
            case GL_UNSIGNED_SHORT:
                return vsInputDataType != VsInputDataType::INT ? GL_UNSIGNED_SHORT : GL_SHORT;
            case GL_FLOAT:
            case GL_UNSIGNED_INT:
                return vsInputDataType != VsInputDataType::INT ? GL_UNSIGNED_INT : GL_INT;
            default:
                ASSERT(0);
                return vsInputDataType != VsInputDataType::INT ? GL_UNSIGNED_INT : GL_INT;
        }
    }

    void Run(VsInputDataType dataType)
    {
        GetTestCases(dataType);
        ASSERT(dataType < VsInputDataType::COUNT);
        glBindFramebuffer(GL_FRAMEBUFFER, mFbo[dataType]);
        glViewport(0, 0, kRboSize, kRboSize);
        glUseProgram(mProgram[dataType]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        for (unsigned i = 0; i < mTestCases.size(); ++i)
        {
            if (mTestCases[i].size() == 0)
                continue;
            ASSERT(mTestCases[i].size() == 2);
            PrepareTestCase(mTestCases[i]);
            EXPECT_GL_NO_ERROR();
            GLint iClearValue[]   = {0, 0, 0, 1};
            GLfloat fClearValue[] = {1.0f, 0.0f, 0.0f, 1.0f};
            switch (dataType)
            {
                case VsInputDataType::FLOAT:
                    glClearBufferfv(GL_COLOR, 0, fClearValue);
                    break;
                case VsInputDataType::INT:
                    glClearBufferiv(GL_COLOR, 0, iClearValue);
                    break;
                case VsInputDataType::UNSIGNED:
                    glClearBufferuiv(GL_COLOR, 0, reinterpret_cast<const GLuint *>(iClearValue));
                    break;
                default:
                    ASSERT(0);
            }
            EXPECT_GL_NO_ERROR();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            EXPECT_GL_NO_ERROR();

            std::shared_ptr<Container> container = mTestCases[i][1].mContainer;
            size_t offset                        = mTestCases[i][1].mOffset;
            GLenum glType = GetMappedGLType(mTestCases[i][1].mGLType, dataType);
            switch (dataType)
            {
                case VsInputDataType::FLOAT:
                    EXPECT_PIXEL_COLOR_EQ(0, 0,
                                          ConvertFloatToUnorm8(GetRefValue<GLColor32F>(
                                              container->getDestination(offset), glType)));
                    break;
                case VsInputDataType::INT:
                    EXPECT_PIXEL_RECT32I_EQ(
                        0, 0, 1, 1,
                        GetRefValue<GLColor32I>(container->getDestination(offset), glType));
                    break;
                case VsInputDataType::UNSIGNED:
                    EXPECT_PIXEL_RECT32UI_EQ(
                        0, 0, 1, 1,
                        GetRefValue<GLColor32UI>(container->getDestination(offset), glType));
                    break;
                default:
                    ASSERT(0);
            }
        }
        mTestCases.clear();
    }

    static const GLushort mIndices[kIndexCount];

    GLuint mProgram[VsInputDataType::COUNT];
    GLuint mFbo[VsInputDataType::COUNT];
    GLuint mRbo[VsInputDataType::COUNT];
    GLuint mIndexBuffer;

    std::vector<TestCase> mTestCases;

    VertexData mCoord;
    VertexData mColor;
};

const GLushort AttributeDataTypeMismatchTest::mIndices[kIndexCount] = {0, 1, 2, 2, 1, 3};

// Test Attribute input data type mismatch with vertex shader input.
// Change the attribute input data type to vertex shader input data type.
TEST_P(AttributeDataTypeMismatchTest, Test)
{
    // At some device. UScale and Scale are emulated.
    // Restrict tests running at nvidia device only.
    ANGLE_SKIP_TEST_IF(!IsVulkan() || !IsNVIDIA());
    Run(VsInputDataType::FLOAT);
    Run(VsInputDataType::INT);
    Run(VsInputDataType::UNSIGNED);
}

ANGLE_INSTANTIATE_TEST_ES3(AttributeDataTypeMismatchTest);

}  // anonymous namespace
