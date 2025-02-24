//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLDeviceImpl.cpp: Implements the class methods for CLDeviceImpl.

#include "libANGLE/renderer/CLDeviceImpl.h"

#include "libANGLE/Debug.h"

namespace rx
{

CLDeviceImpl::Info::Info() = default;

CLDeviceImpl::Info::Info(cl::DeviceType deviceType) : type(deviceType) {}

CLDeviceImpl::Info::~Info() = default;

CLDeviceImpl::Info::Info(Info &&) = default;

CLDeviceImpl::Info &CLDeviceImpl::Info::operator=(Info &&) = default;

CLDeviceImpl::CLDeviceImpl(const cl::Device &device) : mDevice(device) {}

CLDeviceImpl::~CLDeviceImpl() = default;

}  // namespace rx
