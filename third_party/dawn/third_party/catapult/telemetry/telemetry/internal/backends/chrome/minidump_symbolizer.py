# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import multiprocessing
import os
import shutil
import subprocess
import sys
import tempfile
import time

import six

from dependency_manager import exceptions as dependency_exceptions
from telemetry.internal.util import local_first_binary_manager


class MinidumpSymbolizer():
  def __init__(self, os_name, arch_name, dump_finder, build_dir,
               symbols_dir=None):
    """Abstract class for handling all minidump symbolizing code.

    Args:
      os_name: The OS of the host (if running the test on a device), or the OS
          of the test machine (if running the test locally).
      arch_name: The arch name of the host (if running the test on a device), or
          the OS of the test machine (if running the test locally).
      dump_finder: The minidump_finder.MinidumpFinder instance that is being
          used to find minidumps for the test.
      build_dir: The directory containing Chromium build artifacts to generate
          symbols from.
      symbols_dir: An optional path to a directory to store symbols for re-use.
          Re-using symbols will result in faster symbolization times, but the
          provided directory *must* be unique per browser binary, e.g. by
          including the hash of the binary in the directory name.
    """
    self._os_name = os_name
    self._arch_name = arch_name
    self._dump_finder = dump_finder
    self._build_dir = build_dir
    self._symbols_dir = symbols_dir

  def SymbolizeMinidump(self, minidump):
    """Gets the stack trace from the given minidump.

    Args:
      minidump: the path to the minidump on disk

    Returns:
      None if the stack could not be retrieved for some reason, otherwise a
      string containing the stack trace.
    """
    stackwalk = local_first_binary_manager.GetInstance().FetchPath(
        'minidump_stackwalk')
    if not stackwalk:
      logging.warning('minidump_stackwalk binary not found.')
      return None
    # We only want this logic on linux platforms that are still using breakpad.
    # See crbug.com/667475
    if not self._dump_finder.MinidumpObtainedFromCrashpad(minidump):
      with open(minidump, 'rb') as infile:
        minidump += '.stripped'
        with open(minidump, 'wb') as outfile:
          outfile.write(b''.join(infile.read().partition(b'MDMP')[1:]))

    symbols_dir = self._symbols_dir
    if not symbols_dir:
      symbols_dir = tempfile.mkdtemp()
    try:
      self._GenerateBreakpadSymbols(symbols_dir, minidump)
      output = subprocess.check_output([stackwalk, minidump, symbols_dir],
                                       stderr=open(os.devnull, 'w'))
      # This can be removed once fully switched to Python 3 by passing text=True
      # to the check_output call above.
      if not isinstance(output, six.string_types):
        output = output.decode('utf-8')
      return output
    finally:
      if not self._symbols_dir:
        shutil.rmtree(symbols_dir)

  def GetSymbolBinaries(self, minidump):
    """Returns a list of paths to binaries where symbols may be located.

    Args:
      minidump: The path to the minidump being symbolized.
    """
    raise NotImplementedError()

  def GetBreakpadPlatformOverride(self):
    """Returns the platform to be passed to generate_breakpad_symbols."""
    return None

  def _GenerateBreakpadSymbols(self, symbols_dir, minidump):
    """Generates Breakpad symbols for use with stackwalking tools.

    Args:
      symbols_dir: The directory where symbols will be written to.
      minidump: The path to the minidump being symbolized.
    """
    logging.info('Dumping Breakpad symbols.')
    try:
      generate_breakpad_symbols_command = \
          local_first_binary_manager.GetInstance().FetchPath(
              'generate_breakpad_symbols')
    except dependency_exceptions.NoPathFoundError as e:
      logging.warning('Failed to get generate_breakpad_symbols: %s', e)
      generate_breakpad_symbols_command = None
    if not generate_breakpad_symbols_command:
      logging.warning(
          'generate_breakpad_symbols binary not found, cannot symbolize '
          'minidumps')
      return

    try:
      dump_syms_path = local_first_binary_manager.GetInstance().FetchPath(
          'dump_syms')
    except dependency_exceptions.NoPathFoundError as e:
      logging.warning('Failed to get dump_syms: %s', e)
      dump_syms_path = None
    if not dump_syms_path:
      logging.warning('dump_syms binary not found, cannot symbolize minidumps')
      return

    symbol_binaries = self.GetSymbolBinaries(minidump)

    cmds = []
    cached_binaries = []
    missing_binaries = []
    for binary_path in symbol_binaries:
      if not os.path.exists(binary_path):
        missing_binaries.append(binary_path)
        continue
      # Skip dumping symbols for binaries if they already exist in the symbol
      # directory, i.e. whatever is using this symbolizer has opted to cache
      # symbols. The directory will contain a directory with the binary name if
      # it has already been dumped.
      cache_path = os.path.join(symbols_dir, os.path.basename(binary_path))
      if os.path.exists(cache_path) and os.path.isdir(cache_path):
        cached_binaries.append(binary_path)
        continue
      cmd = [
          sys.executable,
          generate_breakpad_symbols_command,
          f'--binary={binary_path}',
          f'--symbols-dir={symbols_dir}',
          f'--build-dir={self._build_dir}',
          f'--dump-syms-path={dump_syms_path}',
      ]
      if self.GetBreakpadPlatformOverride():
        cmd.append('--platform=%s' % self.GetBreakpadPlatformOverride())
      cmds.append(cmd)

    if missing_binaries:
      logging.warning(
          'Unable to find %d of %d binaries for minidump symbolization. This '
          'is likely not an actual issue, but is worth investigating if the '
          'minidump fails to symbolize properly.',
          len(missing_binaries), len(symbol_binaries))
      # 5 is arbitrary, but a reasonable number of paths to print out.
      if len(missing_binaries) < 5:
        logging.warning('Missing binaries: %s', missing_binaries)
      else:
        logging.warning(
            'Run test with high verbosity to get the list of missing binaries.')
        logging.debug('Missing binaries: %s', missing_binaries)

    if cached_binaries:
      logging.info(
          'Skipping symbol dumping for %d of %d binaries due to cached symbols '
          'being present.', len(cached_binaries), len(symbol_binaries))
      if len(cached_binaries) < 5:
        logging.info('Skipped binaries: %s', cached_binaries)
      else:
        logging.info(
            'Run test with high verbosity to get the list of binaries with '
            'cached symbols.')
        logging.debug('Skipped binaries: %s', cached_binaries)

    # We need to prevent the number of file handles that we open from reaching
    # the soft limit set for the current process. This can either be done by
    # ensuring that the limit is suitably large using the resource module or by
    # only starting a relatively small number of subprocesses at once. In order
    # to prevent any potential issues with messing with system limits, that
    # latter is chosen.
    # Typically, this would be handled by using the multiprocessing module's
    # pool functionality, but importing generate_breakpad_symbols and invoking
    # it directly takes significantly longer than alternatives for whatever
    # reason, even if they appear to perform more work. Thus, we could either
    # have each task in the pool create its own subprocess that runs the
    # command or manually limit the number of subprocesses we have at any
    # given time. We go with the latter option since it should be less
    # wasteful.
    processes = {}
    # Symbol dumping is somewhat I/O constrained, so use double the number of
    # logical cores on the system.
    process_limit = multiprocessing.cpu_count() * 2
    while cmds or processes:
      # Clear any processes that have finished.
      processes_to_delete = []
      for p in processes:
        if p.poll() is not None:
          stdout, stderr = p.communicate()
          if p.returncode:
            logging.error(stdout)
            logging.error(stderr)
            logging.warning('Failed to execute %s', processes[p])
          processes_to_delete.append(p)
      for p in processes_to_delete:
        del processes[p]
      # Add as many more processes as we can.
      while len(processes) < process_limit and cmds:
        cmd = cmds.pop(-1)
        p = subprocess.Popen(cmd,
                             text=True,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT)
        processes[p] = cmd
      # 1 second is fairly arbitrary, but strikes a reasonable balance between
      # spending too many cycles checking the current state of running
      # processes and letting cores sit idle.
      time.sleep(1)
