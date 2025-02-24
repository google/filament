//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ExtensionBehavior.h: Extension name enumeration and data structures for storing extension
// behavior.

#ifndef COMPILER_TRANSLATOR_EXTENSIONBEHAVIOR_H_
#define COMPILER_TRANSLATOR_EXTENSIONBEHAVIOR_H_

#include <cstdint>
#include <map>

namespace sh
{

enum class TExtension : uint8_t
{
    UNDEFINED,  // Special value used to indicate no extension.

    ANDROID_extension_pack_es31a,
    ANGLE_base_vertex_base_instance_shader_builtin,
    ANGLE_clip_cull_distance,
    ANGLE_multi_draw,
    ANGLE_shader_pixel_local_storage,
    ANGLE_texture_multisample,
    APPLE_clip_distance,
    ARB_fragment_shader_interlock,
    ARB_texture_rectangle,
    ARM_shader_framebuffer_fetch,
    ARM_shader_framebuffer_fetch_depth_stencil,
    EXT_YUV_target,
    EXT_blend_func_extended,
    EXT_clip_cull_distance,
    EXT_conservative_depth,
    EXT_draw_buffers,
    EXT_frag_depth,
    EXT_geometry_shader,
    EXT_gpu_shader5,
    EXT_primitive_bounding_box,
    EXT_separate_shader_objects,
    EXT_shader_framebuffer_fetch,
    EXT_shader_framebuffer_fetch_non_coherent,
    EXT_shader_io_blocks,
    EXT_shader_non_constant_global_initializers,
    EXT_shader_texture_lod,
    EXT_shadow_samplers,
    EXT_tessellation_shader,
    EXT_texture_buffer,
    EXT_texture_cube_map_array,
    EXT_texture_query_lod,
    EXT_texture_shadow_lod,
    INTEL_fragment_shader_ordering,
    KHR_blend_equation_advanced,
    NV_EGL_stream_consumer_external,
    NV_fragment_shader_interlock,
    NV_shader_framebuffer_fetch,
    NV_shader_noperspective_interpolation,
    OES_EGL_image_external,
    OES_EGL_image_external_essl3,
    OES_geometry_shader,
    OES_gpu_shader5,
    OES_primitive_bounding_box,
    OES_sample_variables,
    OES_shader_image_atomic,
    OES_shader_io_blocks,
    OES_shader_multisample_interpolation,
    OES_standard_derivatives,
    OES_tessellation_shader,
    OES_texture_3D,
    OES_texture_buffer,
    OES_texture_cube_map_array,
    OES_texture_storage_multisample_2d_array,
    OVR_multiview,
    OVR_multiview2,
    WEBGL_video_texture,
};

enum TBehavior : uint8_t
{
    EBhRequire,
    EBhEnable,
    EBhWarn,
    EBhDisable,
    EBhUndefined
};

const char *GetExtensionNameString(TExtension extension);
TExtension GetExtensionByName(const char *extension);
bool CheckExtensionVersion(TExtension extension, int version);

const char *GetBehaviorString(TBehavior b);

// Mapping between extension id and behavior.
typedef std::map<TExtension, TBehavior> TExtensionBehavior;

bool IsExtensionEnabled(const TExtensionBehavior &extBehavior, TExtension extension);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_EXTENSIONBEHAVIOR_H_
