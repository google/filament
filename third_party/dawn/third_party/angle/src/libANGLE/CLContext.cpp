//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLContext.cpp: Implements the cl::Context class.

#include "libANGLE/CLContext.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLCommandQueue.h"
#include "libANGLE/CLEvent.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/CLMemory.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/CLSampler.h"

#include <cstring>

namespace cl
{

angle::Result Context::getInfo(ContextInfo name,
                               size_t valueSize,
                               void *value,
                               size_t *valueSizeRet) const
{
    std::vector<cl_device_id> devices;
    cl_uint valUInt       = 0u;
    const void *copyValue = nullptr;
    size_t copySize       = 0u;

    switch (name)
    {
        case ContextInfo::ReferenceCount:
            valUInt   = getRefCount();
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case ContextInfo::NumDevices:
            valUInt   = static_cast<decltype(valUInt)>(mDevices.size());
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case ContextInfo::Devices:
            devices.reserve(mDevices.size());
            for (const DevicePtr &device : mDevices)
            {
                devices.emplace_back(device->getNative());
            }
            copyValue = devices.data();
            copySize  = devices.size() * sizeof(decltype(devices)::value_type);
            break;
        case ContextInfo::Properties:
            copyValue = mProperties.data();
            copySize  = mProperties.size() * sizeof(decltype(mProperties)::value_type);
            break;
        default:
            ASSERT(false);
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
        // as specified in the Context Attributes table and param_value is not a NULL value.
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

cl_command_queue Context::createCommandQueueWithProperties(cl_device_id device,
                                                           const cl_queue_properties *properties)
{
    CommandQueue::PropArray propArray;
    CommandQueueProperties props;
    cl_uint size = CommandQueue::kNoSize;
    if (properties != nullptr)
    {
        const cl_queue_properties *propIt = properties;
        while (*propIt != 0)
        {
            switch (*propIt++)
            {
                case CL_QUEUE_PROPERTIES:
                    props = static_cast<cl_command_queue_properties>(*propIt++);
                    break;
                case CL_QUEUE_SIZE:
                    size = static_cast<decltype(size)>(*propIt++);
                    break;
            }
        }
        // Include the trailing zero
        ++propIt;
        propArray.reserve(propIt - properties);
        propArray.insert(propArray.cend(), properties, propIt);
    }
    return Object::Create<CommandQueue>(*this, device->cast<Device>(), std::move(propArray), props,
                                        size);
}

cl_command_queue Context::createCommandQueue(cl_device_id device, CommandQueueProperties properties)
{
    return Object::Create<CommandQueue>(*this, device->cast<Device>(), properties);
}

cl_mem Context::createBuffer(const cl_mem_properties *properties,
                             MemFlags flags,
                             size_t size,
                             void *hostPtr)
{
    // If the properties argument specified in clCreateBufferWithProperties or
    // clCreateImageWithProperties used to create memobj was not NULL, the implementation must
    // return the values specified in the properties argument in the same order and without
    // including additional properties. So pass them as is.
    Memory::PropArray propArray;
    if (properties != nullptr)
    {
        const cl_mem_properties *propIt = properties;
        while (*propIt != 0)
        {
            propArray.push_back(*propIt);
            ++propIt;
        }
        // there is at least one property - special property 0
        propArray.push_back(0);
    }
    return Object::Create<Buffer>(*this, std::move(propArray), flags, size, hostPtr);
}

cl_mem Context::createImage(const cl_mem_properties *properties,
                            MemFlags flags,
                            const cl_image_format *format,
                            const cl_image_desc *desc,
                            void *hostPtr)
{
    const ImageDescriptor imageDesc = {FromCLenum<MemObjectType>(desc->image_type),
                                       desc->image_width,
                                       desc->image_height,
                                       desc->image_depth,
                                       desc->image_array_size,
                                       desc->image_row_pitch,
                                       desc->image_slice_pitch,
                                       desc->num_mip_levels,
                                       desc->num_samples};
    Memory::PropArray propArray;

    // If the properties argument specified in clCreateBufferWithProperties or
    // clCreateImageWithProperties used to create memobj was not NULL, the implementation must
    // return the values specified in the properties argument in the same order and without
    // including additional properties. So Pass them as is.
    if (properties != nullptr)
    {
        const cl_mem_properties *propIt = properties;
        while (*propIt != 0)
        {
            propArray.push_back(*propIt);
            ++propIt;
        }
        // there is at least one property - special property 0
        propArray.push_back(0);
    }
    return Object::Create<Image>(*this, std::move(propArray), flags, *format, imageDesc,
                                 Memory::Cast(desc->buffer), hostPtr);
}

cl_mem Context::createImage2D(MemFlags flags,
                              const cl_image_format *format,
                              size_t width,
                              size_t height,
                              size_t rowPitch,
                              void *hostPtr)
{
    const ImageDescriptor imageDesc(MemObjectType::Image2D, width, height, 0u, 0u, rowPitch, 0u, 0u,
                                    0u);
    return Object::Create<Image>(*this, Memory::PropArray{}, flags, *format, imageDesc, nullptr,
                                 hostPtr);
}

cl_mem Context::createImage3D(MemFlags flags,
                              const cl_image_format *format,
                              size_t width,
                              size_t height,
                              size_t depth,
                              size_t rowPitch,
                              size_t slicePitch,
                              void *hostPtr)
{
    const ImageDescriptor imageDesc(MemObjectType::Image3D, width, height, depth, 0u, rowPitch,
                                    slicePitch, 0u, 0u);
    return Object::Create<Image>(*this, Memory::PropArray{}, flags, *format, imageDesc, nullptr,
                                 hostPtr);
}

angle::Result Context::getSupportedImageFormats(MemFlags flags,
                                                MemObjectType imageType,
                                                cl_uint numEntries,
                                                cl_image_format *imageFormats,
                                                cl_uint *numImageFormats)
{
    return mImpl->getSupportedImageFormats(flags, imageType, numEntries, imageFormats,
                                           numImageFormats);
}

cl_sampler Context::createSamplerWithProperties(const cl_sampler_properties *properties)
{
    Sampler::PropArray propArray;
    cl_bool normalizedCoords      = CL_TRUE;
    AddressingMode addressingMode = AddressingMode::Clamp;
    FilterMode filterMode         = FilterMode::Nearest;

    if (properties != nullptr)
    {
        const cl_sampler_properties *propIt = properties;
        while (*propIt != 0)
        {
            switch (*propIt++)
            {
                case CL_SAMPLER_NORMALIZED_COORDS:
                    normalizedCoords = static_cast<decltype(normalizedCoords)>(*propIt++);
                    break;
                case CL_SAMPLER_ADDRESSING_MODE:
                    addressingMode = FromCLenum<AddressingMode>(static_cast<CLenum>(*propIt++));
                    break;
                case CL_SAMPLER_FILTER_MODE:
                    filterMode = FromCLenum<FilterMode>(static_cast<CLenum>(*propIt++));
                    break;
            }
        }
        // Include the trailing zero
        ++propIt;
        propArray.reserve(propIt - properties);
        propArray.insert(propArray.cend(), properties, propIt);
    }

    return Object::Create<Sampler>(*this, std::move(propArray), normalizedCoords, addressingMode,
                                   filterMode);
}

cl_sampler Context::createSampler(cl_bool normalizedCoords,
                                  AddressingMode addressingMode,
                                  FilterMode filterMode)
{
    return Object::Create<Sampler>(*this, Sampler::PropArray{}, normalizedCoords, addressingMode,
                                   filterMode);
}

cl_program Context::createProgramWithSource(cl_uint count,
                                            const char **strings,
                                            const size_t *lengths)
{
    std::string source;
    if (lengths == nullptr)
    {
        while (count-- != 0u)
        {
            source.append(*strings++);
        }
    }
    else
    {
        while (count-- != 0u)
        {
            if (*lengths != 0u)
            {
                source.append(*strings++, *lengths);
            }
            else
            {
                source.append(*strings++);
            }
            ++lengths;
        }
    }
    return Object::Create<Program>(*this, std::move(source));
}

cl_program Context::createProgramWithIL(const void *il, size_t length)
{
    return Object::Create<Program>(*this, il, length);
}

cl_program Context::createProgramWithBinary(cl_uint numDevices,
                                            const cl_device_id *devices,
                                            const size_t *lengths,
                                            const unsigned char **binaries,
                                            cl_int *binaryStatus)
{
    DevicePtrs devs;
    devs.reserve(numDevices);
    while (numDevices-- != 0u)
    {
        devs.emplace_back(&(*devices++)->cast<Device>());
    }
    return Object::Create<Program>(*this, std::move(devs), lengths, binaries, binaryStatus);
}

cl_program Context::createProgramWithBuiltInKernels(cl_uint numDevices,
                                                    const cl_device_id *devices,
                                                    const char *kernelNames)
{
    DevicePtrs devs;
    devs.reserve(numDevices);
    while (numDevices-- != 0u)
    {
        devs.emplace_back(&(*devices++)->cast<Device>());
    }
    return Object::Create<Program>(*this, std::move(devs), kernelNames);
}

cl_program Context::linkProgram(cl_uint numDevices,
                                const cl_device_id *deviceList,
                                const char *options,
                                cl_uint numInputPrograms,
                                const cl_program *inputPrograms,
                                ProgramCB pfnNotify,
                                void *userData)
{
    DevicePtrs devices;
    devices.reserve(numDevices);
    while (numDevices-- != 0u)
    {
        devices.emplace_back(&(*deviceList++)->cast<Device>());
    }
    ProgramPtrs programs;
    programs.reserve(numInputPrograms);
    while (numInputPrograms-- != 0u)
    {
        programs.emplace_back(&(*inputPrograms++)->cast<Program>());
    }
    return Object::Create<Program>(*this, devices, options, programs, pfnNotify, userData);
}

cl_event Context::createUserEvent()
{
    return Object::Create<Event>(*this);
}

angle::Result Context::waitForEvents(cl_uint numEvents, const cl_event *eventList)
{
    return mImpl->waitForEvents(Event::Cast(numEvents, eventList));
}

Context::~Context() = default;

void Context::ErrorCallback(const char *errinfo, const void *privateInfo, size_t cb, void *userData)
{
    Context *const context = static_cast<Context *>(userData);
    if (!Context::IsValid(context))
    {
        WARN() << "Context error for invalid context";
        return;
    }
    if (context->mNotify != nullptr)
    {
        context->mNotify(errinfo, privateInfo, cb, context->mUserData);
    }
}

Context::Context(Platform &platform,
                 PropArray &&properties,
                 DevicePtrs &&devices,
                 ContextErrorCB notify,
                 void *userData,
                 bool userSync)
    : mPlatform(platform),
      mProperties(std::move(properties)),
      mNotify(notify),
      mUserData(userData),
      mImpl(nullptr),
      mDevices(std::move(devices))
{
    ANGLE_CL_IMPL_TRY(platform.getImpl().createContext(*this, mDevices, userSync, &mImpl));
}

Context::Context(Platform &platform,
                 PropArray &&properties,
                 DeviceType deviceType,
                 ContextErrorCB notify,
                 void *userData,
                 bool userSync)
    : mPlatform(platform),
      mProperties(std::move(properties)),
      mNotify(notify),
      mUserData(userData),
      mImpl(nullptr)
{
    if (!IsError(platform.getImpl().createContextFromType(*this, deviceType, userSync, &mImpl)))
    {
        ANGLE_CL_IMPL_TRY(mImpl->getDevices(&mDevices));
    }
}

}  // namespace cl
