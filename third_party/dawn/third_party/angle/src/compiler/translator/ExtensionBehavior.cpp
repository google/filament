//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ExtensionBehavior.cpp: Extension name enumeration and data structures for storing extension
// behavior.

#include "compiler/translator/ExtensionBehavior.h"

#include "common/debug.h"

#include <string.h>

// clang-format off
// Extension Name, Min ESSL Version, Max ESSL Version
//
// Note that OES_EGL_image_external and OES_texture_3D are ESSL 100 only extensions, but one app has
// been found that uses them on GLSL 310.  http://issuetracker.google.com/285871779
//
// Note that OES_texture_storage_multisample_2d_array officially requires ESSL 310
// but ANGLE is able to support it with ESSL 300 in most cases.
#define LIST_EXTENSIONS(OP)                                      \
    OP(ANDROID_extension_pack_es31a,                   310, 320) \
    OP(ANGLE_base_vertex_base_instance_shader_builtin, 300, 320) \
    OP(ANGLE_clip_cull_distance,                       300, 320) \
    OP(ANGLE_multi_draw,                               100, 320) \
    OP(ANGLE_shader_pixel_local_storage,               300, 320) \
    OP(ANGLE_texture_multisample,                      300, 320) \
    OP(APPLE_clip_distance,                            100, 320) \
    OP(ARB_texture_rectangle,                          100, 320) \
    OP(ARM_shader_framebuffer_fetch,                   100, 320) \
    OP(ARM_shader_framebuffer_fetch_depth_stencil,     100, 320) \
    OP(EXT_blend_func_extended,                        100, 320) \
    OP(EXT_clip_cull_distance,                         300, 320) \
    OP(EXT_conservative_depth,                         300, 320) \
    OP(EXT_draw_buffers,                               100, 100) \
    OP(EXT_frag_depth,                                 100, 100) \
    OP(EXT_geometry_shader,                            310, 320) \
    OP(OES_geometry_shader,                            310, 320) \
    OP(OES_shader_io_blocks,                           310, 320) \
    OP(EXT_shader_io_blocks,                           310, 320) \
    OP(EXT_gpu_shader5,                                310, 320) \
    OP(OES_gpu_shader5,                                310, 320) \
    OP(EXT_primitive_bounding_box,                     310, 320) \
    OP(OES_primitive_bounding_box,                     310, 320) \
    OP(EXT_separate_shader_objects,                    100, 320) \
    OP(EXT_shader_framebuffer_fetch,                   100, 320) \
    OP(EXT_shader_framebuffer_fetch_non_coherent,      100, 320) \
    OP(EXT_shader_non_constant_global_initializers,    100, 320) \
    OP(EXT_shader_texture_lod,                         100, 100) \
    OP(EXT_shadow_samplers,                            100, 100) \
    OP(EXT_tessellation_shader,                        310, 320) \
    OP(OES_tessellation_shader,                        310, 320) \
    OP(EXT_texture_buffer,                             310, 320) \
    OP(EXT_texture_cube_map_array,                     310, 320) \
    OP(EXT_texture_query_lod,                          300, 320) \
    OP(EXT_texture_shadow_lod,                         300, 320) \
    OP(EXT_YUV_target,                                 300, 320) \
    OP(KHR_blend_equation_advanced,                    100, 320) \
    OP(NV_EGL_stream_consumer_external,                100, 320) \
    OP(NV_shader_framebuffer_fetch,                    100, 100) \
    OP(NV_shader_noperspective_interpolation,          300, 320) \
    OP(OES_EGL_image_external,                         100, 310) \
    OP(OES_EGL_image_external_essl3,                   300, 320) \
    OP(OES_sample_variables,                           300, 320) \
    OP(OES_shader_multisample_interpolation,           300, 320) \
    OP(OES_shader_image_atomic,                        310, 320) \
    OP(OES_standard_derivatives,                       100, 100) \
    OP(OES_texture_3D,                                 100, 310) \
    OP(OES_texture_buffer,                             310, 320) \
    OP(OES_texture_cube_map_array,                     310, 320) \
    OP(OES_texture_storage_multisample_2d_array,       300, 320) \
    OP(OVR_multiview,                                  300, 320) \
    OP(OVR_multiview2,                                 300, 320) \
    OP(WEBGL_video_texture,                            100, 320)
// clang-format on

namespace sh
{

#define RETURN_EXTENSION_NAME_CASE(ext, min_version, max_version) \
    case TExtension::ext:                                         \
        return "GL_" #ext;

const char *GetExtensionNameString(TExtension extension)
{
    switch (extension)
    {
        LIST_EXTENSIONS(RETURN_EXTENSION_NAME_CASE)
        default:
            UNREACHABLE();
            return "";
    }
}

#define RETURN_EXTENSION_IF_NAME_MATCHES(ext, min_version, max_version) \
    if (strcmp(extWithoutGLPrefix, #ext) == 0)                          \
    {                                                                   \
        return TExtension::ext;                                         \
    }

TExtension GetExtensionByName(const char *extension)
{
    // If first characters of the extension don't equal "GL_", early out.
    if (strncmp(extension, "GL_", 3) != 0)
    {
        return TExtension::UNDEFINED;
    }
    const char *extWithoutGLPrefix = extension + 3;

    LIST_EXTENSIONS(RETURN_EXTENSION_IF_NAME_MATCHES)

    return TExtension::UNDEFINED;
}

#define RETURN_VERSION_CHECK(ext, min_version, max_version) \
    case TExtension::ext:                                   \
        return (version >= min_version) && (version <= max_version);

bool CheckExtensionVersion(TExtension extension, int version)
{
    switch (extension)
    {
        LIST_EXTENSIONS(RETURN_VERSION_CHECK)
        default:
            UNREACHABLE();
            return false;
    }
}

const char *GetBehaviorString(TBehavior b)
{
    switch (b)
    {
        case EBhRequire:
            return "require";
        case EBhEnable:
            return "enable";
        case EBhWarn:
            return "warn";
        case EBhDisable:
            return "disable";
        default:
            return nullptr;
    }
}

bool IsExtensionEnabled(const TExtensionBehavior &extBehavior, TExtension extension)
{
    ASSERT(extension != TExtension::UNDEFINED);
    auto iter = extBehavior.find(extension);
    return iter != extBehavior.end() &&
           (iter->second == EBhEnable || iter->second == EBhRequire || iter->second == EBhWarn);
}

}  // namespace sh
