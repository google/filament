#!/usr/bin/python3
#
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# update_flex_bison_binaries.py:
#   Helper script to update the version of flex and bison in cloud storage.
#   These binaries are used to generate the ANGLE translator's lexer and parser.
#   This script relies on flex and bison binaries to be externally built which
#   is expected to be a rare operation. It currently only works on Windows and
#   Linux. It also will update the hashes stored in the tree. For more info see
#   README.md in this folder.

import os
import platform
import shutil
import sys

sys.path.append('tools/')
import angle_tools


def main():
    if angle_tools.is_linux:
        subdir = 'linux'
        files = ['flex', 'bison']
    elif angle_tools.is_mac:
        subdir = 'mac'
        files = ['flex', 'bison']
    elif angle_tools.is_windows:
        subdir = 'windows'
        files = [
            'flex.exe', 'bison.exe', 'm4.exe', 'msys-2.0.dll', 'msys-iconv-2.dll',
            'msys-intl-8.dll'
        ]
    else:
        print('Script must be run on Linux, Mac or Windows.')
        return 1

    files = [os.path.join(sys.path[0], subdir, f) for f in files]

    # Step 1: Upload to cloud storage
    bucket = 'angle-flex-bison'
    if not angle_tools.upload_to_google_storage(bucket, files):
        print('Error upload to cloud storage')
        return 1

    # Step 2: Stage new SHA to git
    if not angle_tools.stage_google_storage_sha1(files):
        print('Error running git add')
        return 2

    print('')
    print('The updated SHA has been staged for commit. Please commit and upload.')
    print('Suggested commit message (please indicate flex/bison versions):')
    print('----------------------------')
    print('')
    print('Update flex and bison binaries for %s.' % platform.system())
    print('')
    print('These binaries were updated using %s.' % os.path.basename(__file__))
    print('Please see instructions in tools/flex-bison/README.md.')
    print('')
    print('flex is at version TODO.')
    print('bison is at version TODO.')
    print('')
    print('Bug: None')

    return 0


if __name__ == '__main__':
    sys.exit(main())
