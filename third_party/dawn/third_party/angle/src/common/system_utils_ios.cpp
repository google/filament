//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_osx.cpp: Implementation of OS-specific functions for OSX

#include "system_utils.h"

#include <unistd.h>

#include <CoreServices/CoreServices.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <cstdlib>
#include <vector>

#include <array>

namespace angle
{

const char *GetSharedLibraryExtension()
{
    return "framework";
}

}  // namespace angle
