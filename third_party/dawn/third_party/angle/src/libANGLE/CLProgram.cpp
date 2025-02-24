//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgram.cpp: Implements the cl::Program class.

#include "libANGLE/CLProgram.h"

#include "libANGLE/CLContext.h"
#include "libANGLE/CLPlatform.h"
#include "libANGLE/cl_utils.h"

#include <cstring>

namespace cl
{

angle::Result Program::build(cl_uint numDevices,
                             const cl_device_id *deviceList,
                             const char *options,
                             ProgramCB pfnNotify,
                             void *userData)
{
    DevicePtrs devices;
    devices.reserve(numDevices);
    while (numDevices-- != 0u)
    {
        devices.emplace_back(&(*deviceList++)->cast<Device>());
    }
    Program *notify = nullptr;
    if (pfnNotify != nullptr)
    {
        // This program has to be retained until the notify callback is called.
        retain();
        *mCallback = CallbackData(pfnNotify, userData);
        notify     = this;
    }
    return mImpl->build(devices, options, notify);
}

angle::Result Program::compile(cl_uint numDevices,
                               const cl_device_id *deviceList,
                               const char *options,
                               cl_uint numInputHeaders,
                               const cl_program *inputHeaders,
                               const char **headerIncludeNames,
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
    programs.reserve(numInputHeaders);
    while (numInputHeaders-- != 0u)
    {
        programs.emplace_back(&(*inputHeaders++)->cast<Program>());
    }
    Program *notify = nullptr;
    if (pfnNotify != nullptr)
    {
        // This program has to be retained until the notify callback is called.
        retain();
        *mCallback = CallbackData(pfnNotify, userData);
        notify     = this;
    }
    return mImpl->compile(devices, options, programs, headerIncludeNames, notify);
}

angle::Result Program::getInfo(ProgramInfo name,
                               size_t valueSize,
                               void *value,
                               size_t *valueSizeRet) const
{
    std::vector<cl_device_id> devices;
    cl_uint valUInt       = 0u;
    void *valPointer      = nullptr;
    const void *copyValue = nullptr;
    size_t copySize       = 0u;

    switch (name)
    {
        case ProgramInfo::ReferenceCount:
            valUInt   = getRefCount();
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case ProgramInfo::Context:
            valPointer = mContext->getNative();
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case ProgramInfo::NumDevices:
            valUInt   = static_cast<decltype(valUInt)>(mDevices.size());
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case ProgramInfo::Devices:
            devices.reserve(mDevices.size());
            for (const DevicePtr &device : mDevices)
            {
                devices.emplace_back(device->getNative());
            }
            copyValue = devices.data();
            copySize  = devices.size() * sizeof(decltype(devices)::value_type);
            break;
        case ProgramInfo::Source:
            if (!mSource.empty())
            {
                copyValue = mSource.c_str();
                copySize  = mSource.length() + 1u;
            }
            break;
        case ProgramInfo::IL:
            if (!mIL.empty())
            {
                copyValue = mIL.c_str();
                copySize  = mIL.length() + 1u;
            }
            break;
        case ProgramInfo::BinarySizes:
        case ProgramInfo::Binaries:
        case ProgramInfo::NumKernels:
        case ProgramInfo::KernelNames:
        case ProgramInfo::ScopeGlobalCtorsPresent:
        case ProgramInfo::ScopeGlobalDtorsPresent:
            return mImpl->getInfo(name, valueSize, value, valueSizeRet);
        default:
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
        // as described in the Program Object Queries table and param_value is not NULL.
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

angle::Result Program::getBuildInfo(cl_device_id device,
                                    ProgramBuildInfo name,
                                    size_t valueSize,
                                    void *value,
                                    size_t *valueSizeRet) const
{
    return mImpl->getBuildInfo(device->cast<Device>(), name, valueSize, value, valueSizeRet);
}

cl_kernel Program::createKernel(const char *kernel_name)
{
    return Object::Create<Kernel>(*this, kernel_name);
}

angle::Result Program::createKernels(cl_uint numKernels, cl_kernel *kernels, cl_uint *numKernelsRet)
{
    if (kernels == nullptr)
    {
        numKernels = 0u;
    }
    rx::CLKernelImpl::CreateFuncs createFuncs;
    ANGLE_TRY(mImpl->createKernels(numKernels, createFuncs, numKernelsRet));
    KernelPtrs krnls;
    krnls.reserve(createFuncs.size());
    while (!createFuncs.empty())
    {
        krnls.emplace_back(new Kernel(*this, createFuncs.front()));
        if (krnls.back()->mImpl == nullptr)
        {
            return angle::Result::Stop;
        }
        createFuncs.pop_front();
    }
    for (KernelPtr &kernel : krnls)
    {
        *kernels++ = kernel.release();
    }
    return angle::Result::Continue;
}

Program::~Program() = default;

void Program::callback()
{
    CallbackData callbackData;
    mCallback->swap(callbackData);
    const ProgramCB callback = callbackData.first;
    void *const userData     = callbackData.second;
    ASSERT(callback != nullptr);
    callback(this, userData);
    // This program can be released after the callback was called.
    if (release())
    {
        delete this;
    }
}

Program::Program(Context &context, std::string &&source)
    : mContext(&context),
      mDevices(context.getDevices()),
      mNumAttachedKernels(0u),
      mImpl(nullptr),
      mSource(std::move(source))
{
    ANGLE_CL_IMPL_TRY(context.getImpl().createProgramWithSource(*this, mSource, &mImpl));
}

Program::Program(Context &context, const void *il, size_t length)
    : mContext(&context),
      mDevices(context.getDevices()),
      mIL(static_cast<const char *>(il), length),
      mNumAttachedKernels(0u),
      mImpl(nullptr)
{
    ANGLE_CL_IMPL_TRY(context.getImpl().createProgramWithIL(*this, il, length, &mImpl));
}

Program::Program(Context &context,
                 DevicePtrs &&devices,
                 const size_t *lengths,
                 const unsigned char **binaries,
                 cl_int *binaryStatus)
    : mContext(&context), mDevices(std::move(devices)), mNumAttachedKernels(0u), mImpl(nullptr)
{
    ANGLE_CL_IMPL_TRY(
        context.getImpl().createProgramWithBinary(*this, lengths, binaries, binaryStatus, &mImpl));
}

Program::Program(Context &context, DevicePtrs &&devices, const char *kernelNames)
    : mContext(&context), mDevices(std::move(devices)), mNumAttachedKernels(0u), mImpl(nullptr)
{
    ANGLE_CL_IMPL_TRY(
        context.getImpl().createProgramWithBuiltInKernels(*this, kernelNames, &mImpl));
}

Program::Program(Context &context,
                 const DevicePtrs &devices,
                 const char *options,
                 const cl::ProgramPtrs &inputPrograms,
                 ProgramCB pfnNotify,
                 void *userData)
    : mContext(&context),
      mDevices(!devices.empty() ? devices : context.getDevices()),
      mNumAttachedKernels(0u),
      mImpl(nullptr)
{
    if (pfnNotify != nullptr)
    {
        // This program has to be retained until the notify callback is called.
        retain();
        *mCallback = CallbackData(pfnNotify, userData);
    }
    else
    {
        *mCallback = CallbackData();
    }
    ANGLE_CL_IMPL_TRY(context.getImpl().linkProgram(*this, mDevices, options, inputPrograms,
                                                    pfnNotify != nullptr ? this : nullptr, &mImpl));
}

}  // namespace cl
