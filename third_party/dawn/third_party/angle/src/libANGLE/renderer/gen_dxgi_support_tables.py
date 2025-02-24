#!/usr/bin/python3
# Copyright 2015 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# gen_dxgi_support_tables.py:
#  Code generation for the DXGI support tables. Determines which formats
#  are natively support in D3D10+.
#  NOTE: don't run this script directly. Run scripts/run_code_generation.py.
#
#  NOTE: The "never supported" formats should not be combined with the
#  "supported" and "optional" ones. At the moment, this does not cause
#  any issues as ANGLE does not internally check for "never supported".
#
# MSDN links:
#  10Level9 9_3: https://msdn.microsoft.com/en-us/library/windows/desktop/mt790740.aspx
#  10_0: https://msdn.microsoft.com/en-us/library/windows/desktop/cc627090.aspx
#  10_1: https://msdn.microsoft.com/en-us/library/windows/desktop/cc627091.aspx
#  11_0: https://msdn.microsoft.com/en-us/library/windows/desktop/ff471325.aspx
#  11_1: https://msdn.microsoft.com/en-us/library/windows/desktop/hh404483.aspx

import sys
import json

macro_prefix = 'F_'

template = """// GENERATED FILE - DO NOT EDIT. See dxgi_support_data.json.
//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// dxgi_support_table:
//   Queries for DXGI support of various texture formats. Depends on DXGI
//   version, D3D feature level, and is sometimes guaranteed or optional.
//

#include "libANGLE/renderer/dxgi_support_table.h"

#include "common/debug.h"

namespace rx
{{

namespace d3d11
{{

#define {prefix}2D D3D11_FORMAT_SUPPORT_TEXTURE2D
#define {prefix}3D D3D11_FORMAT_SUPPORT_TEXTURE3D
#define {prefix}CUBE D3D11_FORMAT_SUPPORT_TEXTURECUBE
#define {prefix}SAMPLE D3D11_FORMAT_SUPPORT_SHADER_SAMPLE
#define {prefix}RT D3D11_FORMAT_SUPPORT_RENDER_TARGET
#define {prefix}MS D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET
#define {prefix}DS D3D11_FORMAT_SUPPORT_DEPTH_STENCIL
#define {prefix}MIPGEN D3D11_FORMAT_SUPPORT_MIP_AUTOGEN

namespace
{{

const DXGISupport &GetDefaultSupport()
{{
    static UINT AllSupportFlags =
        D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_TEXTURE3D |
        D3D11_FORMAT_SUPPORT_TEXTURECUBE | D3D11_FORMAT_SUPPORT_SHADER_SAMPLE |
        D3D11_FORMAT_SUPPORT_RENDER_TARGET | D3D11_FORMAT_SUPPORT_MULTISAMPLE_RENDERTARGET |
        D3D11_FORMAT_SUPPORT_DEPTH_STENCIL | D3D11_FORMAT_SUPPORT_MIP_AUTOGEN;
    static const DXGISupport defaultSupport(0, 0, AllSupportFlags);
    return defaultSupport;
}}

const DXGISupport &GetDXGISupport_9_3(DXGI_FORMAT dxgiFormat)
{{
    // clang-format off
    switch (dxgiFormat)
    {{
{table_data_9_3}
        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }}
    // clang-format on
}}

const DXGISupport &GetDXGISupport_10_0(DXGI_FORMAT dxgiFormat)
{{
    // clang-format off
    switch (dxgiFormat)
    {{
{table_data_10_0}
        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }}
    // clang-format on
}}

const DXGISupport &GetDXGISupport_10_1(DXGI_FORMAT dxgiFormat)
{{
    // clang-format off
    switch (dxgiFormat)
    {{
{table_data_10_1}
        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }}
    // clang-format on
}}

const DXGISupport &GetDXGISupport_11_0(DXGI_FORMAT dxgiFormat)
{{
    // clang-format off
    switch (dxgiFormat)
    {{
{table_data_11_0}
        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }}
    // clang-format on
}}

const DXGISupport &GetDXGISupport_11_1(DXGI_FORMAT dxgiFormat)
{{
    // clang-format off
    switch (dxgiFormat)
    {{
{table_data_11_1}
        default:
            UNREACHABLE();
            return GetDefaultSupport();
    }}
    // clang-format on
}}

}}

#undef {prefix}2D
#undef {prefix}3D
#undef {prefix}CUBE
#undef {prefix}SAMPLE
#undef {prefix}RT
#undef {prefix}MS
#undef {prefix}DS
#undef {prefix}MIPGEN

const DXGISupport &GetDXGISupport(DXGI_FORMAT dxgiFormat, D3D_FEATURE_LEVEL featureLevel)
{{
    switch (featureLevel)
    {{
        case D3D_FEATURE_LEVEL_9_3:
            return GetDXGISupport_9_3(dxgiFormat);
        case D3D_FEATURE_LEVEL_10_0:
            return GetDXGISupport_10_0(dxgiFormat);
        case D3D_FEATURE_LEVEL_10_1:
            return GetDXGISupport_10_1(dxgiFormat);
        case D3D_FEATURE_LEVEL_11_0:
            return GetDXGISupport_11_0(dxgiFormat);
        case D3D_FEATURE_LEVEL_11_1:
            return GetDXGISupport_11_1(dxgiFormat);
        default:
            return GetDefaultSupport();
    }}
}}

}} // namespace d3d11

}} // namespace rx
"""


def do_format(format_data):
    table_data = {'9_3': '', '10_0': '', '10_1': '', '11_0': '', '11_1': ''}

    json_flag_to_d3d = {
        'texture2D': macro_prefix + '2D',
        'texture3D': macro_prefix + '3D',
        'textureCube': macro_prefix + 'CUBE',
        'shaderSample': macro_prefix + 'SAMPLE',
        'renderTarget': macro_prefix + 'RT',
        'multisampleRT': macro_prefix + 'MS',
        'depthStencil': macro_prefix + 'DS',
        'mipAutoGen': macro_prefix + 'MIPGEN'
    }

    for format_name, format_support in sorted(format_data.items()):

        always_supported = set()
        never_supported = set()
        optionally_supported = set()
        fl_9_3_supported = set()
        fl_9_3_check = set()
        fl_10_0_supported = set()
        fl_10_1_supported = set()
        fl_11_0_supported = set()
        fl_11_0_check = set()
        fl_11_1_supported = set()
        fl_10_0_check_10_1_supported = set()
        fl_10_0_check_11_0_supported = set()

        for json_flag, support in sorted(format_support.items()):

            d3d_flag = [json_flag_to_d3d[json_flag]]

            if support == 'check':
                optionally_supported.update(d3d_flag)
            elif support == 'always':
                always_supported.update(d3d_flag)
            elif support == 'never':
                never_supported.update(d3d_flag)
            elif support == '10_0':
                fl_10_0_supported.update(d3d_flag)
            elif support == '10_1':
                fl_10_1_supported.update(d3d_flag)
            elif support == '11_0':
                fl_11_0_supported.update(d3d_flag)
            elif support == '11_1':
                fl_11_1_supported.update(d3d_flag)
            elif support == 'dxgi1_2':
                # TODO(jmadill): DXGI 1.2 handling.
                always_supported.update(d3d_flag)
            elif support == '10_0check10_1always':
                fl_10_0_check_10_1_supported.update(d3d_flag)
            elif support == '10_0check11_0always':
                fl_10_0_check_11_0_supported.update(d3d_flag)
            elif support == '11_0check':
                fl_11_0_check.update(d3d_flag)
            elif support == '9_3always_10_0check11_0always':
                fl_9_3_supported.update(d3d_flag)
                fl_10_0_check_11_0_supported.update(d3d_flag)
            elif support == '9_3check_10_0always':
                fl_9_3_check.update(d3d_flag)
                fl_10_0_supported.update(d3d_flag)
            elif support == '9_3check11_0always':
                fl_9_3_check.update(d3d_flag)
                fl_10_0_check_11_0_supported.update(d3d_flag)
            else:
                print("Data specification error: " + support)
                sys.exit(1)

        for feature_level in ['9_3', '10_0', '10_1', '11_0', '11_1']:
            always_for_fl = always_supported
            optional_for_fl = optionally_supported
            if feature_level == '9_3':
                always_for_fl = fl_9_3_supported.union(always_for_fl)
                optional_for_fl = fl_9_3_check.union(optional_for_fl)
            elif feature_level == '10_0':
                always_for_fl = fl_10_0_supported.union(always_for_fl)
                optional_for_fl = fl_10_0_check_10_1_supported.union(optional_for_fl)
                optional_for_fl = fl_10_0_check_11_0_supported.union(optional_for_fl)
            elif feature_level == '10_1':
                always_for_fl = fl_10_0_supported.union(always_for_fl)
                always_for_fl = fl_10_1_supported.union(always_for_fl)
                always_for_fl = fl_10_0_check_10_1_supported.union(always_for_fl)
                optional_for_fl = fl_10_0_check_11_0_supported.union(optional_for_fl)
            elif feature_level == '11_0':
                always_for_fl = fl_10_0_supported.union(always_for_fl)
                always_for_fl = fl_10_0_check_10_1_supported.union(always_for_fl)
                always_for_fl = fl_10_0_check_11_0_supported.union(always_for_fl)
                always_for_fl = fl_10_1_supported.union(always_for_fl)
                always_for_fl = fl_11_0_supported.union(always_for_fl)
            elif feature_level == '11_1':
                always_for_fl = fl_10_0_supported.union(always_for_fl)
                always_for_fl = fl_10_0_check_10_1_supported.union(always_for_fl)
                always_for_fl = fl_10_0_check_11_0_supported.union(always_for_fl)
                always_for_fl = fl_10_1_supported.union(always_for_fl)
                always_for_fl = fl_11_0_supported.union(always_for_fl)
                always_for_fl = fl_11_1_supported.union(always_for_fl)

            always = ' | '.join(sorted(always_for_fl))
            never = ' | '.join(sorted(never_supported))
            optional = ' | '.join(sorted(optional_for_fl))

            if not always:
                always = '0'
            if not never:
                never = '0'
            if not optional:
                optional = '0'

            table_data[feature_level] += '        case ' + format_name + ':\n'
            table_data[feature_level] += '        {\n'
            table_data[
                feature_level] += '            static const DXGISupport info(' + always + ', ' + never + ', ' + optional + ');\n'
            table_data[feature_level] += '            return info;\n'
            table_data[feature_level] += '        }\n'

    return table_data


def join_table_data(table_data_1, table_data_2):
    return {
        '9_3': table_data_1['9_3'] + table_data_2['9_3'],
        '10_0': table_data_1['10_0'] + table_data_2['10_0'],
        '10_1': table_data_1['10_1'] + table_data_2['10_1'],
        '11_0': table_data_1['11_0'] + table_data_2['11_0'],
        '11_1': table_data_1['11_1'] + table_data_2['11_1']
    }


def main():

    # auto_script parameters.
    if len(sys.argv) > 1:
        inputs = ['dxgi_support_data.json']
        outputs = ['dxgi_support_table_autogen.cpp']

        if sys.argv[1] == 'inputs':
            print(','.join(inputs))
        elif sys.argv[1] == 'outputs':
            print(','.join(outputs))
        else:
            print('Invalid script parameters')
            return 1
        return 0

    with open('dxgi_support_data.json') as dxgi_file:
        file_data = dxgi_file.read()
        dxgi_file.close()
        json_data = json.loads(file_data)

        table_data = {'9_3': '', '10_0': '', '10_1': '', '11_0': '', '11_1': ''}

        for format_data in json_data:
            table_data = join_table_data(table_data, do_format(format_data))

        out_data = template.format(
            prefix=macro_prefix,
            table_data_9_3=table_data['9_3'],
            table_data_10_0=table_data['10_0'],
            table_data_10_1=table_data['10_1'],
            table_data_11_0=table_data['11_0'],
            table_data_11_1=table_data['11_1'])

        with open('dxgi_support_table_autogen.cpp', 'wt') as out_file:
            out_file.write(out_data)
            out_file.close()
    return 0


if __name__ == '__main__':
    sys.exit(main())
