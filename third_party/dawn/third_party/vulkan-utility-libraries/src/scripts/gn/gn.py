#!/usr/bin/env python3
# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
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
        print("Cloning Chromium depot_tools\n", flush=True)
        clone_cmd = 'git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git depot_tools'.split(" ")
        subprocess.call(clone_cmd)

    os.environ['PATH'] = os.environ.get('PATH') + ":" + RepoRelative("depot_tools")

    print("Updating Repo Dependencies and GN Toolchain\n", flush=True)
    update_cmd = './scripts/gn/update_deps.sh'
    subprocess.check_call(update_cmd)

    print("Checking Header Dependencies\n", flush=True)
    gn_check_cmd = 'gn gen --check out/Debug'.split(" ")
    subprocess.check_call(gn_check_cmd)

    print("Generating Ninja Files\n", flush=True)
    gn_gen_cmd = 'gn gen out/Debug'.split(" ")
    subprocess.check_call(gn_gen_cmd)

    print("Running Ninja Build\n", flush=True)
    ninja_build_cmd = 'ninja -C out/Debug'.split(" ")
    subprocess.check_call(ninja_build_cmd)

#
# Module Entrypoint
def main():
    try:
        BuildGn()

    except subprocess.CalledProcessError as proc_error:
        print('Command "%s" failed with return code %s' % (' '.join(proc_error.cmd), proc_error.returncode))
        sys.exit(proc_error.returncode)
    except Exception as unknown_error:
        print('An unkown error occured: %s', unknown_error)
        sys.exit(1)

    sys.exit(0)

if __name__ == '__main__':
    main()
