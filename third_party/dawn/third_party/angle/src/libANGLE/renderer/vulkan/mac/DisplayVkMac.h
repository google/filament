//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkMac.h:
//    Subclasses DisplayVk for the Mac platform.
//

#ifndef LIBANGLE_RENDERER_VULKAN_MAC_DISPLAYVKMAC_H_
#define LIBANGLE_RENDERER_VULKAN_MAC_DISPLAYVKMAC_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"

namespace rx
{

class DisplayVkMac : public DisplayVk
{
  public:
    DisplayVkMac(const egl::DisplayState &state);

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    SurfaceImpl *createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                               EGLenum buftype,
                                               EGLClientBuffer clientBuffer,
                                               const egl::AttributeMap &attribs) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;

    egl::Error validateClientBuffer(const egl::Config *configuration,
                                    EGLenum buftype,
                                    EGLClientBuffer clientBuffer,
                                    const egl::AttributeMap &attribs) const override;

    const char *getWSIExtension() const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_MAC_DISPLAYVKMAC_H_
