//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceMtl: Metal implementation of egl::Device

#ifndef LIBANGLE_RENDERER_METAL_DEVICEMTL_H_
#define LIBANGLE_RENDERER_METAL_DEVICEMTL_H_

#include "libANGLE/Device.h"
#include "libANGLE/renderer/DeviceImpl.h"

namespace rx
{

// DeviceMTL implementation, implements DeviceImpl
class DeviceMtl : public DeviceImpl
{
  public:
    DeviceMtl();
    ~DeviceMtl() override;

    egl::Error initialize() override;
    egl::Error getAttribute(const egl::Display *display,
                            EGLint attribute,
                            void **outValue) override;
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_METAL_DEVICEMTL_H_
