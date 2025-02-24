//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DeviceNULL.cpp:
//    Implements the class methods for DeviceNULL.
//

#include "libANGLE/renderer/null/DeviceNULL.h"

#include "common/debug.h"

namespace rx
{

DeviceNULL::DeviceNULL() : DeviceImpl() {}

DeviceNULL::~DeviceNULL() {}

egl::Error DeviceNULL::initialize()
{
    return egl::NoError();
}

egl::Error DeviceNULL::getAttribute(const egl::Display *display, EGLint attribute, void **outValue)
{
    UNIMPLEMENTED();
    return egl::EglBadAccess();
}

void DeviceNULL::generateExtensions(egl::DeviceExtensions *outExtensions) const {}

}  // namespace rx
