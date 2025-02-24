//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatform.cpp: Implements the cl::Platform class.

#include "libANGLE/CLPlatform.h"

#include "libANGLE/CLContext.h"
#include "libANGLE/CLDevice.h"
#include "libANGLE/cl_utils.h"

#include <cstring>

namespace cl
{

namespace
{

bool IsDeviceTypeMatch(DeviceType select, DeviceType type)
{
    // The type 'DeviceType' is a bitfield, so it matches if any selected bit is set.
    // A custom device is an exception, which only matches if it was explicitely selected, see:
    // https://www.khronos.org/registry/OpenCL/specs/3.0-unified/html/OpenCL_API.html#clGetDeviceIDs
    return type == CL_DEVICE_TYPE_CUSTOM ? select == CL_DEVICE_TYPE_CUSTOM
                                         : type.intersects(select);
}

Context::PropArray ParseContextProperties(const cl_context_properties *properties,
                                          Platform *&platform,
                                          bool &userSync)
{
    Context::PropArray propArray;
    if (properties != nullptr)
    {
        const cl_context_properties *propIt = properties;
        while (*propIt != 0)
        {
            switch (*propIt++)
            {
                case CL_CONTEXT_PLATFORM:
                    platform = &reinterpret_cast<cl_platform_id>(*propIt++)->cast<Platform>();
                    break;
                case CL_CONTEXT_INTEROP_USER_SYNC:
                    userSync = *propIt++ != CL_FALSE;
                    break;
            }
        }
        // Include the trailing zero
        ++propIt;
        propArray.reserve(propIt - properties);
        propArray.insert(propArray.cend(), properties, propIt);
    }
    return propArray;
}

}  // namespace

void Platform::Initialize(const cl_icd_dispatch &dispatch,
                          rx::CLPlatformImpl::CreateFuncs &&createFuncs)
{
    PlatformPtrs &platforms = GetPointers();
    ASSERT(_cl_platform_id::sDispatch == nullptr && platforms.empty());
    if (_cl_platform_id::sDispatch != nullptr || !platforms.empty())
    {
        ERR() << "Already initialized";
        return;
    }
    Dispatch::sDispatch = &dispatch;

    platforms.reserve(createFuncs.size());
    while (!createFuncs.empty())
    {
        platforms.emplace_back(new Platform(createFuncs.front()));

        // Release initialization reference, lifetime controlled by RefPointer.
        platforms.back()->release();

        // Remove platform on any errors
        if (!platforms.back()->mInfo.isValid() || platforms.back()->mDevices.empty())
        {
            platforms.pop_back();
        }

        createFuncs.pop_front();
    }
}

angle::Result Platform::GetPlatformIDs(cl_uint numEntries,
                                       cl_platform_id *platforms,
                                       cl_uint *numPlatforms)
{
    const PlatformPtrs &availPlatforms = GetPlatforms();
    if (numPlatforms != nullptr)
    {
        *numPlatforms = static_cast<cl_uint>(availPlatforms.size());
    }
    if (platforms != nullptr)
    {
        cl_uint entry   = 0u;
        auto platformIt = availPlatforms.cbegin();
        while (entry < numEntries && platformIt != availPlatforms.cend())
        {
            platforms[entry++] = (*platformIt++).get();
        }
    }
    return angle::Result::Continue;
}

angle::Result Platform::getInfo(PlatformInfo name,
                                size_t valueSize,
                                void *value,
                                size_t *valueSizeRet) const
{
    const void *copyValue = nullptr;
    size_t copySize       = 0u;

    switch (name)
    {
        case PlatformInfo::Profile:
            copyValue = mInfo.profile.c_str();
            copySize  = mInfo.profile.length() + 1u;
            break;
        case PlatformInfo::Version:
            copyValue = mInfo.versionStr.c_str();
            copySize  = mInfo.versionStr.length() + 1u;
            break;
        case PlatformInfo::NumericVersion:
            copyValue = &mInfo.version;
            copySize  = sizeof(mInfo.version);
            break;
        case PlatformInfo::Name:
            copyValue = mInfo.name.c_str();
            copySize  = mInfo.name.length() + 1u;
            break;
        case PlatformInfo::Vendor:
            copyValue = kVendor;
            copySize  = sizeof(kVendor);
            break;
        case PlatformInfo::Extensions:
            copyValue = mInfo.extensions.c_str();
            copySize  = mInfo.extensions.length() + 1u;
            break;
        case PlatformInfo::ExtensionsWithVersion:
            copyValue = mInfo.extensionsWithVersion.data();
            copySize  = mInfo.extensionsWithVersion.size() *
                       sizeof(decltype(mInfo.extensionsWithVersion)::value_type);
            break;
        case PlatformInfo::HostTimerResolution:
            copyValue = &mInfo.hostTimerRes;
            copySize  = sizeof(mInfo.hostTimerRes);
            break;
        case PlatformInfo::IcdSuffix:
            copyValue = kIcdSuffix;
            copySize  = sizeof(kIcdSuffix);
            break;
        default:
            ASSERT(false);
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
        // as specified in the OpenCL Platform Queries table, and param_value is not a NULL value.
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

angle::Result Platform::getDeviceIDs(DeviceType deviceType,
                                     cl_uint numEntries,
                                     cl_device_id *devices,
                                     cl_uint *numDevices) const
{
    cl_uint found = 0u;
    for (const DevicePtr &device : mDevices)
    {
        if (IsDeviceTypeMatch(deviceType, device->getInfo().type))
        {
            if (devices != nullptr && found < numEntries)
            {
                devices[found] = device.get();
            }
            ++found;
        }
    }
    if (numDevices != nullptr)
    {
        *numDevices = found;
    }

    // CL_DEVICE_NOT_FOUND if no OpenCL devices that matched device_type were found.
    if (found == 0u)
    {
        ANGLE_CL_RETURN_ERROR(CL_DEVICE_NOT_FOUND);
    }

    return angle::Result::Continue;
}

bool Platform::hasDeviceType(DeviceType deviceType) const
{
    for (const DevicePtr &device : mDevices)
    {
        if (IsDeviceTypeMatch(deviceType, device->getInfo().type))
        {
            return true;
        }
    }
    return false;
}

cl_context Platform::CreateContext(const cl_context_properties *properties,
                                   cl_uint numDevices,
                                   const cl_device_id *devices,
                                   ContextErrorCB notify,
                                   void *userData)
{
    DevicePtrs devs;
    devs.reserve(numDevices);
    while (numDevices-- != 0u)
    {
        devs.emplace_back(&(*devices++)->cast<Device>());
    }

    Platform *platform           = nullptr;
    bool userSync                = false;
    Context::PropArray propArray = ParseContextProperties(properties, platform, userSync);
    if (platform == nullptr)
    {
        // All devices in the list have already been validated at this point to contain the same
        // platform - just use/select the first device's platform.
        platform = &devs.front()->getPlatform();
    }

    return Object::Create<Context>(*platform, std::move(propArray), std::move(devs), notify,
                                   userData, userSync);
}

cl_context Platform::CreateContextFromType(const cl_context_properties *properties,
                                           DeviceType deviceType,
                                           ContextErrorCB notify,
                                           void *userData)
{
    Platform *platform           = nullptr;
    bool userSync                = false;
    Context::PropArray propArray = ParseContextProperties(properties, platform, userSync);

    // Choose default platform if user does not specify in the context properties field
    if (platform == nullptr)
    {
        platform = Platform::GetDefault();
    }

    return Object::Create<Context>(*platform, std::move(propArray), deviceType, notify, userData,
                                   userSync);
}

angle::Result Platform::unloadCompiler()
{
    return mImpl->unloadCompiler();
}

Platform::~Platform() = default;

Platform::Platform(const rx::CLPlatformImpl::CreateFunc &createFunc)
    : mImpl(createFunc(*this)),
      mInfo(mImpl ? mImpl->createInfo() : rx::CLPlatformImpl::Info{}),
      mDevices(mImpl ? createDevices(mImpl->createDevices()) : DevicePtrs{}),
      mMultiThreadPool(mImpl ? angle::WorkerThreadPool::Create(0, ANGLEPlatformCurrent()) : nullptr)
{}

DevicePtrs Platform::createDevices(rx::CLDeviceImpl::CreateDatas &&createDatas)
{
    DevicePtrs devices;
    devices.reserve(createDatas.size());
    while (!createDatas.empty())
    {
        devices.emplace_back(
            new Device(*this, nullptr, createDatas.front().first, createDatas.front().second));
        // Release initialization reference, lifetime controlled by RefPointer.
        devices.back()->release();
        if (!devices.back()->mInfo.isValid())
        {
            devices.pop_back();
        }
        createDatas.pop_front();
    }
    return devices;
}

constexpr char Platform::kVendor[];
constexpr char Platform::kIcdSuffix[];

}  // namespace cl
