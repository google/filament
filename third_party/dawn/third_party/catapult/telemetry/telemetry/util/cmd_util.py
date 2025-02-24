# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""A wrapper for common cmd operations"""

from __future__ import absolute_import
import logging
import os
import subprocess


def RunCmd(args, cwd=None, quiet=False, env=None):
  """Opens a subprocess to execute a program and returns its return value.

  Args:
    args: A string or a sequence of program arguments. The program to execute is
      the string or the first item in the args sequence.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.

  Returns:
    Return code from the command execution.
  """
  if not quiet:
    logging.debug(' '.join(args) + ' ' + (cwd or ''))
  with open(os.devnull, 'w') as devnull:
    p = subprocess.Popen(
        args=args,
        cwd=cwd,
        stdout=devnull,
        stderr=devnull,
        stdin=devnull,
        shell=False,
        env=env)
    return p.wait()


def GetAllCmdOutput(args, cwd=None, quiet=False, env=None):
  """Open a subprocess to execute a program and returns its output.

  Args:
    args: A string or a sequence of program arguments. The program to execute is
      the string or the first item in the args sequence.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.

  Returns:
    Captures and returns the command's stdout.
    Prints the command's stderr to logger (which defaults to stdout).
  """
  if not quiet:
    logging.debug(' '.join(args) + ' ' + (cwd or ''))
  with open(os.devnull, 'w') as devnull:
    p = subprocess.Popen(
        args=args,
        cwd=cwd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        stdin=devnull,
        env=env)
    stdout, stderr = p.communicate()
    if not quiet:
      logging.debug(' > stdout=[%s], stderr=[%s]', stdout, stderr)
    return stdout, stderr


def StartCmd(args,
             cwd=None,
             quiet=False,
             stdout=None,
             stderr=None,
             env=None):
  """Starts a subprocess to execute a program and returns its handle.

  Args:
    args: A string or a sequence of program arguments. The program to execute is
      the string or the first item in the args sequence.
    cwd: If not None, the subprocess's current directory will be changed to
      |cwd| before it's executed.
    stdout: Optional pipe to override subprocess STDOUT pipe.
    stderr: Optional pipe to override subprocess STDERR pipe.


  Returns:
     An instance of subprocess.Popen associated with the live process.
  """
  if not quiet:
    logging.debug(' '.join(args) + ' ' + (cwd or ''))
  stdout = stdout or subprocess.PIPE
  stderr = stderr or subprocess.PIPE
  return subprocess.Popen(
      args=args, cwd=cwd, stdout=stdout, stderr=stderr, env=env)


def HasSSH():
  try:
    RunCmd(['ssh'], quiet=True)
    RunCmd(['scp'], quiet=True)
    logging.debug("HasSSH()->True")
    return True
  except OSError:
    logging.debug("HasSSH()->False")
    return False
