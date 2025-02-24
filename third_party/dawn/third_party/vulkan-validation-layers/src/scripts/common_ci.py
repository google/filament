#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2024 The Khronos Group Inc.
# Copyright (c) 2015-2024 Valve Corporation
# Copyright (c) 2015-2024 LunarG, Inc.
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

import os
import sys
import subprocess
import platform

# Use Ninja for all platforms for performance/simplicity
os.environ['CMAKE_GENERATOR'] = "Ninja"

# helper to define paths relative to the repo root
def RepoRelative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

# Points to the directory containing the top level CMakeLists.txt
PROJECT_SRC_DIR = os.path.abspath(os.path.join(os.path.split(os.path.abspath(__file__))[0], '..'))
if not os.path.isfile(f'{PROJECT_SRC_DIR}/CMakeLists.txt'):
    print(f'PROJECT_SRC_DIR invalid! {PROJECT_SRC_DIR}')
    sys.exit(1)

# Returns true if we are running in GitHub actions
# https://docs.github.com/en/actions/learn-github-actions/variables#default-environment-variables
def IsGHA():
    if 'GITHUB_ACTION' in os.environ:
        return True
    return False

# Runs a command in a directory and returns its return code.
# Directory is project root by default, or a relative path from project root
def RunShellCmd(command, start_dir = PROJECT_SRC_DIR, env=None, verbose=False):
    # Flush stdout here. Helps when debugging on CI.
    sys.stdout.flush()

    if start_dir != PROJECT_SRC_DIR:
        start_dir = RepoRelative(start_dir)
    cmd_list = command.split(" ")

    # Helps a lot when debugging CI issues
    if IsGHA():
        verbose = True

    if verbose:
        print(f'CICMD({cmd_list}, env={env})')
    subprocess.check_call(cmd_list, cwd=start_dir, env=env)

#
# Check if the system is Windows
def IsWindows(): return 'windows' == platform.system().lower()
