//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// test_utils_uwp.cpp: Implementation of test utility functions for WinUWP.

#include "util/test_utils.h"

namespace angle
{
void PrintStackBacktrace()
{
    // Not available on UWP
}

void InitCrashHandler(CrashCallback *callback)
{
    // Not available on UWP
}

void TerminateCrashHandler()
{
    // Not available on UWP
}

int NumberOfProcessors()
{
    // A portable implementation could probably use GetLogicalProcessorInformation
    return 1;
}
}  // namespace angle
