//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Buffer9.cpp Defines the Buffer9 class.

#include "libANGLE/renderer/d3d/d3d9/Buffer9.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"

namespace rx
{

Buffer9::Buffer9(const gl::BufferState &state, Renderer9 *renderer)
    : BufferD3D(state, renderer), mSize(0)
{}

Buffer9::~Buffer9()
{
    mSize = 0;
}

size_t Buffer9::getSize() const
{
    return mSize;
}

bool Buffer9::supportsDirectBinding() const
{
    return false;
}

angle::Result Buffer9::setData(const gl::Context *context,
                               gl::BufferBinding target,
                               const void *data,
                               size_t size,
                               gl::BufferUsage usage)
{
    if (size > mMemory.size())
    {
        ANGLE_CHECK_GL_ALLOC(GetImplAs<Context9>(context), mMemory.resize(size));
    }

    mSize = size;
    if (data && size > 0)
    {
        memcpy(mMemory.data(), data, size);
    }

    updateD3DBufferUsage(context, usage);

    invalidateStaticData(context);

    return angle::Result::Continue;
}

angle::Result Buffer9::getData(const gl::Context *context, const uint8_t **outData)
{
    if (mMemory.empty())
    {
        *outData = nullptr;
    }
    else
    {
        *outData = mMemory.data();
    }
    return angle::Result::Continue;
}

angle::Result Buffer9::setSubData(const gl::Context *context,
                                  gl::BufferBinding target,
                                  const void *data,
                                  size_t size,
                                  size_t offset)
{
    if (offset + size > mMemory.size())
    {
        ANGLE_CHECK_GL_ALLOC(GetImplAs<Context9>(context), mMemory.resize(size + offset));
    }

    mSize = std::max(mSize, offset + size);
    if (data && size > 0)
    {
        memcpy(mMemory.data() + offset, data, size);
    }

    invalidateStaticData(context);

    return angle::Result::Continue;
}

angle::Result Buffer9::copySubData(const gl::Context *context,
                                   BufferImpl *source,
                                   GLintptr sourceOffset,
                                   GLintptr destOffset,
                                   GLsizeiptr size)
{
    // Note: this method is currently unreachable
    Buffer9 *sourceBuffer = GetAs<Buffer9>(source);
    ASSERT(sourceBuffer);

    memcpy(mMemory.data() + destOffset, sourceBuffer->mMemory.data() + sourceOffset, size);

    invalidateStaticData(context);

    return angle::Result::Continue;
}

// We do not support buffer mapping in D3D9
angle::Result Buffer9::map(const gl::Context *context, GLenum access, void **mapPtr)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Buffer9::mapRange(const gl::Context *context,
                                size_t offset,
                                size_t length,
                                GLbitfield access,
                                void **mapPtr)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Buffer9::unmap(const gl::Context *context, GLboolean *result)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Buffer9::markTransformFeedbackUsage(const gl::Context *context)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

}  // namespace rx
