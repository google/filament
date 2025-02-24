//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkFuchsia.h:
//    Subclasses DisplayVk for the Fuchsia platform.
//

#ifndef LIBANGLE_RENDERER_VULKAN_FUCHSIA_DISPLAYVKFUCHSIA_H_
#define LIBANGLE_RENDERER_VULKAN_FUCHSIA_DISPLAYVKFUCHSIA_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"

namespace rx
{

class DisplayVkFuchsia : public DisplayVk
{
  public:
    DisplayVkFuchsia(const egl::DisplayState &state);

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    const char *getWSIExtension() const override;
    const char *getWSILayer() const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_FUCHSIA_DISPLAYVKFUCHSIA_H_
