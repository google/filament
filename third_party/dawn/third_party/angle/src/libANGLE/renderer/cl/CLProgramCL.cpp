//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgramCL.cpp: Implements the class methods for CLProgramCL.

#include "libANGLE/renderer/cl/CLProgramCL.h"

#include "libANGLE/renderer/cl/CLDeviceCL.h"
#include "libANGLE/renderer/cl/CLKernelCL.h"

#include "libANGLE/CLDevice.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

CLProgramCL::CLProgramCL(const cl::Program &program, cl_program native)
    : CLProgramImpl(program), mNative(native)
{}

CLProgramCL::~CLProgramCL()
{
    if (mNative->getDispatch().clReleaseProgram(mNative) != CL_SUCCESS)
    {
        ERR() << "Error while releasing CL program";
    }
}

angle::Result CLProgramCL::build(const cl::DevicePtrs &devices,
                                 const char *options,
                                 cl::Program *notify)
{
    std::vector<cl_device_id> nativeDevices;
    for (const cl::DevicePtr &device : devices)
    {
        nativeDevices.emplace_back(device->getImpl<CLDeviceCL>().getNative());
    }
    const cl_uint numDevices = static_cast<cl_uint>(nativeDevices.size());
    const cl_device_id *const nativeDevicesPtr =
        !nativeDevices.empty() ? nativeDevices.data() : nullptr;
    const cl::ProgramCB callback = notify != nullptr ? Callback : nullptr;
    ANGLE_CL_TRY(mNative->getDispatch().clBuildProgram(mNative, numDevices, nativeDevicesPtr,
                                                       options, callback, notify));
    return angle::Result::Continue;
}

angle::Result CLProgramCL::compile(const cl::DevicePtrs &devices,
                                   const char *options,
                                   const cl::ProgramPtrs &inputHeaders,
                                   const char **headerIncludeNames,
                                   cl::Program *notify)
{
    std::vector<cl_device_id> nativeDevices;
    for (const cl::DevicePtr &device : devices)
    {
        nativeDevices.emplace_back(device->getImpl<CLDeviceCL>().getNative());
    }
    const cl_uint numDevices = static_cast<cl_uint>(nativeDevices.size());
    const cl_device_id *const nativeDevicesPtr =
        !nativeDevices.empty() ? nativeDevices.data() : nullptr;

    std::vector<cl_program> nativePrograms;
    for (const cl::ProgramPtr &program : inputHeaders)
    {
        nativePrograms.emplace_back(program->getImpl<CLProgramCL>().getNative());
    }
    const cl_uint numInputHeaders = static_cast<cl_uint>(nativePrograms.size());
    const cl_program *const inputHeadersPtr =
        !nativePrograms.empty() ? nativePrograms.data() : nullptr;

    const cl::ProgramCB callback = notify != nullptr ? Callback : nullptr;
    ANGLE_CL_TRY(mNative->getDispatch().clCompileProgram(mNative, numDevices, nativeDevicesPtr,
                                                         options, numInputHeaders, inputHeadersPtr,
                                                         headerIncludeNames, callback, notify));
    return angle::Result::Continue;
}

angle::Result CLProgramCL::getInfo(cl::ProgramInfo name,
                                   size_t valueSize,
                                   void *value,
                                   size_t *valueSizeRet) const
{
    ANGLE_CL_TRY(mNative->getDispatch().clGetProgramInfo(mNative, cl::ToCLenum(name), valueSize,
                                                         value, valueSizeRet));
    return angle::Result::Continue;
}

angle::Result CLProgramCL::getBuildInfo(const cl::Device &device,
                                        cl::ProgramBuildInfo name,
                                        size_t valueSize,
                                        void *value,
                                        size_t *valueSizeRet) const
{
    ANGLE_CL_TRY(mNative->getDispatch().clGetProgramBuildInfo(
        mNative, device.getImpl<CLDeviceCL>().getNative(), cl::ToCLenum(name), valueSize, value,
        valueSizeRet));
    return angle::Result::Continue;
}

angle::Result CLProgramCL::createKernel(const cl::Kernel &kernel,
                                        const char *name,
                                        CLKernelImpl::Ptr *kernelOut)
{
    cl_int errorCode = CL_SUCCESS;

    const cl_kernel nativeKernel = mNative->getDispatch().clCreateKernel(mNative, name, &errorCode);
    ANGLE_CL_TRY(errorCode);

    *kernelOut =
        CLKernelImpl::Ptr(nativeKernel != nullptr ? new CLKernelCL(kernel, nativeKernel) : nullptr);
    return angle::Result::Continue;
}

angle::Result CLProgramCL::createKernels(cl_uint numKernels,
                                         CLKernelImpl::CreateFuncs &createFuncs,
                                         cl_uint *numKernelsRet)
{
    if (numKernels == 0u)
    {
        ANGLE_CL_TRY(
            mNative->getDispatch().clCreateKernelsInProgram(mNative, 0u, nullptr, numKernelsRet));
    }

    std::vector<cl_kernel> nativeKernels(numKernels, nullptr);
    ANGLE_CL_TRY(mNative->getDispatch().clCreateKernelsInProgram(
        mNative, numKernels, nativeKernels.data(), numKernelsRet));
    for (cl_kernel nativeKernel : nativeKernels)
    {
        createFuncs.emplace_back([nativeKernel](const cl::Kernel &kernel) {
            return CLKernelImpl::Ptr(new CLKernelCL(kernel, nativeKernel));
        });
    }
    return angle::Result::Continue;
}

void CLProgramCL::Callback(cl_program program, void *userData)
{
    static_cast<cl::Program *>(userData)->callback();
}

}  // namespace rx
