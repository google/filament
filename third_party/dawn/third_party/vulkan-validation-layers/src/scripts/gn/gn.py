#!/usr/bin/env python3
# Copyright 2023-2024 The Khronos Group Inc.
# Copyright 2023-2024 Valve Corporation
# Copyright 2023-2024 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0

import os
import subprocess
import sys

# helper to define paths relative to the repo root
def RepoRelative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '../../', path))

def BuildGn():
    if not os.path.exists(RepoRelative("depot_tools")):
        clone_cmd = 'git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git depot_tools'.split()
        subprocess.check_call(clone_cmd)

    os.environ['PATH'] = os.environ.get('PATH') + ":" + RepoRelative("depot_tools")

    # Updating Repo Dependencies and GN Toolchain
    update_cmd = './scripts/gn/update_deps.sh'
    subprocess.check_call(update_cmd)

    # Generating Ninja Files with Checking
    gn_gen_cmd = 'gn gen --check out'.split()
    # Forward extra build args from command line
    gn_gen_cmd.extend(sys.argv[1:])
    subprocess.check_call(gn_gen_cmd)

    # Check all source files for validation are in the BUILD.gn file
    gn_export_targets_cmd = f'{sys.executable} ./scripts/gn/export_targets.py out //:VkLayer_khronos_validation'.split()
    subprocess.check_call(gn_export_targets_cmd)

    # Running Ninja Build
    ninja_build_cmd = 'ninja -C out'.split()
    subprocess.check_call(ninja_build_cmd)

#
# Module Entrypoint
def main():
    try:
        BuildGn()
    except subprocess.CalledProcessError as proc_error:
        print(f'Command {proc_error.cmd} failed with return code {proc_error.returncode}')
        sys.exit(proc_error.returncode)
    except Exception as unknown_error:
        print(f'An unkown error occured: {unknown_error}')
        sys.exit(1)

    sys.exit(0)

if __name__ == '__main__':
    main()
