# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import subprocess

from telemetry.internal.util import binary_manager
from telemetry.internal.platform import platform_backend


class DesktopPlatformBackend(platform_backend.PlatformBackend):

  # This is an abstract class. It is OK to have abstract methods.
  # pylint: disable=abstract-method

  def FlushSystemCacheForDirectory(self, directory):
    assert directory and os.path.exists(directory), \
        'Target directory %s must exist' % directory
    flush_command = binary_manager.FetchPath(
        'clear_system_cache', self.GetOSName(), self.GetArchName())
    assert flush_command, 'You must build clear_system_cache first'

    cmd = [flush_command, '--recurse', os.path.abspath(directory)]
    proc = subprocess.Popen(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT
    )
    stdout, _ = proc.communicate()
    if proc.returncode != 0:
      logging.error('%s exited with unexpected returncode %d. Output:\n%s',
                    cmd, proc.returncode,
                    stdout.decode(errors='backslashreplace'))
      raise subprocess.CalledProcessError(proc.returncode, cmd)

  def GetDeviceTypeName(self):
    return 'Desktop'

  def GetTypExpectationsTags(self):
    tags = super().GetTypExpectationsTags()
    tags.append('desktop')
    return tags
