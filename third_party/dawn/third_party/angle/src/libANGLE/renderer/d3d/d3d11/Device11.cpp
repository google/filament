//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Device11.cpp: D3D11 implementation of egl::Device

#include "libANGLE/renderer/d3d/d3d11/Device11.h"

#include "libANGLE/Device.h"
#include "libANGLE/Display.h"

#include <EGL/eglext.h>

namespace rx
{

Device11::Device11(void *nativeDevice)
{
    // Validate the device
    IUnknown *iunknown = static_cast<IUnknown *>(nativeDevice);

    // The QI to ID3D11Device adds a ref to the D3D11 device.
    // Deliberately don't release the ref here, so that the Device11 holds a ref to the
    // D3D11 device.
    iunknown->QueryInterface(__uuidof(ID3D11Device), reinterpret_cast<void **>(&mDevice));
}

Device11::~Device11()
{
    if (mDevice)
    {
        // Device11 holds a ref to an externally-sourced D3D11 device. We must release it.
        mDevice->Release();
        mDevice = nullptr;
    }
}

egl::Error Device11::getAttribute(const egl::Display *display, EGLint attribute, void **outValue)
{
    ASSERT(attribute == EGL_D3D11_DEVICE_ANGLE);
    *outValue = mDevice;
    return egl::NoError();
}

egl::Error Device11::initialize()
{
    if (!mDevice)
    {
        return egl::EglBadAttribute() << "Invalid D3D device passed into EGLDeviceEXT";
    }

    return egl::NoError();
}

void Device11::generateExtensions(egl::DeviceExtensions *outExtensions) const
{
    outExtensions->deviceD3D   = true;
    outExtensions->deviceD3D11 = true;
}

}  // namespace rx
