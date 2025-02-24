//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DeviceEGL.h:
//    Defines the class interface for DeviceEGL, implementing DeviceImpl.
//

#ifndef LIBANGLE_RENDERER_GL_EGL__DEVICEEGL_H_
#define LIBANGLE_RENDERER_GL_EGL__DEVICEEGL_H_

#include "libANGLE/renderer/DeviceImpl.h"

namespace rx
{

class DisplayEGL;

class DeviceEGL : public DeviceImpl
{
  public:
    DeviceEGL(DisplayEGL *display);
    ~DeviceEGL() override;

    egl::Error initialize() override;
    egl::Error getAttribute(const egl::Display *display,
                            EGLint attribute,
                            void **outValue) override;
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override;
    const std::string getDeviceString(EGLint name) override;

  private:
    bool hasExtension(const char *extension) const;

    DisplayEGL *mDisplay;
    EGLDeviceEXT mDevice;
    std::vector<std::string> mExtensions;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL__DEVICEEGL_H_
