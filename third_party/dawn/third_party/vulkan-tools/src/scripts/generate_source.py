#!/usr/bin/env python3
# Copyright (c) 2019-2025 The Khronos Group Inc.
# Copyright (c) 2019-2025 Valve Corporation
# Copyright (c) 2019-2025 LunarG, Inc.
# Copyright (c) 2019-2025 Google Inc.
# Copyright (c) 2023-2025 RasterGrid Kft.
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
#
# Author: Mike Schuchardt <mikes@lunarg.com>

import argparse
import filecmp
import os
import json
import shutil
import subprocess
import sys
import tempfile

import common_codegen

# files to exclude from --verify check
verify_exclude = ['.clang-format']

def main(argv):
    parser = argparse.ArgumentParser(description='Generate source code for this repository')
    parser.add_argument('--api',
                        default='vulkan',
                        choices=['vulkan', 'vulkansc'],
                        help='Specify API name to generate')
    parser.add_argument('--generated-version', help='sets the header version used to generate the repo')
    parser.add_argument('registry', metavar='REGISTRY_PATH', help='path to the Vulkan-Headers registry directory')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('-i', '--incremental', action='store_true', help='only update repo files that change')
    group.add_argument('-v', '--verify', action='store_true', help='verify repo files match generator output')
    args = parser.parse_args(argv)

    # output paths and the list of files in the path
    files_to_gen = {str(os.path.join('icd','generated')) : ['vk_typemap_helper.h',
                                            'function_definitions.h',
                                            'function_declarations.h'],
                    str(os.path.join('vulkaninfo','generated')): ['vulkaninfo.hpp']}

    #base directory for the source repository
    repo_dir = common_codegen.repo_relative('')

    # Update the api_version in the respective json files
    if args.generated_version:
        json_files = []
        json_files.append(common_codegen.repo_relative('icd/VkICD_mock_icd.json.in'))
        for json_file in json_files:
            with open(json_file) as f:
                data = json.load(f)

            data["ICD"]["api_version"] = args.generated_version

            with open(json_file, mode='w', encoding='utf-8', newline='\n') as f:
                f.write(json.dumps(data, indent=4))

    # get directory where generators will run if needed
    if args.verify or args.incremental:
        # generate in temp directory so we can compare or copy later
        temp_obj = tempfile.TemporaryDirectory(prefix='VulkanLoader_generated_source_')
        temp_dir = temp_obj.name
        for path in files_to_gen.keys():
            os.makedirs(os.path.join(temp_dir, path))

    registry = os.path.abspath(os.path.join(args.registry,  'vk.xml'))
    if not os.path.isfile(registry):
        registry = os.path.abspath(os.path.join(args.registry, 'Vulkan-Headers/registry/vk.xml'))
        if not os.path.isfile(registry):
            print(f'cannot find vk.xml in {args.registry}')
            return -1

    # run each code generator
    for path, filenames in files_to_gen.items():
        for filename in filenames:
            if args.verify or args.incremental:
                output_path = os.path.join(temp_dir, path)
            else:
                output_path = common_codegen.repo_relative(path)

            cmd = [common_codegen.repo_relative(os.path.join('scripts','kvt_genvk.py')),
                '-api', args.api,
                '-registry', registry,
                '-quiet', '-directory', output_path, filename]
            print(' '.join(cmd))
            try:
                if args.verify or args.incremental:
                    subprocess.check_call([sys.executable] + cmd, cwd=temp_dir)
                else:
                    subprocess.check_call([sys.executable] + cmd, cwd=repo_dir)

            except Exception as e:
                print('ERROR:', str(e))
                return 1

    # optional post-generation steps
    if args.verify:
        # compare contents of temp dir and repo
        temp_files = {}
        for path in files_to_gen.keys():
            temp_files[path] = set()
            temp_files[path].update(set(os.listdir(os.path.join(temp_dir, path))))

        repo_files = {}
        for path in files_to_gen.keys():
            repo_files[path] = set()
            repo_files[path].update(set(os.listdir(os.path.join(repo_dir, path))) - set(verify_exclude))

        files_match = True
        for path in files_to_gen.keys():
            for filename in sorted((temp_files[path] | repo_files[path])):
                if filename not in repo_files[path]:
                    print('ERROR: Missing repo file', filename)
                    files_match = False
                elif filename not in temp_files[path]:
                    print('ERROR: Missing generator for', filename)
                    files_match = False
                elif not filecmp.cmp(os.path.join(temp_dir, path, filename),
                                   os.path.join(repo_dir, path, filename),
                                   shallow=False):
                    print('ERROR: Repo files do not match generator output for', filename)
                    files_match = False

        # return code for test scripts
        if files_match:
            print('SUCCESS: Repo files match generator output')
            return 0
        return 1

    elif args.incremental:
        # copy missing or differing files from temp directory to repo
        for path in files_to_gen.keys():
            for filename in os.listdir(os.path.join(temp_dir,path)):
                temp_filename = os.path.join(temp_dir, path, filename)
                repo_filename = os.path.join(repo_dir, path, filename)
                if not os.path.exists(repo_filename) or \
                   not filecmp.cmp(temp_filename, repo_filename, shallow=False):
                    print('update', repo_filename)
                    shutil.copyfile(temp_filename, repo_filename)

    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

