#!/usr/bin/env python
#===============================================================================
# Copyright 2018-2019 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

import sys
import datetime
import xml.etree.ElementTree as ET

def banner():
    year_now = datetime.datetime.now().year
    banner_year = '2018' if year_now == 2018 else '2018-%d' % year_now
    return '''\
/*******************************************************************************
* Copyright %s Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* DO NOT EDIT, AUTO-GENERATED */

''' % banner_year


def template(body):
    return '%s%s' % (banner(), body)


def header(body):
    return '''\
#ifndef MKLDNN_DEBUG_H
#define MKLDNN_DEBUG_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/* All symbols shall be internal unless marked as MKLDNN_API */
#if defined _WIN32 || defined __CYGWIN__
#   define MKLDNN_HELPER_DLL_IMPORT __declspec(dllimport)
#   define MKLDNN_HELPER_DLL_EXPORT __declspec(dllexport)
#else
#   if __GNUC__ >= 4
#       define MKLDNN_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
#       define MKLDNN_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
#   else
#       define MKLDNN_HELPER_DLL_IMPORT
#       define MKLDNN_HELPER_DLL_EXPORT
#   endif
#endif

#ifdef MKLDNN_DLL
#   ifdef MKLDNN_DLL_EXPORTS
#       define MKLDNN_API MKLDNN_HELPER_DLL_EXPORT
#   else
#       define MKLDNN_API MKLDNN_HELPER_DLL_IMPORT
#   endif
#else
#   define MKLDNN_API
#endif

#if defined (__GNUC__)
#   define MKLDNN_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#   define MKLDNN_DEPRECATED __declspec(deprecated)
#else
#   define MKLDNN_DEPRECATED
#endif

#include "mkldnn_types.h"
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#ifdef __cplusplus
extern "C" {
#endif

%s
/** Forms a format string for a given memory descriptor.
 *
 * The format is defined as: 'dt:[p|o|0]:fmt_kind:fmt:extra'.
 * Here:
 *  - dt       -- data type
 *  - p        -- indicates there is non-trivial padding
 *  - o        -- indicates there is non-trivial padding offset
 *  - 0        -- indicates there is non-trivial offset0
 *  - fmt_kind -- format kind (blocked, wino, etc...)
 *  - fmt      -- extended format string (format_kind specific)
 *  - extra    -- shows extra fields (underspecified)
 */
int MKLDNN_API mkldnn_md2fmt_str(char *fmt_str, size_t fmt_str_len,
        const mkldnn_memory_desc_t *md);

/** Forms a dimension string for a given memory descriptor.
 *
 * The format is defined as: 'dim0xdim1x...xdimN
 */
int MKLDNN_API mkldnn_md2dim_str(char *dim_str, size_t dim_str_len,
        const mkldnn_memory_desc_t *md);

#ifdef __cplusplus
}
#endif

#endif
''' % body


def source(body):
    return '''\
#include <assert.h>

#include "mkldnn_debug.h"
#include "mkldnn_types.h"

%s
''' % body


def maybe_skip(enum):
    return enum in (
        'mkldnn_padding_kind_t',
        'mkldnn_batch_normalization_flag_t',
        'mkldnn_memory_extra_flags_t',
        'mkldnn_wino_memory_format_t',
        'mkldnn_rnn_cell_flags_t',
        'mkldnn_rnn_packed_memory_format_t',
        'mkldnn_engine_kind_t',
        'mkldnn_query_t',
        'mkldnn_stream_flags_t',
        )


def enum_abbrev(enum):
    return {
        'mkldnn_status_t': 'status',
        'mkldnn_data_type_t': 'dt',
        'mkldnn_round_mode_t': 'rmode',
        'mkldnn_format_kind_t': 'fmt_kind',
        'mkldnn_format_tag_t': 'fmt_tag',
        'mkldnn_prop_kind_t': 'prop_kind',
        'mkldnn_primitive_kind_t': 'prim_kind',
        'mkldnn_alg_kind_t': 'alg_kind',
        'mkldnn_rnn_direction_t': 'rnn_direction',
    }.get(enum, enum)


def sanitize_value(v):
    if 'undef' in v:
        return 'undef'
    v = v.split('mkldnn_format_kind_')[-1]
    v = v.split('mkldnn_')[-1]
    return v


def func_decl(enum, is_header = False):
    abbrev = enum_abbrev(enum)
    return 'const char %s*mkldnn_%s2str(%s v)' % \
            ('MKLDNN_API ' if is_header else '', abbrev, enum)


def func_to_str(enum, values):
    indent = '    '
    abbrev = enum_abbrev(enum)
    func = ''
    func += func_decl(enum) + ' {\n'
    for v in values:
        func += '%sif (v == %s) return "%s";\n' \
                % (indent, v, sanitize_value(v))
    func += '%sassert(!"unknown %s");\n' % (indent, abbrev)
    func += '%sreturn "unknown %s";\n}\n' % (indent, abbrev)
    return func


def generate(ifile):
    h_body = ''
    s_body = ''
    root = ET.parse(ifile).getroot()
    for v_enum in root.findall('Enumeration'):
        enum = v_enum.attrib['name']
        if maybe_skip(enum):
            continue
        values = [v_value.attrib['name'] \
                for v_value in v_enum.findall('EnumValue')]
        h_body += func_decl(enum, is_header = True) + ';\n'
        s_body += func_to_str(enum, values) + '\n'
    return (template(header(h_body)), template(source(s_body)))


def usage():
    print '''\
%s types.xml [output_dir]

Generates MKL-DNN debug header and source files with enum to string mapping.
Input types.xml file can be obtained with CastXML[1]:
$ castxml --castxml-cc-gnu-c clang --castxml-output=1 \\
        include/mkldnn_types.h -o types.xml

[1] https://github.com/CastXML/CastXML''' % sys.argv[0]
    sys.exit(1)


for arg in sys.argv:
    if '-help' in arg:
        usage()

ifile = sys.argv[1] if len(sys.argv) > 1 else usage()
odir = sys.argv[2] if len(sys.argv) > 2 else '.'
ofile_h = odir + '/mkldnn_debug.h'
ofile_s = odir + '/mkldnn_debug_autogenerated.cpp'

(h, s) = generate(ifile)

with open(ofile_h, 'w') as fh:
    fh.write(h)

with open(ofile_s, 'w') as fs:
    fs.write(s)
