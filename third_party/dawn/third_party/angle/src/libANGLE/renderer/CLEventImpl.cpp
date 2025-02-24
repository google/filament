//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLEventImpl.cpp: Implements the class methods for CLEventImpl.

#include "libANGLE/renderer/CLEventImpl.h"

namespace rx
{

CLEventImpl::CLEventImpl(const cl::Event &event) : mEvent(event) {}

CLEventImpl::~CLEventImpl() = default;

}  // namespace rx
