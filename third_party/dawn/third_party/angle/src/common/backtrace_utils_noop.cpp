//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// backtrace_utils_noop.cpp:
//   Implements the placeholder functions related to the backtrace info class for non-Android
//   platforms or when the backtrace feature is disabled.
//

#include "backtrace_utils.h"

namespace angle
{

void BacktraceInfo::populateBacktraceInfo(void **stackAddressBuffer, size_t stackAddressCount) {}

BacktraceInfo getBacktraceInfo()
{
    return {};
}

// The following function has been defined in each platform separately.
// - void printBacktraceInfo(BacktraceInfo backtraceInfo);

}  // namespace angle
