# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import print_function


from __future__ import absolute_import
import argparse


def indent(text, prefix):
  """Prefix each line in text by a given prefix."""
  return ''.join(prefix + line for line in text.splitlines(True))


class MixedHelpAction(argparse.Action):
  """An argparse.Action to display help of multiple connected parsers."""
  def __init__(self, option_strings, dest=argparse.SUPPRESS,
               legacy_parser=None):
    super().__init__(
        option_strings=option_strings, dest=dest, default=argparse.SUPPRESS,
        nargs=0, help=argparse.SUPPRESS)
    assert legacy_parser, 'missing required argument: legacy_parser'
    self.legacy_parser = legacy_parser

  def __call__(self, parser, *args, **kwargs):
    self.legacy_parser.print_help()
    extra_help = parser.format_help().rstrip()
    if extra_help:
      print()
      print(indent(extra_help, '  '))
    parser.exit()
