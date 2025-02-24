//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DeviceWgpu.h:
//    Defines the class interface for DeviceWgpu, implementing DeviceImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_DEVICEWGPU_H_
#define LIBANGLE_RENDERER_WGPU_DEVICEWGPU_H_

#include "libANGLE/renderer/DeviceImpl.h"
#include "libANGLE/renderer/wgpu/wgpu_helpers.h"

namespace rx
{

class DeviceWgpu : public DeviceImpl
{
  public:
    DeviceWgpu();
    ~DeviceWgpu() override;

    egl::Error initialize() override;
    egl::Error getAttribute(const egl::Display *display,
                            EGLint attribute,
                            void **outValue) override;
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_DEVICEWGPU_H_
