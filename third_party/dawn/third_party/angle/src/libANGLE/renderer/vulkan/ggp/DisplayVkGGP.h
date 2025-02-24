//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkGGP.h:
//    Google Game Platform implementation of DisplayVk.
//

#ifndef LIBANGLE_RENDERER_VULKAN_GGP_DISPLAYVKGGP_H_
#define LIBANGLE_RENDERER_VULKAN_GGP_DISPLAYVKGGP_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"

namespace rx
{
class DisplayVkGGP : public DisplayVk
{
  public:
    DisplayVkGGP(const egl::DisplayState &state);

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    const char *getWSIExtension() const override;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_GGP_DISPLAYVKGGP_H_
