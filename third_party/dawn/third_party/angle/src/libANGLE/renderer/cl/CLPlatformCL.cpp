//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatformCL.cpp: Implements the class methods for CLPlatformCL.

#include "libANGLE/renderer/cl/CLPlatformCL.h"

#include "common/angle_version_info.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLDevice.h"
#include "libANGLE/CLPlatform.h"
#include "libANGLE/cl_utils.h"
#include "libANGLE/renderer/cl/CLContextCL.h"
#include "libANGLE/renderer/cl/CLDeviceCL.h"
#include "libANGLE/renderer/cl/cl_util.h"

extern "C" {
#include "icd.h"
}  // extern "C"

namespace rx
{

namespace
{

std::string GetPlatformString(cl_platform_id platform, cl::PlatformInfo name)
{
    size_t size = 0u;
    if (platform->getDispatch().clGetPlatformInfo(platform, cl::ToCLenum(name), 0u, nullptr,
                                                  &size) == CL_SUCCESS)
    {
        std::vector<char> str(size, '\0');
        if (platform->getDispatch().clGetPlatformInfo(platform, cl::ToCLenum(name), size,
                                                      str.data(), nullptr) == CL_SUCCESS)
        {
            return std::string(str.data());
        }
    }
    ERR() << "Failed to query CL platform info for " << name;
    return std::string{};
}

}  // namespace

CLPlatformCL::~CLPlatformCL() = default;

CLPlatformImpl::Info CLPlatformCL::createInfo() const
{
    // Verify that the platform is valid.
    // We ignore clGetPlatformIDs for ICD case since the ICD Loader has already verified
    // clIcdGetPlatformIDsKHR exists and is valid.
    if (mNative == nullptr || (!mIsIcd && mNative->getDispatch().clGetPlatformIDs == nullptr) ||
        mNative->getDispatch().clGetPlatformInfo == nullptr ||
        mNative->getDispatch().clGetDeviceIDs == nullptr ||
        mNative->getDispatch().clGetDeviceInfo == nullptr ||
        mNative->getDispatch().clCreateContext == nullptr ||
        mNative->getDispatch().clCreateContextFromType == nullptr ||
        mNative->getDispatch().clRetainContext == nullptr ||
        mNative->getDispatch().clReleaseContext == nullptr ||
        mNative->getDispatch().clGetContextInfo == nullptr ||
        mNative->getDispatch().clCreateCommandQueue == nullptr ||
        mNative->getDispatch().clRetainCommandQueue == nullptr ||
        mNative->getDispatch().clReleaseCommandQueue == nullptr ||
        mNative->getDispatch().clGetCommandQueueInfo == nullptr ||
        mNative->getDispatch().clSetCommandQueueProperty == nullptr ||
        mNative->getDispatch().clCreateBuffer == nullptr ||
        mNative->getDispatch().clCreateImage2D == nullptr ||
        mNative->getDispatch().clCreateImage3D == nullptr ||
        mNative->getDispatch().clRetainMemObject == nullptr ||
        mNative->getDispatch().clReleaseMemObject == nullptr ||
        mNative->getDispatch().clGetSupportedImageFormats == nullptr ||
        mNative->getDispatch().clGetMemObjectInfo == nullptr ||
        mNative->getDispatch().clGetImageInfo == nullptr ||
        mNative->getDispatch().clCreateSampler == nullptr ||
        mNative->getDispatch().clRetainSampler == nullptr ||
        mNative->getDispatch().clReleaseSampler == nullptr ||
        mNative->getDispatch().clGetSamplerInfo == nullptr ||
        mNative->getDispatch().clCreateProgramWithSource == nullptr ||
        mNative->getDispatch().clCreateProgramWithBinary == nullptr ||
        mNative->getDispatch().clRetainProgram == nullptr ||
        mNative->getDispatch().clReleaseProgram == nullptr ||
        mNative->getDispatch().clBuildProgram == nullptr ||
        mNative->getDispatch().clUnloadCompiler == nullptr ||
        mNative->getDispatch().clGetProgramInfo == nullptr ||
        mNative->getDispatch().clGetProgramBuildInfo == nullptr ||
        mNative->getDispatch().clCreateKernel == nullptr ||
        mNative->getDispatch().clCreateKernelsInProgram == nullptr ||
        mNative->getDispatch().clRetainKernel == nullptr ||
        mNative->getDispatch().clReleaseKernel == nullptr ||
        mNative->getDispatch().clSetKernelArg == nullptr ||
        mNative->getDispatch().clGetKernelInfo == nullptr ||
        mNative->getDispatch().clGetKernelWorkGroupInfo == nullptr ||
        mNative->getDispatch().clWaitForEvents == nullptr ||
        mNative->getDispatch().clGetEventInfo == nullptr ||
        mNative->getDispatch().clRetainEvent == nullptr ||
        mNative->getDispatch().clReleaseEvent == nullptr ||
        mNative->getDispatch().clGetEventProfilingInfo == nullptr ||
        mNative->getDispatch().clFlush == nullptr || mNative->getDispatch().clFinish == nullptr ||
        mNative->getDispatch().clEnqueueReadBuffer == nullptr ||
        mNative->getDispatch().clEnqueueWriteBuffer == nullptr ||
        mNative->getDispatch().clEnqueueCopyBuffer == nullptr ||
        mNative->getDispatch().clEnqueueReadImage == nullptr ||
        mNative->getDispatch().clEnqueueWriteImage == nullptr ||
        mNative->getDispatch().clEnqueueCopyImage == nullptr ||
        mNative->getDispatch().clEnqueueCopyImageToBuffer == nullptr ||
        mNative->getDispatch().clEnqueueCopyBufferToImage == nullptr ||
        mNative->getDispatch().clEnqueueMapBuffer == nullptr ||
        mNative->getDispatch().clEnqueueMapImage == nullptr ||
        mNative->getDispatch().clEnqueueUnmapMemObject == nullptr ||
        mNative->getDispatch().clEnqueueNDRangeKernel == nullptr ||
        mNative->getDispatch().clEnqueueTask == nullptr ||
        mNative->getDispatch().clEnqueueNativeKernel == nullptr ||
        mNative->getDispatch().clEnqueueMarker == nullptr ||
        mNative->getDispatch().clEnqueueWaitForEvents == nullptr ||
        mNative->getDispatch().clEnqueueBarrier == nullptr ||
        mNative->getDispatch().clGetExtensionFunctionAddress == nullptr)
    {
        ERR() << "Missing entry points for OpenCL 1.0";
        return Info{};
    }

    // Fetch common platform info
    Info info;
    const std::string vendor = GetPlatformString(mNative, cl::PlatformInfo::Vendor);
    info.profile             = GetPlatformString(mNative, cl::PlatformInfo::Profile);
    info.versionStr          = GetPlatformString(mNative, cl::PlatformInfo::Version);
    info.name                = GetPlatformString(mNative, cl::PlatformInfo::Name);
    std::string extensionStr = GetPlatformString(mNative, cl::PlatformInfo::Extensions);

    if (vendor.empty() || info.profile.empty() || info.versionStr.empty() || info.name.empty() ||
        extensionStr.empty())
    {
        return Info{};
    }

    // Skip ANGLE CL implementation to prevent passthrough loop
    if (vendor.compare(cl::Platform::GetVendor()) == 0)
    {
        ERR() << "Tried to create CL pass-through back end for ANGLE library";
        return Info{};
    }

    // TODO(jplate) Remove workaround after bug is fixed http://anglebug.com/42264583
    if (info.versionStr.compare(0u, 15u, "OpenCL 3.0 CUDA", 15u) == 0)
    {
        extensionStr.append(" cl_khr_depth_images cl_khr_image2d_from_buffer");
    }

    const cl_version version = ExtractCLVersion(info.versionStr);
    if (version == 0u)
    {
        return Info{};
    }

    // Remove unsupported and initialize extensions
    RemoveUnsupportedCLExtensions(extensionStr);
    info.initializeExtensions(std::move(extensionStr));

    // Skip platform if it is not ICD compatible
    if (!info.khrICD)
    {
        WARN() << "CL platform is not ICD compatible";
        return Info{};
    }

    // Customize version string and name
    info.versionStr += std::string(" (ANGLE ") + angle::GetANGLEVersionString() + ")";
    info.name.insert(0u, "ANGLE pass-through -> ");

    if (version >= CL_MAKE_VERSION(2, 1, 0) &&
        mNative->getDispatch().clGetPlatformInfo(mNative, CL_PLATFORM_HOST_TIMER_RESOLUTION,
                                                 sizeof(info.hostTimerRes), &info.hostTimerRes,
                                                 nullptr) != CL_SUCCESS)
    {
        ERR() << "Failed to query CL platform info for CL_PLATFORM_HOST_TIMER_RESOLUTION";
        return Info{};
    }

    if (version < CL_MAKE_VERSION(3, 0, 0))
    {
        info.version = version;
    }
    else
    {
        if (mNative->getDispatch().clGetPlatformInfo(mNative, CL_PLATFORM_NUMERIC_VERSION,
                                                     sizeof(info.version), &info.version,
                                                     nullptr) != CL_SUCCESS)
        {
            ERR() << "Failed to query CL platform info for CL_PLATFORM_NUMERIC_VERSION";
            return Info{};
        }
        else if (CL_VERSION_MAJOR(info.version) != CL_VERSION_MAJOR(version) ||
                 CL_VERSION_MINOR(info.version) != CL_VERSION_MINOR(version))
        {
            WARN() << "CL_PLATFORM_NUMERIC_VERSION = " << CL_VERSION_MAJOR(info.version) << '.'
                   << CL_VERSION_MINOR(info.version)
                   << " does not match version string: " << info.versionStr;
        }

        size_t valueSize = 0u;
        if (mNative->getDispatch().clGetPlatformInfo(mNative, CL_PLATFORM_EXTENSIONS_WITH_VERSION,
                                                     0u, nullptr, &valueSize) != CL_SUCCESS ||
            (valueSize % sizeof(decltype(info.extensionsWithVersion)::value_type)) != 0u)
        {
            ERR() << "Failed to query CL platform info for CL_PLATFORM_EXTENSIONS_WITH_VERSION";
            return Info{};
        }
        info.extensionsWithVersion.resize(valueSize /
                                          sizeof(decltype(info.extensionsWithVersion)::value_type));
        if (mNative->getDispatch().clGetPlatformInfo(mNative, CL_PLATFORM_EXTENSIONS_WITH_VERSION,
                                                     valueSize, info.extensionsWithVersion.data(),
                                                     nullptr) != CL_SUCCESS)
        {
            ERR() << "Failed to query CL platform info for CL_PLATFORM_EXTENSIONS_WITH_VERSION";
            return Info{};
        }
        RemoveUnsupportedCLExtensions(info.extensionsWithVersion);
    }

    if (info.version >= CL_MAKE_VERSION(1, 1, 0) &&
        (mNative->getDispatch().clSetEventCallback == nullptr ||
         mNative->getDispatch().clCreateSubBuffer == nullptr ||
         mNative->getDispatch().clSetMemObjectDestructorCallback == nullptr ||
         mNative->getDispatch().clCreateUserEvent == nullptr ||
         mNative->getDispatch().clSetUserEventStatus == nullptr ||
         mNative->getDispatch().clEnqueueReadBufferRect == nullptr ||
         mNative->getDispatch().clEnqueueWriteBufferRect == nullptr ||
         mNative->getDispatch().clEnqueueCopyBufferRect == nullptr))
    {
        ERR() << "Missing entry points for OpenCL 1.1";
        return Info{};
    }

    if (info.version >= CL_MAKE_VERSION(1, 2, 0) &&
        (mNative->getDispatch().clCreateSubDevices == nullptr ||
         mNative->getDispatch().clRetainDevice == nullptr ||
         mNative->getDispatch().clReleaseDevice == nullptr ||
         mNative->getDispatch().clCreateImage == nullptr ||
         mNative->getDispatch().clCreateProgramWithBuiltInKernels == nullptr ||
         mNative->getDispatch().clCompileProgram == nullptr ||
         mNative->getDispatch().clLinkProgram == nullptr ||
         mNative->getDispatch().clUnloadPlatformCompiler == nullptr ||
         mNative->getDispatch().clGetKernelArgInfo == nullptr ||
         mNative->getDispatch().clEnqueueFillBuffer == nullptr ||
         mNative->getDispatch().clEnqueueFillImage == nullptr ||
         mNative->getDispatch().clEnqueueMigrateMemObjects == nullptr ||
         mNative->getDispatch().clEnqueueMarkerWithWaitList == nullptr ||
         mNative->getDispatch().clEnqueueBarrierWithWaitList == nullptr ||
         mNative->getDispatch().clGetExtensionFunctionAddressForPlatform == nullptr))
    {
        ERR() << "Missing entry points for OpenCL 1.2";
        return Info{};
    }

    if (info.version >= CL_MAKE_VERSION(2, 0, 0) &&
        (mNative->getDispatch().clCreateCommandQueueWithProperties == nullptr ||
         mNative->getDispatch().clCreatePipe == nullptr ||
         mNative->getDispatch().clGetPipeInfo == nullptr ||
         mNative->getDispatch().clSVMAlloc == nullptr ||
         mNative->getDispatch().clSVMFree == nullptr ||
         mNative->getDispatch().clEnqueueSVMFree == nullptr ||
         mNative->getDispatch().clEnqueueSVMMemcpy == nullptr ||
         mNative->getDispatch().clEnqueueSVMMemFill == nullptr ||
         mNative->getDispatch().clEnqueueSVMMap == nullptr ||
         mNative->getDispatch().clEnqueueSVMUnmap == nullptr ||
         mNative->getDispatch().clCreateSamplerWithProperties == nullptr ||
         mNative->getDispatch().clSetKernelArgSVMPointer == nullptr ||
         mNative->getDispatch().clSetKernelExecInfo == nullptr))
    {
        ERR() << "Missing entry points for OpenCL 2.0";
        return Info{};
    }

    if (info.version >= CL_MAKE_VERSION(2, 1, 0) &&
        (mNative->getDispatch().clCloneKernel == nullptr ||
         mNative->getDispatch().clCreateProgramWithIL == nullptr ||
         mNative->getDispatch().clEnqueueSVMMigrateMem == nullptr ||
         mNative->getDispatch().clGetDeviceAndHostTimer == nullptr ||
         mNative->getDispatch().clGetHostTimer == nullptr ||
         mNative->getDispatch().clGetKernelSubGroupInfo == nullptr ||
         mNative->getDispatch().clSetDefaultDeviceCommandQueue == nullptr))
    {
        ERR() << "Missing entry points for OpenCL 2.1";
        return Info{};
    }

    if (info.version >= CL_MAKE_VERSION(2, 2, 0) &&
        (mNative->getDispatch().clSetProgramReleaseCallback == nullptr ||
         mNative->getDispatch().clSetProgramSpecializationConstant == nullptr))
    {
        ERR() << "Missing entry points for OpenCL 2.2";
        return Info{};
    }

    if (info.version >= CL_MAKE_VERSION(3, 0, 0) &&
        (mNative->getDispatch().clCreateBufferWithProperties == nullptr ||
         mNative->getDispatch().clCreateImageWithProperties == nullptr ||
         mNative->getDispatch().clSetContextDestructorCallback == nullptr))
    {
        ERR() << "Missing entry points for OpenCL 3.0";
        return Info{};
    }

    return info;
}

CLDeviceImpl::CreateDatas CLPlatformCL::createDevices() const
{
    CLDeviceImpl::CreateDatas createDatas;

    // Fetch all regular devices. This does not include CL_DEVICE_TYPE_CUSTOM, which are not
    // supported by the CL pass-through back end because they have no standard feature set.
    // This makes them unreliable for the purpose of this back end.
    cl_uint numDevices = 0u;
    if (mNative->getDispatch().clGetDeviceIDs(mNative, CL_DEVICE_TYPE_ALL, 0u, nullptr,
                                              &numDevices) == CL_SUCCESS)
    {
        std::vector<cl_device_id> nativeDevices(numDevices, nullptr);
        if (mNative->getDispatch().clGetDeviceIDs(mNative, CL_DEVICE_TYPE_ALL, numDevices,
                                                  nativeDevices.data(), nullptr) == CL_SUCCESS)
        {
            // Fetch all device types for front end initialization, and find the default device.
            // If none exists declare first device as default.
            std::vector<cl::DeviceType> types(nativeDevices.size());
            size_t defaultIndex = 0u;
            for (size_t index = 0u; index < nativeDevices.size(); ++index)
            {
                if (nativeDevices[index]->getDispatch().clGetDeviceInfo(
                        nativeDevices[index], CL_DEVICE_TYPE, sizeof(cl_device_type), &types[index],
                        nullptr) == CL_SUCCESS)
                {
                    // If default device found, select it
                    if (types[index].intersects(CL_DEVICE_TYPE_DEFAULT))
                    {
                        defaultIndex = index;
                    }
                }
                else
                {
                    types.clear();
                    nativeDevices.clear();
                }
            }

            for (size_t index = 0u; index < nativeDevices.size(); ++index)
            {
                // Make sure the default bit is set in exactly one device
                if (index == defaultIndex)
                {
                    types[index].set(CL_DEVICE_TYPE_DEFAULT);
                }
                else
                {
                    types[index].clear(CL_DEVICE_TYPE_DEFAULT);
                }

                cl_device_id nativeDevice = nativeDevices[index];
                createDatas.emplace_back(types[index], [nativeDevice](const cl::Device &device) {
                    return CLDeviceCL::Ptr(new CLDeviceCL(device, nativeDevice));
                });
            }
        }
    }

    if (createDatas.empty())
    {
        ERR() << "Failed to query CL devices";
    }
    return createDatas;
}

angle::Result CLPlatformCL::createContext(cl::Context &context,
                                          const cl::DevicePtrs &devices,
                                          bool userSync,
                                          CLContextImpl::Ptr *contextOut)
{
    cl_int errorCode                   = CL_SUCCESS;
    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(mNative),
        userSync && mPlatform.isVersionOrNewer(1u, 2u) ? CL_CONTEXT_INTEROP_USER_SYNC : 0, CL_TRUE,
        0};

    std::vector<cl_device_id> nativeDevices;
    for (const cl::DevicePtr &device : devices)
    {
        nativeDevices.emplace_back(device->getImpl<CLDeviceCL>().getNative());
    }

    cl_context nativeContext = mNative->getDispatch().clCreateContext(
        properties, static_cast<cl_uint>(nativeDevices.size()), nativeDevices.data(),
        cl::Context::ErrorCallback, &context, &errorCode);
    ANGLE_CL_TRY(errorCode);

    *contextOut = CLContextImpl::Ptr(
        nativeContext != nullptr ? new CLContextCL(context, nativeContext) : nullptr);
    return angle::Result::Continue;
}

angle::Result CLPlatformCL::createContextFromType(cl::Context &context,
                                                  cl::DeviceType deviceType,
                                                  bool userSync,
                                                  CLContextImpl::Ptr *contextOut)
{
    cl_int errorCode                   = CL_SUCCESS;
    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(mNative),
        userSync && mPlatform.isVersionOrNewer(1u, 2u) ? CL_CONTEXT_INTEROP_USER_SYNC : 0, CL_TRUE,
        0};

    cl_context nativeContext = mNative->getDispatch().clCreateContextFromType(
        properties, deviceType.get(), cl::Context::ErrorCallback, &context, &errorCode);
    ANGLE_CL_TRY(errorCode);

    *contextOut = CLContextImpl::Ptr(
        nativeContext != nullptr ? new CLContextCL(context, nativeContext) : nullptr);
    return angle::Result::Continue;
}

angle::Result CLPlatformCL::unloadCompiler()
{
    ANGLE_CL_TRY(mNative->getDispatch().clUnloadPlatformCompiler(mNative));
    return angle::Result::Continue;
}

void CLPlatformCL::Initialize(CreateFuncs &createFuncs, bool isIcd)
{
    // Using khrIcdInitialize() of the third party Khronos OpenCL ICD Loader to
    // enumerate the available OpenCL implementations on the system. They will be
    // stored in the singly linked list khrIcdVendors of the C struct KHRicdVendor.
    khrIcdInitialize();

    // The ICD loader will also enumerate ANGLE's OpenCL library if it is registered. Our
    // OpenCL entry points for the ICD enumeration are reentrant, but at this point of the
    // initialization there are no platforms available, so our platforms will not be found.
    // This is intended as this back end should only enumerate non-ANGLE implementations.

    // Iterating through the singly linked list khrIcdVendors to create
    // an ANGLE CL pass-through platform for each found ICD platform.
    for (KHRicdVendor *vendorIt = khrIcdVendors; vendorIt != nullptr; vendorIt = vendorIt->next)
    {
        cl_platform_id nativePlatform = vendorIt->platform;
        createFuncs.emplace_back([nativePlatform, isIcd](const cl::Platform &platform) {
            return Ptr(new CLPlatformCL(platform, nativePlatform, isIcd));
        });
    }
}

CLPlatformCL::CLPlatformCL(const cl::Platform &platform, cl_platform_id native, bool isIcd)
    : CLPlatformImpl(platform), mNative(native), mIsIcd(isIcd)
{}

}  // namespace rx
