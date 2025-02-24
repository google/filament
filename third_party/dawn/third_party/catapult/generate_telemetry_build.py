#!/usr/bin/env python3
# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Generate BUILD.gn to define all data required to run telemetry test.

If a file/folder of large size is added to catapult but is not needed to run
telemetry test, then it should be added to the EXCLUDED_PATHS below and rerun
this script.

This script can also run with --check to see if it needs to rerun to update
BUILD.gn.

This script can also run with --chromium and rewrite the chromium file
   //tools/perf/chrome_telemetry_build/BUILD.gn
This is for the purpose of running try jobs in chromium.

"""

import argparse
import difflib
import logging
import os
import sys
import subprocess
import textwrap

LICENSE = textwrap.dedent(
    """\
    # Copyright 2018 The Chromium Authors. All rights reserved.
    # Use of this source code is governed by a BSD-style license that can be
    # found in the LICENSE file.

    """)

DO_NOT_EDIT_WARNING = textwrap.dedent(
    """\
    # This file is auto-generated from
    #    //third_party/catapult/generated_telemetry_build.py
    # DO NOT EDIT!

    """)

TELEMETRY_SUPPORT_GROUP_NAME = 'telemetry_chrome_test_support'

EXCLUDED_PATHS = {
    'BUILD.gn',
    'TEMP.gn',
    'common/node_runner/',
    'docs/',
    'experimental/',
    'generate_telemetry_build.py',
    'tracing/test_data/',
}


SEPARATE_TARGETS = {
  'devil': 'devil',
  'telemetry': 'telemetry',
  'third_party/gsutil': 'third_party/gsutil',
  'third_party/typ': 'third_party/typ',
  'third_party/vinn': 'third_party/vinn',
}


def GetUntrackedPaths():
  """Return directories/files in catapult/ that are not tracked by git."""
  output = subprocess.check_output([
    'git', 'ls-files', '--others', '--exclude-standard', '--directory',
    '--full-name'], text=True)
  paths = output.split('\n')
  return [os.path.abspath(p) for p in paths if p]


def WriteLists(data, data_deps, build_file, path_prefix):
  if data:
    build_file.write('  data += [\n')
    for path in data:
      if path_prefix:
        path = path_prefix + path
      build_file.write('    "%s",\n' % path)
    build_file.write('  ]\n\n')

  if data_deps:
    build_file.write('  data_deps += [\n')
    for data_dep in data_deps:
      build_file.write('    "%s",\n' % data_dep)
    build_file.write('  ]\n\n')


def ProcessDir(root_path, path, build_file, path_prefix):
  # Write all dirs and files directly under |path| unless they are excluded
  # or need to be processed further because some of their children are excluded.
  # Return a list of dirs that needs to processed further.
  logging.debug('GenerateList for ' + path)
  entries = os.listdir(path)
  entries.sort()
  files = []
  dirs = []
  data_deps = []
  dirs_to_expand = []
  untracked_paths = GetUntrackedPaths()
  for entry in entries:
    full_path = os.path.join(path, entry)
    rel_path = os.path.relpath(full_path, root_path).replace('\\', '/')
    if (any(full_path.startswith(p) for p in untracked_paths) or
        entry.startswith('.') or entry.endswith('~') or
        entry.endswith('.pyc') or entry.endswith('#')):
      logging.debug('ignored %s', rel_path)
      continue
    if rel_path in SEPARATE_TARGETS:
      data_deps.append(SEPARATE_TARGETS[rel_path])
    elif os.path.isfile(full_path):
      if rel_path in EXCLUDED_PATHS:
        logging.debug('excluded %s', rel_path)
      else:
        files.append(rel_path)
    elif os.path.isdir(full_path):
      rel_path = rel_path + '/'
      if rel_path in EXCLUDED_PATHS:
        logging.debug('excluded %s', rel_path)
      elif any(e.startswith(rel_path) for e in EXCLUDED_PATHS):
        dirs_to_expand.append(full_path)
      else:
        dirs.append(rel_path)
    else:
      assert False
  WriteLists(files + dirs,
             data_deps,
             build_file, path_prefix)
  return dirs_to_expand

def WriteBuildFileHeader(build_file):
  build_file.write(LICENSE)
  build_file.write(DO_NOT_EDIT_WARNING)
  build_file.write('import("//build/config/compiler/compiler.gni")\n\n')

def WriteBuildFileBody(build_file, root_path, path_prefix):
  build_file.write(textwrap.dedent(
      """\
      group("%s") {
        testonly = true
        data = []
        data_deps = []

      """ % TELEMETRY_SUPPORT_GROUP_NAME))

  candidates = [root_path]
  while len(candidates) > 0:
    candidate = candidates.pop(0)
    more = ProcessDir(root_path, candidate, build_file, path_prefix)
    candidates.extend(more)

  build_file.write("}")

def GenerateBuildFile(root_path, output_path, chromium):
  CHROMIUM_GROUP = 'group("telemetry_chrome_test_without_chrome")'
  CATAPULT_PREFIX = '//third_party/catapult'
  CATAPULT_GROUP_NAME = CATAPULT_PREFIX + ':' + TELEMETRY_SUPPORT_GROUP_NAME
  TELEMETRY_SUPPORT_GROUP = 'group("%s")' % TELEMETRY_SUPPORT_GROUP_NAME
  if chromium:
    build_file = open(output_path, 'r+')
    contents = build_file.readlines()
    build_file.seek(0)
    remove_telemetry_support_group = False
    for line in contents:
      if TELEMETRY_SUPPORT_GROUP in line:
        # --chromium has already run once, so remove the previously inserted
        # TELEMETRY_SUPPORT_GROUP so we could add an updated one.
        remove_telemetry_support_group = True
        continue
      if remove_telemetry_support_group:
        if line == '}\n':
          remove_telemetry_support_group = False
        continue
      if CHROMIUM_GROUP in line:
        WriteBuildFileBody(build_file, root_path, CATAPULT_PREFIX + '/')
        build_file.write('\n')
      elif CATAPULT_GROUP_NAME in line:
        line = line.replace(CATAPULT_GROUP_NAME,
                            ':' + TELEMETRY_SUPPORT_GROUP_NAME)
      build_file.write(line)
    build_file.close()
  else:
    build_file = open(output_path, 'w')
    WriteBuildFileHeader(build_file)
    WriteBuildFileBody(build_file, root_path, None)
    build_file.close()

def CheckForChanges():
  # Return 0 if no changes are detected; return 1 otherwise.
  root_path = os.path.dirname(os.path.realpath(__file__))
  temp_path = os.path.join(root_path, "TEMP.gn")
  GenerateBuildFile(root_path, temp_path, chromium=False)

  ref_path = os.path.join(root_path, "BUILD.gn")
  if not os.path.exists(ref_path):
    logging.error("Can't localte BUILD.gn!")
    return 1

  temp_file = open(temp_path, 'r')
  temp_content = temp_file.readlines()
  temp_file.close()
  os.remove(temp_path)
  ref_file = open(ref_path, 'r')
  ref_content = ref_file.readlines()
  ref_file.close()

  diff = difflib.unified_diff(temp_content, ref_content, fromfile=temp_path,
                              tofile=ref_path, lineterm='')
  diff_data = []
  for line in diff:
    diff_data.append(line)
  if len(diff_data) > 0:
    logging.error('Diff found. Please rerun generate_telemetry_build.py.')
    logging.debug('\n' + ''.join(diff_data))
    return 1
  logging.debug('No diff found. Everything is good.')
  return 0


def main():
  parser = argparse.ArgumentParser()
  parser.add_argument('-v', '--verbose', action='store_true', default=False,
                      help='Print out debug information')
  parser.add_argument('-c', '--check', action='store_true', default=False,
                      help=('Generate a temporary build file and compare if it '
                            'is the same as the current BUILD.gn'))
  parser.add_argument('--chromium', action='store_true', default=False,
                      help=('Generate the build file into the Chromium '
                            'workspace. This is for the purpose of running '
                            'tryjobs in Chromium.'))
  args = parser.parse_args()
  if args.verbose:
    logging.basicConfig(level=logging.DEBUG)

  if args.check:
    return CheckForChanges()
  if args.chromium:
    root_path = os.path.dirname(os.path.realpath(__file__))
    output_path = os.path.join(
        root_path, "../../tools/perf/chrome_telemetry_build/BUILD.gn")
    GenerateBuildFile(root_path, output_path, chromium=True)
  else:
    root_path = os.path.dirname(os.path.realpath(__file__))
    output_path = os.path.join(root_path, "BUILD.gn")
    GenerateBuildFile(root_path, output_path, chromium=False)
  return 0


if __name__ == '__main__':
  sys.exit(main())
