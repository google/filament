//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkWin32.h:
//    Defines the class interface for DisplayVkWin32, implementing DisplayVk for Windows.
//

#ifndef LIBANGLE_RENDERER_VULKAN_WIN32_DISPLAYVKWIN32_H_
#define LIBANGLE_RENDERER_VULKAN_WIN32_DISPLAYVKWIN32_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"

namespace rx
{
class DisplayVkWin32 : public DisplayVk
{
  public:
    DisplayVkWin32(const egl::DisplayState &state);
    ~DisplayVkWin32() override;

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    const char *getWSIExtension() const override;

  private:
    ATOM mWindowClass;
    HWND mMockWindow;
    std::vector<VkSurfaceFormatKHR> mSurfaceFormats;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_WIN32_DISPLAYVKWIN32_H_
