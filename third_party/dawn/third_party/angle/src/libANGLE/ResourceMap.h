//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ResourceMap:
//   An optimized resource map which packs the first set of allocated objects into a
//   flat array, and then falls back to an unordered map for the higher handle values.
//

#ifndef LIBANGLE_RESOURCE_MAP_H_
#define LIBANGLE_RESOURCE_MAP_H_

#include <mutex>
#include <type_traits>

#include "common/SimpleMutex.h"
#include "common/hash_containers.h"
#include "libANGLE/angletypes.h"

namespace gl
{
// The resource map needs to be internally synchronized for maps that are placed in the share group
// (as opposed to being private to the context) and that are accessed without holding the share
// group lock.
#if defined(ANGLE_ENABLE_SHARE_CONTEXT_LOCK)
using ResourceMapMutex = angle::SimpleMutex;
#else
using ResourceMapMutex = angle::NoOpMutex;
#endif

template <bool NeedsLock = true>
struct SelectResourceMapMutex
{
    using type = ResourceMapMutex;
};

template <>
struct SelectResourceMapMutex<false>
{
    using type = angle::NoOpMutex;
};

// Analysis of ANGLE's traces as well as Chrome usage reveals the following:
//
// - Buffers: Typical applications use no more than 4000 ids.  Very few use over 6000.
// - Textures: Typical applications use no more than 1200 ids.  Very few use over 2000.
// - Samplers: Typical applications use no more than 50 ids.  Very few use over 100.
// - Shaders and Programs: Typical applications use no more than 500.  Very few use over 700.
// - Sync objects: Typical applications use no more than 500.  Very few use over 1500.
//
// For all the other shared types, the maximum used id is small (under 100).  For
// context-private parts (such as vertex arrays and queries), the id count can be in the
// thousands.
//
// The initial size of the flat resource map is based on the above, rounded up to a multiple of
// 1536.  Resource maps that need a lock (kNeedsLock == true) have the maximum flat size identical
// to initial flat size to avoid reallocation.  For others, the maps start small and can grow.
template <typename IDType>
struct ResourceMapParams
{
    static constexpr size_t kInitialFlatResourcesSize = 192;

    // The following are private to the context and don't need a lock:
    //
    // - Vertex Array Objects
    // - Framebuffer Objects
    // - Transform Feedback Objects
    // - Query Objects
    //
    // The rest of the maps need a lock.  However, only a select few are currently locked as API
    // relevant to the rest of the types are protected by the share group lock.  As the share group
    // lock is removed from more types, the resource map lock needs to be enabled for them.
    static constexpr bool kNeedsLock = false;
};
template <>
struct ResourceMapParams<BufferID>
{
    static constexpr size_t kInitialFlatResourcesSize = 6144;
    static constexpr bool kNeedsLock                  = true;
};
template <>
struct ResourceMapParams<TextureID>
{
    static constexpr size_t kInitialFlatResourcesSize = 1536;
    static constexpr bool kNeedsLock                  = false;
};
template <>
struct ResourceMapParams<ShaderProgramID>
{
    static constexpr size_t kInitialFlatResourcesSize = 1536;
    static constexpr bool kNeedsLock                  = false;
};
template <>
struct ResourceMapParams<SyncID>
{
    static constexpr size_t kInitialFlatResourcesSize = 1536;
    static constexpr bool kNeedsLock                  = false;
};
// For the purpose of unit testing, |int| is considered private (not needing lock), and
// |unsigned int| is considered shared (needing lock).
template <>
struct ResourceMapParams<unsigned int>
{
    static constexpr size_t kInitialFlatResourcesSize = 192;
    static constexpr bool kNeedsLock                  = true;
};

template <typename ResourceType, typename IDType>
class ResourceMap final : angle::NonCopyable
{
  public:
    ResourceMap();
    ~ResourceMap();

    ANGLE_INLINE ResourceType *query(IDType id) const
    {
        GLuint handle = GetIDValue(id);
        // No need for a lock when accessing the flat map.  Either the flat map is static, or
        // locking is not needed.
        static_assert(!kNeedsLock || kInitialFlatResourcesSize == kFlatResourcesLimit);

        if (ANGLE_LIKELY(handle < mFlatResourcesSize))
        {
            ResourceType *value = mFlatResources[handle];
            return (value == InvalidPointer() ? nullptr : value);
        }

        return findInHashedResources(handle);
    }

    // Returns true if the handle was reserved. Not necessarily if the resource is created.
    bool contains(IDType id) const;

    // Returns the element that was at this location.
    bool erase(IDType id, ResourceType **resourceOut);

    void assign(IDType id, ResourceType *resource);

    // Clears the map.
    void clear();

    using IndexAndResource = std::pair<GLuint, ResourceType *>;
    using HashMap          = angle::HashMap<GLuint, ResourceType *>;

    class Iterator final
    {
      public:
        bool operator==(const Iterator &other) const;
        bool operator!=(const Iterator &other) const;
        Iterator &operator++();
        const IndexAndResource *operator->() const;
        const IndexAndResource &operator*() const;

      private:
        friend class ResourceMap;
        Iterator(const ResourceMap &origin,
                 GLuint flatIndex,
                 typename HashMap::const_iterator hashIndex,
                 bool skipNulls);
        void updateValue();

        const ResourceMap &mOrigin;
        GLuint mFlatIndex;
        typename HashMap::const_iterator mHashIndex;
        IndexAndResource mValue;
        bool mSkipNulls;
    };

  private:
    friend class Iterator;
    template <typename SameResourceType, typename SameIDType>
    friend class UnsafeResourceMapIter;

    // null values represent reserved handles.
    Iterator begin() const;
    Iterator end() const;

    Iterator beginWithNull() const;
    Iterator endWithNull() const;

    // Used by iterators and related functions only (due to lack of thread safety).
    GLuint nextResource(size_t flatIndex, bool skipNulls) const;

    // constexpr methods cannot contain reinterpret_cast, so we need a static method.
    static ResourceType *InvalidPointer();
    static constexpr intptr_t kInvalidPointer = static_cast<intptr_t>(-1);

    static constexpr bool kNeedsLock = ResourceMapParams<IDType>::kNeedsLock;
    using Mutex                      = typename SelectResourceMapMutex<kNeedsLock>::type;

    static constexpr size_t kInitialFlatResourcesSize =
        ResourceMapParams<IDType>::kInitialFlatResourcesSize;

    // Experimental testing suggests that ~10k is a reasonable upper limit.
    static constexpr size_t kFlatResourcesLimit = kNeedsLock ? kInitialFlatResourcesSize : 0x3000;
    // Due to the way assign() is implemented, kFlatResourcesLimit / kInitialFlatResourcesSize must
    // be a power of 2.
    static_assert(kFlatResourcesLimit % kInitialFlatResourcesSize == 0);
    static_assert(((kFlatResourcesLimit / kInitialFlatResourcesSize) &
                   (kFlatResourcesLimit / kInitialFlatResourcesSize - 1)) == 0);

    bool containsInHashedResources(GLuint handle) const;
    ResourceType *findInHashedResources(GLuint handle) const;
    bool eraseFromHashedResources(GLuint handle, ResourceType **resourceOut);
    void assignAboveCurrentFlatSize(GLuint handle, ResourceType *resource);
    void assignInHashedResources(GLuint handle, ResourceType *resource);

    size_t mFlatResourcesSize;
    ResourceType **mFlatResources;

    // A map of GL objects indexed by object ID.
    HashMap mHashedResources;

    // mFlatResources is allocated at object creation time, with a default size of
    // |kInitialFlatResourcesSize|.  This is thread safe, because the allocation is done by the
    // first context in the share group.  The flat map is allowed to grow up to
    // |kFlatResourcesLimit|, but only for maps that don't need a lock (kNeedsLock == false).
    //
    // For maps that don't need a lock, this mutex is a no-op.  For those that do, the mutex is
    // taken when allocating / deleting objects, as well as when accessing |mHashedResources|.
    // Otherwise, access to the flat map (which never gets reallocated due to
    // |kInitialFlatResourcesSize == kFlatResourcesLimit|) is lockless.  This latter is possible
    // because the application is not allowed to gen/delete and bind the same ID in different
    // threads at the same time.
    //
    // Note that because HandleAllocator is not yet thread-safe, glGen* and glDelete* functions
    // cannot be free of the share group mutex yet.  To remove the share group mutex from those
    // functions, likely the HandleAllocator class should be merged with this class, and the
    // necessary insert/erase operations done under this same lock.
    mutable Mutex mMutex;
};

// A helper to retrieve the resource map iterators while being explicit that this is not thread
// safe.  Usage of iterators are limited to clean up on destruction and capture/replay, neither of
// which can race with other threads in their access to the resource map.
template <typename ResourceType, typename IDType>
class UnsafeResourceMapIter
{
  public:
    using ResMap = ResourceMap<ResourceType, IDType>;

    UnsafeResourceMapIter(const ResMap &resourceMap) : mResourceMap(resourceMap) {}

    typename ResMap::Iterator begin() const { return mResourceMap.begin(); }
    typename ResMap::Iterator end() const { return mResourceMap.end(); }

    typename ResMap::Iterator beginWithNull() const { return mResourceMap.beginWithNull(); }
    typename ResMap::Iterator endWithNull() const { return mResourceMap.endWithNull(); }

    // Not a constant-time operation, should only be used for verification.
    bool empty() const;

  private:
    const ResMap &mResourceMap;
};

template <typename ResourceType, typename IDType>
ResourceMap<ResourceType, IDType>::ResourceMap()
    : mFlatResourcesSize(kInitialFlatResourcesSize),
      mFlatResources(new ResourceType *[kInitialFlatResourcesSize])
{
    memset(mFlatResources, kInvalidPointer, mFlatResourcesSize * sizeof(mFlatResources[0]));
}

template <typename ResourceType, typename IDType>
ResourceMap<ResourceType, IDType>::~ResourceMap()
{
    ASSERT(begin() == end());
    delete[] mFlatResources;
}

template <typename ResourceType, typename IDType>
bool ResourceMap<ResourceType, IDType>::containsInHashedResources(GLuint handle) const
{
    std::lock_guard<Mutex> lock(mMutex);

    return mHashedResources.find(handle) != mHashedResources.end();
}

template <typename ResourceType, typename IDType>
ResourceType *ResourceMap<ResourceType, IDType>::findInHashedResources(GLuint handle) const
{
    std::lock_guard<Mutex> lock(mMutex);

    auto it = mHashedResources.find(handle);
    // Note: it->second can also be nullptr, so nullptr check doesn't work for "contains"
    return (it == mHashedResources.end() ? nullptr : it->second);
}

template <typename ResourceType, typename IDType>
bool ResourceMap<ResourceType, IDType>::eraseFromHashedResources(GLuint handle,
                                                                 ResourceType **resourceOut)
{
    std::lock_guard<Mutex> lock(mMutex);

    auto it = mHashedResources.find(handle);
    if (it == mHashedResources.end())
    {
        return false;
    }
    *resourceOut = it->second;
    mHashedResources.erase(it);
    return true;
}

template <typename ResourceType, typename IDType>
ANGLE_INLINE bool ResourceMap<ResourceType, IDType>::contains(IDType id) const
{
    GLuint handle = GetIDValue(id);
    if (ANGLE_LIKELY(handle < mFlatResourcesSize))
    {
        return mFlatResources[handle] != InvalidPointer();
    }

    return containsInHashedResources(handle);
}

template <typename ResourceType, typename IDType>
bool ResourceMap<ResourceType, IDType>::erase(IDType id, ResourceType **resourceOut)
{
    GLuint handle = GetIDValue(id);
    if (ANGLE_LIKELY(handle < mFlatResourcesSize))
    {
        auto &value = mFlatResources[handle];
        if (value == InvalidPointer())
        {
            return false;
        }
        *resourceOut = value;
        value        = InvalidPointer();
        return true;
    }

    return eraseFromHashedResources(handle, resourceOut);
}

template <typename ResourceType, typename IDType>
void ResourceMap<ResourceType, IDType>::assignAboveCurrentFlatSize(GLuint handle,
                                                                   ResourceType *resource)
{
    if (ANGLE_LIKELY(handle < kFlatResourcesLimit))
    {
        // No need for a lock as the flat map never grows when locking is needed.
        static_assert(!kNeedsLock || kInitialFlatResourcesSize == kFlatResourcesLimit);

        // Use power-of-two.
        size_t newSize = mFlatResourcesSize;
        while (newSize <= handle)
        {
            newSize *= 2;
        }

        ResourceType **oldResources = mFlatResources;

        mFlatResources = new ResourceType *[newSize];
        memset(&mFlatResources[mFlatResourcesSize], kInvalidPointer,
               (newSize - mFlatResourcesSize) * sizeof(mFlatResources[0]));
        memcpy(mFlatResources, oldResources, mFlatResourcesSize * sizeof(mFlatResources[0]));
        mFlatResourcesSize = newSize;
        ASSERT(mFlatResourcesSize <= kFlatResourcesLimit);
        delete[] oldResources;

        ASSERT(mFlatResourcesSize > handle);
        mFlatResources[handle] = resource;
    }
    else
    {
        std::lock_guard<Mutex> lock(mMutex);
        mHashedResources[handle] = resource;
    }
}

template <typename ResourceType, typename IDType>
ANGLE_INLINE void ResourceMap<ResourceType, IDType>::assign(IDType id, ResourceType *resource)
{
    GLuint handle = GetIDValue(id);
    if (ANGLE_LIKELY(handle < mFlatResourcesSize))
    {
        mFlatResources[handle] = resource;
    }
    else
    {
        assignAboveCurrentFlatSize(handle, resource);
    }
}

template <typename ResourceType, typename IDType>
typename ResourceMap<ResourceType, IDType>::Iterator ResourceMap<ResourceType, IDType>::begin()
    const
{
    return Iterator(*this, nextResource(0, true), mHashedResources.begin(), true);
}

template <typename ResourceType, typename IDType>
typename ResourceMap<ResourceType, IDType>::Iterator ResourceMap<ResourceType, IDType>::end() const
{
    return Iterator(*this, static_cast<GLuint>(mFlatResourcesSize), mHashedResources.end(), true);
}

template <typename ResourceType, typename IDType>
typename ResourceMap<ResourceType, IDType>::Iterator
ResourceMap<ResourceType, IDType>::beginWithNull() const
{
    return Iterator(*this, nextResource(0, false), mHashedResources.begin(), false);
}

template <typename ResourceType, typename IDType>
typename ResourceMap<ResourceType, IDType>::Iterator
ResourceMap<ResourceType, IDType>::endWithNull() const
{
    return Iterator(*this, static_cast<GLuint>(mFlatResourcesSize), mHashedResources.end(), false);
}

template <typename ResourceType, typename IDType>
bool UnsafeResourceMapIter<ResourceType, IDType>::empty() const
{
    return begin() == end();
}

template <typename ResourceType, typename IDType>
void ResourceMap<ResourceType, IDType>::clear()
{
    // No need for a lock as this is only called on destruction.
    memset(mFlatResources, kInvalidPointer, kInitialFlatResourcesSize * sizeof(mFlatResources[0]));
    mFlatResourcesSize = kInitialFlatResourcesSize;
    mHashedResources.clear();
}

template <typename ResourceType, typename IDType>
GLuint ResourceMap<ResourceType, IDType>::nextResource(size_t flatIndex, bool skipNulls) const
{
    // This function is only used by the iterators, access to which is marked by
    // UnsafeResourceMapIter.  Locking is the responsibility of the caller.
    for (size_t index = flatIndex; index < mFlatResourcesSize; index++)
    {
        if ((mFlatResources[index] != nullptr || !skipNulls) &&
            mFlatResources[index] != InvalidPointer())
        {
            return static_cast<GLuint>(index);
        }
    }
    return static_cast<GLuint>(mFlatResourcesSize);
}

template <typename ResourceType, typename IDType>
// static
ResourceType *ResourceMap<ResourceType, IDType>::InvalidPointer()
{
    return reinterpret_cast<ResourceType *>(kInvalidPointer);
}

template <typename ResourceType, typename IDType>
ResourceMap<ResourceType, IDType>::Iterator::Iterator(
    const ResourceMap &origin,
    GLuint flatIndex,
    typename ResourceMap<ResourceType, IDType>::HashMap::const_iterator hashIndex,
    bool skipNulls)
    : mOrigin(origin), mFlatIndex(flatIndex), mHashIndex(hashIndex), mSkipNulls(skipNulls)
{
    updateValue();
}

template <typename ResourceType, typename IDType>
bool ResourceMap<ResourceType, IDType>::Iterator::operator==(const Iterator &other) const
{
    return (mFlatIndex == other.mFlatIndex && mHashIndex == other.mHashIndex);
}

template <typename ResourceType, typename IDType>
bool ResourceMap<ResourceType, IDType>::Iterator::operator!=(const Iterator &other) const
{
    return !(*this == other);
}

template <typename ResourceType, typename IDType>
typename ResourceMap<ResourceType, IDType>::Iterator &
ResourceMap<ResourceType, IDType>::Iterator::operator++()
{
    if (mFlatIndex < static_cast<GLuint>(mOrigin.mFlatResourcesSize))
    {
        mFlatIndex = mOrigin.nextResource(mFlatIndex + 1, mSkipNulls);
    }
    else
    {
        mHashIndex++;
    }
    updateValue();
    return *this;
}

template <typename ResourceType, typename IDType>
const typename ResourceMap<ResourceType, IDType>::IndexAndResource *
ResourceMap<ResourceType, IDType>::Iterator::operator->() const
{
    return &mValue;
}

template <typename ResourceType, typename IDType>
const typename ResourceMap<ResourceType, IDType>::IndexAndResource &
ResourceMap<ResourceType, IDType>::Iterator::operator*() const
{
    return mValue;
}

template <typename ResourceType, typename IDType>
void ResourceMap<ResourceType, IDType>::Iterator::updateValue()
{
    if (mFlatIndex < static_cast<GLuint>(mOrigin.mFlatResourcesSize))
    {
        mValue.first  = mFlatIndex;
        mValue.second = mOrigin.mFlatResources[mFlatIndex];
    }
    else if (mHashIndex != mOrigin.mHashedResources.end())
    {
        mValue.first  = mHashIndex->first;
        mValue.second = mHashIndex->second;
    }
}

}  // namespace gl

#endif  // LIBANGLE_RESOURCE_MAP_H_
