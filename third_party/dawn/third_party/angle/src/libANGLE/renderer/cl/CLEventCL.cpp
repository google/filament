//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLEventCL.cpp: Implements the class methods for CLEventCL.

#include "libANGLE/renderer/cl/CLEventCL.h"

#include "libANGLE/CLEvent.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

CLEventCL::CLEventCL(const cl::Event &event, cl_event native) : CLEventImpl(event), mNative(native)
{}

CLEventCL::~CLEventCL()
{
    if (mNative->getDispatch().clReleaseEvent(mNative) != CL_SUCCESS)
    {
        ERR() << "Error while releasing CL event";
    }
}

angle::Result CLEventCL::getCommandExecutionStatus(cl_int &executionStatus)
{
    ANGLE_CL_TRY(mNative->getDispatch().clGetEventInfo(mNative, CL_EVENT_COMMAND_EXECUTION_STATUS,
                                                       sizeof(executionStatus), &executionStatus,
                                                       nullptr));
    return angle::Result::Continue;
}

angle::Result CLEventCL::setUserEventStatus(cl_int executionStatus)
{
    ANGLE_CL_TRY(mNative->getDispatch().clSetUserEventStatus(mNative, executionStatus));
    return angle::Result::Continue;
}

angle::Result CLEventCL::setCallback(cl::Event &event, cl_int commandExecCallbackType)
{
    ANGLE_CL_TRY(mNative->getDispatch().clSetEventCallback(mNative, commandExecCallbackType,
                                                           Callback, &event));
    return angle::Result::Continue;
}

angle::Result CLEventCL::getProfilingInfo(cl::ProfilingInfo name,
                                          size_t valueSize,
                                          void *value,
                                          size_t *valueSizeRet)
{
    ANGLE_CL_TRY(mNative->getDispatch().clGetEventProfilingInfo(mNative, cl::ToCLenum(name),
                                                                valueSize, value, valueSizeRet));
    return angle::Result::Continue;
}

std::vector<cl_event> CLEventCL::Cast(const cl::EventPtrs &events)
{
    std::vector<cl_event> nativeEvents;
    nativeEvents.reserve(events.size());
    for (const cl::EventPtr &event : events)
    {
        nativeEvents.emplace_back(event->getImpl<CLEventCL>().getNative());
    }
    return nativeEvents;
}

void CLEventCL::Callback(cl_event event, cl_int commandStatus, void *userData)
{
    static_cast<cl::Event *>(userData)->callback(commandStatus);
}

}  // namespace rx
