//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkNull.h:
//    Defines the class interface for DisplayVkNull, implementing
//    DisplayVk that doesn't depend on Vulkan extensions. It cannot be
//    used to output any content to a surface.
//

#ifndef LIBANGLE_RENDERER_VULKAN_NULL_DISPLAYVKNULL_H_
#define LIBANGLE_RENDERER_VULKAN_NULL_DISPLAYVKNULL_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"

namespace rx
{

class DisplayVkNull : public DisplayVk
{
  public:
    DisplayVkNull(const egl::DisplayState &state);

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    virtual const char *getWSIExtension() const override;
    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

  private:
    std::vector<VkSurfaceFormatKHR> mSurfaceFormats;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_NULL_DISPLAYVKNULL_H_
