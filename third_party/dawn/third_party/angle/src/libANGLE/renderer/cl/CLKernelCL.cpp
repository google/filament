//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLKernelCL.cpp: Implements the class methods for CLKernelCL.

#include "libANGLE/renderer/cl/CLKernelCL.h"

#include "libANGLE/renderer/cl/CLCommandQueueCL.h"
#include "libANGLE/renderer/cl/CLContextCL.h"
#include "libANGLE/renderer/cl/CLDeviceCL.h"
#include "libANGLE/renderer/cl/CLMemoryCL.h"
#include "libANGLE/renderer/cl/CLSamplerCL.h"

#include "libANGLE/CLCommandQueue.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLMemory.h"
#include "libANGLE/CLPlatform.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/CLSampler.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

namespace
{

template <typename T>
bool GetWorkGroupInfo(cl_kernel kernel,
                      cl_device_id device,
                      cl::KernelWorkGroupInfo name,
                      T &value,
                      cl_int &errorCode)
{
    errorCode = kernel->getDispatch().clGetKernelWorkGroupInfo(kernel, device, cl::ToCLenum(name),
                                                               sizeof(T), &value, nullptr);
    return errorCode == CL_SUCCESS;
}

template <typename T>
bool GetArgInfo(cl_kernel kernel,
                cl_uint index,
                cl::KernelArgInfo name,
                T &value,
                cl_int &errorCode)
{
    errorCode = kernel->getDispatch().clGetKernelArgInfo(kernel, index, cl::ToCLenum(name),
                                                         sizeof(T), &value, nullptr);
    if (errorCode == CL_KERNEL_ARG_INFO_NOT_AVAILABLE)
    {
        errorCode = CL_SUCCESS;
    }
    return errorCode == CL_SUCCESS;
}

template <typename T>
bool GetKernelInfo(cl_kernel kernel, cl::KernelInfo name, T &value, cl_int &errorCode)
{
    errorCode = kernel->getDispatch().clGetKernelInfo(kernel, cl::ToCLenum(name), sizeof(T), &value,
                                                      nullptr);
    return errorCode == CL_SUCCESS;
}

bool GetArgString(cl_kernel kernel,
                  cl_uint index,
                  cl::KernelArgInfo name,
                  std::string &string,
                  cl_int &errorCode)
{
    size_t size = 0u;
    errorCode   = kernel->getDispatch().clGetKernelArgInfo(kernel, index, cl::ToCLenum(name), 0u,
                                                           nullptr, &size);
    if (errorCode == CL_KERNEL_ARG_INFO_NOT_AVAILABLE)
    {
        errorCode = CL_SUCCESS;
        return true;
    }
    else if (errorCode != CL_SUCCESS)
    {
        return false;
    }
    std::vector<char> valString(size, '\0');
    errorCode = kernel->getDispatch().clGetKernelArgInfo(kernel, index, cl::ToCLenum(name), size,
                                                         valString.data(), nullptr);
    if (errorCode != CL_SUCCESS)
    {
        return false;
    }
    string.assign(valString.data(), valString.size() - 1u);
    return true;
}

bool GetKernelString(cl_kernel kernel, cl::KernelInfo name, std::string &string, cl_int &errorCode)
{
    size_t size = 0u;
    errorCode =
        kernel->getDispatch().clGetKernelInfo(kernel, cl::ToCLenum(name), 0u, nullptr, &size);
    if (errorCode != CL_SUCCESS)
    {
        return false;
    }
    std::vector<char> valString(size, '\0');
    errorCode = kernel->getDispatch().clGetKernelInfo(kernel, cl::ToCLenum(name), size,
                                                      valString.data(), nullptr);
    if (errorCode != CL_SUCCESS)
    {
        return false;
    }
    string.assign(valString.data(), valString.size() - 1u);
    return true;
}

}  // namespace

CLKernelCL::CLKernelCL(const cl::Kernel &kernel, cl_kernel native)
    : CLKernelImpl(kernel), mNative(native)
{}

CLKernelCL::~CLKernelCL()
{
    if (mNative->getDispatch().clReleaseKernel(mNative) != CL_SUCCESS)
    {
        ERR() << "Error while releasing CL kernel";
    }
}

angle::Result CLKernelCL::setArg(cl_uint argIndex, size_t argSize, const void *argValue)
{
    void *value = nullptr;
    if (argValue != nullptr)
    {
        // If argument is a CL object, fetch the mapped value
        const CLContextCL &ctx = mKernel.getProgram().getContext().getImpl<CLContextCL>();
        if (argSize == sizeof(cl_mem))
        {
            cl_mem memory = *static_cast<const cl_mem *>(argValue);
            if (ctx.hasMemory(memory))
            {
                value = memory->cast<cl::Memory>().getImpl<CLMemoryCL>().getNative();
            }
        }
        if (value == nullptr && argSize == sizeof(cl_sampler))
        {
            cl_sampler sampler = *static_cast<const cl_sampler *>(argValue);
            if (ctx.hasSampler(sampler))
            {
                value = sampler->cast<cl::Sampler>().getImpl<CLSamplerCL>().getNative();
            }
        }
        if (value == nullptr && argSize == sizeof(cl_command_queue))
        {
            cl_command_queue queue = *static_cast<const cl_command_queue *>(argValue);
            if (ctx.hasDeviceQueue(queue))
            {
                value = queue->cast<cl::CommandQueue>().getImpl<CLCommandQueueCL>().getNative();
            }
        }
    }

    // If mapped value was found, use it instead of original value
    if (value != nullptr)
    {
        argValue = &value;
    }
    ANGLE_CL_TRY(mNative->getDispatch().clSetKernelArg(mNative, argIndex, argSize, argValue));
    return angle::Result::Continue;
}

angle::Result CLKernelCL::createInfo(CLKernelImpl::Info *infoOut) const
{
    cl_int errorCode       = CL_SUCCESS;
    const cl::Context &ctx = mKernel.getProgram().getContext();

    if (!GetKernelString(mNative, cl::KernelInfo::FunctionName, infoOut->functionName, errorCode) ||
        !GetKernelInfo(mNative, cl::KernelInfo::NumArgs, infoOut->numArgs, errorCode) ||
        (ctx.getPlatform().isVersionOrNewer(1u, 2u) &&
         !GetKernelString(mNative, cl::KernelInfo::Attributes, infoOut->attributes, errorCode)))
    {
        ANGLE_CL_RETURN_ERROR(errorCode);
    }

    infoOut->workGroups.resize(ctx.getDevices().size());
    for (size_t index = 0u; index < ctx.getDevices().size(); ++index)
    {
        const cl_device_id device = ctx.getDevices()[index]->getImpl<CLDeviceCL>().getNative();
        WorkGroupInfo &workGroup  = infoOut->workGroups[index];

        if ((ctx.getPlatform().isVersionOrNewer(1u, 2u) &&
             ctx.getDevices()[index]->supportsBuiltInKernel(infoOut->functionName) &&
             !GetWorkGroupInfo(mNative, device, cl::KernelWorkGroupInfo::GlobalWorkSize,
                               workGroup.globalWorkSize, errorCode)) ||
            !GetWorkGroupInfo(mNative, device, cl::KernelWorkGroupInfo::WorkGroupSize,
                              workGroup.workGroupSize, errorCode) ||
            !GetWorkGroupInfo(mNative, device, cl::KernelWorkGroupInfo::CompileWorkGroupSize,
                              workGroup.compileWorkGroupSize, errorCode) ||
            !GetWorkGroupInfo(mNative, device, cl::KernelWorkGroupInfo::LocalMemSize,
                              workGroup.localMemSize, errorCode) ||
            !GetWorkGroupInfo(mNative, device,
                              cl::KernelWorkGroupInfo::PreferredWorkGroupSizeMultiple,
                              workGroup.prefWorkGroupSizeMultiple, errorCode) ||
            !GetWorkGroupInfo(mNative, device, cl::KernelWorkGroupInfo::PrivateMemSize,
                              workGroup.privateMemSize, errorCode))
        {
            ANGLE_CL_RETURN_ERROR(errorCode);
        }
    }

    infoOut->args.resize(infoOut->numArgs);
    if (ctx.getPlatform().isVersionOrNewer(1u, 2u))
    {
        for (cl_uint index = 0u; index < infoOut->numArgs; ++index)
        {
            ArgInfo &arg = infoOut->args[index];
            if (!GetArgInfo(mNative, index, cl::KernelArgInfo::AddressQualifier,
                            arg.addressQualifier, errorCode) ||
                !GetArgInfo(mNative, index, cl::KernelArgInfo::AccessQualifier, arg.accessQualifier,
                            errorCode) ||
                !GetArgString(mNative, index, cl::KernelArgInfo::TypeName, arg.typeName,
                              errorCode) ||
                !GetArgInfo(mNative, index, cl::KernelArgInfo::TypeQualifier, arg.typeQualifier,
                            errorCode) ||
                !GetArgString(mNative, index, cl::KernelArgInfo::Name, arg.name, errorCode))
            {
                ANGLE_CL_RETURN_ERROR(errorCode);
            }
        }
    }

    return angle::Result::Continue;
}

}  // namespace rx
