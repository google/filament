//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IndexDataManagerPerfTest:
//   Performance test for index buffer management.
//

#include "ANGLEPerfTest.h"

#include <gmock/gmock.h>

#include "angle_unittests_utils.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/IndexBuffer.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"

using namespace testing;

namespace
{
constexpr unsigned int kIterationsPerStep = 100;

class MockIndexBuffer : public rx::IndexBuffer
{
  public:
    MockIndexBuffer(unsigned int bufferSize, gl::DrawElementsType indexType)
        : mBufferSize(bufferSize), mIndexType(indexType)
    {}

    MOCK_METHOD4(initialize,
                 angle::Result(const gl::Context *, unsigned int, gl::DrawElementsType, bool));
    MOCK_METHOD4(mapBuffer,
                 angle::Result(const gl::Context *, unsigned int, unsigned int, void **));
    MOCK_METHOD1(unmapBuffer, angle::Result(const gl::Context *));
    MOCK_METHOD1(discard, angle::Result(const gl::Context *));
    MOCK_METHOD3(setSize, angle::Result(const gl::Context *, unsigned int, gl::DrawElementsType));

    // inlined for speed
    gl::DrawElementsType getIndexType() const override { return mIndexType; }
    unsigned int getBufferSize() const override { return mBufferSize; }

  private:
    unsigned int mBufferSize;
    gl::DrawElementsType mIndexType;
};

class MockBufferFactoryD3D : public rx::BufferFactoryD3D
{
  public:
    MockBufferFactoryD3D(unsigned int bufferSize, gl::DrawElementsType indexType)
        : mBufferSize(bufferSize), mIndexType(indexType)
    {}

    MOCK_METHOD0(createVertexBuffer, rx::VertexBuffer *());
    MOCK_CONST_METHOD1(getVertexConversionType, rx::VertexConversionType(angle::FormatID));
    MOCK_CONST_METHOD1(getVertexComponentType, GLenum(angle::FormatID));
    MOCK_CONST_METHOD7(getVertexSpaceRequired,
                       angle::Result(const gl::Context *,
                                     const gl::VertexAttribute &,
                                     const gl::VertexBinding &,
                                     size_t,
                                     GLsizei,
                                     GLuint,
                                     unsigned int *));

    // Dependency injection
    rx::IndexBuffer *createIndexBuffer() override
    {
        return new MockIndexBuffer(mBufferSize, mIndexType);
    }

  private:
    unsigned int mBufferSize;
    gl::DrawElementsType mIndexType;
};

class MockBufferD3D : public rx::BufferD3D
{
  public:
    MockBufferD3D(rx::BufferFactoryD3D *factory) : BufferD3D(mockState, factory), mData() {}

    // BufferImpl
    angle::Result setData(const gl::Context *context,
                          gl::BufferBinding target,
                          const void *data,
                          size_t size,
                          gl::BufferUsage) override
    {
        mData.resize(size);
        if (data && size > 0)
        {
            memcpy(&mData[0], data, size);
        }
        return angle::Result::Continue;
    }

    MOCK_METHOD5(
        setSubData,
        angle::Result(const gl::Context *, gl::BufferBinding, const void *, size_t, size_t));
    MOCK_METHOD5(copySubData,
                 angle::Result(const gl::Context *, BufferImpl *, GLintptr, GLintptr, GLsizeiptr));
    MOCK_METHOD3(map, angle::Result(const gl::Context *context, GLenum, void **));
    MOCK_METHOD5(mapRange, angle::Result(const gl::Context *, size_t, size_t, GLbitfield, void **));
    MOCK_METHOD2(unmap, angle::Result(const gl::Context *context, GLboolean *));

    // BufferD3D
    MOCK_METHOD1(markTransformFeedbackUsage, angle::Result(const gl::Context *));

    // inlined for speed
    bool supportsDirectBinding() const override { return false; }
    size_t getSize() const override { return mData.size(); }

    angle::Result getData(const gl::Context *context, const uint8_t **outData) override
    {
        *outData = &mData[0];
        return angle::Result::Continue;
    }

  private:
    gl::BufferState mockState;
    std::vector<uint8_t> mData;
};

class MockGLFactoryD3D : public rx::MockGLFactory
{
  public:
    MockGLFactoryD3D(MockBufferFactoryD3D *bufferFactory) : mBufferFactory(bufferFactory) {}

    rx::BufferImpl *createBuffer(const gl::BufferState &state) override
    {
        MockBufferD3D *mockBufferD3D = new MockBufferD3D(mBufferFactory);

        EXPECT_CALL(*mBufferFactory, createVertexBuffer())
            .WillOnce(Return(nullptr))
            .RetiresOnSaturation();
        mockBufferD3D->initializeStaticData(nullptr);

        return mockBufferD3D;
    }

    MockBufferFactoryD3D *mBufferFactory;
};

class IndexDataManagerPerfTest : public ANGLEPerfTest
{
  public:
    IndexDataManagerPerfTest();

    void step() override;

    rx::IndexDataManager mIndexDataManager;
    GLsizei mIndexCount;
    unsigned int mBufferSize;
    MockBufferFactoryD3D mMockBufferFactory;
    MockGLFactoryD3D mMockGLFactory;
    gl::Buffer mIndexBuffer;
};

IndexDataManagerPerfTest::IndexDataManagerPerfTest()
    : ANGLEPerfTest("IndexDataManager", "", "_run", kIterationsPerStep),
      mIndexDataManager(&mMockBufferFactory),
      mIndexCount(4000),
      mBufferSize(mIndexCount * sizeof(GLushort)),
      mMockBufferFactory(mBufferSize, gl::DrawElementsType::UnsignedShort),
      mMockGLFactory(&mMockBufferFactory),
      mIndexBuffer(&mMockGLFactory, {1})
{
    std::vector<GLushort> indexData(mIndexCount);
    for (GLsizei index = 0; index < mIndexCount; ++index)
    {
        indexData[index] = static_cast<GLushort>(index);
    }
    EXPECT_EQ(
        angle::Result::Continue,
        mIndexBuffer.bufferData(nullptr, gl::BufferBinding::Array, &indexData[0],
                                indexData.size() * sizeof(GLushort), gl::BufferUsage::StaticDraw));
}

void IndexDataManagerPerfTest::step()
{
    rx::TranslatedIndexData translatedIndexData;
    gl::IndexRange indexRange;
    for (unsigned int iteration = 0; iteration < kIterationsPerStep; ++iteration)
    {
        (void)mIndexBuffer.getIndexRange(nullptr, gl::DrawElementsType::UnsignedShort, 0,
                                         mIndexCount, false, &indexRange);
        (void)mIndexDataManager.prepareIndexData(nullptr, gl::DrawElementsType::UnsignedShort,
                                                 gl::DrawElementsType::UnsignedShort, mIndexCount,
                                                 &mIndexBuffer, nullptr, &translatedIndexData);
    }
}

TEST_F(IndexDataManagerPerfTest, Run)
{
    run();
}

}  // anonymous namespace
