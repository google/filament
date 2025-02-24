//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLDeviceCL.h: Defines the class interface for CLDeviceCL, implementing CLDeviceImpl.

#ifndef LIBANGLE_RENDERER_CL_CLDEVICECL_H_
#define LIBANGLE_RENDERER_CL_CLDEVICECL_H_

#include "libANGLE/renderer/CLDeviceImpl.h"

namespace rx
{

class CLDeviceCL : public CLDeviceImpl
{
  public:
    ~CLDeviceCL() override;

    cl_device_id getNative() const;

    Info createInfo(cl::DeviceType type) const override;

    angle::Result getInfoUInt(cl::DeviceInfo name, cl_uint *value) const override;
    angle::Result getInfoULong(cl::DeviceInfo name, cl_ulong *value) const override;
    angle::Result getInfoSizeT(cl::DeviceInfo name, size_t *value) const override;
    angle::Result getInfoStringLength(cl::DeviceInfo name, size_t *value) const override;
    angle::Result getInfoString(cl::DeviceInfo name, size_t size, char *value) const override;

    angle::Result createSubDevices(const cl_device_partition_property *properties,
                                   cl_uint numDevices,
                                   CreateFuncs &createFuncs,
                                   cl_uint *numDevicesRet) override;

  private:
    CLDeviceCL(const cl::Device &device, cl_device_id native);

    const cl_device_id mNative;

    friend class CLPlatformCL;
};

inline cl_device_id CLDeviceCL::getNative() const
{
    return mNative;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CL_CLDEVICECL_H_
