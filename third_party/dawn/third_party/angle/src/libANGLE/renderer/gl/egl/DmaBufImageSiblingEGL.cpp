//
// Copyright The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DmaBufImageSiblingEGL.cpp: Defines the DmaBufImageSiblingEGL to wrap EGL images
// created from dma_buf objects

#include "libANGLE/renderer/gl/egl/DmaBufImageSiblingEGL.h"

#include "common/linux/dma_buf_utils.h"

namespace rx
{
DmaBufImageSiblingEGL::DmaBufImageSiblingEGL(const egl::AttributeMap &attribs)
    : mAttribs(attribs), mFormat(GL_NONE), mYUV(false), mHasProtectedContent(false)
{
    ASSERT(mAttribs.contains(EGL_WIDTH));
    mSize.width = mAttribs.getAsInt(EGL_WIDTH);
    ASSERT(mAttribs.contains(EGL_HEIGHT));
    mSize.height         = mAttribs.getAsInt(EGL_HEIGHT);
    mSize.depth          = 1;
    mHasProtectedContent = false;

    int fourCCFormat      = mAttribs.getAsInt(EGL_LINUX_DRM_FOURCC_EXT);
    GLenum internalFormat = angle::DrmFourCCFormatToGLInternalFormat(fourCCFormat, &mYUV);
    // These ANGLE formats are used exclusively with the Vulkan backend.
    if (internalFormat == GL_RGBX8_ANGLE || internalFormat == GL_BGRX8_ANGLEX)
    {
        internalFormat = GL_RGB8;
    }
    mFormat = gl::Format(internalFormat);
}

DmaBufImageSiblingEGL::~DmaBufImageSiblingEGL() {}

egl::Error DmaBufImageSiblingEGL::initialize(const egl::Display *display)
{
    return egl::NoError();
}

gl::Format DmaBufImageSiblingEGL::getFormat() const
{
    return mFormat;
}

bool DmaBufImageSiblingEGL::isRenderable(const gl::Context *context) const
{
    return true;
}

bool DmaBufImageSiblingEGL::isTexturable(const gl::Context *context) const
{
    return true;
}

bool DmaBufImageSiblingEGL::isYUV() const
{
    return mYUV;
}

bool DmaBufImageSiblingEGL::hasProtectedContent() const
{
    return mHasProtectedContent;
}

gl::Extents DmaBufImageSiblingEGL::getSize() const
{
    return mSize;
}

size_t DmaBufImageSiblingEGL::getSamples() const
{
    return 0;
}

EGLClientBuffer DmaBufImageSiblingEGL::getBuffer() const
{
    return nullptr;
}

void DmaBufImageSiblingEGL::getImageCreationAttributes(std::vector<EGLint> *outAttributes) const
{
    EGLenum kForwardedAttribs[] = {EGL_WIDTH,
                                   EGL_HEIGHT,
                                   EGL_PROTECTED_CONTENT_EXT,
                                   EGL_LINUX_DRM_FOURCC_EXT,
                                   EGL_DMA_BUF_PLANE0_FD_EXT,
                                   EGL_DMA_BUF_PLANE0_OFFSET_EXT,
                                   EGL_DMA_BUF_PLANE0_PITCH_EXT,
                                   EGL_DMA_BUF_PLANE1_FD_EXT,
                                   EGL_DMA_BUF_PLANE1_OFFSET_EXT,
                                   EGL_DMA_BUF_PLANE1_PITCH_EXT,
                                   EGL_DMA_BUF_PLANE2_FD_EXT,
                                   EGL_DMA_BUF_PLANE2_OFFSET_EXT,
                                   EGL_DMA_BUF_PLANE2_PITCH_EXT,
                                   EGL_YUV_COLOR_SPACE_HINT_EXT,
                                   EGL_SAMPLE_RANGE_HINT_EXT,
                                   EGL_YUV_CHROMA_HORIZONTAL_SITING_HINT_EXT,
                                   EGL_YUV_CHROMA_VERTICAL_SITING_HINT_EXT,

                                   EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT,
                                   EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT,
                                   EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT,
                                   EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT,
                                   EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT,
                                   EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT,
                                   EGL_DMA_BUF_PLANE3_FD_EXT,
                                   EGL_DMA_BUF_PLANE3_OFFSET_EXT,
                                   EGL_DMA_BUF_PLANE3_PITCH_EXT,
                                   EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT,
                                   EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT};

    for (EGLenum forwardedAttrib : kForwardedAttribs)
    {
        if (mAttribs.contains(forwardedAttrib))
        {
            outAttributes->push_back(forwardedAttrib);
            outAttributes->push_back(mAttribs.getAsInt(forwardedAttrib));
        }
    }
}

}  // namespace rx
