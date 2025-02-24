//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_FEATURES_H_
#define LIBANGLE_FEATURES_H_

#include "common/platform.h"

// Feature defaults

// Direct3D9EX
// The "Debug This Pixel..." feature in PIX often fails when using the
// D3D9Ex interfaces.  In order to get debug pixel to work on a Vista/Win 7
// machine, define "ANGLE_D3D9EX=0" in your project file.
#if !defined(ANGLE_D3D9EX)
#    define ANGLE_D3D9EX 1
#endif

// Vsync
// ENABLED allows Vsync to be configured at runtime
// DISABLED disallows Vsync
#if !defined(ANGLE_VSYNC)
#    define ANGLE_VSYNC 1
#endif

// Append HLSL assembly to shader debug info. Defaults to enabled in Debug and off in Release.
#if !defined(ANGLE_APPEND_ASSEMBLY_TO_SHADER_DEBUG_INFO)
#    if !defined(NDEBUG)
#        define ANGLE_APPEND_ASSEMBLY_TO_SHADER_DEBUG_INFO 1
#    else
#        define ANGLE_APPEND_ASSEMBLY_TO_SHADER_DEBUG_INFO 0
#    endif  // !defined(NDEBUG)
#endif      // !defined(ANGLE_APPEND_ASSEMBLY_TO_SHADER_DEBUG_INFO)

// Program link validation of precisions for uniforms. This feature was
// requested by developers to allow non-conformant shaders to be used which
// contain mismatched precisions.
// ENABLED validate that precision for uniforms match between vertex and fragment shaders
// DISABLED allow precision for uniforms to differ between vertex and fragment shaders
#if !defined(ANGLE_PROGRAM_LINK_VALIDATE_UNIFORM_PRECISION)
#    define ANGLE_PROGRAM_LINK_VALIDATE_UNIFORM_PRECISION 1
#endif

#endif  // LIBANGLE_FEATURES_H_
