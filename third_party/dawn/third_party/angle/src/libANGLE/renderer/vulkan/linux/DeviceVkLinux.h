//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DeviceVkLinux.h:
//    Defines the class interface for DeviceVkLinux, implementing DeviceImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DEVICEVKLINUX_H_
#define LIBANGLE_RENDERER_VULKAN_DEVICEVKLINUX_H_

#include "libANGLE/renderer/vulkan/DeviceVk.h"

namespace rx
{

class DisplayVk;

class DeviceVkLinux : public DeviceVk
{
  public:
    DeviceVkLinux(DisplayVk *display);

    egl::Error initialize() override;
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override;
    const std::string getDeviceString(EGLint name) override;

  private:
    DisplayVk *mDisplay;
    std::string mDrmDevice;
    std::string mDrmRenderNode;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DEVICEVKLINUX_H_
