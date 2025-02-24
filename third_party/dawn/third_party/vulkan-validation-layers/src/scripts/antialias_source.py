#!/usr/bin/env python3
#
# Copyright (c) 2020-2023 Valve Corporation
# Copyright (c) 2020-2023 LunarG, Inc.
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

# Scrub source code of "aliases" -- extension types and enums that have been promoted to core.
#
# Usage: antialias_source.py XML_PATH SOURCE_PATH...
# 
# Notes:
# - Source code generators need to be re-run after this script due to
#   inconsitencies in the registry with regard to alias usage
# - Clang-format needs to be re-run after this script

import sys
import os
import re
from xml.etree import ElementTree

SOURCE_EXTS = ['.h', '.c', '.cpp', '.inc', '.py']

def GetAliases(xml_path):
    tree = ElementTree.parse(xml_path)
    root = tree.getroot()

    suffixes = [tag.get('name') for tag in root.findall('tags/tag')]

    aliases = {}
    alias_elements = []
    alias_elements.extend(root.findall('types/type[@alias]'))
    alias_elements.extend(root.findall('enums/enum[@alias]'))
    # extension types are covered above in types/type
    alias_elements.extend(root.findall('extensions/extension/require/enum[@alias]'))
    # DON'T try to do commands, those aliases are not binary compatible and we still need to handle both
    for alias_element in alias_elements:
        alias = alias_element.get('alias')
        name = alias_element.get('name')
        # Look for un-suffixed alias (core)
        if not any(alias.endswith(s) for s in suffixes):
            aliases[name] = alias
    return aliases

def UpdateFile(alias_map, filename):
    with open(filename) as f:
        file_contents = f.read()
    sub_count = 0
    for name, alias in alias_map.items():
        if name in file_contents:
            # Use regex with word break to match whole tokens only
            file_contents, n = re.subn(r'\b{}\b'.format(name), alias, file_contents)
            sub_count += n
    if sub_count > 0:
        print('{} {}'.format(filename, sub_count))
        with open(filename, 'w') as f:
            f.write(file_contents)

def UpdateDir(alias_map, path):
    for dirpath, dirnames, filenames in os.walk(path):
        for filename in filenames:
            if any(filename.endswith(ext) for ext in SOURCE_EXTS):
                UpdateFile(alias_map, os.path.join(dirpath, filename))

def Main():
    if len(sys.argv) < 3 or not sys.argv[1].endswith('.xml'):
        print('Usage: antialias_source.py XML_PATH SOURCE_PATH...')
        sys.exit(1)
    xml_path = sys.argv[1]
    source_paths = sys.argv[2:]

    aliases = GetAliases(xml_path)
    for source_path in source_paths:
        if os.path.isfile(source_path):
            UpdateFile(aliases, source_path)
        elif os.path.isdir(source_path):
            UpdateDir(aliases, source_path)

if __name__ == '__main__':
    Main()


