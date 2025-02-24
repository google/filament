//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "libANGLE/AttributeMap.h"
#include "libANGLE/Config.h"
#include "libANGLE/State.h"
#include "libANGLE/Surface.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/FramebufferImpl_mock.h"
#include "libANGLE/renderer/SurfaceImpl.h"
#include "tests/angle_unittests_utils.h"

using namespace rx;
using namespace testing;

namespace
{

class MockSurfaceImpl : public rx::SurfaceImpl
{
  public:
    MockSurfaceImpl() : SurfaceImpl(mockState), mockState({1}, nullptr, egl::AttributeMap()) {}
    virtual ~MockSurfaceImpl() { destructor(); }

    MOCK_METHOD1(destroy, void(const egl::Display *));
    MOCK_METHOD1(initialize, egl::Error(const egl::Display *));
    MOCK_METHOD1(swap, egl::Error(const gl::Context *));
    MOCK_METHOD3(swapWithDamage, egl::Error(const gl::Context *, const EGLint *, EGLint));
    MOCK_METHOD5(postSubBuffer, egl::Error(const gl::Context *, EGLint, EGLint, EGLint, EGLint));
    MOCK_METHOD2(querySurfacePointerANGLE, egl::Error(EGLint, void **));
    MOCK_METHOD3(bindTexImage, egl::Error(const gl::Context *context, gl::Texture *, EGLint));
    MOCK_METHOD2(releaseTexImage, egl::Error(const gl::Context *context, EGLint));
    MOCK_METHOD3(getSyncValues, egl::Error(EGLuint64KHR *, EGLuint64KHR *, EGLuint64KHR *));
    MOCK_METHOD2(getMscRate, egl::Error(EGLint *, EGLint *));
    MOCK_METHOD2(setSwapInterval, void(const egl::Display *, EGLint));
    MOCK_CONST_METHOD0(getWidth, EGLint());
    MOCK_CONST_METHOD0(getHeight, EGLint());
    MOCK_CONST_METHOD0(isPostSubBufferSupported, EGLint(void));
    MOCK_CONST_METHOD0(getSwapBehavior, EGLint(void));
    MOCK_METHOD5(getAttachmentRenderTarget,
                 angle::Result(const gl::Context *,
                               GLenum,
                               const gl::ImageIndex &,
                               GLsizei,
                               rx::FramebufferAttachmentRenderTarget **));
    MOCK_METHOD2(attachToFramebuffer, egl::Error(const gl::Context *, gl::Framebuffer *));
    MOCK_METHOD2(detachFromFramebuffer, egl::Error(const gl::Context *, gl::Framebuffer *));
    MOCK_METHOD0(destructor, void());

    egl::SurfaceState mockState;
};

TEST(SurfaceTest, DestructionDeletesImpl)
{
    NiceMock<MockEGLFactory> factory;

    MockSurfaceImpl *impl = new MockSurfaceImpl;
    EXPECT_CALL(factory, createWindowSurface(_, _, _)).WillOnce(Return(impl));

    egl::Config config;
    egl::Surface *surface = new egl::WindowSurface(
        &factory, {1}, &config, static_cast<EGLNativeWindowType>(0), egl::AttributeMap(), false);

    EXPECT_CALL(*impl, destroy(_)).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*impl, destructor()).Times(1).RetiresOnSaturation();

    EXPECT_FALSE(surface->onDestroy(nullptr).isError());

    // Only needed because the mock is leaked if bugs are present,
    // which logs an error, but does not cause the test to fail.
    // Ordinarily mocks are verified when destroyed.
    Mock::VerifyAndClear(impl);
}

}  // namespace
