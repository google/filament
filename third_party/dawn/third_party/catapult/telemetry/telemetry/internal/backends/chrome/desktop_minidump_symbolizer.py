# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import logging
import os
import re
import subprocess

import six

from telemetry.internal.backends.chrome import minidump_symbolizer
from telemetry.internal.util import local_first_binary_manager
from telemetry.internal.util import path

class DesktopMinidumpSymbolizer(minidump_symbolizer.MinidumpSymbolizer):
  def __init__(self, os_name, arch_name, dump_finder, build_dir,
               symbols_dir=None):
    """Class for handling all minidump symbolizing code on Desktop platforms.

    Args:
      os_name: The OS of the test machine.
      arch_name: The arch name of the test machine.
      dump_finder: The minidump_finder.MinidumpFinder instance that is being
          used to find minidumps for the test.
      build_dir: The directory containing Chromium build artifacts to generate
          symbols from.
      symbols_dir: An optional path to a directory to store symbols for re-use.
          Re-using symbols will result in faster symbolization times, but the
          provided directory *must* be unique per browser binary, e.g. by
          including the hash of the binary in the directory name.
    """
    super().__init__(
        os_name, arch_name, dump_finder, build_dir, symbols_dir=symbols_dir)

  def SymbolizeMinidump(self, minidump):
    """Gets the stack trace from the given minidump.

    Args:
      minidump: the path to the minidump on disk

    Returns:
      None if the stack could not be retrieved for some reason, otherwise a
      string containing the stack trace.
    """
    if self._os_name == 'win':
      cdb = self._GetCdbPath()
      if not cdb:
        logging.warning('cdb.exe not found.')
        return None
      # Move to the thread which triggered the exception (".ecxr"). Then include
      # a description of the exception (".lastevent"). Also include all the
      # threads' stacks ("~*kb30") as well as the ostensibly crashed stack
      # associated with the exception context record ("kb30"). Note that stack
      # dumps, including that for the crashed thread, may not be as precise as
      # the one starting from the exception context record.
      # Specify kb instead of k in order to get four arguments listed, for
      # easier diagnosis from stacks.
      output = subprocess.check_output([cdb, '-y', self._build_dir,
                                        '-c', '.ecxr;.lastevent;kb30;~*kb30;q',
                                        '-z', minidump])
      if six.PY3:
        output = output.decode('utf-8')

      # The output we care about starts with "Last event:" or possibly
      # other things we haven't seen yet. If we can't find the start of the
      # last event entry, include output from the beginning.
      info_start = 0
      info_start_match = re.search("Last event:", output, re.MULTILINE)
      if info_start_match:
        info_start = info_start_match.start()
      info_end = output.find('quit:')
      return output[info_start:info_end]
    return super().SymbolizeMinidump(minidump)

  def GetSymbolBinaries(self, minidump):
    """Returns a list of paths to binaries where symbols may be located.

    Args:
      minidump: The path to the minidump being symbolized.
    """
    minidump_dump = local_first_binary_manager.GetInstance().FetchPath(
        'minidump_dump')
    assert minidump_dump

    symbol_binaries = []

    minidump_cmd = [minidump_dump, minidump]
    try:
      with open(os.devnull, 'wb') as dev_null:
        minidump_output = subprocess.check_output(minidump_cmd, stderr=dev_null)
    except subprocess.CalledProcessError as e:
      # For some reason minidump_dump always fails despite successful dumping.
      minidump_output = e.output

    if six.PY3:
      minidump_output = minidump_output.decode('utf-8')

    minidump_binary_re = re.compile(r'\W+\(code_file\)\W+=\W\"(.*)\"')
    for minidump_line in minidump_output.splitlines():
      line_match = minidump_binary_re.match(minidump_line)
      if line_match:
        binary_path = line_match.group(1)
        if not os.path.isfile(binary_path):
          continue

        # Filter out system binaries.
        if (binary_path.startswith('/usr/lib/') or
            binary_path.startswith('/System/Library/') or
            binary_path.startswith('/lib/')):
          continue

        # Filter out other binary file types which have no symbols.
        if (binary_path.endswith('.pak') or
            binary_path.endswith('.bin') or
            binary_path.endswith('.dat') or
            binary_path.endswith('.ttf')):
          continue

        symbol_binaries.append(binary_path)
    return self._FilterSymbolBinaries(symbol_binaries)

  def _FilterSymbolBinaries(self, symbol_binaries):
    """Filters out unnecessary symbol binaries to save symbolization time.

    Args:
      symbol_binaries: A list of paths to binaries that will have their
          symbols dumped.

    Returns:
      A copy of |symbol_binaries| with any unnecessary paths removed.
    """
    if self._os_name == 'mac':
      # The vast majority of the symbol binaries for component builds on Mac
      # are .dylib, and none of them appear to contribute any additional
      # information. So, remove them to save a *lot* of time.
      # Do process dylibs that aren't component build dylibs though.
      bundled_dylib_re = re.compile(
          r'Framework\.framework/Versions/\d+.\d+.\d+.\d+/Libraries/.*\.dylib')
      filtered_binaries = []
      for binary in symbol_binaries:
        if not binary.endswith('.dylib') or bundled_dylib_re.search(binary):
          filtered_binaries.append(binary)
      symbol_binaries = filtered_binaries
    return symbol_binaries

  def _GetCdbPath(self):
    # cdb.exe might have been co-located with the browser's executable
    # during the build, but that's not a certainty. (This is only done
    # in Chromium builds on the bots, which is why it's not a hard
    # requirement.) See if it's available.
    colocated_cdb = os.path.join(self._build_dir, 'cdb', 'cdb.exe')
    if path.IsExecutable(colocated_cdb):
      return colocated_cdb
    possible_paths = (
        # Installed copies of the Windows SDK.
        os.path.join('Windows Kits', '*', 'Debuggers', 'x86'),
        os.path.join('Windows Kits', '*', 'Debuggers', 'x64'),
        # Old copies of the Debugging Tools for Windows.
        'Debugging Tools For Windows',
        'Debugging Tools For Windows (x86)',
        'Debugging Tools For Windows (x64)',
        # The hermetic copy of the Windows toolchain in depot_tools.
        os.path.join('win_toolchain', 'vs_files', '*', 'win_sdk',
                     'Debuggers', 'x86'),
        os.path.join('win_toolchain', 'vs_files', '*', 'win_sdk',
                     'Debuggers', 'x64'),
    )
    for possible_path in possible_paths:
      app_path = os.path.join(possible_path, 'cdb.exe')
      app_path = path.FindInstalledWindowsApplication(app_path)
      if app_path:
        return app_path
    return None
