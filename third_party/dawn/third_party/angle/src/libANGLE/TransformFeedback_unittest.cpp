//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Caps.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/renderer/BufferImpl_mock.h"
#include "libANGLE/renderer/TransformFeedbackImpl_mock.h"
#include "tests/angle_unittests_utils.h"

using ::testing::_;
using ::testing::get;
using ::testing::Return;
using ::testing::SetArgumentPointee;

namespace
{

ACTION(CreateMockTransformFeedbackImpl)
{
    return new rx::MockTransformFeedbackImpl(arg0);
}

class TransformFeedbackTest : public testing::Test
{
  protected:
    TransformFeedbackTest() : mImpl(nullptr), mFeedback(nullptr) {}

    void SetUp() override
    {
        EXPECT_CALL(mMockFactory, createTransformFeedback(_))
            .WillOnce(CreateMockTransformFeedbackImpl())
            .RetiresOnSaturation();

        // Set a reasonable number of tf attributes
        mCaps.maxTransformFeedbackSeparateAttributes = 8;

        mFeedback = new gl::TransformFeedback(&mMockFactory, gl::TransformFeedbackID{1}, mCaps);
        mFeedback->addRef();

        mImpl = rx::GetImplAs<rx::MockTransformFeedbackImpl>(mFeedback);
        EXPECT_CALL(*mImpl, destructor());
    }

    void TearDown() override
    {
        if (mFeedback)
        {
            mFeedback->release(nullptr);
        }

        // Only needed because the mock is leaked if bugs are present,
        // which logs an error, but does not cause the test to fail.
        // Ordinarily mocks are verified when destroyed.
        testing::Mock::VerifyAndClear(mImpl);
    }

    rx::MockGLFactory mMockFactory;
    rx::MockTransformFeedbackImpl *mImpl;
    gl::TransformFeedback *mFeedback;
    gl::Caps mCaps;
};

TEST_F(TransformFeedbackTest, SideEffectsOfStartAndStop)
{
    testing::InSequence seq;

    EXPECT_FALSE(mFeedback->isActive());
    EXPECT_CALL(*mImpl, begin(nullptr, gl::PrimitiveMode::Triangles));
    EXPECT_EQ(angle::Result::Continue,
              mFeedback->begin(nullptr, gl::PrimitiveMode::Triangles, nullptr));
    EXPECT_TRUE(mFeedback->isActive());
    EXPECT_EQ(gl::PrimitiveMode::Triangles, mFeedback->getPrimitiveMode());
    EXPECT_CALL(*mImpl, end(nullptr));
    EXPECT_EQ(angle::Result::Continue, mFeedback->end(nullptr));
    EXPECT_FALSE(mFeedback->isActive());
}

TEST_F(TransformFeedbackTest, SideEffectsOfPauseAndResume)
{
    testing::InSequence seq;

    EXPECT_FALSE(mFeedback->isActive());
    EXPECT_CALL(*mImpl, begin(nullptr, gl::PrimitiveMode::Triangles));
    EXPECT_EQ(angle::Result::Continue,
              mFeedback->begin(nullptr, gl::PrimitiveMode::Triangles, nullptr));
    EXPECT_FALSE(mFeedback->isPaused());
    EXPECT_CALL(*mImpl, pause(nullptr));
    EXPECT_EQ(angle::Result::Continue, mFeedback->pause(nullptr));
    EXPECT_TRUE(mFeedback->isPaused());
    EXPECT_CALL(*mImpl, resume(nullptr));
    EXPECT_EQ(angle::Result::Continue, mFeedback->resume(nullptr));
    EXPECT_FALSE(mFeedback->isPaused());
    EXPECT_CALL(*mImpl, end(nullptr));
    EXPECT_EQ(angle::Result::Continue, mFeedback->end(nullptr));
}

TEST_F(TransformFeedbackTest, BufferBinding)
{
    rx::MockBufferImpl *bufferImpl = new rx::MockBufferImpl;
    EXPECT_CALL(*bufferImpl, destructor()).Times(1).RetiresOnSaturation();

    rx::MockGLFactory mockGLFactory;
    EXPECT_CALL(mockGLFactory, createBuffer(_))
        .Times(1)
        .WillOnce(Return(bufferImpl))
        .RetiresOnSaturation();

    gl::Buffer *buffer = new gl::Buffer(&mockGLFactory, {1});

    static const size_t bindIndex = 0;

    EXPECT_EQ(mFeedback->getIndexedBufferCount(),
              static_cast<GLuint>(mCaps.maxTransformFeedbackSeparateAttributes));

    EXPECT_CALL(*mImpl, bindIndexedBuffer(_, _, _));
    EXPECT_EQ(angle::Result::Continue,
              mFeedback->bindIndexedBuffer(nullptr, bindIndex, buffer, 0, 1));
    for (size_t i = 0; i < mFeedback->getIndexedBufferCount(); i++)
    {
        if (i == bindIndex)
        {
            EXPECT_EQ(mFeedback->getIndexedBuffer(i).get(), buffer);
        }
        else
        {
            EXPECT_EQ(mFeedback->getIndexedBuffer(i).get(), nullptr);
        }
    }

    // force-release the feedback object to ensure the buffer is released.
    const size_t releaseCount = mFeedback->getRefCount();
    for (size_t count = 0; count < releaseCount; ++count)
    {
        mFeedback->release(nullptr);
    }

    mFeedback = nullptr;

    testing::Mock::VerifyAndClear(bufferImpl);
}

}  // anonymous namespace
