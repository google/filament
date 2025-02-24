# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import logging
import os


def GetRecursiveDiskUsage(path):
  """Returns the disk usage in bytes of |path|. Similar to `du -sb |path|`."""

  def get_size(filepath):
    try:
      return os.path.getsize(filepath)
    except OSError:
      logging.warning('File or directory no longer found: %s', filepath)
      return 0

  running_size = get_size(path)
  if os.path.isdir(path):
    for root, dirs, files in os.walk(path):
      running_size += sum(
          [get_size(os.path.join(root, f)) for f in files + dirs])
  return running_size
