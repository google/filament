#!/usr/bin/env python3

# Copyright 2026 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import glob
import json
import os
import shutil
import sys


def main():
    if len(sys.argv) != 3:
        print("Usage: copy_mesa_driver.py <mesa_install_dir> <output_dir>")
        sys.exit(1)

    install_dir = sys.argv[1]
    out_dir = sys.argv[2]

    # Find the ICD JSON
    json_pattern = os.path.join(install_dir, '**', 'lvp_icd.*.json')
    json_files = glob.glob(json_pattern, recursive=True)
    if not json_files:
        print(f"Could not find lvp_icd.*.json in {install_dir}")
        sys.exit(1)

    # Find the shared library
    # Linux: libvulkan_lvp.so, Windows: vulkan_lvp.dll, Mac: libvulkan_lvp.dylib
    so_pattern = os.path.join(install_dir, '**', '*vulkan_lvp*')
    so_files = [
        f for f in glob.glob(so_pattern, recursive=True)
        if os.path.isfile(f) and f.endswith(('.so', '.dll', '.dylib'))
    ]
    if not so_files:
        print(f"Could not find vulkan_lvp shared library in {install_dir}")
        sys.exit(1)

    so_file = so_files[0]
    so_name = os.path.basename(so_file)

    # Copy shared library
    shutil.copy2(so_file, os.path.join(out_dir, so_name))

    # Read and update JSON
    with open(json_files[0], 'r') as f:
        icd_data = json.load(f)

    # Update library_path to be relative to the JSON manifest
    icd_data['ICD']['library_path'] = f"./{so_name}"

    # Write updated JSON
    out_json = os.path.join(out_dir, "lvp_icd.json")
    with open(out_json, 'w') as f:
        json.dump(icd_data, f, indent=4)


if __name__ == '__main__':
    main()
