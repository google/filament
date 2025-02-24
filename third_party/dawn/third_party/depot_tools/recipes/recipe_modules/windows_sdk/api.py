# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""The `windows_sdk` module provides safe functions to access a hermetic
Microsoft Visual Studio installation which is derived from Chromium's MSVC
toolchain.

See (internal):
  * go/chromium-msvc-toolchain
  * go/windows-sdk-cipd-update

Available only on Google-run bots.
"""

import collections
from contextlib import contextmanager

from recipe_engine import recipe_api


class WindowsSDKApi(recipe_api.RecipeApi):
  """API for using Windows SDK distributed via CIPD."""

  SDKPaths = collections.namedtuple('SDKPaths', ['win_sdk', 'dia_sdk'])

  def __init__(self, sdk_properties, *args, **kwargs):
    super(WindowsSDKApi, self).__init__(*args, **kwargs)

    self._sdk_properties = sdk_properties

  @contextmanager
  def __call__(self, path=None, version=None, enabled=True, target_arch='x64'):
    """Sets up the SDK environment when enabled.

    Args:
      * path (path): Path to a directory where to install the SDK
        (default is api.path.cache_dir / 'windows_sdk')
      * version (str): CIPD version of the SDK
        (default is set via $infra/windows_sdk.version property)
      * enabled (bool): Whether the SDK should be used or not.
      * target_arch (str): 'x86', 'x64', or 'arm64'

    Yields:
      If enabled, yields SDKPaths object with paths to well-known roots within
      the deployed bundle:
        * win_sdk - a Path to the root of the extracted Windows SDK.
        * dia_sdk - a Path to the root of the extracted Debug Interface Access
          SDK.

    Raises:
        StepFailure or InfraFailure.
    """
    if enabled:
      sdk_dir = self._ensure_sdk(
          path or self.m.path.cache_dir / 'windows_sdk',
          version or self._sdk_properties['version'])
      try:
        with self.m.context(**self._sdk_env(sdk_dir, target_arch)):
          yield WindowsSDKApi.SDKPaths(
              sdk_dir / 'win_sdk',
              sdk_dir / 'DIA SDK')
      finally:
        # cl.exe automatically starts background mspdbsrv.exe daemon which
        # needs to be manually stopped so Swarming can tidy up after itself.
        #
        # Since mspdbsrv may not actually be running, don't fail if we can't
        # actually kill it.
        self.m.step('taskkill mspdbsrv',
                    ['taskkill.exe', '/f', '/t', '/im', 'mspdbsrv.exe'],
                    ok_ret='any')
    else:
      yield

  def _ensure_sdk(self, sdk_dir, sdk_version):
    """Ensures the Windows SDK CIPD package is installed.

    Returns the directory where the SDK package has been installed.

    Args:
      * path (path): Path to a directory.
      * version (str): CIPD instance ID, tag or ref.
    """
    with self.m.context(infra_steps=True):
      pkgs = self.m.cipd.EnsureFile()
      pkgs.add_package('chrome_internal/third_party/sdk/windows', sdk_version)
      self.m.cipd.ensure(sdk_dir, pkgs)
      return sdk_dir

  def _sdk_env(self, sdk_dir, target_arch):
    """Constructs the environment for the SDK.

    Returns environment and environment prefixes.

    Args:
      * sdk_dir (path): Path to a directory containing the SDK.
      * target_arch (str): 'x86', 'x64', or 'arm64'
    """
    env = {}
    env_prefixes = {}

    if target_arch not in ('x86', 'x64', 'arm64'):
      raise ValueError('unknown architecture {!r}'.format(target_arch))

    data = self.m.step('read SetEnv json', [
        'python3',
        self.resource('find_env_json.py'),
        '--sdk_root',
        sdk_dir,
        '--target_arch',
        target_arch,
        '--output_json',
        self.m.json.output(),
    ],
                       step_test_data=lambda: self.m.json.test_api.output({
                           'env': {
                               'PATH': [['..', '..', 'win_sdk', 'bin', 'x64']],
                               'VSINSTALLDIR': [['..', '..\\']],
                           },
                       })).json.output.get('env')
    for key in data:
      # SDK cipd packages prior to 10.0.19041.0 contain entries like:
      #  "INCLUDE": [["..","..","win_sdk","Include","10.0.17134.0","um"], and
      # recipes' Path() does not like .., ., \, or /, so this is cumbersome.
      # What we want to do is:
      #   [sdk_bin_dir.joinpath(*e) for e in env[k]]
      # Instead do that badly, and rely (but verify) on the fact that the paths
      # are all specified relative to the root, but specified relative to
      # win_sdk/bin (i.e. everything starts with "../../".)
      #
      # For 10.0.19041.0 and later, the cipd SDK package json is like:
      #  "INCLUDE": [["Windows Kits","10","Include","10.0.19041.0","um"], so
      # we simply join paths there.
      results = []
      for value in data[key]:
        if value[0] == '..' and (value[1] == '..' or value[1] == '..\\'):
          results.append('%s' % sdk_dir.joinpath(*value[2:]))
        else:
          results.append('%s' % sdk_dir.joinpath(*value))

      # PATH is special-cased because we don't want to overwrite other things
      # like C:\Windows\System32. Others are replacements because prepending
      # doesn't necessarily makes sense, like VSINSTALLDIR.
      if key.lower() == 'path':
        env_prefixes[key] = results
      else:
        env[key] = ';'.join(results)

    return {'env': env, 'env_prefixes': env_prefixes}
