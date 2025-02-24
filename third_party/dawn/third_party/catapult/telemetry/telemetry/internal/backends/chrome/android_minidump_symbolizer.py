# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import datetime
import logging
import os
import platform
import re
import subprocess
import six

from telemetry.internal.backends.chrome import minidump_symbolizer
from telemetry.internal.results import artifact_logger
from telemetry.internal.util import local_first_binary_manager


# Directories relative to the build directory that may contain symbol binaries
# that can be dumped to symbolize a minidump.
_POSSIBLE_SYMBOL_BINARY_DIRECTORIES = [
    'lib.unstripped',
    os.path.join('android_clang_arm', 'lib.unstripped'),
    os.path.join('android_clang_arm64', 'lib.unstripped'),
]

# Mappings from Crashpad/Breakpad processor architecture values to regular
# expressions that will match the output of running "file" on a .so compiled
# for that architecture.
# The Breakpad processor architecture values are hex representations of the
# values in MDCPUArchitecture from Breakpad's minidump_format.h.
_BREAKPAD_ARCH_TO_FILE_REGEX = {
    # 32-bit x86 (emulators).
    '0x0': r'.*32-bit.*Intel.*',
    # 32-bit ARM.
    '0x5': r'.*32-bit.*ARM.*',
    # 64-bit ARM.
    '0xc': r'.*64-bit.*ARM.*',
}

# Line looks like " processor_architecture = 0xc ".
_PROCESSOR_ARCH_REGEX = r'\s*processor_architecture\s*\=\s*(?P<arch>\w*)\s*'


class AndroidMinidumpSymbolizer(minidump_symbolizer.MinidumpSymbolizer):
  def __init__(self, dump_finder, build_dir, symbols_dir=None):
    """Class for handling all minidump symbolizing code on Android.

    Args:
      dump_finder: The minidump_finder.MinidumpFinder instance that is being
          used to find minidumps for the test.
      build_dir: The directory containing Chromium build artifacts to generate
          symbols from.
      symbols_dir: An optional path to a directory to store symbols for re-use.
          Re-using symbols will result in faster symbolization times, but the
          provided directory *must* be unique per browser binary, e.g. by
          including the hash of the binary in the directory name.
    """
    # Map from minidump path (string) to minidump_dump output (string).
    self._minidump_dump_output = {}
    # Map from minidump path (string) to the directory that should be used when
    # looking for symbol binaries (string).
    self._minidump_symbol_binaries_directories = {}
    # We use the OS/arch of the host, not the device.
    super().__init__(
        platform.system().lower(), platform.machine(), dump_finder, build_dir,
        symbols_dir=symbols_dir)

  def SymbolizeMinidump(self, minidump):
    if platform.system() != 'Linux':
      logging.warning(
          'Cannot get Android stack traces unless running on a Posix host.')
      return None
    if not self._build_dir:
      logging.warning(
          'Cannot get Android stack traces without build directory.')
      return None
    return super().SymbolizeMinidump(minidump)

  def GetSymbolBinaries(self, minidump):
    """Returns a list of paths to binaries where symbols may be located.

    Args:
      minidump: The path to the minidump being symbolized.
    """
    libraries = self._ExtractLibraryNamesFromDump(minidump)
    symbol_binary_dir = self._GetSymbolBinaryDirectory(minidump, libraries)
    if not symbol_binary_dir:
      return []

    return [os.path.join(symbol_binary_dir, lib) for lib in libraries]

  def GetBreakpadPlatformOverride(self):
    return 'android'

  def _ExtractLibraryNamesFromDump(self, minidump):
    """Extracts library names that may contain symbols from the minidump.

    This is a duplicate of the logic in Chromium's
    //build/android/stacktrace/crashpad_stackwalker.py.

    Returns:
      A list of strings containing library names of interest for symbols.
    """
    default_library_name = 'libmonochrome.so'

    minidump_dump_output = self._GetMinidumpDumpOutput(minidump)
    if not minidump_dump_output:
      logging.warning(
          'Could not get minidump_dump output, defaulting to library %s',
          default_library_name)
      return [default_library_name]

    library_names = []
    module_library_line_re = re.compile(r'[(]code_file[)]\s+= '
                                        r'"(?P<library_name>lib[^. ]+.so)"')
    in_module = False
    for line in minidump_dump_output.splitlines():
      line = line.lstrip().rstrip('\n')
      if line == 'MDRawModule':
        in_module = True
        continue
      if line == '':
        in_module = False
        continue
      if in_module:
        m = module_library_line_re.match(line)
        if m:
          library_names.append(m.group('library_name'))
    if not library_names:
      logging.warning(
          'Could not find any library name in the dump, '
          'default to: %s', default_library_name)
      return [default_library_name]
    return library_names

  def _GetSymbolBinaryDirectory(self, minidump, libraries):
    """Gets the directory that should contain symbol binaries for |minidump|.

    Args:
      minidump: The path to the minidump being analyzed.
      libraries: A list of library names that are within the minidump.

    Returns:
      A string containing the path to the directory that should contain the
      symbol binaries that can be dumped to symbolize |minidump|. Returns None
      if the directory is unable to be determined for some reason.
    """
    if minidump in self._minidump_symbol_binaries_directories:
      return self._minidump_symbol_binaries_directories[minidump]

    # Get the processor architecture reported by the minidump.
    arch = None
    matcher = re.compile(_PROCESSOR_ARCH_REGEX)
    for line in self._GetMinidumpDumpOutput(minidump).splitlines():
      match = matcher.match(line)
      if match:
        arch = match.groupdict()['arch'].lower()
        break
    if not arch:
      logging.error('Unable to find processor architecture for minidump %s',
                    minidump)
      self._minidump_symbol_binaries_directories[minidump] = None
      return None
    if arch not in _BREAKPAD_ARCH_TO_FILE_REGEX:
      logging.error(
          'Unsupported processor architecture %s for minidump %s. This is '
          'likely fixable by adding the correct mapping for the architecture '
          'in android_minidump_symbolizer._BREAKPAD_ARCH_TO_FILE_REGEX.',
          arch, minidump)
      self._minidump_symbol_binaries_directories[minidump] = None
      return None

    # Look for a directory that contains binaries with the correct architecture.
    matcher = re.compile(_BREAKPAD_ARCH_TO_FILE_REGEX[arch])
    symbol_dir = None
    for symbol_subdir in _POSSIBLE_SYMBOL_BINARY_DIRECTORIES:
      possible_symbol_dir = os.path.join(self._build_dir, symbol_subdir)
      if not os.path.exists(possible_symbol_dir):
        continue
      for f in os.listdir(possible_symbol_dir):
        if f not in libraries:
          continue
        binary_path = os.path.join(possible_symbol_dir, f)
        stdout = six.ensure_str(
            subprocess.check_output(
                ['file', binary_path], stderr=subprocess.STDOUT))
        if matcher.match(stdout):
          symbol_dir = possible_symbol_dir
          break

    if not symbol_dir:
      logging.error(
          'Unable to find suitable symbol binary directory for architecture %s.'
          'This is likely fixable by adding the correct directory to '
          'android_minidump_symbolizer._POSSIBLE_SYMBOL_BINARY_DIRECTORIES.',
          arch)
    self._minidump_symbol_binaries_directories[minidump] = symbol_dir
    return symbol_dir

  def _GetMinidumpDumpOutput(self, minidump):
    """Runs minidump_dump on the given minidump.

    Caches the result for re-use.

    Args:
      minidump: The path to the minidump being analyzed.

    Returns:
      A string containing the output of minidump_dump, or None if it could not
      be retrieved for some reason.
    """
    if minidump in self._minidump_dump_output:
      logging.debug('Returning cached minidump_dump output for %s', minidump)
      return self._minidump_dump_output[minidump]

    dumper_path = local_first_binary_manager.GetInstance().FetchPath(
        'minidump_dump')
    if not os.access(dumper_path, os.X_OK):
      logging.warning('Cannot run minidump_dump because %s is not found.',
                      dumper_path)
      return None

    # Using subprocess.check_output with stdout/stderr mixed can result in
    # errors due to log messages showing up in the minidump_dump output. So,
    # use Popen and combine into a single string afterwards.
    p = subprocess.Popen(
        [dumper_path, minidump], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()
    stdout = six.ensure_str(stdout) + '\n' + six.ensure_str(stderr)

    if p.returncode != 0:
      # Dumper errors often do not affect stack walkability, just a warning.
      # It's possible for the same stack to be symbolized multiple times, so
      # add a timestamp suffix to prevent artifact collisions.
      now = datetime.datetime.now()
      suffix = now.strftime('%Y-%m-%d-%H-%M-%S')
      artifact_name = 'dumper_errors/%s-%s' % (
          os.path.basename(minidump), suffix)
      logging.warning(
          'Reading minidump failed, but likely not actually an issue. Saving '
          'output to artifact %s', artifact_name)
      artifact_logger.CreateArtifact(artifact_name, stdout)
    if stdout:
      self._minidump_dump_output[minidump] = stdout
    return stdout
