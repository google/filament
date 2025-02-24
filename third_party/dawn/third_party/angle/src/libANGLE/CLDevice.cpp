//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLDevice.cpp: Implements the cl::Device class.

#include "libANGLE/CLDevice.h"

#include "libANGLE/CLPlatform.h"
#include "libANGLE/cl_utils.h"

#include "common/string_utils.h"

#include <cstring>

namespace cl
{

angle::Result Device::getInfo(DeviceInfo name,
                              size_t valueSize,
                              void *value,
                              size_t *valueSizeRet) const
{
    static_assert(std::is_same<cl_uint, cl_bool>::value &&
                      std::is_same<cl_uint, cl_device_mem_cache_type>::value &&
                      std::is_same<cl_uint, cl_device_local_mem_type>::value &&
                      std::is_same<cl_uint, cl_version>::value &&
                      std::is_same<cl_ulong, cl_device_type>::value &&
                      std::is_same<cl_ulong, cl_device_fp_config>::value &&
                      std::is_same<cl_ulong, cl_device_exec_capabilities>::value &&
                      std::is_same<cl_ulong, cl_command_queue_properties>::value &&
                      std::is_same<cl_ulong, cl_device_affinity_domain>::value &&
                      std::is_same<cl_ulong, cl_device_svm_capabilities>::value &&
                      std::is_same<cl_ulong, cl_device_atomic_capabilities>::value &&
                      std::is_same<cl_ulong, cl_device_device_enqueue_capabilities>::value,
                  "OpenCL type mismatch");

    cl_uint valUInt   = 0u;
    cl_ulong valULong = 0u;
    size_t valSizeT   = 0u;
    void *valPointer  = nullptr;
    std::vector<char> valString;

    const void *copyValue = nullptr;
    size_t copySize       = 0u;

    // The info names are sorted within their type group in the order they appear in the OpenCL
    // specification, so it is easier to compare them side-by-side when looking for changes.
    // https://www.khronos.org/registry/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clGetDeviceInfo
    switch (name)
    {
        // Handle all cl_uint and aliased types
        case DeviceInfo::VendorID:
        case DeviceInfo::MaxComputeUnits:
        case DeviceInfo::PreferredVectorWidthChar:
        case DeviceInfo::PreferredVectorWidthShort:
        case DeviceInfo::PreferredVectorWidthInt:
        case DeviceInfo::PreferredVectorWidthLong:
        case DeviceInfo::PreferredVectorWidthFloat:
        case DeviceInfo::PreferredVectorWidthDouble:
        case DeviceInfo::PreferredVectorWidthHalf:
        case DeviceInfo::NativeVectorWidthChar:
        case DeviceInfo::NativeVectorWidthShort:
        case DeviceInfo::NativeVectorWidthInt:
        case DeviceInfo::NativeVectorWidthLong:
        case DeviceInfo::NativeVectorWidthFloat:
        case DeviceInfo::NativeVectorWidthDouble:
        case DeviceInfo::NativeVectorWidthHalf:
        case DeviceInfo::MaxClockFrequency:
        case DeviceInfo::AddressBits:
        case DeviceInfo::MaxReadImageArgs:
        case DeviceInfo::MaxWriteImageArgs:
        case DeviceInfo::MaxReadWriteImageArgs:
        case DeviceInfo::MaxSamplers:
        case DeviceInfo::MaxPipeArgs:
        case DeviceInfo::PipeMaxActiveReservations:
        case DeviceInfo::PipeMaxPacketSize:
        case DeviceInfo::MinDataTypeAlignSize:
        case DeviceInfo::GlobalMemCacheType:
        case DeviceInfo::GlobalMemCachelineSize:
        case DeviceInfo::MaxConstantArgs:
        case DeviceInfo::LocalMemType:
        case DeviceInfo::ErrorCorrectionSupport:
        case DeviceInfo::HostUnifiedMemory:
        case DeviceInfo::EndianLittle:
        case DeviceInfo::Available:
        case DeviceInfo::CompilerAvailable:
        case DeviceInfo::LinkerAvailable:
        case DeviceInfo::QueueOnDevicePreferredSize:
        case DeviceInfo::MaxOnDeviceQueues:
        case DeviceInfo::MaxOnDeviceEvents:
        case DeviceInfo::PreferredInteropUserSync:
        case DeviceInfo::PartitionMaxSubDevices:
        case DeviceInfo::PreferredPlatformAtomicAlignment:
        case DeviceInfo::PreferredGlobalAtomicAlignment:
        case DeviceInfo::PreferredLocalAtomicAlignment:
        case DeviceInfo::MaxNumSubGroups:
        case DeviceInfo::SubGroupIndependentForwardProgress:
        case DeviceInfo::NonUniformWorkGroupSupport:
        case DeviceInfo::WorkGroupCollectiveFunctionsSupport:
        case DeviceInfo::GenericAddressSpaceSupport:
        case DeviceInfo::PipeSupport:
            ANGLE_TRY(mImpl->getInfoUInt(name, &valUInt));
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;

        // Handle all cl_ulong and aliased types
        case DeviceInfo::SingleFpConfig:
        case DeviceInfo::DoubleFpConfig:
        case DeviceInfo::GlobalMemCacheSize:
        case DeviceInfo::GlobalMemSize:
        case DeviceInfo::MaxConstantBufferSize:
        case DeviceInfo::LocalMemSize:
        case DeviceInfo::QueueOnHostProperties:
        case DeviceInfo::QueueOnDeviceProperties:
        case DeviceInfo::PartitionAffinityDomain:
        case DeviceInfo::SVM_Capabilities:
        case DeviceInfo::AtomicMemoryCapabilities:
        case DeviceInfo::AtomicFenceCapabilities:
        case DeviceInfo::DeviceEnqueueCapabilities:
        case DeviceInfo::HalfFpConfig:
            ANGLE_TRY(mImpl->getInfoULong(name, &valULong));
            copyValue = &valULong;
            copySize  = sizeof(valULong);
            break;

        // Handle all size_t and aliased types
        case DeviceInfo::MaxWorkGroupSize:
        case DeviceInfo::MaxParameterSize:
        case DeviceInfo::MaxGlobalVariableSize:
        case DeviceInfo::GlobalVariablePreferredTotalSize:
        case DeviceInfo::ProfilingTimerResolution:
        case DeviceInfo::PrintfBufferSize:
        case DeviceInfo::PreferredWorkGroupSizeMultiple:
            ANGLE_TRY(mImpl->getInfoSizeT(name, &valSizeT));
            copyValue = &valSizeT;
            copySize  = sizeof(valSizeT);
            break;

        // Handle all string types
        case DeviceInfo::Name:
        case DeviceInfo::Vendor:
        case DeviceInfo::DriverVersion:
        case DeviceInfo::Profile:
        case DeviceInfo::OpenCL_C_Version:
        case DeviceInfo::LatestConformanceVersionPassed:
            ANGLE_TRY(mImpl->getInfoStringLength(name, &copySize));
            valString.resize(copySize, '\0');
            ANGLE_TRY(mImpl->getInfoString(name, copySize, valString.data()));
            copyValue = valString.data();
            break;

        // Handle all cached values
        case DeviceInfo::Type:
            copyValue = &mInfo.type;
            copySize  = sizeof(mInfo.type);
            break;
        case DeviceInfo::MaxWorkItemDimensions:
            valUInt   = static_cast<cl_uint>(mInfo.maxWorkItemSizes.size());
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case DeviceInfo::MaxWorkItemSizes:
            copyValue = mInfo.maxWorkItemSizes.data();
            copySize  = mInfo.maxWorkItemSizes.size() *
                       sizeof(decltype(mInfo.maxWorkItemSizes)::value_type);
            break;
        case DeviceInfo::MaxMemAllocSize:
            copyValue = &mInfo.maxMemAllocSize;
            copySize  = sizeof(mInfo.maxMemAllocSize);
            break;
        case DeviceInfo::ImageSupport:
            copyValue = &mInfo.imageSupport;
            copySize  = sizeof(mInfo.imageSupport);
            break;
        case DeviceInfo::IL_Version:
            copyValue = mInfo.IL_Version.c_str();
            copySize  = mInfo.IL_Version.length() + 1u;
            break;
        case DeviceInfo::ILsWithVersion:
            copyValue = mInfo.ILsWithVersion.data();
            copySize =
                mInfo.ILsWithVersion.size() * sizeof(decltype(mInfo.ILsWithVersion)::value_type);
            break;
        case DeviceInfo::Image2D_MaxWidth:
            copyValue = &mInfo.image2D_MaxWidth;
            copySize  = sizeof(mInfo.image2D_MaxWidth);
            break;
        case DeviceInfo::Image2D_MaxHeight:
            copyValue = &mInfo.image2D_MaxHeight;
            copySize  = sizeof(mInfo.image2D_MaxHeight);
            break;
        case DeviceInfo::Image3D_MaxWidth:
            copyValue = &mInfo.image3D_MaxWidth;
            copySize  = sizeof(mInfo.image3D_MaxWidth);
            break;
        case DeviceInfo::Image3D_MaxHeight:
            copyValue = &mInfo.image3D_MaxHeight;
            copySize  = sizeof(mInfo.image3D_MaxHeight);
            break;
        case DeviceInfo::Image3D_MaxDepth:
            copyValue = &mInfo.image3D_MaxDepth;
            copySize  = sizeof(mInfo.image3D_MaxDepth);
            break;
        case DeviceInfo::ImageMaxBufferSize:
            copyValue = &mInfo.imageMaxBufferSize;
            copySize  = sizeof(mInfo.imageMaxBufferSize);
            break;
        case DeviceInfo::ImageMaxArraySize:
            copyValue = &mInfo.imageMaxArraySize;
            copySize  = sizeof(mInfo.imageMaxArraySize);
            break;
        case DeviceInfo::ImagePitchAlignment:
            copyValue = &mInfo.imagePitchAlignment;
            copySize  = sizeof(mInfo.imagePitchAlignment);
            break;
        case DeviceInfo::ImageBaseAddressAlignment:
            copyValue = &mInfo.imageBaseAddressAlignment;
            copySize  = sizeof(mInfo.imageBaseAddressAlignment);
            break;
        case DeviceInfo::MemBaseAddrAlign:
            copyValue = &mInfo.memBaseAddrAlign;
            copySize  = sizeof(mInfo.memBaseAddrAlign);
            break;
        case DeviceInfo::ExecutionCapabilities:
            copyValue = &mInfo.execCapabilities;
            copySize  = sizeof(mInfo.execCapabilities);
            break;
        case DeviceInfo::QueueOnDeviceMaxSize:
            copyValue = &mInfo.queueOnDeviceMaxSize;
            copySize  = sizeof(mInfo.queueOnDeviceMaxSize);
            break;
        case DeviceInfo::BuiltInKernels:
            copyValue = mInfo.builtInKernels.c_str();
            copySize  = mInfo.builtInKernels.length() + 1u;
            break;
        case DeviceInfo::BuiltInKernelsWithVersion:
            copyValue = mInfo.builtInKernelsWithVersion.data();
            copySize  = mInfo.builtInKernelsWithVersion.size() *
                       sizeof(decltype(mInfo.builtInKernelsWithVersion)::value_type);
            break;
        case DeviceInfo::Version:
            copyValue = mInfo.versionStr.c_str();
            copySize  = mInfo.versionStr.length() + 1u;
            break;
        case DeviceInfo::NumericVersion:
            copyValue = &mInfo.version;
            copySize  = sizeof(mInfo.version);
            break;
        case DeviceInfo::OpenCL_C_AllVersions:
            copyValue = mInfo.OpenCL_C_AllVersions.data();
            copySize  = mInfo.OpenCL_C_AllVersions.size() *
                       sizeof(decltype(mInfo.OpenCL_C_AllVersions)::value_type);
            break;
        case DeviceInfo::OpenCL_C_Features:
            copyValue = mInfo.OpenCL_C_Features.data();
            copySize  = mInfo.OpenCL_C_Features.size() *
                       sizeof(decltype(mInfo.OpenCL_C_Features)::value_type);
            break;
        case DeviceInfo::Extensions:
            copyValue = mInfo.extensions.c_str();
            copySize  = mInfo.extensions.length() + 1u;
            break;
        case DeviceInfo::ExtensionsWithVersion:
            copyValue = mInfo.extensionsWithVersion.data();
            copySize  = mInfo.extensionsWithVersion.size() *
                       sizeof(decltype(mInfo.extensionsWithVersion)::value_type);
            break;
        case DeviceInfo::PartitionProperties:
            copyValue = mInfo.partitionProperties.data();
            copySize  = mInfo.partitionProperties.size() *
                       sizeof(decltype(mInfo.partitionProperties)::value_type);
            break;
        case DeviceInfo::PartitionType:
            copyValue = mInfo.partitionType.data();
            copySize =
                mInfo.partitionType.size() * sizeof(decltype(mInfo.partitionType)::value_type);
            break;

        // Handle all mapped values
        case DeviceInfo::Platform:
            valPointer = mPlatform.getNative();
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case DeviceInfo::ParentDevice:
            valPointer = Device::CastNative(mParent.get());
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case DeviceInfo::ReferenceCount:
            valUInt   = isRoot() ? 1u : getRefCount();
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;

        default:
            ASSERT(false);
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return
        // type as specified in the Device Queries table and param_value is not a NULL value
        if (valueSize < copySize)
        {
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
        }
        if (copyValue != nullptr)
        {
            std::memcpy(value, copyValue, copySize);
        }
    }
    if (valueSizeRet != nullptr)
    {
        *valueSizeRet = copySize;
    }
    return angle::Result::Continue;
}

angle::Result Device::createSubDevices(const cl_device_partition_property *properties,
                                       cl_uint numDevices,
                                       cl_device_id *subDevices,
                                       cl_uint *numDevicesRet)
{
    if (subDevices == nullptr)
    {
        numDevices = 0u;
    }
    rx::CLDeviceImpl::CreateFuncs subDeviceCreateFuncs;
    ANGLE_TRY(mImpl->createSubDevices(properties, numDevices, subDeviceCreateFuncs, numDevicesRet));
    cl::DeviceType type = mInfo.type;
    type.clear(CL_DEVICE_TYPE_DEFAULT);
    DevicePtrs devices;
    devices.reserve(subDeviceCreateFuncs.size());
    while (!subDeviceCreateFuncs.empty())
    {
        devices.emplace_back(new Device(mPlatform, this, type, subDeviceCreateFuncs.front()));
        // Release initialization reference, lifetime controlled by RefPointer.
        devices.back()->release();
        if (!devices.back()->mInfo.isValid())
        {
            return angle::Result::Stop;
        }
        subDeviceCreateFuncs.pop_front();
    }
    for (DevicePtr &subDevice : devices)
    {
        *subDevices++ = subDevice.release();
    }
    return angle::Result::Continue;
}

Device::~Device() = default;

bool Device::supportsBuiltInKernel(const std::string &name) const
{
    return angle::ContainsToken(mInfo.builtInKernels, ';', name);
}

bool Device::supportsNativeImageDimensions(const cl_image_desc &desc) const
{
    switch (FromCLenum<MemObjectType>(desc.image_type))
    {
        case MemObjectType::Image1D:
            return desc.image_width <= mInfo.image2D_MaxWidth;
        case MemObjectType::Image2D:
            return desc.image_width <= mInfo.image2D_MaxWidth &&
                   desc.image_height <= mInfo.image2D_MaxHeight;
        case MemObjectType::Image3D:
            return desc.image_width <= mInfo.image3D_MaxWidth &&
                   desc.image_height <= mInfo.image3D_MaxHeight &&
                   desc.image_depth <= mInfo.image3D_MaxDepth;
        case MemObjectType::Image1D_Array:
            return desc.image_width <= mInfo.image2D_MaxWidth &&
                   desc.image_array_size <= mInfo.imageMaxArraySize;
        case MemObjectType::Image2D_Array:
            return desc.image_width <= mInfo.image2D_MaxWidth &&
                   desc.image_height <= mInfo.image2D_MaxHeight &&
                   desc.image_array_size <= mInfo.imageMaxArraySize;
        case MemObjectType::Image1D_Buffer:
            return desc.image_width <= mInfo.imageMaxBufferSize;
        default:
            ASSERT(false);
            break;
    }
    return false;
}

bool Device::supportsImageDimensions(const ImageDescriptor &desc) const
{
    switch (desc.type)
    {
        case MemObjectType::Image1D:
            return desc.width <= mInfo.image2D_MaxWidth;
        case MemObjectType::Image2D:
            return desc.width <= mInfo.image2D_MaxWidth && desc.height <= mInfo.image2D_MaxHeight;
        case MemObjectType::Image3D:
            return desc.width <= mInfo.image3D_MaxWidth && desc.height <= mInfo.image3D_MaxHeight &&
                   desc.depth <= mInfo.image3D_MaxDepth;
        case MemObjectType::Image1D_Array:
            return desc.width <= mInfo.image2D_MaxWidth &&
                   desc.arraySize <= mInfo.imageMaxArraySize;
        case MemObjectType::Image2D_Array:
            return desc.width <= mInfo.image2D_MaxWidth && desc.height <= mInfo.image2D_MaxHeight &&
                   desc.arraySize <= mInfo.imageMaxArraySize;
        case MemObjectType::Image1D_Buffer:
            return desc.width <= mInfo.imageMaxBufferSize;
        default:
            ASSERT(false);
            break;
    }
    return false;
}

bool Device::hasDeviceEnqueueCaps() const
{
    return mInfo.queueOnDeviceMaxSize > 0;
}

bool Device::supportsNonUniformWorkGroups() const
{
    cl_bool support = false;

    if (getPlatform().isVersionOrNewer(3, 0))
    {
        if (IsError(mImpl->getInfoUInt(DeviceInfo::NonUniformWorkGroupSupport, &support)))
        {
            UNREACHABLE();
        }
    }
    else
    {
        // Check older platforms support via device extension
        // TODO(aannestrand) Boolean-ify these extension strings rather than string compare
        // http://anglebug.com/381335059
        if (getInfo().extensions.find("cl_arm_non_uniform_work_group_size") != std::string::npos)
        {
            support = true;
        }
    }

    return support;
}

Device::Device(Platform &platform,
               Device *parent,
               DeviceType type,
               const rx::CLDeviceImpl::CreateFunc &createFunc)
    : mPlatform(platform), mParent(parent), mImpl(createFunc(*this)), mInfo(mImpl->createInfo(type))
{}

}  // namespace cl
