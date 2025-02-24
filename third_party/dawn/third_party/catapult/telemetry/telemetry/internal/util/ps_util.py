# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import inspect
import logging
import threading
import os
import subprocess

try:
  import psutil
except ImportError:
  psutil = None

import py_utils
from py_utils import atexit_with_log


def _GetProcessDescription(process):
  if psutil is None:
    return 'unable to get process description without psutil'
  try:
    if inspect.ismethod(process.name):
      name = process.name()
    else:  # Process.name is a property in old versions of psutil.
      name = process.name
    if inspect.ismethod(process.cmdline):
      cmdline = process.cmdline()
    else:
      cmdline = process.cmdline
    return '%s (%s) - %s' % (name, process.pid, cmdline)
  except (psutil.NoSuchProcess, psutil.ZombieProcess, psutil.AccessDenied
         ) as e:
    return 'unknown (%s): %r' % (
        process.pid, e)


def _GetAllSubprocesses():
  if psutil is None:
    logging.warning(
        'psutil is not installed on the system. Not listing possible '
        'leaked processes. To install psutil, see: '
        'https://pypi.python.org/pypi/psutil')
    return []
  telemetry_pid = os.getpid()
  parent = psutil.Process(telemetry_pid)
  if hasattr(parent, 'children'):
    children = parent.children(recursive=True)
  else:  # Some old version of psutil use get_children instead children.
    children = parent.get_children()
  return children


def ListAllSubprocesses():
  children = _GetAllSubprocesses()
  if children:
    processes_info = []
    for p in children:
      processes_info.append(_GetProcessDescription(p))
    logging.warning('Running sub processes (%i processes):\n%s',
                    len(children), '\n'.join(processes_info))

def GetAllSubprocessIDs():
  children = _GetAllSubprocesses()
  processes_id = []
  if children:
    for p in children:
      processes_id.append(p.pid)
  return processes_id


def RunSubProcWithTimeout(args, timeout, process_name):
  # crbug.com/1036447. Added for handle mac screen shot.
  # TODO(crbug.com/984504): Use built-in timeout after python 3 lands.
  sp = subprocess.Popen(args)
  try:
    # Wait for the process to return
    py_utils.WaitFor(
        lambda: sp.poll() is not None,
        timeout)
  except py_utils.TimeoutException:
    logging.warning(
        ('Process %s (pid: %s) failed to finish after %ds. Will terminate it.' %
         (process_name, sp.pid, timeout)))
    threading.Thread(
        target=_TerminateOrKillProcess,
        args=(sp, process_name)
    ).start()
    raise
  return sp


def _TerminateOrKillProcess(process, process_name):
  done = False
  pid = process.id
  try:
    process.terminate()
    py_utils.WaitFor(
        lambda: process.poll() is not None,
        10)
    done = True
  except py_utils.TimeoutException:
    try:
      process.kill()
      py_utils.WaitFor(
          lambda: process.poll() is not None,
          10)
      done = True
    except py_utils.TimeoutException:
      pass
  if not done:
    logging.warning(
        'Failed to terminate/kill the process %s (pid: %s) after 20 seconds.'
        % (process_name, pid))


def EnableListingStrayProcessesUponExitHook():
  atexit_with_log.Register(ListAllSubprocesses)
