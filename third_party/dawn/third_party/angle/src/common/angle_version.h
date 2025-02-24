//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_version.h: ANGLE version constants. Generated from git commands.

#ifndef COMMON_ANGLE_VERSION_H_
#define COMMON_ANGLE_VERSION_H_

#include "ANGLEShaderProgramVersion.h"
#include "angle_commit.h"

#define ANGLE_MAJOR_VERSION 2
#define ANGLE_MINOR_VERSION 1

#ifndef ANGLE_REVISION
#    define ANGLE_REVISION ANGLE_COMMIT_POSITION
#endif

#define ANGLE_STRINGIFY(x) #x
#define ANGLE_MACRO_STRINGIFY(x) ANGLE_STRINGIFY(x)

#if (ANGLE_REVISION != 0)
#    define ANGLE_REVISION_SUFFIX "." ANGLE_MACRO_STRINGIFY(ANGLE_REVISION)
#else
#    define ANGLE_REVISION_SUFFIX ""
#endif

#define ANGLE_VERSION_STRING                                             \
    ANGLE_MACRO_STRINGIFY(ANGLE_MAJOR_VERSION)                           \
    "." ANGLE_MACRO_STRINGIFY(ANGLE_MINOR_VERSION) ANGLE_REVISION_SUFFIX \
        " git hash: " ANGLE_COMMIT_HASH

#endif  // COMMON_ANGLE_VERSION_H_
