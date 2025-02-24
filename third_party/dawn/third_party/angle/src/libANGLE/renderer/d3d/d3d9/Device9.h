//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Device9.h: D3D9 implementation of egl::Device

#ifndef LIBANGLE_RENDERER_D3D_D3D9_DEVICE9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_DEVICE9_H_

#include "libANGLE/Device.h"
#include "libANGLE/renderer/DeviceImpl.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"

namespace rx
{
class Device9 : public DeviceImpl
{
  public:
    Device9(IDirect3DDevice9 *device);
    ~Device9() override;

    egl::Error initialize() override;
    egl::Error getAttribute(const egl::Display *display,
                            EGLint attribute,
                            void **outValue) override;
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override;

  private:
    IDirect3DDevice9 *mDevice;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_DEVICE9_H_
