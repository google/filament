#!/usr/bin/python3
#
# Copyright 2019 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# angle_tools.py:
#   Common functionality to scripts in angle/tools directory.

import os
import platform
import subprocess

is_windows = platform.system() == 'Windows'
is_linux = platform.system() == 'Linux'
is_mac = platform.system() == 'Darwin'


def find_file_in_path(filename):
    """ Finds |filename| by searching the environment paths """
    path_delimiter = ';' if is_windows else ':'
    for env_path in os.environ['PATH'].split(path_delimiter):
        full_path = os.path.join(env_path, filename)
        if os.path.isfile(full_path):
            return full_path
    raise Exception('Cannot find %s in environment' % filename)


def get_exe_name(file_name, windows_extension):
    exe_name = file_name
    if is_windows:
        exe_name += windows_extension
    return exe_name


def upload_to_google_storage(bucket, files):
    file_dir = os.path.dirname(os.path.realpath(__file__))
    upload_script = os.path.join(file_dir, '..', 'third_party', 'depot_tools',
                                 'upload_to_google_storage.py')
    upload_args = ['python3', upload_script, '-b', bucket] + files
    return subprocess.call(upload_args) == 0


def stage_google_storage_sha1(files):
    git_exe = get_exe_name('git', '.bat')
    git_exe = find_file_in_path(git_exe)

    sha1_files = [f + '.sha1' for f in files]
    return subprocess.call([git_exe, 'add'] + sha1_files) == 0
