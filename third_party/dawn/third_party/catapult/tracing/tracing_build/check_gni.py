# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import re

from tracing_build import check_common

_TRACING_BUILD = os.path.abspath(os.path.dirname(__file__))
GNI_FILE = os.path.join(_TRACING_BUILD, '..', 'trace_viewer.gni')


def GniCheck():
  gni = open(GNI_FILE).read()
  listed_files = []
  error = ''
  for group in check_common.FILE_GROUPS:
    filenames = re.search(r'\n%s = \[([^\]]+)\]' % group, gni).groups(1)[0]
    filenames = re.findall(r'"([^"]+)"', filenames)
    filenames = [os.path.normpath(filename) for filename in filenames]
    error += check_common.CheckListedFilesSorted(GNI_FILE, group, filenames)
    listed_files.extend(filenames)

  return error + check_common.CheckCommon(GNI_FILE, listed_files)


if __name__ == '__main__':
  print(GniCheck())
