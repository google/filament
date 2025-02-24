//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLMemory.h: Defines the abstract cl::Memory class, which is a memory object
// and the base class for OpenCL objects such as Buffer, Image and Pipe.

#ifndef LIBANGLE_CLMEMORY_H_
#define LIBANGLE_CLMEMORY_H_

#include "libANGLE/CLObject.h"
#include "libANGLE/renderer/CLMemoryImpl.h"

#include "common/SynchronizedValue.h"

#include <atomic>
#include <stack>

namespace cl
{

class Memory : public _cl_mem, public Object
{
  public:
    // Front end entry functions, only called from OpenCL entry points

    angle::Result setDestructorCallback(MemoryCB pfnNotify, void *userData);

    angle::Result getInfo(MemInfo name, size_t valueSize, void *value, size_t *valueSizeRet) const;

  public:
    using PropArray = std::vector<cl_mem_properties>;

    ~Memory() override;

    virtual MemObjectType getType() const = 0;

    const Context &getContext() const;
    const PropArray &getProperties() const;
    MemFlags getFlags() const;
    void *getHostPtr() const;
    const MemoryPtr &getParent() const;
    size_t getOffset() const;
    size_t getSize() const;

    template <typename T = rx::CLMemoryImpl>
    T &getImpl() const;

    static Memory *Cast(cl_mem memobj);

  protected:
    using CallbackData = std::pair<MemoryCB, void *>;

    Memory(const Buffer &buffer,
           Context &context,
           PropArray &&properties,
           MemFlags flags,
           size_t size,
           void *hostPtr);

    Memory(const Buffer &buffer, Buffer &parent, MemFlags flags, size_t offset, size_t size);

    Memory(Context &context, PropArray &&properties, MemFlags flags, Memory *parent, void *hostPtr);

    const ContextPtr mContext;
    const PropArray mProperties;
    const MemFlags mFlags;
    void *const mHostPtr = nullptr;
    const MemoryPtr mParent;
    const size_t mOffset = 0u;
    rx::CLMemoryImpl::Ptr mImpl;
    size_t mSize;

    angle::SynchronizedValue<std::stack<CallbackData>> mDestructorCallbacks;
    std::atomic<cl_uint> mMapCount;

    friend class Buffer;
    friend class Context;
};

inline const Context &Memory::getContext() const
{
    return *mContext;
}

inline const Memory::PropArray &Memory::getProperties() const
{
    return mProperties;
}

inline MemFlags Memory::getFlags() const
{
    return mFlags;
}

inline void *Memory::getHostPtr() const
{
    return mHostPtr;
}

inline const MemoryPtr &Memory::getParent() const
{
    return mParent;
}

inline size_t Memory::getOffset() const
{
    return mOffset;
}

inline size_t Memory::getSize() const
{
    return mSize;
}

template <typename T>
inline T &Memory::getImpl() const
{
    return static_cast<T &>(*mImpl);
}

inline Memory *Memory::Cast(cl_mem memobj)
{
    return static_cast<Memory *>(memobj);
}

}  // namespace cl

#endif  // LIBANGLE_CLMEMORY_H_
