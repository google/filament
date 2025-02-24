//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ImageImpl.cpp: Defines the rx::ImageImpl class representing the EGLimage object.

#include "libANGLE/renderer/ImageImpl.h"

namespace rx
{

bool ExternalImageSiblingImpl::hasFrontBufferUsage() const
{
    return false;
}

bool ExternalImageSiblingImpl::isCubeMap() const
{
    return false;
}

uint32_t ExternalImageSiblingImpl::getLevelCount() const
{
    return 1;
}

egl::Error ImageImpl::exportVkImage(void *vkImage, void *vkImageCreateInfo)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

bool ImageImpl::isFixedRatedCompression(const gl::Context *context)
{
    UNIMPLEMENTED();
    return false;
}

}  // namespace rx
