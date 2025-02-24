//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLMemoryImpl.cpp: Implements the class methods for CLMemoryImpl.

#include "libANGLE/renderer/CLMemoryImpl.h"

namespace rx
{

CLMemoryImpl::CLMemoryImpl(const cl::Memory &memory) : mMemory(memory) {}

CLMemoryImpl::~CLMemoryImpl() = default;

}  // namespace rx
