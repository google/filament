//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLEvent.cpp: Implements the cl::Event class.

#include "libANGLE/CLEvent.h"

#include "libANGLE/CLCommandQueue.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/cl_utils.h"

#include <cstring>

namespace cl
{

angle::Result Event::setUserEventStatus(cl_int executionStatus)
{
    ANGLE_TRY(mImpl->setUserEventStatus(executionStatus));
    mStatusWasChanged = true;
    return angle::Result::Continue;
}

angle::Result Event::getInfo(EventInfo name,
                             size_t valueSize,
                             void *value,
                             size_t *valueSizeRet) const
{
    cl_int execStatus     = 0;
    cl_uint valUInt       = 0u;
    void *valPointer      = nullptr;
    const void *copyValue = nullptr;
    size_t copySize       = 0u;

    switch (name)
    {
        case EventInfo::CommandQueue:
            valPointer = CommandQueue::CastNative(mCommandQueue.get());
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        case EventInfo::CommandType:
            copyValue = &mCommandType;
            copySize  = sizeof(mCommandType);
            break;
        case EventInfo::ReferenceCount:
            valUInt   = getRefCount();
            copyValue = &valUInt;
            copySize  = sizeof(valUInt);
            break;
        case EventInfo::CommandExecutionStatus:
        {
            ANGLE_TRY(mImpl->getCommandExecutionStatus(execStatus));
            copyValue = &execStatus;
            copySize  = sizeof(execStatus);
            break;
        }
        case EventInfo::Context:
            valPointer = mContext->getNative();
            copyValue  = &valPointer;
            copySize   = sizeof(valPointer);
            break;
        default:
            ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }

    if (value != nullptr)
    {
        // CL_INVALID_VALUE if size in bytes specified by param_value_size is < size of return type
        // as described in the Event Queries table and param_value is not NULL.
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

angle::Result Event::setCallback(cl_int commandExecCallbackType, EventCB pfnNotify, void *userData)
{
    auto callbacks = mCallbacks.synchronize();

    // Spec mentions that the callback will be called when the execution status of the event is
    // equal to past the status specified by commandExecCallbackType
    cl_int currentStatus;
    ANGLE_TRY(mImpl->getCommandExecutionStatus(currentStatus));
    if (currentStatus <= commandExecCallbackType)
    {
        pfnNotify(this, commandExecCallbackType, userData);
        return angle::Result::Continue;
    }

    // Only when required register a single callback with the back end for each callback type.
    if ((*callbacks)[commandExecCallbackType].empty())
    {
        ANGLE_TRY(mImpl->setCallback(*this, commandExecCallbackType));
        // This event has to be retained until the callback is called.
        retain();
    }
    (*callbacks)[commandExecCallbackType].emplace_back(pfnNotify, userData);
    return angle::Result::Continue;
}

angle::Result Event::getProfilingInfo(ProfilingInfo name,
                                      size_t valueSize,
                                      void *value,
                                      size_t *valueSizeRet)
{
    return mImpl->getProfilingInfo(name, valueSize, value, valueSizeRet);
}

Event::~Event() = default;

void Event::callback(cl_int commandStatus)
{
    ASSERT(commandStatus >= 0 && commandStatus < 3);
    const Callbacks callbacks = std::move(mCallbacks->at(commandStatus));
    for (const CallbackData &data : callbacks)
    {
        data.first(this, commandStatus, data.second);
    }
    // This event can be released after the callback was called.
    if (release())
    {
        delete this;
    }
}

EventPtrs Event::Cast(cl_uint numEvents, const cl_event *eventList)
{
    EventPtrs events;
    events.reserve(numEvents);
    while (numEvents-- != 0u)
    {
        events.emplace_back(&(*eventList++)->cast<Event>());
    }
    return events;
}

Event::Event(Context &context) : mContext(&context), mCommandType(CL_COMMAND_USER), mImpl(nullptr)
{
    ANGLE_CL_IMPL_TRY(context.getImpl().createUserEvent(*this, &mImpl));
}

Event::Event(CommandQueue &queue,
             cl_command_type commandType,
             const rx::CLEventImpl::CreateFunc &createFunc)
    : mContext(&queue.getContext()),
      mCommandQueue(&queue),
      mCommandType(commandType),
      mImpl(createFunc(*this))
{}

}  // namespace cl
