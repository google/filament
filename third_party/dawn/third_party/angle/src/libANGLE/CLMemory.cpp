//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLMemory.cpp: Implements the cl::Memory class.

#include "libANGLE/CLMemory.h"

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/cl_utils.h"

#include <cstring>

namespace cl
{

namespace
{

MemFlags InheritMemFlags(MemFlags flags, Memory *parent)
{
    if (parent != nullptr)
    {
        const MemFlags parentFlags = parent->getFlags();
        const MemFlags access(CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY);
        const MemFlags hostAccess(CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY |
                                  CL_MEM_HOST_NO_ACCESS);
        const MemFlags hostPtrFlags(CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR |
                                    CL_MEM_COPY_HOST_PTR);
        if (flags.excludes(access))
        {
            flags.set(parentFlags.mask(access));
        }
        if (flags.excludes(hostAccess))
        {
            flags.set(parentFlags.mask(hostAccess));
        }
        flags.set(parentFlags.mask(hostPtrFlags));
    }
    return flags;
}

}  // namespace

angle::Result Memory::setDestructorCallback(MemoryCB pfnNotify, void *userData)
{
    mDestructorCallbacks->emplace(pfnNotify, userData);
    return angle::Result::Continue;
}

angle::Result Memory::getInfo(MemInfo name,
                              size_t valueSize,
                              void *value,
                              size_t *valueSizeRet) const
{
    static_assert(
        std::is_same<cl_uint, cl_bool>::value && std::is_same<cl_uint, cl_mem_object_type>::value,
        "OpenCL type mismatch");

    cl_uint valUInt       = 0u;
    void *valPointer      = nullptr;
    const void *copyValue = nullptr;
    size_t copySize       = 0u;

    switch (name)
    {
        case MemInfo::Type:
            valUInt   = ToCLenum(getType());
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case MemInfo::Flags:
            copyValue = &mFlags;
            copySize  = sizeof(mFlags);
            break;
        case MemInfo::Size:
            copyValue = &mSize;
            copySize  = sizeof(mSize);
            break;
        case MemInfo::HostPtr:
            copyValue = &mHostPtr;
            copySize  = sizeof(mHostPtr);
            break;
        case MemInfo::MapCount:
            valUInt   = mMapCount;
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case MemInfo::ReferenceCount:
            valUInt   = getRefCount();
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case MemInfo::Context:
            valPointer = mContext->getNative();
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case MemInfo::AssociatedMemObject:
            valPointer = Memory::CastNative(mParent.get());
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case MemInfo::Offset:
            copyValue = &mOffset;
            copySize  = sizeof(mOffset);
            break;
        case MemInfo::UsesSVM_Pointer:
            valUInt   = CL_FALSE;  // TODO(jplate) Check for SVM pointer anglebug.com/42264535
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case MemInfo::Properties:
            copyValue = mProperties.data();
            copySize  = mProperties.size() * sizeof(decltype(mProperties)::value_type);
            break;
        default:
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
        // as described in the Memory Object Info table and param_value is not NULL.
        if (valueSize < copySize)
        {
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
        }
        if (copyValue != nullptr)
        {
            std::memcpy(value, copyValue, copySize);
        }
    }
    if (valueSizeRet != nullptr)
    {
        *valueSizeRet = copySize;
    }
    return angle::Result::Continue;
}

Memory::~Memory()
{
    std::stack<CallbackData> callbacks;
    mDestructorCallbacks->swap(callbacks);
    while (!callbacks.empty())
    {
        const MemoryCB callback = callbacks.top().first;
        void *const userData    = callbacks.top().second;
        callbacks.pop();
        callback(this, userData);
    }
}

Memory::Memory(const Buffer &buffer,
               Context &context,
               PropArray &&properties,
               MemFlags flags,
               size_t size,
               void *hostPtr)
    : mContext(&context),
      mProperties(std::move(properties)),
      mFlags(flags.excludes(CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)
                 ? cl::MemFlags{flags.get() | CL_MEM_READ_WRITE}
                 : flags),
      mHostPtr(flags.intersects(CL_MEM_USE_HOST_PTR) ? hostPtr : nullptr),
      mImpl(nullptr),
      mSize(size),
      mMapCount(0u)
{
    ANGLE_CL_IMPL_TRY(context.getImpl().createBuffer(buffer, hostPtr, &mImpl));
}

Memory::Memory(const Buffer &buffer, Buffer &parent, MemFlags flags, size_t offset, size_t size)
    : mContext(parent.mContext),
      mFlags(InheritMemFlags(flags, &parent)),
      mHostPtr(parent.mHostPtr != nullptr ? static_cast<char *>(parent.mHostPtr) + offset
                                          : nullptr),
      mParent(&parent),
      mOffset(offset),
      mImpl(nullptr),
      mSize(size),
      mMapCount(0u)
{
    ANGLE_CL_IMPL_TRY(parent.mImpl->createSubBuffer(buffer, flags, size, &mImpl));
}

Memory::Memory(Context &context,
               PropArray &&properties,
               MemFlags flags,
               Memory *parent,
               void *hostPtr)
    : mContext(&context),
      mProperties(std::move(properties)),
      mFlags(InheritMemFlags(flags, parent)),
      mHostPtr(flags.intersects(CL_MEM_USE_HOST_PTR) ? hostPtr : nullptr),
      mParent(parent),
      mImpl(nullptr),
      mSize(0u),
      mMapCount(0u)
{}

}  // namespace cl
