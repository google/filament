//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DeviceVkLinux.cpp:
//    Implements the class methods for DeviceVkLinux.
//

#include "libANGLE/renderer/vulkan/linux/DeviceVkLinux.h"

#include <unistd.h>

#include "common/debug.h"
#include "common/vulkan/vulkan_icd.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{

DeviceVkLinux::DeviceVkLinux(DisplayVk *display) : mDisplay(display) {}

egl::Error DeviceVkLinux::initialize()
{
    vk::Renderer *renderer                         = mDisplay->getRenderer();
    VkPhysicalDeviceDrmPropertiesEXT drmProperties = renderer->getPhysicalDeviceDrmProperties();

    // Unfortunately `VkPhysicalDeviceDrmPropertiesEXT` doesn't give us the information about the
    // filesystem layout needed by the EGL version. As ChromeOS/Exo is currently the only user,
    // implement the extension only for Linux where we can reasonably assume `/dev/dri/...` file
    // paths.
    if (drmProperties.hasPrimary)
    {
        char deviceName[50];
        const long long primaryMinor = drmProperties.primaryMinor;
        snprintf(deviceName, sizeof(deviceName), "/dev/dri/card%lld", primaryMinor);

        if (access(deviceName, F_OK) != -1)
        {
            mDrmDevice = deviceName;
        }
    }
    if (drmProperties.hasRender)
    {
        char deviceName[50];
        const long long renderMinor = drmProperties.renderMinor;
        snprintf(deviceName, sizeof(deviceName), "/dev/dri/renderD%lld", renderMinor);

        if (access(deviceName, F_OK) != -1)
        {
            mDrmRenderNode = deviceName;
        }
    }

    if (mDrmDevice.empty() && !mDrmRenderNode.empty())
    {
        mDrmDevice = mDrmRenderNode;
    }

    return egl::NoError();
}

void DeviceVkLinux::generateExtensions(egl::DeviceExtensions *outExtensions) const
{
    DeviceVk::generateExtensions(outExtensions);

    if (!mDrmDevice.empty())
    {
        outExtensions->deviceDrmEXT = true;
    }
    if (!mDrmRenderNode.empty())
    {
        outExtensions->deviceDrmRenderNodeEXT = true;
    }
}

const std::string DeviceVkLinux::getDeviceString(EGLint name)
{
    switch (name)
    {
        case EGL_DRM_DEVICE_FILE_EXT:
            return mDrmDevice;
        case EGL_DRM_RENDER_NODE_FILE_EXT:
            return mDrmRenderNode;
        default:
            UNIMPLEMENTED();
            return std::string();
    }
}

}  // namespace rx
