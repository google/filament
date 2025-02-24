#!/usr/bin/python3
#
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# update_glslang_binary.py:
#   Helper script to update the version of glslang in cloud storage.
#   This glslang is used to precompile Vulkan shaders. This script builds
#   glslang and uploads it to the bucket for Windows or Linux. It
#   currently only works on Windows and Linux. It also will update the
#   hashes stored in the tree. For more info see README.md in this folder.

import os
import platform
import shutil
import subprocess
import sys

sys.path.append('tools/')
import angle_tools

gn_args = """is_clang = true
is_debug = false
angle_enable_vulkan = true"""


def main():
    if not angle_tools.is_windows and not angle_tools.is_linux:
        print('Script must be run on Linux or Windows.')
        return 1

    # Step 1: Generate an output directory
    out_dir = os.path.join('out', 'glslang_release')

    if not os.path.isdir(out_dir):
        os.mkdir(out_dir)

    args_gn = os.path.join(out_dir, 'args.gn')
    if not os.path.isfile(args_gn):
        with open(args_gn, 'w') as f:
            f.write(gn_args)
            f.close()

    gn_exe = angle_tools.get_exe_name('gn', '.bat')

    # Step 2: Generate the ninja build files in the output directory
    if subprocess.call([gn_exe, 'gen', out_dir]) != 0:
        print('Error calling gn')
        return 2

    # Step 3: Compile glslang_validator
    if subprocess.call(['ninja', '-C', out_dir, 'glslang_validator']) != 0:
        print('Error calling ninja')
        return 3

    # Step 4: Copy glslang_validator to the tools/glslang directory
    glslang_exe = angle_tools.get_exe_name('glslang_validator', '.exe')

    glslang_src = os.path.join(out_dir, glslang_exe)
    glslang_dst = os.path.join(sys.path[0], glslang_exe)

    shutil.copy(glslang_src, glslang_dst)

    # Step 5: Delete the build directory
    shutil.rmtree(out_dir)

    # Step 6: Upload to cloud storage
    if not angle_tools.upload_to_google_storage('angle-glslang-validator', [glslang_dst]):
        print('Error upload to cloud storage')
        return 4

    # Step 7: Stage new SHA to git
    if not angle_tools.stage_google_storage_sha1([glslang_dst]):
        print('Error running git add')
        return 5

    print('')
    print('The updated SHA has been staged for commit. Please commit and upload.')
    print('Suggested commit message:')
    print('----------------------------')
    print('')
    print('Update glslang_validator binary for %s.' % platform.system())
    print('')
    print('This binary was updated using %s.' % os.path.basename(__file__))
    print('Please see instructions in tools/glslang/README.md.')
    print('')
    print('Bug: None')

    return 0


if __name__ == '__main__':
    sys.exit(main())
