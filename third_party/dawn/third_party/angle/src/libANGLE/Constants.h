//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Contants.h: Defines some implementation specific and gl constants

#ifndef LIBANGLE_CONSTANTS_H_
#define LIBANGLE_CONSTANTS_H_

#include "common/platform.h"

#include <stddef.h>

namespace gl
{

// The binary cache is currently left disable by default, and the application can enable it.
const size_t kDefaultMaxProgramCacheMemoryBytes = 0;

enum
{
    // Implementation upper limits, real maximums depend on the hardware

    // Only up to 32x MSAA supported.
    IMPLEMENTATION_MAX_SAMPLE_MASK_WORDS = 1,
    IMPLEMENTATION_MAX_SAMPLES           = 32,

    MAX_VERTEX_ATTRIBS         = 16,
    MAX_VERTEX_ATTRIB_BINDINGS = 16,

    IMPLEMENTATION_MAX_VARYING_VECTORS = 32,
    IMPLEMENTATION_MAX_DRAW_BUFFERS    = 8,
    IMPLEMENTATION_MAX_FRAMEBUFFER_ATTACHMENTS =
        IMPLEMENTATION_MAX_DRAW_BUFFERS + 2,  // 2 extra for depth and/or stencil buffers

    // The vast majority of devices support only one dual-source draw buffer
    IMPLEMENTATION_MAX_DUAL_SOURCE_DRAW_BUFFERS = 1,

    IMPLEMENTATION_MAX_VERTEX_SHADER_UNIFORM_BUFFERS   = 16,
    IMPLEMENTATION_MAX_GEOMETRY_SHADER_UNIFORM_BUFFERS = 16,
    IMPLEMENTATION_MAX_FRAGMENT_SHADER_UNIFORM_BUFFERS = 16,
    IMPLEMENTATION_MAX_COMPUTE_SHADER_UNIFORM_BUFFERS  = 16,

    // GL_EXT_geometry_shader increases the minimum value of GL_MAX_COMBINED_UNIFORM_BLOCKS to 36.
    // GL_EXT_tessellation_shader increases the minimum value to 60.
    IMPLEMENTATION_MAX_COMBINED_SHADER_UNIFORM_BUFFERS = 60,

    // GL_EXT_geometry_shader increases the minimum value of GL_MAX_UNIFORM_BUFFER_BINDINGS to 48.
    // GL_EXT_tessellation_shader increases the minimum value to 72.
    IMPLEMENTATION_MAX_UNIFORM_BUFFER_BINDINGS = 72,

    // Transform feedback limits set to the minimum required by the spec.
    IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS = 128,
    IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS       = 4,
    IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS    = 4,
    IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS                = 4,

    // Maximum number of views which are supported by the multiview implementation.
    IMPLEMENTATION_ANGLE_MULTIVIEW_MAX_VIEWS = 4,

    // These are the maximums the implementation can support
    // The actual GL caps are limited by the device caps
    // and should be queried from the Context
    IMPLEMENTATION_MAX_2D_TEXTURE_SIZE         = 32768,
    IMPLEMENTATION_MAX_CUBE_MAP_TEXTURE_SIZE   = 32768,
    IMPLEMENTATION_MAX_3D_TEXTURE_SIZE         = 16384,
    IMPLEMENTATION_MAX_2D_ARRAY_TEXTURE_LAYERS = 4096,

    // 1+log2 of max of MAX_*_TEXTURE_SIZE
    IMPLEMENTATION_MAX_TEXTURE_LEVELS = 16,

    IMPLEMENTATION_MAX_SHADER_TEXTURES = 32,

    // In ES 3.1 and below, the limit for active textures is 64.
    IMPLEMENTATION_MAX_ES31_ACTIVE_TEXTURES = 64,

    // In ES 3.2 we need to support a minimum of 96 maximum textures.
    IMPLEMENTATION_MAX_ACTIVE_TEXTURES = 96,
    IMPLEMENTATION_MAX_IMAGE_UNITS     = IMPLEMENTATION_MAX_ACTIVE_TEXTURES,

    // Maximum framebuffer and renderbuffer size supported.
    IMPLEMENTATION_MAX_FRAMEBUFFER_SIZE  = 32768,
    IMPLEMENTATION_MAX_RENDERBUFFER_SIZE = 32768,

    // Maximum number of slots allocated for atomic counter buffers.
    IMPLEMENTATION_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS = 8,

    // Implementation upper limits, real maximums depend on the hardware.
    IMPLEMENTATION_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 64,

    // Implementation upper limits of max number of clip distances (minimum required per spec)
    IMPLEMENTATION_MAX_CLIP_DISTANCES = 8,

    // Implementation upper limit for layered framebuffer layer count
    IMPLEMENTATION_MAX_FRAMEBUFFER_LAYERS = 256,

    // ANGLE_shader_pixel_local_storage: keep the maximum number of supported planes reasonably
    // similar on all platforms.
    IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES = 8,

    // QCOM foveated rendering constants
    // We support a max of 1 layer and 2 focal points, for now
    // TODO (anglebug.com/42266906): Implement support for multiple layers
    IMPLEMENTATION_MAX_NUM_LAYERS   = 1,
    IMPLEMENTATION_MAX_FOCAL_POINTS = 2,
};

namespace limits
{
// Almost all drivers use 2048 (the minimum value) as GL_MAX_VERTEX_ATTRIB_STRIDE.  ANGLE advertizes
// the same limit.
constexpr uint32_t kMaxVertexAttribStride = 2048;

// Some of the minimums required by GL, used to detect if the backend meets the minimum requirement.
// Currently, there's no need to separate these values per spec version.
constexpr uint32_t kMinimumComputeStorageBuffers = 4;

// OpenGL ES 3.0+ Minimum Values
// Table 6.31 MAX_VERTEX_UNIFORM_BLOCKS minimum value = 12
// Table 6.32 MAX_FRAGMENT_UNIFORM_BLOCKS minimum value = 12
constexpr uint32_t kMinimumShaderUniformBlocks = 12;
// Table 6.31 MAX_VERTEX_OUTPUT_COMPONENTS minimum value = 64
constexpr uint32_t kMinimumVertexOutputComponents = 64;

// OpenGL ES 3.2+ Minimum Values
// Table 21.42 TEXTURE_BUFFER_OFFSET_ALIGNMENT minimum value = 256
constexpr uint32_t kMinTextureBufferOffsetAlignment = 256;

}  // namespace limits

}  // namespace gl

namespace cl
{
enum
{
    // Implementation maximums

    // CL requires a min of 128 maximum read images as kernel arguments
    IMPLEMENATION_MAX_READ_IMAGES = 128,

    // CL requires a min of 64 maximum write images as kernel arguments
    IMPLEMENATION_MAX_WRITE_IMAGES = 64,
};
}

#endif  // LIBANGLE_CONSTANTS_H_
