//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_cl_utils.cpp: These helpers are used in CL entry point routines.

#include "libGLESv2/entry_points_cl_utils.h"

#include "libGLESv2/cl_dispatch_table.h"

#include "libANGLE/CLPlatform.h"
#ifdef ANGLE_ENABLE_CL_PASSTHROUGH
#    include "libANGLE/renderer/cl/CLPlatformCL.h"
#endif
#ifdef ANGLE_ENABLE_VULKAN
#    include "libANGLE/renderer/vulkan/CLPlatformVk.h"
#endif

#include "anglebase/no_destructor.h"

#include <mutex>

namespace cl
{

void InitBackEnds(bool isIcd)
{
    enum struct State
    {
        Uninitialized,
        Initializing,
        Initialized
    };
    static State sState = State::Uninitialized;

    // Fast thread-unsafe check first
    if (sState == State::Initialized)
    {
        return;
    }

    static angle::base::NoDestructor<std::recursive_mutex> sMutex;
    std::lock_guard<std::recursive_mutex> lock(*sMutex);

    // Thread-safe check, return if initialized
    // or if already initializing (re-entry from CL pass-through back end)
    if (sState != State::Uninitialized)
    {
        return;
    }

    sState = State::Initializing;

    rx::CLPlatformImpl::CreateFuncs createFuncs;
#ifdef ANGLE_ENABLE_CL_PASSTHROUGH
    rx::CLPlatformCL::Initialize(createFuncs, isIcd);
#endif
#ifdef ANGLE_ENABLE_VULKAN
    rx::CLPlatformVk::Initialize(createFuncs);
#endif
    Platform::Initialize(gCLIcdDispatchTable, std::move(createFuncs));

    sState = State::Initialized;
}

}  // namespace cl
