# Copyright 2024 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Code to handle the optparse -> argparse migration.

Once all Telemetry and Telemetry-dependent code switches to using these wrappers
instead of directly using optparse, incremental changes can be made to move the
underlying implementation from optparse to argparse before finally switching
directly to argparse.
"""

import argparse


class ArgumentParser(argparse.ArgumentParser):

  def __init__(self, *args, **kwargs):
    super().__init__(*args, **kwargs)
    # parse_args behavior differs between optparse and argparse, so store a
    # reference to the original implementation now before we override it later.
    self.argparse_parse_args = self.parse_args

  def parse_args(self, args=None, namespace=None):
    """optparse-like override of argparse's parse_args."""
    # optparse's parse_args only parses flags that start with -- or -.
    # Positional args are returned as-is as a list of strings. For now, assert
    # that we're using this wrapper like optparse (no positional arguments
    # defined) and ensure that unknown args are all positional arguments.
    # All uses of positional args will have to be updated at the same time this
    # overridden parse_args is removed.
    for action in self._actions:
      if not action.option_strings:
        self.error('Tried to parse a defined positional argument. This is not '
                   'supported until former uses of optparse are migrated to '
                   'argparse')
    known_args, unknown_args = self.parse_known_args(args, namespace)
    for unknown in unknown_args:
      if unknown.startswith('-'):
        self.error(f'no such option: {unknown}')
    return known_args, unknown_args

  @property
  def defaults(self):
    return self._defaults


def CreateFromOptparseInputs(usage=None, description=None):
  """Creates an ArgumentParser using the same constructor arguments as optparse.

  See Python's optparse.OptionParser documentation for argument descriptions.
  The following args have been omitted since they do not appear to be used in
  Telemetry, but can be added later if necessary.
    * option_list
    * option_class
    * version
    * conflict_handler
    * formatter
    * add_help_option
    * prog
    * epilog
  """
  usage = usage or '%prog [options]'
  usage = usage.replace('%prog', '%(prog)s')
  return ArgumentParser(usage=usage, description=description)
