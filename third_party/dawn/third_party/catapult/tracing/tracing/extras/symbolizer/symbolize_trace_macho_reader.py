# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import re
import subprocess
import six
from six.moves import range  # pylint: disable=redefined-builtin


def ReadMachOTextLoadAddress(file_name):
  """
  This function returns the load address of the TEXT segment of a Mach-O file.
  """
  regex = re.compile(r".* vmaddr 0x([\dabcdef]*)")
  cmd = ["otool", "-l", "-m", file_name]
  output = six.ensure_str(subprocess.check_output(cmd)).split('\n')
  for i in range(len(output) - 3):
    # It's possible to use a regex here instead, but these conditionals are much
    # clearer.
    if ("cmd LC_SEGMENT_64" in output[i] and
        "cmdsize" in output[i + 1] and
        "segname __TEXT" in output[i + 2] and
        "vmaddr" in output[i + 3]):
      result = regex.match(output[i + 3])
      assert result
      return int(result.group(1), 16)
  return None
