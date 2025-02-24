#pragma once
#ifdef CL_PROTOTYPES
#undef CL_PROTOTYPES
#endif
#ifndef CL_NO_PROTOTYPES
#define CL_NO_PROTOTYPES
#endif
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif
#include <stdio.h>
#ifdef __APPLE__
#define CL_API_ENTRY
#define CL_API_CALL
#define CL_CALLBACK
#include <OpenCL/opencl.h>
#define CL_EXT_SUFFIX__VERSION_1_2_DEPRECATED
#define CL_EXT_PREFIX__VERSION_1_2_DEPRECATED

#else
#include <CL/opencl.h>
#endif
#include <CL/cl_egl.h>

#include "pvr_openlib.h"

#ifndef DYNAMICCL_NO_NAMESPACE
#define DEFINE_CL_FUNCTION_NAME(name) CL_API_ENTRY name
#else
#define DEFINE_CL_FUNCTION_NAME(name) CL_API_ENTRY cl##name
#endif

/** DEFINE_CL_FUNCTION_NAME THE PLATFORM SPECIFIC LIBRARY NAME **/
namespace cl {
namespace internals {
#ifdef _WIN32
static const char* clLibName = "OpenCL.dll";
#elif defined(TARGET_OS_MAC)
static const char* clLibName = "/System/Library/Frameworks/OpenCL.framework/OpenCL";
#else
static const char* clLibName = "libOpenCL.so";
#endif
static const char* clLibAltName = "libPVROCL.so";
} // namespace internals
} // namespace cl

namespace cl {
namespace CLFunctions {
enum Enum
{
	GetPlatformIDs,
	GetPlatformInfo,
	GetDeviceIDs,
	GetDeviceInfo,
	CreateSubDevices,
	RetainDevice,
	ReleaseDevice,
	SetDefaultDeviceCommandQueue,
	GetDeviceAndHostTimer,
	GetHostTimer,
	CreateContext,
	CreateContextFromType,
	RetainContext,
	ReleaseContext,
	GetContextInfo,
	CreateCommandQueueWithProperties,
	RetainCommandQueue,
	ReleaseCommandQueue,
	GetCommandQueueInfo,
	CreateBuffer,
	CreateSubBuffer,
	CreateImage,
	CreatePipe,
	RetainMemObject,
	ReleaseMemObject,
	GetSupportedImageFormats,
	GetMemObjectInfo,
	GetImageInfo,
	GetPipeInfo,
	SetMemObjectDestructorCallback,
	SVMAlloc,
	SVMFree,
	CreateSamplerWithProperties,
	RetainSampler,
	ReleaseSampler,
	GetSamplerInfo,
	CreateProgramWithSource,
	CreateProgramWithBinary,
	CreateProgramWithBuiltInKernels,
	CreateProgramWithIL,
	RetainProgram,
	ReleaseProgram,
	BuildProgram,
	CompileProgram,
	LinkProgram,
	SetProgramReleaseCallback,
	SetProgramSpecializationConstant,
	UnloadPlatformCompiler,
	GetProgramInfo,
	GetProgramBuildInfo,
	CreateKernel,
	CreateKernelsInProgram,
	CloneKernel,
	RetainKernel,
	ReleaseKernel,
	SetKernelArg,
	SetKernelArgSVMPointer,
	SetKernelExecInfo,
	GetKernelInfo,
	GetKernelArgInfo,
	GetKernelWorkGroupInfo,
	GetKernelSubGroupInfo,
	WaitForEvents,
	GetEventInfo,
	CreateUserEvent,
	RetainEvent,
	ReleaseEvent,
	SetUserEventStatus,
	SetEventCallback,
	GetEventProfilingInfo,
	Flush,
	Finish,
	EnqueueReadBuffer,
	EnqueueReadBufferRect,
	EnqueueWriteBuffer,
	EnqueueWriteBufferRect,
	EnqueueFillBuffer,
	EnqueueCopyBuffer,
	EnqueueCopyBufferRect,
	EnqueueReadImage,
	EnqueueWriteImage,
	EnqueueFillImage,
	EnqueueCopyImage,
	EnqueueCopyImageToBuffer,
	EnqueueCopyBufferToImage,
	EnqueueMapBuffer,
	EnqueueMapImage,
	EnqueueUnmapMemObject,
	EnqueueMigrateMemObjects,
	EnqueueNDRangeKernel,
	EnqueueNativeKernel,
	EnqueueMarkerWithWaitList,
	EnqueueBarrierWithWaitList,
	EnqueueSVMFree,
	EnqueueSVMMemcpy,
	EnqueueSVMMemFill,
	EnqueueSVMMap,
	EnqueueSVMUnmap,
	EnqueueSVMMigrateMem,
	GetExtensionFunctionAddressForPlatform,
	CreateImage2D,
	CreateImage3D,
	EnqueueMarker,
	EnqueueWaitForEvents,
	EnqueueBarrier,
	UnloadCompiler,
	GetExtensionFunctionAddress,
	CreateCommandQueue,
	CreateSampler,
	EnqueueTask,
	CreateFromGLBuffer,
	CreateFromGLTexture,
	CreateFromGLRenderbuffer,
	GetGLObjectInfo,
	GetGLTextureInfo,
	EnqueueAcquireGLObjects,
	EnqueueReleaseGLObjects,
	CreateFromGLTexture2D,
	CreateFromGLTexture3D,
	GetGLContextInfoKHR,
	NUMBER_OF_CL_FUNCTIONS
};
}

namespace internals {
inline void* getClFunction(CLFunctions::Enum func)
{
	static void* CLFunctionTable[CLFunctions::NUMBER_OF_CL_FUNCTIONS];

	if (CLFunctionTable[0] == NULL)
	{
		pvr::lib::LIBTYPE lib = pvr::lib::openlib(clLibName);
		if (lib) { Log_Info("OpenCL Bindings: Successfully loaded library %s for OpenCL", clLibName); }
		else
		{
			lib = pvr::lib::openlib(clLibAltName);
			if (lib) { Log_Info("OpenCL Bindings: Successfully loaded alternative library %s for OpenCL, after %s failed", clLibAltName, clLibName); }
			else
			{
				Log_Error("OpenCL Bindings: Failed to open library %s", clLibName);
				return nullptr;
			}
		}

		CLFunctionTable[CLFunctions::GetPlatformIDs] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetPlatformIDs");
		CLFunctionTable[CLFunctions::GetPlatformInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetPlatformInfo");
		CLFunctionTable[CLFunctions::GetDeviceIDs] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetDeviceIDs");
		CLFunctionTable[CLFunctions::GetDeviceInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetDeviceInfo");
		CLFunctionTable[CLFunctions::CreateSubDevices] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateSubDevices");
		CLFunctionTable[CLFunctions::RetainDevice] = pvr::lib::getLibFunctionChecked<void*>(lib, "clRetainDevice");
		CLFunctionTable[CLFunctions::ReleaseDevice] = pvr::lib::getLibFunctionChecked<void*>(lib, "clReleaseDevice");
		CLFunctionTable[CLFunctions::SetDefaultDeviceCommandQueue] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSetDefaultDeviceCommandQueue");
		CLFunctionTable[CLFunctions::GetDeviceAndHostTimer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetDeviceAndHostTimer");
		CLFunctionTable[CLFunctions::GetHostTimer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetHostTimer");
		CLFunctionTable[CLFunctions::CreateContext] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateContext");
		CLFunctionTable[CLFunctions::CreateContextFromType] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateContextFromType");
		CLFunctionTable[CLFunctions::RetainContext] = pvr::lib::getLibFunctionChecked<void*>(lib, "clRetainContext");
		CLFunctionTable[CLFunctions::ReleaseContext] = pvr::lib::getLibFunctionChecked<void*>(lib, "clReleaseContext");
		CLFunctionTable[CLFunctions::GetContextInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetContextInfo");
		CLFunctionTable[CLFunctions::CreateCommandQueueWithProperties] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateCommandQueueWithProperties");
		CLFunctionTable[CLFunctions::RetainCommandQueue] = pvr::lib::getLibFunctionChecked<void*>(lib, "clRetainCommandQueue");
		CLFunctionTable[CLFunctions::ReleaseCommandQueue] = pvr::lib::getLibFunctionChecked<void*>(lib, "clReleaseCommandQueue");
		CLFunctionTable[CLFunctions::GetCommandQueueInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetCommandQueueInfo");
		CLFunctionTable[CLFunctions::CreateBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateBuffer");
		CLFunctionTable[CLFunctions::CreateSubBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateSubBuffer");
		CLFunctionTable[CLFunctions::CreateImage] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateImage");
		CLFunctionTable[CLFunctions::CreatePipe] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreatePipe");
		CLFunctionTable[CLFunctions::RetainMemObject] = pvr::lib::getLibFunctionChecked<void*>(lib, "clRetainMemObject");
		CLFunctionTable[CLFunctions::ReleaseMemObject] = pvr::lib::getLibFunctionChecked<void*>(lib, "clReleaseMemObject");
		CLFunctionTable[CLFunctions::GetSupportedImageFormats] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetSupportedImageFormats");
		CLFunctionTable[CLFunctions::GetMemObjectInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetMemObjectInfo");
		CLFunctionTable[CLFunctions::GetImageInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetImageInfo");
		CLFunctionTable[CLFunctions::GetPipeInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetPipeInfo");
		CLFunctionTable[CLFunctions::SetMemObjectDestructorCallback] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSetMemObjectDestructorCallback");
		CLFunctionTable[CLFunctions::SVMAlloc] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSVMAlloc");
		CLFunctionTable[CLFunctions::SVMFree] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSVMFree");
		CLFunctionTable[CLFunctions::CreateSamplerWithProperties] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateSamplerWithProperties");
		CLFunctionTable[CLFunctions::RetainSampler] = pvr::lib::getLibFunctionChecked<void*>(lib, "clRetainSampler");
		CLFunctionTable[CLFunctions::ReleaseSampler] = pvr::lib::getLibFunctionChecked<void*>(lib, "clReleaseSampler");
		CLFunctionTable[CLFunctions::GetSamplerInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetSamplerInfo");
		CLFunctionTable[CLFunctions::CreateProgramWithSource] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateProgramWithSource");
		CLFunctionTable[CLFunctions::CreateProgramWithBinary] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateProgramWithBinary");
		CLFunctionTable[CLFunctions::CreateProgramWithBuiltInKernels] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateProgramWithBuiltInKernels");
		CLFunctionTable[CLFunctions::CreateProgramWithIL] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateProgramWithIL");
		CLFunctionTable[CLFunctions::RetainProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "clRetainProgram");
		CLFunctionTable[CLFunctions::ReleaseProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "clReleaseProgram");
		CLFunctionTable[CLFunctions::BuildProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "clBuildProgram");
		CLFunctionTable[CLFunctions::CompileProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCompileProgram");
		CLFunctionTable[CLFunctions::LinkProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "clLinkProgram");
		CLFunctionTable[CLFunctions::SetProgramReleaseCallback] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSetProgramReleaseCallback");
		CLFunctionTable[CLFunctions::SetProgramSpecializationConstant] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSetProgramSpecializationConstant");
		CLFunctionTable[CLFunctions::UnloadPlatformCompiler] = pvr::lib::getLibFunctionChecked<void*>(lib, "clUnloadPlatformCompiler");
		CLFunctionTable[CLFunctions::GetProgramInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetProgramInfo");
		CLFunctionTable[CLFunctions::GetProgramBuildInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetProgramBuildInfo");
		CLFunctionTable[CLFunctions::CreateKernel] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateKernel");
		CLFunctionTable[CLFunctions::CreateKernelsInProgram] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateKernelsInProgram");
		CLFunctionTable[CLFunctions::CloneKernel] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCloneKernel");
		CLFunctionTable[CLFunctions::RetainKernel] = pvr::lib::getLibFunctionChecked<void*>(lib, "clRetainKernel");
		CLFunctionTable[CLFunctions::ReleaseKernel] = pvr::lib::getLibFunctionChecked<void*>(lib, "clReleaseKernel");
		CLFunctionTable[CLFunctions::SetKernelArg] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSetKernelArg");
		CLFunctionTable[CLFunctions::SetKernelArgSVMPointer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSetKernelArgSVMPointer");
		CLFunctionTable[CLFunctions::SetKernelExecInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSetKernelExecInfo");
		CLFunctionTable[CLFunctions::GetKernelInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetKernelInfo");
		CLFunctionTable[CLFunctions::GetKernelArgInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetKernelArgInfo");
		CLFunctionTable[CLFunctions::GetKernelWorkGroupInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetKernelWorkGroupInfo");
		CLFunctionTable[CLFunctions::GetKernelSubGroupInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetKernelSubGroupInfo");
		CLFunctionTable[CLFunctions::WaitForEvents] = pvr::lib::getLibFunctionChecked<void*>(lib, "clWaitForEvents");
		CLFunctionTable[CLFunctions::GetEventInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetEventInfo");
		CLFunctionTable[CLFunctions::CreateUserEvent] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateUserEvent");
		CLFunctionTable[CLFunctions::RetainEvent] = pvr::lib::getLibFunctionChecked<void*>(lib, "clRetainEvent");
		CLFunctionTable[CLFunctions::ReleaseEvent] = pvr::lib::getLibFunctionChecked<void*>(lib, "clReleaseEvent");
		CLFunctionTable[CLFunctions::SetUserEventStatus] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSetUserEventStatus");
		CLFunctionTable[CLFunctions::SetEventCallback] = pvr::lib::getLibFunctionChecked<void*>(lib, "clSetEventCallback");
		CLFunctionTable[CLFunctions::GetEventProfilingInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetEventProfilingInfo");
		CLFunctionTable[CLFunctions::Flush] = pvr::lib::getLibFunctionChecked<void*>(lib, "clFlush");
		CLFunctionTable[CLFunctions::Finish] = pvr::lib::getLibFunctionChecked<void*>(lib, "clFinish");
		CLFunctionTable[CLFunctions::EnqueueReadBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueReadBuffer");
		CLFunctionTable[CLFunctions::EnqueueReadBufferRect] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueReadBufferRect");
		CLFunctionTable[CLFunctions::EnqueueWriteBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueWriteBuffer");
		CLFunctionTable[CLFunctions::EnqueueWriteBufferRect] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueWriteBufferRect");
		CLFunctionTable[CLFunctions::EnqueueFillBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueFillBuffer");
		CLFunctionTable[CLFunctions::EnqueueCopyBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueCopyBuffer");
		CLFunctionTable[CLFunctions::EnqueueCopyBufferRect] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueCopyBufferRect");
		CLFunctionTable[CLFunctions::EnqueueReadImage] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueReadImage");
		CLFunctionTable[CLFunctions::EnqueueWriteImage] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueWriteImage");
		CLFunctionTable[CLFunctions::EnqueueFillImage] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueFillImage");
		CLFunctionTable[CLFunctions::EnqueueCopyImage] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueCopyImage");
		CLFunctionTable[CLFunctions::EnqueueCopyImageToBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueCopyImageToBuffer");
		CLFunctionTable[CLFunctions::EnqueueCopyBufferToImage] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueCopyBufferToImage");
		CLFunctionTable[CLFunctions::EnqueueMapBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueMapBuffer");
		CLFunctionTable[CLFunctions::EnqueueMapImage] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueMapImage");
		CLFunctionTable[CLFunctions::EnqueueUnmapMemObject] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueUnmapMemObject");
		CLFunctionTable[CLFunctions::EnqueueMigrateMemObjects] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueMigrateMemObjects");
		CLFunctionTable[CLFunctions::EnqueueNDRangeKernel] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueNDRangeKernel");
		CLFunctionTable[CLFunctions::EnqueueNativeKernel] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueNativeKernel");
		CLFunctionTable[CLFunctions::EnqueueMarkerWithWaitList] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueMarkerWithWaitList");
		CLFunctionTable[CLFunctions::EnqueueBarrierWithWaitList] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueBarrierWithWaitList");
		CLFunctionTable[CLFunctions::EnqueueSVMFree] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueSVMFree");
		CLFunctionTable[CLFunctions::EnqueueSVMMemcpy] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueSVMMemcpy");
		CLFunctionTable[CLFunctions::EnqueueSVMMemFill] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueSVMMemFill");
		CLFunctionTable[CLFunctions::EnqueueSVMMap] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueSVMMap");
		CLFunctionTable[CLFunctions::EnqueueSVMUnmap] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueSVMUnmap");
		CLFunctionTable[CLFunctions::EnqueueSVMMigrateMem] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueSVMMigrateMem");
		CLFunctionTable[CLFunctions::GetExtensionFunctionAddressForPlatform] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetExtensionFunctionAddressForPlatform");
		CLFunctionTable[CLFunctions::CreateImage2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateImage2D");
		CLFunctionTable[CLFunctions::CreateImage3D] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateImage3D");
		CLFunctionTable[CLFunctions::EnqueueMarker] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueMarker");
		CLFunctionTable[CLFunctions::EnqueueWaitForEvents] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueWaitForEvents");
		CLFunctionTable[CLFunctions::EnqueueBarrier] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueBarrier");
		CLFunctionTable[CLFunctions::UnloadCompiler] = pvr::lib::getLibFunctionChecked<void*>(lib, "clUnloadCompiler");
		CLFunctionTable[CLFunctions::GetExtensionFunctionAddress] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetExtensionFunctionAddress");
		CLFunctionTable[CLFunctions::CreateCommandQueue] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateCommandQueue");
		CLFunctionTable[CLFunctions::CreateSampler] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateSampler");
		CLFunctionTable[CLFunctions::EnqueueTask] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueTask");
		CLFunctionTable[CLFunctions::CreateFromGLBuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateFromGLBuffer");
		CLFunctionTable[CLFunctions::CreateFromGLTexture] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateFromGLTexture");
		CLFunctionTable[CLFunctions::CreateFromGLRenderbuffer] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateFromGLRenderbuffer");
		CLFunctionTable[CLFunctions::GetGLObjectInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetGLObjectInfo");
		CLFunctionTable[CLFunctions::GetGLTextureInfo] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetGLTextureInfo");
		CLFunctionTable[CLFunctions::EnqueueAcquireGLObjects] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueAcquireGLObjects");
		CLFunctionTable[CLFunctions::EnqueueReleaseGLObjects] = pvr::lib::getLibFunctionChecked<void*>(lib, "clEnqueueReleaseGLObjects");
		CLFunctionTable[CLFunctions::CreateFromGLTexture2D] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateFromGLTexture2D");
		CLFunctionTable[CLFunctions::CreateFromGLTexture3D] = pvr::lib::getLibFunctionChecked<void*>(lib, "clCreateFromGLTexture3D");
		CLFunctionTable[CLFunctions::GetGLContextInfoKHR] = pvr::lib::getLibFunctionChecked<void*>(lib, "clGetGLContextInfoKHR");
	}
	return CLFunctionTable[func];
}
} // namespace internals

bool testFunctionExists(CLFunctions::Enum function) { return internals::getClFunction(function) != 0; }

} // namespace cl
#ifndef DYNAMICCL_NO_NAMESPACE
namespace cl {
#endif
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetPlatformIDs)(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetPlatformIDs)(cl_uint num_entries, cl_platform_id * platforms, cl_uint * num_platforms);
	static PFNclGetPlatformIDs _clGetPlatformIDs = (PFNclGetPlatformIDs)cl::internals::getClFunction(cl::CLFunctions::GetPlatformIDs);
	return _clGetPlatformIDs(num_entries, platforms, num_platforms);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetPlatformInfo)(
	cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetPlatformInfo)(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetPlatformInfo _clGetPlatformInfo = (PFNclGetPlatformInfo)cl::internals::getClFunction(cl::CLFunctions::GetPlatformInfo);
	return _clGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetDeviceIDs)(
	cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id* devices, cl_uint* num_devices) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetDeviceIDs)(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id * devices, cl_uint * num_devices);
	static PFNclGetDeviceIDs _clGetDeviceIDs = (PFNclGetDeviceIDs)cl::internals::getClFunction(cl::CLFunctions::GetDeviceIDs);
	return _clGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetDeviceInfo)(
	cl_device_id device, cl_device_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetDeviceInfo)(cl_device_id device, cl_device_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetDeviceInfo _clGetDeviceInfo = (PFNclGetDeviceInfo)cl::internals::getClFunction(cl::CLFunctions::GetDeviceInfo);
	return _clGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateSubDevices)(
	cl_device_id in_device, const cl_device_partition_property* properties, cl_uint num_devices, cl_device_id* out_devices, cl_uint* num_devices_ret) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclCreateSubDevices)(
		cl_device_id in_device, const cl_device_partition_property* properties, cl_uint num_devices, cl_device_id* out_devices, cl_uint* num_devices_ret);
	static PFNclCreateSubDevices _clCreateSubDevices = (PFNclCreateSubDevices)cl::internals::getClFunction(cl::CLFunctions::CreateSubDevices);
	return _clCreateSubDevices(in_device, properties, num_devices, out_devices, num_devices_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(RetainDevice)(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclRetainDevice)(cl_device_id device);
	static PFNclRetainDevice _clRetainDevice = (PFNclRetainDevice)cl::internals::getClFunction(cl::CLFunctions::RetainDevice);
	return _clRetainDevice(device);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(ReleaseDevice)(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclReleaseDevice)(cl_device_id device);
	static PFNclReleaseDevice _clReleaseDevice = (PFNclReleaseDevice)cl::internals::getClFunction(cl::CLFunctions::ReleaseDevice);
	return _clReleaseDevice(device);
}
#ifdef CL_API_SUFFIX__VERSION_2_1
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(SetDefaultDeviceCommandQueue)(cl_context context, cl_device_id device, cl_command_queue command_queue) CL_API_SUFFIX__VERSION_2_1
{
	typedef cl_int(CL_API_CALL * PFNclSetDefaultDeviceCommandQueue)(cl_context context, cl_device_id device, cl_command_queue command_queue);
	static PFNclSetDefaultDeviceCommandQueue _clSetDefaultDeviceCommandQueue =
		(PFNclSetDefaultDeviceCommandQueue)cl::internals::getClFunction(cl::CLFunctions::SetDefaultDeviceCommandQueue);
	return _clSetDefaultDeviceCommandQueue(context, device, command_queue);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetDeviceAndHostTimer)(cl_device_id device, cl_ulong* device_timestamp, cl_ulong* host_timestamp) CL_API_SUFFIX__VERSION_2_1
{
	typedef cl_int(CL_API_CALL * PFNclGetDeviceAndHostTimer)(cl_device_id device, cl_ulong * device_timestamp, cl_ulong * host_timestamp);
	static PFNclGetDeviceAndHostTimer _clGetDeviceAndHostTimer = (PFNclGetDeviceAndHostTimer)cl::internals::getClFunction(cl::CLFunctions::GetDeviceAndHostTimer);
	return _clGetDeviceAndHostTimer(device, device_timestamp, host_timestamp);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetHostTimer)(cl_device_id device, cl_ulong* host_timestamp) CL_API_SUFFIX__VERSION_2_1
{
	typedef cl_int(CL_API_CALL * PFNclGetHostTimer)(cl_device_id device, cl_ulong * host_timestamp);
	static PFNclGetHostTimer _clGetHostTimer = (PFNclGetHostTimer)cl::internals::getClFunction(cl::CLFunctions::GetHostTimer);
	return _clGetHostTimer(device, host_timestamp);
}
#endif
inline cl_context CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateContext)(const cl_context_properties* properties, cl_uint num_devices, const cl_device_id* devices,
	void(CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*), void* user_data, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_context(CL_API_CALL * PFNclCreateContext)(const cl_context_properties* properties, cl_uint num_devices, const cl_device_id* devices,
		void(CL_CALLBACK*)(const char*, const void*, size_t, void*), void* user_data, cl_int* errcode_ret);
	static PFNclCreateContext _clCreateContext = (PFNclCreateContext)cl::internals::getClFunction(cl::CLFunctions::CreateContext);
	return _clCreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
}
inline cl_context CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateContextFromType)(const cl_context_properties* properties, cl_device_type device_type,
	void(CL_CALLBACK* pfn_notify)(const char*, const void*, size_t, void*), void* user_data, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_context(CL_API_CALL * PFNclCreateContextFromType)(
		const cl_context_properties* properties, cl_device_type device_type, void(CL_CALLBACK*)(const char*, const void*, size_t, void*), void* user_data, cl_int* errcode_ret);
	static PFNclCreateContextFromType _clCreateContextFromType = (PFNclCreateContextFromType)cl::internals::getClFunction(cl::CLFunctions::CreateContextFromType);
	return _clCreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(RetainContext)(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclRetainContext)(cl_context context);
	static PFNclRetainContext _clRetainContext = (PFNclRetainContext)cl::internals::getClFunction(cl::CLFunctions::RetainContext);
	return _clRetainContext(context);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(ReleaseContext)(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclReleaseContext)(cl_context context);
	static PFNclReleaseContext _clReleaseContext = (PFNclReleaseContext)cl::internals::getClFunction(cl::CLFunctions::ReleaseContext);
	return _clReleaseContext(context);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetContextInfo)(
	cl_context context, cl_context_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetContextInfo)(cl_context context, cl_context_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetContextInfo _clGetContextInfo = (PFNclGetContextInfo)cl::internals::getClFunction(cl::CLFunctions::GetContextInfo);
	return _clGetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);
}
#ifdef CL_API_SUFFIX__VERSION_2_0
inline cl_command_queue CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateCommandQueueWithProperties)(
	cl_context context, cl_device_id device, const cl_queue_properties* properties, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_command_queue(CL_API_CALL * PFNclCreateCommandQueueWithProperties)(cl_context context, cl_device_id device, const cl_queue_properties* properties, cl_int* errcode_ret);
	static PFNclCreateCommandQueueWithProperties _clCreateCommandQueueWithProperties =
		(PFNclCreateCommandQueueWithProperties)cl::internals::getClFunction(cl::CLFunctions::CreateCommandQueueWithProperties);
	return _clCreateCommandQueueWithProperties(context, device, properties, errcode_ret);
}
#endif
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(RetainCommandQueue)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclRetainCommandQueue)(cl_command_queue command_queue);
	static PFNclRetainCommandQueue _clRetainCommandQueue = (PFNclRetainCommandQueue)cl::internals::getClFunction(cl::CLFunctions::RetainCommandQueue);
	return _clRetainCommandQueue(command_queue);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(ReleaseCommandQueue)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclReleaseCommandQueue)(cl_command_queue command_queue);
	static PFNclReleaseCommandQueue _clReleaseCommandQueue = (PFNclReleaseCommandQueue)cl::internals::getClFunction(cl::CLFunctions::ReleaseCommandQueue);
	return _clReleaseCommandQueue(command_queue);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetCommandQueueInfo)(
	cl_command_queue command_queue, cl_command_queue_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetCommandQueueInfo)(
		cl_command_queue command_queue, cl_command_queue_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetCommandQueueInfo _clGetCommandQueueInfo = (PFNclGetCommandQueueInfo)cl::internals::getClFunction(cl::CLFunctions::GetCommandQueueInfo);
	return _clGetCommandQueueInfo(command_queue, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_mem CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateBuffer)(cl_context context, cl_mem_flags flags, size_t size, void* host_ptr, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_mem(CL_API_CALL * PFNclCreateBuffer)(cl_context context, cl_mem_flags flags, size_t size, void* host_ptr, cl_int* errcode_ret);
	static PFNclCreateBuffer _clCreateBuffer = (PFNclCreateBuffer)cl::internals::getClFunction(cl::CLFunctions::CreateBuffer);
	return _clCreateBuffer(context, flags, size, host_ptr, errcode_ret);
}
inline cl_mem CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateSubBuffer)(
	cl_mem buffer, cl_mem_flags flags, cl_buffer_create_type buffer_create_type, const void* buffer_create_info, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
	typedef cl_mem(CL_API_CALL * PFNclCreateSubBuffer)(cl_mem buffer, cl_mem_flags flags, cl_buffer_create_type buffer_create_type, const void* buffer_create_info, cl_int* errcode_ret);
	static PFNclCreateSubBuffer _clCreateSubBuffer = (PFNclCreateSubBuffer)cl::internals::getClFunction(cl::CLFunctions::CreateSubBuffer);
	return _clCreateSubBuffer(buffer, flags, buffer_create_type, buffer_create_info, errcode_ret);
}
inline cl_mem CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateImage)(
	cl_context context, cl_mem_flags flags, const cl_image_format* image_format, const cl_image_desc* image_desc, void* host_ptr, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_mem(CL_API_CALL * PFNclCreateImage)(
		cl_context context, cl_mem_flags flags, const cl_image_format* image_format, const cl_image_desc* image_desc, void* host_ptr, cl_int* errcode_ret);
	static PFNclCreateImage _clCreateImage = (PFNclCreateImage)cl::internals::getClFunction(cl::CLFunctions::CreateImage);
	return _clCreateImage(context, flags, image_format, image_desc, host_ptr, errcode_ret);
}
#ifdef CL_API_SUFFIX__VERSION_2_0
inline cl_mem CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreatePipe)(cl_context context, cl_mem_flags flags, cl_uint pipe_packet_size, cl_uint pipe_max_packets,
	const cl_pipe_properties* properties, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_mem(CL_API_CALL * PFNclCreatePipe)(
		cl_context context, cl_mem_flags flags, cl_uint pipe_packet_size, cl_uint pipe_max_packets, const cl_pipe_properties* properties, cl_int* errcode_ret);
	static PFNclCreatePipe _clCreatePipe = (PFNclCreatePipe)cl::internals::getClFunction(cl::CLFunctions::CreatePipe);
	return _clCreatePipe(context, flags, pipe_packet_size, pipe_max_packets, properties, errcode_ret);
}
#endif
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(RetainMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclRetainMemObject)(cl_mem memobj);
	static PFNclRetainMemObject _clRetainMemObject = (PFNclRetainMemObject)cl::internals::getClFunction(cl::CLFunctions::RetainMemObject);
	return _clRetainMemObject(memobj);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(ReleaseMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclReleaseMemObject)(cl_mem memobj);
	static PFNclReleaseMemObject _clReleaseMemObject = (PFNclReleaseMemObject)cl::internals::getClFunction(cl::CLFunctions::ReleaseMemObject);
	return _clReleaseMemObject(memobj);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetSupportedImageFormats)(cl_context context, cl_mem_flags flags, cl_mem_object_type image_type, cl_uint num_entries,
	cl_image_format* image_formats, cl_uint* num_image_formats) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetSupportedImageFormats)(
		cl_context context, cl_mem_flags flags, cl_mem_object_type image_type, cl_uint num_entries, cl_image_format * image_formats, cl_uint * num_image_formats);
	static PFNclGetSupportedImageFormats _clGetSupportedImageFormats = (PFNclGetSupportedImageFormats)cl::internals::getClFunction(cl::CLFunctions::GetSupportedImageFormats);
	return _clGetSupportedImageFormats(context, flags, image_type, num_entries, image_formats, num_image_formats);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetMemObjectInfo)(
	cl_mem memobj, cl_mem_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetMemObjectInfo)(cl_mem memobj, cl_mem_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetMemObjectInfo _clGetMemObjectInfo = (PFNclGetMemObjectInfo)cl::internals::getClFunction(cl::CLFunctions::GetMemObjectInfo);
	return _clGetMemObjectInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetImageInfo)(
	cl_mem image, cl_image_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetImageInfo)(cl_mem image, cl_image_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetImageInfo _clGetImageInfo = (PFNclGetImageInfo)cl::internals::getClFunction(cl::CLFunctions::GetImageInfo);
	return _clGetImageInfo(image, param_name, param_value_size, param_value, param_value_size_ret);
}
#ifdef CL_API_SUFFIX__VERSION_2_0
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetPipeInfo)(
	cl_mem pipe, cl_pipe_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_int(CL_API_CALL * PFNclGetPipeInfo)(cl_mem pipe, cl_pipe_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetPipeInfo _clGetPipeInfo = (PFNclGetPipeInfo)cl::internals::getClFunction(cl::CLFunctions::GetPipeInfo);
	return _clGetPipeInfo(pipe, param_name, param_value_size, param_value, param_value_size_ret);
}
inline void* CL_API_CALL DEFINE_CL_FUNCTION_NAME(SVMAlloc)(cl_context context, cl_svm_mem_flags flags, size_t size, cl_uint alignment) CL_API_SUFFIX__VERSION_2_0
{
	typedef void*(CL_API_CALL * PFNclSVMAlloc)(cl_context context, cl_svm_mem_flags flags, size_t size, cl_uint alignment);
	PFNclSVMAlloc _clSVMAlloc = (PFNclSVMAlloc)cl::internals::getClFunction(cl::CLFunctions::SVMAlloc);
	return _clSVMAlloc(context, flags, size, alignment);
}
inline void CL_API_CALL DEFINE_CL_FUNCTION_NAME(SVMFree)(cl_context context, void* svm_pointer) CL_API_SUFFIX__VERSION_2_0
{
	typedef void(CL_API_CALL * PFNclSVMFree)(cl_context context, void* svm_pointer);
	PFNclSVMFree _clSVMFree = (PFNclSVMFree)cl::internals::getClFunction(cl::CLFunctions::SVMFree);
	return _clSVMFree(context, svm_pointer);
}
inline cl_sampler CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateSamplerWithProperties)(
	cl_context context, const cl_sampler_properties* sampler_properties, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_sampler(CL_API_CALL * PFNclCreateSamplerWithProperties)(cl_context context, const cl_sampler_properties* normalized_coords, cl_int* errcode_ret);
	PFNclCreateSamplerWithProperties _clCreateSamplerWithProperties = (PFNclCreateSamplerWithProperties)cl::internals::getClFunction(cl::CLFunctions::CreateSamplerWithProperties);
	return _clCreateSamplerWithProperties(context, sampler_properties, errcode_ret);
}
#endif
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(SetMemObjectDestructorCallback)(
	cl_mem memobj, void(CL_CALLBACK* pfn_notify)(cl_mem memobj, void* user_data), void* user_data) CL_API_SUFFIX__VERSION_1_1
{
	typedef cl_int(CL_API_CALL * PFNclSetMemObjectDestructorCallback)(cl_mem memobj, void(CL_CALLBACK * pfn_notify)(cl_mem memobj, void* user_data), void* user_data);
	static PFNclSetMemObjectDestructorCallback _clSetMemObjectDestructorCallback =
		(PFNclSetMemObjectDestructorCallback)cl::internals::getClFunction(cl::CLFunctions::SetMemObjectDestructorCallback);
	return _clSetMemObjectDestructorCallback(memobj, pfn_notify, user_data);
}

inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(RetainSampler)(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclRetainSampler)(cl_sampler sampler);
	static PFNclRetainSampler _clRetainSampler = (PFNclRetainSampler)cl::internals::getClFunction(cl::CLFunctions::RetainSampler);
	return _clRetainSampler(sampler);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(ReleaseSampler)(cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclReleaseSampler)(cl_sampler sampler);
	static PFNclReleaseSampler _clReleaseSampler = (PFNclReleaseSampler)cl::internals::getClFunction(cl::CLFunctions::ReleaseSampler);
	return _clReleaseSampler(sampler);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetSamplerInfo)(
	cl_sampler sampler, cl_sampler_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetSamplerInfo)(cl_sampler sampler, cl_sampler_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetSamplerInfo _clGetSamplerInfo = (PFNclGetSamplerInfo)cl::internals::getClFunction(cl::CLFunctions::GetSamplerInfo);
	return _clGetSamplerInfo(sampler, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_program CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateProgramWithSource)(
	cl_context context, cl_uint count, const char** strings, const size_t* lengths, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_program(CL_API_CALL * PFNclCreateProgramWithSource)(cl_context context, cl_uint count, const char** strings, const size_t* lengths, cl_int* errcode_ret);
	static PFNclCreateProgramWithSource _clCreateProgramWithSource = (PFNclCreateProgramWithSource)cl::internals::getClFunction(cl::CLFunctions::CreateProgramWithSource);
	return _clCreateProgramWithSource(context, count, strings, lengths, errcode_ret);
}
inline cl_program CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateProgramWithBinary)(cl_context context, cl_uint num_devices, const cl_device_id* device_list, const size_t* lengths,
	const unsigned char** binaries, cl_int* binary_status, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_program(CL_API_CALL * PFNclCreateProgramWithBinary)(cl_context context, cl_uint num_devices, const cl_device_id* device_list, const size_t* lengths,
		const unsigned char** binaries, cl_int* binary_status, cl_int* errcode_ret);
	static PFNclCreateProgramWithBinary _clCreateProgramWithBinary = (PFNclCreateProgramWithBinary)cl::internals::getClFunction(cl::CLFunctions::CreateProgramWithBinary);
	return _clCreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
}
inline cl_program CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateProgramWithBuiltInKernels)(
	cl_context context, cl_uint num_devices, const cl_device_id* device_list, const char* kernel_names, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_program(CL_API_CALL * PFNclCreateProgramWithBuiltInKernels)(
		cl_context context, cl_uint num_devices, const cl_device_id* device_list, const char* kernel_names, cl_int* errcode_ret);
	static PFNclCreateProgramWithBuiltInKernels _clCreateProgramWithBuiltInKernels =
		(PFNclCreateProgramWithBuiltInKernels)cl::internals::getClFunction(cl::CLFunctions::CreateProgramWithBuiltInKernels);
	return _clCreateProgramWithBuiltInKernels(context, num_devices, device_list, kernel_names, errcode_ret);
}
#ifdef CL_API_SUFFIX__VERSION_2_1
inline cl_program CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateProgramWithIL)(cl_context context, const void* il, size_t length, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_2_1
{
	typedef cl_program(CL_API_CALL * PFNclCreateProgramWithIL)(cl_context context, const void* il, size_t length, cl_int* errcode_ret);
	static PFNclCreateProgramWithIL _clCreateProgramWithIL = (PFNclCreateProgramWithIL)cl::internals::getClFunction(cl::CLFunctions::CreateProgramWithIL);
	return _clCreateProgramWithIL(context, il, length, errcode_ret);
}
#endif
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(RetainProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclRetainProgram)(cl_program program);
	static PFNclRetainProgram _clRetainProgram = (PFNclRetainProgram)cl::internals::getClFunction(cl::CLFunctions::RetainProgram);
	return _clRetainProgram(program);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(ReleaseProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclReleaseProgram)(cl_program program);
	static PFNclReleaseProgram _clReleaseProgram = (PFNclReleaseProgram)cl::internals::getClFunction(cl::CLFunctions::ReleaseProgram);
	return _clReleaseProgram(program);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(BuildProgram)(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* options,
	void(CL_CALLBACK* pfn_notify)(cl_program program, void* user_data), void* user_data) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclBuildProgram)(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* options,
		void(CL_CALLBACK * pfn_notify)(cl_program program, void* user_data), void* user_data);
	static PFNclBuildProgram _clBuildProgram = (PFNclBuildProgram)cl::internals::getClFunction(cl::CLFunctions::BuildProgram);
	return _clBuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(CompileProgram)(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* options,
	cl_uint num_input_headers, const cl_program* input_headers, const char** header_include_names, void(CL_CALLBACK* pfn_notify)(cl_program program, void* user_data),
	void* user_data) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclCompileProgram)(cl_program program, cl_uint num_devices, const cl_device_id* device_list, const char* options, cl_uint num_input_headers,
		const cl_program* input_headers, const char** header_include_names, void(CL_CALLBACK * pfn_notify)(cl_program program, void* user_data), void* user_data);
	static PFNclCompileProgram _clCompileProgram = (PFNclCompileProgram)cl::internals::getClFunction(cl::CLFunctions::CompileProgram);
	return _clCompileProgram(program, num_devices, device_list, options, num_input_headers, input_headers, header_include_names, pfn_notify, user_data);
}
inline cl_program CL_API_CALL DEFINE_CL_FUNCTION_NAME(LinkProgram)(cl_context context, cl_uint num_devices, const cl_device_id* device_list, const char* options,
	cl_uint num_input_programs, const cl_program* input_programs, void(CL_CALLBACK* pfn_notify)(cl_program program, void* user_data), void* user_data,
	cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_program(CL_API_CALL * PFNclLinkProgram)(cl_context context, cl_uint num_devices, const cl_device_id* device_list, const char* options, cl_uint num_input_programs,
		const cl_program* input_programs, void(CL_CALLBACK * pfn_notify)(cl_program program, void* user_data), void* user_data, cl_int* errcode_ret);
	static PFNclLinkProgram _clLinkProgram = (PFNclLinkProgram)cl::internals::getClFunction(cl::CLFunctions::LinkProgram);
	return _clLinkProgram(context, num_devices, device_list, options, num_input_programs, input_programs, pfn_notify, user_data, errcode_ret);
}
#ifdef CL_API_SUFFIX__VERSION_2_2
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(SetProgramReleaseCallback)(
	cl_program program, void(CL_CALLBACK* pfn_notify)(cl_program program, void* user_data), void* user_data) CL_API_SUFFIX__VERSION_2_2
{
	typedef cl_int(CL_API_CALL * PFNclSetProgramReleaseCallback)(cl_program program, void(CL_CALLBACK * pfn_notify)(cl_program program, void* user_data), void* user_data);
	static PFNclSetProgramReleaseCallback _clSetProgramReleaseCallback = (PFNclSetProgramReleaseCallback)cl::internals::getClFunction(cl::CLFunctions::SetProgramReleaseCallback);
	return _clSetProgramReleaseCallback(program, pfn_notify, user_data);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(SetProgramSpecializationConstant)(cl_program program, cl_uint spec_id, size_t spec_size, const void* spec_value) CL_API_SUFFIX__VERSION_2_2
{
	typedef cl_int(CL_API_CALL * PFNclSetProgramSpecializationConstant)(cl_program program, cl_uint spec_id, size_t spec_size, const void* spec_value);
	static PFNclSetProgramSpecializationConstant _clSetProgramSpecializationConstant =
		(PFNclSetProgramSpecializationConstant)cl::internals::getClFunction(cl::CLFunctions::SetProgramSpecializationConstant);
	return _clSetProgramSpecializationConstant(program, spec_id, spec_size, spec_value);
}
#endif
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(UnloadPlatformCompiler)(cl_platform_id platform) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclUnloadPlatformCompiler)(cl_platform_id platform);
	static PFNclUnloadPlatformCompiler _clUnloadPlatformCompiler = (PFNclUnloadPlatformCompiler)cl::internals::getClFunction(cl::CLFunctions::UnloadPlatformCompiler);
	return _clUnloadPlatformCompiler(platform);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetProgramInfo)(
	cl_program program, cl_program_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetProgramInfo)(cl_program program, cl_program_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetProgramInfo _clGetProgramInfo = (PFNclGetProgramInfo)cl::internals::getClFunction(cl::CLFunctions::GetProgramInfo);
	return _clGetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetProgramBuildInfo)(
	cl_program program, cl_device_id device, cl_program_build_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetProgramBuildInfo)(
		cl_program program, cl_device_id device, cl_program_build_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetProgramBuildInfo _clGetProgramBuildInfo = (PFNclGetProgramBuildInfo)cl::internals::getClFunction(cl::CLFunctions::GetProgramBuildInfo);
	return _clGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_kernel CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateKernel)(cl_program program, const char* kernel_name, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_kernel(CL_API_CALL * PFNclCreateKernel)(cl_program program, const char* kernel_name, cl_int* errcode_ret);
	static PFNclCreateKernel _clCreateKernel = (PFNclCreateKernel)cl::internals::getClFunction(cl::CLFunctions::CreateKernel);
	return _clCreateKernel(program, kernel_name, errcode_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateKernelsInProgram)(cl_program program, cl_uint num_kernels, cl_kernel* kernels, cl_uint* num_kernels_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclCreateKernelsInProgram)(cl_program program, cl_uint num_kernels, cl_kernel * kernels, cl_uint * num_kernels_ret);
	static PFNclCreateKernelsInProgram _clCreateKernelsInProgram = (PFNclCreateKernelsInProgram)cl::internals::getClFunction(cl::CLFunctions::CreateKernelsInProgram);
	return _clCreateKernelsInProgram(program, num_kernels, kernels, num_kernels_ret);
}
#ifdef CL_API_SUFFIX__VERSION_2_1
inline cl_kernel CL_API_CALL DEFINE_CL_FUNCTION_NAME(CloneKernel)(cl_kernel source_kernel, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_2_1
{
	typedef cl_kernel(CL_API_CALL * PFNclCloneKernel)(cl_kernel source_kernel, cl_int * errcode_ret);
	static PFNclCloneKernel _clCloneKernel = (PFNclCloneKernel)cl::internals::getClFunction(cl::CLFunctions::CloneKernel);
	return _clCloneKernel(source_kernel, errcode_ret);
}
#endif
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(RetainKernel)(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclRetainKernel)(cl_kernel kernel);
	static PFNclRetainKernel _clRetainKernel = (PFNclRetainKernel)cl::internals::getClFunction(cl::CLFunctions::RetainKernel);
	return _clRetainKernel(kernel);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(ReleaseKernel)(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclReleaseKernel)(cl_kernel kernel);
	static PFNclReleaseKernel _clReleaseKernel = (PFNclReleaseKernel)cl::internals::getClFunction(cl::CLFunctions::ReleaseKernel);
	return _clReleaseKernel(kernel);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(SetKernelArg)(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclSetKernelArg)(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value);
	static PFNclSetKernelArg _clSetKernelArg = (PFNclSetKernelArg)cl::internals::getClFunction(cl::CLFunctions::SetKernelArg);
	return _clSetKernelArg(kernel, arg_index, arg_size, arg_value);
}
#ifdef CL_API_SUFFIX__VERSION_2_0
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(SetKernelArgSVMPointer)(cl_kernel kernel, cl_uint arg_index, const void* arg_value) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_int(CL_API_CALL * PFNclSetKernelArgSVMPointer)(cl_kernel kernel, cl_uint arg_index, const void* arg_value);
	static PFNclSetKernelArgSVMPointer _clSetKernelArgSVMPointer = (PFNclSetKernelArgSVMPointer)cl::internals::getClFunction(cl::CLFunctions::SetKernelArgSVMPointer);
	return _clSetKernelArgSVMPointer(kernel, arg_index, arg_value);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(SetKernelExecInfo)(cl_kernel kernel, cl_kernel_exec_info param_name, size_t param_value_size, const void* param_value) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_int(CL_API_CALL * PFNclSetKernelExecInfo)(cl_kernel kernel, cl_kernel_exec_info param_name, size_t param_value_size, const void* param_value);
	static PFNclSetKernelExecInfo _clSetKernelExecInfo = (PFNclSetKernelExecInfo)cl::internals::getClFunction(cl::CLFunctions::SetKernelExecInfo);
	return _clSetKernelExecInfo(kernel, param_name, param_value_size, param_value);
}
#endif
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetKernelInfo)(
	cl_kernel kernel, cl_kernel_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetKernelInfo)(cl_kernel kernel, cl_kernel_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetKernelInfo _clGetKernelInfo = (PFNclGetKernelInfo)cl::internals::getClFunction(cl::CLFunctions::GetKernelInfo);
	return _clGetKernelInfo(kernel, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetKernelArgInfo)(
	cl_kernel kernel, cl_uint arg_indx, cl_kernel_arg_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclGetKernelArgInfo)(
		cl_kernel kernel, cl_uint arg_indx, cl_kernel_arg_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetKernelArgInfo _clGetKernelArgInfo = (PFNclGetKernelArgInfo)cl::internals::getClFunction(cl::CLFunctions::GetKernelArgInfo);
	return _clGetKernelArgInfo(kernel, arg_indx, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetKernelWorkGroupInfo)(cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name, size_t param_value_size,
	void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetKernelWorkGroupInfo)(
		cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetKernelWorkGroupInfo _clGetKernelWorkGroupInfo = (PFNclGetKernelWorkGroupInfo)cl::internals::getClFunction(cl::CLFunctions::GetKernelWorkGroupInfo);
	return _clGetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
}
#ifdef CL_API_SUFFIX__VERSION_2_1
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetKernelSubGroupInfo)(cl_kernel kernel, cl_device_id device, cl_kernel_sub_group_info param_name, size_t input_value_size,
	const void* input_value, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_2_1
{
	typedef cl_int(CL_API_CALL * PFNclGetKernelSubGroupInfo)(cl_kernel kernel, cl_device_id device, cl_kernel_sub_group_info param_name, size_t input_value_size,
		const void* input_value, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetKernelSubGroupInfo _clGetKernelSubGroupInfo = (PFNclGetKernelSubGroupInfo)cl::internals::getClFunction(cl::CLFunctions::GetKernelSubGroupInfo);
	return _clGetKernelSubGroupInfo(kernel, device, param_name, input_value_size, input_value, param_value_size, param_value, param_value_size_ret);
}
#endif
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(WaitForEvents)(cl_uint num_events, const cl_event* event_list) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclWaitForEvents)(cl_uint num_events, const cl_event* event_list);
	static PFNclWaitForEvents _clWaitForEvents = (PFNclWaitForEvents)cl::internals::getClFunction(cl::CLFunctions::WaitForEvents);
	return _clWaitForEvents(num_events, event_list);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetEventInfo)(
	cl_event event, cl_event_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetEventInfo)(cl_event event, cl_event_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetEventInfo _clGetEventInfo = (PFNclGetEventInfo)cl::internals::getClFunction(cl::CLFunctions::GetEventInfo);
	return _clGetEventInfo(event, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_event CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateUserEvent)(cl_context context, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_1
{
	typedef cl_event(CL_API_CALL * PFNclCreateUserEvent)(cl_context context, cl_int * errcode_ret);
	static PFNclCreateUserEvent _clCreateUserEvent = (PFNclCreateUserEvent)cl::internals::getClFunction(cl::CLFunctions::CreateUserEvent);
	return _clCreateUserEvent(context, errcode_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(RetainEvent)(cl_event event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclRetainEvent)(cl_event event);
	static PFNclRetainEvent _clRetainEvent = (PFNclRetainEvent)cl::internals::getClFunction(cl::CLFunctions::RetainEvent);
	return _clRetainEvent(event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(ReleaseEvent)(cl_event event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclReleaseEvent)(cl_event event);
	static PFNclReleaseEvent _clReleaseEvent = (PFNclReleaseEvent)cl::internals::getClFunction(cl::CLFunctions::ReleaseEvent);
	return _clReleaseEvent(event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(SetUserEventStatus)(cl_event event, cl_int execution_status) CL_API_SUFFIX__VERSION_1_1
{
	typedef cl_int(CL_API_CALL * PFNclSetUserEventStatus)(cl_event event, cl_int execution_status);
	static PFNclSetUserEventStatus _clSetUserEventStatus = (PFNclSetUserEventStatus)cl::internals::getClFunction(cl::CLFunctions::SetUserEventStatus);
	return _clSetUserEventStatus(event, execution_status);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(SetEventCallback)(
	cl_event event, cl_int command_exec_callback_type, void(CL_CALLBACK* pfn_notify)(cl_event, cl_int, void*), void* user_data) CL_API_SUFFIX__VERSION_1_1
{
	typedef cl_int(CL_API_CALL * PFNclSetEventCallback)(cl_event event, cl_int command_exec_callback_type, void(CL_CALLBACK * pfn_notify)(cl_event, cl_int, void*), void* user_data);
	static PFNclSetEventCallback _clSetEventCallback = (PFNclSetEventCallback)cl::internals::getClFunction(cl::CLFunctions::SetEventCallback);
	return _clSetEventCallback(event, command_exec_callback_type, pfn_notify, user_data);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetEventProfilingInfo)(
	cl_event event, cl_profiling_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclGetEventProfilingInfo)(cl_event event, cl_profiling_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
	static PFNclGetEventProfilingInfo _clGetEventProfilingInfo = (PFNclGetEventProfilingInfo)cl::internals::getClFunction(cl::CLFunctions::GetEventProfilingInfo);
	return _clGetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(Flush)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclFlush)(cl_command_queue command_queue);
	static PFNclFlush _clFlush = (PFNclFlush)cl::internals::getClFunction(cl::CLFunctions::Flush);
	return _clFlush(command_queue);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(Finish)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclFinish)(cl_command_queue command_queue);
	static PFNclFinish _clFinish = (PFNclFinish)cl::internals::getClFunction(cl::CLFunctions::Finish);
	return _clFinish(command_queue);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueReadBuffer)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t size, void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueReadBuffer)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t size, void* ptr,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueReadBuffer _clEnqueueReadBuffer = (PFNclEnqueueReadBuffer)cl::internals::getClFunction(cl::CLFunctions::EnqueueReadBuffer);
	return _clEnqueueReadBuffer(command_queue, buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueReadBufferRect)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, const size_t* buffer_offset,
	const size_t* host_offset, const size_t* region, size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch, void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_1
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueReadBufferRect)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, const size_t* buffer_offset,
		const size_t* host_offset, const size_t* region, size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch, void* ptr,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueReadBufferRect _clEnqueueReadBufferRect = (PFNclEnqueueReadBufferRect)cl::internals::getClFunction(cl::CLFunctions::EnqueueReadBufferRect);
	return _clEnqueueReadBufferRect(command_queue, buffer, blocking_read, buffer_offset, host_offset, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch,
		host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueWriteBuffer)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t size,
	const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueWriteBuffer)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t size, const void* ptr,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueWriteBuffer _clEnqueueWriteBuffer = (PFNclEnqueueWriteBuffer)cl::internals::getClFunction(cl::CLFunctions::EnqueueWriteBuffer);
	return _clEnqueueWriteBuffer(command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueWriteBufferRect)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, const size_t* buffer_offset,
	const size_t* host_offset, const size_t* region, size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch, const void* ptr,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_1
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueWriteBufferRect)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, const size_t* buffer_offset,
		const size_t* host_offset, const size_t* region, size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch, const void* ptr,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueWriteBufferRect _clEnqueueWriteBufferRect = (PFNclEnqueueWriteBufferRect)cl::internals::getClFunction(cl::CLFunctions::EnqueueWriteBufferRect);
	return _clEnqueueWriteBufferRect(command_queue, buffer, blocking_write, buffer_offset, host_offset, region, buffer_row_pitch, buffer_slice_pitch, host_row_pitch,
		host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueFillBuffer)(cl_command_queue command_queue, cl_mem buffer, const void* pattern, size_t pattern_size, size_t offset,
	size_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueFillBuffer)(cl_command_queue command_queue, cl_mem buffer, const void* pattern, size_t pattern_size, size_t offset, size_t size,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueFillBuffer _clEnqueueFillBuffer = (PFNclEnqueueFillBuffer)cl::internals::getClFunction(cl::CLFunctions::EnqueueFillBuffer);
	return _clEnqueueFillBuffer(command_queue, buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueCopyBuffer)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, size_t src_offset, size_t dst_offset,
	size_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueCopyBuffer)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, size_t src_offset, size_t dst_offset, size_t size,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueCopyBuffer _clEnqueueCopyBuffer = (PFNclEnqueueCopyBuffer)cl::internals::getClFunction(cl::CLFunctions::EnqueueCopyBuffer);
	return _clEnqueueCopyBuffer(command_queue, src_buffer, dst_buffer, src_offset, dst_offset, size, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueCopyBufferRect)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, const size_t* src_origin,
	const size_t* dst_origin, const size_t* region, size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch, size_t dst_slice_pitch, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_1
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueCopyBufferRect)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, const size_t* src_origin,
		const size_t* dst_origin, const size_t* region, size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch, size_t dst_slice_pitch, cl_uint num_events_in_wait_list,
		const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueCopyBufferRect _clEnqueueCopyBufferRect = (PFNclEnqueueCopyBufferRect)cl::internals::getClFunction(cl::CLFunctions::EnqueueCopyBufferRect);
	return _clEnqueueCopyBufferRect(command_queue, src_buffer, dst_buffer, src_origin, dst_origin, region, src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch,
		num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueReadImage)(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t* origin, const size_t* region,
	size_t row_pitch, size_t slice_pitch, void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueReadImage)(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t* origin, const size_t* region,
		size_t row_pitch, size_t slice_pitch, void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueReadImage _clEnqueueReadImage = (PFNclEnqueueReadImage)cl::internals::getClFunction(cl::CLFunctions::EnqueueReadImage);
	return _clEnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueWriteImage)(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t* origin, const size_t* region,
	size_t input_row_pitch, size_t input_slice_pitch, const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueWriteImage)(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t* origin, const size_t* region,
		size_t input_row_pitch, size_t input_slice_pitch, const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueWriteImage _clEnqueueWriteImage = (PFNclEnqueueWriteImage)cl::internals::getClFunction(cl::CLFunctions::EnqueueWriteImage);
	return _clEnqueueWriteImage(command_queue, image, blocking_write, origin, region, input_row_pitch, input_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueFillImage)(cl_command_queue command_queue, cl_mem image, const void* fill_color, const size_t* origin,
	const size_t* region, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueFillImage)(cl_command_queue command_queue, cl_mem image, const void* fill_color, const size_t* origin, const size_t* region,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueFillImage _clEnqueueFillImage = (PFNclEnqueueFillImage)cl::internals::getClFunction(cl::CLFunctions::EnqueueFillImage);
	return _clEnqueueFillImage(command_queue, image, fill_color, origin, region, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueCopyImage)(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, const size_t* src_origin,
	const size_t* dst_origin, const size_t* region, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueCopyImage)(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, const size_t* src_origin, const size_t* dst_origin,
		const size_t* region, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueCopyImage _clEnqueueCopyImage = (PFNclEnqueueCopyImage)cl::internals::getClFunction(cl::CLFunctions::EnqueueCopyImage);
	return _clEnqueueCopyImage(command_queue, src_image, dst_image, src_origin, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueCopyImageToBuffer)(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer, const size_t* src_origin,
	const size_t* region, size_t dst_offset, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueCopyImageToBuffer)(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer, const size_t* src_origin, const size_t* region,
		size_t dst_offset, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueCopyImageToBuffer _clEnqueueCopyImageToBuffer = (PFNclEnqueueCopyImageToBuffer)cl::internals::getClFunction(cl::CLFunctions::EnqueueCopyImageToBuffer);
	return _clEnqueueCopyImageToBuffer(command_queue, src_image, dst_buffer, src_origin, region, dst_offset, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueCopyBufferToImage)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, size_t src_offset,
	const size_t* dst_origin, const size_t* region, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueCopyBufferToImage)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, size_t src_offset, const size_t* dst_origin,
		const size_t* region, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueCopyBufferToImage _clEnqueueCopyBufferToImage = (PFNclEnqueueCopyBufferToImage)cl::internals::getClFunction(cl::CLFunctions::EnqueueCopyBufferToImage);
	return _clEnqueueCopyBufferToImage(command_queue, src_buffer, dst_image, src_offset, dst_origin, region, num_events_in_wait_list, event_wait_list, event);
}
inline void* CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueMapBuffer)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset,
	size_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef void*(CL_API_CALL * PFNclEnqueueMapBuffer)(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset, size_t size,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, cl_int* errcode_ret);
	static PFNclEnqueueMapBuffer _clEnqueueMapBuffer = (PFNclEnqueueMapBuffer)cl::internals::getClFunction(cl::CLFunctions::EnqueueMapBuffer);
	return _clEnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, size, num_events_in_wait_list, event_wait_list, event, errcode_ret);
}
inline void* CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueMapImage)(cl_command_queue command_queue, cl_mem image, cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin,
	const size_t* region, size_t* image_row_pitch, size_t* image_slice_pitch, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event,
	cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef void*(CL_API_CALL * PFNclEnqueueMapImage)(cl_command_queue command_queue, cl_mem image, cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin,
		const size_t* region, size_t* image_row_pitch, size_t* image_slice_pitch, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event,
		cl_int* errcode_ret);
	static PFNclEnqueueMapImage _clEnqueueMapImage = (PFNclEnqueueMapImage)cl::internals::getClFunction(cl::CLFunctions::EnqueueMapImage);
	return _clEnqueueMapImage(
		command_queue, image, blocking_map, map_flags, origin, region, image_row_pitch, image_slice_pitch, num_events_in_wait_list, event_wait_list, event, errcode_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueUnmapMemObject)(
	cl_command_queue command_queue, cl_mem memobj, void* mapped_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueUnmapMemObject)(
		cl_command_queue command_queue, cl_mem memobj, void* mapped_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueUnmapMemObject _clEnqueueUnmapMemObject = (PFNclEnqueueUnmapMemObject)cl::internals::getClFunction(cl::CLFunctions::EnqueueUnmapMemObject);
	return _clEnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueMigrateMemObjects)(cl_command_queue command_queue, cl_uint num_mem_objects, const cl_mem* mem_objects,
	cl_mem_migration_flags flags, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueMigrateMemObjects)(cl_command_queue command_queue, cl_uint num_mem_objects, const cl_mem* mem_objects, cl_mem_migration_flags flags,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueMigrateMemObjects _clEnqueueMigrateMemObjects = (PFNclEnqueueMigrateMemObjects)cl::internals::getClFunction(cl::CLFunctions::EnqueueMigrateMemObjects);
	return _clEnqueueMigrateMemObjects(command_queue, num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueNDRangeKernel)(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t* global_work_offset,
	const size_t* global_work_size, const size_t* local_work_size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueNDRangeKernel)(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t* global_work_offset,
		const size_t* global_work_size, const size_t* local_work_size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueNDRangeKernel _clEnqueueNDRangeKernel = (PFNclEnqueueNDRangeKernel)cl::internals::getClFunction(cl::CLFunctions::EnqueueNDRangeKernel);
	return _clEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueNativeKernel)(cl_command_queue command_queue, void(CL_CALLBACK* user_func)(void*), void* args, size_t cb_args,
	cl_uint num_mem_objects, const cl_mem* mem_list, const void** args_mem_loc, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueNativeKernel)(cl_command_queue command_queue, void(CL_CALLBACK * user_func)(void*), void* args, size_t cb_args,
		cl_uint num_mem_objects, const cl_mem* mem_list, const void** args_mem_loc, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueNativeKernel _clEnqueueNativeKernel = (PFNclEnqueueNativeKernel)cl::internals::getClFunction(cl::CLFunctions::EnqueueNativeKernel);
	return _clEnqueueNativeKernel(command_queue, user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueMarkerWithWaitList)(
	cl_command_queue command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueMarkerWithWaitList)(cl_command_queue command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueMarkerWithWaitList _clEnqueueMarkerWithWaitList = (PFNclEnqueueMarkerWithWaitList)cl::internals::getClFunction(cl::CLFunctions::EnqueueMarkerWithWaitList);
	return _clEnqueueMarkerWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueBarrierWithWaitList)(
	cl_command_queue command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueBarrierWithWaitList)(cl_command_queue command_queue, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueBarrierWithWaitList _clEnqueueBarrierWithWaitList = (PFNclEnqueueBarrierWithWaitList)cl::internals::getClFunction(cl::CLFunctions::EnqueueBarrierWithWaitList);
	return _clEnqueueBarrierWithWaitList(command_queue, num_events_in_wait_list, event_wait_list, event);
}
#ifdef CL_API_SUFFIX__VERSION_2_0
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueSVMFree)(cl_command_queue command_queue, cl_uint num_svm_pointers, void* svm_pointers[],
	void(CL_CALLBACK* pfn_free_func)(cl_command_queue queue, cl_uint num_svm_pointers, void* svm_pointers[], void* user_data), void* user_data, cl_uint num_events_in_wait_list,
	const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueSVMFree)(cl_command_queue command_queue, cl_uint num_svm_pointers, void* svm_pointers[],
		void(CL_CALLBACK * pfn_free_func)(cl_command_queue queue, cl_uint num_svm_pointers, void* svm_pointers[], void* user_data), void* user_data,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueSVMFree _clEnqueueSVMFree = (PFNclEnqueueSVMFree)cl::internals::getClFunction(cl::CLFunctions::EnqueueSVMFree);
	return _clEnqueueSVMFree(command_queue, num_svm_pointers, svm_pointers, pfn_free_func, user_data, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueSVMMemcpy)(cl_command_queue command_queue, cl_bool blocking_copy, void* dst_ptr, const void* src_ptr, size_t size,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueSVMMemcpy)(cl_command_queue command_queue, cl_bool blocking_copy, void* dst_ptr, const void* src_ptr, size_t size,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueSVMMemcpy _clEnqueueSVMMemcpy = (PFNclEnqueueSVMMemcpy)cl::internals::getClFunction(cl::CLFunctions::EnqueueSVMMemcpy);
	return _clEnqueueSVMMemcpy(command_queue, blocking_copy, dst_ptr, src_ptr, size, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueSVMMemFill)(cl_command_queue command_queue, void* svm_ptr, const void* pattern, size_t pattern_size, size_t size,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueSVMMemFill)(cl_command_queue command_queue, void* svm_ptr, const void* pattern, size_t pattern_size, size_t size,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueSVMMemFill _clEnqueueSVMMemFill = (PFNclEnqueueSVMMemFill)cl::internals::getClFunction(cl::CLFunctions::EnqueueSVMMemFill);
	return _clEnqueueSVMMemFill(command_queue, svm_ptr, pattern, pattern_size, size, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueSVMMap)(cl_command_queue command_queue, cl_bool blocking_map, cl_map_flags flags, void* svm_ptr, size_t size,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueSVMMap)(cl_command_queue command_queue, cl_bool blocking_map, cl_map_flags flags, void* svm_ptr, size_t size,
		cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueSVMMap _clEnqueueSVMMap = (PFNclEnqueueSVMMap)cl::internals::getClFunction(cl::CLFunctions::EnqueueSVMMap);
	return _clEnqueueSVMMap(command_queue, blocking_map, flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueSVMUnmap)(
	cl_command_queue command_queue, void* svm_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_2_0
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueSVMUnmap)(cl_command_queue command_queue, void* svm_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueSVMUnmap _clEnqueueSVMUnmap = (PFNclEnqueueSVMUnmap)cl::internals::getClFunction(cl::CLFunctions::EnqueueSVMUnmap);
	return _clEnqueueSVMUnmap(command_queue, svm_ptr, num_events_in_wait_list, event_wait_list, event);
}
#endif
#ifdef CL_API_SUFFIX__VERSION_2_1
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueSVMMigrateMem)(cl_command_queue command_queue, cl_uint num_svm_pointers, const void** svm_pointers, const size_t* sizes,
	cl_mem_migration_flags flags, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_2_1
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueSVMMigrateMem)(cl_command_queue command_queue, cl_uint num_svm_pointers, const void** svm_pointers, const size_t* sizes,
		cl_mem_migration_flags flags, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueSVMMigrateMem _clEnqueueSVMMigrateMem = (PFNclEnqueueSVMMigrateMem)cl::internals::getClFunction(cl::CLFunctions::EnqueueSVMMigrateMem);
	return _clEnqueueSVMMigrateMem(command_queue, num_svm_pointers, svm_pointers, sizes, flags, num_events_in_wait_list, event_wait_list, event);
}
#endif
inline void* CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetExtensionFunctionAddressForPlatform)(cl_platform_id platform, const char* func_name) CL_API_SUFFIX__VERSION_1_2
{
	typedef void*(CL_API_CALL * PFNclGetExtensionFunctionAddressForPlatform)(cl_platform_id platform, const char* func_name);
	static PFNclGetExtensionFunctionAddressForPlatform _clGetExtensionFunctionAddressForPlatform =
		(PFNclGetExtensionFunctionAddressForPlatform)cl::internals::getClFunction(cl::CLFunctions::GetExtensionFunctionAddressForPlatform);
	return _clGetExtensionFunctionAddressForPlatform(platform, func_name);
}
CL_EXT_PREFIX__VERSION_1_1_DEPRECATED inline cl_mem CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateImage2D)(cl_context context, cl_mem_flags flags,
	const cl_image_format* image_format, size_t image_width, size_t image_height, size_t image_row_pitch, void* host_ptr, cl_int* errcode_ret)
{
	typedef cl_mem(CL_API_CALL * PFNclCreateImage2D)(cl_context context, cl_mem_flags flags, const cl_image_format* image_format, size_t image_width, size_t image_height,
		size_t image_row_pitch, void* host_ptr, cl_int* errcode_ret);
	static PFNclCreateImage2D _clCreateImage2D = (PFNclCreateImage2D)cl::internals::getClFunction(cl::CLFunctions::CreateImage2D);
	return _clCreateImage2D(context, flags, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret);
}
CL_EXT_PREFIX__VERSION_1_1_DEPRECATED inline cl_mem CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateImage3D)(cl_context context, cl_mem_flags flags,
	const cl_image_format* image_format, size_t image_width, size_t image_height, size_t image_depth, size_t image_row_pitch, size_t image_slice_pitch, void* host_ptr,
	cl_int* errcode_ret)
{
	typedef cl_mem(CL_API_CALL * PFNclCreateImage3D)(cl_context context, cl_mem_flags flags, const cl_image_format* image_format, size_t image_width, size_t image_height,
		size_t image_depth, size_t image_row_pitch, size_t image_slice_pitch, void* host_ptr, cl_int* errcode_ret);
	static PFNclCreateImage3D _clCreateImage3D = (PFNclCreateImage3D)cl::internals::getClFunction(cl::CLFunctions::CreateImage3D);
	return _clCreateImage3D(context, flags, image_format, image_width, image_height, image_depth, image_row_pitch, image_slice_pitch, host_ptr, errcode_ret);
}
CL_EXT_PREFIX__VERSION_1_1_DEPRECATED inline cl_int CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueMarker)(
	cl_command_queue command_queue, cl_event* event)
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueMarker)(cl_command_queue command_queue, cl_event * event);
	static PFNclEnqueueMarker _clEnqueueMarker = (PFNclEnqueueMarker)cl::internals::getClFunction(cl::CLFunctions::EnqueueMarker);
	return _clEnqueueMarker(command_queue, event);
}
CL_EXT_PREFIX__VERSION_1_1_DEPRECATED inline cl_int CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueWaitForEvents)(
	cl_command_queue command_queue, cl_uint num_events, const cl_event* event_list)
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueWaitForEvents)(cl_command_queue command_queue, cl_uint num_events, const cl_event* event_list);
	static PFNclEnqueueWaitForEvents _clEnqueueWaitForEvents = (PFNclEnqueueWaitForEvents)cl::internals::getClFunction(cl::CLFunctions::EnqueueWaitForEvents);
	return _clEnqueueWaitForEvents(command_queue, num_events, event_list);
}
CL_EXT_PREFIX__VERSION_1_1_DEPRECATED inline cl_int CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueBarrier)(cl_command_queue command_queue)
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueBarrier)(cl_command_queue command_queue);
	static PFNclEnqueueBarrier _clEnqueueBarrier = (PFNclEnqueueBarrier)cl::internals::getClFunction(cl::CLFunctions::EnqueueBarrier);
	return _clEnqueueBarrier(command_queue);
}
CL_EXT_PREFIX__VERSION_1_1_DEPRECATED inline cl_int CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(UnloadCompiler)(void)
{
	typedef cl_int(CL_API_CALL * PFNclUnloadCompiler)(void);
	static PFNclUnloadCompiler _clUnloadCompiler = (PFNclUnloadCompiler)cl::internals::getClFunction(cl::CLFunctions::UnloadCompiler);
	return _clUnloadCompiler();
}
CL_EXT_PREFIX__VERSION_1_1_DEPRECATED inline void* CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetExtensionFunctionAddress)(const char* func_name)
{
	typedef void*(CL_API_CALL * PFNclGetExtensionFunctionAddress)(const char* func_name);
	static PFNclGetExtensionFunctionAddress _clGetExtensionFunctionAddress =
		(PFNclGetExtensionFunctionAddress)cl::internals::getClFunction(cl::CLFunctions::GetExtensionFunctionAddress);
	return _clGetExtensionFunctionAddress(func_name);
}
#ifdef CL_EXT_SUFFIX__VERSION_1_2_DEPRECATED
CL_EXT_PREFIX__VERSION_1_2_DEPRECATED inline cl_command_queue CL_EXT_SUFFIX__VERSION_1_2_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateCommandQueue)(
	cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int* errcode_ret)
{
	typedef cl_command_queue(CL_API_CALL * PFNclCreateCommandQueue)(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int * errcode_ret);
	static PFNclCreateCommandQueue _clCreateCommandQueue = (PFNclCreateCommandQueue)cl::internals::getClFunction(cl::CLFunctions::CreateCommandQueue);
	return _clCreateCommandQueue(context, device, properties, errcode_ret);
}
CL_EXT_PREFIX__VERSION_1_2_DEPRECATED inline cl_sampler CL_EXT_SUFFIX__VERSION_1_2_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateSampler)(
	cl_context context, cl_bool normalized_coords, cl_addressing_mode addressing_mode, cl_filter_mode filter_mode, cl_int* errcode_ret)
{
	typedef cl_sampler(CL_API_CALL * PFNclCreateSampler)(
		cl_context context, cl_bool normalized_coords, cl_addressing_mode addressing_mode, cl_filter_mode filter_mode, cl_int * errcode_ret);
	static PFNclCreateSampler _clCreateSampler = (PFNclCreateSampler)cl::internals::getClFunction(cl::CLFunctions::CreateSampler);
	return _clCreateSampler(context, normalized_coords, addressing_mode, filter_mode, errcode_ret);
}
CL_EXT_PREFIX__VERSION_1_2_DEPRECATED inline cl_int CL_EXT_SUFFIX__VERSION_1_2_DEPRECATED CL_EXT_SUFFIX__VERSION_1_2_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueTask)(
	cl_command_queue command_queue, cl_kernel kernel, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
{
	typedef cl_int(CL_API_CALL * PFNclEnqueueTask)(cl_command_queue command_queue, cl_kernel kernel, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
	static PFNclEnqueueTask _clEnqueueTask = (PFNclEnqueueTask)cl::internals::getClFunction(cl::CLFunctions::EnqueueTask);
	return _clEnqueueTask(command_queue, kernel, num_events_in_wait_list, event_wait_list, event);
}
#endif
/* CL_GL */
inline cl_mem CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateFromGLBuffer)(cl_context context, cl_mem_flags flags, cl_GLuint bufobj, int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_mem(CL_API_CALL * PFN_clCreateFromGLBuffer)(cl_context, cl_mem_flags, cl_GLuint, int*);
	static PFN_clCreateFromGLBuffer _CreateFromGLBuffer = (PFN_clCreateFromGLBuffer)cl::internals::getClFunction(cl::CLFunctions::CreateFromGLBuffer);
	return _CreateFromGLBuffer(context, flags, bufobj, errcode_ret);
}
inline cl_mem CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateFromGLTexture)(
	cl_context context, cl_mem_flags flags, cl_GLenum target, cl_GLint miplevel, cl_GLuint texture, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
	typedef cl_mem(CL_API_CALL * PFN_clCreateFromGLTexture)(cl_context, cl_mem_flags, cl_GLenum, cl_GLint, cl_GLuint, cl_int*);
	static PFN_clCreateFromGLTexture _CreateFromGLTexture = (PFN_clCreateFromGLTexture)cl::internals::getClFunction(cl::CLFunctions::CreateFromGLTexture);
	return _CreateFromGLTexture(context, flags, target, miplevel, texture, errcode_ret);
}
inline cl_mem CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateFromGLRenderbuffer)(cl_context context, cl_mem_flags flags, cl_GLuint renderbuffer, cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_mem(CL_API_CALL * PFN_clCreateFromGLRenderbuffer)(cl_context context, cl_mem_flags flags, cl_GLuint renderbuffer, cl_int * errcode_ret);
	static PFN_clCreateFromGLRenderbuffer _CreateFromGLRenderbuffer = (PFN_clCreateFromGLRenderbuffer)cl::internals::getClFunction(cl::CLFunctions::CreateFromGLRenderbuffer);
	return _CreateFromGLRenderbuffer(context, flags, renderbuffer, errcode_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetGLObjectInfo)(cl_mem memobj, cl_gl_object_type* gl_object_type, cl_GLuint* gl_object_name) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFN_clGetGLObjectInfo)(cl_mem memobj, cl_gl_object_type*, cl_GLuint*);
	static PFN_clGetGLObjectInfo _GetGLObjectInfo = (PFN_clGetGLObjectInfo)cl::internals::getClFunction(cl::CLFunctions::GetGLObjectInfo);
	return _GetGLObjectInfo(memobj, gl_object_type, gl_object_name);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetGLTextureInfo)(
	cl_mem memobj, cl_gl_texture_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFN_clGetGLTextureInfo)(cl_mem, cl_gl_texture_info, size_t, void*, size_t*);
	static PFN_clGetGLTextureInfo _GetGLTextureInfo = (PFN_clGetGLTextureInfo)cl::internals::getClFunction(cl::CLFunctions::GetGLTextureInfo);
	return _GetGLTextureInfo(memobj, param_name, param_value_size, param_value, param_value_size_ret);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueAcquireGLObjects)(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFN_clEnqueueAcquireGLObjects)(cl_command_queue, cl_uint, const cl_mem*, cl_uint, const cl_event*, cl_event* event);

	static PFN_clEnqueueAcquireGLObjects _EnqueueAcquireGLObjects = (PFN_clEnqueueAcquireGLObjects)cl::internals::getClFunction(cl::CLFunctions::EnqueueAcquireGLObjects);
	return _EnqueueAcquireGLObjects(command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event);
}
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(EnqueueReleaseGLObjects)(cl_command_queue command_queue, cl_uint num_objects, const cl_mem* mem_objects,
	cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFN_clEnqueueReleaseGLObjects)(cl_command_queue, cl_uint, const cl_mem*, cl_uint, const cl_event*, cl_event* event);
	static PFN_clEnqueueReleaseGLObjects _EnqueueReleaseGLObjects = (PFN_clEnqueueReleaseGLObjects)cl::internals::getClFunction(cl::CLFunctions::EnqueueReleaseGLObjects);
	return _EnqueueReleaseGLObjects(command_queue, num_objects, mem_objects, num_events_in_wait_list, event_wait_list, event);
}
inline CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateFromGLTexture2D)(
	cl_context context, cl_mem_flags flags, cl_GLenum target, cl_GLint miplevel, cl_GLuint texture, cl_int* errcode_ret)
{
	typedef cl_mem(CL_API_CALL * PFN_clCreateFromGLTexture2D)(cl_context, cl_mem_flags, cl_GLenum, cl_GLint, cl_GLuint, cl_int*);
	static PFN_clCreateFromGLTexture2D _CreateFromGLTexture2D = (PFN_clCreateFromGLTexture2D)cl::internals::getClFunction(cl::CLFunctions::CreateFromGLTexture2D);
	return _CreateFromGLTexture2D(context, flags, target, miplevel, texture, errcode_ret);
}
inline CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_mem CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED CL_API_CALL DEFINE_CL_FUNCTION_NAME(CreateFromGLTexture3D)(
	cl_context context, cl_mem_flags flags, cl_GLenum target, cl_GLint miplevel, cl_GLuint texture, cl_int* errcode_ret)
{
	typedef cl_mem(CL_API_CALL * PFN_clCreateFromGLTexture3D)(cl_context, cl_mem_flags, cl_GLenum, cl_GLint, cl_GLuint, cl_int*);
	static PFN_clCreateFromGLTexture3D _CreateFromGLTexture3D = (PFN_clCreateFromGLTexture3D)cl::internals::getClFunction(cl::CLFunctions::CreateFromGLTexture3D);
	return _CreateFromGLTexture3D(context, flags, target, miplevel, texture, errcode_ret);
}
#ifdef cl_gl_context_info
inline cl_int CL_API_CALL DEFINE_CL_FUNCTION_NAME(GetGLContextInfoKHR)(
	const cl_context_properties* properties, cl_gl_context_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
	typedef cl_int(CL_API_CALL * PFN_clGetGLContextInfoKHR)(const cl_context_properties*, cl_gl_context_info, size_t, void*, size_t*);
	static PFN_clGetGLContextInfoKHR _GetGLContextInfoKHR = (PFN_clGetGLContextInfoKHR)cl::internals::getClFunction(cl::CLFunctions::GetGLContextInfoKHR);
	return _GetGLContextInfoKHR(properties, param_name, param_value_size, param_value, param_value_size_ret);
}
#endif
#ifndef DYNAMICCL_NO_NAMESPACE
}
#endif

namespace cl {
namespace internals {
namespace CLExtFunctions {
enum Enum
{
	CreateFromEGLImageKHR,
	EnqueueAcquireEGLObjectsKHR,
	EnqueueReleaseEGLObjectsKHR,
	CreateEventFromEGLSyncKHR,
	NUMBER_OF_CL_EXT_FUNCTIONS
};

static const std::pair<uint32_t, const char* const> OpenCLExtFunctionNamePairs[] = {
	{ CreateFromEGLImageKHR, "clCreateFromEGLImageKHR" },
	{ EnqueueAcquireEGLObjectsKHR, "clEnqueueAcquireEGLObjectsKHR" },
	{ EnqueueReleaseEGLObjectsKHR, "clEnqueueReleaseEGLObjectsKHR" },
	{ CreateEventFromEGLSyncKHR, "clCreateEventFromEGLSyncKHR" },
};
} // namespace CLExtFunctions

static inline void* GetClExtensionFunction(cl_platform_id platform, const char* funcName) { return (void*)cl::GetExtensionFunctionAddressForPlatform(platform, funcName); }

inline void* getClExtFunction(cl_platform_id platform, CLFunctions::Enum func, bool reset = false)
{
	static void* CLFunctionTable[CLExtFunctions::NUMBER_OF_CL_EXT_FUNCTIONS + 1];

	if (!CLFunctionTable[CLExtFunctions::NUMBER_OF_CL_EXT_FUNCTIONS] || reset)
	{
		uint32_t numExtensionStrings = sizeof(CLExtFunctions::OpenCLExtFunctionNamePairs) / sizeof(CLExtFunctions::OpenCLExtFunctionNamePairs[0]);
#if DEBUG
		uint32_t numExtensionEnums = CLExtFunctions::NUMBER_OF_CL_EXT_FUNCTIONS;
#endif
		assert(numExtensionStrings == numExtensionEnums);

		for (uint32_t i = 0; i < numExtensionStrings; i++)
		{
			assert(CLFunctionTable[i] == 0);
			CLFunctionTable[i] = GetClExtensionFunction(platform, CLExtFunctions::OpenCLExtFunctionNamePairs[i].second);
		}
		// Set the last element of the function table to avoid issues
		CLFunctionTable[CLExtFunctions::NUMBER_OF_CL_EXT_FUNCTIONS] = (void*)1;
	}
	return CLFunctionTable[func];
}
} // namespace internals
} // namespace cl
