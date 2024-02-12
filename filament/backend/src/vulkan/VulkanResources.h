/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_VULKANRESOURCES_H
#define TNT_FILAMENT_BACKEND_VULKANRESOURCES_H

#include <backend/Handle.h>

#include <tsl/robin_set.h>
#include <utils/Mutex.h>
#include <utils/Panic.h>

#include <mutex>
#include <unordered_set>

namespace filament::backend {

class VulkanResourceAllocator;
struct VulkanThreadSafeResource;

// Subclasses of VulkanResource must provide this enum in their construction.
enum class VulkanResourceType : uint8_t {
    BUFFER_OBJECT,
    INDEX_BUFFER,
    PROGRAM,
    RENDER_TARGET,
    SAMPLER_GROUP,
    SWAP_CHAIN,
    RENDER_PRIMITIVE,
    TEXTURE,
    TIMER_QUERY,
    VERTEX_BUFFER,

    // Below are resources that are managed manually (i.e. not ref counted).
    FENCE,
    HEAP_ALLOCATED,
};

#define IS_HEAP_ALLOC_TYPE(f)                                                                      \
    (f == VulkanResourceType::FENCE || f == VulkanResourceType::HEAP_ALLOCATED)


// This is a ref-counting base class that tracks how many references of this resource exist. This
// class is paired with VulkanResourceManagerImpl which is responsible for incrementing or
// decrementing the count. Once mRefCount == 0, VulkanResourceManagerImpl will also call the
// appropriate destructor. VulkanCommandBuffer, VulkanDriver, and composite structure like
// VulkanRenderPrimitive are owners of VulkanResourceManagerImpl instances.
struct VulkanResourceBase {
protected:
    explicit VulkanResourceBase(VulkanResourceType type)
        : mRefCount(IS_HEAP_ALLOC_TYPE(type) ? 1 : 0),
          mType(type),
          mHandleId(0) {
    }

private:
    inline VulkanResourceType getType() {
        return mType;
    }

    inline HandleBase::HandleId getId() {
        return mHandleId;
    }

    inline void initResource(HandleBase::HandleId id) noexcept {
        mHandleId = id;
    }

    inline void ref() noexcept {
        if (IS_HEAP_ALLOC_TYPE(mType)) {
            return;
        }
        assert_invariant(mRefCount < ((1<<24) - 1));
        ++mRefCount;
    }

    inline void deref() noexcept {
        if (IS_HEAP_ALLOC_TYPE(mType)) {
            return;
        }
        assert_invariant(mRefCount > 0);
        --mRefCount;
    }

    inline size_t refcount() noexcept {
        return mRefCount;
    }

    uint32_t mRefCount : 24; // 16M is enough for the refcount
    VulkanResourceType mType : 8;
    HandleBase::HandleId mHandleId;

    friend struct VulkanThreadSafeResource;
    friend class VulkanResourceAllocator;

    template<typename RT, typename ST>
    friend class VulkanResourceManagerImpl;
};

struct VulkanThreadSafeResource {
protected:
    explicit VulkanThreadSafeResource(VulkanResourceType type)
        : mImpl(type) {}

private:
    inline VulkanResourceType getType() {
        return mImpl.getType();
    }

    inline HandleBase::HandleId getId() {
        return mImpl.getId();
    }

    inline void initResource(HandleBase::HandleId id) noexcept {
        std::unique_lock<utils::Mutex> lock(mMutex);
        mImpl.initResource(id);
    }

    inline void ref() noexcept {
        std::unique_lock<utils::Mutex> lock(mMutex);
        mImpl.ref();
    }

    inline void deref() noexcept {
        std::unique_lock<utils::Mutex> lock(mMutex);
        mImpl.deref();
    }

    inline size_t refcount() noexcept {
        std::unique_lock<utils::Mutex> lock(mMutex);
        return mImpl.refcount();
    }

    utils::Mutex mMutex;
    VulkanResourceBase mImpl;

    friend class VulkanResourceAllocator;
    template<typename RT, typename ST>
    friend class VulkanResourceManagerImpl;
};

using VulkanResource = VulkanResourceBase;

namespace {

// When the size of the resource set is known to be small, (for example for VulkanRenderPrimitive),
// we just use a std::array to back the set.
template<std::size_t SIZE>
class FixedCapacityResourceSet {
private:
    using FixedSizeArray = std::array<VulkanResource*, SIZE>;

public:
    using const_iterator = typename FixedSizeArray::const_iterator;

    inline ~FixedCapacityResourceSet() {
        clear();
    }

    inline const_iterator begin() {
        if (mInd == 0) {
            return mArray.cend();
        }
        return mArray.cbegin();
    }

    inline const_iterator end() {
        if (mInd == 0) {
            return mArray.cend();
        }
        if (mInd < SIZE) {
            return mArray.begin() + mInd;
        }
        return mArray.cend();
    }

    inline const_iterator find(VulkanResource* resource) {
        return std::find(begin(), end(), resource);
    }

    inline void insert(VulkanResource* resource) {
        assert_invariant(mInd < SIZE);
        mArray[mInd++] = resource;
    }

    inline void erase(VulkanResource* resource) {
        assert_invariant(false && "FixedCapacityResourceSet::erase should not be called");
    }

    inline void clear() {
        if (mInd == 0) {
            return;
        }
        mInd = 0;
    }

    inline size_t size() {
        return mInd;
    }

private:
    FixedSizeArray mArray{nullptr};
    size_t mInd = 0;
};

// robin_set/map are useful for sets that are acquire only and the set will be iterated when the set
// is cleared.
using FastIterationResourceSet = tsl::robin_set<VulkanResource*>;

// unoredered_set is used in the general case where insert/erase can occur at will. This is useful
// for the basic object ownership count - i.e. VulkanDriver.
using ResourceSet = std::unordered_set<VulkanResource*>;

using ThreadSafeResourceSet = std::unordered_set<VulkanThreadSafeResource*>;

} // anonymous namespace

class VulkanResourceAllocator;

#define LOCK_IF_NEEDED()                                                                           \
    if constexpr (std::is_base_of_v<VulkanThreadSafeResource, ResourceType>) {                     \
        mMutex->lock();                                                                            \
    }

#define UNLOCK_IF_NEEDED()                                                                         \
    if constexpr (std::is_base_of_v<VulkanThreadSafeResource, ResourceType>) {                     \
        mMutex->unlock();                                                                          \
    }

void deallocateResource(VulkanResourceAllocator* allocator, VulkanResourceType type,
        HandleBase::HandleId id);

template<typename ResourceType, typename SetType>
class VulkanResourceManagerImpl {
public:
    explicit VulkanResourceManagerImpl(VulkanResourceAllocator* allocator)
        : mAllocator(allocator) {
        if constexpr (std::is_base_of_v<VulkanThreadSafeResource, ResourceType>) {
            mMutex = std::make_unique<utils::Mutex>();
        }
    }

    VulkanResourceManagerImpl(const VulkanResourceManagerImpl& other) = delete;
    void operator=(const VulkanResourceManagerImpl& other) = delete;
    VulkanResourceManagerImpl(const VulkanResourceManagerImpl&& other) = delete;
    void operator=(const VulkanResourceManagerImpl&& other) = delete;

    ~VulkanResourceManagerImpl() {
        clear();
    }

    inline void acquire(ResourceType* resource) {
        if (IS_HEAP_ALLOC_TYPE(resource->getType())) {
            return;
        }

        LOCK_IF_NEEDED();
        if (mResources.find(resource) != mResources.end()) {
            UNLOCK_IF_NEEDED();
            return;
        }
        mResources.insert(resource);
        UNLOCK_IF_NEEDED();
        resource->ref();
    }

    // Transfers ownership from one resource set to another
    template <typename tSetType>
    inline void acquireAll(VulkanResourceManagerImpl<ResourceType, tSetType>* srcResources) {
        copyAll(srcResources);
        srcResources->clear();
    }

    // Transfers ownership from one resource set to another
    template <typename tSetType>
    inline void copyAll(VulkanResourceManagerImpl<ResourceType, tSetType>* srcResources) {
        LOCK_IF_NEEDED();
        for (auto iter = srcResources->mResources.begin(); iter != srcResources->mResources.end();
                iter++) {
            acquire(*iter);
        }
        UNLOCK_IF_NEEDED();
    }

    inline void release(ResourceType* resource) {
        if (IS_HEAP_ALLOC_TYPE(resource->getType())) {
            return;
        }

        LOCK_IF_NEEDED();
        auto resItr = mResources.find(resource);
        if (resItr == mResources.end()) {
            UNLOCK_IF_NEEDED();
            return;
        }
        mResources.erase(resItr);
        UNLOCK_IF_NEEDED();
        derefImpl(resource);
    }

    inline void clear() {
        LOCK_IF_NEEDED();
        for (auto iter = mResources.begin(); iter != mResources.end(); iter++) {
            derefImpl(*iter);
        }
        mResources.clear();
        UNLOCK_IF_NEEDED();
    }

    inline size_t size() {
        return mResources.size();
    }

private:
    inline void derefImpl(ResourceType* resource) {
        resource->deref();
        if (resource->refcount() != 0) {
            return;
        }
        deallocateResource(mAllocator, resource->getType(), resource->getId());
    }

    VulkanResourceAllocator* mAllocator;
    SetType mResources;
    std::unique_ptr<utils::Mutex> mMutex;

    template <typename, typename> friend class VulkanResourceManagerImpl;
};

using VulkanAcquireOnlyResourceManager
        = VulkanResourceManagerImpl<VulkanResource, FastIterationResourceSet>;
using VulkanResourceManager = VulkanResourceManagerImpl<VulkanResource, ResourceSet>;

template<std::size_t SIZE>
using FixedSizeVulkanResourceManager =
        VulkanResourceManagerImpl<VulkanResource, FixedCapacityResourceSet<SIZE>>;

using VulkanThreadSafeResourceManager
        = VulkanResourceManagerImpl<VulkanThreadSafeResource, ThreadSafeResourceSet>;

#undef LOCK_IF_NEEDED
#undef UNLOCK_IF_NEEDED

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANRESOURCES_H
