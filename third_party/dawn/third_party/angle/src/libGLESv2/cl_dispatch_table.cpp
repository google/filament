//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_dispatch_table_autogen.cpp: Dispatch table for CL ICD Loader.

#include "libGLESv2/cl_dispatch_table.h"

#include "libGLESv2/entry_points_cl_autogen.h"

// clang-format off

// The correct order is required as defined in 'include/CL/cl_icd.h'.
const cl_icd_dispatch gCLIcdDispatchTable = {

    // OpenCL 1.0
    cl::clGetPlatformIDs,
    cl::clGetPlatformInfo,
    cl::clGetDeviceIDs,
    cl::clGetDeviceInfo,
    cl::clCreateContext,
    cl::clCreateContextFromType,
    cl::clRetainContext,
    cl::clReleaseContext,
    cl::clGetContextInfo,
    cl::clCreateCommandQueue,
    cl::clRetainCommandQueue,
    cl::clReleaseCommandQueue,
    cl::clGetCommandQueueInfo,
    cl::clSetCommandQueueProperty,
    cl::clCreateBuffer,
    cl::clCreateImage2D,
    cl::clCreateImage3D,
    cl::clRetainMemObject,
    cl::clReleaseMemObject,
    cl::clGetSupportedImageFormats,
    cl::clGetMemObjectInfo,
    cl::clGetImageInfo,
    cl::clCreateSampler,
    cl::clRetainSampler,
    cl::clReleaseSampler,
    cl::clGetSamplerInfo,
    cl::clCreateProgramWithSource,
    cl::clCreateProgramWithBinary,
    cl::clRetainProgram,
    cl::clReleaseProgram,
    cl::clBuildProgram,
    cl::clUnloadCompiler,
    cl::clGetProgramInfo,
    cl::clGetProgramBuildInfo,
    cl::clCreateKernel,
    cl::clCreateKernelsInProgram,
    cl::clRetainKernel,
    cl::clReleaseKernel,
    cl::clSetKernelArg,
    cl::clGetKernelInfo,
    cl::clGetKernelWorkGroupInfo,
    cl::clWaitForEvents,
    cl::clGetEventInfo,
    cl::clRetainEvent,
    cl::clReleaseEvent,
    cl::clGetEventProfilingInfo,
    cl::clFlush,
    cl::clFinish,
    cl::clEnqueueReadBuffer,
    cl::clEnqueueWriteBuffer,
    cl::clEnqueueCopyBuffer,
    cl::clEnqueueReadImage,
    cl::clEnqueueWriteImage,
    cl::clEnqueueCopyImage,
    cl::clEnqueueCopyImageToBuffer,
    cl::clEnqueueCopyBufferToImage,
    cl::clEnqueueMapBuffer,
    cl::clEnqueueMapImage,
    cl::clEnqueueUnmapMemObject,
    cl::clEnqueueNDRangeKernel,
    cl::clEnqueueTask,
    cl::clEnqueueNativeKernel,
    cl::clEnqueueMarker,
    cl::clEnqueueWaitForEvents,
    cl::clEnqueueBarrier,
    cl::clGetExtensionFunctionAddress,
    nullptr, // clCreateFromGLBuffer,
    nullptr, // clCreateFromGLTexture2D,
    nullptr, // clCreateFromGLTexture3D,
    nullptr, // clCreateFromGLRenderbuffer,
    nullptr, // clGetGLObjectInfo,
    nullptr, // clGetGLTextureInfo,
    nullptr, // clEnqueueAcquireGLObjects,
    nullptr, // clEnqueueReleaseGLObjects,
    nullptr, // clGetGLContextInfoKHR,

    // cl_khr_d3d10_sharing
    nullptr, // clGetDeviceIDsFromD3D10KHR,
    nullptr, // clCreateFromD3D10BufferKHR,
    nullptr, // clCreateFromD3D10Texture2DKHR,
    nullptr, // clCreateFromD3D10Texture3DKHR,
    nullptr, // clEnqueueAcquireD3D10ObjectsKHR,
    nullptr, // clEnqueueReleaseD3D10ObjectsKHR,

    // OpenCL 1.1
    cl::clSetEventCallback,
    cl::clCreateSubBuffer,
    cl::clSetMemObjectDestructorCallback,
    cl::clCreateUserEvent,
    cl::clSetUserEventStatus,
    cl::clEnqueueReadBufferRect,
    cl::clEnqueueWriteBufferRect,
    cl::clEnqueueCopyBufferRect,

    // cl_ext_device_fission
    nullptr, // clCreateSubDevicesEXT,
    nullptr, // clRetainDeviceEXT,
    nullptr, // clReleaseDeviceEXT,

    // cl_khr_gl_event
    nullptr, // clCreateEventFromGLsyncKHR,

    // OpenCL 1.2
    cl::clCreateSubDevices,
    cl::clRetainDevice,
    cl::clReleaseDevice,
    cl::clCreateImage,
    cl::clCreateProgramWithBuiltInKernels,
    cl::clCompileProgram,
    cl::clLinkProgram,
    cl::clUnloadPlatformCompiler,
    cl::clGetKernelArgInfo,
    cl::clEnqueueFillBuffer,
    cl::clEnqueueFillImage,
    cl::clEnqueueMigrateMemObjects,
    cl::clEnqueueMarkerWithWaitList,
    cl::clEnqueueBarrierWithWaitList,
    cl::clGetExtensionFunctionAddressForPlatform,
    nullptr, // clCreateFromGLTexture,

    // cl_khr_d3d11_sharing
    nullptr, // clGetDeviceIDsFromD3D11KHR,
    nullptr, // clCreateFromD3D11BufferKHR,
    nullptr, // clCreateFromD3D11Texture2DKHR,
    nullptr, // clCreateFromD3D11Texture3DKHR,
    nullptr, // clCreateFromDX9MediaSurfaceKHR,
    nullptr, // clEnqueueAcquireD3D11ObjectsKHR,
    nullptr, // clEnqueueReleaseD3D11ObjectsKHR,

    // cl_khr_dx9_media_sharing
    nullptr, // clGetDeviceIDsFromDX9MediaAdapterKHR,
    nullptr, // clEnqueueAcquireDX9MediaSurfacesKHR,
    nullptr, // clEnqueueReleaseDX9MediaSurfacesKHR,

    // cl_khr_egl_image
    nullptr, // clCreateFromEGLImageKHR,
    nullptr, // clEnqueueAcquireEGLObjectsKHR,
    nullptr, // clEnqueueReleaseEGLObjectsKHR,

    // cl_khr_egl_event
    nullptr, // clCreateEventFromEGLSyncKHR,

    // OpenCL 2.0
    cl::clCreateCommandQueueWithProperties,
    cl::clCreatePipe,
    cl::clGetPipeInfo,
    cl::clSVMAlloc,
    cl::clSVMFree,
    cl::clEnqueueSVMFree,
    cl::clEnqueueSVMMemcpy,
    cl::clEnqueueSVMMemFill,
    cl::clEnqueueSVMMap,
    cl::clEnqueueSVMUnmap,
    cl::clCreateSamplerWithProperties,
    cl::clSetKernelArgSVMPointer,
    cl::clSetKernelExecInfo,

    // cl_khr_sub_groups
    nullptr, // clGetKernelSubGroupInfoKHR,

    // OpenCL 2.1
    cl::clCloneKernel,
    cl::clCreateProgramWithIL,
    cl::clEnqueueSVMMigrateMem,
    cl::clGetDeviceAndHostTimer,
    cl::clGetHostTimer,
    cl::clGetKernelSubGroupInfo,
    cl::clSetDefaultDeviceCommandQueue,

    // OpenCL 2.2
    cl::clSetProgramReleaseCallback,
    cl::clSetProgramSpecializationConstant,

    // OpenCL 3.0
    cl::clCreateBufferWithProperties,
    cl::clCreateImageWithProperties,
    cl::clSetContextDestructorCallback
};

// clang-format on
