//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLPlatformImpl.cpp: Implements the class methods for CLPlatformImpl.

#include "libANGLE/renderer/CLPlatformImpl.h"

namespace rx
{

CLPlatformImpl::Info::Info() = default;

CLPlatformImpl::Info::~Info() = default;

CLPlatformImpl::Info::Info(Info &&) = default;

CLPlatformImpl::Info &CLPlatformImpl::Info::operator=(Info &&) = default;

CLPlatformImpl::CLPlatformImpl(const cl::Platform &platform) : mPlatform(platform) {}

CLPlatformImpl::~CLPlatformImpl() = default;

}  // namespace rx
