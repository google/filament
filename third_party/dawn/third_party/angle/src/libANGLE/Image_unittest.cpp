//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image_unittest.cpp : Unittets of the Image and ImageSibling classes.

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "libANGLE/Image.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Texture.h"
#include "libANGLE/renderer/ImageImpl_mock.h"
#include "libANGLE/renderer/RenderbufferImpl_mock.h"
#include "libANGLE/renderer/TextureImpl_mock.h"
#include "tests/angle_unittests_utils.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace angle
{
ACTION(CreateMockImageImpl)
{
    return new rx::MockImageImpl(arg0);
}

// Verify ref counts are maintained between images and their siblings when objects are deleted
TEST(ImageTest, RefCounting)
{
    NiceMock<rx::MockGLFactory> mockGLFactory;
    NiceMock<rx::MockEGLFactory> mockEGLFactory;

    // Create a texture and an EGL image that uses the texture as its source
    rx::MockTextureImpl *textureImpl = new rx::MockTextureImpl();
    EXPECT_CALL(mockGLFactory, createTexture(_)).WillOnce(Return(textureImpl));
    gl::Texture *texture = new gl::Texture(&mockGLFactory, {1}, gl::TextureType::_2D);
    texture->addRef();

    EXPECT_CALL(mockEGLFactory, createImage(_, _, _, _))
        .WillOnce(CreateMockImageImpl())
        .RetiresOnSaturation();

    egl::Image *image = new egl::Image(&mockEGLFactory, {1}, nullptr, EGL_GL_TEXTURE_2D, texture,
                                       egl::AttributeMap());
    rx::MockImageImpl *imageImpl = static_cast<rx::MockImageImpl *>(image->getImplementation());
    image->addRef();

    // Verify that the image does not add a ref to its source so that the source may still be
    // deleted
    EXPECT_EQ(1u, texture->getRefCount());
    EXPECT_EQ(1u, image->getRefCount());

    // Create a renderbuffer and set it as a target of the EGL image
    rx::MockRenderbufferImpl *renderbufferImpl = new rx::MockRenderbufferImpl();
    EXPECT_CALL(mockGLFactory, createRenderbuffer(_)).WillOnce(Return(renderbufferImpl));
    gl::Renderbuffer *renderbuffer = new gl::Renderbuffer(&mockGLFactory, {1});
    renderbuffer->addRef();

    EXPECT_CALL(*renderbufferImpl, setStorageEGLImageTarget(_, _))
        .WillOnce(Return(angle::Result::Continue))
        .RetiresOnSaturation();
    EXPECT_EQ(angle::Result::Continue, renderbuffer->setStorageEGLImageTarget(nullptr, image));

    // Verify that the renderbuffer added a ref to the image and the image did not add a ref to
    // the renderbuffer
    EXPECT_EQ(1u, texture->getRefCount());
    EXPECT_EQ(2u, image->getRefCount());
    EXPECT_EQ(1u, renderbuffer->getRefCount());

    // Simulate deletion of the texture and verify that it is deleted but the image still exists
    EXPECT_CALL(*imageImpl, orphan(_, _))
        .WillOnce(Return(angle::Result::Continue))
        .RetiresOnSaturation();
    EXPECT_CALL(*textureImpl, destructor()).Times(1).RetiresOnSaturation();
    texture->release(nullptr);
    EXPECT_EQ(2u, image->getRefCount());
    EXPECT_EQ(1u, renderbuffer->getRefCount());

    // Simulate deletion of the image and verify that it still exists because the renderbuffer holds
    // a ref
    image->release(nullptr);
    EXPECT_EQ(1u, image->getRefCount());
    EXPECT_EQ(1u, renderbuffer->getRefCount());

    // Simulate deletion of the renderbuffer and verify that the deletion cascades to all objects
    EXPECT_CALL(*imageImpl, destructor()).Times(1).RetiresOnSaturation();
    EXPECT_CALL(*imageImpl, orphan(_, _))
        .WillOnce(Return(angle::Result::Continue))
        .RetiresOnSaturation();

    EXPECT_CALL(*renderbufferImpl, destructor()).Times(1).RetiresOnSaturation();

    renderbuffer->release(nullptr);
}

// Verify that respecifying textures releases references to the Image.
TEST(ImageTest, RespecificationReleasesReferences)
{
    NiceMock<rx::MockGLFactory> mockGLFactory;
    NiceMock<rx::MockEGLFactory> mockEGLFactory;

    // Create a texture and an EGL image that uses the texture as its source
    rx::MockTextureImpl *textureImpl = new rx::MockTextureImpl();
    EXPECT_CALL(mockGLFactory, createTexture(_)).WillOnce(Return(textureImpl));
    gl::Texture *texture = new gl::Texture(&mockGLFactory, {1}, gl::TextureType::_2D);
    texture->addRef();

    gl::PixelUnpackState defaultUnpackState;

    EXPECT_CALL(*textureImpl, setImage(_, _, _, _, _, _, _, _, _))
        .WillOnce(Return(angle::Result::Continue))
        .RetiresOnSaturation();
    EXPECT_EQ(
        angle::Result::Continue,
        texture->setImage(nullptr, defaultUnpackState, nullptr, gl::TextureTarget::_2D, 0, GL_RGBA8,
                          gl::Extents(1, 1, 1), GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    EXPECT_CALL(mockEGLFactory, createImage(_, _, _, _))
        .WillOnce(CreateMockImageImpl())
        .RetiresOnSaturation();

    egl::Image *image = new egl::Image(&mockEGLFactory, {1}, nullptr, EGL_GL_TEXTURE_2D, texture,
                                       egl::AttributeMap());
    image->addRef();

    // Verify that the image did not add a ref to it's source.
    EXPECT_EQ(1u, texture->getRefCount());
    EXPECT_EQ(1u, image->getRefCount());

    // Respecify the texture and verify that the image is orphaned
    rx::MockImageImpl *imageImpl = static_cast<rx::MockImageImpl *>(image->getImplementation());
    EXPECT_CALL(*imageImpl, orphan(_, _))
        .WillOnce(Return(angle::Result::Continue))
        .RetiresOnSaturation();
    EXPECT_CALL(*textureImpl, setImage(_, _, _, _, _, _, _, _, _))
        .WillOnce(Return(angle::Result::Continue))
        .RetiresOnSaturation();

    EXPECT_EQ(
        angle::Result::Continue,
        texture->setImage(nullptr, defaultUnpackState, nullptr, gl::TextureTarget::_2D, 0, GL_RGBA8,
                          gl::Extents(1, 1, 1), GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    EXPECT_EQ(1u, texture->getRefCount());
    EXPECT_EQ(1u, image->getRefCount());

    // Delete the texture and verify that the image still exists
    EXPECT_CALL(*textureImpl, destructor()).Times(1).RetiresOnSaturation();
    texture->release(nullptr);

    EXPECT_EQ(1u, image->getRefCount());

    // Delete the image
    EXPECT_CALL(*imageImpl, destructor()).Times(1).RetiresOnSaturation();
    image->release(nullptr);
}
}  // namespace angle
