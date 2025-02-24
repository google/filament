#! /usr/bin/env python3
# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Updates the chromium binaries used by devil.

This currently must be called from the top-level chromium src directory.
"""

import argparse
import collections
import json
import logging
import os
import sys

_DEVIL_ROOT_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..'))

sys.path.append(_DEVIL_ROOT_DIR)
from devil import base_error
from devil import devil_env
from devil.utils import cmd_helper

_DEVICE_ARCHS = [
    {
        'cpu': 'arm',
        'platform': 'android_armeabi-v7a',
    },
    {
        'cpu': 'arm64',
        'platform': 'android_arm64-v8a',
    },
    {
        'cpu': 'x86',
        'platform': 'android_x86',
    },
    {
        'cpu': 'x64',
        'platform': 'android_x86_64',
    },
]
_HOST_ARCH = [{
    # Host binaries use x86_64, not arm, but they build with the
    # host toolchain within a target_cpu="arm" build.
    'cpu': 'arm',
    'platform': 'linux2_x86_64',
}]

_CHROMIUM_DEPS = {
    'chromium_commands': {
        'archs': _HOST_ARCH,
        'build_path': 'lib.java/chromium_commands.dex.jar',
        'target_name': 'chromium_commands_java',
    },
    'forwarder_device': {
        'archs': _DEVICE_ARCHS,
        'build_path': 'device_forwarder',
        'target_name': 'forwarder2',
    },
    'forwarder_host': {
        'archs': _HOST_ARCH,
        'build_path': 'clang_x64/host_forwarder',
        'target_name': 'forwarder2',
    },
    'md5sum_device': {
        'archs': _DEVICE_ARCHS,
        'build_path': 'md5sum_bin',
        'target_name': 'md5sum',
    },
    'md5sum_host': {
        'archs': _HOST_ARCH,
        'build_path': 'clang_x64/md5sum_bin',
        'target_name': 'md5sum',
    },
}


def BuildTargetsForCpu(targets, cpu, output_dir):
  logging.info('Building %s', cpu)

  gn_args = [
      'ffmpeg_branding="Chrome"',
      'is_component_build=false',
      'is_debug=false',
      'proprietary_codecs=true',
      'symbol_level=1',
      'target_cpu="%s"' % cpu,
      'target_os="android"',
  ]

  cmd = ['gn', 'gen', '--args=%s' % (' '.join(gn_args)), output_dir]
  ec = cmd_helper.RunCmd(cmd)
  if ec:
    raise base_error.BaseError('%s failed with %d' % (cmd, ec))

  ec = cmd_helper.RunCmd(['autoninja', '-C', output_dir] + targets)
  if ec:
    raise base_error.BaseError('building %s failed with %d' % (cpu, ec))


def UpdateDependency(dependency_name, dependency_info, local_path, platform):
  bucket = dependency_info['cloud_storage_bucket']
  folder = dependency_info['cloud_storage_base_folder']

  # determine the hash
  ec, sha1sum_output = cmd_helper.GetCmdStatusAndOutput(['sha1sum', local_path])
  if ec:
    raise base_error.BaseError(
        'Failed to determine SHA1 for %s: %s' % (local_path, sha1sum_output))

  dependency_sha1 = sha1sum_output.split()[0]

  # upload
  remote_path = '%s_%s' % (dependency_name, dependency_sha1)
  gs_dest = 'gs://%s/%s/%s' % (bucket, folder, remote_path)
  ec, gsutil_output = cmd_helper.GetCmdStatusAndOutput(
      ['gsutil.py', 'cp', local_path, gs_dest])
  if ec:
    raise base_error.BaseError(
        'Failed to upload %s to %s: %s' % (remote_path, gs_dest, gsutil_output))

  # update entry in json
  file_info = dependency_info['file_info']
  if platform not in file_info:
    file_info[platform] = {
        'cloud_storage_hash': '',
        # the user will need to manually update the download path after
        # uploading a previously unknown dependency.
        'download_path': 'FIXME',
    }
  file_info[platform]['cloud_storage_hash'] = dependency_sha1


def UpdateChromiumDependencies(dependencies, args):
  deps_by_platform = collections.defaultdict(list)
  for dep_name, dep_info in _CHROMIUM_DEPS.items():
    archs = dep_info.get('archs', [])
    for a in archs:
      deps_by_platform[(a.get('cpu'), a.get('platform'))].append(
          (dep_name, dep_info.get('build_path'), dep_info.get('target_name')))

  for arch, arch_deps in deps_by_platform.items():
    targets = [target_name for _n, _b, target_name in arch_deps]
    cpu, platform = arch
    output_dir = os.path.join(args.chromium_src_dir, 'out-devil-deps', platform)
    BuildTargetsForCpu(targets, cpu, output_dir)

    for dep_name, build_path, _ in arch_deps:
      local_path = os.path.abspath(os.path.join(output_dir, build_path))
      UpdateDependency(dep_name,
                       dependencies.get('dependencies', {}).get(dep_name, {}),
                       local_path, platform)

  return dependencies


def UpdateGivenDependency(dependencies, args):
  dep_name = args.name or os.path.basename(args.path)
  if not dep_name in dependencies.get('dependencies', {}):
    raise base_error.BaseError('Could not find dependency "%s" in %s' %
                               (dep_name, args.dependencies_json))

  UpdateDependency(dep_name,
                   dependencies.get('dependencies', {}).get(dep_name, {}),
                   args.path, args.platform)

  return dependencies


def main(raw_args):
  parser = argparse.ArgumentParser(description=__doc__)

  # pylint: disable=protected-access
  parser.add_argument(
      '--dependencies-json',
      type=os.path.abspath,
      default=devil_env._DEVIL_DEFAULT_CONFIG,
      help='Binary dependency configuration file to update.')
  # pylint: enable=protected-access

  subparsers = parser.add_subparsers()
  chromium_parser = subparsers.add_parser('chromium')
  chromium_parser.add_argument(
      '--chromium-src-dir',
      type=os.path.realpath,
      default=os.getcwd(),
      help='Path to chromium/src checkout root.')
  chromium_parser.set_defaults(update_dependencies=UpdateChromiumDependencies)

  dependency_parser = subparsers.add_parser('dependency')
  dependency_parser.add_argument('--name', help='Name of dependency to update.')
  dependency_parser.add_argument(
      '--path',
      type=os.path.abspath,
      help='Path to file to upload as new version of dependency.')
  dependency_parser.add_argument(
      '--platform', help='Platform of dependency to update.')
  dependency_parser.set_defaults(update_dependencies=UpdateGivenDependency)

  args = parser.parse_args(raw_args)

  logging.getLogger().setLevel(logging.INFO)

  with open(args.dependencies_json) as f:
    dependencies = json.load(f)

  dependencies = args.update_dependencies(dependencies, args)

  with open(args.dependencies_json, 'w') as f:
    json.dump(dependencies, f, indent=2, separators=(',', ': '), sort_keys=True)

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
