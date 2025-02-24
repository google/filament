//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkHeadless.h:
//    Defines the class interface for DisplayVkHeadless, implementing
//    DisplayVk for Linux via VK_EXT_headless_surface.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DISPLAY_DISPLAYVKHEADLESS_H_
#define LIBANGLE_RENDERER_VULKAN_DISPLAY_DISPLAYVKHEADLESS_H_

#include "libANGLE/renderer/vulkan/linux/DisplayVkLinux.h"

namespace rx
{

class DisplayVkHeadless : public DisplayVkLinux
{
  public:
    DisplayVkHeadless(const egl::DisplayState &state);
    void terminate() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    const char *getWSIExtension() const override;

  private:
    std::vector<VkSurfaceFormatKHR> mSurfaceFormats;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DISPLAY_DISPLAYVKHEADLESS_H_
