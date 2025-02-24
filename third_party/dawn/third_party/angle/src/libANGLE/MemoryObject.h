//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryObject.h: Defines the gl::MemoryObject class [EXT_external_objects]

#ifndef LIBANGLE_MEMORYOBJECT_H_
#define LIBANGLE_MEMORYOBJECT_H_

#include "angle_gl.h"
#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class GLImplFactory;
class MemoryObjectImpl;
}  // namespace rx

namespace gl
{
class Context;

class MemoryObject final : public RefCountObject<MemoryObjectID>
{
  public:
    MemoryObject(rx::GLImplFactory *factory, MemoryObjectID id);
    ~MemoryObject() override;

    void onDestroy(const Context *context) override;

    rx::MemoryObjectImpl *getImplementation() const { return mImplementation.get(); }

    bool isImmutable() const { return mImmutable; }

    angle::Result setDedicatedMemory(const Context *context, bool dedicatedMemory);
    bool isDedicatedMemory() const { return mDedicatedMemory; }
    angle::Result setProtectedMemory(const Context *context, bool protectedMemory);
    bool isProtectedMemory() const { return mProtectedMemory; }

    angle::Result importFd(Context *context, GLuint64 size, HandleType handleType, GLint fd);
    angle::Result importZirconHandle(Context *context,
                                     GLuint64 size,
                                     HandleType handleType,
                                     GLuint handle);

  private:
    std::unique_ptr<rx::MemoryObjectImpl> mImplementation;

    bool mImmutable;
    bool mDedicatedMemory;
    bool mProtectedMemory;
};

}  // namespace gl

#endif  // LIBANGLE_MEMORYOBJECT_H_
