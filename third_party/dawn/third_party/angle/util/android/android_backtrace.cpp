//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// android_backtrace.cpp:
//   Implements functions to output the backtrace from the ANGLE code during execution on Android.
//

#include "common/backtrace_utils.h"

namespace angle
{

void printBacktraceInfo(BacktraceInfo backtraceInfo)
{
    // Return if no backtrace data is available.
    if (backtraceInfo.getStackAddresses().empty())
    {
        return;
    }

    WARN() << "Backtrace start";
    for (size_t i = 0; i < backtraceInfo.getSize(); i++)
    {
        WARN() << i << ":" << backtraceInfo.getStackAddress(i) << " -> "
               << backtraceInfo.getStackSymbol(i);
    }
    WARN() << "Backtrace end";
}
}  // namespace angle
