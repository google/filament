//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLDeviceImpl.h: Defines the abstract rx::CLDeviceImpl class.

#ifndef LIBANGLE_RENDERER_CLDEVICEIMPL_H_
#define LIBANGLE_RENDERER_CLDEVICEIMPL_H_

#include "libANGLE/renderer/CLExtensions.h"

namespace rx
{

class CLDeviceImpl : angle::NonCopyable
{
  public:
    using Ptr         = std::unique_ptr<CLDeviceImpl>;
    using CreateFunc  = std::function<Ptr(const cl::Device &)>;
    using CreateFuncs = std::list<CreateFunc>;
    using CreateData  = std::pair<cl::DeviceType, CreateFunc>;
    using CreateDatas = std::list<CreateData>;

    struct Info : public CLExtensions
    {
        Info();
        explicit Info(cl::DeviceType deviceType);
        ~Info();

        Info(const Info &)            = delete;
        Info &operator=(const Info &) = delete;

        Info(Info &&);
        Info &operator=(Info &&);

        bool isValid() const { return version != 0u; }

        // In the order as they appear in the OpenCL specification V3.0.7, table 5
        cl::DeviceType type;
        std::vector<size_t> maxWorkItemSizes;
        cl_ulong maxMemAllocSize = 0u;
        cl_bool imageSupport     = CL_FALSE;
        std::string IL_Version;
        NameVersionVector ILsWithVersion;
        size_t image2D_MaxWidth           = 0u;
        size_t image2D_MaxHeight          = 0u;
        size_t image3D_MaxWidth           = 0u;
        size_t image3D_MaxHeight          = 0u;
        size_t image3D_MaxDepth           = 0u;
        size_t imageMaxBufferSize         = 0u;
        size_t imageMaxArraySize          = 0u;
        cl_uint imagePitchAlignment       = 0u;
        cl_uint imageBaseAddressAlignment = 0u;
        cl_uint memBaseAddrAlign          = 0u;
        cl::DeviceExecCapabilities execCapabilities;
        cl_uint queueOnDeviceMaxSize = 0u;
        std::string builtInKernels;
        NameVersionVector builtInKernelsWithVersion;
        NameVersionVector OpenCL_C_AllVersions;
        NameVersionVector OpenCL_C_Features;
        std::vector<cl_device_partition_property> partitionProperties;
        std::vector<cl_device_partition_property> partitionType;
    };

    CLDeviceImpl(const cl::Device &device);
    virtual ~CLDeviceImpl();

    virtual Info createInfo(cl::DeviceType type) const = 0;

    virtual angle::Result getInfoUInt(cl::DeviceInfo name, cl_uint *value) const             = 0;
    virtual angle::Result getInfoULong(cl::DeviceInfo name, cl_ulong *value) const           = 0;
    virtual angle::Result getInfoSizeT(cl::DeviceInfo name, size_t *value) const             = 0;
    virtual angle::Result getInfoStringLength(cl::DeviceInfo name, size_t *value) const      = 0;
    virtual angle::Result getInfoString(cl::DeviceInfo name, size_t size, char *value) const = 0;

    virtual angle::Result createSubDevices(const cl_device_partition_property *properties,
                                           cl_uint numDevices,
                                           CreateFuncs &createFuncs,
                                           cl_uint *numDevicesRet) = 0;

  protected:
    const cl::Device &mDevice;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLDEVICEIMPL_H_
