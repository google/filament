//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLKernelImpl.h: Defines the abstract rx::CLKernelImpl class.

#ifndef LIBANGLE_RENDERER_CLKERNELIMPL_H_
#define LIBANGLE_RENDERER_CLKERNELIMPL_H_

#include "libANGLE/renderer/cl_types.h"

namespace rx
{

class CLKernelImpl : angle::NonCopyable
{
  public:
    using Ptr         = std::unique_ptr<CLKernelImpl>;
    using CreateFunc  = std::function<Ptr(const cl::Kernel &)>;
    using CreateFuncs = std::list<CreateFunc>;

    struct WorkGroupInfo
    {
        WorkGroupInfo();
        ~WorkGroupInfo();

        WorkGroupInfo(const WorkGroupInfo &)            = delete;
        WorkGroupInfo &operator=(const WorkGroupInfo &) = delete;

        WorkGroupInfo(WorkGroupInfo &&);
        WorkGroupInfo &operator=(WorkGroupInfo &&);

        std::array<size_t, 3u> globalWorkSize       = {};
        size_t workGroupSize                        = 0u;
        std::array<size_t, 3u> compileWorkGroupSize = {};
        cl_ulong localMemSize                       = 0u;
        size_t prefWorkGroupSizeMultiple            = 0u;
        cl_ulong privateMemSize                     = 0u;
    };

    struct ArgInfo
    {
        ArgInfo();
        ~ArgInfo();

        ArgInfo(const ArgInfo &)            = default;
        ArgInfo &operator=(const ArgInfo &) = default;

        ArgInfo(ArgInfo &&);
        ArgInfo &operator=(ArgInfo &&);

        bool isAvailable() const { return !name.empty(); }

        cl_kernel_arg_address_qualifier addressQualifier = 0u;
        cl_kernel_arg_access_qualifier accessQualifier   = 0u;
        std::string typeName;
        cl_kernel_arg_type_qualifier typeQualifier = 0u;
        std::string name;
    };

    struct Info
    {
        Info();
        ~Info();

        Info(const Info &)            = delete;
        Info &operator=(const Info &) = delete;

        Info(Info &&);
        Info &operator=(Info &&);

        bool isValid() const { return !functionName.empty(); }

        std::string functionName;
        cl_uint numArgs = 0u;
        std::string attributes;

        std::vector<WorkGroupInfo> workGroups;
        std::vector<ArgInfo> args;
    };

    CLKernelImpl(const cl::Kernel &kernel);
    virtual ~CLKernelImpl();

    virtual angle::Result setArg(cl_uint argIndex, size_t argSize, const void *argValue) = 0;

    virtual angle::Result createInfo(Info *infoOut) const = 0;

  protected:
    const cl::Kernel &mKernel;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLKERNELIMPL_H_
