//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLEventVk.cpp: Implements the class methods for CLEventVk.

#include "libANGLE/renderer/vulkan/CLEventVk.h"
#include "libANGLE/renderer/vulkan/CLCommandQueueVk.h"

#include "libANGLE/cl_utils.h"

namespace rx
{

CLEventVk::CLEventVk(const cl::Event &event)
    : CLEventImpl(event),
      mStatus(isUserEvent() ? CL_SUBMITTED : CL_QUEUED),
      mProfilingTimestamps(ProfilingTimestamps{})
{
    ANGLE_CL_IMPL_TRY(setTimestamp(*mStatus));
}

CLEventVk::~CLEventVk() {}

angle::Result CLEventVk::getCommandExecutionStatus(cl_int &executionStatus)
{
    executionStatus = *mStatus;
    return angle::Result::Continue;
}

angle::Result CLEventVk::setUserEventStatus(cl_int executionStatus)
{
    ASSERT(isUserEvent());

    // Not much to do here other than storing the user supplied state.
    // Error checking and single call enforcement is responsibility of the front end.
    ANGLE_TRY(setStatusAndExecuteCallback(executionStatus));

    // User event set and callback(s) finished - notify those waiting
    mUserEventCondition.notify_all();

    return angle::Result::Continue;
}

angle::Result CLEventVk::setCallback(cl::Event &event, cl_int commandExecCallbackType)
{
    ASSERT(commandExecCallbackType >= CL_COMPLETE);
    ASSERT(commandExecCallbackType < CL_QUEUED);

    // Not much to do, acknowledge the presence of callback and returns
    mHaveCallbacks->at(commandExecCallbackType) = true;

    return angle::Result::Continue;
}

angle::Result CLEventVk::getProfilingInfo(cl::ProfilingInfo name,
                                          size_t valueSize,
                                          void *value,
                                          size_t *valueSizeRet)
{
    cl_ulong valueUlong   = 0;
    size_t copySize       = 0;
    const void *copyValue = nullptr;

    auto profilingTimestamps = mProfilingTimestamps.synchronize();

    switch (name)
    {
        case cl::ProfilingInfo::CommandQueued:
            valueUlong = profilingTimestamps->commandQueuedTS;
            break;
        case cl::ProfilingInfo::CommandSubmit:
            valueUlong = profilingTimestamps->commandSubmitTS;
            break;
        case cl::ProfilingInfo::CommandStart:
            valueUlong = profilingTimestamps->commandStartTS;
            break;
        case cl::ProfilingInfo::CommandEnd:
            valueUlong = profilingTimestamps->commandEndTS;
            break;
        case cl::ProfilingInfo::CommandComplete:
            valueUlong = profilingTimestamps->commandCompleteTS;
            break;
        default:
            UNREACHABLE();
    }
    copyValue = &valueUlong;
    copySize  = sizeof(valueUlong);

    if ((value != nullptr) && (copyValue != nullptr))
    {
        memcpy(value, copyValue, std::min(valueSize, copySize));
    }

    if (valueSizeRet != nullptr)
    {
        *valueSizeRet = copySize;
    }

    return angle::Result::Continue;
}

angle::Result CLEventVk::waitForUserEventStatus()
{
    ASSERT(isUserEvent());

    cl_int status = CL_QUEUED;
    std::unique_lock<std::mutex> ul(mUserEventMutex);
    ANGLE_TRY(getCommandExecutionStatus(status));
    if (status > CL_COMPLETE)
    {
        // User is responsible for setting the user-event object, we need to wait for that event
        // (We dont care what the outcome is, just need to wait until that event triggers)
        INFO() << "Waiting for user-event (" << &mEvent
               << ") to be set! (aka clSetUserEventStatus)";
        mUserEventCondition.wait(ul);
    }

    return angle::Result::Continue;
}

angle::Result CLEventVk::setStatusAndExecuteCallback(cl_int status)
{
    auto statusHandle        = mStatus.synchronize();
    auto haveCallbacksHandle = mHaveCallbacks.synchronize();

    // we might skip states in some cases i.e. move from QUEUED to COMPLETE, so
    // make sure we are setting time stamps for all transitions
    ASSERT(*statusHandle > status);
    do
    {
        (*statusHandle)--;
        ANGLE_TRY(setTimestamp(*statusHandle));
        if (*statusHandle >= CL_COMPLETE && *statusHandle < CL_QUEUED &&
            haveCallbacksHandle->at(*statusHandle))
        {
            getFrontendObject().callback(*statusHandle);
            haveCallbacksHandle->at(*statusHandle) = false;
        }

    } while (*statusHandle > status);

    return angle::Result::Continue;
}

angle::Result CLEventVk::setTimestamp(cl_int status)
{
    if (!isUserEvent() &&
        mEvent.getCommandQueue()->getProperties().intersects(CL_QUEUE_PROFILING_ENABLE))
    {
        // TODO(aannestrand) Just get current CPU timestamp for now, look into Vulkan GPU device
        // timestamp query instead and later make CPU timestamp a fallback if GPU timestamp cannot
        // be queried http://anglebug.com/357902514
        cl_ulong cpuTS =
            std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now())
                .time_since_epoch()
                .count();

        auto profilingTimestamps = mProfilingTimestamps.synchronize();

        switch (status)
        {
            case CL_QUEUED:
                profilingTimestamps->commandQueuedTS = cpuTS;
                break;
            case CL_SUBMITTED:
                profilingTimestamps->commandSubmitTS = cpuTS;
                break;
            case CL_RUNNING:
                profilingTimestamps->commandStartTS = cpuTS;
                break;
            case CL_COMPLETE:
                profilingTimestamps->commandEndTS = cpuTS;

                // Returns a value equivalent to passing CL_PROFILING_COMMAND_END if the device
                // associated with event does not support device-side enqueue.
                // https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_API.html#_device_side_enqueue
                profilingTimestamps->commandCompleteTS = cpuTS;
                break;
            default:
                UNREACHABLE();
        }
    }

    return angle::Result::Continue;
}

}  // namespace rx
