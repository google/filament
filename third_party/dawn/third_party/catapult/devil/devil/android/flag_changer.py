# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import contextlib
import logging
import posixpath
import re

from devil.android.sdk import version_codes

logger = logging.getLogger(__name__)

_CMDLINE_DIR = '/data/local/tmp'
_CMDLINE_DIR_LEGACY = '/data/local'
_RE_NEEDS_QUOTING = re.compile(r'[^\w-]')  # Not in: alphanumeric or hyphens.
_QUOTES = '"\''  # Either a single or a double quote.
_ESCAPE = '\\'  # A backslash.


@contextlib.contextmanager
def CustomCommandLineFlags(device, cmdline_name, flags):
  """Context manager to change Chrome's command line temporarily.

  Example:

      with flag_changer.CustomCommandLineFlags(device, name, flags):
        # Launching Chrome will use the provided flags.

      # Previous set of flags on the device is now restored.

  Args:
    device: A DeviceUtils instance.
    cmdline_name: Name of the command line file where to store flags.
    flags: A sequence of command line flags to set.
  """
  changer = FlagChanger(device, cmdline_name)
  try:
    changer.ReplaceFlags(flags)
    yield
  finally:
    changer.Restore()


class FlagChanger(object):
  """Changes the flags Chrome runs with.

    Flags can be temporarily set for a particular set of unit tests.  These
    tests should call Restore() to revert the flags to their original state
    once the tests have completed.
  """

  def __init__(self, device, cmdline_file, use_legacy_path=False):
    """Initializes the FlagChanger and records the original arguments.

    Args:
      device: A DeviceUtils instance.
      cmdline_file: Name of the command line file where to store flags.
      use_legacy_path: Whether to use the legacy commandline path (needed for
        M54 and earlier)
    """
    self._device = device
    self._should_reset_enforce = False

    if posixpath.sep in cmdline_file:
      raise ValueError(
          'cmdline_file should be a file name only, do not include path'
          ' separators in: %s' % cmdline_file)
    cmdline_path = posixpath.join(_CMDLINE_DIR, cmdline_file)
    alternate_cmdline_path = posixpath.join(_CMDLINE_DIR_LEGACY, cmdline_file)

    if use_legacy_path:
      cmdline_path, alternate_cmdline_path = (alternate_cmdline_path,
                                              cmdline_path)
      if not self._device.HasRoot():
        raise ValueError('use_legacy_path requires a rooted device')
    self._cmdline_path = cmdline_path

    if self._device.PathExists(alternate_cmdline_path):
      logger.warning('Removing alternate command line file %r.',
                     alternate_cmdline_path)
      self._device.RemovePath(alternate_cmdline_path, as_root=True)

    self._state_stack = [None]  # Actual state is set by GetCurrentFlags().
    self.GetCurrentFlags()

  def GetCurrentFlags(self):
    """Read the current flags currently stored in the device.

    Also updates the internal state of the flag_changer.

    Returns:
      A list of flags.
    """
    if self._device.PathExists(self._cmdline_path):
      command_line = self._device.ReadFile(
          self._cmdline_path, as_root=True).strip()
    else:
      command_line = ''
    flags = _ParseFlags(command_line)

    # Store the flags as a set to facilitate adding and removing flags.
    self._state_stack[-1] = set(flags)
    return flags

  def ReplaceFlags(self, flags, log_flags=True):
    """Replaces the flags in the command line with the ones provided.
       Saves the current flags state on the stack, so a call to Restore will
       change the state back to the one preceeding the call to ReplaceFlags.

    Args:
      flags: A sequence of command line flags to set, eg. ['--single-process'].
             Note: this should include flags only, not the name of a command
             to run (ie. there is no need to start the sequence with 'chrome').

    Returns:
      A list with the flags now stored on the device.
    """
    new_flags = set(flags)
    self._state_stack.append(new_flags)
    self._SetPermissive()
    return self._UpdateCommandLineFile(log_flags=log_flags)

  def AddFlags(self, flags):
    """Appends flags to the command line if they aren't already there.
       Saves the current flags state on the stack, so a call to Restore will
       change the state back to the one preceeding the call to AddFlags.

    Args:
      flags: A sequence of flags to add on, eg. ['--single-process'].

    Returns:
      A list with the flags now stored on the device.
    """
    return self.PushFlags(add=flags)

  def RemoveFlags(self, flags):
    """Removes flags from the command line, if they exist.
       Saves the current flags state on the stack, so a call to Restore will
       change the state back to the one preceeding the call to RemoveFlags.

       Note that calling RemoveFlags after AddFlags will result in having
       two nested states.

    Args:
      flags: A sequence of flags to remove, eg. ['--single-process'].  Note
             that we expect a complete match when removing flags; if you want
             to remove a switch with a value, you must use the exact string
             used to add it in the first place.

    Returns:
      A list with the flags now stored on the device.
    """
    return self.PushFlags(remove=flags)

  def PushFlags(self, add=None, remove=None):
    """Appends and removes flags to/from the command line if they aren't already
       there. Saves the current flags state on the stack, so a call to Restore
       will change the state back to the one preceeding the call to PushFlags.

    Args:
      add: A list of flags to add on, eg. ['--single-process'].
      remove: A list of flags to remove, eg. ['--single-process'].  Note that we
              expect a complete match when removing flags; if you want to remove
              a switch with a value, you must use the exact string used to add
              it in the first place.

    Returns:
      A list with the flags now stored on the device.
    """
    new_flags = self._state_stack[-1].copy()
    if add:
      new_flags.update(add)
    if remove:
      new_flags.difference_update(remove)
    return self.ReplaceFlags(new_flags)

  def _SetPermissive(self):
    """Set SELinux to permissive, if needed.

    On Android N and above this is needed in order to allow Chrome to read the
    legacy command line file.

    TODO(crbug.com/699082): Remove when a better solution exists.
    """
    # TODO(crbug.com/948578): figure out the exact scenarios where the lowered
    # permissions are needed, and document them in the code.
    if not self._device.HasRoot():
      return
    if (self._device.build_version_sdk >= version_codes.NOUGAT
        and self._device.GetEnforce()):
      self._device.SetEnforce(enabled=False)
      self._should_reset_enforce = True

  def _ResetEnforce(self):
    """Restore SELinux policy if it had been previously made permissive."""
    if self._should_reset_enforce:
      self._device.SetEnforce(enabled=True)
      self._should_reset_enforce = False

  def Restore(self):
    """Restores the flags to their state prior to the last AddFlags or
       RemoveFlags call.

    Returns:
      A list with the flags now stored on the device.
    """
    # The initial state must always remain on the stack.
    assert len(self._state_stack) > 1, (
        'Mismatch between calls to Add/RemoveFlags and Restore')
    self._state_stack.pop()
    if len(self._state_stack) == 1:
      self._ResetEnforce()
    return self._UpdateCommandLineFile()

  def _UpdateCommandLineFile(self, log_flags=True):
    """Writes out the command line to the file, or removes it if empty.

    Returns:
      A list with the flags now stored on the device.
    """
    command_line = _SerializeFlags(self._state_stack[-1])
    if command_line is not None:
      self._device.WriteFile(self._cmdline_path, command_line, as_root=True)
    else:
      self._device.RemovePath(self._cmdline_path, force=True, as_root=True)

    flags = self.GetCurrentFlags()
    logging.info('Flags now written on the device to %s', self._cmdline_path)
    if log_flags:
      logging.info('Flags: %s', flags)
    return flags


def _ParseFlags(line):
  """Parse the string containing the command line into a list of flags.

  It's a direct port of CommandLine.java::tokenizeQuotedArguments.

  The first token is assumed to be the (unused) program name and stripped off
  from the list of flags.

  Args:
    line: A string containing the entire command line.  The first token is
          assumed to be the program name.

  Returns:
     A list of flags, with quoting removed.
  """
  flags = []
  current_quote = None
  current_flag = None

  # pylint: disable=unsubscriptable-object
  for c in line:
    # Detect start or end of quote block.
    if (current_quote is None and c in _QUOTES) or c == current_quote:
      if current_flag is not None and current_flag[-1] == _ESCAPE:
        # Last char was a backslash; pop it, and treat c as a literal.
        current_flag = current_flag[:-1] + c
      else:
        current_quote = c if current_quote is None else None
    elif current_quote is None and c.isspace():
      if current_flag is not None:
        flags.append(current_flag)
        current_flag = None
    else:
      if current_flag is None:
        current_flag = ''
      current_flag += c

  if current_flag is not None:
    if current_quote is not None:
      logger.warning('Unterminated quoted argument: %s', current_flag)
    flags.append(current_flag)

  # Return everything but the program name.
  return flags[1:]


def _SerializeFlags(flags):
  """Serialize a sequence of flags into a command line string.

  Args:
    flags: A sequence of strings with individual flags.

  Returns:
    A line with the command line contents to save; or None if the sequence of
    flags is empty.
  """
  if flags:
    # The first command line argument doesn't matter as we are not actually
    # launching the chrome executable using this command line.
    args = ['_']
    args.extend(_QuoteFlag(f) for f in flags)
    return ' '.join(args)

  return None


def _QuoteFlag(flag):
  """Validate and quote a single flag.

  Args:
    A string with the flag to quote.

  Returns:
    A string with the flag quoted so that it can be parsed by the algorithm
    in _ParseFlags; or None if the flag does not appear to be valid.
  """
  if '=' in flag:
    key, value = flag.split('=', 1)
  else:
    key, value = flag, None

  if not flag or _RE_NEEDS_QUOTING.search(key):
    # Probably not a valid flag, but quote the whole thing so it can be
    # parsed back correctly.
    return '"%s"' % flag.replace('"', r'\"')

  if value is None:
    return key

  if _RE_NEEDS_QUOTING.search(value):
    value = '"%s"' % value.replace('"', r'\"')
  return '='.join([key, value])
