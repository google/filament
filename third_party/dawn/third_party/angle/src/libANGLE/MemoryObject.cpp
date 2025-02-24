//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryObject.h: Implements the gl::MemoryObject class [EXT_external_objects]

#include "libANGLE/MemoryObject.h"

#include "common/angleutils.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/MemoryObjectImpl.h"

namespace gl
{

MemoryObject::MemoryObject(rx::GLImplFactory *factory, MemoryObjectID id)
    : RefCountObject(factory->generateSerial(), id),
      mImplementation(factory->createMemoryObject()),
      mImmutable(false),
      mDedicatedMemory(false),
      mProtectedMemory(false)
{}

MemoryObject::~MemoryObject() {}

void MemoryObject::onDestroy(const Context *context)
{
    mImplementation->onDestroy(context);
}

angle::Result MemoryObject::setDedicatedMemory(const Context *context, bool dedicatedMemory)
{
    ANGLE_TRY(mImplementation->setDedicatedMemory(context, dedicatedMemory));
    mDedicatedMemory = dedicatedMemory;
    return angle::Result::Continue;
}

angle::Result MemoryObject::setProtectedMemory(const Context *context, bool protectedMemory)
{
    ANGLE_TRY(mImplementation->setProtectedMemory(context, protectedMemory));
    mProtectedMemory = protectedMemory;
    return angle::Result::Continue;
}

angle::Result MemoryObject::importFd(Context *context,
                                     GLuint64 size,
                                     HandleType handleType,
                                     GLint fd)
{
    ANGLE_TRY(mImplementation->importFd(context, size, handleType, fd));
    mImmutable = true;
    return angle::Result::Continue;
}

angle::Result MemoryObject::importZirconHandle(Context *context,
                                               GLuint64 size,
                                               HandleType handleType,
                                               GLuint handle)
{
    ANGLE_TRY(mImplementation->importZirconHandle(context, size, handleType, handle));
    mImmutable = true;
    return angle::Result::Continue;
}

}  // namespace gl
