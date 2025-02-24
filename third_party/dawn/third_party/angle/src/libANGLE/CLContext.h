//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLContext.h: Defines the cl::Context class, which manages OpenCL objects such as command-queues,
// memory, program and kernel objects and for executing kernels on one or more devices.

#ifndef LIBANGLE_CLCONTEXT_H_
#define LIBANGLE_CLCONTEXT_H_

#include "libANGLE/CLDevice.h"
#include "libANGLE/CLPlatform.h"
#include "libANGLE/renderer/CLContextImpl.h"

namespace cl
{

class Context final : public _cl_context, public Object
{
  public:
    // Front end entry functions, only called from OpenCL entry points

    static bool IsValidAndVersionOrNewer(const _cl_context *context, cl_uint major, cl_uint minor);

    angle::Result getInfo(ContextInfo name,
                          size_t valueSize,
                          void *value,
                          size_t *valueSizeRet) const;

    cl_command_queue createCommandQueueWithProperties(cl_device_id device,
                                                      const cl_queue_properties *properties);

    cl_command_queue createCommandQueue(cl_device_id device, CommandQueueProperties properties);

    cl_mem createBuffer(const cl_mem_properties *properties,
                        MemFlags flags,
                        size_t size,
                        void *hostPtr);

    cl_mem createImage(const cl_mem_properties *properties,
                       MemFlags flags,
                       const cl_image_format *format,
                       const cl_image_desc *desc,
                       void *hostPtr);

    cl_mem createImage2D(MemFlags flags,
                         const cl_image_format *format,
                         size_t width,
                         size_t height,
                         size_t rowPitch,
                         void *hostPtr);

    cl_mem createImage3D(MemFlags flags,
                         const cl_image_format *format,
                         size_t width,
                         size_t height,
                         size_t depth,
                         size_t rowPitch,
                         size_t slicePitch,
                         void *hostPtr);

    angle::Result getSupportedImageFormats(MemFlags flags,
                                           MemObjectType imageType,
                                           cl_uint numEntries,
                                           cl_image_format *imageFormats,
                                           cl_uint *numImageFormats);

    cl_sampler createSamplerWithProperties(const cl_sampler_properties *properties);

    cl_sampler createSampler(cl_bool normalizedCoords,
                             AddressingMode addressingMode,
                             FilterMode filterMode);

    cl_program createProgramWithSource(cl_uint count, const char **strings, const size_t *lengths);

    cl_program createProgramWithIL(const void *il, size_t length);

    cl_program createProgramWithBinary(cl_uint numDevices,
                                       const cl_device_id *devices,
                                       const size_t *lengths,
                                       const unsigned char **binaries,
                                       cl_int *binaryStatus);

    cl_program createProgramWithBuiltInKernels(cl_uint numDevices,
                                               const cl_device_id *devices,
                                               const char *kernelNames);

    cl_program linkProgram(cl_uint numDevices,
                           const cl_device_id *deviceList,
                           const char *options,
                           cl_uint numInputPrograms,
                           const cl_program *inputPrograms,
                           ProgramCB pfnNotify,
                           void *userData);

    cl_event createUserEvent();

    angle::Result waitForEvents(cl_uint numEvents, const cl_event *eventList);

  public:
    using PropArray = std::vector<cl_context_properties>;

    ~Context() override;

    const Platform &getPlatform() const noexcept;
    const DevicePtrs &getDevices() const;
    bool hasDevice(const _cl_device_id *device) const;

    template <typename T = rx::CLContextImpl>
    T &getImpl() const;

    bool supportsImages() const;
    bool supportsIL() const;
    bool supportsBuiltInKernel(const std::string &name) const;
    bool supportsImage2DFromBuffer() const;

    static void CL_CALLBACK ErrorCallback(const char *errinfo,
                                          const void *privateInfo,
                                          size_t cb,
                                          void *userData);

  private:
    Context(Platform &platform,
            PropArray &&properties,
            DevicePtrs &&devices,
            ContextErrorCB notify,
            void *userData,
            bool userSync);

    Context(Platform &platform,
            PropArray &&properties,
            DeviceType deviceType,
            ContextErrorCB notify,
            void *userData,
            bool userSync);

    Platform &mPlatform;
    const PropArray mProperties;
    const ContextErrorCB mNotify;
    void *const mUserData;
    rx::CLContextImpl::Ptr mImpl;
    DevicePtrs mDevices;

    friend class Object;
};

inline bool Context::IsValidAndVersionOrNewer(const _cl_context *context,
                                              cl_uint major,
                                              cl_uint minor)
{
    return IsValid(context) &&
           context->cast<Context>().getPlatform().isVersionOrNewer(major, minor);
}

inline const Platform &Context::getPlatform() const noexcept
{
    return mPlatform;
}

inline const DevicePtrs &Context::getDevices() const
{
    return mDevices;
}

inline bool Context::hasDevice(const _cl_device_id *device) const
{
    return std::find(mDevices.cbegin(), mDevices.cend(), device) != mDevices.cend();
}

template <typename T>
inline T &Context::getImpl() const
{
    return static_cast<T &>(*mImpl);
}

inline bool Context::supportsImages() const
{
    return (std::find_if(mDevices.cbegin(), mDevices.cend(), [](const DevicePtr &ptr) {
                return ptr->getInfo().imageSupport == CL_TRUE;
            }) != mDevices.cend());
}

inline bool Context::supportsImage2DFromBuffer() const
{
    return (std::find_if(mDevices.cbegin(), mDevices.cend(), [](const DevicePtr &ptr) {
                return ptr->getInfo().khrImage2D_FromBuffer == true;
            }) != mDevices.cend());
}

inline bool Context::supportsIL() const
{
    return (std::find_if(mDevices.cbegin(), mDevices.cend(), [](const DevicePtr &ptr) {
                return !ptr->getInfo().IL_Version.empty();
            }) != mDevices.cend());
}

inline bool Context::supportsBuiltInKernel(const std::string &name) const
{
    return (std::find_if(mDevices.cbegin(), mDevices.cend(), [&](const DevicePtr &ptr) {
                return ptr->supportsBuiltInKernel(name);
            }) != mDevices.cend());
}

}  // namespace cl

#endif  // LIBANGLE_CLCONTEXT_H_
