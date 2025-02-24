# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function
from __future__ import division
from __future__ import absolute_import

import contextlib
import logging
import os
import subprocess
import sys
import time
import shutil


def _AddToPathIfNeeded(path):
  if path not in sys.path:
    sys.path.insert(0, path)


@contextlib.contextmanager
def Chdir(path):
  pwd = os.getcwd()
  try:
    yield os.chdir(path)
  finally:
    os.chdir(pwd)


def PackPinpoint(catapult_path, temp_dir, deployment_paths):
  with Chdir(catapult_path):
    _AddToPathIfNeeded(os.path.join(catapult_path, 'common', 'node_runner'))
    # pylint: disable=import-outside-toplevel
    from node_runner import node_util
    node_path = node_util.GetNodePath()
    node_modules = node_util.GetNodeModulesPath()

    def PinpointRelativePath(*components):
      return os.path.join('dashboard', 'pinpoint', *components)

    # When packing Pinpoint, we need some extra symlinks in the temporary
    # directory, so we can find the correct elements at bundle time. This is
    # simulating the paths we would be serving as defined in the pinpoint.yaml
    # file.
    shutil.copytree(
        os.path.join(catapult_path, 'dashboard', 'dashboard', 'pinpoint',
                     'elements'), os.path.join(temp_dir, 'elements'))
    shutil.copytree(
        os.path.join(catapult_path, 'third_party', 'polymer', 'components'),
        os.path.join(temp_dir, 'components'))
    shutil.copytree(
        os.path.join(catapult_path, 'third_party', 'd3'),
        os.path.join(temp_dir, 'd3'))

    # We don't yet use any webpack in Pinpoint, so let's use the polymer bundler
    # for now.
    bundler_cmd = [
        node_path,
        os.path.join(node_modules, 'polymer-bundler', 'lib', 'bin',
                     'polymer-bundler.js'),
        '--inline-scripts',
        '--inline-css',
        # Exclude some paths from the bundling.
        '--exclude',
        '//fonts.googleapis.com/*',
        '--exclude',
        '//apis.google.com/*',
        # Then set up the rest of the options for the bundler.
        '--out-dir',
        os.path.join(temp_dir, 'bundled'),
        '--root',
        temp_dir,
        '--treeshake',
    ]

    # Change to the temporary directory, and run the bundler from there.
    with Chdir(temp_dir):
      bundler_cmd.extend(
          ['--in-file',
           PinpointRelativePath('index', 'index.html')])

      logging.info('Bundler Command:\n%s', ' '.join(bundler_cmd))

      proc = subprocess.Popen(
          bundler_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      _, bundler_err = proc.communicate()
      if proc.returncode != 0:
        print('ERROR from bundler:')
        print(bundler_err)
        raise RuntimeError('Vulcanize failed with exit code', proc.returncode)

      deployment_paths.append(os.path.join(temp_dir, 'bundled'))


def AddTimestamp(js_name):
  # V2SPA displays its version as this timestamp in this format to make it easy
  # to check whether a change is visible.
  now = time.time()
  print('vulcanized',
        time.strftime('%Y-%m-%d %H:%M:%S', time.gmtime(now - (60 * 60 * 7))))

  js = open(js_name).read()
  with open(js_name, 'w') as fp:
    fp.write('window.VULCANIZED_TIMESTAMP=new Date(%d);\n' % (now * 1000))
    fp.write(js)
