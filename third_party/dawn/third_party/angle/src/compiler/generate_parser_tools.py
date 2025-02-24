#!/usr/bin/python3
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# generate_parser_tools.py:
#   Common functionality to call flex and bison to generate lexer and parser of
#   the translator and preprocessor.

import os
import platform
import subprocess
import sys

is_linux = platform.system() == 'Linux'
is_mac = platform.system() == 'Darwin'
is_windows = platform.system() == 'Windows'


def get_tool_path_platform(tool_name, platform):
    exe_path = os.path.join(sys.path[0], '..', '..', '..', 'tools', 'flex-bison', platform)

    return os.path.join(exe_path, tool_name)


def get_tool_path(tool_name):
    if is_linux:
        platform = 'linux'
        ext = ''
    elif is_mac:
        platform = 'mac'
        ext = ''
    else:
        assert (is_windows)
        platform = 'windows'
        ext = '.exe'

    return get_tool_path_platform(tool_name + ext, platform)


def get_tool_file_sha1s():
    files = [
        get_tool_path_platform('flex', 'linux'),
        get_tool_path_platform('bison', 'linux'),
        get_tool_path_platform('flex.exe', 'windows'),
        get_tool_path_platform('bison.exe', 'windows'),
        get_tool_path_platform('m4.exe', 'windows'),
        get_tool_path_platform('flex', 'mac'),
        get_tool_path_platform('bison', 'mac'),
    ]

    files += [
        get_tool_path_platform(dll, 'windows')
        for dll in ['msys-2.0.dll', 'msys-iconv-2.dll', 'msys-intl-8.dll']
    ]

    return [f + '.sha1' for f in files]


def run_flex(basename):
    flex = get_tool_path('flex')
    input_file = basename + '.l'
    output_source = basename + '_lex_autogen.cpp'

    flex_args = [flex, '--noline', '--nounistd', '--outfile=' + output_source, input_file]

    flex_env = os.environ.copy()
    if is_windows:
        flex_env['M4'] = get_tool_path_platform('m4.exe', 'windows')

    process = subprocess.Popen(flex_args, env=flex_env, cwd=sys.path[0])
    process.communicate()
    if process.returncode != 0:
        return process.returncode

    # Patch flex output for 64-bit.  The patch is simple enough that we could do a string
    # replacement.  More importantly, the location of the line of code that needs to be substituted
    # can vary based on flex version, and the string substitution will find the correct place
    # automatically.

    patch_in = """\n\t\tYY_INPUT( (&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move]),\n\t\t\tyyg->yy_n_chars, num_to_read );"""
    patch_out = """
        yy_size_t ret = 0;
        YY_INPUT( (&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move]),
            ret, num_to_read );
        yyg->yy_n_chars = static_cast<int>(ret);"""

    with open(output_source, 'r') as flex_output:
        output = flex_output.read()

        # If flex's output changes such that this line no longer exists, the patch needs to be
        # updated, or possibly removed.
        assert (output.find(patch_in) != -1)

        patched = output.replace(patch_in, patch_out)

    # Remove all tab characters from output. WebKit does not allow any tab characters in source
    # files.
    patched = patched.replace('\t', '    ')

    with open(output_source, 'w') as flex_output_patched:
        flex_output_patched.write(patched)

    return 0


def run_bison(basename, generate_header):
    bison = get_tool_path('bison')
    input_file = basename + '.y'
    output_header = basename + '_tab_autogen.h'
    output_source = basename + '_tab_autogen.cpp'

    bison_args = [bison, '--no-lines', '--skeleton=yacc.c']
    if generate_header:
        bison_args += ['--defines=' + output_header]
    bison_args += ['--output=' + output_source, input_file]

    bison_env = os.environ.copy()
    bison_env['BISON_PKGDATADIR'] = get_tool_path_platform('', 'third_party')
    if is_windows:
        bison_env['M4'] = get_tool_path_platform('m4.exe', 'windows')

    process = subprocess.Popen(bison_args, env=bison_env, cwd=sys.path[0])
    process.communicate()
    return process.returncode


def get_input_files(basename):
    files = [basename + '.l', basename + '.y']
    return [os.path.join(sys.path[0], f) for f in files]


def get_output_files(basename, generate_header):
    optional_header = [basename + '_tab_autogen.h'] if generate_header else []
    files = [basename + '_lex_autogen.cpp', basename + '_tab_autogen.cpp'] + optional_header
    return [os.path.join(sys.path[0], f) for f in files]


def generate_parser(basename, generate_header):
    # Handle inputs/outputs for run_code_generation.py's auto_script
    if len(sys.argv) > 1:
        if sys.argv[1] == 'inputs':
            inputs = get_tool_file_sha1s()
            inputs += get_input_files(basename)
            current_file = __file__
            if current_file.endswith('.pyc'):
                current_file = current_file[:-1]
            inputs += [current_file]
            print(','.join(inputs))
        if sys.argv[1] == 'outputs':
            print(','.join(get_output_files(basename, generate_header)))
        return 0

    # Call flex and bison to generate the lexer and parser.
    flex_result = run_flex(basename)
    if flex_result != 0:
        print('Failed to run flex. Error %s' % str(flex_result))
        return 1

    bison_result = run_bison(basename, generate_header)
    if bison_result != 0:
        print('Failed to run bison. Error %s' % str(bison_result))
        return 2

    return 0
