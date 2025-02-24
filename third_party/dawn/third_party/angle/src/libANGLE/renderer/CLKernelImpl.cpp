//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLKernelImpl.cpp: Implements the class methods for CLKernelImpl.

#include "libANGLE/renderer/CLKernelImpl.h"

namespace rx
{

CLKernelImpl::WorkGroupInfo::WorkGroupInfo() = default;

CLKernelImpl::WorkGroupInfo::~WorkGroupInfo() = default;

CLKernelImpl::WorkGroupInfo::WorkGroupInfo(WorkGroupInfo &&) = default;

CLKernelImpl::WorkGroupInfo &CLKernelImpl::WorkGroupInfo::operator=(WorkGroupInfo &&) = default;

CLKernelImpl::ArgInfo::ArgInfo() = default;

CLKernelImpl::ArgInfo::~ArgInfo() = default;

CLKernelImpl::ArgInfo::ArgInfo(ArgInfo &&) = default;

CLKernelImpl::ArgInfo &CLKernelImpl::ArgInfo::operator=(ArgInfo &&) = default;

CLKernelImpl::Info::Info() = default;

CLKernelImpl::Info::~Info() = default;

CLKernelImpl::Info::Info(Info &&) = default;

CLKernelImpl::Info &CLKernelImpl::Info::operator=(Info &&) = default;

CLKernelImpl::CLKernelImpl(const cl::Kernel &kernel) : mKernel(kernel) {}

CLKernelImpl::~CLKernelImpl() = default;

}  // namespace rx
