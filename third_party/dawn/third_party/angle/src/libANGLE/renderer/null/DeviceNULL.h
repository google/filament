//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DeviceNULL.h:
//    Defines the class interface for DeviceNULL, implementing DeviceImpl.
//

#ifndef LIBANGLE_RENDERER_NULL_DEVICENULL_H_
#define LIBANGLE_RENDERER_NULL_DEVICENULL_H_

#include "libANGLE/renderer/DeviceImpl.h"

namespace rx
{

class DeviceNULL : public DeviceImpl
{
  public:
    DeviceNULL();
    ~DeviceNULL() override;

    egl::Error initialize() override;
    egl::Error getAttribute(const egl::Display *display,
                            EGLint attribute,
                            void **outValue) override;
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_NULL_DEVICENULL_H_
