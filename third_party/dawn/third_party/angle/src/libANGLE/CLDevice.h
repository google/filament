//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLDevice.h: Defines the cl::Device class, which provides information about OpenCL device
// configurations.

#ifndef LIBANGLE_CLDEVICE_H_
#define LIBANGLE_CLDEVICE_H_

#include "libANGLE/CLObject.h"
#include "libANGLE/renderer/CLDeviceImpl.h"

#include "common/SynchronizedValue.h"

#include <functional>

namespace cl
{

class Device final : public _cl_device_id, public Object
{
  public:
    // Front end entry functions, only called from OpenCL entry points

    angle::Result getInfo(DeviceInfo name,
                          size_t valueSize,
                          void *value,
                          size_t *valueSizeRet) const;

    angle::Result createSubDevices(const cl_device_partition_property *properties,
                                   cl_uint numDevices,
                                   cl_device_id *subDevices,
                                   cl_uint *numDevicesRet);

  public:
    ~Device() override;

    Platform &getPlatform() noexcept;
    const Platform &getPlatform() const noexcept;
    bool isRoot() const noexcept;
    const rx::CLDeviceImpl::Info &getInfo() const;
    cl_version getVersion() const;
    bool isVersionOrNewer(cl_uint major, cl_uint minor) const;

    template <typename T = rx::CLDeviceImpl>
    T &getImpl() const;

    bool supportsBuiltInKernel(const std::string &name) const;
    bool supportsNativeImageDimensions(const cl_image_desc &desc) const;
    bool supportsImageDimensions(const ImageDescriptor &desc) const;
    bool hasDeviceEnqueueCaps() const;
    bool supportsNonUniformWorkGroups() const;

    static bool IsValidType(DeviceType type);

  private:
    Device(Platform &platform,
           Device *parent,
           DeviceType type,
           const rx::CLDeviceImpl::CreateFunc &createFunc);

    Platform &mPlatform;
    const DevicePtr mParent;
    const rx::CLDeviceImpl::Ptr mImpl;
    const rx::CLDeviceImpl::Info mInfo;

    angle::SynchronizedValue<CommandQueue *> mDefaultCommandQueue = nullptr;

    friend class CommandQueue;
    friend class Platform;
};

inline Platform &Device::getPlatform() noexcept
{
    return mPlatform;
}

inline const Platform &Device::getPlatform() const noexcept
{
    return mPlatform;
}

inline bool Device::isRoot() const noexcept
{
    return mParent == nullptr;
}

inline const rx::CLDeviceImpl::Info &Device::getInfo() const
{
    return mInfo;
}

inline cl_version Device::getVersion() const
{
    return mInfo.version;
}

inline bool Device::isVersionOrNewer(cl_uint major, cl_uint minor) const
{
    return mInfo.version >= CL_MAKE_VERSION(major, minor, 0u);
}

template <typename T>
inline T &Device::getImpl() const
{
    return static_cast<T &>(*mImpl);
}

inline bool Device::IsValidType(DeviceType type)
{
    return type.get() <= CL_DEVICE_TYPE_CUSTOM || type == CL_DEVICE_TYPE_ALL;
}

}  // namespace cl

#endif  // LIBANGLE_CLDEVICE_H_
