//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferImpl_mock.h:
//   Defines a mock of the FramebufferImpl class.
//

#ifndef LIBANGLE_RENDERER_FRAMEBUFFERIMPLMOCK_H_
#define LIBANGLE_RENDERER_FRAMEBUFFERIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/renderer/FramebufferImpl.h"

namespace rx
{

class MockFramebufferImpl : public rx::FramebufferImpl
{
  public:
    MockFramebufferImpl() : rx::FramebufferImpl(gl::FramebufferState(rx::UniqueSerial())) {}
    virtual ~MockFramebufferImpl() { destructor(); }

    MOCK_METHOD3(discard, angle::Result(const gl::Context *, size_t, const GLenum *));
    MOCK_METHOD3(invalidate, angle::Result(const gl::Context *, size_t, const GLenum *));
    MOCK_METHOD4(invalidateSub,
                 angle::Result(const gl::Context *, size_t, const GLenum *, const gl::Rectangle &));

    MOCK_METHOD2(clear, angle::Result(const gl::Context *, GLbitfield));
    MOCK_METHOD4(clearBufferfv, angle::Result(const gl::Context *, GLenum, GLint, const GLfloat *));
    MOCK_METHOD4(clearBufferuiv, angle::Result(const gl::Context *, GLenum, GLint, const GLuint *));
    MOCK_METHOD4(clearBufferiv, angle::Result(const gl::Context *, GLenum, GLint, const GLint *));
    MOCK_METHOD5(clearBufferfi, angle::Result(const gl::Context *, GLenum, GLint, GLfloat, GLint));

    MOCK_METHOD7(readPixels,
                 angle::Result(const gl::Context *,
                               const gl::Rectangle &,
                               GLenum,
                               GLenum,
                               const gl::PixelPackState &,
                               gl::Buffer *,
                               void *));

    MOCK_CONST_METHOD3(getSamplePosition, angle::Result(const gl::Context *, size_t, GLfloat *));

    MOCK_METHOD5(blit,
                 angle::Result(const gl::Context *,
                               const gl::Rectangle &,
                               const gl::Rectangle &,
                               GLbitfield,
                               GLenum));

    MOCK_CONST_METHOD1(checkStatus, gl::FramebufferStatus(const gl::Context *));

    MOCK_METHOD4(syncState,
                 angle::Result(const gl::Context *,
                               GLenum,
                               const gl::Framebuffer::DirtyBits &,
                               gl::Command command));

    MOCK_METHOD0(destructor, void());
};

inline ::testing::NiceMock<MockFramebufferImpl> *MakeFramebufferMock()
{
    ::testing::NiceMock<MockFramebufferImpl> *framebufferImpl =
        new ::testing::NiceMock<MockFramebufferImpl>();
    // TODO(jmadill): add ON_CALLS for other returning methods
    ON_CALL(*framebufferImpl, checkStatus(testing::_))
        .WillByDefault(::testing::Return(gl::FramebufferStatus::Complete()));

    // We must mock the destructor since NiceMock doesn't work for destructors.
    EXPECT_CALL(*framebufferImpl, destructor()).Times(1).RetiresOnSaturation();

    return framebufferImpl;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_FRAMEBUFFERIMPLMOCK_H_
