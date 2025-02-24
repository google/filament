#!/usr/bin/env python3
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
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

import os
import sys
import json
import argparse
import re
import html
import unicodedata
from collections import defaultdict
from collections import OrderedDict

class ValidationJSON:
    def __init__(self, filename : str):
        self.filename = filename
        self.explicit_vuids = set()
        self.implicit_vuids = set()
        self.all_vuids = set()
        self.vuid_db = defaultdict(list) # Maps VUID string to list of json-data dicts
        self.api_version = "0.0.0"

        # A set of specific regular expression substitutions needed to clean up VUID text
        self.regex_dict = {}
        self.regex_dict[re.compile('<.*?>|&(amp;)+lt;|&(amp;)+gt;')] = ""
        self.regex_dict[re.compile(r'\\\(codeSize \\over 4\\\)')] = "(codeSize/4)"
        self.regex_dict[re.compile(r'\\\(\\lceil\{\\mathit\{rasterizationSamples} \\over 32}\\rceil\\\)')] = "(rasterizationSamples/32)"
        self.regex_dict[re.compile(r'\\\(\\left\\lceil{\\frac{maxFramebufferWidth}{minFragmentDensityTexelSize_{width}}}\\right\\rceil\\\)')] = "the ceiling of maxFramebufferWidth/minFragmentDensityTexelSize.width"
        self.regex_dict[re.compile(r'\\\(\\left\\lceil{\\frac{maxFramebufferHeight}{minFragmentDensityTexelSize_{height}}}\\right\\rceil\\\)')] = "the ceiling of maxFramebufferHeight/minFragmentDensityTexelSize.height"
        self.regex_dict[re.compile(r'\\\(\\left\\lceil{\\frac{width}{maxFragmentDensityTexelSize_{width}}}\\right\\rceil\\\)')] = "the ceiling of width/maxFragmentDensityTexelSize.width"
        self.regex_dict[re.compile(r'\\\(\\left\\lceil{\\frac{height}{maxFragmentDensityTexelSize_{height}}}\\right\\rceil\\\)')] = "the ceiling of height/maxFragmentDensityTexelSize.height"
        self.regex_dict[re.compile(r'\\\(\\textrm\{codeSize} \\over 4\\\)')] = "(codeSize/4)"

        # Regular expression for characters outside ascii range
        self.unicode_regex = re.compile('[^\x00-\x7f]')
        # Mapping from unicode char to ascii approximation
        self.unicode_dict = {
            '\u002b' : '+',  # PLUS SIGN
            '\u00b4' : "'",  # ACUTE ACCENT
            '\u200b' : '',   # ZERO WIDTH SPACE
            '\u2018' : "'",  # LEFT SINGLE QUOTATION MARK
            '\u2019' : "'",  # RIGHT SINGLE QUOTATION MARK
            '\u201c' : '"',  # LEFT DOUBLE QUOTATION MARK
            '\u201d' : '"',  # RIGHT DOUBLE QUOTATION MARK
            '\u2026' : '...',# HORIZONTAL ELLIPSIS
            '\u2032' : "'",  # PRIME
            '\u2192' : '->', # RIGHTWARDS ARROW
            '\u2308' : 'ceil(', # LEFT CEILING (⌈)
            '\u2309' : ')', # RIGHT CEILING (⌉)
            '\u230a' : 'floor(', # LEFT FLOOR (⌊)
            '\u230b' : ')', # RIGHT FLOOR (⌋)
            '\u00d7' : 'x', # MULTIPLICATION SIGN (×)
            '\u2264' : '<=', # LESS-THAN OR EQUAL TO (≤)
            '\u2208' : 'E', # ELEMENT OF (∈)
        }

    def sanitize(self, text, location):
        # Strip leading/trailing whitespace
        text = text.strip()
        # Apply regex text substitutions
        for regex, replacement in self.regex_dict.items():
            text = re.sub(regex, replacement, text)
        # Un-escape html entity codes, ie &#XXXX;
        text = html.unescape(text)
        # Apply unicode substitutions
        for unicode in self.unicode_regex.findall(text):
            try:
                # Replace known chars
                text = text.replace(unicode, self.unicode_dict[unicode])
            except KeyError:
                # Strip and warn on unrecognized chars
                text = text.replace(unicode, '')
                name = unicodedata.name(unicode, 'UNKNOWN')
                print('Warning: Unknown unicode character \\u{:04x} ({}) at {}'.format(ord(unicode), name, location))
        return text

    def isExplicitVUID(self, vuid: str):
         vuid_number = vuid[-5:]
         # explicit end in 5 numeric chars
         return vuid_number.isdecimal()

    def dedup(self):
        unique_explicit_vuids = {}
        for item in sorted(self.explicit_vuids):
            key = item[-5:]
            unique_explicit_vuids[key] = item

        self.explicit_vuids = set(list(unique_explicit_vuids.values()))
        self.all_vuids = self.explicit_vuids | self.implicit_vuids

    def parse(self):
        self.json_dict = {}
        if not os.path.isfile(self.filename):
            print(f'Error: {self.filename} is not a valid file')
            sys.exit(-1)

        json_file = open(self.filename, 'r', encoding='utf-8')
        self.json_dict = json.load(json_file, object_pairs_hook=OrderedDict)
        json_file.close()

        if len(self.json_dict) == 0:
            print(f'Error: cant load {self.filename}')
            sys.exit(-1)

        version = self.json_dict['version info']
        self.api_version = version['api version']

        # Parse vuid from json into local databases
        validation = self.json_dict['validation']
        for api_name in validation.keys():
            api_dict = validation[api_name]
            if (len(api_dict.keys()) > 1):
                print('There use to be muliple keys here, now it should only be "core". If others are added, need to update code accordingly')
                sys.exit(-1)

            for ventry in api_dict["core"]:
                vuid_string = ventry['vuid']
                html_page = ventry['page']
                if (self.isExplicitVUID(vuid_string)):
                    self.explicit_vuids.add(vuid_string)
                    vtype = 'explicit'
                else:
                    self.implicit_vuids.add(vuid_string)
                    vtype = 'implicit'

                vuid_text = self.sanitize(ventry['text'], vuid_string)

                self.vuid_db[vuid_string].append({
                    'api' : api_name,
                    'type': vtype,
                    'text': vuid_text,
                    'page': html_page
                })

        self.all_vuids = self.explicit_vuids | self.implicit_vuids

        duplicate_vuids = set({v for v in self.vuid_db if len(self.vuid_db[v]) > 1})
        if len(duplicate_vuids) > 0:
            print("Warning: duplicate VUIDs found in validusage.json")


# These VUs are huge because they are just listing every possible option that is valid.
# The size of these make these VU error messages more harmful to print then helpful
oversized_vus = {
    'VUID-VkColorBlendEquationEXT-colorBlendOp-07361' : 'colorBlendOp and alphaBlendOp must not be a VkBlendOp from VK_EXT_blend_operation_advanced',
    'VUID-VkDeviceCreateInfo-pNext-pNext' : 'Each pNext member of any structure (including this one) in the pNext chain must be either NULL or a pointer to a valid struct for extending VkDeviceCreateInfo',
    'VUID-VkPhysicalDeviceProperties2-pNext-pNext' : 'Each pNext member of any structure (including this one) in the pNext chain must be either NULL or a pointer to a valid struct for extending VkPhysicalDeviceProperties2',
    'VUID-RuntimeSpirv-OpCooperativeMatrixMulAddKHR-10060' : 'For OpCooperativeMatrixMulAddKHR, the operands must match a supported VkCooperativeMatrixPropertiesKHR'
}

def GenerateSpecErrorMessage(api : str, valid_usage_json : str, out_file : str):
    val_json = ValidationJSON(valid_usage_json)
    val_json.parse()

    out = []
    out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications
// Based on Vulkan specification version: {val_json.api_version}

/***************************************************************************
 *
 * Copyright (c) 2016-2025 Google Inc.
 * Copyright (c) 2016-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/
#pragma once

// clang-format off

// Mapping from VUID string to the corresponding spec text
typedef struct _vuid_spec_text_pair {{
    const char * vuid;
    const char * spec_text;
    const char * url_id;
}} vuid_spec_text_pair;
\n''')

    vuid_list = list(val_json.all_vuids)
    vuid_list.sort()

    out.append('static const vuid_spec_text_pair vuid_spec_text[] = {\n')
    for vuid in vuid_list:
        db_entry = val_json.vuid_db[vuid][0]
        html_page = db_entry['page']

        # Escape quotes and backslashes when generating C strings for source code
        db_text = db_entry['text'].replace('\\', '\\\\').replace('"', '\\"').strip()
        html_remove_tags = re.compile('<.*?>|&([a-z0-9]+|#[0-9]{1,6}|#x[0-9a-f]{1,6});')
        db_text = re.sub(html_remove_tags, '', db_text)
        # In future we could use the `/n` to add new lines to a pretty print in the console
        db_text = db_text.replace('\n', ' ')
        # Remove multiple whitespaces
        db_text = re.sub(' +', ' ', db_text)
        # Override for large VU text messages
        if vuid in oversized_vus:
            db_text = oversized_vus[vuid]

        out.append(f'    {{"{vuid}", "{db_text}", "{html_page}"}},\n')
        # For multiply-defined VUIDs, include versions with extension appended
        if len(val_json.vuid_db[vuid]) > 1:
            print(f'Warning: Found a duplicate VUID: {vuid}')

    out.append('};')

    with open(out_file, 'w', newline='\n', encoding='utf-8') as file:
        file.write("".join(out))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('json_file', help="registry file 'validusage.json'")
    parser.add_argument('out_file', help="file to generate")
    parser.add_argument('-api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to use')
    args = parser.parse_args(sys.argv[1:])
    GenerateSpecErrorMessage(args.api, args.json_file, args.out_file)
