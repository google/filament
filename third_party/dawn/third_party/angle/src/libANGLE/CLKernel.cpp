//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLKernel.cpp: Implements the cl::Kernel class.

#include "libANGLE/CLKernel.h"

#include "libANGLE/CLContext.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/cl_utils.h"

#include <cstring>

namespace cl
{

angle::Result Kernel::setArg(cl_uint argIndex, size_t argSize, const void *argValue)
{
    ANGLE_TRY(mImpl->setArg(argIndex, argSize, argValue));

    mSetArguments[argIndex] = {true, argIndex, argSize, argValue};
    return angle::Result::Continue;
}

angle::Result Kernel::getInfo(KernelInfo name,
                              size_t valueSize,
                              void *value,
                              size_t *valueSizeRet) const
{
    cl_uint valUInt       = 0u;
    void *valPointer      = nullptr;
    const void *copyValue = nullptr;
    size_t copySize       = 0u;

    switch (name)
    {
        case KernelInfo::FunctionName:
            copyValue = mInfo.functionName.c_str();
            copySize  = mInfo.functionName.length() + 1u;
            break;
        case KernelInfo::NumArgs:
            copyValue = &mInfo.numArgs;
            copySize  = sizeof(mInfo.numArgs);
            break;
        case KernelInfo::ReferenceCount:
            valUInt   = getRefCount();
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case KernelInfo::Context:
            valPointer = mProgram->getContext().getNative();
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case KernelInfo::Program:
            valPointer = mProgram->getNative();
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case KernelInfo::Attributes:
            copyValue = mInfo.attributes.c_str();
            copySize  = mInfo.attributes.length() + 1u;
            break;
        default:
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
        // as described in the Kernel Object Queries table and param_value is not NULL.
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

angle::Result Kernel::getWorkGroupInfo(cl_device_id device,
                                       KernelWorkGroupInfo name,
                                       size_t valueSize,
                                       void *value,
                                       size_t *valueSizeRet) const
{
    size_t index = 0u;
    if (device != nullptr)
    {
        const DevicePtrs &devices = mProgram->getContext().getDevices();
        while (index < devices.size() && devices[index] != device)
        {
            ++index;
        }
        if (index == devices.size())
        {
            ANGLE_CL_RETURN_ERROR(CL_INVALID_DEVICE);
        }
    }
    const rx::CLKernelImpl::WorkGroupInfo &info = mInfo.workGroups[index];

    const void *copyValue = nullptr;
    size_t copySize       = 0u;

    switch (name)
    {
        case KernelWorkGroupInfo::GlobalWorkSize:
            copyValue = &info.globalWorkSize;
            copySize  = sizeof(info.globalWorkSize);
            break;
        case KernelWorkGroupInfo::WorkGroupSize:
            copyValue = &info.workGroupSize;
            copySize  = sizeof(info.workGroupSize);
            break;
        case KernelWorkGroupInfo::CompileWorkGroupSize:
            copyValue = &info.compileWorkGroupSize;
            copySize  = sizeof(info.compileWorkGroupSize);
            break;
        case KernelWorkGroupInfo::LocalMemSize:
            copyValue = &info.localMemSize;
            copySize  = sizeof(info.localMemSize);
            break;
        case KernelWorkGroupInfo::PreferredWorkGroupSizeMultiple:
            copyValue = &info.prefWorkGroupSizeMultiple;
            copySize  = sizeof(info.prefWorkGroupSizeMultiple);
            break;
        case KernelWorkGroupInfo::PrivateMemSize:
            copyValue = &info.privateMemSize;
            copySize  = sizeof(info.privateMemSize);
            break;
        default:
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
        // as described in the Kernel Object Device Queries table and param_value is not NULL.
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

angle::Result Kernel::getArgInfo(cl_uint argIndex,
                                 KernelArgInfo name,
                                 size_t valueSize,
                                 void *value,
                                 size_t *valueSizeRet) const
{
    const rx::CLKernelImpl::ArgInfo &info = mInfo.args[argIndex];
    const void *copyValue                 = nullptr;
    size_t copySize                       = 0u;

    switch (name)
    {
        case KernelArgInfo::AddressQualifier:
            copyValue = &info.addressQualifier;
            copySize  = sizeof(info.addressQualifier);
            break;
        case KernelArgInfo::AccessQualifier:
            copyValue = &info.accessQualifier;
            copySize  = sizeof(info.accessQualifier);
            break;
        case KernelArgInfo::TypeName:
            copyValue = info.typeName.c_str();
            copySize  = info.typeName.length() + 1u;
            break;
        case KernelArgInfo::TypeQualifier:
            copyValue = &info.typeQualifier;
            copySize  = sizeof(info.typeQualifier);
            break;
        case KernelArgInfo::Name:
            copyValue = info.name.c_str();
            copySize  = info.name.length() + 1u;
            break;
        default:
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value size is < size of return type
        // as described in the Kernel Argument Queries table and param_value is not NULL.
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

Kernel *Kernel::clone() const
{
    cl_kernel kernel = mProgram->createKernel(getName().c_str());

    for (KernelArg arg : mSetArguments)
    {
        if (arg.isSet && IsError(kernel->cast<Kernel>().setArg(arg.index, arg.size, arg.valuePtr)))
        {
            ANGLE_CL_SET_ERROR(CL_OUT_OF_RESOURCES);
            return nullptr;
        }
    }
    return &kernel->cast<Kernel>();
}

Kernel::~Kernel()
{
    --mProgram->mNumAttachedKernels;
}

Kernel::Kernel(Program &program, const char *name) : mProgram(&program), mImpl(nullptr)
{
    if (!IsError(program.getImpl().createKernel(*this, name, &mImpl)))
    {
        initImpl();
    }
}

Kernel::Kernel(Program &program, const rx::CLKernelImpl::CreateFunc &createFunc)
    : mProgram(&program), mImpl(createFunc(*this))
{
    if (mImpl)
    {
        initImpl();
    }
}

void Kernel::initImpl()
{
    ANGLE_CL_IMPL_TRY(mImpl->createInfo(&mInfo));

    mSetArguments.resize(mInfo.numArgs);
    std::fill(mSetArguments.begin(), mSetArguments.end(), KernelArg{false, 0, 0, 0});

    ++mProgram->mNumAttachedKernels;
}

}  // namespace cl
