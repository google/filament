#!/usr/bin/env python3
#  Copyright 2022 The ANGLE Project Authors. All rights reserved.
#  Use of this source code is governed by a BSD-style license that can be
#  found in the LICENSE file.

# Generate ANGLEShaderProgramVersion.h with hash of files affecting data
# used in serializing/deserializing shader programs.
import hashlib
import argparse
import shlex


# Generate a hash from a list of files defined in angle_code_files
def GenerateHashOfAffectedFiles(angle_code_files):
    hash_md5 = hashlib.md5()
    for file in angle_code_files:
        hash_md5.update(file.encode(encoding='utf-8'))
        with open(file, 'r', encoding='utf-8') as f:
            for chunk in iter(lambda: f.read(4096), ""):
                hash_md5.update(chunk.encode())
    return hash_md5.hexdigest(), hash_md5.digest_size


# The script is expecting two mandatory input arguments:
# 1) output_file: the header file where we will add two constants that indicate
# the version of the code files that affect shader program compilations
# 2) response_file_name: a file that includes a list of code files that affect
# shader program compilations
parser = argparse.ArgumentParser(description='Generate the file ANGLEShaderProgramVersion.h')
parser.add_argument(
    'output_file',
    help='path (relative to build directory) to output file name, stores ANGLE_PROGRAM_VERSION and ANGLE_PROGRAM_VERSION_HASH_SIZE'
)
parser.add_argument(
    'response_file_name',
    help='path (relative to build directory) to response file name. The response file stores a list of program files that ANGLE_PROGRAM_VERSION hashes over. See https://gn.googlesource.com/gn/+/main/docs/reference.md#var_response_file_contents'
)

args = parser.parse_args()

output_file = args.output_file
response_file_name = args.response_file_name

with open(response_file_name, "r") as input_files_for_hash_generation:
    angle_code_files = shlex.split(input_files_for_hash_generation)
digest, digest_size = GenerateHashOfAffectedFiles(angle_code_files)
hfile = open(output_file, 'w')
hfile.write('#define ANGLE_PROGRAM_VERSION "%s"\n' % digest)
hfile.write('#define ANGLE_PROGRAM_VERSION_HASH_SIZE %d\n' % digest_size)
hfile.close()
