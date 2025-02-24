//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgram.h: Defines the cl::Program class, which consists of a set of OpenCL kernels.

#ifndef LIBANGLE_CLPROGRAM_H_
#define LIBANGLE_CLPROGRAM_H_

#include "libANGLE/CLDevice.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/cl_utils.h"
#include "libANGLE/renderer/CLProgramImpl.h"

#include "common/SynchronizedValue.h"

#include <atomic>

namespace cl
{

class Program final : public _cl_program, public Object
{
  public:
    // Front end entry functions, only called from OpenCL entry points

    angle::Result build(cl_uint numDevices,
                        const cl_device_id *deviceList,
                        const char *options,
                        ProgramCB pfnNotify,
                        void *userData);

    angle::Result compile(cl_uint numDevices,
                          const cl_device_id *deviceList,
                          const char *options,
                          cl_uint numInputHeaders,
                          const cl_program *inputHeaders,
                          const char **headerIncludeNames,
                          ProgramCB pfnNotify,
                          void *userData);

    angle::Result getInfo(ProgramInfo name,
                          size_t valueSize,
                          void *value,
                          size_t *valueSizeRet) const;

    angle::Result getBuildInfo(cl_device_id device,
                               ProgramBuildInfo name,
                               size_t valueSize,
                               void *value,
                               size_t *valueSizeRet) const;

    cl_kernel createKernel(const char *kernel_name);

    angle::Result createKernels(cl_uint numKernels, cl_kernel *kernels, cl_uint *numKernelsRet);

  public:
    ~Program() override;

    Context &getContext();
    const Context &getContext() const;
    const DevicePtrs &getDevices() const;
    const std::string &getSource() const;
    bool hasDevice(const _cl_device_id *device) const;

    bool isBuilding() const;
    bool hasAttachedKernels() const;

    template <typename T = rx::CLProgramImpl>
    T &getImpl() const;

    void callback();

  private:
    Program(Context &context, std::string &&source);
    Program(Context &context, const void *il, size_t length);

    Program(Context &context,
            DevicePtrs &&devices,
            const size_t *lengths,
            const unsigned char **binaries,
            cl_int *binaryStatus);

    Program(Context &context, DevicePtrs &&devices, const char *kernelNames);

    Program(Context &context,
            const DevicePtrs &devices,
            const char *options,
            const cl::ProgramPtrs &inputPrograms,
            ProgramCB pfnNotify,
            void *userData);

    using CallbackData = std::pair<ProgramCB, void *>;

    const ContextPtr mContext;
    const DevicePtrs mDevices;
    const std::string mIL;

    // mCallback might be accessed from implementation initialization
    // and needs to be initialized first.
    angle::SynchronizedValue<CallbackData> mCallback;
    std::atomic<cl_uint> mNumAttachedKernels;

    rx::CLProgramImpl::Ptr mImpl;
    const std::string mSource;

    friend class Kernel;
    friend class Object;
};

inline Context &Program::getContext()
{
    return *mContext;
}

inline const Context &Program::getContext() const
{
    return *mContext;
}

inline const DevicePtrs &Program::getDevices() const
{
    return mDevices;
}

inline const std::string &Program::getSource() const
{
    return mSource;
}

inline bool Program::hasDevice(const _cl_device_id *device) const
{
    return std::find(mDevices.cbegin(), mDevices.cend(), device) != mDevices.cend();
}

inline bool Program::isBuilding() const
{
    for (const DevicePtr &device : getDevices())
    {
        cl_build_status buildStatus;
        ANGLE_CL_IMPL_TRY(getBuildInfo(device->getNative(), ProgramBuildInfo::Status,
                                       sizeof(cl_build_status), &buildStatus, nullptr));
        if ((mCallback->first != nullptr) || (buildStatus == CL_BUILD_IN_PROGRESS))
        {
            return true;
        }
    }
    return false;
}

inline bool Program::hasAttachedKernels() const
{
    return mNumAttachedKernels != 0u;
}

template <typename T>
inline T &Program::getImpl() const
{
    return static_cast<T &>(*mImpl);
}

}  // namespace cl

#endif  // LIBANGLE_CLPROGRAM_H_
