//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkGbm.h:
//    Defines the class interface for DisplayVkGbm, implementing DisplayVk for GBM.
//

#ifndef LIBANGLE_RENDERER_VULKAN_GBM_DISPLAYVKGBM_H_
#define LIBANGLE_RENDERER_VULKAN_GBM_DISPLAYVKGBM_H_

#include "libANGLE/renderer/vulkan/linux/DisplayVkLinux.h"

struct gbm_device;

namespace rx
{

class DisplayVkGbm : public DisplayVkLinux
{
  public:
    DisplayVkGbm(const egl::DisplayState &state);

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    const char *getWSIExtension() const override;
    angle::NativeWindowSystem getWindowSystem() const override;

  private:
    gbm_device *mGbmDevice;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_GBM_DISPLAYVKGBM_H_
