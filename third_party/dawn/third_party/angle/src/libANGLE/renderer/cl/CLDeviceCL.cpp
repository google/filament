//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLDeviceCL.cpp: Implements the class methods for CLDeviceCL.

#include "libANGLE/renderer/cl/CLDeviceCL.h"

#include "libANGLE/renderer/cl/cl_util.h"

#include "libANGLE/CLDevice.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

namespace
{

// Object information is queried in OpenCL by providing allocated memory into which the requested
// data is copied. If the size of the data is unknown, it can be queried first with an additional
// call to the same function, but without requesting the data itself. This function provides the
// functionality to request and validate the size and the data.
template <typename T>
bool GetDeviceInfo(cl_device_id device, cl::DeviceInfo name, std::vector<T> &vector)
{
    size_t size = 0u;
    if (device->getDispatch().clGetDeviceInfo(device, cl::ToCLenum(name), 0u, nullptr, &size) ==
            CL_SUCCESS &&
        (size % sizeof(T)) == 0u)  // size has to be a multiple of the data type
    {
        vector.resize(size / sizeof(T));
        if (device->getDispatch().clGetDeviceInfo(device, cl::ToCLenum(name), size, vector.data(),
                                                  nullptr) == CL_SUCCESS)
        {
            return true;
        }
    }
    ERR() << "Failed to query CL device info for " << name;
    return false;
}

// This queries the OpenCL device info for value types with known size
template <typename T>
bool GetDeviceInfo(cl_device_id device, cl::DeviceInfo name, T &value)
{
    if (device->getDispatch().clGetDeviceInfo(device, cl::ToCLenum(name), sizeof(T), &value,
                                              nullptr) != CL_SUCCESS)
    {
        ERR() << "Failed to query CL device info for " << name;
        return false;
    }
    return true;
}

}  // namespace

CLDeviceCL::~CLDeviceCL()
{
    if (!mDevice.isRoot() && mNative->getDispatch().clReleaseDevice(mNative) != CL_SUCCESS)
    {
        ERR() << "Error while releasing CL device";
    }
}

CLDeviceImpl::Info CLDeviceCL::createInfo(cl::DeviceType type) const
{
    Info info(type);
    std::vector<char> valString;

    if (!GetDeviceInfo(mNative, cl::DeviceInfo::MaxWorkItemSizes, info.maxWorkItemSizes))
    {
        return Info{};
    }
    // From the OpenCL specification for info name CL_DEVICE_MAX_WORK_ITEM_SIZES:
    // "The minimum value is (1, 1, 1) for devices that are not of type CL_DEVICE_TYPE_CUSTOM."
    // https://www.khronos.org/registry/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clGetDeviceInfo
    // Custom devices are currently not supported by this back end.
    if (info.maxWorkItemSizes.size() < 3u || info.maxWorkItemSizes[0] == 0u ||
        info.maxWorkItemSizes[1] == 0u || info.maxWorkItemSizes[2] == 0u)
    {
        ERR() << "Invalid CL_DEVICE_MAX_WORK_ITEM_SIZES";
        return Info{};
    }

    if (!GetDeviceInfo(mNative, cl::DeviceInfo::MaxMemAllocSize, info.maxMemAllocSize) ||
        !GetDeviceInfo(mNative, cl::DeviceInfo::ImageSupport, info.imageSupport) ||
        !GetDeviceInfo(mNative, cl::DeviceInfo::Image2D_MaxWidth, info.image2D_MaxWidth) ||
        !GetDeviceInfo(mNative, cl::DeviceInfo::Image2D_MaxHeight, info.image2D_MaxHeight) ||
        !GetDeviceInfo(mNative, cl::DeviceInfo::Image3D_MaxWidth, info.image3D_MaxWidth) ||
        !GetDeviceInfo(mNative, cl::DeviceInfo::Image3D_MaxHeight, info.image3D_MaxHeight) ||
        !GetDeviceInfo(mNative, cl::DeviceInfo::Image3D_MaxDepth, info.image3D_MaxDepth) ||
        !GetDeviceInfo(mNative, cl::DeviceInfo::MemBaseAddrAlign, info.memBaseAddrAlign) ||
        !GetDeviceInfo(mNative, cl::DeviceInfo::ExecutionCapabilities, info.execCapabilities))
    {
        return Info{};
    }

    if (!GetDeviceInfo(mNative, cl::DeviceInfo::Version, valString))
    {
        return Info{};
    }
    info.versionStr.assign(valString.data());

    if (!GetDeviceInfo(mNative, cl::DeviceInfo::Extensions, valString))
    {
        return Info{};
    }
    std::string extensionStr(valString.data());

    // TODO(jplate) Remove workaround after bug is fixed http://anglebug.com/42264583
    if (info.versionStr.compare(0u, 15u, "OpenCL 3.0 CUDA", 15u) == 0)
    {
        extensionStr.append(" cl_khr_depth_images cl_khr_image2d_from_buffer");
    }

    info.version = ExtractCLVersion(info.versionStr);
    if (info.version == 0u)
    {
        return Info{};
    }

    RemoveUnsupportedCLExtensions(extensionStr);
    info.initializeExtensions(std::move(extensionStr));

    if (info.version >= CL_MAKE_VERSION(1, 2, 0))
    {
        if (!GetDeviceInfo(mNative, cl::DeviceInfo::ImageMaxBufferSize, info.imageMaxBufferSize) ||
            !GetDeviceInfo(mNative, cl::DeviceInfo::ImageMaxArraySize, info.imageMaxArraySize) ||
            !GetDeviceInfo(mNative, cl::DeviceInfo::BuiltInKernels, valString))
        {
            return Info{};
        }
        info.builtInKernels.assign(valString.data());
        if (!GetDeviceInfo(mNative, cl::DeviceInfo::PartitionProperties,
                           info.partitionProperties) ||
            !GetDeviceInfo(mNative, cl::DeviceInfo::PartitionType, info.partitionType))
        {
            return Info{};
        }
    }

    if (info.version >= CL_MAKE_VERSION(2, 0, 0) &&
        (!GetDeviceInfo(mNative, cl::DeviceInfo::ImagePitchAlignment, info.imagePitchAlignment) ||
         !GetDeviceInfo(mNative, cl::DeviceInfo::ImageBaseAddressAlignment,
                        info.imageBaseAddressAlignment) ||
         !GetDeviceInfo(mNative, cl::DeviceInfo::QueueOnDeviceMaxSize, info.queueOnDeviceMaxSize)))
    {
        return Info{};
    }

    if (info.version >= CL_MAKE_VERSION(2, 1, 0))
    {
        if (!GetDeviceInfo(mNative, cl::DeviceInfo::IL_Version, valString))
        {
            return Info{};
        }
        info.IL_Version.assign(valString.data());
    }

    if (info.version >= CL_MAKE_VERSION(3, 0, 0) &&
        (!GetDeviceInfo(mNative, cl::DeviceInfo::ILsWithVersion, info.ILsWithVersion) ||
         !GetDeviceInfo(mNative, cl::DeviceInfo::BuiltInKernelsWithVersion,
                        info.builtInKernelsWithVersion) ||
         !GetDeviceInfo(mNative, cl::DeviceInfo::OpenCL_C_AllVersions, info.OpenCL_C_AllVersions) ||
         !GetDeviceInfo(mNative, cl::DeviceInfo::OpenCL_C_Features, info.OpenCL_C_Features) ||
         !GetDeviceInfo(mNative, cl::DeviceInfo::ExtensionsWithVersion,
                        info.extensionsWithVersion)))
    {
        return Info{};
    }
    RemoveUnsupportedCLExtensions(info.extensionsWithVersion);

    return info;
}

angle::Result CLDeviceCL::getInfoUInt(cl::DeviceInfo name, cl_uint *value) const
{
    ANGLE_CL_TRY(mNative->getDispatch().clGetDeviceInfo(mNative, cl::ToCLenum(name), sizeof(*value),
                                                        value, nullptr));
    return angle::Result::Continue;
}

angle::Result CLDeviceCL::getInfoULong(cl::DeviceInfo name, cl_ulong *value) const
{
    ANGLE_CL_TRY(mNative->getDispatch().clGetDeviceInfo(mNative, cl::ToCLenum(name), sizeof(*value),
                                                        value, nullptr));
    return angle::Result::Continue;
}

angle::Result CLDeviceCL::getInfoSizeT(cl::DeviceInfo name, size_t *value) const
{
    ANGLE_CL_TRY(mNative->getDispatch().clGetDeviceInfo(mNative, cl::ToCLenum(name), sizeof(*value),
                                                        value, nullptr));
    return angle::Result::Continue;
}

angle::Result CLDeviceCL::getInfoStringLength(cl::DeviceInfo name, size_t *value) const
{
    ANGLE_CL_TRY(
        mNative->getDispatch().clGetDeviceInfo(mNative, cl::ToCLenum(name), 0u, nullptr, value));
    return angle::Result::Continue;
}

angle::Result CLDeviceCL::getInfoString(cl::DeviceInfo name, size_t size, char *value) const
{
    ANGLE_CL_TRY(
        mNative->getDispatch().clGetDeviceInfo(mNative, cl::ToCLenum(name), size, value, nullptr));
    return angle::Result::Continue;
}

angle::Result CLDeviceCL::createSubDevices(const cl_device_partition_property *properties,
                                           cl_uint numDevices,
                                           CreateFuncs &createFuncs,
                                           cl_uint *numDevicesRet)
{
    if (numDevices == 0u)
    {
        ANGLE_CL_TRY(mNative->getDispatch().clCreateSubDevices(mNative, properties, 0u, nullptr,
                                                               numDevicesRet));
        return angle::Result::Continue;
    }

    std::vector<cl_device_id> nativeSubDevices(numDevices, nullptr);
    ANGLE_CL_TRY(mNative->getDispatch().clCreateSubDevices(mNative, properties, numDevices,
                                                           nativeSubDevices.data(), nullptr));

    for (cl_device_id nativeSubDevice : nativeSubDevices)
    {
        createFuncs.emplace_back([nativeSubDevice](const cl::Device &device) {
            return Ptr(new CLDeviceCL(device, nativeSubDevice));
        });
    }
    return angle::Result::Continue;
}

CLDeviceCL::CLDeviceCL(const cl::Device &device, cl_device_id native)
    : CLDeviceImpl(device), mNative(native)
{}

}  // namespace rx
