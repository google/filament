//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLSamplerImpl.cpp: Implements the class methods for CLSamplerImpl.

#include "libANGLE/renderer/CLSamplerImpl.h"

namespace rx
{

CLSamplerImpl::CLSamplerImpl(const cl::Sampler &sampler) : mSampler(sampler) {}

CLSamplerImpl::~CLSamplerImpl() = default;

}  // namespace rx
