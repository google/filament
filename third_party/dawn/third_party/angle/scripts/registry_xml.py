#!/usr/bin/python3
#
# Copyright 2018 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# registry_xml.py:
#   Parses information from Khronos registry files..

# List of supported extensions. Add to this list to enable new extensions
# available in gl.xml.

import difflib
import os
import sys
import xml.etree.ElementTree as etree

from enum import Enum

khronos_xml_inputs = [
    '../third_party/EGL-Registry/src/api/egl.xml',
    '../third_party/OpenCL-Docs/src/xml/cl.xml',
    '../third_party/OpenGL-Registry/src/xml/gl.xml',
    '../third_party/OpenGL-Registry/src/xml/glx.xml',
    '../third_party/OpenGL-Registry/src/xml/wgl.xml',
]

angle_xml_inputs = [
    'gl_angle_ext.xml',
    'egl_angle_ext.xml',
    'registry_xml.py',
]

xml_inputs = sorted(khronos_xml_inputs + angle_xml_inputs)

# Notes on categories of extensions:
# 'Requestable' extensions are extensions that can be enabled with ANGLE_request_extension
# 'ES-Only' extensions are always implicitly enabled.
# 'Toggleable' extensions are like 'Requestable' except they can be also disabled.
# 'ANGLE' extensions are extensions that are not yet officially upstreamed to Khronos.
# We document those extensions in gl_angle_ext.xml instead of the canonical gl.xml.

angle_toggleable_extensions = [
    "GL_ANGLE_texture_rectangle",
]

angle_requestable_extensions = [
    "GL_ANGLE_base_vertex_base_instance",
    "GL_ANGLE_base_vertex_base_instance_shader_builtin",
    "GL_ANGLE_blob_cache",
    "GL_ANGLE_clip_cull_distance",
    "GL_ANGLE_compressed_texture_etc",
    "GL_ANGLE_copy_texture_3d",
    "GL_ANGLE_framebuffer_multisample",
    "GL_ANGLE_get_image",
    "GL_ANGLE_get_tex_level_parameter",
    "GL_ANGLE_logic_op",
    "GL_ANGLE_lossy_etc_decode",
    "GL_ANGLE_memory_object_flags",
    "GL_ANGLE_memory_object_fuchsia",
    "GL_ANGLE_memory_size",
    "GL_ANGLE_multi_draw",
    "GL_ANGLE_multiview_multisample",
    "GL_ANGLE_polygon_mode",
    "GL_ANGLE_provoking_vertex",
    "GL_ANGLE_read_only_depth_stencil_feedback_loops",
    "GL_ANGLE_renderability_validation",
    "GL_ANGLE_robust_fragment_shader_output",
    "GL_ANGLE_semaphore_fuchsia",
    "GL_ANGLE_shader_pixel_local_storage",
    "GL_ANGLE_shader_pixel_local_storage_coherent",
    "GL_ANGLE_stencil_texturing",
    "GL_ANGLE_texture_compression_dxt3",
    "GL_ANGLE_texture_compression_dxt5",
    "GL_ANGLE_texture_external_update",
    "GL_ANGLE_texture_multisample",
    "GL_ANGLE_vulkan_image",
    "GL_ANGLE_yuv_internal_format",
    "GL_CHROMIUM_color_buffer_float_rgb",
    "GL_CHROMIUM_color_buffer_float_rgba",
    "GL_CHROMIUM_lose_context",
    "GL_CHROMIUM_sync_query",
]

gles_requestable_extensions = [
    "GL_ANGLE_framebuffer_blit",
    "GL_ANGLE_instanced_arrays",
    "GL_ANGLE_pack_reverse_row_order",
    "GL_ANGLE_texture_usage",
    "GL_APPLE_clip_distance",
    "GL_ARB_sync",
    "GL_ARM_rgba8",
    "GL_ARM_shader_framebuffer_fetch",
    "GL_ARM_shader_framebuffer_fetch_depth_stencil",
    "GL_EXT_base_instance",
    "GL_EXT_blend_func_extended",
    "GL_EXT_blend_minmax",
    "GL_EXT_buffer_storage",
    "GL_EXT_clear_texture",
    "GL_EXT_clip_control",
    "GL_EXT_clip_cull_distance",
    "GL_EXT_color_buffer_float",
    "GL_EXT_color_buffer_half_float",
    "GL_EXT_compressed_ETC1_RGB8_sub_texture",
    "GL_EXT_conservative_depth",
    "GL_EXT_copy_image",
    "GL_EXT_depth_clamp",
    "GL_EXT_disjoint_timer_query",
    "GL_EXT_draw_buffers",
    "GL_EXT_draw_buffers_indexed",
    "GL_EXT_draw_elements_base_vertex",
    "GL_EXT_EGL_image_array",
    "GL_EXT_EGL_image_external_wrap_modes",
    "GL_EXT_EGL_image_storage",
    "GL_EXT_EGL_image_storage_compression",
    "GL_EXT_external_buffer",
    "GL_EXT_float_blend",
    "GL_EXT_frag_depth",
    "GL_EXT_geometry_shader",
    "GL_EXT_gpu_shader5",
    "GL_EXT_instanced_arrays",
    "GL_EXT_map_buffer_range",
    "GL_EXT_memory_object",
    "GL_EXT_memory_object_fd",
    "GL_EXT_multi_draw_indirect",
    "GL_EXT_multisampled_render_to_texture",
    "GL_EXT_multisampled_render_to_texture2",
    "GL_EXT_occlusion_query_boolean",
    "GL_EXT_polygon_offset_clamp",
    "GL_EXT_protected_textures",
    "GL_EXT_pvrtc_sRGB",
    "GL_EXT_read_format_bgra",
    "GL_EXT_render_snorm",
    "GL_EXT_semaphore",
    "GL_EXT_semaphore_fd",
    "GL_EXT_separate_depth_stencil",
    "GL_EXT_separate_shader_objects",
    "GL_EXT_shader_framebuffer_fetch",
    "GL_EXT_shader_framebuffer_fetch_non_coherent",
    "GL_EXT_shader_io_blocks",
    "GL_EXT_shader_non_constant_global_initializers",
    "GL_EXT_shader_texture_lod",
    "GL_EXT_shadow_samplers",
    "GL_EXT_sRGB",
    "GL_EXT_tessellation_shader",
    "GL_EXT_texture_border_clamp",
    "GL_EXT_texture_buffer",
    "GL_EXT_texture_compression_astc_decode_mode",
    "GL_EXT_texture_compression_astc_decode_mode_rgb9e5",
    "GL_EXT_texture_compression_bptc",
    "GL_EXT_texture_compression_dxt1",
    "GL_EXT_texture_compression_rgtc",
    "GL_EXT_texture_compression_s3tc",
    "GL_EXT_texture_compression_s3tc_srgb",
    "GL_EXT_texture_cube_map_array",
    "GL_EXT_texture_filter_anisotropic",
    "GL_EXT_texture_filter_minmax",
    "GL_EXT_texture_format_BGRA8888",
    "GL_EXT_texture_mirror_clamp_to_edge",
    "GL_EXT_texture_norm16",
    "GL_EXT_texture_query_lod",
    "GL_EXT_texture_rg",
    "GL_EXT_texture_shadow_lod",
    "GL_EXT_texture_sRGB_R8",
    "GL_EXT_texture_sRGB_RG8",
    "GL_EXT_texture_storage",
    "GL_EXT_texture_storage_compression",
    "GL_EXT_texture_type_2_10_10_10_REV",
    "GL_EXT_unpack_subimage",
    "GL_EXT_YUV_target",
    "GL_IMG_texture_compression_pvrtc",
    "GL_IMG_texture_compression_pvrtc2",
    "GL_KHR_parallel_shader_compile",
    "GL_KHR_texture_compression_astc_hdr",
    "GL_KHR_texture_compression_astc_ldr",
    "GL_KHR_texture_compression_astc_sliced_3d",
    "GL_MESA_framebuffer_flip_y",
    "GL_NV_depth_buffer_float2",
    "GL_NV_EGL_stream_consumer_external",
    "GL_NV_framebuffer_blit",
    "GL_NV_pack_subimage",
    "GL_NV_pixel_buffer_object",
    "GL_NV_polygon_mode",
    "GL_NV_read_depth",
    "GL_NV_read_depth_stencil",
    "GL_NV_read_stencil",
    "GL_NV_shader_noperspective_interpolation",
    "GL_OES_compressed_EAC_R11_signed_texture",
    "GL_OES_compressed_EAC_R11_unsigned_texture",
    "GL_OES_compressed_EAC_RG11_signed_texture",
    "GL_OES_compressed_EAC_RG11_unsigned_texture",
    "GL_OES_compressed_ETC1_RGB8_texture",
    "GL_OES_compressed_ETC2_punchthroughA_RGBA8_texture",
    "GL_OES_compressed_ETC2_punchthroughA_sRGB8_alpha_texture",
    "GL_OES_compressed_ETC2_RGB8_texture",
    "GL_OES_compressed_ETC2_RGBA8_texture",
    "GL_OES_compressed_ETC2_sRGB8_alpha8_texture",
    "GL_OES_compressed_ETC2_sRGB8_texture",
    "GL_OES_compressed_paletted_texture",
    "GL_OES_copy_image",
    "GL_OES_depth_texture_cube_map",
    "GL_OES_draw_buffers_indexed",
    "GL_OES_draw_elements_base_vertex",
    "GL_OES_EGL_image",
    "GL_OES_EGL_image_external",
    "GL_OES_EGL_image_external_essl3",
    "GL_OES_element_index_uint",
    "GL_OES_fbo_render_mipmap",
    "GL_OES_geometry_shader",
    "GL_OES_get_program_binary",
    "GL_OES_gpu_shader5",
    "GL_OES_mapbuffer",
    "GL_OES_required_internalformat",
    "GL_OES_rgb8_rgba8",
    "GL_OES_sample_shading",
    "GL_OES_sample_variables",
    "GL_OES_shader_image_atomic",
    "GL_OES_shader_io_blocks",
    "GL_OES_shader_multisample_interpolation",
    "GL_OES_standard_derivatives",
    "GL_OES_tessellation_shader",
    "GL_OES_texture_3D",
    "GL_OES_texture_border_clamp",
    "GL_OES_texture_buffer",
    "GL_OES_texture_compression_astc",
    "GL_OES_texture_cube_map_array",
    "GL_OES_texture_float",
    "GL_OES_texture_float_linear",
    "GL_OES_texture_half_float",
    "GL_OES_texture_half_float_linear",
    "GL_OES_texture_npot",
    "GL_OES_texture_stencil8",
    "GL_OES_texture_storage_multisample_2d_array",
    "GL_OES_vertex_array_object",
    "GL_OES_vertex_half_float",
    "GL_OES_vertex_type_10_10_10_2",
    "GL_OVR_multiview",
    "GL_OVR_multiview2",
    "GL_QCOM_framebuffer_foveated",
    "GL_QCOM_render_shared_exponent",
    "GL_QCOM_shading_rate",
    "GL_QCOM_texture_foveated",
    "GL_QCOM_tiled_rendering",
    "GL_WEBGL_video_texture",
]

angle_es_only_extensions = [
    "GL_ANGLE_client_arrays",
    "GL_ANGLE_get_serialized_context_string",
    "GL_ANGLE_program_binary",
    "GL_ANGLE_program_binary_readiness_query",
    "GL_ANGLE_program_cache_control",
    "GL_ANGLE_relaxed_vertex_attribute_type",
    "GL_ANGLE_request_extension",
    "GL_ANGLE_rgbx_internal_format",
    "GL_ANGLE_robust_client_memory",
    "GL_ANGLE_robust_resource_initialization",
    "GL_ANGLE_shader_binary",
    "GL_ANGLE_webgl_compatibility",
    "GL_CHROMIUM_bind_generates_resource",
    "GL_CHROMIUM_bind_uniform_location",
    "GL_CHROMIUM_copy_compressed_texture",
    "GL_CHROMIUM_copy_texture",
    "GL_CHROMIUM_framebuffer_mixed_samples",
]

gles_es_only_extensions = [
    "GL_AMD_performance_monitor",
    "GL_ANDROID_extension_pack_es31a",
    "GL_ANGLE_depth_texture",
    "GL_ANGLE_translated_shader_source",
    "GL_EXT_debug_label",
    "GL_EXT_debug_marker",
    "GL_EXT_discard_framebuffer",
    "GL_EXT_multisample_compatibility",
    "GL_EXT_primitive_bounding_box",
    "GL_EXT_robustness",
    "GL_EXT_sRGB_write_control",
    "GL_EXT_texture_format_sRGB_override",
    "GL_EXT_texture_sRGB_decode",
    "GL_KHR_blend_equation_advanced",
    "GL_KHR_blend_equation_advanced_coherent",
    "GL_KHR_debug",
    "GL_KHR_no_error",
    "GL_KHR_robust_buffer_access_behavior",
    "GL_KHR_robustness",
    "GL_NV_fence",
    "GL_NV_robustness_video_memory_purge",
    "GL_OES_depth24",
    "GL_OES_depth32",
    "GL_OES_depth_texture",
    "GL_OES_EGL_sync",
    "GL_OES_packed_depth_stencil",
    "GL_OES_primitive_bounding_box",
    "GL_OES_surfaceless_context",
]

# ES1 (Possibly the min set of extensions needed by Android)
gles1_extensions = [
    "GL_OES_blend_subtract",
    "GL_OES_draw_texture",
    "GL_OES_framebuffer_object",
    "GL_OES_matrix_palette",
    "GL_OES_point_size_array",
    "GL_OES_point_sprite",
    "GL_OES_query_matrix",
    "GL_OES_texture_cube_map",
    "GL_OES_texture_mirrored_repeat",
]


def check_sorted(name, l):
    unidiff = difflib.unified_diff(l, sorted(l, key=str.casefold), 'unsorted', 'sorted')
    diff_lines = list(unidiff)
    assert not diff_lines, '\n\nPlease sort "%s":\n%s' % (name, '\n'.join(diff_lines))


angle_extensions = angle_requestable_extensions + angle_es_only_extensions + angle_toggleable_extensions
gles_extensions = gles_requestable_extensions + gles_es_only_extensions
supported_extensions = sorted(angle_extensions + gles1_extensions + gles_extensions)

assert len(supported_extensions) == len(set(supported_extensions)), 'Duplicates in extension list'
check_sorted('angle_requestable_extensions', angle_requestable_extensions)
check_sorted('angle_es_only_extensions', angle_es_only_extensions)
check_sorted('angle_toggleable_extensions', angle_toggleable_extensions)
check_sorted('gles_requestable_extensions', gles_requestable_extensions)
check_sorted('gles_es_only_extensions', gles_es_only_extensions)
check_sorted('gles_extensions', gles1_extensions)

supported_egl_extensions = [
    "EGL_ANDROID_blob_cache",
    "EGL_ANDROID_create_native_client_buffer",
    "EGL_ANDROID_framebuffer_target",
    "EGL_ANDROID_get_frame_timestamps",
    "EGL_ANDROID_get_native_client_buffer",
    "EGL_ANDROID_native_fence_sync",
    "EGL_ANDROID_presentation_time",
    "EGL_ANGLE_create_surface_swap_interval",
    "EGL_ANGLE_d3d_share_handle_client_buffer",
    "EGL_ANGLE_device_creation",
    "EGL_ANGLE_device_d3d",
    "EGL_ANGLE_device_d3d11",
    "EGL_ANGLE_device_d3d9",
    "EGL_ANGLE_device_vulkan",
    "EGL_ANGLE_display_semaphore_share_group",
    "EGL_ANGLE_display_texture_share_group",
    "EGL_ANGLE_external_context_and_surface",
    "EGL_ANGLE_feature_control",
    "EGL_ANGLE_ggp_stream_descriptor",
    "EGL_ANGLE_memory_usage_report",
    "EGL_ANGLE_metal_create_context_ownership_identity",
    "EGL_ANGLE_metal_shared_event_sync",
    "EGL_ANGLE_no_error",
    "EGL_ANGLE_power_preference",
    "EGL_ANGLE_prepare_swap_buffers",
    "EGL_ANGLE_program_cache_control",
    "EGL_ANGLE_query_surface_pointer",
    "EGL_ANGLE_stream_producer_d3d_texture",
    "EGL_ANGLE_surface_d3d_texture_2d_share_handle",
    "EGL_ANGLE_swap_with_frame_token",
    "EGL_ANGLE_sync_control_rate",
    "EGL_ANGLE_vulkan_image",
    "EGL_ANGLE_wait_until_work_scheduled",
    "EGL_ANGLE_window_fixed_size",
    "EGL_CHROMIUM_sync_control",
    "EGL_EXT_create_context_robustness",
    "EGL_EXT_device_query",
    "EGL_EXT_gl_colorspace_bt2020_hlg",
    "EGL_EXT_gl_colorspace_bt2020_linear",
    "EGL_EXT_gl_colorspace_bt2020_pq",
    "EGL_EXT_gl_colorspace_display_p3",
    "EGL_EXT_gl_colorspace_display_p3_linear",
    "EGL_EXT_gl_colorspace_display_p3_passthrough",
    "EGL_EXT_gl_colorspace_scrgb",
    "EGL_EXT_gl_colorspace_scrgb_linear",
    "EGL_EXT_image_dma_buf_import",
    "EGL_EXT_image_dma_buf_import_modifiers",
    "EGL_EXT_image_gl_colorspace",
    "EGL_EXT_pixel_format_float",
    "EGL_EXT_platform_base",
    "EGL_EXT_platform_device",
    "EGL_EXT_protected_content",
    "EGL_EXT_surface_compression",
    "EGL_IMG_context_priority",
    "EGL_KHR_debug",
    "EGL_KHR_fence_sync",
    "EGL_KHR_gl_colorspace",
    "EGL_KHR_image",
    "EGL_KHR_lock_surface3",
    "EGL_KHR_mutable_render_buffer",
    "EGL_KHR_no_config_context",
    "EGL_KHR_partial_update",
    "EGL_KHR_reusable_sync",
    "EGL_KHR_stream",
    "EGL_KHR_stream_consumer_gltexture",
    "EGL_KHR_surfaceless_context",
    "EGL_KHR_swap_buffers_with_damage",
    "EGL_KHR_wait_sync",
    "EGL_NV_post_sub_buffer",
    "EGL_NV_stream_consumer_gltexture_yuv",
]

check_sorted('supported_egl_extensions', supported_egl_extensions)

supported_cl_extensions = [
    # Since OpenCL 1.1
    "cl_khr_byte_addressable_store",
    "cl_khr_global_int32_base_atomics",
    "cl_khr_global_int32_extended_atomics",
    "cl_khr_local_int32_base_atomics",
    "cl_khr_local_int32_extended_atomics",

    # OpenCL 2.0 - 2.2
    "cl_khr_3d_image_writes",
    "cl_khr_depth_images",
    "cl_khr_image2d_from_buffer",

    # Optional
    "cl_khr_extended_versioning",
    "cl_khr_fp64",
    "cl_khr_icd",
    "cl_khr_int64_base_atomics",
    "cl_khr_int64_extended_atomics",
]

# Strip these suffixes from Context entry point names. NV is excluded (for now).
strip_suffixes = ["AMD", "ANDROID", "ANGLE", "CHROMIUM", "EXT", "KHR", "OES", "OVR", "QCOM"]
check_sorted('strip_suffixes', strip_suffixes)

# The EGL_ANGLE_explicit_context extension is generated differently from other extensions.
# Toggle generation here.
support_EGL_ANGLE_explicit_context = True

# Group names that appear in command/param, but not present in groups/group
unsupported_enum_group_names = {
    'GetMultisamplePNameNV',
    'BufferPNameARB',
    'BufferPointerNameARB',
    'VertexAttribPointerPropertyARB',
    'VertexAttribPropertyARB',
    'FenceParameterNameNV',
    'FenceConditionNV',
    'BufferPointerNameARB',
    'MatrixIndexPointerTypeARB',
    'PointParameterNameARB',
    'ClampColorTargetARB',
    'ClampColorModeARB',
}

# Versions (major, minor). Note that GLES intentionally places 1.0 last.
GLES_VERSIONS = [(2, 0), (3, 0), (3, 1), (3, 2), (1, 0)]
EGL_VERSIONS = [(1, 0), (1, 1), (1, 2), (1, 3), (1, 4), (1, 5)]
CL_VERSIONS = [(1, 0), (1, 1), (1, 2), (2, 0), (2, 1), (2, 2), (3, 0)]


# API types
class apis:
    GL = 'GL'
    GLES = 'GLES'
    WGL = 'WGL'
    GLX = 'GLX'
    EGL = 'EGL'
    CL = 'CL'

# For GLenum types
api_enums = {apis.GL: 'BigGLEnum', apis.GLES: 'GLESEnum'}
default_enum_group_name = 'AllEnums'


def script_relative(path):
    return os.path.join(os.path.dirname(sys.argv[0]), path)


def path_to(folder, file):
    return os.path.join(script_relative(".."), "src", folder, file)


def strip_api_prefix(cmd_name):
    return cmd_name.lstrip("cwegl")


def find_xml_input(xml_file):
    for found_xml in xml_inputs:
        if found_xml == xml_file or found_xml.endswith('/' + xml_file):
            return found_xml
    raise Exception('Could not find XML input: ' + xml_file)


def get_cmd_name(command_node):
    proto = command_node.find('proto')
    cmd_name = proto.find('name').text
    return cmd_name


class CommandNames:

    def __init__(self):
        self.command_names = {}

    def get_commands(self, version):
        return self.command_names[version]

    def get_all_commands(self):
        cmd_names = []
        # Combine all the version lists into a single list
        for version, version_cmd_names in sorted(self.command_names.items()):
            cmd_names += version_cmd_names

        return cmd_names

    def add_commands(self, version, commands):
        # Add key if it doesn't exist
        if version not in self.command_names:
            self.command_names[version] = []
        # Add the commands that aren't duplicates
        self.command_names[version] += commands


class RegistryXML:

    def __init__(self, xml_file, ext_file=None):
        tree = etree.parse(script_relative(find_xml_input(xml_file)))
        self.root = tree.getroot()
        if (ext_file):
            self._AppendANGLEExts(find_xml_input(ext_file))
        self.all_commands = self.root.findall('commands/command')
        self.all_cmd_names = CommandNames()
        self.commands = {}

    def _AppendANGLEExts(self, ext_file):
        angle_ext_tree = etree.parse(script_relative(ext_file))
        angle_ext_root = angle_ext_tree.getroot()

        insertion_point = self.root.findall("./types")[0]
        for t in angle_ext_root.iter('types'):
            insertion_point.extend(t)

        insertion_point = self.root.findall("./commands")[0]
        for command in angle_ext_root.iter('commands'):
            insertion_point.extend(command)

        insertion_point = self.root.findall("./extensions")[0]
        for extension in angle_ext_root.iter('extensions'):
            insertion_point.extend(extension)

        insertion_point = self.root
        for enums in angle_ext_root.iter('enums'):
            insertion_point.append(enums)

    def AddCommands(self, feature_name, annotation):
        xpath = ".//feature[@name='%s']//command" % feature_name
        commands = [cmd.attrib['name'] for cmd in self.root.findall(xpath)]

        # Remove commands that have already been processed
        current_cmds = self.all_cmd_names.get_all_commands()
        commands = [cmd for cmd in commands if cmd not in current_cmds]

        self.all_cmd_names.add_commands(annotation, commands)
        self.commands[annotation] = commands

    def _ClassifySupport(self, extension):
        supported = extension.attrib['supported']
        # Desktop GL extensions exposed in ANGLE GLES for Chrome.
        if extension.attrib['name'] in ['GL_ARB_sync', 'GL_NV_robustness_video_memory_purge']:
            supported += "|gles2"
        if 'gles2' in supported:
            return 'gl2ext'
        elif 'gles1' in supported:
            return 'glext'
        elif 'egl' in supported:
            return 'eglext'
        elif 'wgl' in supported:
            return 'wglext'
        elif 'glx' in supported:
            return 'glxext'
        elif 'cl' in supported:
            return 'clext'
        else:
            assert False, 'Cannot classify support for %s: %s' % (extension.attrib['name'],
                                                                  supported)
            return 'unknown'

    def AddExtensionCommands(self, supported_extensions, apis):
        # Use a first step to run through the extensions so we can generate them
        # in sorted order.
        self.ext_data = {}
        self.ext_dupes = {}
        ext_annotations = {}

        for extension in self.root.findall("extensions/extension"):
            extension_name = extension.attrib['name']
            if not extension_name in supported_extensions:
                continue

            ext_annotations[extension_name] = self._ClassifySupport(extension)

            ext_cmd_names = []

            # There's an extra step here to filter out 'api=gl' extensions. This
            # is necessary for handling KHR extensions, which have separate entry
            # point signatures (without the suffix) for desktop GL. Note that this
            # extra step is necessary because of Etree's limited Xpath support.
            for require in extension.findall('require'):
                if 'api' in require.attrib and require.attrib['api'] not in apis:
                    continue

                # A special case for EXT_texture_storage
                filter_out_comment = "Supported only if GL_EXT_direct_state_access is supported"
                if 'comment' in require.attrib and require.attrib['comment'] == filter_out_comment:
                    continue

                extension_commands = require.findall('command')
                ext_cmd_names += [command.attrib['name'] for command in extension_commands]

            self.ext_data[extension_name] = sorted(ext_cmd_names)

        for extension_name, ext_cmd_names in sorted(self.ext_data.items()):

            # Detect and filter duplicate extensions.
            dupes = []
            for ext_cmd in ext_cmd_names:
                if ext_cmd in self.all_cmd_names.get_all_commands():
                    dupes.append(ext_cmd)

            for dupe in dupes:
                ext_cmd_names.remove(dupe)

            self.ext_data[extension_name] = sorted(ext_cmd_names)
            self.ext_dupes[extension_name] = dupes
            self.all_cmd_names.add_commands(ext_annotations[extension_name], ext_cmd_names)

    def GetEnums(self, override_prefix=None):
        cmd_names = []
        for cmd in self.all_cmd_names.get_all_commands():
            stripped = strip_api_prefix(cmd)
            prefix = override_prefix or cmd[:(len(cmd) - len(stripped))]
            cmd_names.append(
                ('%s%s' % (prefix.upper(), stripped), '%s%s' % (prefix.lower(), stripped)))
        return cmd_names


class EntryPoints:

    def __init__(self, api, xml, commands):
        self.api = api
        self._cmd_info = []

        for command_node in xml.all_commands:
            cmd_name = get_cmd_name(command_node)

            if api == apis.WGL:
                cmd_name = cmd_name if cmd_name.startswith('wgl') else 'wgl' + cmd_name

            if cmd_name not in commands:
                continue

            param_text = ["".join(param.itertext()) for param in command_node.findall('param')]

            # Treat (void) as ()
            if len(param_text) == 1 and param_text[0].strip() == 'void':
                param_text = []

            proto = command_node.find('proto')
            proto_text = "".join(proto.itertext())

            self._cmd_info.append((cmd_name, command_node, param_text, proto_text))

    def get_infos(self):
        return self._cmd_info


def GetEGL():
    egl = RegistryXML('egl.xml', 'egl_angle_ext.xml')
    for major_version, minor_version in EGL_VERSIONS:
        version = "%d_%d" % (major_version, minor_version)
        name_prefix = "EGL_VERSION_"
        feature_name = "%s%s" % (name_prefix, version)
        egl.AddCommands(feature_name, version)
    egl.AddExtensionCommands(supported_egl_extensions, ['egl'])
    return egl


def GetGLES():
    gles = RegistryXML('gl.xml', 'gl_angle_ext.xml')
    for major_version, minor_version in GLES_VERSIONS:
        version = "{}_{}".format(major_version, minor_version)
        name_prefix = "GL_ES_VERSION_"
        if major_version == 1:
            name_prefix = "GL_VERSION_ES_CM_"
        feature_name = "{}{}".format(name_prefix, version)
        gles.AddCommands(feature_name, version)
    gles.AddExtensionCommands(supported_extensions, ['gles2', 'gles1'])
    return gles
