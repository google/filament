//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Resource:
//    Resource lifetime tracking in the Vulkan back-end.
//

#ifndef LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_
#define LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_

#include "common/FixedQueue.h"
#include "common/SimpleMutex.h"
#include "libANGLE/HandleAllocator.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

#include <queue>

namespace rx
{
namespace vk
{
// We expect almost all reasonable usage case should have at most 4 current contexts now. When
// exceeded, it should still work, but storage will grow.
static constexpr size_t kMaxFastQueueSerials = 4;
// Serials is an array of queue serials, which when paired with the index of the serials in the
// array result in QueueSerials. The array may expand if needed. Since it owned by Resource object
// which is protected by shared lock, it is safe to reallocate storage if needed. When it passes to
// renderer at garbage collection time, we will make a copy. The array size is expected to be small.
// But in future if we run into situation that array size is too big, we can change to packed array
// of QueueSerials.
using Serials = angle::FastVector<Serial, kMaxFastQueueSerials>;

// Tracks how a resource is used by ANGLE and by a VkQueue. The serial indicates the most recent use
// of a resource in the VkQueue. We use the monotonically incrementing serial number to determine if
// a resource is currently in use.
class ResourceUse final
{
  public:
    ResourceUse()  = default;
    ~ResourceUse() = default;

    ResourceUse(const QueueSerial &queueSerial) { setQueueSerial(queueSerial); }
    ResourceUse(const Serials &otherSerials) { mSerials = otherSerials; }

    // Copy constructor
    ResourceUse(const ResourceUse &other) : mSerials(other.mSerials) {}
    ResourceUse &operator=(const ResourceUse &other)
    {
        mSerials = other.mSerials;
        return *this;
    }

    // Move constructor
    ResourceUse(ResourceUse &&other) : mSerials(other.mSerials) { other.mSerials.clear(); }
    ResourceUse &operator=(ResourceUse &&other)
    {
        mSerials = other.mSerials;
        other.mSerials.clear();
        return *this;
    }

    bool valid() const { return mSerials.size() > 0; }

    void reset() { mSerials.clear(); }

    const Serials &getSerials() const { return mSerials; }

    void setSerial(SerialIndex index, Serial serial)
    {
        ASSERT(index != kInvalidQueueSerialIndex);
        if (ANGLE_UNLIKELY(mSerials.size() <= index))
        {
            mSerials.resize(index + 1, kZeroSerial);
        }
        ASSERT(mSerials[index] <= serial);
        mSerials[index] = serial;
    }

    void setQueueSerial(const QueueSerial &queueSerial)
    {
        setSerial(queueSerial.getIndex(), queueSerial.getSerial());
    }

    // Returns true if there is at least one serial is greater than
    bool operator>(const AtomicQueueSerialFixedArray &serials) const
    {
        ASSERT(mSerials.size() <= serials.size());
        for (SerialIndex i = 0; i < mSerials.size(); ++i)
        {
            if (mSerials[i] > serials[i])
            {
                return true;
            }
        }
        return false;
    }

    // Compare the recorded serial with the parameter
    bool operator>(const QueueSerial &queuSerial) const
    {
        return mSerials.size() > queuSerial.getIndex() &&
               mSerials[queuSerial.getIndex()] > queuSerial.getSerial();
    }
    bool operator>=(const QueueSerial &queueSerial) const
    {
        return mSerials.size() > queueSerial.getIndex() &&
               mSerials[queueSerial.getIndex()] >= queueSerial.getSerial();
    }

    // Returns true if all serials are less than or equal
    bool operator<=(const AtomicQueueSerialFixedArray &serials) const
    {
        ASSERT(mSerials.size() <= serials.size());
        for (SerialIndex i = 0; i < mSerials.size(); ++i)
        {
            if (mSerials[i] > serials[i])
            {
                return false;
            }
        }
        return true;
    }

    bool usedByCommandBuffer(const QueueSerial &commandBufferQueueSerial) const
    {
        ASSERT(commandBufferQueueSerial.valid());
        // Return true if we have the exact queue serial in the array.
        return mSerials.size() > commandBufferQueueSerial.getIndex() &&
               mSerials[commandBufferQueueSerial.getIndex()] ==
                   commandBufferQueueSerial.getSerial();
    }

    // Merge other's serials into this object.
    void merge(const ResourceUse &other)
    {
        if (mSerials.size() < other.mSerials.size())
        {
            mSerials.resize(other.mSerials.size(), kZeroSerial);
        }

        for (SerialIndex i = 0; i < other.mSerials.size(); ++i)
        {
            if (mSerials[i] < other.mSerials[i])
            {
                mSerials[i] = other.mSerials[i];
            }
        }
    }

  private:
    // The most recent time of use in a VkQueue.
    Serials mSerials;
};
std::ostream &operator<<(std::ostream &os, const ResourceUse &use);

class SharedGarbage final : angle::NonCopyable
{
  public:
    SharedGarbage();
    SharedGarbage(SharedGarbage &&other);
    SharedGarbage(const ResourceUse &use, GarbageObjects &&garbage);
    ~SharedGarbage();
    SharedGarbage &operator=(SharedGarbage &&rhs);

    bool destroyIfComplete(Renderer *renderer);
    bool hasResourceUseSubmitted(Renderer *renderer) const;
    // This is not being used now.
    VkDeviceSize getSize() const { return 0; }

  private:
    ResourceUse mLifetime;
    GarbageObjects mGarbage;
};

// SharedGarbageList list tracks garbage using angle::FixedQueue. It allows concurrent add (i.e.,
// enqueue) and cleanup (i.e. dequeue) operations from two threads. Add call from two threads are
// synchronized using a mutex and cleanup call from two threads are synchronized with a separate
// mutex.
template <class T>
class SharedGarbageList final : angle::NonCopyable
{
  public:
    SharedGarbageList()
        : mSubmittedQueue(kInitialQueueCapacity),
          mUnsubmittedQueue(kInitialQueueCapacity),
          mTotalSubmittedGarbageBytes(0),
          mTotalUnsubmittedGarbageBytes(0),
          mTotalGarbageDestroyed(0)
    {}
    ~SharedGarbageList()
    {
        ASSERT(mSubmittedQueue.empty());
        ASSERT(mUnsubmittedQueue.empty());
    }

    void add(Renderer *renderer, T &&garbage)
    {
        VkDeviceSize size = garbage.getSize();
        if (garbage.destroyIfComplete(renderer))
        {
            mTotalGarbageDestroyed += size;
        }
        else
        {
            std::unique_lock<angle::SimpleMutex> enqueueLock(mMutex);
            if (garbage.hasResourceUseSubmitted(renderer))
            {
                addGarbageLocked(mSubmittedQueue, std::move(garbage));
                mTotalSubmittedGarbageBytes += size;
            }
            else
            {
                addGarbageLocked(mUnsubmittedQueue, std::move(garbage));
                // We use relaxed ordering here since it is always modified with mMutex. The atomic
                // is only for the purpose of make tsan happy.
                mTotalUnsubmittedGarbageBytes.fetch_add(size, std::memory_order_relaxed);
            }
        }
    }

    bool empty() const { return mSubmittedQueue.empty() && mUnsubmittedQueue.empty(); }
    VkDeviceSize getSubmittedGarbageSize() const
    {
        return mTotalSubmittedGarbageBytes.load(std::memory_order_consume);
    }
    VkDeviceSize getUnsubmittedGarbageSize() const
    {
        return mTotalUnsubmittedGarbageBytes.load(std::memory_order_consume);
    }
    VkDeviceSize getDestroyedGarbageSize() const
    {
        return mTotalGarbageDestroyed.load(std::memory_order_consume);
    }
    void resetDestroyedGarbageSize() { mTotalGarbageDestroyed = 0; }

    // Number of bytes destroyed is returned.
    VkDeviceSize cleanupSubmittedGarbage(Renderer *renderer)
    {
        std::unique_lock<angle::SimpleMutex> lock(mSubmittedQueueDequeueMutex);
        VkDeviceSize bytesDestroyed = 0;
        while (!mSubmittedQueue.empty())
        {
            T &garbage        = mSubmittedQueue.front();
            VkDeviceSize size = garbage.getSize();
            if (!garbage.destroyIfComplete(renderer))
            {
                break;
            }
            bytesDestroyed += size;
            mSubmittedQueue.pop();
        }
        mTotalSubmittedGarbageBytes -= bytesDestroyed;
        mTotalGarbageDestroyed += bytesDestroyed;
        return bytesDestroyed;
    }

    // Check if pending garbage is still pending submission. If not, move them to the garbage list.
    // Otherwise move the element to the end of the queue. Note that this call took both locks of
    // this list. Since this call is only used for pending submission garbage list and that list
    // only temporary stores garbage, it does not destroy garbage in this list. And moving garbage
    // around is expected to be cheap in general, so lock contention is not expected.
    void cleanupUnsubmittedGarbage(Renderer *renderer)
    {
        std::unique_lock<angle::SimpleMutex> enqueueLock(mMutex);
        size_t count            = mUnsubmittedQueue.size();
        VkDeviceSize bytesMoved = 0;
        for (size_t i = 0; i < count; i++)
        {
            T &garbage = mUnsubmittedQueue.front();
            if (garbage.hasResourceUseSubmitted(renderer))
            {
                bytesMoved += garbage.getSize();
                addGarbageLocked(mSubmittedQueue, std::move(garbage));
            }
            else
            {
                mUnsubmittedQueue.push(std::move(garbage));
            }
            mUnsubmittedQueue.pop();
        }
        mTotalUnsubmittedGarbageBytes -= bytesMoved;
        mTotalSubmittedGarbageBytes += bytesMoved;
    }

  private:
    void addGarbageLocked(angle::FixedQueue<T> &queue, T &&garbage)
    {
        // Expand the queue storage if we only have one empty space left. That one empty space is
        // required by cleanupPendingSubmissionGarbage so that we do not need to allocate another
        // temporary storage.
        if (queue.size() >= queue.capacity() - 1)
        {
            std::unique_lock<angle::SimpleMutex> dequeueLock(mSubmittedQueueDequeueMutex);
            size_t newCapacity = queue.capacity() << 1;
            queue.updateCapacity(newCapacity);
        }
        queue.push(std::move(garbage));
    }

    static constexpr size_t kInitialQueueCapacity = 64;
    // Protects both enqueue and dequeue of mUnsubmittedQueue, as well as enqueue of
    // mSubmittedQueue.
    angle::SimpleMutex mMutex;
    // Protect dequeue of mSubmittedQueue, which is expected to be more expensive.
    angle::SimpleMutex mSubmittedQueueDequeueMutex;
    // Holds garbage that all of use has been submitted to renderer.
    angle::FixedQueue<T> mSubmittedQueue;
    // Holds garbage with at least one of the queueSerials has not yet submitted to renderer.
    angle::FixedQueue<T> mUnsubmittedQueue;
    // Total bytes of garbage in mSubmittedQueue.
    std::atomic<VkDeviceSize> mTotalSubmittedGarbageBytes;
    // Total bytes of garbage in mUnsubmittedQueue.
    std::atomic<VkDeviceSize> mTotalUnsubmittedGarbageBytes;
    // Total bytes of garbage been destroyed since last resetDestroyedGarbageSize call.
    std::atomic<VkDeviceSize> mTotalGarbageDestroyed;
};

// This is a helper class for back-end objects used in Vk command buffers. They keep a record
// of their use in ANGLE and VkQueues via ResourceUse.
class Resource : angle::NonCopyable
{
  public:
    virtual ~Resource() {}

    // Complete all recorded and in-flight commands involving this resource
    angle::Result waitForIdle(ContextVk *contextVk,
                              const char *debugMessage,
                              RenderPassClosureReason reason);

    void setSerial(SerialIndex index, Serial serial) { mUse.setSerial(index, serial); }

    void setQueueSerial(const QueueSerial &queueSerial)
    {
        mUse.setSerial(queueSerial.getIndex(), queueSerial.getSerial());
    }

    void mergeResourceUse(const ResourceUse &use) { mUse.merge(use); }

    // Check if this resource is used by a command buffer.
    bool usedByCommandBuffer(const QueueSerial &commandBufferQueueSerial) const
    {
        return mUse.usedByCommandBuffer(commandBufferQueueSerial);
    }

    const ResourceUse &getResourceUse() const { return mUse; }

  protected:
    Resource() {}
    Resource(Resource &&other) : Resource() { mUse = std::move(other.mUse); }
    Resource &operator=(Resource &&rhs)
    {
        std::swap(mUse, rhs.mUse);
        return *this;
    }

    // Current resource lifetime.
    ResourceUse mUse;
};

// Similar to |Resource| above, this tracks object usage. This includes additional granularity to
// track whether an object is used for read-only or read/write access.
class ReadWriteResource : public Resource
{
  public:
    virtual ~ReadWriteResource() override {}

    // Complete all recorded and in-flight commands involving this resource
    angle::Result waitForIdle(ContextVk *contextVk,
                              const char *debugMessage,
                              RenderPassClosureReason reason)
    {
        return Resource::waitForIdle(contextVk, debugMessage, reason);
    }

    void setWriteQueueSerial(const QueueSerial &writeQueueSerial)
    {
        mUse.setQueueSerial(writeQueueSerial);
        mWriteUse.setQueueSerial(writeQueueSerial);
    }

    // Check if this resource is used by a command buffer.
    bool usedByCommandBuffer(const QueueSerial &commandBufferQueueSerial) const
    {
        return mUse.usedByCommandBuffer(commandBufferQueueSerial);
    }
    bool writtenByCommandBuffer(const QueueSerial &commandBufferQueueSerial) const
    {
        return mWriteUse.usedByCommandBuffer(commandBufferQueueSerial);
    }

    const ResourceUse &getWriteResourceUse() const { return mWriteUse; }

  protected:
    ReadWriteResource() {}
    ReadWriteResource(ReadWriteResource &&other) { *this = std::move(other); }
    ReadWriteResource &operator=(ReadWriteResource &&other)
    {
        Resource::operator=(std::move(other));
        mWriteUse = std::move(other.mWriteUse);
        return *this;
    }

    // Track write use of the object. Only updated for setWriteQueueSerial().
    ResourceUse mWriteUse;
};

// Adds "void release(Renderer *)" method for collecting garbage.
// Enables RendererScoped<> for classes that support DeviceScoped<>.
template <class T>
class ReleasableResource final : public Resource
{
  public:
    // Calls collectGarbage() on the object.
    void release(Renderer *renderer);

    const T &get() const { return mObject; }
    T &get() { return mObject; }

  private:
    T mObject;
};
}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RESOURCEVK_H_
