#!/usr/bin/env python3
#  Copyright 2018 The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.

# Generate commit.h with git commit hash.
#

import subprocess as sp
import sys
import os

usage = """\
Usage: commit_id.py check                - check if git is present
       commit_id.py get_git_dirs         - prints work-tree and common git directories
       commit_id.py unpack <ref_file>    - check if <ref_file> exists, and if not
                                           create it based on .git/packed-refs
       commit_id.py position             - print commit position
       commit_id.py gen <file_to_write>  - generate commit.h"""


def grab_output(command, cwd):
    return sp.Popen(
        command, stdout=sp.PIPE, shell=True, cwd=cwd).communicate()[0].strip().decode('utf-8')


def get_git_dir(cwd):
    return grab_output('git rev-parse --git-dir', cwd)


def get_git_common_dir(cwd):
    return grab_output('git rev-parse --git-common-dir', cwd)


def get_commit_position(cwd):
    return grab_output('git rev-list HEAD --count', cwd)


def does_git_dir_exist(cwd):
    ret = os.path.exists(os.path.join(cwd, '.git', 'HEAD'))
    # .git may be a file with a gitdir directive pointing elsewhere.
    if not ret and os.path.exists(os.path.join(cwd, '.git')):
        ret = 'true' == grab_output('git rev-parse --is-inside-work-tree', cwd)
    return ret


def unpack_ref(ref_file, ref_file_full_path, packed_refs_full_path):

    with open(packed_refs_full_path) as fin:
        refs = fin.read().strip().split('\n')

    # Strip comments
    refs = [ref.split(' ') for ref in refs if ref.strip()[0] != '#']

    # Parse lines (which are in the format <hash> <ref_file>) and find the input file
    refs = [git_hash for (git_hash, file_path) in refs if file_path == ref_file]

    assert (len(refs) == 1)
    git_hash = refs[0]

    with open(ref_file_full_path, 'w') as fout:
        fout.write(git_hash + '\n')


if len(sys.argv) < 2:
    sys.exit(usage)

operation = sys.argv[1]

# Set the root of ANGLE's repo as the working directory
aosp_angle_path = os.path.join(os.path.dirname('.'), 'external', 'angle')
aosp = os.path.exists(aosp_angle_path)
cwd = aosp_angle_path if aosp else os.path.join(os.path.dirname(os.path.realpath(__file__)), '..')
git_dir_exists = does_git_dir_exist(cwd)

if operation == 'check':
    if git_dir_exists:
        print("1")
    else:
        print("0")
    sys.exit(0)
elif operation == 'get_git_dirs':
    print(get_git_dir(cwd))
    print(get_git_common_dir(cwd))
    sys.exit(0)
elif operation == 'unpack':
    if len(sys.argv) < 3:
        sys.exit(usage)

    ref_file = sys.argv[2]
    git_common_dir = get_git_common_dir(cwd)
    ref_file_full_path = os.path.join(cwd, git_common_dir, ref_file)
    ref_file_exists = os.path.exists(ref_file_full_path)

    if not ref_file_exists:
        packed_refs_full_path = os.path.join(cwd, git_common_dir, 'packed-refs')
        unpack_ref(ref_file, ref_file_full_path, packed_refs_full_path)

    sys.exit(0)
elif operation == 'position':
    if git_dir_exists:
        print(get_commit_position(cwd))
    else:
        print("0")
    sys.exit(0)

if len(sys.argv) < 3 or operation != 'gen':
    sys.exit(usage)

output_file = sys.argv[2]
commit_id_size = 12
commit_date = 'unknown date'
commit_position = '0'

# If the ANGLE_UPSTREAM_HASH environment variable is set, use it as
# commit_id. commit_date will be 'unknown date' and commit_position will be 0
# in this case. See details in roll_aosp.sh where commit_id.py is invoked.
commit_id = os.environ.get('ANGLE_UPSTREAM_HASH')
# If ANGLE_UPSTREAM_HASH environment variable is not set, use the git command
# to get the git hash, when .git is available
if git_dir_exists and not commit_id:
    try:
        commit_id = grab_output('git rev-parse --short=%d HEAD' % commit_id_size, cwd)
        commit_date = grab_output('git show -s --format=%ci HEAD', cwd) or commit_date
        commit_position = get_commit_position(cwd) or commit_position
    except:
        pass

hfile = open(output_file, 'w')

hfile.write('#define ANGLE_COMMIT_HASH "%s"\n' % (commit_id or "unknown hash"))
hfile.write('#define ANGLE_COMMIT_HASH_SIZE %d\n' % commit_id_size)
hfile.write('#define ANGLE_COMMIT_DATE "%s"\n' % commit_date)
hfile.write('#define ANGLE_COMMIT_POSITION %s\n' % commit_position)


hfile.close()
