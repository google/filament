# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# pylint: disable=import-error
# pylint: disable=no-name-in-module
from __future__ import print_function
from __future__ import absolute_import
import distutils.spawn as spawn
import logging
import os
import re
import stat
import subprocess
import sys

from telemetry.internal.platform import desktop_platform_backend


def _BinaryExistsInSudoersFiles(path, sudoers_file_contents):
  """Returns True if the binary in |path| features in the sudoers file.
  """
  for line in sudoers_file_contents.splitlines():
    if re.match(r'\s*\(.+\) NOPASSWD: %s(\s\S+)*$' % re.escape(path), line):
      return True
  return False


def _CanRunElevatedWithSudo(path):
  """Returns True if the binary at |path| appears in the sudoers file.
  If this function returns true then the binary at |path| can be run via sudo
  without prompting for a password.
  """
  sudoers = subprocess.check_output(['/usr/bin/sudo', '-l'], text=True)
  return _BinaryExistsInSudoersFiles(path, sudoers)


class PosixPlatformBackend(desktop_platform_backend.DesktopPlatformBackend):

  # This is an abstract class. It is OK to have abstract methods.
  # pylint: disable=abstract-method

  def HasRootAccess(self):
    return os.getuid() == 0

  def RunCommand(self, args):
    return subprocess.Popen(args, stdout=subprocess.PIPE).communicate()[0]

  def GetFileContents(self, path):
    with open(path, 'r') as f:
      return f.read()

  def CanLaunchApplication(self, application):
    return bool(spawn.find_executable(application))

  def LaunchApplication(
      self, application, parameters=None, elevate_privilege=False):
    assert application, 'Must specify application to launch'

    if os.path.sep not in application:
      application = spawn.find_executable(application)
      assert application, 'Failed to find application in path'

    args = [application]

    if parameters:
      assert isinstance(parameters, list), 'parameters must be a list'
      args += parameters

    def IsElevated():
      """ Returns True if the current process is elevated via sudo i.e. running
      sudo will not prompt for a password. Returns False if not authenticated
      via sudo or if telemetry is run on a non-interactive TTY."""
      # `sudo -v` will always fail if run from a non-interactive TTY.
      p = subprocess.Popen(
          ['/usr/bin/sudo', '-nv'], stdin=subprocess.PIPE,
          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
      stdout = p.communicate()[0]
      # Some versions of sudo set the returncode based on whether sudo requires
      # a password currently. Other versions return output when password is
      # required and no output when the user is already authenticated.
      return not p.returncode and not stdout

    def IsSetUID(path):
      """Returns True if the binary at |path| has the setuid bit set."""
      return (os.stat(path).st_mode & stat.S_ISUID) == stat.S_ISUID

    if elevate_privilege and not IsSetUID(application):
      args = ['/usr/bin/sudo'] + args
      if not _CanRunElevatedWithSudo(application) and not IsElevated():
        if not sys.stdout.isatty():
          # Without an interactive terminal (or a configured 'askpass', but
          # that is rarely relevant), there's no way to prompt the user for
          # sudo. Fail with a helpful error message. For more information, see:
          #   https://code.google.com/p/chromium/issues/detail?id=426720
          text = (
              'Telemetry needs to run %s with elevated privileges, but the '
              'setuid bit is not set and there is no interactive terminal '
              'for a prompt. Please ask an administrator to set the setuid '
              'bit on this executable and ensure that it is owned by a user '
              'with the necessary privileges. Aborting.' % application)
          print(text)
          raise Exception(text)
        # Else, there is a tty that can be used for a useful interactive prompt.
        print('Telemetry needs to run %s under sudo. Please authenticate.' %
              application)
        # Synchronously authenticate.
        subprocess.check_call(['/usr/bin/sudo', '-v'])

    stderror_destination = subprocess.PIPE
    if logging.getLogger().isEnabledFor(logging.DEBUG):
      stderror_destination = None

    return subprocess.Popen(
        args, stdout=subprocess.PIPE, stderr=stderror_destination)
