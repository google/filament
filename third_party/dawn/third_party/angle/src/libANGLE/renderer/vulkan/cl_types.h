//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_types.h: Defines common types for the OpenCL Vulkan back end.

#ifndef LIBANGLE_RENDERER_VULKAN_CL_TYPES_H_
#define LIBANGLE_RENDERER_VULKAN_CL_TYPES_H_

#include "libANGLE/renderer/cl_types.h"

namespace rx
{

class CLContextVk;
class CLDeviceVk;
class CLPlatformVk;
class CLProgramVk;

// Specialization constant Types
enum class SpecConstantType : uint32_t
{
    WorkgroupSizeX,
    WorkgroupSizeY,
    WorkgroupSizeZ,
    WorkDimension,
    GlobalOffsetX,
    GlobalOffsetY,
    GlobalOffsetZ,

    InvalidEnum,
    EnumCount = InvalidEnum
};

enum class ImageBufferCopyDirection
{
    ToImage,
    ToBuffer
};

enum class ImageCopyWith
{
    Image,
    Buffer
};

enum class StagingBufferCopyDirection
{
    ToHost,
    ToStagingBuffer
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CL_TYPES_H_
