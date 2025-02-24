#!/usr/bin/env python
# Copyright 2019 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Generate commit.h with git commit hash.
#

import subprocess as sp
import sys
import os

usage = """\
Usage: commit_id.py check                 - check if git is present
       commit_id.py gen <file_to_write>   - generate commit.h"""


def grab_output(command, cwd):
    return sp.Popen(command, stdout=sp.PIPE, shell=True, cwd=cwd).communicate()[0].strip()


if len(sys.argv) < 2:
    sys.exit(usage)

operation = sys.argv[1]
cwd = sys.path[0]

if operation == 'check':
    index_path = os.path.join(cwd, '.git', 'index')
    if os.path.exists(index_path):
        print("1")
    else:
        print("0")
    sys.exit(0)

if len(sys.argv) < 3 or operation != 'gen':
    sys.exit(usage)

output_file = sys.argv[2]
commit_id_size = 12

commit_id = 'invalid-hash'
commit_date = 'invalid-date'

try:
    commit_id = grab_output('git rev-parse --short=%d HEAD' % commit_id_size, cwd)
    commit_date = grab_output('git show -s --format=%ci HEAD', cwd)
except:
    pass

hfile = open(output_file, 'w')

hfile.write('#define SWIFTSHADER_COMMIT_HASH "%s"\n' % commit_id)
hfile.write('#define SWIFTSHADER_COMMIT_HASH_SIZE %d\n' % commit_id_size)
hfile.write('#define SWIFTSHADER_COMMIT_DATE "%s"\n' % commit_date)
hfile.write('#define SWIFTSHADER_VERSION_STRING    \\\n'
            'MACRO_STRINGIFY(MAJOR_VERSION) \".\"  \\\n'
            'MACRO_STRINGIFY(MINOR_VERSION) \".\"  \\\n'
            'MACRO_STRINGIFY(PATCH_VERSION) \".\"  \\\n'
            'SWIFTSHADER_COMMIT_HASH\n')

hfile.close()
