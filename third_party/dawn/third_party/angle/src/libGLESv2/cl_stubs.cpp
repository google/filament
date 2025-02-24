//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_stubs.cpp: Stubs for CL entry points.

#include "libANGLE/cl_utils.h"
#include "libGLESv2/cl_stubs_autogen.h"

#include "libGLESv2/proc_table_cl.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLCommandQueue.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLDevice.h"
#include "libANGLE/CLEvent.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLMemory.h"
#include "libANGLE/CLPlatform.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/CLSampler.h"

#define WARN_NOT_SUPPORTED(command)                                         \
    do                                                                      \
    {                                                                       \
        static bool sWarned = false;                                        \
        if (!sWarned)                                                       \
        {                                                                   \
            sWarned = true;                                                 \
            WARN() << "OpenCL command " #command " is not (yet) supported"; \
        }                                                                   \
    } while (0)

#define CL_RETURN_ERROR(command)                                    \
    do                                                              \
    {                                                               \
        if (IsError(command))                                       \
        {                                                           \
            ERR() << "failed with error code: " << cl::gClErrorTls; \
        }                                                           \
        return cl::gClErrorTls;                                     \
    } while (0)

#define CL_RETURN_OBJ(command)                                      \
    do                                                              \
    {                                                               \
        auto obj = command;                                         \
        if (cl::gClErrorTls != CL_SUCCESS)                          \
        {                                                           \
            ERR() << "failed with error code: " << cl::gClErrorTls; \
            return nullptr;                                         \
        }                                                           \
        return obj;                                                 \
    } while (0)

#define CL_RETURN_PTR(ptrOut, command)                              \
    do                                                              \
    {                                                               \
        void *ptrOut = nullptr;                                     \
        if (IsError(command))                                       \
        {                                                           \
            ERR() << "failed with error code: " << cl::gClErrorTls; \
            return nullptr;                                         \
        }                                                           \
        return ptrOut;                                              \
    } while (0)

namespace cl
{

cl_int IcdGetPlatformIDsKHR(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms)
{
    CL_RETURN_ERROR((Platform::GetPlatformIDs(num_entries, platforms, num_platforms)));
}

cl_int GetPlatformIDs(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms)
{
    CL_RETURN_ERROR(Platform::GetPlatformIDs(num_entries, platforms, num_platforms));
}

cl_int GetPlatformInfo(cl_platform_id platform,
                       PlatformInfo param_name,
                       size_t param_value_size,
                       void *param_value,
                       size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(Platform::CastOrDefault(platform)->getInfo(param_name, param_value_size,
                                                               param_value, param_value_size_ret));
}

cl_int GetDeviceIDs(cl_platform_id platform,
                    DeviceType device_type,
                    cl_uint num_entries,
                    cl_device_id *devices,
                    cl_uint *num_devices)
{
    CL_RETURN_ERROR(Platform::CastOrDefault(platform)->getDeviceIDs(device_type, num_entries,
                                                                    devices, num_devices));
}

cl_int GetDeviceInfo(cl_device_id device,
                     DeviceInfo param_name,
                     size_t param_value_size,
                     void *param_value,
                     size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(device->cast<Device>().getInfo(param_name, param_value_size, param_value,
                                                   param_value_size_ret));
}

cl_int CreateSubDevices(cl_device_id in_device,
                        const cl_device_partition_property *properties,
                        cl_uint num_devices,
                        cl_device_id *out_devices,
                        cl_uint *num_devices_ret)
{
    CL_RETURN_ERROR(in_device->cast<Device>().createSubDevices(properties, num_devices, out_devices,
                                                               num_devices_ret));
}

cl_int RetainDevice(cl_device_id device)
{
    Device &dev = device->cast<Device>();
    if (!dev.isRoot())
    {
        dev.retain();
    }
    return CL_SUCCESS;
}

cl_int ReleaseDevice(cl_device_id device)
{
    Device &dev = device->cast<Device>();
    if (!dev.isRoot() && dev.release())
    {
        delete &dev;
    }
    return CL_SUCCESS;
}

cl_int SetDefaultDeviceCommandQueue(cl_context context,
                                    cl_device_id device,
                                    cl_command_queue command_queue)
{
    WARN_NOT_SUPPORTED(SetDefaultDeviceCommandQueue);
    return CL_INVALID_OPERATION;
}

cl_int GetDeviceAndHostTimer(cl_device_id device,
                             cl_ulong *device_timestamp,
                             cl_ulong *host_timestamp)
{
    WARN_NOT_SUPPORTED(GetDeviceAndHostTimer);
    return CL_INVALID_OPERATION;
}

cl_int GetHostTimer(cl_device_id device, cl_ulong *host_timestamp)
{
    WARN_NOT_SUPPORTED(GetHostTimer);
    return CL_INVALID_OPERATION;
}

cl_context CreateContext(const cl_context_properties *properties,
                         cl_uint num_devices,
                         const cl_device_id *devices,
                         void(CL_CALLBACK *pfn_notify)(const char *errinfo,
                                                       const void *private_info,
                                                       size_t cb,
                                                       void *user_data),
                         void *user_data)
{
    CL_RETURN_OBJ(Platform::CreateContext(properties, num_devices, devices, pfn_notify, user_data));
}

cl_context CreateContextFromType(const cl_context_properties *properties,
                                 DeviceType device_type,
                                 void(CL_CALLBACK *pfn_notify)(const char *errinfo,
                                                               const void *private_info,
                                                               size_t cb,
                                                               void *user_data),
                                 void *user_data)
{
    CL_RETURN_OBJ(Platform::CreateContextFromType(properties, device_type, pfn_notify, user_data));
}

cl_int RetainContext(cl_context context)
{
    context->cast<Context>().retain();
    return CL_SUCCESS;
}

cl_int ReleaseContext(cl_context context)
{
    Context &ctx = context->cast<Context>();
    if (ctx.release())
    {
        delete &ctx;
    }
    return CL_SUCCESS;
}

cl_int GetContextInfo(cl_context context,
                      ContextInfo param_name,
                      size_t param_value_size,
                      void *param_value,
                      size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(context->cast<Context>().getInfo(param_name, param_value_size, param_value,
                                                     param_value_size_ret));
}

cl_int SetContextDestructorCallback(cl_context context,
                                    void(CL_CALLBACK *pfn_notify)(cl_context context,
                                                                  void *user_data),
                                    void *user_data)
{
    WARN_NOT_SUPPORTED(SetContextDestructorCallback);
    return CL_INVALID_OPERATION;
}

cl_command_queue CreateCommandQueueWithProperties(cl_context context,
                                                  cl_device_id device,
                                                  const cl_queue_properties *properties)
{
    CL_RETURN_OBJ(context->cast<Context>().createCommandQueueWithProperties(device, properties));
}

cl_int RetainCommandQueue(cl_command_queue command_queue)
{
    command_queue->cast<CommandQueue>().retain();
    return CL_SUCCESS;
}

cl_int ReleaseCommandQueue(cl_command_queue command_queue)
{
    CommandQueue &queue = command_queue->cast<CommandQueue>();
    const cl_int err    = queue.onRelease();
    if (queue.release())
    {
        delete &queue;
    }
    return err;
}

cl_int GetCommandQueueInfo(cl_command_queue command_queue,
                           CommandQueueInfo param_name,
                           size_t param_value_size,
                           void *param_value,
                           size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().getInfo(param_name, param_value_size,
                                                                param_value, param_value_size_ret));
}

cl_mem CreateBuffer(cl_context context, MemFlags flags, size_t size, void *host_ptr)
{
    CL_RETURN_OBJ(context->cast<Context>().createBuffer(nullptr, flags, size, host_ptr));
}

cl_mem CreateBufferWithProperties(cl_context context,
                                  const cl_mem_properties *properties,
                                  MemFlags flags,
                                  size_t size,
                                  void *host_ptr)
{
    CL_RETURN_OBJ(context->cast<Context>().createBuffer(properties, flags, size, host_ptr));
}

cl_mem CreateSubBuffer(cl_mem buffer,
                       MemFlags flags,
                       cl_buffer_create_type buffer_create_type,
                       const void *buffer_create_info)
{
    CL_RETURN_OBJ(
        buffer->cast<Buffer>().createSubBuffer(flags, buffer_create_type, buffer_create_info));
}

cl_mem CreateImage(cl_context context,
                   MemFlags flags,
                   const cl_image_format *image_format,
                   const cl_image_desc *image_desc,
                   void *host_ptr)
{
    CL_RETURN_OBJ(
        context->cast<Context>().createImage(nullptr, flags, image_format, image_desc, host_ptr));
}

cl_mem CreateImageWithProperties(cl_context context,
                                 const cl_mem_properties *properties,
                                 MemFlags flags,
                                 const cl_image_format *image_format,
                                 const cl_image_desc *image_desc,
                                 void *host_ptr)
{
    CL_RETURN_OBJ(context->cast<Context>().createImage(properties, flags, image_format, image_desc,
                                                       host_ptr));
}

cl_mem CreatePipe(cl_context context,
                  MemFlags flags,
                  cl_uint pipe_packet_size,
                  cl_uint pipe_max_packets,
                  const cl_pipe_properties *properties)
{
    WARN_NOT_SUPPORTED(CreatePipe);
    cl::gClErrorTls = CL_INVALID_OPERATION;
    return 0;
}

cl_int RetainMemObject(cl_mem memobj)
{
    memobj->cast<Memory>().retain();
    return CL_SUCCESS;
}

cl_int ReleaseMemObject(cl_mem memobj)
{
    Memory &memory = memobj->cast<Memory>();
    if (memory.release())
    {
        delete &memory;
    }
    return CL_SUCCESS;
}

cl_int GetSupportedImageFormats(cl_context context,
                                MemFlags flags,
                                MemObjectType image_type,
                                cl_uint num_entries,
                                cl_image_format *image_formats,
                                cl_uint *num_image_formats)
{
    CL_RETURN_ERROR(context->cast<Context>().getSupportedImageFormats(
        flags, image_type, num_entries, image_formats, num_image_formats));
}

cl_int GetMemObjectInfo(cl_mem memobj,
                        MemInfo param_name,
                        size_t param_value_size,
                        void *param_value,
                        size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(memobj->cast<Memory>().getInfo(param_name, param_value_size, param_value,
                                                   param_value_size_ret));
}

cl_int GetImageInfo(cl_mem image,
                    ImageInfo param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(image->cast<Image>().getInfo(param_name, param_value_size, param_value,
                                                 param_value_size_ret));
}

cl_int GetPipeInfo(cl_mem pipe,
                   PipeInfo param_name,
                   size_t param_value_size,
                   void *param_value,
                   size_t *param_value_size_ret)
{
    WARN_NOT_SUPPORTED(GetPipeInfo);
    return CL_INVALID_MEM_OBJECT;
}

cl_int SetMemObjectDestructorCallback(cl_mem memobj,
                                      void(CL_CALLBACK *pfn_notify)(cl_mem memobj, void *user_data),
                                      void *user_data)
{
    CL_RETURN_ERROR(memobj->cast<Memory>().setDestructorCallback(pfn_notify, user_data));
}

void *SVMAlloc(cl_context context, SVM_MemFlags flags, size_t size, cl_uint alignment)
{
    WARN_NOT_SUPPORTED(SVMAlloc);
    return 0;
}

void SVMFree(cl_context context, void *svm_pointer)
{
    WARN_NOT_SUPPORTED(SVMFree);
}

cl_sampler CreateSamplerWithProperties(cl_context context,
                                       const cl_sampler_properties *sampler_properties)
{
    CL_RETURN_OBJ(context->cast<Context>().createSamplerWithProperties(sampler_properties));
}

cl_int RetainSampler(cl_sampler sampler)
{
    sampler->cast<Sampler>().retain();
    return CL_SUCCESS;
}

cl_int ReleaseSampler(cl_sampler sampler)
{
    Sampler &smplr = sampler->cast<Sampler>();
    if (smplr.release())
    {
        delete &smplr;
    }
    return CL_SUCCESS;
}

cl_int GetSamplerInfo(cl_sampler sampler,
                      SamplerInfo param_name,
                      size_t param_value_size,
                      void *param_value,
                      size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(sampler->cast<Sampler>().getInfo(param_name, param_value_size, param_value,
                                                     param_value_size_ret));
}

cl_program CreateProgramWithSource(cl_context context,
                                   cl_uint count,
                                   const char **strings,
                                   const size_t *lengths)
{
    CL_RETURN_OBJ(context->cast<Context>().createProgramWithSource(count, strings, lengths));
}

cl_program CreateProgramWithBinary(cl_context context,
                                   cl_uint num_devices,
                                   const cl_device_id *device_list,
                                   const size_t *lengths,
                                   const unsigned char **binaries,
                                   cl_int *binary_status)
{
    CL_RETURN_OBJ(context->cast<Context>().createProgramWithBinary(
        num_devices, device_list, lengths, binaries, binary_status));
}

cl_program CreateProgramWithBuiltInKernels(cl_context context,
                                           cl_uint num_devices,
                                           const cl_device_id *device_list,
                                           const char *kernel_names)
{
    CL_RETURN_OBJ(context->cast<Context>().createProgramWithBuiltInKernels(num_devices, device_list,
                                                                           kernel_names));
}

cl_program CreateProgramWithIL(cl_context context, const void *il, size_t length)
{
    CL_RETURN_OBJ(context->cast<Context>().createProgramWithIL(il, length));
}

cl_int RetainProgram(cl_program program)
{
    program->cast<Program>().retain();
    return CL_SUCCESS;
}

cl_int ReleaseProgram(cl_program program)
{
    Program &prog = program->cast<Program>();
    if (prog.release())
    {
        delete &prog;
    }
    return CL_SUCCESS;
}

cl_int BuildProgram(cl_program program,
                    cl_uint num_devices,
                    const cl_device_id *device_list,
                    const char *options,
                    void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                    void *user_data)
{
    CL_RETURN_ERROR(
        program->cast<Program>().build(num_devices, device_list, options, pfn_notify, user_data));
}

cl_int CompileProgram(cl_program program,
                      cl_uint num_devices,
                      const cl_device_id *device_list,
                      const char *options,
                      cl_uint num_input_headers,
                      const cl_program *input_headers,
                      const char **header_include_names,
                      void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                      void *user_data)
{
    CL_RETURN_ERROR(program->cast<Program>().compile(num_devices, device_list, options,
                                                     num_input_headers, input_headers,
                                                     header_include_names, pfn_notify, user_data));
}

cl_program LinkProgram(cl_context context,
                       cl_uint num_devices,
                       const cl_device_id *device_list,
                       const char *options,
                       cl_uint num_input_programs,
                       const cl_program *input_programs,
                       void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                       void *user_data)
{
    CL_RETURN_OBJ(context->cast<Context>().linkProgram(num_devices, device_list, options,
                                                       num_input_programs, input_programs,
                                                       pfn_notify, user_data));
}

cl_int SetProgramReleaseCallback(cl_program program,
                                 void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                                 void *user_data)
{
    WARN_NOT_SUPPORTED(SetProgramReleaseCallback);
    return CL_INVALID_OPERATION;
}

cl_int SetProgramSpecializationConstant(cl_program program,
                                        cl_uint spec_id,
                                        size_t spec_size,
                                        const void *spec_value)
{
    WARN_NOT_SUPPORTED(SetProgramSpecializationConstant);
    return CL_INVALID_OPERATION;
}

cl_int UnloadPlatformCompiler(cl_platform_id platform)
{
    CL_RETURN_ERROR(platform->cast<Platform>().unloadCompiler());
}

cl_int GetProgramInfo(cl_program program,
                      ProgramInfo param_name,
                      size_t param_value_size,
                      void *param_value,
                      size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(program->cast<Program>().getInfo(param_name, param_value_size, param_value,
                                                     param_value_size_ret));
}

cl_int GetProgramBuildInfo(cl_program program,
                           cl_device_id device,
                           ProgramBuildInfo param_name,
                           size_t param_value_size,
                           void *param_value,
                           size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(program->cast<Program>().getBuildInfo(device, param_name, param_value_size,
                                                          param_value, param_value_size_ret));
}

cl_kernel CreateKernel(cl_program program, const char *kernel_name)
{
    CL_RETURN_OBJ(program->cast<Program>().createKernel(kernel_name));
}

cl_int CreateKernelsInProgram(cl_program program,
                              cl_uint num_kernels,
                              cl_kernel *kernels,
                              cl_uint *num_kernels_ret)
{
    CL_RETURN_ERROR(program->cast<Program>().createKernels(num_kernels, kernels, num_kernels_ret));
}

cl_kernel CloneKernel(cl_kernel source_kernel)
{
    CL_RETURN_OBJ(source_kernel->cast<Kernel>().clone(););
}

cl_int RetainKernel(cl_kernel kernel)
{
    kernel->cast<Kernel>().retain();
    return CL_SUCCESS;
}

cl_int ReleaseKernel(cl_kernel kernel)
{
    Kernel &krnl = kernel->cast<Kernel>();
    if (krnl.release())
    {
        delete &krnl;
    }
    return CL_SUCCESS;
}

cl_int SetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value)
{
    CL_RETURN_ERROR(kernel->cast<Kernel>().setArg(arg_index, arg_size, arg_value));
}

cl_int SetKernelArgSVMPointer(cl_kernel kernel, cl_uint arg_index, const void *arg_value)
{
    WARN_NOT_SUPPORTED(SetKernelArgSVMPointer);
    return CL_INVALID_OPERATION;
}

cl_int SetKernelExecInfo(cl_kernel kernel,
                         KernelExecInfo param_name,
                         size_t param_value_size,
                         const void *param_value)
{
    WARN_NOT_SUPPORTED(SetKernelExecInfo);
    return CL_INVALID_OPERATION;
}

cl_int GetKernelInfo(cl_kernel kernel,
                     KernelInfo param_name,
                     size_t param_value_size,
                     void *param_value,
                     size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(kernel->cast<Kernel>().getInfo(param_name, param_value_size, param_value,
                                                   param_value_size_ret));
}

cl_int GetKernelArgInfo(cl_kernel kernel,
                        cl_uint arg_index,
                        KernelArgInfo param_name,
                        size_t param_value_size,
                        void *param_value,
                        size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(kernel->cast<Kernel>().getArgInfo(arg_index, param_name, param_value_size,
                                                      param_value, param_value_size_ret));
}

cl_int GetKernelWorkGroupInfo(cl_kernel kernel,
                              cl_device_id device,
                              KernelWorkGroupInfo param_name,
                              size_t param_value_size,
                              void *param_value,
                              size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(kernel->cast<Kernel>().getWorkGroupInfo(device, param_name, param_value_size,
                                                            param_value, param_value_size_ret));
}

cl_int GetKernelSubGroupInfo(cl_kernel kernel,
                             cl_device_id device,
                             KernelSubGroupInfo param_name,
                             size_t input_value_size,
                             const void *input_value,
                             size_t param_value_size,
                             void *param_value,
                             size_t *param_value_size_ret)
{
    WARN_NOT_SUPPORTED(GetKernelSubGroupInfo);
    return CL_INVALID_OPERATION;
}

cl_int WaitForEvents(cl_uint num_events, const cl_event *event_list)
{
    CL_RETURN_ERROR(
        (*event_list)->cast<Event>().getContext().waitForEvents(num_events, event_list));
}

cl_int GetEventInfo(cl_event event,
                    EventInfo param_name,
                    size_t param_value_size,
                    void *param_value,
                    size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(event->cast<Event>().getInfo(param_name, param_value_size, param_value,
                                                 param_value_size_ret));
}

cl_event CreateUserEvent(cl_context context)
{
    CL_RETURN_OBJ(context->cast<Context>().createUserEvent());
}

cl_int RetainEvent(cl_event event)
{
    event->cast<Event>().retain();
    return CL_SUCCESS;
}

cl_int ReleaseEvent(cl_event event)
{
    Event &evt = event->cast<Event>();
    if (evt.release())
    {
        delete &evt;
    }
    return CL_SUCCESS;
}

cl_int SetUserEventStatus(cl_event event, cl_int execution_status)
{
    CL_RETURN_ERROR(event->cast<Event>().setUserEventStatus(execution_status));
}

cl_int SetEventCallback(cl_event event,
                        cl_int command_exec_callback_type,
                        void(CL_CALLBACK *pfn_notify)(cl_event event,
                                                      cl_int event_command_status,
                                                      void *user_data),
                        void *user_data)
{
    CL_RETURN_ERROR(
        event->cast<Event>().setCallback(command_exec_callback_type, pfn_notify, user_data));
}

cl_int GetEventProfilingInfo(cl_event event,
                             ProfilingInfo param_name,
                             size_t param_value_size,
                             void *param_value,
                             size_t *param_value_size_ret)
{
    CL_RETURN_ERROR(event->cast<Event>().getProfilingInfo(param_name, param_value_size, param_value,
                                                          param_value_size_ret));
}

cl_int Flush(cl_command_queue command_queue)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().flush());
}

cl_int Finish(cl_command_queue command_queue)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().finish());
}

cl_int EnqueueReadBuffer(cl_command_queue command_queue,
                         cl_mem buffer,
                         cl_bool blocking_read,
                         size_t offset,
                         size_t size,
                         void *ptr,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueReadBuffer(
        buffer, blocking_read, offset, size, ptr, num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueReadBufferRect(cl_command_queue command_queue,
                             cl_mem buffer,
                             cl_bool blocking_read,
                             const size_t *buffer_origin,
                             const size_t *host_origin,
                             const size_t *region,
                             size_t buffer_row_pitch,
                             size_t buffer_slice_pitch,
                             size_t host_row_pitch,
                             size_t host_slice_pitch,
                             void *ptr,
                             cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list,
                             cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueReadBufferRect(
        buffer, blocking_read, cl::MemOffsets{buffer_origin[0], buffer_origin[1], buffer_origin[2]},
        cl::MemOffsets{host_origin[0], host_origin[1], host_origin[2]},
        cl::Coordinate{region[0], region[1], region[2]}, buffer_row_pitch, buffer_slice_pitch,
        host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueWriteBuffer(cl_command_queue command_queue,
                          cl_mem buffer,
                          cl_bool blocking_write,
                          size_t offset,
                          size_t size,
                          const void *ptr,
                          cl_uint num_events_in_wait_list,
                          const cl_event *event_wait_list,
                          cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueWriteBuffer(
        buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list,
        event));
}

cl_int EnqueueWriteBufferRect(cl_command_queue command_queue,
                              cl_mem buffer,
                              cl_bool blocking_write,
                              const size_t *buffer_origin,
                              const size_t *host_origin,
                              const size_t *region,
                              size_t buffer_row_pitch,
                              size_t buffer_slice_pitch,
                              size_t host_row_pitch,
                              size_t host_slice_pitch,
                              const void *ptr,
                              cl_uint num_events_in_wait_list,
                              const cl_event *event_wait_list,
                              cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueWriteBufferRect(
        buffer, blocking_write,
        cl::MemOffsets{buffer_origin[0], buffer_origin[1], buffer_origin[2]},
        cl::MemOffsets{host_origin[0], host_origin[1], host_origin[2]},
        cl::Coordinate{region[0], region[1], region[2]}, buffer_row_pitch, buffer_slice_pitch,
        host_row_pitch, host_slice_pitch, ptr, num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueFillBuffer(cl_command_queue command_queue,
                         cl_mem buffer,
                         const void *pattern,
                         size_t pattern_size,
                         size_t offset,
                         size_t size,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueFillBuffer(
        buffer, pattern, pattern_size, offset, size, num_events_in_wait_list, event_wait_list,
        event));
}

cl_int EnqueueCopyBuffer(cl_command_queue command_queue,
                         cl_mem src_buffer,
                         cl_mem dst_buffer,
                         size_t src_offset,
                         size_t dst_offset,
                         size_t size,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueCopyBuffer(
        src_buffer, dst_buffer, src_offset, dst_offset, size, num_events_in_wait_list,
        event_wait_list, event));
}

cl_int EnqueueCopyBufferRect(cl_command_queue command_queue,
                             cl_mem src_buffer,
                             cl_mem dst_buffer,
                             const size_t *src_origin,
                             const size_t *dst_origin,
                             const size_t *region,
                             size_t src_row_pitch,
                             size_t src_slice_pitch,
                             size_t dst_row_pitch,
                             size_t dst_slice_pitch,
                             cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list,
                             cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueCopyBufferRect(
        src_buffer, dst_buffer, cl::MemOffsets{src_origin[0], src_origin[1], src_origin[2]},
        cl::MemOffsets{dst_origin[0], dst_origin[1], dst_origin[2]},
        cl::Coordinate{region[0], region[1], region[2]}, src_row_pitch, src_slice_pitch,
        dst_row_pitch, dst_slice_pitch, num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueReadImage(cl_command_queue command_queue,
                        cl_mem image,
                        cl_bool blocking_read,
                        const size_t *origin,
                        const size_t *region,
                        size_t row_pitch,
                        size_t slice_pitch,
                        void *ptr,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueReadImage(
        image, blocking_read, cl::MemOffsets{origin[0], origin[1], origin[2]},
        cl::Coordinate{region[0], region[1], region[2]}, row_pitch, slice_pitch, ptr,
        num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueWriteImage(cl_command_queue command_queue,
                         cl_mem image,
                         cl_bool blocking_write,
                         const size_t *origin,
                         const size_t *region,
                         size_t input_row_pitch,
                         size_t input_slice_pitch,
                         const void *ptr,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueWriteImage(
        image, blocking_write, cl::MemOffsets{origin[0], origin[1], origin[2]},
        cl::Coordinate{region[0], region[1], region[2]}, input_row_pitch, input_slice_pitch, ptr,
        num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueFillImage(cl_command_queue command_queue,
                        cl_mem image,
                        const void *fill_color,
                        const size_t *origin,
                        const size_t *region,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueFillImage(
        image, fill_color, cl::MemOffsets{origin[0], origin[1], origin[2]},
        cl::Coordinate{region[0], region[1], region[2]}, num_events_in_wait_list, event_wait_list,
        event));
}

cl_int EnqueueCopyImage(cl_command_queue command_queue,
                        cl_mem src_image,
                        cl_mem dst_image,
                        const size_t *src_origin,
                        const size_t *dst_origin,
                        const size_t *region,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueCopyImage(
        src_image, dst_image, cl::MemOffsets{src_origin[0], src_origin[1], src_origin[2]},
        cl::MemOffsets{dst_origin[0], dst_origin[1], dst_origin[2]},
        cl::Coordinate{region[0], region[1], region[2]}, num_events_in_wait_list, event_wait_list,
        event));
}

cl_int EnqueueCopyImageToBuffer(cl_command_queue command_queue,
                                cl_mem src_image,
                                cl_mem dst_buffer,
                                const size_t *src_origin,
                                const size_t *region,
                                size_t dst_offset,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueCopyImageToBuffer(
        src_image, dst_buffer, cl::MemOffsets{src_origin[0], src_origin[1], src_origin[2]},
        cl::Coordinate{region[0], region[1], region[2]}, dst_offset, num_events_in_wait_list,
        event_wait_list, event));
}

cl_int EnqueueCopyBufferToImage(cl_command_queue command_queue,
                                cl_mem src_buffer,
                                cl_mem dst_image,
                                size_t src_offset,
                                const size_t *dst_origin,
                                const size_t *region,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueCopyBufferToImage(
        src_buffer, dst_image, src_offset,
        cl::MemOffsets{dst_origin[0], dst_origin[1], dst_origin[2]},
        cl::Coordinate{region[0], region[1], region[2]}, num_events_in_wait_list, event_wait_list,
        event));
}

void *EnqueueMapBuffer(cl_command_queue command_queue,
                       cl_mem buffer,
                       cl_bool blocking_map,
                       MapFlags map_flags,
                       size_t offset,
                       size_t size,
                       cl_uint num_events_in_wait_list,
                       const cl_event *event_wait_list,
                       cl_event *event)
{
    CL_RETURN_PTR(ptrOut, command_queue->cast<CommandQueue>().enqueueMapBuffer(
                              buffer, blocking_map, map_flags, offset, size,
                              num_events_in_wait_list, event_wait_list, event, ptrOut));
}

void *EnqueueMapImage(cl_command_queue command_queue,
                      cl_mem image,
                      cl_bool blocking_map,
                      MapFlags map_flags,
                      const size_t *origin,
                      const size_t *region,
                      size_t *image_row_pitch,
                      size_t *image_slice_pitch,
                      cl_uint num_events_in_wait_list,
                      const cl_event *event_wait_list,
                      cl_event *event)
{
    CL_RETURN_PTR(
        ptrOut, command_queue->cast<CommandQueue>().enqueueMapImage(
                    image, blocking_map, map_flags, cl::MemOffsets{origin[0], origin[1], origin[2]},
                    cl::Coordinate{region[0], region[1], region[2]}, image_row_pitch,
                    image_slice_pitch, num_events_in_wait_list, event_wait_list, event, ptrOut));
}

cl_int EnqueueUnmapMemObject(cl_command_queue command_queue,
                             cl_mem memobj,
                             void *mapped_ptr,
                             cl_uint num_events_in_wait_list,
                             const cl_event *event_wait_list,
                             cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueUnmapMemObject(
        memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueMigrateMemObjects(cl_command_queue command_queue,
                                cl_uint num_mem_objects,
                                const cl_mem *mem_objects,
                                MemMigrationFlags flags,
                                cl_uint num_events_in_wait_list,
                                const cl_event *event_wait_list,
                                cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueMigrateMemObjects(
        num_mem_objects, mem_objects, flags, num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueNDRangeKernel(cl_command_queue command_queue,
                            cl_kernel kernel,
                            cl_uint work_dim,
                            const size_t *global_work_offset,
                            const size_t *global_work_size,
                            const size_t *local_work_size,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list,
                            cl_event *event)
{
    cl::NDRange ndrange(work_dim, global_work_offset, global_work_size, local_work_size);
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueNDRangeKernel(
        kernel, ndrange, num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueNativeKernel(cl_command_queue command_queue,
                           void(CL_CALLBACK *user_func)(void *),
                           void *args,
                           size_t cb_args,
                           cl_uint num_mem_objects,
                           const cl_mem *mem_list,
                           const void **args_mem_loc,
                           cl_uint num_events_in_wait_list,
                           const cl_event *event_wait_list,
                           cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueNativeKernel(
        user_func, args, cb_args, num_mem_objects, mem_list, args_mem_loc, num_events_in_wait_list,
        event_wait_list, event));
}

cl_int EnqueueMarkerWithWaitList(cl_command_queue command_queue,
                                 cl_uint num_events_in_wait_list,
                                 const cl_event *event_wait_list,
                                 cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueMarkerWithWaitList(
        num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueBarrierWithWaitList(cl_command_queue command_queue,
                                  cl_uint num_events_in_wait_list,
                                  const cl_event *event_wait_list,
                                  cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueBarrierWithWaitList(
        num_events_in_wait_list, event_wait_list, event));
}

cl_int EnqueueSVMFree(cl_command_queue command_queue,
                      cl_uint num_svm_pointers,
                      void *svm_pointers[],
                      void(CL_CALLBACK *pfn_free_func)(cl_command_queue queue,
                                                       cl_uint num_svm_pointers,
                                                       void *svm_pointers[],
                                                       void *user_data),
                      void *user_data,
                      cl_uint num_events_in_wait_list,
                      const cl_event *event_wait_list,
                      cl_event *event)
{
    WARN_NOT_SUPPORTED(EnqueueSVMFree);
    return CL_INVALID_OPERATION;
}

cl_int EnqueueSVMMemcpy(cl_command_queue command_queue,
                        cl_bool blocking_copy,
                        void *dst_ptr,
                        const void *src_ptr,
                        size_t size,
                        cl_uint num_events_in_wait_list,
                        const cl_event *event_wait_list,
                        cl_event *event)
{
    WARN_NOT_SUPPORTED(EnqueueSVMMemcpy);
    return CL_INVALID_OPERATION;
}

cl_int EnqueueSVMMemFill(cl_command_queue command_queue,
                         void *svm_ptr,
                         const void *pattern,
                         size_t pattern_size,
                         size_t size,
                         cl_uint num_events_in_wait_list,
                         const cl_event *event_wait_list,
                         cl_event *event)
{
    WARN_NOT_SUPPORTED(EnqueueSVMMemFill);
    return CL_INVALID_OPERATION;
}

cl_int EnqueueSVMMap(cl_command_queue command_queue,
                     cl_bool blocking_map,
                     MapFlags flags,
                     void *svm_ptr,
                     size_t size,
                     cl_uint num_events_in_wait_list,
                     const cl_event *event_wait_list,
                     cl_event *event)
{
    WARN_NOT_SUPPORTED(EnqueueSVMMap);
    return CL_INVALID_OPERATION;
}

cl_int EnqueueSVMUnmap(cl_command_queue command_queue,
                       void *svm_ptr,
                       cl_uint num_events_in_wait_list,
                       const cl_event *event_wait_list,
                       cl_event *event)
{
    WARN_NOT_SUPPORTED(EnqueueSVMUnmap);
    return CL_INVALID_OPERATION;
}

cl_int EnqueueSVMMigrateMem(cl_command_queue command_queue,
                            cl_uint num_svm_pointers,
                            const void **svm_pointers,
                            const size_t *sizes,
                            MemMigrationFlags flags,
                            cl_uint num_events_in_wait_list,
                            const cl_event *event_wait_list,
                            cl_event *event)
{
    WARN_NOT_SUPPORTED(EnqueueSVMMigrateMem);
    return CL_INVALID_OPERATION;
}

void *GetExtensionFunctionAddressForPlatform(cl_platform_id platform, const char *func_name)
{
    return GetExtensionFunctionAddress(func_name);
}

cl_int SetCommandQueueProperty(cl_command_queue command_queue,
                               CommandQueueProperties properties,
                               cl_bool enable,
                               cl_command_queue_properties *old_properties)
{
    CL_RETURN_ERROR(
        command_queue->cast<CommandQueue>().setProperty(properties, enable, old_properties));
}

cl_mem CreateImage2D(cl_context context,
                     MemFlags flags,
                     const cl_image_format *image_format,
                     size_t image_width,
                     size_t image_height,
                     size_t image_row_pitch,
                     void *host_ptr)
{
    CL_RETURN_OBJ(context->cast<Context>().createImage2D(flags, image_format, image_width,
                                                         image_height, image_row_pitch, host_ptr));
}

cl_mem CreateImage3D(cl_context context,
                     MemFlags flags,
                     const cl_image_format *image_format,
                     size_t image_width,
                     size_t image_height,
                     size_t image_depth,
                     size_t image_row_pitch,
                     size_t image_slice_pitch,
                     void *host_ptr)
{
    CL_RETURN_OBJ(context->cast<Context>().createImage3D(flags, image_format, image_width,
                                                         image_height, image_depth, image_row_pitch,
                                                         image_slice_pitch, host_ptr));
}

cl_int EnqueueMarker(cl_command_queue command_queue, cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueMarker(event));
}

cl_int EnqueueWaitForEvents(cl_command_queue command_queue,
                            cl_uint num_events,
                            const cl_event *event_list)
{
    CL_RETURN_ERROR(
        command_queue->cast<CommandQueue>().enqueueWaitForEvents(num_events, event_list));
}

cl_int EnqueueBarrier(cl_command_queue command_queue)
{
    CL_RETURN_ERROR(IsError(command_queue->cast<CommandQueue>().enqueueBarrier()));
}

cl_int UnloadCompiler()
{
    Platform *const platform = Platform::GetDefault();
    if (platform == nullptr)
    {
        return CL_SUCCESS;
    }
    CL_RETURN_ERROR(platform->unloadCompiler());
}

void *GetExtensionFunctionAddress(const char *func_name)
{
    if (func_name == nullptr)
    {
        return nullptr;
    }
    const ProcTable &procTable = GetProcTable();
    const auto it              = procTable.find(func_name);
    return it != procTable.cend() ? it->second : nullptr;
}

cl_command_queue CreateCommandQueue(cl_context context,
                                    cl_device_id device,
                                    CommandQueueProperties properties)
{
    CL_RETURN_OBJ(context->cast<Context>().createCommandQueue(device, properties));
}

cl_sampler CreateSampler(cl_context context,
                         cl_bool normalized_coords,
                         AddressingMode addressing_mode,
                         FilterMode filter_mode)
{
    CL_RETURN_OBJ(
        context->cast<Context>().createSampler(normalized_coords, addressing_mode, filter_mode));
}

cl_int EnqueueTask(cl_command_queue command_queue,
                   cl_kernel kernel,
                   cl_uint num_events_in_wait_list,
                   const cl_event *event_wait_list,
                   cl_event *event)
{
    CL_RETURN_ERROR(command_queue->cast<CommandQueue>().enqueueTask(kernel, num_events_in_wait_list,
                                                                    event_wait_list, event));
}

}  // namespace cl
