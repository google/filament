//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Device11.h: D3D11 implementation of egl::Device

#ifndef LIBANGLE_RENDERER_D3D_D3D11_DEVICE11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_DEVICE11_H_

#include "libANGLE/Device.h"
#include "libANGLE/renderer/DeviceImpl.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"

namespace rx
{
class Device11 : public DeviceImpl
{
  public:
    Device11(void *nativeDevice);
    ~Device11() override;

    egl::Error initialize() override;
    egl::Error getAttribute(const egl::Display *display,
                            EGLint attribute,
                            void **outValue) override;
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override;

    ID3D11Device* getDevice() const { return mDevice; }

  private:
    ID3D11Device *mDevice = nullptr;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_DEVICE11_H_
