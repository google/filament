//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/Initialize.h"

namespace sh
{

void InitExtensionBehavior(const ShBuiltInResources &resources, TExtensionBehavior &extBehavior)
{
    if (resources.OES_standard_derivatives)
    {
        extBehavior[TExtension::OES_standard_derivatives] = EBhUndefined;
    }
    if (resources.OES_EGL_image_external)
    {
        extBehavior[TExtension::OES_EGL_image_external] = EBhUndefined;
    }
    if (resources.OES_EGL_image_external_essl3)
    {
        extBehavior[TExtension::OES_EGL_image_external_essl3] = EBhUndefined;
    }
    if (resources.NV_EGL_stream_consumer_external)
    {
        extBehavior[TExtension::NV_EGL_stream_consumer_external] = EBhUndefined;
    }
    if (resources.ARB_texture_rectangle)
    {
        // Special: ARB_texture_rectangle extension does not follow the standard for #extension
        // directives - it is enabled by default. An extension directive may still disable it.
        extBehavior[TExtension::ARB_texture_rectangle] = EBhEnable;
    }
    if (resources.EXT_blend_func_extended)
    {
        extBehavior[TExtension::EXT_blend_func_extended] = EBhUndefined;
    }
    if (resources.EXT_conservative_depth)
    {
        extBehavior[TExtension::EXT_conservative_depth] = EBhUndefined;
    }
    if (resources.EXT_draw_buffers)
    {
        extBehavior[TExtension::EXT_draw_buffers] = EBhUndefined;
    }
    if (resources.EXT_frag_depth)
    {
        extBehavior[TExtension::EXT_frag_depth] = EBhUndefined;
    }
    if (resources.EXT_primitive_bounding_box)
    {
        extBehavior[TExtension::EXT_primitive_bounding_box] = EBhUndefined;
    }
    if (resources.OES_primitive_bounding_box)
    {
        extBehavior[TExtension::OES_primitive_bounding_box] = EBhUndefined;
    }
    if (resources.EXT_separate_shader_objects)
    {
        extBehavior[TExtension::EXT_separate_shader_objects] = EBhUndefined;
    }
    if (resources.EXT_shader_texture_lod)
    {
        extBehavior[TExtension::EXT_shader_texture_lod] = EBhUndefined;
    }
    if (resources.EXT_shader_framebuffer_fetch)
    {
        extBehavior[TExtension::EXT_shader_framebuffer_fetch] = EBhUndefined;
    }
    if (resources.EXT_shader_framebuffer_fetch_non_coherent)
    {
        extBehavior[TExtension::EXT_shader_framebuffer_fetch_non_coherent] = EBhUndefined;
    }
    if (resources.NV_shader_framebuffer_fetch)
    {
        extBehavior[TExtension::NV_shader_framebuffer_fetch] = EBhUndefined;
    }
    if (resources.NV_shader_noperspective_interpolation)
    {
        extBehavior[TExtension::NV_shader_noperspective_interpolation] = EBhUndefined;
    }
    if (resources.ARM_shader_framebuffer_fetch)
    {
        extBehavior[TExtension::ARM_shader_framebuffer_fetch] = EBhUndefined;
    }
    if (resources.ARM_shader_framebuffer_fetch_depth_stencil)
    {
        extBehavior[TExtension::ARM_shader_framebuffer_fetch_depth_stencil] = EBhUndefined;
    }
    if (resources.OVR_multiview)
    {
        extBehavior[TExtension::OVR_multiview] = EBhUndefined;
    }
    if (resources.OVR_multiview2)
    {
        extBehavior[TExtension::OVR_multiview2] = EBhUndefined;
    }
    if (resources.EXT_YUV_target)
    {
        extBehavior[TExtension::EXT_YUV_target] = EBhUndefined;
    }
    if (resources.EXT_geometry_shader)
    {
        extBehavior[TExtension::EXT_geometry_shader] = EBhUndefined;
    }
    if (resources.OES_geometry_shader)
    {
        extBehavior[TExtension::OES_geometry_shader] = EBhUndefined;
    }
    if (resources.OES_shader_io_blocks)
    {
        extBehavior[TExtension::OES_shader_io_blocks] = EBhUndefined;
    }
    if (resources.EXT_shader_io_blocks)
    {
        extBehavior[TExtension::EXT_shader_io_blocks] = EBhUndefined;
    }
    if (resources.EXT_gpu_shader5)
    {
        extBehavior[TExtension::EXT_gpu_shader5] = EBhUndefined;
    }
    if (resources.OES_gpu_shader5)
    {
        extBehavior[TExtension::OES_gpu_shader5] = EBhUndefined;
    }
    if (resources.EXT_shader_non_constant_global_initializers)
    {
        extBehavior[TExtension::EXT_shader_non_constant_global_initializers] = EBhUndefined;
    }
    if (resources.OES_texture_storage_multisample_2d_array)
    {
        extBehavior[TExtension::OES_texture_storage_multisample_2d_array] = EBhUndefined;
    }
    if (resources.OES_texture_3D)
    {
        extBehavior[TExtension::OES_texture_3D] = EBhUndefined;
    }
    if (resources.ANGLE_shader_pixel_local_storage)
    {
        extBehavior[TExtension::ANGLE_shader_pixel_local_storage] = EBhUndefined;
    }
    if (resources.ANGLE_texture_multisample)
    {
        extBehavior[TExtension::ANGLE_texture_multisample] = EBhUndefined;
    }
    if (resources.ANGLE_multi_draw)
    {
        extBehavior[TExtension::ANGLE_multi_draw] = EBhUndefined;
    }
    if (resources.ANGLE_base_vertex_base_instance_shader_builtin)
    {
        extBehavior[TExtension::ANGLE_base_vertex_base_instance_shader_builtin] = EBhUndefined;
    }
    if (resources.WEBGL_video_texture)
    {
        extBehavior[TExtension::WEBGL_video_texture] = EBhUndefined;
    }
    if (resources.APPLE_clip_distance)
    {
        extBehavior[TExtension::APPLE_clip_distance] = EBhUndefined;
    }
    if (resources.OES_texture_cube_map_array)
    {
        extBehavior[TExtension::OES_texture_cube_map_array] = EBhUndefined;
    }
    if (resources.EXT_texture_cube_map_array)
    {
        extBehavior[TExtension::EXT_texture_cube_map_array] = EBhUndefined;
    }
    if (resources.EXT_texture_query_lod)
    {
        extBehavior[TExtension::EXT_texture_query_lod] = EBhUndefined;
    }
    if (resources.EXT_texture_shadow_lod)
    {
        extBehavior[TExtension::EXT_texture_shadow_lod] = EBhUndefined;
    }
    if (resources.EXT_shadow_samplers)
    {
        extBehavior[TExtension::EXT_shadow_samplers] = EBhUndefined;
    }
    if (resources.OES_shader_multisample_interpolation)
    {
        extBehavior[TExtension::OES_shader_multisample_interpolation] = EBhUndefined;
    }
    if (resources.OES_shader_image_atomic)
    {
        extBehavior[TExtension::OES_shader_image_atomic] = EBhUndefined;
    }
    if (resources.EXT_tessellation_shader)
    {
        extBehavior[TExtension::EXT_tessellation_shader] = EBhUndefined;
    }
    if (resources.OES_tessellation_shader)
    {
        extBehavior[TExtension::OES_tessellation_shader] = EBhUndefined;
    }
    if (resources.OES_texture_buffer)
    {
        extBehavior[TExtension::OES_texture_buffer] = EBhUndefined;
    }
    if (resources.EXT_texture_buffer)
    {
        extBehavior[TExtension::EXT_texture_buffer] = EBhUndefined;
    }
    if (resources.OES_sample_variables)
    {
        extBehavior[TExtension::OES_sample_variables] = EBhUndefined;
    }
    if (resources.EXT_clip_cull_distance)
    {
        extBehavior[TExtension::EXT_clip_cull_distance] = EBhUndefined;
    }
    if (resources.ANGLE_clip_cull_distance)
    {
        extBehavior[TExtension::ANGLE_clip_cull_distance] = EBhUndefined;
    }
    if (resources.ANDROID_extension_pack_es31a)
    {
        extBehavior[TExtension::ANDROID_extension_pack_es31a] = EBhUndefined;
    }
    if (resources.KHR_blend_equation_advanced)
    {
        extBehavior[TExtension::KHR_blend_equation_advanced] = EBhUndefined;
    }
}

void ResetExtensionBehavior(const ShBuiltInResources &resources,
                            TExtensionBehavior &extBehavior,
                            const ShCompileOptions &compileOptions)
{
    for (auto &ext : extBehavior)
    {
        ext.second = EBhUndefined;
    }
    if (resources.ARB_texture_rectangle)
    {
        if (compileOptions.disableARBTextureRectangle)
        {
            // Remove ARB_texture_rectangle so it can't be enabled by extension directives.
            extBehavior.erase(TExtension::ARB_texture_rectangle);
        }
        else
        {
            // Restore ARB_texture_rectangle in case it was removed during an earlier reset.  As
            // noted above, it doesn't follow the standard for extension directives and is
            // enabled by default.
            extBehavior[TExtension::ARB_texture_rectangle] = EBhEnable;
        }
    }
}

}  // namespace sh
