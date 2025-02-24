#!/usr/bin/env vpython3
#
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Updates the Chrome reference builds.

Usage:
  $ /path/to/update_reference_build.py
  $ git commit -a
  $ git cl upload
"""

from __future__ import print_function
from __future__ import absolute_import
import argparse
import collections
import logging
import os
import six
import shutil
import subprocess
import sys
import tempfile
import zipfile

sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'py_utils'))

from py_utils import cloud_storage
from dependency_manager import base_config


_CHROME_BINARIES_CONFIG = os.path.join(
    os.path.dirname(os.path.abspath(__file__)), '..', '..', 'common',
    'py_utils', 'py_utils', 'chrome_binaries.json')

_CHROME_GS_BUCKET = 'chrome-unsigned'
_CHROMIUM_GS_BUCKET = 'chromium-browser-snapshots'

# How many commit positions to search below and above omaha branch position to
# find closest chromium build snapshot. The value 10 is chosen because it looks
# more than sufficient from manual inspection of the bucket.
_CHROMIUM_SNAPSHOT_SEARCH_WINDOW = 10

# Remove a platform name from this list to disable updating it.
# Add one to enable updating it. (Must also update _PLATFORM_MAP.)
_PLATFORMS_TO_UPDATE = ['mac_arm64', 'mac_x86_64', 'win_x86',
                        'win_AMD64', 'linux_x86_64',
                        'android_k_armeabi-v7a', 'android_l_arm64-v8a',
                        'android_l_armeabi-v7a', 'android_n_armeabi-v7a',
                        'android_n_arm64-v8a', 'android_n_bundle_armeabi-v7a',
                        'android_n_bundle_arm64-v8a']

# Add platforms here if you also want to update chromium binary for it.
# Must add chromium_info for it in _PLATFORM_MAP.
_CHROMIUM_PLATFORMS = ['mac_arm64', 'mac_x86_64', 'win_x86', 'win_AMD64',
                       'linux_x86_64']

# Remove a channel name from this list to disable updating it.
# Add one to enable updating it.
_CHANNELS_TO_UPDATE = ['stable', 'canary', 'dev']


# Omaha is Chrome's autoupdate server. It reports the current versions used
# by each platform on each channel.
_OMAHA_PLATFORMS = { 'stable':  ['mac_arm64', 'mac', 'linux', 'win',
                                 'win64', 'android'],
                    'dev':  ['linux'], 'canary': ['mac', 'win']}


# All of the information we need to update each platform.
#   omaha: name omaha uses for the platforms.
#   zip_name: name of the zip file to be retrieved from cloud storage.
#   gs_build: name of the Chrome build platform used in cloud storage.
#   chromium_info: information needed to update chromium (optional).
#   destination: Name of the folder to download the reference build to.
UpdateInfo = collections.namedtuple('UpdateInfo',
    'omaha, gs_folder, gs_build, chromium_info, zip_name')
# build_dir: name of the build directory in _CHROMIUM_GS_BUCKET.
# zip_name: name of the zip file to be retrieved from cloud storage.
ChromiumInfo = collections.namedtuple('ChromiumInfo', 'build_dir, zip_name')
_PLATFORM_MAP = {'mac_x86_64': UpdateInfo(
                     omaha='mac',
                     gs_folder='desktop-*',
                     gs_build='mac64',
                     chromium_info=ChromiumInfo(
                         build_dir='Mac',
                         zip_name='chrome-mac.zip'),
                     zip_name='chrome-mac.zip'),
                 'mac_arm64': UpdateInfo(
                     omaha='mac_arm64',
                     gs_folder='desktop-*',
                     gs_build='mac-arm64',
                     chromium_info=ChromiumInfo(
                         build_dir='Mac_Arm',
                         zip_name='chrome-mac.zip',
                     ),
                     zip_name='chrome-mac.zip'),
                 'win_x86': UpdateInfo(
                     omaha='win',
                     gs_folder='desktop-*',
                     gs_build='win-clang',
                     chromium_info=ChromiumInfo(
                         build_dir='Win',
                         zip_name='chrome-win.zip'),
                     zip_name='chrome-win-clang.zip'),
                 'win_AMD64': UpdateInfo(
                     omaha='win',
                     gs_folder='desktop-*',
                     gs_build='win64-clang',
                     chromium_info=ChromiumInfo(
                        build_dir='Win_x64',
                        zip_name='chrome-win.zip'),
                     zip_name='chrome-win64-clang.zip'),
                 'linux_x86_64': UpdateInfo(
                     omaha='linux',
                     gs_folder='desktop-*',
                     gs_build='linux64',
                     chromium_info=ChromiumInfo(
                         build_dir='Linux_x64',
                         zip_name='chrome-linux.zip'),
                     zip_name='chrome-linux64.zip'),
                 'android_k_armeabi-v7a': UpdateInfo(
                     omaha='android',
                     gs_folder='android-*',
                     gs_build='arm',
                     chromium_info=None,
                     zip_name='Chrome.apk'),
                 'android_l_arm64-v8a': UpdateInfo(
                     omaha='android',
                     gs_folder='android-*',
                     gs_build='arm_64',
                     chromium_info=None,
                     zip_name='ChromeModern.apk'),
                 'android_l_armeabi-v7a': UpdateInfo(
                     omaha='android',
                     gs_folder='android-*',
                     gs_build='arm',
                     chromium_info=None,
                     zip_name='Chrome.apk'),
                 'android_n_armeabi-v7a': UpdateInfo(
                     omaha='android',
                     gs_folder='android-*',
                     gs_build='arm',
                     chromium_info=None,
                     zip_name='Monochrome.apk'),
                 'android_n_arm64-v8a': UpdateInfo(
                     omaha='android',
                     gs_folder='android-*',
                     gs_build='arm_64',
                     chromium_info=None,
                     zip_name='Monochrome.apk'),
                 'android_n_bundle_armeabi-v7a': UpdateInfo(
                     omaha='android',
                     gs_folder='android-*',
                     gs_build='arm',
                     chromium_info=None,
                     zip_name='Monochrome.apks'),
                 'android_n_bundle_arm64-v8a': UpdateInfo(
                     omaha='android',
                     gs_folder='android-*',
                     gs_build='arm_64',
                     chromium_info=None,
                     zip_name='Monochrome.apks')
}

VersionInfo = collections.namedtuple('VersionInfo',
                                     'version, branch_base_position')


def _ChannelVersionsMap(channel):
  rows = _OmahaReportVersionInfo(channel)
  omaha_versions_map = _OmahaVersionsMap(rows, channel)
  channel_versions_map = {}
  for platform in _PLATFORMS_TO_UPDATE:
    omaha_platform = _PLATFORM_MAP[platform].omaha
    if omaha_platform in omaha_versions_map:
      channel_versions_map[platform] = omaha_versions_map[omaha_platform]
  return channel_versions_map


def _OmahaReportVersionInfo(channel):
  url ='https://omahaproxy.appspot.com/all?channel=%s' % channel
  lines = six.moves.urllib.request.urlopen(url).readlines()
  return [six.ensure_str(l).split(',') for l in lines]


def _OmahaVersionsMap(rows, channel):
  platforms = _OMAHA_PLATFORMS.get(channel, [])
  if (len(rows) < 1 or
      rows[0][0:3] != ['os', 'channel', 'current_version'] or
      rows[0][7] != 'branch_base_position'):
    raise ValueError(
        'Omaha report is not in the expected form: %s.' % rows)
  versions_map = {}
  for row in rows[1:]:
    if row[1] != channel:
      raise ValueError(
          'Omaha report contains a line with the channel %s' % row[1])
    if row[0] in platforms:
      versions_map[row[0]] = VersionInfo(version=row[2],
                                         branch_base_position=int(row[7]))
  logging.warn('versions map: %s' % versions_map)
  if not all(platform in versions_map for platform in platforms):
    raise ValueError(
        'Omaha report did not contain all desired platforms '
        'for channel %s' % channel)
  return versions_map


RemotePath = collections.namedtuple('RemotePath', 'bucket, path')


def _ResolveChromeRemotePath(platform_info, version_info):
  # Path example: desktop-*/30.0.1595.0/precise32/chrome-precise32.zip
  return RemotePath(bucket=_CHROME_GS_BUCKET,
                    path=('%s/%s/%s/%s' % (platform_info.gs_folder,
                                           version_info.version,
                                           platform_info.gs_build,
                                           platform_info.zip_name)))


def _FindClosestChromiumSnapshot(base_position, build_dir):
  """Returns the closest chromium snapshot available in cloud storage.

  Chromium snapshots are pulled from _CHROMIUM_BUILD_DIR in CHROMIUM_GS_BUCKET.

  Continuous chromium snapshots do not always contain the exact release build.
  This function queries the storage bucket and find the closest snapshot within
  +/-_CHROMIUM_SNAPSHOT_SEARCH_WINDOW to find the closest build.
  """
  min_position = base_position - _CHROMIUM_SNAPSHOT_SEARCH_WINDOW
  max_position = base_position + _CHROMIUM_SNAPSHOT_SEARCH_WINDOW

  # Getting the full list of objects in cloud storage bucket is prohibitively
  # slow. It's faster to list objects with a prefix. Assuming we're looking at
  # +/- 10 commit positions, for commit position 123456, we want to look at
  # positions between 123446 an 123466. We do this by getting all snapshots
  # with prefix 12344*, 12345*, and 12346*. This may get a few more snapshots
  # that we intended, but that's fine since we take the min distance anyways.
  min_position_prefix = min_position // 10;
  max_position_prefix = max_position // 10;

  available_positions = []
  for position_prefix in range(min_position_prefix, max_position_prefix + 1):
    query = '%s/%d*' % (build_dir, position_prefix)
    try:
      ls_results = cloud_storage.ListDirs(_CHROMIUM_GS_BUCKET, query)
    except cloud_storage.NotFoundError:
      # It's fine if there is no chromium snapshot available for one prefix.
      # We will look at the rest of the prefixes.
      continue

    for entry in ls_results:
      # entry looks like '/Linux_x64/${commit_position}/'.
      position = int(entry.split('/')[2])
      available_positions.append(position)

  if len(available_positions) == 0:
    raise ValueError('No chromium build found +/-%d commit positions of %d' %
                     (_CHROMIUM_SNAPSHOT_SEARCH_WINDOW, base_position))

  distance_function = lambda position: abs(position - base_position)
  min_distance_snapshot = min(available_positions, key=distance_function)
  return min_distance_snapshot


def _ResolveChromiumRemotePath(channel, platform, version_info):
  platform_info = _PLATFORM_MAP[platform]
  branch_base_position = version_info.branch_base_position
  omaha_version = version_info.version
  build_dir = platform_info.chromium_info.build_dir
  # Look through chromium-browser-snapshots for closest match.
  closest_snapshot = _FindClosestChromiumSnapshot(
      branch_base_position, build_dir)
  if closest_snapshot != branch_base_position:
    print('Channel %s corresponds to commit position ' % channel +
          '%d on %s, ' % (branch_base_position, platform) +
          'but closest chromium snapshot available on ' +
          '%s is %d' % (_CHROMIUM_GS_BUCKET, closest_snapshot))
  return RemotePath(bucket=_CHROMIUM_GS_BUCKET,
                    path = ('%s/%s/%s' % (build_dir, closest_snapshot,
                                        platform_info.chromium_info.zip_name)))


def _QueuePlatformUpdate(binary, platform, version_info, config, channel):
  """ platform: the name of the platform for the browser to
      be downloaded & updated from cloud storage. """
  platform_info = _PLATFORM_MAP[platform]

  if binary == 'chrome':
    remote_path = _ResolveChromeRemotePath(platform_info, version_info)
  elif binary == 'chromium':
    remote_path = _ResolveChromiumRemotePath(channel, platform, version_info)
  else:
    raise ValueError('binary must be \'chrome\' or \'chromium\'')

  if not cloud_storage.Exists(remote_path.bucket, remote_path.path):
    cloud_storage_path = 'gs://%s/%s' % (remote_path.bucket, remote_path.path)
    logging.warn('Failed to find %s build for version %s at path %s.' % (
        platform, version_info.version, cloud_storage_path))
    logging.warn('Skipping this update for this platform/channel.')
    return

  reference_builds_folder = os.path.join(
      os.path.dirname(os.path.abspath(__file__)), 'chrome_telemetry_build',
      'reference_builds', binary, channel)
  if not os.path.exists(reference_builds_folder):
    os.makedirs(reference_builds_folder)
  local_dest_path = os.path.join(reference_builds_folder,
                                 platform,
                                 platform_info.zip_name)
  cloud_storage.Get(remote_path.bucket, remote_path.path, local_dest_path)
  _ModifyBuildIfNeeded(binary, local_dest_path, platform)
  config.AddCloudStorageDependencyUpdateJob('%s_%s' % (binary, channel),
      platform, local_dest_path, version=version_info.version,
      execute_job=False)


def _ModifyBuildIfNeeded(binary, location, platform):
  """Hook to modify the build before saving it for Telemetry to use.

  This can be used to remove various utilities that cause noise in a
  test environment. Right now, it is just used to remove Keystone,
  which is a tool used to autoupdate Chrome.
  """
  if binary != 'chrome':
    return

  if platform in ['mac_x86_64', 'mac_arm64']:
    _RemoveKeystoneFromBuild(location)
    return

  if 'mac' in platform:
    raise NotImplementedError(
        'Platform <%s> sounds like it is an OSX version. If so, we may need to '
        'remove Keystone from it per crbug.com/932615. Please edit this script'
        ' and teach it what needs to be done :).')


def _RemoveKeystoneFromBuild(location):
  """Removes the Keystone autoupdate binary from the chrome mac zipfile."""
  logging.info('Removing keystone from mac build at %s' % location)
  temp_folder = tempfile.mkdtemp(prefix='RemoveKeystoneFromBuild')
  try:
    subprocess.check_call(['unzip', '-q', location, '-d', temp_folder])
    keystone_folder = os.path.join(
        temp_folder, 'chrome-mac', 'Google Chrome.app', 'Contents',
        'Frameworks', 'Google Chrome Framework.framework', 'Frameworks',
        'KeystoneRegistration.framework')
    shutil.rmtree(keystone_folder)
    os.remove(location)
    subprocess.check_call(['zip', '--quiet', '--recurse-paths', '--symlinks',
                           location, 'chrome-mac'],
                           cwd=temp_folder)
  finally:
    shutil.rmtree(temp_folder)


def _NeedsUpdate(config, binary, channel, platform, version_info):
  channel_version = version_info.version
  print('Checking %s (%s channel) on %s' % (binary, channel, platform))
  current_version = config.GetVersion('%s_%s' % (binary, channel), platform)
  print('current: %s, channel: %s' % (current_version, channel_version))
  if current_version and current_version == channel_version:
    print('Already up to date.')
    return False
  return True


def UpdateBuilds(args):
  config = base_config.BaseConfig(_CHROME_BINARIES_CONFIG, writable=True)
  for channel in _CHANNELS_TO_UPDATE:
    channel_versions_map = _ChannelVersionsMap(channel)
    for platform in channel_versions_map:
      version_info = channel_versions_map.get(platform)
      if args.update_chrome:
        if _NeedsUpdate(config, 'chrome', channel, platform, version_info):
          _QueuePlatformUpdate('chrome', platform, version_info, config,
                               channel)
      if args.update_chromium and platform in _CHROMIUM_PLATFORMS:
        if _NeedsUpdate(config, 'chromium', channel, platform, version_info):
          _QueuePlatformUpdate('chromium', platform, version_info,
                               config, channel)

  print('Updating builds with downloaded binaries')
  config.ExecuteUpdateJobs(force=True)


def main():
  logging.getLogger().setLevel(logging.DEBUG)
  parser = argparse.ArgumentParser(
      description='Update reference binaries used by perf bots.')
  parser.add_argument('--no-update-chrome', action='store_false',
                      dest='update_chrome', default=True,
                      help='do not update chrome binaries')
  parser.add_argument('--no-update-chromium', action='store_false',
                      dest='update_chromium', default=True,
                      help='do not update chromium binaries')
  args = parser.parse_args()
  UpdateBuilds(args)

if __name__ == '__main__':
  main()
