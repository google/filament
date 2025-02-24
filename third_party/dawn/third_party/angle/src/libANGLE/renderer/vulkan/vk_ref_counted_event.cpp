//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RefCountedEvent:
//    Manages reference count of VkEvent and its associated functions.
//

#include "libANGLE/renderer/vulkan/vk_ref_counted_event.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"

namespace rx
{
namespace vk
{
namespace
{
void DestroyRefCountedEvents(VkDevice device, RefCountedEventCollector &events)
{
    while (!events.empty())
    {
        events.back().destroy(device);
        events.pop_back();
    }
}
}  // namespace

bool RefCountedEvent::init(Context *context, EventStage eventStage)
{
    ASSERT(mHandle == nullptr);
    ASSERT(eventStage != EventStage::InvalidEnum);

    // First try with recycler. We must issue VkCmdResetEvent before VkCmdSetEvent
    if (context->getRefCountedEventsGarbageRecycler()->fetch(context->getRenderer(), this))
    {
        ASSERT(valid());
        ASSERT(!mHandle->isReferenced());
    }
    else
    {
        // If failed to fetch from recycler, then create a new event.
        mHandle                      = new RefCounted<EventAndStage>;
        VkEventCreateInfo createInfo = {};
        createInfo.sType             = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
        // Use device only for performance reasons.
        createInfo.flags = context->getFeatures().supportsSynchronization2.enabled
                               ? VK_EVENT_CREATE_DEVICE_ONLY_BIT_KHR
                               : 0;
        VkResult result  = mHandle->get().event.init(context->getDevice(), createInfo);
        if (result != VK_SUCCESS)
        {
            WARN() << "event.init failed. Clean up garbage and retry again";
            // Proactively clean up garbage and retry
            context->getRefCountedEventsGarbageRecycler()->cleanup(context->getRenderer());
            result = mHandle->get().event.init(context->getDevice(), createInfo);
            if (result != VK_SUCCESS)
            {
                // Drivers usually can allocate huge amount of VkEvents, and we should never use
                // that many VkEvents under normal situation. If we failed to allocate, there is a
                // high chance that we may have a leak somewhere. This macro should help us catch
                // such potential bugs in the bots if that happens.
                UNREACHABLE();
                // If still fail to create, we just return. An invalid event will trigger
                // pipelineBarrier code path
                return false;
            }
        }
    }

    mHandle->addRef();
    mHandle->get().eventStage = eventStage;
    return true;
}

void RefCountedEvent::release(Context *context)
{
    if (mHandle != nullptr)
    {
        releaseImpl(context->getRenderer(), context->getRefCountedEventsGarbageRecycler());
    }
}

void RefCountedEvent::release(Renderer *renderer)
{
    if (mHandle != nullptr)
    {
        releaseImpl(renderer, renderer->getRefCountedEventRecycler());
    }
}

template <typename RecyclerT>
void RefCountedEvent::releaseImpl(Renderer *renderer, RecyclerT *recycler)
{
    ASSERT(mHandle != nullptr);
    // This should never be called from async clean up thread since the refcount is not atomic. It
    // is expected only called under context share lock.
    ASSERT(std::this_thread::get_id() != renderer->getCleanUpThreadId());

    const bool isLastReference = mHandle->getAndReleaseRef() == 1;
    if (isLastReference)
    {
        ASSERT(recycler != nullptr);
        recycler->recycle(std::move(*this));
        ASSERT(mHandle == nullptr);
    }
    else
    {
        mHandle = nullptr;
    }
}

void RefCountedEvent::destroy(VkDevice device)
{
    ASSERT(mHandle != nullptr);
    ASSERT(!mHandle->isReferenced());
    mHandle->get().event.destroy(device);
    SafeDelete(mHandle);
}

VkPipelineStageFlags RefCountedEvent::getPipelineStageMask(Renderer *renderer) const
{
    return renderer->getPipelineStageMask(getEventStage());
}

// RefCountedEventArray implementation.
void RefCountedEventArray::release(Renderer *renderer)
{
    for (EventStage eventStage : mBitMask)
    {
        ASSERT(mEvents[eventStage].valid());
        mEvents[eventStage].release(renderer);
    }
    mBitMask.reset();
}

void RefCountedEventArray::release(Context *context)
{
    for (EventStage eventStage : mBitMask)
    {
        ASSERT(mEvents[eventStage].valid());
        mEvents[eventStage].release(context);
    }
    mBitMask.reset();
}

void RefCountedEventArray::releaseToEventCollector(RefCountedEventCollector *eventCollector)
{
    for (EventStage eventStage : mBitMask)
    {
        eventCollector->emplace_back(std::move(mEvents[eventStage]));
    }
    mBitMask.reset();
}

bool RefCountedEventArray::initEventAtStage(Context *context, EventStage eventStage)
{
    if (mBitMask[eventStage])
    {
        return true;
    }

    // Create the event if we have not yet so. Otherwise just use the already created event.
    if (!mEvents[eventStage].init(context, eventStage))
    {
        return false;
    }
    mBitMask.set(eventStage);
    return true;
}

template <typename CommandBufferT>
void RefCountedEventArray::flushSetEvents(Renderer *renderer, CommandBufferT *commandBuffer) const
{
    for (EventStage eventStage : mBitMask)
    {
        VkPipelineStageFlags pipelineStageFlags = renderer->getPipelineStageMask(eventStage);
        commandBuffer->setEvent(mEvents[eventStage].getEvent().getHandle(), pipelineStageFlags);
    }
}

template void RefCountedEventArray::flushSetEvents<VulkanSecondaryCommandBuffer>(
    Renderer *renderer,
    VulkanSecondaryCommandBuffer *commandBuffer) const;
template void RefCountedEventArray::flushSetEvents<priv::SecondaryCommandBuffer>(
    Renderer *renderer,
    priv::SecondaryCommandBuffer *commandBuffer) const;
template void RefCountedEventArray::flushSetEvents<priv::CommandBuffer>(
    Renderer *renderer,
    priv::CommandBuffer *commandBuffer) const;

// EventArray implementation.
void EventArray::init(Renderer *renderer, const RefCountedEventArray &refCountedEventArray)
{
    mBitMask = refCountedEventArray.getBitMask();
    for (EventStage eventStage : mBitMask)
    {
        ASSERT(refCountedEventArray.getEvent(eventStage).valid());
        mEvents[eventStage] = refCountedEventArray.getEvent(eventStage).getEvent().getHandle();
        mPipelineStageFlags[eventStage] = renderer->getPipelineStageMask(eventStage);
    }
}

void EventArray::flushSetEvents(PrimaryCommandBuffer *primary)
{
    for (EventStage eventStage : mBitMask)
    {
        ASSERT(mEvents[eventStage] != VK_NULL_HANDLE);
        primary->setEvent(mEvents[eventStage], mPipelineStageFlags[eventStage]);
        mEvents[eventStage] = VK_NULL_HANDLE;
    }
    mBitMask.reset();
}

// RefCountedEventsGarbage implementation.
void RefCountedEventsGarbage::destroy(Renderer *renderer)
{
    ASSERT(renderer->hasQueueSerialFinished(mQueueSerial));
    while (!mRefCountedEvents.empty())
    {
        ASSERT(mRefCountedEvents.back().valid());
        mRefCountedEvents.back().release(renderer);
        mRefCountedEvents.pop_back();
    }
}

bool RefCountedEventsGarbage::releaseIfComplete(Renderer *renderer,
                                                RefCountedEventsGarbageRecycler *recycler)
{
    if (!renderer->hasQueueSerialFinished(mQueueSerial))
    {
        return false;
    }

    while (!mRefCountedEvents.empty())
    {
        ASSERT(mRefCountedEvents.back().valid());
        mRefCountedEvents.back().releaseImpl(renderer, recycler);
        ASSERT(!mRefCountedEvents.back().valid());
        mRefCountedEvents.pop_back();
    }
    return true;
}

bool RefCountedEventsGarbage::moveIfComplete(Renderer *renderer,
                                             std::deque<RefCountedEventCollector> *releasedBucket)
{
    if (!renderer->hasQueueSerialFinished(mQueueSerial))
    {
        return false;
    }

    releasedBucket->emplace_back(std::move(mRefCountedEvents));
    return true;
}

// RefCountedEventRecycler implementation.
void RefCountedEventRecycler::destroy(VkDevice device)
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);

    while (!mEventsToReset.empty())
    {
        DestroyRefCountedEvents(device, mEventsToReset.back());
        mEventsToReset.pop_back();
    }

    ASSERT(mResettingQueue.empty());

    while (!mEventsToReuse.empty())
    {
        DestroyRefCountedEvents(device, mEventsToReuse.back());
        mEventsToReuse.pop_back();
    }
}

void RefCountedEventRecycler::resetEvents(ErrorContext *context,
                                          const QueueSerial queueSerial,
                                          PrimaryCommandBuffer *commandbuffer)
{
    std::lock_guard<angle::SimpleMutex> lock(mMutex);

    if (mEventsToReset.empty())
    {
        return;
    }

    Renderer *renderer = context->getRenderer();
    while (!mEventsToReset.empty())
    {
        RefCountedEventCollector &events = mEventsToReset.back();
        ASSERT(!events.empty());
        for (const RefCountedEvent &refCountedEvent : events)
        {
            VkPipelineStageFlags stageMask = refCountedEvent.getPipelineStageMask(renderer);
            commandbuffer->resetEvent(refCountedEvent.getEvent().getHandle(), stageMask);
        }
        mResettingQueue.emplace(queueSerial, std::move(events));
        mEventsToReset.pop_back();
    }
}

size_t RefCountedEventRecycler::cleanupResettingEvents(Renderer *renderer)
{
    size_t eventsReleased = 0;
    std::lock_guard<angle::SimpleMutex> lock(mMutex);
    while (!mResettingQueue.empty())
    {
        bool released = mResettingQueue.front().moveIfComplete(renderer, &mEventsToReuse);
        if (released)
        {
            mResettingQueue.pop();
            ++eventsReleased;
        }
        else
        {
            break;
        }
    }
    return eventsReleased;
}

bool RefCountedEventRecycler::fetchEventsToReuse(RefCountedEventCollector *eventsToReuseOut)
{
    ASSERT(eventsToReuseOut != nullptr);
    ASSERT(eventsToReuseOut->empty());
    std::lock_guard<angle::SimpleMutex> lock(mMutex);
    if (mEventsToReuse.empty())
    {
        return false;
    }
    eventsToReuseOut->swap(mEventsToReuse.back());
    mEventsToReuse.pop_back();
    return true;
}

// RefCountedEventsGarbageRecycler implementation.
RefCountedEventsGarbageRecycler::~RefCountedEventsGarbageRecycler()
{
    ASSERT(mEventsToReset.empty());
    ASSERT(mGarbageQueue.empty());
    ASSERT(mEventsToReuse.empty());
    ASSERT(mGarbageCount == 0);
}

void RefCountedEventsGarbageRecycler::destroy(Renderer *renderer)
{
    VkDevice device = renderer->getDevice();
    DestroyRefCountedEvents(device, mEventsToReset);
    ASSERT(mGarbageQueue.empty());
    ASSERT(mGarbageCount == 0);
    mEventsToReuse.destroy(device);
}

void RefCountedEventsGarbageRecycler::cleanup(Renderer *renderer)
{
    // First cleanup already completed events and add to mEventsToReset
    while (!mGarbageQueue.empty())
    {
        size_t count  = mGarbageQueue.front().size();
        bool released = mGarbageQueue.front().releaseIfComplete(renderer, this);
        if (released)
        {
            mGarbageCount -= count;
            mGarbageQueue.pop();
        }
        else
        {
            break;
        }
    }

    // Move mEventsToReset to the renderer so that it can be reset.
    if (!mEventsToReset.empty())
    {
        renderer->getRefCountedEventRecycler()->recycle(std::move(mEventsToReset));
    }
}

bool RefCountedEventsGarbageRecycler::fetch(Renderer *renderer, RefCountedEvent *outObject)
{
    if (mEventsToReuse.empty())
    {
        // Retrieve a list of ready to reuse events from renderer.
        RefCountedEventCollector events;
        if (!renderer->getRefCountedEventRecycler()->fetchEventsToReuse(&events))
        {
            return false;
        }
        mEventsToReuse.refill(std::move(events));
        ASSERT(!mEventsToReuse.empty());
    }
    mEventsToReuse.fetch(outObject);
    return true;
}

// EventBarrier implementation.
void EventBarrier::addDiagnosticsString(std::ostringstream &out) const
{
    if (mMemoryBarrierSrcAccess != 0 || mMemoryBarrierDstAccess != 0)
    {
        out << "Src: 0x" << std::hex << mMemoryBarrierSrcAccess << " &rarr; Dst: 0x" << std::hex
            << mMemoryBarrierDstAccess << std::endl;
    }
}

void EventBarrier::execute(PrimaryCommandBuffer *primary)
{
    if (isEmpty())
    {
        return;
    }
    ASSERT(mEvent != VK_NULL_HANDLE);
    ASSERT(mImageMemoryBarrierCount == 0 ||
           (mImageMemoryBarrierCount == 1 && mImageMemoryBarrier.image != VK_NULL_HANDLE));

    // Issue vkCmdWaitEvents call
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask   = mMemoryBarrierSrcAccess;
    memoryBarrier.dstAccessMask   = mMemoryBarrierDstAccess;

    primary->waitEvents(1, &mEvent, mSrcStageMask, mDstStageMask, 1, &memoryBarrier, 0, nullptr,
                        mImageMemoryBarrierCount,
                        mImageMemoryBarrierCount == 0 ? nullptr : &mImageMemoryBarrier);
}

// EventBarrierArray implementation.
void EventBarrierArray::addAdditionalStageAccess(const RefCountedEvent &waitEvent,
                                                 VkPipelineStageFlags dstStageMask,
                                                 VkAccessFlags dstAccess)
{
    for (EventBarrier &barrier : mBarriers)
    {
        if (barrier.hasEvent(waitEvent.getEvent().getHandle()))
        {
            barrier.addAdditionalStageAccess(dstStageMask, dstAccess);
            return;
        }
    }
    UNREACHABLE();
}

void EventBarrierArray::addEventMemoryBarrier(Renderer *renderer,
                                              const RefCountedEvent &waitEvent,
                                              VkAccessFlags srcAccess,
                                              VkPipelineStageFlags dstStageMask,
                                              VkAccessFlags dstAccess)
{
    ASSERT(waitEvent.valid());
    VkPipelineStageFlags srcStageFlags = waitEvent.getPipelineStageMask(renderer);
    mBarriers.emplace_back(srcStageFlags, dstStageMask, srcAccess, dstAccess,
                           waitEvent.getEvent().getHandle());
}

void EventBarrierArray::addEventImageBarrier(Renderer *renderer,
                                             const RefCountedEvent &waitEvent,
                                             VkPipelineStageFlags dstStageMask,
                                             const VkImageMemoryBarrier &imageMemoryBarrier)
{
    ASSERT(waitEvent.valid());
    VkPipelineStageFlags srcStageFlags = waitEvent.getPipelineStageMask(renderer);
    mBarriers.emplace_back(srcStageFlags, dstStageMask, waitEvent.getEvent().getHandle(),
                           imageMemoryBarrier);
}

void EventBarrierArray::execute(Renderer *renderer, PrimaryCommandBuffer *primary)
{
    while (!mBarriers.empty())
    {
        mBarriers.back().execute(primary);
        mBarriers.pop_back();
    }
    reset();
}

void EventBarrierArray::addDiagnosticsString(std::ostringstream &out) const
{
    out << "Event Barrier: ";
    for (const EventBarrier &barrier : mBarriers)
    {
        barrier.addDiagnosticsString(out);
    }
    out << "\\l";
}
}  // namespace vk
}  // namespace rx
