#!/usr/bin/env python3
# Copyright (c) 2024 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Uploads files to Google Storage and output DEPS blob."""

import hashlib
import optparse
import os
import json
import tempfile

import re
import sys
import tarfile

from download_from_google_storage import Gsutil
from download_from_google_storage import GSUTIL_DEFAULT_PATH
from typing import List

MISSING_GENERATION_MSG = (
    'missing generation number, please retrieve from Cloud Storage'
    'before saving to DEPS')

USAGE_STRING = """%prog [options] target [target2 ...].
Target(s) is the files or directies intended to be uploaded to Google Storage.
If a single target is a directory, it will be compressed and uploaded as a
tar.gz file.
If target is "-", then a list of directories will be taken from standard input.
The list of directories will be compressed together and uploaded as one tar.gz
file.

Example usage
------------
./upload_to_google_storage_first_class.py --bucket gsutil-upload-playground
--object-name my_object_name hello_world.txt

./upload_to_google_storage_first_class.py --bucket gsutil-upload-playground
--object-name my_object_name my_dir1

./upload_to_google_storage_first_class.py --bucket gsutil-upload-playground
--object-name my_object_name my_dir1 my_dir2

Scan the current directory and upload all files larger than 1MB:
find . -name .svn -prune -o -size +1000k -type f -print0 |
./upload_to_google_storage_first_class.py --bucket gsutil-upload-playground
--object-name my_object_name -
"""


def get_targets(args: List[str], parser: optparse.OptionParser,
                use_null_terminator: bool) -> List[str]:
    """Get target(s) to upload to GCS"""
    if not args:
        parser.error('Missing target.')

    if len(args) == 1 and args[0] == '-':
        # Take stdin as a newline or null separated list of files.
        if use_null_terminator:
            return sys.stdin.read().split('\0')

        return sys.stdin.read().splitlines()

    return args


def create_archive(dirs: List[str]) -> str:
    """Given a list of directories, compress them all into one tar file"""
    # tarfile name cannot have a forward slash or else an error will be
    # thrown
    _, filename = tempfile.mkstemp(suffix='.tar.gz')
    with tarfile.open(filename, 'w:gz') as tar:
        for d in dirs:
            tar.add(d)
    return filename


def validate_archive_dirs(dirs: List[str]) -> bool:
    """Validate the list of directories"""
    for d in dirs:
        # We don't allow .. in paths in our archives.
        if d == '..':
            return False
        # We only allow dirs.
        if not os.path.isdir(d):
            return False
        # Symlinks must point to a target inside the dirs
        if os.path.islink(d) and not any(
                os.realpath(d).startswith(os.realpath(dir_prefix))
                for dir_prefix in dirs):
            return False
        # We required that the subdirectories we are archiving are all just
        # below cwd.
        if d not in next(os.walk('.'))[1]:
            return False

    return True


def get_sha256sum(filename: str) -> str:
    """Get the sha256sum of the file"""
    sha = hashlib.sha256()
    with open(filename, 'rb') as f:
        while True:
            # Read in 1mb chunks, so it doesn't all have to be loaded into
            # memory.
            chunk = f.read(1024 * 1024)
            if not chunk:
                break
            sha.update(chunk)
    return sha.hexdigest()


def upload_to_google_storage(file: str, base_url: str, object_name: str,
                             gsutil: Gsutil, force: bool, gzip: str,
                             dry_run: bool) -> str:
    """Upload file to GCS"""
    file_url = '%s/%s' % (base_url, object_name)
    if gsutil.check_call('ls', file_url)[0] == 0 and not force:
        # File exists, check MD5 hash.
        _, out, _ = gsutil.check_call_with_retries('ls', '-L', file_url)
        etag_match = re.search(r'ETag:\s+\S+', out)
        if etag_match:
            raise Exception('File with url %s already exists' % file_url)
    if dry_run:
        return
    print("Uploading %s as %s" % (file, file_url))
    gsutil_args = ['-h', 'Cache-Control:public, max-age=31536000', 'cp', '-v']
    if gzip:
        gsutil_args.extend(['-z', gzip])
    gsutil_args.extend([file, file_url])
    code, _, err = gsutil.check_call_with_retries(*gsutil_args)
    if code != 0:
        raise Exception(
            code, 'Encountered error on uploading %s to %s\n%s' %
            (file, file_url, err))
    pattern = re.escape(file_url) + '#(?P<generation>\d+)'
    # The geneartion number is printed as part of the progress / status info
    # which gsutil outputs to stderr to keep separated from any final output
    # data.
    for line in err.strip().splitlines():
        m = re.search(pattern, line)
        if m:
            return m.group('generation')
    print('Warning: generation number could not be parsed from status'
          f'info: {err}')
    return MISSING_GENERATION_MSG


def construct_deps_blob(bucket: str, object_name: str, file: str,
                        generation: str) -> dict:
    """Output a blob hint that would need be added to a DEPS file"""
    return {
        'path': {
            'dep_type':
            'gcs',
            'bucket':
            bucket,
            'objects': [{
                'object_name': object_name,
                'sha256sum': get_sha256sum(file),
                'size_bytes': os.path.getsize(file),
                'generation': int(generation),
            }],
        }
    }


def main():
    parser = optparse.OptionParser(USAGE_STRING)
    parser.add_option('-b',
                      '--bucket',
                      help='Google Storage bucket to upload to.')
    parser.add_option('-p',
                      '--prefix',
                      help='Prefix that goes before object-name (i.e. in '
                      'between bucket and object name).')
    parser.add_option('-o',
                      '--object-name',
                      help='Optional object name of uploaded tar file. '
                      'If empty, the sha256sum will be the object name.')
    parser.add_option('-d',
                      '--dry-run',
                      action='store_true',
                      help='Check if file already exists on GS without '
                      'uploading it and output DEP blob.')
    parser.add_option('-c',
                      '--config',
                      action='store_true',
                      help='Alias for "gsutil config".  Run this if you want '
                      'to initialize your saved Google Storage '
                      'credentials.  This will create a read-only '
                      'credentials file in ~/.boto.depot_tools.')
    parser.add_option('-e', '--boto', help='Specify a custom boto file.')
    parser.add_option('-f',
                      '--force',
                      action='store_true',
                      help='Force upload even if remote file exists.')
    parser.add_option('-g',
                      '--gsutil_path',
                      default=GSUTIL_DEFAULT_PATH,
                      help='Path to the gsutil script.')
    parser.add_option('-0',
                      '--use_null_terminator',
                      action='store_true',
                      help='Use \\0 instead of \\n when parsing '
                      'the file list from stdin.  This is useful if the input '
                      'is coming from "find ... -print0".')
    parser.add_option('-z',
                      '--gzip',
                      metavar='ext',
                      help='For files which end in <ext> gzip them before '
                      'upload. '
                      'ext is a comma-separated list')
    (options, args) = parser.parse_args()

    # Enumerate our inputs.
    input_filenames = get_targets(args, parser, options.use_null_terminator)

    # Allow uploading the entire directory
    if len(input_filenames) == 1 and input_filenames[0] in ('.', './'):
        input_filenames = next(os.walk('.'))[1]

    if len(input_filenames) > 1 or (len(input_filenames) == 1
                                    and os.path.isdir(input_filenames[0])):
        if not validate_archive_dirs(input_filenames):
            parser.error(
                'Only directories just below cwd are valid entries. '
                'Entries cannot contain .. and entries can not be symlinks. '
                'Entries was %s' % input_filenames)
            return 1
        file = create_archive(input_filenames)
    else:
        file = input_filenames[0]

    object_name = options.object_name
    if not object_name:
        object_name = get_sha256sum(file)

    if options.prefix:
        object_name = f'{options.prefix}/{object_name}'

    # Make sure we can find a working instance of gsutil.
    if os.path.exists(GSUTIL_DEFAULT_PATH):
        gsutil = Gsutil(GSUTIL_DEFAULT_PATH, boto_path=options.boto)
    else:
        gsutil = None
        for path in os.environ["PATH"].split(os.pathsep):
            if os.path.exists(path) and 'gsutil' in os.listdir(path):
                gsutil = Gsutil(os.path.join(path, 'gsutil'),
                                boto_path=options.boto)
        if not gsutil:
            parser.error('gsutil not found in %s, bad depot_tools checkout?' %
                         GSUTIL_DEFAULT_PATH)

    # Passing in -g/--config will run our copy of GSUtil, then quit.
    if options.config:
        print('===Note from depot_tools===')
        print('If you do not have a project ID, enter "0" when asked for one.')
        print('===End note from depot_tools===')
        print()
        gsutil.check_call('version')
        return gsutil.call('config')

    assert '/' not in options.bucket, "Slashes not allowed in bucket name"

    base_url = f'gs://{options.bucket}'

    generation = upload_to_google_storage(file, base_url, object_name, gsutil,
                                          options.force, options.gzip,
                                          options.dry_run)
    print(
        json.dumps(construct_deps_blob(options.bucket, object_name, file,
                                       generation),
                   indent=2))


if __name__ == '__main__':
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.stderr.write('interrupted\n')
        sys.exit(1)
