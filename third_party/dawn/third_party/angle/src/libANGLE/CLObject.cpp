//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLObject.cpp: Implements the cl::Object class.

#include "libANGLE/CLObject.h"

namespace cl
{

Object::Object() : mRefCount(1u) {}

Object::~Object()
{
    if (mRefCount != 0u)
    {
        WARN() << "Deleted object with references";
    }
}

}  // namespace cl
