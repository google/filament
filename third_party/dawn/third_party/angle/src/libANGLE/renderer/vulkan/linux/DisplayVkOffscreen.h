//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkOffscreen.h:
//    Defines the class interface for DisplayVkOffscreen, implementing
//    DisplayVk for Linux when window surfaces are not supported.
//

#ifndef LIBANGLE_RENDERER_VULKAN_LINUX_DISPLAYVKOFFSCREEN_H_
#define LIBANGLE_RENDERER_VULKAN_LINUX_DISPLAYVKOFFSCREEN_H_

#include "libANGLE/renderer/vulkan/linux/DisplayVkLinux.h"

namespace rx
{

class DisplayVkOffscreen : public DisplayVkLinux
{
  public:
    DisplayVkOffscreen(const egl::DisplayState &state);
    bool isValidNativeWindow(EGLNativeWindowType window) const override;
    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;
    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;
    const char *getWSIExtension() const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_LINUX_DISPLAYVKOFFSCREEN_H_
