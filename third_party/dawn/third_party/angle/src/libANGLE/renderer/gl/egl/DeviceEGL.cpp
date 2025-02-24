//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DeviceEGL.cpp:
//    Implements the class methods for DeviceEGL.
//

#include "libANGLE/renderer/gl/egl/DeviceEGL.h"

#include <stdint.h>

#include "common/debug.h"
#include "common/string_utils.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/gl/egl/DisplayEGL.h"
#include "libANGLE/renderer/gl/egl/FunctionsEGL.h"

namespace rx
{

DeviceEGL::DeviceEGL(DisplayEGL *display) : mDisplay(display) {}

DeviceEGL::~DeviceEGL() {}

egl::Error DeviceEGL::initialize()
{
    if (mDisplay->getFunctionsEGL()->hasExtension("EGL_EXT_device_query") &&
        mDisplay->getFunctionsEGL()->queryDisplayAttribEXT(EGL_DEVICE_EXT, (EGLAttrib *)&mDevice))
    {
        const char *extensions =
            mDisplay->getFunctionsEGL()->queryDeviceStringEXT(mDevice, EGL_EXTENSIONS);
        if (extensions != nullptr)
        {
            angle::SplitStringAlongWhitespace(extensions, &mExtensions);
        }
    }

    return egl::NoError();
}

egl::Error DeviceEGL::getAttribute(const egl::Display *display, EGLint attribute, void **outValue)
{
    UNREACHABLE();
    return egl::EglBadAttribute();
}

void DeviceEGL::generateExtensions(egl::DeviceExtensions *outExtensions) const
{
    if (hasExtension("EGL_EXT_device_drm"))
    {
        outExtensions->deviceDrmEXT = true;
    }

    if (hasExtension("EGL_EXT_device_drm_render_node"))
    {
        outExtensions->deviceDrmRenderNodeEXT = true;
    }
}

const std::string DeviceEGL::getDeviceString(EGLint name)
{
    switch (name)
    {
        case EGL_DRM_DEVICE_FILE_EXT:
        case EGL_DRM_RENDER_NODE_FILE_EXT:
        {
            // eglQueryDeviceStringEXT can return nullptr if there is no render
            // node attached to egl device.
            const char *str = mDisplay->getFunctionsEGL()->queryDeviceStringEXT(mDevice, name);
            return str ? std::string(str) : std::string();
        }
        default:
            UNREACHABLE();
            return std::string();
    }
}

bool DeviceEGL::hasExtension(const char *extension) const
{
    return std::find(mExtensions.begin(), mExtensions.end(), extension) != mExtensions.end();
}

}  // namespace rx
