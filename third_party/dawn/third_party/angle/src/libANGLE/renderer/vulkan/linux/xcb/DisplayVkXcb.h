//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkXcb.h:
//    Defines the class interface for DisplayVkXcb, implementing DisplayVk for X via XCB.
//

#ifndef LIBANGLE_RENDERER_VULKAN_XCB_DISPLAYVKXCB_H_
#define LIBANGLE_RENDERER_VULKAN_XCB_DISPLAYVKXCB_H_

#include "libANGLE/renderer/vulkan/linux/DisplayVkLinux.h"

struct xcb_connection_t;

namespace rx
{

class DisplayVkXcb : public DisplayVkLinux
{
  public:
    DisplayVkXcb(const egl::DisplayState &state);

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    const char *getWSIExtension() const override;
    angle::Result waitNativeImpl() override;

    angle::NativeWindowSystem getWindowSystem() const override
    {
        return angle::NativeWindowSystem::X11;
    }

  private:
    xcb_connection_t *mXcbConnection;
    // If there is no X Display, obviously it's impossible to connect to it with Xcb,
    // so rendering to windows is not supported, but rendering to pbuffers is still supported.
    // This mode is used in headless ozone testing.
    bool mHasXDisplay;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_XCB_DISPLAYVKXCB_H_
