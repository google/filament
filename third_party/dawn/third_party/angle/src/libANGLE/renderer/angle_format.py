#!/usr/bin/python3
# Copyright 2016 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# angle_format.py:
#  Utils for ANGLE formats.

import json
import os
import re

kChannels = "ABDEGLRSX"


def get_angle_format_map_abs_path():
    return os.path.join(os.path.dirname(os.path.realpath(__file__)), 'angle_format_map.json')


def reject_duplicate_keys(pairs):
    found_keys = {}
    for key, value in pairs:
        if key in found_keys:
            raise ValueError("duplicate key: %r" % (key,))
        else:
            found_keys[key] = value
    return found_keys


def load_json(path):
    with open(path) as map_file:
        return json.loads(map_file.read(), object_pairs_hook=reject_duplicate_keys)


def load_forward_table(path, key=None):
    pairs = load_json(path)
    if key is not None:
        pairs = pairs[key]
    reject_duplicate_keys(pairs)
    return {gl: angle for gl, angle in pairs}


def load_inverse_table(path):
    pairs = load_json(path)
    reject_duplicate_keys(pairs)
    for x in range(0, 8):
        pairs.append(("GL_NONE", "EXTERNAL" + str(x)))
    return {angle: gl for gl, angle in pairs}


def load_without_override():
    map_path = get_angle_format_map_abs_path()
    return load_forward_table(map_path)


def load_with_override(override_path):
    results = load_without_override()
    overrides = load_json(override_path)

    for k, v in sorted(overrides.items()):
        results[k] = v

    return results


def get_all_angle_formats():
    map_path = get_angle_format_map_abs_path()
    return load_inverse_table(map_path).keys()


def get_component_type(format_id):
    if "SNORM" in format_id:
        return "snorm"
    elif "UNORM" in format_id:
        return "unorm"
    elif "FLOAT" in format_id:
        return "float"
    elif "FIXED" in format_id:
        return "float"
    elif "UINT" in format_id:
        return "uint"
    elif "SINT" in format_id:
        return "int"
    elif "USCALED" in format_id:
        return "uint"
    elif "SSCALED" in format_id:
        return "int"
    elif format_id == "NONE":
        return "none"
    elif "SRGB" in format_id:
        return "unorm"
    elif "TYPELESS" in format_id:
        return "unorm"
    elif "EXTERNAL" in format_id:
        return "unorm"
    elif format_id == "R9G9B9E5_SHAREDEXP":
        return "float"
    else:
        raise ValueError("Unknown component type for " + format_id)


def get_channel_tokens(format_id):
    if 'EXTERNAL' in format_id:
        return ['R8', 'G8', 'B8', 'A8']
    r = re.compile(r'([' + kChannels + '][\d]+)')
    return list(filter(r.match, r.split(format_id)))


def get_channels(format_id):
    channels = ''
    tokens = get_channel_tokens(format_id)
    if len(tokens) == 0:
        return None
    for token in tokens:
        channels += token[0].lower()

    return channels


def get_bits(format_id):
    bits = {}
    if "_RED_" in format_id:
        # BC4
        bits["R"] = 16
    elif "_RG_" in format_id:
        # BC5
        bits["R"] = bits["G"] = 16
    elif "_RGB_" in format_id:
        # BC1-3, BC6H, PVRTC
        bits["R"] = bits["G"] = bits["B"] = 16 if "BC6H" in format_id else 8
    elif "_RGBA_" in format_id or "ASTC_" in format_id:
        # ASTC, BC7, PVRTC
        bits["R"] = bits["G"] = bits["B"] = bits["A"] = 8
    else:
        tokens = get_channel_tokens(format_id)
        for token in tokens:
            bits[token[0]] = int(token[1:])
    return bits


def get_format_info(format_id):
    return get_component_type(format_id), get_bits(format_id), get_channels(format_id)


# TODO(oetuaho): Expand this code so that it could generate the gl format info tables as well.
def gl_format_channels(internal_format):
    if internal_format == 'GL_BGR5_A1_ANGLEX':
        return 'bgra'
    if internal_format == 'GL_R11F_G11F_B10F':
        return 'rgb'
    if internal_format == 'GL_RGB5_A1':
        return 'rgba'
    if internal_format.find('GL_RGB10_A2') == 0:
        return 'rgba'
    if internal_format.find('GL_RGB10') == 0:
        return 'rgb'
    # signed/unsigned int_10_10_10_2 for vertex format
    if internal_format.find('INT_10_10_10_2_OES') == 0:
        return 'rgba'

    channels_pattern = re.compile('GL_(COMPRESSED_)?(SIGNED_)?(ETC\d_)?([A-Z]+)')
    match = re.search(channels_pattern, internal_format)
    channels_string = match.group(4)

    if channels_string == 'ALPHA':
        return 'a'
    if channels_string == 'LUMINANCE':
        if (internal_format.find('ALPHA') >= 0):
            return 'la'
        return 'l'
    if channels_string == 'SRGB' or channels_string == 'RGB':
        if (internal_format.find('ALPHA') >= 0):
            return 'rgba'
        return 'rgb'
    if channels_string == 'DEPTH':
        if (internal_format.find('STENCIL') >= 0):
            return 'ds'
        return 'd'
    if channels_string == 'STENCIL':
        return 's'
    return channels_string.lower()


def get_internal_format_initializer(internal_format, format_id):
    gl_channels = gl_format_channels(internal_format)
    gl_format_no_alpha = gl_channels == 'rgb' or gl_channels == 'l'
    component_type, bits, channels = get_format_info(format_id)

    # ETC2 punchthrough formats have per-pixel alpha values but a zero-filled block is parsed as opaque black.
    # Ensure correct initialization when the formats are emulated.
    if 'PUNCHTHROUGH_ALPHA1_ETC2' in internal_format and 'ETC2' not in format_id:
        return 'Initialize4ComponentData<GLubyte, 0x00, 0x00, 0x00, 0xFF>'

    if not gl_format_no_alpha or channels != 'rgba':
        return 'nullptr'

    elif internal_format == 'GL_RGB10_EXT':
        return 'nullptr'

    elif 'BC1_' in format_id:
        # BC1 is a special case since the texture data determines whether each block has an alpha channel or not.
        # This if statement is hit by COMPRESSED_RGB_S3TC_DXT1, which is a bit of a mess.
        # TODO(oetuaho): Look into whether COMPRESSED_RGB_S3TC_DXT1 works right in general.
        # Reference: https://www.opengl.org/registry/specs/EXT/texture_compression_s3tc.txt
        return 'nullptr'

    elif component_type == 'uint' and bits['R'] == 8:
        return 'Initialize4ComponentData<GLubyte, 0x00, 0x00, 0x00, 0x01>'
    elif component_type == 'unorm' and bits['R'] == 8:
        return 'Initialize4ComponentData<GLubyte, 0x00, 0x00, 0x00, 0xFF>'
    elif component_type == 'unorm' and bits['R'] == 16:
        return 'Initialize4ComponentData<GLushort, 0x0000, 0x0000, 0x0000, 0xFFFF>'
    elif component_type == 'int' and bits['R'] == 8:
        return 'Initialize4ComponentData<GLbyte, 0x00, 0x00, 0x00, 0x01>'
    elif component_type == 'snorm' and bits['R'] == 8:
        return 'Initialize4ComponentData<GLbyte, 0x00, 0x00, 0x00, 0x7F>'
    elif component_type == 'snorm' and bits['R'] == 16:
        return 'Initialize4ComponentData<GLushort, 0x0000, 0x0000, 0x0000, 0x7FFF>'
    elif component_type == 'float' and bits['R'] == 16:
        return 'Initialize4ComponentData<GLhalf, 0x0000, 0x0000, 0x0000, gl::Float16One>'
    elif component_type == 'uint' and bits['R'] == 16:
        return 'Initialize4ComponentData<GLushort, 0x0000, 0x0000, 0x0000, 0x0001>'
    elif component_type == 'int' and bits['R'] == 16:
        return 'Initialize4ComponentData<GLshort, 0x0000, 0x0000, 0x0000, 0x0001>'
    elif component_type == 'float' and bits['R'] == 32:
        return 'Initialize4ComponentData<GLfloat, 0x00000000, 0x00000000, 0x00000000, gl::Float32One>'
    elif component_type == 'int' and bits['R'] == 32:
        return 'Initialize4ComponentData<GLint, 0x00000000, 0x00000000, 0x00000000, 0x00000001>'
    elif component_type == 'uint' and bits['R'] == 32:
        return 'Initialize4ComponentData<GLuint, 0x00000000, 0x00000000, 0x00000000, 0x00000001>'
    else:
        raise ValueError(
            'warning: internal format initializer could not be generated and may be needed for ' +
            internal_format)


def get_format_gl_type(format):
    sign = ''
    base_type = None
    if 'FLOAT' in format:
        bits = get_bits(format)
        redbits = bits and bits.get('R')
        base_type = 'float'
        if redbits == 16:
            base_type = 'half'
    else:
        bits = get_bits(format)
        redbits = bits and bits.get('R')
        if redbits == 8:
            base_type = 'byte'
        elif redbits == 16:
            base_type = 'short'
        elif redbits == 32:
            base_type = 'int'

        if 'UINT' in format or 'UNORM' in format or 'USCALED' in format:
            sign = 'u'

    if base_type is None:
        return None

    return 'GL' + sign + base_type


def get_vertex_copy_function(src_format, dst_format):
    if dst_format == "NONE":
        return "nullptr"

    src_num_channel = len(get_channel_tokens(src_format))
    dst_num_channel = len(get_channel_tokens(dst_format))
    if src_num_channel < 1 or src_num_channel > 4:
        return "nullptr"

    if src_format.endswith('_VERTEX'):
        is_signed = 'true' if 'SINT' in src_format or 'SNORM' in src_format or 'SSCALED' in src_format else 'false'
        is_normal = 'true' if 'NORM' in src_format else 'false'
        if 'A2' in src_format:
            return 'CopyW2XYZ10ToXYZWFloatVertexData<%s, %s, true>' % (is_signed, is_normal)
        else:
            return 'CopyXYZ10ToXYZWFloatVertexData<%s, %s, true>' % (is_signed, is_normal)

    if 'FIXED' in src_format:
        assert 'FLOAT' in dst_format, (
            'get_vertex_copy_function: can only convert fixed to float,' + ' not to ' + dst_format)
        return 'Copy32FixedTo32FVertexData<%d, %d>' % (src_num_channel, dst_num_channel)

    src_gl_type = get_format_gl_type(src_format)
    dst_gl_type = get_format_gl_type(dst_format)

    if src_gl_type == None:
        return "nullptr"

    if src_gl_type == dst_gl_type:
        default_alpha = '1'

        if src_num_channel == dst_num_channel or dst_num_channel < 4:
            default_alpha = '0'
        elif 'A16_FLOAT' in dst_format:
            default_alpha = 'gl::Float16One'
        elif 'A32_FLOAT' in dst_format:
            default_alpha = 'gl::Float32One'
        elif 'NORM' in dst_format:
            default_alpha = 'std::numeric_limits<%s>::max()' % (src_gl_type)

        return 'CopyNativeVertexData<%s, %d, %d, %s>' % (src_gl_type, src_num_channel,
                                                         dst_num_channel, default_alpha)

    assert 'FLOAT' in dst_format, (
        'get_vertex_copy_function: can only convert to float,' + ' not to ' + dst_format)
    normalized = 'true' if 'NORM' in src_format else 'false'

    dst_is_half = 'true' if dst_gl_type == 'GLhalf' else 'false'
    return "CopyToFloatVertexData<%s, %d, %d, %s, %s>" % (src_gl_type, src_num_channel,
                                                          dst_num_channel, normalized, dst_is_half)
