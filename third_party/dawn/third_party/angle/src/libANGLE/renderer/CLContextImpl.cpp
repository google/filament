//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLContextImpl.cpp: Implements the class methods for CLContextImpl.

#include "libANGLE/renderer/CLContextImpl.h"

namespace rx
{

CLContextImpl::CLContextImpl(const cl::Context &context) : mContext(context) {}

CLContextImpl::~CLContextImpl() = default;

}  // namespace rx
