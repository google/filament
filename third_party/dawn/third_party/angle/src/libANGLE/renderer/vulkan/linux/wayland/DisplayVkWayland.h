//
// Copyright 2021-2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkWayland.h:
//    Defines the class interface for DisplayVkWayland, implementing DisplayVk for Wayland.
//

#ifndef LIBANGLE_RENDERER_VULKAN_WAYLAND_DISPLAYVKWAYLAND_H_
#define LIBANGLE_RENDERER_VULKAN_WAYLAND_DISPLAYVKWAYLAND_H_

#include "libANGLE/renderer/vulkan/linux/DisplayVkLinux.h"

struct wl_display;

namespace rx
{

class DisplayVkWayland : public DisplayVkLinux
{
  public:
    DisplayVkWayland(const egl::DisplayState &state);

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
    bool mOwnDisplay;
    wl_display *mWaylandDisplay;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_WAYLAND_DISPLAYVKWAYLAND_H_
