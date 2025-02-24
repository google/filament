//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceMtl: Metal implementation of egl::Device

#include "libANGLE/renderer/metal/DeviceMtl.h"

#include "libANGLE/Device.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"

#include <EGL/eglext.h>
namespace rx
{

// DeviceMtl implementation, implements DeviceImpl
DeviceMtl::DeviceMtl() {}
DeviceMtl::~DeviceMtl() {}

egl::Error DeviceMtl::initialize()
{
    return egl::NoError();
}

egl::Error DeviceMtl::getAttribute(const egl::Display *display, EGLint attribute, void **outValue)
{
    DisplayMtl *displayImpl = mtl::GetImpl(display);

    switch (attribute)
    {
        case EGL_METAL_DEVICE_ANGLE:
            *outValue = displayImpl->getMetalDevice();
            break;
        default:
            return egl::EglBadAttribute();
    }

    return egl::NoError();
}

void DeviceMtl::generateExtensions(egl::DeviceExtensions *outExtensions) const
{
    outExtensions->deviceMetal = true;
}

}  // namespace rx
