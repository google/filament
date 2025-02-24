//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationCL.h: Validation functions for generic CL entry point parameters

#ifndef LIBANGLE_VALIDATIONCL_H_
#define LIBANGLE_VALIDATIONCL_H_

#include "libANGLE/CLBuffer.h"
#include "libANGLE/CLCommandQueue.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLDevice.h"
#include "libANGLE/CLEvent.h"
#include "libANGLE/CLImage.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLMemory.h"
#include "libANGLE/CLPlatform.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/CLSampler.h"

#define ANGLE_CL_VALIDATE_VOID(EP, ...)              \
    do                                               \
    {                                                \
        if (Validate##EP(__VA_ARGS__) != CL_SUCCESS) \
        {                                            \
            return;                                  \
        }                                            \
    } while (0)

#define ANGLE_CL_VALIDATE_ERROR(EP, ...)              \
    do                                                \
    {                                                 \
        cl_int errorCode = Validate##EP(__VA_ARGS__); \
        if (errorCode != CL_SUCCESS)                  \
        {                                             \
            return errorCode;                         \
        }                                             \
    } while (0)

#define ANGLE_CL_VALIDATE_ERRCODE_RET(EP, ...)        \
    do                                                \
    {                                                 \
        cl_int errorCode = Validate##EP(__VA_ARGS__); \
        if (errorCode != CL_SUCCESS)                  \
        {                                             \
            if (errcode_ret != nullptr)               \
            {                                         \
                *errcode_ret = errorCode;             \
            }                                         \
            return nullptr;                           \
        }                                             \
    } while (0)

#define ANGLE_CL_VALIDATE_POINTER(EP, ...)            \
    do                                                \
    {                                                 \
        cl_int errorCode = Validate##EP(__VA_ARGS__); \
        if (errorCode != CL_SUCCESS)                  \
        {                                             \
            return nullptr;                           \
        }                                             \
    } while (0)

#endif  // LIBANGLE_VALIDATIONCL_H_
