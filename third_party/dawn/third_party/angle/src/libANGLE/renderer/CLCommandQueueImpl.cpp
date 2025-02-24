//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLCommandQueueImpl.cpp: Implements the class methods for CLCommandQueueImpl.

#include "libANGLE/renderer/CLCommandQueueImpl.h"

namespace rx
{

CLCommandQueueImpl::CLCommandQueueImpl(const cl::CommandQueue &commandQueue)
    : mCommandQueue(commandQueue)
{}

CLCommandQueueImpl::~CLCommandQueueImpl() = default;

}  // namespace rx
