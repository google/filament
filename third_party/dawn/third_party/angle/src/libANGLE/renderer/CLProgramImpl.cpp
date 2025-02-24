//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLProgramImpl.cpp: Implements the class methods for CLProgramImpl.

#include "libANGLE/renderer/CLProgramImpl.h"

namespace rx
{

CLProgramImpl::CLProgramImpl(const cl::Program &program) : mProgram(program) {}

CLProgramImpl::~CLProgramImpl() = default;

}  // namespace rx
