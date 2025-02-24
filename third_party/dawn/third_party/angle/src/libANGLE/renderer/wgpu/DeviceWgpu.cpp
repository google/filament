//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DeviceWgpu.cpp:
//    Implements the class methods for DeviceWgpu.
//

#include "libANGLE/renderer/wgpu/DeviceWgpu.h"

#include "common/debug.h"

namespace rx
{

DeviceWgpu::DeviceWgpu() : DeviceImpl() {}

DeviceWgpu::~DeviceWgpu() {}

egl::Error DeviceWgpu::initialize()
{
    return egl::NoError();
}

egl::Error DeviceWgpu::getAttribute(const egl::Display *display, EGLint attribute, void **outValue)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

void DeviceWgpu::generateExtensions(egl::DeviceExtensions *outExtensions) const {}

}  // namespace rx
