//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLBuffer.cpp: Implements the cl::Buffer class.

#include "libANGLE/CLBuffer.h"

namespace cl
{

cl_mem Buffer::createSubBuffer(MemFlags flags,
                               cl_buffer_create_type createType,
                               const void *createInfo)
{
    const cl_buffer_region &region = *static_cast<const cl_buffer_region *>(createInfo);
    return Object::Create<Buffer>(*this, flags, region.origin, region.size);
}

Buffer::~Buffer() = default;

Buffer::Buffer(Context &context, PropArray &&properties, MemFlags flags, size_t size, void *hostPtr)
    : Memory(*this, context, std::move(properties), flags, size, hostPtr)
{}

Buffer::Buffer(Buffer &parent, MemFlags flags, size_t offset, size_t size)
    : Memory(*this, parent, flags, offset, size)
{}

}  // namespace cl
