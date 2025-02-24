//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_version_info.h: ANGLE version queries.

#ifndef COMMON_VERSION_INFO_H_
#define COMMON_VERSION_INFO_H_

namespace angle
{
int GetANGLERevision();
const char *GetANGLEVersionString();
const char *GetANGLECommitHash();
int GetANGLECommitHashSize();
const char *GetANGLEShaderProgramVersion();
int GetANGLEShaderProgramVersionHashSize();
int GetANGLESHVersion();
}  // namespace angle

#endif  // COMMON_VERSION_INFO_H_
