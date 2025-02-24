# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
import pipes


def ShellFormat(command, trim=False):
  """Format the command for easy copy/pasting into a shell.

  Args:
    command: a list of the executable and its arguments.
    trim: Trim finch and feature flags from a command to reduce its length.

  Returns:
    A string.
  """
  if trim:
    # Copy the command since trimming is done in-place.
    command = list(command)
    _Trim(command)
  quoted_command = [_ShellQuote(part) for part in command]
  return ' '.join(quoted_command)


def _Trim(command):
  """Shorten long arguments out of a command.

  This is useful for keeping logs short to reduce Swarming UI load time
  and prevent us from getting truncated. See crbug.com/943650.
  """
  ARGUMENTS_TO_SHORTEN = (
      '--force-fieldtrial-params=',
      '--force-fieldtrials=',
      '--disable-features=',
      '--enable-features=')
  for index, part in enumerate(command):
    for substring in ARGUMENTS_TO_SHORTEN:
      if part.startswith(substring):
        command[index] = substring + '...'
        break


def _ShellQuote(command_part):
  """Escape a part of a command to enable copy/pasting it into a shell.
  """
  return pipes.quote(command_part)
