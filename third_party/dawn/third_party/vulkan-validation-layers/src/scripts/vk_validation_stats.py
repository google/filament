#!/usr/bin/env python3
# Copyright (c) 2015-2024 The Khronos Group Inc.
# Copyright (c) 2015-2024 Valve Corporation
# Copyright (c) 2015-2024 LunarG, Inc.
# Copyright (c) 2015-2024 Google Inc.
# Copyright (c) 2023-2024 RasterGrid Kft.
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
import csv
import glob
import os
import re
import sys
import subprocess
from collections import defaultdict
from generate_spec_error_message import ValidationJSON

_VENDOR_SUFFIXES = ['IMG', 'AMD', 'AMDX', 'ARM', 'FSL', 'BRCM', 'NXP', 'NV', 'NVX',
                    'VIV', 'VSI', 'KDAB', 'ANDROID', 'CHROMIUM', 'FUCHSIA', 'GGP',
                    'GOOGLE', 'QCOM', 'LUNARG', 'NZXT', 'SAMSUNG', 'SEC', 'TIZEN',
                    'RENDERDOC', 'NN', 'MVK', 'MESA', 'INTEL', 'HUAWEI', 'VALVE',
                    'QNX', 'JUICE', 'FB', 'RASTERGRID', 'MSFT']
_CROSS_VENDOR_SUFFIXES = ['KHR', 'EXT']

# helper to define paths relative to the repo root
def repo_relative(path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..', path))

def IsVendor(vuid : str, target_vendor : str = None):
    vkObject = vuid.split('-')[1]

    if target_vendor and target_vendor != "ALL":
        return vkObject.endswith(target_vendor)

    for vendor in _VENDOR_SUFFIXES:
        if vkObject.endswith(vendor):
            return True
    return False

verbose_mode = False
remove_duplicates = False
vuid_prefixes = ['VUID-']

# These files should not change unless event there is a major refactoring in SPIR-V Tools
# Paths are relative from root of SPIR-V Tools repo
spirvtools_source_files = ["source/val/validation_state.cpp"]
spirvtools_test_files = ["test/val/*.cpp"]

class ValidationSource:
    def __init__(self, source_file_list):
        self.source_files = source_file_list
        self.vuid_count_dict = {} # dict of vuid values to the count of how much they're used, and location of where they're used
        self.duplicated_checks = 0
        self.explicit_vuids = set()
        self.implicit_vuids = set()
        self.all_vuids = set()

    def dedup(self):
        unique_explicit_vuids = {}
        for item in sorted(self.explicit_vuids):
            key = item[-5:]
            unique_explicit_vuids[key] = item

        self.explicit_vuids = set(list(unique_explicit_vuids.values()))
        self.all_vuids = self.explicit_vuids | self.implicit_vuids

    def parse(self, spirv_val):

        if spirv_val and spirv_val.enabled:
            self.source_files.extend(spirv_val.source_files)

        # build self.vuid_count_dict
        prepend = None
        for sf in self.source_files:
            spirv_file = True if spirv_val.enabled and sf.startswith(spirv_val.repo_path) else False
            line_num = 0
            with open(sf, encoding='utf-8') as f:
                for line in f:
                    line_num = line_num + 1
                    if True in [line.strip().startswith(comment) for comment in ['//', '/*']]:
                        if 'VUID-' not in line or 'TODO:' in line:
                            continue
                    # Find vuid strings
                    if prepend is not None:
                        line = prepend[:-2] + line.lstrip().lstrip('"') # join lines skipping CR, whitespace and trailing/leading quote char
                        prepend = None
                    if any(prefix in line for prefix in vuid_prefixes):
                        # Replace the '(' of lines containing validation helper functions with ' ' to make them easier to parse
                        line = line.replace("(", " ")
                        line_list = line.split()

                        # A VUID string that has been broken by clang will start with a vuid prefix and end with -, and will be last in the list
                        broken_vuid = line_list[-1].strip('"')
                        if any(broken_vuid.startswith(prefix) for prefix in vuid_prefixes) and broken_vuid.endswith('-'):
                            prepend = line
                            continue

                        vuid_list = []
                        for str in line_list:
                            if any(prefix in str for prefix in vuid_prefixes):
                                vuid_list.append(str.strip(',);{}"*'))
                        for vuid in vuid_list:
                            if vuid not in self.vuid_count_dict:
                                self.vuid_count_dict[vuid] = {}
                                self.vuid_count_dict[vuid]['count'] = 1
                                self.vuid_count_dict[vuid]['file_line'] = []
                                self.vuid_count_dict[vuid]['spirv'] = False # default
                            else:
                                if self.vuid_count_dict[vuid]['count'] == 1:    # only count first time duplicated
                                    self.duplicated_checks = self.duplicated_checks + 1
                                self.vuid_count_dict[vuid]['count'] = self.vuid_count_dict[vuid]['count'] + 1
                            self.vuid_count_dict[vuid]['file_line'].append(f'{sf},{line_num}')
                            if spirv_file:
                                self.vuid_count_dict[vuid]['spirv'] = True
        # Sort vuids by type
        for vuid in self.vuid_count_dict.keys():
            if (vuid.startswith('VUID-')):
                vuid_number = vuid[-5:]
                if (vuid_number.isdecimal()):
                    self.explicit_vuids.add(vuid)    # explicit end in 5 numeric chars
                    if self.vuid_count_dict[vuid]['spirv']:
                        spirv_val.source_explicit_vuids.add(vuid)
                else:
                    self.implicit_vuids.add(vuid)
                    if self.vuid_count_dict[vuid]['spirv']:
                        spirv_val.source_implicit_vuids.add(vuid)
            else:
                print(f'Unable to categorize VUID: {vuid}')
                print("Confused while parsing VUIDs in layer source code - cannot proceed. (FIXME)")
                exit(-1)
        self.all_vuids = self.explicit_vuids | self.implicit_vuids
        if spirv_file:
            spirv_val.source_all_vuids = spirv_val.source_explicit_vuids | spirv_val.source_implicit_vuids

# Class to parse the validation layer test source and store testnames
class ValidationTests:
    def __init__(self, test_file_list):
        self.test_files = test_file_list
        self.test_trigger_txt_list = ['TEST_F(']
        self.explicit_vuids = set()
        self.implicit_vuids = set()
        self.all_vuids = set()
        #self.test_to_vuids = {} # Map test name to VUIDs tested
        self.vuid_to_tests = defaultdict(set) # Map VUIDs to set of test names where implemented

    def dedup(self):
        unique_explicit_vuids = {}
        for item in sorted(self.explicit_vuids):
            key = item[-5:]
            unique_explicit_vuids[key] = item

        self.explicit_vuids = set(list(unique_explicit_vuids.values()))
        self.all_vuids = self.explicit_vuids | self.implicit_vuids

    # Parse test files into internal data struct
    def parse(self, spirv_val):
        if spirv_val and spirv_val.enabled:
            self.test_files.extend(spirv_val.test_files)

        # For each test file, parse test names into set
        grab_next_line = False # handle testname on separate line than wildcard
        testname = ''
        prepend = None
        for test_file in self.test_files:
            spirv_file = True if spirv_val.enabled and test_file.startswith(spirv_val.repo_path) else False
            with open(test_file, encoding='utf-8') as tf:
                for line in tf:
                    if True in [line.strip().startswith(comment) for comment in ['//', '/*']]:
                        continue
                    elif True in [x in line for x in ['TEST_DESCRIPTION', 'vvl_vuid_hash']]:
                        continue # Tests have extra place it might not want to report VUIDs

                    # if line ends in a broken VUID string, fix that before proceeding
                    if prepend is not None:
                        line = prepend[:-2] + line.lstrip().lstrip('"') # join lines skipping CR, whitespace and trailing/leading quote char
                        prepend = None
                    if any(prefix in line for prefix in vuid_prefixes):
                        line_list = line.split()

                        # A VUID string that has been broken by clang will start with a vuid prefix and end with -, and will be last in the list
                        broken_vuid = line_list[-1].strip('"')
                        if any(broken_vuid.startswith(prefix) for prefix in vuid_prefixes) and broken_vuid.endswith('-'):
                            prepend = line
                            continue

                    if any(ttt in line for ttt in self.test_trigger_txt_list):
                        testname = line.split(',')[-1]
                        testname = testname.strip().strip(' {)')
                        if ('' == testname):
                            grab_next_line = True
                            continue
                        testgroup = line.split(',')[0][line.index('(') + 1:]
                        testname = testgroup + '.' + testname
                    if grab_next_line: # test name on its own line
                        grab_next_line = False
                        testname = testname.strip().strip(' {)')
                    # Don't count anything in disabled tests
                    if 'DISABLED_' in testname:
                        continue
                    if any(prefix in line for prefix in vuid_prefixes):
                        line_list = re.split('[\s{}[\]()"]+',line)
                        for sub_str in line_list:
                            if any(prefix in sub_str for prefix in vuid_prefixes):
                                vuid_str = sub_str.strip(',);:"*')
                                self.vuid_to_tests[vuid_str].add(testname)
                                if (vuid_str.startswith('VUID-')):
                                    vuid_number = vuid_str[-5:]
                                    if (vuid_number.isdecimal()):
                                        self.explicit_vuids.add(vuid_str)    # explicit end in 5 numeric chars
                                        if spirv_file:
                                            spirv_val.test_explicit_vuids.add(vuid_str)
                                    else:
                                        self.implicit_vuids.add(vuid_str)
                                        if spirv_file:
                                            spirv_val.test_implicit_vuids.add(vuid_str)
                                else:
                                    print(f'Unable to categorize VUID: {vuid_str}')
                                    print("Confused while parsing VUIDs in test code - cannot proceed. (FIXME)")
                                    exit(-1)
        self.all_vuids = self.explicit_vuids | self.implicit_vuids

# Class to do consistency checking
#
class Consistency:
    def __init__(self, all_json, all_checks, all_tests):
        self.valid = all_json
        self.checks = all_checks
        self.tests = all_tests
        # don't report
        self.discard = [
            'VUID-PrimitiveTriangleIndicesEXT-' # Currently a bug with clang-format in spirv-tools
        ]

    # Report undefined VUIDs in source code
    def undef_vuids_in_layer_code(self):
        undef_set = self.checks - self.valid
        [undef_set.discard(item) for item in self.discard]
        if (len(undef_set) > 0):
            print(f'\nFollowing VUIDs found in layer code are not defined in validusage.json ({len(undef_set)}):')
            undef = list(undef_set)
            undef.sort()
            for vuid in undef:
                print(f'    {vuid}')
            return False
        return True

    # Report undefined VUIDs in tests
    def undef_vuids_in_tests(self):
        undef_set = self.tests - self.valid
        [undef_set.discard(item) for item in self.discard]
        if (len(undef_set) > 0):
            print(f'\nFollowing VUIDs found in layer tests are not defined in validusage.json ({len(undef_set)}):')
            undef = list(undef_set)
            undef.sort()
            for vuid in undef:
                print(f'    {vuid}')
            return False
        return True

    # Report vuids in tests that are not in source
    def vuids_tested_not_checked(self):
        undef_set = self.tests - self.checks
        [undef_set.discard(item) for item in self.discard]
        if (len(undef_set) > 0):
            print(f'\nFollowing VUIDs found in tests but are not checked in layer code ({len(undef_set)}):')
            undef = list(undef_set)
            undef.sort()
            for vuid in undef:
                print(f'    {vuid}')
            return False
        return True


# Class to output database in various flavors
#
class OutputDatabase:
    def __init__(self, val_json, val_source, val_tests, spirv_val, target_vendor=None):
        self.vj = val_json
        self.vs = val_source
        self.vt = val_tests
        self.sv = spirv_val
        self.target_vendor = target_vendor

    def _is_vendor_skip(self, vuid):
        return self.target_vendor and not IsVendor(vuid, self.target_vendor)

    def dump_txt(self, filename, only_unimplemented=False):
        print(f'\nDumping database to text file: {filename}')
        with open(filename, 'w', encoding='utf-8') as txt:
            txt.write("## VUID Database\n")
            txt.write("## Format: VUID_NAME | CHECKED | SPIRV-TOOL | TEST | TYPE | API/STRUCT | VUID_TEXT\n##\n")
            vuid_list = list(self.vj.all_vuids)
            vuid_list.sort()
            for vuid in vuid_list:
                if self._is_vendor_skip(vuid):
                    continue

                db_list = self.vj.vuid_db[vuid]
                for db_entry in db_list:
                    checked = 'N'
                    spirv = 'N'
                    if vuid in self.vs.all_vuids:
                        if only_unimplemented:
                            continue
                        else:
                            checked = 'Y'
                            if vuid in self.sv.source_all_vuids:
                                spirv = 'Y'
                    test = 'None'
                    if vuid in self.vt.vuid_to_tests:
                        test_list = list(self.vt.vuid_to_tests[vuid])
                        test_list.sort()   # sort tests, for diff-ability
                        sep = ', '
                        test = sep.join(test_list)

                    vuid_text = db_entry["text"].replace('\n', ' ')
                    txt.write(f'{vuid} | {checked} | {test} | {spirv} | {db_entry["type"]} | {db_entry["api"]} | {vuid_text}\n')

    def dump_csv(self, filename, only_unimplemented=False):
        print(f'\nDumping database to csv file: {filename}')
        with open(filename, 'w', newline='', encoding='utf-8') as csvfile:
            cw = csv.writer(csvfile)
            cw.writerow(['VUID_NAME','CHECKED','SPIRV-TOOL', 'TEST','TYPE','API/STRUCT','VUID_TEXT'])
            vuid_list = list(self.vj.all_vuids)
            vuid_list.sort()
            for vuid in vuid_list:
                if self._is_vendor_skip(vuid):
                    continue

                for db_entry in self.vj.vuid_db[vuid]:
                    row = [vuid]
                    if vuid in self.vs.all_vuids:
                        if only_unimplemented:
                            continue
                        else:
                            row.append('Y') # checked
                            if vuid in self.sv.source_all_vuids:
                                row.append('Y') # spirv-tool
                            else:
                                row.append('N') # spirv-tool

                    else:
                        row.append('N') # checked
                        row.append('N') # spirv-tool
                    test = 'None'
                    if vuid in self.vt.vuid_to_tests:
                        sep = ', '
                        test = sep.join(sorted(self.vt.vuid_to_tests[vuid]))
                    row.append(test)
                    row.append(db_entry['type'])
                    row.append(db_entry['api'])
                    row.append(db_entry['text'])
                    cw.writerow(row)

    def dump_html(self, filename, only_unimplemented=False):
        print(f'\nDumping database to html file: {filename}')
        preamble = '<!DOCTYPE html>\n<html>\n<head>\n<style>\ntable, th, td {\n border: 1px solid black;\n border-collapse: collapse; \n}\n</style>\n<body>\n<h2>Valid Usage Database</h2>\n<font size="2" face="Arial">\n<table style="width:100%">\n'
        headers = '<tr><th>VUID NAME</th><th>CHECKED</th><th>SPIRV-TOOL</th><th>TEST</th><th>TYPE</th><th>API/STRUCT</th><th>VUID TEXT</th></tr>\n'
        with open(filename, 'w', encoding='utf-8') as hfile:
            hfile.write(preamble)
            hfile.write(headers)
            vuid_list = list(self.vj.all_vuids)
            vuid_list.sort()
            for vuid in vuid_list:
                if self._is_vendor_skip(vuid):
                    continue

                for db_entry in self.vj.vuid_db[vuid]:
                    checked = '<span style="color:red;">N</span>'
                    spirv = ''
                    if vuid in self.vs.all_vuids:
                        if only_unimplemented:
                            continue
                        else:
                            checked = '<span style="color:limegreen;">Y</span>'
                            if vuid in self.sv.source_all_vuids:
                                spirv = 'Y'
                    hfile.write(f'<tr><th>{vuid}</th>')
                    hfile.write(f'<th>{checked}</th>')
                    hfile.write(f'<th>{spirv}</th>')
                    test = 'None'
                    if vuid in self.vt.vuid_to_tests:
                        sep = ', '
                        test = sep.join(sorted(self.vt.vuid_to_tests[vuid]))
                    hfile.write(f'<th>{test}</th>')
                    hfile.write(f'<th>{db_entry["type"]}</th>')
                    hfile.write(f'<th>{db_entry["api"]}</th>')
                    hfile.write(f'<th>{db_entry["text"]}</th></tr>\n')
            hfile.write('</table>\n</body>\n</html>\n')

class SpirvValidation:
    def __init__(self, repo_path):
        self.enabled = (repo_path is not None)
        self.repo_path = repo_path
        self.version = 'unknown'
        self.source_files = []
        self.test_files = []
        self.source_explicit_vuids = set()
        self.source_implicit_vuids = set()
        self.source_all_vuids = set()
        self.test_explicit_vuids = set()
        self.test_implicit_vuids = set()

    def load(self, verbose):
        if self.enabled is False:
            return
        # Get hash from git if available
        try:
            git_dir = os.path.join(self.repo_path, '.git')
            process = subprocess.Popen(['git', '--git-dir='+git_dir ,'rev-parse', 'HEAD'], shell=False, stdout=subprocess.PIPE)
            self.version = process.communicate()[0].strip().decode('utf-8')[:7]
            if process.poll() != 0:
                throw
            elif verbose:
                print(f'Found SPIR-V Tools version {self.version}')
        except:
            # leave as default
            if verbose:
                print(f'Could not find .git file for version of SPIR-V tools, marking as {self.version}')

        # Find and parse files with VUIDs in source
        for path in spirvtools_source_files:
            self.source_files.extend(glob.glob(os.path.join(self.repo_path, path)))
        for path in spirvtools_test_files:
            self.test_files.extend(glob.glob(os.path.join(self.repo_path, path)))


def main(argv):
    TXT_FILENAME = "validation_error_database.txt"
    CSV_FILENAME = "validation_error_database.csv"
    HTML_FILENAME = "validation_error_database.html"

    parser = argparse.ArgumentParser()
    parser.add_argument('json_file', help="registry file 'validusage.json'")
    parser.add_argument('-api',
                        default='vulkan',
                        choices=['vulkan'],
                        help='Specify API name to use')
    parser.add_argument('-c', action='store_true',
                        help='report consistency warnings')
    parser.add_argument('-todo', action='store_true',
                        help='report unimplemented VUIDs')
    parser.add_argument('-vuid', metavar='VUID_NAME',
                        help='report status of individual VUID <VUID_NAME>')
    parser.add_argument('-spirvtools', metavar='PATH',
                        help='when pointed to root directory of SPIRV-Tools repo, will search the repo for VUs that are implemented there')
    parser.add_argument('-text', nargs='?', const=TXT_FILENAME, metavar='FILENAME',
                        help=f'export the error database in text format to <FILENAME>, defaults to {TXT_FILENAME}')
    parser.add_argument('-csv', nargs='?', const=CSV_FILENAME, metavar='FILENAME',
                        help=f'export the error database in csv format to <FILENAME>, defaults to {CSV_FILENAME}')
    parser.add_argument('-html', nargs='?', const=HTML_FILENAME, metavar='FILENAME',
                        help=f'export the error database in html format to <FILENAME>, defaults to {HTML_FILENAME}')
    parser.add_argument('-remove_duplicates', action='store_true',
                        help='remove duplicate VUID numbers')
    parser.add_argument('-summary', action='store_true',
                        help='output summary of VUID coverage')
    parser.add_argument('-verbose', action='store_true',
                        help='show your work (to stdout)')
    parser.add_argument('-vendor', default=None, choices=_VENDOR_SUFFIXES + _CROSS_VENDOR_SUFFIXES + ["ALL"],
                        help='report only vendor specific vuids, defaults to none, "ALL" means only vendor specific vuids')
    args = parser.parse_args()

    # We need python modules found in the registry directory. This assumes that the validusage.json file is in that directory,
    # and hasn't been copied elsewhere.
    registry_dir = os.path.dirname(args.json_file)
    sys.path.insert(0, registry_dir)

    layer_source_files = [repo_relative(path) for path in [
        'layers/error_message/unimplementable_validation.h',
        'layers/state_tracker/video_session_state.cpp',
        'layers/layer_options.cpp',
        f'layers/{args.api}/generated/stateless_validation_helper.cpp',
        f'layers/{args.api}/generated/object_tracker.cpp',
        f'layers/{args.api}/generated/spirv_validation_helper.cpp',
        f'layers/{args.api}/generated/command_validation.cpp',
    ]]
    # Be careful not to add vk_validation_error_messages.h or it will show 100% test coverage
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/core_checks/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/stateless/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/sync/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/object_tracker/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/drawdispatch/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/validation_cmd/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/core/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/descriptor_validation/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/error_message/'), '*.cpp')))
    layer_source_files.extend(glob.glob(os.path.join(repo_relative('layers/gpuav/instrumentation/'), '*.cpp')))

    test_source_files = glob.glob(os.path.join(repo_relative('tests/unit'), '*.cpp'))

    global verbose_mode
    verbose_mode = args.verbose

    global remove_duplicates
    remove_duplicates = args.remove_duplicates

    # Load in SPIRV-Tools if passed in
    spirv_val = SpirvValidation(args.spirvtools)
    spirv_val.load(verbose_mode)

    # Parse validusage json
    val_json = ValidationJSON(args.json_file)
    val_json.parse()
    if remove_duplicates:
        val_json.dedup()
    exp_json = len(val_json.explicit_vuids)
    imp_json = len(val_json.implicit_vuids)
    all_json = len(val_json.all_vuids)
    if verbose_mode:
        print('Found {all_json} unique error vuids in validusage.json file.')
        print(f'  {exp_json} explicit')
        print(f'  {imp_json} implicit')

    # Parse layer source files
    val_source = ValidationSource(layer_source_files)
    val_source.parse(spirv_val)
    if remove_duplicates:
        val_source.dedup()
    exp_checks = len(val_source.explicit_vuids)
    imp_checks = len(val_source.implicit_vuids)
    all_checks = exp_checks + imp_checks
    spirv_exp_checks = len(spirv_val.source_explicit_vuids) if spirv_val.enabled else 0
    spirv_imp_checks = len(spirv_val.source_implicit_vuids) if spirv_val.enabled else 0
    spirv_all_checks = (spirv_exp_checks + spirv_imp_checks) if spirv_val.enabled else 0
    if verbose_mode:
        print('Found {all_checks} unique vuid checks in layer source code.')
        print(f'  {exp_checks} explicit')
        if spirv_val.enabled:
            print(f'    SPIR-V Tool make up {spirv_exp_checks}')
        print(f'  {imp_checks} implicit')
        if spirv_val.enabled:
            print(f'    SPIR-V Tool make up {spirv_imp_checks}')
        print(f'  {val_source.duplicated_checks} checks are implemented more that once')

    # Parse test files
    val_tests = ValidationTests(test_source_files)
    val_tests.parse(spirv_val)
    if remove_duplicates:
        val_tests.dedup()
    exp_tests = len(val_tests.explicit_vuids)
    imp_tests = len(val_tests.implicit_vuids)
    all_tests = len(val_tests.all_vuids)
    spirv_exp_tests = len(spirv_val.test_explicit_vuids) if spirv_val.enabled else 0
    spirv_imp_tests = len(spirv_val.test_implicit_vuids) if spirv_val.enabled else 0
    spirv_all_tests = (spirv_exp_tests + spirv_imp_tests) if spirv_val.enabled else 0
    if verbose_mode:
        print('Found {all_tests} unique error vuids in test source code.')
        print('  {exp_tests} explicit')
        if spirv_val.enabled:
            print('    From SPIRV-Tools: {spirv_exp_tests}')
        print('  {imp_tests} implicit')
        if spirv_val.enabled:
            print('    From SPIRV-Tools: {spirv_imp_tests}')

    # Process stats
    if args.summary:
        if spirv_val.enabled:
            print(f'\nValidation Statistics (using validusage.json version {val_json.api_version} and SPIRV-Tools version {spirv_val.version})')
        else:
            print(f'\nValidation Statistics (using validusage.json version {val_json.api_version})')
        print(f"  VUIDs defined in JSON file:  {exp_json:04d} explicit, {imp_json:04d} implicit, {all_json:04d} total.")
        print(f"  VUIDs checked in layer code: {exp_checks:04d} explicit, {imp_checks:04d} implicit, {all_checks:04d} total.")
        if spirv_val.enabled:
            print(f"             From SPIRV-Tools: {spirv_exp_checks:04d} explicit, {spirv_imp_checks:04d} implicit, {spirv_all_checks:04d} total.")
        print(f"  VUIDs tested in layer tests: {exp_tests:04d} explicit, {imp_tests:04d} implicit, {all_tests:04d} total.")
        if spirv_val.enabled:
            print(f"             From SPIRV-Tools: {spirv_exp_tests:04d} explicit, {spirv_imp_tests:04d} implicit, {spirv_all_tests:04d} total.")

        print("\nVUID check coverage")
        print(f"  Explicit VUIDs checked: {(100.0 * exp_checks / exp_json):.1f}% ({exp_checks} checked vs {exp_json} defined)")
        print(f"  Implicit VUIDs checked: {(100.0 * imp_checks / imp_json):.1f}% ({imp_checks} checked vs {imp_json} defined)")
        print(f"  Overall VUIDs checked:  {(100.0 * all_checks / all_json):.1f}% ({all_checks} checked vs {all_json} defined)")

        unimplemented_explicit = val_json.all_vuids - val_source.all_vuids
        vendor_count = sum(1 for vuid in unimplemented_explicit if IsVendor(vuid))
        print(f'    {len(unimplemented_explicit)} VUID checks remain unimplemented ({vendor_count} are from Vendor objects)')

        print("\nVUID test coverage")
        print(f"  Explicit VUIDs tested: {(100.0 * exp_tests / exp_checks):.1f}% ({exp_tests} tested vs {exp_checks} checks)")
        print(f"  Implicit VUIDs tested: {(100.0 * imp_tests / imp_checks):.1f}% ({imp_tests} tested vs {imp_checks} checks)")
        print(f"  Overall VUIDs tested:  {(100.0 * all_tests / all_checks):.1f}% ({all_tests} tested vs {all_checks} checks)")

    # Report status of a single VUID
    if args.vuid:
        print(f'\n\nChecking status of <{args.vuid}>')
        if args.vuid not in val_json.all_vuids:
            print('  Not a valid VUID string.')
        else:
            if args.vuid in val_source.explicit_vuids:
                print('  Implemented!')
                line_list = val_source.vuid_count_dict[args.vuid]['file_line']
                for line in line_list:
                    print(f'    => {line}')
            elif args.vuid in val_source.implicit_vuids:
                print('  Implemented! (Implicit)')
                line_list = val_source.vuid_count_dict[args.vuid]['file_line']
                for line in line_list:
                    print(f'    => {line}')
            else:
                print('  Not implemented.')
            if args.vuid in val_tests.all_vuids:
                print('  Has a test!')
                test_list = val_tests.vuid_to_tests[args.vuid]
                for test in test_list:
                    print(f'    => {test}')
            else:
                print('  Not tested.')

    # Report unimplemented explicit VUIDs
    if args.todo:
        unim_explicit = val_json.explicit_vuids - val_source.explicit_vuids
        print(f'\n\n{len(unim_explicit)} explicit VUID checks remain unimplemented:')
        ulist = list(unim_explicit)
        ulist.sort()
        for vuid in ulist:
            print(f'  => {vuid}')

    # Consistency tests
    if args.c:
        print("\n\nRunning consistency tests...")
        con = Consistency(val_json.all_vuids, val_source.all_vuids, val_tests.all_vuids)
        ok = con.undef_vuids_in_layer_code()
        ok &= con.undef_vuids_in_tests()
        ok &= con.vuids_tested_not_checked()

        if ok:
            print("  OK! No inconsistencies found.")

    # Output database in requested format(s)
    db_out = OutputDatabase(val_json, val_source, val_tests, spirv_val, args.vendor)
    if args.text:
        db_out.dump_txt(args.text, args.todo)
    if args.csv:
        db_out.dump_csv(args.csv, args.todo)
    if args.html:
        db_out.dump_html(args.html, args.todo)

if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
