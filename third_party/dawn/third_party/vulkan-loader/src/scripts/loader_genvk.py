#!/usr/bin/python3
#
# Copyright (c) 2013-2025 The Khronos Group Inc.
# Copyright (c) 2021-2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# Copyright (c) 2023-2023 RasterGrid Kft.
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

import argparse
import os
import common_codegen
from generate_source import RunGenerators

# This is a legacy way to run the code gen
# Should call generate_source.py instead
if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('target', metavar='target', nargs='?', help='Specify target')
    parser.add_argument('-api', action='store',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to generate')
    parser.add_argument('-registry', action='store',
                        default='vk.xml',
                        help='Use specified registry file instead of vk.xml')
    parser.add_argument('-o', action='store', dest='directory',
                        default='.',
                        help='Create target and related files in specified directory')
    # This argument tells us where to load the script from the Vulkan-Headers registry
    parser.add_argument('-scripts', action='store',
                        help='Find additional scripts in this directory')

    args = parser.parse_args()

    # Currently don't run clang format
    # repo_dir = common_codegen.repo_relative('.')
    # style_file = os.path.join(repo_dir, '.clang-format')

    RunGenerators(api=args.api,registry=args.registry, directory=args.directory, styleFile=None, targetFilter=[args.target], flatOutput=True)

