#!/usr/bin/env python

# Copyright (c) 2022-2023 LunarG, Inc.
# Copyright (C) 2016 The ANGLE Project Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Generate copies of the Vulkan layers JSON files, with no paths, forcing
Vulkan to use the default search path to look for layers."""

from __future__ import print_function

import argparse
import glob
import json
import os
import platform
import sys

def glob_slash(dirname):
    r"""Like regular glob but replaces \ with / in returned paths."""
    return [s.replace('\\', '/') for s in glob.glob(dirname)]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--icd', action='store_true')
    parser.add_argument('--no-path-prefix', action='store_true')
    parser.add_argument('--platform', type=str, default=platform.system(),
        help='Target platform to build validation layers for: '
             'Linux|Darwin|Windows|Fuchsia|...')
    parser.add_argument('source_dir')
    parser.add_argument('target_dir')
    parser.add_argument('json_files', nargs='*')
    args = parser.parse_args()

    source_dir = args.source_dir
    target_dir = args.target_dir

    json_files = [j for j in args.json_files if j.endswith('.json')]
    json_in_files = [j for j in args.json_files if j.endswith('.json.in')]

    data_key = 'ICD' if args.icd else 'layer'

    if not os.path.isdir(source_dir):
        print(source_dir + ' is not a directory.', file=sys.stderr)
        return 1

    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    # Copy the *.json files from source dir to target dir
    if (set(glob_slash(os.path.join(source_dir, '*.json'))) != set(json_files)):
        print(glob.glob(os.path.join(source_dir, '*.json')))
        print('.json list in gn file is out-of-date', file=sys.stderr)
        return 1

    for json_fname in json_files:
        if not json_fname.endswith('.json'):
            continue
        with open(json_fname) as infile:
            data = json.load(infile)

        # Update the path.
        if not data_key in data:
            raise Exception(
                "Could not find '%s' key in %s" % (data_key, json_fname))

        # The standard validation layer has no library path.
        if 'library_path' in data[data_key]:
            prev_name = os.path.basename(data[data_key]['library_path'])
            data[data_key]['library_path'] = prev_name

        target_fname = os.path.join(target_dir, os.path.basename(json_fname))
        with open(target_fname, 'w') as outfile:
            json.dump(data, outfile)

    # Set json file prefix and suffix for generating files, default to Linux.
    if args.no_path_prefix:
        relative_path_prefix = ''
    elif args.platform == 'Windows':
        relative_path_prefix = r'..\\'  # json-escaped, hence two backslashes.
    else:
        relative_path_prefix = '../lib'
    file_type_suffix = '.so'
    if args.platform == 'Windows':
        file_type_suffix = '.dll'
    elif args.platform == 'Darwin':
        file_type_suffix = '.dylib'

    # For each *.json.in template files in source dir generate actual json file
    # in target dir
    if (set(glob_slash(os.path.join(source_dir, '*.json.in'))) !=
            set(json_in_files)):
        print('.json.in list in gn file is out-of-date', file=sys.stderr)
        return 1
    for json_in_name in json_in_files:
        if not json_in_name.endswith('.json.in'):
            continue
        json_in_fname = os.path.basename(json_in_name)
        layer_name = json_in_fname[:-len('.json.in')]
        layer_lib_name = layer_name + file_type_suffix
        json_out_fname = os.path.join(target_dir, json_in_fname[:-len('.in')])
        with open(json_out_fname,'w') as json_out_file, \
             open(json_in_name) as infile:
            for line in infile:
                line = line.replace('@JSON_LIBRARY_PATH@', relative_path_prefix + layer_lib_name)
                json_out_file.write(line)

if __name__ == '__main__':
    sys.exit(main())
